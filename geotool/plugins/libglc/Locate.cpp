/** \file Locate.cpp
 *  \brief Defines class Locate.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <pwd.h>
#include <algorithm>

using namespace std;

#include "Locate.h"
#include "Details.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "Residuals.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
#include "locsat.h"
#include "ibase/libloc.h"
}

using namespace libglc;

static bool load_resources = true;

static long tmp_orid = -2;

static int num_acceptable_phases = 0;
static char **acceptable_phases = NULL;

static void addAcceptablePhases(void);
static bool isAcceptablePhase(char *p);
static void getStringValue(char *value, int maxlen, char *member_address,
		int type);
static int getAlignment(char *format);
static enum TimeFormat getTimeCode(char *format);
extern "C" {
static int sort_by_dist(const void *A, const void *B);
static int sort_by_time(const void *A, const void *B);
static int sort_by_sta_time(const void *A, const void *B);
};
static CssSiteClass * getArrivalSite(CssArrivalClass *a);
static bool isTrue(char *s);
static void getOridLabels(int num_orids, gvector<OriginData *> &origin_data,
		char **s);


Locate::Locate(const char *name, Component *parent, DataSource *ds) :
		Frame(name, parent), DataReceiver(NULL), Parser()
{
    setDataSource(ds);
    createInterface();
    init();
}

void Locate::createInterface()
{
    int n;
    Arg args[20];

    loadResources();

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    setSize(580, 670);

    file_menu = new Menu("File", menu_bar);
    locate_button = new Button("Locate", file_menu, this);
    new_locate_button = new Button("New Locate...", file_menu, this);
    reload_button = new Button("Reload", file_menu, this);
    auto_load = new Toggle("Auto-load", file_menu, this);
    auto_load->set(true);
    save_button = new Button("Save", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    remove_origins = new Button("Remove Selected Origins", edit_menu, this);
    remove_arrivals = new Button("Remove Selected Arrivals", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    origin_attributes_button = new Button("Origin Attributes...", view_menu,
				this);
    arrival_attributes_button = new Button("Arrival Attributes...", view_menu,
				this);
    location_details = new Button("Location Details...", view_menu, this);
    select_menu = new Menu("Select", view_menu);
    select_all_button = new Button("Select All", select_menu, this);
    deselect_all_button = new Button("Deselect All", select_menu, this);
    sort_menu = new Menu("Sort", view_menu);
    distance_time = new Button("distance/time", sort_menu, this);
    orid_distance_time = new Button("orid/distance/time", sort_menu, this);
    time_button = new Button("time", sort_menu, this);
    sta_time_button = new Button("sta/time", sort_menu, this);
    sort_by_column = new Button("Sort on Selected Columns", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    defining_menu = new Menu("Defining", option_menu);
    time_all_button = new Button("Time All", defining_menu, this);
    time_none_button = new Button("Time None", defining_menu, this);
    az_all_button = new Button("Azimuth All", defining_menu, this);
    az_none_button = new Button("Azimuth None", defining_menu, this);
    slow_all_button = new Button("Slowness All", defining_menu, this);
    slow_none_button = new Button("Slowness None", defining_menu, this);
    method_menu = new Menu("Method", option_menu);
    colors_button = new Button("Residual Colors...", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Locate Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNtableInfoWidget, info_area->leftInfo()); n++;
    XtSetArg(args[n], XtNcursorInfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 5); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNtableTitle, "Origins"); n++;
    XtSetArg(args[n], XmNpaneMinimum, 100); n++;
    XtSetArg(args[n], XtNcolumns, 16); n++;
    origin_table = new MultiTable("origin_table", pane, args, n);
//    origin_table->setAttributes();
    origin_table->addActionListener(this, XtNselectRowCallback);
    origin_table->addActionListener(this, XtNattributeChangeCallback);
    origin_table->addActionListener(this, XtNeditSaveCallback);
 
    n = 0;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 100); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNtableTitle,
	" Arrivals   (P = phase, T = timedef, A = azdef, S = slodef)"); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XmNpaneMinimum, 100); n++;
    XtSetArg(args[n], XtNcolumns, 16); n++;
    arrival_table = new Table("arrival_table", pane, info_area, args, n);
//    arrival_table->setAttributes();
 
    arrival_table->addActionListener(this, XtNselectRowCallback);
    arrival_table->addActionListener(this, XtNattributeChangeCallback);
    arrival_table->addActionListener(this, XtNeditSaveCallback);
    arrival_table->addActionListener(this, XtNcolumnMovedCallback);

    n = 0;
    XtSetArg(args[n], XmNpaneMinimum, 100); n++;
    form = new Form("form", pane, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Location Parameters", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNborderWidth, 0); n++;
    tab = new TabClass("tab", form, args, n);
    tab->addActionListener(this, XtNtabCallback);

    residuals_window = new Residuals("Residual Colors", this);

    enableCallbackType(XtNdataChangeCallback);

    addPlugins("Locate", this, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(locate_button, "Locate");
	tool_bar->add(time_all_button, "T-d");
	tool_bar->add(time_none_button, "T-n");
	tool_bar->add(az_all_button, "A-d");
	tool_bar->add(az_none_button, "A-n");
	tool_bar->add(slow_all_button, "S-d");
	tool_bar->add(slow_none_button, "S-n");
	tool_bar->add(reload_button, "Reload");
	tool_bar->add(sort_by_column, "Sort");
	tool_bar->add(save_button, "Save");
    }
}

void Locate::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
	if(data_source) {
	    data_source->removeDataListener(this);
	    data_source->removeDataReceiver(this);
        }
        data_source = ds;
        if(data_source) {
	    data_source->addDataListener(this);
            data_source->addDataReceiver(this);
	    tv = data_source->getTableViewerInstance();
        }
        else {
	    tv = NULL;
	}
    }
}

Locate::~Locate(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void Locate::init(void)
{
    methods = (LocationMethod *)mallocWarn(sizeof(LocationMethod));
    num_methods = 0;
    location_status = -1;

    methods[0].name = strdup("LocSAT");
    methods[0].toggle = NULL;
    methods[0].shared_lib =
	strdup("libinterp.so:libLP.so:libloc.so:libgmath.so");
    methods[0].function_name = strdup("locsat");
    methods[0].init_function_name = strdup("initLocSAT");
    methods[0].locFunction = NULL;
    methods[0].locInitFunction = NULL;
    methods[0].param_table = NULL;
    if( !getLocFunctions(&methods[0]) ) {
	free(methods[0].name);
	free(methods[0].shared_lib);
	free(methods[0].function_name);
	free(methods[0].init_function_name);
    }
    else {
	num_methods = 1;
    }

    /* set vmodel_spec_file and sasc_dir_prefix from GEOTOOL_HOME if necessary
     */
    char *geotool_home;
    string prop;
    if(!getProperty("locate.LocSAT.vmodel_spec_file", prop)) {
	if((geotool_home = (char *)getenv("GEOTOOL_HOME")) != NULL) {
	    prop.assign(geotool_home+string("/tables/data/TT/vmsf/idc.defs"));
	    putProperty("locate.LocSAT.vmodel_spec_file", prop, false, this);
	}
    }

    if(!getProperty("locate.LocSAT.sasc_dir_prefix", prop)) {
        if((geotool_home = (char *)getenv("GEOTOOL_HOME")) != NULL) {
	    prop.assign(geotool_home+string("/tables/data/SASC/sasc"));
	    putProperty("locate.LocSAT.sasc_dir_prefix", prop, false, this);
	}
    }

    /* look for dynamic location methods
     */
    getDynamicMethods();

    for(int i = 0; i < num_methods; i++)
    {
	methods[i].toggle = new Toggle(methods[i].name, method_menu, this);
	if(i == 0) methods[i].toggle->set(true);
	createParamTab(&methods[i]);
    }

    addAcceptablePhases();

    const char *names[]= {cssOrigin, cssOrigerr};
    const char *extra[] = {"file", "file_index"};
    const char *extra_format[] = {"%s", "%d"};
    const char *origin_display = "orid,lat,lon,depth,time,%t";

    if(getProperty("locate.origin_table.attributes", prop)) {
	origin_table->setType(2, names, 2, extra, extra_format, prop);
    }
    else {
	origin_table->setType(2, names, 2, extra, extra_format, origin_display);
    }

    const char *arrival_list =
"sta,%s, time,%t, arid,%d, jdate,%d, stassid,%d, chanid,%d, chan,%s, \
iphase,%s, stype,%s, deltim,%.3f, azimuth,%.2f, delaz,%.2f, slow,%.2f, \
delslo,%.2f,ema,%.2f,rect,%.3f,amp,%.2f,per,%.2f,amptype,%s,logat,%.2f,clip,%s,\
fm,%s, snr,%.2f, qual,%s, auth,%s, commid,%d, lddate,%s, orid,%d, phase,%s, \
belief,%.2f, delta,%.2f, seaz,%.2f, esaz,%.2f, timeres,%.2f, timedef,%s, \
azres,%.2f, azdef,%s, slores,%.2f, slodef,%s, emares,%.2f, wgt,%.2f, vmodel,%s,\
file,%s,file_index,%d";

    const char *arrival_display = "sta, phase, timeres, timedef, azimuth,\
azres, azdef, slow, slores, slodef, delta, time";

    if(getProperty("locate.arrival_table.attributes", prop)) {
	arrival_attributes = new TableAttributes("arrivalTable", this,
					arrival_list, prop);
    }
    else {
	arrival_attributes = new TableAttributes("arrivalTable", this,
					arrival_list, arrival_display);
    }
    arrival_attributes->addActionListener(this);
}

static void
addAcceptablePhases(void)
{
    char *c, *last, *prop, *tok, buf[2000];
    const char *defaultList =
"H,I,Lg,P,P3KP,P3KPbc,P3KPbc_B,P3KPdf,P3KPdf_B,P4KP,P4KPbc,P4KPdf,P4KPdf_B,\
P5KP,P5KPbc,P5KPbc_B,P5KPdf,P5KPdf_B,P5KPdf_C,P7KP,P7KPbc,P7KPbc_B,P7KPbc_C,\
P7KPdf,P7KPdf_B,P7KPdf_C,P7KPdf_D,Pb,PcP,PcS,Pdiff,Pg,PKiKP,PKKP,PKKPab,PKKPbc,\
PKKPdf,PKKS,PKKSab,PKKSbc,PKKSdf,PKP,PKP2,PKP2ab,PKP2bc,PKP2df,PKP3ab,PKP3bc,\
PKP3df,PKP3df_B,PKPab,PKPbc,PKPdf,PKPPKP,PKS,PKSab,PKSbc,PKSdf,Pn,PnPn,pP,\
PP,PP_B,pPdiff,pPKiKP,pPKP,pPKPab,pPKPbc,pPKPdf,PPP,PPP_B,PPS,PPS_B,pS,PS,\
pSdiff,pSKS,pSKSac,pSKSdf,Rg,S,Sb,ScP,ScS,Sdiff,SKiKP,SKKP,SKKPab,SKKPbc,\
SKKPdf,SKKS,SKKSac,SKKSdf,SKP,SKPab,SKPbc,SKPdf,SKS,SKS2,SKS2ac,SKS2df,SKSac,\
SKSdf,SKSSKS,Sn,SnSn,sP,SP,sPdiff,sPKiKP,sPKP,sPKPab,sPKPbc,sPKPdf,sS,SS,SS_B,\
sSdiff,sSKS,sSKSac,sSKSdf,SSS,SSS_B,T,T_B";

    if (num_acceptable_phases > 0) return;

    stringcpy(buf, "locate.acceptablePhases", sizeof(buf));
    if((prop = Application::getProperty(buf)) == NULL)
    {
	prop = strdup(defaultList);
    }
    tok = prop;

    num_acceptable_phases = 0;
    acceptable_phases = (char **)malloc(sizeof(char *));

    while((c = strtok_r(tok, ",", &last)) != (char *)NULL) {
	tok = (char *)NULL;
	acceptable_phases = (char **)realloc(acceptable_phases,
				(num_acceptable_phases+1)*sizeof(char *));
	acceptable_phases[num_acceptable_phases++] = strdup(stringTrim(c));
    }
    free(prop);
}

static bool
isAcceptablePhase(char *p)
{
    for(int i = 0; i < num_acceptable_phases; i++) {
	if(!strcmp(p, acceptable_phases[i])) return true;
    }
    return false;
}

void Locate::setVisible(bool visible)
{
    Frame::setVisible(visible);

    if( !visible ) {
	origin_data.clear();
	arrival_data.clear();
	initializeParams();
    }
    else {
	loadData();
    }
}

void Locate::reload(void)
{
    origin_data.clear();
    arrival_data.clear();
    arrival_table->removeAllRows();
    initializeParams();
    loadData();
}

void Locate::loadData(void)
{
    if( tv ) {
	loadFromTableQuery(tv);
    }
    else {
	loadTables();
    }

    listOriginData();

    sortArrivals("orid/distance/time");

    for(int i = 0; i < arrival_data.size(); i++)
    {
	ArrivalData *a = arrival_data[i];
	if(a->assocs.size() > 0)
	{
	    CssAssocClass *assoc = a->assocs[0];
	    stringcpy(a->phase, assoc->phase, sizeof(a->phase));
	    stringcpy(a->timedef, assoc->timedef, sizeof(a->timedef));
	    stringcpy(a->azdef, assoc->azdef, sizeof(a->azdef));
	    stringcpy(a->slodef, assoc->slodef, sizeof(a->slodef));
	}
	else {
	    stringcpy(a->phase, a->arrival->iphase, sizeof(a->phase));
	    stringcpy(a->timedef, "d", sizeof(a->timedef));
	    stringcpy(a->azdef,   "n", sizeof(a->azdef));
	    stringcpy(a->slodef,  "n", sizeof(a->slodef));
	}
    }
    listArrivalData();
}

static CssSiteClass *
getArrivalSite(CssArrivalClass *a)
{
    DataSource *ds = a->getDataSource();
    if(!ds) return NULL;

    return ds->getSite(a->sta, a->jdate);
}

void Locate::initializeParams(void)
{
    const char **labels = NULL;
    Table *table;

    int num_tab = tab->getLabels((char ***)&labels);
    for(int i = 0; i < num_tab; i++)
    {
	if( !(table = findTable(labels[i])) ) return;

	table->removeAllRows();

	CssTableClass *css = CssTableClass::createCssTable(labels[i]);
	getDefaultParams(css);
	addParamLine(css, 0);	// add the default (top) line
	css->deleteObject();
    }
    Free(labels);
}

Table * Locate::findTable(const char *name)
{
    for(int i = 0; i < num_methods; i++) {
	if( !strcmp(methods[i].name, name) ) {
	    return methods[i].param_table;
	}
    }
    return NULL; // should not happen
}

void Locate::loadFromTableQuery(TableViewer *table_viewer)
{
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssAssocClass> assocs;
    cvector<CssArrivalClass> arrivals;

    if(!table_viewer->getTabName().compare(cssOrigin)) {
	loadFromOriginTab(table_viewer, origins, origerrs, assocs, arrivals);
    }
    else if(!table_viewer->getTabName().compare(cssArrival)) {
	table_viewer->getSelectedTable(cssArrival, arrivals);
    }

    if(arrivals.size()) {
	loadStructs(origins, origerrs, assocs, arrivals);
    }
}

int Locate::loadFromOriginTab(TableViewer *table_viewer,
		cvector<CssOriginClass> &origins, cvector<CssOrigerrClass> &origerrs,
		cvector<CssAssocClass> &assocs, cvector<CssArrivalClass> &arrivals)
{
    long *ids = NULL;

/* depending on the location config, either use orids/assocs to get arrivals,
or use time limits to get arrivals.
*/
    table_viewer->getSelectedTable(cssOrigin, origins);
    if(origins.size() <= 0) return 0;

    if(!(ids = (long *)mallocWarn(origins.size()*sizeof(long)))) return 0;

    for(int i = 0; i < origins.size(); i++) {
	ids[i] = origins[i]->orid;
    }

    setCursor("hourglass");

    table_viewer->getRecordsFromIds(cssOrigerr, "orid", origins.size(), ids,
				origerrs);
    table_viewer->getRecordsFromIds(cssAssoc, "orid", origins.size(), ids,
				assocs);

    Free(ids);
    if(assocs.size() &&
	!(ids =(long *)mallocWarn(assocs.size()*sizeof(long)))) return 0;

    for(int i = 0; i < assocs.size(); i++) {
	ids[i] = assocs[i]->arid;
    }
    table_viewer->getRecordsFromIds(cssArrival, "arid", assocs.size(), ids,
				arrivals);
    Free(ids);

    setCursor("default");

    return origins.size();
}

void Locate::loadTables(void)
{
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;

    if( data_source->getSelectedTable(origins) <= 0 ) {
	data_source->getTable(origins);
    }
    data_source->getTable(origerrs);
    data_source->getTable(arrivals);
    data_source->getTable(assocs);

    loadStructs(origins, origerrs, assocs, arrivals);
}

void Locate::loadStructs(cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssArrivalClass> &arrivals)
{
    CssOrigerrClass *oe;
    CssSiteClass *site;
    OriginData *o;
    ArrivalData *a;
    int i, j;

    for(i = 0; i < origins.size(); i++)
    {
	if(haveOrid(origins[i]->orid)) continue;

	for(j = 0; j < origerrs.size() && origins[i]->orid != origerrs[j]->orid;
			j++);
	if(j < origerrs.size()) {
	    oe = origerrs[j];
	}
	else {
	    oe = new CssOrigerrClass();
	    origins[i]->copySourceTo(oe, origins[i]->orid);
	}
	o = new OriginData(origins[i], oe, NULL);

	if(origins[i]->orid < -1000 || origins[i]->orid > -1) {
	    o->saved = true;
	}
	else {
	    o->saved = false;
	}
	origin_data.add(o);
	origins[i]->setSelected(false);
    }

    for(i = 0; i < arrivals.size(); i++)
    {
	if(haveArid(arrivals[i]->arid)) continue;

	TableListener::addListener(arrivals[i], this);

	a = new ArrivalData(arrivals[i]);
	arrival_data.add(a);
	for(j = 0; j < assocs.size(); j++)
	{
	    if(arrivals[i]->arid == assocs[j]->arid)
	    {
		a->putAssoc(assocs[j]);
	    }
	}
	if((site = getArrivalSite(arrivals[i])) != NULL) {
	    a->site = site;
	    if(!sites.contains(site)) {
		sites.push_back(site);
	    }
	}
    }
}

bool Locate::haveArid(int arid)
{
    for(int i = 0; i < arrival_data.size(); i++)
    {
	CssArrivalClass *a = arrival_data[i]->arrival;
	if(a->arid == arid) {
	    return true;
	}
    }
    return false;
}

bool Locate::haveOrid(int orid)
{
    for(int i = 0; i < origin_data.size(); i++)
    {
   	CssOriginClass *o = origin_data[i]->origin;
	if(o->orid == orid) {
	    return true;
	}
    }
    return false;
}


void Locate::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    resetDataChange(false);

    for(int i = 0; i < num_methods; i++) {
	if(comp == methods[i].toggle) {
	    if(methods[i].toggle->state()) {
		tab->setOnTop(methods[i].name);
	    }
	    return;
	}
    }

    if(!strcmp(reason, XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(auto_load->state() && (c->arrival || c->assoc || c->origin)) {
	    reload();
	}
    }
    else if(!strcmp(reason, CssTableChange)) {
	TableListenerCallback *tc =
	    (TableListenerCallback *)action_event->getCalldata();
	if(!strcasecmp(tc->table->getName(), cssArrival)) {
	    arrivalChange(tc);
	}
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Locate Help")) {
	showHelp(cmd);
    }
    else if(!strcmp(cmd, "Origin Attributes...")) {
	origin_table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Arrival Attributes...")) {
	arrival_attributes->setVisible(true);
    }
    else if(!strcmp(cmd, "Remove Selected Origins")) {
	removeSelectedOrigin();
	change.origin = true;
	change.origerr = true;
    }
    else if(!strcmp(cmd, "Remove Selected Arrivals")) {
	removeSelectedArrival();
	change.arrival = true;
	change.assoc = true;
    }
    else if(!strcmp(cmd, "Location Details...")) {
	showDetails();
    }
    else if(!strcmp(reason, XtNcolumnMovedCallback)) {
	columnMoved();
    }
    else if(comp == arrival_attributes) {
	listArrivalData();
    }
    else if(comp == origin_table) {
	if(!strcmp(reason, XtNselectRowCallback)) {
	    selectOrigin();
	    listArrivalData();
	}
    }
    else if(!strcmp(reason, XtNtabCallback)) { // tab select
	char *name = tab->labelOnTop();
	for(int i = 0; i < num_methods; i++) {
	    if(!strcmp(methods[i].name, name)) {
		methods[i].toggle->set(true);
	    }
	}
    }
    else if(!strcmp(reason, XtNchoiceChangedCallback)) { // param_table choice
	Table *t = comp->getTableInstance();
	if(t) paramChoice(t,
		(MmTableEditCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "distance/time") || !strcmp(cmd, "orid/distance/time")
		|| !strcmp(cmd, "time") || !strcmp(cmd, "sta/time"))
    {
	sortArrivals(cmd);
    }
    else if(!strcmp(cmd, "sort selected-columns")) {
	vector<int> cols;
	arrival_table->getSelectedColumns(cols);
	arrival_table->sortByColumns(cols);
    }
    else if(!strcmp(cmd, "Locate")) {
	locateEvents(true);
	change.origin = true;
	change.origerr = true;
    }
    else if(!strcmp(cmd, "Select All")) {
	arrival_table->selectAllRows(true);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	arrival_table->selectAllRows(false);
    }
    else if(!strcmp(cmd, "Time All")) {
	setDefining(1, "d");
    }
    else if(!strcmp(cmd, "Time None")) {
	setDefining(1, "n");
    }
    else if(!strcmp(cmd, "Azimuth All")) {
	setDefining(2, "d");
    }
    else if(!strcmp(cmd, "Azimuth None")) {
	setDefining(2, "n");
    }
    else if(!strcmp(cmd, "Slowness All")) {
	setDefining(3, "d");
    }
    else if(!strcmp(cmd, "Slowness None")) {
	setDefining(3, "n");
    }
    else if(!strcmp(cmd, "Save")) {
	saveOrigins();
    }
    else if(!strcmp(cmd, "Residual Colors...")) {
	if(!residuals_window) {
	    residuals_window = new Residuals("Residual Colors", this);
	}
	residuals_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Reload")) {
	reload();
    }
    else if(!strcmp(cmd, "New Locate..."))
    {
        int i=0;
        for(i = 0; i < (int)windows.size(); i++) {
            if( !windows[i]->isVisible() ) {
                windows[i]->setVisible(true);
                break;
            }
        }
        if(i == (int)windows.size()) {
            Locate *loc = new Locate(getName(), this, data_source);
	    windows.push_back(loc);
	    loc->setVisible(true);
        }
    }
    if(dataChange()) {
	doCallbacks(baseWidget(), (XtPointer)&change, XtNdataChangeCallback);
    }
}

ParseCmd Locate::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;
    int i = -1;
    char *name = tab->labelOnTop();

    if(name) {
	for(i = 0; i < num_methods; i++) {
	    if(!strcmp(methods[i].name, name)) break;
	}
    }

    if(parseArg(cmd, "origin_attributes", c)) {
	return origin_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "arrival_attributes", c)) {
	return arrival_attributes->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "remove_selected_origns")) {
	remove_origins->activate();
    }
    else if(parseCompare(cmd, "remove_selected_arrivals")) {
	remove_arrivals->activate();
    }
/*
    else if(parseArg(cmd, "Location Details", c, sizeof(c))) {
	showDetails();
    }
*/
    else if(parseArg(cmd, "sort", c)) {
	if(parseCompare(c, "distance/time")) {
	    distance_time->activate();
	}
	else if(parseCompare(c, "orid/distance/time")) {
	    orid_distance_time->activate();
	}
	else if(parseCompare(c, "time")) {
	    time_button->activate();
	}
	else if(parseCompare(c, "sta/time")) {
	    sta_time_button->activate();
	}
	else if(parseCompare(c, "selected_columns")) {
	    sort_by_column->activate();
	}
	else {
	    msg.assign(string("sort: unknown type: ") + c);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseCompare(cmd, "locate")) {
	if( !isVisible() && arrival_data.size() == 0 ) {
	    loadData();
	}
	locate_button->activate();
    }
    else if(parseCompare(cmd, "select_all")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "deselect_all")) {
	deselect_all_button->activate();
    }
    else if(parseCompare(cmd, "define_time_all")) {
	time_all_button->activate();
    }
    else if(parseCompare(cmd, "define_time_none")) {
	time_none_button->activate();
    }
    else if(parseCompare(cmd, "define_azimuth_all")) {
	az_all_button->activate();
    }
    else if(parseCompare(cmd, "define_azimuth_none")) {
	az_none_button->activate();
    }
    else if(parseCompare(cmd, "define_slowness_all")) {
	slow_all_button->activate();
    }
    else if(parseCompare(cmd, "define_slowness_none")) {
	slow_none_button->activate();
    }
    else if(parseCompare(cmd, "save")) {
	saveOrigins();
    }
    else if(parseArg(cmd, "residual_colors", c)) {
	if(!residuals_window) {
	    residuals_window = new Residuals("Residual Colors", this);
	}
	return residuals_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "reload")) {
	reload_button->activate();
    }
    else if(parseArg(cmd, "origins", c)) {
	return origin_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "arrivals", c)) {
	return arrival_table->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "parameters", c)) {
	if(i >= 0 && i < num_methods) {
	    return methods[i].param_table->parseCmd(c, msg);
	}
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = dataParseCmd(cmd.c_str(), msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if( (ret = pluginParse(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else {
	return Frame::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar Locate::parseVar(const string &name, string &value)
{
    ParseVar ret;
    int i = -1;
    string c;
    char *param_tab = tab->labelOnTop();

    if(param_tab) {
	for(i = 0; i < num_methods; i++) {
	    if(!strcmp(methods[i].name, param_tab)) break;
	}
    }

    if(parseCompare(name, "status")) {
	parsePrintInt(value, location_status);
    }
    else if(parseArg(name, "origins", c)) {
	return origin_table->parseVar(c, value);
    }
    else if((ret=parseArrivalData(name, value)) != VARIABLE_NOT_FOUND){
	return ret;
    }
    else if(parseArg(name, "arrivals", c)) {
	return arrival_table->parseVar(c, value);
    }
    else if(parseArg(name, "parameters", c)) {
	if(i >= 0 && i < num_methods) {
	    return methods[i].param_table->parseVar(c, value);
	}
    }
    else if((ret = dataParse(name, value)) != VARIABLE_NOT_FOUND) {
        return ret;
    }
    else {
	return VARIABLE_NOT_FOUND;
    }
    return STRING_RETURNED;
}

ParseVar Locate::parseArrivalData(const string &name, string &value)
{
    int i, num, nextc;
    vector<int> rows;
    ParseVar ret;

    if(!parseCompare(name, "arrival_data", 12) &&
		!parseCompare(name, "sel_arrival_data", 16))
    {
        return VARIABLE_NOT_FOUND;
    }
    else if(parseCompare(name, "arrival_data.size_")) {
        num = arrival_data.size();
	parsePrintInt(value, num);
        return STRING_RETURNED;
    }
    else if(parseCompare(name, "sel_arrival_data.size_")) {
	num = arrival_table->getSelectedRows(rows);
	parsePrintInt(value, num);
        return STRING_RETURNED;
    }

    num = arrival_data.size();
    if(!parseArrayIndex(name, "arrival_data", num, &i, &nextc, value, &ret))
    {
	num = arrival_table->getSelectedRows(rows);
	if(!parseArrayIndex(name, "sel_arrival_data", num, &i,
			&nextc, value, &ret))
	{
	    return ret;
	}
	i = rows[i];
    }
    if(ret == STRING_RETURNED) {
	const char *c = name.c_str() + nextc;
	ArrivalData *a = arrival_data[i];
	if(*c == '.') c++;
	if(!strcasecmp(c, "phase")) {
	    value.assign(a->phase);
	    return STRING_RETURNED;
	}
	else if(!strcasecmp(c, "timedef")) {
	    value.assign(a->timedef);
	    return STRING_RETURNED;
	}
	else if(!strcasecmp(c, "azdef")) {
	    value.assign(a->azdef);
	    return STRING_RETURNED;
	}
	else if(!strcasecmp(c, "slodef")) {
	    value.assign(a->slodef);
	    return STRING_RETURNED;
	}
	else if(getTabMember(c, a->site, value)) {
	    return STRING_RETURNED;
	}
	else if(getTabMember(c, a->arrival, value)) {
	    return STRING_RETURNED;
	}
	return VARIABLE_NOT_FOUND;
    }
    return ret;
}

void Locate::parseHelp(const char *prefix)
{
    printf("%sremove_selected_origins\n", prefix);
    printf("%sremove_selected_arrivals\n", prefix);
    printf("%ssort=(distance/time,orid/distance/time,time,sta/time)\n", prefix);
    printf("%ssort_selected_columns\n", prefix);
    printf("%slocate\n", prefix);
    printf("%sselect_all\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sdefine_azimuth_all\n", prefix);
    printf("%sdefine_azimuth_none\n", prefix);
    printf("%sdefine_slowness_all\n", prefix);
    printf("%sdefine_slowness_none\n", prefix);
    printf("%sdefine_time_all\n", prefix);
    printf("%sdefine_time_none\n", prefix);
    printf("%sreload\n", prefix);
    char p[200];
    snprintf(p, sizeof(p), "%sorigins.", prefix);
    Table::parseHelp(p);
    snprintf(p, sizeof(p), "%sarrivals.", prefix);
    Table::parseHelp(p);

    printf("%sparameters.set_cell row=\"COLUMN_LABEL=VALUE...\"\
 column=COLUMN_LABEL value=CELL_VALUE\n", prefix);

    printf("%sorigin_attributes.help\n", prefix);
    printf("%sarrival_attributes.help\n", prefix);
}

void Locate::paramChoice(Table *table, MmTableEditCallbackStruct *c)
{
    vector<const char *> labels;

    if( !table->getColumnLabels(labels) ) return;

    if(!strcasecmp(labels[c->column], "use_location") && isTrue(c->new_string))
    {
	updateInitialLocation();
    }
}

static bool
isTrue(char *s)
{
    return (s[0] == '1' || s[0] == 'T' || s[0] == 't') ? True : False;
}

void Locate::setDefining(int col_index, const char *defining)
{
    int num = arrival_table->numRows();
    const char **col = NULL;

    if(!(col = (const char **)mallocWarn(num*sizeof(char *)))) return;
    for(int i = 0; i < num; i++) col[i] = defining;

    arrival_table->setColumn(col_index, col, num);
}

void Locate::getDynamicMethods(void)
{
    char *c, *last, *prop, *fname, *initname, *lib, *name, buf[200], *type;
    XrmDatabase database;
    XrmValue v1, v2, v3, v4;

    stringcpy(buf, "locate.methods", sizeof(buf));
    if((prop = getProperty(buf)) != NULL)
    {
	c = prop;
	while((name = strtok_r(c, ",\t ", &last)) != NULL)
	{
	    c = NULL;
	    methods = (LocationMethod *)reallocWarn(methods,
				(num_methods+1)*sizeof(LocationMethod));
	    snprintf(buf, sizeof(buf), "locate.%s.sharedLib", name);
	    if((lib = getProperty(buf)) != NULL)
	    {
		snprintf(buf, sizeof(buf), "locate.%s.locFunction", name);
		if( (fname = getProperty(buf)) != NULL)
		{
		    snprintf(buf, sizeof(buf),"locate.%s.locInitFunction",name);
		    if( (initname = getProperty(buf)) != NULL)
		    {
			methods[num_methods].name = strdup(name);
			methods[num_methods].toggle = NULL;
			methods[num_methods].shared_lib = strdup(lib);
			methods[num_methods].function_name = strdup(fname);
			methods[num_methods].init_function_name
				= strdup(initname);
			methods[num_methods].locFunction = NULL;
			methods[num_methods].locInitFunction = NULL;
			methods[num_methods].param_table = NULL;
			if( !getLocFunctions(&methods[num_methods]) ) {
			    free(methods[num_methods].name);
			    free(methods[num_methods].shared_lib);
			    free(methods[num_methods].function_name);
			    free(methods[num_methods].init_function_name);
			}
			else {
			    num_methods++;
			}
			free(initname);
		    }
		    free(fname);
		}
		free(lib);
	    }
	}
	free(prop);
	return;
    }

    database = XrmGetDatabase(XtDisplay(base_widget));

    stringcpy(buf, "Geotool.locate.methods", sizeof(buf));
    if(XrmGetResource(database, buf, buf, &type, &v1))
    {
	prop = strdup(v1.addr);
	c = prop;
	while((name = strtok_r(c, ",\t ", &last)) != NULL)
	{
	    c = NULL;
	    methods = (LocationMethod *)reallocWarn(methods,
				(num_methods+1)*sizeof(LocationMethod));
	    snprintf(buf, sizeof(buf), "Geotool.locate.%s.sharedLib", name);
	    if(XrmGetResource(database, buf, buf, &type, &v2))
	    {
		snprintf(buf,sizeof(buf),"Geotool.locate.%s.locFunction",name);
		if(XrmGetResource(database, buf, buf, &type, &v3))
		{
		    snprintf(buf, sizeof(buf),
				"Geotool.locate.%s.locInitFunction", name);
		    if(XrmGetResource(database, buf, buf, &type, &v4))
		    {
			methods[num_methods].name = strdup(name);
			methods[num_methods].toggle = NULL;
			methods[num_methods].shared_lib = strdup(v2.addr);
			methods[num_methods].function_name =strdup(v3.addr);
			methods[num_methods].init_function_name =
					strdup(v4.addr);
			methods[num_methods].locFunction = NULL;
			methods[num_methods].locInitFunction = NULL;
			methods[num_methods].param_table = NULL;
			if( !getLocFunctions(&methods[num_methods]) ) {
			    free(methods[num_methods].name);
			    free(methods[num_methods].shared_lib);
			    free(methods[num_methods].function_name);
			    free(methods[num_methods].init_function_name);
			}
			else {
			    num_methods++;
			}
		    }
		}
	    }
	}
	free(prop);
    }
}

bool Locate::getLocFunctions(LocationMethod *method)
{
    void *handle;
    LocInitFunction loc_init;
    LocFunction loc;

    const char *install_dir;
    char *lib[100], *last, *tok, path[MAXPATHLEN+1];
    int n = 0;
    tok = method->shared_lib;
    while(n < 99 && (lib[n] = strtok_r(tok, ":", &last))) {
	tok = NULL;
	n++;
    }
    if(!n) {
	showWarning("getLocFunction failed for %s", method->name);
	return false;
    }
    for(int i = 0; i < n-1; i++) {
	if(!dlopen(lib[i], RTLD_NOW | RTLD_GLOBAL)) {
	    showWarning("dlopen failed for %s\n%s", lib[i], dlerror());
	    return false;
	}
    }
    if((handle = dlopen(lib[n-1], RTLD_NOW)) == NULL)
    {
	install_dir = Application::getApplication()->installationDir();
	snprintf(path, sizeof(path), "%s/lib/%s", install_dir, lib[n-1]);

	if((handle = dlopen(path, RTLD_NOW)) == NULL)
	{
	    showWarning("dlopen failed for %s\n%s", lib[n-1], dlerror());
	    return false;
	}
    }
    if(!(loc_init = (LocInitFunction)dlsym(handle, method->init_function_name)))
    {
	showWarning("dlsym failed for %s\nand %s", lib[n-1],
			method->init_function_name, dlerror());
	return false;
    }
    method->locInitFunction = loc_init;

    if((loc = (LocFunction)dlsym(handle, method->function_name)) == NULL)
    {
	showWarning("dlsym failed for %s\nand %s", lib[n-1],
			method->function_name, dlerror());
	return false;
    }
    method->locFunction = loc;
    return true;
}

void Locate::createParamTab(LocationMethod *method)
{
    const char **labels = NULL;
    int i, n, num_members, num_extra, ncols;
    int *alignment = NULL;
    vector<enum TimeFormat> time_code;
    bool row_editable[1], *col_editable = NULL;
    CssClassDescription *des;
    CssClassExtra *extra;
    CssTableClass *css;
    Arg args[3];

    num_members = CssTableClass::getDescription(method->name, &des);
    if(num_members <= 0) {
	(*method->locInitFunction)();
	num_members = CssTableClass::getDescription(method->name, &des);
	if(num_members <= 0) {
	    showWarning("Cannot get table description for %s",method->name);
	    return;
	}
    }
    num_extra = CssTableClass::getExtra(method->name, &extra);
    if(num_extra < 0) num_extra = 0;

    n = 0;
    XtSetArg(args[n], XtNselectable, False); n++;
    Table *table = new Table(method->name, tab, args, n);
    method->param_table = table;

    table->addActionListener(this, XtNchoiceChangedCallback);

    ncols = num_members + num_extra + 1;
    if(!(labels = (const char **)mallocWarn(ncols*sizeof(char *)))) return;

    labels[0] = "orid";
    for(i = 0; i < num_members; i++) {
	labels[i+1] = des[i].name;
    }
    for(i = 0; i < num_extra; i++) {
	labels[i+1+num_members] = extra[i].name;
    }
    XtSetArg(args[0], XtNcolumns, ncols);
    XtSetArg(args[1], XtNcolumnLabels, labels);
    table->setValues(args, 2);

    Free(labels);

    alignment = new int[ncols];
    col_editable = new bool[ncols];

    alignment[0] = RIGHT_JUSTIFY;
    time_code.push_back(NOT_TIME);
    for(i = 0; i < num_members; i++) {
	alignment[i+1] = (des[i].type == CSS_STRING ||
			des[i].type == CSS_QUARK || des[i].type == CSS_BOOL) ?
			LEFT_JUSTIFY : RIGHT_JUSTIFY;

	time_code.push_back((des[i].type == CSS_TIME) ? YMONDHMS : NOT_TIME);

	if(des[i].type == CSS_BOOL) {
	    table->setColumnChoice(i+1, "true:false");
	}
	else if (!strcmp(des[i].name, "conf_level"))
	{
	   table->setColumnChoice(i+1, "0.90:0.95:0.99");
	}
	// not sure if we can use ellip_cor_level == 1, ask Dima
	else if (!strcmp(des[i].name, "ellip_cor_level"))
	{
	    table->setColumnChoice(i+1, "0:1:2");
	}
	else if (!strcmp(des[i].name, "sssc_level"))
	{
	    table->setColumnChoice(i+1, "0:1:2");
	}
    }
    for(i = 0; i < num_extra; i++) {
	alignment[i+1+num_members] = (extra[i].type == CSS_STRING ||
		extra[i].type == CSS_QUARK || extra[i].type == CSS_BOOL) ?
			LEFT_JUSTIFY : RIGHT_JUSTIFY;
	time_code.push_back((extra[i].type == CSS_TIME) ? YMONDHMS : NOT_TIME);
	if(extra[i].type == CSS_BOOL) {
	    table->setColumnChoice(i+1+num_members, "true:false");
	}
    }

    table->setAlignment(ncols, alignment);
    delete [] alignment;
    table->setColumnTime(time_code);
    row_editable[0] = true;
    table->setRowEditable(row_editable);
    col_editable[0] = false;
    for(i = 1; i < ncols; i++) col_editable[i] = true;
    table->setColumnEditable(col_editable);
    delete [] col_editable;

    css = CssTableClass::createCssTable(method->name);
	
    getDefaultParams(css);
    addParamLine(css, 0);	// add the default (top) line

    css->deleteObject();
}

void Locate::getDefaultParams(CssTableClass *css)
{
    const char *name;
    string prop_name, prop;
    char *type;
    int i, num_members, num_extra;
    CssClassDescription *des;
    CssClassExtra *extra;
    XrmDatabase database;
    XrmValue value;

    database = XrmGetDatabase(XtDisplay(base_widget));

    name = css->getName();
    des = css->description();
    num_members = css->getNumMembers();
    num_extra = css->getNumExtra();
    extra = css->getExtra();

    for(i = 0; i < num_members; i++)
    {
	prop_name.assign(string("locate.") + name + "." + des[i].name);
	if(getProperty(prop_name, prop)) {
	    css->setMember(i, prop);
	}
	else {
	    prop_name.assign(string("Geotool.locate.") + name+"."+des[i].name);
	    if(XrmGetResource(database, prop_name.c_str(), prop_name.c_str(),
				&type,&value))
	    {
		css->setMember(i, value.addr);
	    }
	}
    }
    for(i = 0; i < num_extra; i++)
    {
	prop_name.assign(string("locate.") + name + "." + extra[i].name);
	if(getProperty(prop_name, prop)) {
	    css->setExtra(i, prop);
	}
	else {
	    prop_name.assign(string("Geotool.locate.") +name+"."+extra[i].name);
	    if(XrmGetResource(database, prop_name.c_str(), prop_name.c_str(),
				&type,&value))
	    {
		css->setExtra(i, value.addr);
	    }
	}
    }
}

void Locate::addParamLine(CssTableClass *css, int orid)
{
    int		i, nrows, maxlen=200;
    int		num_members, num_extra, num_columns;
    const char	**row = NULL;
    char	*member_address, *value;
    const char	*name;
    bool	*editable = NULL;
    Table	*table;
    CssClassExtra	*extra;
    CssClassDescription *des;

    name = css->getName();

    if( !(table = findTable(name)) ) return;

    des = css->description();
    num_members = css->getNumMembers();
    num_extra = css->getNumExtra();
    extra = css->getExtra();
    num_columns = num_members + num_extra + 1;

    if(!(row = (const char **)mallocWarn(num_columns*sizeof(char *)))) return;

    if(!(row[0] = (char *)mallocWarn(maxlen))) return;

    if(!orid) {
	snprintf((char *)row[0], maxlen, "next");
    }
    else {
	snprintf((char *)row[0], maxlen, "%d", orid);
    }

    for(i = 0; i < num_members; i++)
    {
	if(!(row[1+i] = (char *)mallocWarn(maxlen))) return;
	value = (char *)row[1+i];
	member_address = (char *)css + des[i].offset;
	getStringValue(value, maxlen, member_address, des[i].type);
    }
    for(i = 0; i < num_extra; i++)
    {
	if(!(row[1+num_members+i] = (char *)mallocWarn(maxlen))) return;
	value = (char *)row[1+num_members+i];
	member_address = (char *)css + extra[i].offset;
	getStringValue(value, maxlen, member_address, extra[i].type);
    }

    table->addRow(row, false);
    for(i = 0; i < num_columns; i++) free((char *)row[i]);
    Free(row);

    nrows = table->numRows();
    if(!(editable = (bool *)mallocWarn(nrows*sizeof(bool)))) return;
    editable[0] = true;
    for(i = 1; i < nrows; i++) editable[i] = false;
    table->setRowEditable(editable);
    Free(editable);

    table->adjustColumns();
}

static void
getStringValue(char *value, int maxlen, char *member_address, int type)
{
    bool b;
    double time;

    switch(type)
    {
	case CSS_STRING:
	    stringcpy(value, member_address, maxlen);
	    break;
	case CSS_QUARK:
	    stringcpy(value, quarkToString(*(int *)member_address), maxlen);
	    break;
	case CSS_DATE:
	case CSS_LDDATE:
	    stringcpy(value, timeDateString((DateTime *)member_address),maxlen);
	    break;
	case CSS_LONG:
	case CSS_JDATE:
	    snprintf(value, maxlen, "%ld", *(long *)member_address);
	    break;
	case CSS_INT:
	    snprintf(value, maxlen, "%d", *(int *)member_address);
	    break;
	case CSS_DOUBLE:
	    snprintf(value, maxlen, "%.2f", *(double *)member_address);
	    break;
	case CSS_FLOAT:
	    snprintf(value, maxlen, "%.2f", *(float *)member_address);
	    break;
	case CSS_TIME:
	    time = *(double *)member_address;
	    timeEpochToString(time, value, maxlen, YMONDHMS);
	    break;
	case CSS_BOOL:
	    b = *(bool *)member_address;
	    if(b) {
		stringcpy(value, "true", maxlen);
	    }
	    else {
		stringcpy(value, "false", maxlen);
	    }
	    break;
    }
    value[maxlen-1] = '\0';
}

void Locate::listOriginData(void)
{
    origin_table->removeAllRecords();

    for(int i = 0; i < origin_data.size(); i++)
    {
	OriginData *o = origin_data[i];
	origin_table->addRecord(new TableRow(o->origin, o->origerr), false);
    }
    if((int)origin_data.size() == 0) return;

    origin_table->list();

    /* Sort origin table by first column (orid) */
    vector<int> sortcols(1);
    sortcols[0] = 0;
    origin_table->sortByColumns(sortcols);

    int nrows = origin_data.size();
    char **row_labels = NULL;
    if(!(row_labels = (char **)mallocWarn(nrows*sizeof(char *)))) return;
    for(int i = 0; i < nrows; i++) {
	char buf[20];
	snprintf(buf, sizeof(buf), "%d", i+1);
	row_labels[i] = strdup(buf);
    }
    origin_table->setRowLabels((const char **)row_labels);
    for(int i = 0; i < nrows; i++) {
	free(row_labels[i]);
    }
    Free(row_labels);

    for(int i = 0; i < origin_data.size(); i++)
    {
	OriginData *o = origin_data[i];
	if(o->origin->getSelected()) {
	    origin_table->selectRow(i, true);
	}
    }
}

void Locate::selectOrigin(void)
{
    vector<int> rows;
    int num = origin_table->getSelectedRows(rows);

    updateInitialLocation();

    for(int i = 0; i < origin_data.size(); i++) {
	origin_data[i]->origin->setSelected(false);
    }
    for(int j = 0; j < arrival_data.size(); j++) {
	ArrivalData *a = arrival_data[j];
	for(int k = 0; k < a->assocs.size(); k++) {
	    a->assocs[k]->setSelected(false);
	}
    }
    for(int i = 0; i < num; i++)
    {
	OriginData *o = origin_data[rows[i]];
	o->origin->setSelected(true);

	for(int j = 0; j < arrival_data.size(); j++) {
	    ArrivalData *a = arrival_data[j];
	    for(int k = 0; k < a->assocs.size(); k++) {
		if(a->assocs[k]->orid == o->origin->orid) {
		    a->assocs[k]->setSelected(true);
		}
	    }
	}
    }
}

void Locate::updateInitialLocation(void)
{
    int i, num;
    vector<int> rows;
    int horizontal_pos, vertical_pos;
    vector<const char *> labels;
    char buf[50];
    CssOriginClass *o=NULL;

    num = origin_table->getSelectedRows(rows);
    if(num > 0) {
	i = rows[num-1];
	o = origin_data[i]->origin;
    }

    LocationMethod *method =  getMethod();
    Table *table = method->param_table;

    if( !table->getColumnLabels(labels) ) return;
    table->getScrolls(&horizontal_pos, &vertical_pos);

    table->editModeOff();

    if(num > 0) {
	for(i = 0; i < (int)labels.size(); i++) {
	    if(!strcasecmp(labels[i], "time_init")) {
		timeEpochToString(o->time, buf, sizeof(buf), YMONDHMS);
		table->setField(0, i, buf, False);
	    }
	    else if(!strcasecmp(labels[i], "lat_init")) {
		snprintf(buf, sizeof(buf), "%.2f", o->lat);
		table->setField(0, i, buf, False);
	    }
	    else if(!strcasecmp(labels[i], "lon_init")) {
		snprintf(buf, sizeof(buf), "%.2f", o->lon);
		table->setField(0, i, buf, False);
	    }
	    else if(!strcasecmp(labels[i], "depth_init")) {
		snprintf(buf, sizeof(buf), "%.2f", o->depth);
		table->setField(0, i, buf, False);
	    }
	}
    }
    else {
	for(i = 0; i < (int)labels.size(); i++) {
	    if(!strcasecmp(labels[i], "time_init")) {
		table->setField(0, i, "none", False);
	    }
	    else if(!strcasecmp(labels[i], "lat_init")) {
		table->setField(0, i, "-999.00", False);
	    }
	    else if(!strcasecmp(labels[i], "lon_init")) {
		table->setField(0, i, "-999.00", False);
	    }
	    else if(!strcasecmp(labels[i], "depth_init")) {
		table->setField(0, i, "-999.00", False);
	    }
	}
    }
    table->adjustColumns();
    table->setScrolls(horizontal_pos, vertical_pos);
}

void Locate::listArrivalData(void)
{

    int	i, j, k, l, m, num, num_columns, maxlen=200;
    int	num_members1, num_members2 = 0, num_orids, num_rows;
    int	*oi = NULL, *index = NULL;
    vector<enum TimeFormat> time_code;
    int	*alignment = NULL, *rowIndex = NULL;
    int	hor_scroll, vert_scroll;
    vector<const char *> row;
    char **format=NULL, buf[50], **s=NULL, *value;
    const char **labels = NULL, **choice = NULL, **r = NULL;
    bool *col_editable = NULL;
    vector<bool> states;
    Pixel *colors, bg, bg_brighter;
    Arg	args[3];
    double time;
    string svalue;
    ArrivalData	*a;
    CssClassDescription *des[2];

    num_rows = arrival_table->numRows();
    arrival_table->getScrolls(&hor_scroll, &vert_scroll);

    if(num_rows == arrival_data.size())
    {
	arrival_table->getRowStates(states);
	for(i = 0; i < num_rows; i++)
	{
	    a = arrival_data[i];
	    arrival_table->getRow(i, row);
	    stringcpy(a->phase, row[0], sizeof(a->phase));
	    stringcpy(a->timedef, row[1], sizeof(a->timedef));
	    stringcpy(a->azdef, row[2], sizeof(a->azdef));
	    stringcpy(a->slodef, row[3], sizeof(a->slodef));
	}
    }
    else {
	hor_scroll = vert_scroll = 0;
	num_rows = arrival_data.size();
	for(k = 0; k < arrival_data.size(); k++) states.push_back(true);
    }

    arrival_table->removeAllRows();

    if(arrival_data.size() <= 0) {
	arrival_data.clear();
	return;
    }

    num_members1 = CssTableClass::getDescription(cssArrival, &des[0]);
    num_members2 = CssTableClass::getDescription(cssAssoc, &des[1]);

    num_orids = origin_data.size();

    int num_display_attributes = arrival_attributes->numDisplayAttributes();
    num = num_display_attributes*(1 + num_orids);

    if(!(index = (int *)mallocWarn(num*sizeof(int)))) return;
    if(!(oi = (int *)mallocWarn(num*sizeof(int)))) return;
    if(!(labels = (const char **)mallocWarn(num*sizeof(char *)))) return;
    if(!(choice = (const char **)mallocWarn(num*sizeof(char *)))) return;
    if(!(alignment = (int *)mallocWarn(num*sizeof(int)))) return;
    if(!(format = (char **)mallocWarn(num*sizeof(char *)))) return;
    if(!(col_editable = (bool *)mallocWarn(num*sizeof(bool)))) return;
    if(!(colors = (Pixel *)mallocWarn(num*sizeof(Pixel)))) return;

    /*
     * Put timedef, azdef and slodef in first three columns.
     */

    labels[0] = strdup("P");
    labels[1] = strdup("T");
    labels[2] = strdup("A");
    labels[3] = strdup("S");
    for(i = 0; i < 4; i++) {
	format[i] = strdup("%s");
	alignment[i] = LEFT_JUSTIFY;
	index[i] = -1;
	oi[i] = -1;
    }

    s = (char **)mallocWarn(num_orids*sizeof(char *));

    getOridLabels(num_orids, origin_data, s);

    num_columns = 4;
    for(i = 0; i < num_display_attributes; i++)
    {
	TAttribute ta = arrival_attributes->getAttribute(i);

	for(j = 0; j < num_members1; j++)
	{
	    if(!strcmp(des[0][j].name, ta.name))
	    {
		labels[num_columns] = strdup(ta.name);
		alignment[num_columns] = getAlignment(ta.format);
		oi[num_columns] = 0;
		index[num_columns] = j;
		time_code.push_back((des[0][j].type == CSS_TIME) ?
			getTimeCode(ta.format) : NOT_TIME);
		format[num_columns] = strdup(ta.format);
		num_columns++;
		break;
	    }
	}
	if(j == num_members1)
	{
	    for(j = 0; j < num_members2; j++)
	    {
		if(!strcmp(des[1][j].name, ta.name))
		{
		    for(k = 0; k < num_orids; k++)
		    {
			OriginData *o = origin_data[k];
			if(o->origin->getSelected())
			{
			    if(!strcmp(ta.name, "phase")) {
				snprintf(buf, sizeof(buf), "P%s", s[k]);
			    }
			    else if(!strcmp(ta.name, "timedef")) {
				snprintf(buf, sizeof(buf), "T%s", s[k]);
			    }
			    else if(!strcmp(ta.name, "azdef")) {
				snprintf(buf, sizeof(buf), "A%s", s[k]);
			    }
			    else if(!strcmp(ta.name, "slodef")) {
				snprintf(buf, sizeof(buf), "S%s", s[k]);
			    }
			    else {
				snprintf(buf, sizeof(buf),"%s%s", ta.name,s[k]);
			    }
			    labels[num_columns] = strdup(buf);
			    alignment[num_columns] = getAlignment(ta.format);
			    oi[num_columns] = k+1;
			    index[num_columns] = j;
			    time_code.push_back((des[1][j].type == CSS_TIME) ?
				getTimeCode(ta.format) : NOT_TIME);
			    format[num_columns] = strdup(ta.format);
			    num_columns++;
			}
		    }
		    break;
		}
	    }
	    if(j == num_members2) {
		labels[num_columns] = strdup(ta.name);
		alignment[num_columns] = LEFT_JUSTIFY;
		format[num_columns] = strdup(ta.format);
		index[num_columns++] = -1;
	    }
	}
    }
    for(i = 0; i < num_orids; i++) free(s[i]);
    Free(s);

    XtSetArg(args[0], XmNbackground, &bg);
    tab->getValues(args, 1);
    bg_brighter = pixelBrighten(bg, .9);
    choice[0] = "";
    col_editable[0] = true;
    colors[0] = bg_brighter;
    for(i = 1; i < 4; i++) {
	choice[i] = "d:n";
	col_editable[i] = true;
	colors[i] = bg_brighter;
    }
    for(i = 4; i < num_columns; i++) {
	choice[i] = "";
	col_editable[i] = false;
	colors[i] = bg;
    }
    XtSetArg(args[0], XtNcolumns, num_columns);
    XtSetArg(args[1], XtNcolumnLabels, labels);
    XtSetArg(args[2], XtNcolumnChoice, choice);
    arrival_table->setValues(args, 3);
    Free(choice);
    arrival_table->setAlignment(num_columns, alignment);
    Free(alignment);
    arrival_table->setColumnEditable(col_editable);
    Free(col_editable);

    if(!(rowIndex = (int *)mallocWarn(num_rows*sizeof(int)))) return;
    for(k = 0; k < num_rows; k++) {
	rowIndex[k] = k;
    }

    if(!(r = (const char **)mallocWarn(num_columns*sizeof(char *)))) return;
    for(i = 0; i < num_columns; i++) {
	if(!(r[i] = (char *)mallocWarn(maxlen))) return;
    }

    for(k = 0; k < arrival_data.size(); k++)
    {
//	double dvalue = -999.;
	a = arrival_data[k];

	stringcpy((char *)r[0], a->phase, maxlen);
	stringcpy((char *)r[1], a->timedef, maxlen);
	stringcpy((char *)r[2], a->azdef, maxlen);
	stringcpy((char *)r[3], a->slodef, maxlen);

	for(i = 4; i < num_columns; i++)
	{
	    value = (char *)r[i];
	    j = index[i];
	    if(j >= 0)
	    {
		char *member_address;
		int type;

		m = oi[i];
		if(m == 0) {
		    member_address = (char *)a->arrival + des[0][j].offset;
		    type = des[0][j].type;
		}
		else {
		    OriginData *o = origin_data[m-1];
		    int orid = o->origin->orid;
		    for(l = 0; l < a->assocs.size()
				&& a->assocs[l]->orid != orid; l++);
		    if(l < a->assocs.size()) {
			member_address = (char *)a->assocs[l]+des[1][j].offset;
		    }
		    else {
			value[0] = '\0';
			continue;
		    }
		    type = des[1][j].type;
		}

		switch(type)
		{
		    case CSS_STRING:
			stringcpy(value, member_address, maxlen);
			break;
		    case CSS_QUARK:
			stringcpy(value,
				quarkToString(*(int *)member_address), maxlen);
			break;
		    case CSS_DATE:
		    case CSS_LDDATE:
			stringcpy(value, timeDateString(
				(DateTime *)member_address), maxlen);
			break;
		    case CSS_LONG:
		    case CSS_JDATE:
			snprintf(value, maxlen, format[i],
					*(long *)member_address);
//			dvalue = (double) *(long *)member_address;
			break;
		    case CSS_INT:
			snprintf(value, maxlen, format[i],
					*(int *)member_address);
//			dvalue = (double) *(int *)member_address;
			break;
		    case CSS_DOUBLE:
			snprintf(value, maxlen, format[i],
					*(double *)member_address);
//			    dvalue = *(double *)member_address;
			    break;
		    case CSS_FLOAT:
			snprintf(value, maxlen, format[i],
					*(float *)member_address);
//			dvalue = (float) *(float *)member_address;
			break;
		    case CSS_TIME:
			time = *(double *)member_address;
			if(!strcmp(format[i], "%t")) {
			    timeEpochToString(time, value, maxlen,YMONDHMS);
			}
			else if(!strcmp(format[i], "%T")) {
			    timeEpochToString(time, value,maxlen,YMONDHMS3);
			}
			else if(!strcmp(format[i], "%g")) {
			    timeEpochToString(time, value, maxlen, GSE20);
			}
			else if(!strcmp(format[i], "%G")) {
			    timeEpochToString(time, value, maxlen, GSE21);
			}
			else {
			    snprintf(value, maxlen, format[i], time);
			}
//			dvalue = (float) *(double *)member_address;
			break;
		    default:
			stringcpy(value, "na", maxlen);
		}
	    }
	    else if(!strcmp(labels[i], "file")) {
		stringcpy(value, quarkToString(a->arrival->getFile()), maxlen);
	    }
	    else if(!strcmp(labels[i], "file_index")) {
		snprintf(value, maxlen, format[i], a->arrival->filePosition());
	    }
	    else if(a->arrival->getValue(labels[i], svalue)) {
		stringcpy(value, svalue.c_str(), maxlen);
	    }
	    else {
		stringcpy(value, "na", maxlen);
	    }
	    value[maxlen-1] = '\0';
	}
	arrival_table->addRow(r, false);
/*	colorResidual(gt->table, labels[i], dvalue); */
    }
    for(i = 0; i < num_columns; i++) {
	free(format[i]);
	free((char *)r[i]);
	free((char *)labels[i]);
    }
    Free(r);
    Free(labels);

    arrival_table->setColumnColors(colors);
    arrival_table->setColumnTime(time_code);
    arrival_table->adjustColumns();
    bool *b = new bool[states.size()];
    for(i = 0; i < (int)states.size(); i++) b[i] = states[i];
    arrival_table->selectRows(num_rows, rowIndex, b);
    delete [] b;
    arrival_table->setScrolls(hor_scroll, vert_scroll);

    Free(colors);
    Free(format);
    Free(oi);
    Free(index);
    Free(rowIndex);
}

static void
getOridLabels(int num_orids, gvector<OriginData *> &origin_data, char **s)
{
    int num = 1000;
    bool duplicate = true;
    while(duplicate) {
	for(int i = 0; i < num_orids; i++) {
	    OriginData *o = origin_data[i];
	    if(o->origin->getSelected()) {
		char s_orid[50];
		if(o->origin->orid > num) {
		    int orid = o->origin->orid - (o->origin->orid/num)*num;
		    snprintf(s_orid, sizeof(s_orid), "..%d", orid);
		}
		else {
		    snprintf(s_orid, sizeof(s_orid), "%ld", o->origin->orid);
		}
		s[i] = strdup(s_orid);
	    }
	    else {
		s[i] = strdup("");
	    }
	}
	duplicate = false;
	for(int i = 0; i < num_orids; i++) if(strlen(s[i]) > 0)
	{
	    for(int j = 0; j < num_orids; j++) if(strlen(s[j]) > 0)
	    {
		if(j  != i && !strcmp(s[i], s[j])) {
		    duplicate = true;
		    break;
		}
	    }
	    if(duplicate) break;
	}
	if(duplicate) {
	    num *= 10;
	    int k;
	    for(k = 0; k < num_orids; k++) {
		OriginData *o = origin_data[k];
		if(o->origin->getSelected() && o->origin->orid > num) break;
	    }
	    if(k == num_orids) return;
	    for(k = 0; k < num_orids; k++) free(s[k]);
	}
    }
}

static int
getAlignment(char *format)
{
    return (!strcmp(format, "%s")) ? LEFT_JUSTIFY : RIGHT_JUSTIFY;
}

static enum TimeFormat
getTimeCode(char *format)
{
    if(!strcmp(format, "%t")) {
	return YMONDHMS;
    }
    else if(!strcmp(format, "%T")) {
	return YMONDHMS3;
    }
    else if(!strcmp(format, "%g")) {
	return GSE20;
    }
    else if(!strcmp(format, "%G")) {
	return GSE21;
    }
    else if(!strcmp(format, "%D")) {
	return YMOND;
    }
    return YMONDHMS;
}

void Locate::locateEvents(bool selected)
{
    int num_members, num_extra, num_phases, num, err;
    int horizontal_pos, vertical_pos;
    vector<int> rows;
    CssTableClass *css;
    CssClassDescription *des;
    CssClassExtra *extra;
    CssOriginClass *origin;
    CssOrigerrClass *origerr;
    CssArrivalClass *arrival;
    CssAssocClass *assoc;
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    vector<const char *> phase, timedef, azdef, slodef, row;
    char **phases = NULL, *details = NULL;
//    double otime = NULL_TIME;
    LocationMethod *method;

    location_status = -1;

    if( !(method = getMethod()) ) return;

    num_members = CssTableClass::getDescription(method->name, &des);
    num_extra = CssTableClass::getExtra(method->name, &extra);

    if( !method->param_table->getRow(0, row) ) {
	location_status = -2;
	return;
    }
    method->param_table->getScrolls(&horizontal_pos, &vertical_pos);

    css = CssTableClass::createCssTable(method->name);

    /* start at 1 since the first column (origin number) is not part of
     * the CssTableClass
     */
    for(int i = 0; i < num_members; i++) {
	if(!css->setMember(i, row[1+i])) {
	    showWarning("Location Parameters: %s", CssTableClass::getError());
	    css->deleteObject();
	    location_status = -3;
	    return;
	}
/*
	if(!strcmp(des[i].name, "time_init") && des[i].type == CSS_TIME) {
	    char *member_address = (char *)css + des[i].offset;
	    otime = *(double *)member_address;
	}
*/
    }
    for(int i = 0; i < num_extra; i++) {
	if(!css->setExtra(i, row[1+num_members+i])) {
	    showWarning("Location Parameters: %s", CssTableClass::getError());
	    css->deleteObject();
	    location_status = -4;
	    return;
	}
    }

    if(!getDefining(phase, timedef, azdef, slodef)) {
	location_status = -5;
	return;
    }

    if(selected) {
	num = arrival_table->getSelectedRows(rows);
	for(int i = 0; i < arrival_data.size(); i++) {
	    arrival_data[i]->arrival->setSelected(false);
	}
	for(int i = 0; i < num; i++) {
	    arrival_data[rows[i]]->arrival->setSelected(true);
	}
    }
    else {
	for(int i = 0; i < arrival_data.size(); i++) {
	    arrival_data[i]->arrival->setSelected(true);
	}
    }

    for(int i = 0; i < arrival_data.size(); i++)
    {
	ArrivalData *a = arrival_data[i];
	if(a->arrival->getSelected())
	{
	    arrivals.push_back(a->arrival);
	    assoc = new CssAssocClass();
	    assoc->arid = a->arrival->arid;
	    a->arrival->copySourceTo(assoc, assoc->arid);
	    stringcpy(assoc->phase, phase[i], sizeof(assoc->phase));
	    stringcpy(assoc->sta, a->arrival->sta, sizeof(assoc->sta));
	    stringcpy(assoc->timedef, timedef[i], sizeof(assoc->timedef));
	    stringcpy(assoc->azdef, azdef[i], sizeof(assoc->azdef));
	    stringcpy(assoc->slodef, slodef[i], sizeof(assoc->slodef));
	    assoc->orid = tmp_orid;
	    a->assocs.push_back(assoc);
	    assocs.push_back(assoc);
	}
    }

    if(arrivals.size() == 0) {
	showWarning("No arrival data.");
	location_status = -6;
	return;
    }

    if(!(phases = (char **)mallocWarn(assocs.size()*sizeof(char *)))) {
	location_status = -7;
	return;
    }

    num_phases = 0;
    for(int i = 0; i < assocs.size(); i++) {
	char *p = assocs[i]->phase;

	/* See if this is an acceptable phase. The location code does not handle
	 * LR or LQ correctly, so do not include them
	 */
	if (isAcceptablePhase(p) )
	{
	    int j;
	    for(j = 0; j < num_phases && strcmp(phases[j], p); j++);
	    if(j == num_phases) {
		phases[num_phases++] = p;
	    }
	}
    }

    arrival = arrivals[0];

    origin = new CssOriginClass();
    DataSource::copySourceInfo(origin, arrival);
    origerr = new CssOrigerrClass();
    DataSource::copySourceInfo(origerr, arrival);
    origin->orid = tmp_orid;
    origin->evid = tmp_orid;
    origerr->orid = tmp_orid;

    setCursor("hourglass");
    err = (*method->locFunction)(css, num_phases, phases, sites,
			arrivals, assocs, origin, origerr, &details);
    Free(phases);
    location_status = err;

    origin_data.add(new OriginData(origin, origerr, details));

    if(data_source->getWaveformWindowInstance() != NULL || 
	data_source->getWaveformPlotInstance() != NULL)
    {
	// avoid feed-back
	data_source->removeDataListener(this);
	for(int i = 0; i < assocs.size(); i++) {
	    data_source->putTable(assocs[i], false);
	}
	data_source->putTable(origerr, false);
	data_source->putTable(origin);
	data_source->addDataListener(this);
    }

    addParamLine(css, tmp_orid);

    method->param_table->setScrolls(horizontal_pos, vertical_pos);

    tmp_orid--;

    css->deleteObject();

    listOriginData();

    int i = origin_data.size()-1;
    origin_table->selectRow(i, true);

    OriginData *o = origin_data[i];
    o->origin->setSelected(true);

    listArrivalData();

    setCursor("default");

    if(err) {
	showWarning("Error return from location method. See location details. Constrain the event depth to the surface manually (change fix_depth from false to true, and depth_init from -999.00 to 0.0).");
    }
}

bool Locate::getDefining(vector<const char *> &phase,
	vector<const char *> &timedef,
	vector<const char *> &azdef,
	vector<const char *> &slodef)
{
    if( !arrival_table->getColumn(0, phase) ) {
	showWarning("Error getting phase column.");
	return false;
    }
    if( !arrival_table->getColumn(1, timedef) ) {
	showWarning("Error getting timedef column.");
	return false;
    }
    if( !arrival_table->getColumn(2, azdef) ) {
	showWarning("Error getting azdef column.");
	return false;
    }
    if( !arrival_table->getColumn(3, slodef) ) {
	showWarning("Error getting slodef column.");
	return false;
    }
    return true;
}

void Locate::showDetails(void)
{
    vector<int> rows;

    origin_table->getSelectedRows(rows);
    if(!(int)rows.size()) {
	showWarning("No origins selected.");
	return;
    }
    for(int i = 0; i < (int)rows.size(); i++)
    {
	OriginData *o = origin_data[rows[i]];
	if(o->details != NULL)
	{
	    if(o->details_dialog == NULL)
	    {
		char title[20];
		snprintf(title, 20, "Location-%d Details", rows[i]+1);
		o->details_dialog = new Details(title, this, o->details);
	    }
	    else {
		o->details_dialog->setText(o->details);
	    }
	    o->details_dialog->setVisible(true);
	}
    }
}

void Locate::removeSelectedOrigin(void)
{
    gvector<OriginData *> new_origin_data;
    vector<const char *> orids;
    vector<int> rows;
    int i, j, k, num, nrows, orid, nfound;

    num = origin_table->getSelectedRows(rows);

    std::sort(rows.begin(), rows.end());

    // avoid feed-back from data_source->removeTable()
    data_source->removeDataListener(this);

    for(i = num-1; i >= 0; i--)
    {
	OriginData *o = origin_data[rows[i]];

	for(j = 0; j < num_methods; j++) {
	    Table *table = methods[j].param_table;
	    nrows = table->getColumn(0, orids);
	    for(k = nrows-1, nfound = 0; k > 0; k--) {
		if(stringToInt(orids[k], &orid) && orid == o->origin->orid)
		{
		    table->removeRow(k);
		    nfound++;
		}
	    }
	    if (nfound > 0) table->adjustColumns();
	}

	orid = o->origin->orid;
	for(k = 0; k < arrival_data.size(); k++)
	{
	    for(j = arrival_data[k]->assocs.size()-1; j >= 0; j--) {
		if(arrival_data[k]->assocs[j]->orid == orid) {
		    data_source->removeTable(arrival_data[k]->assocs[j], false);
		    arrival_data[k]->assocs.removeAt(j);
		}
	    }
	}
	data_source->removeTable(o->origerr, false);
	if(i > 0) {
	    data_source->removeTable(o->origin, false);
	}
	else {
	    data_source->removeTable(o->origin);
	}
printf("origin_data row: %d\n", rows[i]);
	origin_data.removeAt(rows[i]);
    }
    listOriginData();
    listArrivalData();
    data_source->addDataListener(this);
}

void Locate::removeSelectedArrival(void)
{
    vector<int> rows;

    arrival_table->getSelectedRows(rows);

    for(int i = (int)rows.size()-1; i >= 0; i--) {
	arrival_data.removeAt(rows[i]);
    }
    listArrivalData();
}


void Locate::columnMoved(void)
{
    vector<int> order;
    arrival_table->getColumnOrder(order);
    arrival_attributes->setOrder(order);
    arrival_table->resetColumnOrder();

    TAttribute **a;
    int num;

    if( !(num = arrival_attributes->displayAttributes(&a)) ) return;

    char value[2000];
    int n = 0;
    for(int i = 0; i < num; i++) {
	snprintf(value+n, sizeof(value)-n, "%s,%s,", a[i]->name, a[i]->format);
	n = (int)strlen(value);
    }
    if(n > 0) value[n-1] = '\0';  // the last ','

    putProperty("locate.arrival_table.attributes", value);
    Application::writeApplicationProperties();
}

void Locate::arrivalChange(TableListenerCallback *tc)
{
    vector<const char *> labels;
    char *format, *name, *member_address, value[50];
    int i, j, k, ncols, type, num_members, num_display_attributes, row;
    int num_memb, *members=NULL;
    double time;
    CssClassDescription *des;
    ArrivalData *a=NULL;

    for(i = 0; i < arrival_data.size(); i++)
    {
	if(tc->table == (CssTableClass *)arrival_data[i]->arrival) {
	    a = arrival_data[i];
	    row = i;
	    break;
	}
    }
    if(!a) return;

    if( !arrival_table->getColumnLabels(labels) ) return;

    num_members = CssTableClass::getDescription(cssArrival, &des);

    if(tc->num_members > 0) {
	num_memb = tc->num_members;
	members = (int *)malloc(num_memb*sizeof(int));
	memcpy(members, tc->members, num_memb*sizeof(int));
    }
    else { // do all members
	num_memb = num_members;
	members = (int *)malloc(num_memb*sizeof(int));
	for(i = 0; i < num_memb; i++) members[i] = i;
    }

    ncols = arrival_table->numColumns();

    for(k = 0; k < num_memb; k++)
    {
	if(members[k] < 0 || members[k] >= num_members) continue;

	name = des[members[k]].name;
	member_address = (char *)a->arrival + des[members[k]].offset;
	type = des[members[k]].type;

	for(j = 0; j < ncols && strcmp(labels[j], name); j++);
	if(j == ncols) continue;

	num_display_attributes = arrival_attributes->numDisplayAttributes();
	format = NULL;
	for(i = 0; i < num_display_attributes
	    && strcmp(arrival_attributes->getAttribute(i).name, name); i++);
	if(i < num_display_attributes) {
	    format = arrival_attributes->getAttribute(i).format;
	}

	if( !format ) continue;

	value[0] = '\0';
	switch(type)
	{
	    case CSS_STRING:
		stringcpy(value, member_address, 50);
		break;
	    case CSS_QUARK:
		stringcpy(value, quarkToString(*(int *)member_address), 50);
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		stringcpy(value, timeDateString((DateTime *)member_address),50);
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		snprintf(value, 50, format, *(long *)member_address);
		break;
	    case CSS_INT:
		snprintf(value, 50, format, *(int *)member_address);
		break;
	    case CSS_DOUBLE:
		snprintf(value, 50, format, *(double *)member_address);
		break;
	    case CSS_FLOAT:
		snprintf(value, 50, format, *(float *)member_address);
		break;
	    case CSS_TIME:
		time = *(double *)member_address;
		if(!strcmp(format, "%t")) {
		    timeEpochToString(time, value, 50, YMONDHMS);
		}
		else if(!strcmp(format, "%T")) {
		    timeEpochToString(time, value, 50, YMONDHMS3);
		}
		else if(!strcmp(format, "%g")) {
		    timeEpochToString(time, value, 50, GSE20);
		}
		else if(!strcmp(format, "%G")) {
		    timeEpochToString(time, value, 50, GSE21);
		}
		else {
		    snprintf(value, 50, format, time);
		}
		break;
	    default:
		stringcpy(value, "na", 50);
	}
	if(value[0] != '\0') arrival_table->setField(row, j, value, true);
    }
    Free(members);
}

void Locate::saveOrigins(void)
{
    bool found_one = False;
    int i, j, k, tp_orid;
    Password password = getpwuid(getuid());

    for(i = 0; i < origin_data.size(); i++)
    {
	OriginData *o = origin_data[i];
	DataSource *ds = o->origin->getDataSource();
	if(ds && !o->saved)
	{
	    tp_orid = o->origin->orid;

	    if(ds->originCreate(o->origin, o->origerr, o->origin, password))
	    {
		o->saved = true;
		k = o->origin->memberIndex("orid");
		TableListener::doCallbacks(o->origin, this, k);
		TableListener::doCallbacks(o->origerr, this, k);

		for(k = 0; k < arrival_data.size(); k++)
		{
		    ArrivalData *a = arrival_data[k];
		    for(j = a->assocs.size()-1; j >= 0; j--) {
			if(a->assocs[j]->orid == tp_orid) {
			    a->assocs[j]->orid = o->origin->orid;
			    ds->addAssoc(a->assocs[j]);
			}
		    }
		}
		found_one = true;
	    }
	}
	if(found_one) {
	    listOriginData();
	    listArrivalData();
	}
    }
}

int Locate::getPathInfo(PathInfo **path_info)
{
    PathInfo *path = (PathInfo *)mallocWarn(sizeof(PathInfo));
    int num = 0;

    for(int k = 0; k < origin_data.size(); k++)
    {
	OriginData *o = origin_data[k];

	for(int i = 0; i < arrival_data.size(); i++) {
	    ArrivalData *a = arrival_data[i];
	    for(int j = 0; j < a->assocs.size(); j++) {
		if(a->assocs[j]->orid == o->origin->orid) {
		    int l;
		    for(l = 0; l < num; l++) {
			if(path[k].origin == o->origin &&
			    !strcmp(path[k].sta, a->assocs[j]->sta)) break;
		    }
		    if(l == num) {
			path = (PathInfo *)reallocWarn(path,
					(num+1)*sizeof(PathInfo));
			strcpy(path[num].sta, a->assocs[j]->sta);
			path[num].lat = -999.;
			path[num].lon = -999.;
			if(a->site) {
			    path[num].lat = a->site->lat;
			    path[num].lon = a->site->lon;
			}
			path[num].origin = o->origin;
			path[num].fg = stringToPixel("navy blue");
			num++;
		    }
		}
	    }
	}
    }
    if(!num) Free(path)
    *path_info = path;
    return num;
}

static bool use_orid = false;

void Locate::sortArrivals(const char *sort)
{
    if(!strcasecmp(sort, "distance/time")) {
	use_orid = false;
	arrival_data.sort(sort_by_dist);
    }
    else if(!strcasecmp(sort, "orid/distance/time")) {
	use_orid = true;
	arrival_data.sort(sort_by_dist);
    }
    else if(!strcasecmp(sort, "time")) {
	arrival_data.sort(sort_by_time);
    }
    else if(!strcasecmp(sort, "sta/time")) {
	arrival_data.sort(sort_by_sta_time);
    }
    else {
	showWarning("Unknown sort type: %s", sort);
	return;
    }
}

/* sort by orid/distance/time or distance/time
 */
static int
sort_by_dist(const void *A, const void *B)
{
    ArrivalData *a = *(ArrivalData **)A;
    ArrivalData *b = *(ArrivalData **)B;

    if(a->assocs.size() > 0 && b->assocs.size() > 0)
    {
	int i;
	long a_orid=0, b_orid=0;
	double a_delta = -1., b_delta = -1.;

	for(i = 0; i < a->assocs.size(); i++) {
	    if(i == 0) {
		a_delta = a->assocs[i]->delta;
		a_orid = a->assocs[i]->orid;
	    }
	    if(a->assocs[i]->getSelected()) {
		a_delta = a->assocs[i]->delta;
		a_orid = a->assocs[i]->orid;
		break;
	    }
	}
	for(i = 0; i < b->assocs.size(); i++) {
	    if(i == 0) {
		b_delta = b->assocs[i]->delta;
		b_orid = b->assocs[i]->orid;
	    }
	    if((b->assocs[i]->getSelected())) {
		b_delta = b->assocs[i]->delta;
		b_orid = b->assocs[i]->orid;
		break;
	    }
	}

	if(use_orid) {
	    if(a_orid > b_orid) return 1;
	    else if(a_orid < b_orid) return -1;
	}

	if(a_delta > b_delta) return 1;
	else if(a_delta < b_delta) return -1;

	if(a->arrival->time > b->arrival->time) return 1;
	else if(a->arrival->time < b->arrival->time) return -1;
	return 0;
    }
    else if(a->assocs.size() > 0) {
	return 1;
    }
    else if(b->assocs.size() > 1) {
	return -1;
    }
    else {
	if(a->arrival->time > b->arrival->time) return 1;
	else if(a->arrival->time < b->arrival->time) return -1;
	return 0;
    }
}

static int
sort_by_time(const void *A, const void *B)
{
    ArrivalData *a = *(ArrivalData **)A;
    ArrivalData *b = *(ArrivalData **)B;

    if(a->arrival->time > b->arrival->time) return 1;
    else if(a->arrival->time < b->arrival->time) return -1;
    else return 0;
}

static int
sort_by_sta_time(const void *A, const void *B)
{
    ArrivalData *a = *(ArrivalData **)A;
    ArrivalData *b = *(ArrivalData **)B;
    int c;

    if((c = strcmp(a->arrival->sta, b->arrival->sta)) != 0) {
	return (c > 0) ? 1 : -1;
    }
    if(a->arrival->time > b->arrival->time) return 1;
    else if(a->arrival->time < b->arrival->time) return -1;
    else return 0;
}

void Locate::loadResources(void)
{
    if(load_resources) { // only need to do this one time.
	load_resources = false;
	ResourceManager *rm = Application::getResourceManager(this);

	rm->putResource(
    "*Locate_Details_text.fontList: -*-courier-medium-r-*-*-14-*-*-*-*-*-*-*");
	rm->close();
    }
}

/*
Delete origin from Origins window should cause corresponding origin to be
remove from the origin MultiTable, but it doesn't
****** this is because need to fix MultiTable::actionPerformed to catch the
****** "delete" command with CssTableChange
*/
