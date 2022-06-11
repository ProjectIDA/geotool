/** \file WaveformView.cpp
 *  \brief Defines class WaveformView.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "WaveformView.h"
#include "gobject++/DataSource.h"
#include "DataMethod.h"
#include "IIRFilter.h"
#include "TaperData.h"
#include "CalibData.h"
#include "OffsetData.h"
#include "Demean.h"
#include "AmpData.h"
#include "CopyData.h"
#include "CutData.h"
#include "gobject++/WaveformTag.h"
#include "motif++/MotifClasses.h"
#include "WaveformWindow.h"
#include "Waveform.h"
#include "BasicSource.h"
#include "CSSTable.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

static gvector<GTimeSeries *> data_clipboard;
static cvector<CssArrivalClass> arrival_clipboard;
static cvector<CssAssocClass> assoc_clipboard;
static cvector<CssStassocClass> stassoc_clipboard;
static cvector<CssOriginClass> origin_clipboard;
static cvector<CssWftagClass> wftag_clipboard;

class UndoEditView : public UndoAction
{
    public:
	UndoEditView(WaveformView *w, bool re_name) {
	    a = w;
	    rename = re_name;
	    error_msg = NULL;
	}
	~UndoEditView() {
	}
	WaveformView *a;
	bool rename;
	cvector<CssArrivalClass> arrivals;
	cvector<CssArrivalClass> arrivals_copy;
	cvector<CssAssocClass> assocs;
	cvector<CssAssocClass> assocs_copy;

	bool undo(void) {
	    return a->undoEditArrival(this);
	}
	void getLabel(string &label) {
	    if(rename) label.assign("Undo Rename Arrival");
	    else label.assign("Undo Retime Arrival");
	}
	bool errMsg(string &msg) {
	    if(error_msg) { msg.assign(error_msg); return true; }
	    else { msg.clear(); return false; }
	}

    protected:
	char *error_msg;
};

class UndoCut : public UndoAction
{
    public:
	UndoCut(WaveformView *w, bool delete_data) {
	    gvector<Waveform *> wvec;
	    wv = w;
	    wv->getWaveforms(wvec);
	    for(int i = 0; i < wvec.size(); i++) {
		ts.push_back(wvec[i]->ts);
		y.push_back(wvec[i]->scaled_y0);
	    }
	    if(delete_data) lab.assign("Undo Delete Data");
	    else lab.assign("Undo Cut");
	}
	~UndoCut() { }

	WaveformView *wv;
	vector<CutData *> cuts;
	vector<double> cut_y0;
	gvector<GTimeSeries *> cut_ts;
	gvector<GTimeSeries *> ts;
	vector<double> y;
	string lab;

	bool undo(void) {
	    int i, j, k = 0;
	    vector<double> scaled_y0;
	    gvector<Waveform *> wvec, v;
	    wv->getWaveforms(wvec);

	    for(i = 0; i < wvec.size(); i++) {
		bool b = false;
		for(j = 0; j < (int)cuts.size(); j++) {
		    if(wvec[i]->ts->removeMethod(cuts[j])) b = true;
		}
		if(b) v.push_back(wvec[i]);
	    }
	    wv->modify(v, NULL, true);
	    wv->pasteData(cut_ts);

	    wvec.clear();
	    wv->getWaveforms(wvec);
	    for(i = 0; i < wvec.size(); i++) {
		for(j = 0; j < ts.size() && ts[j] != wvec[i]->ts; j++);
		if(j < ts.size()) {
		    scaled_y0.push_back(y[j]);
		}
		else if(k < (int)cut_y0.size()) {
		    scaled_y0.push_back(cut_y0[k++]);
		}
	    }
	    wv->positionY(wvec, scaled_y0);
	    return true;
	}
	void getLabel(string &label) {
	    label.assign(lab);
	}
};

using namespace std;

WaveformView::WaveformView(const string &name, Component *parent, Arg *args,
		int n, DataSource *ds) : WaveformPlot(name, parent, args, n, ds)
{
    init(name, parent, args, n);
}

WaveformView::WaveformView(const string &name, Component *parent,
		InfoArea *infoarea, Arg *args, int n, DataSource *ds) :
		WaveformPlot(name, parent, infoarea, args, n, ds)
{
    init(name, parent, args, n);
}

WaveformView::WaveformView(const string &name, Component *parent, Arg *args,
		int n) : WaveformPlot(name, parent, args, n)
{
    init(name, parent, args, n);
}

WaveformView::WaveformView(const string &name, Component *parent,
		InfoArea *infoarea, Arg *args, int n) :
		WaveformPlot(name, parent, infoarea, args, n)
{
    init(name, parent, args, n);
}

void WaveformView::init(const string &name, Component *parent, Arg *args, int n)
{
    arrival_popup = NULL;
    arrival_retime = NULL;
    arrival_delete = NULL;
    arrival_rename = NULL;

    waveform_popup = NULL;
    calib_menu = NULL;
    calib_apply = NULL;
    calib_remove = NULL;
    demean_toggle = NULL;
    polarity_toggle = NULL;
    filter_menu = NULL;
    unfilter_button = NULL;
    copy_one_button = NULL;
    cut_one_button = NULL;
    copy_button = NULL;
    cut_button = NULL;
    paste_button = NULL;
    display_button = NULL;
    elements_button = NULL;

    cplot_menu.main_plot = NULL;
    cplot_menu.reason = -1;
    cplot_menu.w = NULL;
    cplot_menu.arrival = NULL;
    cplot_menu.event = NULL;

    paste_table = NULL;
    num_filter_buttons = 0;

    rename_arrival_popup = NULL;
    rename_arrival_text = NULL;
    memset((void *)reset_rename, 0, sizeof(reset_rename));
    retime_arrival_popup = NULL;
    retime_arrival_text1 = NULL;
    retime_arrival_text2 = NULL;
    retime_arrival_text3 = NULL;

    addActionListener(this, XtNarrivalMenuCallback);
    addActionListener(this, XtNwaveformMenuCallback);
    addActionListener(this, XtNarrivalInfoCallback);
    addActionListener(this, XtNwaveformInfoCallback);
}

WaveformView::~WaveformView(void)
{
}

void WaveformView::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(reason, XtNarrivalMenuCallback)) {
	arrivalMenu((CPlotMenuCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNwaveformMenuCallback)) {
	waveformMenu((CPlotMenuCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNarrivalInfoCallback)) {
	arrivalInfo((CPlotInfoCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNwaveformInfoCallback)) {
	waveformInfo((CPlotInfoCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(reason, XtNretimeDragCallback)) {
	printTime((CssArrivalClass *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "popup-menu Rename")) {
	renameArrivalCB();
    }
    else if(!strcmp(cmd, "popup-menu Retime")) {
	retimeArrival();
    }
    else if(!strcmp(cmd, "popup-menu Beam")) {
	beamArrival(cplot_menu.arrival);
    }
    else if(!strcmp(cmd, "popup-menu FK")) {
	arrivalFK(cplot_menu.arrival, cplot_menu.w, false);
    }
    else if(!strcmp(cmd, "popup-menu FK-MB")) {
	arrivalFK(cplot_menu.arrival, cplot_menu.w, true);
    }
    else if(!strcmp(cmd, "popup-menu Polarization")) {
	arrivalPolar(cplot_menu.arrival, cplot_menu.w);
    }
    else if(!strcmp(cmd, "popup-menu Measure ml amp")) {
	measureAmps(cplot_menu.arrival, cplot_menu.w, ML_MEASURE);
    }
    else if(!strcmp(cmd, "popup-menu Measure mb amp")) {
	measureAmps(cplot_menu.arrival, cplot_menu.w, MB_MEASURE);
    }
    else if(!strcmp(cmd, "popup-menu Delete")) {
	deleteArrivalCB();
    }
    else if(!strncmp(cmd, "retime_text", 11)) {
	TextField *tf = comp->getTextFieldInstance();
	if(tf) editTimeText(tf);
    }
    else if(!strcmp(cmd, "Retime Apply")) {
	retimeApply();
    }
    else if(!strcmp(cmd, "Retime Cancel") || !strcmp(cmd, "Retime popupdown")) {
	retimeCancel();
    }
    else if(!strcmp(cmd, "Rename Apply")) {
	if(rename_arrival_text) {
	    char *name = rename_arrival_text->getString();
	    renameApply(name);
	    Free(name);
	}
    }
    else if(!strcmp(cmd, "Rename Cancel")) {
	renameCancel();
    }
    else if(!strcmp(cmd, "popup-menu counts*calib")) {
	Toggle *t = comp->getToggleInstance();
        if(t && t->state()) calibApply(true);
    }
    else if(!strcmp(cmd, "popup-menu counts")) {
	Toggle *t = comp->getToggleInstance();
        if(t && t->state()) calibApply(false);
    }
    else if(!strcmp(cmd, "popup-menu Demean")) {
	Toggle *t = comp->getToggleInstance();
	if(t) demean( t->state() );
    }
    else if(!strcmp(cmd, "popup-menu Polarity")) {
	changePolarity();
    }
    else if(!strcmp(cmd, "popup-menu Display Waveform")) {
	displayWaveform();
    }
    else if(!strcmp(cmd, "popup-menu Display Elements")) {
	displayElements();
    }
    else if(!strcmp(cmd, "popup-menu Filter")) {
	Button *b = comp->getButtonInstance();
	if(b) filter(b);
    }
    else if(!strcmp(cmd, "popup-menu Unfilter")) {
	unfilter();
    }
    else if(!strcmp(cmd, "popup-menu Copy")) {
	copyWaveforms();
    }
    else if(!strcmp(cmd, "Copy")) {
	gvector<Waveform *> wvec;
	getSelectedWaveforms(wvec);
	copyWaveforms(wvec);
    }
    else if(!strcmp(cmd, "popup-menu Cut")) {
	cutWaveforms();
    }
    else if(!strcmp(cmd, "Cut") || !strcmp(cmd, "Delete Data")) {
	gvector<Waveform *> wvec;
	getSelectedWaveforms(wvec);
	bool b = !strcmp(cmd, "Delete Data");
	cutWaveforms(wvec, b);
    }
    else if(!strcmp(cmd, "Paste")) {
	pasteData();
    }
    else if(!strcmp(cmd, "waveform-zoom")) {
	gvector<Waveform *> v(cplot_menu.w);
	zoomOnWaveforms(v);
    }
    else {
	WaveformPlot::actionPerformed(action_event);
    }
}

PopupMenu * WaveformView::createArrivalPopup(void)
{
    int n;
    Arg args[20];
    PopupMenu *arr_popup;
    XtTranslations translations;
    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

    /* the default is button3, which will inactivate button3
     * in the AxesWidget, so set to 6, which is > all buttons
     */
    n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffTitle
#define XmNtearOffTitle "tearOffTitle"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;
    XtSetArg(args[n], XmNradioBehavior, False); n++;

    arr_popup = new PopupMenu("arrivalPopup", getParent(), args, n);

    new Label("Edit Arrival", arr_popup);
    new Separator("sep", arr_popup);

    translations = XtParseTranslationTable(trans);
    n = 0;
    XtSetArg(args[n], XmNtranslations, translations); n++;
    arrival_rename = new Button("Rename", arr_popup, args, n, this);
    arrival_rename->setCommandString("popup-menu Rename");
    arrival_retime = new Button("Retime", arr_popup, args, n, this);
    arrival_retime->setCommandString("popup-menu Retime");
    arrival_beam = new Button("Beam", arr_popup, args, n, this);
    arrival_beam->setCommandString("popup-menu Beam");
    arrival_fk = new Button("FK", arr_popup, args, n, this);
    arrival_fk->setCommandString("popup-menu FK");
    arrival_fk_mb = new Button("FK-MB", arr_popup, args, n, this);
    arrival_fk_mb->setCommandString("popup-menu FK-MB");
    arrival_polar = new Button("Polarization", arr_popup, args, n, this);
    arrival_polar->setCommandString("popup-menu Polarization");
    arrival_ml_amp = new Button("Measure ml amp", arr_popup, args, n, this);
    arrival_ml_amp->setCommandString("popup-menu Measure ml amp");
    arrival_mb_amp = new Button("Measure mb amp", arr_popup, args, n, this);
    arrival_mb_amp->setCommandString("popup-menu Measure mb amp");
    arrival_delete = new Button("Delete", arr_popup, args, n, this);
    arrival_delete->setCommandString("popup-menu Delete");

    return arr_popup;
}
void WaveformView::arrivalMenu(CPlotMenuCallbackStruct *c)
{
    if((retime_arrival_popup && retime_arrival_popup->isVisible()) ||
       (rename_arrival_popup && rename_arrival_popup->isVisible())) return;

    if(arrival_popup == NULL) {
	arrival_popup = createArrivalPopup();
    }
    cplot_menu = *c;
 
    if(!strcasecmp(c->arrival->sta, "cb") ||
	strcasecmp(c->w->net(), c->w->sta()))
    { // array
	arrival_fk->setVisible(true);
	arrival_fk_mb->setVisible(true);
	arrival_polar->setVisible(false);
    }
    else {
	arrival_fk->setVisible(false);
	arrival_fk_mb->setVisible(false);
	arrival_polar->setVisible(true);
    }

    arrival_popup->position((XButtonPressedEvent *)c->event);
    arrival_popup->setVisible(true);
}

PopupMenu * WaveformView::createWaveformPopup(void)
{
    char button_label[50];
    Arg args[20];
    int i, n;
    PopupMenu *wave_popup;
    XtTranslations translations;
    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";

    /* the default is button3, which will inactivate button3
     * in the AxesWidget, so set to 6 > all buttons
     */
    n = 0;
#if(XmVersion >= 2)
#ifndef XmNtearOffTitle
#define XmNtearOffTitle "tearOffTitle"
#endif
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
#endif
    XtSetArg(args[n], XmNwhichButton, 6); n++;
    XtSetArg(args[n], XmNradioBehavior, False); n++;
    wave_popup = new PopupMenu("waveformPopup", this, args, n);

    new Label("Edit Waveform", wave_popup);
    new Separator("sep", wave_popup);
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
    XtSetArg(args[n], XmNradioBehavior, True); n++;
    calib_menu = new Menu("Calib", wave_popup, args, 2);

    translations = XtParseTranslationTable(trans);
    n = 0;
    XtSetArg(args[n], XmNtranslations, translations); n++;
    calib_apply = new Toggle("Display counts*calib",calib_menu,this,args,n);
    calib_apply->setCommandString("popup-menu counts*calib");
    calib_remove = new Toggle("Display counts", calib_menu, this, args, n);
    calib_remove->setCommandString("popup-menu counts");

    demean_toggle = new Toggle("Demean", wave_popup, this, args, n);
    demean_toggle->setCommandString("popup-menu Demean");

    polarity_toggle = new Toggle("Reverse Polarity", wave_popup, this, args, n);
    polarity_toggle->setCommandString("popup-menu Polarity");

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_DISABLED); n++;
    filter_menu = new Menu("Filter", wave_popup, args, 1);

    n = 0;
    XtSetArg(args[n], XmNtranslations, translations); n++;
    for(i = 0; i < MAX_FILTER; i++) {
	snprintf(button_label, 50, "filter%d", i+1);
	filter_buttons[i].b = new Button(button_label, filter_menu,args,n,this);
	filter_buttons[i].b->setCommandString("popup-menu Filter");
    }
    unfilter_button = new Button("Unfilter", wave_popup, args, n, this);
    unfilter_button->setCommandString("popup-menu Unfilter");
    copy_one_button = new Button("Copy", wave_popup, args, n, this);
    copy_one_button->setCommandString("popup-menu Copy");
    cut_one_button = new Button("Cut", wave_popup, args, n, this);
    cut_one_button->setCommandString("popup-menu Cut");
    display_button = new Button("Display Waveform", wave_popup, args, n, this);
    display_button->setCommandString("popup-menu Display Waveform");
    elements_button = new Button("Display Elements", wave_popup, args, n, this);
    elements_button->setCommandString("popup-menu Display Elements");
    zoom_button = new Button("Zoom", wave_popup, args, n, this);
    zoom_button->setCommandString("waveform-zoom");

    return wave_popup;
}

void WaveformView::waveformMenu(CPlotMenuCallbackStruct *c)
{
    char button_label[50], *prop=NULL, *tok, *last, *r[5];
    int i, j;

    if(waveform_popup == NULL) {
	waveform_popup = createWaveformPopup();
    }
    cplot_menu = *c;

    if(c->w->length() > 2)
    {
	bool calib_applied = c->w->ts->getMethod("CalibData") ? true : false;
	bool dm = c->w->ts->getMethod("Demean") ? true : false;
	gvector<DataMethod *> *methods = c->w->dataMethods();
	for(i = 0; i < (int)methods->size(); i++) {
	    AmpData *amp;
	    if( (amp = methods->at(i)->getAmpDataInstance())
		&& !strcmp(amp->getComment(), "polarity")) break;
	}
	bool p = (i < (int)methods->size()) ? true : false;
	delete methods;
	    
	display_button->setVisible(false);
	calib_menu->setVisible(true);
	demean_toggle->setVisible(true);;
	polarity_toggle->setVisible(true);
	filter_menu->setVisible(true);
	unfilter_button->setVisible(true);
	copy_one_button->setVisible(true);
	cut_one_button->setVisible(true);
	zoom_button->setVisible(true);

	calib_apply->set(calib_applied);
	calib_remove->set(!calib_applied);
	demean_toggle->set(dm);
	polarity_toggle->set(p);
    }
    else {
	calib_menu->setVisible(false);
	demean_toggle->setVisible(false);
	polarity_toggle->setVisible(false);
	filter_menu->setVisible(false);
	unfilter_button->setVisible(false);
	copy_one_button->setVisible(false);
	cut_one_button->setVisible(false);
	zoom_button->setVisible(false);
	display_button->setVisible(true);
    }
    if(c->w->ts->getValue("elements")) {
	elements_button->setVisible(true);
    }
    else {
	elements_button->setVisible(false);
    }

    if((prop = getProperty("IIR_Filters")) == NULL) {
	putProperty("IIR_Filters",
"0.6,4.5,3,BP,zero phase,3.0,6.0,3,BP,causal,2.0,5.0,3,BP,causal,2.0,4.0,3,BP,causal,1.0,5.0,3,BP,causal,1.0,4.0,3,BP,causal,1.0,3.0,3,BP,causal,1.0,2.0,3,BP,causal,0.5,2.0,3,BP,causal,0.5,1.5,3,BP,causal,6.0,12.0,3,BP,causal");
	prop = getProperty("IIR_Filters");
    }

    for(i = 0; i < MAX_FILTER; i++) {
	filter_buttons[i].b->setVisible(false);
    }
    i = 0;
    tok = prop;
    while((r[0]=strtok_r(tok, ",", &last)) && i < MAX_FILTER)
    {
	tok = NULL;
	if(!(r[1] = strtok_r(tok, ",", &last))) break;
	if(!(r[2] = strtok_r(tok, ",", &last))) break;
	if(!(r[3] = strtok_r(tok, ",", &last))) break;
	if(!(r[4] = strtok_r(tok, ",", &last))) break;
	for(j = 0; j < 5; j++) stringTrim(r[j]);
	if(!stringToDouble(r[0], &filter_buttons[i].flo)) {
	    fprintf(stderr, "Invalid filter low frequency: %s\n", r[0]);
	    fprintf(stderr, "From IIR_Filters property.\n");
	    continue;
	}
	if(!stringToDouble(r[1], &filter_buttons[i].fhi)) {
	    fprintf(stderr, "Invalid filter high frequency: %s\n", r[1]);
	    fprintf(stderr, "From IIR_Filters property.\n");
	    continue;
	}
	if(!stringToInt(r[2], &filter_buttons[i].order)) {
	    fprintf(stderr, "Invalid filter order: %s\n", r[2]);
	    fprintf(stderr, "From IIR_Filters property.\n");
	    continue;
	}
	if(!strcasecmp(r[3], "NA")) continue;
	if( strcasecmp(r[3], "BP") && strcasecmp(r[3], "BR") &&
	    strcasecmp(r[3], "LP") && strcasecmp(r[3], "HP"))
	{
	    fprintf(stderr, "Invalid filter type: %s\n", r[3]);
	    fprintf(stderr, "From IIR_Filters property.\n");
	    continue;
	}
	strcpy(filter_buttons[i].type, r[3]);

	if(!strcasecmp(r[4], "zero phase")) {
	    filter_buttons[i].zero_phase = 1;
	}
	else if(!strcasecmp(r[4], "causal")) {
	    filter_buttons[i].zero_phase = 0;
	}
	else {
	    fprintf(stderr,
		    "Invalid filter zero phase/causal setting: %s\n", r[4]);
	    fprintf(stderr, "From IIR_Filters property.\n");
	    continue;
	}

	snprintf(button_label, 50, "%s %s %s %s %s", r[0],r[1],r[2],r[3],r[4]);
	filter_buttons[i].b->setLabel(button_label);
	filter_buttons[i].b->setVisible(true);
	i++;
    }
    Free(prop);
    num_filter_buttons = i;

    waveform_popup->position((XButtonPressedEvent *)c->event);
    waveform_popup->setVisible(true);
}

void WaveformView::arrivalInfo(CPlotInfoCallbackStruct *c)
{
    CssArrivalClass *a = c->a;
    int i;

    if( !getProperty("showArrivalInfo", true) ) {
	c->label[0] = '\0';
	return;
    }

    snprintf(c->label, sizeof(c->label), "%s/%s\n%ld\n",
		a->sta, a->chan, a->arid);
    i = (int)strlen(c->label);
    timeEpochToString(a->time, c->label+i, sizeof(c->label)-i, HMS2);
}

void WaveformView::waveformInfo(CPlotInfoCallbackStruct *c)
{
    CssOriginClass *o;
    gvector<DataMethod *> *methods;
    int i, len = sizeof(c->label);

    c->label[0] = '\0';

    if( !getProperty("showWaveformInfo", true) ) return;

    timeEpochToString(c->w->tbeg(), c->label, sizeof(c->label), YMONDHMS2);

    if((o = getPrimaryOrigin(c->w)))
    {
	double delta, az, baz;
	deltaz(o->lat, o->lon, c->w->lat(), c->w->lon(), &delta, &az, &baz);
	if(c->label[0] != '\0') strcat(c->label, "\n");
	i = (int)strlen(c->label);
	snprintf(c->label+i, sizeof(c->label)-i, "distance: %.2fdeg",delta);
    }

    methods = c->w->dataMethods();
    for(i = 0; i < (int)methods->size(); i++) {
	char *s = (char *)methods->at(i)->toString();
	if(strlen(c->label) + strlen(s) + 1 >= (unsigned int)len) break;
	if(c->label[0] != '\0') strcat(c->label, "\n");
	strcat(c->label, s);
    }
    delete methods;
}

void WaveformView::renameArrivalCB(void)
{
    Arg args[20];
    int i, n;

    stringcpy(reset_rename, cplot_menu.arrival->phase, sizeof(reset_rename));

    if(rename_arrival_popup == NULL)
    {
	rename_arrival_popup = new FormDialog("Rename Arrival", getParent(),
					false, false);
	rename_arrival_popup->addActionListener(this, XmNpopdownCallback);
	rename_arrival_popup->setCommandString("Rename popupdown");

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	Label *label = new Label("Rename Arrival", rename_arrival_popup,args,n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNcolumns, 10); n++;
	XtSetArg(args[n], XmNvalue, cplot_menu.arrival->phase); n++;
	rename_arrival_text = new TextField("rename_text", rename_arrival_popup,
					args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, rename_arrival_text->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	RowColumn *rc = new RowColumn("rc", rename_arrival_popup, args, n);

	Button *button = new Button("Apply", rc, this);
	button->setCommandString("Rename Apply");
	button = new Button("Cancel", rc, this);
	button->setCommandString("Rename Cancel");
    }
    else {
	rename_arrival_text->setString(cplot_menu.arrival->phase);
    }

    if(!isSelected(cplot_menu.arrival)) {
	selectArrivalWithCB(cplot_menu.arrival, true);
    }
	
    cvector<CssArrivalClass> *arrivals = getArrivalRef();
    for(i = 0; i < arrivals->size(); i++) {
	if(arrivals->at(i) != cplot_menu.arrival && isSelected(arrivals->at(i)))
	{
	    selectArrivalWithCB(arrivals->at(i), false);
	}
    }

    rename_arrival_popup->setVisible(true);
}

void WaveformView::renameApply(const string &new_name)
{
    renameApply(cplot_menu.arrival, new_name);
}

void WaveformView::renameApply(CssArrivalClass *a, const string &new_name)
{
    long orid = getWorkingOrid();
    char *name = strdup(new_name.c_str());
    CssAssocClass *assoc=NULL;
    DataSource *ds;
    int i;
 
    if(strlen(stringTrim(name)) <= 0) {
	Free(name);
  	return;
    }
    if(strlen(name) > 8) name[8] = '\0';

    if(!strcmp(name, a->phase)) {
	Free(name);
	return;
    }
    ds = a->getDataSource();
    if( !ds ) {
	showWarning("No data source for arrival.");
	renameCancel();
	Free(name);
    }

    cvector<CssAssocClass> *assocs = getAssocRef();

    for(i = 0; i < assocs->size(); i++) {
	if(assocs->at(i)->orid == orid && assocs->at(i)->arid == a->arid) {
	    assoc = assocs->at(i);
	    break;
	}
    }

    BasicSource::startBackup();

    UndoEditView *undo = new UndoEditView(this, true);

    if(assoc)
    {
	undo->assocs.push_back(assoc);
	CssAssocClass *t = new CssAssocClass(*assoc);
	undo->assocs_copy.push_back(t);

	errorMsg(); // clear last error message
	snprintf(assoc->phase, sizeof(assoc->phase), "%s", name);

	if( ds->changeTable(t, assoc) )
	{
	    renameAssoc(assoc, a, name);
	    Application::getApplication()->addUndoAction(undo);
	}
	else {
	    showErrorMsg();
	    renameCancel();
	    delete undo;
	}
    }
    else
    {
	undo->arrivals.push_back(a);
	undo->arrivals_copy.push_back(new CssArrivalClass(*a));

	errorMsg(); // clear last error message
	snprintf(a->phase, sizeof(a->phase), "%s", name);

	if( ds->changeArrival(a, NULL, CHANGE_PHASE_NAME) ) {
	    renameArrival(a, name);
	    Application::getApplication()->addUndoAction(undo);
	}
	else {
	    showErrorMsg();
	    renameCancel();
	    delete undo;
	}
    }
    Free(name);
    if(rename_arrival_popup) rename_arrival_popup->setVisible(false);
}

bool WaveformView::undoEditArrival(UndoEditView *undo)
{
    if( !BasicSource::undoFileModification() ) return false;

    for(int i = 0; i < undo->assocs.size(); i++)
    {
        CssAssocClass *a1 = undo->assocs[i];
        CssAssocClass *a2 = undo->assocs_copy[i];

        a1->azres = a2->azres;
        strcpy(a1->azdef, a2->azdef);
        a1->timeres = a2->timeres;
        strcpy(a1->timedef, a2->timedef);
        a1->slores = a2->slores;
        strcpy(a1->slodef, a2->slodef);
        a1->wgt = a2->wgt;
        a1->delta = a2->delta;
        a1->seaz = a2->seaz;
        a1->esaz = a2->esaz;
        a1->arid = a2->arid;
        a1->orid = a2->orid;
        strcpy(a1->sta, a2->sta);
        strcpy(a1->phase, a2->phase);
    }

//  num_gs = getGeotools(&gs);

    for(int i = 0; i < undo->arrivals.size(); i++)
    {
        CssArrivalClass *a1 = undo->arrivals[i];
        CssArrivalClass *a2 = undo->arrivals_copy[i];

        a1->stassid = a2->stassid;
        a1->time = a2->time;
        strcpy(a1->iphase, a2->iphase);
        strcpy(a1->phase, a2->phase);
        strcpy(a1->sta, a2->sta);
        strcpy(a1->chan, a2->chan);
        a1->amp_cnts = a2->amp_cnts;
        a1->amp_Nnms = a2->amp_Nnms;
        a1->amp_nms = a2->amp_nms;
        a1->zp_Nnms = a2->zp_Nnms;
        a1->period = a2->period;
        a1->azimuth = a2->azimuth;
        a1->slow = a2->slow;
        a1->ema = a2->ema;
        a1->rect = a2->rect;

        modifyArrival(a1);
    }
    return true;
}

void WaveformView::renameCancel(void)
{
    if(rename_arrival_popup) rename_arrival_popup->setVisible(false);
}

void WaveformView::destroyRetimeCB(Widget w, XtPointer client, XtPointer data)
{
    WaveformView *wp = (WaveformView *)client;

    wp->retime_arrival_popup = NULL;
    wp->retimeCancel();
}

void WaveformView::retimeArrival(void)
{
    Arg args[20];
    char text1[50], text2[50], text3[50];
    int i, n;
    CssOriginClass *origin;

    reset_retime = cplot_menu.arrival->time;
    retime_arrival.clear();
    retime_origin.clear();
    retime_arrival.push_back(cplot_menu.arrival);

    timeEpochToString(cplot_menu.arrival->time, text1, 50, YMONDHMS);
    snprintf(text2, sizeof(text2), "%.3f", cplot_menu.arrival->time);
    if((origin = getPrimaryOrigin(cplot_menu.w)) != NULL) {
	snprintf(text3, sizeof(text3), "%.3f",
		cplot_menu.arrival->time - origin->time);
	retime_origin.push_back(origin);
    }
    else {
	snprintf(text3, sizeof(text3), "No Origin");
    }

    if(retime_arrival_popup == NULL)
    {
	retime_arrival_popup = new FormDialog("Retime Arrival", getParent(),
					false, false);
	retime_arrival_popup->addActionListener(this, XmNpopdownCallback);
	retime_arrival_popup->setCommandString("Retime popupdown");

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	Label *label1 = new Label("Reposition the Selected Arrival",
				retime_arrival_popup, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label1->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	Label *label2 = new Label("UTC", retime_arrival_popup, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label2->baseWidget()); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNcolumns, 20); n++;
	XtSetArg(args[n], XmNvalue, text1); n++;
	retime_arrival_text1 = new TextField("retime_text1",
				retime_arrival_popup, this, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,retime_arrival_text1->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 2); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	Label *label3 = new Label("Epochal Time", retime_arrival_popup, args,n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label3->baseWidget()); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNcolumns, 20); n++;
	XtSetArg(args[n], XmNvalue, text2); n++;
	retime_arrival_text2 = new TextField("retime_text2",
				retime_arrival_popup, this, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,retime_arrival_text2->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 2); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	Label *label4 = new Label("Travel Time", retime_arrival_popup, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, label4->baseWidget()); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNcolumns, 20); n++;
	XtSetArg(args[n], XmNvalue, text3); n++;
	retime_arrival_text3 = new TextField("retime_text3",
				retime_arrival_popup, this, args, n);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,retime_arrival_text3->baseWidget()); n++;
	XtSetArg(args[n], XmNtopOffset, 10); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightOffset, 10); n++;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	RowColumn *rc = new RowColumn("rc", retime_arrival_popup, args, n);

	Button *b = new Button("Apply", rc, this);
	b->setCommandString("Retime Apply");
	
	b = new Button("Cancel", rc, this);
	b->setCommandString("Retime Cancel");
    }
    else {
	retime_arrival_text1->setString(text1);
	retime_arrival_text2->setString(text2);
	retime_arrival_text3->setString(text3);
    }
    if(!strcmp(text3, "No Origin")) {
	retime_arrival_text3->setSensitive(false);
    }
    else {
	retime_arrival_text3->setSensitive(true);
    }

    if(!isSelected(cplot_menu.arrival)) {
	selectArrivalWithCB(cplot_menu.arrival, true);
    }
	
    cvector<CssArrivalClass> *arrivals = getArrivalRef();
    for(i = 0; i < arrivals->size();i++) {
	if(arrivals->at(i) != cplot_menu.arrival && isSelected(arrivals->at(i)))
	{
	    selectArrivalWithCB(arrivals->at(i), false);
	}
    }

    retimeArrivalOn(cplot_menu.arrival, true);

    retime_arrival_popup->setVisible(true);

    addActionListener(this, XtNretimeDragCallback);
}

void WaveformView::editTimeText(TextField *comp)
{
    double time;
    CssArrivalClass *a;
    char text[50], *value = NULL;

    if(retime_arrival.size() == 0) return;
    a = retime_arrival[0];

    value = comp->getString();
    if(comp == retime_arrival_text1) {
	if(timeParseString(value, &time) != 1) {
	    showWarning("Invalid time.");
	    XtFree(value);
	    return;
	}
	a->time = time;
	moveArrival(a, time, true);
	retime_arrival_text2->setString("%.3f", time);
	if(retime_origin.size() > 0) {
	    CssOriginClass *origin = retime_origin[0];
	    retime_arrival_text3->setString("%.3f", time - origin->time);
	}
    }
    else if(comp == retime_arrival_text2) {
	if(stringToDouble(value, &time)) {
	    timeEpochToString(time, text, 50, YMONDHMS);
	    retime_arrival_text1->setString(text);
	    if(retime_origin.size() > 0) {
		CssOriginClass *origin = retime_origin[0];
		retime_arrival_text3->setString("%.3f", time - origin->time);
	    }
	    a->time = time;
	    moveArrival(a, time, true);
	}
    }
    else if(comp == retime_arrival_text3 && retime_origin.size() > 0)
    {
	CssOriginClass *origin = retime_origin[0];
	if(stringToDouble(value, &time)) {
	    timeEpochToString(origin->time + time, text, 50, YMONDHMS);
	    retime_arrival_text1->setString(text);
	    retime_arrival_text2->setString("%.3f", origin->time + time);
	    a->time = origin->time + time;
	    moveArrival(a, a->time, true);
	}
    }
    XtFree(value);
}

void WaveformView::printTime(CssArrivalClass *a)
{
    char text[50];

    timeEpochToString(a->time, text, 50, YMONDHMS);
    retime_arrival_text1->setString(text);

    retime_arrival_text2->setString("%.3f", a->time);

    if(retime_origin.size() > 0) {
	CssOriginClass *origin = retime_origin[0];
	retime_arrival_text3->setString("%.3f", a->time - origin->time);
    }
}

void WaveformView::retimeApply(void)
{
    errorMsg(); // clear last error message

    removeActionListener(this, XtNretimeDragCallback);
    if(retime_arrival.size() > 0)
    {
	CssArrivalClass *a = retime_arrival[0];

	BasicSource::startBackup();

	DataSource *ds = a->getDataSource();
	UndoEditView *undo = new UndoEditView(this, false);
	undo->arrivals.push_back(a);
	undo->arrivals_copy.push_back(new CssArrivalClass(*a));
	undo->arrivals_copy[0]->time = reset_retime;

	if(!ds) {
	    showWarning("No data source for arrival.");
	    retimeCancel();
	    delete undo;
	}
	else if( !ds->changeArrival(a, NULL, CHANGE_TIME) ) {
	    showErrorMsg();
	    retimeCancel();
	    delete undo;
	}
	else {
	    retime_arrival.clear();
	    retime_origin.clear();
	    retimeArrivalAllOff();
	    resetDataChange(false);
	    change.arrival = true;
	    change.assoc = true;
	    doDataChangeCallbacks();
	    resetDataChange(false);
	    Application::getApplication()->addUndoAction(undo);
	    TableListener::doCallbacks(a, this, a->memberIndex("time"));
	}
    }
    if(retime_arrival_popup && retime_arrival_popup->isVisible()) {
	retime_arrival_popup->setVisible(false);
    }
}

void WaveformView::retimeCancel(void)
{
    removeActionListener(this, XtNretimeDragCallback);
    retimeArrivalAllOff();

    if(retime_arrival.size() > 0) {
	moveArrival(retime_arrival[0], reset_retime, true);
	retime_arrival.clear();
    }

    if(retime_arrival_popup && retime_arrival_popup->isVisible()) {
	retime_arrival_popup->setVisible(false);
    }
}

void WaveformView::deleteArrivalCB(void)
{
    cvector<CssArrivalClass> *arrivals = getArrivalRef();

    for(int i = 0; i < arrivals->size(); i++) {
	if(arrivals->at(i) == cplot_menu.arrival) {
//	    cvector<CssArrivalClass> a(arrivals->at(i));
	    cvector<CssArrivalClass> a;
	    a.push_back(arrivals->at(i));
	    deleteArrivals(a);
	    return;
	}
    }
    showWarning("Internal error. Cannot find arrival.");
}

void WaveformView::calibApply(bool apply)
{
    if(apply)
    {
	if(!cplot_menu.w->ts->getMethod("CalibData")) {
	    CalibData *calib = new CalibData();
	    gvector<Waveform *> v(cplot_menu.w);
	    calib->apply(v);
	    modifyWaveform(cplot_menu.w);
	}
    }
    else {
	gvector<Waveform *> v(cplot_menu.w);
	if(DataMethod::remove("CalibData", v)) {
	    modifyWaveform(cplot_menu.w);
	}
    }
}

void WaveformView::demean(bool apply)
{
    if(apply)
    {
	gvector<DataMethod *> *methods;
	methods = cplot_menu.w->dataMethods();
	if((int)methods->size() == 0 || !methods->back()->getDemeanInstance())
	{
	    Demean *dm = new Demean();
	    gvector<Waveform *> v(cplot_menu.w);
	    dm->apply(v);
	    modifyWaveform(cplot_menu.w);
	}
	delete methods;
    }
    else
    {
	gvector<Waveform *> v(cplot_menu.w);
	if(DataMethod::remove("Demean", v)) {
	    modifyWaveform(cplot_menu.w);
	}
    }
}

void WaveformView::changePolarity(void)
{
    int i;
    gvector<DataMethod *> methods;
    bool found_one = false;

    cplot_menu.w->ts->getMethods("AmpData", methods);
    for(i = 0; i < (int)methods.size() && !found_one; i++)
    {
	AmpData *amp = (AmpData *)methods[i];
	if(!strcmp(amp->getComment(), "polarity"))
	{
	    cplot_menu.w->ts->removeMethod(amp);
	    found_one = true;
	}
    }

    if(!found_one) {
	AmpData *amp = new AmpData(-1., "polarity");
	gvector<Waveform *> v(cplot_menu.w);
	amp->apply(v);
    }
    modifyWaveform(cplot_menu.w);
}

void WaveformView::copyWaveforms(void)
{
    gvector<Waveform *> v(cplot_menu.w);
    cutOrCopy(v, false, NULL_TIME, NULL_TIME, false);
}

void WaveformView::cutWaveforms(void)
{
    gvector<Waveform *> v(cplot_menu.w);
    cutOrCopy(v, false, NULL_TIME, NULL_TIME, true);
}

void WaveformView::filter(Button *button)
{
    int i;

    for(i = 0; i < num_filter_buttons && button != filter_buttons[i].b; i++);

    if(i < num_filter_buttons)
    {
	setCursor("hourglass");
	try {
	    GSegment *s = cplot_menu.w->segment(0);
	    DataMethod *dm[2];
	    IIRFilter *iir = new IIRFilter(filter_buttons[i].order,
			filter_buttons[i].type, filter_buttons[i].flo,
			filter_buttons[i].fhi, s->tdel(),
			filter_buttons[i].zero_phase);
	    dm[0] = new Demean();
	    dm[1] = new TaperData("cosineBeg", 10, 5, 200);
	    dm[2] = iir;
	    gvector<Waveform *> v(cplot_menu.w);
	    if( DataMethod::changeMethods(3, dm, v) ) {
		gvector<Waveform *> v(cplot_menu.w);
		modifyWaveforms(v);
	    }
	    else {
		showWarning(GError::getMessage());
	    }
	}
	catch(...) {
	    showWarning(GError::getMessage());
	}
	setCursor("default");
    }
}

void WaveformView::unfilter(void)
{
    const char *method_types[3] = {"Demean", "TaperData", "IIRFilter"};

    gvector<Waveform *> v(cplot_menu.w);
    DataMethod::remove(3, method_types, v);
    modifyWaveforms(v);
}

void WaveformView::displayElements(void)
{
    DataSource *ds;

    if( cplot_menu.w->ts->array_elements.size() == 0
	|| !(ds = cplot_menu.w->getDataSource()) )
    {
	return;
    }
    int i;
    for(i = 0; i < (int)windows.size(); i++) {
	if( !windows[i]->isVisible() ) {
	    windows[i]->setTitle(cplot_menu.w->net());
	    windows[i]->clear();
	    windows[i]->wplot->unzoomAll();
	    windows[i]->setVisible(true);
	    break;
	}
    }
    if(i == (int)windows.size()) {
	WaveformWindow *w = new WaveformWindow(cplot_menu.w->net(),
			Application::getApplication(), cplot_menu.w->net());
	windows.push_back(w);
	w->setVisible(true);
    }

    gvector<SegmentInfo *> s;
    for(int j = 0; j < (int)cplot_menu.w->ts->array_elements.size(); j++) {
	s.add((SegmentInfo *)cplot_menu.w->ts->array_elements[j]);
    }

    windows[i]->wplot->inputData(&s, ds, true);
}

void WaveformView::displayWaveform(void)
{
    Waveform *w = cplot_menu.w;
    DataSource *ds = w->getDataSource();

    setCursor("hourglass");
    ds->readWaveform(w, this);
    setCursor("default");
}

void WaveformView::cutOrCopy(gvector<Waveform *> &wvec, bool delete_data,
				double tmin, double tmax, bool cut)
{
    int	i, j, k, dc;
    double tbeg, tend;
    GTimeSeries *ts;
    gvector<Waveform *> delw;
    cvector<CssArrivalClass> &arrivals = *getArrivalRef();
    cvector<CssAssocClass> &assocs = *getAssocRef();
    cvector<CssStassocClass> &stassocs = *getStassocRef();
    cvector<CssWftagClass> &wftags = *getWftagRef();
    Waveform *w;
    CssOriginClass *o;
    UndoCut *undo = NULL;

    data_clipboard.clear();
    arrival_clipboard.clear();
    assoc_clipboard.clear();
    stassoc_clipboard.clear();
    origin_clipboard.clear();
    wftag_clipboard.clear();

    if(cut) {
	undo = new UndoCut(this, delete_data);
	for(i = 0; i < wvec.size(); i++) {
	    undo->cut_y0.push_back(wvec[i]->scaled_y0);
	}
    }

    for(i = 0; i < wvec.size(); i++)
    {
	w = wvec[i];
	tbeg = tend = 0.;
	if(tmin != NULL_TIME && tmax != NULL_TIME && tmax > tmin) {
	    tbeg = tmin;
	    tend = tmax;
	}
	else if(w->endSelect > w->begSelect) {
	    tbeg = w->begSelect;
	    tend = w->endSelect;
	}

	if(tbeg != tend) {
	    ts = w->ts->subseries(tbeg, tend);
	    if(!ts) continue; // should not happen
	}
	else {
	    ts = new GTimeSeries(w->ts);
	}

	if((tbeg != 0. && tbeg != ts->tbeg())
	|| (tend != 0. && tend != ts->tend()))
	{
	    gvector<DataMethod *> *methods = ts->dataMethods();
	    methods->push_back(new CopyData(tbeg, tend));
	    ts->setDataMethods(methods);
	    delete methods;
	}

	ts->setSta(w->sta());
	ts->setChan(w->chan());
	ts->setNet(w->net());
	ts->setLat(w->lat());
	ts->setLon(w->lon());
	ts->setElev(w->elev());

	data_clipboard.add(ts);

	for(j = 0; j < arrivals.size(); j++)
	{
	    if((!strcmp(w->sta(), arrivals[j]->sta) ||
                !strcmp(w->net(), arrivals[j]->sta)) &&
		ts->tbeg() <= arrivals[j]->time &&
		ts->tend() >= arrivals[j]->time)
	    {
		arrival_clipboard.push_back(arrivals[j]);
		dc = arrivals[j]->getDC();

		for(k = 0; k < assocs.size(); k++) if(assocs[k]->getDC() ==dc
			&& assocs[k]->arid == arrivals[j]->arid)
		{
		    assoc_clipboard.push_back(assocs[k]);
		}
		for(k = 0; k < stassocs.size(); k++)
		    if(stassocs[k]->stassid == arrivals[j]->stassid)
		{
		    stassoc_clipboard.push_back(stassocs[k]);
		}
	    }
	}

	if((o = getPrimaryOrigin(w)) != NULL)
	{
	    origin_clipboard.push_back(o);
	    ts->putValue("primary-origin", o);

	    dc = o->getDC();
	    for(j = 0; j < wftags.size(); j++)
	    {
		if(dc == wftags[j]->getDC() && o->orid == wftags[j]->tagid)
		{
		    wftag_clipboard.push_back(wftags[j]);
		}
	    }
	}

	if(cut) {
	    if(w->endSelect == w->begSelect || 
		(w->begSelect <= w->tbeg() && w->endSelect >= w->tend()))
	    {
		delw.push_back(w);
		undo->cut_ts.push_back(w->ts);
	    }
	    else {
		CutData *c = new CutData(w->begSelect, w->endSelect);
		gvector<Waveform *> v(w);
		c->apply(v);
		selectWaveform(w, false, false);
		modifyWaveform(w);
		undo->cuts.push_back(c);
	    }
	}
    }
    if(delw.size() > 0) {
	deleteWaveforms(delw);
    }
    if(cut) {
	Application::getApplication()->addUndoAction(undo);
    }
}

int WaveformView::numClipBoard(void)
{
    return data_clipboard.size();
}

void WaveformView::pasteData()
{
    pasteData(data_clipboard);
}

void WaveformView::pasteData(gvector<GTimeSeries *> &data)
{
    int i, j, num, width, height, max_tag_width, time_scale;
    double tbeg, tend, tmin=0., tmax=0., t1, t2, epoch_min=0.;
    CPlotInputStruct **input=NULL;
//    RecentInput r;
    Arg args[1];

    if((num = data.size()) <= 0) return;

    input = new CPlotInputStruct *[num];

    if(time_align ==  ALIGN_TRUE_TIME)
    {
	XtSetArg(args[0], XtNtimeScale, &time_scale);
	getValues(args, 1);
	if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
	    gvector<Waveform *> wvec;
	    int n = getWaveforms(wvec, 1);
	    if(n > 0) epoch_min = wvec[0]->tbeg();
	    for(i = 1; i < n; i++) {
		double t = wvec[i]->tbeg();
		if(t < epoch_min) epoch_min = t;
	    }
	    if(n == 0 && num > 0) {
		GTimeSeries *ts = data[0];
		epoch_min = ts->tbeg();
	    }
	    for(i = 0; i < num; i++) {
		GTimeSeries *ts = data[i];
		double t = ts->tbeg();
		if(t < epoch_min) epoch_min = t;
	    }
	    for(i = 0; i < n; i++) {
		double t = wvec[i]->tbeg();
		wvec[i]->scaled_x0 = t - epoch_min;
	    }
	}
    }

    max_tag_width = 0;

    cvector<CssArrivalClass> &cur_arrivals = *getArrivalRef();

    for(i = 0; i < num; i++)
    {
	GTimeSeries *ts = data[i];
	int dc= -1, id= -1;
	ts->getValue("dc", &dc);
	ts->getValue("id", &id);

	tbeg = ts->tbeg();
	tend = ts->tend();
	input[i] = new CPlotInputStruct();
	ts->getChan(input[i]->chan);

	CssOriginClass *o;
	if( (o = (CssOriginClass *)ts->getValue("primary-origin")) )
	{
	    input[i]->origin = o;
	}
	else {
	    input[i]->origin = dataToOrigin(ts);
	}

	input[i]->display_t0 = 0.;

	if(time_align == ALIGN_TRUE_TIME) {
	    if(time_scale != TIME_SCALE_HMS && !time_scale_epoch) {
		input[i]->display_t0 = tbeg - epoch_min;
	    }
	    else {
		input[i]->display_t0 = tbeg;
	    }
	}
	else if(time_align == ALIGN_FIRST_POINT) {
	    input[i]->display_t0 = 0.;
	}
	else if(time_align == ALIGN_ORIGIN) {
	    if(input[i]->origin && input[i]->origin->time > NULL_TIME_CHECK)
	    {
		input[i]->display_t0 = input[i]->origin->time;
	    }
	    else {
		input[i]->display_t0 = 0.;
	    }
	}
	else if(time_align == ALIGN_PREDICTED_ARRIVAL)
	{
	    string phase;
	    double tt, slow;

	    if(input[i]->origin && input[i]->origin->time > NULL_TIME_CHECK)
	    {
		if(!phase_align.compare("FirstP") ||
		   !phase_align.compare("FirstS"))
		{
		    char c = !phase_align.compare("FirstP") ? 'P' : 'S';

		    if(firstArrival(input[i]->origin, ts->lat(), ts->lon(),
			ts->elev(), ts->net(), ts->sta(), c, phase,
			&tt, &slow) == 1)
		    {
			input[i]->display_t0 = tbeg-(input[i]->origin->time+tt);
		    }
		}
		else if((tt = getTravelTime(phase_align, input[i]->origin,
			ts->lat(), ts->lon(), ts->elev(), ts->net(), ts->sta(),
			&slow, phase)) > 0.)
		{
		    input[i]->display_t0 = tbeg - (input[i]->origin->time + tt);
		}
	    }
	}
	else if(time_align == ALIGN_OBSERVED_ARRIVAL)
	{
	    for(j = 0; j < cur_arrivals.size(); j++) {
		CssArrivalClass *a = cur_arrivals[j];
		if((!strcmp(ts->sta(), a->sta) || !strcmp(ts->net(), a->sta))
			&& tbeg <= a->time && tend >= a->time)
		{
		    if(a->ts_dc > 0 && (dc != a->ts_dc || id != a->ts_id))
//			    CssTable_copy(t) != a->ts_copy))
		    {
			continue;
		    }
		    input[i]->display_t0 = tbeg - a->time;
		    break;
		}
	    }
	}
	ts->tag.members.clear();
	setTag(ts, input[i]);
	getTagDimensions(input[i]->tag, &width, &height);
	if(width > max_tag_width) max_tag_width = width;

	if(i == 0) {
	    tmin = input[i]->display_t0;
	    tmax = tmin + (tend - tbeg);
	}
	else {
	    if(input[i]->display_t0 < tmin) tmin = input[i]->display_t0;
	    t2 = input[i]->display_t0 + tend - tbeg;
	    if(t2 > tmax) tmax = t2;
	}
    }

    width = getMaxTagWidth();
    if(max_tag_width > width) setMaxTagWidth(max_tag_width);

    if(getDataDuration(&t1, &t2))
    {
	if(tmin < t1 || tmax > t2) {
	    if(t1 < tmin) tmin = t1;
	    if(t2 > tmax) tmax = t2;
	    setTimeLimits(tmin, tmax);
	}
    }
    else {
	setTimeLimits(tmin, tmax);
    }

    resetDataChange(false);

    for(i = 0; i < num; i++)
    {
	GTimeSeries *ts = new GTimeSeries(data[i]);

	setTag(ts, input[i]);
	setInputColor(ts, input[i]);
	input[i]->on = True;
	addTimeSeries(ts, input[i]);

/*
	r = (RecentInput)Hashtable_get(ts->hashtable, "recent_input");
	if(r != NULL) {
	    for(j = 0; j < r->num_w; j++) {
		if(r->w_ids[j] == -1) {
		    r->w_ids[j] = w->id;
		    r->wvec[j] = w;
		    break;
		}
	    }
	}
*/
	change.waveform = true;
	delete input[i];
    }
    delete [] input;

    for(i = 0; i < arrival_clipboard.size(); i++) {
	Pixel fg = getArrivalFg(arrival_clipboard[i]);
	putArrivalWithColor(arrival_clipboard[i], fg);
	change.arrival = true;
    }
    for(i = 0; i < assoc_clipboard.size(); i++) {
	putTable(assoc_clipboard[i]);
    }
    for(i = 0; i < stassoc_clipboard.size(); i++) {
	putTable(stassoc_clipboard[i]);
	change.assoc = true;
    }
    for(i = 0; i < origin_clipboard.size(); i++) {
	putTable(origin_clipboard[i]);
	change.origin = true;
    }
    for(i = 0; i < wftag_clipboard.size(); i++) {
	putTable(wftag_clipboard[i]);
	change.wftag = true;
    }

/* leave the copied or cut items in the clipboard
    data_clipboard.clear();
    arrival_clipboard.clear();
    assoc_clipboard.clear();
    stassoc_clipboard.clear();
    origin_clipboard.clear();
    wftag_clipboard.clear();
*/

    doDataChangeCallbacks();

    resetDataChange(false);
}

void WaveformView::makePopupMenu(void)
{
    if(menu_made) return;

    char trans[] =
"<Btn2Up>: ArmAndActivate()\n<Btn3Up>: ArmAndActivate()\n<EnterWindow>: Enter()\n<LeaveWindow>: Leave()";
    XtTranslations translations = XtParseTranslationTable(trans);
    Arg args[1];

    XtSetArg(args[0], XmNtranslations, translations);

    copy_button = new Button("Copy", menu_popup, args, 1, this);
    cut_button = new Button("Cut", menu_popup, args, 1, this);
    paste_button = new Button("Paste", menu_popup, args, 1, this);

    WaveformPlot::makePopupMenu();
}

ParseCmd WaveformView::parseCmd(const string &cmd, string &msg)
{
    string c, s1, s2;
    int id = -1;
    double tmin=NULL_TIME, tmax=NULL_TIME;
    gvector<Waveform *> wvec;
    Waveform *w;

    if(parseCompare(cmd, "Copy")) {
	getSelectedWaveforms(wvec);
	copyWaveforms(wvec);
    }
    else if(parseCompare(cmd, "Copy ", 5)) {
	if(parseGetArg(cmd, "_wave_", &id)) {
	    if(!(w = getWaveform(id))) {
		msg.assign("Waveform not found.");
		return ARGUMENT_ERROR;
	    }
	    gvector<Waveform *> v(w);
	    if(parseGetArg(cmd, "tmin", s1) && parseGetArg(cmd, "tmax", s2)) {
		if(timeParseString(s1.c_str(), &tmin) != 1) {
		    msg.assign("Cannot parse tmin.");
		    return ARGUMENT_ERROR;
		}
		if(timeParseString(s2.c_str(), &tmax) != 1) {
		    msg.assign("Cannot parse tmax.");
		    return ARGUMENT_ERROR;
		}
	    }
	    copyWaveforms(v, tmin, tmax);
	}
	else {
	    return parseCopyTable(cmd, msg);
	}
    }
    else if(parseCompare(cmd, "Cut")) {
	getSelectedWaveforms(wvec);
	cutWaveforms(wvec);
    }
    else if(parseCompare(cmd, "Cut ", 4)) {
	if(parseGetArg(cmd, "_wave_", &id)) {
	    if(!(w = getWaveform(id))) {
		msg.assign("Waveform not found.");
		return ARGUMENT_ERROR;
	    }
	    else {
		getSelectedWaveforms(wvec);
		cutWaveforms(wvec);
	    }
	}
    }
    else if(parseCompare(cmd, "Paste")) {
	pasteData();
    }
    else if(parseArg(cmd, "display_waveform", c)) {
	return parseDisplayWaveform(c, msg);
    }
    else if(parseArg(cmd, "arrival_menu", c)) {
	if(!arrival_popup) {
	    arrival_popup = createArrivalPopup();
	}
 	return arrival_popup->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "waveform_menu", c)) {
	if(!waveform_popup) {
	    waveform_popup = createWaveformPopup();
	}
 	return waveform_popup->parseCmd(c, msg);
    }
    else {
	return WaveformPlot::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd WaveformView::parseCopyTable(const string &cmd, string &msg)
{
    char name[100];
    int i, n;
    long addr;
    const char **table_names=NULL;
    char table[100];
    int num_css = CssTableClass::getAllNames(&table_names);

    memset(table, 0, sizeof(table));
    for(i = 0; i < num_css; i++) {
	snprintf(name, sizeof(name), "_%s_", table_names[i]);
	if(parseGetArg(cmd, name, &addr)) {
	    strncpy(table, table_names[i], sizeof(table));
	    break;
	}
    }
    free(table_names);

    if(table[0] == '\0') {
	msg.assign("copy: unrecognized object.");
	return ARGUMENT_ERROR;
    }
    gvector<CssTableClass *> v;
    getTable(table, v);

    if(!paste_table) {
	paste_table = new CSSTable("paste_table", this);
    }
    paste_table->removeAllRecords();
    for(i = 0; i < v.size(); i++) {
	if(addr == (long)v[i]) {
	    paste_table->addRecord(v[i], true);
//	    paste_table->selectRecord(v[i], true);
	    paste_table->selectAllRows(true);
	    paste_table->copy(TABLE_COPY_ROWS, CurrentTime);
	    break;
	}
    }
    n = v.size();
    return (i < n) ? COMMAND_PARSED : COMMAND_NOT_FOUND;
}

ParseVar WaveformView::parseVar(const string &name, string &value)
{
    string c;
    ostringstream os;
    int i;
    gvector<Waveform *> wvec;

    if(parseCompare(name, "edit_arrival")) {
	if(cplot_menu.arrival) {
	    os << "arrival=" << (long)cplot_menu.arrival;
	    value.assign(os.str());
	    return STRING_RETURNED;
	}
	return VARIABLE_ERROR;
    }
    else if(parseArg(name, "edit_arrival", c)) {
	if(cplot_menu.arrival) {
	    if(getTabMember(c.c_str(), cplot_menu.arrival, value))
	    {
		return STRING_RETURNED;
	    }
	    return VARIABLE_NOT_FOUND;
	}
	return VARIABLE_ERROR;
    }
    else if(parseCompare(name, "edit_wave")) {
	if(cplot_menu.w) {
	    getWaveforms(wvec, false);
	    for(i = 0; i < wvec.size() && wvec[i] != cplot_menu.w; i++);
	    if(i < wvec.size()) {
		os << "wave[" << i+1 << "]";
		return WaveformPlot::parseVar(os.str(), value);
	    }
	}
	return VARIABLE_ERROR;
    }
    else if(parseArg(name, "edit_wave", c)) {
	if(cplot_menu.w) {
	    if(Parser::getTSMember(c.c_str(), cplot_menu.w, value)) {
		return STRING_RETURNED;
	    }
	    return VARIABLE_NOT_FOUND;
	}
	return VARIABLE_ERROR;
    }
    else if(parseArg(name, "arrival_menu", c)) {
	if(!arrival_popup) {
	    arrival_popup = createArrivalPopup();
	}
 	return arrival_popup->parseVar(c, value);
    }
    else if(parseArg(name, "waveform_menu", c)) {
	if(!waveform_popup) {
	    waveform_popup = createWaveformPopup();
	}
 	return waveform_popup->parseVar(c, value);
    }
    return WaveformPlot::parseVar(name, value);
}

void WaveformView::parseHelp(const char *prefix)
{
    printf("%scopy\n", prefix);
    printf("%scut\n", prefix);
    printf("%spaste\n", prefix);
    printf("%sdisplay_waveform sta=STA chan=CHAN\n", prefix);

    WaveformPlot::parseHelp(prefix);
}

ParseCmd WaveformView::parseDisplayWaveform(const string &cmd, string &msg)
{
    string sta, chan;
    gvector<Waveform *> wvec;
    Waveform *w;
    int id = -1;

    if(parseGetArg(cmd, "_wave_", &id)) {
	if(!(w = getWaveform(id))) {
	    msg.assign("Waveform not found.");
	    return ARGUMENT_ERROR;
	}
	cplot_menu.w = w;
	displayWaveform();
	cplot_menu.w = NULL;
	return COMMAND_PARSED;
    }

    if(!parseGetArg(cmd, "sta", sta) || !parseGetArg(cmd, "chan", chan)) {
        msg.assign("invalid display_waveform sta=STA chan=CHAN");
        return ARGUMENT_ERROR;
    }
    getWaveforms(wvec);

    for(int i = 0; i < wvec.size(); i++) {
	if(parseCompare(sta, wvec[i]->sta()) &&
		compareChan(chan, wvec[i]->chan()) && wvec[i]->length() == 2)
	{
	    cplot_menu.w = wvec[i];
	    displayWaveform();
	    cplot_menu.w = NULL;
	    break;
	}
    }
    return COMMAND_PARSED;
}
