/** \file Origins.cpp
 *  \brief Defines class Origins.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <pwd.h>
using namespace std;

#include "Origins.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GSourceInfo.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "WaveformPlot.h"
#include "BasicMap.h"
#include "OridList.h"
#include "gobject++/CssTables.h"

extern "C" {
static int sort_by_time(const void *A, const void *B);
static int sort_by_orid(const void *A, const void *B);
}
using namespace libgorigin;

namespace libgorigin {

class UndoEditOrigin : public UndoAction
{
    public:
	UndoEditOrigin(Origins *origin) {
	    o = origin;
	    error_msg = NULL;
	}
	~UndoEditOrigin() {
	}
	Origins *o;
	cvector<CssOriginClass> origins, origins_copy;
	cvector<CssOrigerrClass> origerrs, origerrs_copy;

	bool undo(void) {
	    return o->undoEditOrigin(this);
	}
	void getLabel(string &label) { 
	    if(origins.size() > 1) {
		label.assign("Undo Edit Origins");
	    }
	    else {
		label.assign("Undo Edit Origin");
	    }
	}
	bool errMsg(string msg) {
	    if(error_msg) { msg.assign(error_msg); return true; }
	    else { msg.clear(); return false; }
	}

    protected:
	char *error_msg;
};

} // namespace libgorigin

Origins::Origins(const char *name, Component *parent, DataSource *ds)
		: Frame(name, parent), DataReceiver(ds)
{
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;
    createInterface();
    init();
}

void Origins::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    create_button = new Button("Create", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);
    edit_button = new Button("Edit", edit_menu, this);
    undo_button = new UndoButton("Undo", edit_menu);
    undo_button->setSensitive(false);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
    remove_button = new Button("Remove Selected From List", view_menu, this);
    select_button = new Button("Select Associated Arrivals", view_menu, this);

    sort_menu = new Menu("Sort", view_menu, true);
    time_toggle = new Toggle("Time", sort_menu, this);
    time_toggle->set(true);
    orid_toggle = new Toggle("Orid", sort_menu, this);

    string prop;
    if(getProperty("originTableSort", prop)) {
	if(!strcasecmp(prop.c_str(), "orid")) {
	    time_toggle->set(false);
	    orid_toggle->set(true);
	}
    }

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    assoc_origin_button = new Button("Associate with Selected Waveforms",
				option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Origins Help", help_menu, this);
    
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    orid_list = new OridList("Working Orid", frame_form, args, n,data_source);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, orid_list->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    sep = new Separator("sep", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 2); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableBackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XmNwidth, 500); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    table = new MultiTable("table", frame_form, data_source, args, n);
    table->setVisible(true);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNselectColumnCallback);
    table->addActionListener(this, XtNeditSaveCallback);


//    fileDialog = NULL;
//    saveDialog = NULL;
//    print_window = NULL;

    addPlugins("Origins", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(create_button, "Create");
	tool_bar->add(edit_button, "Edit");
    }
}

void Origins::init(void)
{
    string prop;
    const char *extra[] = {"file"};
    const char *extra_format[] = {"%s"};

    const char *names[] = {cssOrigin, cssOrigerr};

    if(getProperty("originAttributes", prop)) {
	table->setType(2, names, 1, extra, extra_format, prop);
    }
    else {
	table->setType(2, names, 1, extra, extra_format);
    }

    if(data_source) {
	data_source->addDataListener(this);
    }

    list();

    setButtonsSensitive();

    map = Application::getMap(this);
    if(map) {
	map->addActionListener(this, "selectSourceCallback");
    }
   else {
	Application::addMapListener(this, getParent());
    }
}

Origins::~Origins(void)
{
}

void Origins::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(reason, XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(c->origin || c->origerr || c->select_origin) {
	    list(); // need to list netmags also
	}
    }
    else if(!strcmp(cmd, "Attributes...")) {
        table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Delete")) {
	deleteOrigins();
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectRow((MmTableSelectCallbackStruct *)action_event->getCalldata());
	setButtonsSensitive();
    }
    else if(!strcmp(action_event->getReason(), XtNeditSaveCallback)) {
	editSave((gvector<MultiTableEdit *> *)action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNeditCancelCallback)) {
	// nothing to do.
    }
    else if(!strcmp(cmd, "Remove Selected From List")) {
	removeOrigin();
    }
    else if(!strcmp(cmd, "Edit")) {
	table->editModeOn();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Create")) {
	create();
    }
    else if(!strcmp(cmd, "Select Associated Arrivals")) {
	selectAssociatedArrivals();
    }
    else if(!strcmp(cmd, "Associate with Selected Waveforms")) {
	associateOrigin();
    }
    else if(!strcmp(reason, XtNmapCallback)) {
        if(!map) {
            map = (BasicMap *)comp;
            map->addActionListener(this, XtNsetVisibleCallback);
//            redrawMap();
        }
    }
    else if(!strcmp(cmd, "map")) { // selectSourceCallback
	selectOriginFromMap((MapPlotSource *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Time") || !strcmp(cmd, "Orid")) {
	Toggle *t = comp->getToggleInstance();
	if(t && t->state()) {
	    list();
	    putProperty("originTableSort", comp->getName());
	}
    }
    else if(!strcmp(cmd, "Origins Help")) {
	showHelp(cmd);
    }
}

ParseCmd Origins::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;

    if(parseCompare(cmd, "Edit")) {
	edit_button->activate();
    }
    else if(parseCompare(cmd, "Create")) {
	create_button->activate();
    }
    else if(parseCompare(cmd, "Delete")) {
	delete_button->activate();
    }
    else if(parseCompare(cmd, "Remove_Selected")) {
	remove_button->activate();
    }
    else if(parseCompare(cmd, "Select_Associated_Arrivals")) {
	select_button->activate();
    }
    else if(parseCompare(cmd, "Associate_with_Waveform")) {
	assoc_origin_button->activate();
    }
    else if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
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

void Origins::parseHelp(const char *prefix)
{
    Table::parseHelp(prefix);

    printf("%sedit\n", prefix);
    printf("%screate\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%sremove_selected\n", prefix);
    printf("%sselect_associated_arrivals\n", prefix);
    printf("%sset_primary_origin\n", prefix);
}

void Origins::list(void)
{
    int i, j, l, n;
    int num_rows;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;

    table->removeAllRecords();

    if(!data_source || data_source->getTable(origins) <= 0) return;

    data_source->getTable(origerrs);

    if( time_toggle->state() ) {
	origins.sort(sort_by_time);
    }
    else {
	origins.sort(sort_by_orid);
    }

    num_rows = 0;

    for(i = num_rows = 0; i < origins.size(); i++)
    {
	int dc = origins[i]->getDC();
	int id = origins[i]->orid;
	for(j = n = 0; j < origerrs.size(); j++) {
	    if(origerrs[j]->getDC() == dc && origerrs[j]->orid==id) n++;
	}
	num_rows += (n > 0 ? n : 1);
    }
    for(i = 0; i < origins.size(); i++)
    {
	int dc = origins[i]->getDC();
	int id = origins[i]->orid;
	for(l = n = 0; l < origerrs.size(); l++)
		if(origerrs[l]->getDC() == dc && origerrs[l]->orid == id)
	{
	    n++;
	    table->addRecord(new TableRow(origins[i], origerrs[l]), false);
	}
	if(n == 0) {
	    table->addRecord(new TableRow(origins[i]), false);
	}
    }

    table->list();

//    setSelected(n_arrivals, arrivals);

//    tableSorts();

    setButtonsSensitive();
}

/*
void Arrivals::tableSorts(void)
{
    if(listing_sort == SELECTED_COLUMNS)
    {
	int *cols = (int *)NULL;
	int num = table->getSelectedColumns(&cols);
	table->sortByColumns(cols, num);
	Free(cols);
    }
    else if(listing_sort == PROMOTE_SELECTED)
    {
	table->promoteSelectedRows();
    }
    else if(listing_sort == SORT_ON_COLUMNS)
    {
	table->sortByColumnLabels(sort_on_columns);
    }
}
*/

static int
sort_by_time(const void *A, const void *B)
{
    CssOriginClass *a = *(CssOriginClass **)A;
    CssOriginClass *b = *(CssOriginClass **)B;

    if (fabs(a->time - b->time) < 0.000009) return(0);

    return((a->time < b->time) ? -1 : 1);
}

static int
sort_by_orid(const void *A, const void *B)
{
    CssOriginClass *a = *(CssOriginClass **)A;
    CssOriginClass *b = *(CssOriginClass **)B;

    if( a->orid == b->orid ) return(0);

    return((a->orid < b->orid) ? -1 : 1);
}

void Origins::create(void)
{
    int             i, n_waveforms;
    CssOriginClass       *o, *origin=NULL;
    CssOrigerrClass      *oerr;
    cvector<CssArrivalClass> arrivals;
    gvector<Waveform *> wvec;
    Password        password = NULL;
    CssTableClass        *a;
    CssWftagClass	    *w = NULL;
    gvector<TableRow *> v;

    setCursor("hourglass");
    o = new CssOriginClass();
    o->lat = 0.;    /* so it can be plotted on the map */
    o->lon = 0.;
    oerr = new CssOrigerrClass();

    password = getpwuid(getuid());

    DataSource *ds = NULL;
    a = NULL;

    if((n_waveforms = data_source->getWaveforms(wvec)) > 0)
    {
	for(i = 0; i < n_waveforms; i++)
	{
	    if(wvec[i]->selected)
	    {
		w = new CssWftagClass();
		wvec[i]->ts->source_info.copySource(w);
		a = (CssTableClass *)w;
		o->time = wvec[i]->tbeg() - 3600.;
		ds = wvec[0]->getDataSource();
		break;
	    }
	}
    }
    if(a == NULL && table->getSelectedRecords(v) > 0)
    {
	TableRow *t = v[0];
	origin = (CssOriginClass *)t->tables[0];
	a = (CssTableClass *)origin;
	o->time = origin->time;
	o->lat = origin->lat;
	o->lon = origin->lon;
	o->depth = origin->depth;
	ds = a->getDataSource();
    }

    if(a == NULL && data_source->getTable(arrivals) > 0)
    {
	for(i = 0; i < arrivals.size(); i++)
	{
	    if(data_source->isSelected(arrivals[i]))
	    {
		a = (CssTableClass *)arrivals[i];
		o->time = arrivals[i]->time - 3600.;
		ds = a->getDataSource();
		break;
	    }
	}
    }
    if(a == NULL)
    {
	if(n_waveforms > 0)
	{
	    w = new CssWftagClass();
	    wvec[0]->ts->source_info.copySource(w);
	    a = (CssTableClass *)w;
	    o->time = wvec[0]->tbeg() - 3600.;
	    ds = wvec[0]->getDataSource();
	}
	else if( table->getRecords(v) > 0)
	{
	    TableRow *t = v[0];
	    origin = (CssOriginClass *)t->tables[0];
	    a = (CssTableClass *)origin;
	    o->time = origin->time;
	    o->lat = origin->lat;
	    o->lon = origin->lon;
	    o->depth = origin->depth;
	    ds = a->getDataSource();
	}
	else if(arrivals.size() > 0)
	{
	    a = (CssTableClass *)arrivals[0];
	    o->time = arrivals[0]->time - 3600.;
	    ds = a->getDataSource();
	}
	else
	{
	    o->deleteObject();
	    oerr->deleteObject();
	    showWarning("Need an existing waveform, origin or arrival.");
	    setCursor("default");
	    return;
	}
    }

    if(!ds) {
	showWarning("No data source.\nCannot create origin.");
    }
    else if( ds->originCreate(o, oerr, a, password) )
    {
	for(i = 0; i < n_waveforms; i++)
	{
	    vector<WfdiscPeriod> *wf = wvec[i]->ts->getWfdiscPeriods();
	    if( wf && (int)wf->size() > 0)
	    {
		CssWftagClass *wftag = new CssWftagClass();
		strcpy(wftag->tagname, "orid");
		wftag->tagid = o->orid;
		wftag->wfid = wf->at(0).wf.wfid;
		wftag->setIds(o->getDC(), wf->at(0).wf.wfid);
		data_source->putTable(wftag, false);
	    }
	}
	data_source->putTable(o, false);
	data_source->putTable(oerr);
    }

    if(w) w->deleteObject();

    setCursor("default");
}

void Origins::editSave(gvector<MultiTableEdit *> *v)
{
    int n_waveforms = 0;
    gvector<Waveform *> wvec;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if(data_source) {
	data_source->removeDataListener(this);
	n_waveforms = data_source->getWaveforms(wvec);
    }

    BasicSource::startBackup();

    UndoEditOrigin *undo = new UndoEditOrigin(this);

    for(int i = 0; i < v->size(); i++)
    {
	MultiTableEdit *o = v->at(i);
	TableRow *new_row = o->new_row;
	TableRow *old_row = o->old_row;

	if(!old_row->tables[0]->strictlyEquals(*new_row->tables[0]))
	{
	    if(!undo->origins.contains((CssOriginClass *)new_row->tables[0]))
	    {
		undo->origins.push_back((CssOriginClass *)new_row->tables[0]);
		undo->origins_copy.push_back(
			new CssOriginClass(*(CssOriginClass *)old_row->tables[0]));
		if(wp) {
		    CssOriginClass *origin = (CssOriginClass *)new_row->tables[0];

		    /* make CPlotClass update after changed origins
		     */
		    for(int j = 0; j < n_waveforms; j++) {
			if(wp->getPrimaryOrigin(wvec[j]) == origin)
			{
			    wp->setPrimaryOrigin(wvec[j], origin);
			}
		    }
		    wp->updatePredicted();
		}
            }
        }
        if((new_row->tables[1] && !old_row->tables[1]) ||
                !new_row->tables[1]->strictlyEquals(*old_row->tables[1]))
        {
            if(old_row->tables[1]) {
                undo->origerrs.push_back((CssOrigerrClass *)new_row->tables[1]);
                undo->origerrs_copy.push_back(
			new CssOrigerrClass(*(CssOrigerrClass *)old_row->tables[1]));
            }
            else {
                CssOriginClass *origin = (CssOriginClass *)new_row->tables[0];
                CssOrigerrClass *origerr = (CssOrigerrClass *)new_row->tables[1];
                origerr->orid = origin->orid;
                data_source->putTable(origerr);
            }
        }
    }

    Application::getApplication()->addUndoAction(undo);

    if(data_source) {
	data_source->addDataListener(this);
    }
}

bool Origins::undoEditOrigin(UndoEditOrigin *undo)
{
    if( !BasicSource::undoFileModification() ) return false;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return false;

    for(int i = 0; i < undo->origerrs.size(); i++)
    {
        undo->origerrs_copy[i]->copyTo(undo->origerrs[i], false);
    }

//  num_gs = getGeotools(&gs);
    int n_waveforms = 0;
    gvector<Waveform *> wvec;
    n_waveforms = data_source->getWaveforms(wvec);

    for(int i = 0; i < undo->origins.size(); i++)
    {
        undo->origins_copy[i]->copyTo(undo->origins[i], false);

	if(wp) {
	    CssOriginClass *o = undo->origins[i];
	    for(int j = 0; j < n_waveforms; j++) {
		if(data_source->getPrimaryOrigin(wvec[j]) == o) {
		    wp->setPrimaryOrigin(wvec[j], o);
		}
	    }
	    wp->updatePredicted();
	}
/*
	for(j = 0; j < num_gs; j++) {
	    CPlotChangeOrigin(gs[j]->main_plot, o1);
	}
*/
    }
    list();

    return true;
}

void Origins::removeOrigin(void)
{
    gvector<TableRow *> v;

    table->getSelectedRecords(v);

    if(v.size() <= 0) {
	showWarning("No origin selected.");
	return;
    }

    if(data_source) data_source->removeDataListener(this);

    for(int i = v.size()-1; i >= 0; i--)
    {
	TableRow *t = v[i];
        data_source->removeTable(t->tables[0]);
    }

    list();

    if(data_source) data_source->addDataListener(this);
}

void Origins::deleteOrigins(void)
{
    if(!data_source) return;
    gvector<TableRow *> v;

    if(table->getSelectedRecords(v) <= 0) {
        showWarning("No origin selected.");
        return;
    }
    cvector<CssOriginClass> origins(v.size());
    for(int i = 0; i < v.size(); i++) {
	origins.push_back(v[i]->tables[0]);
    }
    data_source->deleteOrigins(origins);
}

void Origins::selectRow(MmTableSelectCallbackStruct *c)
{
    int j;
    cvector<CssOriginClass> origins;

    if(!data_source || c->nchanged_rows <= 0) return;

    data_source->getTable(origins);

    for(int i = 0; i < c->nchanged_rows; i++)
    {
	int row = c->changed_rows[i];
	TableRow *t = (TableRow *)table->getRecord(row);
	CssOriginClass *o = (CssOriginClass *)t->tables[0];
	int dc = o->getDC();

	for(j = 0; j < origins.size(); j++) {
	    if(dc == origins[j]->getDC() && o->orid == origins[j]->orid) {
		break;
	    }
	}
	if(j == origins.size()) continue;

	bool selected = data_source->isSelected(origins[j]);

	if(c->states[row] != selected)
	{
	    data_source->selectTable(origins[j], c->states[row]);
	}
    }
}

void Origins::setButtonsSensitive(void)
{
    int num = table->numSelectedRows();

    if( !num ) {
	edit_button->setSensitive(false);
	remove_button->setSensitive(false);
	delete_button->setSensitive(false);
	assoc_origin_button->setSensitive(false);
	select_button->setSensitive(false);
    }
    else {
	edit_button->setSensitive(true);
	remove_button->setSensitive(true);
	delete_button->setSensitive(true);
	assoc_origin_button->setSensitive(true);
	select_button->setSensitive(true);
    }
}

void Origins::selectAssociatedArrivals(void)
{
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    gvector<TableRow *> v;

    table->getSelectedRecords(v);

    if(v.size() <= 0) {
	showWarning("No origins selected.");
	return;
    }
    data_source->getTable(arrivals);
    data_source->getTable(assocs);

    for(int i = 0; i < arrivals.size(); i++)
    {
	bool select = false;
	for(int j = 0; j < v.size(); j++)
	{
	    CssOriginClass *o = (CssOriginClass *)v[j]->tables[0];

	    for(int k = 0; k < assocs.size(); k++)
	    {
		if( assocs[k]->arid == arrivals[i]->arid &&
		    assocs[k]->orid == o->orid)
		{
		    select = true;
		    break;
		}
	    }
	    if(select) break;
	}
	data_source->selectRecord(arrivals[i], select, true);
    }
}

void Origins::associateOrigin(void)
{
    int num_waveforms;
    gvector<Waveform *> wvec;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    gvector<TableRow *> v;

    table->getSelectedRecords(v);

    if(v.size() <= 0) {
	showWarning("No origins selected.");
	return;
    }
    else if(v.size() > 1) {
	showWarning("More than one origin selected.");
	return;
    }

    CssOriginClass *o = (CssOriginClass *)v[0]->tables[0];

    if( (num_waveforms = data_source->getSelectedWaveforms(wvec)) > 0)
    {
	for(int i = 0; i < num_waveforms; i++) {
	    wp->setPrimaryOrigin(wvec[i], o);
	}
	wp->updatePredicted();
//	Waveform_List(widget);
    }
    else {
	showWarning("No waveforms selected.");
    }
}

void Origins::selectOriginFromMap(MapPlotSource *src)
{
    gvector<TableRow *> v;

    table->getRecords(v);
    
    for(int i = 0; i < v.size(); i++)
    {
	CssOriginClass *o = (CssOriginClass *)v[i]->tables[0];
	if( src->orid == o->orid &&
		fabs(src->lat - o->lat) < 0.01 &&
		fabs(src->lon - o->lon) < 0.01 &&
		fabs(src->depth - o->depth) < 0.01 &&
		fabs(src->time - o->time) < 0.01)
	{
	    table->selectRow(i, src->sym.selected);
	    break;
	}
    }
}
