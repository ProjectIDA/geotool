/** \file WaveformPlot.cpp
 *  \brief Defines class WaveformPlot.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "WaveformPlot.h"
#include "Amp.h"
#include "FKData.h"
#include "Waveform.h"
#include "BasicSource.h"
#include "CalibData.h"
#include "TaperData.h"
#include "Demean.h"
#include "Beam.h"
#include "Response.h"
#include "ArrivalParams.h"
#include "AmplitudeParams.h"
#include "gobject++/cvector.h"

extern "C" {
#include "libgmath.h"
}

using namespace std;


class UndoDeleteArrival : public UndoAction
{
    public:
	UndoDeleteArrival(WaveformPlot *w) {
	    wplot = w;
	    error_msg = NULL; // use showErrorMsg instead of this.
        }
	~UndoDeleteArrival() {
	}
	WaveformPlot *wplot;
	cvector <CssArrivalClass> arrivals;
	cvector <CssAssocClass> assocs;
	cvector <CssAmplitudeClass> amps;
	cvector <CssStamagClass> stamags;
	cvector <CssHydroFeaturesClass> hydros;
	cvector <CssInfraFeaturesClass> infras;

	bool undo(void) {
	    return wplot->undoDeleteArrival(this);
	}
	void getLabel(string &label) {
	    if(arrivals.size() > 1) {
		label.assign("Undo Delete Arrivals");
	    }
	    else label.assign("Undo Delete Arrival");
	}
	bool errMsg(string &label) {
	    if(error_msg) { label.assign(error_msg); return true; }
	    else { label.clear(); return false; }
	}
    protected:
	char *error_msg;
};

static bool isPPhase(char *phase);
static void scaleWaveforms(int npts, float *z, float *n, float *e);
static bool getFilter(Waveform *w, int *order, double *flo, double *fhi,
		bool *zp);

static ArrivalParams *arrival_params = NULL;
static AmplitudeParams *amplitude_params = NULL;

ArrivalParams *WaveformPlot::arrivalParams(void)
{
    if( !arrival_params ) {
	vector<Component *> *w = Application::getApplication()->getWindows();
	arrival_params = new ArrivalParams("Arrival Parameters", w->at(0));
    }
    return arrival_params;
}

AmplitudeParams *WaveformPlot::amplitudeParams(void)
{
    if( !amplitude_params ) {
	vector<Component *> *w = Application::getApplication()->getWindows();
	amplitude_params = new AmplitudeParams("Amplitude Parameters",w->at(0));
    }
    amplitude_params->doCallbacks(amplitude_params, NULL,
			"amplitudeParametersCallback");
    return amplitude_params;
}

void WaveformPlot::showArrivalParams(void)
{
    if( !arrival_params ) {
	vector<Component *> *w = Application::getApplication()->getWindows();
	arrival_params = new ArrivalParams("Arrival Parameters", w->at(0));
    }
    arrival_params->setVisible(true);
}

void WaveformPlot::showAmplitudeParams(void)
{
    if( !amplitude_params ) {
	vector<Component *> *w = Application::getApplication()->getWindows();
	amplitude_params = new AmplitudeParams("Amplitude Parameters",w->at(0));
    }
    amplitude_params->setVisible(true);
}

bool WaveformPlot::deleteArrivals(cvector<CssArrivalClass> &arrivals)
{
    int i, j, dc;
    bool found_one = false;
    DataSource *ds;
    cvector<CssAssocClass> assocs;
    cvector<CssAmplitudeClass> amps;
    cvector<CssStamagClass> stamags;
    cvector<CssHydroFeaturesClass> hydros;
    cvector<CssInfraFeaturesClass> infras;

    getTable(assocs);
    getTable(amps);
    getTable(stamags);
    getTable(hydros);
    getTable(infras);

    UndoDeleteArrival *undo = new UndoDeleteArrival(this);

    errorMsg(); // clear last error message
    bool err = false;

    for(i = 0; i < arrivals.size(); i++)
	if( (ds = arrivals[i]->getDataSource()) )
    {
	BasicSource::startBackup();

	CssArrivalClass *a = arrivals[i];
	undo->arrivals.push_back(a);
	removeTable(a);

	dc = a->getDC();

	for(j = 0; j < assocs.size(); j++)
	    if(assocs[j]->getDC() == dc && assocs[j]->arid == a->arid)
	{
	    undo->assocs.push_back(assocs[j]);
	    removeTable(assocs[j], false);
	}
	for(j = 0; j < amps.size(); j++)
	    if(amps[j]->getDC() == dc && amps[j]->arid == a->arid)
	{
	    undo->amps.push_back(amps[j]);
	    removeTable(amps[j], false);
	}
	for(j = 0; j < stamags.size(); j++)
	    if(stamags[j]->getDC() == dc && stamags[j]->arid == a->arid)
	{
	    undo->stamags.push_back(stamags[j]);
	    removeTable(stamags[j], false);
	}
	for(j = 0; j < hydros.size(); j++)
	    if(hydros[j]->getDC() == dc && hydros[j]->arid == a->arid)
	{
	    undo->hydros.push_back(hydros[j]);
	    removeTable(hydros[j], false);
	}
	for(j = 0; j < infras.size(); j++)
	    if(infras[j]->getDC() == dc && infras[j]->arid == a->arid)
	{
	    undo->infras.push_back(infras[j]);
	    removeTable(infras[j], false);
	}

	if(!ds->deleteArrival(this, a, assocs, amps, stamags, hydros, infras))
	{
	    err = true;
	    showErrorMsg();
	}
	else found_one = true;
    }

    if(found_one) {
	Application::getApplication()->addUndoAction(undo);
    }
    else {
	delete undo;
	if( !err ) showWarning("No arrival selected.");
    }
    return true;
}

bool WaveformPlot::undoDeleteArrival(UndoDeleteArrival *undo)
{
    int i, j, n, dc;
    DataSource *ds;

    n = 0;
    for(i = undo->arrivals.size()-1; i >= 0; i--)
	if( (ds = undo->arrivals[i]->getDataSource())
		&& ds->undoDeleteTable(undo->arrivals[i]))
    {
	CssArrivalClass *a = undo->arrivals[i];
	dc = a->getDC();

	putTable(a);

	for(j = 0; j < undo->assocs.size(); j++)
	    if(undo->assocs[j]->getDC() == dc && undo->assocs[j]->arid == a->arid)
	{
	    putTable(undo->assocs[j]);
	}
	for(j = 0; j < undo->amps.size(); j++)
	    if(undo->amps[j]->getDC() == dc && undo->amps[j]->arid == a->arid)
	{
	    putTable(undo->amps[j]);
	}
	for(j = 0; j < undo->stamags.size(); j++)
	    if(undo->stamags[j]->getDC() == dc && undo->stamags[j]->arid ==a->arid)
	{
	    putTable(undo->stamags[j]);
	}
	for(j = 0; j < undo->hydros.size(); j++)
	    if(undo->hydros[j]->getDC() == dc && undo->hydros[j]->arid == a->arid)
	{
	    putTable(undo->hydros[j]);
	}
	for(j = 0; j < undo->infras.size(); j++)
	    if(undo->infras[j]->getDC() == dc && undo->infras[j]->arid == a->arid)
	{
	    putTable(undo->infras[j]);
	}
	n++;
    }
    if( n != undo->arrivals.size() ) {
	showWarning("Error encountered while undoing delete arrival.");
    }
    if(n > 0) {
//	list();
    }
    return (n == undo->arrivals.size()) ? true : false;
}

void WaveformPlot::initArrivalKeys(void)
{
    ArrivalKey a;
    char *prop;

    if((prop = getProperty("arrival_keys")))
    {
	char *tok, *last, *name, *key;

	tok = prop;
	while( (name = strtok_r(tok, " ,\t", &last)) ) {
	    tok = NULL;
	    if( (key = strtok_r(NULL, " ,\t", &last)) ) {
		a.name.assign(name);
		a.key.assign(key);
		arrival_keys.push_back(a);
	    }
	}
	Free(prop);
    }
    else {
	a.name.assign("P");
	a.key.assign("p");
	arrival_keys.push_back(a);
	a.name.assign("Pn");
	a.key.assign("n");
	arrival_keys.push_back(a);
	a.name.assign("Pg");
	a.key.assign("g");
	arrival_keys.push_back(a);
	a.name.assign("S");
	a.key.assign("s");
	arrival_keys.push_back(a);
	a.name.assign("Lg");
	a.key.assign("a");
	arrival_keys.push_back(a);
    }
    setCPlotArrivalKeys(arrival_keys);
}

CssArrivalClass *WaveformPlot::addArrivalFromKey(CPlotArrivalCallbackStruct *p)
{
    CssArrivalClass *arrival;
    double deltim = -1., delaz = -1., delslo = -1.;
    double azimuth = -1., slow = -1., ema = -1., rect = -1., snr = -1.;
    Password password = getpwuid(getuid());
    bool add_to_net = !p->shift;

    arrival = makeArrival(p->w, password, p->time, p->name, deltim, azimuth,
			delaz, slow, delslo, ema, rect, snr, add_to_net);
    if(arrival) {
	putArrivalWithColor(arrival, stringToPixel("black"));
    }
    return arrival;
}

CssArrivalClass *WaveformPlot::makeArrival(Waveform *w, Password password,
		double time, const string &phase, double deltim, double azimuth,
		double delaz, double slow, double delslo, double ema,
		double rect, double snr, bool add_to_net)
{
    CssArrivalClass *arrival;
    CssOriginClass *origin;
    CssAmplitudeClass *amp_mb=NULL, *amp_ml=NULL;
    GTimeSeries *ts;
    GSourceInfo *s=NULL;
    GSegment *seg;
    DateTime dt;
    double fmin, fmax;
    Application *app = Application::getApplication();
    gvector<Waveform *> wvec, v;
    bool array_chan;

    DataSource *ds = w->getDataSource();
    if(!ds) {
	showWarning("Cannot add arrival. Null data source.");
	return (CssArrivalClass *)NULL;
    }
    ts = w->ts;

    if( !(seg = ts->nearestSegment(time)) ) {
	showWarning("Cannot get segment for time. Cannot add arrival.");
	return (CssArrivalClass *)NULL;
    }
    s = &ts->source_info;

    if(!s) {
	showWarning("No source information. Cannot add arrival.");
	return (CssArrivalClass *)NULL;
    }

    arrival = new CssArrivalClass();
    s->copySource(arrival);

    if(add_to_net) {
	stringcpy(arrival->sta, w->net(), sizeof(arrival->sta));
    }
    else {
	stringcpy(arrival->sta, w->sta(), sizeof(arrival->sta));
    }
    ds->getChanid(w->sta(), w->chan(), &arrival->chanid);

    stringcpy(arrival->chan, w->chan(), sizeof(arrival->chan));
    arrival->time = time;
    stringcpy(arrival->iphase, phase.c_str(), sizeof(arrival->iphase));
    stringcpy(arrival->phase, phase.c_str(), sizeof(arrival->phase));
    arrival->deltim = deltim;
    arrival->azimuth = azimuth;
    if(arrival->azimuth != -1. && arrival->azimuth < 0.) {
	arrival->azimuth += 360.;
    }
    arrival->delaz = delaz;
    arrival->slow = slow;
    arrival->delslo = delslo;
    arrival->ema = ema;
    arrival->rect = rect;
    arrival->snr = snr;
    arrival->amp_cnts = -1.;
    arrival->amp_Nnms = -1.;
    arrival->amp_nms = -1.;
    arrival->zp_Nnms = -1.;
    arrival->period = -1.;
    arrival->box_location = False;
    arrival->boxtime = 0.;
    arrival->boxmin = 0.;

    timeEpochToDate(arrival->time, &dt);
    arrival->jdate = timeJDate(&dt);

    setCursor("hourglass");

    app->putParseProperty("ma_net", w->sta());
    app->putParseProperty("ma_sta", arrival->sta);
    app->putParseProperty("ma_chan", arrival->chan);
    app->putParseProperty("ma_phase", arrival->phase);
    doCallbacks(this, NULL, "measureArrivalCallback");

    array_chan = false;
    // Azimuth and slowness already by FK
    if(!w->channel.compare(0, 2, "fk")) {
	getSnr(w->ts, arrival->time, &arrival->snr, &arrival->deltim);
    }
    else if(add_to_net && BasicSource::isArrayChannel(
		w->net(), w->sta(), w->chan()))
    {
	if(getArrayElements(w, arrival->time, wvec, false) >= 3) {
	    array_chan = true;
	}
    }
    if(array_chan) {
	stringcpy(arrival->chan, arrivalParams()->arrival_beam_recipe.c_str(),
		    sizeof(arrival->chan));

	getArrayStats(wvec, arrival, &fmin, &fmax);

	TimeParams tp = getTimeParams();

	double tbeg = arrival->time - tp.ltav_len - 1.;
	double tend = arrival->time + tp.stav_len + 1.;
	int order;
	double flo, fhi;
	bool zp;
	if( !getFilter(w, &order, &flo, &fhi, &zp) ) {
	    flo = fmin;
	    fhi = fmax;
	}
	GTimeSeries *beam = makeBeam(wvec, arrival->azimuth, arrival->slow,
				tbeg, tend, order, flo, fhi, zp, true);
	if(beam) {
	    beam->addOwner(this);
	    getSnr(beam, arrival->time, &arrival->snr, &arrival->deltim);
	    beam->removeOwner(this);
	}
    }

    if(arrival->snr < 0.) {
	getSnr(w->ts, arrival->time, &arrival->snr, &arrival->deltim);
	printLog("%x %s/%s measured snr=%.3lf deltim=%.3lf\n", w->ts,
		w->sta(), w->chan(), arrival->snr, arrival->deltim);
    }

    if( !array_chan && getThreeComponents(w, wvec) )
    {
	if(arrival->azimuth== -1. || arrival->ema== -1. || arrival->rect== -1.)
	{
	    double tmax;
	    getThreeCompStats(w, wvec, arrival->time, arrival, &tmax);
	}
	else {
	    double rad = M_PI/180.;
	    double incidence = arrival->ema;

	    if(arrival->slow == -1.) {
		arrival->slow = arrivalParams()->polar_alpha *
					sin(rad*incidence/2.) * DEG_TO_KM;
	    }
	    if(arrival->delslo == -1.) {
		double dk = arrivalParams()->polar_dk;
		arrival->delslo = sqrt(0.5*dk*dk*(1.-arrival->rect))*DEG_TO_KM;
	    }
	    if(arrival->delaz == -1.) {
		arrival->delaz= 2.*asin(arrival->delslo/(2.*arrival->slow))/rad;
	    }
	}
    }

    TimeParams tp = getTimeParams();
    if(arrival->snr > 0. && arrival->deltim < 0.) {
	arrival->deltim = tp.max_deltim - tp.deltim_diff*
			log10(arrival->snr/tp.min_snr)/
			log10(tp.max_snr/tp.min_snr);
    }
    if(tp.min_deltim > arrival->deltim) arrival->deltim = tp.min_deltim;

    // get max arid in case lastid file cannot be accessed
    cvector<CssArrivalClass> *a = getArrivalRef();
    long max_arid = 0;
    for(int i = 0; i < a->size(); i++) if(max_arid < a->at(i)->arid) max_arid = a->at(i)->arid;

    if( !ds->addArrival(arrival, ts, password, max_arid) )
    {
	showWarning("Cannot add arrival.");
	delete arrival;
	setCursor("default");
	return NULL;
    }

    addTableToAll(arrival, this);

    if((origin = getPrimaryOrigin(w))) {
	associateWithOrigin(ds, arrival, origin, w->lat(), w->lon());
    }

    AmplitudeParams *ap = amplitudeParams();
    if(ap->auto_measure && w->channel.compare(0, 2, "fk"))
    {
	if(array_chan) {
	    measureAmplitudes(wvec, arrival, &amp_mb, &amp_ml);
	}
	else {
	    v.push_back(w);
	    measureAmplitudes(v, arrival, &amp_mb, &amp_ml);

	    if(amp_mb) {
		strcpy(amp_mb->inarrival, "y");

		CssArrivalClass *new_arr = new CssArrivalClass(*arrival);
		new_arr->amp = amp_mb->amp;
		new_arr->period = amp_mb->per;

		ds->changeTable(arrival, new_arr);

		delete new_arr;

		arrival->amp = amp_mb->amp;
		arrival->period = amp_mb->per;
		arrival->amp_cnts = amp_mb->amp_cnts;
		arrival->amp_Nnms = amp_mb->amp_Nnms;
		arrival->amp_nms = amp_mb->amp;
		arrival->zp_Nnms = -1.;
		arrival->box_location = false;
		arrival->boxtime = amp_mb->amptime;
		arrival->boxmin = 0.;
	    }
	}

	if(amp_mb) {
	    amp_mb->snr = arrival->snr;
	    saveAmp(ds, arrival, amp_mb);
	}
	if(amp_ml) saveAmp(ds, arrival, amp_ml);
    }
    else if(ap->auto_measure && !w->channel.compare(0, 2, "fk")) {
	v.push_back(w);
	measureAmplitudes(v, arrival, &amp_mb, &amp_ml);
	if(amp_mb) {
	    strcpy(amp_mb->inarrival, "y");

	    CssArrivalClass *new_arr = new CssArrivalClass(*arrival);
	    new_arr->amp = amp_mb->amp;
	    new_arr->period = amp_mb->per;

	    ds->changeTable(arrival, new_arr);

	    delete new_arr;

	    arrival->amp = amp_mb->amp;
	    arrival->period = amp_mb->per;
	    arrival->amp_cnts = amp_mb->amp_cnts;
	    arrival->amp_Nnms = amp_mb->amp_Nnms;
	    arrival->amp_nms = amp_mb->amp;
	    arrival->zp_Nnms = -1.;
	    arrival->box_location = false;
	    arrival->boxtime = amp_mb->amptime;
	    arrival->boxmin = 0.;

	    amp_mb->snr = arrival->snr;
	    saveAmp(ds, arrival, amp_mb);
	}
	if(amp_ml) saveAmp(ds, arrival, amp_ml);
    }

    setCursor("default");

    return arrival;
}

static bool
getFilter(Waveform *w, int *order, double *flo, double *fhi, bool *zp)
{
    bool ret = false;
    gvector<DataMethod *> *methods = w->dataMethods();
    *order = -1;
    *flo = 0.;
    *fhi = 0.;
    *zp = false;

    for(int i = 0; i < (int)methods->size(); i++) {
	IIRFilter *filter = methods->at(i)->getIIRFilterInstance();
	if(filter) {
	    *order = filter->getOrder();
	    *flo = filter->getFlow();
	    *fhi = filter->getFhigh();
	    *zp = filter->getZeroPhase();
	    ret = true;
	    break;
	}
    }
    delete methods;
    return ret;
}

bool WaveformPlot::getThreeComponents(Waveform *w, gvector<Waveform *> &wvec)
{
    int i, n, ngroups;
    vector<int> ncmpts;
    gvector<Waveform *> ws;

    ngroups = getComponents(ncmpts, ws);

    n = 0;
    for(i = 0; i < ngroups; i++, n += ncmpts[i]) if(ncmpts[i] == 3)
    {
	if(ws[n]->ts == w->ts || ws[n+1]->ts == w->ts || ws[n+2]->ts == w->ts)
	{
	    wvec.push_back(ws[n]);
	    wvec.push_back(ws[n+1]);
	    wvec.push_back(ws[n+2]);
	    break;
	}
    }
    return (i < ngroups) ? true : false;
}

bool WaveformPlot::beamArrival(CssArrivalClass *arrival, bool selected_only,
			bool replace)
{
    int nsta;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    vector<double> t_lags, weights;
    double beam_lat, beam_lon;
    BeamLocation beam_location = DNORTH_DEAST;
    gvector<Waveform *> ws, wvec;

    if(!arrival) return false;

    setCursor("hourglass");

    if( !gbeam->beamRecipe(arrival->sta, arrival->chan, recipe) ) {
	showWarning("No beam recipe found.");
	setCursor("default");
	return false;
    }
    if((nsta = gbeam->getGroup(recipe, beam_sta)) <= 0) {
	showWarning(GError::getMessage());
	setCursor("default");
	return false;
    }

    if(!selected_only) {
	getWaveforms(ws, false);
    }
    else {
	getSelectedWaveforms(ws);
    }

    for(int j = 0; j < nsta; j++)
    {
	for(int i = 0; i < (int)ws.size(); i++)
	{
	    if( !strcasecmp(ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[j].chan) &&
		    ws[i]->tbeg() < arrival->time &&
		    arrival->time < ws[i]->tend())
	    {
		wvec.push_back(ws[i]);
		break;
	    }
	}
    }

    if(wvec.size() < 1) {
	showWarning("No waveform elements found.");
	setCursor("default");
	return false;
    }

    if( !Beam::getTimeLags(this, wvec, recipe.azi, recipe.slow,
			beam_location, t_lags, &beam_lat, &beam_lon) )
    {
	showWarning(GError::getMessage());
	setCursor("default");
	return false;
    }

    IIRFilter *iir = NULL;
    if( strcasecmp(recipe.ftype.c_str(), "NA") ) {
        iir = new IIRFilter(recipe.ford, recipe.ftype, recipe.flo,
		recipe.fhi, wvec[0]->segment(0)->tdel(), recipe.zp);
	iir->addOwner(this);
    }

    for(int i = 0; i < wvec.size(); i++) weights.push_back(1.);

    computeBeam(wvec, t_lags, arrival->chan, beam_lat, beam_lon, iir,
		weights, replace, false, this);

    if(iir) iir->removeOwner(this);

    setCursor("default");

    return true;
}

bool WaveformPlot::measureAmps(CssArrivalClass *arrival, Waveform *cd,
			AmpMeasureMode mode)
{
    bool is_array;
    int i, num, nsta;
    DataSource *ds;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    gvector<Waveform *> ws, v;
    CssAmplitudeClass *amp_mb=NULL, *amp_ml=NULL;

    if( !arrival || !(ds = arrival->getDataSource()) ) {
	return false;
    }

    // get the waveform

    num = getWaveforms(ws);

    for(i = 0; i < num; i++) {
	if( (!strcasecmp(ws[i]->sta(), arrival->sta) ||
	     !strcasecmp(ws[i]->net(), arrival->sta)) &&
	    DataSource::compareChan(ws[i]->chan(), arrival->chan) &&
	    ws[i]->segment(arrival->time) ) break;
    }
    if(i < num) { // found a waveform with sta/chan same as arrival sta/chan
	v.push_back(ws[i]);
	measureAmplitudes(v, arrival, &amp_mb, &amp_ml, mode);
	if(amp_mb) saveAmp(ds, arrival, amp_mb);
	if(amp_ml) saveAmp(ds, arrival, amp_ml);
	return true;
    }

    is_array = BasicSource::isArray(arrival->sta);
    if(is_array) {
	// find any waveform from the network
	for(i = 0; i < num; i++) {
	    if((!strcmp(ws[i]->net(), arrival->sta) ||
		!strcmp(ws[i]->sta(), arrival->sta)) &&
		ws[i]->segment(arrival->time) ) break;
	}
    }
    else if( gbeam->beamRecipe(arrival->sta, arrival->chan, recipe) &&
		(nsta = gbeam->getGroup(recipe, beam_sta)) == 1)
    {
	for(i = 0; i < num; i++) {
	    if( (!strcasecmp(ws[i]->sta(), arrival->sta) ||
		!strcasecmp(ws[i]->net(), arrival->sta)) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[0].chan) &&
		ws[i]->segment(arrival->time) ) break;
	}
    }

    if(i == num) {
	putWarning("Cannot find waveform for %s/%s",arrival->sta,arrival->chan);
	printLog("No waveforms found for arrival %s/%s arid=%d\n",
		arrival->sta, arrival->chan, arrival->arid);
	if(cd) {
	    // make measurement on selected waveform
	    v.clear(); v.push_back(cd);
	    measureAmplitudes(v, arrival, &amp_mb, &amp_ml, mode);
	    if(amp_mb) saveAmp(ds, arrival, amp_mb);
	    if(amp_ml) saveAmp(ds, arrival, amp_ml);
	    return true;
	}
	else {
	    return false;
	}
    }
    if( is_array )
    {
	gvector<Waveform *> wvec;
	if( (num = getArrayElements(ws[i], arrival->time, wvec, false)) > 1)
	{
	    measureAmplitudes(wvec, arrival, &amp_mb, &amp_ml, mode);
	}
	else {
	    putWarning("No array elements. Cannot compute array stats.");
	    printLog("No array elements. Cannot compute array stats.");

	    // make measurement on selected waveform
	    if(cd) {
		v.clear(); v.push_back(cd);
		measureAmplitudes(v, arrival, &amp_mb, &amp_ml, mode);
	    }
	    else {
		return false;
	    }
	}
    }
    else {
	v.clear(); v.push_back(ws[i]);
	measureAmplitudes(v, arrival, &amp_mb, &amp_ml, mode);
    }

    if(amp_mb) saveAmp(ds, arrival, amp_mb);
    if(amp_ml) saveAmp(ds, arrival, amp_ml);

    return true;
}

void WaveformPlot::getSnr(GTimeSeries *ts, double time, double *snr,
			double *deltim)
{
    TimeParams tp;
    double stav, ltav;
    GDataPoint *d1, *d2;
    int n;

    *snr = -1.;
    *deltim = -1.;

    tp = getTimeParams();

    d1 = ts->upperBound(time - tp.ltav_len);
    d2 = ts->lowerBound(time);
    ltav = 0.;
    n = 0;
    for(int j = d1->segmentIndex(); j <= d2->segmentIndex(); j++) {
	GSegment *s = ts->segment(j);
	int i1 = (j == d1->segmentIndex()) ? d1->index() : 0;
	int i2 = (j < d2->segmentIndex()) ? s->length()-1 : d2->index();
	for(int i = i1; i <= i2; i++) {
	    ltav += fabs(s->data[i]);
	    n++;
	}
    }
    if(n) ltav /= n;
    delete d1;
    delete d2;

    d1 = ts->upperBound(time);
    d2 = ts->lowerBound(time + tp.stav_len);
    stav = 0.;
    n = 0;
    for(int j = d1->segmentIndex(); j <= d2->segmentIndex(); j++) {
	GSegment *s = ts->segment(j);
	int i1 = (j == d1->segmentIndex()) ? d1->index() : 0;
	int i2 = (j < d2->segmentIndex()) ? s->length()-1 : d2->index();
	for(int i = i1; i <= i2; i++) {
	    stav += fabs(s->data[i]);
	    n++;
	}
    }
    if(n) stav /= n;
    delete d1;
    delete d2;

    if(ltav != 0.) {
	*snr = stav/ltav;

	*deltim = tp.max_deltim - tp.deltim_diff * log10(*snr/tp.min_snr)/
				log10(tp.max_snr/tp.min_snr);
	if(tp.min_deltim > *deltim) *deltim = tp.min_deltim;
    }
}

void WaveformPlot::getArrayStats(gvector<Waveform *> &wvec, CssArrivalClass *arrival,
			 double *fmin, double *fmax)
{
    double fk_lead, fk_lag, fk_dk, bandw;
    FKArgs args;
    FKData *fk = NULL;

    getFkParams(&fk_lead, &fk_lag, &fk_dk, &args, &bandw);
    *fmin = args.fmin[0];
    *fmax = args.fmax[0];

    double tstart = arrival->time - fk_lead;
    double tend = arrival->time + fk_lag;
    double slowness, delaz, cfreq;

    args.num_slowness = 81;
    args.slowness_max = 0.36;
    if(args.slowness_max < args.signal_slow_max) {
	args.slowness_max = 1.1*args.signal_slow_max;
    }

    try {
	fk = new FKData(wvec, tstart, tend, args);
    }
    catch(...) {
	cerr << GError::getMessage();
	if(fk == NULL) return;
    }

    if(args.full_compute) {
	fk->searchFBands(bandw, args.fmin[0], args.fmax[0],args.signal_slow_min,
		args.signal_slow_max, args.signal_az_min, args.signal_az_max);
    }

    slowness = sqrt(fk->xmax[0]*fk->xmax[0] + fk->ymax[0]*fk->ymax[0]);
    if(slowness == 0.) slowness = 1.e-06;
    slowness *= DEG_TO_KM;
    arrival->azimuth = atan2(fk->xmax[0], fk->ymax[0]);
    arrival->azimuth *= (180./M_PI);
    if(arrival->azimuth < 0.) arrival->azimuth += 360;
    cfreq = .5*(args.fmin[0] + args.fmax[0]);
    arrival->delslo = fk_dk/(sqrt(fk->fstat[0])*cfreq);
    arrival->delslo *= DEG_TO_KM;
    delaz = arrival->delslo/(2.*slowness);
    if(delaz < 1.0) {
	delaz = 2.*asin(delaz)*180./M_PI;
    }
    else {
	delaz = 180;
    }
    arrival->slow = slowness;
    arrival->delaz = delaz;

    delete fk;
}

int WaveformPlot::getArrayElements(Waveform *cd, double time,
			gvector<Waveform *> &cdlist, bool show_warning)
{
    Waveform **wvec=NULL;
    gvector<Waveform *> ws, w_sel;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    int i, j, k, n, n_all, nsta, num, num_selected;
    ArrivalParams *ap = arrivalParams();
    DataSource *ds = cd->getDataSource();

    cdlist.clear();

    if( !gbeam->beamRecipe(cd->net(),ap->arrival_beam_recipe.c_str(),recipe) ) {
	showWarning("No origin '%s' recipe for network %s",
		ap->arrival_beam_recipe.c_str(), cd->net());
	return 0;
    }
    if((nsta = gbeam->getGroup(recipe, beam_sta)) <= 0)
    {
	if(show_warning) {
	    showWarning(GError::getMessage());
	}
	return 0;
    }

    if( cd->ts->array_elements.size() > 0 )
    {
	if( !cd->ts->getValue("elements_loaded") )
	{
	    inputData(&cd->ts->array_elements, ds, false);
	    cd->ts->putValue("elements_loaded", true);
	}
	n_all = getWaveforms(ws, false);
    }
    else if( (n_all = ds->getBeamElements(cd->sta(), cd->chan(), ws)) <= 0) {
	n_all = getWaveforms(ws, false);
    }

    if(!(wvec = (Waveform **)mallocWarn(
		nsta*sizeof(Waveform *)))) return 0;

    printLog("Getting beam elements for %s/cb\n", cd->net());
    num = 0;
    for(j = 0; j < nsta; j++)
    {
	for(i = 0; i < n_all; i++)
	{
	    if( !strcasecmp(ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[j].chan) &&
		    ws[i]->tbeg() < time && time < ws[i]->tend())
	    {
		wvec[num++] = ws[i];
		break;
	    }
	}
    }

    // restrict the elements by selection
    num_selected = getSelectedWaveforms(w_sel);

    n = 0;
    if(num_selected > 0) {
	for(i = 0; i < num; i++) {
	    for(k = 0; k < num_selected && w_sel[k] != wvec[i]; k++);
	    if(k  < num_selected) n++;
	}
    }
    if(n > 1) {
	printLog("Elements restricted by waveform selection.\n");
    }
    for(j = 0; j < nsta; j++) {
	for(i = 0; i < num; i++) if(wvec[i])
	{
	    if( !strcasecmp(wvec[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(wvec[i]->chan(),beam_sta[j].chan)) {
			break;
	    }
	}
	if(i < num) {
	    if(n > 1) {
		for(k = 0; k < num_selected && w_sel[k] != wvec[i]; k++);
		if(k < num_selected) {
		    printLog("%x %s/%s selected\n", wvec[i]->ts,
			beam_sta[j].sta, beam_sta[j].chan);
		}
		else {
		    printLog("%s/%s --- not selected\n", wvec[i]->ts,
			beam_sta[j].sta, beam_sta[j].chan);
		    wvec[i] = NULL; // not selected
		}
	    }
	    else {
		printLog("%x %s/%s found\n", wvec[i]->ts, beam_sta[j].sta,
			beam_sta[j].chan);
	    }
	}
	else {
	    printLog("%s/%s --- not found\n", beam_sta[j].sta,beam_sta[j].chan);
	}
    }
    if(n > 1) {
	// if restricted by selection, drop waveforms that are not selected 
	n = 0;
	for(i = 0; i < num; i++) if(wvec[i]) {
	    wvec[n++] = wvec[i];
	}
	num = n;
    }

    for(i = 0; i < num; i++) cdlist.push_back(wvec[i]);
    Free(wvec);
    return num;
}

void WaveformPlot::getThreeCompStats(Waveform *cd, gvector<Waveform *> &wvec,
				double time, CssArrivalClass *arrival, double *tmax)
{
    int i, k, kmax=0;
    GTimeSeries *ts[3];
    DataMethod *dm = NULL;
    TaperData *taper = NULL;
    IIRFilter *filter = NULL;
    Demean *dmean;
    ArrivalParams *ap = arrivalParams();

    for(i = 0; i < 3; i++) ts[i] = new GTimeSeries(wvec[i]->ts);

    if(wvec[2]->selected) cd = wvec[2]; // z-component

    if((dm = cd->ts->getMethod("IIRFilter")) == NULL) {
	GSegment *s = cd->segment(0);
	filter = new IIRFilter(ap->polar_order, ap->polar_filter_type,
			ap->polar_lofreq, ap->polar_hifreq, s->tdel(),
			ap->polar_zp);
	taper = new TaperData("cosine", (int)(ap->polar_taper_frac*100.),
				5, 200);
	dmean = new Demean();
	for(i = 0; i < 3; i++) {
	    dmean->apply(ts[i]);
	    taper->apply(ts[i]);
	    filter->apply(ts[i]);
	}
	dmean->deleteObject();
	taper->deleteObject();
	filter->deleteObject();
    }

    double tbeg = time - ap->polar_signal_lead;

    GDataPoint *e1 = ts[0]->upperBound(tbeg);
    GDataPoint *n1 = ts[1]->upperBound(tbeg);
    GDataPoint *z1 = ts[2]->upperBound(tbeg);

    double zdt = z1->segment()->tdel();
    double ndt = n1->segment()->tdel();
    double edt = e1->segment()->tdel();
    if(fabs((ndt-zdt)/zdt) > .01 || fabs((edt-zdt)/zdt) > .01) {
	delete z1; delete n1; delete e1;
	putWarning("Variable sample rates. Cannot compute arrival stats.");
	return;
    }
    double samprate = 1./zdt;

    int npts = (int)(ap->polar_signal_len*samprate+.5) + 1;
    if(npts > z1->segment()->length() - z1->index()) {
	npts = z1->segment()->length() - z1->index();
    }
    if(npts > n1->segment()->length() - n1->index()) {
	npts = n1->segment()->length() - n1->index();
    }
    if(npts > e1->segment()->length() - e1->index()) {
	npts = e1->segment()->length() - e1->index();
    }

    float *z = z1->segment()->data + z1->index();
    float *n = n1->segment()->data + n1->index();
    float *e = e1->segment()->data + e1->index();
 
    int polar_window_width = (int)(ap->polar_window*samprate+.5);
    int window_pts = polar_window_width + 1;

    double s[9], d[3], o[3], v[9], c[3][3];
    double alpha = ts[0]->currentAlpha();
    double beta = ts[0]->currentBeta();
    double gamma = ts[0]->currentGamma();

    bool rotated;
    if(alpha != 0. || beta != 0. || gamma != 0.) {
        alpha *= M_PI/180.;
        beta *= M_PI/180.;
        gamma *= M_PI/180.;
        rotation_matrix(alpha, beta, gamma, c);
        rotated = true;
    }
    else {
        rotated = false;
    }

    scaleWaveforms(npts, z, n, e); // not really needed

    double p_az=0., s_az=0., incidence=0., recti=0., max_recti=0.;
    double rad = M_PI/180.;

    for(k = 0; k + polar_window_width < npts; k++)
    {
	covar(window_pts, e+k, n+k, z+k, s);

	tred2(3, s, d, o, v);
	tql2(3, d, o, v);
	for(i = 0; i < 3; i++) if(d[i] < 0.) d[i] = 0.;

	if(d[2] == 0.) {
	    recti = 0.;
	}
	else {
	    recti = 1. - .5*(d[0] + d[1])/d[2];
	}
	// if the stations Components are not E,N,UP, then find the coordinates
	// of the eignvector v[6],v[7],v[8] in the E,N,UP coordinate system.
	if(rotated) {
	    double vx = c[0][0]*v[6] + c[0][1]*v[7] + c[0][2]*v[8];
	    double vy = c[1][0]*v[6] + c[1][1]*v[7] + c[1][2]*v[8];
	    double vz = c[2][0]*v[6] + c[2][1]*v[7] + c[2][2]*v[8];
	    v[6] = vx;
	    v[7] = vy;
	    v[8] = vz;
	    vx = c[0][0]*v[0] + c[0][1]*v[1] + c[0][2]*v[2];
	    vy = c[1][0]*v[0] + c[1][1]*v[1] + c[1][2]*v[2];
	    vz = c[2][0]*v[0] + c[2][1]*v[1] + c[2][2]*v[2];
	    v[0] = vx;
	    v[1] = vy;
	    v[2] = vz;
	}

	if(recti > 0. && recti > max_recti) {
	    kmax = k;
	    max_recti = recti;
	    i = (v[8] > 0.) ? -1 : 1;
	    p_az = atan2(i*v[6], i*v[7])/rad;
	    i = (v[2] > 0.) ? -1 : 1;
	    s_az = atan2(i*v[0], i*v[1])/rad;
	    incidence = acos(fabs(v[8]))/rad;
	}
    }

    if(max_recti > 0.)  {
	*tmax = z1->segment()->time(z1->index() + kmax);
	if(arrival) {
	    arrival->azimuth = isPPhase(arrival->iphase) ? p_az : s_az;
	    if(arrival->azimuth < 0.) arrival->azimuth += 360;
	    arrival->ema = incidence;
	    arrival->rect = max_recti;
	    arrival->slow = ap->polar_alpha * sin(rad*incidence/2.) * DEG_TO_KM;
	    arrival->delslo = sqrt(0.5*ap->polar_dk*ap->polar_dk*(1.-recti))
				* DEG_TO_KM;
	    arrival->delaz = 2.*asin(arrival->delslo/(2.*arrival->slow))/rad;
	    printLog(
"%x %s/%s: azimuth=%.2lf slow=%.2lf ema=%.2lf rect=%.3lf delslo=.2lf delaz=%.2lf\n",
		cd->ts, cd->sta(), cd->chan(),
		arrival->azimuth, arrival->slow, arrival->ema, arrival->rect,
		arrival->delslo, arrival->delaz);
	}
    }
    else {
	printLog("%x %s/%s: Polarization analysis failed.\n",
		cd->ts, cd->sta(), cd->chan());
    }

    for(i = 0; i < 3; i++) ts[i]->deleteObject();

    delete z1; delete n1; delete e1;
}

static bool
isPPhase(char *phase)
{
    for(int i = (int)strlen(phase)-1; i >= 0; i--) {
	if(phase[i] == 'P' || phase[i] == 'p') {
	    return true;
	}
	else if(phase[i] == 'S' || phase[i] == 's') {
	    return false;
	}
    }
    return true;
}

static void
scaleWaveforms(int npts, float *z, float *n, float *e)
{
    int i;
    double d, scale;

    scale = 0.;
    for(i = 0; i < npts; i++)
    {
	d = fabs(z[i]);
	if(d > scale) scale = d;
	d = fabs(e[i]);
	if(d > scale) scale = d;
	d = fabs(n[i]);
	if(d > scale) scale = d;
    }
    if(scale != 0.) {
	scale = 1./scale;
	for(i = 0; i < npts; i++) {
	    z[i] *= scale;
	    n[i] *= scale;
	    e[i] *= scale;
	}
    }
}

TimeParams WaveformPlot::getTimeParams(void)
{
    TimeParams time_params;
    ArrivalParams *ap = arrivalParams();

    time_params.stav_len = ap->stav_len;
    time_params.ltav_len = ap->ltav_len;
    time_params.min_snr = ap->min_snr;
    time_params.max_snr = ap->max_snr;
    time_params.min_deltim = ap->min_deltim;
    time_params.max_deltim = ap->max_deltim;
    time_params.deltim_diff = time_params.max_deltim - time_params.min_deltim;
    return time_params;
}

void WaveformPlot::getFkParams(double *fk_lead, double *fk_lag, double *fk_dk,
		FKArgs *args, double *bandw)
{
    ArrivalParams *ap = arrivalParams();

    *fk_lead = ap->fk_lead;
    *fk_lag = ap->fk_lag;
    *fk_dk = ap->fk_dk;

    if(args) {
	args->signal_slow_min = ap->signal_slow_min;
	args->signal_slow_max = ap->signal_slow_max;
	args->signal_az_min = ap->signal_az_min;
	args->signal_az_max = ap->signal_az_max;
	args->taper_type = ap->fk_taper_type;
	args->taper_beg = ap->fk_taper_frac;
	args->taper_end = ap->fk_taper_frac;

	args->num_bands = 1;
	if( !ap->search_bands ) {
	    args->full_compute = false;
	    args->fmin[0] = ap->single_fmin;
	    args->fmax[0] = ap->single_fmax;
	}
	else {
	    args->full_compute = true;
	    args->fmin[0] = ap->search_fmin;
	    args->fmax[0] = ap->search_fmax;
	    if(bandw) *bandw = ap->search_bandw;
	}
    }
}

bool WaveformPlot::associateWithOrigin(DataSource *ds, CssArrivalClass *arrival,
			CssOriginClass *origin, double lat, double lon)
{
    CssAssocClass *assoc = new CssAssocClass();
    stringcpy(assoc->timedef, "d", sizeof(assoc->timedef));
    arrival->copySourceTo(assoc, origin->orid);
    assoc->arid = arrival->arid;
    assoc->orid = origin->orid;
    assoc->setIds(arrival->getDC(), arrival->arid);
    stringcpy(assoc->sta, arrival->sta, sizeof(assoc->sta));
    stringcpy(assoc->phase, arrival->iphase, sizeof(assoc->phase));
    deltaz(origin->lat, origin->lon, lat, lon, &assoc->delta, &assoc->esaz,
		&assoc->seaz);

    if(ds->addTable(assoc)) {
	putTable(assoc);
	return true;
    }
    else {
	delete assoc;
	return false;
    }
}

bool WaveformPlot::measureAmplitudes(gvector<Waveform *> &wvec,
		CssArrivalClass *arrival, CssAmplitudeClass **amp_mb,
		CssAmplitudeClass **amp_ml, AmpMeasureMode mode)
{
    int i, time_pos;
    bool measure_mb_amp = false, measure_ml_amp = false;
    double sps;
    GTimeSeries *ts;
    GSegment *segment = NULL;
    cvector<CssAssocClass> assocs;
    CssAssocClass *assoc=NULL;
    CssOriginClass *origin=NULL;
    CssAmplitudeClass *mb_amp=NULL, *ml_amp=NULL;
    AmplitudeParams *ap;
    Application *app = Application::getApplication();

    if(wvec.size() > 0) app->putParseProperty("ma_net", wvec[0]->net());
    app->putParseProperty("ma_sta", arrival->sta);
    app->putParseProperty("ma_chan", arrival->chan);
    app->putParseProperty("ma_phase", arrival->phase);
    doCallbacks(this, NULL, "measureAmplitudeCallback");

    ap = amplitudeParams();

    *amp_mb = NULL;
    *amp_ml = NULL;

    if(wvec.size() <= 0) {
	printLog("measureAmplitudes: num_waveforms = 0\n");
	return false;
    }

    // find assoc for arrival.arid
    getTable(assocs);
    for(i = 0; i < assocs.size() && arrival->arid != assocs[i]->arid; i++);
    if(i < assocs.size()) {
	assoc = assocs[i];
    }

    if(assoc) {
	// find origin for assoc.orid
	cvector<CssOriginClass> origins;
	getTable(origins);
	for(i = 0; i < origins.size() && assoc->orid != origins[i]->orid; i++);
	if(i < origins.size()) {
	    origin = origins[i];
	}
    }

    if(origin)
    {
	// check for acceptable ml phase, distance and depth
	for(i = 0; i < ap->ml_num_phases &&
	    strcmp(ap->ml_phases[i], assoc->phase); i++);
	if(i < ap->ml_num_phases
	    && assoc->delta >= ap->ml_dist_min
	    && assoc->delta <= ap->ml_dist_max
	    && origin->depth >= ap->ml_depth_min
	    && origin->depth <= ap->ml_depth_max)
	{
	    measure_ml_amp = true;
	}
    }

    if(assoc)
    {
	// check for acceptable mb phase and distance
	for(i = 0; i < ap->mb_num_phases &&
	    strcmp(ap->mb_phases[i], assoc->phase); i++);
	if(i < ap->mb_num_phases
	    && assoc->delta >= ap->mb_dist_min
	    && assoc->delta <= ap->mb_dist_max)
	{
	    measure_mb_amp = true;
	}
    }

    if(mode == ML_MEASURE) {
	measure_ml_amp = true;
	measure_mb_amp = false;
    }
    else if(mode == MB_MEASURE) {
	measure_ml_amp = false;
	measure_mb_amp = true;
    }

    if( !measure_ml_amp && !measure_mb_amp ) {
	return false;
    }

    if( !(segment = wvec[0]->segment(arrival->time)) ) {
	return false;
    }

    if( measure_ml_amp ) {
	ml_amp = new CssAmplitudeClass();
	ml_amp->arid = arrival->arid;
	strncpy(ml_amp->chan, wvec[0]->chan(), sizeof(ml_amp->chan));
	ml_amp->chan_quark = stringToQuark(ml_amp->chan);
    	ml_amp->amptime = arrival->time;
	ml_amp->start_time = segment->tbeg();
	ml_amp->duration = segment->tend() - segment->tbeg();
	strncpy(ml_amp->amptype,ap->ml_amptype.c_str(),sizeof(ml_amp->amptype));
	strncpy(ml_amp->inarrival, "n", sizeof(ml_amp->inarrival));
	ml_amp->bandw = ap->ml_filter_hicut - ap->ml_filter_locut;
    }
    if( measure_mb_amp ) {
	mb_amp = new CssAmplitudeClass();
	mb_amp->arid = arrival->arid;
	strncpy(mb_amp->chan, wvec[0]->chan(), sizeof(mb_amp->chan));
	mb_amp->chan_quark = stringToQuark(mb_amp->chan);
    	mb_amp->amptime = arrival->time;
	mb_amp->start_time = segment->tbeg();
	mb_amp->duration = segment->tend() - segment->tbeg();
	strncpy(mb_amp->amptype,ap->mb_amptype.c_str(),sizeof(mb_amp->amptype));
	strncpy(mb_amp->inarrival, "y", sizeof(mb_amp->inarrival));
	mb_amp->bandw = ap->mb_filter_hicut - ap->mb_filter_locut;
    }

    if( wvec.size() > 1 ) {
	const char *recipe = arrivalParams()->arrival_beam_recipe.c_str();
	char *phase = (assoc) ? assoc->phase : arrival->iphase;
	if(measure_mb_amp) {
	    measureArrayAmp(wvec, arrival, origin, phase, true, mb_amp);
	    strncpy(mb_amp->chan, recipe, sizeof(mb_amp->chan));
	}
	else if(mb_amp) {
	    delete mb_amp;
	    mb_amp = NULL;
	}
	if(measure_ml_amp) {
	    measureArrayAmp(wvec, arrival, origin, phase, false, ml_amp);
	    strncpy(ml_amp->chan, recipe, sizeof(mb_amp->chan));
	}
	else if(ml_amp) {
	    delete ml_amp;
	    ml_amp = NULL;
	}
    }
    else {
	if(measure_mb_amp) {
	    if( !measureMBAmp(wvec[0]->ts, arrival->time, mb_amp) ) {
		delete mb_amp;
		mb_amp = NULL;
	    }
	    else {
		printLog("%x %s/%s: measured mb amp=%.2lf per=%.2lf\n",
		    wvec[0]->ts, wvec[0]->sta(), wvec[0]->chan(),
		    mb_amp->amp, mb_amp->per);
	    }
	}
	if(measure_ml_amp) {
	    double tbeg = arrival->time - ap->ml_lta_lead
			- ap->mb_filter_margin - 0.5;
	    double tend = arrival->time - ap->ml_lta_lead + ap->ml_lta_length
			+ ap->mb_filter_margin + 0.5;
	    ts = wvec[0]->ts->subseries(tbeg, tend);
	    ts->setOriginalStart(tbeg);
	    ts->setOriginalEnd(tend);
	    ts->removeAllMethods();

	    TaperData *taper = new TaperData("cosine", 5, 5, 200);
	    taper->apply(ts);
	    taper->deleteObject();
		
	    IIRFilter *filter = new IIRFilter(ap->ml_filter_order,
			ap->ml_filter_type, ap->ml_filter_locut,
			ap->ml_filter_hicut,
			ts->segment(0)->tdel(), ap->ml_filter_zp);
	    filter->apply(ts);
	    filter->deleteObject();

	    if( (segment = ts->segment(arrival->time)) ) {
		double calib = segment->calib();
		sps = 1/segment->tdel();
		time_pos = (int)((arrival->time - segment->tbeg())/
				segment->tdel());
		for(i = 0; i < segment->length(); i++) {
		    segment->data[i] = fabs(calib*segment->data[i]);
		}
		mlppn(segment->data, segment->length(), time_pos, sps,
				&ml_amp->amp, &ml_amp->snr);
		printLog("%x %s/%s: measured ml amp=%.2lf snr=%.2lf\n",
		    ts, ts->sta(), ts->chan(), ml_amp->amp, ml_amp->snr);
	    }
	    ts->deleteObject();
	}
    }

    *amp_mb = mb_amp;
    *amp_ml = ml_amp;

    return true;
}

bool WaveformPlot::measureMBAmp(GTimeSeries *ts, double time, CssAmplitudeClass *amp)
{
    double tbeg, tend;
    GSegment *s;
    GTimeSeries *ts_sub;
    AmplitudeParams *ap = amplitudeParams();

    tbeg = time - ap->mb_lead - ap->mb_filter_margin - 0.5;
    tend = time - ap->mb_lead + ap->mb_length + ap->mb_filter_margin + 0.5;
    s = ts->segment(tbeg);
    if(s != ts->segment(tend))
    {
        showWarning(
           "Cannot measure amplitude for %s/%s.\nData is not continuous for the time window",
	    ts->sta(), ts->chan());
	return false;
    }

    if( !(ts_sub = ts->subseries(tbeg, tend)) ) {
        showWarning("Cannot measure amplitude for %s/%s", ts->sta(),ts->chan());
        return false;
    }
    ts_sub->addOwner(this);

    /* need to set tbeg and tend to reread only that time period.
     */
    ts_sub->setOriginalStart(tbeg);
    ts_sub->setOriginalEnd(tend);

    amp->start_time = tbeg;
    amp->duration = tend - tbeg;

    if( !ts_sub->removeAllMethods() )
    {
	showWarning("Cannot reread %s/%s", ts->sta(), ts->chan());
	ts_sub->removeOwner(this);
        return false;
    }
    int percent = (int)(100*ap->mb_taper_frac + .5);
    TaperData *taper = new TaperData("cosine", percent, 5, 200);
    taper->apply(ts_sub);
    taper->deleteObject();
		
    IIRFilter *filter = new IIRFilter(ap->mb_filter_order, ap->mb_filter_type,
			ap->mb_filter_locut, ap->mb_filter_hicut,
			ts->segment(0)->tdel(), ap->mb_filter_zp);
    filter->apply(ts_sub);
    filter->deleteObject();

    Amp::getAmpError();
    if( !Amp::autoMeasureAmpPer(ts_sub, ap->mb_amptype, time,
		ap->mb_allow_counts, ap->mb_counts_amptype, amp) )
    {
	char *err_msg = Amp::getAmpError();
	if(err_msg != NULL) logErrorMsg(LOG_ERR, err_msg);
	if( (err_msg = GError::getMessage()) ) {
	    showWarning("Cannot measure amplitude.\n%s",err_msg);
	}
	ts_sub->removeOwner(this);
	return false;
    }
    else {
	double deltim;
	getSnr(ts_sub, time, &amp->snr, &deltim);
    }
    ts_sub->removeOwner(this);
    return true;
}

bool WaveformPlot::saveAmp(DataSource *ds, CssArrivalClass *arrival,
			CssAmplitudeClass *amp)
{
    int i;
    cvector<CssAmplitudeClass> amps;

    if( !amp ) return false;

    getTable(amps);

    for(i = 0; i < amps.size(); i++) {
	if(amps[i]->arid == amp->arid &&
		!strcmp(amps[i]->amptype, amp->amptype)) break;
    }
    if(i < amps.size()) {
	Password password = getpwuid(getuid());
	strncpy(amp->auth, password->pw_name, sizeof(amp->auth));
	amp->ampid = amps[i]->ampid;
	ds->changeTable(amps[i], amp);
	amp->copyTo(amps[i], false);
	TableListener::doCallbacks(amps[i], this, "edit");
	delete amp;
    }
    else if(ds->addAmplitude(arrival, amp)) {
	addTableToAll(amp);
    }
    else {
	delete amp;
	return false;
    }
    return true;
}

/*
 * mlppn - calculating IDC ML amplitude namely the maximum one second
 *         average in the first four seconds after phase time arrival (sta)
 *         corrected by the noise (30 seconds before phase time arrival) (lta)
 *         amp = sqrt(sta^2 - lta^2)
 * @param(in)  databuf - array of data float for ml, it should be filtered
 *             between 2 and 4 Hz
 * @param(in)  dim - length of databuf
 * @param(in)  atPoint - position of arrival time in databuf
 * @param(in)  sps - samples per second
 * @param(out) amp - array[2] of amplitude, snr (= sta/lta)
 *
 */
void WaveformPlot::mlppn(float *databuf, int dim, int atPoint, double sps,
		   double *amp, double *snr)
{

    double tamp, sta = 0, lta = 0;   
    int i, spoint, epoint, step, npts;
    AmplitudeParams *ap = amplitudeParams();

    /* calculate the long term average in a window starting at ml_lta_lead
     * seconds before the arrival with a length of ml_lta_length.
     */
    spoint = (int)floor((atPoint - ap->ml_lta_lead * sps) + 0.5);
    epoint = (int)floor((spoint + ap->ml_lta_length * sps) + 0.5);

    if(spoint < 0) spoint = 0;
    if(epoint >= dim) epoint = dim-1;

    if (spoint > 0) {

	for (i = spoint; i < epoint; i++)
	    lta += fabs(databuf[i]);

	lta = lta / (epoint-spoint);

    }

    /* calculate the short term average in a window of length ml_sta_length
     * starting a ml_sta_lead before the arrival.
     */
    spoint = (int)floor((atPoint - ap->ml_sta_lead * sps) + 0.5);
    epoint = (int)floor((spoint + ap->ml_sta_length * sps) + 0.5);

    // compute average for the first window of length ml_sta_length
    for (i = spoint; i < epoint; i++)
	sta += fabs(databuf[i]); 

    npts = epoint - spoint; // number of values in the window
    sta = sta / npts;
    spoint = epoint;

    // this is equal to atPoint - ml_sta_lead*sps + ml_sta_window*sps
    epoint = (int)floor((epoint +
		(ap->ml_sta_window - ap->ml_sta_length) * sps) + 0.5);
    step = (int)floor((ap->ml_sta_length * sps - 1) + 0.5);
    tamp = sta;

    for(i = spoint; i < epoint; i++) {

	// subtract the oldest point and add the newest point as the end of
	// the window slides from spoint to epoint
	tamp = tamp + (fabs(databuf[i]) - fabs(databuf[(i - step)])) / npts;

	/* keep the highest sta */
	if( tamp > sta) sta = tamp;

    }

    if (sta > lta)
	*amp = sqrt(pow(sta, 2) - pow(lta, 2));

    if (lta >= sta)
	*amp = sta;

    *snr = sta/lta;

}

void WaveformPlot::measureArrayAmp(gvector<Waveform *> &wvec,
			CssArrivalClass *arrival, CssOriginClass *origin,
			const string &phase, bool measure_mb, CssAmplitudeClass *amp)
{
    AmplitudeParams *ap = amplitudeParams();

    if(wvec.size() > 0) {
	printLog("Making amplitude beam %s/cb for arid=%d phase=%s\n",
		wvec[0]->net(), arrival->arid, phase.c_str());
    }
    GTimeSeries *beam = getAmpBeam(wvec, arrival, origin, phase, measure_mb);

    if(!beam) {
	showWarning("Cannot get beam for amplitude measurement.");
	return;
    }
    beam->addOwner(this);
    beam->setSta(wvec[0]->net());
    beam->setChan(arrival->chan);

    GSegment *s = beam->segment(arrival->time);
    if(!s) {
	showWarning("Cannot find waveform segment for %s/%s arrival time.",
		arrival->sta, arrival->chan);
	beam->removeOwner(this);
	return;
    }
    double sps;
    int time_pos;
    sps = 1./s->tdel();
    time_pos = (int)((arrival->time - s->tbeg()) / s->tdel());
    if(measure_mb) {
	double deltim;
	printLog("Measuring mb amplitude for %s/cb arid=%d phase=%s\n",
		wvec[0]->net(), arrival->arid, phase.c_str());
	getSnr(beam, arrival->time, &amp->snr, &deltim);
#ifdef USE_MBPPC
	int amp_pos;
	if(mbppc(s->data, s->length(), time_pos, sps, &amp->amp,
			&amp->per, &amp_pos))
	{
	    /* convert counts to nms;
	     * use the response of the first element
	     */
	    BasicSource::cts2nms(cd->ts,s,amp->amp,amp->per,&amp->amp);
	    amp->amptime = s->tbeg() + amp_pos/sps;
	    printLog("%s/%s mb amp=%.2lf per=%.2lf snr=%.2lf\n",
		arrival->sta, arrival->chan, amp->amp, amp->per, amp->snr);
	}
#else
	Amp::getAmpError();
	if(!Amp::autoMeasureAmpPer(beam, ap->mb_amptype, arrival->time,
		ap->mb_allow_counts, ap->mb_counts_amptype, amp))
	{
	    char *err_msg = Amp::getAmpError();
	    if(err_msg != NULL) logErrorMsg(LOG_ERR, err_msg);
	    if( (err_msg = GError::getMessage()) ) {
		char msg[2000];
		snprintf(msg, sizeof(msg), "Cannot measure amplitude.\n%s",
			err_msg);
		showWarning(msg);
	    }
	}
	else {
	    printLog("%s/%s mb amp=%.2lf per=%.2lf snr=%.2lf\n",
		arrival->sta, arrival->chan, amp->amp, amp->per, amp->snr);
	}
#endif
    }
    else {
	mlppn(s->data, s->length(), time_pos, sps, &amp->amp, &amp->snr);
	printLog("%s/%s ml amp=%.2lf snr=%.2lf\n",
		arrival->sta, arrival->chan, amp->amp, amp->snr);
    }
    beam->removeOwner(this);
}

GTimeSeries * WaveformPlot::getAmpBeam(gvector<Waveform *> &wvec,
		CssArrivalClass *arrival, CssOriginClass *origin, const string &phase,
		bool measure_mb)
{
    bool zp;
    int order;
    string op;
    double delta, az, baz, lat, lon;
    double azimuth, slowness, tbeg, tend, flo, fhi;
    AmplitudeParams *ap = amplitudeParams();

    if(Beam::getLocation(this, DNORTH_DEAST, wvec, &lon, &lat)
		== BEAM_LOCATION_ERROR)
    {
	showWarning(GError::getMessage());
	return NULL;
    }

    if(origin) {
	deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);
	azimuth = baz;

	if(getTravelTime(phase, origin, lat, lon, wvec[0]->elev(),
		wvec[0]->net(), wvec[0]->sta(), &slowness, op) <= 0.
		|| slowness < 0.)
	{
	    showWarning("Cannot compute slowness for phase %s.", phase.c_str());
	    return NULL;
	}
    }
    else {
	azimuth = arrival->azimuth;
	slowness = arrival->slow;
    }
    // convert slowness from sec/deg to sec/km.
    slowness *= 180./(M_PI*6371.);

    if(measure_mb) {
	order = ap->mb_filter_order;
	flo = ap->mb_filter_locut;
	fhi = ap->mb_filter_hicut;
	zp = ap->mb_filter_zp;
    }
    else {
	order = ap->ml_filter_order;
	flo = ap->ml_filter_locut;
	fhi = ap->ml_filter_hicut;
	zp = ap->ml_filter_zp;
    }
    // no need to read in the entire waveform
    if(measure_mb) {
	tbeg = arrival->time - ap->mb_lead - ap->mb_filter_margin - 5.;
	tend = arrival->time - ap->mb_lead + ap->mb_length
			+ ap->mb_filter_margin + 5.;
    }
    else {
	tbeg = arrival->time - ap->ml_lta_lead - ap->mb_filter_margin - 5.;
	tend = arrival->time - ap->ml_lta_lead + ap->ml_lta_length
			+ ap->mb_filter_margin + 5.;
    }

    return makeBeam(wvec, azimuth, slowness, tbeg, tend, order, flo, fhi, zp,
			measure_mb);
}

GTimeSeries * WaveformPlot::makeBeam(gvector<Waveform *> &wvec,
		double azimuth, double slowness, double tbeg, double tend,
		int order, double flo, double fhi, bool zp, bool coherent)
{
    int i;
    vector<double> weights, tlags;
    double lat, lon;
    BeamLocation beam_location = DNORTH_DEAST;
    Demean *demean=NULL;
    TaperData *taper=NULL;
    IIRFilter *filter=NULL;
    GTimeSeries **ts=NULL, *beam;

    for(i = 0; i < wvec.size(); i++) weights.push_back(1.);

    if((beam_location = Beam::getLocation(this, beam_location, wvec, &lon,
				&lat)) == BEAM_LOCATION_ERROR)
    {
	showWarning(GError::getMessage());
	return NULL;
    }

    if(!Beam::getTimeLags(this, wvec, azimuth, slowness, beam_location, tlags))
    {
	showWarning(GError::getMessage());
	return NULL;
    }

    ts = new GTimeSeries *[wvec.size()];
    for(i = 0; i < wvec.size(); i++) ts[i] = wvec[i]->ts;

    if(order >= 0) {
	demean = new Demean();
	taper = new TaperData("cosine", 5, 5, 200);
	filter = new IIRFilter(order,"BP",flo,fhi,ts[0]->segment(0)->tdel(),zp);
    }

    // filter raw waveforms before beaming
    for(i = 0; i < wvec.size(); i++)
    {
	wvec[i]->ts = new GTimeSeries(wvec[i]->ts);

	if(tbeg < wvec[i]->tbeg()) {
	    wvec[i]->setOriginalStart(wvec[i]->tbeg());
	}
	else {
	    wvec[i]->setOriginalStart(tbeg);
	}

	if(tend > wvec[i]->tend()) {
	    wvec[i]->setOriginalEnd(wvec[i]->tend());
	}
	else {
	    wvec[i]->setOriginalEnd(tend);
	}

	wvec[i]->ts->removeAllMethods();

	if(taper && filter) {
	    demean->apply(wvec[i]->ts);
	    taper->apply(wvec[i]->ts);
	    filter->apply(wvec[i]->ts);
	}
    }
    if(demean) demean->deleteObject();
    if(taper) taper->deleteObject();
    if(filter) filter->deleteObject();

    beam = Beam::BeamTimeSeries(wvec, tlags, weights, coherent);

    for(i = 0; i < wvec.size(); i++) {
	wvec[i]->ts->deleteObject();
	wvec[i]->ts = ts[i];
    }

    delete [] ts;

    return beam;
}

/*
 * mbppc - Amplitude for IDC mb - half maximum pick to through (or vice versa)
 *         in six seconds starting half second before phase arrival time, the
 *         data should be filtered between 0.8 and 4.5
 * @param(in)  databuf - array of data float for ml, it should be filtered
 *             between 2 and 4 Hz
 * @param(in)  dim - length of data
 * @param(in)  atPoint - position of arrival time in databuf
 * @param(in)  sps - samples per second
 * @param(out) bamp - array[3] of amplitude, period and position of measurement
 *             (can be transformed back time)
 *
 */
bool WaveformPlot::mbppc(float *databuf, int dim, int atPoint, double sps,
		   double *amplitude, double *period, int *amp_pos)
{

    double *Mdata, ampT, perT;
    int *data_diff, *Pdata, nOm = 0;
    int i, segL, sPoint, n, m;
    AmplitudeParams *ap = amplitudeParams();

    // Segment length of mb_length starting mb_lead before arrival time
    segL = (int)floor((ap->mb_length * sps) + 0.5);

    sPoint = (int)floor((atPoint - ap->mb_lead * sps) + 0.5);

    if(sPoint < 0) sPoint = 0;
    if(sPoint + segL - 1 >= dim) segL = dim - sPoint;
    if(segL <= 0) {
	printLog("mbppc segment length too short.\n");
	return false;
    }

    *amplitude = 0;

    Mdata = new double[segL];
    data_diff = new int[segL];
    Pdata = new int[segL];

    /* In order to find the maximum amplitude we first calculate the sign of
     * data first derivative 
     * The turning points are the places where sign of derivative is changed so
     * we register the position where the multiplication of two adjoint cells
     * is negative. Finally we calculate the amplitude and search the maximum
     * amplitude. 
     */
    n = sPoint;
    m = n - 1;
    data_diff[0] = 1;

    if (databuf[n] - databuf[m] < 0)
	data_diff[0] = -1;

    for (i = 1; i < segL; i++) {

	n = sPoint + i;
	m = n - 1;
	data_diff[i] = 1;
	if((databuf[n] - databuf[m]) < 0) data_diff[i] = -1;

	/* turning point */
	if(data_diff[i] * data_diff[(i-1)] < 0) {
	    Pdata[nOm] = i + sPoint - 1;
	    Mdata[nOm] = databuf[(i + sPoint - 1)];
	    nOm += 1;
	}
    }
 
    // maximum amplitude and corresponding period
    for (i = 0; i < nOm-1; i++) {

       ampT = fabs(Mdata[i] - Mdata[(i + 1)]);
       perT = 2 * abs(Pdata[(i + 1)] - Pdata[i]) / sps;

       if(ampT > *amplitude) {
	   *amplitude = ampT;
	   *period = perT;
	   *amp_pos = Pdata[i];
       }
    }

    delete [] Mdata;
    delete [] data_diff;
    delete [] Pdata;

    *amplitude = *amplitude / 2;

    return true;
}

bool WaveformPlot::arrivalFK(CssArrivalClass *arrival, Waveform *cd,bool fk_mb)
{
    string msg;
    double fk_lead, fk_lag, fk_dk, bandw;
    FKArgs args;
    TopWindow *tpw = topWindowParent();

    getFkParams(&fk_lead, &fk_lag, &fk_dk, &args, &bandw);

    positionDoubleLine("a", arrival->time-fk_lead, arrival->time+fk_lag, false);

    selectAll(false);

    selectWaveform(cd, true, false);

    if(fk_mb) {
	tpw->parseCmd("fk_multi_band.open", msg);
	tpw->parseCmd("fk_multi_band.compute", msg);
    }
    else {
	ostringstream os;
	os.precision(2);
	tpw->parseCmd("fk.open", msg);
	os << "fk.flo1=" << args.fmin[0];
	tpw->parseCmd(os.str(), msg);
	os.str("");
	os << "fk.fhi1=" << args.fmax[0];
	tpw->parseCmd(os.str(), msg);
	tpw->parseCmd("fk.compute", msg);
    }
    return true;
}

bool WaveformPlot::arrivalPolar(CssArrivalClass *arrival, Waveform *w)
{
    string msg;
    double tmax;
    gvector<Waveform *> wvec;
    DataMethod *dm;
    ostringstream os;
    ArrivalParams *ap = arrivalParams();
    TopWindow *tpw = topWindowParent();

    if( !getThreeComponents(w, wvec) ) return false;

    getThreeCompStats(w, wvec, arrival->time, NULL, &tmax);

    selectAll(false);

    selectWaveform(w, true, false);

    os.precision(2);

    tpw->parseCmd("polarization.open", msg);
    os.str("");
    os << "polarization.window_length=" << ap->polar_window;
    tpw->parseCmd(os.str(), msg);

    if((dm = wvec[2]->ts->getMethod("IIRFilter")) == NULL) {
	tpw->parseCmd("polarization.filter=on", msg);
	tpw->parseCmd(string("polarization.type=") + ap->polar_filter_type,msg);
	os.str("");
	os << "polarization.low=" << ap->polar_lofreq;
	tpw->parseCmd(os.str(), msg);
	os.str("");
	os << "polarization.high=" << ap->polar_hifreq;
	tpw->parseCmd(os.str(), msg);
	os.str("");
	os << "polarization.order=" << ap->polar_order;
	tpw->parseCmd(os.str(), msg);
	os.str("");
	os << "polarization.zp=" << (ap->polar_zp ? "true" : "false");
	tpw->parseCmd(os.str(), msg);
    }
    else {
	tpw->parseCmd("polarization.filter=off", msg);
    }

    double tbeg = arrival->time - ap->polar_signal_lead;

    tpw->parseCmd("polarization.compute", msg);

    os.precision(15);
    os.str("");
    os << "polarization.time_zoom tbeg=" << tbeg << " duration="
	<< ap->polar_signal_len;
    tpw->parseCmd(os.str(), msg);

    os.str("");
    os << "polarization.position_time_window label='b' " << "xmin="
	<< tmax << " xmax=" << (tmax + ap->polar_window);
    tpw->parseCmd(os.str(), msg);

    return true;
}
