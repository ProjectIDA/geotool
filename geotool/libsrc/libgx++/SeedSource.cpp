/** \file SeedSource.cpp
 *  \brief Defines class SeedSource.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <sys/param.h>
#include <errno.h>

#include "SeedSource.h"
#include "seed/SeedInput.h"
#include "SeedToCss.h"
#include "gobject++/GTimeSeries.h"
#include "Response.h"
#include "cssio.h"
#include <fstream>
#include <map>

extern "C" {
#include "libgmath.h"
#include "libtime.h"
#include "libstring.h"
}

class ChannelResponse
{
    public:
	ChannelResponse(Station &sta, Channel &chan) {
	    network = sta.b50.network;
	    station = sta.b50.station;
	    channel = chan.b52.channel;
	    location = chan.b52.location;
	}
	~ChannelResponse() {
	    for(int i = 0; i < (int)rsp.size(); i++) delete rsp[i];
	}
	string network;
	string station;
	string channel;
	string location;
	vector<Response *> rsp;
};

SeedSource::SeedSource(const string &name, const string &file)
		: TableSource(name)
{
    allow_owners = true;
    read_path = file;

    openPrefix(file);
    queryAllPrefixTables();
}

SeedSource::~SeedSource(void)
{
    seed_tables.clear();
    for(int i = 0; i < (int)seed_data.size(); i++) delete seed_data[i];
    seed_data.clear();
    for(int i = 0; i < (int)responses.size(); i++) delete responses[i];
    responses.clear();
}

void SeedSource::removeDataReceiver(DataReceiver *owner)
{
    TableSource::removeDataReceiver(owner);

    if((int)receivers.size() == 0) {
/*
	seed_tables.clear();
	for(int i = 0; i < (int)seed_data.size(); i++) delete seed_data[i];
	seed_data.clear();
*/
/*
	sites.clear();
	sitechans.clear();
	affiliations.clear();
*/
    }
}

gvector<SegmentInfo *> *SeedSource::getSegmentList(void)
{
    int i, k, nsel, record_index, path_quark;
//    bool first_b52 = true;
    char error[MAXPATHLEN+100];
    gvector<SegmentInfo *> *segs;
    Seed *o;
    SeedData *sd;
/*
    Blockette50 b50;
    Blockette52 *b52;
    Blockette71 *b71;
    Blockette72 *b72;
*/
    ifstream ifs;

    if(read_path.empty()) {
	return NULL;
    }

    seed_tables.clear();
    queryAllPrefixTables();

    ifs.open(read_path.c_str());
    if( !ifs.good() ) {
	snprintf(error, sizeof(error),"seed: cannot open %s",read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }

    segs = new gvector<SegmentInfo *>;

    path_quark = (int)stringToQuark(read_path);

    SeedInput in(&ifs);

    record_index = 0;
    if( (nsel = selected_records.size()) ) {
	selected.clear();
    }
    k = 0;

    while( (o = in.readSeed()) )
    {
	if( (sd = o->getSeedData()) && (int)sd->records.size() > 0)
	{
	    if(nsel > 0)
	    {
		int m = -1;
		SeedData *a = NULL;
		for(i = 0; i < (int)sd->records.size(); i++, record_index++) {
		    for(; k < nsel && selected_records[k] < record_index; k++);
		    if(k < nsel && selected_records[k] == record_index) {
			if(!a) {
			    a = new SeedData(&sd->records[i]);
			    a->calib = sd->calib;
			    a->calper = sd->calper;
			    if(sd->channel) a->channel = new Channel(*sd->channel);
			}
			else if(i != m+1) {
			    addSeg(segs, a, path_quark);
			    a = new SeedData(&sd->records[i]);
			    a->calib = sd->calib;
			    a->calper = sd->calper;
			    if(sd->channel) a->channel = new Channel(*sd->channel);
			}
			else a->records.push_back(sd->records[i]);
			m = i;
		    }
		}
		if( a ) {
		    addSeg(segs, a, path_quark);
		}
		delete o;
	    }
	    else {
		addSeg(segs, sd, path_quark);
	    }
	}
/*
	else if( (b71 = o->getBlockette71()) ) {
	    CssOriginClass *origin = new CssOriginClass();
	    origin->lat = b71->latitude;
	    origin->lon = b71->longitude;
	    origin->depth = b71->depth;
	    origin->time = b71->origin_time.epoch();
	    origin->jdate = timeEpochToJDate(origin->time);
	    origin->grn = b71->seismic_region;
	    origin->srn = b71->seismic_location;
	    for(i = 0; i < (int)b71->mag_type.size(); i++) {
		if(strstr(b71->mag_type[i].c_str(), "mb") ||
		   strstr(b71->mag_type[i].c_str(), "MB")) {
		    origin->mb = b71->magnitude[i];
		}
		else if(strstr(b71->mag_type[i].c_str(), "ms") ||
			strstr(b71->mag_type[i].c_str(), "MS")) {
		    origin->ms = b71->magnitude[i];
		}
		else if(strstr(b71->mag_type[i].c_str(), "ml") ||
			strstr(b71->mag_type[i].c_str(), "ML")) {
		    origin->ml = b71->magnitude[i];
		}
	    }
	    for(i = 0; i < seed_tables.size(); i++) {
		if(!seed_tables[i]->empty() &&
			seed_tables[i]->at(0)->nameIs(cssOrigin)) {
		    seed_tables[i]->push_back(origin);
		    break;
		}
	    }
	    if(i == seed_tables.size()) {
		gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
		v->push_back(origin);
		seed_tables.push_back(v);
	    }
	    delete o;
	}
	else if( (b72 = o->getBlockette72()) ) {
	    CssArrivalClass *arrival = new CssArrivalClass();
	    stringcpy(arrival->sta, b72->station.c_str(), sizeof(arrival->sta));
	    snprintf(arrival->chan, sizeof(arrival->chan), "%s", b72->channel.c_str());
	    arrival->sta_quark = stringUpperToQuark(arrival->sta);
	    arrival->chan_quark = stringUpperToQuark(arrival->chan);
	    arrival->time = b72->time.epoch();
	    arrival->jdate = timeEpochToJDate(arrival->time);
	    stringcpy(arrival->iphase, b72->phase_name.c_str(),
			sizeof(arrival->iphase));
	    arrival->amp = b72->amplitude;
	    arrival->per = b72->period;
	    arrival->snr = b72->snr;
	    for(i = 0; i < seed_tables.size(); i++) {
		if(!seed_tables[i]->empty() &&
			seed_tables[i]->at(0)->nameIs(cssArrival))
		{
		    seed_tables[i]->push_back(arrival);
		    break;
		}
	    }
	    if(i == seed_tables.size()) {
		gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
		v->push_back(arrival);
		seed_tables.push_back(v);
	    }
	    delete o;
	}
	else if( o->getBlockette50() ) {
	    b50 = *o->getBlockette50();
	    first_b52 = true;
	    int net_q = stringUpperToQuark(b50.network.c_str());
	    int sta_q = stringUpperToQuark(b50.station.c_str());

	    for(i = 0; i < (int)affiliations.size(); i++) {
		if(affiliations[i]->net_quark == net_q &&
		   affiliations[i]->sta_quark == sta_q) break;
	    }
	    if(i == (int)affiliations.size()) {
		CssAffiliationClass *aff = new CssAffiliationClass();
		stringcpy(aff->net, b50.network.c_str(), sizeof(aff->net));
		stringcpy(aff->sta, b50.station.c_str(), sizeof(aff->sta));
		aff->net_quark = net_q;
		aff->sta_quark = sta_q;
		affiliations.push_back(aff);
	    }
	    delete o;
	}
	else if( (b52 = o->getBlockette52()) ) {
	    if(first_b52) {
		first_b52 = false;
		CssSiteClass *site = new CssSiteClass();
		stringcpy(site->sta, b50.station.c_str(), sizeof(site->sta));
		site->sta_quark = stringUpperToQuark(site->sta);
		site->ondate = 1000*b52->start.year + b52->start.doy;
		site->lat = b52->latitude;
		site->lon = b52->longitude;
		site->elev = b52->elevation/1000.;
		stringcpy(site->staname,b50.name.c_str(),sizeof(site->staname));
		for(i = 0; i < (int)sites.size(); i++) {
		    if(site->sta_quark == sites[i]->sta_quark) break;
		}
		if(i < (int)sites.size()) {
		    sites.removeAt(i);
		}
		sites.push_back(site);
	    }

	    CssSitechanClass *sitechan = new CssSitechanClass();
	    stringcpy(sitechan->sta, b50.station.c_str(),sizeof(sitechan->sta));
	    snprintf(sitechan->chan, sizeof(sitechan->chan), "%s", b52->channel.c_str());
	    sitechan->sta_quark = stringUpperToQuark(sitechan->sta);
	    sitechan->chan_quark = stringUpperToQuark(sitechan->chan);
	    sitechan->ondate = 1000*b52->start.year + b52->start.doy;
	    sitechan->edepth = b52->local_depth;
	    sitechan->hang = b52->azimuth;
	    sitechan->vang = b52->dip + 90.;
	    for(i = 0; i < (int)sitechans.size(); i++) {
		if(!strcasecmp(sitechan->sta, sitechans[i]->sta) &&
		   !strcasecmp(sitechan->chan, sitechans[i]->chan)) break;
	    }
	    if(i < (int)sitechans.size()) {
		sitechans.removeAt(i);
	    }
	    sitechans.push_back(sitechan);

	    delete o;
	}
	else {
	    delete o;
	}
*/
    }

/*
    for(i = 0; i < (int)in.stations.size(); i++) {
	for(k = 0; k < (int)in.stations[i]->channels.size(); k++) {
	    storeResponse(*in.stations[i], *in.stations[i]->channels[k], in.dictionary, read_path);
	}
    }
*/

    bool any = false;
    for(i = 0; i < (int)selected.size(); i++) {
	if(selected[i] >= 0 && selected[i] < (int)segs->size()) {
	    any = true;
	    segs->at(selected[i])->selected = true;
	}
    }
    if(!any) {
	for(i = 0; i < (int)segs->size(); i++) segs->at(i)->selected = true;
    }

    getNetworks(segs);

    getSites(segs);

    getSitechans(segs);

    cvector<CssOriginClass> origins;

/*
    if( getTable(origins) > 0 )
    {
	for(i = 0; i < (int)segs->size(); i++) {
	    SegmentInfo *s = segs->at(i);
	    double tmin = s->start;
	    double tmax = s->end;
	    for(int j = 0; j < origins.size(); j++) {
		if(origins[j]->time > tmin - 3600. && origins[j]->time < tmax) {
		    double delta, az, baz;
		    deltaz(origins[j]->lat, origins[j]->lon, s->station_lat,
			s->station_lon, &delta, &az, &baz);
		    s->origin_id = origins[j]->orid;
		    s->origin_delta = delta;
		    s->origin_depth = origins[j]->depth;
		    s->origin_time = origins[j]->time;
		    s->origin_lat = origins[j]->lat;
		    s->origin_lon = origins[j]->lon;
		    s->origin_azimuth = az;
		}
	    }
	}
    }
*/

    return segs;
}

void SeedSource::addSeg(gvector<SegmentInfo *> *segs, SeedData *sd,
			int path_quark)
{
    char name[MAXPATHLEN+1];
    SegmentInfo *s;
    DataHeader *h;
    CssWfdiscClass *w;

    h = &sd->records[0].header;
    s = new SegmentInfo();
    segs->push_back(s);
    snprintf(name, sizeof(name), "%s.%d", read_path.c_str(), (int)segs->size());
    s->path = path_quark;
    s->id = (int)seed_data.size() + 1;
    s->format = stringToQuark("seed");
    s->file_order = (int)segs->size();
    stringcpy(s->sta, h->station.c_str(), sizeof(s->sta));
/*
    snprintf(s->chan, sizeof(s->chan), "%s%s", h->channel.c_str(), h->location.c_str());
*/
    snprintf(s->chan, sizeof(s->chan), "%s", h->channel.c_str());
    stringcpy(s->net, h->network.c_str(), sizeof(s->net));
    s->start = h->startTime();
    s->end = sd->records.back().header.endTime();
    s->nsamp = sd->nsamples();
    s->samprate = h->sampleRate();
    s->jdate = timeEpochToJDate(s->start);

    if(sd->channel) {
	s->station_lat = sd->channel->b52.latitude;
	s->station_lon = sd->channel->b52.longitude;
	s->station_elev = sd->channel->b52.elevation/1000.;
	s->station_depth = sd->channel->b52.local_depth;
    }

    w = new CssWfdiscClass();
    w->setDataSource(this);
    stringcpy(w->sta, s->sta, sizeof(w->sta));
    stringcpy(w->chan, s->chan, sizeof(w->chan));
    w->sta_quark = stringUpperToQuark(w->sta);
    w->chan_quark = stringUpperToQuark(w->chan);
    w->time = s->start;
    w->endtime = s->end;
    w->nsamp = s->nsamp;
    w->samprate = s->samprate;
    w->jdate = s->jdate;
    w->calib = sd->calib;
    w->calper = sd->calper;
    s->setWfdisc(w);
    s->selected = false;

    seed_data.push_back(sd);
} 

void SeedSource::listWaveforms()
{
    Seed *o;
    SeedData *sd;
    ifstream ifs;

    if(read_path.empty()) return;

    seed_tables.clear();
    tables.clear();

    ifs.open(read_path.c_str());
    if( !ifs.good() ) {
	char error[MAXPATHLEN+100];
	snprintf(error, sizeof(error),"seed: cannot open %s",read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return;
    }

    SeedInput in(&ifs);

    while( (o = in.readSeed()) ) {
	if( (sd = o->getSeedData()) ) listSeedData(*sd);
	delete o;
    }
}

void SeedSource::listRecords()
{
    int i;
    Seed *o;
    SeedData *sd;
    ifstream ifs;

    if(read_path.empty()) return;

    tables.clear();

    for(int i = 0; i < seed_tables.size(); i++) {
	if( !seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Data_Records") )
	{
	    seed_tables[i]->clear();
	    seed_tables.removeAt(i);
	    break;
	}
    }

    ifs.open(read_path.c_str());
    if( !ifs.good() ) {
	char error[MAXPATHLEN+100];
	snprintf(error, sizeof(error),"seed: cannot open %s",read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return;
    }

    SeedInput in(&ifs);

    while( (o = in.readSeed()) ) {
	if( (sd = o->getSeedData()) ) {
	    for(i = 0; i < (int)sd->records.size(); i++) {
		listDataRec(sd->records[i]);
	    }
	}
	delete o;
    }
}

void SeedSource::listBlockettes()
{
    char sta[6];
    int i, j;
    Seed *o;
    SeedData *sd;
    Blockette50 *b50;
    ifstream ifs;

    if(read_path.empty()) return;

    tables.clear();

    for(int i = 0; i < seed_tables.size(); i++) {
	if( !seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Blockettes") )
	{
	    seed_tables[i]->clear();
	    seed_tables.removeAt(i);
	    break;
	}
    }

    ifs.open(read_path.c_str());
    if( !ifs.good() ) {
	char error[MAXPATHLEN+100];
	snprintf(error, sizeof(error),"seed: cannot open %s",read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return;
    }

    SeedInput in(&ifs);

    while( (o = in.readSeed()) ) {
	if( (sd = o->getSeedData()) ) {
	    for(i = 0; i < (int)sd->records.size(); i++) {
		listBlocketteInfo(in, &sd->records[i]);
		for(j = 0; j < (int)sd->records[i].blockettes.size(); j++) {
		    listBlocketteInfo(in, sd->records[i].blockettes[j]);
		}
	    }
	}
	else {
	    listBlocketteInfo(in, o);

	    if( (b50 = o->getBlockette50()) ) {
		listStation(in, b50);
		stringcpy(sta, b50->station.c_str(), sizeof(sta));
	    }
	    else if( o->getBlockette52() ) {
		listChannel(in, sta, o->getBlockette52());
	    }
	    else if( o->getBlockette31() ) {
		listComment(in, o->getBlockette31());
	    }
	    else if( o->getBlockette33() ) {
		listAbbreviation(in, o->getBlockette33());
	    }
	    else if( o->getBlockette34() ) {
		listUnit(in, o->getBlockette34());
	    }
	}
	delete o;
    }
}

int SeedSource::getTable(const string &cssTableName, gvector<CssTableClass *> &v)
{
    v.clear();
/*
    if(!cssTableName.compare("site")) {
        v.load(sites);
        return v.size();
    }
    else if(!cssTableName.compare("sitechan")) {
        v.load(sitechans);
        return v.size();
    }
    else if(!cssTableName.compare("affiliation")) {
        v.load(affiliations);
        return v.size();
    }
*/

    for(int i = 0; i < seed_tables.size(); i++) {
        if( !seed_tables[i]->empty() &&
                seed_tables[i]->at(0)->nameIs(cssTableName) )
        {
            v.load(seed_tables[i]);
        }
    }

    gvector<CssTableClass *> *t = TableSource::findTable(cssTableName);
    if(t) {
        v.ensureCapacity(v.size() + t->size());
        for(int i = 0; i < t->size(); i++) v.add(t->at(i));
    }

    return v.size();
}

bool SeedSource::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    const char *full_path;
    string dir, prefix;
    int n, j;
    GSegment *segment;

    if(s->id < 1 || s->id > (int)seed_data.size()) return false;
    j = s->id-1;

    *err_msg = NULL;

    full_path = quarkToString(s->path);
    getDir(full_path, dir, prefix);

    if(*ts == NULL)
    {
	*ts = new GTimeSeries();
	(*ts)->waveform_io = new WaveformIO();
	(*ts)->putValue("dc", s->path);
	(*ts)->putValue("id", s->id);
	(*ts)->source_info.setSource(full_path);
	(*ts)->source_info.format = stringToQuark("seed");
	(*ts)->source_info.data_source = stringToQuark("seed");
	for(int i = 0; i < s->array_elements.size(); i++) {
	    (*ts)->array_elements.add(s->array_elements[i]);
	}
    }

    segment = SeedToCss::readSegment(seed_data[j], read_path, tbeg, tend);

    if(!segment) return false;

    if( (n = invalidData(segment->length(), segment->data, (float)0.)) )
    {
	char error[MAXPATHLEN+100];
	snprintf(error, MAXPATHLEN+100,
	    "cssIO: setting %d invalid binary data from %s to 0.",n, full_path);
	logErrorMsg(LOG_WARNING, error);
    }

    WfdiscPeriod wp;
    wp.tbeg = segment->tbeg();
    wp.tend = segment->tend();
    wp.dir = stringToQuark(dir);
    wp.prefix = stringToQuark(prefix);
    wp.wfdisc_file = stringToQuark(full_path);
    wp.wfdisc_index = s->id;
    wp.pts_needed = pts;
    wp.wf = *s->wfdisc();
    getChanid(wp.wf.sta, wp.wf.chan, &wp.wf.chanid);

    if((*ts)->waveform_io) (*ts)->waveform_io->wp.push_back(wp);

    (*ts)->addSegment(segment);

    return true;
}

bool SeedSource::reread(GTimeSeries *ts)
{
    int i, j;
    double tbeg, tend, d;
    GSegment *segment;

    if( !ts->waveform_io) {
	logErrorMsg(LOG_WARNING, "seed.reread: cannot re-read data");
	return false;
    }
    if((tbeg = ts->originalStart()) == NULL_TIME) tbeg = ts->tbeg();
    if((tend = ts->originalEnd()) == NULL_TIME) tend = ts->tend();

    ts->removeAllSegments();

    for(i = 0; i < (int)ts->waveform_io->wp.size(); i++)
    {
	j = ts->waveform_io->wp[i].wfdisc_index-1;
	segment = SeedToCss::readSegment(seed_data[j], read_path, tbeg, tend);
	if(segment != NULL) {
	    ts->addSegment(segment);
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

// static
bool SeedSource::isSeedFile(const string &path)
{
    Seed *o;
    ifstream ifs;
    SeedInput in(&ifs, 0, FMTBIT | HDRBIT | LENBIT | SEQNOBIT);

    ifs.open(path.c_str());
    if( ifs.good() ) {
	try {
	    if((o = in.readSeed()) ) {
		delete o;
		return true;
	    }
	}
	catch(...) { }
    }
    return false;
}

#define offset(field) ((unsigned int)(((const char *)&a.field) - ((const char *)&a))), sizeof(a.field)

class SeedDataInfo : public CssTableClass
{
    public:
	char sta[6];
	char loc[3];
	char chan[4];
	char net[3];
	char start[24];
	int nsamp;
	double samprate;
	char end[24];
	double calib;
	double calper;

	static CssTableClass *createSeedDataInfo(void) {
            return (CssTableClass *) new SeedDataInfo();
        }
        SeedDataInfo(void);
        SeedDataInfo(int flag) { } // do not initialize members
};

static void defineSeedDataInfo(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	SeedDataInfo a(0);
	CssClassDescription des[] = {
	{1,  1,        "sta",   "%s",      offset(sta), 0, CSS_STRING,   ""},
	{2,  2,        "loc",   "%s",      offset(loc), 0, CSS_STRING,   ""},
	{3,  3,       "chan",   "%s",     offset(chan), 0, CSS_STRING,   ""},
	{4,  4,        "net",   "%s",      offset(net), 0, CSS_STRING,   ""},
	{5,  5, "start time",   "%s",    offset(start), 0, CSS_STRING,   ""},
	{6,  6,      "nsamp",   "%d",    offset(nsamp), 0, CSS_INT,     "0"},
	{7,  7,   "samprate", "%.6g", offset(samprate), 0, CSS_DOUBLE, "0."},
	{8,  8,   "end time",   "%s",      offset(end), 0, CSS_STRING,   ""},
	{9,  9,      "calib", "%.6g",    offset(calib), 0, CSS_DOUBLE, "0."},
	{10,10,     "calper", "%.6g",   offset(calper), 0, CSS_DOUBLE, "0."}
        };
	CssTableClass::define("Waveforms", sizeof(des)/sizeof(CssClassDescription), des,0,
		NULL, SeedDataInfo::createSeedDataInfo,sizeof(SeedDataInfo));
    }
}

SeedDataInfo::SeedDataInfo(void) : CssTableClass("Waveforms")
{
    defineSeedDataInfo();
    initTable();
}

void SeedSource::listSeedData(SeedData &sd)
{
    if((int)sd.records.size() == 0) return;

    int i;
    DateTime dt;
    SeedDataInfo *s = new SeedDataInfo();
    DataHeader *h = &sd.records[0].header;

    stringcpy(s->sta, h->station.c_str(), sizeof(s->sta));
    stringcpy(s->loc, h->location.c_str(), sizeof(s->loc));
    stringcpy(s->chan, h->channel.c_str(), sizeof(s->chan));
    stringcpy(s->net, h->network.c_str(), sizeof(s->net));
    stringcpy(s->start, h->start_time.str().c_str(), sizeof(s->start));
    s->nsamp = sd.nsamples();
    s->samprate = sd.samprate();
    timeEpochToDate(sd.endTime(), &dt);
    snprintf(s->end, sizeof(s->end), "%4d/%03d %02d:%02d:%07.4f",
		dt.year, timeDOY(&dt), dt.hour, dt.minute, dt.second);
    if(s->end[15] == '0') s->end[15] = ' ';
    s->calib = sd.calib;
    s->calper = sd.calper;

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Waveforms")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class BlocketteInfo : public CssTableClass
{
    public:
	int seqno;
	char code[2];
	char type[16];
	char summary[250];

	static CssTableClass *createBlocketteInfo(void) {
            return (CssTableClass *) new BlocketteInfo();
        }
        BlocketteInfo(void);
        BlocketteInfo(int flag) { } // do not initialize members
};

static void defineBlocketteInfo(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	BlocketteInfo a(0);
	CssClassDescription des[] = {
	{1, 1,     "rec",  "%d",   offset(seqno), 0, CSS_INT,    "0"},
	{2, 2,       "c",  "%s",    offset(code), 0, CSS_STRING, "0"},
	{3, 3,    "type",  "%s",    offset(type), 0, CSS_STRING,  ""},
	{4, 4, "summary",  "%s", offset(summary), 0, CSS_STRING,  ""}
        };
	CssTableClass::define("Blockettes",sizeof(des)/sizeof(CssClassDescription), des,0,
		NULL, BlocketteInfo::createBlocketteInfo,sizeof(BlocketteInfo));
    }
}

BlocketteInfo::BlocketteInfo(void) : CssTableClass("Blockettes")
{
    defineBlocketteInfo();
    initTable();
}

void SeedSource::listBlocketteInfo(SeedInput &in, Seed *o)
{
    int i;
    BlocketteInfo *info = new BlocketteInfo();

    info->seqno = in.recordSeqenceNumber();
    info->code[0] = in.recordType();
    info->code[1] = '\0';
    snprintf(info->type, sizeof(info->type), "B%s", o->getType().c_str());
    snprintf(info->summary, sizeof(info->summary), "%s", o->str().c_str());

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Blockettes")) {
	    seed_tables[i]->push_back(info);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(info);
	seed_tables.push_back(v);
    }
}

class StationB50 : public CssTableClass
{
    public:
	int seqno;
	int nchan;
	int ncmts;
	int netid;
	char sta[6];
	char name[61];
	char start[23];
	char end[23];
	char bit32[5];
	char bit16[5];
	char update[2];
	char net[3];
	double lat;
	double lon;
	double elev;

	static CssTableClass *createStationB50(void) {
            return (CssTableClass *) new StationB50();
        }
        StationB50(void);
        StationB50(int flag) { } // do not initialize members
};

static void defineStationB50(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	StationB50 a(0);
	CssClassDescription des[] = {
	{1,   1, "seqno",  "%d", offset(seqno), 0, CSS_INT,    "0"},
	{2,   2, "sta",    "%s", offset(sta),   0, CSS_STRING,  ""},
	{3,   3, "lat",  "%.6f", offset(lat),   0, CSS_DOUBLE, "0"},
	{4,   4, "lon",  "%.6f", offset(lon),   0, CSS_DOUBLE, "0"},
	{5,   5, "elev", "%.1f", offset(elev),  0, CSS_DOUBLE, "0"},
	{6,   6, "nchan",  "%d", offset(nchan), 0, CSS_INT,    "0"},
	{7,   7, "ncmts",  "%d", offset(ncmts), 0, CSS_INT,    "0"},
	{8,   8, "site",   "%s", offset(name),  0, CSS_STRING,  ""},
	{9,   9, "netid",  "%d", offset(netid), 0, CSS_INT,    "0"},
	{10, 10, "32bit",  "%s", offset(bit32), 0, CSS_STRING,  ""},
	{11, 11, "16bit",  "%s", offset(bit16), 0, CSS_STRING,  ""},
	{12, 12, "start",  "%s", offset(start), 0, CSS_STRING,  ""},
	{13, 13, "end",    "%s", offset(end),   0, CSS_STRING,  ""},
	{14, 14, "update", "%s", offset(update),0, CSS_STRING,  ""},
	{15, 15, "net",    "%s", offset(net),   0, CSS_STRING,  ""}
        };
	CssTableClass::define("Stations",sizeof(des)/sizeof(CssClassDescription), des,0,
		NULL, StationB50::createStationB50,sizeof(StationB50));
    }
}

StationB50::StationB50(void) : CssTableClass("Stations")
{
    defineStationB50();
    initTable();
}

void SeedSource::listStation(SeedInput &in, Blockette50 *b50)
{
    int i;
    StationB50 *s = new StationB50();

    s->seqno = in.recordSeqenceNumber();
    stringcpy(s->sta, b50->station.c_str(), sizeof(s->sta));
    stringcpy(s->name, b50->name.c_str(), sizeof(s->name));
    stringcpy(s->bit32, b50->word_order.c_str(), sizeof(s->bit32));
    stringcpy(s->bit16, b50->short_order.c_str(), sizeof(s->bit16));
    stringcpy(s->update, b50->update.c_str(), sizeof(s->update));
    stringcpy(s->net, b50->network.c_str(), sizeof(s->net));

    stringcpy(s->start, b50->start.str().c_str(), sizeof(s->start));
    stringcpy(s->end, b50->end.str().c_str(), sizeof(s->end));

    s->lat = b50->latitude;
    s->lon = b50->longitude;
    s->elev = b50->elevation;
    s->nchan = b50->num_channels;
    s->ncmts = b50->num_comments;
    s->netid = b50->network_id;

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Stations")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class ChannelB52 : public CssTableClass
{
    public:
	int seqno;
	int subchan;
	int instr;
	int ncmts;
	int sig_units;
	int cal_units;
	int format;
	int reclen;
	char sta[6];
	char chan[4];
	char loc[3];
	char start[23];
	char end[23];
	char update[2];
	char flags[27];
	char comment[31];
	double lat;
	double lon;
	double elev;
	double depth;
	double az;
	double dip;
	double samprate;
	double drift;

	static CssTableClass *createChannelB52(void) {
            return (CssTableClass *) new ChannelB52();
        }
        ChannelB52(void);
        ChannelB52(int flag) { } // do not initialize members
};

static void defineChannelB52(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	ChannelB52 a(0);
	CssClassDescription des[] = {
	{1,   1, "seqno",     "%d", offset(seqno),     0, CSS_INT,    "0"},
	{2,   2, "sta",       "%s", offset(sta),       0, CSS_STRING,  ""},
	{3,   3, "chan",      "%s", offset(chan),      0, CSS_STRING,  ""},
	{4,   4, "loc",       "%s", offset(loc),       0, CSS_STRING,  ""},
	{5,   5, "subchan",   "%d", offset(subchan),   0, CSS_INT,     ""},
	{6,   6, "lat",     "%.6f", offset(lat),       0, CSS_DOUBLE, "0"},
	{7,   7, "lon",     "%.6f", offset(lon),       0, CSS_DOUBLE, "0"},
	{8,   8, "elev",    "%.1f", offset(elev),      0, CSS_DOUBLE, "0"},
	{9,   9, "depth",   "%.1f", offset(depth),     0, CSS_DOUBLE, "0"},
	{10, 10, "az",      "%.1f", offset(az),        0, CSS_DOUBLE, "0"},
	{11, 11, "dip",     "%.1f", offset(dip),       0, CSS_DOUBLE, "0"},
	{12, 12, "samprate","%.4g", offset(samprate),  0, CSS_DOUBLE, "0"},
	{13, 13, "drift",   "%.4g", offset(drift),     0, CSS_DOUBLE, "0"},
	{14, 14, "instr",     "%d", offset(instr),     0, CSS_INT,    "0"},
	{15, 15, "sig_units", "%d", offset(sig_units), 0, CSS_INT,    "0"},
	{16, 16, "cal_units", "%d", offset(cal_units), 0, CSS_INT,    "0"},
	{17, 17, "format",    "%d", offset(format),    0, CSS_INT,    "0"},
	{18, 18, "reclen",    "%d", offset(reclen),    0, CSS_INT,    "0"},
	{19, 19, "ncmts",     "%d", offset(ncmts),     0, CSS_INT,    "0"},
	{21, 21, "start",     "%s", offset(start),     0, CSS_STRING,  ""},
	{22, 22, "end",       "%s", offset(end),       0, CSS_STRING,  ""},
	{23, 23, "update",    "%s", offset(update),    0, CSS_STRING,  ""},
	{24, 24, "flags",     "%s", offset(flags),     0, CSS_STRING,  ""},
	{25, 25, "comment",   "%s", offset(comment),   0, CSS_STRING,  ""},
        };
	CssTableClass::define("Channels",sizeof(des)/sizeof(CssClassDescription), des,0,
		NULL, ChannelB52::createChannelB52,sizeof(ChannelB52));
    }
}

ChannelB52::ChannelB52(void) : CssTableClass("Channels")
{
    defineChannelB52();
    initTable();
}

void SeedSource::listChannel(SeedInput &in, const string &sta, Blockette52 *b52)
{
    int i;
    ChannelB52 *s = new ChannelB52();

    s->seqno = in.recordSeqenceNumber();

    s->subchan = b52->subchannel;
    s->instr = b52->instrument;
    s->ncmts = b52->num;
    s->sig_units = b52->signal_units;
    s->cal_units = b52->calib_units;
    s->format = b52->format_code;
    s->reclen = b52->reclen;
    stringcpy(s->sta, sta.c_str(), sizeof(s->sta));
    stringcpy(s->chan, b52->channel.c_str(), sizeof(s->chan));
    stringcpy(s->loc, b52->location.c_str(), sizeof(s->loc));
    stringcpy(s->start, b52->start.str().c_str(), sizeof(s->start));
    stringcpy(s->end, b52->end.str().c_str(), sizeof(s->end));
    stringcpy(s->update, b52->update.c_str(), sizeof(s->update));
    stringcpy(s->flags, b52->channel_flags.c_str(), sizeof(s->flags));
    stringcpy(s->comment, b52->comment.c_str(), sizeof(s->comment));
    s->lat = b52->latitude;
    s->lon = b52->longitude;
    s->elev = b52->elevation;
    s->depth = b52->local_depth;
    s->az = b52->azimuth;
    s->dip = b52->dip;
    s->samprate = b52->sample_rate;
    s->drift = b52->clock_drift;

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Channels")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class Comment31 : public CssTableClass
{
    public:
	int seqno;
	int code_key;
	char class_code[2];
	char comment[71];
	int units;

	static CssTableClass *createComment31(void) {
            return (CssTableClass *) new Comment31();
        }
        Comment31(void);
        Comment31(int flag) { } // do not initialize members
};

static void defineComment31(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	Comment31 a(0);
	CssClassDescription des[] = {
	{1,   1, "seqno",     "%d", offset(seqno),     0, CSS_INT,    "0"},
	{2,   2, "code_key",  "%d", offset(code_key),  0, CSS_INT,    "0"},
	{3,   3, "class_code","%s", offset(class_code),0, CSS_STRING,  ""},
	{4,   4, "comment",   "%s", offset(comment),   0, CSS_STRING,  ""},
	{5,   5, "units",     "%d", offset(units),     0, CSS_INT,     ""},
        };
	CssTableClass::define("Comments",sizeof(des)/sizeof(CssClassDescription), des, 0,
		NULL, Comment31::createComment31, sizeof(Comment31));
    }
}

Comment31::Comment31(void) : CssTableClass("Comments")
{
    defineComment31();
    initTable();
}

void SeedSource::listComment(SeedInput &in, Blockette31 *b31)
{
    int i;
    Comment31 *s = new Comment31();

    s->seqno = in.recordSeqenceNumber();
    s->code_key = b31->lookup_code;
    stringcpy(s->class_code, b31->class_code.c_str(), sizeof(s->class_code));
    stringcpy(s->comment, b31->comment.c_str(), sizeof(s->comment));
    s->units = b31->units;

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Comments")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class Units34 : public CssTableClass
{
    public:
	int seqno;
	int code;
	char name[21];
	char description[51];

	static CssTableClass *createUnits34(void) {
            return (CssTableClass *) new Units34();
        }
        Units34(void);
        Units34(int flag) { } // do not initialize members
};

static void defineUnits34(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	Units34 a(0);
	CssClassDescription des[] = {
	{1,   1, "seqno",       "%d", offset(seqno),       0, CSS_INT,    "0"},
	{2,   2, "code",        "%d", offset(code),        0, CSS_INT,    "0"},
	{3,   3, "name",        "%s", offset(name),        0, CSS_STRING,  ""},
	{4,   4, "description", "%s", offset(description), 0, CSS_STRING,  ""},
        };
	CssTableClass::define("Units",sizeof(des)/sizeof(CssClassDescription), des, 0,
		NULL, Units34::createUnits34, sizeof(Units34));
    }
}

Units34::Units34(void) : CssTableClass("Units")
{
    defineUnits34();
    initTable();
}

void SeedSource::listUnit(SeedInput &in, Blockette34 *b34)
{
    int i;
    Units34 *s = new Units34();

    s->seqno = in.recordSeqenceNumber();
    s->code = b34->lookup_code;
    stringcpy(s->name, b34->name.c_str(), sizeof(s->name));
    stringcpy(s->description, b34->description.c_str(), sizeof(s->description));

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Units")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class Abbrev33 : public CssTableClass
{
    public:
	int seqno;
	int code;
	char description[51];

	static CssTableClass *createAbbrev33(void) {
            return (CssTableClass *) new Abbrev33();
        }
        Abbrev33(void);
        Abbrev33(int flag) { } // do not initialize members
};

static void defineAbbrev33(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	Abbrev33 a(0);
	CssClassDescription des[] = {
	{1,   1, "seqno",       "%d", offset(seqno),       0, CSS_INT,    "0"},
	{2,   2, "code",        "%d", offset(code),        0, CSS_INT,    "0"},
	{3,   3, "description", "%s", offset(description), 0, CSS_STRING,  ""},
        };
	CssTableClass::define("Abbreviations",sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, Abbrev33::createAbbrev33, sizeof(Abbrev33));
    }
}

Abbrev33::Abbrev33(void) : CssTableClass("Abbreviations")
{
    defineAbbrev33();
    initTable();
}

void SeedSource::listAbbreviation(SeedInput &in, Blockette33 *b33)
{
    int i;
    Abbrev33 *s = new Abbrev33();

    s->seqno = in.recordSeqenceNumber();
    s->code = b33->lookup_code;
    stringcpy(s->description, b33->description.c_str(), sizeof(s->description));

    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Abbreviations")) {
	    seed_tables[i]->push_back(s);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(s);
	seed_tables.push_back(v);
    }
}

class DataRec : public CssTableClass
{
    public:
	int seqno;
	char qual[2];
	char sta[6];
	char loc[3];
	char chan[4];
	char net[3];
	char start[24];
	int nsamp;
	int rate;
	int multiplier;
	int activity;
	int io;
	int quality;
	int num;
	int correction;
	char end[24];
	int offset;
	int boffset;
	double samprate;

	static CssTableClass *createDataRec(void) {
            return (CssTableClass *) new BlocketteInfo();
        }
        DataRec(void);
        DataRec(int flag) { } // do not initialize members
};

static void defineDataRec(void)
{
    static bool defined = false;
    if( !defined ) {
	defined = true;
	DataRec a(0);
	CssClassDescription des[] = {
	{1,  1,      "seqno", "%d",      offset(seqno), 0, CSS_INT,   "0"},
	{2,  2,          "q", "%s",       offset(qual), 0, CSS_STRING, ""},
	{3,  3,        "sta", "%s",        offset(sta), 0, CSS_STRING, ""},
	{4,  4,        "loc", "%s",        offset(loc), 0, CSS_STRING, ""},
	{5,  5,       "chan", "%s",       offset(chan), 0, CSS_STRING, ""},
	{6,  6,        "net", "%s",        offset(net), 0, CSS_STRING, ""},
	{7,  7," start time", "%s",      offset(start), 0, CSS_STRING, ""},
	{8,  8,      "nsamp", "%d",      offset(nsamp), 0, CSS_INT,   "0"},
	{9,  9,       "rate", "%d",       offset(rate), 0, CSS_INT,   "0"},
	{10,10,  "multplier", "%d", offset(multiplier), 0, CSS_INT,   "0"},
	{11,11,   "activity", "%d",   offset(activity), 0, CSS_INT,    ""},
	{12,12,         "io", "%d",         offset(io), 0, CSS_INT,    ""},
	{13,13,       "qual", "%d",    offset(quality), 0, CSS_INT,    ""},
	{14,14,        "num", "%d",        offset(num), 0, CSS_INT,    ""},
	{15,15,       "corr", "%d", offset(correction), 0, CSS_INT,    ""},
	{16,16,   "end time", "%d",        offset(end), 0, CSS_STRING, ""},
	{17,17,    "doffset", "%d",     offset(offset), 0, CSS_INT,    ""},
	{18,18,    "boffset", "%d",    offset(boffset), 0, CSS_INT,    ""},
	{19,19, "100-samprate", "%.6g", offset(samprate), 0, CSS_DOUBLE, "0"}
        };
	CssTableClass::define("Data_Records", sizeof(des)/sizeof(CssClassDescription),
		des,0, NULL, DataRec::createDataRec,sizeof(BlocketteInfo));
    }
}

DataRec::DataRec(void) : CssTableClass("Data_Records")
{
    defineDataRec();
    initTable();
}

void SeedSource::listDataRec(DataRecord &dr)
{
    int i;
    unsigned char c;
    Blockette100 *b100;
    char s[20];
    DataHeader *h = &dr.header;
    DataRec *rec = new DataRec();

    rec->seqno = h->seqno;
    rec->qual[0] = h->dhqual;
    rec->qual[1] = '\0';
    stringcpy(rec->sta, h->station.c_str(), sizeof(rec->sta));
    stringcpy(rec->loc, h->location.c_str(), sizeof(rec->loc));
    stringcpy(rec->chan, h->channel.c_str(), sizeof(rec->chan));
    stringcpy(rec->net, h->network.c_str(), sizeof(rec->net));
    stringcpy(rec->start, h->start_time.str().c_str(), sizeof(rec->start));
    rec->nsamp = h->nsamples;
    rec->rate = h->sample_rate;
    rec->multiplier = h->multiplier;
    for(i = 0, c = 0x40; i < 7; i++) {  // Activity flags uses 7 bits
	s[i] = (h->activity & c) ? '1' : '0';
	c = c >> 1;
    }
    s[7] = '\0';
    for(i = 0; i < 7 && s[i] != '1'; i++) s[i] = ' ';
    stringToInt(s, &rec->activity);

    for(i = 0, c = 0x20; i < 6; i++) {  // I/O flags uses 6 bits
	s[i] = (h->io & c) ? '1' : '0';
	c = c >> 1;
    }
    s[6] = '\0';
    for(i = 0; i < 6 && s[i] != '1'; i++) s[i] = ' ';
    stringToInt(s, &rec->io);

    for(i = 0, c = 0x80; i < 8; i++) { // Data quality flags uses 8 bits
	s[i] = (h->quality & c) ? '1' : '0';
	c = c >> 1;
    }
    s[8] = '\0';
    for(i = 0; i < 8 && s[i] != '1'; i++) s[i] = ' ';
    stringToInt(s, &rec->quality);

    rec->num = h->num;
    rec->correction = h->correction;
    stringcpy(rec->end, h->end_time.str().c_str(), sizeof(rec->end));
    rec->offset = h->offset;
    rec->boffset = h->boffset;
    rec->samprate = 0.;

    for(i = 0; i < (int)dr.blockettes.size(); i++) {
	if( (b100 = dr.blockettes[i]->getBlockette100()) ) {
	    rec->samprate = b100->sample_rate;
	}
    }
    for(i = 0; i < seed_tables.size(); i++) {
	if(!seed_tables[i]->empty() &&
		seed_tables[i]->at(0)->nameIs("Data_Records")) {
	    seed_tables[i]->push_back(rec);
	    break;
	}
    }
    if(i == seed_tables.size()) {
	gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
	v->push_back(rec);
	seed_tables.push_back(v);
    }
}

void SeedSource::storeResponse(Station &sta, Channel &chan, Dictionary &d, const string &path)
{
    int i, j, k, in_units, out_units;
    bool scale_stage;
    Response *r;
    ChannelResponse *c = NULL;
    Blockette33 *b33;
    Blockette34 *b34;
    Blockette53 *b53;
    Blockette54 *b54;
    Blockette55 *b55;
    Blockette57 *b57;
    Blockette58 *b58;
    Blockette61 *b61;

    // check if already have this response
    for(i = 0; i < (int)responses.size(); i++) {
	if( sta.b50.network == responses[i]->network &&
	    sta.b50.station == responses[i]->station &&
	    chan.b52.channel == responses[i]->channel &&
	    chan.b52.location == responses[i]->location) return;
    }

    for(i = 0; i < (int)chan.response.size(); i++)
    {
	r = NULL;
	scale_stage = false;
	in_units = -1;
	out_units = -1;
	if( (b53 = chan.response[i]->getBlockette53()) ) {
	    r = new Response();
	    if(!c) { c = new ChannelResponse(sta, chan); responses.push_back(c); }
	    c->rsp.push_back(r);
	    r->source.assign("blockette53");
	    r->stage = b53->stage;
	    in_units = b53->input_units;
	    out_units = b53->output_units;
	    if(b53->type == "A") r->des.assign("Laplace rad/sec");
	    else if(b53->type == "B") r->des.assign("Analog Hz");
	    else if(b53->type == "C") r->des.assign("Composite");
	    else if(b53->type == "D") r->des.assign("Digital Z-tran");
	    r->type.assign("paz");
	    r->a0 = b53->a0_norm;
//	    r->calfreq = b53->norm_freq;
	    r->npoles = (int)b53->pr.size();
	    r->pole = (FComplex *)malloc(r->npoles*sizeof(FComplex));
	    r->pole_err = (FComplex *)malloc(r->npoles*sizeof(FComplex));
	    double fac = (b53->type == "B") ? 2*M_PI : 1.0;
	    for(j = 0; j < r->npoles; j++) {
		r->pole[j].re = b53->pr[j] * fac;
		r->pole[j].im = b53->pi[j] * fac;
		r->pole_err[j].re = b53->pr_error[j];
		r->pole_err[j].im = b53->pi_error[j];
	    }

	    r->nzeros = (int)b53->zr.size();
	    int nadd = 0;
	    if(r->nzeros > 0 || r->npoles > 0) {
		if( (b34 = d.getB34(in_units)) && !strcasecmp(b34->name.c_str(), "M/S")) {
		    // add one zero
		    nadd = 1;
		}
		else if( (b34 = d.getB34(in_units)) && !strcasecmp(b34->name.c_str(), "M/S**2")) {
		    // add two zeros
		    nadd = 2;
		}
	    }
	    r->nzeros = (int)b53->zr.size();
	    r->zero = (FComplex *)malloc((r->nzeros+nadd)*sizeof(FComplex));
	    r->zero_err = (FComplex *)malloc((r->nzeros+nadd)*sizeof(FComplex));
	    for(j = 0; j < r->nzeros; j++) {
		r->zero[j].re = b53->zr[j] * fac;
		r->zero[j].im = b53->zi[j] * fac;
		r->zero_err[j].re = b53->zr_error[j];
		r->zero_err[j].im = b53->zi_error[j];
	    }
	    for(j = 0; j < nadd; j++) {
		r->zero[r->nzeros+j].re = 0.0;
		r->zero[r->nzeros+j].im = 0.0;
		r->zero_err[r->nzeros+j].re = 0.0;
		r->zero_err[r->nzeros+j].im = 0.0;
	    }
	    r->nzeros += nadd;

	    // compute A0
	    double num_re=1.0, num_im=0.0, re, im, fre=0., fim;
	    fim = (b53->type == "A") ? 2.*M_PI*b53->norm_freq : b53->norm_freq;
	
	    for(j = 0; j < r->nzeros; j++) {
		re = num_re*(fre + r->zero[j].re) - num_im*(fim + r->zero[j].im);
		im = num_im*(fre + r->zero[j].re) + num_re*(fim + r->zero[j].im);
		num_re = re;
		num_im = im;
	    }
	    double dnom_re=1.0, dnom_im=0.0;
	    for(j = 0; j < r->npoles; j++) {
		re = dnom_re*(fre + r->pole[j].re) - dnom_im*(fim + r->pole[j].im);
		im = dnom_im*(fre + r->pole[j].re) + dnom_re*(fim + r->pole[j].im);
		dnom_re = re;
		dnom_im = im;
	    }
	    r->a0 = sqrt(dnom_re*dnom_re + dnom_im*dnom_im)/sqrt(num_re*num_re + num_im*num_im);
	}
	else if( (b54 = chan.response[i]->getBlockette54()) ) {
	    if((int)b54->numerator.size() == 0) {
		if(c && (int)c->rsp.size() > 0) {
		    r = c->rsp.back();
		    r->stage = b54->stage;
		    scale_stage = true;
		    out_units = b54->output_units;
		}
	    }
	    else {
		r = new Response();
		if(!c) { c = new ChannelResponse(sta, chan); responses.push_back(c); }
		c->rsp.push_back(r);
		r->source.assign("blockette54");
		r->stage = b54->stage;
		in_units = b54->input_units;
		out_units = b54->output_units;
		if(b54->type == "A") r->des.assign("Laplace rad/sec");
		else if(b54->type == "B") r->des.assign("Analog Hz");
		else if(b54->type == "C") r->des.assign("Composite");
		else if(b54->type == "D")r->des.assign("Digital Z-tran");
		r->type.assign("fir");
		r->num_n = (int)b54->numerator.size();
		r->fir_n = (float *)malloc(r->num_n*sizeof(float));
		r->fir_n_error = (float *)malloc(r->num_n*sizeof(float));
		for(j = 0; j < r->num_n; j++) {
		    r->fir_n[j] = b54->numerator[j];
		    r->fir_n_error[j] = b54->nerror[j];
		}

		r->num_d = (int)b54->denominator.size();
		r->fir_d = (float *)malloc(r->num_d*sizeof(float));
		r->fir_d_error = (float *)malloc(r->num_d*sizeof(float));
		for(j = 0; j < r->num_d; j++) {
		    r->fir_d[j] = b54->denominator[j];
		    r->fir_d_error[j] = b54->derror[j];
		}
	    }
	}
	else if( (b55 = chan.response[i]->getBlockette55()) ) {
	    r = new Response();
	    if(!c) { c = new ChannelResponse(sta, chan); responses.push_back(c); }
	    c->rsp.push_back(r);
	    r->source.assign("blockette55");
	    r->stage = b55->stage;
	    in_units = b55->input_units;
	    out_units = b55->output_units;
	    r->type.assign("fap");
	    r->nfap = (int)b55->frequency.size();
	    r->fap_f = (float *)malloc(r->nfap*sizeof(float));
	    r->fap_a = (float *)malloc(r->nfap*sizeof(float));
	    r->fap_p = (float *)malloc(r->nfap*sizeof(float));
	    r->amp_error = (float *)malloc(r->nfap*sizeof(float));
	    r->phase_error = (float *)malloc(r->nfap*sizeof(float));

	    for(j = 0; j < r->nfap; j++) {
		r->fap_f[j] = b55->frequency[j];
		r->fap_a[j] = b55->amplitude[j];
		r->fap_p[j] = b55->phase[j];
		r->amp_error[j] = b55->amp_error[j];
		r->phase_error[j] = b55->phase_error[j];
	    }
	}
	else if( (b61 = chan.response[i]->getBlockette61()) ) {
	    r = new Response();
	    if(!c) { c = new ChannelResponse(sta, chan); responses.push_back(c); }
	    c->rsp.push_back(r);
	    r->source.assign("blockette61");
	    r->stage = b61->stage;
	    in_units = b61->input_units;
	    out_units = b61->output_units;
	    r->type.assign("fir");
	    r->num_d = 0;
	    if(b61->symmetry_code == "A") {
		r->num_n = (int)b61->coef.size();
		r->fir_n = (float *)malloc(r->num_n*sizeof(float));
		r->fir_n_error = (float *)malloc(r->num_n*sizeof(float));
		for(j = 0; j < r->num_n; j++) {
		    r->fir_n[j] = b61->coef[j];
		    r->fir_n_error[j] = 0.;
		}
	    }
	    else if(b61->symmetry_code == "B") {
		int n = (int)b61->coef.size();
		r->num_n = 2*n - 1;
		r->fir_n = (float *)malloc(r->num_n*sizeof(float));
		r->fir_n_error = (float *)malloc(r->num_n*sizeof(float));
		for(j = 0; j < n-1; j++) {
		    r->fir_n[j] = b61->coef[j];
		    r->fir_n[r->num_n-1-j] = b61->coef[j];
		}
		r->fir_n[n-1] = b61->coef[n-1];
		for(j = 0; j < r->num_n; j++) r->fir_n_error[j] = 0.;
	    }
	    else if(b61->symmetry_code == "C") {
		int n = (int)b61->coef.size();
		r->num_n = 2*n;
		r->fir_n = (float *)malloc(r->num_n*sizeof(float));
		r->fir_n_error = (float *)malloc(r->num_n*sizeof(float));
		for(j = 0; j < n; j++) {
		    r->fir_n[j] = b61->coef[j];
		    r->fir_n[r->num_n-1-j] = b61->coef[j];
		}
		for(j = 0; j < r->num_n; j++) r->fir_n_error[j] = 0.;
	    }
	}

	if(r) {
	    for(j = 0; j < (int)chan.response.size(); j++) {
		if( (b57 = chan.response[j]->getBlockette57())
			&& b57->stage == r->stage)
		{
		    r->input_samprate = b57->input_sample_rate;
		    if( (k = (int)c->rsp.size()) > 1) {
			c->rsp[k-2]->output_samprate = r->input_samprate;
		    }
		    r->delay = b57->delay;
		    r->correction = b57->correction;
		}
	    }
	    for(j = 0; j < (int)chan.response.size(); j++) {
		if( (b58 = chan.response[j]->getBlockette58())
			&& b58->stage == r->stage)
		{
		    if(scale_stage) {
			r->b58_sensitivity *= b58->sensitivity;
		    }
		    else {
			r->b58_sensitivity = b58->sensitivity;
			r->b58_frequency = b58->frequency;
		    }
		}
	    }
	    if( (b34 = d.getB34(in_units)) ) {
		r->input_units.assign(b34->name);
	    }
	    if((int)c->rsp.size() == 1) {
		// get response units from b52. not always the same
		if( (b34 = d.getB34(chan.b52.signal_units)) ) {
		    r->response_units.assign(b34->name);
		}
	    }
	    else {
		r->response_units.assign(r->input_units);
	    }
	    if( (b34 = d.getB34(out_units)) ) {
		r->output_units.assign(b34->name);
	    }
	    if( (b33 = d.getB33(chan.b52.instrument)) ) {
		r->insname.assign(b33->description);
	    }
	    for(j = (int)path.length()-1; j > 0 && path[j] != '/'; j--);
	    if(path[j] == '/') j++;
	    r->author.assign(path.substr(j));
	}
    }
    // Set output sample rate of the last response to the data sample rate.
    if(c && c->rsp.back()) {
//	c->rsp.back()->output_samprate = sd.samprate();
	c->rsp.back()->output_samprate = chan.b52.sample_rate;
    }
}

/*
vector<Response *> * SeedSource::channelResponse(GTimeSeries *ts,bool print_err)
{
    string chan;

    for(int i = 0; i < (int)responses.size(); i++) {
//	chan = responses[i]->channel + responses[i]->location;
	chan = responses[i]->channel;
	if( responses[i]->network == ts->net() &&
	    responses[i]->station == ts->sta() &&
	    chan == ts->chan())
	{
	    return &responses[i]->rsp;
	}
    }
    return NULL;
}
*/
