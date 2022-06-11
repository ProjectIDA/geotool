/** \file TableQuery.cpp
 *  \brief Defines class TableQuery.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "TableQuery.h"
#include "QueryViews.h"
#include "WaveformWindow.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "OpenDB.h"
#include "IIRFilter.h"
#include "CalibData.h"
#include "TaperData.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GSourceInfo.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
static int sortLong(const void *A, const void *B);
static int sortByDist(const void *A, const void *B);
static int sortByOrid(const void *A, const void *B);
static int sortByTime(const void *A, const void *B);
static int sortBySta(const void *A, const void *B);
}
#ifdef HAVE_LIBODBC
static int ODBC_queryTableResults(ODBC_QueryTableStruct *qs, int numToFetch,
		int *numFetched, gvector<CssTableClass *> &table);
#endif

#define PERIOD(a) (a[0] != '\0' ? "." : "")

TableQuery::TableQuery(const string &name, Component *parent) :
		TableViewer(name, parent)
{
    createInterface(true);
    init();
}

TableQuery::TableQuery(const string &name, Component *parent, bool auto_connect)
		: TableViewer(name, parent)
{
    createInterface(auto_connect);
    init();
}

void TableQuery::createInterface(bool auto_connect)
{
    setSize(570, 650);

    setFileMenu();
    setOptionMenu();
    setHelpMenu();

    open_db = new OpenDB("Database Connection", this, auto_connect, this);

    if(open_db->sourceType() == PREFIX_CONNECTION) {
	get_all_tables->setVisible(true);
    }
    else {
	get_all_tables->setVisible(false);
    }

    display_text_area = true;
    fileDialog = NULL;
}

void TableQuery::setFileMenu(void)
{
    new_tv_button->setVisible(false);
    connection_button = new Button("Database Connection...", file_menu, 0, this);
    query_button = new Button("Query", file_menu, 2, this);
    tool_bar->add(query_button, "Query", 0);
    new_tq_button = new Button("New TableQuery...", file_menu, 6, this);
    recent_files_menu->setVisible(false);
}

void TableQuery::setEditMenu(void)
{
    TableViewer::setEditMenu();
}

void TableQuery::setViewMenu(void)
{
    TableViewer::setViewMenu();
}

void TableQuery::setOptionMenu(void)
{
    option_menu->setVisible(true);
    display_arrivals = new Button("Display Arrivals", option_menu, this);
    tool_bar->add(display_arrivals);
    display_arrivals->setSensitive(false);
    display_waveforms = new Button("Display Waveforms", option_menu, this);
    tool_bar->add(display_waveforms);
    display_waveforms->setSensitive(false);
//    map_origins = new Button("Map Origins", option_menu, this);
    review_origins = new Button("Review Origins", option_menu, this);
    tool_bar->add(review_origins);
    review_origins->setSensitive(false);
    option_sep1 = option_menu->addSeparator("sep1");
//    get_ampdescripts = new Button("Get Ampdescripts", option_menu, this);
//    get_ampdescripts->setCommandString("Get ampdescript");
    get_all_tables = new Button("Get All Tables", option_menu, this);
    get_all_tables->setVisible(false);
    get_amplitudes = new Button("Get Amplitudes", option_menu, this);
    get_amplitudes->setCommandString("Get amplitude");
	saveButton(cssAmplitude, get_amplitudes, "ampid");
	saveButton(cssAmplitude, get_amplitudes, "arid");
	saveButton(cssAmplitude, get_amplitudes, "parid");
    get_aao = new Button("Get Arrivals,Assocs,Origerrs", option_menu, this);
    get_aao->setSensitive(false);
    get_aawo = new Button("Get Arrivals,Assocs,Origerrs,Wfdiscs", option_menu,
			this);
    get_aawo->setSensitive(false);
    get_arrivals = new Button("Get Arrivals", option_menu, this);
    get_arrivals->setCommandString("Get arrival");
	saveButton(cssArrival, get_arrivals, "arid");
	saveButton(cssArrival, get_arrivals, "stassid");
    get_assocs = new Button("Get Assocs", option_menu, this);
    get_assocs->setCommandString("Get assoc");
	saveButton(cssAssoc, get_assocs, "orid");
	saveButton(cssAssoc, get_assocs, "arid");
    get_filters = new Button("Get Filters", option_menu, this);
    get_filters->setCommandString("Get filter");
	saveButton(cssFilter, get_filters, "chanid");
	saveButton(cssFilter, get_filters, "arid");
	saveButton(cssFilter, get_filters, "wfid");
    get_fsdiscs = new Button("Get Fsdiscs", option_menu, this);
    get_fsdiscs->setCommandString("Get fsdisc");
	saveButton(cssFsdisc, get_fsdiscs, "arid");
	saveButton(cssFsdisc, get_fsdiscs, "wfid");
	saveButton(cssFsdisc, get_fsdiscs, "chanid");
	saveButton(cssFsdisc, get_fsdiscs, "fsrid");
	saveButton(cssFsdisc, get_fsdiscs, "fsid");
    get_fsrecipes = new Button("Get Fsrecipes", option_menu, this);
    get_fsrecipes->setCommandString("Get fsrecipe");
	saveButton(cssFsrecipe, get_fsrecipes, "fsrid");
    get_fstags = new Button("Get Fstags", option_menu, this);
    get_fstags->setCommandString("Get fstag");
	saveButton(cssFstag, get_fstags, "afsid");
	saveButton(cssFstag, get_fstags, "fsid");
    get_hydrofeatures = new Button("Get HydroFeatures", option_menu, this);
    get_hydrofeatures->setCommandString("Get hydrofeatures");
	saveButton(cssHydroFeatures, get_hydrofeatures, "arid");
    get_infrafeatures = new Button("Get InfraFeatures", option_menu, this);
    get_infrafeatures->setCommandString("Get infrafeatures");
	saveButton(cssInfraFeatures, get_infrafeatures, "arid");
    get_instruments = new Button("Get Instruments", option_menu, this);
    get_instruments->setCommandString("Get instrument");
	saveButton("instrument", get_instruments, "inid");
    get_netmags = new Button("Get Netmags", option_menu, this);
    get_netmags->setCommandString("Get netmag");
	saveButton(cssNetmag, get_netmags, "magid");
	saveButton(cssNetmag, get_netmags, "orid");
	saveButton(cssNetmag, get_netmags, "evid");
    get_origaux = new Button("Get Origaux", option_menu, this);
    get_origaux->setCommandString("Get origaux");
	saveButton(cssOrigaux, get_origaux, "evid");
	saveButton(cssOrigaux, get_origaux, "orid");
    get_origerrs = new Button("Get Origerrs", option_menu, this);
    get_origerrs->setCommandString("Get origerr");
	saveButton(cssOrigerr, get_origerrs, "orid");
    get_origins = new Button("Get Origins", option_menu, this);
    get_origins->setCommandString("Get origin");
	saveButton(cssOrigin, get_origins, "orid");
	saveButton(cssOrigin, get_origins, "evid");
    get_sensors = new Button("Get Sensors", option_menu, this);
    get_sensors->setCommandString("Get sensor");
	saveButton("sensor", get_sensors, "inid");
	saveButton("sensor", get_sensors, "chanid");
    get_sitechans = new Button("Get Sitechans", option_menu, this);
    get_sitechans->setCommandString("Get sitechan");
	saveButton(cssSitechan, get_sitechans, "chanid");
    get_stamags = new Button("Get Stamags", option_menu, this);
    get_stamags->setCommandString("Get stamag");
	saveButton(cssStamag, get_stamags, "magid");
	saveButton(cssStamag, get_stamags, "ampid");
	saveButton(cssStamag, get_stamags, "arid");
	saveButton(cssStamag, get_stamags, "orid");
	saveButton(cssStamag, get_stamags, "evid");
    get_stassocs = new Button("Get Stassocs", option_menu, this);
    get_stassocs->setCommandString("Get stassoc");
	saveButton(cssStassoc, get_stassocs, "stassid");
    get_wfdiscs = new Button("Get Wfdiscs", option_menu, this);
    get_wfdiscs->setCommandString("Get wfdisc");
	saveButton(cssWfdisc, get_wfdiscs, "wfid");
    get_wftags = new Button("Get Wftags", option_menu, this);
    get_wftags->setCommandString("Get wftag");
	saveButton(cssWftag, get_wftags, "tagid");
	saveButton(cssWftag, get_wftags, "wfid");
    option_sep2 = option_menu->addSeparator("sep2");
    option_sep3 = option_menu->addSeparator("sep3");
//    select_tables_button = new Button("Select Tables...", option_menu, this);
//    option_sep4 = option_menu->addSeparator("sep4");
    views_menu = new Menu("Views", option_menu, true);
}

void TableQuery::setHelpMenu(void)
{
    help_button->setVisible(false);
    new Button("TableQuery Help", help_menu, this);
}

TableQuery::~TableQuery(void)
{
    if(assocs_sort) delete assocs_sort;
    delete gbeam;
}

void TableQuery::saveButton(const string &name, Button *b,const string &id_name)
{
    TableId t;
    t.table_name = name;
    t.id_name = id_name;
    t.button = b;
    table_ids.push_back(t);
    b->setSensitive(false);
}

void TableQuery::init()
{
    int n;
    const char *c;
    string s;
    char *tok, *prop, *last;

    last_arid_input = false;
    last_display_waveforms = true;
    waveform_subclass = false;
    no_records_warn = true;
    current_query = NULL;
    ignore_getpath = false;

    numSchema = 0;
#ifdef HAVE_LIBODBC
    schema = NULL;
#endif

    assocs_sort = NULL;

    query_views = new QueryViews("Views", this, views_menu);
    query_views->addActionListener(this);

    if(!getProperty("queryTables", s)) {
        putProperty("queryTables", "origin,arrival,wfdisc", false);
    }

    if((prop = getProperty("queryTables")) != NULL) {
	tok = prop;
	n = 0;
	while((c = (char *)strtok_r(tok, " ,;:\t", &last)) != NULL)
	{
	    tok = NULL;
	    addTableTab(c);
	    if(n == 0) {
		tabSelect(c);
	    }
	    n++;
	}
	free(prop);
    }

    rc = NULL;
    num_review_channels = 0;

    readReviewChannels();

    for(int i = 0; i < query_views->num_views; i++) {
	query_views->views[i].b->setSensitive(false);
    }

    enableCallbackType(XtNdataChangeCallback);

    addPlugins("TableQuery", this, NULL);

    string recipe_dir, recipe_dir2;

    if(!getProperty("recipeDir", recipe_dir)) {
	char path[MAXPATHLEN+1];
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    snprintf(path, sizeof(path), "%s/tables/recipes", c);
	    recipe_dir.assign(path);
	}
    }
    getProperty("recipeDir2", recipe_dir2);
    gbeam = new Beam(recipe_dir, recipe_dir2);
}

void TableQuery::readReviewChannels(void)
{
    const char *cs;
    string file;
    char *last, line[101], *sta, *chan, *c;
    int i, n;
    FILE *fp;

    if(!getProperty("reviewChannelsFile", file)) {
	if((cs = Application::getInstallDir("GEOTOOL_HOME"))) {
	    file = cs + string("/tables/static/reviewChannels");
	}
	else return;
    }

    if(!(fp = fopen(file.c_str(), "r"))) {
	putWarning("Cannot open %s", file.c_str());
	return;
    }
    Free(rc);
    num_review_channels = 0;
    n = 0;
    while(stringGetLine(fp, line, 100) != EOF && num_review_channels < 1000)
    {
	n++;
	stringTrim(line);
	if(line[0] == '#' || line[0] == '\0') continue;

	if((sta = (char *)strtok_r(line," ,;:\t/",&last)) == NULL) continue;

	if(strlen(sta) >= 10) {
	    fprintf(stderr,
		"Error in file %s\nline %d: station name is too long.\n",
		file.c_str(), n);
	    break;
	}
	if((chan = (char *)strtok_r(NULL," ,;:\t/",&last)) == NULL) {
	    fprintf(stderr,
		"Error in file %s\nline %d: missing channel name.\n",
		file.c_str(), n);
	    break;
	}
	if(strlen(chan) >= 10) {
	    fprintf(stderr,
		"Error in file %s\nline %d: channel name is too long.\n",
		file.c_str(), n);
	    break;
	}

	rc = (ReviewChannel *)reallocWarn(rc,
			(num_review_channels+1)*sizeof(ReviewChannel));
	if(!rc) break;
	i = num_review_channels;
	strcpy(rc[i].sta, sta);
	strcpy(rc[i].chan, chan);
	num_review_channels++;
	rc[i].filter = false;
	rc[i].flow = 0.;
	rc[i].fhigh = 0.;
	rc[i].order = 0;
	rc[i].zero_phase = 0;
	rc[i].type[0] = '\0';

	if((c = (char *)strtok_r(NULL," ,;:\t/",&last)) != NULL) {
	    if(!getReviewFilter(&rc[i], c, &last)) {
		fprintf(stderr, "Error in file %s\nline %d: filter format.\n",
			file.c_str(), n);
	    }
	}
    }
    fclose(fp);
}

bool TableQuery::getReviewFilter(ReviewChannel *rch, char *c, char **last)
{
    if(!stringToDouble(c, &rch->flow)) return false;
    if((c = (char *)strtok_r(NULL, " ,;:\t/", last)) == NULL) return false;
    if(!stringToDouble(c, &rch->fhigh)) return false;
    if((c = (char *)strtok_r(NULL, " ,;:\t/", last)) == NULL) return false;
    if(!stringToInt(c, &rch->order)) return false;
    if((c = (char *)strtok_r(NULL, " ,;:\t/", last)) == NULL) return false;
    if(!stringToInt(c, &rch->zero_phase)) return false;
    if((c = (char *)strtok_r(NULL, " ,;:\t/", last)) == NULL) return false;
    stringcpy(rch->type, c, sizeof(rch->type));
    if(!strcasecmp(rch->type, "NA")) {
	rch->filter = false;
    }
    else if(strcasecmp(rch->type, "BP") && strcasecmp(rch->type, "BR") &&
	    strcasecmp(rch->type, "LP") && strcasecmp(rch->type, "HP"))
    {
	return false;
    }
    rch->filter = true;
    return true;
}

void TableQuery::queryView(char *view_query)
{           
    char *query_string, *c, *q, qtime[20];
    const char *labels[1];
    int n;
    CSSTable *table;
    double otime;
    cvector<CssOriginClass> v;
        
    if((table = getCSSTable(cssOrigin)) == NULL) return;

    table->getSelectedRecords(v);
    if(v.size() <= 0) {
	showWarning("No origin selected.");
	return;
    }
    else if(v.size() > 1) {
	showWarning("More than one origin selected.");
	return;
    }
    otime = v[0]->time;

    query_string = (char *)malloc(1);
    query_string[0] = '\0';
    q = view_query;
    while((c = strstr(q, "origin_time")) != NULL)
    {
	n = strlen(query_string) + c - q;
	query_string = (char *)realloc(query_string, n+21);
	strncat(query_string, q, c-q);
	query_string[n] = '\0';
	snprintf(qtime, 20, "%.2f", otime);
	strcat(query_string, qtime);
	q = c + 11;
    }
    n = strlen(query_string) + strlen(q);
    query_string = (char *)realloc(query_string, n+1);
    strcat(query_string, q);

    clearTab(cssWfdisc);
    labels[0] = cssWfdisc;
    showWorking(1, labels);
    runQuery(cssWfdisc, query_string);
    closeWorking();

    saveQueryText(cssWfdisc, query_string);
    free(query_string);
}

void TableQuery::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    string tableName = getTopTabName();
    Component *comp = action_event->getSource();

    resetDataChange(false);

    TableViewer::actionPerformed(action_event);

    if(!strcmp(cmd, "Open File...")) {
	open();
    }
    else if(!strcmp(cmd, "Clear") || comp == query_views) {
	tabSelect(tableName);
    }
    else if(!strcmp(cmd, "Database Connection...")) {
	open_db->setVisible(true);
    }
    else if(!strcmp(reason, XtNselectRowCallback))
    {
	selectTableRow(tableName);
	change.unknown_select = true;
    }
    else if(!strcmp(cmd, "Display Author")) {
    }
    else if(!strncmp(cmd, "view ", 5)) {
	int i;
	if(stringToInt(cmd+5, &i) && i >= 0 && i < query_views->num_views)
	{
	    queryView(query_views->views[i].query);
	}
    }
    else if(!strcmp(cmd, "Database Connection")) {
	if(!strcmp(reason, XmNactivateCallback)) {
	    OpenDBStruct *op = (OpenDBStruct *)action_event->getCalldata();
	    if(!op->tabName.compare("time")) {
		timeRunQuery(op);
	    }
	    else if(!op->tabName.compare(cssArrival)) {
		arrivalRunQuery(op);
	    }
	    else if(!op->tabName.compare(cssOrigin)) {
		originRunQuery(op);
	    }
	    else if(!op->tabName.compare("general")) {
		unspecifiedRunQuery(op);
	    }
	}
	else if(!strcmp(reason, XtNconnectionCallback)) {
	    if(open_db->sourceType() == PREFIX_CONNECTION) {
		get_all_tables->setVisible(true);
	    }
	    else {
		get_all_tables->setVisible(false);
	    }
	    tabSelect(tableName);
	}
    }
    else if(!strcmp(cmd, "queryText") || !strcmp(cmd, "Query")) {
	tabQuery();
    }
    else if(!strcmp(cmd, "Get All Tables")) {
	getAllTables();
    }
    else if(!strncmp(cmd, "Get ", 4))
    {
	string tab_name = getTopTabName();
	const char *labels[1];

	if(tab_name.empty()) return;
	if(!strcmp(cmd, "Get wftag") && !tab_name.compare(cssOrigin))
	{
	    labels[0] = cssWftag;
	    showWorking(1, labels);
	    getWftagsFromOrigins(); /* special case */
	    closeWorking();
	}
	else if(!strcmp(cmd, "Get wfdisc") && !tab_name.compare(cssArrival))
	{
	    labels[0] = cssWfdisc;
	    showWorking(1, labels);
	    wfdiscsFromArrival();
	    closeWorking();
	}
	else if(!strcmp(cmd, "Get Arrivals,Assocs,Origerrs"))
	{
	    getAAWO(false);
	}
	else if(!strcmp(cmd, "Get Arrivals,Assocs,Origerrs,Wfdiscs"))
	{
	    getAAWO(true);
	}
	else {
	    getAnotherTable(tab_name.c_str(), cmd+4);
	}
    }
    else if(!strcmp(cmd, "Display Arrivals")) {
	displayWaveforms(DISPLAY_ARRIVALS);
    }
    else if(!strcmp(cmd, "Display Waveforms")) {
	displayWaveforms(DISPLAY_WAVEFORMS);
    }
    else if(!strcmp(cmd, "Review Origins")) {
	displayWaveforms(REVIEW_ORIGINS);
    }
    else if(!strcmp(cmd, "New TableQuery..."))
    {
	int i;
	for(i = 0; i < (int)windows.size(); i++) {
	    if( !windows[i]->isVisible() ) {
		windows[i]->setVisible(true);
		break;
	    }
	}
	if(i == (int)windows.size()) {
	    TableQuery *tq = new TableQuery(getName(), this);
	    windows.push_back(tq);
	    tq->setVisible(true);
	    tq->setWaveformReceiver(waveform_receiver);
	}
    }
    else if(!strcmp(cmd, "TableQuery") &&	
	!strcmp(reason, XtNsetVisibleCallback))
    {
	if(!(bool)action_event->getCalldata()) {
	    action_event->getSource()->destroy();
	}
    }
    else if(!strcmp(cmd, "Reset All")) {
	resetAll();
    }
    else if(!strcmp(cmd, "staList") || !strcmp(cmd, "chanList") ||
	    !strcmp(cmd, "ctypeList") || !strcmp(cmd, "dateList"))
    {
	selectConstraint();
    }
    else if(!strcmp(cmd, "StartPhaseList")) {
	time_before_phase->setSensitive(start_phase_list->itemSelected());
    }
    else if(!strcmp(cmd, "EndPhaseList")) {
	time_after_phase->setSensitive(end_phase_list->itemSelected());
    }
    else if(!strcmp(cmd, "arrow")) {
	arrowSelect();
    }
    else if(!strcmp(cmd, "TableQuery Help")) {
	showHelp("TableQuery Help");
    }
    if(dataChange()) {
	doCallbacks(baseWidget(), (XtPointer)&change, XtNdataChangeCallback);
    }
}

ParseCmd TableQuery::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = COMMAND_PARSED;
    bool selected_origin=false;
    bool selected_arrival=false;
    int i, j, n;
    long addr;
    stringstream os;
    string c, s;
    const char *p;

    for(i = 0; i < 10; i++) {
	os.str("");
	os << i+2;
	if(parseString(cmd, os.str(), s)) {
	    if(i < (int)windows.size()) {
		return windows[i]->parseCmd(s, msg);
	    }
	    for(j = (int)windows.size(); j <= i; j++) {
		TableQuery *tq = new TableQuery(getName(), this);
		tq->removeAllTabs();
		windows.push_back(tq);
		if(j == i) {
		    return tq->parseCmd(s, msg);
		}
	    }
	}
    }

    resetDataChange(false);

    if(parseArg(cmd, "select_tab", s)) {
	tab->setOnTop(s);
    }
    else if(parseArg(cmd, "database_connection", s) ||
	parseArg(cmd, "database", s) || parseArg(cmd, "connection", s))
    {
	ret = open_db->parseCmd(s, msg);
    }
    else if(parseCompare(cmd, "query")) {
	tabQuery();
    }
    else if(parseCompare(cmd, "clear")) {
	TableViewer::parseCmd(cmd, msg);
	tabSelect(getTopTabName());
    }
    else if(parseString(cmd, "query", s)) {
	string table_name;
	char quote;
	
 	p = s.c_str();
	while(*p != '\0' && isspace((int)*p)) p++;

	if(*p == '"' || *p == '\'' || *p == '`') p++;
	i = 0;
	while(*p != '\0' && !isspace((int)*p)){
	    if(*p != '"' && *p != '\'' && *p != '`') {
		table_name.push_back(*p);
		i++;
	    }
	    p++;
	}

	while(*p != '\0' && isspace((int)*p)) p++;
	quote = '\0';
	if(*p == '"' || *p == '\'' || *p == '`') { quote = *p; p++; }
	i = 0;
	while(*p != '\0') {
	    c.push_back(*p++);
	    i++;
	}
	if(c[i-1] == quote) c.erase(i-1);

	if(table_name.empty()) {
	    msg.assign("query: missing table_name");
	    return ARGUMENT_ERROR;
	}
	if(c.empty()) {
	    msg.assign("query: missing query");
	    return ARGUMENT_ERROR;
	}

	if(!CssTableClass::isTableName(table_name)) {
	    msg.assign(string("query: unknown table name: ") + table_name);
	    return ARGUMENT_ERROR;
	}
	char *q = strdup(c.c_str());
	runQuery(table_name, q);
	free(q);
    }
    else if(parseCompare(cmd, "get_all_tables")) {
	getAllTables();
    }
    else if(parseCompare(cmd, "get_", 4))
    {
	string tab_name = getTopTabName();
	const char *labels[1];

	if(tab_name.empty()) return ARGUMENT_ERROR;
	if(parseCompare(cmd, "get_wftag") && !tab_name.compare(cssOrigin))
	{
	    labels[0] = cssWftag;
	    showWorking(1, labels);
	    getWftagsFromOrigins(); /* special case */
	    closeWorking();
	}
	else if(parseCompare(cmd, "get_wfdisc")
			&& parseCompare(tab_name, cssArrival))
	{
	    labels[0] = cssWfdisc;
	    showWorking(1, labels);
	    wfdiscsFromArrival();
	    closeWorking();
	}
	else if(parseCompare(cmd, "get_wfdiscs ", 12)) {
	    wfdiscsFromArrival();
	}
	else if(parseCompare(cmd, "get_arrivals_assocs_origerrs")
		|| parseCompare(cmd, "get_aao ", 8))
	{
	    getAAWO(false);
	}
	else if(parseCompare(cmd, "get_arrivals_assocs_origerrs_wfdiscs")
		|| parseCompare(cmd, "get_aaow ", 9))
	{
	    getAAWO(true);
	}
	else {
	    getAnotherTable(tab_name, cmd.substr(4));
	}
    }
    else if(parseArg(cmd, "waveform_display_constraints", s)) {
	ret = parseWaveformConstraint(s);
    }
    else if(parseArg(cmd, "view", s)) {
	if(stringToInt(s.c_str(), &i) && i > 0 && i <= query_views->num_views)
	{
	    queryView(query_views->views[i-1].query);
	}
    }
    else if(parseCompare(cmd, "display_arrivals")) {
	displayWaveforms(DISPLAY_ARRIVALS);
    }
    else if(parseCompare(cmd, "display_waveforms")) {
	displayWaveforms(DISPLAY_WAVEFORMS);
    }
    else if(parseCompare(cmd, "review_origins")) {
	displayWaveforms(REVIEW_ORIGINS);
    }
    else if(parseCompare(cmd, "select_beam_channels", 20)) {
	string sta, chan;
	long arid;
	if(parseGetArg(cmd, "sta", sta)) {
	    if(!parseGetArg(cmd, "chan", chan)) chan.assign("cb");
	    selectBeamChannels(sta, chan);
	}
	else if(parseGetArg(cmd, "arid", &arid)) {
	    gvector<CssTableClass *> *v = getTableRecords(cssArrival, true);
	    if( v && v->size() > 0) {
		CssArrivalClass *arrival = (CssArrivalClass *)v->at(0);
		if(!parseGetArg(cmd, "chan", chan)) chan.assign("cb");
		selectBeamChannels(arrival->sta, chan);
	    }
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "open_file", s)) {
	char *prefix = strdup(s.c_str());
	for(i = (int)strlen(prefix)-1; i >= 0; i--) {
	    if(prefix[i] == '.') {
		if(CssTableClass::isTableName(prefix+i+1)) {
		    prefix[i] = '\0';
		}
		else {
		    Free(prefix);
                }
		break;
            }
	}
        if(i <= 0) {
	    Free(prefix);
	}
	if(prefix) {
	    open_db->autoConnectFilePrefix(prefix);
	    getAllTables();
	    Free(prefix);
	}
	else {
	    msg.assign(string("Unknown table file: ") + s);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseString(cmd, "connect", s)) {
	ret = parseConnect(s, msg);
    }
    else if(parseCompare(cmd, "disconnect")) {
	open_db->disconnect();
    }
    else if(parseCompare(cmd, "read_waveforms", 14)) {
	ostringstream os;

	// turn off constraints before parsing command
	resetAll();

	if(parseGetArg(cmd, "_origin_", &addr)) {
	    CSSTable *table;
	    if((table = getCSSTable(cssOrigin)) == NULL) {
		msg.assign("origin not found");
		return ARGUMENT_ERROR;
	    }
	    table->selectAllRows(false);
	    os.str("");
	    os << "select _origin_=" << addr;
	    if(TableViewer::parseCmd(os.str(), msg) != COMMAND_PARSED) {
		msg.assign("origin not found");
		return ARGUMENT_ERROR;
	    }
	    selected_origin = true;
	}
	if(parseGetArg(cmd, "_arrival_", &addr)) {
	    CSSTable *table;
	    if((table = getCSSTable(cssArrival)) == NULL) {
		msg.assign("arrival not found");
		return ARGUMENT_ERROR;
	    }
	    table->selectAllRows(false);
	    os.str("");
	    os << "select _arrival_=" << addr;
	    if(TableViewer::parseCmd(os.str(), msg) != COMMAND_PARSED) {
		msg.assign("arrival not found");
		return ARGUMENT_ERROR;
	    }
	    CssArrivalClass *a = (CssArrivalClass *)addr;
	    parse_start_phase.assign(a->iphase);
	    parse_end_phase.assign(a->iphase);
	    selected_arrival = true;
	}
	if(selected_origin) {
	    tab->setOnTop(cssOrigin);
	}
	else if(selected_arrival) {
	    tab->setOnTop(cssArrival);
	}
	else {
	    tab->setOnTop(cssWfdisc);
	}

	parseWaveformConstraint(cmd);

	displayWaveforms(DISPLAY_WAVEFORMS);
    }
    else if(parseString(cmd, "mapping", s)) {
	ret = open_db->parseCmd(cmd, msg);
    }
    else if(parseString(cmd, "list", s)) {
	string file;
	if(!parseGetArg(s, "file", file)) {
	    p = s.c_str();
	    for(i = 0; p[i] != '\0' &&  isspace(p[i]); i++);
	    if(p[i] == '"' || p[i] == '\'' || p[i] == '`') {
		for(j = i+1; p[j] != '\0' && p[j] != p[i]; j++);
		i++;
	    }
	    else {
		for(j = i; p[j] != '\0' && !isspace(p[j]); j++);
	    }
	    file.assign(p+i, j-i);
	}
	if( !file.empty() ) {
	    // get prefix
	    n = (int)file.length();
	    for(i = n-1; i >= 0 && file[i] != '/'; i--) {
		if(file[i] == '.') {
		    file.erase(i);
		    break;
		}
	    }
	    string full_path;
	    Application::getFullPath(file, full_path);
	    putProperty("file_prefix", full_path, false);
	    open_db->disconnect();
	    open_db->autoConnectFilePrefix();
	    getAllTables();
	    tabSetOnTop(cssWfdisc);
	    setVisible(true);
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if((ret = pluginParse(cmd, msg)) != COMMAND_NOT_FOUND) {
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	ret = TableViewer::parseCmd(cmd, msg);
    }
    if(dataChange()) {
	doCallbacks(baseWidget(), (XtPointer)&change, XtNdataChangeCallback);
    }
    return ret;
}

ParseCmd TableQuery::parseConnect(const string &cmd, string &msg)
{
    ParseCmd ret;
    string s, ds, user, passwd, param_root, seg_root, prefix;

    parseGetArg(cmd, "data_source", ds);
    if(!parseGetArg(cmd, "user", user)) parseGetArg(cmd, "account", user);
    parseGetArg(cmd, "password", passwd);

    if( !ds.empty() || !user.empty() || !passwd.empty() )
    {
	if( ds.empty() ) {
	    msg.assign("connect: missing 'data_source' argument.");
	    return ARGUMENT_ERROR;
	}
	if( user.empty() )
	{
	    msg.assign("connect: missing 'user' argument.");
	    return ARGUMENT_ERROR;
	}
	if( passwd.empty() ) {
	    msg.assign("connect: missing 'password' argument.");
	    return ARGUMENT_ERROR;
	}

	s.assign(string("data_source=") + ds);
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) {
	    return ret;
	}
	s.assign(string("user=") + user);
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) {
	    return ret;
	}
	s.assign(string("password=") + user);
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) {
	    return ret;
	}
	if( (ret=open_db->parseCmd("connect", msg)) != COMMAND_PARSED)
	{
	    return ret;
	}
	return COMMAND_PARSED;
    }
    else if( parseGetArg(cmd, "prefix", prefix) ) {
	if( (ret = open_db->parseCmd("data_source='File-Prefix'", msg))
		!= COMMAND_PARSED)
	{
	    return ret;
	}
	s.assign(string("prefix='") + prefix + "'");
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) return ret;

	if( (ret = open_db->parseCmd("connect", msg))
		!= COMMAND_PARSED) return ret;

	return COMMAND_PARSED;
    }

    parseGetArg(cmd, "param_root", param_root);
    parseGetArg(cmd, "seg_root", seg_root);

    if(!param_root.empty() || !seg_root.empty()) {
	if(param_root.empty()) {
	    msg.assign("connect: missing 'param_root' argument.");
	    return ARGUMENT_ERROR;
	}
	if(seg_root.empty()) {
	    msg.assign("connect: missing 'seg_root' argument.");
	    return ARGUMENT_ERROR;
	}
	if( (ret = open_db->parseCmd("data_source=Flat-File-DB", msg))
		!= COMMAND_PARSED)
	{
	    return ret;
	}
	s.assign(string("parameter_root=") + param_root);
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) return ret;

	s.assign(string("segment_root=") + seg_root);
	if( (ret = open_db->parseCmd(s, msg)) != COMMAND_PARSED) return ret;

	if( (ret = parseCmd("connect", msg)) != COMMAND_PARSED) return ret;
	return COMMAND_PARSED;
    }
    msg.assign("connect: missing arguments.");
    return ARGUMENT_ERROR;
}

ParseCmd TableQuery::parseWaveformConstraint(const string &cmd)
{
    string c, s, msg;

    if(parseGetArg(cmd, "sta", c)) {
	sta_list->selectItem(c);
    }
    if(parseGetArg(cmd, "chan", c)) {
	chan_list->selectItem(c);
    }
    if(parseGetArg(cmd, "ctype", c)) {
	ctype_list->selectItem(c);
    }
    if(parseGetArg(cmd, "date", c)) {
	jdate_list->selectItem(c);
    }
    if(parseGetArg(cmd, "start_phase", c)) {
	start_phase_list->selectItem(c);
	parse_start_phase.assign(c);
    }
    if(parseGetArg(cmd, "end_phase", c)) {
	end_phase_list->selectItem(c);
	parse_end_phase.assign(c);
    }
    if(parseGetArg(cmd, "start_time", c)) {
	global_start_time->setString(c);
    }
    if(parseGetArg(cmd, "end_time", c)) {
	global_end_time->setString(c);
    }
    if(parseGetArg(cmd, "azimuth_min", c)) {
	azimuth_min->setString(c);
    }
    if(parseGetArg(cmd, "azimuth_max", c)) {
	azimuth_max->setString(c);
    }
    if(parseGetArg(cmd, "distance_min", c)) {
	distance_min->setString(c);
    }
    if(parseGetArg(cmd, "distance_max", c)) {
	distance_max->setString(c);
    }
    if(parseGetArg(cmd, "time_before_start_phase", c) ||
	parseGetArg(cmd, "start_phase_lead", c))
    {
	time_before_phase->setString(c);
	s.assign(string("arrival.time_before=") +  c);
	open_db->parseCmd(s, msg);
    }
    if(parseGetArg(cmd, "time_after_end_phase", c) ||
	parseGetArg(cmd, "end_phase_lag", c))
    {
	time_after_phase->setString(c);
	s.assign(string("arrival.time_after=") + c);
	open_db->parseCmd(s, msg);
    }
    if(parseCompare(cmd, "reset_all")) {
	resetAll();
    }
    return COMMAND_PARSED;
}

void TableQuery::parseHelp(const char *prefix)
{
    printf("%squery\n", prefix);
    printf("%squery=CSSTABLE QUERY_STRING\n", prefix);
    printf("%sget_all_tables\n", prefix);
    printf("%sget CSSTABLE\n", prefix);
    printf("%sget_arrivals_assocs_origerrs or get aao\n", prefix);
    printf("%sget_arrivals_assocs_origerrs_wfdiscs or get aaow\n", prefix);
    printf("%sview NUM\n", prefix);
    printf("%sclear\n", prefix);
    printf("%sdisplay_arrivals\n", prefix);
    printf("%sdisplay_waveforms\n", prefix);
    printf("%sreview_origins\n", prefix);
    printf("%sselect_tab TABNAME\n", prefix);
    printf("%sconnection.help\n", prefix);
    printf("%swaveform_display_constraints.sta=STA\n", prefix);
    printf("%swaveform_display_constraints.chan=CHAN\n", prefix);
    printf("%swaveform_display_constraints.ctype=TYPE\n", prefix);
    printf("%swaveform_display_constraints.date=DATE\n", prefix);
    printf("%swaveform_display_constraints.start_phase=PHASE\n", prefix);
    printf("%swaveform_display_constraints.end_phase=PHASE\n", prefix);
    printf("%swaveform_display_constraints.start_time=TIME\n", prefix);
    printf("%swaveform_display_constraints.end_time=TIME\n", prefix);
    printf("%swaveform_display_constraints.azimuth_min=DEGREES\n", prefix);
    printf("%swaveform_display_constraints.azimuth_max=DEGREES\n", prefix);
    printf("%swaveform_display_constraints.distance_min=DEGREES\n", prefix);
    printf("%swaveform_display_constraints.distance_max=DEGREES\n", prefix);
    printf("%swaveform_display_constraints.time_before_start_phase=SECONDS\n",
		prefix);
    printf("%swaveform_display_constraints.time_after_end_phase=SECONDS\n",
		prefix);
    printf("%swaveform_display_constraints.reset_all\n", prefix);
    TableViewer::parseHelp(prefix);
}

string TableQuery::getTopTabName(void)
{
    return getTabName();
}

void TableQuery::selectTableRow(const string &tableName)
{
    if( tableName.empty() ) return;

    CSSTable *table = tableOnTop();
    bool set = (table->numSelectedRows() > 0) ? true : false;
    if(open_db->sourceType() == NO_CONNECTION) set = false;
    for(int i = 0; i < (int)table_ids.size(); i++) {
	if(!tableName.compare(table_ids[i].table_name)) {
	    for(int j = 0; j < (int)table_ids.size(); j++)
		if(table_ids[i].table_name.compare(table_ids[j].table_name))
	    {
		if(!table_ids[i].id_name.compare(table_ids[j].id_name)) {
		    table_ids[j].button->setSensitive(set);
		}
	    }
	}
    }
    bool odbc = (open_db->sourceType() == ODBC_CONNECTION);

    if( !tableName.compare(cssArrival) || !tableName.compare(cssOrigin) )
    {
	if(!waveform_receiver) {
	    display_arrivals->setSensitive(false);
	    display_waveforms->setSensitive(false);
	}
	else if(odbc) {
	    display_arrivals->setSensitive(set);
	    display_waveforms->setSensitive(set);
	}
	if(odbc) {
	    if(!tableName.compare(cssOrigin)) {
		if(!waveform_receiver) display_waveforms->setSensitive(false);
		else display_waveforms->setSensitive(set);
		if(!waveform_receiver || !waveform_subclass) {
		    review_origins->setSensitive(false);
		}
		else review_origins->setSensitive(set);
		get_aao->setSensitive(set);
		get_aawo->setSensitive(set);
	    }
	    if(!tableName.compare(cssArrival)) {
		get_wfdiscs->setSensitive(set);
	    }
	}
	else {
	    display_arrivals->setSensitive(false);
	    display_waveforms->setSensitive(false);
	    review_origins->setSensitive(false);
	    get_aao->setSensitive(set);
	    get_aawo->setSensitive(set);
	}
    }
    if( !tableName.compare(cssWfdisc) )
    {
	if(!waveform_receiver) display_waveforms->setSensitive(false);
	else display_waveforms->setSensitive(set);
    }
    if( !tableName.compare(cssOrigin) ) {
	for(int i = 0; i < query_views->num_views; i++) {
	    query_views->views[i].b->setSensitive(set);
//	    view_menu->setSensitive(set);
	}
    }
}

void TableQuery::open(void)
{
    char *file;

    if(fileDialog == NULL) {
	fileDialog = new FileDialog("Open file", this, EXISTING_FILE, ".",
				(char *)"*");
    }
    else {
	fileDialog->setVisible(true);
    }
    if((file = fileDialog->getFile()) != NULL)
    {
//	readFile(file);
	XtFree(file);
    }
}

CSSTable *TableQuery::addTableTab(const string &tableName)
{
    return addTableTab(tableName, true);
}

CSSTable *TableQuery::addTableTab(const string &tableName,bool display_textarea)
{
    if(tableName.compare(cssWfdisc)) {
	return TableViewer::addTableTab(tableName, display_textarea);
    }

    Form *topform, *form, *fm;
    ScrolledWindow *sw;
    Separator *sep;
    CSSTable *table;
    int n;
    Arg args[20];
    char name[200];

    if(num_tabs == MAX_TABS) {
	cerr << "addTable: too many tabs." << endl;
	return NULL;
    }

    n = 0;
    XtSetArg(args[n], XtNtitle, tableName.c_str()); n++;
    topform =  new Form(tableName, tab, args, n, false);

    tab_forms[num_tabs].tab_form = topform;

    form = makeWfdiscTab(topform);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    fm = new Form("fm", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    Form *fm2 = new Form("fm2", fm, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_UP); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    tab_forms[num_tabs].up_arrow =
			new ArrowButton("up_arrow", fm2, args, n, this);
    tab_forms[num_tabs].up_arrow->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_DOWN); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    tab_forms[num_tabs].dn_arrow =
			new ArrowButton("dn_arrow", fm2, args, n, this);
    tab_forms[num_tabs].dn_arrow->setSensitive(false);


    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, fm2->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", fm, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    XtSetArg(args[n], XmNscrollHorizontal, False); n++;
    XtSetArg(args[n], XmNscrollVertical, True); n++;

    tab_forms[num_tabs].query_text = new TextField("queryText", sw, args, n);
    tab_forms[num_tabs].query_text->addActionListener(this,XmNactivateCallback);
    tab_forms[num_tabs].query_text->setActivationKeys(";");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, fm->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep =  new Separator("sep", form, args, n);

    snprintf(name, sizeof(name), "%s table", tableName.c_str());
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNdoInternalSelectRowCB, True); n++;
    table = new CSSTable(tableName, form, info_area, args, n);

    const char *extra[] = {"display start", "display end"};
    const char *extra_formats[] = {"%T", "%T"};
/*
    CssClassDescription *des;
    int num_members = CssTableClass::getDescription(cssWfdisc, &des);
    char display_list[2000];
    snprintf(display_list, sizeof(display_list), "display start,display end");
    n = (int)strlen(display_list);
    for(int i = 0; i < num_members; i++) {
	snprintf(display_list+n, sizeof(display_list)-n, ",%s", des[i].name);
	n = (int)strlen(display_list);
    }
    table->setType(cssWfdisc, 2, extra,extra_formats,(const char *)display_list);
*/
    table->setType(cssWfdisc, 2, extra, extra_formats);

    table->setVisible(true);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNselectColumnCallback);

    tab_forms[num_tabs].history = new vector<string *>;
    tab_forms[num_tabs].history_index = 0;
    for(int i = 0; i < MAX_TABS; i++) {
	string prop;
	snprintf(name, sizeof(name), "query.%s.%d", tableName.c_str(), i);
	if( getProperty(name, prop) ) {
	    tab_forms[num_tabs].history->push_back(new string(prop));
	}
	else {
	    break;
	}
    }
    if((int)tab_forms[num_tabs].history->size() > 0) {
	tab_forms[num_tabs].up_arrow->setSensitive(true);
	tab_forms[num_tabs].history_index = -1;
    }

    tab_forms[num_tabs++].table = table;
    tab->update();

    return table;
}

Form * TableQuery::makeWfdiscTab(Form *topform)
{
    int n;
    Arg args[10];
    RowColumn *form4, *rcm;
    Form *rcw;
    ScrolledWindow *sw;
    Label *label;
    const char *phase_list[] = {"FirstP", "FirstS", "P", "PP", "S", "SS", "PcP",
			"pP", "sP", "ScS", "T", "LR", "FirstObs",};
    int num_phases = 13;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    wt.rcbot = new RowColumn("Before rc", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Time Before Start Phase", wt.rcbot, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    time_before_phase = new TextField("BeforeStartPhase", wt.rcbot, args, n);
    time_before_phase->setString("60.");
    time_before_phase->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
    XtSetArg(args[n], XmNwidth, 10); n++;
    new Separator("sepV1", wt.rcbot, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Time After End Phase", wt.rcbot, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    time_after_phase = new TextField("AfterEndPhase", wt.rcbot, args, n);
    time_after_phase->setString("240.");
    time_after_phase->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNwidth, 10); n++;
    new Separator("sepVV", wt.rcbot, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Button("Reset All", wt.rcbot, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.rcbot->baseWidget()); n++;
    wt.sep4 = new Separator("sep4", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.sep4->baseWidget()); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    wt.rcmid = new RowColumn("Azimuth rc", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Azimuth", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Min", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    azimuth_min = new TextField("AzimuthMin", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Max", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    azimuth_max = new TextField("AzimuthMax", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
    XtSetArg(args[n], XmNwidth, 20); n++;
    new Separator("sepV2", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Distance(deg)", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Min", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    distance_min = new TextField("DistanceMin", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Max", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    distance_max = new TextField("DistanceMax", wt.rcmid, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.rcmid->baseWidget()); n++;
    wt.sep3 = new Separator("sep3", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.sep3->baseWidget()); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    wt.rctop = new RowColumn("Time rc", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("Start Time", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 12); n++;
    global_start_time = new TextField("globalStartTime", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
    XtSetArg(args[n], XmNwidth, 3); n++;
    new Separator("sepV3", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    new Label("End Time", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNcolumns, 12); n++;
    global_end_time = new TextField("GlobalEndTime", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
    XtSetArg(args[n], XmNwidth, 3); n++;
    new Separator("sepV4", wt.rctop, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.rctop->baseWidget()); n++;
    wt.sep2 =  new Separator("sep2", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNresizeHeight, True); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNspacing, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.sep2->baseWidget()); n++;
    wt.form2 =  new RowColumn("form2", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNresizeHeight, True); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNspacing, 10); n++;
    form4 =  new RowColumn("form4", wt.form2, args, n);

    rcw = new Form("rc1", form4);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    label = new Label("         Sta         ", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    sta_sw = new ScrolledWindow("staSW", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    sta_list = new List("staList", sta_sw, args, n, this);

    rcw = new Form("rc2", form4);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    label = new Label("        Chan         ", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    chan_sw = new ScrolledWindow("chanSW", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    chan_list = new List("chanList", chan_sw, args, n, this);

    rcw = new Form("rc3", form4);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    label = new Label(" ctype ", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    ctype_sw = new ScrolledWindow("ctypeSW", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    ctype_list = new List("ctypeList", ctype_sw, args, n, this);

    rcw = new Form("rc4", form4);
    label = new Label("      Date      ", rcw);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    jdate_sw = new ScrolledWindow("dateSW", rcw, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    jdate_list = new List("dateList", jdate_sw, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNresizeHeight, True); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    rcm =  new RowColumn("rc5", form4);

    new Label("Start Phase", rcm);
    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("StartPhaseSW", rcm, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    start_phase_list = new List("StartPhaseList", sw, args, n, this);
    start_phase_list->addItems(phase_list, num_phases);

    n = 0;
    XtSetArg(args[n], XmNresizeHeight, True); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    rcm =  new RowColumn("rc6", form4, args, n);

    new Label("End Phase", rcm);

    n = 0;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("EndPhaseSW", rcm, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    end_phase_list = new List("EndPhaseList", sw, args, n, this);
    end_phase_list->addItems(phase_list, num_phases);

    n = 0;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_DOWN); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.form2->baseWidget()); n++;
    wt.arrow =  new ArrowButton("arrow", topform, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, wt.arrow->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.form2->baseWidget()); n++;
    wt.label = new Label("Waveform Display Constraints", topform, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, wt.arrow->baseWidget()); n++;
    Form *form =  new Form("form", topform, args, n);

    return form;
}

void TableQuery::tabSelect(const string &tab_name)
{
    int i, j;

    if(tab_name.empty()) return;

    bool set = (numSelectedRows(tab_name) > 0) ? true : false;
    if(open_db->sourceType() == NO_CONNECTION) set = false;

    if(!tab_name.compare(cssOrigin) || !tab_name.compare(cssWfdisc)) {
	option_sep3->setVisible(true);
	views_menu->setVisible(true);
	bool origin_selected = (numSelectedRows(cssOrigin) > 0) ? true : false;
	if(open_db->sourceType() == NO_CONNECTION) origin_selected = false;
	for(i = 0; i < query_views->num_views; i++) {
	    query_views->views[i].b->setSensitive(origin_selected);
	}
    }
    else {
	option_sep3->setVisible(false);
	views_menu->setVisible(false);
    }

    for(i = 0; i < (int)table_ids.size(); i++) {
	table_ids[i].button->setVisible(false);
    }
	
    for(i = 0; i < (int)table_ids.size(); i++)
    {
	if(!tab_name.compare(table_ids[i].table_name))
	{
	    for(j = 0; j < (int)table_ids.size(); j++)
		if(table_ids[i].table_name.compare(table_ids[j].table_name))
	    {
		if(!table_ids[i].id_name.compare(table_ids[j].id_name)) {
		    table_ids[j].button->setVisible(true);
		    table_ids[j].button->setSensitive(set);
		}
	    }
	}
    }

    bool odbc = (open_db->sourceType() == ODBC_CONNECTION);

    /* special cases
     */
    if(!tab_name.compare(cssArrival) || !tab_name.compare(cssOrigin)) {
	display_waveforms->setVisible(odbc);
    }
    else if(!tab_name.compare(cssWfdisc)) {
	display_waveforms->setVisible(true);
    }
    else {
	display_waveforms->setVisible(false);
	display_waveforms->setSensitive(false);
    }
    if(!tab_name.compare(cssArrival)) {
	get_wfdiscs->setVisible(true);
	get_wfdiscs->setSensitive(set);
    }
    if(!tab_name.compare(cssOrigin)) {
	get_aao->setVisible(true);
	get_aawo->setVisible(true);
	review_origins->setVisible(odbc);
	display_waveforms->setVisible(odbc);
	if(!waveform_receiver || !waveform_subclass) {
	    review_origins->setSensitive(false);
	}
	else if(odbc) {
	    review_origins->setSensitive(set);
	}
	else {
	    review_origins->setSensitive(false);
	}
	
	if(!waveform_receiver) display_waveforms->setSensitive(false);
	else if(odbc) {
	    display_waveforms->setSensitive(set);
	}
	else {
	    display_waveforms->setSensitive(false);
	}
	get_aao->setSensitive(set);
	get_aawo->setSensitive(set);
    }
    else {
	get_aao->setVisible(false);
	get_aawo->setVisible(false);
	review_origins->setVisible(false);
	review_origins->setSensitive(false);
    }
    if(odbc && (!tab_name.compare(cssArrival) || !tab_name.compare(cssOrigin)))
    {
	option_sep2->setVisible(true);
//	this doesn't work
//	Component *c = option_menu->findComponent("Locate Event...");
//	if(c) c->setVisible(true);
	display_arrivals->setVisible(true);
	if(!waveform_receiver) {
	    display_arrivals->setSensitive(false);
	    display_waveforms->setSensitive(false);
	}
	else {
	    display_arrivals->setSensitive(set);
	    display_waveforms->setSensitive(set);
	}
    }
    else {
	option_sep2->setVisible(false);
//	Component *c = option_menu->findComponent("Locate Event...");
//	if(c) c->setVisible(false);
	display_arrivals->setSensitive(false);
	display_arrivals->setVisible(false);
    }

    if(!tab_name.compare(cssWfdisc)) {
	if(!waveform_receiver) {
	    display_waveforms->setSensitive(false);
	}
	else {
	    display_waveforms->setSensitive(set);
	}
    }

    set = tab_name.compare("dynamic") ? true : false;

    delete_button->setSensitive(set);
    edit_button->setSensitive(set);

    setButtonsSensitive();
}

void TableQuery::resetAll(void)
{
    sta_list->deselectAll();
    chan_list->deselectAll();
    ctype_list->deselectAll();
    jdate_list->deselectAll();
    start_phase_list->deselectAll();
    end_phase_list->deselectAll();

    time_before_phase->setString("");
    time_before_phase->setSensitive(false);
    time_after_phase->setString("");
    time_after_phase->setSensitive(false);

    global_start_time->setString("");
    global_end_time->setString("");
    azimuth_min->setString("");
    azimuth_max->setString("");
    distance_min->setString("");
    distance_max->setString("");

    parse_start_phase.clear();
    parse_end_phase.clear();
}

void TableQuery::selectConstraint(void)
{
    int i, j, nsta, nchan, njd, nctype, nrows;
//    int *sta_q=NULL, *chan_q=NULL, *ctype_q=NULL;
    vector<string> sta_v, chan_v, ctype_v, jdate_v;
    long *jdate=NULL;
//    char **items=NULL;
    CSSTable *table;
    bool proto;
    vector<bool> states;

    if((table = getCSSTable(cssWfdisc)) == NULL) {
	cerr << "selectConstraintCB: cannot find wfdisc tab." << endl;
	return;
    }

    gvector<CssTableClass *> *v = getTableRecords(cssWfdisc, true);
    if(v && v->size() == 0) return;

    table->deselectAllRows();

    nrows = table->getRowStates(states);
    if(nrows != v->size()) {
	showWarning("selectConstraintCB error: nrows != num_wfs");
	return;
    }

    for(j = 0; j < v->size(); j++) {
	v->at(j)->setSelected(false);
    }

    proto = false;

    nsta = nchan = njd = nctype = 0;

    nsta = sta_list->getSelectedItems(sta_v);
    nchan = chan_list->getSelectedItems(chan_v);
    njd = jdate_list->getSelectedItems(jdate_v);
    nctype = ctype_list->getSelectedItems(ctype_v);

    jdate = new long[njd];
    for(i = 0; i < njd; i++) {
	timeParseJDate(jdate_v[i].c_str(), &jdate[i]);
    }

    if(nsta > 0 || nchan > 0 || njd > 0 || nctype > 0)
    {
	char prev_sta[9];
	bool have_one = false;
	int sindex, cindex, priority;
	int kk = 0;
	if(nctype > 0)
	{
	    if( !ctype_v[0].compare("proto") )
	    {
		proto = true;
		kk++;
	    }	
	}
 
	strcpy(prev_sta, "-");
	for(i = 0; i < v->size(); i++)
	{
	    CssWfdiscClass *wf = (CssWfdiscClass *)v->at(i);
	    for(j = 0; j < nsta; j++) {
		if(!sta_v[j].compare(wf->sta)) break;
	    }
	    if(nsta > 0 && j == nsta) {
		wf->setSelected(false);
		continue;
	    }	

	    for(j = 0; j < nchan; j++) {
		if(!chan_v[j].compare(wf->chan)) break;
	    }
	    if(nchan > 0 && j == nchan) {
		wf->setSelected(false);
		continue;
	    }	

	    for(j = 0; j < njd; j++) {
		if(wf->jdate == jdate[j]) break;
	    }
	    if(njd > 0 && j == njd) {
		wf->setSelected(false);
		continue;
	    }	

	    getIndex(wf->sta, wf->chan, &sindex, &cindex, &priority);

	    /* only break out when we find a match */
	    for(j = kk; j < nctype; j++) {
		if(checkCtype(ctype_v[j].c_str(), priority)) break;
	    }
	    /* works when no proto is used */
	    if(nctype > 0 && j == nctype && kk == 0) {
		wf->setSelected(false);
		continue;
	    }	

	    if(kk == 1 && j == nctype && j > 1) {
		wf->setSelected(false);
		continue;
	    }	

	    wf->setSelected(true);
	    have_one = true;
	}

	if(proto)
	{
	    initStaPresent(false);
		 
	    for(i = 0; i < v->size(); i++)
		if(!have_one || (have_one && v->at(i)->getSelected()))
	    {
		CssWfdiscClass *wf = (CssWfdiscClass *)v->at(i);
		getIndex(wf->sta, wf->chan, &sindex, &cindex, &priority);
		setStaPresent(sindex, cindex);
	    }
		 
	    cleanStaPresent();
 
	    for(i = 0; i < v->size(); i++)
	    {
		if(!have_one || (have_one && v->at(i)->getSelected()))
		{
		    CssWfdiscClass *wf = (CssWfdiscClass *)v->at(i);
		    getIndex(wf->sta, wf->chan, &sindex, &cindex, &priority);
		 
		    wf->setSelected(checkStaPresent(sindex, cindex));
		}
	    }
	}
			
	states.clear();
	for(i = 0; i < v->size(); i++) {
	    states.push_back(v->at(i)->getSelected());
	}
    }
    delete [] jdate;

    table->setRowStates(states);
    selectTableRow(cssWfdisc);
}

void TableQuery::arrowSelect(void)
{
    Arg args[3];

    if(wt.form2->isVisible())
    {
	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_FORM);
	XtSetArg(args[1], XmNarrowDirection, XmARROW_UP);
	wt.arrow->setValues(args, 2);

	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_FORM);
	wt.label->setValues(args, 1);

	wt.form2->setVisible(false);
	wt.sep2->setVisible(false);
	wt.rctop->setVisible(false);
	wt.sep3->setVisible(false);
	wt.rcmid->setVisible(false);
	wt.sep4->setVisible(false);
	wt.rcbot->setVisible(false);
    }
    else {
	wt.arrow->setVisible(false);
	wt.label->setVisible(false);
	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_WIDGET);
	XtSetArg(args[1], XmNbottomWidget, wt.form2->baseWidget());
	XtSetArg(args[2], XmNarrowDirection, XmARROW_DOWN);
	wt.arrow->setValues(args, 3);
	XtSetArg(args[0], XmNbottomAttachment, XmATTACH_WIDGET);
	XtSetArg(args[1], XmNbottomWidget, wt.form2->baseWidget());
	wt.label->setValues(args, 2);
	wt.rcbot->setVisible(true);
	wt.sep4->setVisible(true);
	wt.rcmid->setVisible(true);
	wt.sep3->setVisible(true);
	wt.rctop->setVisible(true);
	wt.sep2->setVisible(true);
	wt.form2->setVisible(true);
	wt.arrow->setVisible(true);
	wt.label->setVisible(true);
    }
}

void TableQuery::wfdiscConstraintsInit(void)
{
    int		i, num;
    vector<int>	original_order, row_order;
    string	priority_file;
    char	buf[20];
    char 	**list = NULL;
    int		sindex, cindex, priority;
    int		array_sp, _3c_sp, _3c_bb;
    CSSTable	*wfdisc_table;
    CssWfdiscClass	*wf;

    if(!(wfdisc_table = getCSSTable(cssWfdisc))) return;
    gvector<CssTableClass *> *v = getTableRecords(cssWfdisc, true);
    if(!v || v->size() == 0) return;

    wfdisc_table->getRowOrder(original_order);

    sta_list->deleteAll();
    chan_list->deleteAll();
    ctype_list->deleteAll();
    jdate_list->deleteAll();

    const char **column_labels = wfdisc_table->getColumnLabels();

    num = wfdisc_table->numRows();
    for(i = 0; i < num && strcasecmp(column_labels[i], "sta"); i++);

    if(i < num)
    {
	/* find distinct sta */
	vector<int> vec;
	vec.push_back(i);
	wfdisc_table->sortUnique(vec);
	num = wfdisc_table->getRowOrder(row_order);
	if(num > 0) {
	    sta_sw->setVisible(false);
	    list = (char **)malloc(num*sizeof(char *));
	    for(i = 0; i < num; i++) {
		wf = (CssWfdiscClass *)v->at(row_order[i]);
		list[i] = wf->sta;
	    }
	    sta_list->addItems(list, num);
	    free(list);
	    sta_sw->setVisible(true);
	    
	    wfdisc_table->showAll();
	    wfdisc_table->sortByColumnLabels("wfid");
	}
    }

    num = wfdisc_table->numRows();
    for(i = 0; i < num && strcasecmp(column_labels[i], "chan"); i++);

    if(i < num)
    {
	/* find distinct chan */
	vector<int> vec;
	vec.push_back(i);
	wfdisc_table->sortUnique(vec);
	num = wfdisc_table->getRowOrder(row_order);
	if(num > 0) {
	    chan_sw->setVisible(false);
	    list = (char **)malloc(num*sizeof(char *));
	    for(i = 0; i < num; i++) {
		wf = (CssWfdiscClass *)v->at(row_order[i]);
		list[i] = wf->chan;
	    }
	    chan_list->addItems(list, num);
	    free(list);
	    chan_sw->setVisible(true);
	    
	    wfdisc_table->showAll();
	    wfdisc_table->sortByColumnLabels("wfid");
	}
    }

    num = wfdisc_table->numRows();
    for(i = 0; i < num && strcasecmp(column_labels[i], "jdate"); i++);
    Free(column_labels);

    if(i < num)
    {
	/* find distinct jdate */
	vector<int> vec;
	vec.push_back(i);
	wfdisc_table->sortUnique(vec);
	num = wfdisc_table->getRowOrder(row_order);
	if(num > 0) {
	    jdate_sw->setVisible(false);
	    list= (char **)malloc(num*sizeof(char *));
	    for(i = 0; i < num; i++) {
		wf = (CssWfdiscClass *)v->at(row_order[i]);
		timeEpochToString(wf->time, buf, 20, YMOND);
		list[i] = strdup(buf);
	    }
	    jdate_list->addItems(list, num);
	    for(i = 0; i < num; i++) free(list[i]);
	    free(list);
	    jdate_sw->setVisible(true);
	    
	    wfdisc_table->showAll();
	    wfdisc_table->sortByColumnLabels("wfid");
	}
    }
    wfdisc_table->setRowOrder(original_order);

    list = (char **)malloc(4*sizeof(char *));

    /* do the channel types */
    num = 0;
    list[num++] = strdup("proto");

    if( !getProperty("priority_file", priority_file) ) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME"))) {
	    readPriorityFile(c+string("/tables/static/global.priority"), false);
	}
    }
    else {
	readPriorityFile(priority_file, true);
    }

    array_sp = _3c_sp = _3c_bb = false;

    for(i = 0; i < v->size(); i++)
    {
	wf = (CssWfdiscClass *)v->at(i);
	getIndex(wf->sta, wf->chan, &sindex, &cindex, &priority);

	if(!array_sp && priority > 0 && priority < 10) {
		array_sp = true;
	}
	if(!_3c_bb && priority > 9 && priority < 15) {
		_3c_bb = true;
	}
	else if(!_3c_sp && (priority==1 || (priority>14 && priority<20))) {
		_3c_sp = true;
	}
	if(array_sp && _3c_bb && _3c_sp) break;
    }

    if(array_sp) {
        list[num++] = strdup("array-sp");
    }

    if(_3c_bb) {
        list[num++] = strdup("3c-bb");
    }

    if(_3c_sp) {
	list[num++] = strdup("3c-sp");
    }

    ctype_list->addItems(list, num);
    for(i = 0; i < num; i++) free(list[i]);
    free(list);
}

typedef struct
{
	char	refsta[7];
	char	sta[7];
	char	chan[9];
	int	priority;
	int	present;
} Element;

typedef struct
{
	int	nelements;
	int	present;
	Element *e;
} Sta;

static 	Sta	*sta_elements = NULL;
static 	int	nsta_elements = 0;

#ifndef True
#define	True	1
#endif
#ifndef False
#define	False	0
#endif

int TableQuery::checkCtype(const string &type, int priority)
{
    int *priorities = NULL, status, np;

    np = 0;
    typeToPriorities(type, &priorities, &np);

    status = false;
    for(int i = 0; i < np; i++) {
	if (priority == priorities[i]) {
	    status = true;
	    break;
	}
    }
    if (np > 0) Free(priorities);
    return(status);
}

void TableQuery::typeToPriorities(const string &type, int **priorities, int *np)
{
   int *p = NULL;

    if (!type.compare("array-sp"))
    {
	p = (int *)mallocWarn(9*sizeof(int));
	for(int i = 0; i < 9; i++) p[i] = i+1;
	*np = 9;
    }
    else if (!type.compare("3c-sp"))
    {
	p = (int *)mallocWarn(4*sizeof(int));
	p[0] = 1;
	p[1] = 15;
	p[2] = 16;
	p[3] = 17;
	*np = 4;
    }
    else if (!type.compare("3c-bb"))
    {
	p = (int *)mallocWarn(3*sizeof(int));
	p[0] = 10;
	p[1] = 11;
	p[2] = 12;
	*np = 3;
    }
    else {
	*np = 0;
    }
    *priorities = p;
}

void TableQuery::initStaPresent(int present)
{
    for(int i = 0; i < nsta_elements; i++) {
	for(int j = 0; j < sta_elements[i].nelements; j++) {
	    sta_elements[i].e[j].present = present;
	}
    }
}

void TableQuery::setStaPresent(int sindex, int cindex)
{
    if(sindex >= 0 && sindex < nsta_elements) {
	sta_elements[sindex].present = true;
	sta_elements[sindex].e[cindex].present = true;
    }
}

void TableQuery::cleanStaPresent(void)
{
    for(int i = 0; i < nsta_elements; i++) if (sta_elements[i].present)
    {
	bool full = false;
	for(int j = 0; j < sta_elements[i].nelements; j++)
		if (sta_elements[i].e[j].present)
	{
	    if (!full) {
		full = true;
	    }
	    else if (full) {
		sta_elements[i].e[j].present = false;
	    }
	}
    }
}

int TableQuery::checkStaPresent(int sindex, int cindex)
{
    if(sindex >= 0 && sindex < nsta_elements) {
	return(sta_elements[sindex].e[cindex].present);
    }
    else {
	return(0);
    }
}

int TableQuery::readPriorityFile(const string &priority_file, bool warn)
{
    FILE *fp2;
    char junk[12], _refsta[7], _sta[7];
    int i, j, nele, nelements;
    int *elements;
    char prev_sta[9];

    if(nsta_elements > 0) return(nsta_elements);

    if((fp2 = fopen(priority_file.c_str(), "r")) == NULL)   {
	if(warn) {
	    fprintf(stderr, "Cannot open priority_file '%s' for read\n", 
			priority_file.c_str());
	}
	return(-1);
    }
    else
    {
	/* go through and find how many stations, and how
	 * many elements at each station
	 */

	/* nele in this array
	 * nelements number of members in elements
	 */

	 if (fscanf(fp2, "%s %s %s %d", _refsta, _sta, junk, &j) == 4)
	 {
	    nele = 1;
	    nelements = 120;
	    stringcpy(prev_sta, _refsta, sizeof(prev_sta));

	    if(!(elements = (int *)mallocWarn(nelements*sizeof(int)))) {
		return(-1);
	    }
	}
	else {
	    nele = 0;
	    fprintf(stderr, "Cannot read '%s' \n", priority_file.c_str());
	    return(-1);
	}

	nsta_elements = 0;
	while (fscanf(fp2, "%s %s %s %d", _refsta, _sta, junk, &j) == 4)
	{
	    if (!strcmp(prev_sta, _refsta))
	    {
		nele++;
	    }
	    else
	    {
		if (nsta_elements >= nelements)
		{
		    nelements += 100;
		    elements = (int *)reallocWarn(elements,
					nelements*sizeof(int));
		}
		elements[nsta_elements] = nele;
		nele = 1;
		nsta_elements++;
		stringcpy(prev_sta, _refsta, sizeof(prev_sta));
	    }
	}
	if (nele > 0)
	{
	    if (nsta_elements >= nelements)
	    {
		nelements += 1;
		elements = (int *)reallocWarn(elements, nelements*sizeof(int));
	    }
	    elements[nsta_elements] = nele;
	    nele = 0;
	    nsta_elements++;
	}

	/* now fill up the sta structure */
	if(!(sta_elements = (Sta *)mallocWarn(nsta_elements*sizeof(Sta))))
	{
	    return(-1);
	}
	for (i=0; i<nsta_elements; i++)
	{
	    sta_elements[i].nelements = elements[i];
	    sta_elements[i].present = 0;
	    sta_elements[i].e = (Element *)mallocWarn(elements[i]*sizeof(Element));
	}
	fseek(fp2, 0, 0);
	for (i=0; i<nsta_elements; i++)
	    for (j=0; j<elements[i]; j++)
	{
	    fscanf(fp2, "%s %s %s %d", 
		sta_elements[i].e[j].refsta, sta_elements[i].e[j].sta,
		sta_elements[i].e[j].chan, &sta_elements[i].e[j].priority);
	    sta_elements[i].e[j].present = 0;
	}
	fclose(fp2);
	Free(elements);
    }
	
    return(nsta_elements);
}

void TableQuery::getIndex(const string &_sta, const string &_chan, int *idx,
				int *eno,int *priority)
{
    for(int i = 0; i < nsta_elements; i++)
	if (_sta[0] == sta_elements[i].e[0].sta[0])
    {
	for(int j = 0; j < sta_elements[i].nelements; j++)
	{
	    if(!_sta.compare(sta_elements[i].e[j].sta)
		&& !_chan.compare(sta_elements[i].e[j].chan))
	    {
		*idx = i;
                *eno = j;
		*priority = sta_elements[i].e[j].priority;
                return;
	    }
	}
    }

    defaultPriority(_chan, priority);
 
    *idx = -1;
    *eno = -1;
    return;
}

void TableQuery::defaultPriority(const string &chan, int *priority)
{
    int len = (int)chan.length();
    const char *c = chan.c_str();

    *priority = -1;

    if (len == 1)
    {
	if (!strcasecmp(c, "z"))      *priority = 15;
	else if (!strcasecmp(c, "n")) *priority = 16;
	else if (!strcasecmp(c, "e")) *priority = 17;
    }
    else if (len == 2)
    {
	if (!strcasecmp(c, "sz"))      *priority = 15;
	else if (!strcasecmp(c, "sn")) *priority = 16;
	else if (!strcasecmp(c, "se")) *priority = 17;
	else if (!strcasecmp(c, "bz")) *priority = 10;
	else if (!strcasecmp(c, "bn")) *priority = 11;
	else if (!strcasecmp(c, "be")) *priority = 12;
	else if (!strcasecmp(c, "lz")) *priority = 20;
	else if (!strcasecmp(c, "ln")) *priority = 21;
	else if (!strcasecmp(c, "le")) *priority = 22;
	else if (!strcasecmp(c, "mz")) *priority = 35;
	else if (!strcasecmp(c, "mn")) *priority = 36;
	else if (!strcasecmp(c, "me")) *priority = 37;
    }
    else if (len == 3)
    {
	if (!strcasecmp(c, "shz"))      *priority = 15;
	else if (!strcasecmp(c, "shn")) *priority = 16;
	else if (!strcasecmp(c, "she")) *priority = 17;
	else if (!strcasecmp(c, "bhz")) *priority = 10;
	else if (!strcasecmp(c, "bhn")) *priority = 11;
	else if (!strcasecmp(c, "bhe")) *priority = 12;
	else if (!strcasecmp(c, "lhz")) *priority = 20;
	else if (!strcasecmp(c, "lhn")) *priority = 21;
	else if (!strcasecmp(c, "lhe")) *priority = 22;
	else if (!strcasecmp(c, "mhz")) *priority = 35;
	else if (!strcasecmp(c, "mhn")) *priority = 36;
	else if (!strcasecmp(c, "mhe")) *priority = 37;
    }
}

void TableQuery::timeRunQuery(OpenDBStruct *op)
{
    int num;
    const char *labels[5];
    bool query_selected = false;

    if(open_db->sourceType() == NO_CONNECTION) {
	showWarning("Not connected to a data source.");
	return;
    }
    /* Clear all tables that can be refilled by this routine.
     */
    clearTab(cssOrigin);
    clearTab(cssOrigerr);
    clearTab(cssArrival);
    clearTab(cssAssoc);
    clearTab(cssWfdisc);

    num = 0;
    if(op->get_origins) {
	labels[num++] = cssOrigin;
	labels[num++] = cssOrigerr;
    }
    if(op->get_arrivals) {
	labels[num++] = cssArrival;
	labels[num++] = cssAssoc;
    }
    if(op->get_wfdiscs) {
	labels[num++] = cssWfdisc;
    }

    showWorking(num, labels);

    if(op->get_origins && op->origin_query[0] != '\0')
    {
	runQuery(cssOrigin, op->origin_query);
	saveQueryText(cssOrigin, op->origin_query);
	getLinkedTable(cssOrigin, "orid", cssOrigerr);
	query_selected = true;
    }
    if(op->get_arrivals && op->arrival_query[0] != '\0')
    {
	runQuery(cssArrival, op->arrival_query);
	saveQueryText(cssArrival, op->arrival_query);
	getLinkedTable(cssArrival, "arid", cssAssoc);
	query_selected = true;
    }
    if(op->get_wfdiscs && op->wfdisc_query[0] != '\0')
    {
	runQuery(cssWfdisc, op->wfdisc_query, op->tmin, op->tmax);
	saveQueryText(cssWfdisc, op->wfdisc_query);
	query_selected = true;
    }
    if(!query_selected) {
	showWarning("No query selected.");
    }
    closeWorking();
}

void TableQuery::arrivalRunQuery(OpenDBStruct *op)
{
    const char *labels[5];
    int num;

    if(open_db->sourceType() == NO_CONNECTION) {
	showWarning("Not connected to a data source.");
	return;
    }

    /* Clear all tables that can be refilled by this routine.
     */
    clearTab(cssArrival);
    clearTab(cssAssoc);
    clearTab(cssOrigin);
    clearTab(cssOrigerr);
    clearTab(cssWfdisc);

    num = 0;
    labels[num++] = cssArrival;
    labels[num++] = cssAssoc;

    if(op->get_origins) {
	labels[num++] = cssOrigin;
	labels[num++] = cssOrigerr;
    }

    if(op->get_wfdiscs) {
	labels[num++] = cssWfdisc;
    }

    showWorking(num, labels);

    if(op->arrival_query[0] != '\0')
    {
	runQuery(cssArrival, op->arrival_query);
	saveQueryText(cssArrival, op->arrival_query);
    }

    getLinkedTable(cssArrival, "arid", cssAssoc);

    if(op->get_origins) {
	getLinkedTable(cssAssoc, "orid", cssOrigin);
	getLinkedTable(cssOrigin, "orid", cssOrigerr);
    }

    if(op->get_wfdiscs) {
	wfdiscsFromArrival();
    }

    closeWorking();
}

void TableQuery::originRunQuery(OpenDBStruct *op)
{
    const char *labels[6];
    int num;

    if(open_db->sourceType() == NO_CONNECTION) {
	showWarning("Not connected to a data source.");
	return;
    }

    /* Clear all tables that can be refilled by this routine.
     */
    clearTab(cssArrival);
    clearTab(cssAssoc);
    clearTab(cssOrigin);
    clearTab(cssOrigerr);
    clearTab(cssWfdisc);
    clearTab(cssWftag);

    num = 0;
    labels[num++] = cssOrigin;
    labels[num++] = cssOrigerr;

    if(op->get_arrivals) {
	labels[num++] = cssArrival;
	labels[num++] = cssAssoc;
    }
    if(op->get_wfdiscs) {
	labels[num++] = cssWftag;
	labels[num++] = cssWfdisc;
    }

    showWorking(num, labels);

    if(op->origin_query[0] != '\0')
    {
	runQuery(cssOrigin, op->origin_query);
	saveQueryText(cssOrigin, op->origin_query);
    }

    getLinkedTable(cssOrigin, "orid", cssOrigerr);

    if(op->get_arrivals) {
	getLinkedTable(cssOrigin, "orid", cssAssoc);
	getLinkedTable(cssAssoc, "arid", cssArrival);
    }

    if(op->get_wfdiscs)
    {
	if(!strcmp(op->use, "UseWftags")) {
	    getWftagsFromOrigins();
	    getLinkedTable(cssWftag, "wfid", cssWfdisc);
	}
	else if(!strcmp(op->use, "UseArrivals")) {
	    wfdiscsFromArrival();
	}
    }
    closeWorking();
}

void TableQuery::runQuery(const string &tableName, char *query_string,
			bool raise_tab)
{
    runQuery(tableName, query_string, NULL_TIME, NULL_TIME, raise_tab);
    setQueryText(tableName, query_string);
}

void TableQuery::runQuery(const string &tableName, char *query_string,
			double tmin, double tmax, bool raise_tab)
{
    gvector<CssTableClass *> v;
    CSSTable *table = NULL;

    if(!(table = getCSSTable(tableName)))
    {
	table = addTableTab(tableName);
	putTabProperty();
    }

    if(raise_tab) tab->setOnTop(tableName);
    setChange(table);
    table->removeAllRecords();
    tabSelect(tableName);

    setCursor("hourglass");

    runQuery(tableName, query_string, v);

    sortRows(v, tableName);
    
    if(!tableName.compare(cssWfdisc))
    {
	for(int i = 0; i < v.size(); i++) {
	    v[i]->putValue("display start", tmin);
	    v[i]->putValue("display end", tmax);
	}
    }
    table->addRecords(v);
    setChange(v);
    if(!tableName.compare(cssWfdisc)) {
	wfdiscConstraintsInit();
    }
    setButtonsSensitive();

    setCursor("default");

    if(table->numRows() == 0 && no_records_warn) {
	showWarning("No %s rows returned.", tableName.c_str());
    }
}

bool TableQuery::runQuery(const string &tableName, char *query_string,
			gvector<CssTableClass *> &records)
{
    int i, j;
    char c;
    gvector<CssTableClass *> v;

    /* loop over multiple queries separated by ';' */
    for(i = 0; query_string[i] != '\0'; i = j)
    {
	/* find the end of this query */
	for(j = i; query_string[j] != '\0' && query_string[j] != ';'; j++);
	c = query_string[j];
	query_string[j] = '\0';

	if(open_db->sourceType() == ODBC_CONNECTION)
	{
#ifdef HAVE_LIBODBC
	    if(!tableName.compare("dynamic")) {
		if(ODBCQuery(open_db->getHDBC(), query_string+i, records)) {
		    query_string[j] = c;
		    return false;
		}
	    }
	    else if(!runOdbcQuery(records, query_string+i, tableName)) {
		query_string[j] = c;
		return false;
	    }
#endif
	}
	else if(open_db->sourceType() == FFDB_CONNECTION) {
	    if(!tableName.compare("dynamic")) {
		/* no dynamic query routine for flat-file-db yet */
	    }
	    else if(!runFlatQuery(records, query_string+i, tableName)) {
		showWarning("Cannot interpret query.");
		query_string[j] = c;
		return false;
	    }
	}
	else if(open_db->sourceType() == PREFIX_CONNECTION) {
	    // queryPrefix removes all elements from the input vector
	    // instead of appending elements, so input v and then append to
	    // records.
	    if(!tableName.compare("dynamic")) {
		/* no dynamic query routine for file_prefix yet */
	    }
	    else if( !open_db->getFFDB()->queryPrefix(query_string+i,
				tableName, &v) )
	    {
		showWarning("Cannot interpret query.\n%s",
				FFDatabase::FFDBErrMsg());
		query_string[j] = c;
		return false;
	    }
	    for(int k = 0; k < v.size(); k++) {
		records.push_back(v[k]);
	    }
	    v.clear();
	}
	else {
	    showWarning("Unknown connection type.");
	    return false;
	}
	query_string[j] = c;
	if(c == ';') j++;
    }

    for(i = 0; i < records.size(); i++) {
	records[i]->setDataSource(this);
    }
    return true;
}

#ifdef HAVE_LIBODBC
bool TableQuery::runOdbcQuery(gvector<CssTableClass *> &records, char *query_string,
			const string &tableName)
{
    ODBC_QueryTableStruct *qs;

    if(!updateWorking(tableName, records.size())) {
	return true;
    }

    if(ODBCQueryTableInit(open_db->getHDBC(), query_string, tableName, &qs)) {
	showWarning("query: \"%s\" failed.\n%s\nPerhaps table does not exist.",
		query_string, ODBCErrMsg());
	return false;
    }

    Free(current_query);
    current_query = strdup(query_string);
    int i0 = records.size();
    int num, ret;

    while((ret = ODBC_queryTableResults(qs, 400, &num, records)) > 0)
    {
	if(!updateWorking(tableName, records.size()))  {
	    ODBCQueryTableClose(qs);
	    setSensitive(true);
	    return false;
	}
    }
    ODBCQueryTableClose(qs);

    if(records.size() > i0) {
	setIds(i0, records);
    }
    return true;
}

void TableQuery::setIds(int i0, gvector<CssTableClass *> &records)
{
    const char *name;
    int i, source, user, passwd;

    name = records[i0]->getName();
    records[i0]->getSource(&source, &user, &passwd);

    if(!strcmp(name, cssArrival)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssArrivalClass *)records[i])->arid);
	}
    }
    else if(!strcmp(name, cssParrival)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssParrivalClass *)records[i])->parid);
	}
    }
    else if(!strcmp(name, cssStamag)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssStamagClass *)records[i])->arid);
	}
    }
    else if(!strcmp(name, cssAssoc)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssAssocClass *)records[i])->arid);
	}
    }
    else if(!strcmp(name, cssOrigin)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssOriginClass *)records[i])->orid);
	}
    }
    else if(!strcmp(name, cssOrigerr)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssOrigerrClass *)records[i])->orid);
	}
    }
    else if(!strcmp(name, cssWfdisc)) {
	for(i = i0; i < records.size(); i++) {
	    records[i]->setIds(source, ((CssWfdiscClass *)records[i])->wfid);
	}
    }
}
#endif

bool TableQuery::runFlatQuery(gvector<CssTableClass *> &records, char *query_string,
			const string &tableName)
{
     int num, ret;
     FFDBQuery *qs;

    if(!updateWorking(tableName, records.size()))  {
	return true;
    }

    if( !(qs = open_db->getFFDB()->startQuery(query_string, tableName)) ) {
	showWarning(FFDatabase::FFDBErrMsg());
	return false;
    }

    Free(current_query);
    current_query = strdup(query_string);

    while((ret = qs->getResults(100, &num, &records)) > 0)
    {
	if(!updateWorking(tableName, records.size()))  {
	    delete qs;
	    return false;
	}
    }
    delete qs;

    return true;
}

void TableQuery::unspecifiedRunQuery(OpenDBStruct *op)
{
    if(open_db->sourceType() == NO_CONNECTION) {
	showWarning("Not connected to a data source.");
	return;
    }
    clearTab("dynamic");

    if(op->general_query[0] != '\0') {
	runUnspecifiedQuery(op->general_query);
	setQueryText("dynamic", op->general_query);
	saveQueryText("dynamic", op->general_query);
    }
}

void TableQuery::runUnspecifiedQuery(char *query_string)
{
    const char *tableName = "dynamic";
    CSSTable *table;
    gvector<CssTableClass *> v;

    setCursor("hourglass");

    runQuery(tableName, query_string, v);

    setCursor("default");

    if(v.size() == 0) {
	if( no_records_warn ) showWarning("No rows returned.");
	return;
    }

    if(!(table = getCSSTable(tableName)))
    {
	table = addTableTab(tableName);
//	putTabProperty();
    }

    tab->setOnTop(tableName);

    setChange(table);
    table->removeAllRecords();
    tabSelect(tableName);
    sortRows(v, tableName);
    table->addRecords(v);
    setChange(v);
    setButtonsSensitive();
}

void TableQuery::sortRows(gvector<CssTableClass *> &v, const string &tableName)
{
    if( !tableName.compare(cssArrival) ||
	!tableName.compare(cssAssoc) ||
	!tableName.compare(cssAmplitude) ||
	!tableName.compare(cssStamag) ||
	!tableName.compare(cssHydroFeatures) ||
	!tableName.compare(cssInfraFeatures))
    {
	CssTableClass::sort(v, "arid");
    }
    else if(!tableName.compare(cssOrigin) ||
	    !tableName.compare(cssOrigerr) ||
	    !tableName.compare(cssNetmag))
    {
	CssTableClass::sort(v, "orid");
    }
    else if(!tableName.compare(cssWfdisc))
    {
	CssTableClass::sort(v, "sta");
    }
    else if(!tableName.compare(cssWftag))
    {
	CssTableClass::sort(v, "wfid");
    }
    else if(!tableName.compare(cssStassoc)) {
	CssTableClass::sort(v, "stassid");
    }
    else if(!tableName.compare(cssSitechan)) {
	CssTableClass::sort(v, "chanid");
    }
    else if(!tableName.compare(cssSite)) {
	CssTableClass::sort(v, "sta");
    }
    else if(!tableName.compare(cssGregion)) {
	CssTableClass::sort(v, "grn");
    }

}

void TableQuery::tabQuery(void)
{
    Widget tab_form;
    const char *tableName;
    char *query_string;

    if((tab_form = tab->getTabOnTop()) == NULL) return;

    int i;
    for(i = 0; i < num_tabs &&
		tab_forms[i].tab_form->baseWidget() != tab_form; i++);
    if(i == num_tabs) return;
    query_string = tab_forms[i].query_text->getString();
    if(query_string[0] == '\0') {
	Free(query_string);
	return;
    }

    tableName = XtName(tab_forms[i].tab_form->baseWidget());
    saveQueryText(tableName, query_string);

    XmTextSetInsertionPosition(tab_forms[i].query_text->baseWidget(),
			(int)strlen(query_string));
    if(!strcmp(tableName, "dynamic")) {
	const char *labels = "dynamic";
	showWorking(1, &labels);
	runUnspecifiedQuery(query_string);
    }
    else {
	const char *labels = tableName;
	showWorking(1, &labels);
	runQuery(tableName, query_string, NULL_TIME, NULL_TIME);
    }
    closeWorking();
    Free(query_string);
}

void TableQuery::getWftagsFromOrigins(void)
{
    char query_string[MAX_Q_LEN], *account, orid[20];
    string wftag_table;
    int i, j, num, *rows = NULL;
    gvector<CssTableClass *> origins, v;
    CSSTable *table;

    if(!(table = getCSSTable(cssOrigin))) return;

    table->getSelectedRecords(origins);

    if(origins.size() > 0) {
	num = origins.size();
	for(i = 0; i < origins.size(); i++) {
	    origins[i]->setSelected(false);
	}
	for(i = 0; i < num; i++) {
	    origins[rows[i]]->setSelected(true);
	}
    }
    else { /* if none selected, do all rows */
	table->getRecords(origins);
	num = origins.size();
	for(i = 0; i < origins.size(); i++) {
	    origins[i]->setSelected(true);
	}
    }

    if(!(table = getCSSTable(cssWftag))) {
	table = addTableTab(cssWftag);
    }
    if(!table) {
	return; /* should not be */
    }

    clearTab(cssWftag);

    wftag_table = open_db->getMapping(cssWftag);
    if( !(account = open_db->getAccount()) ) {
	return;
    }

    /* do multiple queries for MAX_N_EXPR origins at a time
     */
    setCursor("hourglass");

    for(i = j = 0; i < origins.size(); i++)
	if(origins[i]->getSelected())
    {
	CssOriginClass *o = (CssOriginClass *)origins[i];
	if(j == 0) {
	    snprintf(query_string, MAX_Q_LEN,
		"select * from %s%s%s where tagname = 'orid' and tagid in (",
		account, PERIOD(account), wftag_table.c_str());
	}
	snprintf(orid, 20, "%ld", o->orid);
	if(strlen(query_string) + strlen(orid) + 2 > MAX_Q_LEN) {
	    showWarning("Maximum query length exceeded.");
	    break;
	}
	strcat(query_string, orid);

	if(j == MAX_N_EXPR-1 || i == origins.size()-1)
	{
	    j = 0;
	    strcat(query_string, ")");
	    appendQueryText(cssWftag, query_string);

	    if(!runQuery(cssWftag, query_string, v)) {
		break;
	    }
	}
	else {
	    j++;
	    strcat(query_string, ", ");
	}
    }
    Free(account);

    sortRows(v, cssWftag);
    table->addRecords(v);
    setChange(v);
    setButtonsSensitive();

    setCursor("default");

    if(v.size() == 0) {
	showWarning("No wftag rows returned.");
    }

    for(i = 0; i < origins.size(); i++) {
	origins[i]->setSelected(false);
    }
}

void TableQuery::getLinkedTable(const string &table1, const string &id_member,
			const string &table2)
{
    string table2_map;
    char query_string[MAX_Q_LEN], *account, buf[20];
    int i, j, num_ids, num_members, offset1;
    long *ids = NULL;
    gvector<CssTableClass *> v1, v;
    CSSTable *table;
    CssClassDescription *des;

    if(!(table = getCSSTable(table1))) return;

    table->getSelectedRecords(v1);
    if( v1.size() <= 0 ){
	table->getRecords(v1);
    }

    if(!(table = getCSSTable(table2))) {
	table = addTableTab(table2);
    }
    if(!table) return; /* should not be */

    clearTab(table2);

    table2_map = open_db->getMapping(table2);
    if( !(account = open_db->getAccount()) ) return;

    num_members = CssTableClass::getDescription(table1, &des);
    for(i = 0; i < num_members && id_member.compare(des[i].name); i++);
    if(i == num_members) {
	showWarning("getLinkedTable: member %s not in table %s",
			id_member.c_str(), table1.c_str());
	return;
    }
    if(des[i].type != CSS_LONG) {
	showWarning("getLinkedTable: member %s.%s is not a long",
			table1.c_str(), id_member.c_str());
	return;
    }
    offset1 = des[i].offset;

    num_members = CssTableClass::getDescription(table2, &des);
    for(i = 0; i < num_members && id_member.compare(des[i].name); i++);
    if(i == num_members) {
	showWarning("getLinkedTable: member %s not in table %s",
			id_member.c_str(), table2.c_str());
	return;
    }
    if(des[i].type != CSS_LONG) {
	showWarning("getLinkedTable: member %s.%s is not a long",
			table2.c_str(), id_member.c_str());
	return;
    }

    /* sort unique ids
     */
    ids = (long *)malloc(v1.size()*sizeof(long));
    for(i = 0; i < v1.size(); i++)
    {
	ids[i] = *(long *)((char *)v1[i] + offset1);
    }
    qsort(ids, v1.size(), sizeof(long), sortLong);
    num_ids = 0;
    for(i = 0; i < v1.size(); i++) {
	if(i == 0) {
	    num_ids++;
	}
	else if(ids[i] != ids[i-1]) {
	    ids[num_ids++] = ids[i];
	}
    }
    setQueryText(table2, "");

    setCursor("hourglass");

    /* do multiple queries for MAX_N_EXPR table2's at a time
     */
    query_string[0] = '\0'; /* for num_ids=0 */

    for(i = j = 0; i < num_ids; i++)
    {
	if(j == 0) {
	    snprintf(query_string, MAX_Q_LEN,
		"select * from %s%s%s where %s in (", account, PERIOD(account),
		table2_map.c_str(), id_member.c_str());
	}
	snprintf(buf, 20, "%ld", ids[i]);
	if(strlen(query_string) + strlen(buf) + 2 > MAX_Q_LEN) {
	    showWarning("Maximum query length exceeded.");
	    break;
	}
	strcat(query_string, buf);

	if(j == MAX_N_EXPR-1 || i == num_ids-1)
	{
	    j = 0;
	    strcat(query_string, ")");
	    appendQueryText(table2, query_string);

	    if(!runQuery(table2, query_string, v)) break;
	}
	else {
	    j++;
	    strcat(query_string, ", ");
	}
    }
    XtFree(account);
    if(ids) free(ids);

    sortRows(v, table2);
    table->addRecords(v);
    setChange(v);
    if(!strcasecmp(table2.c_str(), cssWfdisc)) {
	wfdiscConstraintsInit();
    }
    setButtonsSensitive();

    setCursor("default");
}

void TableQuery::getAllRows(const string &tableName)
{
    char query_string[MAX_Q_LEN];
    CSSTable *table;
    gvector<CssTableClass *> v;

    // avoid repetitive query for static tables that are missing.
    if( open_db->noTable(tableName) ) return;

    if(!(table = getCSSTable(tableName))) {
	table = addTableTab(tableName);
    }
    if(!table) return; /* should not be */

    if(open_db->sourceType() == ODBC_CONNECTION)
    {
	char *account = open_db->getAccount();
	if(!account) return;
	string mapped_table = open_db->getMapping(tableName);

	if(!tableName.compare(cssAffiliation)) {
	    snprintf(query_string, MAX_Q_LEN,
"select * from %s%s%s where net in (select distinct(net) from %s%s%s where \
sta=net) order by net,sta",
		account, PERIOD(account), mapped_table.c_str(),
		account, PERIOD(account), mapped_table.c_str());
	}
	else if(!tableName.compare(cssSite)) {
	    snprintf(query_string, MAX_Q_LEN, "select * from %s%s%s where offdate = -1",
		account, PERIOD(account), mapped_table.c_str());
	}
	else if(!tableName.compare(cssSitechan)) {
	    snprintf(query_string, MAX_Q_LEN, "select * from %s%s%s where offdate = -1",
		account, PERIOD(account), mapped_table.c_str());
	}
	else {
	    snprintf(query_string, MAX_Q_LEN, "select * from %s%s%s",
		account, PERIOD(account), mapped_table.c_str());
	}
	XtFree(account);
    }
    else
    {
	snprintf(query_string, MAX_Q_LEN, "select * from %s",tableName.c_str());
    }

    runQuery(tableName, query_string, v);

    if( !v.size() && (!tableName.compare(cssSite)
		      || !tableName.compare(cssSitechan)
		      || !tableName.compare(cssGregion)
		      || !tableName.compare(cssAffiliation)) )
    {
	char *path = NULL;
	FFDatabase *ffdb = FFDatabase::FFDBOpenPrefix("local");
	ffdb->readStaticTable(tableName, v, &path);
	//printf("DEBUG: %s %s %d\n", tableName.c_str(), path, v.size());
	delete ffdb;
    }
    if( !v.size() ) {
	open_db->setNoTable(tableName);
	showWarning("No %s table", tableName.c_str());
    }
    sortRows(v, tableName);
    setChange(table);
    table->removeAllRecords();
    table->addRecords(v);
    setChange(v);
    if(!tableName.compare(cssWfdisc)) {
	wfdiscConstraintsInit();
    }
    setButtonsSensitive();

    setQueryText(tableName, query_string);
    saveQueryText(tableName, query_string);
}

void TableQuery::wfdiscsFromArrival(void)
{
    char *channels = NULL;
    char channels_clause[256]="";
    char query_string[MAX_Q_LEN], a_string[20], *account;
    string wfdisc_table, affiliation_table;
    double start, end=0., time_before = NULL_TIME, time_after = NULL_TIME;
    int i, j, n;
    gvector<CssTableClass *> arrivals;
    gvector<CssTableClass *> v;
    CSSTable *table;

    if(open_db->sourceType() != ODBC_CONNECTION) return;

    if(!open_db->arrivalTimeBefore(&time_before)) {
	showWarning("Invalid time before.");
	return;
    }
    if(!open_db->arrivalTimeAfter(&time_after)) {
	showWarning("Invalid time after.");
	return;
    }

    if(!getSelectedTableRecords(cssArrival, arrivals, false)) {
	getTableRecords(cssArrival, arrivals);
	if(arrivals.size() == 0) return;
    }
    for(i = 0; i < arrivals.size(); i++) {
	arrivals[i]->setSelected(true);
    }

    if(!(table = getCSSTable(cssWfdisc))) {
	table = addTableTab(cssWfdisc);
    }
    if(!table) {
	return; /* should not be */
    }

    if(!(account = open_db->getAccount())) return;
    wfdisc_table = open_db->getMapping(cssWfdisc);
    affiliation_table = open_db->getMapping(cssAffiliation);

    channels = open_db->arrivalChannels();
    check_query_char(channels, channels_clause, 256, "chan", 2);
    XtFree(channels);

    clearTab(cssWfdisc);

    setQueryText(cssWfdisc, "");

    setCursor("hourglass");

    for(i = 0; i < arrivals.size(); i++)
	if(arrivals[i]->getSelected())
    {
	CssArrivalClass *a = (CssArrivalClass *)arrivals[i];
	snprintf(query_string, MAX_Q_LEN,
"select distinct w.* from %s%s%s a, %s%s%s w where a.net='%s' and w.sta=a.sta ",
		account, PERIOD(account), affiliation_table.c_str(),
		account, PERIOD(account), wfdisc_table.c_str(), a->sta);

	if(channels_clause != NULL && (int)strlen(channels_clause) > 0)
	{
	    n = strlen(query_string);
	    snprintf(query_string+n, MAX_Q_LEN-n, "and %s ", channels_clause);
	}
	if(time_before > NULL_TIME_CHECK) {
	    start = a->time - time_before;
	}
	else {
	    start = a->time - 300.;
	}
	if(time_after > NULL_TIME_CHECK) {
	    end = a->time + time_after;
	}
	else {
	    end = a->time + 300.;
	}
	n = strlen(query_string);
	snprintf(query_string+n, MAX_Q_LEN-n,
	    "and endtime > %.2f and time between %.2f - %.2f and %.2f",
	    start, start, open_db->queryBuffer(), end);

	n = v.size();

	if(!runQuery(cssWfdisc, query_string, v)) break;

	if(time_before <= NULL_TIME_CHECK) start = NULL_TIME;
	if(time_after <= NULL_TIME_CHECK) end = NULL_TIME;

	snprintf(a_string, 20, "arid:%ld", a->arid);
	for(j = n; j < v.size(); j++) {
	    v[j]->putValue("display start", start);
	    v[j]->putValue("display end", end);
	    v[j]->putValue("arid", a->arid);
	    v[j]->putValue(a_string, a->arid);
	}

	appendQueryText(cssWfdisc, query_string);
    }
    XtFree(account);

    v.sort(sortByTime);

    /*
     * discard duplicates if there are no time limits.
     */
    if(time_before <= NULL_TIME_CHECK || time_after <= NULL_TIME_CHECK)
    {
	gvector<CssTableClass *> wfdiscs;
	for(i = 1; i < v.size(); i++) {
	    CssWfdiscClass *wf = (CssWfdiscClass *)v[i];
	    CssWfdiscClass *wf1 = (CssWfdiscClass *)v[i-1];
	    if(wf->wfid != wf1->wfid || wf->time != wf1->time
		    || strcmp(wf->sta, wf1->sta) || strcmp(wf->chan, wf1->chan))
	    {
		wfdiscs.push_back(v[i]);
	    }
	}
	v.clear();
	v.load(wfdiscs);
    }
    else /* keep duplicates only if the time windows are far apart */
    {
	long arid = -1;
	double wi_start=0., wj_start=0.;
	gvector<CssTableClass *> wfdiscs;
	CssWfdiscClass *wfi, *wfj;
	for(i = 0; i < v.size(); )
	{
	    wfi = (CssWfdiscClass *)v[i];
	    wfi->getValue("display start", &wi_start);
	    wfdiscs.push_back(v[i]);
	    for(j = i+1; j < v.size(); j++)
	    {
		wfj = (CssWfdiscClass *)v[j];
		wfj->getValue("display start", &wj_start);
		if(wfj->wfid != wfi->wfid || wfj->time != wfi->time
		    	|| strcmp(wfj->sta, wfi->sta)
		    	|| strcmp(wfj->chan, wfi->chan)) break;
		if(wj_start - wi_start > 2*time_after) break;
		wfj->getValue("display end", &end);
		wfi->putValue("display end", end);
		wfj->getValue("arid", &arid);
		snprintf(a_string, 20, "arid:%ld", arid);
		wfi->putValue(a_string, arid);
		wi_start = wj_start;
	    }
	    i = j;
	}
	v.clear();
	v.load(wfdiscs);
    }
    if(v.size() > 0) {
/*
	const char *extra[2] = {"display start", "display end"};
	const char *extra_formats[2] = {"%T", "%T"};
	table->setType(cssWfdisc, 2, extra, extra_formats, "");
*/
	sortRows(v, cssWfdisc);
	setChange(table);
	table->removeAllRecords();
	table->addRecords(v);
	setChange(v);
	setButtonsSensitive();
	wfdiscConstraintsInit();
    }
    else if( no_records_warn ) {
	showWarning("No wfdisc rows returned.");
    }

    setCursor("default");
}

void TableQuery::wfdiscsFromTime(char *sta, double qtime)
{
    char *channels = NULL;
    char channels_clause[256]="";
    char query_string[MAX_Q_LEN], *account;
    string wfdisc_table, affiliation_table;
    double start, end, time_before = NULL_TIME, time_after = NULL_TIME;
    int j, n;
    CSSTable *table;
    gvector<CssTableClass *> v;

    if(!open_db->arrivalTimeBefore(&time_before)) {
	showWarning("Invalid time before.");
	return;
    }
    if(!open_db->arrivalTimeAfter(&time_after)) {
	showWarning("Invalid time after.");
	return;
    }

    if(!(table = getCSSTable(cssWfdisc))) {
	table = addTableTab(cssWfdisc);
    }
    if(!table) return; /* should not be */

    if(!(account = open_db->getAccount())) return;
    wfdisc_table = open_db->getMapping(cssWfdisc);
    affiliation_table = open_db->getMapping(cssAffiliation);

    channels = open_db->arrivalChannels();
    check_query_char(channels, channels_clause, 256, "chan", 2);
    XtFree(channels);

    clearTab(cssWfdisc);

    setQueryText(cssWfdisc, "");

    setCursor("hourglass");

    snprintf(query_string, MAX_Q_LEN,
"select distinct w.* from %s%s%s a, %s%s%s w where a.net='%s' and w.sta=a.sta ",
		account, PERIOD(account), affiliation_table.c_str(),
		account, PERIOD(account), wfdisc_table.c_str(), sta);

    if(channels_clause != NULL && (int)strlen(channels_clause) > 0)
    {
	n = strlen(query_string);
	snprintf(query_string+n, MAX_Q_LEN-n, "and %s ", channels_clause);
    }
    if(time_before > NULL_TIME_CHECK) {
	start = qtime - time_before;
    }
    else {
	start = qtime - 300.;
    }
    if(time_after > NULL_TIME_CHECK) {
	end = qtime + time_after;
    }
    else {
	end = qtime + 300.;
    }
    n = strlen(query_string);
    snprintf(query_string+n, MAX_Q_LEN-n,
	"and endtime > %.2f and time between %.2f - %.2f and %.2f",
	start, start, open_db->queryBuffer(), end);

    runQuery(cssWfdisc, query_string, v);

    if(time_before <= NULL_TIME_CHECK) start = NULL_TIME;
    if(time_after <= NULL_TIME_CHECK) end = NULL_TIME;

    for(j = 0; j < v.size(); j++) {
	v[j]->putValue("display start", start);
	v[j]->putValue("display end", end);
    }

    appendQueryText(cssWfdisc, query_string);

    XtFree(account);

    v.sort(sortByTime);

    if(v.size() > 0) {
	sortRows(v, cssWfdisc);
	setChange(table);
	table->removeAllRecords();
	table->addRecords(v);
	setChange(v);
	setButtonsSensitive();
	wfdiscConstraintsInit();
    }
    else if( no_records_warn ) {
	showWarning("No wfdisc rows returned.");
    }

    setCursor("default");
}

void TableQuery::storeOrids(gvector<CssTableClass *> &v)
{
    for(int i = 0; i < v.size(); i++) {
	v[i]->putValue("orid", getOrid(v[i]));
    }
}

int TableQuery::getAAWO(bool query_wfdiscs, bool get_net_tables)
{
    const char *labels[20];
    int num;
    CSSTable *table;

    if(open_db->sourceType() == NO_CONNECTION) {
	showWarning("Not connected to a data source.");
	return -1;
    }
    if(!(table = getCSSTable(cssOrigin))) return -1;

    if(!table->numSelectedRows()) {
	showWarning("No origins selected.");
    }

    /* Clear all tables that can be refilled by this routine.
     */
    clearTab(cssOrigerr);
    clearTab(cssAssoc);
    clearTab(cssArrival);
    clearTab(cssWfdisc);
    clearTab(cssNetmag);
    clearTab(cssStamag);
    clearTab(cssAmplitude);
    clearTab(cssParrival);

    num = 0;
    labels[num++] = cssOrigerr;
    labels[num++] = cssNetmag;
    labels[num++] = cssAssoc;
    labels[num++] = cssParrival;
    labels[num++] = cssArrival;
    labels[num++] = cssAmplitude;
    labels[num++] = cssStamag;
    if(query_wfdiscs) labels[num++] = cssWfdisc;
    if(get_net_tables) {
	// these tables are retrieved after this routine.
	if(emptyTab(cssAffiliation)) labels[num++] = cssAffiliation;
	if(emptyTab(cssSite)) labels[num++] = cssSite;
	if(emptyTab(cssSitechan)) labels[num++] = cssSitechan;
	if(emptyTab(cssGregion)) labels[num++] = cssGregion;
    }
    showWorking(num, labels);

    getLinkedTable(cssOrigin, "orid", cssOrigerr);
    getLinkedTable(cssOrigin, "orid", cssNetmag);
    getLinkedTable(cssOrigin, "orid", cssAssoc);
    getLinkedTable(cssOrigin, "orid", cssParrival);
    if(emptyTab(cssAssoc)) {
	if(!get_net_tables) closeWorking();
	return 1;
    }
    getLinkedTable(cssAssoc, "arid", cssArrival);
    getLinkedTable(cssAssoc, "arid", cssAmplitude);
    getLinkedTable(cssAssoc, "arid", cssStamag);

    if(emptyTab(cssArrival)) {
	if(!get_net_tables) closeWorking();
	return 2;
    }

    if(query_wfdiscs) {
	wfdiscsFromArrival();
    }
    if(!get_net_tables) closeWorking();

    return 0;
}

void TableQuery::getAnotherTable(const string &tab_name,const string &tableName)
{
    int i, j;
    const char *labels[1];


    for(i = 0; i < (int)table_ids.size(); i++)
    {
	if(!tab_name.compare(table_ids[i].table_name))
	{
	    for(j = 0; j < (int)table_ids.size(); j++)
	    {
		if(!table_ids[i].id_name.compare(table_ids[j].id_name) &&
		   !table_ids[j].table_name.compare(tableName))
		{
		    labels[0] = tableName.c_str();
		    showWorking(1, labels);
		    getLinkedTable(tab_name, table_ids[j].id_name, tableName);
		    closeWorking();
		    return;
		}
	    }
	}
    }
    fprintf(stderr, "getAnotherTable failed for %s %s\n",
		tab_name.c_str(), tableName.c_str());
}

void TableQuery::setWaveformReceiver(DataReceiver *dr)
{
    waveform_receiver = dr;
    if(waveform_receiver)
    {
	string ontop = getTopTabName();
	if( ontop.empty() ) {
	    addTableTab(cssOrigin);
	    tabSelect(cssOrigin);
	    ontop.assign(cssOrigin);
	}
	bool set = ((!ontop.compare(cssOrigin) || !ontop.compare(cssArrival) ||
		!ontop.compare(cssWfdisc)) && numSelectedRows(ontop));
	display_waveforms->setSensitive(set);

	if(waveform_receiver->getWaveformWindowInstance() ||
	    waveform_receiver->getWaveformPlotInstance()) 
	{
	    waveform_subclass = true;
	    set = (!ontop.compare(cssOrigin) && numSelectedRows(cssOrigin) > 0)
			? true : false;
	    review_origins->setSensitive(set);
	}
	else {
	    review_origins->setSensitive(false);
	}
    }
    else {
	display_waveforms->setSensitive(false);
	review_origins->setSensitive(false);
    }
}

void TableQuery::displayWaveforms(TQDisplayType display_type,
			const string &ontop, bool visible)
{
    WaveformWindow *ww = NULL;
    WaveformPlot *wp=NULL;

    if(!waveform_receiver) {
	cerr<< "TableQuery::displayWaveforms: waveform_receiver is NULL"<< endl;
	return;
    }
    if( (ww = waveform_receiver->getWaveformWindowInstance()) ) {
	wp = ww->wplot;
    }
    else if( !(wp = waveform_receiver->getWaveformPlotInstance()) &&
		display_type == REVIEW_ORIGINS)
    {
	// can review only if waveform_receiver is a WaveformWindow
	return;
    }

    const char *labels[20];
    int num = 0;

    if(!ontop.compare(cssOrigin))
    {
	if(open_db->sourceType() == NO_CONNECTION) {
	    showWarning("Not connected to a data source.");
	    return;
	}
	if(!numSelectedRows(cssOrigin)) {
	    showWarning("No origins selected.");
	    return;
	}
	int ret = getAAWO(true, true);
	if(ret == -1) {
	    return;
	}
	else if(ret == 1) {
	    showWarning("No assocs found.");
	    return;
	}
	else if(ret == 2) {
	    showWarning("No arrivals found.");
	    return;
	}
    }
    else if(!ontop.compare(cssArrival))
    {
	labels[num++] = cssWfdisc;
	labels[num++] = cssAssoc;
	labels[num++] = cssOrigin;
	labels[num++] = cssOrigerr;
	labels[num++] = cssNetmag;
	labels[num++] = cssStamag;
	labels[num++] = cssAmplitude;
	labels[num++] = cssParrival;
	if(emptyTab(cssAffiliation)) labels[num++] = cssAffiliation;
	if(emptyTab(cssSite)) labels[num++] = cssSite;
	if(emptyTab(cssGregion)) labels[num++] = cssGregion;
	if(emptyTab(cssSitechan)) labels[num++] = cssSitechan;

	showWorking(num, labels);

	wfdiscsFromArrival();

	getLinkedTable(cssArrival, "arid", cssAssoc);
	getLinkedTable(cssAssoc, "orid", cssOrigin);
	getLinkedTable(cssOrigin, "orid", cssOrigerr);
	getLinkedTable(cssOrigin, "orid", cssNetmag);
	getLinkedTable(cssOrigin, "orid", cssStamag);
	getLinkedTable(cssArrival, "arid", cssAmplitude);
	getLinkedTable(cssOrigin, "orid", cssParrival);
    }

    if(display_type != DISPLAY_ARRIVALS) {
	if( !selectWfdiscs(ontop, display_type) ) {
	    closeWorking();
	    return;
	}
    }

    if(emptyTab(cssAffiliation) || emptyTab(cssSite) || emptyTab(cssGregion) || emptyTab(cssSitechan)) {
	if(open_db->sourceType() == NO_CONNECTION) {
	    showWarning("Not connected to a data source.");
	    closeWorking();
	    return;
	}
	bool show_working = ontop.compare(cssOrigin) && ontop.compare(cssArrival);
	getNetworkTables(show_working);
    }
    closeWorking();

    if(display_type == DISPLAY_ARRIVALS) {
	arrivalElements();
	last_display_waveforms = false;
	if(waveform_receiver) waveform_receiver->setDataSource(this);
	last_display_waveforms = true;
    }
    else
    {
	if(!applyDisplayConstraints() && display_type != REVIEW_ORIGINS) return;

	if(display_type == REVIEW_ORIGINS) {
	    // add beams to wfdiscs
	    setCursor("hourglass");
	    reviewBeams();
	    setCursor("default");
	}
	int n_old = wp->numWaveforms();
	if(waveform_receiver) {
	    if(visible && (ww=waveform_receiver->getWaveformWindowInstance())) {
		if( isVisible() ) {
		    ww->setVisible(true);
		}
	    }
	    waveform_receiver->setDataSource(this);
	}

	if(display_type == REVIEW_ORIGINS) {
	    applyFilters(wp, n_old);
	}
	else if(display_type == DISPLAY_WAVEFORMS) {
	    doRecentInput();
	}
    }
}

void TableQuery::doRecentInput(void)
{
    gvector<CssTableClass *> wf;
    WaveformWindow *ww;
    
    if( !sourceIsODBC() && waveform_receiver &&
	(ww = waveform_receiver->getWaveformWindowInstance()) &&
	getSelectedTable(cssWfdisc, wf) > 0 )

    {
	RecentInput *r = new RecentInput();

	int path = wf[0]->getFile();

	r->read_all = false;
	r->file.assign(quarkToString(path));
	r->format.assign("css");
	for(int i = 0; i < wf.size(); i++) {
	    r->wavs.push_back(wf[i]->filePosition());
	}
	ww->addRecentInput(r);
    }
}

void TableQuery::applyFilters(WaveformPlot *wp, int n_old)
{
    gvector<Waveform *> wvec;

    wp->getWaveforms(wvec, false);
    for(int i = n_old; i < wvec.size(); i++)
    {
	for(int j = 0; j < num_review_channels; j++)
	    if( !strcasecmp(wvec[i]->sta(), rc[j].sta) &&
		!strcasecmp(wvec[i]->chan(), rc[j].chan))
	{
	    BeamRecipe recipe;
	    IIRFilter *iir = NULL;
	    if(rc[j].filter)
	    {
		iir = new IIRFilter(rc[j].order,rc[j].type, rc[j].flow,
			rc[j].fhigh, wvec[i]->segment(0)->tdel(),
			rc[j].zero_phase);
	    }
	    else if( gbeam->beamRecipe(rc[j].sta, rc[j].chan, recipe) )
	    {
		iir = new IIRFilter(recipe.ford, recipe.ftype, recipe.flo,
			recipe.fhi, wvec[i]->segment(0)->tdel(),
			recipe.zp);
	    }

	    if(iir) {
		DataMethod *dm[2];
		dm[0] = new TaperData("cosine", 10, 5, 200);
		dm[1] = iir;
		DataMethod::changeMethods(2, dm, wvec[i]);
		wp->modify(wvec[i]);
	    }
	}
    }
}

// BasicSource virtual function
void TableQuery::getNetworkTables(void)
{
    getNetworkTables(true);
}

void TableQuery::getNetworkTables(bool show_working)
{
    int num = 0;
    bool get_affiliation=false, get_site=false, get_sitechan=false, get_gregion=false;
    const char *labels[3];

    if(emptyTab(cssAffiliation) && !open_db->noTable(cssAffiliation)) {
	get_affiliation = true;
	labels[num++] = cssAffiliation;
    }
    if(emptyTab(cssSite) && !open_db->noTable(cssSite)) {
	get_site = true;
	labels[num++] = cssSite;
    }

    if(emptyTab(cssGregion) && !open_db->noTable(cssGregion)) {
	get_gregion = true;
	labels[num++] = cssGregion;
    }


    if(emptyTab(cssSitechan) && !open_db->noTable(cssSitechan)) {
	get_sitechan = true;
	labels[num++] = cssSitechan;
    }

    if(show_working && num) {
	showWorking(num, labels);
    }
    if(get_affiliation) {
	getAllRows(cssAffiliation);
    }
    if(get_site) {
	getAllRows(cssSite);
    }
    if(get_sitechan) {
	getAllRows(cssSitechan);
    }
    if(get_gregion) {
	getAllRows(cssGregion);
    }


    if(show_working && num) {
	closeWorking();
    }
}

bool TableQuery::selectWfdiscs(const string &tableName,
			TQDisplayType display_type)
{
    char a_string[20];
    int i, j, num;
    long arid;

    CSSTable *wfdisc_table = getCSSTable(cssWfdisc);
    if( !wfdisc_table || !wfdisc_table->numRows() ) {
	showWarning("No wfdisc records.");
	return false;
    }

    if(!tableName.compare(cssArrival))
    {
	gvector<CssTableClass *> arrivals;
	CSSTable *table = tableOnTop();
	table->getSelectedRecords(arrivals);
	if( arrivals.size() <= 0) {
	    // if none selected, do all rows
	    table->getRecords(arrivals);
	    if( arrivals.size() <= 0) {
		return false;
	    }
	}
	gvector<CssTableClass *> *wfdiscs = wfdisc_table->getRecords();
	num = 0;
	if(wfdiscs) {
	    for(i = 0; i < arrivals.size(); i++)
	    {
		CssArrivalClass *a = (CssArrivalClass *)arrivals[i];
		snprintf(a_string, 20, "arid:%ld", a->arid);
		for(j = 0; j < wfdiscs->size(); j++)
		{
		    if(wfdiscs->at(j)->getValue(a_string, &arid))
		    {
			wfdisc_table->selectRow(j, true);
			num++;
		    }
		}
	    }
	}
	if(!num) {
	    if( no_records_warn ) {
		showWarning("No wfdiscs found for selected arrivals.");
	    }
	    return false;
	}
    }
    else {
	/* display only selected rows or all rows if no rows are selected
	 */
	if(display_type == REVIEW_ORIGINS) {
	    /* display only preferred channels */
	    gvector<CssTableClass *> *wfdiscs = wfdisc_table->getRecords();
	    if(wfdiscs) {
		for(i = 0; i < wfdiscs->size(); i++) {
		    CssWfdiscClass *f = (CssWfdiscClass *)wfdiscs->at(i);
		    bool set = reviewChannel(f->sta, f->chan);
//		    wfdiscs->at(i)->setSelected(set);
		    wfdisc_table->selectRow(i, set);
		}
	    }
	}
	else if(!wfdisc_table->numSelectedRows()) {
	    wfdisc_table->selectAllRows(true);
	}
    }
    if(display_type != REVIEW_ORIGINS) {
	return (wfdisc_table->numSelectedRows() > 0) ? true : false;
    }
    else {
	return true;
    }
}

bool TableQuery::reviewChannel(char *sta, char *chan)
{
    int i;

    for(i = 0; i < num_review_channels && (strcasecmp(sta, rc[i].sta)
	|| !compareChan(chan, rc[i].chan)); i++);
    return (i < num_review_channels) ? true : false;
}

bool TableQuery::reviewBeams(void)
{
    int i, j, k, nsta, n_origins;
    long arid = -1, orid;
    string net;
    double display_start=0., display_end=0.;
    vector<BeamSta> beam_sta;
    BeamRecipe recipe;
    CssWfdiscClass *wf;
    CssOriginClass **o=NULL;
    cvector<CssSiteClass> sites;
    cvector<CssWfdiscClass> wfdiscs, sel_wfdiscs;
    cvector<CssOriginClass> origins;

    getTableRecords(cssWfdisc, wfdiscs, true);
    getTableRecords(cssOrigin, origins, true);

    if(!wfdiscs.size() || !origins.size()) {
	return false;
    }

    getSelectedTableRecords(cssWfdisc, sel_wfdiscs, true);

    if(assocs_sort) delete assocs_sort;
    assocs_sort = new gvector<CssTableClass *>(getTableRecords(cssAssoc, true));

    storeOrids(wfdiscs);
    wfdiscs.sort(sortByOrid);

    sel_wfdiscs.sort(sortByOrid);

    o = (CssOriginClass **)mallocWarn(origins.size()*sizeof(CssOriginClass *));
    /* get origins
     */
    n_origins = 0;
    orid = getOrid(wfdiscs[0]);
    if(orid >= 0) {
	for(i = 0; i < origins.size() && orid != origins[i]->orid; i++);
	if(i < origins.size()) o[n_origins++] = origins[i];
    }

    for(j = 1; j < wfdiscs.size(); j++) {
	orid = getOrid(wfdiscs[j]);
	if(orid >= 0 && orid != o[n_origins-1]->orid) {
	    for(i=0; i < origins.size() && orid != origins[i]->orid; i++);
	    if(i < origins.size()) o[n_origins++] = origins[i];
	}
    }

    /* check for review beams that must be created
     */
    for(i = 0; i < num_review_channels; i++)
    {
	if( gbeam->beamRecipe(rc[i].sta, rc[i].chan, recipe) )
	{
	    net = getNet(rc[i].sta);
	    /* rc[i] is a beam. For each origin, check if we already have
	     * the beam in the wfdiscs
	     */
	    for(k = 0; k < n_origins; k++)
	    {
		for(j = 0; j < sel_wfdiscs.size(); j++) {
		    wf = sel_wfdiscs[j];
		    if( !strcasecmp(rc[i].sta, wf->sta) &&
			!strcasecmp(rc[i].chan, wf->chan) &&
			    o[k]->orid == getOrid(wf)) break;
		}
		if(j < sel_wfdiscs.size()) continue; /* found it */

		/* check if we have any waveforms from the same net/orid
		 */
		    
		for(j = 0; j < wfdiscs.size(); j++) {
		    wf = wfdiscs[j];
		    if(!strcasecmp(net.c_str(), getNet(wf->sta).c_str()) &&
			o[k]->orid == getOrid(wf)) break;
		}
		if(j < wfdiscs.size()) {
		    // Beam not found. Must create it.
		    if((nsta=gbeam->getGroup(recipe, beam_sta)) <= 0) {
			showWarning("Error getting beam group:\n%s",
				GError::getMessage());
		    }
		    else {
			wfdiscs[j]->getValue("display start", &display_start);
			wfdiscs[j]->getValue("display end", &display_end);
			wfdiscs[j]->getValue("arid", &arid);
			makeBeam(rc[i].sta, rc[i].chan, display_start,
				display_end, arid, wfdiscs, recipe,
				beam_sta, o[k], sel_wfdiscs);
		    }
		}
	    }
	}
    }
    Free(o);

    getTableRecords(cssSite, sites, false);
    if(sites.size() == 0) {
	return true;
    }
    sites.sort(sortBySta);

    for(i = 0; i < sel_wfdiscs.size(); i++) {
	CssOriginClass *origin;
	CssWfdiscClass *wf = sel_wfdiscs[i];
	double delta = -999., az, baz;
	if((j = findSta(wf->sta_quark, sites)) >= 0
			&& (origin = getOrigin(wf, origins)))
	{
	    deltaz(origin->lat, origin->lon, sites[j]->lat, sites[j]->lon,
			&delta, &az, &baz);
	}
	wf->putValue("delta", delta);
    }

    storeOrids(sel_wfdiscs);
    sel_wfdiscs.sort(sortByDist);

    reviewElements(sel_wfdiscs, wfdiscs);

    CSSTable *wfdisc_table;
    if((wfdisc_table = getCSSTable(cssWfdisc))) {
	setChange(sel_wfdiscs);
	wfdisc_table->removeAllRecords();
	wfdisc_table->addRecords(sel_wfdiscs);
	setChange(sel_wfdiscs);
    }

    return true;
}

bool TableQuery::arrivalElements(void)
{
    int i, j;
    gvector<CssTableClass *> *sites, *wfdiscs;
    gvector<CssTableClass *> *t = getTableRecords(cssWfdisc, true);
    gvector<CssTableClass *> *origins = getTableRecords(cssOrigin, true);

    if(!t || !t->size() || !origins || !origins->size()) return false;

    wfdiscs = new gvector<CssTableClass *>(*t);

    if(assocs_sort) delete assocs_sort;
    assocs_sort = new gvector<CssTableClass *>(getTableRecords(cssAssoc, true));

    storeOrids(*wfdiscs);
    wfdiscs->sort(sortByOrid);

    t = getTableRecords(cssSite, false);
    if(!t || t->size() == 0) {
	delete wfdiscs;
	return true;
    }
    sites = new gvector<CssTableClass *>(*t);
    sites->sort(sortBySta);

    for(i = 0; i < wfdiscs->size(); i++) {
	CssOriginClass *origin;
	CssWfdiscClass *wf = (CssWfdiscClass *)wfdiscs->at(i);
	double delta = -999., az, baz;
	if((j = findSta(wf->sta_quark, *sites)) >= 0
		&& (origin = getOrigin(wf, *origins)))
	{
	    deltaz(origin->lat, origin->lon, ((CssSiteClass *)sites->at(j))->lat,
		((CssSiteClass *)sites->at(j))->lon, &delta, &az, &baz);
	}
	wf->putValue("delta", delta);
    }
    delete sites;

    storeOrids(*wfdiscs);
    wfdiscs->sort(sortByDist);

    saveArrivalElements(*wfdiscs);

    delete wfdiscs;

    return true;
}

void TableQuery::saveArrivalElements(gvector<CssTableClass *> &wfdiscs)
{
    int j;

    gvector<CssTableClass *> *arrivals = getTableRecords(cssArrival, true);
    if( !arrivals || !arrivals->size() || !assocs_sort) return;

    for(int i = 0; i < arrivals->size(); i++)
    {
	CssArrivalClass *a = (CssArrivalClass *)arrivals->at(i);
	for(j = 0; j < assocs_sort->size() &&
		((CssAssocClass *)assocs_sort->at(j))->arid != a->arid; j++);
	long orid = (j < assocs_sort->size()) ?
			((CssAssocClass *)assocs_sort->at(j))->orid : -1;
	string net = getNet(a->sta);

	gvector<SegmentInfo *> *segs = new gvector<SegmentInfo *>;
	a->putValue("elements", segs);

	for(j = 0; j < wfdiscs.size(); j++)
	{
	    CssWfdiscClass *w = (CssWfdiscClass *)wfdiscs[j];

	    if(!strcasecmp(net.c_str(), getNet(w->sta).c_str()) && orid == getOrid(w))
	    {
		int k;
		for(k = 0; k < (int)segs->size(); k++) {
		    SegmentInfo *s =segs->at(k);
		    if( !strcmp(s->sta, w->sta) &&
			compareChan(s->chan, w->chan) &&
			s->wfdisc()->time == w->time) break;
		}
		if(k < (int)segs->size()) continue; /* prevent duplicates */

		SegmentInfo *s = new SegmentInfo();

		s->id = w->wfid;
		stringcpy(s->sta, w->sta, sizeof(s->sta));
		stringcpy(s->chan, w->chan, sizeof(s->chan));
		s->path = w->getFile();
		s->file_order = w->filePosition();
//	gnetChangeStaChan(s->sta, sizeof(s->sta), s->chan, sizeof(s->chan));

		s->start = w->time;
		s->jdate = timeEpochToJDate(s->start);
		if(w->samprate > 0.) {
		    s->end = w->time + (w->nsamp-1)/w->samprate;
		}
		else {
		    s->end = w->time + (w->nsamp-1);
		}
		double tbeg=0., tend=0.;
		w->getValue("display start", &tbeg);
		w->getValue("display end", &tend);
		if(tbeg > NULL_TIME_CHECK && tbeg > s->start) {
		    s->start = tbeg;
		}
		if(tend > NULL_TIME_CHECK && tend < s->end) {
		    s->end = tend;
		}
		s->selected = true;
		s->setWfdisc(new CssWfdiscClass(*w));
		segs->push_back(s);
	    }
	}
	getNetworks(segs);
	getSites(segs);
	getSitechans(segs);
    }
}

bool TableQuery::selectChannel(char *sta, char *chan)
{
    CSSTable *wfdisc_table;

    if( !(wfdisc_table = getCSSTable(cssWfdisc)) ) return false;

    wfdisc_table->selectAllRows(false);

    vector<const char *> wf_sta, wf_chan;
    int nrows = wfdisc_table->getColumnByLabel("sta", wf_sta);
    wfdisc_table->getColumnByLabel("chan", wf_chan);

    for(int i = 0; i < nrows; i++) {
	if( !strcasecmp(wf_sta[i], sta) && compareChan(wf_chan[i], chan) )
	{
	    wfdisc_table->selectRow(i, true);
	    return true;
	}
    }
    return false;
}

bool TableQuery::selectChannel(char *sta, char *chan, long arid,
			double display_start, double display_end)
{
    int i;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    CSSTable *wfdisc_table;

    if(selectChannel(sta, chan)) return true;

    if( !(wfdisc_table = getCSSTable(cssWfdisc)) ) return false;

    wfdisc_table->selectAllRows(false);
    int nrows = wfdisc_table->numRows();

    if( gbeam->beamRecipe(sta, chan, recipe) )
    {
	cvector<CssWfdiscClass> *wfdiscs =
		(cvector<CssWfdiscClass> *)getTableRecords(cssWfdisc, true);

	if(wfdiscs || !wfdiscs->size()) return false;

	/* get origins
	 */
	gvector<CssTableClass *> *origins = getTableRecords(cssOrigin, true);
	if(!origins || origins->size() == 0) return false;

	long orid = -1;
	gvector<CssTableClass *> *assocs= getTableRecords(cssAssoc, false);
	if(assocs && assocs->size()) {
	    for(i = 0; i < assocs->size() &&
		arid != ((CssAssocClass *)assocs->at(i))->arid; i++);
	    if(i < assocs->size()) orid = ((CssAssocClass *)assocs->at(i))->orid;
	}
	if(orid < 0) {
	    // no origin
	    return false;
	}

        for(i = 0; i < origins->size() &&
		orid != ((CssOriginClass *)origins->at(i))->orid; i++);
        if(i == origins->size()) {
	    // no origin
	    return false;
	}

	int nsta;
	if((nsta=gbeam->getGroup(recipe, beam_sta)) <= 0) {
	    showWarning("Error getting beam group:\n%s", GError::getMessage());
	    return false;
	}
	cvector<CssWfdiscClass> v;
	makeBeam(sta, chan, display_start, display_end, arid, *wfdiscs, recipe,
		beam_sta, (CssOriginClass *)origins->at(i), v);

	// add the beam wfdiscs
	wfdisc_table->addRecords(v);

	// select the wfdisc table rows just added.
	int n = wfdisc_table->numRows();
	for(i = nrows; i < n; i++) {
	    wfdisc_table->selectRow(i, true);
	}
	return true;
    }
    return false;
}

void TableQuery::selectBeamChannels(const string &sta, const string &chan,
		int num_exclude, char **exclude_sta)
{
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    int nsta;
    CSSTable *wfdisc_table;

    if( !(wfdisc_table = getCSSTable(cssWfdisc)) ) return;

    if( !gbeam->beamRecipe(sta, chan, recipe) ) {
	showWarning("Error getting beam recipe for %s/%s:",
		sta.c_str(), chan.c_str());
	return;
    }
    if((nsta = gbeam->getGroup(recipe, beam_sta)) <= 0) {
	showWarning("Error getting beam group:\n%s", GError::getMessage());
	return;
    }

    wfdisc_table->selectAllRows(false);

    vector<const char *> wf_sta, wf_chan;
    wfdisc_table->getColumnByLabel("sta", wf_sta);
    int n = wfdisc_table->getColumnByLabel("chan", wf_chan);

    for(int i = 0; i < n; i++) {
	for(int j = 0; j < nsta; j++) {
	    if(	!strcasecmp(wf_sta[i], beam_sta[j].sta) &&
		compareChan(wf_chan[i], beam_sta[j].chan))
	    {
		int k;
		for(k = 0; k < num_exclude &&
			strcasecmp(wf_sta[i], exclude_sta[k]); k++);
		if(k == num_exclude) {
		    wfdisc_table->selectRow(i, true);
		}
		break;
	    }
	}
    }
}

void TableQuery::reviewElements(gvector<CssTableClass *> &wfdiscs,
		gvector<CssTableClass *> &all)
{
    long orid;
    string net;

    for(int i = 0; i < wfdiscs.size(); i++)
    {
	gvector<SegmentInfo *> *segs = new gvector<SegmentInfo *>;
	wfdiscs[i]->putValue("elements", segs);

	orid = getOrid(wfdiscs[i]);
	net = getNet(((CssWfdiscClass *)wfdiscs[i])->sta);

	for(int j = 0; j < all.size(); j++)
	{
	    CssWfdiscClass *w = (CssWfdiscClass *)all[j];

	    if(!strcasecmp(net.c_str(), getNet(w->sta).c_str()) && orid == getOrid(w))
	    {
		int k;
		for(k = 0; k < segs->size(); k++) {
		    SegmentInfo *s = segs->at(k);
		    if( !strcmp(s->sta, w->sta) &&
			compareChan(s->chan, w->chan) &&
			s->wfdisc()->time == w->time) break;
		}
		if(k < segs->size()) continue; /* prevent duplicates */

		SegmentInfo *s = new SegmentInfo();

		s->id = w->wfid;
		stringcpy(s->sta, w->sta, sizeof(s->sta));
		stringcpy(s->chan, w->chan, sizeof(s->chan));
		s->path = w->getFile();
		s->file_order = w->filePosition();
//	gnetChangeStaChan(s->sta, sizeof(s->sta), s->chan, sizeof(s->chan));

		s->start = w->time;
		s->jdate = timeEpochToJDate(s->start);
		if(w->samprate > 0.) {
		    s->end = w->time + (w->nsamp-1)/w->samprate;
		}
		else {
		    s->end = w->time + (w->nsamp-1);
		}
		double tbeg=0., tend=0.;
		w->getValue("display start", &tbeg);
		w->getValue("display end", &tend);
		if(tbeg > NULL_TIME_CHECK && tbeg > s->start) {
		    s->start = tbeg;
		}
		if(tend > NULL_TIME_CHECK && tend < s->end) {
		    s->end = tend;
		}
		s->selected = true;
		s->setWfdisc(new CssWfdiscClass(*w));
		segs->push_back(s);
	    }
	}
	getNetworks(segs);
	getSites(segs);
	getSitechans(segs);
    }
}

long TableQuery::getOrid(CssTableClass *css)
{
    int i;
    long arid = -1;

    if(!css->getValue("arid", &arid) || !assocs_sort) {
	return -1;
    }
    for(i = 0; i < assocs_sort->size() &&
		arid != ((CssAssocClass *)assocs_sort->at(i))->arid; i++);
    return (i < assocs_sort->size()) ? ((CssAssocClass *)assocs_sort->at(i))->orid
		: -1;
}

CssOriginClass * TableQuery::getOrigin(CssTableClass *css, gvector<CssTableClass *> &origins)
{
    long orid;
    int i;

    if((orid = getOrid(css)) >= 0) {
	for(i = 0; i < origins.size() &&
		orid != ((CssOriginClass *)origins[i])->orid; i++);
	if(i < origins.size()) return (CssOriginClass *)origins[i];
    }
    return NULL;
}

bool TableQuery::applyDisplayConstraints(void)
{
    char **items=NULL;
    int i, j, num, *item_q=NULL;
    gvector<CssTableClass *> v;
    CssWfdiscClass *w;
    CSSTable *table;

    if( !getSelectedTableRecords(cssWfdisc, v, true) ) return false;
    table = getCSSTable(cssWfdisc);

    num = sta_list->getSelectedItems(&item_q);
    if(num > 0) {
	for(j = v.size()-1; j >= 0; j--)
	{
	    w = (CssWfdiscClass *)v[j];
	    for(i = 0; i < num && w->sta_quark != item_q[i]; i++);
	    if(i == num) {
		table->selectRecord(w, false);
		v.removeAt(j);
	    }
	}
	free(item_q);
    }

    num = chan_list->getSelectedItems(&item_q);
    if(num > 0) {
	for(j = v.size()-1; j >= 0; j--)
	{
	    w = (CssWfdiscClass *)v[j];
	    for(i = 0; i < num && w->chan_quark != item_q[i];i++);
	    if(i == num) {
		table->selectRecord(w, false);
		v.removeAt(j);
	    }
	}
	free(item_q);
    }

    num = jdate_list->getSelectedItems(&items);
    if(num > 0) {
	long *jdate = new long[num];
	for(i = 0; i < num; i++) {
	    timeParseJDate(items[i], &jdate[i]);
	}
	for(j = v.size()-1; j >= 0; j--)
	{
	    w = (CssWfdiscClass *)v[j];
	    for(i = 0; i < num && w->jdate != jdate[i];i++);
	    if(i == num) {
		table->selectRecord(w, false);
		v.removeAt(j);
	    }
	}
	for(i = 0; i < num; i++) {
	    free(items[i]);
	}
	Free(items);
	delete [] jdate;
    }

    applyOriginConstraints(table, v);
    applyTimeConstraint(table, v);

    num = ctype_list->getSelectedItems(&items);
    if(num > 0)
    {
	int sindex, cindex, priority, proto, i1;
	bool have_one;

	have_one = (v.size()) ? true : false;

	proto = !strcmp(items[0], "proto") ? true : false;
	i1 = proto ? 1 : 0;

	for(j = v.size()-1; j >= 0; j--)
	{
	    w = (CssWfdiscClass *)v[j];
	    getIndex(w->sta, w->chan, &sindex, &cindex, &priority);

	    for(i = i1; i < num && !checkCtype(items[i], priority); i++);

	    if( (!proto && i == num) || (num > 1 && i == num) ) {
		table->selectRecord(w, false);
		v.removeAt(j);
	    }
	}
	for(i = 0; i < num; i++) free(items[i]);
	Free(items);

	if(proto)
	{
	    initStaPresent(false);

	    if(!have_one)
	    {
		v.clear();
		table->getRecords(v);
	    }
	    for(i = 0; i < v.size(); i++) {
		w = (CssWfdiscClass *)v[i];
		getIndex(w->sta, w->chan, &sindex, &cindex, &priority);
		setStaPresent(sindex, cindex);
	    }
	    cleanStaPresent();
	    for(i = v.size()-1; i >= 0; i--) {
		w = (CssWfdiscClass *)v[i];
		getIndex(w->sta, w->chan, &sindex, &cindex, &priority);
		if(checkStaPresent(sindex, cindex)) {
		    table->selectRecord(w, true);
		}
		else {
		    table->selectRecord(w, false);
		    v.removeAt(i);
		}
	    }
	}
    }

    bool ret = (v.size() > 0) ? true : false;
    return ret;
}

void TableQuery::applyTimeConstraint(CSSTable *table, gvector<CssTableClass *> &v)
{
    int i;
    bool passed;
    CssWfdiscClass *w;
    double tmin, tmax, endtime;

    for(i = 0; i < v.size(); i++) {
        w = (CssWfdiscClass *)v[i];
        w->removeValue("global_start");
        w->removeValue("global_end");
    }
    for(i = v.size()-1; i >= 0; i--)
    {
	passed = true;
	w = (CssWfdiscClass *)v[i];
	if(w->samprate > 0.) endtime = w->time + (w->nsamp-1)/w->samprate;
	else endtime = w->endtime;

	if(w->getValue("display start", &tmin) && tmin > NULL_TIME_CHECK			&& tmin > endtime) passed = false;

	if(w->getValue("display end", &tmax) && tmax > NULL_TIME_CHECK
		&& tmax < w->time) passed = false;
	if(!passed) {
	    table->selectRecord(w, false);
	    v.removeAt(i);
	}
    }

    if(global_start_time->getTime(&tmin))
    {
	for(i = v.size()-1; i >= 0; i--)
	{
	    w = (CssWfdiscClass *)v[i];
	    if(w->samprate > 0.) endtime = w->time + (w->nsamp-1)/w->samprate;
	    else endtime = w->endtime;
	    if(tmin > endtime) {
		table->selectRecord(w, false);
		v.removeAt(i);
	    }
	    else {
		w->putValue("global_start", tmin);
	    }
	}
    }

    if(global_end_time->getTime(&tmax))
    {
	for(i = v.size()-1; i >= 0; i--)
	{
	    w = (CssWfdiscClass *)v[i];
	    if(tmax < w->time) {
		table->selectRecord(w, false);
		v.removeAt(i);
	    }
	    else {
		w->putValue("global_end", tmax);
	    }
	}
    }
}

void TableQuery::applyOriginConstraints(CSSTable *table, gvector<CssTableClass *> &v)
{
    char **items;
    char *start_phase = NULL, *end_phase = NULL;
    string net, op;
    double az_min=0., az_max=360., dist_min=0., dist_max=1.e+6;
    double *lat=NULL, *lon=NULL, delta, az, baz, slow;
    double tmin=0., tmax=0., lead, lag, endtime;
    int i, j, num;
    CssWfdiscClass *w;
    gvector<CssTableClass *> *origins;
    gvector<CssTableClass *> *assocs=NULL, *wftags=NULL;
    CssOriginClass *o = NULL;
    WaveformWindow *ww;
    WaveformPlot *wp=NULL;
    bool do_az=false, do_dist=false, passed;

    if( (ww = waveform_receiver->getWaveformWindowInstance()) ) {
	wp = ww->wplot;
    }
    else {
	wp = waveform_receiver->getWaveformPlotInstance();
    }

    for(i = 0; i < v.size(); i++) {
        v[i]->removeValue("start_phase_time");
        v[i]->removeValue("end_phase_time");
    }

    if(azimuth_min->getDouble(&az_min) || azimuth_max->getDouble(&az_max)) {
	do_az = true;
    }
    if(distance_min->getDouble(&dist_min) || distance_max->getDouble(&dist_max))
    {
	do_dist = true;
    }

    if((num = start_phase_list->getSelectedItems(&items)) > 0) {
	start_phase = items[0];
	for(i = 1; i < num; i++) free(items[i]); // should never be more than 1
	free(items);
    }
    if(!parse_start_phase.empty()) { // from parseCmd
	Free(start_phase);
	start_phase = strdup(parse_start_phase.c_str());
    }

    if((num = end_phase_list->getSelectedItems(&items)) > 0) {
	end_phase = items[0];
	for(i = 1; i < num; i++) free(items[i]); // should never be more than 1
	free(items);
    }
    if(!parse_end_phase.empty()) { // from parseCmd
	Free(end_phase);
	end_phase = strdup(parse_end_phase.c_str());
    }

    if(!do_az && !do_dist && start_phase == NULL && end_phase == NULL) {
	return;
    }

    if((start_phase || end_phase) && !wp) {
	Free(start_phase); Free(end_phase);
	for(i = 0; i < v.size(); i++) {
	    table->selectRecord(v[i], false);
	    v.removeAt(i);
	}
	cerr << "Cannot compute travel times." << endl;
	return;
    }

    gvector<CssTableClass *> *sites = getTableRecords(cssSite, false);
    gvector<CssTableClass *> *aff = getTableRecords(cssAffiliation, false);
    gvector<CssTableClass *> *t = getTableRecords(cssOrigin, false);
    if( !sites || !sites->size() || !aff || !aff->size() || !t || !t->size()) {
	Free(start_phase); Free(end_phase);
	return;
    }
    origins = new gvector<CssTableClass *>(*t);
    CssTableClass::sort(*origins, "orid");

    lat = (double *)malloc(v.size()*sizeof(double));
    lon = (double *)malloc(v.size()*sizeof(double));
    getLatLon(*sites, v, lat, lon);

    t = getTableRecords(cssWftag, false);
    if(t && t->size() > 0) {
	wftags = new gvector<CssTableClass *>(*t);
	CssTableClass::sort(*wftags, "wfid");
    }

    gvector<CssTableClass *> *arrivals = getTableRecords(cssArrival, false);
    if(arrivals && arrivals->size() > 0)
    {
	t = getTableRecords(cssAssoc, false);
	if(t && t->size()) {
	    assocs = new gvector<CssTableClass *>(*t);
	    CssTableClass::sort(*assocs, "arid");
	}
    }

    for(i = v.size()-1; i >= 0; i--)
    {
	passed = true;
	w = (CssWfdiscClass *)v[i];
	if(lat[i] < -900. ||
	    !(o = getWfdiscOrigin(w, *origins, wftags, *arrivals, assocs)))
	{
	    table->selectRecord(w, false);
	    v.removeAt(i);
	    continue;
	}
	if(w->samprate > 0.) endtime = w->time + (w->nsamp-1)/w->samprate;
	else endtime = w->endtime;
	    
	deltaz(o->lat, o->lon, lat[i], lon[i], &delta, &az, &baz);

	if(do_az && (az < az_min || az > az_max)) passed = false;

	if(do_dist && (delta < dist_min || delta > dist_max)) passed =false;

	if(passed && (start_phase || end_phase))
	{
	    net.assign("");
	    /* get network */
	    for(j = 0; j < aff->size() &&
		((CssAffiliationClass *)aff->at(j))->sta_quark != w->sta_quark; j++);
	    if(j == aff->size()) {
		fprintf(stderr, "Cannot find network for %s\n", w->sta);
	    }
	    else {
		net.assign(((CssAffiliationClass *)aff->at(j))->net);
	    }

	    if(start_phase) {
		tmin = wp->getTravelTime(start_phase, o, lat[i], lon[i],
				0., net, w->sta, &slow, op);
		if(tmin > 0) {
		    tmin += o->time;
		    if(time_before_phase->getDouble(&lead)) tmin -= lead;
		    if(tmin > endtime) {
			passed = false;
		    }
		    else {
			w->putValue("start_phase_time", tmin);
		    }
		}
		else {
		    passed = false;
		    if(tmin == -2.) {
			showWarning("Unknown phase: %s", start_phase);
			Free(start_phase);
		    }
		}
	    }
	    if(end_phase) {
		tmax = wp->getTravelTime(end_phase, o, lat[i], lon[i],
				0., net, w->sta, &slow, op);
		if(tmax > 0.) {
		    tmax += o->time;
		    if(time_after_phase->getDouble(&lag)) tmax += lag;
		    if(tmax < w->time) {
			passed = false;
		    }
		    else {
			w->putValue("end_phase_time", tmax);
		    }
		}
		else {
		    passed = false;
		    if(tmax == -2.) {
			showWarning("Unknown phase: %s", end_phase);
			Free(end_phase);
		    }
		}
	    }
	}
	if(!passed) {
	    table->selectRecord(w, false);
	    v.removeAt(i);
	}
    }
    delete origins;
    if(wftags) delete wftags;
    if(assocs) delete assocs;
    Free(lat);
    Free(lon);
    Free(start_phase);
    Free(end_phase);
}

/* return an origin that is associated with the wfdisc, either through a
 * wftag, an arrival/assoc, or simply by time.
 */
CssOriginClass *TableQuery::getWfdiscOrigin(CssWfdiscClass *wfdisc,
		gvector<CssTableClass *> &origins, gvector<CssTableClass *> *wftags,
		gvector<CssTableClass *> &arrivals, gvector<CssTableClass *> *assocs)
{
    int i;
    string net;
    double start_time, end_time, t=0., two_hours;
    CssWftagClass *w;
    CssOriginClass *o;
    CssAssocClass *assoc;

    /* if wftags are available, search them first.
     */
    if(wftags)
    {
	if( (w = (CssWftagClass *)CssTableClass::find(*wftags, "wfid", wfdisc->wfid)) )
	{
	    if((o = (CssOriginClass *)CssTableClass::find(origins, "orid", w->tagid))) {
		return o;
	    }
	}
    }
    /* check for arrivals/assocs
     */

    net = getNet(wfdisc->sta);

    start_time = wfdisc->time;
    if(wfdisc->getValue("display start", &t) && t > NULL_TIME_CHECK
			&& t > start_time)
    {
	start_time = t;
    }

    if(wfdisc->samprate > 0.) {
	end_time = wfdisc->time + (wfdisc->nsamp-1)/wfdisc->samprate;
    }
    else {
	end_time = wfdisc->endtime;
    }
    if(wfdisc->getValue("display end", &t) && t > NULL_TIME_CHECK
			&& t < end_time)
    {
	end_time = t;
    }

    if(arrivals.size() && assocs)
    {
	for(i = 0; i < arrivals.size(); i++)
	{
	    CssArrivalClass *a = (CssArrivalClass *)arrivals[i];
	    if((!strcmp(a->sta, wfdisc->sta) || !strcmp(a->sta, net.c_str()))
		&& start_time <= a->time && a->time <= end_time)
	    {
		assoc = (CssAssocClass *)CssTableClass::find(*assocs, "arid", a->arid);
		if(assoc != NULL && (o = (CssOriginClass *)CssTableClass::find(
			origins, "orid", assoc->orid)) != NULL)
		{
		    return o;
		}
	    }
	}
    }

    /* check for origin with two hours.
     */
    two_hours = 2*60*60;
    for(i = 0; i < origins.size(); i++)
    {
	o = (CssOriginClass *)origins[i];
	if(o->time > start_time - two_hours && o->time < end_time)
	{
	    return o;
	}
    }
    return NULL;
}

void TableQuery::getLatLon(gvector<CssTableClass *> &sites,
		gvector<CssTableClass *> &wfdiscs, double *lat, double *lon)
{
    int i, j;
    gvector<CssTableClass *> *s = new gvector<CssTableClass *>(sites);

    s->sort(sortBySta);

    for(i = 0; i < wfdiscs.size(); i++)
    {
	CssWfdiscClass *w = (CssWfdiscClass *)wfdiscs[i];
	if((j = findSta(w->sta_quark, *s)) >= 0)
	{
	    lat[i] = ((CssSiteClass *)s->at(j))->lat;
	    lon[i] = ((CssSiteClass *)s->at(j))->lon;
	}
	else {
	    lat[i] = -999.;
	    lon[i] = -999.;
	}
    }
    delete s;
}

int TableQuery::findSta(int sta, gvector<CssTableClass *> &s)
{
    int n = s.size();
    int jl = -1, ju = n, jm;

    if(n <= 0 || sta < ((CssSiteClass *)s[0])->sta_quark
		|| sta > ((CssSiteClass *)s[n-1])->sta_quark)
    {
	return(-1);
    }
    if(sta == ((CssSiteClass *)s[0])->sta_quark) return(0);

    while(ju - jl > 1)
    {
	jm = (ju + jl)/2;

	if(sta > ((CssSiteClass *)s[jm])->sta_quark) {
	    jl = jm;
	}
	else {
	    ju = jm;
	}
    }
    return((sta == ((CssSiteClass *)s[jl+1])->sta_quark) ? jl+1 : -1);
}

// DataSource routine

bool TableQuery::getRecordsFromIds(const string &tableName,
		const string &idName, int n_table_ids, long *tableids,
		gvector<CssTableClass *> &records)
{
    int i, j, num_ids;
    long *ids;
    string table_map;
    char *account, buf[20];
    char query_string[MAX_Q_LEN];

    table_map = open_db->getMapping(tableName);
    if((account = open_db->getAccount()) == NULL) return false;

    /* sort unique ids
     */
    ids = new long[n_table_ids];
    memcpy(ids, tableids, n_table_ids*sizeof(long));

    qsort(ids, n_table_ids, sizeof(long), sortLong);

    num_ids = 0;
    for(i = 0; i < n_table_ids; i++) {
	if(i == 0) {
	    num_ids++;
	}
	else if(ids[i] != ids[i-1]) {
	    ids[num_ids++] = ids[i];
	}
    }

    /* do multiple queries for MAX_N_EXPR origerr's at a time
     */
    query_string[0] = '\0'; /* for num_ids=0 */

    for(i = j = 0; i < num_ids; i++)
    {
	if(j == 0) {
	    snprintf(query_string, MAX_Q_LEN,
		    "select * from %s%s%s where %s in (",
		    account, PERIOD(account), table_map.c_str(),idName.c_str());
	}
	snprintf(buf, 20, "%ld", ids[i]);
	if(strlen(query_string) + strlen(buf) + 2 > MAX_Q_LEN) {
	    showWarning("Maximum query length exceeded.");
	    break;
	}
	strcat(query_string, buf);

	if(j == MAX_N_EXPR-1 || i == num_ids-1)
	{
	    j = 0;
	    strcat(query_string, ")");

	    if(!runQuery(tableName, query_string, records)) break;
	}
	else {
	    j++;
	    strcat(query_string, ", ");
	}
    }
    XtFree(account);
    delete[] ids;

    return true;
}

void TableQuery::makeBeam(char *net, char *chan, double tbeg, double tend,
	long arid, cvector<CssWfdiscClass> &w, BeamRecipe &recipe,
	vector<BeamSta> &beam_sta, CssOriginClass *o, cvector<CssWfdiscClass> &wfdiscs)
{
    char missing[500];
    int i, j, k, n;
    GTimeSeries *ts;
    gvector<Waveform *> wvec, ws;
    vector<double> t_lags, weights;
    double delta, daz, baz, endtime;
    double lat = -999., lon = -999.;
    gvector<SegmentInfo *> *segs;

    if(w.size() <= 0) return;

    segs = new gvector<SegmentInfo *>;

    for(i = 0; i < w.size(); i++)
    {
	CssWfdiscClass *wf = w[i];
	if(wf->samprate > 0.) endtime = wf->time + (wf->nsamp-1)/wf->samprate;
	else endtime = wf->endtime;

	for(j = 0; j < (int)beam_sta.size(); j++) {
	    if( !strcmp(beam_sta[j].sta, wf->sta) &&
		compareChan(beam_sta[j].chan, wf->chan) &&
		wf->time < tend && endtime > tbeg) break;
	}
	if(j < (int)beam_sta.size())
	{
	    for(k = 0; k < (int)segs->size(); k++) {
		SegmentInfo *s = segs->at(k);
		if(!strcmp(s->sta, wf->sta) && compareChan(s->chan, wf->chan) &&
                        s->wfdisc()->time == wf->time) break;
	    }
	    if(k < segs->size()) continue; /* prevent duplicates */

	    SegmentInfo *s = new SegmentInfo();

	    s->id = wf->wfid;
	    stringcpy(s->sta, wf->sta, sizeof(s->sta));
	    stringcpy(s->chan, wf->chan, sizeof(s->chan));
	    if(!sourceIsODBC()) {
		s->path = wf->getFile();
		s->file_order = wf->filePosition();
/*		changeStaChan(widget, s->sta,sizeof(s->sta),
			 s->chan, sizeof(s->chan));
*/
	    }
	    s->start = wf->time;
	    s->jdate = timeEpochToJDate(s->start);
	    if(wf->samprate > 0.) {
		s->end = wf->time + (wf->nsamp-1)/wf->samprate;
	    }
	    else {
		s->end = wf->time + (wf->nsamp-1);
	    }

	    if(tbeg > s->start) {
		s->start = tbeg;
	    }
	    if(tend < s->end) {
		s->end = tend;
	    }
	    s->selected = true;
	    s->setWfdisc(new CssWfdiscClass(*wf));
	    s->hang = -999;
	    s->vang = -999;
	    segs->push_back(s);
	}
    }

    getNetworks(segs);
    getSites(segs);
    getSitechans(segs);

    WaveformConstraint cons;

    cons.readWaveforms(this, segs, wvec);

    for(i = 0; i < wvec.size(); i++) {
	wvec[i]->ts->addOwner(this);
    }

    delete segs;

    for(j = 0; j < (int)beam_sta.size(); j++) weights.push_back(0.);
    missing[0] = '\0';
    for(j = n = 0; j < (int)beam_sta.size(); j++) if(beam_sta[j].wgt != 0.)
    {
	for(i = 0; i < wvec.size(); i++)
	{
	    if( !strcasecmp(wvec[i]->sta(), beam_sta[j].sta) &&
		compareChan(wvec[i]->chan(), beam_sta[j].chan))
	    {
		ws.push_back(wvec[i]);
		weights[n] = beam_sta[j].wgt;
		n++;
		break;
	    }
	}
	if(i == wvec.size()) {
	    if(strlen(missing) + 3 + strlen(beam_sta[j].sta)
		+ strlen(beam_sta[j].chan) +1 < sizeof(missing))
	    {
		if(missing[0] != '\0') strcat(missing, ", ");
		strcat(missing, beam_sta[j].sta);
		strcat(missing, "/");
		strcat(missing, beam_sta[j].chan);
	    }
	}
    }

    if(missing[0] != '\0') {
	putWarning("Missing beam channels: %s.", missing);
    }
    if(n == 0) {
	for(i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts->removeOwner(this);
	}
	return;
    }

    if(o == NULL || o->lat < -900. || o->lon <= -900) {
	showWarning("No origin information.");
	for(i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts->removeOwner(this);
	}
	return;
    }
    for(j = 0; j < ws.size(); j++) {
	if(ws[j]->lat() > -900. && ws[j]->lon() > -900.) {
	    lat = ws[j]->lat();
	    lon = ws[j]->lon();
	    break;
	}
    }
    if(j == ws.size()) {
	showWarning("Missing station lat/lon.");
	for(i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts->removeOwner(this);
	}
	return;
    }
    deltaz(o->lat, o->lon, lat, lon, &delta, &daz, &baz);

    if( !Beam::getTimeLags(this, ws, baz, recipe.slow,
			REFERENCE_STATION, t_lags) )
    {
	for(i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts->removeOwner(this);
	}
	return;
    }

    if((ts = Beam::BeamTimeSeries(ws, t_lags, weights, true)) != NULL)
    {
	/* write the beam to a tmp file, so it can be filtered and
	 * unfiltered.
	 */
	GSourceInfo s;
	s.setSource(o);
	writeBeamDotw(s, net, chan, ts);

        vector<WfdiscPeriod> *p = ts->getWfdiscPeriods();

	if( !p )
	{
	    fprintf(stderr, "TableQuery.makeBeam: internal error.\n");
	    return;
	}
	for(i = 0; i < (int)p->size(); i++)
	{
	    CssWfdiscClass *wf = new CssWfdiscClass();
	    o->copySourceTo(wf, 1);
	    strcpy(wf->sta, p->at(i).wf.sta);
	    wf->sta_quark = stringUpperToQuark(wf->sta);
	    strcpy(wf->chan, p->at(i).wf.chan);
	    wf->chan_quark = stringUpperToQuark(wf->chan);
	    wf->time = p->at(i).wf.time;
	    wf->wfid = p->at(i).wf.wfid;
	    wf->chanid = p->at(i).wf.chanid;
	    wf->jdate = p->at(i).wf.jdate;
	    wf->endtime = p->at(i).wf.endtime;
	    wf->nsamp = p->at(i).wf.nsamp;
	    wf->samprate = p->at(i).wf.samprate;
	    wf->calib = p->at(i).wf.calib;
	    wf->calper = p->at(i).wf.calper;
	    strcpy(wf->instype, p->at(i).wf.instype);
	    strcpy(wf->segtype, p->at(i).wf.segtype);
	    strcpy(wf->datatype, p->at(i).wf.datatype);
	    strcpy(wf->clip, p->at(i).wf.clip);
	    strcpy(wf->dir, p->at(i).wf.dir);
	    strcpy(wf->dfile, p->at(i).wf.dfile);
	    wf->foff = p->at(i).wf.foff;
	    wf->commid = p->at(i).wf.commid;
	    wf->putValue("display start", tbeg);
	    wf->putValue("display end", tend);
	    wf->putValue("arid", arid);
	    wfdiscs.push_back(wf);
	}
    }

    for(i = 0; i < wvec.size(); i++) {
	wvec[i]->ts->removeOwner(this);
    }
}

bool TableQuery::numSelectedRows(const string &tableName)
{
    CSSTable *table;
    if(!(table = getCSSTable(tableName))) return 0;
    return table->numSelectedRows();
}

void TableQuery::getAllTables(void)
{
    getAllTables(false);
}

void TableQuery::getAllTables(bool static_tables)
{
    char query_string[100];
    const char **tnames = NULL;
    int num_table_names = CssTableClass::getAllNames(&tnames);
    CSSTable *table;
    gvector<CssTableClass *> v;

    if(open_db->sourceType() != PREFIX_CONNECTION) return;

    clearAllTabs();

    if(!static_tables) conn_handle.ffdb->setReadGlobalTables(false);

    for(int i = 0; i < num_table_names; i++)
	if(strncmp(tnames[i], "dynamic", 7)) // skip dynamic tables
    {
	const char *tableName = (const char *)tnames[i];

	snprintf(query_string, sizeof(query_string),
			"select * from %s", tableName);
	v.clear();
	runQuery(tableName, query_string, v);
	if(v.size() > 0) {
	    sortRows(v, tableName);
	    if(!(table = getCSSTable(tableName))) {
		    table = addTableTab(tableName);
	    }
	    else {
		    table->removeAllRecords();
	    }
	    table->addRecords(v);
	    setChange(v);
	    setQueryText(tableName, query_string);
	    if(!strcasecmp(tableName, cssWfdisc)) {
		wfdiscConstraintsInit();
	    }
	}
    }
    Free(tnames);

    if(!static_tables) conn_handle.ffdb->setReadGlobalTables(true);
}

void TableQuery::setTimeWindow(double tmin, double tmax, bool selected_only)
{
    if( selected_only ) {
	gvector<CssTableClass *> v;
	if(getSelectedTableRecords(cssWfdisc, v, false)) {
	    for(int i = 0; i < v.size(); i++) {
		v[i]->putValue("display start", tmin);
		v[i]->putValue("display end", tmax);
	    }
	}
    }
    else {
	gvector<CssTableClass *> *v = getTableRecords(cssWfdisc, false);
	if(v) {
	    for(int i = 0; i < v->size(); i++) {
		v->at(i)->putValue("display start", tmin);
		v->at(i)->putValue("display end", tmax);
	    }
	}
    }

}

void TableQuery::setArrivalTimeWindow(void)
{
    double time_before, time_after;

    if(!open_db->arrivalTimeBefore(&time_before)) {
	showWarning("Invalid time before.");
	return;
    }
    if(!open_db->arrivalTimeAfter(&time_after)) {
	showWarning("Invalid time after.");
	return;
    }
    setArrivalTimeWindow(time_before, time_after);
}

void TableQuery::setArrivalTimeWindow(double qtime)
{
    double time_before, time_after, start, end;
    gvector<CssTableClass *> v;

    if(!open_db->arrivalTimeBefore(&time_before)) {
	showWarning("Invalid time before.");
	return;
    }
    if(!open_db->arrivalTimeAfter(&time_after)) {
	showWarning("Invalid time after.");
	return;
    }
    if(!getSelectedTableRecords(cssWfdisc, v, false)) return;

    for(int i = 0; i < v.size(); i++)
    {
	if(time_before > NULL_TIME_CHECK) {
	    start = qtime - time_before;
	}
	else {
	    start = qtime - 300.;
	}
	if(time_after > NULL_TIME_CHECK) {
	    end = qtime + time_after;
	}
	else {
	    end = qtime + 300.;
	}
	v[i]->putValue("display start", start);
	v[i]->putValue("display end", end);
    }
}

void TableQuery::setArrivalTimeWindow(double time_before, double time_after)
{
    gvector<CssTableClass *> v;
    long arid;
    double start, end;

    gvector<CssTableClass *> *arrivals = getTableRecords(cssArrival, false);
    if(!arrivals || !arrivals->size()) return;
    if(!getSelectedTableRecords(cssWfdisc, v, false)) return;

    for(int i = 0; i < v.size(); i++) {
	arid = -1;
	v[i]->getValue("arid", &arid);
	for(int j = 0; j < arrivals->size(); j++) {
	    CssArrivalClass *a = (CssArrivalClass *)arrivals->at(j);
	    if(a->arid == arid) {
		if(time_before > NULL_TIME_CHECK) {
		    start = a->time - time_before;
		}
		else {
		    start = a->time - 300.;
		}
		if(time_after > NULL_TIME_CHECK) {
		    end = a->time + time_after;
		}
		else {
		    end = a->time + 300.;
		}
		v[i]->putValue("display start", start);
		v[i]->putValue("display end", end);
	    }
	}
    }
}

void TableQuery::setChange(CSSTable *table)
{
    if(table->numRows() > 0) {
	const char *name = table->getRecord(0)->getName();
	setChange(name);
    }
}

void TableQuery::setChange(gvector<CssTableClass *> &v)
{
    if(v.size() > 0) {
	setChange(v[0]->getName());
    }
}

void TableQuery::setChange(const string &name)
{
/*
    if(!name.compare(cssWfdisc)) change.select_waveform = true;
    else if(!name.compare(cssArrival)) change.select_arrival = true;
*/
    if(!name.compare(cssWfdisc)) change.waveform = true;
    else if(!name.compare(cssArrival)) change.arrival = true;
    else if(!name.compare(cssAssoc)) change.assoc = true;
    else if(!name.compare(cssOrigin)) change.origin = true;
    else if(!name.compare(cssOrigerr)) change.origerr = true;
    else if(!name.compare(cssStassoc)) change.stassoc = true;
    else if(!name.compare(cssStamag)) change.stamag = true;
    else if(!name.compare(cssNetmag)) change.netmag = true;
    else if(!name.compare(cssHydroFeatures)) change.hydro = true;
    else if(!name.compare(cssInfraFeatures)) change.infra = true;
    else if(!name.compare(cssWftag)) change.wftag = true;
    else if(!name.compare(cssAmplitude)) change.amplitude = true;
    else if(!name.compare(cssAmpdescript)) change.ampdescript = true;
    else if(!name.compare(cssFilter)) change.filter = true;
    else if(!name.compare(cssParrival)) change.parrival = true;
}

CssOriginClass *TableQuery::getPrimaryOrigin(Waveform *w)
{
printf("TableQuery.getPrimaryOrigin not implemented.\n");
    return NULL;
}

void TableQuery::addRow(void)
{
    CSSTable *table = tableOnTop();
    if( !table ) return;
    const char *name = table->getType();
    string table_name = open_db->getMapping(name);
    CssTableClass *css = CssTableClass::createCssTable(name);
    if(conn_handle.connection_type != NO_CONNECTION) {
        if( !writeTable(&conn_handle, css, table_name) ) {
            if(conn_handle.connection_type == ODBC_CONNECTION) {
#ifdef HAVE_LIBODBC
                showWarning(ODBCErrMsg());
#endif
            }
            else  {
                showWarning(FFDatabase::FFDBErrMsg());
            }
        }
	else {
	    css->setDataSource(this);
	}
    }
    table->addRecord(css, true);
}

// DataSource routine

CssSiteClass *TableQuery::getSite(const string &sta, int jdate)
{
    if(emptyTab(cssSite)) {
	getAllRows(cssSite);
    }
    return TableViewer::getSite(sta, jdate);
}

int TableQuery::getTable(const string &cssTableName, gvector<CssTableClass *> &v)
{
    if(!cssTableName.compare(cssSite)) {
	if(emptyTab(cssSite)) getAllRows(cssSite);
    }
    else if(!cssTableName.compare(cssSitechan)) {
	if(emptyTab(cssSitechan)) getAllRows(cssSitechan);
    }
    else if(!cssTableName.compare(cssAffiliation)) {
	if(emptyTab(cssAffiliation)) getAllRows(cssAffiliation);
    }
    else if(!cssTableName.compare(cssGregion)) {
	if(emptyTab(cssGregion)) getAllRows(cssGregion);
    }

    return TableViewer::getTable(cssTableName, v);
}

static int
sortBySta(const void *A, const void *B)
{
    CssSiteClass *a = *(CssSiteClass **)A;
    CssSiteClass *b = *(CssSiteClass **)B;
    return(a->sta_quark - b->sta_quark);
}

static int
sortByTime(const void *A, const void *B)
{
    CssWfdiscClass *a = *(CssWfdiscClass **)A;
    CssWfdiscClass *b = *(CssWfdiscClass **)B;
    register int r;
    double a_start_time, b_start_time;

    if(a->wfid != b->wfid) return (a->wfid - b->wfid);
    if((r = strcmp(a->sta, b->sta))) return r;
    if((r = strcmp(a->chan, b->chan))) return r;

    if(a->getValue("display start", &a_start_time) &&
       b->getValue("display start", &b_start_time))
    {
	if(a_start_time > b_start_time) return 1;
	else if(a_start_time < b_start_time) return -1;
    }
    return 0;
}

static int
sortByOrid(const void *A, const void *B)
{
    CssTableClass *a = *(CssTableClass **)A;
    CssTableClass *b = *(CssTableClass **)B;
    long a_orid = 0;
    long b_orid = 0;

    a->getValue("orid", &a_orid);
    b->getValue("orid", &b_orid);

    return(a_orid - b_orid);
}

static int
sortByDist(const void *A, const void *B)
{
    CssTableClass *a = *(CssTableClass **)A;
    CssTableClass *b = *(CssTableClass **)B;
    long a_orid = 0;
    long b_orid = 0;

    a->getValue("orid", &a_orid);
    b->getValue("orid", &b_orid);

    if(a_orid == b_orid) {
	double a_delta=0., b_delta=0.;
	a->getValue("delta", &a_delta);
	b->getValue("delta", &b_delta);
	if(a_delta != b_delta) return (a_delta > b_delta) ? 1 : -1;
	return 0;
    }
    return(a_orid - b_orid);
}

static int
sortLong(const void *A, const void *B)
{
    long *a = (long *)A;
    long *b = (long *)B;
    return(*a - *b);
}

#ifdef HAVE_LIBODBC
static int
ODBC_queryTableResults(ODBC_QueryTableStruct *qs, int numToFetch,
			int *numFetched, gvector<CssTableClass *> &table)
{
    int pos = table.size();
    int ret = ODBCQueryTableResults(qs, numToFetch, numFetched, table);

    // This should be in ODBCQueryTableResults. It is in CssTable_setMember,
    // which is used by CssTable_readFile.
    if(ret > 0 && table.size() > pos)
    {
	CssClassDescription *des = table[0]->description();
	int num_members = table[0]->getNumMembers();
	for(int i = 0; i < num_members; i++) {
	    if(des[i].quark_offset > 0) {
		for(int j = pos; j < table.size(); j++) {
		    char *o = (char *)table[j];
		    char *member_address = o + des[i].offset;
		    char *q = o + des[i].quark_offset;
		    *(int *)q = stringUpperToQuark(member_address);
		}
	    }
	}
    }
    return ret;
}
#endif
