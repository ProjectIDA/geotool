/** \file Arrivals.cpp
 *  \brief Defines class Arrivals
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#ifdef HAVE_GSL
#include <gsl/gsl_spline.h>
#endif

#include "Arrivals.h"
#include "WaveformWindow.h"
#include "libgx++.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/gvector.h"
#include "motif++/MotifClasses.h"
#include "AmpMag.h"
#include "MeasureAmpPer.h"
#include "Stassocs.h"
#include "Amp.h"
#include "Response.h"
#include "OridList.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

namespace libgarrival {

class UndoEditArrival : public UndoAction
{
    public:
	UndoEditArrival(Arrivals *arr, bool re_name) {
	    a = arr;
	    rename = re_name;
	    error_msg = NULL;
	}
	~UndoEditArrival() {
	}
	Arrivals *a;
	bool rename;
	cvector<CssArrivalClass> arrivals;
	cvector<CssArrivalClass> arrivals_copy;
	cvector<CssAssocClass> assocs;
	cvector<CssAssocClass> assocs_copy;

	bool undo(void) {
	    return a->undoEditArrival(this);
	}
	void getLabel(string &label) { 
	    if(arrivals.size() > 1) {
		if(rename) label.assign("Undo Rename Arrivals");
		else label.assign("Undo Edit Arrivals");
	    }
	    else {
		if(rename) label.assign("Undo Rename Arrival");
		else label.assign("Undo Edit Arrival");
	    }
	}
	bool errMsg(string msg) {
	    if(error_msg) { msg.assign(error_msg); return true; }
	    else { msg.clear(); return false; }
	}

    protected:
	char *error_msg;
};

} // namespace libgarrival

using namespace libgarrival;

Arrivals::Arrivals(const string &name, Component *parent, DataSource *ds)
			: Frame(name, parent), DataReceiver(ds)
{
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    createInterface();
    init();
}

void Arrivals::createInterface(void)
{
    Arg args[20];
    int n;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    // File Menu
    file_menu = new Menu("File", menu_bar);
    open_button = new Button("Open", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // Edit Menu
    edit_menu = new Menu("Edit", menu_bar);
    add_sta_button = new Button("Add to sta", edit_menu, this);
    add_net_button = new Button("Add to net", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);
    edit_button = new Button("Edit", edit_menu, this);
    rename_button = new Button("Rename", edit_menu, this);
    undo_button = new UndoButton("Undo", edit_menu);

    // View Menu
    view_menu = new Menu("View", menu_bar);
    align_menu = new Menu("Align", view_menu);
    align_button1 = new Button("Align On Predicted", align_menu, this);
    align_button2 = new Button("Align On Selected Arrival", align_menu, this);
    align_button3 = new Button("Align On Selected Waveform Arrival",
				align_menu, this);
    align_button4 = new Button("Align On Selected Waveform Selected Arrival",
				align_menu, this);
    align_button5 = new Button("Align On Next Min", align_menu, this);
    align_button6 = new Button("Align On Next Max", align_menu, this);

    n = 0;
    XtSetArg(args[n], XmNvisibleWhenOff, True); n++;
    selected_orid_toggle = new Toggle("Selected Orid Only", view_menu,
					this, args, n);
    selected_orid_toggle->set(true);

    attributes_button = new Button("Attributes...", view_menu, this);
    pick_attributes_button = new Button("Pick Attributes", view_menu, this);
    loc_attributes_button = new Button("Loc Attributes", view_menu, this);
    n = 0;
    XtSetArg(args[n], XmNvisibleWhenOff, True); n++;
    main_only_toggle = new Toggle("List Main Window Arrivals Only", view_menu,
				this, args, n);
    remove_button = new Button("Remove Selected From List", view_menu, this);

    select_menu = new Menu("Select", view_menu);
    select_all_button = new Button("Select All", select_menu, this);
    deselect_all_button = new Button("Deselect All", select_menu, this);

    sort_menu = new Menu("Sort", view_menu, true);
    n = 0;
    XtSetArg(args[n], XmNvisibleWhenOff, False); n++;
    time_toggle = new Toggle("Time", sort_menu, this, true, args, n);
    time_toggle->set(true);
    sta_time_toggle = new Toggle("Sta/Time", sort_menu, this, true, args, n);
    selected_toggle = new Toggle("Selected", sort_menu, this, true, args, n);
    orid_time_toggle = new Toggle("Orid/Time", sort_menu, this, true, args, n);
    orid_sta_time_toggle = new Toggle("Orid/Sta/Time", sort_menu, this, true,
				args, n);
    delta_sta_time_toggle = new Toggle("Delta/Sta/Time", sort_menu, this, true,
				args, n);
    phase_delta_time_toggle = new Toggle("Phase/Delta/Time", sort_menu, this,
				true, args, n);
    sel_cols_button = new Button("Sort on Selected Column(s)", view_menu, this);
    zoom_sp_button = new Button("Zoom SP", view_menu, this);
    zoom_lp_button = new Button("Zoom LP", view_menu, this);

    // Option Menu
    option_menu = new Menu("Option", menu_bar);
    amp_button = new Button("Amplitudes/Magnitudes...", option_menu, this);
    amp_button->setCommandString("Amplitudes_Magnitudes");
    n = 0;
    XtSetArg(args[n], XmNvisibleWhenOff, True);
    auto_amp_toggle = new Toggle("Auto AmpPer", option_menu, this, args, n);
    assoc_origin_button = new Button("Associate With Origin", option_menu,this);
    disassoc_origin_button = new Button("Disassociate With Origin", option_menu,
				this);
    assoc_stassoc_button = new Button("Associate With Stassoc", option_menu,
				this);
    disassoc_stassoc_button = new Button("Disassociate With Stassoc",
				option_menu, this);
    measure_button = new Button("Measure Amp Per...", option_menu, this);
    stassocs_button = new Button("Stassocs...", option_menu, this);

    // Help Menu
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Arrivals Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    form = new Form("form", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    list_choice = new Choice("list_choice", form, this, args, n);
    list_choice->addItem("List0");
    list_choice->addItem("List1");
    list_choice->addItem("List2");
    list_choice->addItem("List3");
    list_choice->setChoice("List0");

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    add_text = new TextField("add_text", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, list_choice->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, add_text->baseWidget()); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("sw", form, args, n);

    int num_list1 = 86;
    const char *list1[86] = {"FirstP", "FirstS", "Lg", "Lg2", "LR", "Rg", "LQ",
	"P", "T", "I", "PKP", "PKPdf", "PKPab", "PKPbc","Pn", "Pb", "Pg",
	"PKiKP", "pPKPdf", "pPKP", "pPKPbc",
	"pPKPab", "pP", "pPn", "pPb", "pPg", "sPKPdf", "sPKPab", "sP", "sPn",
	"sPb", "sPg", "pPKiKP", "sPKiKP", "PcP", "ScP", "SKPdf", "SKPab",
	"SKiKP", "PKKPdf", "PKKPab", "SKKPdf", "SKKPab", "P'P'df", "P'P'ab",
	"PP", "PnPn", "PbPb", "PgPg", "S", "SKSdf", "SKSac", "Sn", "Sb", "Sg",
	"pSKSdf", "pSKSac", "pS", "sSKSdf", "sSKSac", "sS", "sSn", "sSb",
	"sSg", "ScS", "PcS", "PKSdf", "PKSab", "PKKSdf", "PKKSab", "SKKSdf",
	"SKKSac", "S'S'df", "S'S'ac", "SS", "SnSn", "SbSb", "SgSg", "SP",
	"SPn", "SPg", "PS", "PnS", "PgS", "Pdiff", "Sdiff"};
    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 15); n++;
    alist = new List("alist", sw, args, n, this);
    alist->addItems(list1, num_list1, 1);
    alist->addActionListener(this, XmNsingleSelectionCallback);
    alist->addActionListener(this, XmNmultipleSelectionCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    orid_list = new OridList("orids", frame_form, args, n, data_source);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, orid_list->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    sepv = new Separator("sepv", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, sepv->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
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
    table->addActionListener(this, XtNeditCancelCallback);


//    fileDialog = NULL;
//    saveDialog = NULL;
//    print_window = NULL;

    addPlugins("Arrivals", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(add_sta_button, "Add2sta");
	tool_bar->add(add_net_button, "Add2net");
	tool_bar->add(edit_button, "Edit");
	tool_bar->add(delete_button, "Delete");
	tool_bar->add(rename_button, "Rename");
	tool_bar->add(measure_button, "Measure Amp Per");
	tool_bar->add(select_all_button, "Sel All");
	tool_bar->add(deselect_all_button, "Des All");
    }
}

void Arrivals::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
	if(data_source) {
	    data_source->removeDataListener(this);
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdoubleLineDragCallback);
		wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    data_source->addDataListener(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		wp->addActionListener(this, XtNdoubleLineDragCallback);
		wp->addActionListener(this, XtNdoubleLineScaleCallback);
	    }
	}
	else wp = NULL;
    }
    list();
}

void Arrivals::init(void)
{
    if( wp ) {
	wp->addActionListener(this, XtNdoubleLineDragCallback);
	wp->addActionListener(this, XtNdoubleLineScaleCallback);
    }

    string display_list;
    const char *extra[] = {"file"};
    const char *extra_format[] = {"%s"};

    const char *names[] = {cssArrival, cssAssoc};

    if(getProperty("arrivalAttributes", display_list)) {
	table->setType(2, names, 1, extra, extra_format, display_list);
    }
    else {
	table->setType(2, names, 1, extra, extra_format);
    }
    time_column = -1;
    listing_sort = TIME;
    amp_mag_window = NULL;
    measure_window = NULL;
    stassocs_window = NULL;

    warn_if_hidden = false;

    if(data_source) {
	data_source->addDataListener(this);
    }

    list();

    setButtonsSensitive();
    phaseLineButtons(false);
    add_sta_button->setSensitive(false);
    add_net_button->setSensitive(false);
    sel_cols_button->setSensitive(false);

    phase_line_added = false;
}

Arrivals::~Arrivals(void)
{
    if(data_source) {
	data_source->removeDataListener(this);
	data_source->removeDataReceiver(this);
	if(wp) {
	    wp->removeActionListener(this, XtNdoubleLineDragCallback);
	    wp->removeActionListener(this, XtNdoubleLineScaleCallback);
	}
    }
}

void Arrivals::setVisible(bool visible)
{
    Frame::setVisible(visible);

    if(visible) {
	phaseLine();
    }
    else if(phase_line_added) {
	if( wp ) {
	    wp->deletePhaseLine();
	}
	phase_line_added = false;
    }	    	    
}

void Arrivals::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    const char *extra[] = {"file", "file_index"};
    const char *extra_format[] = {"%s", "%d"};
    const char *names[] = {cssArrival, cssAssoc};
    Toggle *t = comp->getToggleInstance();

    if(!strcmp(cmd, "Attributes...")) {
        table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Amplitudes_Magnitudes")) {
	if(!amp_mag_window) {
	    amp_mag_window = new AmpMag("Amplitudes/Magnitudes", this,
				data_source, wp);
	}
	amp_mag_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Measure Amp Per...")) {
	if(!measure_window) {
	    measure_window = new MeasureAmpPer("Measure Amp Per", this,
					data_source, wp);
	    measure_window->addActionListener(this);
	}
	measure_window->setVisible(true);
    }
    else if(comp == measure_window) {	// amp/per changed
	list();
    }
    else if(!strcmp(cmd, "Stassocs...")) {
	if(!stassocs_window) {
	    stassocs_window = new Stassocs("stassocs", this, data_source);
	}
	stassocs_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Associate With Stassoc")) {
	associateWithStassoc(true);
    }
    else if(!strcmp(cmd, "Disassociate With Stassoc")) {
	associateWithStassoc(true);
    }
    else if(!strcmp(cmd, "Selected Orid Only")) {
	list();
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(c->arrival || c->assoc) {
//	    updateArrivals();
	    list(); // need to list stassocs also
	}
	else if(c->select_arrival) {
	    if(!table->editMode()) {
		table->setSelected();
//	    arrivalSelect();
		setButtonsSensitive();
	    }
	}
	else if(c->select_waveform) {
	    double cur_x;
	    char *p;
	    if(wp && wp->getPhaseLine(&cur_x, &p)) {
		add_sta_button->setSensitive(wp->numSelected() >0 ? true:false);
		add_net_button->setSensitive(wp->numSelected() >0 ? true:false);
	    }
	}
	else if(c->working_orid && selected_orid_toggle->state()) {
	    list();
	}
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectRow((MmTableSelectCallbackStruct *)action_event->getCalldata());
	setButtonsSensitive();
    }
    else if(!strcmp(action_event->getReason(), XtNselectColumnCallback)) {
	selectColumn();
    }
    else if(!strcmp(action_event->getReason(), XtNeditSaveCallback)) {
	editSave((gvector<MultiTableEdit *> *)action_event->getCalldata());
    }
    else if(!strcmp(action_event->getReason(), XtNeditCancelCallback)) {
	editCancel();
    }
    else if(!strcmp(action_event->getReason(), XtNretimeDragCallback)) {
	retime((CssArrivalClass *)action_event->getCalldata());
    }
    else if(comp == alist) {
	phaseLine();
    }
    else if(comp == add_text) {
	renamePhaseLine();
    }
    else if(!strcmp(cmd, "Time") && t && t->state()) {
	if(listing_sort != TIME) {
	    listing_sort = TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Sta/Time") && t && t->state()) {
	if(listing_sort != STA_TIME) {
	    listing_sort = STA_TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Selected") && t && t->state()) {
	if(listing_sort != SELECTED) {
	    listing_sort = SELECTED;
	    list();
	}
    }
    else if(!strcmp(cmd, "Orid/Time") && t && t->state()) {
	if(listing_sort != ORID_TIME) {
	    listing_sort = ORID_TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Orid/Sta/Time") && t && t->state()) {
	if(listing_sort != ORID_STA_TIME) {
	    listing_sort = ORID_STA_TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Delta/Sta/Time") && t && t->state()) {
	if(listing_sort != DELTA_STA_TIME) {
	    listing_sort = DELTA_STA_TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Phase/Delta/Time") && t && t->state()) {
	if(listing_sort != PHASE_DELTA_TIME) {
	    listing_sort = PHASE_DELTA_TIME;
	    tableSorts();
	}
    }
    else if(!strcmp(cmd, "Sort on Selected Column(s)")) {
	sortByColumns();
    }
    else if(!strcmp(cmd, "Column Sort") && t && t->state()) {
	table->sortByColumnLabels(comp->getName());
	listing_sort = SORT_ON_COLUMNS;
	sort_on_columns = comp->getName();
    }
    else if(!strcmp(cmd, "Zoom SP")) {
	arrivalZoom(true);
    }
    else if(!strcmp(cmd, "Zoom LP")) {
	arrivalZoom(false);
    }
    else if(!strncmp(cmd, "Align ", 6)) {
	alignWaveforms(cmd);
    }
    else if(!strcmp(cmd, "Select All")) {
	selectAll(true);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	selectAll(false);
    }
    else if(!strcmp(cmd, "Pick Attributes")) {

	table->setType(2, names, 2, extra, extra_format,
	    "sta,%s,chan,%s,qual,%s,fm,%s,phase,%s,time,%t,amp,%.2f,per,%.2f");
	table->list();
	tableSorts();
    }
    else if(!strcmp(cmd, "Loc Attributes")) {
	table->setType(2, names, 2, extra, extra_format,
		"sta,%s,phase,%s,time,%t,deltim,%.3f,timedef,%s,azimuth,\
%.2f,delaz,%.2f,azdef,%s,slow,%.2f,delslo,%.2f,slodef,%s");
	table->list();
	tableSorts();
    }
    else if(!strcmp(cmd, "Remove Selected From List")) {
	updateArrivals();
	list(); // need to list stassocs also
    }
    else if(!strcmp(cmd, "Auto AmpPer")) {
	if(auto_amp_toggle->state()) {
	    putProperty("doAutoAmp", "true", false, this);
	}
	else {
	    putProperty("doAutoAmp", "false", false, this);
	}
    }
    else if(!strcmp(cmd, "Associate With Origin")) {
	associateWithOrigin(true);
    }
    else if(!strcmp(cmd, "Disassociate With Origin")) {
	associateWithOrigin(false);
    }
    else if(!strcmp(cmd, "Add to sta")) {
	addArrival(false);
    }
    else if(!strcmp(cmd, "Add to net")) {
	addArrival(true);
    }
    else if(!strcmp(cmd, "Edit")) {
	editOn();
    }
    else if(!strcmp(cmd, "Rename")) {
	string new_name;
	TextQuestion *q = new TextQuestion("Rename Arrival", this,
			"Enter new name", "Apply", "Cancel");
	q->getAnswer(new_name);
	q->destroy();
	if(!new_name.empty()) {
	    renameArrival(new_name);
	}
    }
    else if(!strcmp(cmd, "Delete")) {
	deleteArrivals();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Arrivals Help")) {
	showHelp("Arrivals Help");
    }
}

ParseCmd Arrivals::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if(parseArg(cmd, "attributes", c)) {
	TableAttributes *ta = table->showAttributes(true);
	if(ta) {
	    return ta->parseCmd(c, msg);
	}
    }
    else if(parseArg(cmd, "Amplitudes_Magnitudes", c) ||
		parseArg(cmd, "Amp_Mag", c)) {
	if(!amp_mag_window) {
	    amp_mag_window = new AmpMag("Amplitudes/Magnitudes", this,
					data_source, wp);
	}
	return amp_mag_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Measure_Amp_Per", c)) {
	if(!measure_window) {
	    measure_window = new MeasureAmpPer("Measure Amp Per", this,
					data_source, wp);
	}
	return measure_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Stassocs", c)) {
	if(!stassocs_window) {
	    stassocs_window = new Stassocs("stassocs", this, data_source);
	}
	return stassocs_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "select_cursor", c)) {
	return alist->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "position_cursor", c)) {
    }
    else if(parseCompare(cmd, "select_all")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "deselect_all")) {
	deselect_all_button->activate();
    }
    else if(parseArg(cmd, "sort", c)) {
	return sort_menu->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "align_on_predicted")) {
	align_button1->activate();
    }
    else if(parseCompare(cmd, "align_on_selected_arrival")) {
	align_button2->activate();
    }
    else if(parseCompare(cmd, "align_on_selected_waveform_arrival")) {
	align_button3->activate();
    }
    else if(parseCompare(cmd, "align_on_selected_waveform_selected_arrival")) {
	align_button4->activate();
    }
    else if(parseCompare(cmd, "align_on_next_min")) {
	align_button5->activate();
    }
    else if(parseCompare(cmd, "align_on_next_max")) {
	align_button6->activate();
    }
    else if(parseCompare(cmd, "add")) {
	add_net_button->activate();
    }
    else if(parseCompare(cmd, "delete")) {
	delete_button->activate();
    }
    else if(parseArg(cmd, "rename", c)) {
    }
    else if(parseCompare(cmd, "measure_amplitude") ||
	    parseCompare(cmd, "measure_ml") ||
	    parseCompare(cmd, "measure_mb") ||
	    parseCompare(cmd, "compute_magnitudes") ||
	    parseCompare(cmd, "amplitudes.", 11) ||
	    parseCompare(cmd, "stamags.", 8) ||
	    parseCompare(cmd, "netmags.", 8) ||
	    parseCompare(cmd, "wplot.", 6))
    {
	if(!amp_mag_window) {
	    amp_mag_window = new AmpMag("Amplitudes/Magnitudes", this,
					data_source, wp);
	}
	return amp_mag_window->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar Arrivals::parseVar(const string &name, string &value)
{
    string c;

    if(parseString(name, "table", c)) {
	return table->parseVar(c, value);
    }
    else if(parseString(name, "Amplitudes_Magnitudes", c) ||
	parseString(name, "amp_mag", c)) {
	if(!amp_mag_window) {
	    amp_mag_window = new AmpMag("Amplitudes_Magnitudes", this,
					data_source, wp);
	}
	return amp_mag_window->parseVar(c, value);
    }
    else if(parseString(name, "Measure_Amp_Per", c)) {
	if(!measure_window) {
	    measure_window = new MeasureAmpPer("Measure Amp Per", this,
					data_source, wp);
	}
	return measure_window->parseVar(c, value);
    }
    else if(parseString(name, "Stassocs", c)) {
	if(!stassocs_window) {
	    stassocs_window = new Stassocs("stassocs", this, data_source);
	}
	return stassocs_window->parseVar(c, value);
    }
    return Frame::parseVar(name, value);
}

void Arrivals::parseHelp(const char *prefix)
{
    printf("%sadd\n", prefix);
    printf("%salign_on_predicted\n", prefix);
    printf("%salign_on_selected_arrival\n", prefix);
    printf("%salign_on_selected_waveform_arrival\n", prefix);
    printf("%salign_on_selected_waveform_selected_arrival\n", prefix);
    printf("%salign_on_next_min\n", prefix);
    printf("%salign_on_next_max\n", prefix);

    char p[200];
    snprintf(p, sizeof(p), "%sattributes.", prefix);
    TableAttributes::parseHelp(p);

    printf("%sdelete\n", prefix);

    printf("%sselect_cursor PHASE\n", prefix);
//    printf("%sposition cursor TIME\n", prefix);
    printf("%sselect_all\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%ssort=(Time,Sta/Time,Selected,Orid/Time,Orid/Sta/Time,\n", prefix);
    printf("\tDelta/Sta/Time,Phase/Delta/Time)\n");

    Table::parseHelp(prefix);
}

void Arrivals::selectAll(bool select)
{
    table->selectAllRows(select);
    if(wp) {
	wp->selectAllArrivals(select);
    }
    setButtonsSensitive();
}

void Arrivals::list(void)
{
    int dc, i, j, n;
    long arid, orid = -1;
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;

    table->removeAllRecords();

    data_source->getTable(arrivals);
    data_source->getTable(assocs);

    if(selected_orid_toggle->state() && data_source) {
	orid = data_source->getWorkingOrid();
    }
    if(!selected_orid_toggle->state()) {
	for(i = 0; i < arrivals.size(); i++)
	{
	    dc = arrivals[i]->getDC();
	    arid = arrivals[i]->arid;
	    for(j = n = 0; j < assocs.size(); j++)
		if(assocs[j]->getDC() == dc && assocs[j]->arid == arid)
	    {
		n++;
		table->addRecord(new TableRow(arrivals[i], assocs[j]), false);
	    }
	    if(n == 0) {
		table->addRecord(new TableRow(arrivals[i]), false);
	    }
	}
    }
    else {
	for(i = 0; i < arrivals.size(); i++)
	{
	    dc = arrivals[i]->getDC();
	    arid = arrivals[i]->arid;
	    for(j = n = 0; j < assocs.size(); j++)
		if(assocs[j]->getDC() == dc && assocs[j]->arid == arid)
	    {
		n++;
		if(assocs[j]->orid == orid) {
		    table->addRecord(new TableRow(arrivals[i], assocs[j]), false);
		}
	    }
	    if(n == 0) {
		table->addRecord(new TableRow(arrivals[i]), false);
	    }
	}
    }

    table->list();

    tableSorts();
}

void Arrivals::tableSorts(void)
{
    if(listing_sort == TIME) {
	table->sortByColumnLabels("time");
    }
    else if(listing_sort ==  STA_TIME) {
	table->sortByColumnLabels("sta,time");
    }
    else if(listing_sort == SELECTED) {
	table->promoteSelectedRows();
    }
    else if(listing_sort ==  ORID_TIME) {
	table->sortByColumnLabels("orid,time");
    }
    else if(listing_sort ==  ORID_STA_TIME) {
	table->sortByColumnLabels("orid,sta,time");
    }
    else if(listing_sort ==  DELTA_STA_TIME) {
	table->sortByColumnLabels("delta,sta,time");
    }
    else if(listing_sort ==  PHASE_DELTA_TIME) {
	table->sortByColumnLabels("phase,delta,time");
    }
    else if(listing_sort == SELECTED_COLUMNS)
    {
	vector<int> cols;
	table->getSelectedColumns(cols);
	table->sortByColumns(cols);
    }
    else if(listing_sort == SORT_ON_COLUMNS)
    {
	table->sortByColumnLabels(sort_on_columns);
    }
}

void Arrivals::selectRow(MmTableSelectCallbackStruct *c)
{
    int i, row;
    Waveform *w;
    CssArrivalClass *a;
    bool selected;

    if(!data_source || c->nchanged_rows <= 0) return;

    gvector<TableRow *> rows;
    table->getRecords(rows);

    for(i = 0; i < c->nchanged_rows; i++)
    {
	row = c->changed_rows[i];
	a = (CssArrivalClass *)rows[row]->tables[0];
	selected = data_source->isSelected(a);

	if(c->states[row] != selected) {
	    data_source->selectRecord(a, c->states[row]);
	}
    }

    if(c->changed_rows > 0 && wp)
    {
	Arg args[1];
	bool measure_box;
	XtSetArg(args[0], XtNmeasureBox, &measure_box);
	wp->getValues(args, 1);
	if(measure_box && measure_window &&
		measure_window->review_toggle->state())
	{
	    row = c->changed_rows[c->nchanged_rows-1];
	    a = (CssArrivalClass *)rows[row]->tables[0];
	    if(data_source->isSelected(a))
	    {
		wp->reviewMeasurement(a);
	    }
	    else if((w = wp->getArrivalData(a)) != NULL)
	    {
		wp->removeMeasureBox(w);
		measure_window->list();
	    }
	}
    }
}

void Arrivals::phaseLine(void)
{
    if( wp )
    {
	int num_selected;
	char *p = NULL;
	vector<string> items;
	double cur_x, xmin, xmax;

	num_selected = alist->getSelectedItems(items);
	if(num_selected < 1)
	{
	    wp->deletePhaseLine();
	    phaseLineButtons(false);
	    add_sta_button->setSensitive(false);
	    add_net_button->setSensitive(false);
	    add_text->setString("");
	    return;
	}
	add_text->setString(items[0]);
	phaseLineButtons(true);
	add_sta_button->setSensitive(wp->numSelected() > 0 ? true : false);
	add_net_button->setSensitive(wp->numSelected() > 0 ? true : false);

        /* if we have a phase line out of view, delete it */
	if(wp->getPhaseLine(&cur_x, &p))
	{
	    wp->getXLimits(&xmin, &xmax);

	    if(cur_x < xmin || cur_x > xmax) wp->deletePhaseLine();
	}

	wp->addPhaseLine(items[0].c_str());
	phase_line_added = true;
    }
}

void Arrivals::renamePhaseLine(void)
{
    if( wp )
    {
	vector<string> items;
	string phase;

	add_text->getString(phase);

	if(phase.length() > 0) {
	    char *p = NULL;
	    double cur_x, xmin, xmax;
	    /* if we have a phase line out of view, delete it */
	    if(wp->getPhaseLine(&cur_x, &p))
	    {
		wp->getXLimits(&xmin, &xmax);

		if(cur_x < xmin || cur_x > xmax) wp->deletePhaseLine();
	    }
	    wp->renamePhaseLine(phase.c_str());
	}
	else if(alist->getSelectedItems(items) > 0) {
	    wp->renamePhaseLine(items[0].c_str());
	}
    }
}

void Arrivals::addArrival(bool add_to_net)
{
    int i, n, ngroups;
    vector<int> ncmpts;
    char *phase, warning[50], msg[2048];
    string ph;
    bool duplicate, not_visible, no_data, all_selected;
    vector<bool> do_it;
    double x, time, tt, slowness;
    gvector<Waveform *> wvec, c;
    Waveform *w;
    CssArrivalClass *arrival;
    Password password = NULL;
    CPlotMeasure m;
    double deltim = -1., azimuth = -1., delaz = -1., slow = -1., delslo = -1.;
    double ema = -1., rect = -1., snr = -1.;

    if( !wp ) return;

    if(!wp->getPhaseLine(&x, &phase)) {
	showWarning("No arrival name selected.");
	return;
    }

    ngroups = wp->getSelectedComponents(ncmpts, c);
    n = 0;
    for(i = 0; i < ngroups; i++) {
	int num = 0;
	for(int j = 0; j < ncmpts[i]; j++) {
	    if(c[n+j]->selected) num++;
	}
	if(num > 1) {
	    showWarning("More that one component selected for sta %s",
		c[n]->sta());
	    return;
	}
    }

    if(wp->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }

    duplicate = false;
    not_visible = false;
    no_data = false;
    all_selected = false;

    msg[0] = '\0';
    for(i = 0; i < wvec.size(); i++)
    {
	w = wvec[i];
	if(!w->visible && !not_visible && warn_if_hidden)
	{
	    not_visible = true;
	    snprintf(warning,sizeof(warning),"Selected waveforms not visible.");
	    if((int)strlen(msg) + (int)strlen(warning) + 1 < (int)sizeof(msg))
	    {
		strcat(msg, warning);
	    }
	}
	/* check for data at arrival time
	 */
	time = w->tbeg() + x - w->scaled_x0;
	if(w->segment(time) == NULL)
	{
	    snprintf(warning, sizeof(warning), "%s/%s: no data\n",
			w->sta(), w->chan());
	    if((int)strlen(msg) + (int)strlen(warning)+1 < (int)sizeof(msg)) {
		strcat(msg, warning);
	    }
	    no_data = true;
	}
    }

    /* see if all waveforms are selected */
    if(!duplicate && !not_visible && !no_data && wvec.size() > 1)
    {
	gvector<Waveform *> v;
	if(wp->getWaveforms(v) == wvec.size()) {
	    all_selected = true;
	    stringcpy(msg, "All waveforms are selected", sizeof(msg));
	}
    }

    if(duplicate || not_visible || no_data || all_selected)
    {
	int answer = Question::askQuestion("Add Arrival Confirm", this, msg,
					"Add", "Cancel");
	if(answer == 2) { // Cancel
	    return;
	}
    }

    setCursor("hourglass");

    password = getpwuid(getuid());

    if(wvec.size() > 1) {
	excludeDuplicates(wvec, x, do_it);
    }
		
    for(i = 0; i < wvec.size(); i++)
	if(wvec.size() == 1 || do_it[i])
    {
	time = wvec[i]->tbeg() + x - wvec[i]->scaled_x0;

	if(!strcmp(phase, "FirstP"))
	{
	    CssOriginClass *o = wp->getPrimaryOrigin(wvec[i]);

	    if(wp->firstArrival(o, wvec[i]->lat(), wvec[i]->lon(),
		wvec[i]->elev(), wvec[i]->net(), wvec[i]->sta(),
		'P', ph, &tt, &slowness) <= 0)
	    {
		showWarning("Cannot compute FirstP for %s/%s; no origin.",
			wvec[i]->sta(), wvec[i]->chan());
		continue;
	    }
	}
	else if(!strcmp(phase, "FirstS"))
	{
	    CssOriginClass *o = wp->getPrimaryOrigin(wvec[i]);

	    if(wp->firstArrival(o, wvec[i]->lat(), wvec[i]->lon(),
		wvec[i]->elev(), wvec[i]->net(), wvec[i]->sta(),
		'S', ph, &tt, &slowness) <= 0)
	    {
		showWarning("Cannot compute FirstS for %s/%s; no origin.",
			wvec[i]->sta(), wvec[i]->chan());
		continue;
	    }
	}
	else {
	    ph.assign(phase);
	}

	arrival = wp->makeArrival(wvec[i], password, time, ph, deltim,
			azimuth, delaz, slow, delslo, ema, rect, snr,
			add_to_net);

	if(arrival) {
	    wp->putArrivalWithColor(arrival, stringToPixel("black"));
	}

	if(arrival && wp->getWaveformMeasureBox(wvec[i], &m))
	{
	    DataSource *ds = arrival->getDataSource();

	    arrival->amp_cnts = m.amp_cnts;
	    arrival->amp_Nnms = m.amp_Nnms;
	    arrival->amp_nms = m.amp_nms;
	    arrival->zp_Nnms = m.zp_Nnms;
	    arrival->period = m.period;
	    arrival->box_location = True;
	    arrival->boxtime = m.w->tbeg() + m.left_side;
	    arrival->boxmin = m.bottom_side;
	    errorMsg(); // clear last error message
	    if(ds) ds->changeArrival(arrival, m.w->ts, CHANGE_AMP_PER);
	    showErrorMsg();
	}
    }

    list();

    setCursor("default");
}

void Arrivals::excludeDuplicates(gvector<Waveform *> &wvec, double x,
			vector<bool> &do_it)
{
    string prev_net;

    Waveform::sortByNet(wvec);

    do_it.clear();
    if(wvec.size() > 0) {
	prev_net.assign(wvec[0]->net());
	do_it.push_back(true);
    }

    for(int i = 1; i < wvec.size(); i++)
    {
	do_it.push_back(true);
	if(!prev_net.compare(wvec[i]->net()))
	{
	    if (fabs((wvec[i-1]->tbeg() + x-wvec[i-1]->scaled_x0) -
		(wvec[i]->tbeg() + x - wvec[i]->scaled_x0)) < 0.1)
	    {
		do_it[i] = false;
	    }
	}
	else {
	    prev_net.assign(wvec[i]->net());
	}
    }
}
void Arrivals::arrivalZoom(bool sp)
{
    char    *phase;
    double  cur_x, x_min, x_max, y_min, y_max;

    if( !wp ) return;

    if(!wp->getPhaseLine(&cur_x, &phase)) {
	showWarning("Need a phase line.");
	return;
    }

    if(sp) {
	double sp_window = getProperty("spArrivalWindow", 20.);
	x_min = cur_x - .5*sp_window;
	x_max = cur_x + .5*sp_window;
    }
    else {
	double lp_window = getProperty("lpArrivalWindow", 100.);
	x_min = cur_x - .5*lp_window;
	x_max = cur_x + .5*lp_window;
    }
/*
    if((mag = localWidget(widget, "*_Magnify")) != NULL && IsUp(mag)) {
        n = 0;
        XtSetArg(args[n], XtNyMin, &y_min); n++;
        XtSetArg(args[n], XtNyMax, &y_max); n++;
        if((w = findWidget("ArrivalZoomCB", widget, "*_Magnify*plot1"))
                != NULL)
        {
                XtGetValues(w, args, n);
        }
        AxesMag((AxesWidget)w, x_min, x_max, y_min, y_max);
    }
    else {
*/
	wp->getYLimits(&y_min, &y_max);
        wp->zoom(x_min, x_max, y_min, y_max, false);
//    }
}

void Arrivals::alignWaveforms(const string &cmd)
{
    if( !wp ) return;

    if(!cmd.compare("Align On Predicted")) {
	alignPredicted();
    }
    else if(!cmd.compare("Align On Selected Arrival")) {
	if( !wp->alignOnArrival(CPLOT_SELECTED_ARRIVAL) ) {
	    showWarning(wp->getError());
	}
    }
    else if(!cmd.compare("Align On Selected Waveform Arrival")) {
	if( !wp->alignOnArrival(CPLOT_SELECTED_WAVEFORM) ) {
	    showWarning(wp->getError());
	}
    }
    else if(!cmd.compare("Align On Selected Waveform Selected Arrival")) {
	if( !wp->alignOnArrival(CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM) ) {
	    showWarning(wp->getError());
	}
    }
    else if(!cmd.compare("Align On Next Min")) {
	alignMinMax(true);
    }
    else if(!cmd.compare("Align On Next Max")) {
	alignMinMax(false);
    }
}

void Arrivals::alignPredicted(void)
{
    char *phase;
    double x;

    if( !wp ) return;

    if( !wp->getPhaseLine(&x, &phase) ) {
	showWarning("No Phase selected.");
	return;
    }

    if(!strcmp(phase, "FirstP")) {
	wp->alignWaveforms(ALIGN_PREDICTED_P);
    }
    else {
	wp->alignWaveforms(ALIGN_PREDICTED_ARRIVAL, phase);
    }
}

#define UP 1
#define DN 2

void Arrivals::alignMinMax(bool align_min)
{
#ifdef HAVE_GSL
    char	*phase;
    int		i, j, k, l, n, dir, look_for;
    double	x[11], y[11], spline_x, spline_y, dx, sx, sy;
    double	cur_x, scaled_x0, tbeg;
    GDataPoint	*d1;
    GSegment	*s;
    gvector<Waveform *> wvec;
    Waveform *w;
    gsl_spline *spline;
    gsl_interp_accel *acc;

    if( !wp ) return;

    if(wp->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }
    if(align_min) {
	look_for = UP;
    }
    else {
	look_for = DN;
    }

    if( !wp->getPhaseLine(&cur_x, &phase) ) {
	showWarning("Need a phase line.");
	return;
    }

    spline = gsl_spline_alloc(gsl_interp_cspline, 11);
    acc = gsl_interp_accel_alloc();

    for(k = 0; k < wvec.size(); k++)
    {
	w = wvec[k];
	tbeg = w->tbeg();
	d1 = w->upperBound(tbeg + cur_x-w->scaled_x0);
	s = d1->segment();
	n = s->length();
	for(i = d1->index()+1; i < n; i++)
	{
	    dir = (s->data[i] > s->data[i-1]) ? UP : DN;
	    if(dir == look_for) break;
	}
	for(; i < n; i++)
	{
	    dir = (s->data[i] > s->data[i-1]) ? UP : DN;
	    if(dir != look_for) {
		i--;
		break;
	    }
	}
	if(d1) d1->deleteObject();
	if(i > 4 && i < n - 5)
	{
	    for(j=i-5, l=0; j <= i+5; j++, l++) {
		x[l] = s->tbeg()+j*s->tdel() - tbeg;
		y[l] = s->data[j];
	    }
	    /* cubic spline interpolation */
	    gsl_spline_init(spline, x, y, 11);

	    spline_x = x[5];
	    spline_y = s->data[i];
	    dx = (x[6] - x[5])/100.;
	    for(j = 1; j <= 100; j++)
	    {
		sx = x[5] + j*dx;
		sy = gsl_spline_eval(spline, sx, acc);
		dir = (sy > spline_y) ? UP : DN;
		if(dir != look_for) break;
		spline_x = sx;
		spline_y = sy;
	    }
	    if(j == 1)
	    {
		spline_x = x[5];
		spline_y = s->data[i];
		dx = (x[5] - x[4])/100;
		for(j = 1; j <= 100; j++)
		{
		    sx = x[5] - j*dx;
		    sy = gsl_spline_eval(spline, sx, acc);
		    dir = (sy > spline_y) ? UP : DN;
		    if(dir != look_for) break;
		    spline_x = sx;
		    spline_y = sy;
		}
	    }
	}
	else {
	    spline_x = s->time(i) - tbeg;
	}
	scaled_x0 = cur_x - spline_x;
	wp->positionOne(w, scaled_x0, w->scaled_y0);
    }
    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);
#else
    fprintf(stderr, "Option unavailable without libgsl.\n");
#endif
}

void Arrivals::updateArrivals(void)
{
    int i, j, k;
    gvector<Waveform *> wvec;
    cvector<CssArrivalClass> sel_arr;

    if( !wp ) return;

    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    cvector<CssStassocClass> stassocs;
    wp->getTable(arrivals);
    wp->getTable(assocs);
    wp->getTable(stassocs);

    wp->getSelectedTable(sel_arr);

    wp->getWaveforms(wvec, false);

    bool main_window_only = main_only_toggle->state();

    for(i = 0; i < arrivals.size(); i++)
    {
	for(k = 0; k < sel_arr.size() && arrivals[i] != sel_arr[k]; k++);
	if(main_window_only || k < sel_arr.size())
	{
	    int dc = arrivals[i]->getDC();
	    long arid = arrivals[i]->arid;
	    for(j = 0; j < wvec.size(); j++)
	    {
		if((!strcmp(wvec[j]->sta(), arrivals[i]->sta) ||
		    !strcmp(wvec[j]->net(), arrivals[i]->sta)) &&
		    wvec[j]->tbeg() <= arrivals[i]->time &&
		    wvec[j]->tend() >= arrivals[i]->time) break;
	    }
	    if(j == wvec.size())
	    {
		for(j = 0; j < assocs.size(); j++)
		    if( assocs[j]->getDC() == dc && assocs[j]->arid == arid)
		{
		    wp->removeTable(assocs[j], false);
		}
		for(j = 0; j < stassocs.size(); j++)
		    if(stassocs[j]->stassid == arrivals[i]->stassid)
		{
		    wp->removeTable(stassocs[j], false);
		}
		wp->removeTable(arrivals[i]);
	    }
	}
    }
}

void Arrivals::associateWithOrigin(bool associate)
{
    int i, j, k, n, n_assocs;
    cvector<CssArrivalClass> arrivals;
    cvector<CssOriginClass> origins;
    cvector<CssAssocClass> a;
    CssAssocClass **assocs=NULL;
    CssOriginClass *o = NULL;
    CssTableClass *t;

    if(!data_source) return;

    data_source->getTable(origins);

    if (origins.size() < 1) {
      showWarning("There are no origins in location table.");
      return;
    }
    else if(origins.size() == 1) {
	o = origins[0];
    }
    else if(origins.size() > 1) {
	long orid;
	if((orid = data_source->getWorkingOrid()) < 0) {
	    showWarning("No orid selected.");
	    return;
	}
	for(i = 0; i < origins.size() && origins[i]->orid != orid; i++);
	if(i < origins.size()) {
	    o = origins[i];
	}
	else {
	    showWarning("Cannot find origin.");
	    return;
	}
    }

    if( data_source->getSelectedTable(arrivals) <= 0 )
    {
	showWarning("No arrivals selected.");
	return;
    }
    n_assocs = data_source->getTable(a);
    assocs = new CssAssocClass * [n_assocs];
    a.copyInto(assocs);

    setCursor("hourglass");

    for(i = n = 0; i < arrivals.size(); i++)
    {
	int dc = arrivals[i]->getDC();
	long arid = arrivals[i]->arid;
	n++;
	DataSource *ds = arrivals[i]->getDataSource();
	if( !ds ) {
	    showWarning("No data source for arrival %d", arrivals[i]->arid);
	    continue;
	}
	if(!associate)
	{
	    for(j = 0; j < n_assocs; j++)
		if( assocs[j]->getDC() == dc && assocs[j]->arid == arid &&
		    assocs[j]->orid == o->orid)
	    {
		ds->deleteTable(this, assocs[j]);
		n_assocs--;
		for(k = j; k < n_assocs; k++) assocs[k] = assocs[k+1];
	    }
	}
	else
	{
	    for(j = 0; j < n_assocs; j++)
	    {
		if(assocs[j]->getDC() == dc && assocs[j]->arid == arid &&
		    (assocs[j]->orid == -1 || assocs[j]->orid ==o->orid)) break;
	    }
	    if(j < n_assocs && assocs[j]->orid == -1)
	    {
		assocs[j]->orid = o->orid;
		t = new CssAssocClass(*assocs[j]);
		stringcpy(assocs[j]->timedef, "d", sizeof(assocs[j]->timedef));
		ds->changeTable(t, assocs[j]);
		t->deleteObject();
	    }
	    else if(j == n_assocs)
	    {
		CssAssocClass *assoc = new CssAssocClass();
		stringcpy(assoc->timedef, "d", sizeof(assoc->timedef));
		arrivals[i]->copySourceTo(assoc, o->orid);
		assoc->arid = arrivals[i]->arid;
		assoc->orid = o->orid;
		assoc->setIds(dc, arrivals[i]->arid);
		stringcpy(assoc->sta, arrivals[i]->sta, sizeof(assoc->sta));
		stringcpy(assoc->phase, arrivals[i]->iphase,
				sizeof(assoc->phase));

		ds->addTable(assoc);
		data_source->putTable(assoc);
	    }
	}
    }

    delete [] assocs;

    if(n) {
	list();
    }
    setCursor("default");
}

void Arrivals::associateWithStassoc(bool associate)
{
    int i, stassid, n;
    cvector<CssArrivalClass> arrivals;
    cvector<CssStassocClass> stassocs;
    CssStassocClass *s=NULL;

    if(!data_source) return;

    data_source->getTable(stassocs);

    for(i = n = 0; i < stassocs.size(); i++)
    {
	if(data_source->isSelected(stassocs[i])) {
	    n++;
	    s = stassocs[i];
	}
    }

    if(n == 0) {
	showWarning("No stassoc selected.");
	return;
    }
    else if(n > 1) {
	showWarning("More than one stassoc selected.");
	return;
    }

    data_source->getSelectedTable(arrivals);

    stassid = (associate) ? s->stassid : -1;

    for(i = n = 0; i < arrivals.size(); i++)
    {
	if(strcmp(arrivals[i]->sta, s->sta)) continue;

	n++;
	DataSource *ds = arrivals[i]->getDataSource();
	if( !ds ) {
	    showWarning("No data source for arrival %d", arrivals[i]->arid);
	    continue;
	}
	arrivals[i]->stassid = stassid;
	ds->changeArrival(arrivals[i], NULL, CHANGE_STASSID);
    }

    if(!n) {
	showWarning("No arrivals selected.");
    }
    else {
	list();
    }
}

void Arrivals::setButtonsSensitive(void)
{
    int num = table->numSelectedRows();

    if(!num) {
	edit_button->setSensitive(false);
	delete_button->setSensitive(false);
	rename_button->setSensitive(false);
	remove_button->setSensitive(false);
	assoc_origin_button->setSensitive(false);
	disassoc_origin_button->setSensitive(false);
	assoc_stassoc_button->setSensitive(false);
	disassoc_stassoc_button->setSensitive(false);
    }
    else {
	edit_button->setSensitive(true);
	delete_button->setSensitive(true);
	rename_button->setSensitive(true);
	remove_button->setSensitive(true);
	assoc_origin_button->setSensitive(true);
	disassoc_origin_button->setSensitive(true);
	assoc_stassoc_button->setSensitive(true);
	disassoc_stassoc_button->setSensitive(true);
    }
}

void Arrivals::phaseLineButtons(bool set)
{
    align_button1->setSensitive(set);
    align_button2->setSensitive(set);
    align_button3->setSensitive(set);
    align_button4->setSensitive(set);
    zoom_sp_button->setSensitive(set);
    zoom_lp_button->setSensitive(set);
}

void Arrivals::selectColumn(void)
{
    bool set = table->numSelectedColumns() ? true : false;
    sel_cols_button->setSensitive(set);
}

void Arrivals::sortByColumns(void)
{
    vector<int> cols;
    int len, num = table->getSelectedColumns(cols);
    const char **labels = table->getColumnLabels();
    Toggle *toggle;
    char label[200];

    len = 0;
    for(int i = 0; i < num; i++) {
	if(i < num-1) {
	    snprintf(label+len, sizeof(label)-len, "%s,", labels[cols[i]]);
	}
	else {
	    snprintf(label+len, sizeof(label)-len, "%s", labels[cols[i]]);
	}
	len = (int)strlen(label);
    }
    if( (toggle = sort_menu->findToggle(label)) ) {
	toggle->set(false, false);
	toggle->set(true, true);
    }
    else {
	Arg args[1];
	int n = 0;
	XtSetArg(args[n], XmNvisibleWhenOff, False); n++;
	toggle = new Toggle(label, sort_menu, this, true, args, n);
	toggle->setCommandString("Column Sort");
	toggle->set(true, true);
    }
}

void Arrivals::renameArrival(const string &phase)
{
    int i, j, dc;
    long arid;
    string name;
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    CssAssocClass *t;

    if(!data_source) return;

    data_source->getSelectedTable(arrivals);

    if(arrivals.size() == 0) {
	showWarning("No arrivals selected.");
	return;
    }
    data_source->getTable(assocs);

    setCursor("hourglass");

    BasicSource::startBackup();

    UndoEditArrival *undo = new UndoEditArrival(this, true);

    errorMsg(); // clear last error message

    for(i = 0; i < arrivals.size(); i++)
    {
	undo->arrivals.push_back(arrivals[i]);
	undo->arrivals_copy.push_back(new CssArrivalClass(*arrivals[i]));

	name.assign(arrivals[i]->phase);
	stringcpy(arrivals[i]->phase, phase.c_str(),sizeof(arrivals[i]->phase));

	DataSource *ds = arrivals[i]->getDataSource();

	if(ds && ds->changeArrival(arrivals[i], NULL, CHANGE_PHASE_NAME))
	{
	    if(wp) wp->renameArrival(arrivals[i], phase);
	}
	else {
	    stringcpy(arrivals[i]->phase, name.c_str(),
			sizeof(arrivals[i]->phase));
	    showErrorMsg();
	    break;
	}

	dc = arrivals[i]->getDC();
	arid = arrivals[i]->arid;

	for(j = 0; j < assocs.size(); j++)
	{
	    if(assocs[j]->getDC() == dc && assocs[j]->arid == arid)
	    {
		ds = assocs[j]->getDataSource();
		if(ds) {
		    undo->assocs.push_back(assocs[j]);
		    undo->assocs_copy.push_back(new CssAssocClass(*assocs[j]));
		    t = new CssAssocClass(*assocs[j]);
		    stringcpy(assocs[j]->phase, phase.c_str(),
				sizeof(assocs[j]->phase));

		    if( !ds->changeTable(t, assocs[j]) ) {
			stringcpy(assocs[j]->phase, t->phase,
				sizeof(assocs[j]->phase));
		    }
		    t->deleteObject();
		}
	    }
	}
    }

    list();

    /* update the undo menu label
     */
    Application::getApplication()->addUndoAction(undo);

    setCursor("default");
}

void Arrivals::deleteArrivals(void)
{
    cvector<CssArrivalClass> arrivals;

    if(!data_source) return;

    if( data_source->getSelectedTable(arrivals) > 0) {
	data_source->deleteArrivals(arrivals);
    }
    else {
	showWarning("No arrival selected.");
    }
    return;
}

void Arrivals::editOn(void)
{
    vector<bool> states;
    int num_rows = table->getRowStates(states);

    table->editModeOn();

    if(!wp) return;

    wp->addActionListener(this, XtNretimeDragCallback);

    time_column = -1;

    int num_columns = table->numColumns();
    for(int i = 0; i < num_columns; i++) {
	TAttribute ta = table->getAttribute(i);
	if(!strcasecmp(ta.name, "time"))
	{
	    time_column = i;
	    time_format.assign(ta.format);
	    break;
	}
    }

    for(int i = 0; i < num_rows; i++)
    {
	TableRow *o = (TableRow *)table->getRecord(i);
	CssArrivalClass *arrival = (CssArrivalClass *)o->tables[0];
	arrival->putValue("reset_time", arrival->time);
	if(states[i]) {
	    wp->retimeArrivalOn(arrival, true);
	}
    }
}

void Arrivals::retime(CssArrivalClass *a)
{
    if(time_column >= 0) {
	int num_rows = table->numRows();
	
	for(int i = 0; i < num_rows; i++) {
	    TableRow *o = (TableRow *)table->getRecord(i);
	    CssArrivalClass *arrival = (CssArrivalClass *)o->tables[0];
	    if(arrival == a) {
		char s[50];
		s[0] = '\0';
		if(!time_format.compare("%t")) {
		    timeEpochToString(a->time, s, sizeof(s), YMONDHMS);
		}
		else if(!time_format.compare("%T")) {
		    timeEpochToString(a->time, s, sizeof(s), YMONDHMS3);
		}
		else if(!time_format.compare("%g")) {
		    timeEpochToString(a->time, s, sizeof(s), GSE20);
		}
		else if(!time_format.compare("%G")) {
		    timeEpochToString(a->time, s, sizeof(s), GSE21);
		}
		else {
		    snprintf(s, sizeof(s), time_format.c_str(), a->time);
		}
		table->setField(i, time_column, s, true);
	    }
	}
    }
}

void Arrivals::editCancel(void)
{
    if(!wp) return;

    wp->removeDataListener(this);

    int num_rows = table->numRows();

    for(int i = 0; i < num_rows; i++)
    {
	TableRow *o = (TableRow *)table->getRecord(i);
	CssArrivalClass *arrival = (CssArrivalClass *)o->tables[0];
	double reset_time=0.;
	if(arrival->getValue("reset_time", &reset_time)
		&& reset_time != arrival->time)
	{
	    wp->moveArrival(arrival, reset_time, true);
	}
	arrival->removeValue("reset_time");
    }
    wp->addDataListener(this);
    wp->retimeArrivalAllOff();
}

void Arrivals::editSave(gvector<MultiTableEdit *> *v)
{
    if(wp) {
	wp->removeActionListener(this, XtNretimeDragCallback);
	wp->retimeArrivalAllOff();
    }

    if(data_source) {
	data_source->removeDataListener(this);
    }

    BasicSource::startBackup();

    UndoEditArrival *undo = new UndoEditArrival(this, false);

    for(int i = 0; i < v->size(); i++)
    {
	MultiTableEdit *o = v->at(i);
	TableRow *new_row = o->new_row;
	TableRow *old_row = o->old_row;

	if(!old_row->tables[0]->strictlyEquals(*new_row->tables[0]))
	{
	    if(!undo->arrivals.contains((CssArrivalClass *)new_row->tables[0]))
	    {
		undo->arrivals.push_back((CssArrivalClass *)new_row->tables[0]);
		undo->arrivals_copy.push_back(
			new CssArrivalClass(*(CssArrivalClass *)old_row->tables[0]));
		if(wp) wp->modifyArrival((CssArrivalClass *)new_row->tables[0]);
	    }
	}
	if((new_row->tables[1] && !old_row->tables[1]) ||
		!new_row->tables[1]->strictlyEquals(*old_row->tables[1]))
	{
	    if(old_row->tables[1]) {
		undo->assocs.push_back((CssAssocClass *)new_row->tables[1]);
		undo->assocs_copy.push_back(
			new CssAssocClass(*(CssAssocClass *)old_row->tables[1]));
	    }
	    else {
		CssArrivalClass *arrival = (CssArrivalClass *)new_row->tables[0];
		CssAssocClass *assoc = (CssAssocClass *)new_row->tables[1];
		assoc->arid = arrival->arid;
		stringcpy(assoc->sta, arrival->sta, sizeof(assoc->sta));
		stringcpy(assoc->phase, arrival->iphase, sizeof(assoc->phase));
		if(wp) wp->putTable(assoc);
	    }
	}
    }
    int num_rows = table->numRows();
    for(int i = 0; i < num_rows; i++) {
	TableRow *o = (TableRow *)table->getRecord(i);
	o->tables[0]->removeValue("reset_time");
    }

    /* update the undo menu label
     */
    Application::getApplication()->addUndoAction(undo);

    if(data_source) {
	data_source->addDataListener(this);
    }
}

bool Arrivals::undoEditArrival(UndoEditArrival *undo)
{
    if( !BasicSource::undoFileModification() ) return false;

    for(int i = 0; i < undo->assocs.size(); i++)
    {
	undo->assocs_copy[i]->copyTo(undo->assocs[i], false);
    }

    for(int i = 0; i < undo->arrivals.size(); i++)
    {
	undo->arrivals_copy[i]->copyTo(undo->arrivals[i], false);
	if(wp) wp->modifyArrival(undo->arrivals[i]);
    }

    list();

    return true;
}
