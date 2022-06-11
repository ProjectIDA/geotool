/** \file CPlotClass.cpp
 *  \brief Defines class CPlotClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include "widget/CPlotClass.h"
#include "motif++/Application.h"
#include "Waveform.h"
#include "motif++/Frame.h"
#include "widget/PrintDialog.h"
#include "PrintClient.h"
#include "widget/TableListener.h"

extern "C" {
#include "libstring.h"
}

using namespace std;

static bool load_resources = true;
static bool load_key_actions = true;

static vector<CPlotClass *> all_cplots;

class MagnifyWindow : public Frame, public PrintClient
{
    public :
	MagnifyWindow(const string &name, Component *parent, CPlotClass *cp) :
		Frame(name, parent, true)
	{
	    int n;
	    Arg args[20];

	    setSize(425, 380);
	    cp_client = cp;

	    n = 0;
	    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	    RowColumn *rc = new RowColumn("rc", frame_form, args, n);
	    new Button("Close", rc, this);
	    new Button("Print", rc, this);

	    info_area = new InfoArea("info_area", this);

	    n = 0;
	    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
	    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
	    XtSetArg(args[n], XtNinfoWidget2, info_area->leftInfo()); n++;
	    XtSetArg(args[n], XtNautoYScale, False); n++;
	    XtSetArg(args[n], XtNcurvesOnly, False); n++;
	    XtSetArg(args[n], XtNallowPartialSelect, False); n++;
	    plot1 = new CPlotClass("Magnify", frame_form, args, n);

	    print_window = NULL;
	}
	~MagnifyWindow(void) {}
	virtual void print(FILE *fp, PrintParam *p) {
	    AxesParm *a;
	    p->right -= 1.;
	    if((a = plot1->hardCopy(fp, p, NULL))) {
		free(a);
	    }
	}
	virtual void setVisible(bool visible)
	{
	    Frame::setVisible(visible);
	    if(!visible) {
		cp_client->magnifyWidget(NULL, false);
	    }
	}
	CPlotClass *cp_client;
	CPlotClass *plot1;

    protected :
	PrintDialog *print_window;

	void actionPerformed(ActionEvent *action_event) {
	    const char *cmd = action_event->getActionCommand();
	    if(!strcmp(cmd, "Close")) {
		setVisible(false);
	    }
	    else if(!strcmp(cmd, "Print")) {
		if(print_window == NULL) {
		    print_window = new PrintDialog("Print Waveforms",this,this);
		}
		print_window->setVisible(true);
	    }
	}
};

CPlotClass::CPlotClass(const string &name, Component *parent, Arg *args, int n)
		: AxesClass(cPlotWidgetClass, name, parent, args, n), Parser()
{
    initWidget(NULL, args, n);
}

CPlotClass::CPlotClass(const string &name, Component *parent,
		InfoArea *infoarea, Arg *args, int n) :
		AxesClass(cPlotWidgetClass, name, parent, args, n), Parser()
{
    initWidget(infoarea, args, n);
}

// use this constructor in a "Widget" subclass (ie TtPlot, etc)
CPlotClass::CPlotClass(WidgetClass widget_class, const string &name,
		Component *parent, Arg *args, int n) :
		AxesClass(widget_class, name, parent, args, n), Parser()
{
    initWidget(NULL, args, n, false);
}

// use this constructor in a "Widget" subclass (ie TtPlot, etc)
CPlotClass::CPlotClass(WidgetClass widget_class, const string &name,
		Component *parent, InfoArea *infoarea, Arg *args, int n) :
		AxesClass(widget_class, name, parent, args, n), Parser()
{
    initWidget(infoarea, args, n, false);
}

void CPlotClass::initWidget(InfoArea *infoarea, Arg *args, int n,
		bool reset_y_limits)
{
    int m;
    Arg a[20];

    cw = (CPlotWidget)base_widget;
    CPlotSetClass(cw, this);

    all_cplots.push_back(this);

    info_area = infoarea;

    loadResources();

    if(reset_y_limits) {
	setYLimits(0., 1., true);
    }

    magnify_window = NULL;
   
    XtAddCallback(base_widget, XtNsingleSelectDataCallback,
		CPlotClass::singleSelectDataCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectDataCallback,
		CPlotClass::selectDataCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNpositionCallback,
		CPlotClass::positionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNpositionDragCallback,
		CPlotClass::positionDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectArrivalCallback,
		CPlotClass::selectArrivalCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmeasureCallback,
		CPlotClass::measureCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNretimeCallback,
		CPlotClass::retimeCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNretimeDragCallback,
		CPlotClass::retimeDragCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNwaveformMenuCallback,
		CPlotClass::waveformMenuCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNarrivalMenuCallback,
		CPlotClass::arrivalMenuCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNwaveformInfoCallback,
		CPlotClass::waveformInfoCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNarrivalInfoCallback,
		CPlotClass::arrivalInfoCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmodifyWaveformCallback,
		CPlotClass::modifyWaveformCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmagnifyCallback,
		CPlotClass::magnifyCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmenuCallback,
		CPlotClass::menuCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNaddArrivalCallback,
		CPlotClass::addArrivalCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNaddArrivalMenuCallback,
		CPlotClass::addArrivalMenuCallback, (XtPointer)this);

    enableCallbackType(XtNsingleSelectDataCallback);
    enableCallbackType(XtNselectDataCallback);
    enableCallbackType(XtNpositionCallback);
    enableCallbackType(XtNpositionDragCallback);
    enableCallbackType(XtNselectArrivalCallback);
    enableCallbackType(XtNmeasureCallback);
    enableCallbackType(XtNretimeCallback);
    enableCallbackType(XtNretimeDragCallback);
    enableCallbackType(XtNwaveformMenuCallback);
    enableCallbackType(XtNarrivalMenuCallback);
    enableCallbackType(XtNwaveformInfoCallback);
    enableCallbackType(XtNarrivalInfoCallback);
    enableCallbackType(XtNmodifyWaveformCallback);
    enableCallbackType(XtNmagnifyCallback);
    enableCallbackType(XtNmenuCallback);
    enableCallbackType(XtNaddArrivalCallback);
    enableCallbackType(XtNaddArrivalMenuCallback);

    m = 0;
    if(info_area) {
        XtSetArg(a[m], XtNinfoWidget, info_area->rightInfo()); m++;
        XtSetArg(a[m], XtNinfoWidget2, info_area->leftInfo()); m++;
    }
    if( !inArgs(XtNcurvesOnly, args, n) ) {
        XtSetArg(a[m], XtNcurvesOnly, True); m++;
    }
    if( !inArgs(XtNtimeScale, args, n) ) {
        XtSetArg(a[m], XtNtimeScale, TIME_SCALE_SECONDS); m++;
    }
    if( !inArgs(XtNautoYScale, args, n) ) {
        XtSetArg(a[m], XtNautoYScale, True); m++;
    }
    if(m) setValues(a, m);

    setAppContext(Application::getApplication()->appContext());
}

CPlotClass::~CPlotClass(void)
{
    for(int i = 0; i < (int)all_cplots.size(); i++) {
	if(all_cplots.at(i) == this) {
	    all_cplots.erase(all_cplots.begin()+i);
	    break;
	}
    }
}

void CPlotClass::destroy(void)
{
    if(base_widget) {
	XtRemoveCallback(base_widget, XtNsingleSelectDataCallback,
		CPlotClass::singleSelectDataCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNselectDataCallback,
		CPlotClass::selectDataCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNpositionCallback,
		CPlotClass::positionCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNpositionDragCallback,
		CPlotClass::positionDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNselectArrivalCallback,
		CPlotClass::selectArrivalCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNmeasureCallback,
		CPlotClass::measureCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNretimeCallback,
		CPlotClass::retimeCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNretimeDragCallback,
		CPlotClass::retimeDragCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNwaveformMenuCallback,
		CPlotClass::waveformMenuCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNarrivalMenuCallback,
		CPlotClass::arrivalMenuCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNwaveformInfoCallback,
		CPlotClass::waveformInfoCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNarrivalInfoCallback,
		CPlotClass::arrivalInfoCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNmodifyWaveformCallback,
		CPlotClass::modifyWaveformCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNmagnifyCallback,
		CPlotClass::magnifyCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNmenuCallback,
		CPlotClass::menuCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNaddArrivalCallback,
		CPlotClass::addArrivalCallback, (XtPointer)this);
	XtRemoveCallback(base_widget, XtNaddArrivalMenuCallback,
		CPlotClass::addArrivalMenuCallback, (XtPointer)this);
    }
    AxesClass::destroy();
}

void CPlotClass::singleSelectDataCallback(Widget w, XtPointer client,
			XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNsingleSelectDataCallback);
/* always get a XtNselectDataCallback with a XtNsingleSelectDataCallback
    cplot->resetDataChange(false);
    cplot->change.select_waveform = true;
    cplot->doCallbacks(w, (XtPointer)&cplot->change, XtNdataChangeCallback);
*/
}
void CPlotClass::selectDataCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNselectDataCallback);
    cplot->change.select_waveform = true;
    cplot->doCallbacks(w, (XtPointer)&cplot->change, XtNdataChangeCallback);
    cplot->resetDataChange(false);
}
void CPlotClass::positionCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    CPlotPositionCallbackStruct *c = (CPlotPositionCallbackStruct *)data;
    cplot->setParseWaveforms(c->wvec);
    cplot->doCallbacks(w, data, XtNpositionCallback);
}
void CPlotClass::positionDragCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNpositionDragCallback);
}
void CPlotClass::selectArrivalCallback(Widget w, XtPointer client,
			XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->setParseArrival((CssArrivalClass *)data);
    cplot->doCallbacks(w, data, XtNselectArrivalCallback);
    cplot->change.select_arrival = true;
    cplot->doCallbacks(w, (XtPointer)&cplot->change, XtNdataChangeCallback);
    cplot->resetDataChange(false);
}
void CPlotClass::measureCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNmeasureCallback);
}
void CPlotClass::retimeCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->setParseArrival((CssArrivalClass *)data);
    cplot->doCallbacks(w, data, XtNretimeCallback);
}
void CPlotClass::retimeDragCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNretimeDragCallback);
}
void CPlotClass::waveformMenuCallback(Widget w, XtPointer client,XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNwaveformMenuCallback);
}
void CPlotClass::arrivalMenuCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNarrivalMenuCallback);
}
void CPlotClass::waveformInfoCallback(Widget w, XtPointer client,XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNwaveformInfoCallback);
}
void CPlotClass::arrivalInfoCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    CPlotInfoCallbackStruct *c = (CPlotInfoCallbackStruct *)data;
    cplot->setParseArrival(c->a);
    cplot->setParseWaveform(c->w);
    cplot->doCallbacks(w, data, XtNarrivalInfoCallback);
}

void CPlotClass::setParseArrival(CssArrivalClass *a)
{
    int i;
    cvector<CssArrivalClass> *arrivals = getArrivalRef();

    for(i = 0; i < arrivals->size() && arrivals->at(i) != a; i++);
    if(i < arrivals->size()) {
	char s[20];
	snprintf(s, sizeof(s), "%d", i+1);
	Application::putParseProperty("wpa", s);
    }
}
void CPlotClass::setParseWaveform(Waveform *w)
{
    int i, num;
    gvector<Waveform *> wvec;

    if((num = getWaveforms(wvec, false)) > 0) {
	for(i = 0; i < num && wvec[i] != w; i++);
	if(i < num) {
	    char s[20];
	    snprintf(s, sizeof(s), "%d", i+1);
	    Application::putParseProperty("wpw", s);
	}
    }
}
void CPlotClass::setParseWaveforms(gvector<Waveform *> &ws)
{
    int i, len;
    char s[1000];
    gvector<Waveform *> wvec;

    if(ws.size() > 0 && getWaveforms(wvec, false) > 0) {
	s[0] = '\0';
	for(int j = 0; j < wvec.size(); j++) {
	    for(i = 0; i < ws.size() && wvec[j] != ws[i]; i++);
	    if(i < ws.size()) {
		len = (int)strlen(s);
		snprintf(s+len, sizeof(s)-len, "%d,", j+1);
	    }
	}
	len = (int)strlen(s);
	if(len > 0) s[len-1] = '\0'; // the last comma
	Application::putParseProperty("wpw", s);
    }
}

void CPlotClass::modifyWaveformCallback(Widget w, XtPointer client,
			XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    CPlotModifyCallbackStruct *c = (CPlotModifyCallbackStruct *)data;
    cplot->setParseWaveforms(c->wvec);
    cplot->doCallbacks(w, data, XtNmodifyWaveformCallback);
}
void CPlotClass::magnifyCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    if(!cplot->magnify_window) {
	cplot->magnify_window = new MagnifyWindow("Magnify",
		cplot->formDialogParent(), cplot);
    }
    cplot->magnify_window->setVisible(true);
    cplot->magnifyWidget(
	(CPlotWidget)cplot->magnify_window->plot1->baseWidget(), false);
    cplot->doCallbacks(w, data, XtNmagnifyCallback);
}
void CPlotClass::menuCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNmenuCallback);
}
void CPlotClass::addArrivalCallback(Widget w, XtPointer client, XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    CPlotArrivalCallbackStruct *c = (CPlotArrivalCallbackStruct *)data;
    cplot->setParseWaveform(c->w);
    char s[20];
    snprintf(s, sizeof(s), "%.15g", c->time);
    Application::putParseProperty("new_arrival_time", s);
    cplot->doCallbacks(w, data, XtNaddArrivalCallback);
}
void CPlotClass::addArrivalMenuCallback(Widget w, XtPointer client,
		XtPointer data)
{
    CPlotClass *cplot = (CPlotClass *)client;
    cplot->doCallbacks(w, data, XtNaddArrivalMenuCallback);
}

bool CPlotClass::selectRecord(CssTableClass *record, bool select, bool do_callback)
{
    if(record->nameIs(cssArrival)) {
	if(do_callback) {
	    return selectArrivalWithCB((CssArrivalClass *)record, select);
	}
	else {
	    return selectArrival((CssArrivalClass *)record, select);
	}
    }
    return false;
}

int CPlotClass::getPathInfo(PathInfo **path_info)
{
    gvector<Waveform *> wvec;
    int num = getWaveforms(wvec);
    *path_info = NULL;
    if(!num) return 0;

    PathInfo *p = (PathInfo *)mallocWarn(num*sizeof(PathInfo));
    for(int i = 0; i < num; i++) {
	strcpy(p[i].sta, wvec[i]->sta());
	p[i].lat = wvec[i]->lat();
	p[i].lon = wvec[i]->lon();
	p[i].origin = getPrimaryOrigin(wvec[i]);
	p[i].fg = wvec[i]->fg;
    }
    *path_info = p;
    return num;
}

/** Hide the magnify window.
 */
void CPlotClass::hideMagWindow(void)
{
    if(magnify_window) magnify_window->setVisible(false);
}

/** Load resources. These resources do not effect the first CPlotClass instance
 *  that is created. They effect all subsequent instances. These resources
 *  are overridden by the resources in the args argument of the constructors.
 */
void CPlotClass::loadResources(void)
{
    if(load_resources) { // only need to do this one time.
	load_resources = false;
	ResourceManager *rm = Application::getResourceManager(this);

	rm->putResource("*CPlot*background: White");
	rm->putResource("*CPlot*scrollVert*background: grey80");
	rm->putResource("*CPlot*scrollHoriz*background: grey80");

	rm->putResource("*TtPlot*background: White");
	rm->putResource("*TtPlot*scrollVert*background: grey80");
	rm->putResource("*TtPlot*scrollHoriz*background: grey80");

	rm->close();
    }
}

int CPlotClass::copySelectedTimeSeries(gvector<GTimeSeries *> &tvec)
{
    gvector<Waveform *> v;
    GTimeSeries *ts;

    tvec.clear();
    if(getSelectedWaveforms("a", v) > 0)
    {
	for(int i = 0; i < v.size(); i++) {
	    double tbeg  = v[i]->dw[0].d1->time();
	    double tend  = v[i]->dw[0].d2->time();
	    ts = v[i]->ts->subseries(tbeg, tend);
	    if(!ts) ts = new GTimeSeries(v[i]->ts);
	    tvec.push_back(ts);
	}
    }
    else if(getSelectedWaveforms(v) > 0)
    {
	for(int i = 0; i < v.size(); i++) {
	    ts = v[i]->ts->subseries(v[i]->begSelect, v[i]->endSelect);
	    if(!ts) {
		ts = new GTimeSeries(v[i]->ts);
	    }
	    tvec.push_back(ts);
	}
    }
    return tvec.size();
}

void CPlotClass::copyAllTables(DataSource *ds)
{
    const char *table_names[] = {cssArrival, cssAssoc, cssOrigin, cssOrigerr,
		cssStamag, cssAmplitude, cssNetmag, cssStassoc, cssAmpdescript,
		cssHydroFeatures, cssInfraFeatures, cssWftag, cssParrival};
    gvector<CssTableClass *> tables;
    int i, j, ntable_names, num;

    ntable_names = sizeof(table_names)/sizeof(const char *);
    for(i = 0; i < ntable_names; i++) {
	tables.clear();
	num = ds->getTable(table_names[i], tables);
	for(j = 0; j < num; j++) putTable(tables[j]);
    }
}

void CPlotClass::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(strcasecmp(reason, CssTableChange)) return;

    TableListenerCallback *tc =
		(TableListenerCallback *)action_event->getCalldata();
    CssClassDescription *des = tc->table->description();

    resetDataChange(false);

    if(!strcasecmp(cmd, cssArrival))
    {
	CssArrivalClass *a = (CssArrivalClass *)tc->table;
	for(int i = 0; i < tc->num_members; i++)
	{
	    char *name = des[tc->members[i]].name;
	    if(!strcmp(name, "phase")) {
		renameArrival(a, a->phase, false);
	    }
	    else if(!strcmp(name, "iphase")) {
		renameArrival(a, a->iphase, false);
	    }
	    else if(!strcmp(name, "time")) {
		moveArrival(a, a->time, false);
	    }
	}
	change.arrival = true;
    }
    else if(!strcasecmp(cmd, cssAssoc))
    {
	cvector<CssArrivalClass> *arrivals = getArrivalRef();
	CssAssocClass *asc = (CssAssocClass *)tc->table;
	for(int i = 0; i < tc->num_members; i++)
	{
	    char *name = des[tc->members[i]].name;
	    if(!strcmp(name, "phase")) {
		int j;
		for(j = 0; j < arrivals->size() &&
			arrivals->at(j)->arid != asc->arid; j++);
		if(j < arrivals->size()) {
		    renameAssoc(asc, arrivals->at(j), asc->phase, false);
		}
	    }
	}
    }
    else if(!strcasecmp(cmd, "delete"))
    {
	if(tc->table->nameIs(cssArrival)) {
	    change.arrival = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssAssoc)) {
	    change.assoc = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssStassoc)) {
	    change.stassoc = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssAmplitude)) {
	    change.amplitude = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssAmpdescript)) {
	    change.ampdescript = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssFilter)) {
	    change.filter = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssHydroFeatures)) {
	    change.hydro = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssInfraFeatures)) {
	    change.infra = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssStamag)) {
	    change.stamag = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssParrival)) {
	    change.parrival = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssNetmag)) {
	    change.netmag = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssOrigerr)) {
	    change.origerr = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssWftag)) {
	    change.wftag = true;
	    removeTable(tc->table, false);
	}
	else if(tc->table->nameIs(cssOrigin)) {
	    change.origin = true;
	    removeTable(tc->table, false);
	}
    }
    doDataChangeCallbacks();
}

// static 
void CPlotClass::addTableToAll(CssTableClass *table, CPlotClass *except_cp)
{
    for(int i = 0; i < (int)all_cplots.size(); i++)
	if(!except_cp || all_cplots.at(i) != except_cp)
    {
	CPlotClass *cp = all_cplots.at(i);
	cp->putTable(table);
    }
}

int CPlotClass::getKeyActions(CPlotKeyAction **key_actions)
{
    char *prop = NULL;

    if( load_key_actions && (prop = getProperty("key_actions")) )
    {
	int num = 0;
	CPlotKeyAction *keys=NULL;
	char *key_code=NULL, *proc_call=NULL, *last;

	load_key_actions = false;

	key_code = strtok_r(prop, ",", &last);
	proc_call = strtok_r(NULL, ",", &last);

	if(key_code && proc_call)
	{
	    keys = (CPlotKeyAction *)malloc(sizeof(CPlotKeyAction));
	    stringTrimCopy(keys[0].key_code, key_code,
				sizeof(keys[num].key_code));
	    stringTrimCopy(keys[0].proc_call, proc_call,
				sizeof(keys[num].proc_call));
	    num = 1;
	    while( (key_code = strtok_r(NULL, ",", &last)) &&
		   (proc_call = strtok_r(NULL, ",", &last)) )
	    {
		keys = (CPlotKeyAction *)realloc(keys,
				(num+1)*sizeof(CPlotKeyAction));
		stringTrimCopy(keys[num].key_code, key_code,
				sizeof(keys[num].key_code));
		stringTrimCopy(keys[num].proc_call, proc_call,
				sizeof(keys[num].proc_call));
		num++;
	    }
	}
	CPlotSetKeyActions(num, keys);
	Free(keys);
    }

    return CPlotGetKeyActions(key_actions);
}

void CPlotClass::setKeyActions(int num, CPlotKeyAction *keys)
{
    CPlotSetKeyActions(num, keys);

    char *prop=NULL;
    int i, len, n;

    len = 0;
    for(i = 0; i < num; i++) {
	len += strlen(keys[i].key_code);
	len += strlen(keys[i].proc_call);
	len += 2;
    }
    len++;

    prop = (char *)malloc(len*sizeof(char));
    prop[0] = '\0';
    n = 0;
    for(i = 0; i < num; i++) {
	n = strlen(prop);
	if(i == 0) {
	    snprintf(prop+n, len-n, "%s,%s", keys[i].key_code,
		keys[i].proc_call);
	}
	else {
	    snprintf(prop+n, len-n, ",%s,%s", keys[i].key_code,
		keys[i].proc_call);
	}
    }
    putProperty("key_actions", prop);
    Free(prop);
}

ParseCmd CPlotClass::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    if((ret = dataParseCmd(cmd.c_str(), msg)) != COMMAND_NOT_FOUND) return ret;
    return AxesClass::parseCmd(cmd, msg);
}

ParseVar CPlotClass::parseVar(const string &name, string &value)
{
    ParseVar ret;
    if((ret = dataParse(name, value)) != VARIABLE_NOT_FOUND) return ret;
    return AxesClass::parseVar(name, value);
}

void CPlotClass::parseHelp(const char *prefix)
{
    printf("\n");
    printf("Attributes:\n");
    printf("%sselected_waveforms (true/false)\n", prefix);
}
