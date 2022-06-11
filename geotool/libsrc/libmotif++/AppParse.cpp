/** \file AppParse.cpp
 *  \brief Defines class AppParse.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "motif++/AppParse.h"
#include "motif++/FormDialog.h"
#include "motif++/MotifClasses.h"
#include "widget/TableListener.h"
#include "motif++/IPCClient.h"
#include <X11/Xmu/Editres.h>


extern "C" {
#include "libtime.h"
#include "libstring.h"
static void * ReadInput(void *client_data);
}
//#define DEBUG_APP 

static void parseCmdCB(XtPointer data, XtIntervalId *id);
static void waitCB(XtPointer data, XtIntervalId *id);
static bool isForeach(char *line);
static bool isEndfor(char *line);
static bool parseToDouble(const char *s, double *d);
static void getWindowPrefix(char *line, char *prefix, int prefix_size);
static void applyPrefix(char *sline, int sline_size, char *line, int line_size,
			const char *prefix);
static void checkForeachPrefix(Foreach *fr, char *line, int line_size);

static AppParse *theAppParse = NULL;
static bool first_line=true;

AppParse::AppParse(const string &name, Component *parent, int *argcp,
		const char **argv, const string &installation_dir)
		: Component(name, parent)
{
    const char *c, *rname;
    char *var;
    struct stat buf;
    int i, n;

    if( theAppParse ) {
	cerr << "AppParse: cannot create a second AppParse instance." << endl;
	exit(1);
    }
    if(!installation_dir.empty()) install_dir.assign(installation_dir);

    nargcp = (argcp) ? *argcp : 0;
    arg_v = argv;

    // get the actual program name to use in X-resource lines.
    if(nargcp > 0 && arg_v) {
	c = (nargcp > 0) ? arg_v[0] : "";
	for(i = (int)strlen(c)-1; i >= 0 && c[i] != '/'; i--);
	rname = (i >= 0) ? c + i+1 : c;
	resource_name.assign(rname);
    }

    parse_only = false;
    redirect_warnings = false;

    getScriptFiles();

    if(nargcp > 0 && arg_v) {
	for(i = 1; i < nargcp; i++) {
	    if(!strcmp(argv[i], "-i")) {
		parse_only = true;
		redirect_warnings = true;
	    }
	    else if(!strncmp(argv[i], "parse=", 6)) {
		command_line_parse_string.push_back(strdup(argv[i]+6));
	    }
	    else { // other assignments become global variables
		for(c = argv[i]; *c != '\0' && *c != '"' && *c != '\''
		    && *c != '`' && *c != '='; c++);
		if(*c == '=' && c > argv[i] && *(c+1) != '\0') {
		    n = c - argv[i];
		    var = (char *)malloc(n+1);
		    strncpy(var, argv[i], n);
		    var[n] = '\0';
		    putGlobalVariable(var, c+1);
		    free(var);
		}
	    }
	}
    }
    application_class.assign(name);

    base_widget = XtVaAppInitialize(&app_context, application_class.c_str(),
			NULL, 0, &nargcp, (char **)argv, NULL,
			XmNdeleteResponse, XmDO_NOTHING, NULL);
    *argcp = nargcp;

    installDestroyHandler();

    // Center the shell and make sure it isn't visible
    XtVaSetValues(base_widget,
		XmNmappedWhenManaged, False,
		XmNx, DisplayWidth(XtDisplay(base_widget), 0)/2,
		XmNy, DisplayHeight(XtDisplay(base_widget), 0)/2,
		XmNwidth, 1,
		XmNheight, 1,
		NULL);

    // Force the shell window to exist so dialogs popped up from this shell
    // behave correctly

    XtRealizeWidget(base_widget);

    theAppParse = this;

    file_states.push_back(new FileState());
    start_fs = NULL;

    /* Use a thread to read from stdin. An alternative is to use
     * XtAppAddInput(app_context, 0, (XtPointer)XtInputReadMask,
     *          (XtInputCallbackProc)parseCmdCB, (XtPointer)this);
     * but this causes the stdin to stop using line buffering on some systems.
     * The backspace and other such keys do not work in this case.
     */

    bool sem_ok = true;
    if( sem_init(&sem_stdin, 0, 0) ) {
	fprintf(stderr, "sem_init failed: %s\n", strerror(errno));
	thread = (pthread_t)NULL;
	sem_ok = false;
    }
    if( sem_init(&sem_work, 0, 0) ) {
	fprintf(stderr, "sem_init failed: %s\n", strerror(errno));
	thread = (pthread_t)NULL;
	sem_ok = false;
    }

    standard_input = (fstat(0, &buf) || !S_ISREG(buf.st_mode)) ? 1 : 0;
    if(standard_input) putGlobalVariable("standard_input", "true");
    else putGlobalVariable("standard_input", "false");

    print_fp = NULL;
    write_fp = NULL;
    get_line_fp = NULL;
    thread = (pthread_t)NULL;
    input_ready = false;
    memset(parse_line, 0, sizeof(parse_line));
    modified_table = NULL;

    if(parse_only) putGlobalVariable("parse_only", "true");
    else putGlobalVariable("parse_only", "false");

    addReservedName("for_index");
    addReservedName("standard_input");
    addReservedName("parse_only");
    addReservedName("pi");
    addReservedName("degrees_to_radians");
    addReservedName("radians_to_degrees");
    addReservedName("degrees_to_km");
    addReservedName("km_to_degrees");
    addReservedName("true");
    addReservedName("false");

    // some predefined aliases
    setAlias("alias tv=tableviewer");
    setAlias("alias tq=tablequery");
    setAlias("alias fkm=fk_multi_band");
    setAlias("alias ir=instrument_response");

    initCreateMethods();

    if(sem_ok) {
	// Start the thread.
	if(pthread_create(&thread, NULL, ReadInput, (void *)this)) {
	    fprintf(stderr, "pthread_create failed.\n");
	}
	XtAppAddTimeOut(app_context, 100, parseCmdCB, (XtPointer)this);
    }
}

// cannot actually copy the AppParse instance. This is just to avoid
// compiler warnings from the -Weffc++ option, from AppParse.h
AppParse::AppParse(const AppParse &a) : Component(NULL, NULL)
{
}

/** Get the AppParse instance.
 */
AppParse *AppParse::getAppParse(void)
{
    if(!theAppParse) {
	theAppParse = new AppParse("appParse", NULL, NULL, NULL, NULL);
    }
    return theAppParse;
}

AppParse::~AppParse(void)
{
    int i;

    for(i = 0; i < (int)command_line_parse_string.size(); i++) {
        free(command_line_parse_string[i]);
    }

    for(i = 0; i < (int)file_states.size(); i++) {
	delete file_states[i];
    }
    file_states.clear();
}

void AppParse::getScriptFiles(void)
{
    char *home, dir[MAXPATHLEN+1], path[MAXPATHLEN+100];
    struct dirent *dp;
    DIR *dirp;

    // get all files in the install_dir/scripts/init directory
    snprintf(dir, sizeof(dir), "%s/scripts/init", install_dir.c_str());
    if( (dirp = opendir(dir)) ) {
	for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		if(dp->d_name[0] != '.')
	{
	    snprintf(path, sizeof(path), "parse %s/%s", dir, dp->d_name);
	    command_line_parse_string.push_back(strdup(path));
	}
	closedir(dirp);
    }

    // get all files in the $HOME/.geotool++/scripts/init directory
    if((home = getenv("HOME")) != NULL && (int)strlen(home) > 0)
    {
	snprintf(dir, sizeof(dir), "%s/.geotool++/scripts/init", home);

	if( (dirp = opendir(dir)) ) {
	    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		if(dp->d_name[0] != '.')
	    {
		snprintf(path, sizeof(path), "parse %s/%s", dir, dp->d_name);
		command_line_parse_string.push_back(strdup(path));
	    }
	    closedir(dirp);
	}
    }
}

static void *
ReadInput(void *client_data)
{
    return AppParse::readInput(client_data);
}

void * AppParse::readInput(void *client_data)
{
    AppParse *app = (AppParse *)client_data;
    char line[2000];
    struct stat buf;

    for(int i = 0; i < (int)app->command_line_parse_string.size(); i++)
    {
	if( !stat(app->command_line_parse_string[i], &buf) ) {
	    snprintf(line, sizeof(line), "parse \"%s\"",
		app->command_line_parse_string[i]);
	}
	else {
	    snprintf(line, sizeof(line),"%s",app->command_line_parse_string[i]);
	}
	if(first_line) first_line = false;
	else sem_wait(&app->sem_stdin); // wait for access to app

	strcpy(app->parse_line, line);
	app->input_ready = true;
	// allow the other thread to run while this one gets the next input line
	sem_post(&app->sem_stdin);
	sem_wait(&app->sem_work);
    }

    if(first_line) { // need this or the interface will wait for stdin input
	first_line = false;
	strcpy(app->parse_line, "");
	app->input_ready = true;
	// allow the other thread to run while this one gets the next input line
	sem_post(&app->sem_stdin);
	sem_wait(&app->sem_work);
    }

#ifdef HAVE_READLINE
    if( app->standard_input ) using_history();
#endif

    for(;;)
    {
	if( app->getLine(stdin, line, sizeof(line)) )
	{
	    if(first_line) first_line = false;
	    else sem_wait(&app->sem_stdin); // wait for access to app

	    strcpy(app->parse_line, line);
	    app->input_ready = true;

	    // allow the other thread to run while this one gets the
	    // next input line
	    sem_post(&app->sem_stdin);
	    sem_wait(&app->sem_work);

	    if(app->stop_flag) {
		app->input_ready = false;
		break;
	    }
	}
    }
#ifdef HAVE_READLINE
    rl_cleanup_after_signal();
#endif
    return NULL;
}

void AppParse::stop(int code)
{
    if(print_fp) {
	fclose(print_fp);
	print_fp = NULL;
    }
    if(write_fp) {
	fclose(write_fp);
	write_fp = NULL;
    }
#ifdef HAVE_READLINE
    rl_cleanup_after_signal();
#endif
    cleanUp();

    XtDestroyApplicationContext(app_context);

    exit(code);
}

void AppParse::parseCmdCallback(void)
{
    char line[2000] = "";

    sem_wait(&sem_stdin); // wait for access to app.input_ready

    if(!stop_flag) {
	if(!input_ready) {
	    sem_post(&sem_stdin);
	    XtAppAddTimeOut(app_context, 100, parseCmdCB, (XtPointer)this);
	    return;
	}
	strncpy(line, parse_line, sizeof(line)-1);
	line[sizeof(line)-1] = '\0';
	input_ready = false;
	sem_post(&sem_stdin);
    }

    if( stop_flag || (int)comp_children.size() <= 0) {
	sem_post(&sem_work);
	stop(0);
    }

    if(line[0] != '\0') {
	string msg;
	parseLine(line, msg);
    }

    XtAppAddTimeOut(app_context, 100, parseCmdCB, (XtPointer)this);

    sem_post(&sem_work);
}

static void parseCmdCB(XtPointer data, XtIntervalId *id)
{
    AppParse *app = (AppParse *)data;
    app->parseCmdCallback();
}

bool AppParse::parseLine(const char *line, string &msg)
{
    char s[100000], quote = '\0';
    const char *b, *e;

    memset(s, 0, sizeof(s));

    b = line;
    while(*b != '\0') {
	e = b;
	while(*e != '\0' && *e != ';' && *e != '\n')
	{
	    if(*e == '"' || *e == '\'' || *e == '`' || *e == '{') {
		quote = (*e == '{') ? '}' : *e;
		e++;
		while(*e != '\0' && *e != quote) e++;
		if(*e != quote) {
		    printParseError("missing end quote: %c", quote);
		    return false;
		}
	    }
	    if(*e != '\0') e++;
	}
	strncpy(s, b, (int)(e-b));
	s[(int)(e-b)] = '\0';
	if( !parseCommand(s, sizeof(s), msg) ) return false;

	if(*e == ';' || *e == '\n') {
	    b = e+1;
	}
	else {
	    return true;
	}
    }
    return true;
}

bool AppParse::parseCommand(char *line, int line_size, string &msg)
{
    ParseCmd ret;
    ParseVar var;
    string c;
    char *file, sline[100000], prefix[500], *s;
    int i, if_cmd;
    FileState *fs = file_states.back();
    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    Component *top = comp_children[0];

    msg.clear();

    if(!strcasecmp(line, "parse_string")) {
	msg.assign("parse_string: missing argument");
	return false;
    }
    else if(!strncasecmp(line, "parse_string ", 13)) {
	// evaluate a string and parse it
	for(i = 13; line[i] != '\0' && isspace((int)line[i]); i++);
	if(line[i] == '\0') {
	    msg.assign("parse_string: missing argument");
	    return false;
	}
	s = NULL;
	if((var = getVariable(line+i, &s)) == VARIABLE_ERROR) {
	    return false;
	}
	else if(var == STRING_RETURNED) {
	    if(s[0] == '"') {
		s[(int)strlen(s)-1] = '\0'; // the ending '"'
		snprintf(line, line_size, "%s", s+1);
	    }
	    else {
		snprintf(line, line_size, "%s", s);
	    }
	}
	else {
	    snprintf(line, line_size, "%s", line+i);
	}
	Free(s);
    }

    for(i = (int)strlen(line)-1; i > 0 && line[i] != '#'; i--);
    if(line[i] == '#') line[i] = '\0';
    if(line[0] == '\0') return true;

    stringTrim(line);

    if(!checkLine(line, msg)) return false;

    // don't apply aliases to the unalias command
    if(strncasecmp(line, "unalias ", 8)) {
	applyAliases(line, line_size);
    }

    if(fr && (fr->mode == FOREACH_READ || fr->mode == FOREACH_SKIP) )
    {
	// collect inside foreach
#ifdef DEBUG_APP 
printf("read foreach line: %s\n", line);
#endif
	if( isEndfor(line) ) {
	    fs->foreach_blocks--;
#ifdef DEBUG_APP 
printf("foreach end: foreach_blocks = %d\n", fs->foreach_blocks);
#endif
	    if(fs->foreach_blocks == 0) {
		if(fs->if_blocks > 0) {
		    printParseError("missing endif.");
		    return false;
		}
		return foreachEnd(top, line, msg);
	    }
	}
	if( isForeach(line) ) {
	    fs->foreach_blocks++;
#ifdef DEBUG_APP 
printf("foreach_blocks = %d\n", fs->foreach_blocks);
#endif
	}
	else if(!strncasecmp(line, "if", 2)) {
	    for(i = 2; line[i] != '\0' && line[i] != '('; i++);
	    if(line[i] == '(') {
		fs->if_blocks++;
	    }
	}
	else if(!strcasecmp(line, "endif")) {
	    fs->if_blocks--;
	}
	struct line_struct st;
	st.line = strdup(line);
	st.line_number = ((int)parse_file.size() > 0) ?
		parse_file.back().line_number : 0;
	fr->lines.push_back(st);
    }
    else if((if_cmd = conditionalCmd(top, line, msg)) != 0) {
	if(if_cmd < 0) return false;
    }
    else if( fr && fr->mode == FOREACH_BREAK) {
	// in this mode skip all commands except the conditional controls
    }
    else if( isForeach(line) ) {
	fs->foreach_blocks = 1;
#ifdef DEBUG_APP 
printf("foreach command: %s\n", line);
#endif
	return foreachCmd(top, line, msg);
    }
    else if( isEndfor(line) ) {
	msg.assign("unexpected \"endfor\".");
	printParseError("unexpected \"endfor\".");
	return false;
    }
    else if(!strncasecmp(line, "alias ", 6)) {
	return setAlias(line);
    }
    else if(!strcasecmp(line, "alias")) {
	listAliases();
    }
    else if(!strncasecmp(line, "unalias ", 8) || !strcasecmp(line, "unalias")) {
	return unalias(line);
    }
    else if(!strncasecmp(line, "print ", 6)) {
	return printCmd(top, line+6, msg, ' ', NULL , false);
    }
    else if(!strcasecmp(line, "print")) {
	printParseError("print: missing arguments");
	return false;
    }
    else if(!strncasecmp(line, "sprint ", 7)) {
	return sprintCmd(top, line+7, '\0', msg);
    }
    else if(!strcasecmp(line, "sprint")) {
	printParseError("sprint: missing arguments");
	return false;
    }
    else if(!strncasecmp(line, "printf ", 7)) {
	return printCmd(top, line+7, msg, ' ', NULL , true);
    }
    else if(!strcasecmp(line, "printf")) {
	printParseError("printf: missing arguments");
	return false;
    }
    else if(!strncasecmp(line, "sprintf ", 8)) {
	return sprintCmd(top, line+8, '\0', msg);
    }
    else if(!strcasecmp(line, "sprintf")) {
	printParseError("sprintf: missing arguments");
	return false;
    }
    else if(!strcasecmp(line, "listReserved")) {
	for(i = 0; i < (int)reserved_names.size(); i++) {
	    printf("%s\n", quarkToString(reserved_names[i]));
	}
    }
    else if(!strcasecmp(line, "printProperty")) {
	printParseError("printProperty: missing argument");
	return false;
    }
    else if( parseArg(line, "printProperty", c) ) {
	string prop;
	getProperty(c, prop);
	printf("%s: %s\n", c.c_str(), prop.c_str());
    }
    else if( !strcasecmp(line, "help") ) {
	printf("parse FILE\n");
	printf("alias\n");
	printf("alias NAME=COMMAND\n");
	printf("print ATTRIBUTE ATTRIBUTE...\n");
	printf("printOpen FILE [append=(true,false)]\n");
	printf("printClose\n");
	printf("set name=VALUE\n");
	printf("export name\n");
//	printf("setb name=BOOL_VALUE\n");
	printf("unalias NAME\n");
	printf("unset name\n");
	top->parseCmd(line, msg);
    }
    else if( parseArg(line, "relative_file_paths", c) ) {
	if((int)parse_file.size() > 0) {
	    if(parseCompare(c, "true")) {
		if((int)parse_file.size() > 0) {
		    parse_file.back().relative_parse_paths = true;
		}
	    }
	    else if(parseCompare(c, "false")) {
		if((int)parse_file.size() > 0) {
		    parse_file.back().relative_parse_paths = false;
		}
	    }
	    else {
		printParseError("%s: command argument error: %s", file, line);
		return false;
	    }
	}
    }
    else if( parseArg(line, "printOpen", c) ) {
	return printOpenFile(c);
    }
    else if(!strcasecmp(line, "printOpen")) {
	printParseError("printOpen: missing arguments");
	return false;
    }
    else if( parseArg(line, "add_message_handler", c) ) {
	return addMessageHandler(c);
    }
    else if(!strcasecmp(line, "add_message_handler")) {
	printParseError("add_message_handler: missing arguments");
	return false;
    }
    else if( parseArg(line, "send_message", c) ) {
	return sendMessage(top, fr, line);
    }
    else if( parseArg(line, "modified_table_cb", c) ) {
	setTableCallback(c, TABLE_MODIFIED);
	return true;
    }
    else if(!strcasecmp(line, "modified_table_cb")) {
	printParseError("modified_table_cb: missing arguments");
	return false;
    }
    else if( parseArg(line, "deleted_table_cb", c) ) {
	setTableCallback(c, TABLE_DELETED);
	return true;
    }
    else if(!strcasecmp(line, "deleted_table_cb")) {
	printParseError("deleted_table_cb: missing arguments");
	return false;
    }
    else if( parseArg(line, "add_table_cb", c) ) {
	setTableCallback(c, TABLE_ADDED);
	return true;
    }
    else if(!strcasecmp(line, "add_table_cb")) {
	printParseError("add_table_cb: missing arguments");
	return false;
    }
    else if( !strcasecmp(line, "printClose") ) {
	if(print_fp) {
	    fclose(print_fp);
	    print_fp = NULL;
	}
    }
    else if( parseArg(line, "writeOpen", c) ) {
	return writeOpenFile(c);
    }
    else if(!strcasecmp(line, "writeOpen")) {
	printParseError("writeOpen: missing arguments");
	return false;
    }
    else if( !strcasecmp(line, "writeClose") ) {
	if(write_fp) {
	    fclose(write_fp);
	    write_fp = NULL;
	}
    }
    else if(!strncasecmp(line, "writeString(", 12) ||
	    !strncasecmp(line, "writeString ", 12) ||
	    !strncasecmp(line, "writeInt(", 9) ||
	    !strncasecmp(line, "writeInt ", 9) ||
	    !strncasecmp(line, "writeLong(", 10) ||
	    !strncasecmp(line, "writeLong ", 10) ||
	    !strncasecmp(line, "writeFloat(", 11) ||
	    !strncasecmp(line, "writeFloat ", 11) ||
	    !strncasecmp(line, "writeDouble(", 12) ||
	    !strncasecmp(line, "writeDouble ", 12) ||
	    !strncasecmp(line, "writeArray(", 11) ||
	    !strncasecmp(line, "writeArray ", 11) )
    {
	// force these commands to be interpreted as function calls
	snprintf(sline, sizeof(sline), "_tmp_ = %s", line);
	if(assignVariable(top, sline, &var) && var != STRING_RETURNED) {
	    return false;
	}
    }
    else if( stringArg(line, "parse", &file) )
    {
	return parseFile(file, msg);
    }
    else if(!strcasecmp(line, "parse")) {
	printParseError("parse: missing file argument");
	return false;
    }
    else if( !strcasecmp(line, "return") && (int)parse_file.size() > 0) {
	if((int)file_states.size() > 0) {
	    file_states.back()->cs.clear();
	    file_states.back()->foreach.clear();
	}
	return false;
    }
    else if( standard_input && !parse_only && (int)parse_file.size() > 0
		&& !strcasecmp(line, "quit") )
    {
	// skip "quit" in a file.
    }
    else if( !strncasecmp(line, "set ", 4) || !strncasecmp(line, "setb ", 5)
		|| !strncasecmp(line, "setProperty ", 12))
    {
	return assignString(top, line);
    }
    else if(!strcasecmp(line, "set")) {
	printParseError("set: missing arguments");
	return false;
    }
    else if(!strcasecmp(line, "setb")) {
	printParseError("setb: missing arguments");
	return false;
    }
    else if(!strcasecmp(line, "setProperty")) {
	printParseError("setProperty: missing arguments");
	return false;
    }
    else if( !strncasecmp(line, "unset ", 6) ) {
	char u[1000];
	strncpy(u, line+6, sizeof(u)-1);
	stringTrim(u);
	removeVariable(u);
    }
    else if(!strcasecmp(line, "unset")) {
	printParseError("unset: missing arguments");
	return false;
    }
    else if( !strncasecmp(line, "system ", 7) ) {
	if( !doSubstitutions(top,line,sline,sizeof(sline)) ) return false;
	for(i = 7; sline[i] != '\0' && isspace((int)sline[i]); i++);
	if(sline[i] == '"' || sline[i] == '\'' || sline[i] == '`') i++;
	for(int j = (int)strlen(sline)-1; j > i; j--) {
	    if(sline[j] == '"' || sline[j] == '\'' || sline[j] == '`') {
		sline[j] = '\0';
		break;
	    }
	    if(!isspace((int)sline[j])) break;
	}
	system(sline+i);
    }
    else if(!strcasecmp(line, "system")) {
	printParseError("system: missing arguments");
	return false;
    }
    else if( !strncasecmp(line, "wait ", 5) ) {
	if( !doSubstitutions(top,line,sline,sizeof(sline)) ) return false;
	long msecs;
	if(sscanf(sline+5, "%ld", &msecs) == 1) {
	    parseWait(msecs);
	}
	else {
	    printParseError("wait: invalid argument");
	}
    }
    else if(!strcasecmp(line, "wait")) {
	printParseError("wait: missing argument");
	return false;
    }
    else if( !strncasecmp(line, "export ", 7) ) {
	exportVariable(line+7);
    }
    else if(!strcasecmp(line, "export")) {
	printParseError("export: missing arguments");
	return false;
    }
    else if( !strcasecmp(line, "break") ) {
	if(!fr) {
	    printParseError("unexpected 'break' outside foreach");
	    return false;
	}
	fr->mode = FOREACH_BREAK;
    }
    else if( assignVariable(top, line, &var) ) {
	if(var != STRING_RETURNED) return false;
    }
    else if(isCommand(line))
    {
	// look for table command add_row
	for(i = 0; line[i] != '\0' && (isalpha((int)line[i]) ||
	    isdigit((int)line[i]) || line[i] == '_' || line[i] == '.'); i++);
	if(isspace((int)line[i]) && i > 8 && !strncmp(&line[i-8],".add_row",8))
	{
	    s = NULL;
	    i++;
	    strncpy(sline, line, i);
	    if(!printCmd(top, line+i, msg, ',', &s)) return false;
	    snprintf(sline+i, sizeof(sline)-i, "%s", s);
	    Free(s);
	}
	else {
	    getWindowPrefix(line, prefix, sizeof(prefix));

	    if( !doSubstitutions(top,line,sline,sizeof(sline)) ) return false;

	    if(prefix[0] != '\0') {
		// look for lines like 'filter _wave_=45 ...'
		applyPrefix(sline, sizeof(sline), line, line_size, prefix);
	    }
	    else if(fr) {
		checkForeachPrefix(fr, sline, sizeof(sline));
	    }
	}

	ret = top->parseCmd(sline, msg);
	if(ret == ARGUMENT_ERROR) {
	    if(!msg.empty()) printParseError("%s", msg.c_str());
	    else if(strlen(stringTrim(sline)) > 0) {
		printParseError("command argument error: %s", sline);
	    }
	    return false;
	}
	else if(ret == COMMAND_NOT_FOUND) {
	    char *b=sline, *e;
	    while(*b != '\0' &&  isspace((int)*b)) b++;
	    e = b;
	    while(*e != '\0' && !isspace((int)*e) && *e != '=') e++;
	    s = e;
	    while(*s != '\0' &&  isspace((int)*s) && *s != '=') s++;
	    if(*s == '=') { // an assignment
		*e = '\0';
		printParseError("predefined variable not found: %s", b);
	    }
	    else {
		*e = '\0';
		printParseError("command not found: %s", b);
	    }
	    return false;
	}
	else if(ret == COMMAND_PARSED) {
	    if(!strncasecmp(line, "printObject ", 12) ||
		strstr(line, ".printObject "))
	    {
		if(print_fp) {
		    fprintf(print_fp, "%s\n", msg.c_str());
		}
		else {
		    printf("%s\n", msg.c_str());
		}
	    }
	    XEvent event;
	    int check = 0;
	    while((XtAppPending(app_context) & XtIMXEvent) && ++check < 1000)
	    {
		XtAppNextEvent(app_context, &event);
		XtDispatchEvent(&event);
	    }
	}
    }
    else if( line[0] != '\0' ) {
	printParseError("cannot interpret line: %s", line);
	return false;
    }
    return true;
}

static void
getWindowPrefix(char *line, char *prefix, int prefix_size)
{
    int n;
    char *s, *p;

    prefix[0] = '\0';

    // do not change the "edit"  or "remove_from_db" commands (table objects)
    if(!strncmp(line, "edit ", 5) || !strncmp(line, "remove_from_db ", 15)) {
	return;
    }

    // look for lines like 'filter 2.wave[1] ...
    // change 'filter 2.wave[1] ...' to '2.filter 2.wave[1] ...'
    // change 'select tq.arrival[3]' to 'tq.select tq.arrival[3]'
    // etc.
    // the command (filter, select, etc.) cannot already have a prefix ( a '.')


    // look for 'command something.something.something[...'
    // command must be only alphanumeric and '_'. No '.' in command
    s = line;
    while(*s != '\0' && !isspace((int)*s)
	&& (isalpha((int)*s) || isdigit((int)*s) || *s == '_')) s++;
    if(*s == '\0' || !isspace((int)*s)) return;

    // skip space after command
    p = s+1;
    while(*p != '\0' && isspace((int)*p)) p++;
    if(*p == '\0') return;

    // look for '[' before a space
    s = p+1;
    while(*s != '\0' && !isspace((int)*s) && *s != '[') s++;
    if(*s != '[') return;

    // backup to '.', if there is one
    while(s > p && *s != '.') s--;
    if(*s != '.') return; // no '.' means no prefix

    n = s-p;
    if(n > prefix_size-1) n = prefix_size-1; // should warn need more space
    strncpy(prefix, p, n);
    prefix[n] = '\0';
}

static void
applyPrefix(char *sline, int sline_size, char *line, int line_size,
		const char *prefix)
{
    char *s;

    // if the sline is 'command _something_='
    // put the prefix before command

    s = sline;
    while(*s != '\0' && !isspace((int)*s)) s++;
    while(*s != '\0' && isspace((int)*s)) s++;
    if(*s != '_') return;
    s++;

    while(*s != '\0' && (isalpha((int)*s) || isdigit((int)*s))) s++;
    if(strncmp(s, "_=", 2)) return;

    strncpy(line, sline, line_size-1);
    line[line_size-1] = '\0';

    snprintf(sline, sline_size, "%s.%s", prefix, line);
}

static void
checkForeachPrefix(Foreach *fr, char *line, int line_size)
{
    char *s;
    char c[100000];

    if(!fr || !fr->prefix || strcmp(fr->type, "wave")) return;

    // foreach(2.wave, w)
    //    filter w ...
    // at this point the line will be 'filter _wave_=...'
    // the command cannot have a '.'
    s = line;
    while(*s != '\0' && !isspace((int)*s)
	&& (isalpha((int)*s) || isdigit((int)*s) || *s == '_')) s++;
    if(*s == '\0' || !isspace((int)*s)) return;

    while(*s != '\0' && isspace((int)*s)) s++;

    if(strncmp(s, "_wave_", 6)) return;

    snprintf(c, sizeof(c), "%s.%s", fr->prefix, line);
    strncpy(line, c, line_size);
}

bool AppParse::printOpenFile(const string &c)
{
    string file;
    char *prop;
    int append=0;

    if(print_fp) {
	fclose(print_fp);
	print_fp = NULL;
    }
    if(!parseGetArg(c, "file", file)) {
	printParseError("printOpen: missing file argument");
	return false;
    }
    else {
	if(getVariable(file, &prop) == VARIABLE_ERROR) {
	    return false;
	}
	else if(prop) {
	    file.assign(prop);
	    free(prop);
	}
	if(stringGetBoolArg(c.c_str(), "append", &append) == -2) {
	    printParseError("printOpen: non-boolean value for append: %s",
				c.c_str());
	    return false;
	}
	const char *access = (append) ? "a" : "w";
	if( !(print_fp = fopen(file.c_str(), access)) ) {
	    printParseError("printOpen: cannot open %s\n%s",
			file.c_str(), strerror(errno));
	    return false;
	}
	else {
	    print_file.assign(file);
	}
    }
    return true;
}

bool AppParse::writeOpenFile(const string &c)
{
    string file;
    char *prop;
    int append=0;

    if(write_fp) {
	fclose(write_fp);
	write_fp = NULL;
    }
    if(!parseGetArg(c, "file", file)) {
	printParseError("writeOpen: missing file argument");
	return false;
    }
    else {
	if(getVariable(file.c_str(), &prop) == VARIABLE_ERROR) {
	    return false;
	}
	else if(prop) {
	    file.assign(prop);
	    free(prop);
	}
	if(stringGetBoolArg(c.c_str(), "append", &append) == -2) {
	    printParseError("writeOpen: non-boolean value for append: %s",
			c.c_str());
	    return false;
	}
	const char *access = (append) ? "a" : "w";
	if( !(write_fp = fopen(file.c_str(), access)) ) {
	    printParseError("writeOpen: cannot open %s\n%s",
			file.c_str(), strerror(errno));
	    return false;
	}
	else {
	    write_file.assign(file);
	}
    }
    return true;
}

static bool
isForeach(char *line)
{
    char buf[10];
    int i, n;

    // check if command starts with "foreach(" (spaces ignored)
    for(i = 0, n = 0; line[i] != '\0' && n < 8; i++) {
	if(!isspace((int)line[i])) buf[n++] = line[i];
    }
    buf[n] = '\0';

    return( !strcasecmp(buf, "foreach(") ? true : false );
}

static bool
isEndfor(char *line)
{
    if(!strcasecmp(line, "endfor") || !strncasecmp(line, "endfor ", 7)) {
	return true;
    }
    else {
	return false;
    }
}

bool AppParse::foreachEnd(Component *top, char *line, string &msg)
{
    FileState *fs = file_states.back();
    bool ret = true;

    if( !isEndfor(line) ) return true;

    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    if(fr) {
	if(fr->mode == FOREACH_READ) {
	    fr->mode = FOREACH_NONE;
	    ret = processForeach(top, fr, msg);
	}
	delete fr;
	if((int)fs->foreach.size() > 0) fs->foreach.pop_back();
    }
    return ret;
}

bool AppParse::foreachCmd(Component *top, char *line, string &msg)
{
    FileState *fs = file_states.back();
    ParseVar ret;
    char *a=NULL, *b=NULL, arg[1000], prefix[1000], type[200], name[200];
    int i, j, start, end;

    if(line[0] == '\0') return true;

    // if not foreach return.
    if( !isForeach(line) ) return true;

    memset(name, 0, sizeof(name));
    memset(prefix, 0, sizeof(prefix));
    memset(type, 0, sizeof(type));

    for(i = 0; line[i] != '\0' && line[i] != '('; i++);
    if(line[i] == '(') i++;

    // foreach( tableviewer.arrival , a )
    //         i
    while(line[i] != '\0' && isspace((int)line[i])) i++;
    // foreach( tableviewer.arrival , a )
    //          i
    if(line[i] == '\0' || line[i] == '.' || line[i] == ',') {
	printParseError("foreach: argument syntax error");
	return false;
    }
    for(j = (int)strlen(line)-1; j > i && line[j] != ')'; j--);
    // foreach( tableviewer.arrival , a )
    //          i                       j
    if(line[j] != ')') {
	printParseError("foreach: missing ')'");
	return false;
    }
    while(--j > i && isspace((int)line[j]));
    // foreach( tableviewer.arrival , a )
    //          i                     j
    line[j+1] = '\0';
    a = strstr(line+1, ",");
    if(a) b = strstr(a+1, ",");

//    if( !strstr(line+i, ",") ) {
    if( a && b && strstr(b+1, ",") ) {
	printParseError("foreach: argument syntax error");
	return false;
    }
    else if(!a && !b) {
	// foreach( name )
	//          i  j
	// single argument
	strncpy(name, line+i, sizeof(name)-1);
	strncpy(type, "_string_", sizeof(type)-1);
    }
    else if(a && b) {
	*a = '\0';
	strncpy(arg, line+i, sizeof(arg));
	*b = '\0';
	if((ret=parseExpression(top, arg, false, msg)) != STRING_RETURNED
		|| !stringToInt(msg.c_str(), &start))
	{
	    if(ret == VARIABLE_NOT_FOUND) {
		printParseError("foreach: start index: %s is not defined", arg);
	    }
	    else {
	       printParseError("foreach: cannot interpret start index: %s",arg);
	    }
	    return false;
	}
	strncpy(arg, a+1, sizeof(arg));
	if((ret=parseExpression(top, arg, false, msg)) != STRING_RETURNED
		|| !stringToInt(msg.c_str(), &end))
	{
	    if(ret == VARIABLE_NOT_FOUND) {
		printParseError("foreach: end index: %s is not defined", arg);
	    }
	    else {
		printParseError("foreach: cannot interpret end index: %s", arg);
	    }
	    return false;
	}
	strncpy(name, b+1, sizeof(name));
	stringTrim(name);
	if(name[0] == '\0') {
	    printParseError("foreach: missing index name");
	    return false;
	}
	strncpy(type, "_int_", sizeof(type)-1);
    }
    else { // k = 1
	// foreach( tableviewer.arrival , a )
	//          i                     j0
	while(--j > i && line[j] != ',' && !isspace((int)line[j]));
	// foreach( tableviewer.arrival , a )
	//          i                    j 0
	strncpy(name, line+j+1, sizeof(name)-1);

	while(j > i && line[j] != ',' && isspace((int)line[j])) j--;
	// foreach( tableviewer.arrival , a )
	//          i                   j  0
	if(j == i || line[j] != ',') {
	    printParseError("foreach: argument syntax error");
	    return false;
	}
	while(--j > i && isspace((int)line[j]));
	// foreach( tableviewer.arrival , a )
	//          i                 j    0
	line[j+1] = '\0';
	// foreach( tableviewer.arrival , a )
	//          i                 j0   0
	while(j > i && !isspace((int)line[j]) && line[j] != '.') j--;

	if(line[j] == '.') {
	    // foreach( tableviewer.arrival , a )
	    //          i          j       0   0
	    strncpy(type, line+j+1, sizeof(type)-1);
	    line[j] = '\0';
	    // foreach( tableviewer.arrival , a )
	    //          i          0       0   0
	    strncpy(prefix, line+i, sizeof(prefix)-1);
	}
	else if(j == i) {
	    // foreach( arrival , a )
	    //          i      0   0
	    //	    j
	    strncpy(type, line+j, sizeof(type)-1);
	}
	if(type[0] == '\0') {
	    printParseError("foreach: argument syntax error");
	    return false;
	}
    }

    Foreach *fr = new Foreach();
    fs->foreach.push_back(fr);

    if(!strcmp(type, "_string_")) { // single argument: comma delimited string
	ret = startArray(name);
    }
    else if(!strcmp(type, "_int_")) { // start,end arguments
	if(end >= start) {
	    snprintf(arg, sizeof(arg), "%d", start);
	    putVariable(name, arg);
	    ret = FOREACH_MORE;
	}
	else {
	    ret = FOREACH_NO_MORE;
	}
    }
    else {
	if(prefix[0] != '\0') {
	    snprintf(arg, sizeof(arg), "%s.foreach_start_%s:%s",
			prefix, type, name);
	}
	else {
	    snprintf(arg, sizeof(arg), "foreach_start_%s:%s", type, name);
	}
	ret = top->parseVar(arg, msg);
    }
#ifdef DEBUG_APP 
printf("foreach cmd: prefix: %s, type: %s, name: %s\n", prefix, type, name);
#endif

    if(ret == FOREACH_MORE || ret == FOREACH_NO_MORE)
    {
	if(ret == FOREACH_MORE) fr->mode = FOREACH_READ;
	else fr->mode = FOREACH_SKIP;

	fr->type = strdup(type);
	fr->name = strdup(name);
	fr->prefix = (prefix[0] != '\0') ? strdup(prefix) : NULL;
	fr->start = start;
	fr->end = end;
	fr->i = start;
    }
    else {
	delete fr;
	if((int)fs->foreach.size() > 0) fs->foreach.pop_back();
	if(ret == VARIABLE_NOT_FOUND) {
	    if(!strcmp(type, "_string_")) {
		printParseError("foreach: %s is not a defined string", name);
	    }
	    else if(prefix[0] != '\0') {
		printParseError("foreach: invalid argument type %s.%s",
				prefix, type);
	    }
	    else {
		printParseError("foreach: invalid argument type %s", type);
	    }
	}
	else {
	    if(prefix[0] != '\0') {
		printParseError("foreach: cannot interpret argument: %s.%s,%s",
			prefix, type, name);
	    }
	    else {
		printParseError("foreach: cannot interpret argument: %s,%s",
			type, name);
	    }
	}
	return false;
    }
    return true;
}

bool AppParse::processForeach(Component *top, Foreach *fr, string &msg)
{
    FileState *fs = file_states.back();
    ParseVar ret = FOREACH_NO_MORE;
    int i, n = 0, ln=0;
    char line[100000], s[20];

    if(fr->mode != FOREACH_SKIP) {
	do {
	    n++;
#ifdef DEBUG_APP 
printf("process foreach %d\n", n);
#endif
	    snprintf(s, sizeof(s), "%d", n);
	    putVariable("for_index", s);
	    if((int)parse_file.size() > 0) ln = parse_file.back().line_number;
	    for(i = 0; i < (int)fr->lines.size(); i++) {
		if((int)parse_file.size() > 0) {
		    parse_file.back().line_number = fr->lines[i].line_number;
		}
		if( !parseLine(fr->lines[i].line, msg) ) {
		    ret = VARIABLE_ERROR;
		    break;
		}
	    }
	    if((int)parse_file.size() > 0) parse_file.back().line_number = ln;
	    if(fr->mode == FOREACH_BREAK || i < (int)fr->lines.size()) {
		break;
	    }

	    if( !strcmp(fr->type, "_string_") ) {
		ret = nextArray(msg);
	    }
	    else if( !strcmp(fr->type, "_int_") ) {
		if(++fr->i <= fr->end) {
		    snprintf(s, sizeof(s), "%d", fr->i);
		    putVariable(fs->foreach.back()->name, s);
		    ret = FOREACH_MORE;
		}
		else {
		    ret = FOREACH_NO_MORE;
		}
	    }
	    else {
		if(fr->prefix) {
		    snprintf(line, sizeof(line), "%s.foreach_next_%s:%s",
			fr->prefix, fr->type, fr->name);
		}
		else {
		    snprintf(line, sizeof(line), "foreach_next_%s:%s",
			fr->type, fr->name);
		}
		ret = top->parseVar(line, msg);
	    }
	} while(ret == FOREACH_MORE);

	if(ret != FOREACH_NO_MORE) {
	    if(ret == VARIABLE_ERROR) {
		printParseError("foreach: syntax error");
	    }
	    else if(ret == VARIABLE_NOT_FOUND) {
		printParseError("foreach: undefined variable");
	    }
	    if( !strcmp(fs->foreach.back()->type, "_string_") ) {
		putVariable(fs->foreach.back()->name,
				fs->foreach.back()->array_save);
	    }
	}
    }

    if( !strcmp(fs->foreach.back()->type, "_int_") ||
	!strcmp(fs->foreach.back()->type, "_string_") )
    {
	if(ret == FOREACH_NO_MORE) ret = STRING_RETURNED;
    }
    else {
	if(fr->prefix) {
	    snprintf(line, sizeof(line), "%s.foreach_stop_%s:%s",
			fr->prefix, fr->type, fr->name);
	}
	else {
	    snprintf(line, sizeof(line), "foreach_stop_%s:%s",
			fr->type, fr->name);
	}
	ParseVar r = top->parseVar(line, msg);
	if(ret == FOREACH_NO_MORE) ret = r;
    }
    return (ret == VARIABLE_ERROR || ret == VARIABLE_NOT_FOUND) ? false : true;
}

int AppParse::conditionalCmd(Component *top, char *line, string &msg)
{
    FileState *fs = file_states.back();
    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    enum ConditionalState cs = ((int)fs->cs.size() > 0) ?
		fs->cs.back() : CONDITIONAL_NONE;
    ParseVar ret;
    char buf[10], *s;
    int i, j, n;

    if(line[0] == '\0') return 0;

    if(!strcasecmp(line, "else")) {
	if(cs == CONDITIONAL_IF_FALSE || cs == CONDITIONAL_ELSEIF_FALSE)
	{
	    cs = CONDITIONAL_ELSE_TRUE;
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	    fs->cs.push_back(cs);
	}
	else if(cs == CONDITIONAL_IF_TRUE || cs == CONDITIONAL_ELSEIF_TRUE)
	{
	    cs = CONDITIONAL_SKIP;
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	    fs->cs.push_back(cs);
	}
	else if(cs != CONDITIONAL_SKIP) {
	    msg.assign("unexpected \"else\"");
	    printParseError("unexpected \"else\"");
	    return -1;
	}
	return 1;
    }
    else if(!strcasecmp(line, "endif") || !strncasecmp(line, "endif ", 6)) {
	if(cs == CONDITIONAL_NONE) {
	    msg.assign("unexpected \"endif\"");
	    printParseError("unexpected \"endif\"");
	    return -1;
	}
	else {
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	}
	return 1;
    }

    if(!strcasecmp(line, "elseif") || !strcasecmp(line, "else if")) {
	msg.assign(string(line) + ": missing argument");
	printParseError("%s: missing argument", line);
	return -1;
    }
    // check if command starts with "if(" (spaces ignored)
    for(i = 0, n = 0; line[i] != '\0' && n < 3; i++) {
	if(!isspace((int)line[i])) buf[n++] = line[i];
    }
    buf[n] = '\0';
    if( strcasecmp(buf, "if(") ) {
	// check if command starts with "elseif(" (spaces ignored)
	for(i = 0, n = 0; line[i] != '\0' && n < 7; i++) {
	    if(!isspace((int)line[i])) buf[n++] = line[i];
	}
	buf[n] = '\0';
    }

    if( !strcasecmp(buf, "if(") || !strcasecmp(buf, "elseif(") )
    {
	buf[n-1] = '\0';
	while(line[i] != '\0' && isspace((int)line[i])) i++;
	if(line[i] == '\0' || line[i] == ')') {
	    msg.assign(string("missing \"") + buf + "\" argument");
	    printParseError("missing \"%s\" argument", buf);
	    return -1;
	}
	for(j = (int)strlen(line)-1; j > i && line[j] != ')'; j--);
	if(line[j] != ')') {
	    msg.assign(string("missing ')' after \"") + buf + "\"");
	    printParseError("missing ')' after \"%s\"", buf);
	    return -1;
	}
	while(--j > i && isspace((int)line[j]));
	line[j+1] = '\0';

	if( !strcasecmp(buf, "if") )
	{
	    if(cs == CONDITIONAL_SKIP || cs == CONDITIONAL_IF_FALSE
		|| cs == CONDITIONAL_ELSEIF_FALSE)
	    {
		fs->cs.push_back(CONDITIONAL_SKIP);
		return 1;
	    }
	}
	else { // elseif
	    if(cs == CONDITIONAL_SKIP) {
		return 1;
	    }
	    else if(cs == CONDITIONAL_IF_TRUE || cs == CONDITIONAL_ELSEIF_TRUE)
	    {
		// skip remaining lines until endif
		if((int)fs->cs.size() > 0) fs->cs.pop_back();
		fs->cs.push_back(CONDITIONAL_SKIP);
		return 1;
	    }
	    else if(cs != CONDITIONAL_IF_FALSE && cs!=CONDITIONAL_ELSEIF_FALSE)
	    {
		// 'elseif' must follow an 'if' or an 'elseif'
		msg.assign("unexpected \"elseif\"");
		printParseError("unexpected \"elseif\"");
		if((int)fs->cs.size() > 0) fs->cs.pop_back();
		return -1;
	    }
	}

	if(fr && fr->mode == FOREACH_BREAK) {
	    // does not matter if true or false since we will skip until the
	    // the end of the foreach loop
	    ret = VARIABLE_TRUE;
	}
	else {
	    s = strdup(line+i);
	    ret = parseExpression(top, s, true, msg);
	    free(s);
	}

	if(ret == VARIABLE_TRUE) {
	    if( !strcasecmp(buf, "if") ) {
		fs->cs.push_back(CONDITIONAL_IF_TRUE);
	    }
	    else {
		if((int)fs->cs.size() > 0) fs->cs.pop_back();
		fs->cs.push_back(CONDITIONAL_ELSEIF_TRUE);
	    }
	}
	else if(ret == VARIABLE_FALSE) {
	    if( !strcasecmp(buf, "if") ) {
		fs->cs.push_back(CONDITIONAL_IF_FALSE);
	    }
	    else {
		if((int)fs->cs.size() > 0) fs->cs.pop_back();
		fs->cs.push_back(CONDITIONAL_ELSEIF_FALSE);
	    }
	}
	else if(ret == VARIABLE_NOT_FOUND) {
	    msg.assign(string(buf)+" argument '"+string(line+i)+"' not found.");
	    printParseError("%s argument '%s' not found.", buf, line+i);
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	    return -1;
	}
	else if(ret == VARIABLE_ERROR) {
	    msg.assign(string("cannot interpret ") + buf + " argument '" +
			string(line+i) +"'");
	    printParseError("cannot interpret %s argument '%s'", buf, line+i);
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	    return -1;
	}
	else {
	    if(!msg.empty()) printParseError("%s", msg.c_str());
	    if((int)fs->cs.size() > 0) fs->cs.pop_back();
	    return -1;
	}
	return 1;
    }
    else { // not an if, elseif, else, or endif
	if(cs == CONDITIONAL_SKIP || cs == CONDITIONAL_IF_FALSE ||
		cs == CONDITIONAL_ELSEIF_FALSE) 
	{
	    return 1; // skip the line
	}
	return 0; // proess the line
    }
}

ParseVar AppParse::parseExpression(Component *top, char *line, bool logical,
				string &msg)
{
    ParseVar ret;
    char *b, *c, *e, a, sline[100000], tmp[100000];
    double d;
    Item items[100];
    int i, i1, i2, j, n, nitems = 0;
  
    msg.clear();

    // (((a+b)/c - d) + (g*h/r))

    stringTrim(line);

    // check for a single int or double
    if(stringToInt(line, &i)) {
	if(logical) {
	    if(i != 0) {
		msg.assign("true");
		return VARIABLE_TRUE;
	    }
	    else {
		msg.assign("false");
		return VARIABLE_FALSE;
	    }
	}
	msg.assign(line);
	return STRING_RETURNED;
    }
    else if(stringToDouble(line, &d)) {
	if(logical) {
	    if(d != 0.) {
		msg.assign("true");
		return VARIABLE_TRUE;
	    }
	    else {
		msg.assign("false");
		return VARIABLE_FALSE;
	    }
	}
	msg.assign(line);
	return STRING_RETURNED;
    }
    else if(!strcasecmp(line, "true"))
    {
	msg.assign(line);
	if(logical) {
	    return VARIABLE_TRUE;
	}
	return STRING_RETURNED;
    }
    else if(!strcasecmp(line, "false"))
    {
	msg.assign(line);
	if(logical) {
	    return VARIABLE_FALSE;
	}
	return STRING_RETURNED;
    }

    memset(sline, 0, sizeof(sline));
    strncpy(sline, line, sizeof(sline)-1);

    c = sline;

    // evaluate all bracket [] arguments

    for(;;) // process inner brackets [] first
    {
	for(i1 = 0; c[i1] != '\0' && c[i1] != '['; i1++);
	if(c[i1] == '\0') break; // no more brackets

	// find i1 = '[' and i2 = ']'
	for(i2 = i1+1; c[i2] != '\0'; i2++) {
	    if(c[i2] == '[') {
		i1 = i2;
	    }
	    else if(c[i2] == ']') {
		// put inner brackets back
		for(i = i1+1; i < i2; i++) {
		    if(c[i] == '\f') c[i] = '[';
		    else if(c[i] == '\b') c[i] = ']';
		}
		for(i = i1+1; i <= i2 && isspace((int)c[i]); i++);
		if(i1 == i2) {
		    printParseError("empty brackets at: %s", c+i1);
		    return VARIABLE_ERROR;
		}
		b = c+i;
		e = c+i2;
		while(isspace((int)(*(e-1)))) e--;
		a = *e;
		*e = '\0';
		if(stringToInt(b, &i)) {
		    msg.assign(b);
		    ret = STRING_RETURNED;
		}
		else {
		    ret = parseExpression(top, b, false, msg);
		}
		*e = a;
		if(ret == STRING_RETURNED) {
		    // rename the brackets so they are not processed again
		    c[i1] = '\f';
		    c[i2] = '\b';
		    n = i1+1;
		    strncpy(tmp, sline, n);
		    strncpy(tmp+n, msg.c_str(), sizeof(tmp)-n);
		    n += (int)msg.length();
		    strncpy(tmp+n, c+i2, sizeof(tmp)-n);
		    strncpy(sline, tmp, sizeof(sline)-1);
		}
		else {
		    return VARIABLE_ERROR;
		}
		break;
            }
        }
    }
    // put brackets back
    for(i = 0; c[i] != '\0'; i++) {
	if(c[i] == '\f') c[i] = '[';
	else if(c[i] == '\b') c[i] = ']';
    }

    // look for exponential formats like 1.2e-04 or 1.e+12
    // mask the '-' or '+' in exponential formats to make parsing easier
    c = sline;
    for(i = 0; c[i] != '\0'; i++) {
	if(c[i] == '"' || c[i] == '\'' || c[i] == '`') {
	    a = c[i];
	    while(c[i+1] != '\0' && c[++i] != a);
	}
	else if( (c[i] == '.' || isdigit((int)c[i])) // a '.' or digit
		&& (c[i+1]=='e' || c[i+1]=='E') // followed by 'e' or 'E'
		&& (c[i+2]=='-' || c[i+2]=='+') // followed by '-' or '+'
		&& isdigit((int)c[i+3]) )	// followed by a digit
	{
	    if(c[i+2] == '-') c[i+2] = '\b';
	    else c[i+2] = '\f';
	}
    }

    // parse the string into pointers to terms and operators.
    nitems = parseTerms(sline, items, 100);

#ifdef DEBUG_APP 
printf("after parseTerms:\n");
for(i = 0; i < nitems; i++) {
    char *t = items[i].c;
    for(j = 0; t[j] != '\0'; j++) {
	if(t[j] == '\b') printf("-");
	else if(t[j] == '\f') printf("+");
	else printf("%c", t[j]);
    }
    printf("\n");
}
printf("\n");
#endif

    if(nitems <= 0) {
	return VARIABLE_ERROR;
    }

    // find minus signs
    if(findMinusSigns(&nitems, items, line) == VARIABLE_ERROR) {
	for(i = 0; i < nitems; i++) { Free(items[i].c); Free(items[i].func); }
	return VARIABLE_ERROR;
    }

    // find ! signs
    if(findNotSigns(&nitems, items, line) == VARIABLE_ERROR) {
	for(i = 0; i < nitems; i++) { Free(items[i].c); Free(items[i].func); }
	return VARIABLE_ERROR;
    }

    if(checkSyntax(&nitems, items, logical, sline) == VARIABLE_ERROR) {
	for(i = 0; i < nitems; i++) { Free(items[i].c); Free(items[i].func); }
	return VARIABLE_ERROR;
    }

    // fix exponentials
    for(i = 0; i < nitems; i++) {
	if(items[i].c) {
	    for(j = 0; items[i].c[j] != '\0'; j++) {
		if(items[i].c[j] == '\b') items[i].c[j] = '-';
		else if(items[i].c[j] == '\f') items[i].c[j] = '+';
	    }
	}
    }

    // evaluate terms
    if((ret = evaluateTerms(top, nitems, items, msg)) != STRING_RETURNED)
    {
	for(i = 0; i < nitems; i++) { Free(items[i].c); Free(items[i].func); }
	return ret;
    }

//    if(nitems == 1) return ret;

#ifdef DEBUG_APP 
for(i = 0; i < nitems; i++) {
a = items[i].c[0];
if(items[i].minus) printf("-");
if(items[i].not_sign) printf("!");
if(items[i].func && items[i].oper == '(') printf("%s ", items[i].func);
if(items[i].oper != '\0') {
    if(items[i].oper == '=') printf("== ");
    else if(items[i].oper == '~') printf("~= ");
    else if(items[i].oper == '^') printf("^= ");
    else if(items[i].oper == '\v') printf("!= ");
    else if(items[i].oper == '\n') printf(">= ");
    else if(items[i].oper == '\r') printf("<= ");
    else if(items[i].oper == '&') printf("&& ");
    else if(items[i].oper == '|') printf("|| ");
    else printf("%c ", items[i].oper);
}
else if(items[i].type == STRING_ITEM) printf("%s ", items[i].c);
else if(items[i].type == INT_ITEM) printf("%ld ", items[i].l);
else printf("%.15g ", items[i].d);
}
printf("\n");
#endif

    // process code within the interior parentheses () until none are left.
    for(;;)
    {
	// find the next '('
	for(i1 = 0; i1 < nitems && items[i1].oper != '('; i1++);
	if(i1 == nitems) {
	    ret = parseParen(top, items, &nitems, 0, nitems-1, msg);
	    break;
	}
	for(i2 = i1+1; i2 < nitems; i2++)
	{
	    if(items[i2].oper == '(') {
		i1 = i2;
	    }
	    else if(items[i2].oper == ')')
	    {
		n = i2 - i1 -1;

		if(n == 0) { // empty parentheses (). must be a function call
		    if(!callFunc(top, items, i1, &nitems)) {
			for(i = 0; i < nitems; i++) {
			    Free(items[i].c); Free(items[i].func);
			}
			return VARIABLE_ERROR;
		    }
		}
		else
		{
		    ret = parseParen(top, items, &nitems, i1+1, i2-1, msg);

		    if(ret == STRING_RETURNED)
		    {
			// apply a function, if found.
			if(items[i1].func)
			{
			    if(!callFunc(top, items, i1, &nitems)) {
				for(i = 0; i < nitems; i++) {
				    Free(items[i].c); Free(items[i].func);
				}
				return VARIABLE_ERROR;
			    }
			}
			else {
			    // the resulting term is in items[i1+1]
			    // remove the parentheses that were at i1 and i2
			    // apply a sign, if found. ex: (-(a+b) +c )
			    if(items[i1].minus) {
				items[i1+1].l = -items[i1+1].l;
				items[i1+1].d = -items[i1+1].d;
			    }
			    if(items[i1].not_sign) {
				items[i1+1].l = !items[i1+1].l;
				items[i1+1].d = !items[i1+1].d;
			    }
			    Free(items[i1].c);
			    Free(items[i1].func);
			    Free(items[i1+2].c);
			    items[i1] = items[i1+1];
			    for(i=i1+3, j=0; i < nitems; i++, j++) {
				items[i1+1+j] = items[i];
			    }
			    nitems -= 2;
			}
		    }
		    else {
			for(i = 0; i < nitems; i++) {
			    Free(items[i].c); Free(items[i].func);
			}
			return ret;
		    }
		}
		break;
	    }
	}
    }
    if(nitems > 0) {
	if(ret == STRING_RETURNED) {
	    if( !logical ) {
		char buf[30];
		if(items[0].type==INT_ITEM) {
		    snprintf(buf, sizeof(buf), "%ld", items[0].l);
		    msg.assign(buf);
		}
		else if(items[0].type==DOUBLE_ITEM) {
		    snprintf(buf, sizeof(buf), "%.15g", items[0].d);
		    msg.assign(buf);
		}
		else if(items[0].c) {
		    msg.assign(items[0].c);
		}
	    }
	    else if(logical) {
		if(items[0].type==INT_ITEM) {
		    if(items[0].l != 0) {
			msg.assign("true");
			ret = VARIABLE_TRUE;
		    }
		    else {
			msg.assign("false");
			ret = VARIABLE_FALSE;
		    }
		}
		else if(items[0].type == DOUBLE_ITEM) {
		    if(items[0].d != 0.) {
			msg.assign("true");
			ret = VARIABLE_TRUE;
		    }
		    else {
			msg.assign("false");
			ret = VARIABLE_FALSE;
		    }
		}
		else { // STRING_ITEM
		    c = items[0].c;
		    if(!strcasecmp(c, "true") || !strcasecmp(c, "1")) {
			msg.assign("true");
			ret = VARIABLE_TRUE;
		    }
		    else if(!strcasecmp(c, "false") || !strcasecmp(c, "0")) {
			msg.assign("false");
			ret = VARIABLE_FALSE;
		    }
		    else {
			printParseError("cannot interpret: %s", c);
			return VARIABLE_ERROR;
		    }
		}
	    }
	}
	for(i = 0; i < nitems; i++) { Free(items[i].c); Free(items[i].func); }
    }
    return ret;
}

int AppParse::parseTerms(char *line, Item *items, int max_items)
{
    char c1[2], q, *b, *c, *e;
    int j, k, n, nitems = 0;

    // Change the two-character codes ==, ~=, ^=, !=, >=, <=, &&,|| to one-char.
    c = line;
    j = k = 0;
    while(c[j] != '\0') {
	if(c[j] == '"' || c[j] == '`' || c[j] == '\'') {
	    c[k++] = c[j];
	    q = c[j++];
	    while(c[j] != '\0' && c[j] != q) c[k++] = c[j++];
	    if(c[j] != q) {
		printParseError("missing end quote: %c", q);
		return -1;
	    }
	}

	if(c[j] == '=' && c[j+1] == '=') {
	    c[k++] = '=';
	    j += 2; // skip the following '='
	}
	else if(c[j] == '~' && c[j+1] == '=') {
	    c[k++] = '~';      // use '~' as a one-char substitute for '~='
	    j += 2; // skip the following '='
	}
	else if(c[j] == '^' && c[j+1] == '=') {
	    c[k++] = '^';      // use '^' as a one-char substitute for '^='
	    j += 2; // skip the following '='
	}
	else if(c[j] == '!' && c[j+1] == '=') {
	    c[k++] = '\v';      // use '\v' as a one-char substitute for '!='
	    j += 2; // skip the following '='
	}
	else if(c[j] == '>' && c[j+1] == '=') {
	    c[k++] = '\n';      // use '\n' as a one-char substitute for '>='
	    j += 2; // skip the following '='
	}
	else if(c[j] == '<' && c[j+1] == '=') {
	    c[k++] = '\r';      // use '\r' as a one-char substitute for '<='
	    j += 2; // skip the following '='
	}
	else if(c[j] == '&' && c[j+1] == '&') {
	    c[k++] = '&';
	    j += 2; // skip the following '&'
	}
	else if(c[j] == '|' && c[j+1] == '|') {
	    c[k++] = '|';
	    j += 2; // skip the following '|'
	}
	else {
	    c[k++] = c[j++];
	}
    }
    c[k] = '\0';

    c1[1] = '\0';
    b = line;
    while(*b != '\0') {
	while(*b == ' ') b++;
	if(*b == '\0') break;

	if(nitems+1 >= max_items) {
	    printParseError("maximum items per line exceeded: %d", max_items);
	    for(int i = 0; i < nitems; i++) {
		Free(items[i].c); Free(items[i].func);
	    }
	    return -1;
	}
	items[nitems].c = NULL;
	items[nitems].func = NULL;
	items[nitems].oper = '\0';
	items[nitems].minus = false;
	items[nitems].not_sign = false;
	items[nitems].type = STRING_ITEM;
	items[nitems].l = 0;
	items[nitems].d = 0.;

	c1[0] = *b;
	if(strpbrk(c1, "()+-*/,=!~^\v<>\n\r&|")) {
	    items[nitems].oper = *b;
	    if(*b == '=')	items[nitems++].c = strdup("==");
	    else if(*b == '~')	items[nitems++].c = strdup("~=");
	    else if(*b == '^')	items[nitems++].c = strdup("^=");
	    else if(*b == '\v')	items[nitems++].c = strdup("!=");
	    else if(*b == '\n')	items[nitems++].c = strdup(">=");
	    else if(*b == '\r')	items[nitems++].c = strdup("<=");
	    else if(*b == '&')	items[nitems++].c = strdup("&&");
	    else if(*b == '|')	items[nitems++].c = strdup("||");
	    else items[nitems++].c = strdup(c1);
	    b++;
	}
	else if(strpbrk(c1, "\"'`")) {
	    q = c1[0];
	    e = b+1;
	    while(*e != '\0' && *e != q) e++;
	    if(*e == q) {
		n = e-b+1;
		c = (char *)malloc(n+1);
		strncpy(c, b, n);
		c[n] = '\0';
		items[nitems++].c = c;
		b = e+1;
	    }
	    else {
		printParseError("missing end quote: %c", q);
		for(int i = 0; i < nitems; i++) {
		    Free(items[i].c); Free(items[i].func);
		}
		return -1;
	    }
	}
	else if( (e = strpbrk(b, "()+-*/,=!~^\v<>\n\r&|")) ) {
	    n = e-b;
	    c = (char *)malloc(n+1);
	    strncpy(c, b, n);
	    c[n] = '\0';
	    stringTrim(c);
	    items[nitems++].c = c;
	    b = e;
	}
	else { // the last term, which is not followed by ')'
	    c = strdup(b);
	    stringTrim(c);
	    items[nitems++].c = c;
	    break;
	}
    }
    return nitems;
}

ParseVar AppParse::findMinusSigns(int *nitems, Item *items, char *line)
{
    int i, j;
//    char a, c;
    char b, c1[2], c2[2];

    if(*nitems <= 0) {
	printParseError("cannot interpret: %s", line);
	return VARIABLE_ERROR;
    }

    /* Must be able to handle expressions like:
	set s=( a + -b )
	set s=( a - +b )
	set s=( a * -b )
	set s=( a / -b )
	set s=( -a + b )
	set s=( a + -(b + c))
	set s=( a + -(-b * -c))
	set s=-(a + b)
	set s=(a + (-(c+d) / -a))
    */

    // if the first item is '+', remove it
    if(items[0].c[0] == '+') {
	Free(items[0].c);
	for(i = 0; i < *nitems-1; i++) items[i] = items[i+1];
	*nitems = *nitems - 1;
    }

#ifdef DEBUG_APP 
for(i = 0; i < *nitems; i++) {
    char *t = items[i].c;
    for(j = 0; t[j] != '\0'; j++) {
	if(t[j] == '\b') printf("-");
	else if(t[j] == '\f') printf("+");
	else printf("%c", t[j]);
    }
    printf(" ");
}
printf("\n");
#endif

    // change all "+-" or "-+" sequences to "-", all "--" to "+", "++" to "+"
    for(i = 0; i < *nitems-1; i++) {
	if(items[i].c[0] == '+' && items[i+1].c[0] == '-') {
	    items[i].c[0] = '\f'; // flag it to be removed
	}
	else if(items[i].c[0] == '-' && items[i+1].c[0] == '+') {
	    items[i+1].c[0] = '\f'; // flag it to be removed
	}
	else if(items[i].c[0] == '-' && items[i+1].c[0] == '-') {
	    items[i].c[0] = '\f'; // flag it to be removed
	}
	else if(items[i].c[0] == '+' && items[i+1].c[0] == '+') {
	    items[i].c[0] = '\f'; // flag it to be removed
	}
    }

    // discard flagged items
    for(i=0, j=0; i < *nitems; i++) {
	if(items[i].c[0] == '\f') { Free(items[i].c); Free(items[i].func); }
	else items[j++] = items[i];
    }
    *nitems = j;

    if(*nitems == 0) {
	printParseError("cannot interpret: %s", line);
	return VARIABLE_ERROR;
    }

#ifdef DEBUG_APP 
printf("after removing +-, etc.\n");
for(i = 0; i < *nitems; i++) {
    char *t = items[i].c;
    for(j = 0; t[j] != '\0'; j++) {
	if(t[j] == '\b') printf("-");
	else if(t[j] == '\f') printf("+");
	else printf("%c", t[j]);
    }
    printf(" ");
}
printf("\n");
#endif
	
    // find all minus signs.

    for(i = 0; i < *nitems-1; i++) {
//	a = (i > 0) ? items[i-1].c[0] : '\0';
	b = items[i].c[0];
//	c = items[i+1].c[0];
	if(b == '-') {
/*
	    if(a == '\0' || a == '*' || a == '/' || a == '(' || a == ',') {
		if(c!='*' && c!='/' && c!='-' && c!='+' && c!=')' && c!=','
		    && c!='"' && c!='`' && c!='\'')
*/
	    if(i == 0 || (items[i-1].oper != '\0' && items[i-1].oper != ')'))
//	    if(items[i+1].oper == '(' || items[i+1].oper == '\0')
		{
		    items[i].c[0] = '\a'; // flag it to be removed
		    items[i+1].minus = true; // this term has a minus sign
		}
//	    }
	}
    }
    // discard flagged items
    for(i=0, j=0; i < *nitems; i++) {
	if(items[i].c[0] == '\a') { Free(items[i].c); Free(items[i].func); }
	else items[j++] = items[i];
    }
    *nitems = j;

#ifdef DEBUG_APP 
printf("after detecting minus signs.\n");
for(i = 0; i < *nitems; i++) {
    char *t = items[i].c;
    if(items[i].minus) printf("minus ");
    for(j = 0; t[j] != '\0'; j++) {
	if(t[j] == '\b') printf("-");
	else if(t[j] == '\f') printf("+");
	else printf("%c", t[j]);
    }
    printf(" ");
}
printf("\n");
#endif

    // check for operators next to each other

    c1[1] = '\0';
    c2[1] = '\0';
    for(i = 0; i < *nitems-1; i++) {
	c1[0] = items[i].c[0];
	c2[0] = items[i+1].c[0];
	if(strpbrk(c1, "=+-*/,") && strpbrk(c2, "=+-*/,")) {
	    if(!(c1[0]==',' && c2[0]==',')) {
		printParseError("cannot interpret: %c followed by %c",
			c1[0], c2[0]);
		return VARIABLE_ERROR;
	    }
	}
	if(strpbrk(c1, "+-*/\"'`") && strpbrk(c2, "+-*/\"'`")) {
	    printParseError("cannot interpret: %c followed by %c", c1[0],c2[0]);
	    return VARIABLE_ERROR;
	}
    }

    return STRING_RETURNED;
}

ParseVar AppParse::findNotSigns(int *nitems, Item *items, char *line)
{
    int i, j;

    if(*nitems == 0) {
	printParseError("cannot interpret: %s", line);
	return VARIABLE_ERROR;
    }

    // find all '!'s

    for(i = 0; i < *nitems-1; i++) {
	if(!strcmp(items[i].c, "!")) { // don't want to get "!=" operator
	    if(items[i+1].oper == '(' || items[i+1].oper == '\0') {
		items[i].c[0] = '\a'; // flag it to be removed
		items[i+1].not_sign = true; // this term has a ! sign
	    }
	}
    }
    // discard flagged items
    for(i=0, j=0; i < *nitems; i++) {
	if(items[i].c[0] == '\a') { Free(items[i].c); Free(items[i].func); }
	else items[j++] = items[i];
    }
    *nitems = j;

#ifdef DEBUG_APP 
printf("after detecting ! signs.\n");
for(i = 0; i < *nitems; i++) {
    char *t = items[i].c;
    if(items[i].minus) printf("minus ");
    if(items[i].not_sign) printf("not ");
    for(j = 0; t[j] != '\0'; j++) {
	if(t[j] == '\b') printf("-");
	else if(t[j] == '\f') printf("+");
	else printf("%c", t[j]);
    }
    printf(" ");
}
printf("\n");
#endif

    return STRING_RETURNED;
}

ParseVar AppParse::checkSyntax(int *nitems, Item *items, bool logical,
				char *line)
{
    int i, i1, i2, n, m;

    // check parentheses syntax
    for(i = n = 0; i < *nitems; i++) if(items[i].oper == '(') n++;
    for(i = m = 0; i < *nitems; i++) if(items[i].oper == ')') m++;
    if(n > m) {
	printParseError("cannot interpret: %s\nmissing ')'", line);
	return VARIABLE_ERROR;
    }
    else if(n < m) {
	printParseError("cannot interpret: %s\nmissing '('", line);
	return VARIABLE_ERROR;
    }

    // Check for invalid char after ')'. Must be another ')', or an operator
    for(i = 0; i < *nitems-1; i++) {
	if(items[i].oper == ')' &&
		(items[i+1].oper == '(' || items[i+1].oper == '\0') )
	{
	    printParseError("cannot interpret: %s", line);
	    return VARIABLE_ERROR;
	}
    }

    // Look for functions. Must handle functions inside of functions:
    // fabs(2.*sin(24*degees_to_radians))

    for(i = 0; i < *nitems; i++) {
	items[i].flag = (items[i].oper == '(' || items[i].oper == ')');
    }
    for(;;) { // process the inner parentheses first
	for(i1 = 0; i1 < *nitems
		&& (!items[i1].flag || items[i1].oper != '('); i1++);
	if(i1 == *nitems) break; // no more parentheses
	
	// find i1 = '(' and i2 = ')'
	for(i2 = i1+1; i2 < *nitems; i2++) {
	    if(items[i2].flag && items[i2].oper == '(') {
		i1 = i2;
	    }
	    else if(items[i2].flag && items[i2].oper == ')') {
		// if the item before this '(' is not an operator,
		// then it must be a function name
		if(i1 > 0 && items[i1-1].oper == '\0') {
		    items[i1].func = items[i1-1].c;
		    items[i1].minus = items[i1-1].minus;
		    items[i1].not_sign = items[i1-1].not_sign;
		    // flag this item (the function name) to be removed
		    items[i1-1].c = NULL;
		    Free(items[i1-1].func);
		    items[i1-1].oper = '\0';

		    for(i = i1+1; i <= i2; i++) {
			// if this is not already an argument of an inner
			// function, then make it argument of this function
			if(items[i].c && !items[i].func) {
			    items[i].func = strdup(items[i1].func);
			}
		    }
		}
		items[i1].flag = false; // mark these parentheses done
		items[i2].flag = false;
		break;
	    }
	}
    }
    // remove flagged items.
    n = 0;
    for(i = 0; i < *nitems; i++) {
	if(items[i].c || items[i].func || items[i].oper != '\0') {
	    items[n++] = items[i];
	}
    }

    // Check that all commas are function arguments.
    for(i = 1; i < *nitems; i++) {
	if(items[i].c[0] == ',') {
	    if(!items[i].func) {
		printParseError("cannot interpret comma: %s", line);
		return VARIABLE_ERROR;
	    }
	}
    }
    if( !logical ) {
	// Check that string items are function arguments.
	// In an arithmetic expression all strings must be function arguments.
	for(i = 1; i < *nitems; i++) {
	    char c = items[i].c[0];
	    if(c == '"' || c == '\'' || c == '`') {
		if(!items[i].func) {
		    printParseError("cannot interpret quotes: %s", line);
		    return VARIABLE_ERROR;
		}
	    }
	}
    }
    *nitems = n;

    return STRING_RETURNED;
}

ParseVar AppParse::evaluateTerms(Component *top, int nitems, Item *items,
				string &msg)
{
    int i, j;
    char c, *s;
    ParseVar ret;

    for(i = 0; i < nitems; i++)
    {
	items[i].type = STRING_ITEM;
	c = items[i].c[0];

	if(items[i].oper=='\0' && c!='"' && c!='`' && c!='\''
		&& (!items[i].func || strcasecmp(items[i].func, "defined")))
	{
	    if(!strcasecmp(items[i].c, "true")) {
		// nothing to do
	    }
	    else if(!strcasecmp(items[i].c, "false")) {
		// nothing to do
	    }
	    else if(stringToLong(items[i].c, &items[i].l)) {
		items[i].type = INT_ITEM;
	    }
	    else if(parseToDouble(items[i].c, &items[i].d)) {
		items[i].type = DOUBLE_ITEM;
	    }
	    else {
		if(getVariable(items[i].c, &s) == VARIABLE_ERROR) {
		    return VARIABLE_ERROR;
		}
		else if( !s ) {
		    ret = parseWithPrefix(top, items[i].c, msg);
		    if(ret == STRING_RETURNED || ret == VARIABLE_TRUE ||
				ret == VARIABLE_FALSE)
		    {
			s = strdup(msg.c_str());
		    }
		    else {
			printParseError("undefined variable: %s", items[i].c);
			return VARIABLE_NOT_FOUND;
		    }
		}
		if(stringToLong(s, &items[i].l)) {
		    items[i].type = INT_ITEM;
		}
		else if(parseToDouble(s, &items[i].d)) {
		    items[i].type = DOUBLE_ITEM;
		}
		else {
		    items[i].type = STRING_ITEM;
		    Free(items[i].c);
		    items[i].c = s;
		    c = s[0];
		    if(c == '"' || c == '\'' || c == '`') {
			// remove the quotes
			for(j = 0; s[j+1] != c && s[j+1] != '\0'; j++) {
			    s[j] = s[j+1];
			}
			s[j] = '\0';
		    }
		}
		if(items[i].type != STRING_ITEM) Free(s);
	    }
	    if(items[i].minus) {
		if(items[i].type == INT_ITEM) items[i].l = -items[i].l;
		else if(items[i].type == DOUBLE_ITEM) items[i].d = -items[i].d;
		else {
		   printParseError("cannot interpret '-' before %s",items[i].c);
		    return VARIABLE_ERROR;
		}
		items[i].minus = false;
	    }
	    if(items[i].not_sign) {
		if(items[i].type == INT_ITEM) items[i].l = !items[i].l;
		else if(items[i].type == DOUBLE_ITEM) items[i].d = !items[i].d;
		else if(items[i].type == STRING_ITEM) {
		    if(!strcasecmp(items[i].c, "true")) {
			items[i].c[0] = '0';
			items[i].c[1] = '\0';
		    }
		    else if(!strcmp(items[i].c, "1")) {
			items[i].c[0] = '0';
		    }
		    else if(!strcasecmp(items[i].c, "false")) {
			items[i].c[0] = '1';
			items[i].c[1] = '\0';
		    }
		    else if(!strcmp(items[i].c, "0")) {
			items[i].c[0] = '1';
		    }
		    else {
			printParseError("cannot interpret '!' before %s",
				items[i].c);
			return VARIABLE_ERROR;
		    }
		}
		items[i].not_sign = false;
	    }
	}
	else if(items[i].oper=='\0' && (c=='"' || c=='`' || c=='\''))
	{
	    // remove the quotes
	    for(j = 0; items[i].c[j+1] != c && items[i].c[j+1] != '\0'; j++) {
		items[i].c[j] = items[i].c[j+1];
	    }
	    items[i].c[j] = '\0';
	}
    }
    return STRING_RETURNED;
}

ParseVar AppParse::parseParen(Component *top, Item *items, int *nitems,
			int i1, int i2, string &msg)
{
    bool is_int;
    Item *a, *b;
    long cl;
    double cd;
    char oper, c[2];
    int i, j, k;

    msg.clear();

#ifdef DEBUG_APP 
printf("Paren: ");
for(i = i1; i <= i2; i++) {
char c = (items[i].c) ? items[i].c[0] : '\0';
if(items[i].minus) printf("-");
if(items[i].func && items[i].oper == '(') printf("%s ", items[i].func);
if(items[i].oper != '\0') {
    if(items[i].oper == '=') printf("== ");
    else if(items[i].oper == '\b') printf("~= ");
    else if(items[i].oper == '\f') printf("^= ");
    else if(items[i].oper == '\v') printf("!= ");
    else if(items[i].oper == '\n') printf(">= ");
    else if(items[i].oper == '\r') printf("<= ");
    else if(items[i].oper == '&') printf("&& ");
    else if(items[i].oper == '|') printf("|| ");
    else printf("%c ", items[i].oper);
}
else if(c=='"' || c=='\'' || c=='`') printf("%s ", items[i].c);
else if(items[i].type==INT_ITEM) printf("%ld ", items[i].l);
else printf("%.15g ", items[i].d);
}
printf("\n");
#endif

    for(;;)
    {
	oper = '\0';
	for(i = i1+1; i < i2; i++) {
	    if(items[i].oper == '*') {
		a = &items[i-1];
		b = &items[i+1];
		oper = items[i].oper;
		break;
	    }
	}
	if( oper == '\0' ) {
	    for(i = i1+1; i < i2; i++) {
		if(items[i].oper == '/') {
		    a = &items[i-1];
		    b = &items[i+1];
		    oper = items[i].oper;
		    break;
		}
	    }
	}
	if( oper == '\0' ) {
	    for(i = i1+1; i < i2; i++) {
		if(items[i].oper == '+' || items[i].oper == '-') {
		    a = &items[i-1];
		    b = &items[i+1];
		    oper = items[i].oper;
		    break;
		}
	    }
	}
	if( oper == '\0' ) {
	    c[1] = '\0';
	    for(i = i1+1; i < i2; i++) {
		c[0] = items[i].oper;
		if(strpbrk(c, "=~^\v<>\n\r")) {
		    a = &items[i-1];
		    b = &items[i+1];
		    oper = items[i].oper;
		    break;
		}
	    }
	}
	if( oper == '\0' ) {
	    for(i = i1+1; i < i2; i++) {
		if(items[i].oper == '&' || items[i].oper == '|') {
		    a = &items[i-1];
		    b = &items[i+1];
		    oper = items[i].oper;
		    break;
		}
	    }
	}
	
	if(oper == '\0') break;

	cl = 0;
	cd = 0.;
	if(oper == '+' || oper == '-' || oper == '*' || oper == '/')
	{
	    is_int = false;
	    if(a->type == INT_ITEM && b->type == INT_ITEM) {
		if(oper == '+')		cl = a->l + b->l;
		else if(oper == '-')	cl = a->l - b->l;
		else if(oper == '*')	cl = a->l * b->l;
		else			cl = a->l / b->l;
		is_int = true;
#ifdef DEBUG_APP 
printf("%ld %c %ld = %ld\n", a->l, oper, b->l, cl);
#endif
	    }
	    else if(a->type == DOUBLE_ITEM && b->type == INT_ITEM) {
		if(oper == '+') 	cd = a->d + b->l;
		else if(oper == '-')	cd = a->d - b->l;
		else if(oper == '*')	cd = a->d * b->l;
		else 			cd = a->d / b->l;
#ifdef DEBUG_APP 
printf("%.15g %c %ld = %.15g\n", a->d, oper, b->l, cd);
#endif
	    }
	    else if(a->type == INT_ITEM && b->type == DOUBLE_ITEM) {
		if(oper == '+') 	cd = a->l + b->d;
		else if(oper == '-')	cd = a->l - b->d;
		else if(oper == '*')	cd = a->l * b->d;
		else 			cd = a->l / b->d;
#ifdef DEBUG_APP 
printf("%ld %c %.15g = %.15g\n", a->l, oper, b->d, cd);
#endif
	    }
	    else if(a->type == DOUBLE_ITEM && b->type == DOUBLE_ITEM) {
		if(oper == '+')		cd = a->d + b->d;
		else if(oper == '-')	cd = a->d - b->d;
		else if(oper == '*')	cd = a->d * b->d;
		else		cd = a->d / b->d;
#ifdef DEBUG_APP 
printf("%.15g %c %.15g = %.15g\n", a->d, oper, b->d, cd);
#endif
	    }
	    else if(a->type == STRING_ITEM) {
		printParseError("cannot interpret: %s %c", a->c, oper);
		return VARIABLE_ERROR;
	    }
	    else if(b->type == STRING_ITEM) {
		printParseError("cannot interpret: %c %s", oper, b->c);
		return VARIABLE_ERROR;
	    }
	}
	else if(oper != '&' && oper != '|')
	{
	    is_int = true;
	    if(a->type == INT_ITEM && b->type == INT_ITEM) {
		if(oper == '=')       cl = (a->l == b->l);
		else if(oper == '~') cl = (a->l == b->l);
		else if(oper == '^') cl = (a->l == b->l);
		else if(oper == '\v') cl = (a->l != b->l);
		else if(oper == '<')  cl = (a->l <  b->l);
		else if(oper == '>')  cl = (a->l >  b->l);
		else if(oper == '\n') cl = (a->l >= b->l);
		else if(oper == '\r') cl = (a->l <= b->l);
	    }
	    else if(a->type == DOUBLE_ITEM && b->type == INT_ITEM) {
		if(oper == '=')       cl = (a->d == b->l);
		else if(oper == '~') cl = (a->d == b->l);
		else if(oper == '^') cl = (a->d == b->l);
		else if(oper == '\v') cl = (a->d != b->l);
		else if(oper == '<')  cl = (a->d <  b->l);
		else if(oper == '>')  cl = (a->d >  b->l);
		else if(oper == '\n') cl = (a->d >= b->l);
		else if(oper == '\r') cl = (a->d <= b->l);
	    }
	    else if(a->type == INT_ITEM && b->type == DOUBLE_ITEM) {
		if(oper == '=')       cl = (a->l == b->d);
		else if(oper == '~') cl = (a->l == b->d);
		else if(oper == '^') cl = (a->l == b->d);
		else if(oper == '\v') cl = (a->l != b->d);
		else if(oper == '<')  cl = (a->l <  b->d);
		else if(oper == '>')  cl = (a->l >  b->d);
		else if(oper == '\n') cl = (a->l >= b->d);
		else if(oper == '\r') cl = (a->l <= b->d);
	    }
	    else if(a->type == DOUBLE_ITEM && b->type == DOUBLE_ITEM) {
		if(oper == '=')       cl = (a->d == b->d);
		else if(oper == '~') cl = (a->d == b->d);
		else if(oper == '^') cl = (a->d == b->d);
		else if(oper == '\v') cl = (a->d != b->d);
		else if(oper == '<')  cl = (a->d <  b->d);
		else if(oper == '>')  cl = (a->d >  b->d);
		else if(oper == '\n') cl = (a->d >= b->d);
		else if(oper == '\r') cl = (a->d <= b->d);
	    }
	    else if(a->type == STRING_ITEM && b->type == STRING_ITEM) {
		if(oper == '=')       cl = !strcmp(a->c, b->c);
		else if(oper == '~') cl = !strcasecmp(a->c, b->c);
		else if(oper == '^') {
			int na = (int)strlen(a->c);
			int nb = (int)strlen(b->c);
			int n = (na < nb) ? na : nb;
			cl = !strncasecmp(a->c, b->c, n);
		}
		else if(oper == '\v') cl =  strcmp(a->c, b->c);
		else if(oper == '<')  cl = (strcmp(a->c, b->c) < 0);
		else if(oper == '>')  cl = (strcmp(a->c, b->c) > 0);
		else if(oper == '\n') cl = (strcmp(a->c, b->c) >= 0);
		else if(oper == '\r') cl = (strcmp(a->c, b->c) <= 0);
	    }
	    else if(oper == '\v') {
		cl = 0;
	    }
	}
	else // oper == '&' || oper == '|'
	{
	    const char *op = (oper == '&') ? "&&" : "||";

	    if(a->type == STRING_ITEM) {
		if(!strcasecmp(a->c, "true") || !strcasecmp(a->c, "1")) {
		    a->type = INT_ITEM;
		    a->l = 1;
		}
		else if(!strcasecmp(a->c, "false") || !strcasecmp(a->c, "0")) {
		    a->type = INT_ITEM;
		    a->l = 0;
		}
		else {
		    printParseError("cannot interpret: %s %s", op, a->c);
		    return VARIABLE_ERROR;
		}
	    }
	    else if(a->type == DOUBLE_ITEM) {
		printParseError("cannot interpret: %.15g %s", a->d, op);
		return VARIABLE_ERROR;
	    }

	    if(b->type == STRING_ITEM) {
		if(!strcasecmp(b->c, "true") || !strcasecmp(b->c, "1")) {
		    b->type = INT_ITEM;
		    b->l = 1;
		}
		else if(!strcasecmp(b->c, "false") || !strcasecmp(b->c, "0")) {
		    b->type = INT_ITEM;
		    b->l = 0;
		}
		else {
		    printParseError("cannot interpret: %s %s", op, b->c);
		    return VARIABLE_ERROR;
		}
	    }
	    else if(a->type == DOUBLE_ITEM) {
		printParseError("cannot interpret: %s %.15g", op, a->d);
		return VARIABLE_ERROR;
	    }

	    if(oper == '&') {
		cl = (a->l && b->l);
	    }
	    else { // oper == '|'
		cl = (a->l || b->l);
	    }
	    is_int = true;
	}
	Free(items[i-1].c);
	Free(items[i].c);
	Free(items[i+1].c);
	items[i-1].type = is_int ? INT_ITEM : DOUBLE_ITEM;
	items[i-1].l = cl;
	items[i-1].d = cd;

	for(j=i+2, k=0; j < *nitems; j++, k++) {
	    items[i+k] = items[j];
	}
	*nitems = *nitems - 2;
	i2 -= 2;
#ifdef DEBUG_APP 
printf("i2=%d *nitems=%d\n", i2, *nitems);
#endif
	if(i2 <= i1) break;
    }

    return STRING_RETURNED;
}

bool AppParse::callFunc(Component *top, Item *items, int i1, int *nitems)
{
    char s[1000], value[200];
    char *func, *prop, *last, *tok;
    string msg, tmpstr;
    double d=0.;
    ParseVar ret;
    bool string_size = false;
    int i, j, n, nargs=0, need;

    memset(value, 0, sizeof(value));

    for(i = i1+1; i < *nitems && items[i].func; i++) {
	if(items[i].oper != ',' && items[i].oper != ')') nargs++;
    }
    func = items[i1].func;
    items[i1].oper = '\0';
    items[i1].type = DOUBLE_ITEM;
    Free(items[i1].c);

    if(nargs == 0) {
	// check for string.size()
	if(stringEndsWith(func, ".size")) {
	    n = (int)strlen(func) - 5;
	    if(n < (int)sizeof(s)-1) {
		strncpy(s, func, n);
		s[n] = '\0';
		if(getVariable(s, &prop) == VARIABLE_ERROR) {
		    return false;
		}
		else if( prop ) {
		    tok = prop;
		    n = 0;
		    if(strcmp(tok, "\"\"")) {
			while(strtok_r(tok, ",", &last)) {tok=NULL; n++;}
		    }
		    free(prop);
		    items[i1].type = INT_ITEM;
		    items[i1].l = n;
		    string_size = true;
		}
	    }
	}
    }
    else if(nargs == 1) {
	if(items[i1+1].type==INT_ITEM) d = (double)items[i1+1].l;
	else d = items[i1+1].d;
    }

    if(string_size) {
    }
    else if(!strcasecmp(func, "defined")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type!=STRING_ITEM) goto ARG_STRING;
	items[i1].type = INT_ITEM;
	items[i1].l = 0;
	if(parseToDouble(items[i1+1].c, &d)) {
	    items[i1].l = 1;
	}
	else if(getVariable(items[i1+1].c, &prop) == VARIABLE_ERROR) {
	    return false;
	}
	else if( prop ) {
	    items[i1].l = 1;
	    free(prop);
	}
	else if(parseWithPrefix(top, items[i1+1].c, msg) == STRING_RETURNED)
	{
	    items[i1].l = 1;
	}
    }
    else if(!strcasecmp(func, "int")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+1].type==DOUBLE_ITEM) items[i1].l = (int)items[i1+1].d;
	else items[i1].l = items[i1+1].l;
	items[i1].type = INT_ITEM;
    }
    else if(!strcasecmp(func, "double")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+1].type==INT_ITEM) items[i1].d = (double)items[i1+1].l;
	else items[i1].d = items[i1+1].d;
	items[i1].type = DOUBLE_ITEM;
    }
    else if(!strcasecmp(func, "floor")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = floor(d);
    }
    else if(!strcasecmp(func, "ceil")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = ceil(d);
    }
    else if(!strcasecmp(func, "fabs")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = fabs(d);
    }
    else if(!strcasecmp(func, "abs")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+1].type==INT_ITEM) {
	    items[i1].l = (items[i1+1].l >= 0) ? items[i1+1].l : -items[i1+1].l;
	    items[i1].type = INT_ITEM;
	}
	else items[i1].d = fabs(d);
    }
    else if(!strcasecmp(func, "sqrt")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = sqrt(d);
    }
    else if(!strcasecmp(func, "sin")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = sin(d);
    }
    else if(!strcasecmp(func, "cos")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = sin(d);
    }
    else if(!strcasecmp(func, "tan")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = tan(d);
    }
    else if(!strcasecmp(func, "asin")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = asin(d);
    }
    else if(!strcasecmp(func, "acos")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = asin(d);
    }
    else if(!strcasecmp(func, "atan")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = asin(d);
    }
    else if(!strcasecmp(func, "log")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = log(d);
    }
    else if(!strcasecmp(func, "exp")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = exp(d);
    }
    else if(!strcasecmp(func, "log10")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	items[i1].d = log10(d);
    }
    else if(!strcasecmp(func, "time")) {
	if(nargs != 0) {need = 0; goto NUM_ARGS;}
	items[i1].type = DOUBLE_ITEM;
	items[i1].d = timeGetEpoch();
    }
    else if(!strcasecmp(func, "epoch")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	// if the argument is int or double, assume it's already epoch time
	if(items[i1+1].type==INT_ITEM) {
	    items[i1].d = (double)items[i1+1].l;
	}
	else if(items[i1+1].type==DOUBLE_ITEM) {
	    items[i1].d = items[i1+1].d;
	}
	else { // string
	    // don't input the quotes '"'
	    if(items[i1+1].c[0] == '"' || items[i1+1].c[0] == '\'' || 
		items[i1+1].c[0] == '`') {
		strncpy(s, items[i1+1].c+1, sizeof(s)-1);
		s[(int)strlen(s)-1] = '\0';
	    }
	    else {
		strncpy(s, items[i1+1].c, sizeof(s)-1);
	    }
	    if( timeParseString(s, &items[i1].d) != 1 ) {
		printParseError("epoch: cannot parse argument");
		return false;
	    }
	}
    }
    else if(!strcasecmp(func, "etoh")) {
	if(nargs != 1) {need = 1; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	timeEpochToString(d, value, sizeof(value), YMONDHMS);
	items[i1].type = STRING_ITEM;
	items[i1].c = strdup(value);
    }
    else if(!strcasecmp(func, "atan2")) {
	if(nargs != 2) {need = 2; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+3].type==STRING_ITEM) goto ARG_NUMERIC2;
	if(items[i1+1].type==INT_ITEM) items[i1+1].d = (double)items[i1+1].l;
	if(items[i1+3].type==INT_ITEM) items[i1+3].d = (double)items[i1+3].l;
	items[i1].d = atan2(items[i1+1].d, items[i1+3].d);
    }
    else if(!strcasecmp(func, "pow")) {
	if(nargs != 2) {need = 2; goto NUM_ARGS;}
	if(items[i1+1].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+3].type==STRING_ITEM) goto ARG_NUMERIC2;
	if(items[i1+1].type==INT_ITEM) items[i1+1].d = (double)items[i1+1].l;
	if(items[i1+3].type==INT_ITEM) items[i1+3].d = (double)items[i1+3].l;
	items[i1].d = pow(items[i1+1].d, items[i1+3].d);
    }
    else if(!strcasecmp(func, "maxabs")) {
	if(!maxabs(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeString")) {
	if(!writeString(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeInt")) {
	if(!writeInt(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeLong")) {
	if(!writeLong(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeFloat")) {
	if(!writeFloat(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeDouble")) {
	if(!writeDouble(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "writeArray")) {
	if(!writeArray(items, i1, nargs)) return false;
    }
    else if(!strcasecmp(func, "substr")) {
	if(nargs != 3) {need = 3; goto NUM_ARGS;}
	if(items[i1+3].type==STRING_ITEM) goto ARG_NUMERIC;
	if(items[i1+5].type==STRING_ITEM) goto ARG_NUMERIC2;
	tmpstr = items[i1+1].c; 
	i = (int)items[i1+3].l;
	j = (int)items[i1+5].l;
		if ((int)tmpstr.size() > i) {
			tmpstr = tmpstr.substr(i,j);	
			items[i1].type = STRING_ITEM;
			items[i1].c = strdup(tmpstr.c_str());
		}
		else
		{
			printParseError("%s: first argument out of range", func);
		        return false;	
		}
    }
    
    else {
	snprintf(s, sizeof(s), "%s_", func);
	j = 1;
	for(i=i1+1; i < *nitems && items[i].func; i++)
		if(items[i].oper != ',' && items[i].oper != ')')
	{
	    n = (int)strlen(s);
	    if(items[i].type==INT_ITEM) {
		snprintf(s+n, sizeof(s)-n, " arg%d=%ld", j, items[i].l);
	    }
	    else if(items[i].type==DOUBLE_ITEM) {
		snprintf(s+n, sizeof(s)-n, " arg%d=%.15g", j, items[i].d);
	    }
	    else if(items[i].type==STRING_ITEM) {
		snprintf(s+n, sizeof(s)-n, " arg%d=\"%s\"", j, items[i].c);
	    }
	    j++;
	}

#ifdef DEBUG_APP 
printf("callFunc: %s\n", s);
#endif
	ret = parseWithPrefix(top, s, msg);

	if(ret == VARIABLE_NOT_FOUND) {
	    printParseError("unknown function: %s", func);
	    return false;
	}
	else if(ret != STRING_RETURNED) {
	    printParseError("function call failed: %s", msg.c_str());
	    return false;
	}
	else if(stringToLong(msg.c_str(), &items[i1].l)) {
	    items[i1].type = INT_ITEM;
	}
	else if(stringToDouble(msg.c_str(), &items[i1].d)) {
	    items[i1].type = DOUBLE_ITEM;
	}
	else {
	    items[i1].type = STRING_ITEM;
	    n = (int)msg.length() + 2;
	    items[i1].c = (char *)malloc(n+1);
	    snprintf(items[i1].c, n+1, "\"%s\"", msg.c_str());
	}
    }
    if(items[i1].minus) {
	if(items[i1].type==INT_ITEM) items[i1].l = -items[i1].l;
	else items[i1].d = -items[i1].d;
	items[i1].minus = false;
    }
    if(items[i1].not_sign) {
	if(items[i1].type==INT_ITEM) items[i1].l = !items[i1].l;
	else items[i1].d = !items[i1].d;
	items[i1].not_sign = false;
    }

    // The result goes in the '(' item which is items[i1]
    // Remove all the argument items and the ')' item.
    for(i = i1+1; i < *nitems && items[i].func
		&& !strcasecmp(items[i].func, func); i++)
    {
	Free(items[i].c);
	Free(items[i].func);
    }
    // Shift the remaining items forward.
    j = i1+1;
    while(i < *nitems) items[j++] = items[i++];
    *nitems = j;

    items[i1].oper = '\0';
    Free(items[i1].func);
    if(i1 > 0 && items[i1-1].oper && items[i1-1].func) {
	items[i1].func = strdup(items[i1-1].func);
    }

    return true;

NUM_ARGS:
    if(need == 1) printParseError("%s: need %d argument", func, need);
    else printParseError("%s: need %d arguments", func, need);
    return false;

ARG_STRING:
    printParseError("%s: expecting string argument", func);
    return false;

ARG_NUMERIC:
    printParseError("%s: cannot evaluate argument numerically", func);
    return false;

ARG_NUMERIC2:
    printParseError("%s: cannot evaluate second argument numerically", func);
    return false;
}

bool AppParse::maxabs(Item *items, int i1, int nargs)
{
    char *c;
    int i, j, n, start, end;
    long addr;

    if(nargs == 0) {
	printParseError("maxabs: missing argument(s)");
	return false;
    }
    if(items[i1+1].type != STRING_ITEM) {
	printParseError("maxabs: expecting string argument");
	return false;
    }
    c = items[i1+1].c;
    if( (strncmp(c, "@f", 2) && strncmp(c, "@d", 2))
	|| sscanf(c+2, "%ld:%d", &addr, &n) != 2)
    {
	printParseError("maxabs: cannot interpret arguments");
	return false;
    }
    start = 0;
    end = n-1;
    if(nargs >= 2) {
	if(items[i1+2].type != INT_ITEM) {
	    printParseError("maxabs: expecting integer for argument 2");
	    return false;
	}
	start = items[i1+2].l-1;
	if(start < 0 || start >= n) {
	    if(start < 0) printParseError("maxabs: start < 1");
	    else printParseError("maxabs: start > array length");
	    return false;
	}
    }
    if(nargs == 3) {
	if(items[i1+3].type != INT_ITEM) {
	    printParseError("maxabs: expecting integer for argument 3");
	    return false;
	}
	end = items[i1+3].l-1;
	if(end < start || end >= n) {
	    if(end < start) printParseError("maxabs: end < start");
	    else printParseError("maxabs: end > array length");
	    return false;
	}
    }
    if(!strncmp(c, "@f", 2)) {
	float *f = (float *)addr, max=0.;
	for(i = start, j = start; i <= end; i++) {
	    if(fabs(f[i]) > max) { max = fabs(f[i]); j = i; }
	}
    }
    else { // "@d"
	double *f = (double *)addr, max=0.;
	for(i = start, j = start; i <= end; i++) {
	    if(fabs(f[i]) > max) { max = fabs(f[i]); j = i; }
	}
    }
    items[i1].l = j+1;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeFloat(Item *items, int i1, int nargs)
{
    int n;
    float f;

    if(!write_fp) {
	printParseError("writeFloat: file not open");
	return false;
    }
    if(nargs < 1) {
	printParseError("writeFloat: missing argument");
	return false;
    }
    if(nargs > 1) {
	printParseError("writeFloat: too many arguments");
	return false;
    }
    if(items[i1+1].type == INT_ITEM) {
	f = (float)items[i1+1].l;
	n = fwrite(&f, sizeof(float), 1, write_fp);
    }
    else if(items[i1+1].type == DOUBLE_ITEM) {
	f = (float)items[i1+1].d;
	n = fwrite(&f, sizeof(float), 1, write_fp);
    }
    else {
	printParseError("writeFloat: invalid argument type");
	return false;
    }

    items[i1].l = n;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeDouble(Item *items, int i1, int nargs)
{
    int n;

    if(!write_fp) {
	printParseError("writeDouble: file not open");
	return false;
    }
    if(nargs < 1) {
	printParseError("writeDouble: missing argument");
	return false;
    }
    if(nargs > 1) {
	printParseError("writeDouble: too many arguments");
	return false;
    }
    if(items[i1+1].type == INT_ITEM) {
	double d = (double)items[i1+1].l;
	n = fwrite(&d, sizeof(double), 1, write_fp);
    }
    else if(items[i1+1].type == DOUBLE_ITEM) {
	n = fwrite(&items[i1+1].d, sizeof(float), 1, write_fp);
    }
    else {
	printParseError("writeDouble: invalid argument type");
	return false;
    }

    items[i1].l = n;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeInt(Item *items, int i1, int nargs)
{
    int n;

    if(!write_fp) {
	printParseError("writeInt: file not open");
	return false;
    }
    if(nargs < 1) {
	printParseError("writeInt: missing argument");
	return false;
    }
    if(nargs > 1) {
	printParseError("writeInt: too many arguments");
	return false;
    }
    if(items[i1+1].type == INT_ITEM) {
	int i = (int)items[i1+1].l;
	n = fwrite(&i, sizeof(int), 1, write_fp);
    }
    else {
	printParseError("writeInt: invalid argument type");
	return false;
    }

    items[i1].l = n;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeString(Item *items, int i1, int nargs)
{
    int n;
    char *c;

    if(!write_fp) {
	printParseError("writeString: file not open");
	return false;
    }
    if(nargs < 1) {
	printParseError("writeString: missing argument");
	return false;
    }
    if(nargs > 1) {
	printParseError("writeString: too many arguments");
	return false;
    }

    c = items[i1+1].c;

    if(items[i1+1].type == STRING_ITEM && strncmp(c, "@f", 2) &&
	strncmp(c, "@d", 2) && strncmp(c, "@i", 2) && strncmp(c, "@l", 2))
    {
	int len = strlen(c);
	n = fwrite(c, sizeof(char), len, write_fp);
    }
    else {
	printParseError("writeString: invalid argument type");
	return false;
    }

    items[i1].l = n;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeLong(Item *items, int i1, int nargs)
{
    int n;

    if(!write_fp) {
	printParseError("writeLong: file not open");
	return false;
    }
    if(nargs < 1) {
	printParseError("writeLong: missing argument");
	return false;
    }
    if(nargs > 1) {
	printParseError("writeLong: too many arguments");
	return false;
    }
    if(items[i1+1].type == INT_ITEM) {
	n = fwrite(&items[i1+1].l, sizeof(long), 1, write_fp);
    }
    else {
	printParseError("writeLong: invalid argument type");
	return false;
    }

    items[i1].l = n;
    items[i1].type = INT_ITEM;
    return true;
}

bool AppParse::writeArray(Item *items, int i1, int nargs)
{
    char *c;
    int n, start, num;
    long addr;

    if(!write_fp) {
	printParseError("writeArray: file not open");
	return false;
    }
    if(nargs != 1 && nargs != 3) {
	printParseError("writeArray: need 1 or 3 arguments");
	return false;
    }
    if(items[i1+1].type != STRING_ITEM) {
	printParseError("writeArray: argument 1 is invalid type");
	return false;
    }

    c = items[i1+1].c;

    if( strncmp(c, "@f", 2) && strncmp(c, "@d", 2) &&
	strncmp(c, "@i", 2) && strncmp(c, "@l", 2))
    {
	printParseError("writeArray: argument 1 is invalid type");
	return false;
    }

    if(sscanf(c+2, "%ld:%d", &addr, &n) != 2) {
	printParseError("writeArray: cannot interpret argument 1");
	return false;
    }

    start = 0;
    num = n;
    if(nargs >= 2) {
	if(items[i1+2].type != INT_ITEM) {
	    printParseError("writeArray: expecting integer for argument 2");
	    return false;
	}
	start = items[i1+2].l-1;
	if(start < 0) {
	    printParseError("writeArray: start(%d) < 1", start+1);
	    return false;
	}
	else if(start >= n) {
	    printParseError("writeArray: start(%d) > array length(%d)",
			start+1, n);
	    return false;
	}
    }
    if(nargs == 3) {
	if(items[i1+3].type != INT_ITEM) {
	    printParseError("writeArray: expecting integer for argument 3");
	    return false;
	}
	num = items[i1+3].l;
	if(num < 0) {
	    printParseError("writeArray: number of elements(%d) < 0", num);
	    return false;
	}
	else if(start + num > n) {
	    printParseError(
		"writeArray: number of elements(%d) > array length(%d)", num,n);
	    return false;
	}
    }
    if(!strncmp(c, "@f", 2)) {
	n = fwrite((float *)addr+start, sizeof(float), num, write_fp);
    }
    else if(!strncmp(c, "@d", 2)) {
	n = fwrite((double *)addr+start, sizeof(double), num, write_fp);
    }
    else if(!strncmp(c, "@i", 2)) {
	n = fwrite((int *)addr+start, sizeof(int), num, write_fp);
    }
    else if(!strncmp(c, "@l", 2)) {
	n = fwrite((long *)addr+start, sizeof(long), num, write_fp);
    }
    items[i1].l = n;
    items[i1].type = INT_ITEM;

    return true;
}

bool AppParse::sprintCmd(Component *top, char *line, char sep, string &msg)
{
    int n;
    char *c, *e, *name, *s=NULL;

    c = line;
    while(*c != '\0' && isspace((int)*c)) c++;
    if(*c == '\0') {
	printParseError("sprint: missing string_name.");
	return false;
    }
    e = c+1;
    while(*e != '\0' && !isspace((int)*e)) e++;

    if(*e == '\0') {
	printParseError("sprint: missing print arguments.");
	return false;
    }

    if(printCmd(top, e, msg, sep, &s)) {
	n = e-c;
	name = (char *)malloc(n+1);
	strncpy(name, c, n);
	name[n] = '\0';
	putVariable(name, s);
	Free(name);
	Free(s)
	return true;
    }
    Free(s);
    return false;
}

bool AppParse::printCmd(Component *top, char *line, string &msg, char sep,
			char **to_string, bool isprintf )
{
    ParseVar ret;
    int i, lasti, n, m;
    bool must_be_format;
    bool carret = false;
    long l;
    double d;
    string str, outp;
    string str2 = "\\n";
    char out[100000], buf[100000], *o, fm[50], sp[2];

    char *c = line, b, *s, *fmt, *p;
    vector<char *> objects, prefixes;

    memset(&out, 0, sizeof(out));

    sp[0] = sep;
    sp[1] = '\0';

//	printf("Executing PrintCmd \n");
//	printf("%s \n",line);
	

    while( *c != '\0' ) {
	while(isspace((int)*c) && *c != '\0') c++;
	
	if(*c == '\'' || *c == '`' || *c == '"' || *c == '{') {
	    // the item is a quoted string. find the end.
	    b = (*c == '{') ? '}' : *c;
	    c++;
	    s = c;
	    while(*c != b && *c != '\0') c++;

	    if(*c == '\0') {
		printParseError("print: missing %c", b);
		return false;
	    }

	    
	    *c = '\0';
	    n = (int)strlen(out);
	    
	    if (isprintf) {
	    snprintf(out+n, sizeof(out)-n, "%s", s);
	    } 
	    else {
	    snprintf(out+n, sizeof(out)-n, "%s%s", s, sp);
	    }
	    
	    c++;
	    continue;
	}
	else if(*c == '(') // evaluate an expression within parentheses ()
	{
	    c++;
	    n = (int)sizeof(buf)-1;
	    m = 1;
	    for(i = 0; *c != '\0' && i < n; i++) {
		if(*c == '(') m++;
		else if(*c == ')' && --m == 0) break;
		buf[i] = *c++;
	    }
	    if(*c != ')') {
		printParseError("print: missing ')'");
		return false;
	    }
	    if(i >= n) {
		printParseError("print: line too long");
		return false;
	    }
	    c++;
	    buf[i] = '\0';
	    ret = parseExpression(top, buf, false, msg);
	    n = (int)sizeof(out) - (int)strlen(out);
	    o = out + strlen(out);
	    snprintf(o, n, "%s%s", msg.c_str(), sp);
	    if(ret == VARIABLE_NOT_FOUND) {
		return false;
	    }
	    else if(ret != STRING_RETURNED) {
		return false;
	    }
	    continue;
	}

	// the item does not start with a quote nor '('
	n = (int)sizeof(out) - (int)strlen(out);
	o = out + strlen(out);

	// got to '(' or the first space
	s = c;
	must_be_format = false;
	while(*s != '\0' && !isspace((int)*s) && *s != '(') {
	    // if the item has a invalid character for a function name, then
	    // if we find a '(', it should be a format
	    if( !isalpha((int)*s) && !isdigit((int)*s)
		&& *s != '.' && *s != '_') must_be_format = true;
	    if(*s == '.') must_be_format = false; // reset for each part
	    s++;
	}

	fmt = NULL;

	if(*s == '(')
	{
	    p = s; // save this pointer the the '(' after the variable

	    // if the first non-space character after the '(' is '%',
	    // then the item is a format, otherwise it is a function call
	    s++;
	    while(*s != '\0' && isspace((int)*s)) s++;
	    if(*s != '%') {
		if(must_be_format) {
		    printParseError("print: invalid function: %s", c);
		    return false;
		}
	
		// it is a function call. find the end parenthesis. can
		// have parentheses inside parentheses
		m = 1;
		while(*s != '\0' && m > 0) {
		    if(*s == '(') m++;
		    else if(*s == ')') m--;
		    s++;
		}
		if(m != 0) {
		    printParseError("print: missing ')': %s", c);
		    return false;
		}
		// s is at the character after ')'. temporary make
		// the string end here.
		b = *s; // save the character. put it back below
		*s = '\0';
		ret = parseExpression(top, c, false, msg);
		// check for an error from parseExpression. the
		// error message will have been printed already
		if(ret != STRING_RETURNED && ret != VARIABLE_TRUE
			&& ret != VARIABLE_FALSE) return false;
		
		*s = b; // put the character after ')' back
		c = s;

		if(*s == '(') { // should be a format. look for '%'
		    s++;
		    while(*s != '\0' && isspace((int)*s)) s++;
		    if(*s != '%') {
			printParseError("print: expecting '%' at: %s", s);
			return false;
		    }
		    // find the ')' end of the format.
		    while(*s != '\0' && *s != ')') s++;
		    if(*s != ')') {
			printParseError("print: missing ')': %s", c);
			return false;
		    }
		    fmt = c+1; // one char after '('
		    *s = '\0'; // the ')'

		    if(ret == STRING_RETURNED) {
			if(stringToLong(msg.c_str(), &l)) {
			    snprintf(fm, sizeof(fm), "%s%s", fmt, sp);
			    snprintf(o, n, fm, l);
			}
			else if(stringToDouble(msg.c_str(), &d)) {
			    strncpy(fm, "%s", sizeof(fm));
			    m = (int)sizeof(buf)-1;
			    if(!strncmp(fmt, "%t", 2)) {
				timeEpochToString(d, buf, m, YMONDHMS);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+2,sp);
				snprintf(o, n, fm, buf);
			    }
			    else if(!strncmp(fmt, "%2t", 3)) {
				timeEpochToString(d, buf, m, YMONDHMS3);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3,sp);
				snprintf(o, n, fm, buf);
			    }
			    else if(!strncmp(fmt, "%3t", 3)) {
				timeEpochToString(d, buf, m, GSE20);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3,sp);
				snprintf(o, n, fm, buf);
			    }
			    else if(!strncmp(fmt, "%4t", 3)) {
				timeEpochToString(d, buf, m, GSE21);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3,sp);
				snprintf(o, n, fm, buf);
			    }
			    else if(!strncmp(fmt, "%5t", 3)) {
				timeEpochToString(d, buf, m, YMOND);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3,sp);
				snprintf(o, n, fm, buf);
			    }
			    else if(!strncmp(fmt, "%6t", 3)) {
				timeEpochToString(d, buf, m, GSE22);
				snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3,sp);
				snprintf(o, n, fm, buf);
			    }
			    
		    
			    
			    else {
				snprintf(fm, sizeof(fm), "%s%s", fmt, sp);
				snprintf(o, n, fm, d);
			    }
			}
			else {
			    snprintf(fm, sizeof(fm), "%s%s", fmt, sp);
			    snprintf(o, n, fm, msg.c_str());
			}
		    }
		    else if(ret == VARIABLE_TRUE) {
			snprintf(o, n, "true%s", sp);
		    }
		    else if(ret == VARIABLE_FALSE) {
			snprintf(o, n, "false%s", sp);
		    }
		    *s = ')'; // put it back
		    c = s+1; // go on to the next item
		}
		else { // not format, just print the string
		    if(ret == STRING_RETURNED) {
			snprintf(o, n, "%s%s", msg.c_str(), sp);
		    }
		    else if(ret == VARIABLE_TRUE) {
			snprintf(o, n, "true%s", sp);
		    }
		    else if(ret == VARIABLE_FALSE) {
			snprintf(o, n, "false%s", sp);
		    }
		}
		continue; // it was a function call. continue with next item
	    }
	    else { // *s == '%' it's a format.
		fmt = s;
		// find the ')' ending the format
		m = 1;
		while(*s != '\0' && m > 0) {
		    if(*s == '(') m++;
		    else if(*s == ')') m--;
		    s++;
		}
		if(m != 0) {
		    printParseError("print: missing ')': %s", c);
		    return false;
		}
		// s is at the character after ')'.
		b = *s; // save the character. put it back below
		*(s-1) = '\0'; // fmt not points to the format
	    }
	}

	if( !fmt ) {
	    b = *s; // save the character. put it back below
	    *s = '\0'; // s not points to a space or '\0'

	    ret = parseExpression(top, c, false, msg);
	    if(ret == STRING_RETURNED) {
		if(!msg.empty() && msg[0] == '_' && strstr(msg.c_str(), "_=")) {
		    objects.push_back(strdup(msg.c_str()));
		    // get object prefix 
		    lasti = 0;
		    for(i = 0; c[i] != '\0'; i++) {
			if( !isalpha((int)c[i]) && !isdigit((int)c[i])
				&& c[i] != '_' && c[i] != '.' ) break;
			if(c[i] == '.') lasti = i;
		    }

//		    for(i = (int)strlen(c)-1; i >= 0 && c[i] != '.'; i--);
		    if(lasti > 0) {
			if(lasti > (int)sizeof(buf)-2) lasti=(int)sizeof(buf)-2;
			strncpy(buf, c, lasti+1);
			buf[lasti+1] = '\0';
		    }
		    else {
			buf[0] = '\0';
		    }
		    prefixes.push_back(strdup(buf));
		}
		else {
		    snprintf(o, n, "%s%s", msg.c_str(), sp);
		}
	    }
	    else if(ret == VARIABLE_TRUE) {
		snprintf(o, n, "true%s", sp);
	    }
	    else if(ret == VARIABLE_FALSE) {
		snprintf(o, n, "false%s", sp);
	    }
	    else if(ret == VARIABLE_NOT_FOUND) {
		return false;
	    }
	    else if(ret == VARIABLE_ERROR) {
		return false;
	    }
	    else {
		snprintf(o, n, "- ");
	    }
	    msg.clear();
	}
	else {
	    // variable is at c, *p == '(' the beginning of the format
	    // fmt points to the format.
	    // null p so c points to the variable name
	    *p = '\0';
	    formattedPrint(top, o, n, fmt, c, sp);
	}
	*s = b;
	c = s;
    }

    n = (int)strlen(out);
    if(n > 0) {
	if(out[n-1] == ' ') out[n-1] = '\0';
        if(to_string) {
	    *to_string = strdup(out);
	}
	else if(print_fp) {
	    fprintf(print_fp, "%s\n", out);
	}
	else {

//            printf ( "%s\n", out[n-2] );

	
	if(out[n-1] == 'n') {
		if(out[n-2] == '\\') {
		carret = true;
		out[n-2] = '\0';
		}
	}
		
	if (!carret && isprintf) {
	    printf("%s", out);
	    }
	    else 
	    {
	    printf("%s\n", out);		    
	    }

	}
    }
    else if(to_string) {
	*to_string = strdup("");
    }

    for(i = 0; i < (int)objects.size(); i++) {
	snprintf(out, sizeof(out), "%sprintObject %s", prefixes[i], objects[i]);
	free(prefixes[i]);
	free(objects[i]);
	parseLine(out, str);
    }

    return true;
}

bool AppParse::formattedPrint(Component *top, char *o, int n, char *fmt,
				char *c, char *sep)
{
    char buf[100000], fm[50], t[50];
    string str;
    ParseVar ret;
    int m;
    long l;
    double d;

    memset(buf, 0, sizeof(buf));


    if(stringToLong(c, &l)) {
	snprintf(fm, sizeof(fm), "%s%s", fmt, sep);
	snprintf(o, n, fm, l);
	return true;
    }
    else if(parseToDouble(c, &d)) {
	m = (int)sizeof(buf)-1;
	strncpy(fm, "%s", sizeof(fm));
	if(!strncmp(fmt, "%t", 2)) {
	    timeEpochToString(d, buf, m, YMONDHMS);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+2, sep);
	    snprintf(o, n, fm, buf);
	}
	else if(!strncmp(fmt, "%2t", 3)) {
	    timeEpochToString(d, buf, m, YMONDHMS3);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3, sep);
	    snprintf(o, n, fm, buf);
	}
	else if(!strncmp(fmt, "%3t", 3)) {
	    timeEpochToString(d, buf, m, GSE20);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3, sep);
	    snprintf(o, n, fm, buf);
	}
	else if(!strncmp(fmt, "%4t", 3)) {
	    timeEpochToString(d, buf, m, GSE21);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3, sep);
	    snprintf(o, n, fm, buf);
	}
	else if(!strncmp(fmt, "%5t", 3)) {
	    timeEpochToString(d, buf, m, YMOND);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3, sep);
	    snprintf(o, n, fm, buf);
	}

	else if(!strncmp(fmt, "%6t", 3)) {
	    timeEpochToString(d, buf, m, GSE22);
	    snprintf(fm+2, sizeof(fm)-2, "%s%s", fmt+3, sep);
	    snprintf(o, n, fm, buf);
	}
	
	else {
	    snprintf(fm, sizeof(fm), "%s%s", fmt, sep);
	    snprintf(o, n, fm, d);
	}
	return true;
    }

    ret = parseExpression(top, c, false, str);

    if(ret == STRING_RETURNED) {
	snprintf(buf, sizeof(buf), "%s", str.c_str());
	m = (int)sizeof(t)-1;

	if(buf[0] == '_' && strstr(buf, "_=")) {
	    snprintf(buf, sizeof(buf), "'use printObject %s'", c);
	    snprintf(o, n, "%s%s", buf, sep);
	}
	else if(!strncmp(fmt, "%t", 2) || !strncmp(fmt, "%2t", 3) ||
		!strncmp(fmt, "%3t", 3) || !strncmp(fmt, "%4t", 3) ||
		!strncmp(fmt, "%5t", 3) || !strncmp(fmt, "%6t", 3))
	{
	    if(!stringToDouble(buf, &d)) {
		printParseError(
		"print: expecting numeric argument with %%t format: %s(%s)",
			buf, fmt);
		return false;
	    }
	    strncpy(fm, "%s", sizeof(fm));
	    if(!strncmp(fmt, "%t", 2)) {
		timeEpochToString(d, t, m, YMONDHMS);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+2);
		snprintf(buf, sizeof(buf), fm, t);
	    }
	    else if(!strncmp(fmt, "%2t", 3)) {
		timeEpochToString(d, t, m, YMONDHMS3);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+3);
		snprintf(buf, sizeof(buf), fm, t);
	    }
	    else if(!strncmp(fmt, "%3t", 3)) {
		timeEpochToString(d, t, m, GSE20);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+3);
		snprintf(buf, sizeof(buf), fm, t);
	    }
	    else if(!strncmp(fmt, "%4t", 3)) {
		timeEpochToString(d, t, m, GSE21);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+3);
		snprintf(buf, sizeof(buf), fm, t);
	    }
	    else if(!strncmp(fmt, "%5t", 3)) {
		timeEpochToString(d, t, m, YMOND);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+3);
		snprintf(buf, sizeof(buf), fm, t);
	    }
    	    else if(!strncmp(fmt, "%6t", 3)) {
		timeEpochToString(d, t, m, GSE22);
		snprintf(fm+2, sizeof(fm)-2, "%s", fmt+3);
		snprintf(buf, sizeof(buf), fm, t);
	    }

	    snprintf(o, n, "%s%s", buf, sep);
	}
	else if(strstr(fmt, "f") || strstr(fmt, "e") || strstr(fmt, "g"))
	{
	    if(!stringToDouble(buf, &d)) {
		printParseError("print: expecting numeric argument: %s(%s)",
				buf, fmt);
		return false;
	    }
	    snprintf(buf, sizeof(buf), fmt, d);
	    snprintf(o, n, "%s%s", buf, sep);
	}
	else if(strstr(fmt, "d")) {
	    if(!stringToLong(buf, &l)) {
		printParseError("print: expecting integer argument: %s(%s)",
				buf, fmt);
		return false;
	    }
	    snprintf(buf, sizeof(buf), fmt, l);
	    snprintf(o, n, "%s%s", buf, sep);
	}
	else {
	    snprintf(fm, sizeof(fm), "%s%s", fmt, sep);
	    snprintf(o, n, fm, buf);
	}
    }
    else if(ret == VARIABLE_TRUE) {
	snprintf(o, n, "true%s", sep);
    }
    else if(ret == VARIABLE_FALSE) {
	snprintf(o, n, "false%s", sep);
    }
    else if(ret == VARIABLE_NOT_FOUND) {
	return false;
    }
    else if(ret == VARIABLE_ERROR) {
	return false;
    }
    else {
	snprintf(o, n, "-%s", sep);
    }
    return true;
}

bool AppParse::getLine(FILE *fp, char *line, int line_size)
{
    FileState *fs = file_states.back();
    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    char *c, *p, s[100000];
    int i, n;
    bool continued = true, look_for_bracket = false;

    get_line_fp = fp;
    line[0] = '\0';

//    if( fp == stdin && standard_input && !parse_only) {
    if( fp == stdin && standard_input) {
	char prompt[100], more_prompt[100];
	snprintf(prompt, sizeof(prompt), "%s> ", getName());
	snprintf(more_prompt, sizeof(more_prompt), "%s+ ", getName());
	p = ((int)fs->cs.size() == 0 && !fr) ? prompt : more_prompt;

	while(continued || look_for_bracket)
	{
	    continued = false;

#ifdef HAVE_READLINE
	    if( (c = readline(p)) ) {
		add_history(c);
#else
	    printf("%s", p);
	    fflush(stdout);
	    if( !stringGetLine(fp, s, (int)sizeof(s)-1) ) {
		c = s;
#endif
		for(i = (int)strlen(c)-1; i > 0 && c[i] != '#'; i--);
		if(c[i] == '#') c[i] = '\0';
		stringTrim(c);
		p = more_prompt;
		n = (int)strlen(c);
		if(n > 0 && c[n-1] == '\\') {
		    continued = true;
		    c[n-1] = '\0';
		    n = (int)strlen(line);
		    snprintf(line+n, line_size-n, "%s", c);
		}
		else if(n > 0 && c[n-1] == '{') {
		    if(look_for_bracket) {
			printParseError("unexpected '{' before missing '}'");
		    }
		    look_for_bracket = true;
		    n = (int)strlen(line);
		    snprintf(line+n, line_size-n, "%s", c);
		}
		else if(n > 0 && c[n-1] == '}') {
		    look_for_bracket = false;
		    n = (int)strlen(line);
		    snprintf(line+n, line_size-n, "%s", c);
		}
		else if(look_for_bracket) {
		    stringTrim(c);
		    n = (int)strlen(line);
		    snprintf(line+n, line_size-n, "%s\n", c);
		}
		else {
		    n = (int)strlen(line);
		    snprintf(line+n, line_size-n, "%s", c);
		}
#ifdef HAVE_READLINE
		free(c);
#endif

	    }
	    else {
		return (line[0] != '\0');
	    }
	}
    }
    else {
	while(continued || look_for_bracket)
	{
	    continued = false;

	    if( !stringGetLine(fp, s, (int)sizeof(s)-1) ) {
		for(i = (int)strlen(s)-1; i > 0 && s[i] != '#'; i--);
		if(s[i] == '#') s[i] = '\0';
		stringTrim(s);
		if((int)parse_file.size() > 0) parse_file.back().line_number++;
		i = 0;
		while(s[i] != '\0' && isspace((int)s[i])) i++;
		if(s[i] != '#')
		{
		    n = (int)strlen(s);
		    if(n > 0 && s[n-1] == '\\') {
			continued = true;
			s[n-1] = '\0';
			n = (int)strlen(line);
			snprintf(line+n, line_size-n, "%s", s);
		    }
		    else if(n > 0 && s[n-1] == '{') {
			if(look_for_bracket) {
			 printParseError("unexpected '{' before missing '}'");
			}
			look_for_bracket = true;
			n = (int)strlen(line);
			snprintf(line+n, line_size-n, "%s", s);
		    }
		    else if(n > 0 && s[n-1] == '}') {
			look_for_bracket = false;
			n = (int)strlen(line);
			snprintf(line+n, line_size-n, "%s", s);
		    }
		    else if(look_for_bracket) {
			stringTrim(s);
			n = (int)strlen(line);
			snprintf(line+n, line_size-n, "%s\n", s);
		    }
		    else {
			n = (int)strlen(line);
			snprintf(line+n, line_size-n, "%s", s);
		    }
		}
	    }
	    else {
		return (line[0] != '\0');
	    }
	}
    }

    stringTrim(line);

    if(line[0] == '!') {
	system(line+1);
	line[0] = '\0';
    }
    else if(line[0] == '#') {
	line[0] = '\0';
    }
    return true;
}

bool AppParse::readParseLine(char *line, int line_size)
{
    char *c, s[100000];
    int i;

    line[0] = '\0';

    if(!get_line_fp) {
	return false;
    }
    if( get_line_fp == stdin && standard_input) {
	char prompt[100];
	snprintf(prompt, sizeof(prompt), "%s+ ", getName());
#ifdef HAVE_READLINE
	if( !(c = readline(prompt)) ) {
	    return false;
	}
#else
	printf("%s", prompt);
	fflush(stdout);
	if( !stringGetLine(get_line_fp, s, (int)sizeof(s)-1) ) {
	    c = s;
	}
	else {
	    return false;
	}
#endif
    }
    else {
	if( !stringGetLine(get_line_fp, s, (int)sizeof(s)-1) ) {
	    c = s;
	}
	else {
	    return false;
	}
    }
    for(i = (int)strlen(c)-1; i > 0 && c[i] != '#'; i--);
    if(c[i] == '#') c[i] = '\0';
    stringTrim(c);
    snprintf(line, line_size, "%s", c);
    if(c != s) free(c);
    return true;
}

bool AppParse::parseFile(char *file, string &msg)
{
    FileState *fs = file_states.back();
    char *b, *e, *s, *full_path, *prop;
    char line[100000];
    string *value=NULL;
    int i, n;
    FILE *fp;
    ParseFileInfo pf;

    /* the file can be a string with quotes, without quotes, or a variable
	parse "scripts/file1" {a=1.3; set net="GERES"}
	parse scripts/file1 {a=1.3; set net="GERES"}
	parse file_variable {a=1.3; set net="GERES"}
     */

    b = file;
    while(isspace((int)*b) && *b != '\0') b++;
    if(*b == '\0') {
	printParseError("parse: missing file argument.");
	return false;
    }

    if(*b != '"' && *b != '\'' && *b != '`') {
	e = b;
	while(!isspace((int)*e) && *e != '\0') e++;
	n = e - b;
	s = (char *)malloc(n+1);
	memcpy(s, b, n);
	s[n] = '\0';

	if( fs->variables.get(s, &value) ) {
	    prop = strdup(value->c_str());
	}
	else {
	    prop = getGlobalVariable(s);
	}
	if(prop) {
	    free(s);
	    if(prop[0] == '"' || prop[0] == '\'' || prop[0] == '`') {
		s = strdup(prop+1);
		free(prop);
		n = (int)strlen(s);
		if(n > 0 && (s[n-1] == '"' || s[0] == '\'' || s[0] == '`')) {
		    s[n-1] = '\0';
		}
	    }
	    else {
		s = prop;
	    }
	}
    }
    else {
	e = b++;
	while(!isspace((int)*e) && *e != '\0') e++;

	if(*(e-1) == '"' || *(e-1) == '\'' || *(e-1) == '`') e--;
	if(e == b) {
	    printParseError("parse: missing file argument.");
	    return false;
	}
	n = e - b;
	s = (char *)malloc(n+1);
	memcpy(s, b, n);
	s[n] = '\0';
    }
    full_path = getFullPath(s);
    free(s);

    if( !(fp = fopen(full_path, "r")) ) {
	printParseError("parse: cannot open file: %s\n%s",
			full_path, strerror(errno));
	Free(full_path);
	return false;
    }

    pf.line_number = 0;

    pf.parse_file_q = stringToQuark(full_path);
    pf.parse_dir_q = stringToQuark("");

    // get the parse file directory
    n = strlen(full_path);
    for(i = n-1; i >= 0 && full_path[i] != '/'; i--);
    if(i >= 0) {
	char *parse_dir = (char *)malloc(i+2);
	strncpy(parse_dir, full_path, i+1);
	parse_dir[i+1] = '\0';
	pf.parse_dir_q = stringToQuark(parse_dir);
	Free(parse_dir);
    }
    Free(full_path);
    pf.relative_parse_paths = true;

    start_fs = new FileState();

    // first parse any commands inside {} on the parse line
    b = e;
    while(*b != '\0' && *b != '{') b++;
    if(*b == '{') {
	b++;
	e = b;
	while(*e != '\0' && *e != '}') e++;
	if(*e == '}') e--;
	n = e-b+1;
	if(n > 0) {
	    s = (char *)malloc(n+1);
	    memcpy(s, b, n);
	    s[n] = '\0';
	    if( !parseLine(s, msg) ) {
		if(start_fs) {
		    delete start_fs;
		    start_fs = NULL;
		}
		free(s);
		fclose(fp);
		get_line_fp = NULL;
		return false;
	    }
	    free(s);
	}
    }

    parse_file.push_back(pf);
    file_states.push_back(start_fs);

    while( getLine(fp, line, sizeof(line)) && parseLine(line, msg) );

    fclose(fp);
    get_line_fp = NULL;

    if(start_fs) {
	if((n = (int)start_fs->cs.size()) > 0) {
	    if(n == 1) printParseError("parse: missing 1 endif", n);
	    else printParseError("parse: missing %d endif's", n);
	}
	if((int)start_fs->foreach.size() > 0) {
	    printParseError("parse: missing endfor");
	}
	delete start_fs;
	start_fs = NULL;
    }

    if((int)file_states.size() > 0) file_states.pop_back();
    if((int)parse_file.size() > 0) parse_file.pop_back();

    return true;
}

bool AppParse::parseCallback(const char *script, vector<ParseFileInfo> &pf,
			string &msg)
{
    parse_file.swap(pf);

    start_fs = new FileState();
    file_states.push_back(start_fs);

    bool ret = parseLine(script, msg);

    if(start_fs) {
	delete start_fs;
	start_fs = NULL;
    }

    if((int)file_states.size() > 0) file_states.pop_back();

    pf.swap(parse_file);

    return ret;
}

char * AppParse::getFullPath(const string &file)
{
    if((int)parse_file.size() == 0) {
	return strdup(file.c_str());
    }

    ParseFileInfo pf = parse_file.back();
    const char *parse_dir = quarkToString(pf.parse_dir_q);

    int i = 0;
    if(file[i] == '"' || file[i] == '\'' || file[i] == '`') i++;

    if( parse_dir[0] == '\0' || file[i] == '/' || !pf.relative_parse_paths )
    {
	char *s = strdup(file.c_str());
	i = (int)strlen(s)-1;
	if(i >= 0 && (s[i] == '"' || s[i] == '\'' || s[i] == '`')) s[i] = '\0';
	return s;
    }
    int n = strlen(parse_dir) + strlen(file.c_str()+i);
    char *full_path = (char *)malloc(n+1);
    strcpy(full_path, parse_dir);
    // parse_dir always ends with '/'
    strcat(full_path, file.c_str());
    i = (int)strlen(full_path)-1;
    if(i>=0 && (full_path[i]=='"' || full_path[i]=='\'' || full_path[i]=='`')) {
	 full_path[i] = '\0';
    }
    return full_path;
}

void AppParse::listAliases(void)
{
    int i, j, n;
    FileState *fs = file_states.back();
    Alias *a;
    FILE *fp = (print_fp) ? print_fp : stdout;

    // list aliases
    n = 0;
    for(i = 0; i < (int)fs->aliases.size(); i++) {
	printf("%s=%s\n", fs->aliases[i]->alias.c_str(),
		fs->aliases[i]->name.c_str());
	n++;
    }
    for(j = 0; j < (int)fs->foreach.size(); j++) {
	for(i = 0; i < (int)fs->foreach[j]->aliases.size(); i++) {
	    a = fs->foreach[j]->aliases[i];
	    fprintf(fp, "%s=%s\n", a->alias.c_str(), a->name.c_str());
	}
    }
    if(!n) {
	fprintf(fp, "No aliases.\n");
    }
}

bool AppParse::unalias(char *line)
{
    int i, j, n;
    string c;
    FileState *fs = file_states.back();
    Alias *a;

    if(parseArg(line, "unalias", c)) {
	n = 0;
	for(i = (int)fs->aliases.size()-1; i >= 0; i--) {
	    if(!strcasecmp(c.c_str(), fs->aliases[i]->alias.c_str())) {
		delete fs->aliases[i];
		fs->aliases.erase(fs->aliases.begin()+i);
		n++;
	    }
	}
	for(j = 0; j < (int)fs->foreach.size(); j++) {
	    for(i = (int)fs->foreach[j]->aliases.size()-1; i >= 0; i--) {
		a = fs->foreach[j]->aliases[i];
		if(!strcasecmp(c.c_str(), a->alias.c_str())) {
		    delete a;
		    fs->foreach[j]->aliases.erase(
			fs->foreach[j]->aliases.begin()+i);
		    n++;
		}
	    }
	}
	if(standard_input && file_states.size() == 1) {
	    if(n) {
		printf("%s removed\n", c.c_str());
	    }
	    else {
		printf("%s not defined\n", c.c_str());
	    }
	}
	return true;
    }
    printParseError("unalias: missing arguments");
    return false;
}

bool AppParse::setAlias(const char *line)
{
    int i, j, n, m;
    size_t k;
    string c, name;
    const char *reserved[] = {"alias", "defined", "export", "help", "parse",
	"parse_string", "print", "printClose", "printOpen", "set", "setb",
	"sprint", "unalias", "writeArray", "writeClose", "writeDouble",
	"writeInt", "writeFloat", "writeLong", "writeOpen", "writeString",
	"quit", "foreach", "endfor", "if", "endif", "break", "for_index",
	"read", "recent_input", "connect", "disconnect", "get_aao",
	"get_aaow", "get_all_tables", "get_wfdiscs", "mapping", "query",
	"read_waveforms", "import", "output", "deselect", "edit",
	"select", "remove_from_db", "system"};
    FileState *fs = file_states.back();
    Alias *a;

    if(!parseArg(line, "alias", c)) {
	printParseError("alias: missing arguments");
	return false;
    }

    parseTrim(c);
    if((k = c.find('=')) == string::npos) {
	printParseError("alias: missing argument");
	return true;
    }

    name = c.substr(k+1);
    c.erase(k);
    parseTrim(name);
    parseTrim(c);
    if((int)c.length() == 0) {
	printParseError("alias: missing argument");
	return true;
    }
    m = sizeof(reserved)/sizeof(const char *);
    for(j = 0; j < m && !parseCompare(c, reserved[j]); j++);
    if(j < m) {
	printParseError("alias: %s is a reserved name.", c.c_str());
	return true;
    }

    m = (int)name.length();
    for(i = 0; i < m; i++) {
	if( !(isalpha((int)name[i]) || isdigit((int)name[i])
		|| name[i] == '_' || name[i] == '.') )
	{
	    printParseError(
	    "alias: name contains non-alphanumeric, non-'_', non-'.' char: %s",
			name.c_str());
	    return true;
	}
    }

    if(m > 0) {
	n = 0;
	for(i = 0; i < (int)fs->aliases.size(); i++) {
	    a = fs->aliases[i];
	    if(parseCompare(c, a->alias)) {
		a->name.assign(name);
		n = 1;
		break;
	    }
	}
	if(!n) {
	    for(j = 0; j < (int)fs->foreach.size(); j++) {
		for(i=0; i < (int)fs->foreach[j]->aliases.size(); i++){
		    a = fs->foreach[j]->aliases[i];
		    if(parseCompare(c, a->alias)) {
			a->name.assign(name);
			n = 1;
			break;
		    }
		}
	    }
	}
	if(!n) {
	    a = new Alias(c, name);
	    if((int)fs->foreach.size() > 0) {
		fs->foreach.back()->aliases.push_back(a);
	    }
	    else {
		fs->aliases.push_back(a);
	    }
	}
	return true;
    }
    printParseError("alias: cannot interpret arguments");
    return true;
}

void AppParse::applyAliases(char *line, int line_size)
{
    char *s, *tok, *last, *buf, *save, c, *p;
    int i, j, i1, i2, k, l, n, len;
    FileState *fs = file_states.back();
    Alias *a;

    if((int)fs->aliases.size() == 0) {
	for(j = 0; j < (int)fs->foreach.size(); j++) {
	    if((int)fs->foreach[j]->aliases.size() > 0) break;
	}
	if(j == (int)fs->foreach.size()) return; // no aliases
    }

    buf = strdup(line);
    save = strdup(line);

    // blank out characters between quotes and between {}
    k = 0;
    while(buf[k] != '\0') {
	if(buf[k] == '"' || buf[k]== '\'' || buf[k] == '`' || buf[k] == '{') {
	    c = (buf[k] == '{') ? '}' : buf[k];
	    buf[k] = ' ';
	    while(buf[k] != c && buf[k] != '\0') buf[k++] = ' ';
	    if(buf[k] == c) buf[k++] = ' ';
	}
	else {
	    k++;
	}
    }

    // blank out foreach, if, endif, endfor
    k = 0;
    while(buf[k] != '\0') {
	if(!strncasecmp(buf+k, "foreach(", 8)) {
	    strncpy(buf+k, "       ", 7);
	    k += 8;
	}
	else if(!strncasecmp(buf+k, "if(", 3)) {
	    strncpy(buf+k, "  ", 2);
	    k += 3;
	}
	else if(!strncasecmp(buf+k, "endif", 5)) {
	    strncpy(buf+k, "     ", 5);
	    k += 5;
	}
	else if(!strncasecmp(buf+k, "endfor", 6)) {
	    strncpy(buf+k, "      ", 6);
	    k += 6;
	}
	else {
	    k++;
	}
    }

    // blank out the left side of the '=' in an alias command
    if(!strncasecmp(buf, "alias ", 6)) {
	k = 0;
	while(buf[k] != '\0' && buf[k] != '=') buf[k++] = ' ';
    }
 
    k = 0;
    line[0] = '\0';
    p = buf;
    tok = buf;
    while((s = strtok_r(tok, " \t.~!@#$%^&*,;:()[]{}=><", &last)))
    {
	tok = NULL;
	i1 = (int)(p-buf);
	i2 = (int)(s-buf);
	for(i = i1; i < i2; i++) line[k++] = save[i];

	n = 0;
	for(j = 0; j < (int)fs->aliases.size(); j++) {
	    a = fs->aliases[j];
	    if(!strcasecmp(a->alias.c_str(), s)) {
		len = (int)a->name.size();
		for(i = 0; i < len; i++) line[k++] = a->name.at(i);
		n = 1;
		break;
	    }
	}
	if(!n) {
	    for(j = 0; j < (int)fs->foreach.size(); j++) {
		for(l = 0; l < (int)fs->foreach[j]->aliases.size(); l++) {
		    a = fs->foreach[j]->aliases[l];
		    if(!strcasecmp(a->alias.c_str(), s)) {
			len = (int)a->name.size();
			for(i = 0; i < len; i++) {
			    line[k++] = a->name.at(i);
			}
			n = 1;
			break;
		    }
		}
		if(n) break;
	    }
	}
	if(!n) {
	    for(i = 0; s[i] != '\0'; i++) line[k++] = s[i];
	}
	p = s + strlen(s);
    }
    i1 = (int)(p-buf);
    for(i = i1; save[i] != '\0'; i++) line[k++] = save[i];
    line[k] = '\0';

    free(buf);
    free(save);
}

bool AppParse::assignString(Component *top, char *line)
{
    int i, j, len, n;
    bool parse_bool=false, parse_prop=false;
    ParseVar ret;
    double d;
    const char *cmd, *s;
    string msg;
    char b, *c, name[100], value[100000];

    if(!strncasecmp(line, "set ", 4)) {
	cmd = "set";
	for(c = line+4; *c != '\0' && isspace((int)*c); c++);
    }
    else if(!strncasecmp(line, "setb ", 5)) {
	cmd = "setb";
	for(c = line+5; *c != '\0' && isspace((int)*c); c++);
	parse_bool = true;
    }
    else if(!strncasecmp(line, "setProperty ", 12)) {
	cmd = "setProperty";
	for(c = line+12; *c != '\0' && isspace((int)*c); c++);
	parse_prop = true;
    }
    else {
	return false;
    }

    if(*c == '\0' || *c == '\'' || *c == '"' || *c == '`') {
	printParseError("%s: missing name", cmd);
	return false;
    }
    i = 0;
    while(*c != '\0' && !isspace((int)*c) && *c != '=' && *c != '\''
	&& *c != '"' && *c != '`' && *c != '{' && i < 99)
    {
	if(i==0 && !isalpha((int)*c)) {
	    printParseError("%s: first character of name is not alpabetic",cmd);
	    return false;
	}
	else if( !isalpha((int)*c) && !isdigit((int)*c) && *c != '_') {
	    printParseError(
		"%s: name contains non-alphanumeric, non-underscore char: %c",
		cmd, *c);
	    return false;
	}
	name[i++] = *c++;
    }
    name[i] = '\0';

    if(name[0] == '\0') {
	printParseError("%s: missing name", cmd);
	return false;
    }

    while(*c != '\0' && isspace((int)*c)) c++;

    if(*c != '=') {
	printParseError("%s: missing '='", cmd);
	return false;
    }
    c++;
    while(*c != '\0' && isspace((int)*c)) c++;

    if(parse_bool) {
	ret = parseExpression(top, c, true, msg);
	if(ret == VARIABLE_TRUE) {
	    putVariable(name, "true");
	}
	else if(ret == VARIABLE_FALSE) {
	    putVariable(name, "false");
	}
	else {
	    printParseError("%s: cannot interpret logical expression", cmd);
	    return false;
	}
	return true;
    }
    else if(parse_prop) {
	putProperty(name, c, false);
	ret = parseExpression(top, c, true, msg);
	return true;
    }

    memset(value, 0, sizeof(value));
    value[0] = '"';
    len = sizeof(value)-1;
    while(*c != '\0') {
	while(*c != '\0' && isspace((int)*c)) c++;
	if(*c == '\0') {
	    value[1] = '"';
	    break;
	}
	if(*c == '\'' || *c == '"' || *c == '`' || *c == '{') {
	    b = (*c == '{') ? '}' : *c;
	    c++;
	    i = (int)strlen(value);
	    while(*c != '\0' && *c != b && i < len) value[i++] = *c++;
	    if(*c != '\0') c++;
	    value[i] = '\0';
	}
	else if(*c == ',') {
	    i = (int)strlen(value);
	    if(i < len) {
		value[i++] = *c;
		value[i] = '\0';
	    }
	    c++;
	}
	else {
	    i = j = (int)strlen(value);
	    while(*c != '\0' && !isspace((int)*c) && *c != '\'' && *c != '"'
			&& *c != '`' && *c != '{' && *c !=  ',' && j < len)
	    {
		value[j++] = *c;
		if(*c == '(') { // function call. include to the ')'
		    c++;
		    while(*c != '\0' && *c != ')') value[j++] = *c++;
		}
		else c++;
	    }
	    value[j] = '\0';
	    if(stringToDouble(value+i, &d)) { // nothing to do
	    }
	    else if(parseToDouble(value+i, &d)) {
		snprintf(value+i, sizeof(value)-i, "%.16g", d);
	    }
	    else {
		msg.clear();
		ret = parseExpression(top, value+i, false, msg);
		s = (char *)msg.c_str();
		n = (int)msg.length();
		if(n > 0 && (*s == '"' || *s == '\'' || *s == '`')) {
		    if(msg[n-1] == *s) msg.erase(n-1);
		    s++;
		}
		if(ret == STRING_RETURNED) strncpy(value+i, s, len-i);
		else if(ret == VARIABLE_TRUE) strncpy(value+i, "true", len-i);
		else if(ret == VARIABLE_FALSE)strncpy(value+i, "false", len-i);
		else {
		    printParseError("%s: %s is not defined.", cmd, value+i);
		    return false;
		}
	    }
	}
	if(value[1] == '\0') {
	    value[1] = '"';
	    break;
	}
	while(*c != '\0' && isspace((int)*c)) c++;
    }
    i = (int)strlen(value);
    if(i < len && value[i-1] != '"') value[i] = '"';
    
    putVariable(name, value);
    return true;
}

bool AppParse::assignVariable(Component *top, char *line, ParseVar *ret)
{
    int i, n;
    double d;
    string value;
    char *c, name[101];

    *ret = VARIABLE_NOT_FOUND;

    // check for the syntax name = something

    c = line;
    while(*c != '\0' &&  isspace((int)*c)) c++;
    while(*c != '\0' && !isspace((int)*c) && *c != '=') c++;
    while(*c != '\0' &&  isspace((int)*c) && *c != '=') c++;

    if(*c != '=') return false; // not an assignment

    // check for a valid name
    memset(name, 0, sizeof(name));
    n = (int)sizeof(name)-1;
    c = line;
    while(*c != '\0' && isspace((int)*c)) c++;
    i = 0;
    while(*c != '\0' && !isspace((int)*c) && *c != '=' && i < n)
    {
	if(isalpha((int)*c) || isdigit((int)*c) || *c == '_') {
	    name[i++] = *c++;
	}
	else if(*c == '.') { // could be a predefined variable assignment
	    return false; // not a local assignment
	}
	else {
	    printParseError(
    "assignment: name contains non-alphanumeric, non-'_' non-'.' char: %c", *c);
	    *ret = VARIABLE_ERROR;
	    return true; // return true since this is an assignment, but invalid
	}
    }
    if(i == n) {
	printParseError("assignment: name length exceeds limit: %d", n);
	return true;
    }
    if(i == 0) {
	printParseError("assignment: missing name");
	return true;
    }
    name[i] = '\0';

    if(isdigit(name[0])) {
	printParseError(
	    "assignment: first character of name is non-alphabetic: %s", name);
	*ret = VARIABLE_ERROR;
	return true;
    }

    if( reservedName(name) ) {
	printParseError("assignment: %s is a reserved name", name);
	*ret = VARIABLE_ERROR;
	return true;
    }

    while(*c != '\0' && isspace((int)*c) && *c != '=') c++;
    c++;
    while(*c != '\0' && isspace((int)*c)) c++;

    if(*c == '"' || *c == '\'' || *c == '`' || *c == '{') {
	return false; // can't be an assignment. must be a command
    }

    *ret = parseExpression(top, c, false, value);
    if(*ret == STRING_RETURNED) {
	if(!stringToDouble(value.c_str(), &d)) {
	    printParseError("assignment: not a number: %s", value.c_str());
	}
	else {
	    putVariable(name, value.c_str());
#ifdef DEBUG_APP 
printf("parseExpression value: %s\n", value.c_str());
#endif
	}
    }
    return true;
}

bool AppParse::isCommand(char *line)
{
    char *c = line;

    // the line can be in one of the forms:
    //		command_name ...
    //		prefix.command_name ...
    //		prefix.name = something ...

    // check first item for valid characters
    while(*c != '\0' &&  isspace((int)*c)) c++;

    while(*c != '\0' && !isspace((int)*c) && *c != '=')
    {
	if(!isalpha((int)*c) && !isdigit((int)*c) && *c != '_' && *c != '.') {
	    return false; // found invalid character in the command name
	}
	c++;
    }
    return true; // looks ok
}

bool AppParse::reservedName(const string &name)
{
    int i, q = stringToQuark(name.c_str());

    for(i = 0; i < (int)reserved_names.size() && q != reserved_names[i]; i++);
    return (i < (int)reserved_names.size()) ? true : false;
}

void AppParse::addReservedName(const string &name)
{
    int i, q = stringToQuark(name.c_str());

    for(i = 0; i < (int)reserved_names.size() && q != reserved_names[i]; i++);
    if(i == (int)reserved_names.size()) {
	reserved_names.push_back(q);
    }
}

bool AppParse::exportVariable(char *name)
{
    char *value;

    stringTrim(name);
    for(int i = 0; name[i] != '\0'; i++) {
	if(!(isalpha((int)name[i]) || isdigit((int)name[i]) || name[i]=='_')) {
	    printParseError("Invalid export variable name: %s", name);
	    return false;
	}
    }

    if(strstr(name, "[")) { // cannot export an array element
	printParseError("Invalid export variable name: %s", name);
	return false;
    }
    else if(getVariable(name, &value) == VARIABLE_ERROR) {
	return false;
    }
    else if( value ) {
	putGlobalVariable(name, value);
	free(value);
	return true;
    }
    else {
	printParseError("export variable %s has not been set.", name);
	return false;
    }
}

bool AppParse::doSubstitutions(Component *top, char *line, char *sline,
			int sline_size, bool quotes)
{
    ParseVar ret;
    int i, j, len;
    double d;
    string msg;
    char *c, b, *s, quote = '\0';
    char name[1000], value[100000];

    len = sline_size-1;
    memset(sline, 0, sline_size);

    memset(name, 0, sizeof(name));
    memset(value, 0, sizeof(value));

    c = line;
    i = 0;
    // skip the command
    while(*c != '\0' && *c != '=' && !isspace((int)*c)) {
	sline[i++] = *c++;
    }
    while(*c != '\0') {
	while(*c != '\0' && *c != '=' && !isspace((int)*c) && *c != '\''
		&& *c != '"' && *c != '`' && *c != '{' && i < len)
	{
	    sline[i++] = *c++;
	}
	if(i == len) return false;

	if(*c == '\'' || *c == '"' || *c == '`' || *c == '{') {
	    if(i >= len-1) return false;
	    sline[i++] = *c;
	    b = (*c == '{') ? '}' : *c;
	    c++;
	    while(*c != '\0' && *c != b && i < len) sline[i++] = *c++;
	    if(i >= len-1) return false;
	    if(*c != '\0') sline[i++] = *c++;
	}
	else if(*c == '=' || isspace((int)*c)) {
	    if(i >= len-1) return false;
	    sline[i++] = *c++;
	    while(*c != '\0' && isspace((int)*c)) c++;
	    if(i >= len-1) return false;
	    if(*c == '\'' || *c == '"' || *c == '`' || *c == '{') {
		sline[i++] = *c;
		b = (*c == '{') ? '}' : *c;
		c++;
		while(*c != '\0' && *c != b && i < len) sline[i++] = *c++;
		if(i >= len-1) return false;
		if(*c != '\0') sline[i++] = *c++;
		continue;
	    }
	    j = 0;
	    while(*c != '\0' && !isspace((int)*c) && *c != '=' && j < 1000) {
		name[j++] = *c++;
	    }
	    name[j] = '\0';

	    // check if this is on the left side of '='
	    if(*c == '=') {
		j = 0;
		while(name[j] != '\0' && i < len) sline[i++] = name[j++];
		continue;
	    }
	    else { // *c == ' '
		for(s = c; *s != '\0' && isspace((int)*s); s++);
		if(*s == '=') {
		    j = 0;
		    while(name[j] != '\0' && i < len) sline[i++] = name[j++];
		    continue;
		}
	    }

	    // check for a number or 'true' or 'false'
	    if(parseToDouble(name, &d) || !strcasecmp(name, "true")
		|| !strcasecmp(name, "false"))
	    {
		j = 0;
		while(name[j] != '\0' && i < len) sline[i++] = name[j++];
		continue;
	    }
	    memset(value, 0, sizeof(value));

	    ret = parseExpression(top, name, false, msg);

	    if(ret == STRING_RETURNED) {
		strncpy(value, msg.c_str(), sizeof(value));
	    }
	    else if(ret==VARIABLE_TRUE) {
		strncpy(value,"true",sizeof(value));
	    }
	    else if(ret == VARIABLE_FALSE) {
		strncpy(value,"false",sizeof(value));
	    }
	    else {
		return false;
	    }

	    if(value[0] != '\0') {
#ifdef DEBUG_APP
printf("substitution: %s=%s\n", name, value);
#endif
		if(quotes) {
		    if( !strchr(value, (int)'"') ) quote = '"';
		    else if( !strchr(value, (int)'\'') ) quote = '\'';
		    else quote = '`';
		    sline[i++] = quote;
		}
		j = 0;
		while(value[j] != '\0' && i < len) sline[i++] = value[j++];
		if(quotes) sline[i++] = quote;
	    }
	    else {
		j = 0;
		while(name[j] != '\0' && i < len) sline[i++] = name[j++];
	    }
	}
    }
    return true;
}

ParseVar AppParse::parseWithPrefix(Component *top, char *input, string &msg)
{
    FileState *fs = file_states.back();
    char in[200], new_input[200], *name, c;
    int i, j, k;

    strncpy(in, input, sizeof(in)-1);

    for(i = 0; in[i] != '\0' && isspace((int)in[i]); i++);
    if(in[i] != '\0') {
	for(j=i+1; in[j]!='\0' && in[j]!='.' && !isspace(in[j]); j++);
	c = in[j];
	in[j] = '\0';
	name = in+i;
	for(k = 0; k < (int)fs->foreach.size(); k++) {
	    if(fs->foreach[k]->name && !strcmp(fs->foreach[k]->name, name)) {
		break;
	    }
	}
	in[j] = c;
	if(k < (int)fs->foreach.size()) {
	    if(fs->foreach[k]->prefix) {
		snprintf(new_input, sizeof(new_input), "%s.%s",
			fs->foreach[k]->prefix, in);
		applyAliases(new_input, sizeof(new_input));
		return top->parseVar(new_input, msg);
	    }
	}
    }
    
    return top->parseVar(in, msg);
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __STDC__
void AppParse::printParseError(const char *format, ...)
#else
void AppParse::printParseError(va_alist) va_dcl
#endif
{
    va_list	va;
    char	error[5000], c;

#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    vsnprintf(error, sizeof(error), format, va);
    va_end(va);

    if((int)parse_file.size() > 0) {
	int i, n;
	for(i = (int)parse_file.size()-1; i >= 0; i--) {
	    n = (int)strlen(error);
	    c = (i > 0) ? '\n' : '\0';
	    snprintf(error+n, sizeof(error)-n, "\nline %d of %s%c",
		parse_file[i].line_number,
		quarkToString(parse_file[i].parse_file_q), c);
	}
    }
    logErrorMsg(LOG_ERR, error);
}

ParseVar AppParse::startArray(char *name)
{
    FileState *fs = file_states.back();
    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    char *c;

    if(!fr) {
	printParseError("startArray: Internal error.");
	return VARIABLE_ERROR;
    }
    Free(fr->array_prop);
    Free(fr->array_save);

    if(getVariable(name, &c) == VARIABLE_ERROR) {
	return VARIABLE_ERROR;
    }
    else if( !c ) return VARIABLE_NOT_FOUND;

    fr->array_save = c;

    int n = (int)strlen(c);
    if(c[0] == '"' && c[n-1] == '"') { // save without the quotes
	fr->array_prop = (char *)malloc(n-1);
	strncpy(fr->array_prop, c+1, n-2);
	fr->array_prop[n-2] = '\0';
    }
    else {
	fr->array_prop = strdup(c);
    }

    if( !(c = strtok_r(fr->array_prop, ",", &fr->array_last)) )
    {
	return VARIABLE_ERROR;
    }
    stringTrim(c);
    putVariable(name, c);
    return FOREACH_MORE;
}

ParseVar AppParse::nextArray(string &msg)
{
    FileState *fs = file_states.back();
    Foreach *fr = ((int)fs->foreach.size() > 0) ? fs->foreach.back() : NULL;
    char *c;

    if(!fr) {
	printParseError("nextArray: Internal error.");
	return VARIABLE_ERROR;
    }
    msg.clear();
    if( !fr->array_prop ) return VARIABLE_NOT_FOUND;

    if( !fr->array_last || !(c = strtok_r(NULL, ",", &fr->array_last)) )
    {
	putVariable(fr->name, fr->array_save);
	return FOREACH_NO_MORE;
    }
    else {
	stringTrim(c);
	putVariable(fr->name, c);
	return FOREACH_MORE;
    }
}

bool AppParse::checkLine(char *line, string &msg)
{
    int nleft, nright;

    // check for the same number of left and right parentheses characters,
    // and right parenthesis before left parenthesis.
    nleft = 0;
    nright = 0;
    for(int i = 0; line[i] != '\0'; i++) {
	if(line[i] == '(') nleft++;
	else if(line[i] == ')') {
	    if(++nright > nleft) {
		printParseError("Missing '('");
		return false;
	    }
	}
    }
    if(nleft > nright) {
	printParseError("Missing ')'");
	return false;
    }
    else if(nright > nleft) {
	printParseError("Missing '('");
	return false;
    }

    // check for the same number of left and right brackets,
    // and right bracket before left bracket.
    nleft = 0;
    nright = 0;
    for(int i = 0; line[i] != '\0'; i++) {
	if(line[i] == '[') nleft++;
	else if(line[i] == ']') {
	    if(++nright > nleft) {
		printParseError("Missing '['");
		return false;
	    }
	}
    }
    if(nleft > nright) {
	printParseError("Missing ']'");
	return false;
    }
    else if(nright > nleft) {
	printParseError("Missing '['");
	return false;
    }

    return true;
}

void AppParse::putVariable(const string &name, const string &value)
{
    char *prop;
    if( (prop = getGlobalVariable(name)) ) {
	// if it is already defined as a global variable, set it globally
	free(prop);
	putGlobalVariable(name, value);
    }
    else {
	string s(value);
	// otherwise set it locally
	if(start_fs) { // for the commands inside {} for parse file { }
	    start_fs->variables.put(name, &s);

	}
	else {
	    FileState *fs = file_states.back();
	    fs->variables.put(name, &s);
	}
    }
}

void AppParse::removeVariable(const string &name)
{
    FileState *fs = file_states.back();
    fs->variables.remove(name);
}

ParseVar AppParse::getVariable(const string &name, char **var)
{
    FileState *fs = file_states.back();
    char *s, *c, *e, *prop, *nam, *last;
    string *value = NULL;
    int i, n;

    *var = NULL;

    if( (s = (char *)strstr(name.c_str(), "[")) ) { // array
	n = s-name.c_str();
	nam = (char *)malloc(n+1);
	strncpy(nam, name.c_str(), n);
	nam[n] = '\0';
	if( fs->variables.get(nam, &value) ) {
	    prop = strdup(value->c_str());
	}
	else {
	    prop = getGlobalVariable(nam);
	}
	free(nam);
	if(prop) {
	    s++;
	    while(*s != '\0' && isspace((int)*s)) s++;
	    if(*s == ']') {
		printParseError("missing ']': %s", name.c_str());
		free(prop);
		return VARIABLE_ERROR;
	    }
	    e = s;
	    while(*e != '\0' && *e != ']') e++;
	    if(*e != ']') {
		printParseError("missing ']': %s", name.c_str());
		free(prop);
		return VARIABLE_ERROR;
	    }
	    // check that there is nothing after the ']'
	    c = e+1;
	    while(*c != '\0' && isspace((int)*c)) c++;
	    if(*c != '\0') {
		printParseError("invalid reference after ']': %s",name.c_str());
		free(prop);
		return VARIABLE_ERROR;
	    }
	    e--;
	    while(e > s && isspace((int)*e)) e--;

	    n = e-s+1;
	    nam = (char *)malloc(n+1);
	    strncpy(nam, s, n);
	    nam[n] = '\0';
	    if(!stringToInt(nam, &n)) {
		printParseError("invalid index %s in %s", nam, name.c_str());
		free(prop);
		free(nam);
		return VARIABLE_ERROR;
	    }
	    free(nam);
	    c = prop;
	    i = 1;
	    while((s = strtok_r(c, "\",", &last)) && i < n) {
		c = NULL;
		i++;
	    }
	    if(i == n && s) {
		stringTrim(s);
		c = strdup(s);
		free(prop);
		*var = c;
		return STRING_RETURNED;
	    }
	    free(prop);
	    printParseError("invalid index in %s", name.c_str());
	    return VARIABLE_ERROR;
	}
    }
    else if( fs->variables.get(name, &value) ) {
	prop = strdup(value->c_str());
    }
    else {
	prop = getGlobalVariable(name);
    }
    *var = prop;
    return (prop) ? STRING_RETURNED : VARIABLE_NOT_FOUND;
}

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

static bool parseToDouble(const char *s, double *d)
{
    if(stringToDouble(s, d)) {
    }
    else if(!strcasecmp(s, "pi")) {
	*d = M_PI;
    }
    else if(!strcasecmp(s, "degrees_to_radians")) {
	*d = M_PI/180.;
    }
    else if(!strcasecmp(s, "radians_to_degrees")) {
	*d = 180./M_PI;
    }
    else if(!strcasecmp(s, "degrees_to_km")) {
	*d = 111.1954;
    }
    else if(!strcasecmp(s, "km_to_degrees")) {
	*d = 1./111.1954;
    }
    else if(timeParseString(s, d) == 1) {
    }
    else {
	return false;
    }
    return true;
}

ParseCmd AppParse::create(Component *comp, const string &input, string &msg)
{
    string nam, type;
    ParseCmd ret;
    CreateClass method=NULL;

    if( !parseGetArg(input, "name", nam) ) {
	msg.assign("create: missing 'name' argument.");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(input, "type", type) ) {
	msg.assign("create: missing 'type' argument.");
	return ARGUMENT_ERROR;
    }

    for(int i = 0; i < (int)create_methods.size(); i++) {
	if(!strcasecmp(create_methods[i].class_name, type.c_str())) {
//		(*create_methods[i].create)(nam, comp, input);
		method = create_methods[i].create;
	}
    }

    if( !method ) {
	msg.assign(string("create: unknown type: ") + type);
	return ARGUMENT_ERROR;
    }
    
    if((ret = parseXArgs(comp, nam, input, msg)) == ARGUMENT_ERROR)
    {
	return ret;
    }

    vector<Component *> *ch = comp->getChildren();

    for(int i = 0; i < (int)ch->size(); i++) {
	if(!strcmp(ch->at(i)->getName(), nam.c_str())) {
printf("deleting existing %s\n", nam.c_str());
	    ch->at(i)->destroy();
	    break;
	}
    }
    delete ch;

    (*method)(nam, comp, input);

    return COMMAND_PARSED;
}

ParseCmd AppParse::parseXArgs(Component *comp, const string &nam,
			const string &input, string &msg)
{
    XrmDatabase db = XrmGetDatabase(XtDisplay(comp->baseWidget()));
    char *c, *s, *tok, *last;
    char line[5000];
    string str;
    int n;

    if(parseGetArg(input, "args", str)) {
	s = strdup(str.c_str());
	n = (int)strlen(s);
	if(n > 1 && s[0] == '(' && s[n-1] == ')') {
	    tok = s+1;
	    s[n-1] = '\0';
	}
	else {
	    tok = s;
	}
	while( (c = strtok_r(tok, "\n", &last)) ) {
	    tok = NULL;
	    stringTrim(c);
	    snprintf(line, sizeof(line), "%s*%s*%s.%s",
		resource_name.c_str(), comp->getName(), nam.c_str(), c);
	    XrmPutLineResource(&db, line);
#ifdef DEBUG_APP 
printf("parseXArgs: '%s'\n", line);
#endif
	}
	Free(s);
    }
    return COMMAND_PARSED;
}

// static
void AppParse::addCreateMethod(const string &class_name, CreateClass method)
{
    AppParse *app = getAppParse();
    int i;
    for(i = 0; i < (int)app->create_methods.size(); i++) {
	if(!strcasecmp(app->create_methods[i].class_name, class_name.c_str())) {
	    break;
	}
    }
    if(i == (int)app->create_methods.size()) {
	CreateCompStruct c;
	c.class_name = strdup(class_name.c_str());
	c.create = method;
	app->create_methods.push_back(c);
    }
}

static void createFormDialog(const string &name, Component *parent,
			const string &params)
{
    string title;
    int i, pointer_focus=0, independent=1;
    Component *comp, *cp=NULL;
    vector<Component *> *ch;

    parseGetArg(params, "title", title);
    stringGetBoolArg(params.c_str(), "pointer_focus", &pointer_focus);
    stringGetBoolArg(params.c_str(), "independent", &independent);

    comp = parent->formDialogParent();
    ch = comp->getChildren();

    for(i = 0; i < (int)ch->size() && name.compare(ch->at(i)->getName()); i++);
    if(i < (int)ch->size()) {
	cp = ch->at(i);
    }
    delete ch;
    if(cp) {
	cp->setVisible(false);
	ch = cp->getChildren();
	for(i = (int)ch->size()-1; i >= 0; i--) {
	    ch->at(i)->destroy();
	}
	delete ch;
	return;
    }

    if(!title.empty()) {
	new FormDialog(name, comp, title,(bool)pointer_focus,(bool)independent);
    }
    else {
	new FormDialog(name, comp, (bool)pointer_focus, (bool)independent);
    }
}

static void createFrame(const string &name, Component *parent,
			const string &params)
{
    string title;
    int i, pointer_focus=0, independent=1;
    Frame *frame;
    Component *comp, *cp=NULL;
    vector<Component *> *ch;

    parseGetArg(params, "title", title);
    stringGetBoolArg(params.c_str(), "pointer_focus", &pointer_focus);
    stringGetBoolArg(params.c_str(), "independent", &independent);

    comp = parent->formDialogParent();
    ch = comp->getChildren();

    for(i = 0; i < (int)ch->size() && name.compare(ch->at(i)->getName()); i++);
    if(i < (int)ch->size()) {
	cp = ch->at(i);
    }
    delete ch;
    if(cp) {
	cp->setVisible(true);
	return;
    }

    if(!title.empty()) {
	frame = new Frame(name, comp, title, (bool)pointer_focus,
				(bool)independent);
    }
    else {
	frame = new Frame(name, comp, (bool)pointer_focus, (bool)independent);
    }
    frame->createInterface();
}

static void createArrowButton(const string &name, Component *parent,
			const string &params)
{
    new ArrowButton(name, parent);
}

static void createButton(const string &name, Component *parent,
			const string &params)
{
    int position = -1;

    stringGetIntArg(params.c_str(), "position", &position);

    if(parent->getPopupMenuInstance()) {
	Arg args[1];
	char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";
	XtTranslations translations = XtParseTranslationTable(trans);
	XtSetArg(args[0], XmNtranslations, translations);
	new Button(name, parent, args, 1, NULL, position);
    }
    else {
	new Button(name, parent, position, NULL);
    }
}

static void createChoice(const string &name, Component *parent,
			const string &params)
{
    string str;
    Choice *choice = new Choice(name, parent);

    if(params.empty()) return;

    if(parseGetArg(params, "choices", str)) {
	char *s = strdup(str.c_str());
	char *tok, *c, *last;
	tok = s;
	while((c = strtok_r(tok, ",", &last)) != NULL) {
	    tok = NULL;
	    stringTrim(c);
	    choice->addItem(c);
	}
	free(s);
    }
    else {
	cerr << "createChoice: missing 'choices' argument." << endl;
    }
}
static void createFileChoice(const string &name, Component *parent,
			const string &params)
{
    string msg;
    bool choose_file = true;

    parseGetArg(params, "create", msg, "choose_file", &choose_file);

    FileChoice *fc = new FileChoice(name, parent, NULL, 0, NULL, choose_file);

    if(parseGetArg(params, "choices", msg)) {
	char *s = strdup(msg.c_str());
	char *tok, *c, *last;
	tok = s;
	while((c = strtok_r(tok, ",", &last)) != NULL) {
	    tok = NULL;
	    stringTrim(c);
	    fc->addItem(c);
	}
	free(s);
    }
}
static void createFileDialog(const string &name, Component *parent,
				const string &params)
{
    int num_file_suffixes = 0;
    string filetype, dir, suffixes;
    char *c, *tok, *last, *s;
    const char *file_suffixes[100];
    FileType file_type = EXISTING_FILE;

    if(parseGetArg(params, "file_type", filetype)) {
	if(parseCompare(filetype, "EXISTING_FILE_OR_DIR")) {
	    file_type = EXISTING_FILE_OR_DIR;
	}
	else if(parseCompare(filetype, "EXISTING_FILE")) {
	    file_type = EXISTING_FILE;
	}
	else if(parseCompare(filetype, "EXISTING_DIR")) {
	    file_type = EXISTING_DIR;
	}
	else if(parseCompare(filetype, "FILE_OR_DIR")) {
	    file_type = FILE_OR_DIR;
	}
	else if(parseCompare(filetype, "FILE_ONLY")) {
	    file_type = FILE_ONLY;
	}
	else if(parseCompare(filetype, "DIR_ONLY")) {
	    file_type = DIR_ONLY;
	}

// September 2012 - Start
	
	else if(parseCompare(filetype, "WRITE_FILE")) {
	    file_type = WRITE_FILE;
	}	
	
// END
	
	else {
            cerr << "createFileDialog: invalid file_type: " << filetype << endl;
	    return;
	}
    }
    if(!parseGetArg(params, "dir", dir)) dir.assign(".");

    if(parseGetArg(params, "file_suffixes", suffixes)) {
	s = strdup(suffixes.c_str());
	tok = s;
	while(num_file_suffixes < 100 && (c = strtok_r(tok, ", \t", &last))) {
	    tok = NULL;
	    file_suffixes[num_file_suffixes++] = c;
	}
	free(s);
    }
	
    if(num_file_suffixes == 0) {
	new FileDialog(name, parent, file_type, dir, "*");
    }
    else {
	new FileDialog(name, parent, file_type, dir,
		num_file_suffixes, file_suffixes, file_suffixes[0], "Open");
    }
}
static void createInfoArea(const string &name, Component *parent,
			const string &params) {
    new InfoArea(name, parent);
}
static void createList(const string &name, Component *parent,
			const string &params)
{
    string str;
    List *list = new List(name, parent);

    if(params.empty()) return;

    if(parseGetArg(params, "items", str)) {
	char *s = strdup(str.c_str());
	int num = 0;
	char *tok, *c, *last;
	tok = s;
	while((c = strtok_r(tok, ",", &last)) != NULL) {
	    tok = NULL;
	    stringTrim(c);
	    list->addItem(c);
	    num++;
	}
	free(s);
	if(num > 0) {
	    Arg args[2];
	    XtSetArg(args[0], XmNitemCount, num);
	    XtSetArg(args[1], XmNvisibleItemCount, num);
	    list->setValues(args, 2);
	}
    }
}
static void createMenuBar(const string &name, Component *parent,
			const string &params) {
    new MenuBar(name, parent);
}
static void createMenu(const string &name, Component *parent,
			const string &params)
{
    if(parent->getPopupMenuInstance()) {
	Arg args[1];
	XtSetArg(args[0], XmNtearOffModel, XmTEAR_OFF_DISABLED);
	new Menu(name, parent->getPopupMenuInstance(), args, 1);
    }
    else if(parent->getMenuInstance()) {
        new Menu(name, parent->getMenuInstance());
    }
    else if(parent->getRowColumnInstance()) {
        new Menu(name, parent->getRowColumnInstance());
    }
    else {
        cerr << "createMenu: invalid parent class." << endl;
    }
}
static void createRowColumn(const string &name, Component *parent,
			const string &params) {
    new RowColumn(name, parent);
}
static void createScale(const string &name, Component *parent,
			const string &params) {
   new Scale(name, parent);
}
static void createScrollBar(const string &name, Component *parent,
			const string &params) {
    new ScrollBar(name, parent);
}
static void createScrolledPane(const string &name, Component *parent,
			const string &params)
{
    new ScrolledPane(name, parent);
}
static void createTextDialog(const string &name, Component *parent,
			const string &params) {
    new TextDialog(name, parent);
}
static void createTextField(const string &name, Component *parent,
			const string &params) {
    new TextField(name, parent);
}
static void createToggle(const string &name, Component *parent,
			const string &params)
{
    if(parent->getPopupMenuInstance()) {
	Arg args[1];
	char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";
	XtTranslations translations = XtParseTranslationTable(trans);
	XtSetArg(args[0], XmNtranslations, translations);
	new Toggle(name, parent, args, 1);
    }
    else {
	new Toggle(name, parent);
    }
}
static void createToolBar(const string &name, Component *parent, const string &params) {
    new ToolBar(name, parent);
}
static void createForm(const string &name, Component *parent, const string &params) {
    new Form(name, parent);
}
static void createLabel(const string &name, Component *parent, const string &params) {
    new Label(name, parent);
}
static void createMainWindow(const string &name, Component *parent, const string &params) {
    new MainWindow(name, parent);
}
static void createScrolledWindow(const string &name, Component *parent, const string &params) {
    new ScrolledWindow(name, parent);
}
static void createSeparator(const string &name, Component *parent, const string &params) {
    new Separator(name, parent);
}
static void createRadioBox(const string &name, Component *parent, const string &params)
{
    RadioBox *radio = new RadioBox(name, parent);
    
    if(params.empty()) return;
    string str;

    if(parseGetArg(params, "toggles", str)) {
	char *s = strdup(str.c_str());
	Toggle *toggle;
	bool first = true;
	char *tok, *c, *last;
	tok = s;
	while((c = strtok_r(tok, ",", &last)) != NULL) {
	    tok = NULL;
	    stringTrim(c);
	    toggle = new Toggle(c, radio);
	    if(first) {
		first = false;
		toggle->set(true);
	    }
	}
	free(s);
    }
}
static void createPane(const string &name, Component *parent, const string &params) {
    new Pane(name, parent);
}
static void createFileSelectionBox(const string &name, Component *parent, const string &params) {
    new FileSelectionBox(name, parent);
}
static void createPopupMenu(const string &name, Component *parent, const string &params) {
    new PopupMenu(name, parent);
}
static void createGroup(const string &name, Component *parent, const string &params) {
    new Group(name, parent);
}

void AppParse::initCreateMethods(void)
{
    addCreateMethod("ArrowButton", createArrowButton);
    addCreateMethod("Button", createButton);
    addCreateMethod("Choice", createChoice);
    addCreateMethod("FileChoice", createFileChoice);
    addCreateMethod("FileDialog", createFileDialog);
    addCreateMethod("FormDialog", createFormDialog);
    addCreateMethod("Frame", createFrame);
    addCreateMethod("InfoArea", createInfoArea);
    addCreateMethod("List", createList);
    addCreateMethod("MenuBar", createMenuBar);
    addCreateMethod("Menu", createMenu);
    addCreateMethod("RowColumn", createRowColumn);
    addCreateMethod("Scale", createScale);
    addCreateMethod("ScrollBar", createScrollBar);
    addCreateMethod("ScrolledPane", createScrolledPane);
    addCreateMethod("TextDialog", createTextDialog);
    addCreateMethod("TextField", createTextField);
    addCreateMethod("Toggle", createToggle);
    addCreateMethod("ToolBar", createToolBar);
    addCreateMethod("Form", createForm);
    addCreateMethod("Label", createLabel);
    addCreateMethod("MainWindow", createMainWindow);
    addCreateMethod("ScrolledWindow", createScrolledWindow);
    addCreateMethod("Separator", createSeparator);
    addCreateMethod("RadioBox", createRadioBox);
    addCreateMethod("Pane", createPane);
    addCreateMethod("FileSelectionBox", createFileSelectionBox);
    addCreateMethod("PopupMenu", createPopupMenu);
    addCreateMethod("Group", createGroup);
}

bool AppParse::addMessageHandler(const string &c)
{
    string msg_id, s;

    if( !parseGetArg(c, "msg_id", msg_id) ) {
	printParseError("message: missing msg_id argument.");
	return false;
    }

    if( !parseGetArg(c, "callback", s) ) {
	printParseError("message: missing callback argument.");
	return false;
    }
    enableCallbackType(msg_id);
    addScriptCallback(s, msg_id);

    return true;
}

void AppParse::parseIPCMessage(const char *msg_id, const char *msg,
			const char *msg_class)
{
    vector<ScriptCallback> *s=NULL;
    vector<char *> names;

    if( scripts.get(msg_id, &s) ) {
	// There is a script assigned to this msg_id. The script will parse
	// the message. The message is composed of variable assignments.
	char *c, *m=NULL, *name, *value;
	string err_msg;

	putGlobalVariable("msg_id", msg_id);

	m = strdup(msg);

	// store the assignments from msg as global variables
	c = m;
	while(*c != '\0') {
	    // get name
	    while(*c != '\0' && isspace((int)*c)) c++;
	    name = c;
	    while(*c != '\0' && *c != '=' && !isspace((int)*c)) c++;
	    if(*c == '=') {
		*c = '\0';
		c++;
		// get value;
		while(*c != '\0' && isspace((int)*c)) c++;
		value = c;
		if(*value == '"' || *value == '\'' || *value == '`') {
		    char quote = *value;
		    value++;
		    while(*c != '\0' && *c != quote) c++;
		    if(*c != quote) {
			cerr << "parseIPCMessage: missing end quote." << endl;
			cerr << msg << endl;
			Free(m);
			return;
		    }
		    *c = '\0';
		    c++;
		}
		else {
		    while(*c != '\0' && !isspace((int)*c)) c++;
		    if(*c != '\0') { *c = '\0'; c++; }
		}
		putGlobalVariable(name, value);
		names.push_back(name);
	    }
	}
	    
	for(int i = 0; i < (int)s->size(); i++) {
	    parse_file.swap(s->at(i).parse_file);
	    start_fs = new FileState();
	    file_states.push_back(start_fs);

	    err_msg.clear();
	    parseLine(s->at(i).script.c_str(), err_msg);
	    if(!err_msg.empty()) printf("%s\n", err_msg.c_str());

	    if(start_fs) {
		delete start_fs;
		start_fs = NULL;
	    }

	    if((int)file_states.size() > 0) file_states.pop_back();

	    s->at(i).parse_file.swap(parse_file);
	}

	removeGlobalVariable("msg_id");

	for(int i = 0; i < (int)names.size(); i++) {
	    removeGlobalVariable(names[i]);
	}

	Free(m);
    }
    else if(!strcasecmp(msg_id, "parse")) {
	// parse the message contents as script commands
	string error;
        parseLine(msg, error);
    }
}

bool AppParse::sendMessage(Component *top, Foreach *fr, char *line)
{
    char sline[100000];
    string dest, msg_id, msg, cl;

    if( !doSubstitutions(top, line, sline, sizeof(sline), true) ) return false;

    if(fr) {
	checkForeachPrefix(fr, sline, sizeof(sline));
    }

    if( !parseGetArg(sline, "dest", dest) ) {
	printParseError("send_message: missing dest argument.");
	return false;
    }
    if( !parseGetArg(sline, "msg_id", msg_id) ) {
	printParseError("send_message: missing msg_id argument.");
	return false;
    }
    if( !parseGetArg(sline, "msg", msg) ) {
	printParseError("send_message: missing msg argument.");
	return false;
    }
    if( !parseGetArg(sline, "class", cl) ) {
	cl.assign("class");
    }

    IPCClient* ipcc = IPCClient::getInstance();

    try {
	ipcc->sendIPCMessage(dest, msg, cl, msg_id);
    } catch(runtime_error) {
	cerr << "sendMessage failed." << endl;
    }

    return true;
}

void AppParse::setTableCallback(const string &cmd, enum TableCBType type)
{
    int i, n;
    char *s, *cb = strdup(cmd.c_str());

    n = (int)strlen(cb);
    if((*cb == '"' && cb[n-1] == '"') || (*cb == '\'' && cb[n-1] == '\'') ||
        (*cb == '`' && cb[n-1] == '`') || (*cb == '{' && cb[n-1] == '}'))
    {
	cb[n-1] = '\0';
	s = cb+1;
    }
    else {
	s = cb;
    }
    if(type == TABLE_MODIFIED) {
	table_modify.script.assign(s);
	for(i = 0; i < (int)parse_file.size(); i++) {
	    table_modify.parse_file.push_back(parse_file[i]);
	}
    }
    else if(type == TABLE_DELETED) {
	table_delete.script.assign(s);
	for(i = 0; i < (int)parse_file.size(); i++) {
	    table_delete.parse_file.push_back(parse_file[i]);
	}
    }
    else if(type == TABLE_ADDED) {
	table_add.script.assign(s);
	for(i = 0; i < (int)parse_file.size(); i++) {
	    table_add.parse_file.push_back(parse_file[i]);
	}
    }
    free(cb);
}

void AppParse::tableModified(TableListenerCallback *tc, const char *command)
{
    string msg;

    if(!strcmp(command, "delete")) {
	if(!table_delete.script.empty()) {
	    modified_table = tc->table;
	    parseCallback(table_delete.script.c_str(), table_delete.parse_file,
				msg);
	    if(!msg.empty()) printf("%s\n", msg.c_str());
	}
    }
    else if(!strcmp(command, "add")) {
	if(!table_add.script.empty()) {
	    modified_table = tc->table;
	    parseCallback(table_add.script.c_str(), table_add.parse_file, msg);
	    if(!msg.empty()) printf("%s\n", msg.c_str());
	}
    }
    else if(!table_modify.script.empty()) {
	char s[200];
	int i, j, num = tc->table->getNumMembers();
	CssClassDescription *des = tc->table->description();
	for(i = 0; i < num; i++) {
	    snprintf(s, sizeof(s), "%s_modified", des[i].name);
	    putGlobalVariable(s, "false");
	}

	for(i = 0; i < tc->num_members; i++) {
	    j = tc->members[i];
	    snprintf(s, sizeof(s), "%s_modified", des[j].name);
	    putGlobalVariable(s, "true");
	}
	
	modified_table = tc->table;
	parseCallback(table_modify.script.c_str(), table_modify.parse_file,msg);
	if(!msg.empty()) printf("%s\n", msg.c_str());

	for(i = 0; i < num; i++) {
	    snprintf(s, sizeof(s), "%s_modified", des[i].name);
	    removeGlobalVariable(s);
	}
    }
    modified_table = NULL;
}

void AppParse::parseWait(long msecs)
{
    XtAppAddTimeOut(app_context, msecs, waitCB, (XtPointer)this);

    for(;;) {
	XEvent event;
	XtAppNextEvent(app_context, &event);
	if(event.type == -9999) return;
	XtDispatchEvent(&event);
    }
}

static void waitCB(XtPointer data, XtIntervalId *id)
{
    AppParse *app = (AppParse *)data;
    app->parseContinue();
}

void AppParse::parseContinue()
{
    XEvent event;
    event.type = -9999;
    event.xany.display = XtDisplay(base_widget);
    // Since the XtAppNextEvent in parseWait blocks until an event is on the
    // queue, put a dummy event on the queue.
    XPutBackEvent(XtDisplay(base_widget), &event);
}
