/** \file TQDataSource.cpp
 *  \brief Defines the TableQuery DataSource interface.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <errno.h>
using namespace std;

#include "motif++/MotifClasses.h"
#include "libgio.h"
#include "libgx++.h"
#include "TableQuery.h"
#include "OpenDB.h"
#include "IIRFilter.h"
#include "CalibData.h"
#include "TaperData.h"
#include "gobject++/GSourceInfo.h"
#include "gobject++/CssTables.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
static int sort_by_net_time(const void *A, const void *B);
}

#define PERIOD(a) (a[0] != '\0' ? "." : "")

#define MAX_ARRIVAL_SEPARATION  300
#define MAX_WAVEFORM_LENGTH     1800


// DataSource routine
gvector<SegmentInfo *> * TableQuery::getSegmentList(void)
{
    if(last_display_waveforms) {
	return BasicSource::getSegmentList();
    }

    // display arrivals only
    double tmin, tmax, time_before, time_after;
    gvector<SegmentInfo *> *segs;
    int i, j, k, l;
    cvector<CssArrivalClass> a;
    cvector<CssOriginClass> origins;
    string tab_name = getTopTabName();

    if(!open_db->arrivalTimeBefore(&time_before)) {
	showWarning("Cannot interpret time before.");
	return NULL;
    }
    if(!open_db->arrivalTimeAfter(&time_after)) {
	showWarning("Cannot interpret time after.");
	return NULL;
    }

    if( !tab_name.compare(cssArrival) ) {
	getSelectedTableRecords(cssArrival, a, true);
	getTableRecords(cssOrigin, origins);
    }
    else {
	getTableRecords(cssArrival, a, true);
	getSelectedTableRecords(cssOrigin, origins, false);
    }
    if(!a.size()) {
	return NULL;
    }

    getArrivalNetworks(a);

    a.sort(sort_by_net_time);

    segs = new gvector<SegmentInfo *>;

    for(i = 0; i < a.size(); i++)
    {
	for(j = i+1; j < a.size(); j++)
	{
	    if( a[j]->net_quark != a[j-1]->net_quark ||
		a[j]->time - a[j-1]->time > MAX_ARRIVAL_SEPARATION ||
		a[j]->time - a[i]->time > MAX_WAVEFORM_LENGTH) break;
	}
	tmin = a[i]->time - time_before;
	tmax = a[j-1]->time + time_after;

	for(k = i; k < j; k++)
	{
	    for(l = i; l < k; l++) {
		if(a[l]->sta_quark == a[k]->sta_quark &&
		   a[l]->chan_quark == a[k]->chan_quark) break;
	    }
	    if(l == k)
	    {
		gvector<SegmentInfo *> *v;
		SegmentInfo *s = new SegmentInfo();
		segs->push_back(s);
		s->id = a[k]->arid;
		stringcpy(s->net,quarkToString(a[k]->net_quark),sizeof(s->net));
		stringcpy(s->sta, a[k]->sta, sizeof(s->sta));
		stringcpy(s->chan, a[k]->chan, sizeof(s->chan));
//		changeStaChan(s->sta, sizeof(s->sta), s->chan, sizeof(s->chan));
		s->start = tmin;
		s->end = tmax;
		s->jdate = timeEpochToJDate(s->start);
		s->nsamp = 2;
		s->selected = true;
		s->setWfdisc(new CssWfdiscClass());
		a[i]->copySourceTo(s->wfdisc(), 1);
		s->wfdisc()->chanid = a[k]->chanid;
		if((v = (gvector<SegmentInfo *> *)a[k]->getValue("elements")))
		{
		    for(l = 0; l < v->size(); l++) {
			s->array_elements.add(v->at(l));
		    }
		}
	    }
	}
	i = j-1;
    }

    getNetworks(segs);

    getSites(segs);

    cvector<CssSitechanClass> *sitechans =
		(cvector<CssSitechanClass> *)getTableRecords(cssSitechan, false);

    if(sitechans && sitechans->size())
    {
	// when the sta == net, but the chan == a real channel (bz, etc),
	// then find the sta name.
	for(i = 0; i < (int)segs->size(); i++)
	{
	    SegmentInfo *s = segs->at(i);
	    if(!strcasecmp(s->net, s->sta))
	    {
		int chan_quark = stringUpperToQuark(s->chan);
		const char **elements=NULL;
		int num_elements = networkStations(s->net, &elements);
		
		for(j = 0; j < num_elements; j++)
		{
		    int sta_quark = stringUpperToQuark(elements[j]);
		    for(k = 0; k < sitechans->size(); k++)
		    {
			CssSitechanClass *sc = sitechans->at(k);
			if(sta_quark == sc->sta_quark &&
			   chan_quark == sc->chan_quark)
			{
			    stringcpy(s->sta, sc->sta, sizeof(s->sta));
			    break;
			}
		    }
		    if(k < sitechans->size()) break;
		}
		Free(elements);
	    }
	}
    }

    getSitechans(segs);

    cvector<CssAssocClass> *assocs = 
		(cvector<CssAssocClass> *)getTableRecords(cssAssoc, false);

    if(origins.size() && assocs && assocs->size())
    {
	for(i = 0; i < (int)segs->size(); i++)
	{
	    SegmentInfo *s = segs->at(i);
	    long arid = s->id;
	    for(j = 0; j < assocs->size() && assocs->at(j)->arid != arid; j++);
	    if(j < assocs->size())
	    {
		CssAssocClass *assoc = assocs->at(j);
		for(k = 0; k < origins.size() &&
		    origins[k]->orid != assoc->orid; k++);
		if(k < origins.size())
		{
		    CssOriginClass *origin = origins[k];
		    double delta, az, baz;
		    deltaz(origin->lat, origin->lon, s->station_lat,
				s->station_lon, &delta, &az, &baz);
		    s->origin_id = origin->orid;
		    s->origin_delta = delta;
	    	    s->origin_depth = origin->depth;
	    	    s->origin_time = origin->time;
	    	    s->origin_lat = origin->lat;
	    	    s->origin_lon = origin->lon;
	    	    s->origin_azimuth = az;
		}
	    }
	}
    }

    return segs;
}

void TableQuery::getArrivalNetworks(cvector<CssArrivalClass> &arrivals)
{
    cvector<CssAffiliationClass> *aff =
	(cvector<CssAffiliationClass> *)getTableRecords(cssAffiliation, false);
    
    if( !aff || !aff->size() ) {
	showWarning("No affiliations found.");
	return;
    }
    int null = stringToQuark("");

    for(int i = 0; i < arrivals.size(); i++)
    {
	CssArrivalClass *a = arrivals[i];
	int quark = a->sta_quark;
	a->net_quark = null;
	int j;
	for(j = 0; j < aff->size() && quark != aff->at(j)->net_quark; j++);
	if(j < aff->size()) {
	    a->net_quark = aff->at(j)->net_quark;
	}
	else {
	    for(j = 0; j < aff->size() && quark != aff->at(j)->sta_quark; j++);
	    if(j < aff->size()) {
		a->net_quark = aff->at(j)->net_quark;
	    }
	    else {
		a->net_quark = a->sta_quark;
	    }
	}
    }
}

static int
sort_by_net_time(const void *A, const void *B)
{
    register int dif;
    CssArrivalClass *a = *(CssArrivalClass **)A;
    CssArrivalClass *b = *(CssArrivalClass **)B;

    if((dif = a->net_quark - b->net_quark) != 0) return dif;

    if(a->time == b->time) return 0;
    return (a->time > b->time) ? 1 : -1;
}

// DataSource routine
bool TableQuery::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    if(last_display_waveforms) {
	return BasicSource::makeTimeSeries(s, tbeg, tend, pts, ts, err_msg);
    }

    // display arrivals only
    *err_msg = NULL;

    if( !s ) return false;

    error_msg[0] = '\0';

    if(*ts == NULL)
    {
	*ts = new GTimeSeries();
	(*ts)->source_info.setSource(s->wfdisc());
	(*ts)->array_elements.load(s->array_elements);
    }
    float data[2] = {0., 0.};
    double tdel = tend - tbeg;

    GSegment *segment = new GSegment(data, 2, tbeg, tdel, 1., 1.);

    if(segment == (GSegment *)NULL) return false;

    (*ts)->addSegment(segment);

    return true;
}

// DataSource routine
bool TableQuery::readWaveform(Waveform *cd, WaveformPlot *wp)
{
    if(cd->length() > 2) {
	return BasicSource::reread(cd->ts);
    }
    else {
	int odbc = stringToQuark("odbc");
	int ffdb = stringToQuark("ffdb");
	int css_format = stringToQuark("css");
	GSourceInfo *s = &cd->ts->source_info;

	if(s->format == odbc || s->format == ffdb || s->format == css_format)
	{
	    return readWaveform(s, cd, wp);
	}
    }
    return false;
}

bool TableQuery::readWaveform(GSourceInfo *s, Waveform *cd,
		WaveformPlot *wp)
{
    char wfdisc_query[1000], assoc_query[1000], origin_query[1000];
    string tableName, affiliation_table;
    int i;
    double tbeg, tend;
    BeamRecipe recipe;
    vector<BeamSta> beam_sta;
    cvector<CssArrivalClass> arrivals;
    cvector<CssWfdiscClass> wfdiscs;
    cvector<CssOriginClass> origins;

#ifdef HAVE_LIBODBC
    SQLHDBC hdbc;
#endif

    tableName = open_db->getMapping(cssWfdisc);
    affiliation_table = open_db->getMapping("affiliation");

    tbeg = cd->tbeg();
    tend = cd->tend();

    if(s->format != stringToQuark("odbc")) {
	snprintf(wfdisc_query, sizeof(wfdisc_query), "select * from wfdisc");
    }
    else {
	snprintf(wfdisc_query, sizeof(wfdisc_query),
"select distinct w.* from %s%s%s a, %s%s%s w where a.net='%s' and w.sta=a.sta \
and endtime > %.2f and time between %.2f - %.2f and %.2f",
	s->getAccount(), PERIOD(s->getAccount()), affiliation_table.c_str(),
	s->getAccount(), PERIOD(s->getAccount()), tableName.c_str(), cd->sta(),
	tbeg, tbeg, open_db->queryBuffer(), tend);
    }

    wp->getArrivalsOnWaveform(cd, arrivals);

    assoc_query[0] = '\0';
    if(arrivals.size() > 0) {
	for(i=0; i < arrivals.size() && !wp->isSelected(arrivals[i]); i++);
	if(i == arrivals.size()) i = 0;
	tableName = open_db->getMapping(cssAssoc);
	if(s->format != stringToQuark("odbc")) {
	    snprintf(assoc_query, sizeof(assoc_query),
		"select * from assoc where arid=%ld", arrivals[i]->arid);
	}
	else {
	    snprintf(assoc_query, sizeof(assoc_query),
		"select * from %s%s%s where arid=%ld",
		s->getAccount(), PERIOD(s->getAccount()), tableName.c_str(),
		arrivals[i]->arid);
	}
    }

    if(s->format == stringToQuark("odbc"))
    {
#ifdef HAVE_LIBODBC
	cvector<CssAssocClass> assocs;
	int auto_commit = 0;
	if((hdbc = ODBCConnect(s->getODBCDataSource(), s->getUser(),
		s->getPassword(), auto_commit)) == NULL) {
	    showWarning(ODBCErrMsg());
            return false;
	}
	if(ODBCQueryTable(hdbc, wfdisc_query, cssWfdisc, wfdiscs)) {
	    showWarning(ODBCErrMsg());
	    ODBCDisconnect(hdbc);
	    return false;
	}
	if(ODBCQueryTable(hdbc, assoc_query, cssAssoc, assocs)) {
	    showWarning(ODBCErrMsg());
	    ODBCDisconnect(hdbc);
	    return false;
	}
	if(assocs.size() > 0) {
	    assocs[0]->setDataSource(this);
	    wp->putTable(assocs[0]);
	    tableName = open_db->getMapping(cssOrigin);
	    snprintf(origin_query, sizeof(origin_query),
		"select * from %s%s%s where orid=%ld", s->getAccount(),
		PERIOD(s->getAccount()), tableName.c_str(), assocs[0]->orid);

	    if(ODBCQueryTable(hdbc, origin_query, cssOrigin, origins)) {
		showWarning(ODBCErrMsg());
		return false;
	    }
	    if(origins.size() > 0) {
		origins[0]->setDataSource(this);
		wp->putTable(origins[0]);
	    }
	}

	ODBCDisconnect(hdbc);
#endif
    }
    else if(s->format == stringToQuark("ffdb"))
    {
	FFDatabase *ffdb;
	cvector<CssAssocClass> assocs;

	if( !(ffdb = FFDatabase::FFDBOpen(s->getUser(), s->getPassword(),
			s->getDirStruct(), s->directory_duration)) )
//			open_db->directoryStructure(),
//			open_db->directoryDuration())) == NULL)
	{
	    showWarning(FFDatabase::FFDBErrMsg());
	    return false;
	}
	ffdb->setDefaultAuthor(s->getAccount());

	if( !ffdb->queryTable(wfdisc_query, cssWfdisc, &wfdiscs))
	{
	    showWarning(FFDatabase::FFDBErrMsg());
	    delete ffdb;
	    return false;
	}
	if(!ffdb->queryTable(assoc_query, cssAssoc, &assocs)) {
	    showWarning(FFDatabase::FFDBErrMsg());
	    delete ffdb;
	    return false;
	}
	if(assocs.size() > 0) {
	    assocs[0]->setDataSource(this);
	    wp->putTable(assocs[0]);
	    tableName = open_db->getMapping(cssOrigin);
	    snprintf(origin_query, sizeof(origin_query),
		"select * from %s%s%s where orid=%ld",
		s->getAccount(), PERIOD(s->getAccount()), tableName.c_str(),
		assocs[0]->orid);

	    if(!ffdb->queryTable(origin_query, cssOrigin, &origins)) {
		showWarning(FFDatabase::FFDBErrMsg());
		delete ffdb;
		return false;
	    }
	    if(origins.size() > 0) {
		origins[0]->setDataSource(this);
		wp->putTable(origins[0]);
	    }
	}

	delete ffdb;
    }
    else if(s->format == stringToQuark("css"))
    {
	cvector<CssAssocClass> assocs;
	char path[MAXPATHLEN+1];
	FFDatabase *ffdb;

	snprintf(path, sizeof(path), "%s/%s", s->getDir(), s->getPrefix());

	if((ffdb = FFDatabase::FFDBOpenPrefix(path)) == NULL)
	{
	    showWarning(FFDatabase::FFDBErrMsg());
	    return false;
	}

	if(!ffdb->queryPrefix(wfdisc_query, cssWfdisc, &wfdiscs))
	{
	    showWarning(FFDatabase::FFDBErrMsg());
	    delete ffdb;
	    return false;
	}
	if(!ffdb->queryPrefix(assoc_query, cssAssoc, &assocs)) {
	    showWarning(FFDatabase::FFDBErrMsg());
	    delete ffdb;
	    return false;
	}
	if(assocs.size() > 0) {
	    assocs[0]->setDataSource(this);
	    wp->putTable(assocs[0]);
	    snprintf(origin_query, sizeof(origin_query),
		"select * from origin where orid=%ld", assocs[0]->orid);

	    if(!ffdb->queryPrefix(origin_query, cssOrigin, &origins)) {
		showWarning(FFDatabase::FFDBErrMsg());
		delete ffdb;
		return false;
	    }
	    if(origins.size() > 0) {
		origins[0]->setDataSource(this);
		wp->putTable(origins[0]);
	    }
	}
	delete ffdb;
    }
    if(wfdiscs.size() == 0) {
	showWarning("No waveform data found for %s", cd->sta());
	return false;
    }
    for(i = 0; i < wfdiscs.size(); i++) {
        wfdiscs[i]->setDataSource(this);
    }

    CssOriginClass *o = (origins.size() > 0) ? origins[0] : NULL;

    /* check if this is a beam
     */
    if( gbeam->beamRecipe(cd->net(), cd->chan(), recipe) ) {
	if( gbeam->getGroup(recipe, beam_sta) <= 0 ) {
	    showWarning("Error getting beam group:\n%s", GError::getMessage());
	    return false;
	}
	loadBeam(cd, wp, tbeg, tend, wfdiscs, recipe, beam_sta, o);
    }
    else {
	loadOneWaveform(cd, wp, tbeg, tend, wfdiscs, o);
    }

    return true;
}

void TableQuery::loadOneWaveform(Waveform *cd, CPlotClass *cp,double tbeg,
		double tend, cvector<CssWfdiscClass> &wf, CssOriginClass *o)
{
    int i;
    GTimeSeries *ts;
    cvector<CssWfdiscClass> v;

    if(wf.size() == 0) return;

    v.push_back(wf[0]);

    for(i = 1; i < wf.size(); i++) {
	if(!strcmp(wf[0]->sta,wf[i]->sta) && !strcmp(wf[0]->chan,wf[i]->chan)) {
	    v.push_back(wf[i]);
	}
    }
    ts = readTimeSeries(v, tbeg, tend);

    if(ts != NULL) {
	ts->setDataSource(this);
//	applyCalib(widget, ts);
//	stringcpy(cd->sta, wf[0]->sta, sizeof(cd->sta));
	cd->ts->setSta(wf[0]->sta);
	Gobject *g = cd->ts->getValue("elements");
	if(g) ts->putValue("elements", g);
	ts->copyInfo(*cd->ts);
	cd->ts = ts;
	cp->modifyWaveform(cd);

	if(o) {
	    CssWftagClass *wftag = new CssWftagClass();
	    wftag->wfid = wf[0]->wfid;
	    wftag->tagid = o->orid;
	    stringcpy(wftag->tagname, "orid", sizeof(wftag->tagname));
	    cp->putTable(wftag);
	}
    }
}

void TableQuery::loadBeam(Waveform *cd, WaveformPlot *wp, double tbeg,
	double tend, cvector<CssWfdiscClass> &wfdiscs, BeamRecipe &recipe,
	vector<BeamSta> &beam_sta, CssOriginClass *o)
{
    char missing[500];
    int i, j, k, n_old, n_new, n;
    long wfid = 0;
    GTimeSeries *ts;
    gvector<Waveform *> w_old, w_new, ws;
    vector<double> t_lags, weights;
    double delta, daz, baz, lat = -999., lon = -999.;
    gvector<SegmentInfo *> *segs;

    if(wfdiscs.size() == 0) return;

    segs = new gvector<SegmentInfo *>;

    for(i = 0; i < wfdiscs.size(); i++)
    {
	CssWfdiscClass *wf = wfdiscs[i];
	if(i == 0) wfid = wf->wfid;
	for(j = 0; j < (int)beam_sta.size(); j++) {
	    if( !strcmp(beam_sta[j].sta, wf->sta) &&
		compareChan(beam_sta[j].chan, wf->chan)) break;
	}
	if(j < (int)beam_sta.size())
	{
	    for(k = 0; k < segs->size(); k++) {
		SegmentInfo *s = segs->at(k);
		if(!strcmp(s->sta, wf->sta) && compareChan(s->chan, wf->chan)
			&& s->wfdisc()->time == wf->time) break;
	    }
	    if(k < segs->size()) continue; /* prevent duplicates */

	    SegmentInfo *s = new SegmentInfo();

	    s->id = wf->wfid;
	    stringcpy(s->sta, wf->sta, sizeof(s->sta));
	    stringcpy(s->chan, wf->chan, sizeof(s->chan));
	    if(!sourceIsODBC()) {
		s->path = wf->getFile();
		s->file_order = wf->filePosition();
//		gnetChangeStaChan(s->sta, sizeof(s->sta), s->chan,
//				sizeof(s->chan));
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
	    segs->add(s);
	}
    }

    getNetworks(segs);
    getSites(segs);
    getSitechans(segs);

    n_old = wp->getWaveforms(w_old, false);

    wp->inputData(segs, this, false);

    delete segs;

    n_new = wp->getWaveforms(w_new, false);

    for(j = 0; j < (int)beam_sta.size(); j++) weights.push_back(0.);
    missing[0] = '\0';
    for(j = n = 0; j < (int)beam_sta.size(); j++) if(beam_sta[j].wgt != 0.)
    {
	for(i = n_old; i < n_new; i++)
	{
	    if( !strcasecmp(w_new[i]->sta(), beam_sta[j].sta) &&
		compareChan(w_new[i]->chan(), beam_sta[j].chan))
	    {
		ws.push_back(w_new[i]);
		weights[n] = beam_sta[j].wgt;
		n++;
		break;
	    }
	}
	if(i == n_new) {
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
    if(ws.size() == 0) {
	return;
    }

    if(o == NULL || o->lat < -900. || o->lon <= -900) {
	showWarning("No origin information.");
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
	return;
    }
    deltaz(o->lat, o->lon, lat, lon, &delta, &daz, &baz);

    if( !Beam::getTimeLags(this, ws, baz, recipe.slow, REFERENCE_STATION,
		t_lags, &lat, &lon) )
    {
	showWarning(GError::getMessage());
	return;
    }

    if((ts = Beam::BeamTimeSeries(ws, t_lags, weights, true)) != NULL)
    {
	ts->makeCopy();
	ts->setDataSource(ws[0]->getDataSource());
	ts->copyWfdiscPeriods(ws[0]->ts); // to addArrivals
	WfdiscPeriod *wpd = ts->getWfdiscPeriod(-1.e+60);
	snprintf(wpd->wf.sta, sizeof(wpd->wf.sta), "%s", cd->net());
	snprintf(wpd->wf.chan, sizeof(wpd->wf.chan), "%s", cd->chan());
	ts->setSta(cd->net());
	ts->setNet(cd->net());
	ts->setChan(cd->chan());
	ts->setLat(lat);
	ts->setLon(lon);
	ts->setComponent(1);

//	ts->setDataSource(this);

	Gobject *g = cd->ts->getValue("elements");
	if(g) ts->putValue("elements", g);
	cd->ts = ts;

	try {
	    IIRFilter *iir = new IIRFilter(recipe.ford, recipe.ftype,
				recipe.flo, recipe.fhi,
				ts->segment(0)->tdel(), recipe.zp);

	    DataMethod *dm[3];
	    dm[0] = new CalibData();
	    dm[1] = new TaperData("cosineBeg", 5, 5, 200);
	    dm[2] = iir;
	    DataMethod::changeMethods(3, dm, cd);
	    wp->modify(cd);

	    if(o) {
		CssWftagClass *wftag = new CssWftagClass();
		wftag->wfid = wfid;
		wftag->tagid = o->orid;
		stringcpy(wftag->tagname, "orid", sizeof(wftag->tagname));
		wp->putTable(wftag);
	    }
	}
	catch(int nerr) {
	    showWarning(GError::getMessage());
	}
    }
}

void TableQuery::writeBeamDotw(GSourceInfo &gs, char *net, char *chan,
			GTimeSeries *ts)
{
    const char *prefix = cssioGetTmpPrefix("/tmp", "geotl");
    char access[2];
    char dotw_file[MAXPATHLEN+1];

    stringcpy(dotw_file, prefix, MAXPATHLEN+1);
    if(strlen(dotw_file) + 2 < MAXPATHLEN) {
	strcat(dotw_file, ".w");
    }
    stringcpy(access, "w", sizeof(access));

    ts->waveform_io = new WaveformIO();

    ts->source_info.setSource(gs);

    for(int i = 0; i < ts->size(); i++)
    {
	WfdiscPeriod wp;
	GSegment *s = ts->segment(i);

	if(!writeDotw(access, dotw_file, s->data, net, chan, s->tbeg(),
	    s->length(), 1./s->tdel(), s->calib(), s->calper(), &wp.wf)) break;

	stringcpy(access, "a",sizeof(access));        /* append */

	wp.tbeg = s->tbeg();
	wp.tend = s->tend();
	wp.dir = stringToQuark(wp.wf.dir);
	wp.prefix = stringToQuark(prefix);
	wp.wfdisc_file = stringToQuark("");
	wp.wfdisc_index = 0;
	wp.pts_needed = 0;
//	getChanid(wfdisc.sta, wfdisc.chan, &wfdisc.chanid);
	ts->waveform_io->wp.push_back(wp);
    }
}

int TableQuery::writeDotw(char *access, char *dotw, float *y, char *sta,
		char *chan, double qtime, int nsamp, double samprate,
		double calib, double calper, CssWfdiscClass *w)
{
    static long	wfid = 1;
    int	i, n, flip_bytes;
    FILE *fp;
    struct stat	buf;
    DateTime dt;
    union
    {
	char	a[4];
	float	f;
	int	i;
	short	s;
    } e1, e2;

    e1.a[0] = 0; e1.a[1] = 0;
    e1.a[2] = 0; e1.a[3] = 1;
    flip_bytes = (e1.i == 1) ? 0 : 1;

    timeEpochToDate(timeGetEpoch(), &w->lddate);

    stringcpy(w->sta, sta, sizeof(w->sta));
    stringcpy(w->chan, chan, sizeof(w->chan));
    w->time = qtime;
    timeEpochToDate(w->time, &dt);
    w->jdate = timeJDate(&dt);
/*  w->chanid = chanid; */
    w->nsamp = nsamp;
    w->calib = calib;
    w->calper = calper;
    w->samprate = (samprate > 0.) ? samprate : 1.;
    w->endtime = qtime + (nsamp-1)/w->samprate;
    w->wfid = wfid++;
    stringcpy(w->datatype, "t4", sizeof(w->datatype));

    if(access[0] == 'a' && !stat(dotw, &buf)) {
	w->foff = buf.st_size;
    }

    n = strlen(dotw);
    for(i = n-1; i >= 0 && dotw[i] != '/'; i--);
    if(i < 0) {
	stringcpy(w->dir, ".", sizeof(w->dir));
    }
    else {
	strncpy(w->dir, dotw, i+1);
	w->dir[i+1] = '\0';
    }
    stringcpy(w->dfile, &dotw[i+1], sizeof(w->dfile));

    if((fp = fopen(dotw, access)) == NULL) {
	if(errno > 0) {
	    showWarning("Cannot open: %s\n%s", dotw, strerror(errno));
	} 
	else {
	    showWarning("Cannot open: %s", dotw); \
	}
	return(0);
    }

    if(flip_bytes) {
	for(i = 0; i < nsamp; i++) {
	    e1.f = y[i];
	    e2.a[0] = e1.a[3];
	    e2.a[1] = e1.a[2];
	    e2.a[2] = e1.a[1];
	    e2.a[3] = e1.a[0];
	    if(fwrite(&e2.f, sizeof(float), 1, fp) != 1) {
		if(errno > 0) {
		    showWarning("write error: %s\n%s", dotw, strerror(errno));
		}
		else {
		    showWarning("write error: %s", dotw);
		}
		fclose(fp);
		return(0);
	    }
	}
    }
    else if((int)fwrite(y, sizeof(float), nsamp, fp) != nsamp)
    {
	if(errno > 0) {
	    showWarning("write error: %s\n%s", dotw, strerror(errno));
	}
	else {
	    showWarning("write error: %s", dotw);
	}
	fclose(fp);
	return(0);
    }
    fclose(fp);

    return 1;
}

// DataSource routine
CssTableClass *TableQuery::createTable(CssTableClass *table, const string &table_name,
                const string &id_name, Password password)
{
    CssTableClass *t = NULL;
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    t = ffdbTableCreate(table, table_name, id_name, password, true);
	}
	else if(!src.compare("File-Prefix")) {
	     t = cssCreateTable(table, table_name, id_name, password,
				open_db->outputPrefix());
	}
	else {
#ifdef HAVE_LIBODBC
	    t = odbcCreateTable(table, table_name, id_name, password, true);
#endif
	}
    }
    else
    {
	int q_format = table->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    t = odbcCreateTable(table, table_name, id_name, password, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    t = ffdbTableCreate(table, table_name, id_name, password, false);
	}
	else if(q_format == stringToQuark("css")) {
	    t = cssCreateTable(table, table_name, id_name, password);
	}
	cerr << "TableQuery::createTable: unknown format." << endl;
    }
    if(t) t->setDataSource(this);
    return t;
}

// DataSource routine
bool TableQuery::changeTable(CssTableClass *old_table, CssTableClass *new_table)
{
    int q_format = old_table->getFormat();

    if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	return odbcChangeTable(old_table, new_table);
#endif
    }
    else if(q_format == stringToQuark("ffdb")) {
	return BasicSource::changeTable(old_table, new_table);
    }
    else if(q_format == stringToQuark("css")) {
	return BasicSource::changeTable(old_table, new_table);
    }
    cerr << "TableQuery::changeTable: unknown format." << endl;
    return false;
}

// DataSource routine
bool TableQuery::deleteTable(Component *caller, CssTableClass *table,
			const string &table_name)
{
    bool ret;
    int q_format = table->getFormat();

    // hold on to the table before the callbacks
    table->addOwner(this);

    if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	ret = odbcDeleteTable(table, table_name);
#endif
    }
    else if(q_format == stringToQuark("ffdb")) {
	ret = cssDeleteTable(caller, table);
    }
    else if(q_format == stringToQuark("css")) {
	ret = cssDeleteTable(caller, table);
    }
    else {
	ret = false;
	cerr << "TableQuery::deleteTable: unknown format." << endl;
    }
    if(ret && q_format != stringToQuark("odbc")) {
	// if it was successfully deleted, notify all listeners
	// (odbcDeleteTable already does this)
	TableListener::doCallbacks(table, this, "delete");
    }

    // release the table object
    table->removeOwner(this);

    return ret;
}

// DataSource routine
bool TableQuery::addArrival(CssArrivalClass *arrival, GTimeSeries *ts,
			Password password, long max_arid)
{
    arrival->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return ffdbAddArrival(arrival, ts, password, true);
	}
	else if(!src.compare("File-Prefix")) {
	    return cssAddArrival(arrival, ts, password,open_db->outputPrefix(), max_arid);
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddArrival(arrival, ts, true);
#endif
	}
    }
    else
    {
	int q_format = arrival->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddArrival(arrival, ts, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return ffdbAddArrival(arrival, ts, password, false);
	}
	else if(q_format == stringToQuark("css")) {
	    return cssAddArrival(arrival, ts, password, "", max_arid);
	}
	cerr << "TableQuery::addArrival: unknown format." << endl;
    }
    return false;
}

// DataSource routine
bool TableQuery::changeArrival(CssArrivalClass *arrival, GTimeSeries *ts, int type)
{
    int q_format = arrival->getFormat();

    if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	return odbcChangeArrival(arrival, ts, type);
#endif
    }
    else if(q_format == stringToQuark("ffdb")) {
	return cssChangeArrival(arrival, ts, type);
    }
    else if(q_format == stringToQuark("css")) {
	return cssChangeArrival(arrival, ts, type);
    }
    cerr << "TableQuery::changeArrival: unknown format." << endl;
    return false;
}

// DataSource routine
bool TableQuery::deleteArrival(Component *caller, CssArrivalClass *arrival,
	cvector<CssAssocClass> &assocs, cvector<CssAmplitudeClass> &amps,
	cvector<CssStamagClass> &stamags, cvector<CssHydroFeaturesClass> &hydros,
	cvector<CssInfraFeaturesClass> &infras)
{
    int q_format = arrival->getFormat();

    if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	return odbcDeleteArrival(arrival, assocs, amps, stamags, hydros,infras);
#endif
    }
    else if(q_format == stringToQuark("ffdb")) {
	return cssDeleteArrival(caller, arrival);
    }
    else if(q_format == stringToQuark("css")) {
	return cssDeleteArrival(caller, arrival);
    }
    cerr << "TableQuery::deleteArrival: unknown format." << endl;
    return false;
}

// DataSource routine
bool TableQuery::originCreate(CssOriginClass *origin, CssOrigerrClass *origerr,
		CssTableClass *table, Password password)
{
    origin->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return ffdbOriginCreate(origin, origerr, table, password, true);
	}
	else if(!src.compare("File-Prefix")) {
	    return cssCreateOrigin(origin, origerr, table, password,
			open_db->outputPrefix());
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcOriginCreate(origin, origerr, table, password, true);
#endif
	}
    }
    else
    {
	int q_format = table->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcOriginCreate(origin, origerr, table, password, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return ffdbOriginCreate(origin, origerr, table, password, false);
	}
	else if(q_format == stringToQuark("css")) {
	    return cssCreateOrigin(origin, origerr, table, password);
	}
	cerr << "TableQuery::originCreate: unknown format." << endl;
    }
    return false;
}

// DataSource routine
bool TableQuery::deleteOrigin(Component *caller, CssOriginClass *origin,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssWftagClass> &wftags, cvector<CssNetmagClass> &netmags,
		cvector<CssStamagClass> &stamags)
{
    int q_format = origin->getFormat();

    if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	return odbcDeleteOrigin(origin, origerrs, assocs, wftags, netmags,
			stamags);
#endif
    }
    else if(q_format == stringToQuark("ffdb")) {
	return cssDeleteOrigin(caller, origin);
    }
    else if(q_format == stringToQuark("css")) {
	return cssDeleteOrigin(caller, origin);
    }
    cerr << "TableQuery::deleteOrigin: unknown format." << endl;
    return false;
}

// DataSource routine
bool TableQuery::addAssoc(CssAssocClass *assoc)
{
    assoc->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return cssAddAssoc(assoc) ? true : false;
	}
	else if(!src.compare("File-Prefix")) {
	    return cssAddAssoc(assoc) ? true : false;
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddAssoc(assoc, true);
#endif
	}
    }
    else
    {
	int q_format = assoc->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddAssoc(assoc, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return cssAddAssoc(assoc) ? true : false;
	}
	else if(q_format == stringToQuark("css")) {
	    return cssAddAssoc(assoc) ? true : false;
	}
	cerr << "TableQuery::addAssoc: unknown format." << endl;
	return false;
    }
    return false;
}

// DataSource routine
bool TableQuery::addTable(CssTableClass *table)
{
    table->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return BasicSource::addTable(table);
	}
	else if(!src.compare("File-Prefix")) {
	    return BasicSource::addTable(table);
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddTable(table, true);
#endif
	}
    }
    else
    {
	int q_format = table->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddTable(table, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return BasicSource::addTable(table);
	}
	else if(q_format == stringToQuark("css")) {
	    return BasicSource::addTable(table);
	}
	cerr << "TableQuery::addTable: unknown format." << endl;
	return false;
    }
    return false;
}

// DataSource routine
CssOrigerrClass * TableQuery::addOrigerr(CssOriginClass *origin)
{
    CssOrigerrClass *origerr = NULL;
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    origerr = cssCreateOrigerr(origin);
	}
	else if(!src.compare("File-Prefix")) {
	    origerr = cssCreateOrigerr(origin);
	}
	else {
#ifdef HAVE_LIBODBC
	    origerr = odbcAddOrigerr(origin, true);
#endif
	}
    }
    else
    {
	int q_format = origin->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    origerr = odbcAddOrigerr(origin, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    origerr = cssCreateOrigerr(origin);
	}
	else if(q_format == stringToQuark("css")) {
	    origerr = cssCreateOrigerr(origin);
	}
	cerr << "TableQuery::addOrigerr: unknown format." << endl;
    }
    if(origerr) origerr->setDataSource(this);
    return origerr;
}

// DataSource routine
bool TableQuery::addAmplitude(CssArrivalClass *arrival, CssAmplitudeClass *amplitude)
{
    amplitude->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return addAmplitude(arrival, amplitude) ? true : false;
	}
	else if(!src.compare("File-Prefix")) {
	    return addAmplitude(arrival, amplitude) ? true : false;
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddAmplitude(arrival, amplitude, true);
#endif
	}
    }
    else
    {
	int q_format = arrival->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddAmplitude(arrival, amplitude, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return addAmplitude(arrival, amplitude) ? true : false;
	}
	else if(q_format == stringToQuark("css")) {
	    return addAmplitude(arrival, amplitude) ? true : false;
	}
	cerr << "TableQuery::addAmplitude: unknown format." << endl;
	return false;
    }
    return false;
}

// DataSource routine
bool TableQuery::addNetmag(CssNetmagClass *netmag, CssTableClass *table)
{
    netmag->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return cssAddNetmag(netmag, table);
	}
	else if(!src.compare("File-Prefix")) {
	    return cssAddNetmag(netmag, table);
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddNetmag(netmag, table, true);
#endif
	}
    }
    else
    {
	int q_format = table->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddNetmag(netmag, table, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return cssAddNetmag(netmag, table);
	}
	else if(q_format == stringToQuark("css")) {
	    return cssAddNetmag(netmag, table);
	}
	cerr << "TableQuery::addNetmag: unknown format." << endl;
    }
    return false;
}

// DataSource routine
bool TableQuery::addStamag(CssStamagClass *stamag, CssTableClass *table)
{
    stamag->setDataSource(this);
    if(open_db->outputConnected())
    {
	string src = open_db->outputSource();

	if(!src.compare("Flat-File-DB")) {
	    return cssAddStamag(stamag, table);
	}
	else if(!src.compare("File-Prefix")) {
	    return cssAddStamag(stamag, table);
	}
	else {
#ifdef HAVE_LIBODBC
	    return odbcAddStamag(stamag, table, true);
#endif
	}
    }
    else
    {
	int q_format = table->getFormat();

	if(q_format == stringToQuark("odbc")) {
#ifdef HAVE_LIBODBC
	    return odbcAddStamag(stamag, table, false);
#endif
	}
	else if(q_format == stringToQuark("ffdb")) {
	    return cssAddStamag(stamag, table);
	}
	else if(q_format == stringToQuark("css")) {
	    return cssAddStamag(stamag, table);
	}
	cerr << "TableQuery::addStamag: unknown format." << endl;
    }
    return false;
}

// DataSource routine
int TableQuery::getPathInfo(PathInfo **path_info)
{
    cvector<CssArrivalClass> arrival;
    cvector<CssAssocClass> assoc;
    cvector<CssOriginClass> origin;
    int i,j,k;

    *path_info = NULL;
    if( ignore_getpath ) return 0;
    ignore_getpath = true;

    getTable(arrival);
    getTable(assoc);
    getTable(origin);

    PathInfo *path = (PathInfo *)mallocWarn(sizeof(PathInfo));
    int num = 0;

    for(i = 0; i < arrival.size(); i++)
    {
	for(j = 0; j < assoc.size() && assoc[j]->arid != arrival[i]->arid; j++);
	if(j < assoc.size()) {
	    for(k = 0; k < origin.size(); k++) {
		if(origin[k]->orid == assoc[j]->orid) break;
	    }
	    if(k < origin.size())
	    {
		path = (PathInfo *)reallocWarn(path, (num+1)*sizeof(PathInfo));
		strcpy(path[num].sta, arrival[i]->sta);
		path[num].lat = -999.;
		path[num].lon = -999.;
		path[num].origin = origin[k];
		path[num].fg = stringToPixel("blue");
		num++;
	    }
	}
    }
    if(num) {
	getNetworkTables(false);
	cvector<CssSiteClass> *sites = getSiteTable();
	for(i = 0; i < num; i++) {
	    for(j = 0; j < sites->size(); j++) {
		if(!strcasecmp(sites->at(j)->sta, path[i].sta)) {
		    path[i].lat = sites->at(j)->lat;
		    path[i].lon = sites->at(j)->lon;
		    break;
		}
	    }
	}
    }
    *path_info = path;

    ignore_getpath = false;
    return num;
}
