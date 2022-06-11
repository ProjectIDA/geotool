/** \file Import.cpp
 *  \brief Defines class Import.
 *  \author Ivan Henson
 */
#include "config.h"
#include <errno.h>
#include "Import.h"
#include "motif++/MotifClasses.h"
#include "CssFileDialog.h"
#include "CSSTable.h"
#include "libgio.h"

extern "C" {
#include "libstring.h"
}


Import::Import(const string &name, Component *parent, TableViewer *table_viewer)
		: FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    tv = table_viewer;

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    import_button = new Button("Import", controls, this);
    import_button->setSensitive(false);
    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    label = new Label("Import Tables", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    rc = new RowColumn("rc", this, args, n);

    prefix_button = new Button("Choose Import File", rc, this);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    file_text = new TextField("file_text", rc, this, args, n);

    new Space("space", rc, XmVERTICAL, 10);

    tables_label = new Label("Table Names (table1,table2,...)", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    tables_text = new TextField("tables_text", rc, this, args, n);

    members_label = new Label("Members (member1,member2,...)", rc);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    XtSetArg(args[n], XmNscrollHorizontal, False); n++;
    XtSetArg(args[n], XmNscrollVertical, True); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    members_text = new TextField("members_text", sw, this, args, n);
    members_text->tabKeyFocus();

    assignments_label = new Label(
		"Assignments (member=value, member=value++, ...)", rc);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    assignments_text = new TextField("assignments_text", rc, this, args, n);

    open_file = NULL;

    tv->ondate.clear();
    tv->offdate.clear();
}

Import::~Import(void)
{
    clearNames();
}

void Import::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Import")) {
	import();
    }
    else if(!strcmp(cmd, "Choose Import File")) {
	chooseFile();
    }
    else if(action_event->getSource() == file_text ||
	action_event->getSource() == tables_text ||
	action_event->getSource() == members_text)
    {
	import_button->setSensitive(
		!file_text->empty() &&
		!tables_text->empty() &&
		!members_text->empty());
    }
}

ParseCmd Import::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseString(cmd, "import", c)) {
	return parseImport(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd Import::parseImport(const string &cmd, string &msg)
{
    char *a, *b, *c, *p, *t, *tok, *last, buf[2000], st[200];
    string s;
    CssClassDescription *des;

    if( !parseGetArg(cmd, "members", s) ) {
	msg.assign("import: missing members argument.");
	return ARGUMENT_ERROR;
    }
    members_text->setString(s);

    if( !parseGetArg(cmd, "tables", s) ) {
	msg.assign("import: missing tables argument.");
	return ARGUMENT_ERROR;
    }
    tables_text->setString(s);

    assignments_text->setString("");

    c = strdup(s.c_str());
    tok = c;
    while((t = (char *)strtok_r(tok, " ,\t", &last)) != NULL)
    {
	tok = NULL;
	if(CssTableClass::isTableName(t)) {
	    int n = CssTableClass::getDescription(t, &des);
	    for(int i = 0; i < n; i++) {
		snprintf(st, sizeof(st), "%s_start", des[i].name);
		if( (a = stringGetArg(c, des[i].name)) ) {
		    b = assignments_text->getString();
		    if(!(p = stringGetArg(b, des[i].name))) { // avoid repeat
			snprintf(buf, sizeof(buf), "%s=%s ", des[i].name, a);
			assignments_text->append(buf);
		    }
		    Free(a);
		    Free(b);
		    Free(p);
		}
		else if( (a = stringGetArg(c, st)) ) {
		    b = assignments_text->getString();
		    if(!(p = stringGetArg(b, st))) { // avoid repeat
			snprintf(buf, sizeof(buf), "%s=%s ", st, a);
			assignments_text->append(buf);
		    }
		    Free(a);
		    Free(b);
		    Free(p);
		}
	    }
	}
    }
    free(c);

    if( parseGetArg(cmd, "file", s) ) {
	file_text->setString(s);
    }
    else {
    }

    import();

    return COMMAND_PARSED;
}

void Import::parseHelp(const char *prefix)
{
    printf("%simport\n", prefix);
}

void Import::chooseFile(void)
{
    char *file;

    if(!open_file) {
	open_file = new FileDialog("Choose File", this, EXISTING_FILE,
				"./", (char *)"*", "Choose File");
    }
    open_file->setVisible(true);

    if((file = open_file->getFile()) != NULL)
    {
	file_text->setString(file, true);
	file_text->showPosition((int)strlen(file));
	Free(file);
    }
}

bool Import::import(void)
{
    FILE *fp;
    char *file = file_text->getString();
    char *tables = tables_text->getString();
    char *members = members_text->getString();
    char *assignments = assignments_text->getString();
    bool ret = true;

    if(file && file[0] != '\0') {
	if( !(fp = fopen(file, "r")) ) {
	    showWarning("Cannot open file.\n%s", strerror(errno));
	    ret = false;
	}
	else {
	    ret = import(fp, tables, members, assignments);
	    fclose(fp);
	}
    }
    else {
	ret = importStdin(tables, members, assignments);
    }

    Free(file);
    Free(tables);
    Free(members);

    return ret;
}

bool Import::import(FILE *fp, const string &tables, const string &members,
			const string &assignments)
{
    char line[5000];

    if( !startImport(tables, members) ) {
	// clean up.
	clearNames();
	return false;
    }

    while(fgets(line, sizeof(line)-1, fp)) {
	stringTrim(line);
	if(line[0] != '#' && line[0] != '\0') {
	    if( !importLine(line, assignments) ) break;
	}
    }

    for(int i = 0; i < (int)tv->table_names.size(); i++) {
	CSSTable *table = tv->getCSSTable(tv->table_names[i]);
	if(table) table->list();
    }

    return true;
}

bool Import::importStdin(const string &tables, const string &members,
			const string &assignments)
{
    Application *app = Application::getApplication();
    char line[5000];

    if( !startImport(tables, members) ) {
	// clean up.
	clearNames();
	return false;
    }

    while(app->readParseLine(line, sizeof(line))) {
	if(!strcasecmp(line, "end")) break;
	if(line[0] != '#' && line[0] != '\0') {
	    if( !importLine(line, assignments) ) break;
	}
    }

    for(int i = 0; i < (int)tv->table_names.size(); i++) {
	CSSTable *table = tv->getCSSTable(tv->table_names[i]);
	if(table) table->list();
    }

    return true;
}

void Import::clearNames(void)
{
    for(int i = 0; i < (int)tv->table_names.size(); i++) {
	free(tv->table_names[i]);
    }
    tv->table_names.clear();

    for(int i = 0; i < (int)tv->member_names.size(); i++) {
	free(tv->member_names[i]);
    }
    tv->member_names.clear();

    tv->ondate.clear();
    tv->offdate.clear();
}

bool Import::startImport(const string &tables, const string &members)
{
    char *p, *t, *tok, *last;
    int i, j, num_members;
    CssClassDescription *des;

    clearNames();
    ids.clear();

    p = strdup(tables.c_str());
    tok = p;
    while((t = (char *)strtok_r(tok, " ,\t", &last)) != NULL)
    {
	tok = NULL;
	if(!CssTableClass::isTableName(t)) {
	    showWarning("Import: unknown table name %s", t);
	    free(p);
	    return false;
	}
	tv->table_names.push_back(strdup(t));
    }
    free(p);

    if(tv->table_names.size() == 0) {
	showWarning("Import: no tables specified.");
	return false;
    }

    if(!members.compare("*")) {
	for(i = 0; i < (int)tv->table_names.size(); i++) {
	    num_members = CssTableClass::getDescription(tv->table_names[i], &des);
	    for(j = 0; j < num_members; j++) {
		tv->member_names.push_back(strdup(des[j].name));
	    }
	}
     }
    else {
	p = strdup(members.c_str());
	tok = p;
	while((t = (char *)strtok_r(tok, " ,\t\n)", &last)) != NULL)
	{
	    tok = NULL;

	    if(strcmp(t, "-") && strcmp(t, "_"))  // use to skip columns
	    {
		for(i = 0; i < (int)tv->table_names.size()
                        && !CssTableClass::isMember(tv->table_names[i], t); i++);
		if(i == (int)tv->table_names.size()) {
		    showWarning("Import: member not found in import tables: %s",
                                t);
		    free(p);
		    return false;
		}
	    }
	    tv->member_names.push_back(strdup(t));
	}
	free(p);
    }

    return true;
}

bool Import::importLine(char *line, const string &assignments)
{
    int i, j, k, s, e;
    long id;
    char *a, *l, c, b[200];
    CssTableClass *t;
    CSSTable *table;
    gvector<CssTableClass *> v;

    if(tv->table_names.size() == 0) {
	showWarning("importLine: no import tables specified.");
	return false;
    }
    if(tv->member_names.size() == 0) {
	showWarning("importLine: no import members specified.");
	return false;
    }

//    if(!(v = new_Vector2((int)tv->table_names.size(), 1))) return False;

    for(i = 0; i < (int)tv->table_names.size(); i++) {
	t = CssTableClass::createCssTable(tv->table_names[i]);
	v.push_back(t);

	if( !assignments.empty() ) {
	    int n = t->getNumMembers();
	    CssClassDescription *des = t->description();
	    for(j = 0; j < n; j++) {
		snprintf(b, sizeof(b), "%s_start", des[j].name);
		if( (a = stringGetArg(assignments.c_str(), des[j].name)) ) {
		    t->setMember(j, a);
		    Free(a);
		}
		else if( (a = stringGetArg(assignments.c_str(), b)) ) {
		    if(!ids.get(des[j].name, &id) && stringToLong(a, &id)) {
			ids.put(des[j].name, id);
		    }
		    else {
			snprintf(b, sizeof(b), "%ld", id);
			Free(a);
			a = strdup(b);
		    }
		    t->setMember(j, a);
		    Free(a);
		}
	    }
	}
    }

    l = strdup(line);
    s = 0;

    for(j = 0; j < (int)tv->member_names.size(); j++)
    {
	while(l[s] != '\0' && isspace((int)l[s])) s++;

	if(l[s] == '"') {
	    s++;
	    for(e = s; l[e] != '\0' && l[e] != '"'; e++);
	}
	else {
	    for(e = s; l[e] != '\0' && !isspace((int)l[e]); e++);
	}

	c = l[e];
	l[e] = '\0';

	for(i = 0; i < (int)tv->table_names.size(); i++)
	{
	    t = v[i];
	    if((k = t->memberIndex(tv->member_names[j])) >= 0)
	    {
		if(!t->setMember(k, l+s)) {
		    free(l);
		    return false;
		}
	    }
	}
	if(c == '\0') break;
	s = e+1;
    }
    free(l);

    for(i = 0;  i < (int)tv->table_names.size(); i++) {
	if(!(table = tv->getCSSTable(tv->table_names[i]))) {
	    table = tv->addTableTab(tv->table_names[i], false);
	}
	gvector<CssTableClass *> *w = table->getRecords();
	if(w) {
	    for(j = 0; j < w->size() && (*(w->at(j)) != *v[i]); j++);
	    if(j == w->size()) {
		table->addRecord(v[i], false);
	    }
	}
    }

    for(i = 0; i < (int)ids.elements.size(); i++) {
	ids.elements[i]->second++;
    }

    return true;
}
