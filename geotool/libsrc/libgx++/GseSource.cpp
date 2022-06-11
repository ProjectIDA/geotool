/** \file GseSource.cpp
 *  \brief Defines class GseSource.
 *  \author Ivan Henson
 */
#include "config.h"
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <pwd.h>
#include <errno.h>

#include "GseSource.h"
#include "gobject++/GTimeSeries.h"
extern "C" {
#include "libgmath.h"
}

class GseData
{
    public:
    GseData() {
        offset = 0;
        msg_version = 0;
    }
    ~GseData() { }
    int offset;
    int msg_version;
};

typedef struct
{
    char    sta[9];
    char    chan[9];
    char    auxid[5];
    char    instype[9];
    long    nsamp;
    double  time;
    double  endtime;
    double  samprate;
    double  calib;
    double  calper;
    double  hang;
    double  vang;
    double  lat;
    double  lon;
} GSEParam;

/*
  when reading a message, this is the datatype which is returned
*/

#define END             0
#define STOP            10
#define WAVEFORM        11
#define PURIFY          12
#define CHANNEL         13
#define STATION         14
#define BULLETIN        15
#define FTP_LOG         16
#define LOG             17
#define ERROR_LOG       18
#define IRIS_MSG        19
#define NO_DATA         20
#define PARSEFINGER     21
#define CRON            22
#define AT              23
#define WAVES4U         24

#define SAMPLEPHD       30
#define BLANKPHD        31
#define DETBKPHD        32
#define CALIBPHD        33
#define QCPHD           34
#define RMSSOH          35
#define MET             36
#define ALERT           37

#define UNKNOWN         99

/*
  message and data type versions
  can also use UNKNOWN 99
*/
#define GSE_20          1
#define IMS_10          2
#define RMS_20          3
#define RMS_30          4


/*
  possible problems when parsing
*/

#define NO_PROBLEM              0
#define NO_DATA_AVAIL           1
#define WAVE_LOCK               2
#define N_NE_OBS                3
#define UNK_FORMAT              4
#define MALLOC_FAILURE          5
#define NO_DAT_LINE             6
#define NO_CHK_LINE             7
#define ERR_DECODING_WAVEFORM   8
#define DATA_DELAYED            9

static const char *parse_error_table[] =
    {
	/* 0 */ "NA",
	/* 1 */ "parse warning: no data available",
	/* 2 */ "parse failure: could not get waveform lock",
	/* 3 */ "parse failure: number of observed samps ne number expected samps",
	/* 4 */ "parse failure: data in unknown format",
	/* 5 */ "parse failure: malloc failure",
	/* 6 */ "parse failure: no DAT line",
	/* 7 */ "parse failure: no CHK line",
	/* 8 */ "parse failure: error decoding waveform",
	/* 9 */ "parse failure: data will be delayed"
    };


#ifdef HAVE_LIBZ
static int get_waveform(gzFile zfp, FILE *fp, int msg_version, int *dt_version, GSEParam *gp,
		int **dat, int *checksum, int *err);
#else
static int get_waveform(FILE *fp, int msg_version, int *dt_version, GSEParam *gp,
		int **dat, int *checksum, int *err);
#endif
static void remdif1(int *data, int npts);
static int dcomp6(char *buf, int **data);
static int dcomp7(char *buf, int **data);
static int dcomp8(char *buf, int **data);
static void computeChecksum(int *signal_int, int nsamp, int *checksum);
static void returnDataType(char *line, int msg_version, int *type,
		int *dt_version);
static int gseVersion(char *line);


GseSource::GseSource(const string &name, const string &file) : TableSource(name)
{
    read_path = file;
    openPrefix(file);
    queryAllPrefixTables();
}

GseSource::~GseSource(void) { }

gvector<SegmentInfo *> * GseSource::getSegmentList(void)
{
#ifdef HAVE_LIBZ
    gzFile  zfp = Z_NULL;
#endif
    FILE *fp = NULL;
    int path_quark, offset, checksum, err, data_type;
    int dt_version, msg_version = GSE_20;
    int *dat = NULL;
    struct stat buf;
    cvector<CssOriginClass> o;
    GSEParam gp;
    char error[MAXPATHLEN+100];
    gvector<SegmentInfo *> *segs;

    if(read_path.empty()) return NULL;

    if(read_path.length() > 3 && read_path.substr(read_path.length()-3).compare(".gz") == 0) {
#ifdef HAVE_LIBZ
	if(stat(read_path.c_str(), &buf) < 0 || (zfp = gzopen(read_path.c_str(), "rb")) == NULL) {
	    snprintf(error, MAXPATHLEN+100,"gse: cannot open %s",read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return NULL;
	}
#else
	snprintf(error, MAXPATHLEN+100,"gse: cannot open %s. Compile with libz", read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
#endif
    }
    else {
	if(stat(read_path.c_str(), &buf) < 0 || (fp = fopen(read_path.c_str(), "r")) == NULL)
	{
	    snprintf(error, MAXPATHLEN+100,"gse: cannot open %s",read_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return NULL;
	}
    }
    path_quark = (int)stringToQuark(read_path);

    for(int i = 0; i < (int)gse_data.size(); i++) delete gse_data[i];
    gse_data.clear();

#ifdef HAVE_LIBZ
    if(zfp != Z_NULL) offset = gztell(zfp);
    else offset = ftell(fp);
#else
    offset = ftell(fp);
#endif
    data_type = WAVEFORM;
    segs = new gvector<SegmentInfo *>;

    while(data_type == WAVEFORM || data_type == LOG || data_type == ERROR_LOG)
    {
	gp.time = 0.;
	gp.sta[0] = '\0';
	gp.chan[0] = '\0';
	gp.nsamp = 0;
	
#ifdef HAVE_LIBZ
	if(zfp != Z_NULL) {
	    offset = gztell(zfp);
	    data_type = get_waveform(zfp, fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
	}
	else {
	    offset = ftell(fp);
	    data_type = get_waveform(zfp, fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
	}
#else
	offset = ftell(fp);
	data_type = get_waveform(fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
#endif

	if(err != NO_PROBLEM)
	{
	    snprintf(error,MAXPATHLEN+100,"GSE2.0 format problem %s/%s: %s",
				gp.sta, gp.chan, parse_error_table[err]);
	    logErrorMsg(LOG_WARNING, error);
	}

	if(data_type == STOP || data_type == END)
	{
	    if ((int)segs->size() == 0) {
		snprintf(error, MAXPATHLEN+100, "No data found in %s",
			read_path.c_str());
		logErrorMsg(LOG_WARNING, error);
	    }
	    break;
	}

	Free(dat);

	if(gp.nsamp < 1) continue;

	if(gp.samprate <= 0.)
	{
	    snprintf(error, MAXPATHLEN+100, "%s: bad samprate = %e",
			read_path.c_str(), gp.samprate);
	    logErrorMsg(LOG_WARNING, error);
#ifdef HAVE_LIBZ
	    if(zfp != Z_NULL) gzclose(zfp);
	    else fclose(fp);
#else
	    fclose(fp);
#endif
	    return segs;
	}

	SegmentInfo *s = new SegmentInfo();

	s->id = gse_data.size() + 1;
	GseData *gd = new GseData();
	gse_data.push_back(gd);
	s->path = path_quark;
	s->format = stringToQuark("gse");
	s->file_order = (int)segs->size();
	s->nsamp = gp.nsamp;
	s->samprate = gp.samprate;
	s->start = gp.time;
	s->end = gp.time + (gp.nsamp-1)/gp.samprate;
	s->jdate = timeEpochToJDate(s->start);
	gd->offset = offset;
	stringcpy(s->sta, gp.sta, sizeof(s->sta));
	stringcpy(s->chan, gp.chan, sizeof(s->chan));
//	gnetGetWaveSites(1, s);

	s->selected = true;
	CssWfdiscClass *w = new CssWfdiscClass();
	stringcpy(w->sta, gp.sta, sizeof(w->sta));
	stringcpy(w->chan, gp.chan, sizeof(w->chan));
	w->nsamp = s->nsamp;
	w->samprate = s->samprate;
	w->time = s->start;
	w->endtime = s->end;
	s->setWfdisc(w);
	segs->push_back(s);
    }
#ifdef HAVE_LIBZ
    if(zfp != Z_NULL) gzclose(zfp);
    else fclose(fp);
#else
    fclose(fp);
#endif

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

bool GseSource::makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
			int pts, GTimeSeries **ts, const char **err_msg)
{
    const char *full_path;
    string dir, prefix;
//    int n = 0;
    GseData *gd;

    *err_msg = NULL;

    if(s->id < 1 || s->id > (int)gse_data.size()) return false;
    gd = gse_data[s->id-1];

    full_path = quarkToString(s->path);
    getDir(full_path, dir, prefix);

    if(*ts == NULL)
    {
	*ts = new GTimeSeries();
	(*ts)->waveform_io = new WaveformIO();
	(*ts)->putValue("dc", s->path);
	(*ts)->putValue("id", s->id);
	(*ts)->source_info.setSource(full_path);
	(*ts)->source_info.format = stringToQuark("gse");
	(*ts)->source_info.data_source = stringToQuark("gse");
	for(int i = 0; i < s->array_elements.size(); i++) {
	    (*ts)->array_elements.add(s->array_elements[i]);
	}
    }

    GSegment *segment = readSegment(full_path, gd->offset, tbeg, tend, pts);
    if(!segment) return false;

    (*ts)->addSegment(segment);

/*
    if( (n = invalidData(segment->length(), segment->data, (float)0.)) )
    {
	char error[MAXPATHLEN+100];
	snprintf(error, MAXPATHLEN+100,
		"cssIO: setting %d invalid binary data from %s to 0.",
                        n, full_path);
	logErrorMsg(LOG_WARNING, error);
    }
*/

    WfdiscPeriod wp;
    wp.tbeg = segment->tbeg();
    wp.tend = segment->tend();
    wp.dir = stringToQuark(dir);
    wp.prefix = stringToQuark(prefix);
    wp.wfdisc_file = stringToQuark(full_path);
    wp.wfdisc_index = s->file_order;
    wp.pts_needed = pts;
    wp.wf = *s->wfdisc();
    getChanid(wp.wf.sta, wp.wf.chan, &wp.wf.chanid);

    if((*ts)->waveform_io) {
	(*ts)->waveform_io->wp.push_back(wp);
    }

    return true;
}

GSegment * GseSource::readSegment(const string &full_path, int offset,
			double start_time, double end_time, int pts_wanted)
{
    char error[MAXPATHLEN+100];
    int *dat = NULL;
    int npts, start, checksum, err, dt_version, msg_version = GSE_20;
    GSEParam gp;
    GSegment *seg=NULL;
    struct stat buf;
    double tbeg, tdel, calib;
    float *data=NULL;
#ifdef HAVE_LIBZ
    gzFile  zfp = Z_NULL;
    FILE *fp = NULL;
#else
    FILE *fp = NULL;
#endif
    if(full_path.length() > 3 && full_path.substr(full_path.length()-3).compare(".gz") == 0) {
#ifdef HAVE_LIBZ
    	if(stat(full_path.c_str(),&buf) < 0 || (zfp = gzopen(full_path.c_str(), "rb")) == NULL)
	{
	    snprintf(error, MAXPATHLEN+100,"gse: cannot open %s",full_path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return NULL;
	}
#else
	snprintf(error, MAXPATHLEN+100,"gse: cannot open %s. Compile with libz", full_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
#endif
    }
    else if(stat(full_path.c_str(),&buf) < 0 || (fp = fopen(full_path.c_str(), "r")) == NULL)
    {
	snprintf(error, MAXPATHLEN+100,"gse: cannot open %s",full_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }
#ifdef HAVE_LIBZ
    if((zfp != Z_NULL && gzseek(zfp, offset, 0) == -1) || (fp != NULL && fseek(fp, offset, 0)))
#else
    if(fseek(fp, offset, 0))
#endif
    {
	snprintf(error, MAXPATHLEN+100, "%s: read error", full_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }
    gp.time = 0.;
    gp.endtime = 0.;
    gp.nsamp = 0;
    gp.samprate = 0.;

#ifdef HAVE_LIBZ
    if(zfp != Z_NULL) {
	get_waveform(zfp, fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
	gzclose(zfp);
    }
    else {
	get_waveform(zfp, fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
    	fclose(fp);
    }
#else
    get_waveform(fp, msg_version, &dt_version, &gp, &dat, &checksum, &err);
    fclose(fp);
#endif
    if(err != NO_PROBLEM)  {
      snprintf(error, MAXPATHLEN+100, "gse: in %s, failed to get waveform: %s",
	       full_path.c_str(), parse_error_table[err]);
      logErrorMsg(LOG_WARNING, error);
      fprintf(stderr, "%s\n", error); 
      return NULL;
    }
    if (dat == NULL) {  /* just to make sure dat is properly allocated */
      snprintf(error, MAXPATHLEN+100, "gse: in %s, failed to get waveform: parse error",
	       full_path.c_str());
      logErrorMsg(LOG_WARNING, error);
      return NULL;
    }

    if(gp.samprate <= 0.) {
	snprintf(error, MAXPATHLEN+100, "%s: bad samprate = %e",
		full_path.c_str(), gp.samprate);
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }

    start = 0;
    if(start_time > gp.time) {
	start = (int)((start_time - gp.time)*gp.samprate+.5);
    }

    if(end_time > gp.endtime) {
	npts = gp.nsamp - start;
    }
    else {
	npts = (int)(((end_time - gp.time)*gp.samprate+.5) - start + 1);
    }
    if(start + npts > gp.nsamp) npts = gp.nsamp - start;

    if(npts <= 0) return NULL;

    data = (float *)malloc(npts*sizeof(float));
    if (data == NULL) {
      snprintf(error, MAXPATHLEN+100,"gse: cannot allocate memory for data buffer, needed %lu bytes", npts*sizeof(float));
      logErrorMsg(LOG_WARNING, error);
      return NULL;
    }
    for(int i = 0; i < npts; i++) {
      data[i] = (float)(dat[start+i]);
    }

    Free(dat);

    calib = (gp.calib != 0.) ? gp.calib : 1.;
    tdel = 1./gp.samprate;
    tbeg = gp.time + start*tdel;
    seg = new GSegment(data, npts, tbeg, tdel, calib, gp.calper);

    Free(data);

    return seg;
}

bool GseSource::reread(GTimeSeries *ts)
{
    char error[MAXPATHLEN+100];
    const char *path;
#ifdef HAVE_LIBZ
    gzFile zfp;
    FILE *fp;
#else
    FILE *fp;
#endif
    GSEParam gp;
    float *data;
    int	i, j, offset, checksum, err, dt_version;
    int *dat = NULL;
    double d, tdel, tbeg, tend, calib;
    GseData *gd;

    if( !ts->waveform_io ) {
	logErrorMsg(LOG_WARNING, "gse.reread: cannot re-read data");
	return false;
    }
    if((tbeg = ts->originalStart()) == NULL_TIME) tbeg = ts->tbeg();
    if((tend = ts->originalEnd()) == NULL_TIME) tend = ts->tend();

    ts->removeAllSegments();

    data = (float *)malloc(sizeof(float));
    path = read_path.c_str();

    for(i = 0; i < (int)ts->waveform_io->wp.size(); i++)
    {
	j = ts->waveform_io->wp[i].wfdisc_index;
	if(j < 0 || j >= (int)gse_data.size()) continue;
	gd = gse_data[j];
	fp = NULL;
#ifdef HAVE_LIBZ
	zfp = Z_NULL;
#endif
	if(strlen(path) > 3 && !strcmp(path+strlen(path)-3, ".gz")) {
#ifdef HAVE_LIBZ
	    if((zfp = gzopen(path, "rb")) == NULL)
	    {
		snprintf(error, MAXPATHLEN+100, "gse: cannot open %s", path);
	    	logErrorMsg(LOG_WARNING, error);
		return false;
	    }
#else
	    snprintf(error, MAXPATHLEN+100,"gse: cannot open %s. Compile with libz", path.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return false;
#endif
	}
	else if((fp = fopen(path, "r")) == NULL)
	{
	    snprintf(error, MAXPATHLEN+100, "gse: cannot open %s", path);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	offset = gd->offset;
#ifdef HAVE_LIBZ
	if((zfp != Z_NULL && gzseek(zfp, offset, 0) == -1) || (fp != NULL && fseek(fp, offset, 0))) {
#else
	if(fseek(fp, offset, 0)) {
#endif
	    snprintf(error, MAXPATHLEN+100, "%s: read error", path);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

#ifdef HAVE_LIBZ
	/* gzseek(zfp, offset, 0); */
	if(zfp != Z_NULL) {
	    get_waveform(zfp, fp, gd->msg_version, &dt_version, &gp, &dat, &checksum, &err);
	    gzclose(zfp);
	}
	else {
	  get_waveform((gzFile)fp, fp, gd->msg_version, &dt_version, &gp, &dat, &checksum, &err);
	    fclose(fp);
	}
#else
	/* redundant ?  fseek(fp, offset, 0); */
	get_waveform(fp, gd->msg_version, &dt_version, &gp, &dat, &checksum, &err);
	fclose(fp);
#endif

	data = (float *)realloc(data, gp.nsamp*sizeof(float));

	for(j = 0; j < gp.nsamp; j++) data[j] = (float)dat[j];
	Free(dat);

	tdel = (gp.samprate != 0.) ? 1./gp.samprate : 1.;
	calib = (gp.calib != 0.) ? gp.calib : 1.;
	GSegment *segment = new GSegment(data, gp.nsamp, gp.time, tdel, calib,
				gp.calper);
	ts->addSegment(segment);
    }
    Free(data);

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

static void cleanName(char* in, char* out, int len);

#define WID1	1
#define WID2	2

/* 
   returns a value which describes the next datatype

	err - returns possible problem parsing this waveform, or NO_PROBLEM
*/

int
#ifdef HAVE_LIBZ
static get_waveform(gzFile zfp, FILE *fp, int msg_version, int *dt_version, GSEParam *gp,
		int **dat, int *checksum, int *err)
#else /* HAVE_LIBZ */
static get_waveform(FILE *fp, int msg_version, int *dt_version, GSEParam *gp,
		int **dat, int *checksum, int *err)
#endif
{
    char	tmp[1024], datatype[5]; /* 5 is to support WID1 */
    char	line[257];
    char	*buf = NULL, *endptr, *p;
    long	n;
    double	s, cb, cp, h, v, epoch, msec;
    bool	found_dat;
    int		*d = NULL;
    int		i, doy, type, found, ndiff, buf_len;
    int		len, error, obs_nsamp, obs_checksum, rec_checksum, read_to_buf;
    DateTime	dt;

    gp->nsamp = -1;
    gp->time = -1.;
    gp->endtime = -1.;
    stringcpy(gp->sta, "", sizeof(gp->sta));
    stringcpy(gp->chan, "", sizeof(gp->chan));
    ndiff = 0;

    *err = NO_PROBLEM;

    found = false;
#ifdef HAVE_LIBZ
    while( (fp != NULL && fgets(line, 256, fp) != NULL) || (zfp != Z_NULL && gzgets(zfp, line, 256) != NULL))
#else /* HAVE_LIBZ */
    while ( fgets(line, 256, fp) != NULL)
#endif /* HAVE_LIBZ */
    {
	if (!strncasecmp(line, "WID2", 4))
	{
	    found = WID2;
	    break;
	}
	else if (!strncasecmp(line, "WID1", 4))
	{
	    found = WID1;
	    break;
	}
	else if (!strncasecmp(line, "STOP", 4))
	{
	    *err = NO_PROBLEM;
	    return(STOP);
	}
	else if (!strncasecmp(line, "DATA_TYPE", 9))
	{
	    *err = NO_PROBLEM;

	    stringToUpper(line);

	    returnDataType(line, msg_version, &type, dt_version);
	    return(type);
	}
    }
    if (!found) return(END);

    strncpy(tmp, "", 2);

    /* this is here since some WID1 lines extend over 2 lines */
    found_dat = false;

    if (found == WID2)
    {
	timeParseLine(line,5,27,&epoch, sizeof(epoch), "yyyymmdd_hhmmssssss");

	timeParseLine(line,  29, 33, tmp, sizeof(tmp), "s");
	cleanName(tmp, gp->sta, sizeof(gp->sta));
	stringToUpper(gp->sta);

	timeParseLine(line,  35, 37, tmp, sizeof(tmp), "s");
	cleanName(tmp, gp->chan, sizeof(gp->chan));
#ifdef MIDDLE
	/* get rid of middle code */
	if (gp->chan[1] == 'h' && (int)strlen(gp->chan) == 3)
	{
	    gp->chan[1] = gp->chan[2];
		gp->chan[2] = '\0';
	}
#endif

	timeParseLine(line,  39, 42, gp->auxid, sizeof(gp->auxid), "s");
	timeParseLine(line,  44, 46, datatype, sizeof(datatype), "s");

	ndiff = !strncasecmp(datatype, "CM6", 3) ? 2 : 0;

	timeParseLine(line,  48,  55, &n, sizeof(n), "ld");
	timeParseLine(line,  57,  67, &s, sizeof(s), "lf");
	timeParseLine(line,  69,  78, &cb, sizeof(cb), "lf");
	timeParseLine(line,  80,  86, &cp, sizeof(cp), "lf");

	len = (int) strlen(line);

	if (len > 88) {
	    timeParseLine(line,88,93, gp->instype, sizeof(gp->instype),"s");
	}
	else {
	    stringcpy(gp->instype, "-", sizeof(gp->instype));
	}

	if (len > 95) {
	    timeParseLine(line, 95, 99, &h, sizeof(h), "lf");
	}
	else {
	    h = -1.0;
	}
	
	if (len > 101) {
	    timeParseLine(line, 101, 104, &v, sizeof(v), "lf");
	}
	else {
	    v = -1.0;
	}
    }
    else if (found == WID1)
    {
	timeParseLine(line,   5,  9, &dt.year, sizeof(dt.year), "d");
	timeParseLine(line,  10, 12, &doy, sizeof(doy), "d");

	timeParseLine(line,  14, 15, &dt.hour, sizeof(dt.hour), "d");
	timeParseLine(line,  17, 18, &dt.minute, sizeof(dt.minute), "d");
	timeParseLine(line,  20, 21, &dt.second, sizeof(dt.second), "lf");
	timeParseLine(line,  23, 25, &msec, sizeof(msec), "lf");

	dt.second += (msec/1000.);
	epoch = timeDateToEpoch(&dt);

	timeParseLine(line,  27, 34, &n, sizeof(n), "ld");
	timeParseLine(line,  36, 41, tmp, sizeof(tmp), "s");
	cleanName(tmp, gp->sta, sizeof(gp->sta));
	stringToUpper(gp->sta);

	timeParseLine(line,  52, 53, tmp, sizeof(tmp), "s");
	cleanName(tmp, gp->chan, sizeof(gp->chan));

	timeParseLine(line,  55, 65, &s, sizeof(s), "lf");

	timeParseLine(line,  74, 77, datatype, sizeof(datatype), "s");

	ndiff = 0;
	timeParseLine(line,  79, 80, &ndiff, sizeof(ndiff), "d");

	if (strlen(datatype) > 3 && !strncmp(datatype, "INT", 3))
	{
	    stringcpy(datatype, "INT", sizeof(datatype));
	}

	/* in case WID1 line extends over 2 lines */
	if ((int)strlen(line) < 88)
	{
#ifdef HAVE_LIBZ
	    if(zfp != Z_NULL) gzgets(zfp, line, 256);
	    else fgets(line, 256, fp);
#else /* HAVE_LIBZ */
	    fgets(line, 256, fp);
#endif /* HAVE_LIBZ */
	    if (!strncasecmp(line, "DAT2", 4) || !strncasecmp(line, "DAT1", 4))
	    {
		found_dat = true;
	    }
	    else
	    {
		timeParseLine(line,   0,  8, &cb, sizeof(cb), "lf");
		timeParseLine(line,  10, 16, &cp, sizeof(cp), "lf");
	    }
	}
	else
	{
	    timeParseLine(line,  80, 88, &cb, sizeof(cb), "lf");
	    timeParseLine(line,  90, 96, &cp, sizeof(cp), "lf");
	}

	h = -1.0;
	v = -1.0;
    }
	
    gp->time = epoch;
    gp->endtime = gp->time + (double)(n/s);
	
    gp->nsamp = n;
    gp->samprate = s;
    gp->calper = cp;
    gp->calib = cb;
    gp->hang = h;
    gp->vang = v;

#ifdef HAVE_LIBZ
    while (!found_dat && ((fp != NULL && fgets(line, 256, fp) != NULL) ||
		(zfp != Z_NULL && gzgets(zfp, line, 256) != NULL)) )
#else /* HAVE_LIBZ */
    while (!found_dat && fgets(line, 256, fp) != NULL)
#endif /* HAVE_LIBZ */
    {
	if (!strncasecmp(line, "DAT2", 4) || !strncasecmp(line, "DAT1", 4))
	{
	    found_dat = true;
	    break;
	}
    }
    if (!found_dat) 
    {
	*err = NO_DAT_LINE;
	return(END);
    }

    read_to_buf = 1;
    if (!strncasecmp(datatype, "INT", 3)) read_to_buf = 0;

/* read the waveform */
    buf = (char *)malloc((n+1)*sizeof(int));
    if(!buf)
    {
	*err = MALLOC_FAILURE;
	return(WAVEFORM);
    }

    d = (int *)malloc(n*sizeof(int));
    if(!d)
    {
	*err = MALLOC_FAILURE;
	return(WAVEFORM);
    }

    memset(buf, 0, n*sizeof( int ));
    buf_len = n;
	
    i=0;
    obs_nsamp=0;
    found = false;

    if(read_to_buf) {
#ifdef HAVE_LIBZ
	while ((fp != NULL && fgets(line, 256, fp) != NULL) || (zfp != Z_NULL && gzgets(zfp, line, 256) != NULL))
#else
	while ( fgets(line, 256, fp) != NULL)
#endif
	{
	    if (!strncasecmp(line, "CHK2 ", 5) ||
		!strncasecmp(line, "CHK1 ", 5) ||
		!strncasecmp(line, "CHK2\t", 5) ||
		!strncasecmp(line, "CHK1\t", 5))
	    {
		if (sscanf(line, "%s %d", tmp, &rec_checksum) != 2) {
		    rec_checksum = -1;
		}
		found = true;
		break;
	    }
	    len = (int) strlen(line);
	    len -= 1;
	    if ((i+len) > buf_len)
	    {
		buf = (char*)realloc(buf, (buf_len + 1024) * sizeof(int));
		memset(buf+i, 0, 1024*sizeof( int ));
		buf_len += 1024;
	    }
	    strncpy(buf+i, line, len);
	    i += len;
	}
    }
    else {
#ifdef HAVE_LIBZ
	while ((fp != NULL && fgets(line, 256, fp) != NULL) || (zfp != Z_NULL && gzgets(zfp, line, 256) != NULL))
#else
	while ( fgets(line, 256, fp) != NULL)
#endif
	{
	    errno = 0;
	    for(p = line; errno == 0 && *p != '\0' && obs_nsamp < n; p = endptr) {
		errno = 0;
		d[obs_nsamp] = strtol(p, &endptr, 10);
		if(p == endptr || errno != 0) break;
		obs_nsamp++;
	    }
	    if(*endptr != '\n') break;
	}
	while(1) { // skip blank lines before CHK line
	    int j = 0;
	    while(line[j] != '\0' && isspace(line[j])) j++;
	    if(line[j] != '\0') break;
	    if( fgets(line, 256, fp) == NULL) break;
	}
	if (!strncasecmp(line, "CHK2 ", 5) ||
	    !strncasecmp(line, "CHK1 ", 5) ||
	    !strncasecmp(line, "CHK2\t", 5) ||
	    !strncasecmp(line, "CHK1\t", 5))
	{
	    if (sscanf(line, "%s %d", tmp, &rec_checksum) != 2) {
		rec_checksum = -1;
	    }
	    found = true;
	}
    }
    if (!found) 
    {
	Free(buf);
	*err = NO_CHK_LINE;
	return(END);
    }

    if (read_to_buf) buf[i] = '\0';

/* convert waveform */

    error = 0;
    if (!strncasecmp(datatype, "CM6", 3) || !strncasecmp(datatype, "CMP6", 4))
    {
	obs_nsamp = dcomp6(buf, &d);
    }
    else if (!strncasecmp(datatype, "CM7", 3) || !strncasecmp(datatype, "CMP7", 4))
    {
	obs_nsamp = dcomp7(buf, &d);
    }
    else if (!strncasecmp(datatype, "CM8", 3) || !strncasecmp(datatype, "CMP8", 4))
    {
	obs_nsamp = dcomp8(buf, &d);
    }
    else if (!strncasecmp(datatype, "INT", 3))
    {
    }
    else
    {
	/*
	 sprintf(msg, "unknown datatype '%s'\n only CM6 and INT are supported",
		datatype);
	 logging(0, "parsedrm", msg);
	*/
	Free(buf);
	*err = UNK_FORMAT;
        return(WAVEFORM);
    }

    *dat = d;

    if (ndiff > 0 && obs_nsamp > 0)
    {
	for (i = 0; i < ndiff; i++) remdif1(d, obs_nsamp);

	if (d[0] > 16000000) d[0] = d[1];
    }

    if (n != obs_nsamp )
    {
	char msg[200];
	/*
	sprintf(msg, "expected n=%d, obs_nsamp=%d", n, obs_nsamp);
	logging(0, "parsedrm", msg);
	*/
	snprintf(msg, 200, "expected n=%ld, obs_nsamp=%d", n, obs_nsamp);
	logErrorMsg(LOG_WARNING, msg);

	gp->nsamp = (long) obs_nsamp;
	Free(buf);
	*err = N_NE_OBS;
	return(WAVEFORM);
    }
    else if ( error == -1)
    {
	/*
	sprintf(msg, "error decoding waveform data");
	logging(0, "parsedrm", msg);
	*/
	logErrorMsg(LOG_WARNING, "error decoding waveform data");
	Free(buf);
	*err = ERR_DECODING_WAVEFORM;
	return(WAVEFORM);
    }

    computeChecksum(d, obs_nsamp, &obs_checksum); 
/*
    if (rec_checksum == obs_checksum)
    {
	logging(0, "parsedrm", "checksum passed");
    }
    else
    {
	sprintf(msg, "checksum  FAILED, observed = %d, recorded = %d\n",
			obs_checksum, rec_checksum);
	logging(0, "parsedrm", msg);
    }
*/
    *checksum = (rec_checksum == obs_checksum) ? 1 : 0;

    Free(buf);
    return(WAVEFORM);
}

static void
cleanName(char *in, char *out, int len)
{
    if(!len) return;

    if(*in == '\0') { *out = '\0'; return; }

    while ( *in && isspace((int)*in) ) in++;

    while ( *in && !isspace((int)*in) && *in != ','
		&& *in != '(' && *in != ')' && --len > 0) *out++ = *in++;

    *out = '\0';
}

/**
 *   The following data compression and decompression routines have
 *   been developed by Dr. Shane Ingate and Dr. Ken Muirhead, at the
 *   Australian Seismological Centre, Bureau of Mineral Resources,
 *   Canberra, Australia.
 *   <p>
 *   They provided me with these routines in March 1989 during the
 *   session of the Group of Scientific Experts (GSE) in Geneva.
 *   These compression/decompression algorithms are used by all members
 *   of the GSE during the large-scale data exchange experiment GSETT-2,
 *   carried out from 1989 to 1991.
 *   <p>
 *   It is recommended to use second differences and the six-bit compression.
 *   The second differences can be computed by two subsequent calls of
 *   subroutine DIF1 (see above).
 *   <p>
 *   These routines are already running on many different machines.
 *   Because the AND function is not standard FORTRAN 77, this operation
 *   has been moved to a separate subroutine INTAND. All users of these
 *   routines will have to modify INTAND so that it performs correctly on
 *   their computers.
 *   <p>
 *                                Urs Kradolfer, Swiss Seismological Service
 */

/* Translated from Fortran by I. Henson, Dec. 1995
 */

/** 
 * Remove Data-compression (first differences).
 */
static void
remdif1(int *data, int npts)
{
	/* Remove Data-compression (first differences)
	 *
	 * Urs Kradolfer, January 1990
	 */
	int	i;

	for(i = 1; i < npts; i++) data[i] += data[i-1];
}
	
/** 
 *  Decompress 6-bit integer data. Decompress integer data that has been
 *  compressed into ascii characters. Returns values in int format.
 *  See subroutine cmprs6 for compression format.
 *  @param buf null terminated string.
 *  @return the length of *data
 */
static int
dcomp6(char *buf, int **data)
{
    static int ch[128] = 
    {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 
	 3, 4, 5, 6, 7, 8, 9,10,11, 0, 0, 0, 0, 0, 0, 0, 
	12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
	28,29,30,31,32,33,34,35,36,37, 0, 0, 0, 0, 0, 0,
	38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
	54,55,56,57,58,59,60,61,62,63, 0, 0, 0, 0, 0, 0
    };
    int	i, j, k, ibyte, joflow, jsign, itemp, size_data = 0;
    int	isign = 16, ioflow = 32, mask1 = 15, mask2 = 31;
    union
    {
	char	a[4];
	int	i;
    } ai;

    ai.a[0] = '1';
    ai.a[1] = '2';
    ai.a[2] = '3';
    ai.a[3] = '4';

    *data = NULL;

    /* work out which way bytes are stored in computer
     */
    ibyte = (ai.i == (((52*256+51)*256+50)*256+49)) ? 0 : 3;

    /* start of decoding
     */
    for(i = j = 0; buf[i] != '\0'; i++)
    {
	while(buf[i] != '\0' && (buf[i] == '\n' || buf[i] == '\r')) i++;

	if(buf[i] == '\0' || buf[i] == ' ') return(j);

	ai.i = 0;
	ai.a[ibyte] = buf[i];

	/* strip off any higher order bits */
	k = ai.i & 127;

	/* get number representation of input character */
	ai.i = ch[k-1];

	/* get sign bit */
	jsign = ai.i & isign;

	/* get continuation bit (if any) */
	joflow = ai.i & ioflow;

	/* remove bits we don't want */
	itemp = ai.i & mask1;

	while(joflow != 0)
	{
	    /* there is another byte in this sample */
	    itemp *= 32;
	    i++;
	    while(buf[i] != '\0' && (buf[i] == '\n' || buf[i] == '\r')) i++;

	    if(buf[i] == '\0' || buf[i] == ' ') return(j);

	    ai.a[ibyte] = buf[i];
	    /* strip off any higher order bits */
	    k = ai.i & 127;
	    ai.i = ch[k-1];
			
	    /* get continuation bit (if any) */
	    joflow = ai.i & ioflow;
	    k = ai.i & mask2;
	    itemp += k;
	}

	if(jsign != 0) itemp = -itemp;

	if(j+1 > size_data) {
	    size_data += 1024;
	    if((*data = (int *)realloc(*data, size_data*sizeof(int))) == NULL) {
		fprintf(stderr, "dcomp6: malloc error.\n");
		return(-1);
	    }
	}
	(*data)[j++] = itemp;
    }
    return(j);
}

/** 
 *  Decompress 7-bit integer data. Decompress integer data that has been
 *  compressed into 7 bit characters. Returns values in int format.
 *  See subroutine cmprs7 for compression format.
 *  @param buf null terminated string.
 *  @return the length of *data
 */
static int
dcomp7(char *buf, int **data)
{
    int	isign = 32, ioflow = 64, mask1 = 31, mask2 = 63;
    int	icorr = 32, mask3 = 127, ncntrl = 64, size_data = 0;
    int	i, j, k, iend, itemp, joflow, jsign, correction, ibyte;
    union
    {
	char	a[4];
	int	i;
    } ai;

    ai.a[0] = '1';
    ai.a[1] = '2';
    ai.a[2] = '3';
    ai.a[3] = '4';

    *data = NULL;

    /* work out which way bytes are stored in computer
     */
    ibyte = (ai.i == (((52*256+51)*256+50)*256+49)) ? 0 : 3;

    /* start of decoding
     */
    for(i = -1, j = 0 ; buf[i+1] != '\0'; )
    {
	iend = 0;
	i++;
	ai.a[ibyte] = buf[i];

	/* remove most significant bit - not used */
	ai.i = ai.i & mask3;

	/* test if character is equal to ncntrl */
	if(ai.i == ncntrl) iend = 1;

	/* get sign bit */
	jsign = ai.i & isign;

	/* get continuation bit */
	joflow = ai.i & ioflow;

	/* remove sign and continuation bits */
	itemp = ai.i & mask1;

	correction = 0;
	while(joflow != 0.)
	{
	    /* there is another byte in this sample */
	    i++;
	    ai.a[ibyte] = buf[i];

	    /* remove most significant bit - not used */
	    ai.i = ai.i & mask3;

	    /* test if character is equal to ncntrl
	     * bit pattern 1000000 followed by
	     * 1000000 means end of data.
	     */
	    if(iend == 1 && ai.i == ncntrl) return(j);

	    if(iend == 1 && ai.i < 16)
	    {
		/* See if previous sample has to be modified.
		 */
		itemp = icorr;
		/* subtract correction from previous sample */
		if((*data)[j-1] < 0) itemp = -itemp;
		(*data)[j-1] -= itemp;
		correction = 1;
		break;
	    }
	    else
	    {
		iend = 0;

		/* get continuation bit */
		joflow = ai.i & ioflow;
		k = ai.i & mask2;

		/* shift what we have 6 bits left and add in new bits */
		itemp = itemp*64 + k;
	    }
	}
	if(correction) continue;

	if(jsign != 0) itemp = -itemp;

	if(j+1 > size_data) {
	    size_data += 1024;
	    if((*data = (int *)realloc(*data, size_data*sizeof(int))) == NULL) {
		fprintf(stderr, "dcomp7: malloc error.\n");
		return(-1);
	    }
	}
	(*data)[j++] = itemp;
    }
    return(j);
}
		
/** 
 *  Decompress 8-bit integer data. Decompress integer data that has been
 *  compressed into 8 bit characters. Returns values in int format.
 *  See subroutine cmprs8 for compression format.
 *  @param buf null terminated string.
 *  @return the length of *data
 */
static int
dcomp8(char *buf, int **data)
{
    int	isign = 64, ioflow = 128, mask1 = 63, mask2 = 127;
    int	ncntrl = 128, icorr = 32, size_data = 0;
    int	i, j, k, iend, itemp, jsign, joflow, ibyte, correction;
    union
    {
	char	a[4];
	int	i;
    } ai;

    ai.a[0] = '1';
    ai.a[1] = '2';
    ai.a[2] = '3';
    ai.a[3] = '4';

    *data = NULL;

    /* work out which way bytes are stored in computer
     */
    ibyte = (ai.i == (((52*256+51)*256+50)*256+49)) ? 0 : 3;


    for(i = -1, j = 0; buf[i+1] != '\0'; )
    {
	iend = 0;
	i++;
	ai.a[ibyte] = buf[i];

	/* test if character is equal to ncntrl (100000000) */
	if(ai.i == ncntrl) iend = 1;

	/* get sign bit */
	jsign = ai.i & isign;

	/* get continuation bit */
	joflow = ai.i & ioflow;

	/* remove sign and continuation bits */
	itemp = ai.i & mask1;

	correction = 0;
	while(joflow != 0)
	{
	    /* there is another byte in this sample */
	    i++;
	    ai.a[ibyte] = buf[i];

	    /* test if character is equal to ncntrl
	     * if two control characters in a row then end of data.
	     */
	    if(iend == 1 && ai.i == ncntrl) return(j);

	    /* see if previous sample has to be modified
	     */
	    if(iend == 1 && ai.i < 16)
	    {
		itemp = icorr;
		/* now subtract correction from previous sample */
		if((*data)[j-1] < 0) itemp = -itemp;
		(*data)[j-1] -= itemp;
		correction = 1;
		break;
	    }
	    else
	    {
		iend = 0;

		/* get continuation bit */
		joflow = ai.i & ioflow;

		/* mask off continuation bit */
		k = ai.i & mask2;

		/* shift what we have so far 7 bits left and
		 * add in next bits
		 */
		itemp = itemp*128 + k;
	    }
	}
	if(correction) continue;

	if(jsign != 0) itemp = -itemp;

	if(j+1 > size_data) {
	    size_data += 1024;
	    if((*data = (int *)realloc(*data, size_data*sizeof(int))) == NULL) {
		fprintf(stderr, "dcomp8: malloc error.\n");
		return(-1);
	    }
	}
	(*data)[j++] = itemp;
    }
    return(j);
}

// Compute the GSE2.0 checksum used in the CHK2 line
static void
computeChecksum(int *signal_int, int nsamp, int *checksum)
{
    int sample_value, modulo, sum;
    int MODULO_VALUE = 100000000;

    sum = 0;

    modulo = MODULO_VALUE;

    for(int i = 0; i < nsamp; i++)
    {
	// check on sample value overflow

	sample_value = signal_int[i];

	if(abs(sample_value) >= modulo) {
	    sample_value = sample_value - (sample_value/modulo)*modulo;
	}

	// add the sample value to the checksum 

	sum += sample_value;

	// apply modulo division to the checksum

	if(abs(sum) >= modulo) {
	    sum = sum - (sum/modulo)*modulo;
	}
    }

    // compute absolute value of the checksum

    *checksum = abs(sum);
}

static void
returnDataType(char * line, int msg_version, int * type, int * dt_version)
{
    *type = UNKNOWN;

    if(strstr(line, "WAVEFORM") || strstr(line, "waveform") ||
	strstr(line, "Waveform"))
    {
	*type = WAVEFORM;
    }
    else if(strstr(line, "ERROR_LOG") || strstr(line, "error_log"))
    {
	*type = ERROR_LOG;
    }
    else if(strstr(line, "LOG") || strstr(line, "log"))
    {
	*type = LOG;
    }
    else if(strstr(line, "CHANNEL")  || strstr(line, "channel")  ||
		strstr(line, "Channel"))
    {
	*type = CHANNEL;
    }
    else if(strstr(line, "BULLETIN")  || strstr(line, "bulletin")  ||
		strstr(line, "Bulletin"))
    {
	*type = BULLETIN;
    }
    else if(strstr(line, "FTP_LOG")  || strstr(line, "ftp_log")  ||
		strstr(line, "Ftp_log"))
    {
	*type = FTP_LOG;
    }
    else if(strstr(line, "STATION")  || strstr(line, "Station")  ||
		strstr(line, "station"))
    {
	*type = STATION;
    }

    *dt_version = gseVersion(line);

    if(*dt_version == UNKNOWN)
    {
	*dt_version = msg_version;
    }
}

static int
gseVersion(char *line)
{
    if(strstr(line, "GSE2.0") || strstr(line, "Gse2.0") ||
	strstr(line, "gse2.0"))
    {
	return GSE_20;
    }
    else if(strstr(line, "IMS1.0") || strstr(line, "Gse2.0") ||
	    strstr(line, "gse2.0"))
    {
	return IMS_10;
    }
    else if(strstr(line, "RMS2.0") || strstr(line, "Gse2.0") ||
	    strstr(line, "gse2.0"))
    {
	return RMS_20;
    }
    else if(strstr(line, "RMS3.0") || strstr(line, "Gse2.0") ||
	    strstr(line, "gse2.0"))
    {
	return RMS_30;
    }
    return UNKNOWN;
}
