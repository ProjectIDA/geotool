/** \file AscSource.cpp
 *  \brief Defines class AscSource.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sys/param.h>
#include <errno.h>

#include "AscSource.h"
#include "gobject++/GTimeSeries.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libtime.h"
}

class ExtraName
{
    public:
    ExtraName(const string &in_name, const string &in_value) {
	name = in_name;
	value = in_value;
    }
    ~ExtraName(void) {
    }
    string name;
    string value;
};

class AscData
{
    public:
    AscData() {
	npts = 0;
	data_size = 1000;
	data = (float *)malloc(data_size*sizeof(float));
    }
    ~AscData() {
	free(data);
	for(int i = 0; i < (int)extra.size(); i++) {
	    delete extra[i];
	}
    }
    CssWfdiscClass w;
    int npts;
    int data_size;
    float *data;
    vector<ExtraName *> extra;
};


AscSource::AscSource(const string &name, const string &file) :
	TableSource(name), read_path(""), line_no(0), last_newline(true)
{
    read_path = file;
#ifdef HAVE_LIBZ
    zfp = NULL;
#else
    fp = NULL;
#endif
    mode = READING_NONE;
    line_no = 0;
    last_newline = true;

    openPrefix(file);
    queryAllPrefixTables();
}

AscSource::~AscSource(void)
{
    closeFile();
    clear();
    asc_tables.clear();
}

void AscSource::removeDataReceiver(DataReceiver *owner)
{
    TableSource::removeDataReceiver(owner);

    if((int)receivers.size() == 0) {
	closeFile();
	clear();
	asc_tables.clear();
    }
}

gvector<SegmentInfo *> * AscSource::getSegmentList(void)
{
    int path_quark;
    CssWfdiscClass *wf;
    cvector<CssOriginClass> o;
    gvector<SegmentInfo *> *segs;

    if(read_path.empty()) return NULL;

    clear();
    queryAllPrefixTables();

    if( !openFile(read_path) ) return NULL;
    path_quark = (int)stringToQuark(read_path.c_str());

    segs = new gvector<SegmentInfo *>;

    readFile();

    closeFile();

    for(int i = 0; i < (int)asc_data.size(); i++)
    {
	AscData *asc = asc_data[i];
	SegmentInfo *s = new SegmentInfo();

	s->id = i+1;
	s->path = path_quark;
	s->format = stringToQuark("asc");
	s->file_order = i;
	if(asc->npts > 0) asc->w.nsamp = asc->npts;
	s->nsamp = asc->w.nsamp;
	s->samprate = asc->w.samprate;
	s->jdate = timeEpochToJDate(asc->w.time);
	s->start = asc->w.time;
	s->end = asc->w.time + (s->nsamp-1)/asc->w.samprate;
	stringcpy(s->sta, asc->w.sta, sizeof(s->sta));
	stringcpy(s->chan, asc->w.chan, sizeof(s->chan));
	s->selected = true;
	wf = new CssWfdiscClass();
	asc_data[i]->w.copyTo(wf, true);
	s->setWfdisc(wf);
	
	segs->push_back(s);
    }

    getNetworks(segs);

    getSites(segs);

    getSitechans(segs);

    if( getTable(o) > 0)
    {
	for(int i = 0; i < (int)segs->size(); i++) {
	    SegmentInfo *s = segs->at(i);
	    double tmin = s->start;
	    double tmax = s->end;
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

int AscSource::getTable(const string &cssTableName, gvector<CssTableClass *> &v)
{
    v.clear();
    for(int i = 0; i < asc_tables.size(); i++) {
	if(!asc_tables[i]->empty() && asc_tables[i]->at(0)->nameIs(cssTableName)){
	    v.load(asc_tables[i]);
	}
    }

    gvector<CssTableClass *> *t = TableSource::findTable(cssTableName);
    if(!t) return 0;

    v.ensureCapacity(v.size() + t->size());
    for(int i = 0; i < t->size(); i++) v.add(t->at(i));

    return v.size();
}

cvector<CssSiteClass> * AscSource::getSiteTable()
{
    if(!sites.empty()) {
	return &sites;
    }
    for(int i = 0; i < tables.size(); i++) {
	if( !tables[i]->empty() && tables[i]->at(0)->nameIs(cssSite) ) {
	    sites.load((cvector<CssSiteClass> *)tables[i]);
	}
    }

    // get global sites
    cvector<CssSiteClass> *s = TableSource::getSiteTable();

    if(s) {
	sites.ensureCapacity(sites.size() + s->size());
	for(int i = 0; i < s->size(); i++) sites.push_back(s->at(i));
    }

    return &sites;
}

cvector<CssSitechanClass> * AscSource::getSitechanTable()
{
    if(!sitechans.empty()) {
	return &sitechans;
    }
    for(int i = 0; i < tables.size(); i++) {
	if( !tables[i]->empty() && tables[i]->at(0)->nameIs(cssSitechan) ) {
	    sitechans.load((cvector<CssSitechanClass> *)tables[i]);
	}
    }

    // get global sitechans
    cvector<CssSitechanClass> *s = TableSource::getSitechanTable();

    if(s) {
	sitechans.ensureCapacity(sitechans.size() + s->size());
	for(int i = 0; i < s->size(); i++) sitechans.push_back(s->at(i));
    }

    return &sitechans;
}

cvector<CssAffiliationClass> * AscSource::getAffiliationTable()
{
    if(!affiliations.empty()) {
	return &affiliations;
    }
    for(int i = 0; i < tables.size(); i++) {
	if( !tables[i]->empty() && tables[i]->at(0)->nameIs(cssAffiliation) ) {
	    affiliations.load((cvector<CssAffiliationClass> *)tables[i]);
	}
    }

    // get global affiliations
    cvector<CssAffiliationClass> *s = TableSource::getAffiliationTable();

    if(s) {
	affiliations.ensureCapacity(affiliations.size() + s->size());
	for(int i = 0; i < s->size(); i++) affiliations.push_back(s->at(i));
    }

    return &affiliations;
}

bool AscSource::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    const char *full_path;
    string dir, prefix;
    int j;
    GSegment *segment;
    CssWfdiscClass *w;

    if(s->id < 1 || s->id > (int)asc_data.size()) return false;
    j = s->id-1;
    w = &asc_data[j]->w;

    if(w->dfile[0] != '\0' && strcmp(w->dfile, "-")) {
	return TableSource::makeTimeSeries(s, tbeg, tend, pts, ts, err_msg);
    }

    *err_msg = NULL;

    full_path = quarkToString(s->path);
    getDir(full_path, dir, prefix);

    if(*ts == NULL)
    {
	*ts = new GTimeSeries();
//	Component::printLog("%x %s/%s: waveform created.\n",
//		*ts, wfdisc.sta, wfdisc.chan);
	(*ts)->waveform_io = new WaveformIO();
	(*ts)->putValue("dc", s->path);
	(*ts)->putValue("id", s->id);
	(*ts)->source_info.setSource(full_path);
	(*ts)->source_info.format = stringToQuark("asc");
	(*ts)->source_info.data_source = stringToQuark("asc");
	for(int i = 0; i < s->array_elements.size(); i++) {
	    (*ts)->array_elements.add(s->array_elements[i]);
	}
    }

    segment = readSeg(s, asc_data[j], tbeg, tend);

    if(!segment) return false;

    WfdiscPeriod wp;
    wp.tbeg = segment->tbeg();
    wp.tend = segment->tend();
    wp.dir = stringToQuark(dir.c_str());
    wp.prefix = stringToQuark(prefix.c_str());
    wp.wfdisc_file = stringToQuark(full_path);
    wp.wfdisc_index = s->file_order;
    wp.pts_needed = pts;
    wp.wf = *s->wfdisc();
    getChanid(wp.wf.sta, wp.wf.chan, &wp.wf.chanid);

    if((*ts)->waveform_io) (*ts)->waveform_io->wp.push_back(wp);

    (*ts)->addSegment(segment);

    return true;
}

GSegment * AscSource::readSeg(SegmentInfo *s, AscData *asc,
			double start_time, double end_time)
{
    int npts, start;
    double tbeg, tdel;

    start = 0;
    if(start_time > s->start) {
	start = (int)((start_time - s->start)*s->samprate+.5);
    }

    if(end_time > s->end) {
	npts = s->nsamp - start;
    }
    else {
	npts = (int)(((end_time - s->start)*s->samprate+.5) - start + 1);
    }
    if(start + npts > s->nsamp) npts = s->nsamp - start;

    if(npts <= 0) return NULL;

    tdel = 1./s->samprate;
    tbeg = s->start + start*tdel;

    return new GSegment(asc->data+start, npts, tbeg, tdel, asc->w.calib,
			asc->w.calper);
}

bool AscSource::reread(GTimeSeries *ts)
{
    const char *err = NULL;
    double tbeg, tend, d;
    GSegment *segment;

    if( !ts->waveform_io ) {
	logErrorMsg(LOG_WARNING, "asc.reread: cannot re-read data");
	return false;
    }
    if((tbeg = ts->originalStart()) == NULL_TIME) tbeg = ts->tbeg();
    if((tend = ts->originalEnd()) == NULL_TIME) tend = ts->tend();

    ts->removeAllSegments();

    for(int i = 0; i < (int)ts->waveform_io->wp.size(); i++)
    {
	CssWfdiscClass *w = &ts->waveform_io->wp[i].wf;

	if(w->dfile[0] == '\0' || !strcmp(w->dfile, "-"))
	{
	    if(ts->waveform_io->wp[i].wfdisc_index < (int)asc_data.size()) {
		int j = ts->waveform_io->wp[i].wfdisc_index;
		segment = new GSegment(asc_data[j]->data, asc_data[j]->npts,
				w->time, 1./w->samprate, w->calib, w->calper);
		ts->addSegment(segment);
	    }
	}
	else {
	    segment = readSegment(&ts->waveform_io->wp[i].wf,
			quarkToString(ts->waveform_io->wp[i].dir), tbeg, tend,
			ts->waveform_io->wp[i].pts_needed);
	    if(segment != NULL) {
		ts->addSegment(segment);
	    }
	    if((err = cssioGetErrorMsg()) != NULL) {
		logErrorMsg(LOG_WARNING, err);
		GError::setMessage(err);
		break;
	    }
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

    return (!err) ? true : false;
}

bool AscSource::readFile(void)
{
    int i, n;
    double d;
    char *c, phrase[100], *name, *value;
    CssTableClass *css=NULL;

    for(i = 0; i < (int)asc_data.size(); i++) {
	delete asc_data[i];
    }
    asc_data.clear();
    asc_tables.clear();
    line_no = 0;
    last_newline = true;

    while(getPhrase(phrase, (int)sizeof(phrase)))
    {
	if((c = strstr(phrase, "="))) {
	    if(mode == READING_NONE) {
		cerr << "Format error: unexpected assignment before table name"
		    << "\nline " << line_no << " of " << read_path << endl;
		GError::setMessage(
		    "Format error: unexpected assignment before table name\n\
line %d of %s\n", line_no, read_path.c_str());
		return false;
	    }
	    else if(mode == READING_DATA) {
		cerr << "Format error: unexpected phrase " << phrase
		    << "\nline " << line_no << " of " << read_path << endl;
		GError::setMessage("Format error: unexpected phrase %s\n\
line %d of %s\n", phrase, line_no, read_path.c_str());
		return false;
	    }
	    *c = '\0';
	    name = phrase;
	    c++;
	    value = c;
	    if(*value == '\0') {
		cerr << "Format error: missing value for " << name
		    << "\nline " << line_no << " of " << read_path << endl;
		GError::setMessage("Format error: missing value for %s\n\
line %d of %s\n", name, line_no, read_path.c_str());
		return false;
	    }
	    else if(*value == '"') {
		value++;
		if(*value == '\0') {
		    cerr << "Format error: missing value for " << name
			<< "\nline " << line_no << " of " << read_path << endl;
		GError::setMessage("Format error: missing value for %s\n\
line %d of %s\n", name, line_no, read_path.c_str());
		    return false;
		}
		n = (int)strlen(value);
		if(value[n-1] != '"') {
		    cerr << "Format error: missing end-quote for " << name
			<< "\nline " << line_no << " of " << read_path <<endl;
		GError::setMessage("Format error: missing end-quote for %s\n\
line %d of %s\n", name, line_no, read_path.c_str());
		    return false;
		}
		value[n-1] = '\0';
	    }
	    if(mode == READING_WFDISC) {
		if( !readWfdiscValue(name, value) ) return false;
	    }
	    else if(mode == READING_TABLE) {
		if( !readTableValue(css, name, value) ) return false;
	    }
	}
	else if(!strcasecmp(phrase, cssWfdisc)) {
	    AscData *asc = new AscData();
	    asc->w.setDataSource(this);
	    asc_data.push_back(asc);
	    mode = READING_WFDISC;
	}
	else if(!strcasecmp(phrase, "data")) {
	    if((int)asc_data.size() == 0) {
		cerr << "Format error: data found before wfdisc\nline "
			<< line_no << " of " << read_path <<endl;
		GError::setMessage("Format error: data found before wfdisc\n\
line %d of %s\n", line_no, read_path.c_str());
		return false;
	    }
	    mode = READING_DATA;
	}
	else if(stringToDouble(phrase, &d)) {
	    if(mode == READING_DATA) {
		storeDataValue(d);
	    }
	    else {
		cerr << "Format error: unexpected data " << phrase
		    << "\nline " << line_no << " of " << read_path << endl;
		GError::setMessage("Format error: unexpected data %s\n\
line %d of %s\n", phrase, line_no, read_path.c_str());
		return false;
	    }
	}
	else {
	    if( (css = CssTableClass::createCssTable(phrase)) ) {
		css->setDataSource(this);
		mode = READING_TABLE;
		for(i = 0; i < tables.size(); i++) {
		    if( !tables[i]->empty() &&
			    tables[i]->at(0)->nameIs(css->getName()) )
		    {
			tables[i]->push_back(css);
			break;
		    }
		}
		if(i == tables.size()) {
		    gvector<CssTableClass *> *v = new gvector<CssTableClass *>;
		    v->push_back(css);
		    tables.push_back(v);
		}
	    }
	    else {
		cerr << "Format error: unknown name: " << phrase << "\nline "
			<< line_no << " of " << read_path << endl;
		GError::setMessage("Format error: unknown name: %s\n\
line %d of %s\n", phrase, line_no, read_path.c_str());
		return false;
	    }
	}
    }
    return true;
}

bool AscSource::getPhrase(char *phrase, int len)
{
    char *p = phrase;
    int c = '\0';

    if(len <= 0) return false;
    memset(phrase, 0, len);
    if(last_newline) line_no++;

    while((c = readChar()) != EOF && (c == '#' || isspace(c))) {
	if(c == '#' && last_newline) { // skip comment lines
	    while((c = readChar()) != EOF && c != '\n');
	    if(c == EOF) return false;
	    line_no++;
	}
	else if(c == '\n') line_no++;
    }
    if(c == EOF) return false;
    *p++ = c;
    len--;
    while(--len > 0 && (c = readChar()) != EOF && !isspace(c)) {
	*p++ = c;
	if(c == '"') {
	    while(--len > 0 && (c = readChar()) != EOF && c != '"' && c!='\n') {
		*p++ = c;
	    }
	    if(c == '"' && --len > 0) *p++ = c;
	    if(c == '\n') line_no++;
	    return true;
	}
    }
    last_newline = (c == '\n') ? true : false;

    return (phrase[0] != '\0') ? true : false;
}

bool AscSource::readWfdiscValue(const string &name, const string &value)
{
    if((int)asc_data.size() == 0) {
	cerr << "AscSource::readWfiscValue: no wfdisc found." << endl;
	return false;
    }
    AscData *asc = asc_data.back();

    int i;
    if((i = asc->w.memberIndex(name)) >= 0) {
	if(!asc->w.setMember(i, value)) {
	    cerr << "line " << line_no << " of " << read_path << endl;
	    GError::setMessage("Format error: invalid value: %s\n\
line %d of %s\n", value.c_str(), line_no, read_path.c_str());
	    return false;
	}
	if(asc->w.calib == 0.) asc->w.calib = 1.;
    }
    else {
	asc->extra.push_back(new ExtraName(name, value));
    }
    return true;
}

bool AscSource::readTableValue(CssTableClass *css, const string &name,
			const string &value)
{
    int i;
    if(!css) {
	cerr << "Format error: no table name before " << name
	     <<"\nline " << line_no << " of " << read_path << endl;
	return false;
    }
    if((i = css->memberIndex(name)) < 0) {
	cerr << "Format error: invalid " << css->getName()
	    << " member name: " << name <<"\nline " << line_no
		<< " of " << read_path << endl;
	GError::setMessage("Format error: invalid %s member name: %s\n\
line %d of %s\n", css->getName(), name.c_str(), line_no, read_path.c_str());
	return false;
    }
    if(!css->setMember(i, value)) {
	cerr << "line " << line_no << " of " << read_path << endl;
	GError::setMessage("Format error: invalid %s value: %s\n\
line %d of %s\n", css->getName(), value.c_str(), line_no, read_path.c_str());
	return false;
    }
    return true;
}

void AscSource::storeDataValue(double d)
{
    if((int)asc_data.size() == 0) {
	cerr << "AscSource::storeDataValue: no wfdisc found." << endl;
	GError::setMessage("Format error: data found before wfdisc.\n\
line %d of %s\n", line_no, read_path.c_str());
	return;
    }
    AscData *asc = asc_data.back();
    if(asc->npts >= asc->data_size) {
	asc->data_size += 1000;
	asc->data = (float *)realloc(asc->data, asc->data_size*sizeof(float));
    }
    asc->data[asc->npts++] = d;
}

bool AscSource::openFile(const string &path)
{
    struct stat buf;
    char error[MAXPATHLEN+50];

    closeFile();

#ifdef HAVE_LIBZ
    if(stat(path.c_str(), &buf) < 0 || S_ISDIR(buf.st_mode)
	|| (zfp = gzopen(path.c_str(), "rb")) == NULL)
#else
   if(stat(path.c_str(), &buf) < 0 || S_ISDIR(buf.st_mode)
	|| (fp = fopen(path.c_str(), "r")) == NULL)
#endif
    {
	snprintf(error, sizeof(error), "AscSource: cannot open %s",
			path.c_str());
	logErrorMsg(LOG_WARNING, error);
	GError::setMessage(error);
	return false;
    }

/*
    // check for SAC ascii file name "19970907_2350.YKB9_.shz"
    // in general: YYYYMMDD_HHMM.STA.CHAN ignore '_' in sta
    sac_ascii = false;
    DateTime dt;
    int i, j, n = (int)strlen(path);
    if(n > 16) {
	char s[20];
	strncpy(s, path, 4);
	s[4] = '\0';
	if(!stringToInt(s, &dt.year) || dt.year < 1950 || dt.year > 3000) {
	    return true;
	}
	strncpy(s, path+4, 2);
	s[3] = '\0';
	if(!stringToInt(s, &dt.month) || dt.month < 1 || dt.month > 12) {
	    return true;
	}
	strncpy(s, path+6, 2);
	s[3] = '\0';
	if(!stringToInt(s, &dt.day) || dt.day < 1 || dt.day > 31) {
	    return true;
	}
	strncpy(s, path+9, 2);
	s[3] = '\0';
	if(!stringToInt(s, &dt.hour) || dt.hour < 0 || dt.hour > 23) {
	    return true;
	}
	strncpy(s, path+11, 2);
	s[3] = '\0';
	if(!stringToInt(s, &dt.minute) || dt.minute < 0 || dt.minute > 59) {
	    return true;
	}
	dt.second = 0.;
	if(path[13] != '.') return true;
	for(i = 14; i < n && path[i] != '.' 
	time = timeDateToEpoch(&dt);
not finished
*/

    return true;
}

bool AscSource::closeFile(void)
{
#ifdef HAVE_LIBZ
    if(zfp) gzclose(zfp);
    zfp = NULL;
#else
    if(fp) fclose(fp);
    fp = NULL;
#endif
    return true;
}

void AscSource::clear(void)
{
    for(int i = 0; i < (int)asc_data.size(); i++) delete asc_data[i];
    asc_data.clear();
    tables.clear();
}

// static
bool AscSource::isAscFile(const string &path)
{
    int i;
    char phrase[100];
    AscSource *asc = new AscSource("tmp", path);

    if(!asc->openFile(path)) {
	delete asc;
	return false;
    }

    for(i = 0; i < 10; i++) {
	if(asc->getPhrase(phrase, (int)sizeof(phrase))) {
	    if(!strcasecmp(phrase, cssWfdisc) || CssTableClass::isTableName(phrase))
	    {
		break;
	    }
	}
    }
    asc->closeFile();
    asc->clear();

    delete asc;
    return (i < 10) ? true : false;
}

bool AscSource::output(const string &prefix, const string &access,
		gvector<Waveform *> &wvec, const string &remark,
		bool raw)
{
    char path[MAXPATHLEN+1];
    FILE *fp;

    snprintf(path, sizeof(path), "%s.asc", prefix.c_str());
    if( !(fp = fopen(path, access.c_str())) ) {
	GError::setMessage("Cannot open %s", path);
	return false;
    }
    if(!remark.empty()) {
	fprintf(fp, "# %s\n", remark.c_str());
    }

    for(int i = 0; i < wvec.size(); i++)
    {
	if(wvec[i]->num_dw <= 0)
	{
	    GTimeSeries *ts = new GTimeSeries(wvec[i]->ts);

	    if(raw) {
		if( !ts->removeAllMethods() ) {
		    GError::setMessage("Cannot reread %s/%s",
				ts->sta(), ts->chan());
		}
	    }
	    if( !write_ts(fp, ts, raw) ) {
		ts->deleteObject();
		fclose(fp);
		return false;
	    }
	    ts->deleteObject();
	    continue;
        }

	for(int j = 0; j < wvec[i]->num_dw; j++)
	{
	    GDataPoint *d1 = wvec[i]->dw[j].d1;
	    GDataPoint *d2 = wvec[i]->dw[j].d2;

	    GTimeSeries *ts = wvec[i]->ts->subseries(d1->time(), d2->time());
	    ts->setOriginalStart(d1->time());
            ts->setOriginalEnd(d2->time());
	    if(raw) {
		if( !ts->removeAllMethods() ) {
		    GError::setMessage("Cannot reread %s/%s",
				ts->sta(), ts->chan());
		}
	    }
	    if(!write_ts(fp, ts, raw) ) {
                ts->deleteObject();
		fclose(fp);
                return false;
            }
            ts->deleteObject();
        }
    }
    fclose(fp);
    return true;
}

bool AscSource::write_ts(FILE *fp, GTimeSeries *ts, bool raw)
{
    int i, j, jdate, n;
    double calib, calper, samprate;
    DateTime dt;
    char *date = timeLoadDate();
    bool calib_applied = false;

    if(raw) {
	calib_applied = ts->getMethod("CalibData") ? true : false;
    }

    for(j = 0; j < ts->size(); j++)
    {
	GSegment *s = ts->segment(j);
	samprate = 1./s->tdel();

	fprintf(fp,"wfdisc sta=%s chan=%s time=%.4lf nsamp=%d samprate=%.3lf\n",
		ts->sta(), ts->chan(), s->tbeg(), s->length(), samprate);
	timeEpochToDate(s->tbeg(), &dt);
	jdate = timeJDate(&dt);
	fprintf(fp, "chanid=%d instype=\"%s\" jdate=%d lddate=\"%s\"\n",
		ts->chanid(), ts->instype(), jdate, date);

	calib = (s->calib() != 0.) ? s->calib() : 1.;
	calper = (s->calper() != 0.) ? s->calper() : 1.;
	if(raw) {
	    fprintf(fp, "calib=%.4f calper=%.2f\n", calib, calper);
	}
	fprintf(fp, "data\n");
	n = s->length();
	if(calib_applied && calib != 1.) {
	    for(i = 0; i < n; i++) {
		fprintf(fp, "%.6g\n", s->data[i]/calib);
	    }
	}
	else {
	    for(i = 0; i < n; i++) {
		fprintf(fp, "%.6g\n", s->data[i]);
	    }
	}
    }
    return true;
}
