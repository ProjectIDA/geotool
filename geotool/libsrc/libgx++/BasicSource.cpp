/** \file BasicSource.cpp
 *  \brief Defines class BasicSource.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sys/param.h>
#include <pwd.h>
#include <errno.h>

#include "BasicSource.h"
#include "motif++/Application.h"
#include "gobject++/GTimeSeries.h"
#include "WaveformPlot.h"
#include "Beam.h"
#include "libgio.h"
#include "ConvolveData.h"
#include "gobject++/CssTables.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

#ifndef PI
#define PI 3.14159265358979323846
#endif

static void checkWftags(GTimeSeries *ts, cvector<CssOriginClass> &origin,
		cvector<CssOriginClass> &origins, cvector<CssWftagClass> &wftags);
static bool flipBytes(void);


static string recipe_dir, recipe_dir2;

static GStation *gstations = NULL;
static int num_gstations = 0;

BasicSource::BasicSource(const string &name) : Parser()
{
    nextid = 1;

    string prop;
    if( !Application::getProperty("backup_file", prop) ) {
	const char *backup_file = cssioGetTmpPrefix("/tmp", "geotl");
	Application::putProperty("backup_file", backup_file, false);
    }

    if(!Application::getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    recipe_dir = c + string("/tables/recipes");
	}
    }
    Application::getProperty("recipeDir2", recipe_dir2);
}

BasicSource::~BasicSource(void)
{
    waveforms.clear();
}

gvector<SegmentInfo *> * BasicSource::getSegmentList(void)
{
    int i;
    bool odbc_source;
    double t, tmin, tmax, endtime, start_time, end_time;
    gvector<SegmentInfo *> *segs;
    cvector<CssWfdiscClass> wf;
    cvector<CssOriginClass> o;

    if( getSelectedTable(wf) <= 0) {
	if( getTable(wf) <= 0) return NULL;
    }

    segs = new gvector<SegmentInfo *>;

    start_time = getStartTime();
    end_time = getEndTime();
    odbc_source = sourceIsODBC();

    for(i = 0; i < wf.size(); i++)
    {
	SegmentInfo *s = new SegmentInfo();
	if(wf[i]->samprate > 0.) {
	    endtime = wf[i]->time + (wf[i]->nsamp-1)/wf[i]->samprate;
	}
	else {
	    endtime = wf[i]->endtime;
	}
	tmin = wf[i]->time;
	tmax = endtime;
	t = NULL_TIME_CHECK;
	if(wf[i]->getValue("display start", &t)
			&& t > NULL_TIME_CHECK && t > tmin)
	{
	    tmin = t;
	}
	if(wf[i]->getValue("start_phase_time", &t)
			&& t > NULL_TIME_CHECK && t > tmin)
	{
	    tmin = t;
	}
	if(wf[i]->getValue("global_start", &t)
			&& t > NULL_TIME_CHECK && t > tmin)
	{
	    tmin = t;
	}
	if(wf[i]->getValue("display end", &t)
			&& t > NULL_TIME_CHECK && t < tmax)
	{
	    tmax = t;
	}
	if(wf[i]->getValue("end_phase_time", &t)
			&& t > NULL_TIME_CHECK && t < tmax)
	{
	    tmax = t;
	}
	if(wf[i]->getValue("global_end", &t)
			&& t > NULL_TIME_CHECK && t < tmax)
	{
	    tmax = t;
	}

	if(start_time > NULL_TIME_CHECK && start_time > tmin) {
	    tmin = start_time;
	}
	if(end_time > NULL_TIME_CHECK && end_time < tmax) {
	    tmax = end_time;
	}
	s->id = wf[i]->wfid;
	stringcpy(s->sta, wf[i]->sta, sizeof(s->sta));
	stringcpy(s->chan, wf[i]->chan, sizeof(s->chan));
	s->path = wf[i]->getFile();
	if(!odbc_source) {
	    s->file_order = wf[i]->filePosition();
//	    gnetChangeStaChan(s->sta, sizeof(s->sta), s->chan, sizeof(s->chan));
	}
	s->start = tmin;
	s->end = tmax;
	s->jdate = timeEpochToJDate(s->start);
	s->selected = true;
	s->setWfdisc(wf[i]);
	s->hang = -999;
	s->vang = -999;
	wf[i]->setDataSource(this);

	gvector<SegmentInfo *> *elements;
	elements = (gvector<SegmentInfo *> *)wf[i]->getValue("elements");
	if(elements)
	{
	    s->array_elements.clear();
	    s->array_elements.load(elements);
	}
	segs->add(s);
    }

    getNetworks(segs);

    getSites(segs);

    getSitechans(segs);

    if( getTable(o) > 0)
    {
	for(i = 0; i < (int)segs->size(); i++) {
	    SegmentInfo *s = segs->at(i);
	    tmin = s->start;
	    tmax = s->end;
	    for(int j = 0; j < o.size(); j++) {
		if(o[j]->time > tmin - 3600. && o[j]->time < tmax) {
		    double delta, az, baz;
		    deltaz(o[j]->lat, o[j]->lon, s->station_lat, s->station_lon,
				&delta, &az, &baz);
		    s->origin_id = o[j]->orid;
		    s->origin_delta = delta;
		    s->origin_depth = o[j]->depth;
		    s->origin_time = o[j]->time;
		    s->origin_lat = o[j]->lat;
		    s->origin_lon = o[j]->lon;
		    s->origin_azimuth = az;
		}
	    }
	}
    }

    return segs;
}

int BasicSource::readData(gvector<SegmentInfo *> *seginfo, double start_time,			double end_time, int pts, bool preview_arr, GTimeSeries **ts,
		cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
		cvector<CssHydroFeaturesClass> &hydro_features,
		cvector<CssInfraFeaturesClass> &infra_features,
		cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
		cvector<CssAmplitudeClass> &amplitudes,
		cvector<CssAmpdescriptClass> &ampdescripts,
		cvector<CssParrivalClass> &parrivals, const char **err_msg)
{
    int			i;
    SegmentInfo	*s;
    GTimeSeries		*timeSeries = NULL;
    double		tbeg, tend, tmin, tmax;

    *err_msg = NULL;

    if(!seginfo || (int)seginfo->size() == 0) return 0;

    error_msg[0] = '\0';

    s = seginfo->front();
    tmin = s->start;
    s = seginfo->back();
    tmax = s->end;

    for(i = 0; i < (int)seginfo->size(); i++)
    {
	s = seginfo->at(i);

	if(s && !s->selected) continue;

	if(end_time > start_time && (s->start > end_time || s->end <start_time))
	{
	    continue;
	}
	if(start_time < end_time) {
	    tbeg = start_time;
	    tend = end_time;
	}
	else {
	    tbeg = tmin;
	    tend = tmax;
	}
	if(!timeSeries) {
	    resetLoaded();
	}
	makeTimeSeries(s, tbeg, tend, pts, &timeSeries, err_msg);
    }
    *ts = timeSeries;

    if(timeSeries == NULL) return(0);

    if(timeSeries->length() == 0) {
	timeSeries->deleteObject();
	*ts = (GTimeSeries *)NULL;
	return(0);
    }

    cvector<CssXtagClass> xtags;

    for(i = 0; i < (int)seginfo->size(); i++)
    {
	s = seginfo->at(i);

	if(s && !s->selected) continue;

	if(end_time > start_time && (s->start > end_time || s->end <start_time))
	{
	    continue;
	}

	if(preview_arr || !pts) {
	    loadAllTables(timeSeries, s, arrivals, origins, origerrs, assocs,
			stassocs, wftags, xtags, hydro_features, infra_features,
			stamags, netmags, amplitudes, ampdescripts, parrivals);
	}
    }

    xtags.clear();

    addStations(seginfo);

    /* save the initial tbeg, tend */
    timeSeries->setOriginalStart(timeSeries->tbeg());
    timeSeries->setOriginalEnd(timeSeries->tend());

    return(timeSeries->length());
}

GTimeSeries * BasicSource::readTimeSeries(cvector<CssWfdiscClass> &wfdiscs,
				double tbeg, double tend)
{
    GTimeSeries  *timeSeries = NULL;
    const char  *err_msg = NULL;
    SegmentInfo s;
    CssWfdiscClass *w;

    s.start = tbeg;
    s.end = tend;
    s.selected = true;
    s.hang = -999.;
    s.vang = -999.;
    error_msg[0] = '\0';

    for(int i = 0; i < wfdiscs.size(); i++)
    {
	s.id = wfdiscs.at(i)->wfid;
	stringcpy(s.sta, wfdiscs.at(i)->sta, sizeof(s.sta));
	stringcpy(s.chan, wfdiscs.at(i)->chan, sizeof(s.chan));
	s.path = wfdiscs.at(i)->getFile();
	w = new CssWfdiscClass();
	*w = *wfdiscs.at(i);
	s.setWfdisc(w);

	if(!makeTimeSeries(&s, tbeg, tend, 0, &timeSeries, &err_msg) && err_msg)
	{
	    ShowWarning(err_msg);
	}
    }

    if(!timeSeries || timeSeries->length() == 0) {
	if(timeSeries) timeSeries->deleteObject();
	return NULL;
    }
    // save the initial tbeg, tend
    timeSeries->setOriginalStart(timeSeries->tbeg());
    timeSeries->setOriginalEnd(timeSeries->tend());

    return timeSeries;
}

bool BasicSource::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    const char *full_path, *err;
    string dir, prefix;
    int n;
    CssWfdiscClass wfdisc;

    *err_msg = NULL;

    full_path = quarkToString(s->path);
    getDir(full_path, dir, prefix);

//******* check this. is owners being copied from s->wfdisc() to wfdisc 
//need operator= in CssTableClass.h
//    wfdisc = *s->wfdisc();
    s->wfdisc()->copyTo(&wfdisc);

    if(cssioCheckWfdisc(&wfdisc, dir) != 0) {
	char error[MAXPATHLEN+100];
	snprintf(error, MAX_MSG, "%s/%s record %d: %s", s->sta,
		s->chan, s->file_order+1, cssioGetErrorMsg());
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    if(*ts == NULL)
    {
	char full_prefix[MAXPATHLEN+1];

	*ts = new GTimeSeries();
	Component::printLog("%x %s/%s: waveform created.\n",
		*ts, wfdisc.sta, wfdisc.chan);
	(*ts)->waveform_io = new WaveformIO();
	DataSource::copySourceInfo(*ts, s->wfdisc());

	snprintf(full_prefix, MAXPATHLEN+1, "%s%s", dir.c_str(),prefix.c_str());
 	(*ts)->putValue("dc", stringToQuark(full_prefix));
 	(*ts)->putValue("id", s->id);
	for(int i = 0; i < s->array_elements.size(); i++) {
	    (*ts)->array_elements.add(s->array_elements[i]);
	}
    }

    GSegment *segment = readSegment(&wfdisc, dir, tbeg, tend, pts);

    if((err = cssioGetErrorMsg()) != NULL) {
	int len = strlen(error_msg);
	if(len > 0) {
	    snprintf(error_msg + len, sizeof(error_msg)-len, "\n%s", err);
	}
	else {
	    snprintf(error_msg + len, sizeof(error_msg)-len, "%s", err);
	}
	*err_msg = error_msg;
    }

    if(!segment) return false;

    if((n=invalidData(segment->length(), segment->data, (float)0.)))
    {
	char error[MAXPATHLEN+100];
	snprintf(error, MAXPATHLEN+100,
		"cssIO: setting %d invalid binary data from %s to 0.",
                        n, full_path);
	logErrorMsg(LOG_WARNING, error);
    }

    WfdiscPeriod wp;
    wp.tbeg = segment->tbeg();
    wp.tend = segment->tend();
    wp.dir = stringToQuark(dir);
    wp.prefix = stringToQuark(prefix);
    wp.wfdisc_file = stringToQuark(full_path);
    wp.wfdisc_index = s->file_order;
    wp.pts_needed = pts;
    getChanid(wfdisc.sta, wfdisc.chan, &wfdisc.chanid);
    wp.wf = wfdisc;

    if((*ts)->waveform_io) (*ts)->waveform_io->wp.push_back(wp);

    (*ts)->addSegment(segment);

    return true;
}

void BasicSource::loadAllTables(GTimeSeries *ts, SegmentInfo *s,
	cvector<CssArrivalClass> &arrivals, cvector<CssOriginClass> &origins,
	cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
	cvector<CssStassocClass> &stassocs, cvector<CssWftagClass> &wftags,
	cvector<CssXtagClass> &xtags, cvector<CssHydroFeaturesClass> &hydro_features,
	cvector<CssInfraFeaturesClass> &infra_features,
	cvector<CssStamagClass> &stamags, cvector<CssNetmagClass> &netmags,
	cvector<CssAmplitudeClass> &amplitudes,
	cvector<CssAmpdescriptClass> &ampdescripts,
	cvector<CssParrivalClass> &parrivals)
{
    loadTsDatabase(ts, s);

    loadArrivals(ts, s->sta, s->net, arrivals);

    loadAssocs(arrivals, assocs);
    loadStassocs(arrivals, stassocs);
    loadHydroFeatures(arrivals, hydro_features);
    loadInfraFeatures(arrivals, infra_features);
    loadStamags(arrivals, stamags);
    loadOridWftags(s, wftags);
    loadXtags(s, xtags);
    loadOrigins(ts, assocs, wftags, xtags, origins);
    loadOrigerrs(origins, origerrs);
    loadNetmags(origins, netmags);
    loadParrivals(origins, parrivals);
    loadAmplitudes(arrivals, amplitudes, ampdescripts);
}

void BasicSource::loadTsDatabase(GTimeSeries *ts, SegmentInfo *s)
{
    if(!ts) return;

    CssWfdiscClass *w = s->wfdisc();

    ts->setSta(s->sta);
    ts->setChan(s->chan);
    ts->setNet(s->net);

    ts->setChanid(w->chanid);
    ts->setJDate(w->jdate);
    ts->setInstype(w->instype);

    ts->setLat(s->station_lat);
    ts->setLon(s->station_lon);
    ts->setElev(s->station_elev);
    ts->setDnorth(s->dnorth);
    ts->setDeast(s->deast);
    ts->setRefsta(s->refsta);

    if(s->hang != -999) {
	ts->setHang(s->hang);
	ts->setVang(s->vang);
    }
    if(s->alpha > -900) {
	ts->setCurrentAlpha(s->alpha);
	ts->setCurrentBeta(s->beta);
	ts->setCurrentGamma(s->gamma);
    }
    else {
	ts->setCurrentAlpha(0.);
	ts->setCurrentBeta(0.);
	ts->setCurrentGamma(0.);
    }
    ts->setAlpha(s->alpha);
    ts->setBeta(s->beta);
    ts->setGamma(s->gamma);
    ts->setXChan(s->x_chan);
    ts->setYChan(s->y_chan);
    ts->setZChan(s->z_chan);
}


void BasicSource::loadArrivals(GTimeSeries *ts, const string &sta,
		const string &net, cvector<CssArrivalClass> &arrivals)
{
    cvector<CssArrivalClass> a;

    if( getTable(a) <= 0 ) return;

    double tbeg = ts->tbeg();
    double tend = ts->tend();
    int dc = 1;
    ts->getValue("dc", &dc);

    for(int i = 0; i < a.size(); i++)
    {
	if(!a[i]->getLoaded()
		&& (!sta.compare(a[i]->sta) || !net.compare(a[i]->sta))
		&& tbeg <= a[i]->time && a[i]->time <= tend)
	{
	    a[i]->setLoaded(true);
	    arrivals.push_back(a[i]);
	    a[i]->setIds(dc, a[i]->arid);

	    a[i]->amp_Nnms = a[i]->amp;
	    strcpy(a[i]->phase, a[i]->iphase);

	    a[i]->putValue("sta", a[i]->sta);
	    a[i]->putValue("chan", a[i]->chan);
	}
    }
}

void BasicSource::loadAssocs(cvector<CssArrivalClass> &arrivals,
			cvector<CssAssocClass> &assocs)
{
    cvector<CssAssocClass> a;

    if( getTable(a) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++)
    {
	for(int j = 0; j < a.size(); j++)
	    if(a[j]->arid == arrivals.at(i)->arid && !a[j]->getLoaded())
	{
	    a[j]->setLoaded(true);
	    assocs.push_back(a[j]);
	    a[j]->setIds(arrivals.at(i)->getDC(), 1);

	    stringcpy(arrivals.at(i)->phase, a[j]->phase,
			sizeof(arrivals.at(i)->phase));
	}
    }
}

void BasicSource::loadStassocs(cvector<CssArrivalClass> &arrivals,
			cvector<CssStassocClass> &stassocs)
{
    cvector<CssStassocClass> s;

    if( getTable(s) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++)
    {
	for(int j = 0; j < s.size(); j++)
	    if(s[j]->stassid == arrivals.at(i)->arid && !s[j]->getLoaded())
	{
	    s[j]->setLoaded(true);
	    stassocs.push_back(s[j]);
	    s[j]->setIds(arrivals.at(i)->getDC(), 1);
	}
    }
}

void BasicSource::loadHydroFeatures(cvector<CssArrivalClass> &arrivals,
				cvector<CssHydroFeaturesClass> &hydro_features)
{
    cvector<CssHydroFeaturesClass> h;

    if( getTable(h) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++)
    {
	for(int j = 0; j < h.size(); j++)
	    if(h[j]->arid == arrivals.at(i)->arid && !h[j]->getLoaded())
	{
	    h[j]->setLoaded(true);
	    hydro_features.push_back(h[j]);
	    h[j]->setIds(arrivals.at(i)->getDC(), 1);
	    h[j]->arid = arrivals.at(i)->arid;
	}
    }
}

void BasicSource::loadInfraFeatures(cvector<CssArrivalClass> &arrivals,
				cvector<CssInfraFeaturesClass> &infra_features)
{
    cvector<CssInfraFeaturesClass> infra;

    if( getTable(infra) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++) 
    {
	for(int j = 0; j < infra.size(); j++)
	    if(infra[j]->arid == arrivals.at(i)->arid && !infra[j]->getLoaded())
	{
	    infra[j]->setLoaded(true);
	    infra_features.push_back(infra[j]);
	    infra[j]->setIds(arrivals.at(i)->getDC(), 1);
	    infra[j]->arid = arrivals.at(i)->arid;
	}
    }
}

void BasicSource::loadStamags(cvector<CssArrivalClass> &arrivals,
				cvector<CssStamagClass> &stamags)
{
    cvector<CssStamagClass> s;

    if( getTable(s) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++) 
    {
	for(int j = 0; j < s.size(); j++)
	    if(s[j]->arid == arrivals.at(i)->arid && !s[j]->getLoaded())
	{
	    s[j]->setLoaded(true);
	    stamags.push_back(s[j]);
	    s[j]->setIds(arrivals.at(i)->getDC(), 1);
	    s[j]->arid = arrivals.at(i)->arid;
	}
    }
}

void BasicSource::loadOridWftags(SegmentInfo *s, cvector<CssWftagClass> &wftags)
{
    cvector<CssWftagClass> w;

    if( getTable(w) <= 0 ) return;

    for(int j = 0; j < w.size(); j++) {
	if(w[j]->wfid == s->wfdisc()->wfid &&
	   !strcasecmp(w[j]->tagname, "orid") && !w[j]->getLoaded())
	{
	    w[j]->setLoaded(true);
	    wftags.push_back(w[j]);
	}
    }
}

void BasicSource::loadXtags(SegmentInfo *s, cvector<CssXtagClass> &xtags)
{
    cvector<CssXtagClass> x;

    if( getTable(x) <= 0 ) return;

    for(int j = 0; j < x.size(); j++) {
	if(((x[j]->thisid == s->wfdisc()->wfid
		&& !strcmp(x[j]->thisname, "wfid")
		&& !strcmp(x[j]->thatname, "orid")) ||
	   (x[j]->thatid == s->wfdisc()->wfid
		&& !strcmp(x[j]->thatname, "wfid")
		&& !strcmp(x[j]->thisname, "orid")))
		 && !x[j]->getLoaded())
	{
	    x[j]->setLoaded(true);
	    xtags.push_back(x[j]);
	}
    }
}

void BasicSource::loadOrigins(GTimeSeries *ts, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssXtagClass> &xtags,
		cvector<CssOriginClass> &origins)
{
    int origin_load;
    cvector<CssOriginClass> origin;

    if( getTable(origin) <= 0 ) return;

    origin_load = Application::getProperty("origin_load", 1);

    // load orids that have assocs
    for(int i = 0; i < assocs.size(); i++) {
	for(int j = 0; j < origin.size(); j++) 
	    if(origin[j]->orid == assocs.at(i)->orid && !origin[j]->getLoaded())
	{
	    origin[j]->setLoaded(true);
	    origins.push_back(origin[j]);
//	    addWftag(ts, origin[j], wftags);
	}
    }
    // load orids that have wftags
    for(int i = 0; i < wftags.size(); i++) {
	for(int j = 0; j < origin.size(); j++) 
	    if(origin[j]->orid == wftags.at(i)->tagid && !origin[j]->getLoaded())
	{
	    origin[j]->setLoaded(true);
	    origins.push_back(origin[j]);
	}
    }
    // load orids that have xtags
    for(int i = 0; i < xtags.size(); i++) {
	CssXtagClass *x = xtags.at(i);
	long orid;
	if(!strcmp(x->thisname, "orid")) orid = x->thisid;
	else if(!strcmp(x->thatname, "orid")) orid = x->thatid;
	else continue;
	for(int j = 0; j < origin.size(); j++)
	    if(origin[j]->orid == orid && !origin[j]->getLoaded())
	{
	    origin[j]->setLoaded(true);
	    origins.push_back(origin[j]);
//	    addWftag(ts, origin[j], wftags);
	}
    }
    if(origin_load == 1) {
	// load orids that have time > tbeg-4hrs && time < tend
	for(int j = 0; j < origin.size(); j++)
	    if(!origin[j]->getLoaded() && origin[j]->time < ts->tend() && 
		origin[j]->time > ts->tbeg() - 4*60*60)
	{
	    origin[j]->setLoaded(true);
	    origins.push_back(origin[j]);
//	    addWftag(ts, origin[j], wftags);
	}
    }
    if(origin_load == 2) {
	// load all orids available
	for(int j = 0; j < origin.size(); j++) if(!origin[j]->getLoaded())
	{
	    origin[j]->setLoaded(true);
	    origins.push_back(origin[j]);
//	    addWftag(ts, origin[j], wftags);
	}
    }
    if( createOridWftags() ) {
	checkWftags(ts, origin, origins, wftags);
    }
}

static void
checkWftags(GTimeSeries *ts, cvector<CssOriginClass> &origin,
		cvector<CssOriginClass> &origins, cvector<CssWftagClass> &wftags)
{
    if( ts->waveform_io && (int)ts->waveform_io->wp.size() > 0)
    {
	long wfid = ts->waveform_io->wp[0].wf.wfid;

	for(int i = 0; i < origin.size(); i++) if( !origin[i]->getLoaded() )
	{
	    origin[i]->setLoaded(true);
	    origins.push_back(origin[i]);

	    CssWftagClass *w = new CssWftagClass();
	    wftags.push_back(w);
	    w->setIds(origin[i]->getDC(), 1);
	    strcpy(w->tagname, "orid");
	    w->tagid = origin[i]->orid;
	    w->wfid = wfid;
	}
    }
}

void BasicSource::loadOrigerrs(cvector<CssOriginClass> &origins,
			cvector<CssOrigerrClass> &origerrs)
{
    cvector<CssOrigerrClass> origerr;

    if( getTable(origerr) <= 0 ) return;

    for(int i = 0; i < origins.size(); i++)
    {
	for(int j = 0; j < origerr.size(); j++)
	    if(origerr[j]->orid == origins.at(i)->orid && !origerr[j]->getLoaded())
	{
	    origerr[j]->setLoaded(true);
	    origerrs.push_back(origerr[j]);
	}
    }
}

void BasicSource::loadNetmags(cvector<CssOriginClass> &origins,
			cvector<CssNetmagClass> &netmags)
{
    cvector<CssNetmagClass> netmag;

    if( getTable(netmag) <= 0 ) return;

    for(int i = 0; i < origins.size(); i++)
    {
	for(int j = 0; j < netmag.size(); j++)
	    if(netmag[j]->orid ==origins.at(i)->orid && !netmag[j]->getLoaded())
	{
	    netmag[j]->setLoaded(true);
	    netmags.push_back(netmag[j]);
	}
    }
}

void BasicSource::loadParrivals(cvector<CssOriginClass> &origins,
			cvector<CssParrivalClass> &parrivals)
{
    cvector<CssParrivalClass> parrival;

    if( getTable(parrival) <= 0 ) return;

    for(int i = 0; i < origins.size(); i++)
    {
	for(int j = 0; j < parrival.size(); j++)
	    if(parrival[j]->orid==origins.at(i)->orid && !parrival[j]->getLoaded())
	{
	    parrival[j]->setLoaded(true);
	    parrivals.push_back(parrival[j]);
	}
    }
}

void BasicSource::loadAmplitudes(cvector<CssArrivalClass> &arrivals,
			cvector<CssAmplitudeClass> &amplitudes,
			cvector<CssAmpdescriptClass> &ampdescripts)
{
    cvector<CssAmplitudeClass> amp;

    if( getTable(amp) <= 0 ) return;

    for(int i = 0; i < arrivals.size(); i++)
    {
	for(int j = 0; j < amp.size(); j++)
	    if(amp[j]->arid == arrivals.at(i)->arid && !amp[j]->getLoaded())
	{
	    amp[j]->setLoaded(true);
	    amplitudes.push_back(amp[j]);
	}
    }
// no ampdescripts yet.
}

void BasicSource::getDir(const string &full_path, string &dir, string &prefix)
{
    int i, n, prefix_len, dir_len;
    /*
     * get the directory name and prefix from full_path
     */
    n = (int)full_path.length();
    for(i = n-1; i >= 0; i--) {
	if(full_path[i] == '/') break;
    }
    dir_len = i+1;
    dir.assign(full_path, 0, dir_len);

    /*
     * get the prefix
     */
    if(n >= 7 && !strcmp(full_path.c_str()+n-7, ".wfdisc")) {
	prefix_len = n - dir_len - (int)strlen(".wfdisc");
    }
    else {
	prefix_len = n - dir_len;
    }
    prefix.assign(full_path, dir_len, prefix_len);
}

void BasicSource::getNetworks(gvector<SegmentInfo *> *segs)
{
    int i, j;
    static bool warn = true;
    cvector<CssAffiliationClass> *a = getAffiliationTable();

    if( !a || a->size() == 0) {
	/* Need affiliations
	 */
	getNetworkTables();

	a = getAffiliationTable();
	if( !a || a->size() == 0 ) {
	    if(warn) {
		logErrorMsg(LOG_WARNING, "No affiliations found.");
		warn = false;
	    }
	    return;
	}
    }
    for(i = 0; i < (int)segs->size(); i++)
    {
	SegmentInfo *s = segs->at(i);
	int quark = stringUpperToQuark(s->sta);
	for(j = 0; j < a->size() && quark != a->at(j)->net_quark; j++);
	if(j < a->size()) {
	    stringcpy(s->net, a->at(j)->net, sizeof(s->net));
	}
	else {
	    for(j = 0; j < a->size() && quark != a->at(j)->sta_quark;j++);
	    if(j < a->size()) {
		stringcpy(s->net, a->at(j)->net, sizeof(s->net));
	    }
	    else {
		stringcpy(s->net, s->sta, sizeof(s->net));
	    }
	}
    }
}

void BasicSource::getSites(gvector<SegmentInfo *> *segs)
{
    int i, j;
    long ondate, offdate;
    static bool warn = true;
    cvector<CssSiteClass> *site = getSiteTable();

    if( !site || site->size() == 0 ) {
	/* Need sites
	 */
	getNetworkTables();

	site = getSiteTable();
	if( !site || site->size() == 0 ) {
	    if(warn) {
		logErrorMsg(LOG_WARNING, "No sites found.");
		warn = false;
	    }
	    return;
	}
    }
    for(i = 0; i < segs->size(); i++)
    {
	SegmentInfo *s = segs->at(i);
	int quark = stringUpperToQuark(s->sta);
	long jdate = s->jdate;

	for(j = 0; j < site->size(); j++)
	{
	    if(quark == site->at(j)->sta_quark) {
		ondate = site->at(j)->ondate;
		offdate = site->at(j)->offdate;
		// the ondate <= wfdisc.jdate <= offdate, or wfdisc.jdate <= 0
		if( (jdate <= 0 || ondate <= 0 || jdate >= ondate) &&
		    (jdate <= 0 || offdate <= 0 || jdate <= offdate) ) break;
	    }
	}
	if(j < site->size())
	{
	    s->station_lat = site->at(j)->lat;
	    s->station_lon = site->at(j)->lon;
	    s->station_elev = site->at(j)->elev;
	    s->dnorth = site->at(j)->dnorth;
	    s->deast = site->at(j)->deast;
	    stringcpy(s->refsta, site->at(j)->refsta, sizeof(s->refsta));
	}
    }
}

typedef struct
{
    CssSitechanClass *s;
    double samprate;
} SiteSamprate;

void BasicSource::getSitechans(gvector<SegmentInfo *> *segs)
{
    int i, j, k;
    static bool warn = true;
    cvector<CssSitechanClass> *sitechan = getSitechanTable();
    double rad = PI/180.;
    double theta, phi, x[3], y[3], z[3];

    if( !sitechan || sitechan->size() == 0 ) {
	/* Need sitechans
	 */
	getNetworkTables();

	sitechan = getSitechanTable();
	if( !sitechan || sitechan->size() == 0 ) {
	    if(warn) {
		logErrorMsg(LOG_WARNING, "No sitechans found.");
		warn = false;
	    }
	    return;
	}
    }
    for(k = 0; k < segs->size(); k++)
    {
	SegmentInfo *s = segs->at(k);
	long jdate = s->jdate;
	int m, n=0;
	int sta_quark = stringUpperToQuark(s->sta);
	int sta_depth = (int)(s->station_depth+.5);
	CssSitechanClass *sc[3];
	SiteSamprate tmp[100], this_sc = {NULL, 0.};

	for(i = 0; i < sitechan->size() && n < 100; i++)
	{
	    // collect all sitechans with the wfdisc station name,
	    // station depth, and valid date
	    if(sta_quark == sitechan->at(i)->sta_quark && (sta_depth < 0
		|| sta_depth == (int)(sitechan->at(i)->edepth+.5)))
	    {
		long ondate = sitechan->at(i)->ondate;
		long offdate = sitechan->at(i)->offdate;

		// the ondate <= wfdisc.jdate <= offdate, or wfdisc.jdate <= 0
		if( (jdate <= 0 || ondate <= 0 || jdate >= ondate) &&
		    (jdate <= 0 || offdate <= 0 || jdate <= offdate) )
		{
		    tmp[n].samprate = 0.;
		    for(j = 0; j < segs->size(); j++)
			if(!strcasecmp(segs->at(j)->sta, sitechan->at(i)->sta)
			&& !strcasecmp(segs->at(j)->chan,sitechan->at(i)->chan))
		    {
			tmp[n].samprate = segs->at(j)->samprate;
			break;
		    }
		    tmp[n].s = sitechan->at(i);

		    if( compareChan(s->chan, sitechan->at(i)->chan) )
		    {
			s->hang = sitechan->at(i)->hang;
			s->vang = sitechan->at(i)->vang;
			this_sc.samprate = tmp[n].samprate;
			this_sc.s = sitechan->at(i);
		    }
		    n++;
		}
	    }
	}

	if(this_sc.s && n >= 3) {
	    sc[0] = this_sc.s;
	    theta = rad*sc[0]->hang;
	    phi = rad*sc[0]->vang;
	    x[0] = sin(theta)*sin(phi);
	    y[0] = cos(theta)*sin(phi);
	    z[0] = cos(phi);
	    m = 1;
	    // find two other channels that look like components
	    for(i = 0; i < n && m < 3; i++)
		if(fabs(tmp[i].samprate - this_sc.samprate) < .001)
	    {
		for(j = 0; j < m && tmp[i].s != sc[j]; j++);
		if(j == m) {
		    theta = rad*tmp[i].s->hang;
		    phi = rad*tmp[i].s->vang;
		    x[m] = sin(theta)*sin(phi);
		    y[m] = cos(theta)*sin(phi);
		    z[m] = cos(phi);
		    for(j = 0; j < m; j++) {
			double dot = x[j]*x[m] + y[j]*y[m] + z[j]*z[m];
			// components must be separated by more than 20deg
			if(dot > .94) break;
		    }
		    if(j == m) sc[m++] = tmp[i].s;
		}
	    }

	    // If there are three sitechans for this station with vang >= 0 and
	    // this channel name is one of them, then determine the station
	    // three component orientation.
	    if(m == 3) {
		getComponentOrientation(s, sc, this_sc.s);
	    }
	}
    }
}

void BasicSource::getComponentOrientation(SegmentInfo *s, CssSitechanClass **sc,
			CssSitechanClass *this_sc)
{
    // determine the component orientation.
    double rad = PI/180.;
    double x[3], y[3], z[3], cx, cy, cz, dot;

    // compute the channel vectors
    for(int i = 0; i < 3; i++) {
	double theta = rad*sc[i]->hang;
	double phi = rad*sc[i]->vang;
	x[i] = sin(theta)*sin(phi);
	y[i] = cos(theta)*sin(phi);
	z[i] = cos(phi);
    }
    // find the vertical component
    int i1, i2, i3 = 0;
    for(int i = 1; i < 3; i++) {
	if( fabs(z[i]) > fabs(z[i3]) ) {
	    i3 = i;
	}
    }
    // compute the cross-product of the other two components to order
    // the components as a right-handed orthogonal system (x,y,z)
    if(i3 == 0) { // the vertical (z) component is sc[0]
	i1 = 1; // horizontal components are 1 and 2
	i2 = 2;
    }
    else if(i3 == 1) { // the vertical (z) component is sc[1]
	i1 = 0; // horizontal components are 0 and 2
	i2 = 2;
    }
    else { // the vertical (z) component is sc[2]
	i1 = 0; // horizontal components are 0 and 1
	i2 = 1;
    }
    // the cross-product of the horizontal component X x Y should be
    // parallel to the vertical z component.
    cx = y[i1]*z[i2] - z[i1]*y[i2];
    cy = z[i1]*x[i2] - x[i1]*z[i2];
    cz = x[i1]*y[i2] - y[i1]*x[i2];
    dot = cx*x[i3] + cy*y[i3] + cz*z[i3];
    if(fabs(dot - 1.) < .001) {
	// the dot product is approximately 1.
	// x = i1, y = i2, z = i3
    }
    else  {
	// compute the cross product of i2 X i1
	cx = y[i2]*z[i1] - z[i2]*y[i1];
	cy = z[i2]*x[i1] - x[i2]*z[i1];
	cz = x[i2]*y[i1] - y[i2]*x[i1];
	dot = cx*x[i3] + cy*y[i3] + cz*z[i3];
	if(fabs(dot - 1.) < .001) {
	    // the dot product is approximately 1.
	    // x = i2, y = i1, z = i3
	    int i = i1;  // reverse i1 and i2 so x=i1, y=i2, z=i3
	    i1 = i2;
	    i2 = i;
	}
	else {
	   // the three components are not orthogonal.
	   printf("Warning: Non-orthogonal station components.\n");
	}
    }
    strcpy(s->x_chan, sc[i1]->chan);
    strcpy(s->y_chan, sc[i2]->chan);
    strcpy(s->z_chan, sc[i3]->chan);

    if(this_sc == sc[i1]) {
	s->component = 1;
    }
    else if(this_sc == sc[i2]) {
	s->component = 2;
    }
    else if(this_sc == sc[i3]) {
	s->component = 3;
    }
    /* Determine the Euler angles from E,N,UP coordinate system to the
     * station coordinate system.
     * beta is the angle between the E,N,UP z axis and the station z axis.
     * alpha is the angle between the E,N,UP x axis and the projection of the
     *	station z axis onto the E,N,UP xy plane.
     *
     * gamma is determined by transforming the E,N,UP x axis (1,0,0)
     *	to the station x axis using the transformation matrix
     *	(a = alpha, b = beta, g = gamma):
     *
  cos(a)cos(b)cos(g)-sin(a)sin(g)  sin(a)cos(b)cos(g)+cos(a)sin(g) -sin(b)cos(g)
 -cos(a)cos(b)sin(g)-sin(a)cos(g) -sin(a)cos(b)sin(g)+cos(a)cos(g)  sin(b)sin(g)
           cos(a)cos(b)			  sin(a)sin(b)			cos(b)
     *
     * Multiplying this matrix on the right by (1,0,0) and using the
     * following relations
     * Zx = cos(a) the x coordinate of the station vertical component
     * Zy = sin(a) the y coordinate of the station vertical component
     * Zz = cos(b) the z coordinate of the station vertical component
     * Xx = the x coordinate of the station horizontal X component
     * Xy = the y coordinate of the station horizontal X component
     * we get the two equations for g (gamma):
     *	 Zx*Zz*cos(g) - Zy*sin(g) = Xx
     *	-Zx*Zz*sin(g) - Zy*cos(g) = Xy
     *
     * The solution for gamma is
     *	gamma = atan2(-Xx*Zy - Xy*Zx*Zz, Xx*Zx*Zz - Xy*Zy)
     */

    s->beta = acos(z[i3])/rad;
    if(fabs(y[i3]) < 1.e-15 && fabs(x[i3]) < 1.e-15) {
	s->alpha = atan2(y[i1], x[i1])/rad;
	s->gamma = 0.;
    }
    else {
	s->alpha = atan2(y[i3], x[i3])/rad;
	s->gamma = atan2(-x[i1]*y[i3] - y[i1]*x[i3]*z[i3],
			x[i1]*x[i3]*z[i3] - y[i1]*y[i3])/rad;
    }
    if(s->alpha < 1.e-10) s->alpha = 0.;
}

int BasicSource::getWaveforms(gvector<Waveform *> &wvec, bool displayed_only)
{
    int i, num_selected, npts;
    SegmentInfo **wav = NULL;
    gvector<SegmentInfo *> *seginfo;
    vector<WaveInput *> *wav_inputs;
    vector<ArrivalInfo> arr_info;
    WaveformConstraint wc;
    GTimeSeries *ts;
    cvector<CssArrivalClass> arrivals;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssAssocClass> assocs;
    cvector<CssStassocClass> stassocs;
    cvector<CssHydroFeaturesClass> hydro_features;
    cvector<CssInfraFeaturesClass> infra_features;
    cvector<CssStamagClass> stamags;
    cvector<CssNetmagClass> netmags;
    cvector<CssWftagClass> wftags;
    cvector<CssAmplitudeClass> amplitudes;
    cvector<CssAmpdescriptClass> ampdescripts;
    cvector<CssParrivalClass> parrivals;
    const char *err_msg;

    wvec.clear();

    if(!(seginfo = getSegmentList())) return 0;

    num_selected = (int)seginfo->size();

    wav = (SegmentInfo **)malloc(num_selected*sizeof(SegmentInfo *));

    for(i = 0; i < num_selected; i++) {
	wav[i] = seginfo->at(i);
    }

wc.window_length = 10.;
    if(!(wav_inputs = wc.getTimeLimits(num_selected, wav, arr_info)))
    {
	Free(wav);
	return 0;
    }

    if((int)wav_inputs->size() > 0)
    {
	for(i = 0; i < waveforms.size(); i++) {
	    waveforms[i]->ts->removeOwner(waveforms[i]);
	}
	waveforms.clear();
    }
    for(i = 0; i < (int)wav_inputs->size(); i++)
    {
	WaveInput *w = wav_inputs->at(i);

	arrivals.clear();
	origins.clear();
	origerrs.clear();
	assocs.clear();
	stassocs.clear();
	wftags.clear();
	hydro_features.clear();
	infra_features.clear();
	stamags.clear();
	netmags.clear();
	amplitudes.clear();
	ampdescripts.clear();
	parrivals.clear();

	npts = readData(&w->segments, w->start, w->end, 0, false, &ts,
			arrivals, origins, origerrs, assocs, stassocs, wftags,
			hydro_features, infra_features, stamags, netmags,
			amplitudes, ampdescripts, parrivals, &err_msg);
	if(err_msg != NULL) {
	    ShowWarning("%s", err_msg);
	}
	if(npts <= 0) {
	    ts->deleteObject();
	    continue;
	}

	Waveform *wf = new Waveform(ts);
	waveforms.push_back(wf);
	wf->ts = ts;
	wf->ts->addOwner(w);
	wf->visible = true;
	wf->selected = true;

	wvec.push_back(wf);
    }

    Free(wav);
    for(i = 0; i < (int)wav_inputs->size(); i++) delete wav_inputs->at(i);
    delete wav_inputs;

    return waveforms.size();
}

Waveform * BasicSource::getWaveform(int id)
{
    for(int i = 0; i < waveforms.size(); i++) {
	if(waveforms[i]->getId() == id) return waveforms[i];
    }
    return NULL;
}

int BasicSource::getArrivalsOnWaveform(Waveform *w,
			cvector<CssArrivalClass> &arrivals)
{
    resetLoaded(cssArrival);

    arrivals.clear();
    loadArrivals(w->ts, w->sta(), w->net(), arrivals);

    return arrivals.size();
}

void BasicSource::resetLoaded(void)
{
    resetLoaded(cssArrival);
    resetLoaded(cssOrigin);
    resetLoaded(cssOrigerr);
    resetLoaded(cssAssoc);
    resetLoaded(cssStassoc);
    resetLoaded(cssWftag);
    resetLoaded(cssHydroFeatures);
    resetLoaded(cssInfraFeatures);
    resetLoaded(cssStamag);
    resetLoaded(cssNetmag);
    resetLoaded(cssAmplitude);
    resetLoaded(cssAmpdescript);
    resetLoaded(cssParrival);
}

void BasicSource::resetLoaded(const string &css_table_name)
{
    gvector<CssTableClass *> v;

    getTable(css_table_name, v);

    for(int i = 0; i < v.size(); i++) {
	v[i]->setLoaded(false);
    }
}

bool BasicSource::reread(GTimeSeries *ts)
{
    const char *err;
    double tbeg, tend, d;

    if( !ts->waveform_io || (int)ts->waveform_io->wp.size() == 0)
    {
	snprintf(error_msg, MAX_MSG,
		"BasicSource.rereadData: cannot re-read data");
	return false;
    }
    if( (tbeg = ts->originalStart()) == NULL_TIME) {
	tbeg = ts->tbeg();
    }
    if( (tend = ts->originalEnd()) == NULL_TIME) {
	tend = ts->tend();
    }

    ts->removeAllSegments();

    for(int i = 0; i < (int)ts->waveform_io->wp.size(); i++) {
	GSegment *segment = readSegment(&ts->waveform_io->wp[i].wf,
				quarkToString(ts->waveform_io->wp[i].dir),
				tbeg, tend, ts->waveform_io->wp[i].pts_needed);
	if(segment != NULL) {
	    ts->addSegment(segment);
	}
	if((err = cssioGetErrorMsg()) != NULL) {
	    stringcpy(error_msg, err, sizeof(error_msg));
	    return false;
	}
    }
    ts->truncate(tbeg, tend);

    d = ts->alpha();
    if(d < -900.) d = 0.;
    ts->setCurrentAlpha(d);
    
    d = ts->beta();
    if(d < -900.) d = 0.;
    ts->setCurrentBeta(d);

    d = ts->gamma();
    if(d < -900.) d = 0.;
    ts->setCurrentGamma(d);

    return true;
}

bool BasicSource::getChanid(const string &sta, const string &chan, long *chanid)
{
    cvector<CssSitechanClass> v;

    if( getTable(v) <= 0 ) return false;

    // could also check ondate, offdate, and edepth as in getSiteChans
    for(int i = 0; i < v.size(); i++) {
	if( !strcasecmp(sta.c_str(),v[i]->sta) && compareChan(chan,v[i]->chan) )
	{
	    *chanid = v[i]->chanid;
	    return true;
	}
    }
    return false;
}

int BasicSource::getPathInfo(PathInfo **path_info)
{
    cvector<CssArrivalClass> arrivals;
    cvector<CssAssocClass> assocs;
    cvector<CssOriginClass> origins;
    CssSiteClass *site;
    int i,j,k;

    getTable(arrivals);
    getTable(assocs);
    getTable(origins);

    PathInfo *path = (PathInfo *)mallocWarn(sizeof(PathInfo));
    int num = 0;

    for(i = 0; i < arrivals.size(); i++)
    {
	for(j = 0; j < assocs.size() &&
		assocs[j]->arid != arrivals[i]->arid; j++);
	if(j < assocs.size()) {
	    for(k = 0; k < origins.size(); k++) {
		if(origins[k]->orid == assocs[j]->orid) break;
	    }
	    if(k < origins.size())
	    {
		path = (PathInfo *)reallocWarn(path, (num+1)*sizeof(PathInfo));
		strcpy(path[num].sta, arrivals[i]->sta);
		site = getSite(arrivals[i]->sta,
				timeEpochToJDate(arrivals[i]->time));
		if(site) {
		    path[num].lat = site->lat;
		    path[num].lon = site->lon;
		}
		else {
		    path[num].lat = -999.;
		    path[num].lon = -999.;
		}
		path[num].origin = origins[k];
		path[num].fg = 0;
		num++;
	    }
	}
    }
    *path_info = path;
    return num;
}

int BasicSource::getNetwork(const string &net, GStation ***stations)
{
    GStation **sta = (GStation **)malloc(sizeof(GStation *));
    int net_quark = stringUpperToQuark(net);
    int n = 0;

    int jdate = timeEpochToJDate(timeGetEpoch());
    fillStations(net, jdate);

    for(int i = 0; i < num_gstations; i++) {
	if(net_quark == gstations[i].net) {
	    sta = (GStation **)realloc(sta, (n+1)*sizeof(GStation *));
	    sta[n++] = &gstations[i];
	}
    }
    if(!n) {
	free(sta);
	sta = NULL;
    }
    *stations = sta;
    return n;
}

void BasicSource::addStations(gvector<SegmentInfo *> *segs)
{
    int i, j, k;
    int sta, chan, net;

    for(i = 0; i < (int)segs->size(); i++) {
	SegmentInfo *s = segs->at(i);
	sta = stringUpperToQuark(s->sta);
	chan = stringUpperToQuark(s->chan);
	net = stringUpperToQuark(s->net);

	for(j = 0; j < num_gstations; j++) {
	    if(sta == gstations[j].sta) break;
	}
	if(j == num_gstations) {
	    if(!(gstations = (GStation *)reallocWarn(gstations,
			(num_gstations+1)*sizeof(GStation)))) return;
	    gstations[num_gstations].sta = sta;
	    gstations[num_gstations].net = net;
	    gstations[num_gstations].refsta =stringUpperToQuark(s->refsta);
	    gstations[num_gstations].lat = s->station_lat;
	    gstations[num_gstations].lon = s->station_lon;
	    gstations[num_gstations].elev = s->station_elev;
	    gstations[num_gstations].nchan = 1;
	    if(!(gstations[num_gstations].chan =
			(int *)mallocWarn(sizeof(int)))) return;
	    gstations[num_gstations].chan[0] = chan;
	    num_gstations++;
	}
	else {
	    for(k = 0; k < gstations[j].nchan; k++) {
		    if(chan == gstations[j].chan[k]) break;
	    }
	    if(k == gstations[j].nchan) {
		if(!(gstations[j].chan = (int *)reallocWarn(gstations[j].chan,
				(gstations[j].nchan+1)*sizeof(int)))) return;
		gstations[j].chan[gstations[j].nchan] = chan;
		gstations[j].nchan++;
	    }
	}
    }

    for(i = 0; i < (int)segs->size(); i++)
    {
	SegmentInfo *s = segs->at(i);
	if(i == 0 || strcmp(s->net, segs->at(i-1)->net))
	{
	    int jdate;
	    if(s->origin_time > NULL_TIME_CHECK) {
		DateTime dt;
		timeEpochToDate(s->origin_time, &dt);
		jdate = timeJDate(&dt);
	    }
	    else {
		jdate = -1;
	    }

	    fillStations(s->net, jdate);
	}
    }
}

void BasicSource::fillStations(const string &net, int jdate)
{
    const char **elements = NULL, **channels = NULL;
    int net_q = stringUpperToQuark(net);
    int num = networkStations(net, &elements);

    for(int i = 0; i < num; i++)
    {
	int j, sta_q = stringUpperToQuark(elements[i]);
	for(j = 0; j < num_gstations; j++) {
	    if(sta_q == gstations[j].sta) break;
	}
	if(j == num_gstations)
	{
	    CssSiteClass *site;
	    site = getSite(elements[i], jdate);
	    if(site == NULL) continue;
	    if(!(gstations = (GStation *)reallocWarn(gstations,
			(num_gstations+1)*sizeof(GStation)))) return;
	    gstations[j].sta = sta_q;
	    gstations[j].net = net_q;
	    gstations[j].refsta = site->refsta_quark;
	    gstations[j].lat = site->lat;
	    gstations[j].lon = site->lon;
	    gstations[j].elev = site->elev;
	    gstations[j].nchan = 1;
	    gstations[j].chan = NULL;
	    gstations[j].nchan = getChannels(elements[i], &channels);

	    if(gstations[j].nchan > 0)
	    {
		if(!(gstations[j].chan = (int *)reallocWarn(gstations[j].chan,
			gstations[j].nchan*sizeof(int)))) return;
		for(int l = 0; l < gstations[j].nchan; l++) {
		    gstations[j].chan[l] = stringUpperToQuark(channels[l]);
		}
		num_gstations++;
	    }
	    Free(channels);
	}
    }
    Free(elements);
}

int BasicSource::networkStations(const string &net, const char ***elements)
{
    static bool warn = true;
    cvector<CssAffiliationClass> *a = getAffiliationTable();

    if( !a || a->size() == 0 ) {
	/* Need affiliations
	 */
	getNetworkTables();

	a = getAffiliationTable();
	if( !a || a->size() == 0 ) {
	    if(warn) {
		cerr << "No affiliation table." << endl;
		warn = false;
	    }
	    return 0;
	}
    }
    int q_net = stringUpperToQuark(net);
    int i, num;
    for(i = num = 0; i < a->size(); i++) {
	if(q_net == a->at(i)->net_quark) num++;
    }
    if(!num) {
	*elements = NULL;
	return 0;
    }

    const char **elem = (const char **)mallocWarn(num*sizeof(char *));
    for(i = num = 0; i < a->size(); i++) {
	if(q_net == a->at(i)->net_quark) {
	    elem[num++] = a->at(i)->sta;
	}
    }
    *elements = elem;
    return num;
}

CssSiteClass * BasicSource::getSite(const string &sta, int jdate)
{
    static bool warn = true;
    cvector<CssSiteClass> *site = getSiteTable();

    if( !site || site->size() == 0 ) {
	/* Need sites
	 */
	getNetworkTables();

	site = getSiteTable();
	if( !site || site->size() == 0 ) {
	    if(warn) {
		cerr << "No site table." << endl;
		warn = false;
	    }
	    return NULL;
	}
    }

    int sta_q = stringUpperToQuark(sta);

    CssSiteClass *s = NULL;
    for(int i = 0; i < site->size(); i++) {
	if(sta_q == site->at(i)->sta_quark) {
	    if(jdate < 0) { s = site->at(i); break; }
	    if(site->at(i)->ondate <= jdate && (site->at(i)->offdate == -1 ||
		site->at(i)->offdate > jdate)) { s = site->at(i); break; }
	}
    }
    return s;
}

CssSitechanClass * BasicSource::getSitechan(const string &sta, const string &chan,
				int jdate)
{
    static bool warn = true;
    cvector<CssSitechanClass> *sitechan = getSitechanTable();

    if( !sitechan || sitechan->size() == 0 ) {
	/* Need sitechans
	 */
	getNetworkTables();

	sitechan = getSitechanTable();
	if( !sitechan || sitechan->size() == 0 ) {
	    if(warn) {
		cerr << "No sitechan table." << endl;
		warn = false;
	    }
	    return NULL;
	}
    }

    int sta_q = stringUpperToQuark(sta);
    int chan_q = stringUpperToQuark(chan);

    CssSitechanClass *s = NULL;
    for(int i = 0; i < sitechan->size(); i++) {
	if(sta_q == sitechan->at(i)->sta_quark
		&& chan_q == sitechan->at(i)->chan_quark)
	{
	    if(jdate < 0) { s = sitechan->at(i); break; }
	    if(sitechan->at(i)->ondate <= jdate &&
		(sitechan->at(i)->offdate == -1 ||
		sitechan->at(i)->offdate > jdate))
	    {
			s = sitechan->at(i); break;
	    }
	}
    }
    return s;
}

int BasicSource::getChannels(const string &sta, const char ***channels)
{
    int i, nchan;
    int q_sta = stringUpperToQuark(sta);
    const char **chan = NULL;
    static bool warn = true;

    cvector<CssSitechanClass> *v = getSitechanTable();
    if(!v || v->size() == 0) {
	/* Need sitechans
	 */
	getNetworkTables();

	v = getSitechanTable();
	if(!v || v->size() == 0) {
	    if(warn) {
		cerr << "No sitechan table." << endl;
		warn = false;
	    }
	    return 0;
	}
    }

    for(i = nchan = 0; i < v->size(); i++) {
 	if(q_sta == v->at(i)->sta_quark) nchan++;
    }
    if(!nchan) {
	*channels = NULL;
	return 0;
    }

    chan = (const char **)mallocWarn(nchan*sizeof(char *));

    for(i = nchan = 0; i < v->size(); i++) {
	if(q_sta == v->at(i)->sta_quark) {
	    chan[nchan++] = v->at(i)->chan;
	}
    }
    *channels = chan;
    return nchan;
}

string BasicSource::getNet(const string &sta)
{
    cvector<CssAffiliationClass> *a = getAffiliationTable();

    if( !a || a->size() == 0 ) {
	/* Need affiliations
	 */
	getNetworkTables();

	a = getAffiliationTable();
	if( !a || a->size() == 0 ) {
	    cerr << "No affiliation table." << endl;
	    return sta;
	}
    }

    int sta_q = stringUpperToQuark(sta);

    /* if sta is also a network name, return sta
     */
    int i;
    for(i = 0; i < a->size() && sta_q != a->at(i)->net_quark; i++);
    if(i < a->size()) {
	return sta;
    }

    const char *net = NULL;
    for(i = 0; i < a->size() && sta_q != a->at(i)->sta_quark; i++);
    if(i < a->size()) {
	net = a->at(i)->net;
    }

    return((net != NULL && net[0] != '\0' && strcmp(net, "-")) ?
		string(net) : sta);
}

bool BasicSource::changeTable(CssTableClass *old_table, CssTableClass *new_table)
{
    const char	*err_msg, *table_name, *file;
    char	error[MAXPATHLEN+50];
    FILE	*fp, *fp_backup;
    CssTableClass	*t;
    int		err;
    long	pos;

    file = quarkToString(old_table->getFile());

    if((fp = fopen(file, "r+")) == NULL) {
	snprintf(error, sizeof(error), "cannot write to: %s\n%s",
		file, strerror(errno));
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    table_name = old_table->getName();
    t = CssTableClass::createCssTable(table_name);
    t->setIds(old_table->getDC(), 1);

    pos = 0;
    while((err = t->read(fp, &err_msg)) != EOF) {
	if(*t == *old_table) break;
	pos = ftell(fp);
    }
    if(err) {
	fclose(fp);
	if(err == EOF) {
	    snprintf(error, sizeof(error), "cannot find record in: %s", file);
	}
	else {
	    snprintf(error, sizeof(error), "Error reading %s: %s",file,err_msg);
	}
	logErrorMsg(LOG_WARNING, error);
	delete t;
	return false;
    }
    if( (fp_backup = cssOpenBackupFile()) )
    {
	fprintf(fp_backup, "replace %s %ld\nrecord ", file, pos);

	if(t->write(fp_backup, &err_msg))
	{
	    string backup_file;
	    Application::getProperty("backup_file", backup_file);
	    snprintf(error, sizeof(error), "write error: %s",
			backup_file.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp_backup);
	    delete t;
	    return false;
	}
	fclose(fp_backup);
    }
    delete t;

    DateTime *dt = (DateTime *)new_table->memberAddress("lddate");
    if(dt && new_table->memberType("lddate") == CSS_LDDATE) {
	timeEpochToDate(timeGetEpoch(), dt);
    }

    fseek(fp, pos, 0);
    if(new_table->write(fp, &err_msg)) {
	snprintf(error, sizeof(error), "write error: %s", file);
	logErrorMsg(LOG_WARNING, error);
    }
    fclose(fp);

    return true;
}

bool BasicSource::addTable(CssTableClass *css)
{
    FILE *fp;
    char file[MAXPATHLEN+1];
    char error[MAXPATHLEN+50];

    snprintf(file, sizeof(file), "%s/%s.%s", quarkToString(css->getDir()),
	quarkToString(css->getPrefix()), css->getName());
    css->setFile(stringToQuark(file));

    /* don't use a+ first, since on linux, ftell(fp) will give 0 for
     * the file position, even if the file size is not zero.
     */
    if((fp = fopen(file, "r+")) == NULL && (fp = fopen(file, "a+")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nTable changes can not be saved.",
		file, strerror(errno));
	}
	else {
	    snprintf(error, MAXPATHLEN+50,
		"Cannot write to: %s\nTable changes can not be saved.", file);
	}
	logErrorMsg(LOG_WARNING, error);
	return false;
    }
    else
    {
	fseek(fp, 0, 2);

	css->setId(ftell(fp));
	css->setFileOffset(ftell(fp));
	DateTime *dt = (DateTime *)css->memberAddress("lddate");
	if(dt && css->memberType("lddate") == CSS_LDDATE) {
	    timeEpochToDate(timeGetEpoch(), dt);
	}

	const char *err_msg = NULL;
	if(css->write(fp, &err_msg)) {
	    snprintf(error, sizeof(error), "write error: %s", file);
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return false;
	}
	fclose(fp);
	css->setDataSource(this);
	Application::getApplication()->addTableCB(css);
    }
    return true;
}

GSegment * BasicSource::readSegment(CssWfdiscClass *wfdisc,
		const string &working_dir, double start_time, double end_time,
		int pts_wanted)
{
    bool T4 = false;
    int npts;
    double tbeg, tdel;
    float *data=NULL;

    // should put this check for 'T4' in cssio intead of here.
    // T4 means 4-byte floats in native byte order.
    // t4 means 4-byte floats in Sun byte order.
    if(!strcmp(wfdisc->datatype, "T4")) {
	T4 = true;
	wfdisc->datatype[0] = 't';
    }

    cssioReadData(wfdisc, working_dir, start_time, end_time, pts_wanted,
			&npts, &tbeg, &tdel, &data);

    if(T4) wfdisc->datatype[0] = 'T';

    if(npts > 0) {
	double calib = (wfdisc->calib != 0.) ? wfdisc->calib : 1.;
	if(T4 && flipBytes()) {
	    union {
		char    a[4];
		float   f;
	    } e1, e2;

	    for(int i = 0; i < npts; i++) {
		e1.f = data[i];
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		data[i] = e2.f;
            }
	}
	GSegment *s = new GSegment(data, npts, tbeg, tdel, calib,
					(double)wfdisc->calper);
	free(data);
	return s;
    }
    else {
	return NULL;
    }
}

static bool
flipBytes(void)
{
    union {
	char a[4];
	int i;
    } e;
    e.a[0] = 0; e.a[1] = 0;
    e.a[2] = 0; e.a[3] = 1;
    return (e.i == 1) ? false : true;
}

FILE * BasicSource::openBackupFile(void)
{
    return cssOpenBackupFile();
}

bool BasicSource::addAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude)
{
    int		len;
    char 	amp_file[MAXPATHLEN+1];
    char	prefix[MAXPATHLEN+1], error[MAXPATHLEN+100];
    const char	*err_msg, *sep, *dir, *pref;
    FILE	*fp;
    Password password = getpwuid(getuid());
    DateTime load_dt;

    dir = quarkToString(arrival->getDir());
    len = (dir != NULL) ? strlen(dir) : 0;
    sep = (len > 0 && dir[len-1] != '/') ? "/" : "";

    pref = quarkToString(arrival->getPrefix());

    prefix[0] = '\0';
    snprintf(prefix, sizeof(prefix), "%s%s%s", dir ? dir : "", sep,
                        pref ? pref : "");

    if(prefix[0] != '\0') {
	snprintf(amp_file, sizeof(amp_file), "%s.amplitude", prefix);
    }
    else {
	stringcpy(amp_file, "dot.amplitude", sizeof(amp_file));
    }
    /* don't use a+ first, since on linux, ftell(fp) will give 0 for
     * the file position, even if the file size is not zero.
     */
    if((fp = fopen(amp_file, "r+")) == NULL
                && (fp = fopen(amp_file, "a+")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\n%s\nNo amplitude changes will be saved.",
		amp_file, strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error),
		"Cannot write to: %s\nNo amplitude changes will be saved.",
		amp_file);
	}
	logErrorMsg(LOG_WARNING, error);
	return false;
    }
    fseek(fp, 0, 2);
    amplitude->setDir(arrival->getDir());
    amplitude->setPrefix(arrival->getPrefix());
    amplitude->setFile(stringToQuark(amp_file));

    amplitude->ampid = cssGetNextId(arrival, "ampid",
				quarkToString(arrival->getDC()));
    amplitude->setIds(amplitude->ampid, arrival->getDC());

    timeEpochToDate(timeGetEpoch(), &load_dt);
    amplitude->lddate = load_dt;

    stringcpy(amplitude->auth, password->pw_name, sizeof(amplitude->auth));

    if(amplitude->write(fp, &err_msg))
    {
	snprintf(error, sizeof(error), "write error: %s", amp_file);
	logErrorMsg(LOG_WARNING, error);
    }
    fclose(fp);

    amplitude->setDataSource(this);

    Application::getApplication()->addTableCB(amplitude);

    return true;
}

#include "Response.h"
#include "ResponseFile.h"
#include "DataMethod.h"

/** Get the instrument response for a waveform.
 *  @param[in] ts a GTimeSeries object.
 *  @param[in] print_err if true, warn if no response is found.
 *  @returns a vector of Response objects, one for each of the components of 
 *	the cascaded response. Returns NULL if ts->length() <= 2 or the
 *	response cannot be found.
 */
vector<Response *> * BasicSource::getResponse(GTimeSeries *ts, bool print_err)
{
    DataSource *ds;
    if( (ds = ts->getDataSource()) ) {
	return ds->channelResponse(ts, print_err);
    }
    return getInstrumentResponse(ts, print_err);
}

vector<Response *> * BasicSource::getInstrumentResponse(GTimeSeries *ts,
			bool print_err)
{
    string sta, chan;
    string file, name;
    const char *err_msg = NULL;
    ResponseFile *rf;
    int inid = -1;

    if(ts->length() <= 2 || ts->no_response) return NULL;

    if((int)ts->response.size() > 0) return &ts->response;

/* ****** XXXXXX   change Response() to read sensor only ***********/

    if(!getResponseFile(ts, name, file, &inid, &err_msg))
    {
	ts->getSta(sta);
	ts->getChan(chan);

	// only complain one time per sta/chan

	if(print_err && newMissingResp(stringUpperToQuark(sta), 
					   stringUpperToQuark(chan)))
	{
	    char error[MAXPATHLEN+100];
	    if(err_msg != NULL && (int)strlen(err_msg) > 0)
	    {
		snprintf(error, sizeof(error),
			"%s/%s: no response information found.\n%s",
			sta.c_str(), chan.c_str(), err_msg);
	    }
	    else {
		snprintf(error, sizeof(error),
			"%s/%s: no response information found.",
			sta.c_str(), chan.c_str());
	    }
	    logErrorMsg(LOG_WARNING, error);
	    GError::setMessage(error);
	}
	ts->no_response = true;
	ts->inid = -1;
	return NULL;
    }
    if((rf = ResponseFile::readFile(file, print_err)) == NULL)
    {
	return NULL;
    }
    ts->response_file = rf;
    ts->inid = inid;

    ts->response.clear();

    for(int i = 0; i < (int)rf->responses.size(); i++)
    {
	Response *r = rf->responses[i];
	if(!r->source.compare("theoretical") || !r->source.compare("geotech"))
	{
	    ts->response.push_back(r);
	}
    }
    if((int)ts->response.size() > 0) {
	return &ts->response;
    }
    return NULL;
}

/** Get the instrument name and the response filename for a GTimeSeries.
 *  @param[in] ts a GTimeSeries object.
 *  @param[out] insname the instrument name for the waveform.
 *  @param[in] insname_len the size of insname.
 *  @param[out] file the response file for the waveform.
 *  @param[in] file_len the size of file.
 *  @param[out] inid the inid from the instrument table
 *  @param[out] err_msg on error return, *err_msg is set to a static character
 *      string error message.
 *  @return true for success, false for an error.
 */
bool BasicSource::getResponseFile(GTimeSeries *ts, string &insname,
		string &file, int *inid, const char **err_msg)
{
    string sta, chan;

    if( ts->waveform_io )
    {
        for(int i = 0; i < (int)ts->waveform_io->wp.size(); i++) {
	    const char *dir = quarkToString(ts->waveform_io->wp[i].dir);
	    const char *prefix = quarkToString(ts->waveform_io->wp[i].prefix);
	    sta.assign(ts->waveform_io->wp[i].wf.sta);
	    chan.assign(ts->waveform_io->wp[i].wf.chan);
	    if( ResponseFile::getFile(sta, chan, ts->waveform_io->wp[i].wf.time,
			ts->waveform_io->wp[i].wf.endtime, dir, prefix, insname,
			file, inid, err_msg) )
	    {
		return true;
	    }
	    else {
		// checkForBeam takes a long time so don't call it if not needed
		checkForBeam(ts, sta, chan);
		return ResponseFile::getFile(sta, chan,
			ts->waveform_io->wp[i].wf.time,
			ts->waveform_io->wp[i].wf.endtime,
			dir, prefix, insname, file, inid, err_msg);
	    }
	}
    }
    if(ts->getSta(sta) && ts->getChan(chan))
    {
        if( ResponseFile::getFile(sta, chan, ts->tbeg(), ts->tend(), insname,
				file, inid, err_msg) )
	{
	    return true;
	}
	else {
	    // checkForBeam takes a long time so don't call it if not needed
	    checkForBeam(ts, sta, chan);
	    return ResponseFile::getFile(sta, chan, ts->tbeg(), ts->tend(),
					insname, file, inid, err_msg);
	}
    }
    return false;
}

bool BasicSource::checkForBeam(GTimeSeries *ts, string &sta, string &chan)
{
    bool ret = false;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    Beam *gbeam = new Beam(recipe_dir, recipe_dir2);

    // If the sta/chan is a beam, use the sta/chan of the first element
    if( gbeam->beamRecipe(sta, chan, recipe) &&
	gbeam->getGroup(recipe, beam_sta) > 0)
    {
	sta.assign(beam_sta[0].sta);
	stringToUpper(sta);
	chan.assign(beam_sta[0].chan);
	ret = true;
    }
    else if((int)ts->beam_elements.size() > 0)
    {
	sta.assign(ts->beam_elements[0].sta);
	stringToUpper(sta);
	chan.assign(ts->beam_elements[0].chan);
	ret = true;
    }
    else if(!chan.compare(0, 2, "bm") && gbeam->beamRecipe(sta, "cb", recipe)			&& gbeam->getGroup(recipe, beam_sta) > 0)
    {
	sta.assign(beam_sta[0].sta);
	stringToUpper(sta);
	chan.assign(beam_sta[0].chan);
	ret = true;
    }
    delete gbeam;
    return ret;
}

/** Convert an amplitude from counts to nanometers.
 *  @param[in] ts a GTimeSeries object.
 *  @param[in] s a GSegment member of ts for the calib.
 *  @param[in] ampcts the amplitude in counts.
 *  @param[in] period the period in seconds.
 *  @param[out] ampnms the amplitude in nanometers.
 *  @param[in] warn if true, warn if the instrument response cannot be found.
 *  @returns true for success, returns false if the response cannot be found,
 *	or the calper is <= 0.
 */
bool BasicSource::cts2nms(GTimeSeries *ts, GSegment *s, double ampcts,
			double period, double *ampnms, bool warn)
{
    double real, imag, den, amp;
    double freq, calper, calib;
    gvector<DataMethod *> *methods;
    vector<Response*> *rsp;
    ConvolveData *con;

    if( !(rsp = getResponse(ts, warn)) ) return false;

    freq = 1./period;

    if((calper = s->initialCalper()) < 0.) return false;
    calib = s->initialCalib();

    Response::compute(rsp, freq, calib, calper, &real, &imag);

    den = sqrt(real*real + imag*imag);
    if( den == 0.)  {
	return false;
    }

    *ampnms = ampcts/den;

    methods = ts->dataMethods();

    for(int i = 0; i < (int)methods->size(); i++) {
	if( (con = methods->at(i)->getConvolveDataInstance()) ) {
	    rsp = &con->responses;
	    calib = con->getCalib();
	    calper = con->getCalper();
	    Response::compute(rsp, freq, calib, calper, &real, &imag);
	    amp = sqrt(real*real + imag*imag);
	    if(con->getDirection() == 1) {
		if(amp != 0.) *ampnms /= amp;
	    }
	    else {
		*ampnms *= amp;
	    }
	}
    }
    delete methods;

    return true;
}

/** Convert an amplitude from nanometers to counts.
 *  @param[in] ts a GTimeSeries object.
 *  @param[in] s a GSegment member of ts for the calib.
 *  @param[in] ampnms the amplitude in nanometers.
 *  @param[in] period the period in seconds.
 *  @param[out] ampcts the amplitude in counts.
 *  @param[in] warn if true, warn if the instrument response cannot be found.
 *  @returns true for success, returns false if the response cannot be found,
 *	or the calper is <= 0.
 */
bool BasicSource::nms2cts(GTimeSeries *ts, GSegment *s, double ampnms,
			double period, double *ampcts, bool warn)
{
    double real, imag, fac;
    double freq, calper, calib;
    gvector<DataMethod *> *methods;
    vector<Response *> *rsp;
    ConvolveData *con;

    if( !(rsp = getResponse(ts, warn)) ) return false;

    if(ts->getMethod("ConvolveData")) return false;

    freq = 1./period;
    if((calper = s->initialCalper()) <= 0.) return false;
    calib = s->initialCalib();

    Response::compute(rsp, freq, calib, calper, &real, &imag);

    fac = sqrt(real*real + imag*imag);

    *ampcts = ampnms*fac;

    methods = ts->dataMethods();

    for(int i = 0; i < (int)methods->size(); i++) {
	if( (con = methods->at(i)->getConvolveDataInstance()) ) {
	    rsp = &con->responses;
	    calib = con->getCalib();
	    calper = con->getCalper();
	    Response::compute(rsp, freq, calib, calper, &real, &imag);
	    fac = sqrt(real*real + imag*imag);
	    if(con->getDirection() == 1) {
		*ampcts = *ampcts * fac;
	    }
	    else if(fac != 0.) {
		*ampcts = *ampcts / fac;
	    }
	}
    }
    delete methods;

    return true;
}

bool BasicSource::newMissingResp(int sta, int chan)
{
    typedef struct
    {
	int	sta;
	int	nchan;
	int	*chan;
    } StaLIST;

    static StaLIST *stalist = NULL;
    static int n = 0;

    for(int i = 0; i < n; i++) if (sta == stalist[i].sta)
    {
	for(int j = 0; j < stalist[i].nchan; j++)
	    if(chan == stalist[i].chan[j])
	{
	    return false;
	}

	stalist[i].chan = (int *) realloc(stalist[i].chan,
				(stalist[i].nchan + 1) * sizeof(int));

	stalist[i].chan[stalist[i].nchan] = chan;
	stalist[i].nchan++;
	return true;
    }

    if(n == 0) {
	stalist = (StaLIST *) malloc(sizeof(StaLIST));
    }
    else {
	stalist = (StaLIST *) realloc(stalist, (n+1) * sizeof(StaLIST));
    }

    stalist[n].chan = (int *) malloc(sizeof(int));

    stalist[n].sta = sta;
    stalist[n].nchan = 1;
    stalist[n].chan[0] = chan;
    n++;

    return true;
}


// DataSource routine
CssTableClass * BasicSource::createTable(CssTableClass *table, const string &table_name,
                const string &id_name, Password password)
{
    table->setDataSource(this);
    CssTableClass *t = cssCreateTable(table, table_name, id_name, password);
    if(t) {
	Application::getApplication()->addTableCB(t);
    }
    return t;
}

// DataSource routine
bool BasicSource::deleteTable(Component *caller, CssTableClass *table,
		const string &table_name)
{
    return cssDeleteTable(caller, table);
}

// DataSource routine
bool BasicSource::addArrival(CssArrivalClass *arrival, GTimeSeries *ts,
			Password password, long max_arid)
{
    arrival->setDataSource(this);
    bool ret = cssAddArrival(arrival, ts, password, "", max_arid);
    if(ret) {
	Application::getApplication()->addTableCB(arrival);
    }
    return ret;
}

// DataSource routine
bool BasicSource::changeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int type)
{
    return cssChangeArrival(arrival, ts, type);
}

// DataSource routine
bool BasicSource::deleteArrival(Component *caller, CssArrivalClass *arrival,
	cvector<CssAssocClass> &assocs, cvector<CssAmplitudeClass> &amps,
	cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
	cvector<CssInfraFeaturesClass> &infras)
{
    return cssDeleteArrival(caller, arrival);
}

// DataSource routine
bool BasicSource::originCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *table, Password password)
{
    origin->setDataSource(this);
    bool ret = cssCreateOrigin(origin, origerr, table, password);
    if(ret) {
	Application::getApplication()->addTableCB(origin);
	Application::getApplication()->addTableCB(origerr);
    }
    return ret;
}

// DataSource routine
bool BasicSource::deleteOrigin(Component *caller, CssOriginClass *origin,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &ftags, cvector<CssNetmagClass> &netmags,
		cvector<CssStamagClass> &stamags)
{
    return cssDeleteOrigin(caller, origin);
}

// DataSource routine
bool BasicSource::addAssoc(CssAssocClass *assoc)
{
    assoc->setDataSource(this);
    bool ret = cssAddAssoc(assoc);
    if(ret) {
	Application::getApplication()->addTableCB(assoc);
    }
    return ret;
}

// DataSource routine
CssOrigerrClass * BasicSource::addOrigerr(CssOriginClass *origin)
{
    CssOrigerrClass *origerr = cssCreateOrigerr(origin);
    if(origerr) {
	origerr->setDataSource(this);
	Application::getApplication()->addTableCB(origerr);
    }
    return origerr;
}

// DataSource routine
bool BasicSource::addNetmag(CssNetmagClass *netmag, CssTableClass *table)
{
    netmag->setDataSource(this);
    bool ret = cssAddNetmag(netmag, table);
    if(ret) {
	Application::getApplication()->addTableCB(netmag);
    }
    return ret;
}

// DataSource routine
bool BasicSource::addStamag(CssStamagClass *stamag, CssTableClass *table)
{
    stamag->setDataSource(this);
    bool ret = cssAddStamag(stamag, table);
    if(ret) {
	Application::getApplication()->addTableCB(stamag);
    }
    return ret;
}

bool BasicSource::startBackup(void)
{
    FILE *fp_backup;

    if((fp_backup = cssOpenBackupFile()) == NULL) {
	return false;
    }
    fprintf(fp_backup, "#backup\n");
    fclose(fp_backup);
    return true;
}

bool BasicSource::undoFileModification(void)
{
    string backup_file;

    if(!Application::getProperty("backup_file", backup_file)) return false;

    int ret = cssUndoFileModification(backup_file.c_str());
    return (ret == 1) ? true : false;
}

/** Undo an table deletion.
 */
bool BasicSource::undoDeleteTable(CssTableClass *table)
{
    string backup_file;

    if(!Application::getProperty("backup_file", backup_file)) return false;

    bool ret = cssUndoFileModification(backup_file.c_str());
    return ret;
}

bool BasicSource::isArrayChannel(const string &net, const string &sta,
		const string &chan)
{
    int i, nsta;
    vector<BeamSta> beam_sta;
    Beam *gbeam = new Beam(recipe_dir, recipe_dir2);
    BeamRecipe recipe;

    if( !gbeam->beamRecipe(net, "cb", recipe) ) {
	delete gbeam;
	return false;
    }

    nsta = gbeam->getGroup(recipe, beam_sta);
    delete gbeam;

    for(i = 0; i < nsta; i++) {
	if(!strcasecmp(sta.c_str(), beam_sta[i].sta) &&
	    DataSource::compareChan(chan, beam_sta[i].chan)) break;
    }

    return (i < nsta) ? true : false;
}

bool BasicSource::isArray(const string &net)
{
    Beam *gbeam = new Beam(recipe_dir, recipe_dir2);
    BeamRecipe recipe;
    bool ret = gbeam->beamRecipe(net, "cb", recipe);
    delete gbeam;
    return ret;
}

bool BasicSource::isThreeComponentChannel(const string &sta, const string &chan)
{
    return false;
}
