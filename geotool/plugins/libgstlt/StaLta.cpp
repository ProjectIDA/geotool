/** \file StaLta.cpp
 *  \brief Defines class StaLta.
 *  \author John Coyne
 */
#include "config.h"
#include <iostream>
#include <pwd.h>
using namespace std;

#include "StaLta.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "WaveformPlot.h"
#include "DataMethod.h"
#include "Demean.h"
#include "TaperData.h"
#include "IIRFilter.h"
#include "Beam.h"
#include "BasicSource.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"

static int sort_by_net(const void *A, const void *B);
static int sort_by_sta(const void *A, const void *B);
static int sort_by_chan(const void *A, const void *B);
static int sort_can_by_time(const void *A, const void *B);
}
using namespace libgstlt;

StaLta::StaLta(const char *name, Component *parent, DataSource *ds)
		: Frame(name, parent), DataReceiver(NULL)
{
    createInterface();
    init();
    wp = NULL;
    setDataSource(ds);
}

void StaLta::createInterface(void)
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    arrivals_toggle = new Toggle("Consider Arrivals", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    select_menu = new Menu("Select", view_menu);
    select_all_button = new Button("Select All", select_menu, this);
    deselect_all_button = new Button("Deselect All", select_menu, this);

    option_menu = new Menu("Option", menu_bar);
    apply_button = new Button("Apply StaLta", option_menu, this);
    apply_button->setSensitive(false);
    XtSetArg(args[0], XmNvisibleWhenOff, True);
    rtd_compute_toggle = new Toggle("RTD Compute", option_menu, this, args, 1);
    rtd_compute_toggle->setVisible(false);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("StaLta Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("form", frame_form, args, n);

    more_button = new Button("+", rc, this);
    less_button = new Button("-", rc, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XtNtableTitle, "StaLta Parameters"); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XtNcolumns, 17); n++;
    const char *labels[] = {"name","type","group","staSec","ltaSec","htrig",
		"ltrig","snrThresh","wtrigSec","trgsepSec","bufSec","method",
		"low","high","order","constraint","type"};
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    const char *column_choice[] = {"","coh:inc","vertical:horizontal","","","",
		"","","","","","1:0","","","0:1:2:3:4:5:6:7:8:9:10",
		"zero phase:causal","BP:BR:LP:HP"};
    XtSetArg(args[n], XtNcolumnChoice, column_choice); n++;
    const char *cells[] = {
	"Z0816","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","0.8","1.6","3","causal","BP",
	"Z0822","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","0.8","2.2","3","causal","BP",
	"z0822","coh","vertical","0.5","60.0","4.0","4.0","4.0","1.0","2.0",
		"0.1","1","0.8","2.2","3","causal","BP",
	"Z0515","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","0.5","1.5","3","causal","BP",
	"Z1020","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","1.0","2.0","3","causal","BP",
	"Z1530","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","1.5","3.0","3","causal","BP",
	"Z2040","coh","vertical","1.0","60.0","3.0","3.0","3.0","1.0","2.0",
		"0.1","1","2.0","4.0","3","causal","BP",
	"Z4080","coh","vertical","1.0","60.0","3.0","3.0","3.5","1.0","2.0",
		"0.1","1","4.0","8.0","3","causal","BP", NULL,
    };
    XtSetArg(args[n], XtNcells, cells); n++;
    table = new Table("table", frame_form, args, n);
    table->addActionListener(this, XtNselectRowCallback);

    if(!tool_bar->loadDefaults()) {
	tool_bar->add(close_button, "Close");
	tool_bar->add(apply_button, "Apply");
    }
}

StaLta::~StaLta(void)
{
}

void StaLta::setDataSource(DataSource *ds)
{
    if(ds != data_source) {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, "RTDUpdate");
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) {
		if(wp->isRealTime()) {
		    wp->addActionListener(this, "RTDUpdate");
	 	    rtd_compute_toggle->setVisible(true);
		}
	    }
	}
	else wp = NULL;
    }
}

void StaLta::init(void)
{
/*
    char name[100], *last, *prop, *row[7], *tok, *s;

    if((prop = getProperty("Geotool.locate.numResiduals")) != NULL)
    {
	if(!stringToInt(prop, &num)) {
	    Free(prop);
	    return;
	}
	Free(prop);
    }

    for(i = 0; i < num; i++)
    {
	snprintf(name, sizeof(name), "Geotool.locate.residual%d", i);
	if((prop = getProperty(name)) != NULL)
	{
	    tok = prop;
	    for(j = 0; j < 7 && (s = strtok_r(tok, ",\t ", &last)) != NULL; j++)
	    {
		tok = NULL;
		row[j] = s;
	    }
	    table->addRow(row, false);
	    Free(prop);
	}
    }
    if(num <= 0) {
	char *row1[7] = {"P", "30", "100", "timres", "5.", "10.", "yellow"};
	char *row2[7] = {"P", "30", "100", "timres", "10.", "", "red"};
	table->addRow(row1, true);
	table->addRow(row2, true);
    }
    table->adjustColumns();
*/
}

void StaLta::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "+")) {
	more();
    }
    else if(!strcmp(cmd, "-")) {
	less();
    }
    else if(!strcmp(cmd, "Select All")) {
	table->selectAllRows(true);
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Deselect All")) {
	table->selectAllRows(false);
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Apply StaLta")) {
	apply();
    }
    else if(!strcmp(cmd, "StaLta Help")) {
	showHelp("StaLta Help");
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	setButtonsSensitive();
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *c = (DataChange *)action_event->getCalldata();
	if(c->select_waveform) {
	    setButtonsSensitive();
	}
    }
}

ParseCmd StaLta::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;
    string c;

    if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseArg(cmd, "consider_arrivals", c)) {
	return arrivals_toggle->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "select_all")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "deselect_all")) {
	deselect_all_button->activate();
    }
    else if(parseCompare(cmd, "apply")) {
	apply_button->activate();
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void StaLta::parseHelp(const char *prefix)
{
    printf("%sconsider_arrivals=(true,false)\n", prefix);
    printf("%sselect_all\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sapply\n", prefix);
    Table::parseHelp(prefix);
}

void StaLta::setButtonsSensitive(void)
{
    gvector<Waveform *> wvec;
    apply_button->setSensitive(table->rowSelected() &&
		data_source->getSelectedWaveforms(wvec) > 0);
}

void StaLta::more(void)
{
    int nrows = table->numRows();
    if(nrows > 0) {
	vector<const char *> row;
	if( !table->getRow(nrows-1, row)) return;
	table->addRow(row, true);
    }
    else {
	const char *row[17] = {"Z0816","coh","vertical","1.0","60.0","3.0",
	    "3.0","3.0","1.0","2.0","0.1","1","0.8","1.6","3","causal","BP"};
	table->addRow(row, true);
    }
}

void StaLta::less(void)
{
    int nrows;

    if((nrows = table->numRows()) > 1) {
	table->removeRow(nrows-1);
    }
}

void StaLta::apply(void)
{
    gvector<Waveform *> wvec;
    StaLtaDef *staltadef = NULL;
    int nrecipes;
    int i;
    DetectCandidate *c = NULL;
    int nC;
    cvector<CssArrivalClass> arrivals;

    if(!getStaLtaDef(&staltadef, &nrecipes)) {
	showWarning("Cannot get StaLta parameters.");
	return;
    }

    if(data_source->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No waveforms selected.");
	return;
    }

    // sort wvec by net, sta and channel name
    wvec.sort(sort_by_net);

    if( arrivals_toggle->state() ) {
	data_source->getTable(arrivals);
    }

/*
    for(i = 0; i < nrecipes; i++)
    {
	printf(" %.2f  %.2f %.2f %.2f %.2f %.2f %d\n",
		staltadef[i].staSec,
		staltadef[i].ltaSec,
		staltadef[i].htrig,
		staltadef[i].ltrig,
		staltadef[i].wtrigSec,
		staltadef[i].trgsepSec,
		staltadef[i].method);
    }
*/

    /* initialize number of candidates (nC) */
    nC = 0;
    detectOnWaveforms(wvec, staltadef, nrecipes, &c, &nC);

    processCandidates(staltadef, c, nC);

    compareWithArrivals(arrivals, staltadef, c, nC);

    showAllCandidates(c, nC);

    addDetectArrivals(wvec, staltadef, c, nC);

    for(i = 0; i < nrecipes; i++)
    {
	freeStaltaSpace(&staltadef[i]);
    }
    Free(staltadef);

    Free(c);
}

void StaLta::detectOnWaveforms(gvector<Waveform *> &wvec,
	StaLtaDef *staltadef, int nrecipes, DetectCandidate **c, int *nC)
{
    int	    i, j;
    int	    maxLen;
    int     zero_phase;
    double  cfreq;
    int     width = 5;
    int     minpts = 5;
    int     maxpts = 50;
    GTimeSeries *ts;
    int	nUsedMax, nUsed;
    bool restoreEverNeeded, restoreNeeded;

    // find the max length of the segments, which is used to allocate arrays
    maxLen = 0;
    for(i = 0; i < wvec.size(); i++)
    {
	if(wvec[i]->length() > maxLen) maxLen = wvec[i]->length();
    }

    for(i = 0; i < nrecipes; i++) {
	allocStaltaSpace(&staltadef[i], maxLen);
    }
	
    /* loop over the wavforms, and try to apply all the recipes.
	Note that some recipes are for single channels (e.g.,group == vertical),
	while other recipes are for > 1 channel (e.g., group == horizontal).
	In some cases, a recipe will not match a channel(s), in which case it
	will be skipped
     */
    gvector<DataMethod *> orig_methods;

    for(i = 0; i < wvec.size(); )
    {
	/* save any currently applied taper or filter, to possibly restore
	   them later */
	orig_methods.clear();

	gvector<DataMethod *> *m = wvec[i]->dataMethods();
	for(j = (int)m->size()-2; j > 0; j--) {
	    if(m->at(j)->getTaperDataInstance() &&
		m->at(j+1)->getIIRFilterInstance())
	    {
		orig_methods.push_back(m->at(j));
		orig_methods.push_back(m->at(j+1));
		m->at(j)->addOwner(this);
		m->at(j+1)->addOwner(this);
		break;
	    }
	}
	delete m;
    
	nUsedMax = 0;
	restoreEverNeeded = False;
	
	for(j = 0; j < nrecipes; j++)
	{
	    /* return the time series for this group (e.g., vertical or
		horizontal)
	     */
	    /*
	    restoreNeeded = True;
	    ts = wvec[i]->ts;
	    nUsed = 1;
	    */
	    ts = returnTS(staltadef[j].group, wvec, i, &nUsed, &restoreNeeded);

	    // record maximum number of used channels, so increment i correctly
	    if (nUsed > nUsedMax) nUsedMax = nUsed;

	    // if we ever need to restore the taper/filter, record that
	    if (restoreNeeded) restoreEverNeeded = restoreNeeded;

	    if (ts == NULL)
	    {
		continue;
	    }

	    /* apply the taper and current filter */
	    zero_phase = (staltadef[j].f[0].constraint[0] == 'z') ? 1 : 0;

	    Demean *demean = new Demean();
	    TaperData *taper = new TaperData("cosine", width, minpts, maxpts);
	    IIRFilter *iir = new IIRFilter(staltadef[j].f[0].forder,
			staltadef[j].f[0].ftype,
			staltadef[j].f[0].lofreq,
			staltadef[j].f[0].hifreq,
			wvec[i]->segment(0)->tdel(),
			zero_phase);

	    cfreq = staltadef[j].f[0].lofreq + ((staltadef[j].f[0].hifreq -
				staltadef[j].f[0].lofreq)/2.);

	    DataMethod *dm[3];
	    dm[0] = (DataMethod *)demean;
	    dm[1] = (DataMethod *)iir;
	    dm[2] = (DataMethod *)taper;
	    DataMethod::changeMethods(3, dm, 1, &ts);

	    applyDetector(wvec[i]->net(), ts, i, staltadef[j], j, cfreq, c,nC);

	    demean->deleteObject();
	    iir->deleteObject();
	    taper->deleteObject();

	    if(restoreNeeded) {
		ts->deleteObject();
	    }
	}

	if (restoreEverNeeded )
	{
	    restoreWaveform(wvec[i], &orig_methods);
	}
	i = i + nUsedMax;

	for(j = 0; j < (int)orig_methods.size(); j++) {
	    orig_methods[j]->removeOwner(this);
	}
    }
}

int StaLta::addDetectArrivals(gvector<Waveform *> &wvec,
		StaLtaDef *staltadef, DetectCandidate *c, int nC)
{
    int	i, n;
    CssArrivalClass *arrival;
    double azimuth = -1., slow = -1., ema = -1., rect = -1.;
    double deltim = -1., delaz = -1., delslo = -1.;
    bool add_to_net = false;
    Password password;
    char phase[9];

    if( !wp ) return 0;

    password = getpwuid(getuid());

    for (i=0, n=0; i<nC; i++)
    {
	if (c[i].state == 3)
	{
	    snprintf(phase, sizeof(phase), "%s-%.1f",
			staltadef[c[i].bandIndex].name, c[i].snr);
	    arrival = wp->makeArrival(wvec[c[i].dataIndex],password,
				c[i].time, phase, deltim, azimuth, delaz, slow,
				delslo, ema, rect, c[i].snr, add_to_net);
	    if(arrival) {
		wp->putArrivalWithColor(arrival, stringToPixel("black"));
	    }
	    n++;
	}
    }
    return n;
}

bool StaLta::getStaLtaDef(StaLtaDef **staltadef, int *nrecipes)
{
    vector<const char *> row;
    vector<int> rows;
    StaLtaDef *s = NULL;
    double dval;
    int	i, n;

    if( (n = table->getSelectedRows(rows)) <= 0)
    {
	showWarning("No StaLta parameters selected.");
	return false;
    }

    if(!(s = (StaLtaDef *)mallocWarn(n*sizeof(StaLtaDef)))) return false;

    for(i = 0; i < n; i++)
    {
	if( !table->getRow(rows[i], row) ) {
	    showWarning("getStaLtaDef: cannot get StaLta parameters.");
	    return false;
	}

	if(!strncpy(s[i].name, row[0], sizeof(s[i].name))) {
	    showWarning("Invalid name.");
	    return false;
	}
	if(!strncpy(s[i].beam_type, row[1], sizeof(s[i].beam_type))) {
	    showWarning("Invalid beam_type.");
	    return false;
	}
	if(!strncpy(s[i].group, row[2], sizeof(s[i].group))) {
	    showWarning("Invalid name.");
	    return false;
	}
	if(!stringToDouble(row[3], &s[i].staSec)) {
	    showWarning("Invalid staSec.");
	    return false;
	}
	if(!stringToDouble(row[4], &s[i].ltaSec)) {
	    showWarning("Invalid ltaSec.");
	    return false;
	}
	if(!stringToDouble(row[5], &s[i].htrig)) {
	    showWarning("Invalid htrig.");
	    return false;
	}
	if(!stringToDouble(row[6], &s[i].ltrig)) {
	    showWarning("Invalid ltrig.");
	    return false;
	}
	if(!stringToDouble(row[7], &s[i].snrThreshold)) {
	    showWarning("Invalid snrThreshold.");
	    return false;
	}
	if(!stringToDouble(row[8], &s[i].wtrigSec)) {
	    showWarning("Invalid wtrig.");
	    return false;
	}
	if(!stringToDouble(row[9], &s[i].trgsepSec)) {
	    showWarning("Invalid trgsep.");
	    return false;
	}
	if(!stringToDouble(row[10], &s[i].bufSec)) {
	    showWarning("Invalid bufSec.");
	    return false;
	}
	if(!stringToInt(row[11], &s[i].method)) {
	    showWarning("Invalid method.");
	    return false;
	}
	s[i].nfilters = 1;
        if(!(s[i].f = (FilterDef *)mallocWarn(1*sizeof(FilterDef)))) {
	    return false;
	}

	if(!stringToDouble(row[12], &dval)) {
	    showWarning("Invalid lofreq.");
	    return false;
	}
	s[i].f[0].lofreq = dval;

	if(!stringToDouble(row[13], &dval)) {
	    showWarning("Invalid hifreq.");
	    return false;
	}
	s[i].f[0].hifreq = dval;

	if(!stringToInt(row[14], &s[i].f[0].forder)) {
	    showWarning("Invalid forder.");
	    return false;
	}
	if(!strncpy(s[i].f[0].constraint, row[15],sizeof(s[i].f[0].constraint)))
	{
	    showWarning("Invalid zero phase/causal.");
	    return false;
	}
	if(!strncpy(s[i].f[0].ftype, row[16], sizeof(s[i].f[0].ftype))) {
	    showWarning("Invalid ftype.");
	    return false;
	}
    }
/*
name,type,group,staSec,ltaSec,htrig,ltrig,snrThresh,wtrigSec,trgsepSec,method,flo,fhi,ford,zp,ftype
*/
    *staltadef = s;
    *nrecipes = n;

    return true;
}

/*
	  	ApplyDetector(ts, staltadef[i], i, cfreq, &c, &nC);
        ProcessCandidates(p->s, c, nC);
*/

#define	TDEL_TOLERANCE  0.02
#define	CALIB_TOLERANCE  0.02
#define	CALPER_TOLERANCE  0.02

int StaLta::applyDetector(const char * net, GTimeSeries *ts, int dataIndex,
	StaLtaDef s, int bandIndex, double cfreq, DetectCandidate **c, int *nC)
{
    int	i, k, n;
    int	ndet = 0;
    bool dataExtended;
    double gapSec;
    int	data_length;
    float *data = NULL;
    int	offset, prev_nsamp;
    int	half_lta;

    n = *nC;

    /*
	at some point we should test TimeSeries_splineSegments(TimeSeries ts,
		double maxDtChange, int maxGap)
    */

    for(i=0; i<ts->size(); i++)
    {
	GSegment *seg = ts->segment(i);
	applyStaLtaSamprate(&s, seg->tdel());


	dataExtended = False;
	/*
	  if this is not the first segment, determine the gap with the preceeding segment.
	  if gap < lta, then prepend the ltaSec of data from the preceeding segment.
	  if gap < 4 * lta, then prepend the ltaSec/2 of data from the preceeding segment.
	  if gap > 4 * lta, then don't prepend anything
	 */
	if (i > 0 
	    && fabs(ts->segment(i-1)->tdel() - seg->tdel())/seg->tdel()
		< TDEL_TOLERANCE 
            && fabs(ts->segment(i-1)->calib() - seg->calib())/seg->calib()
		< CALIB_TOLERANCE 
            && fabs(ts->segment(i-1)->calper() - seg->calper())/seg->calper()
		< CALPER_TOLERANCE )
	{
	    gapSec = seg->tbeg() - ts->segment(i-1)->tbeg() +
		    (ts->segment(i-1)->length() * ts->segment(i-1)->tdel());
	    if (gapSec < s.ltaSec)
	    {
		// determine number of samples to take from previous segment
		if (ts->segment(i-1)->length() >= s.lta)
		{
		    offset = ts->segment(i-1)->length() - s.lta;
		    prev_nsamp = s.lta;
		}
		else
		{
		    offset = 0;
		    prev_nsamp = ts->segment(i-1)->length();
		}

		data_length = seg->length() + prev_nsamp;
printf(" alloc length of %d\n", data_length);
		data = (float *) mallocWarn(data_length * sizeof(float));
		if (data == NULL) {
		    return 0;
		}

		memcpy(data, ts->segment(i-1)->data+offset,
			prev_nsamp * sizeof(float) );

	        offset = prev_nsamp;
		memcpy(data+prev_nsamp, ts->segment(i)->data,
			ts->segment(i)->length() * sizeof(float) );

		dataExtended = True;
	    }
	    else if (gapSec < s.ltaSec * 4.)
	    {
		half_lta = s.lta/2;

		// determine number of samples to take from previous segment
		if (ts->segment(i-1)->length() >= half_lta)
		{
		    offset = ts->segment(i-1)->length() - half_lta;
	            prev_nsamp = half_lta;
		}
		else
		{
		    offset = 0;
		    prev_nsamp = ts->segment(i-1)->length();
		}

		data_length = seg->length() + prev_nsamp;
printf(" alloc length of %d\n", data_length);
		data = (float *) mallocWarn(data_length * sizeof(float));
		if (data == NULL)
		{
		    return 0;
		}

		memcpy(data, ts->segment(i-1)->data+offset,
			prev_nsamp * sizeof(float) );
		/* memcpy(&data, &ts->s[i-1]->data[offset],
			prev_nsamp * sizeof(float) );*/

		offset = prev_nsamp;
		memcpy(data+prev_nsamp, ts->segment(i)->data,
			ts->segment(i)->length() * sizeof(float) );
		/*memcpy(&data[prev_nsamp], &ts->s[i]->data,
			ts->s[i]->data_length * sizeof(float) );*/

		dataExtended = True;
	    }
	}
	/*	
	  s.sta  s.lta
 	  *seg = {core = 0x88aa830, tbeg = 1127545200.0090001, tdel = 0.025000000000000001, \
          data = 0x88aa900, data_length = 26799, calib = 0.26100000739097595, calper = 1}
	*/
	if (dataExtended)
	{
	    ndet = stalta(&s, data_length, data);
	    Free(data);
	}
	else
	{
	    ndet = stalta(&s, seg->length(), seg->data);
	}

	printf("ndet = %d\n", ndet);

	if (ndet < 1) continue;

	/* the extra condition for *c == NULL is because we may have already
	 * allocated for *c, but there may have been no detections so n may
	 * still be 0.
         */
	if (n == 0 && *c == NULL)
	{
	    *c = (DetectCandidate *) mallocWarn(ndet*sizeof(DetectCandidate));
	}
	else
	{
	    *c = (DetectCandidate *) reallocWarn(*c,
				(n + ndet)*sizeof(DetectCandidate));
	}

	/* record the candidate detections if snr > snrThreshold.  For each
	 * successful detection record the attributes: time, duration, snr,
	 * cfreq, bandIndex bandIndex - points back to all the processing
	 * parameters.
	 */
	for (k=0; k<ndet; k++)
	{
	    if (s.maxratio[k] > s.snrThreshold)
	    {
		(*c)[n].time = seg->tbeg() + (s.ontime[k]*seg->tdel());
		(*c)[n].duration = (s.offtime[k] - s.ontime[k])*seg->tdel();
		(*c)[n].snr = s.maxratio[k];
		(*c)[n].cfreq = cfreq;
		(*c)[n].bandIndex = bandIndex;
		(*c)[n].dataIndex = dataIndex;
		(*c)[n].state = 0;
		(*c)[n].arid = -1;
		strcpy((*c)[n].group, s.group);
		strcpy((*c)[n].sta, net);
		n++;
	    }
	}
    }
    *nC = n;
    return ndet;
}

void StaLta::showAllCandidates(DetectCandidate *c, int nC)
{
    int i;
    char tlab[60];

    printf("num                   time  sta state snr   dur   cfq bandIndex\n");

    for (i=0; i<nC; i++)
    {
	timeEpochToString(c[i].time, tlab, 60, GSE20);
	printf("%03d  %s  %-5.5s %1d %5.1f %5.1f %5.1f   %d\n",
		i, 
		tlab,
		c[i].sta,
		c[i].state,
		c[i].snr,
		c[i].duration, 
		c[i].cfreq,
		c[i].bandIndex);
    }
}

bool StaLta::processCandidates(StaLtaDef *s, DetectCandidate *c, int nC)
{
    int	i, j, best;

    // sort the candidates by time
    qsort(c, nC,  sizeof(DetectCandidate), sort_can_by_time);

    /* loop over the candidates. for the ones which are close in time, 
	choose the one with the highest snr
     */
    // i is increased inside the loop, depending on which candidates are processed
    for (i=0; i<nC; )
    {
        /* find number of additional candidates within trgsepSec of this candidate */
/* TBD, maybe we should have just one value for trgsepSec? */
	j = nearbyCandidates(c, nC, i, s[c[i].bandIndex].trgsepSec);

	// for candidates within this group, find the one with the highest SNR
	best = bestSNR(c, i, j);

	/* change the state of the best candidate */
	c[best].state = 1;

	/* increment i */
	i = i + 1 + j;
    }

    return true;
}

int StaLta::nearbyCandidates(DetectCandidate *c, int nC, int i,double trgsepSec)
{
    int k, n;
    double maxTime;

    maxTime = c[i].time + trgsepSec;

    for (k=i+1, n=0; k<nC; k++)
    {
	if (c[k].time > maxTime) break;
	n++;
    }

    return n;
}
	    
int StaLta::bestSNR(DetectCandidate *c, int i, int j)
{
    int k, best;
    char sta[8];
    double SNR;

    SNR = c[i].snr;
    strcpy(sta, c[i].sta);
    best = i;
	
    for(k=i+1; k<(i+j+1); k++)
    {
	if (c[k].snr > SNR && !strcmp(sta, c[k].sta))
	{
	    SNR = c[k].snr;
	    best = k;
	}
    }

    return best;
}

void StaLta::compareWithArrivals(cvector<CssArrivalClass> &arrivals,
		StaLtaDef *s, DetectCandidate *c, int nC)
{
    int	i, j;
    double minTime, maxTime;
    bool foundLargerSNR;

    if(nC == 0) return;

    if(arrivals.size() <= 0)
    {
	for (i=0; i<nC; i++)
	{
	    if (c[i].state == 1) c[i].state = 3;
	}
	return;
    }

    for (i=0; i<nC; i++)
    {
	minTime = c[i].time - s[c[i].bandIndex].trgsepSec;
	maxTime = c[i].time + s[c[i].bandIndex].trgsepSec;
	if (c[i].state == 1) 
	{
	    foundLargerSNR = False;
            for(j = 0; j < arrivals.size(); j++)
            {
            	CssArrivalClass *a = arrivals[j];

		if (a->time >= minTime && a->time <= maxTime
		     && !strcmp(a->sta, c[i].sta))
		{
		    if (!foundLargerSNR && c[i].snr > a->snr)
		    {
			/* record the arid of the arrival which will be replaced
			   with a higher SNR arrival
			*/
			c[i].state = 2;
			c[i].arid = a->arid;
		    }
		    else if (c[i].snr <= a->snr)
		    {
			foundLargerSNR = True;
		    }
		}
	    }

	    if (c[i].state == 1 && !foundLargerSNR) c[i].state = 3;
	}
    }
}

static int
sort_by_net(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->net(), (*b)->net())) ) return(cnd);

    return(sort_by_sta(a, b));
}

static int
sort_by_sta(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->sta(), (*b)->sta())) ) return(cnd);

    return(sort_by_chan(a, b));
}

static int
sort_by_chan(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->chan(), (*b)->chan())) ) return(cnd);

    return(0);
}

static int
sort_can_by_time(const void *A, const void *B)
{
    DetectCandidate *a = (DetectCandidate *)A;
    DetectCandidate *b = (DetectCandidate *)B;

    if (a->time > b->time)
    {
	return 1;
    }
    else if (a->time < b->time)
    {
	return -1;
    }
    return 0;
}

GTimeSeries * StaLta::returnTS(char * group, gvector<Waveform *> &wvec,
			 int pos, int *nUsed, bool *restoreNeeded)
{
    vector<double> t_lag;
    GTimeSeries *ts;
    bool coherent= false;
    gvector<Waveform *> ws;

    *nUsed = 1;
    *restoreNeeded = false;

    if (!strcmp(group, "vertical"))
    {
	// return the vertical element
	*restoreNeeded = true;
	return wvec[pos]->ts;
    }
    else if (!strcmp(group, "horizontal"))
    {
	// find the horizontal components
	if(wvec[pos] == wvec[pos]->c[0] && wvec[pos+1] == wvec[pos+1]->c[1])
	{
	    *nUsed = 2;
	    // form a new time series based on the sum of the fabs (horizontals)
	    t_lag.push_back(0.);
	    t_lag.push_back(0.);
	    ws.push_back(wvec[pos]);
	    ws.push_back(wvec[pos+1]);
	    ts = Beam::BeamTimeSeries(ws, t_lag, coherent);
	    return ts;
	}
    }

    return NULL;
}

void StaLta::restoreWaveform(Waveform *wvec,
			gvector<DataMethod *> *orig_methods)
{
    if((int)orig_methods->size() == 2)  { // change back to the original filter.
	DataMethod *dm[2];
	dm[0] = orig_methods->at(0);
	dm[1] = orig_methods->at(1);
	DataMethod::changeMethods(2, dm, wvec);
    }
    else { // original data was not filtered
	const char *method_types[] = {"Demean", "TaperData", "IIRFilter"};
	DataMethod::remove(3, method_types, wvec);
    }
}
