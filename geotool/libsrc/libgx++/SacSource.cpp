/** \file SacSource.cpp
 *  \brief Defines class SacSource.
 *  \author Ivan Henson
 */
#include "config.h"
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#include <stdio.h>
#include <sys/param.h>
#include <pwd.h>
#include <errno.h>
#include <inttypes.h>

#include "SacSource.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GSourceInfo.h"
#include "libgio.h"
#include "gobject++/CssTableClass.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "sac.h"
#include "fnan.h"
}

class SacData
{
    public:
    SacData() {
	binary = false;
	flip_bytes = false;
	header_offset = 0;
	data_offset = 0;
	ref_time = 0.;
    }
    ~SacData() { }
    bool binary;
    bool flip_bytes;
    int header_offset;
    int data_offset;
    double ref_time;
    SAC sac;
};

#define file_warn(path) \
    if(errno > 0) { \
	snprintf(error, sizeof(error), "sac: cannot open %s\n%s", \
		path, strerror(errno)); \
    } \
    else { \
	snprintf(error, sizeof(error), "sac: cannot open %s", path); \
    } \
    logErrorMsg(LOG_WARNING, error);


static void sacToString(char *a, char *b, int n);
#ifdef HAVE_LIBZ
static int read_ascii_sac_header(gzFile fp, SAC *s, int *lineno,
			const char **err_msg);
static int readHeader(gzFile fp, SAC *sac, int *binary, int *lineno, int *flip_bytes,
			const char **err_msg);
static bool sacReadWaveform(gzFile fp, int binary, int flip_bytes, float *y, int *npts, SAC *sac);
#else
static int read_ascii_sac_header(FILE *fp, SAC *s, int *lineno, const char **err_msg);
static int readHeader(FILE *fp, SAC *sac, int *binary, int *lineno, int *flip_bytes, const char **err_msg);
static bool sacReadWaveform(FILE *fp, int binary, int flip_bytes, float *y, int *npts,SAC *sac);
#endif

static void getArrivals(DataSource *ds, SacData *sd, SegmentInfo *s,
		int path_quark, gvector<CssTableClass *> &v);
static void flipHeader(SAC *sac);
static void loadDatabase(GTimeSeries *ts, SAC *sac, SegmentInfo *s);


SacSource::SacSource(const string &name, const string &file) :
		TableSource(name)
{
    struct stat buf;
    string s;
    read_path = file;
    openPrefix(file);
    queryAllPrefixTables();

    s = file + ".arrival";
    css_arrival = (stat(s.c_str(), &buf) == 0) ? true : false;
}

SacSource::~SacSource(void)
{
}

gvector<SegmentInfo *> *SacSource::getSegmentList(void)
{
    int		path_quark, err, jdate, binary;
    int		lineno, header_offset, bad_time, flip_bytes;
    char	error[MAXPATHLEN+100];
    double	az, baz;
    char	name[MAXPATHLEN+100];
    const char	*err_msg = NULL;
    char	msg[1024];
    double	ref_time;
    SAC		sac;
#ifdef HAVE_LIBZ
    gzFile	fp;
#else
    FILE	*fp;
#endif
    DateTime	dt;
    cvector<CssArrivalClass> arrivals;
    gvector<CssTableClass *> origins, assocs, wftags;	
    gvector<SegmentInfo *> *segs;

    if(read_path.empty()) {
	return NULL;
    }
#ifdef HAVE_LIBZ
    if((fp = gzopen(read_path.c_str(), "rb")) == NULL)
#else
    if((fp = fopen(read_path.c_str(), "r")) == NULL)
#endif
    {
	file_warn(read_path.c_str());
	return NULL;
    }    

    for(int i = 0; i < (int)sac_data.size(); i++) delete sac_data[i];
    sac_data.clear();

    segs = new gvector<SegmentInfo *>;

    path_quark = (int)stringToQuark(read_path);

    header_offset = 0;
    lineno = 0;
    while(!(err = readHeader(fp, &sac, &binary, &lineno, &flip_bytes, &err_msg)))
    {
	if(fNaN(sac.b)) {
	    snprintf(error, sizeof(error),
		    "Header format error. Bad B: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    err = -1;
	    break;
	}
	if(fNaN(sac.e)) {
	    snprintf(error, sizeof(error),
		    "Header format error. Bad E: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    err = -1;
	    break;
	}
	if(fNaN(sac.delta) || sac.delta < 0.) {
	    snprintf(error, sizeof(error),
		    "Header format error. Bad DELTA: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    err = -1;
	    break;
	}
	if(sac.npts < 0) {
	    snprintf(error, sizeof(error),
		    "Header format error. Bad NPTS: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    err = -1;
	    break;
	}
	if( sac.iftype != ITIME && sac.iftype != IRLIM &&
	    sac.iftype != IAMPH && sac.iftype != IXY)
	{
	    snprintf(error, sizeof(error),
		    "Header format error. Bad IFTYPE: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    err = -1;
	    break;
	}
	if(sac.iftype != ITIME) {
#ifdef HAVE_LIBZ
	    gzseek(fp, (long)(sac.npts*sizeof(int)), 1);
#else
	    fseek(fp, (long)(sac.npts*sizeof(int)), 1);
#endif
	    continue;
	}
	if(sac.nzjday == -12345) sac.nzjday = 0;
	if(sac.nzyear == -12345) sac.nzyear = 0;
	if(sac.nzhour == -12345) sac.nzhour = 0;
	if(sac.nzmin == -12345) sac.nzmin = 0;
	if(sac.nzsec == -12345) sac.nzsec = 0;
	if(sac.nzmsec == -12345) sac.nzmsec = 0;

	bad_time = 0;
	if(sac.nzjday < 0 || sac.nzjday > 366) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzjday: %d in %s",
			sac.nzjday, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}
	if(!bad_time && (sac.nzyear < 0 || sac.nzyear > 5000)) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzyear: %d in %s",
			sac.nzyear, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}
	if(!bad_time && (sac.nzhour < 0 || sac.nzhour > 24)) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzhour: %d in %s",
			sac.nzhour, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}
	if(!bad_time && (sac.nzmin < 0 || sac.nzmin > 60)) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzmin: %d in %s",
			sac.nzmin, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}
	if(!bad_time && (sac.nzsec < 0 || sac.nzsec > 60)) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzsec: %d in %s",
			sac.nzsec, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}
	if(!bad_time && (sac.nzmsec < 0 || sac.nzmsec > 1000)) {
	    snprintf(error, sizeof(error),
			"Header format error. Bad nzmsec: %d in %s",
			sac.nzmsec, read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    sac.nzjday = 0;
	    sac.nzyear = 0;
	    sac.nzhour = 0;
	    sac.nzmin = 0;
	    sac.nzsec = 0;
	    sac.nzmsec = 0;
	    bad_time = 1;
	}

	snprintf(name, sizeof(name), "%s.%d", read_path.c_str(),
		(int)segs->size());

	SegmentInfo *s = new SegmentInfo();
	CssWfdiscClass *wf = new CssWfdiscClass();
	s->setWfdisc(wf);

	s->id = (int)sac_data.size() + 1;
	SacData *sd = new SacData();
	sac_data.push_back(sd);
	sd->sac = sac;
	
	s->path = path_quark;
	s->format = stringToQuark("sac");
	s->file_order = (int)segs->size();

	if(!strcmp(sac.kstnm, "-")) {
	    stringcpy(s->sta, "--", sizeof(s->sta));
	}
	else {
	    stringcpy(s->sta, sac.kstnm, sizeof(s->sta));
	}
	if(!strcmp(sac.kcmpnm, "-")) {
	    stringcpy(s->chan, "--", sizeof(s->chan));
	}
	else {
	    stringcpy(s->chan, sac.kcmpnm, sizeof(s->chan));
	}
	stringcpy(wf->sta, s->sta, sizeof(wf->sta));
	stringcpy(wf->chan, s->chan, sizeof(wf->chan));

	dt.year = sac.nzyear;
	timeMonthDay(dt.year, sac.nzjday, &dt.month, &dt.day);
	dt.hour = sac.nzhour;
	dt.minute = sac.nzmin;
	dt.second = (float)sac.nzsec + (float)sac.nzmsec/1000.;
	ref_time = timeDateToEpoch(&dt);
	s->start = ref_time + sac.b;
	wf->time = s->start;
	timeEpochToDate(s->start, &dt);
	jdate = timeJDate(&dt);

	s->nsamp = sac.npts;
	if(sac.odelta != FVAL_UNDEF && sac.odelta != 0.) {
	    s->samprate = 1./sac.odelta;
	}
	else if(sac.delta != 0.) {
	    s->samprate = 1./sac.delta;
	}	
	else {
	    s->samprate = 0.;
	}
	wf->samprate = s->samprate;

	if(s->samprate != 0.) {
	    s->end = s->start + (s->nsamp-1)/s->samprate;
	}
	else {
	    s->end = s->start;
	}
	s->jdate = jdate;
	wf->jdate = s->jdate;

	if(fNaN(sac.stla)) {
	    snprintf(error, sizeof(error),
		    "Header format error. bad STALA: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	}
	else if(sac.stla != FVAL_UNDEF) {
	    s->station_lat = sac.stla;
	}
	if(fNaN(sac.stlo)) {
	    snprintf(error, sizeof(error),
		    "Header format error. bad STALO: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	}
	else if(sac.stlo != FVAL_UNDEF) {
	    s->station_lon = sac.stlo;
	}

	s->origin_id = -1;
	if(fNaN(sac.o)) {
	    snprintf(error, sizeof(error),
			"Header format error. bad O: %s", read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	}
	else if(sac.o != FVAL_UNDEF)
	{
	    s->origin_id = s->id;
	    s->origin_time = sac.o + ref_time;
	    if(sac.evla != FVAL_UNDEF) s->origin_lat = sac.evla;
	    if(sac.evlo != FVAL_UNDEF) s->origin_lon = sac.evlo;
	    if(sac.evdp != FVAL_UNDEF) s->origin_depth = sac.evdp;
	    if(sac.gcarc != FVAL_UNDEF) s->origin_delta = sac.gcarc;
	}
	if(s->origin_lat != -999. && s->origin_lon != -999. &&
		s->station_lat != -999. && s->station_lon != -999.)
	{
	    deltaz(s->origin_lat, s->origin_lon, s->station_lat, s->station_lon,
			 &s->origin_delta, &az, &baz);
	}

	sd->binary = binary;
	sd->flip_bytes = flip_bytes;
	sd->header_offset = header_offset;
#ifdef HAVE_LIBZ
	sd->data_offset = gztell(fp);
#else
	sd->data_offset = ftell(fp);
#endif
       	sd->ref_time = ref_time;

	if(binary) {
#ifdef HAVE_LIBZ
	    gzseek(fp, (long)(sac.npts*sizeof(int)), 1);
#else
	    fseek(fp, (long)(sac.npts*sizeof(int)), 1);
#endif
	}
	else {
	    break;
	}
#ifdef HAVE_LIBZ
	header_offset = gztell(fp);
#else
	header_offset = ftell(fp);
#endif
	if(!css_arrival) { // don't have a .arrival file
	    getArrivals(this, sd, s, path_quark, arrivals);

	    if(s->origin_id >= 0) {
		int format = stringToQuark("sac");
		CssOriginClass *o = new CssOriginClass();
		origins.push_back(o);

		o->setIds(path_quark, (int)origins.size());
		o->setFormat(format);
		o->time = s->origin_time;
		o->lat = s->origin_lat;
		o->lon = s->origin_lon;
		o->depth = s->origin_depth;

		CssWftagClass *w = new CssWftagClass();
		wftags.push_back(w);

		w->setIds(path_quark, (int)wftags.size());
		w->setFormat(format);
		w->tagid = o->orid;
		w->wfid = (int)wftags.size();

		for(int i = 0; i < arrivals.size(); i++)
		{
		    CssAssocClass *a = new CssAssocClass();
		    assocs.push_back(a);
	
		    a->setIds(path_quark, (int)assocs.size());
		    a->setFormat(format);
		    a->arid = arrivals[i]->arid;
		    a->orid = o->orid;
		    stringcpy(a->sta, arrivals[i]->sta, sizeof(a->sta));
		    stringcpy(a->phase, arrivals[i]->iphase, sizeof(a->phase));
		}
	    }
	}

	s->selected = true;
	segs->push_back(s);
    }
#ifdef HAVE_LIBZ
    gzclose(fp);
#else
    fclose(fp);
#endif
    storeRecords(arrivals);
    storeRecords(origins);
    storeRecords(assocs);
    storeRecords(wftags);

    if(err < -1)
    {
	if(binary) {
	    logErrorMsg(LOG_WARNING, strerror(errno));
	}
	snprintf(error, sizeof(error), "header read error: %s",
		read_path.c_str());
	logErrorMsg(LOG_WARNING, error);

	if(err_msg != NULL) {
	    sprintf(msg, "line %d: %s", lineno, err_msg);
	    logErrorMsg(LOG_WARNING, msg);
	}
	if((int)segs->size() == 0) {
	    delete segs;
	    return NULL;
	}
    }
    else if(err == -1 && (int)segs->size() == 0)
    {
	snprintf(error, sizeof(error), "header read error: %s",
		read_path.c_str());
	logErrorMsg(LOG_WARNING, error);

	if(err_msg != NULL) {
	    sprintf(msg, "line %d: %s", lineno, err_msg);
	    logErrorMsg(LOG_WARNING, msg);
	}
	delete segs;
	return NULL;
    }

    return segs;
}

static int
#ifdef HAVE_LIBZ
readHeader(gzFile fp, SAC *sac, int *binary, int *lineno, int *flip_bytes, const char **err_msg)
#else
readHeader(FILE *fp, SAC *sac, int *binary, int *lineno, int *flip_bytes, const char **err_msg)
#endif
{
    int pos, err;
    SAC_HEADER s, sac_header_null = SAC_HEADER_NULL;

    memcpy(&s, &sac_header_null, sizeof(SAC_HEADER));
    /* try ascii first
     */
#ifdef HAVE_LIBZ
    pos = gztell(fp);
#else
    pos = ftell(fp);
#endif
    if(!(err = read_ascii_sac_header(fp, sac, lineno, err_msg))) {
	*binary = 0;
	return(0);
    }
    else if(err == EOF) {
	return(-1);
    }
    *binary = 1;
    memcpy(&s, &sac_header_null, sizeof(SAC_HEADER));

#ifdef HAVE_LIBZ
    gzseek(fp, pos, 0);
    if(gzread(fp, &s, sizeof(SAC_HEADER)) != sizeof(SAC_HEADER)) {
	return( gzeof(fp) ? -1 : -2 );
    }
#else
    fseek(fp, pos, 0);
    if(fread(&s, sizeof(SAC_HEADER), 1, fp) != 1) {
	return( feof(fp) ? -1 : -2 );
    }
#endif

    memcpy(sac, &s.a, sizeof(SAC_HEADER_A));

    if(sac->nzyear != -12345 && (sac->nzyear < 1900 || sac->nzyear > 2100)) {
	flipHeader(sac);
	if(sac->nzyear == -12345 || (sac->nzyear >= 1900 && sac->nzyear <= 2100)) {
	    *flip_bytes = true;
	}
	else {
	    memcpy(sac, &s.a, sizeof(SAC_HEADER_A));
	}
    }

    /* left justify all character strings
     */
    sacToString(s.b.kstnm, sac->kstnm, 8);
    sacToString(s.b.kevnm, sac->kevnm, 16);
    sacToString(s.b.khole, sac->khole, 8);
    sacToString(s.b.ko, sac->ko, 8);
    sacToString(s.b.ka, sac->ka, 8);
    sacToString(s.b.kt0, &sac->kt0[0], 8);
    sacToString(s.b.kt1, &sac->kt1[0], 8);
    sacToString(s.b.kt2, &sac->kt2[0], 8);
    sacToString(s.b.kt3, &sac->kt3[0], 8);
    sacToString(s.b.kt4, &sac->kt4[0], 8);
    sacToString(s.b.kt5, &sac->kt5[0], 8);
    sacToString(s.b.kt6, &sac->kt6[0], 8);
    sacToString(s.b.kt7, &sac->kt7[0], 8);
    sacToString(s.b.kt8, &sac->kt8[0], 8);
    sacToString(s.b.kt9, &sac->kt9[0], 8);
    sacToString(s.b.kf, sac->kf, 8);
    sacToString(s.b.kuser0, sac->kuser0, 8);
    sacToString(s.b.kuser1, sac->kuser1, 8);
    sacToString(s.b.kuser2, sac->kuser2, 8);
    sacToString(s.b.kcmpnm, sac->kcmpnm, 8);
    sacToString(s.b.knetwk, sac->knetwk, 8);
    sacToString(s.b.kdatrd, sac->kdatrd, 8);
    sacToString(s.b.kinst, sac->kinst, 8);
	
    return(0);
}

static void flipHeader(SAC *sac)
{
    union
    {
	char    a[4];
	float   f;
	int    i;
	short   s;
    } e1, e2;
    int i;

    float *f = &sac->delta;
    int *l = &sac->nzyear;
    int n_floats = ((uintptr_t)&sac->nzyear - (uintptr_t)&sac->delta)/4;
    int n_ints = ((uintptr_t)&sac->unused27 - (uintptr_t)&sac->unused12)/4;

    for(i = 0; i < n_floats; i++) {
	e1.f = f[i];
	e2.a[0] = e1.a[3];
	e2.a[1] = e1.a[2];
	e2.a[2] = e1.a[1];
	e2.a[3] = e1.a[0];
	f[i] = e2.f;
    }
    for(i = 0; i < n_ints; i++) {
	e1.i = l[i];
	e2.a[0] = e1.a[3];
	e2.a[1] = e1.a[2];
	e2.a[2] = e1.a[1];
	e2.a[3] = e1.a[0];
	l[i] = e2.i;
    }
}

/* a has no '\0' at the end. All n bytes are used as characters.
 * copy a to b, left-justifying and adding '\0'.
 */
static void
sacToString(char *a, char *b, int n)
{
    char c[20];
	
    if(a[0] == '\0')
    {
	strncpy(b, "-", n);
	return;
    }

    strncpy(c, a, n);
    c[n] = '\0';
    sscanf(c, "%s", b);
    if(!strcmp(b, KVAL_UNDEF))
    {
	strncpy(b, "-", n);
    }
}

bool SacSource::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    char	error[MAXPATHLEN+100];
    const char	*full_path;
    string	dir, prefix;
    float	*data;
    double	calib;
    SacData	*sd;
#ifdef HAVE_LIBZ
    gzFile	fp;
#else
    FILE	*fp;
#endif

    if(s->id < 1 || s->id > (int)sac_data.size()) return false;
    sd = sac_data[s->id-1];

    *err_msg = NULL;

    full_path = (const char *)quarkToString(s->path);
    getDir(full_path, dir, prefix);

#ifdef HAVE_LIBZ
    if((fp = gzopen(full_path, "r")) == NULL) {
#else
    if((fp = fopen(full_path, "r")) == NULL) {
#endif
	file_warn(full_path);
	return false;
    }

#ifdef HAVE_LIBZ
    if(gzseek(fp, sd->data_offset, 0) == -1)
#else
    if(fseek(fp, sd->data_offset, 0))
#endif
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error), "sac: cannot fseek in\n%s\n%s",
			full_path, strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error), "sac: cannot fseek in %s",full_path);
	}
	logErrorMsg(LOG_WARNING, error);
#ifdef HAVE_LIBZ
	gzclose(fp);
#else
	fclose(fp);
#endif
	return false;
    }

    data = (float *)malloc(s->nsamp*sizeof(float));

    if(!sacReadWaveform(fp, sd->binary, sd->flip_bytes, data, &s->nsamp, &sd->sac))
    {
	if(s->nsamp <= 0)
	{
	    snprintf(error, sizeof(error),"read error: %s", full_path);
	    logErrorMsg(LOG_WARNING, error);
#ifdef HAVE_LIBZ
	    gzclose(fp);
#else
	    fclose(fp);
#endif
	    return false;
	}
	else if(s->nsamp != sd->sac.npts)
	{
	    sd->sac.npts = s->nsamp;
	    snprintf(error, sizeof(error),"short read: %s", full_path);
	    logErrorMsg(LOG_WARNING, error);
	}
    }
#ifdef HAVE_LIBZ
    gzclose(fp);
#else
    fclose(fp);
#endif

    if(sd->sac.scale != FVAL_UNDEF && sd->sac.scale != 0.) {
	calib = sd->sac.scale;
    }
    else {
	calib = 1.;
    }

    if(*ts == NULL) {
	*ts = new GTimeSeries();
        (*ts)->waveform_io = new WaveformIO();
	(*ts)->putValue("dc", s->path);
	(*ts)->putValue("id", s->id);
	(*ts)->source_info.setSource(full_path);
	(*ts)->source_info.format = stringToQuark("sac");
	(*ts)->source_info.data_source = stringToQuark("sac");

	(*ts)->setDataSource(this);

	if(!pts) loadDatabase(*ts, &sd->sac, s);
    }

    GSegment *segment = readSeg(s, data, calib, tbeg, tend);
    if(!segment) return false;

    (*ts)->addSegment(segment);
    Free(data);

    WfdiscPeriod wp;
    wp.tbeg = segment->tbeg();
    wp.tend = segment->tend();
    wp.dir = stringToQuark(dir.c_str());
    wp.prefix = stringToQuark(prefix.c_str());
    wp.wfdisc_file = stringToQuark(full_path);
    wp.wfdisc_index = s->id;
    wp.pts_needed = pts;
    wp.wf = *s->wfdisc();
    getChanid(wp.wf.sta, wp.wf.chan, &wp.wf.chanid);

    if((*ts)->waveform_io) (*ts)->waveform_io->wp.push_back(wp);

    return true;
}

GSegment * SacSource::readSeg(SegmentInfo *s, float *data, double calib,
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

    return new GSegment(data+start, npts, tbeg, tdel, calib, 1.);
}

static bool
#ifdef HAVE_LIBZ
sacReadWaveform(gzFile fp, int binary, int flip_bytes, float *y, int *npts, SAC *sac)
#else
sacReadWaveform(FILE *fp, int binary, int flip_bytes, float *y, int *npts, SAC *sac)
#endif
{
    int i, n;
    union
    {
	char    a[4];
	float   f;
	int     i;
	short   s;
    } e1, e2;

    if(binary) {
#ifdef HAVE_LIBZ
	n = gzread(fp, y, (size_t)*npts*sizeof(float))/sizeof(float);
#else
	n = fread(y, sizeof(float), (size_t)*npts, fp);
#endif
	if(flip_bytes) {
	    for(i = 0; i < n; i++) {
		e1.f = y[i];
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		y[i] = e2.f;
	    }
	}
	if((i = invalidData(n, y, (float)0.)) > 0) {
	    char error[100];
	    snprintf(error, 100,
		    "sacIO: setting %d invalid binary data to 0.", i);
	    logErrorMsg(LOG_WARNING, error);
	}
    }
    else
    {
#ifdef HAVE_LIBZ
	int c;
	char s[51];
	for(n = 0; n < *npts; n++) {
	    while((c = gzgetc(fp)) != -1 && isspace(c));
	    if(c == -1) break;
	    s[0] = (char)c;
	    for(i=1; i<50 && (c = gzgetc(fp)) != -1 && !isspace(c); i++) {
		s[i] = (char)c;
	    }
	    s[i] = '\0';
	    if(sscanf(s, "%e", y+n) != 1) break;
	}
#else
	for(n = 0; n < *npts; n++) {
	    if(fscanf(fp, "%e", y+n) != 1) break;
	}
#endif
    }

    if(n != *npts)
    {
	if(binary) {
	    logErrorMsg(LOG_WARNING, strerror(errno));
	}
	*npts = n;
	return false;
    }
    *npts = n;
    return true;
}


bool SacSource::reread(GTimeSeries *ts)
{
    int		j, n;
    char	error[MAXPATHLEN+100];
    const char *path;
#ifdef HAVE_LIBZ
    gzFile	fp;
#else
    FILE	*fp;
#endif
    SacData	*sd;
    double	tbeg, tend;
    float	*data = NULL;

    if( !ts->waveform_io ) {
	logErrorMsg(LOG_WARNING, "sac.reread: cannot re-read data");
	return false;
    }

    if( (tbeg = ts->originalStart()) == NULL_TIME) tbeg = ts->tbeg();
    if( (tend = ts->originalEnd()) == NULL_TIME) tend = ts->tend();

    ts->removeAllSegments();

    data = (float *)malloc(sizeof(float));
    path = read_path.c_str();

    for(int i = 0; i < (int)ts->waveform_io->wp.size(); i++)
    {
	j = ts->waveform_io->wp[i].wfdisc_index - 1;
	if(j < 0 || j >= (int)sac_data.size()) continue;
	sd = sac_data[j];

#ifdef HAVE_LIBZ
	if((fp = gzopen(path, "rb")) == NULL) {
#else
	if((fp = fopen(path, "r")) == NULL) {
#endif
	    snprintf(error, sizeof(error), "sac: cannot open %s", path);
	    logErrorMsg(LOG_WARNING, error);
#ifdef HAVE_LIBZ
	    gzclose(fp);
#else
	    fclose(fp);
#endif
	    Free(data);
	    return false;
	}
#ifdef HAVE_LIBZ
	if(gzseek(fp, sd->data_offset, 0) == -1) {
#else
	if(fseek(fp, sd->data_offset, 0)) {
#endif
	    snprintf(error, sizeof(error), "%s: read error", path);
	    logErrorMsg(LOG_WARNING, error);
#ifdef HAVE_LIBZ
	    gzclose(fp);
#else
	    fclose(fp);
#endif
	    Free(data);
	    return false;
	}

	data = (float *)realloc(data, sd->sac.npts*sizeof(float));

	n = sd->sac.npts;

	if(!sacReadWaveform(fp, sd->binary, sd->flip_bytes, data, &n, &sd->sac)) {
	    if(n <= 0) {
		snprintf(error, sizeof(error), "read error: %s", path);
		logErrorMsg(LOG_WARNING, error);
		Free(data);
		return false;
	    }
	}
#ifdef HAVE_LIBZ
	gzclose(fp);
#else
	fclose(fp);
#endif

	if(n > 0) {
	    double t0 = sd->ref_time + sd->sac.b;
	    double tdel = 1., calib = 1.;
	    if(sd->sac.odelta != FVAL_UNDEF && sd->sac.odelta != 0.) {
		tdel = sd->sac.odelta;
	    }
	    else if(sd->sac.delta != 0.) {
		tdel = sd->sac.delta;
	    }
	    if(sd->sac.scale != FVAL_UNDEF && sd->sac.scale != 0.) {
		calib = sd->sac.scale;
	    }
	    GSegment *segment = new GSegment(data, n, t0, tdel, calib, 1.);
	    ts->addSegment(segment);
	}
    }
    Free(data);

    ts->truncate(tbeg, tend);

    return true;
}

static void
loadDatabase(GTimeSeries *ts, SAC *sac, SegmentInfo *s)
{
    if(ts == NULL) return;

    ts->setSta(s->sta);
    ts->setChan(s->chan);
    ts->setOriginalStart(s->start);
    ts->setJDate(s->jdate);
    ts->setOriginalEnd(s->end);

// ******** should check for null sac value, or set nulls in sac
    ts->setLat(s->station_lat);
    ts->setLon(s->station_lon);
    ts->setHang(sac->cmpaz);
    ts->setVang(sac->cmpinc);
}

static void
getArrivals(DataSource *ds, SacData *sd, SegmentInfo *s, int path_quark,
		gvector<CssTableClass *> &v)
{
    char	*kt[10];
    float	*atime;
    CssArrivalClass	*a;
    int dc = path_quark;

    atime = &sd->sac.t0;

    kt[0] = sd->sac.kt0;
    kt[1] = sd->sac.kt1;
    kt[2] = sd->sac.kt2;
    kt[3] = sd->sac.kt3;
    kt[4] = sd->sac.kt4;
    kt[5] = sd->sac.kt5;
    kt[6] = sd->sac.kt6;
    kt[7] = sd->sac.kt7;
    kt[8] = sd->sac.kt8;
    kt[9] = sd->sac.kt9;

    for(int i = 0; i < 10; i++) if(atime[i] != FVAL_UNDEF)
    {
	a = new CssArrivalClass();
	a->setDataSource(ds);
	a->setIds(dc, i+1);

	a->time = sd->ref_time + atime[i];
	stringcpy(a->iphase, kt[i], sizeof(a->iphase));
	stringcpy(a->sta, sd->sac.kstnm, sizeof(a->sta));

	stringcpy(a->chan, sd->sac.kcmpnm, sizeof(a->chan));
	a->amp_cnts = -1.;
	a->amp_Nnms = -1.;
	a->amp_nms = -1.;
	a->period = -1.;
	a->box_location = false;
	a->boxtime = 0.0;
	a->boxmin = 0.0;

	a->putValue("sta", sd->sac.kstnm);
	a->putValue("chan", sd->sac.kcmpnm);
	v.push_back(a);
    }
}

/*
 * NAME
 *      routines for reading sac formatted headers
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

extern "C" {
#include <string.h>
#include "libstring.h"
}

typedef struct
{
	int		start;
	int		end;
	const char	name[12];
	const char	format[4];
	int		offset;
	int		size;
} SacLine;

#define WRONG_FORMAT 1

#define MAX_ERROR_LEN 1024
static char error[MAX_ERROR_LEN];

static void check_for_null(char *s, int size);
static int read_sac_line(const char *line, SAC *s, SacLine *table, int n);

#define offset(field) \
((unsigned int) (((const char *) (&(((SAC*)NULL)->field))) - ((const char *) NULL)))

#define Sizeof(field) sizeof(((SAC*)NULL)->field)

static int
#ifdef HAVE_LIBZ
read_ascii_sac_header(gzFile fp, SAC *s, int *lineno, const char **err_msg)
#else
read_ascii_sac_header(FILE *fp, SAC *s, int *lineno, const char **err_msg)
#endif
{
	char line[100];
	int c, i, k, m, n;
	SacLine table[] =
	{
	    { 1, 15,     "delta",  "%f", offset(delta),     Sizeof(delta)},
	    {16, 30,    "depmin",  "%f", offset(depmin),    Sizeof(depmin)},
	    {31, 45,    "depmax",  "%f", offset(depmax),    Sizeof(depmax)},
	    {46, 60,     "scale",  "%f", offset(scale),     Sizeof(scale)},
	    {61, 75,    "odelta",  "%f", offset(odelta),    Sizeof(odelta)},
	    { 1, 15,         "b",  "%f", offset(b),         Sizeof(b)},
	    {16, 30,         "e",  "%f", offset(e),         Sizeof(e)},
	    {31, 45,         "o",  "%f", offset(o),         Sizeof(o)},
	    {46, 60,         "a",  "%f", offset(a),         Sizeof(a)},
	    {61, 75, "internal1",  "%f", offset(internal1), Sizeof(internal1)},
	    { 1, 15,        "t0",  "%f", offset(t0),        Sizeof(t0)},
	    {16, 30,        "t1",  "%f", offset(t1),        Sizeof(t1)},
	    {31, 45,        "t2",  "%f", offset(t2),        Sizeof(t2)},
	    {46, 60,        "t3",  "%f", offset(t3),        Sizeof(t3)},
	    {61, 75,        "t4",  "%f", offset(t4),        Sizeof(t4)},
	    { 1, 15,        "t5",  "%f", offset(t5),        Sizeof(t5)},
	    {16, 30,        "t6",  "%f", offset(t6),        Sizeof(t6)},
	    {31, 45,        "t7",  "%f", offset(t7),        Sizeof(t7)},
	    {46, 60,        "t8",  "%f", offset(t8),        Sizeof(t8)},
	    {61, 75,        "t9",  "%f", offset(t9),        Sizeof(t9)},
	    { 1, 15,         "f",  "%f", offset(f),         Sizeof(f)},
	    {16, 30,     "resp0",  "%f", offset(resp0),     Sizeof(resp0)},
	    {31, 45,     "resp1",  "%f", offset(resp1),     Sizeof(resp1)},
	    {46, 60,     "resp2",  "%f", offset(resp2),     Sizeof(resp2)},
	    {61, 75,     "resp3",  "%f", offset(resp3),     Sizeof(resp3)},
	    { 1, 15,     "resp4",  "%f", offset(resp4),     Sizeof(resp4)},
	    {16, 30,     "resp5",  "%f", offset(resp5),     Sizeof(resp5)},
	    {31, 45,     "resp6",  "%f", offset(resp6),     Sizeof(resp6)},
	    {46, 60,     "resp7",  "%f", offset(resp7),     Sizeof(resp7)},
	    {61, 75,     "resp8",  "%f", offset(resp8),     Sizeof(resp8)},
	    { 1, 15,     "rezp9",  "%f", offset(rezp9),     Sizeof(rezp9)},
	    {16, 30,      "stla",  "%f", offset(stla),      Sizeof(stla)},
	    {31, 45,      "stlo",  "%f", offset(stlo),      Sizeof(stlo)},
	    {46, 60,      "stel",  "%f", offset(stel),      Sizeof(stel)},
	    {61, 75,      "stdp",  "%f", offset(stdp),      Sizeof(stdp)},
	    { 1, 15,      "evla",  "%f", offset(evla),      Sizeof(evla)},
	    {16, 30,      "evlo",  "%f", offset(evlo),      Sizeof(evlo)},
	    {31, 45,      "evel",  "%f", offset(evel),      Sizeof(evel)},
	    {46, 60,      "evdp",  "%f", offset(evdp),      Sizeof(evdp)},
	    {61, 75,   "unused1",  "%f", offset(unused1),   Sizeof(unused1)},
	    { 1, 15,     "user0",  "%f", offset(user0),     Sizeof(user0)},
	    {16, 30,     "user1",  "%f", offset(user1),     Sizeof(user1)},
	    {31, 45,     "user2",  "%f", offset(user2),     Sizeof(user2)},
	    {46, 60,     "user3",  "%f", offset(user3),     Sizeof(user3)},
	    {61, 75,     "user4",  "%f", offset(user4),     Sizeof(user4)},
	    { 1, 15,     "user5",  "%f", offset(user5),     Sizeof(user5)},
	    {16, 35,     "user6",  "%f", offset(user6),     Sizeof(user6)},
	    {31, 45,     "user7",  "%f", offset(user7),     Sizeof(user7)},
	    {46, 60,     "user8",  "%f", offset(user8),     Sizeof(user8)},
	    {61, 75,     "user9",  "%f", offset(user9),     Sizeof(user9)},
	    { 1, 15,	 "dist",  "%f", offset(dist),       Sizeof(dist)},
	    {16, 30,        "az",  "%f", offset(az),        Sizeof(az)},
	    {31, 45,       "baz",  "%f", offset(baz),       Sizeof(baz)},
	    {46, 60,     "gcarc",  "%f", offset(gcarc),     Sizeof(gcarc)},
	    {61, 75, "internal2",  "%f", offset(internal2), Sizeof(internal2)},
	    { 1, 15, "internal3",  "%f", offset(internal3), Sizeof(internal3)},
	    {16, 35,    "depmen",  "%f", offset(depmen),    Sizeof(depmen)},
	    {31, 45,     "cmpaz",  "%f", offset(cmpaz),     Sizeof(cmpaz)},
	    {46, 60,    "cmpinc",  "%f", offset(cmpinc),    Sizeof(cmpinc)},
	    {61, 75,   "unused2",  "%f", offset(unused2),   Sizeof(unused2)},
	    { 1, 15,   "unused3",  "%f", offset(unused3),   Sizeof(unused3)},
	    {16, 30,   "unused4",  "%f", offset(unused4),   Sizeof(unused4)},
	    {31, 45,   "unused5",  "%f", offset(unused5),   Sizeof(unused5)},
	    {46, 60,   "unused6",  "%f", offset(unused6),   Sizeof(unused6)},
	    {61, 75,   "unused7",  "%f", offset(unused7),   Sizeof(unused7)},
	    { 1, 15,   "unused8",  "%f", offset(unused8),   Sizeof(unused8)},
	    {16, 35,   "unused9",  "%f", offset(unused9),   Sizeof(unused9)},
	    {31, 45,  "unused10",  "%f", offset(unused10),  Sizeof(unused10)},
	    {46, 60,  "unused11",  "%f", offset(unused11),  Sizeof(unused11)},
	    {61, 75,  "unused12",  "%f", offset(unused12),  Sizeof(unused12)},

	    { 1, 10,    "nzyear",  "%d", offset(nzyear),    Sizeof(nzyear)},
	    {11, 20,    "nzjday",  "%d", offset(nzjday),    Sizeof(nzjday)},
	    {21, 30,    "nzhour",  "%d", offset(nzhour),    Sizeof(nzhour)},
	    {31, 40,     "nzmin",  "%d", offset(nzmin),     Sizeof(nzmin)},
	    {41, 50,     "nzsec",  "%d", offset(nzsec),     Sizeof(nzsec)},
	    { 1, 10,    "nzmsec",  "%d", offset(nzmsec),    Sizeof(nzmsec)},
	    {11, 20, "internal4",  "%d", offset(internal4), Sizeof(internal4)},
	    {21, 30, "internal5",  "%d", offset(internal5), Sizeof(internal5)},
	    {31, 40, "internal6",  "%d", offset(internal6), Sizeof(internal6)},
	    {41, 50,      "npts",  "%d", offset(npts),      Sizeof(npts)},
	    { 1, 10, "internal7",  "%d", offset(internal7), Sizeof(internal7)},
	    {11, 20, "internal8",  "%d", offset(internal8), Sizeof(internal8)},
	    {21, 30,  "unused13",  "%d", offset(unused13),  Sizeof(unused13)},
	    {31, 40,  "unused14",  "%d", offset(unused14),  Sizeof(unused14)},
	    {41, 50,  "unused15",  "%d", offset(unused15),  Sizeof(unused15)},
	    { 1, 10,    "iftype",  "%d", offset(iftype),    Sizeof(iftype)},
	    {11, 20,      "idep",  "%d", offset(idep),      Sizeof(idep)},
	    {21, 30,    "iztype",  "%d", offset(iztype),    Sizeof(iztype)},
	    {31, 40,  "unused16",  "%d", offset(unused16),  Sizeof(unused16)},
	    {41, 50,     "iinst",  "%d", offset(iinst),     Sizeof(iinst)},
	    { 1, 10,    "istreg",  "%d", offset(istreg),    Sizeof(istreg)},
	    {11, 20,    "ievreg",  "%d", offset(ievreg),    Sizeof(ievreg)},
	    {21, 30,    "ievtyp",  "%d", offset(ievtyp),    Sizeof(ievtyp)},
	    {31, 40,     "iqual",  "%d", offset(iqual),     Sizeof(iqual)},
	    {41, 50,    "isynth",  "%d", offset(isynth),    Sizeof(isynth)},
	    { 1, 10,  "unused17",  "%d", offset(unused17),  Sizeof(unused17)},
	    {11, 20,  "unused18",  "%d", offset(unused18),  Sizeof(unused18)},
	    {21, 30,  "unused19",  "%d", offset(unused19),  Sizeof(unused19)},
	    {31, 40,  "unused20",  "%d", offset(unused20),  Sizeof(unused20)},
	    {41, 50,  "unused21",  "%d", offset(unused21),  Sizeof(unused21)},
	    { 1, 10,  "unused22",  "%d", offset(unused22),  Sizeof(unused22)},
	    {11, 20,  "unused23",  "%d", offset(unused23),  Sizeof(unused23)},
	    {21, 30,  "unused24",  "%d", offset(unused24),  Sizeof(unused24)},
	    {31, 40,  "unused25",  "%d", offset(unused25),  Sizeof(unused25)},
	    {41, 50,  "unused26",  "%d", offset(unused26),  Sizeof(unused26)},
	    { 1, 10,     "leven",  "%d", offset(leven),     Sizeof(leven)},
	    {11, 20,    "lpspol",  "%d", offset(lpspol),    Sizeof(lpspol)},
	    {21, 30,    "lovrok",  "%d", offset(lovrok),    Sizeof(lovrok)},
	    {31, 40,    "lcalda",  "%d", offset(lcalda),    Sizeof(lcalda)},
	    {41, 50,  "unused27",  "%d", offset(unused27),  Sizeof(unused27)},

	    { 1,  8,     "kstnm",  "%s", offset(kstnm),     Sizeof(kstnm)},
	    {16, 24,     "kevnm",  "%s", offset(kevnm),     Sizeof(kevnm)},
	    { 1,  8,     "khole",  "%s", offset(khole),     Sizeof(khole)},
	    { 9, 16,        "ko",  "%s", offset(ko),        Sizeof(ko)},
	    {17, 24,        "ka",  "%s", offset(ka),        Sizeof(ka)},
	    { 1,  8,       "kt0",  "%s", offset(kt0),       Sizeof(kt0)},
	    { 9, 16,       "kt1",  "%s", offset(kt1),       Sizeof(kt1)},
	    {17, 24,       "kt2",  "%s", offset(kt2),       Sizeof(kt2)},
	    { 1,  8,       "kt3",  "%s", offset(kt3),       Sizeof(kt3)},
	    { 9, 16,       "kt4",  "%s", offset(kt4),       Sizeof(kt4)},
	    {17, 24,       "kt5",  "%s", offset(kt5),       Sizeof(kt5)},
	    { 1,  8,       "kt6",  "%s", offset(kt6),       Sizeof(kt6)},
	    { 9, 16,       "kt7",  "%s", offset(kt7),       Sizeof(kt7)},
	    {17, 24,       "kt8",  "%s", offset(kt8),       Sizeof(kt8)},
	    { 1,  8,       "kt9",  "%s", offset(kt9),       Sizeof(kt9)},
	    { 9, 16,        "kf",  "%s", offset(kf),        Sizeof(kf)},
	    {17, 24,    "kuser0",  "%s", offset(kuser0),    Sizeof(kuser0)},
	    { 1,  8,    "kuser1",  "%s", offset(kuser1),    Sizeof(kuser1)},
	    { 9, 16,    "kuser2",  "%s", offset(kuser2),    Sizeof(kuser2)},
	    {17, 24,    "kcmpnm",  "%s", offset(kcmpnm),    Sizeof(kcmpnm)},
	    { 1,  8,    "knetwk",  "%s", offset(knetwk),    Sizeof(knetwk)},
	    { 9, 16,    "kdatrd",  "%s", offset(kdatrd),    Sizeof(kdatrd)},
	    {17, 24,     "kinst",  "%s", offset(kinst),     Sizeof(kinst)},

	    {0, 0, "",  "", 0, 0},
	};

	/* read 30 lines
	 */
	for(i = k = 0; i < 30; i++)
	{
	    m = i < 14 ? 75 : (i < 22 ? 50 : 24);

	    /* read the next m characters up to the next '\n'
	     */
#ifdef HAVE_LIBZ
	    for(n = 0; (c = gzgetc(fp)) != '\n' && c != EOF && n < m; n++)
#else
	    for(n = 0; (c = getc(fp)) != '\n' && c != EOF && n < m; n++)
#endif
	    {
		line[n] = c;
	    }
	    line[n] = '\0';
	    if(c == EOF)
	    {
		return(EOF);
	    }
	    if(c != '\n')
	    {
		/* read to the next '\n'
		 */
#ifdef HAVE_LIBZ
		while((c = gzgetc(fp)) != '\n' && c != EOF);
#else
		while((c = getc(fp)) != '\n' && c != EOF);
#endif
	    }
	    m = i < 14 ? 61 : (i < 22 ? 41 : (i == 22 ? 9 : 17));
	    if(n < m)
	    {
		stringcpy(error, "format error: short record.", MAX_ERROR_LEN);
		*err_msg = error;
		return(WRONG_FORMAT);
	    }

	    m = i < 22 ? 5 : (i == 22 ? 2 : 3);

	    if(read_sac_line(line, s, table+k, m))
	    {
		*err_msg = error;
		return(WRONG_FORMAT);
	    }
	    k += m;
	    (*lineno)++;
	}
	for(i = 110; i <= 132; i++)
	{
	    check_for_null((char *)s + table[i].offset, table[i].size);
	}
	return(0);
}

static void
check_for_null(char *s, int size)
{
	if(!strcmp(s, KVAL_UNDEF))
	{
	    stringcpy(s, "-", size);
	}
}

static int
read_sac_line(const char *line, SAC *s, SacLine *table, int n)
{
	int nc;
	char buf[20];

	while(--n >= 0)
	{
	    nc = table->end - table->start + 1;
	    strncpy(buf, line+table->start-1, nc);
	    buf[nc] = '\0';
	    if(sscanf(buf, table->format, (char *)s + table->offset) != 1)
	    {
		if(!strcmp(table->format, "%s"))
		{
		    /* null value */
		    strncpy((char *)s + table->offset, "-", table->size); 
		}
		else
		{
		    snprintf(error, MAX_ERROR_LEN,
			"format error: attribute name: %s", table->name);
		    return(1);
		}
	    }
	    table++;
	}
	return(0);
}
