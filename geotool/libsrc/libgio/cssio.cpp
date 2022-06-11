/*
 * NAME
 *      routines for reading data from a database wfdisc.
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdarg.h>

#include "cssio.h"
#include "logErrorMsg.h"

static void getDataFilename(const char *dir, const char *dfile,
			const char *working_dir, char *path);
static void getCDName(const char *working_dir, const char *dir,
			const char *dfile, char *newpath);
static void unlinkTmpFile(const char *name);

#define MAX_MSG 51200
static char err_msg[MAX_MSG];

/**
 * Read waveform data. Use the input wfdisc to read and uncompress waveform
 * data.  Values between <b>start_time</b> and <b>end_time</b> are returned.
 * If <b>pts_wanted</b> is > 0, then the data will be decimated if necessary
 * to insure that no more than <b>pts_wanted</b> data values are returned.
 * Set <b>pts_wanted</b> to 0 to return all data values.
 * @param wfdisc The input wfdisc structure.
 * @param working_dir The working directory. The wfdisc dir is relative to this.
 * @param start_time The lower limit of the data times.
 * @param end_time The upper limit of the data times.
 * @param pts_wanted The maximum number of data values wanted. Set to 0 to \
 *		return all values.
 * @param npts The actual number of values returned (output).
 * @param tbeg The actual start time of the data values returned (output).
 * @param tdel The time increment of the data values returned (output).
 * @param pdata  The data values returned. (The array is allocated.)
 */
int
cssioReadData(CssWfdiscClass *wfdisc, const string &working_dir, double start_time,
	    double end_time, int pts_wanted, int *npts, double *tbeg,
	    double *tdel, float **pdata)
{
	int	start, err_no, need;
	int	fd, num_per_interval = 0;
	char	newpath[MAXPATHLEN+1], *type, path[MAXPATHLEN+1];
	float	*data = NULL;
	double	deci_dt = 0.;
#ifdef HAVE_LIBZ
        gzFile  zfd = Z_NULL;
#endif

	*npts = 0;
	*pdata = NULL;
	err_msg[0] = '\0';

	getDataFilename(wfdisc->dir, wfdisc->dfile, working_dir.c_str(), path);

	type = wfdisc->datatype;
	if( strcmp(type, "s4") && strcmp(type, "i4") && strcmp(type, "s3") && strcmp(type, "i3")
	    && strcmp(type, "s2") && strcmp(type, "i2") && strcmp(type, "t4") && strcmp(type, "f4")
	    && strcmp(type, "v4") && strcmp(type, "g2") && strcmp(type, "c2") && strcmp(type, "c4")
	    && strcmp(type, "sd") && strcmp(type, "e1") && strcmp(type, "ca"))
	{	
	    snprintf(err_msg,MAX_MSG,"Unknown datatype = %s",wfdisc->datatype);
	    return -1;
	}

	if((fd = open(path, O_RDONLY)) == -1)
	{
	    err_no = errno;
	    getCDName(working_dir.c_str(), wfdisc->dir, wfdisc->dfile, newpath);
	    if((fd = open(newpath, O_RDONLY)) == -1)
	    {
		if(err_no >= 0) {
		    snprintf(err_msg, MAX_MSG, "Cannot open: %s\n%s",
				path, strerror(err_no));
		}
		else {
		    snprintf(err_msg, MAX_MSG, "Cannot open: %s", path);
		}
		return -1;
	    }
#ifdef HAVE_LIBZ
	    if((int)strlen(newpath) > 3 && !strcmp(newpath+strlen(newpath)-3, ".gz")) {
		zfd = gzdopen(fd, "rb");
	    }
#endif
	    errno = -1;
	}
#ifdef HAVE_LIBZ
	else if((int)strlen(path) > 3 && !strcmp(path+strlen(path)-3, ".gz")) {
	    zfd = gzdopen(fd, "rb");
	}
#endif

	start = 0;

	if(start_time > wfdisc->time) {
	    start = (int)((start_time - wfdisc->time)*wfdisc->samprate+.5);
	}

	if(end_time > wfdisc->endtime) {
	    *npts = wfdisc->nsamp - start;
	}
	else {
	    *npts = (int)(((end_time - wfdisc->time)*wfdisc->samprate+.5) - start + 1);
	}
	if(start + *npts > wfdisc->nsamp) *npts = wfdisc->nsamp - start;

	if(*npts <= 0) {
#ifdef HAVE_LIBZ
	    if (zfd != Z_NULL) gzclose(zfd);
	    else close(fd);
#else
	    close(fd);
#endif
	    return 0;
	}

	if(pts_wanted < 1) {
	    need = *npts;
	}
	else {
	    num_per_interval = (int)(*npts/((float)pts_wanted*.5) + .5);
	    if(num_per_interval < 1) num_per_interval = *npts;
	    need = (*npts/num_per_interval)*2 + 2;
	    deci_dt = num_per_interval/(2.*wfdisc->samprate);
	}
	data = (float *)malloc(need*sizeof(float));

	if(data == NULL) {
	    snprintf(err_msg, MAX_MSG,
			"cssioReadData: out of memory. reading %s", path);
#ifdef HAVE_LIBZ
	    if (zfd != Z_NULL) gzclose(zfd);
	    else close(fd);
#else
	    close(fd);
#endif
	    return -1;
	}

	if(pts_wanted < 1)
	{
#ifdef HAVE_LIBZ
	    *npts = cssioReadDotw(path, zfd, fd, wfdisc->foff, start, *npts, data,
			wfdisc->datatype, FLOAT_DATA, wfdisc->commid);
#else
	    *npts = cssioReadDotw(path, fd, wfdisc->foff, start, *npts, data,
			wfdisc->datatype, FLOAT_DATA, wfdisc->commid);
#endif
	}
	else
	{
#ifdef HAVE_LIBZ
	    *npts = cssioReadDotwDeci(path, zfd, fd, wfdisc->foff, start, *npts,
			data, wfdisc->datatype, FLOAT_DATA, num_per_interval);
#else
	    *npts = cssioReadDotwDeci(path, fd, wfdisc->foff, start, *npts,
			data, wfdisc->datatype, FLOAT_DATA, num_per_interval);
#endif
	}
	if(err_msg[0] != '\0') {
	    strncat(err_msg, "Short read for: ", MAX_MSG - strlen(err_msg));
	    strncat(err_msg, path, MAX_MSG - strlen(err_msg));
	    strncat(err_msg, "\n", MAX_MSG - strlen(err_msg));
	}
#ifdef HAVE_LIBZ
        if (zfd != Z_NULL) gzclose(zfd);
	else close(fd);
#else
	close(fd);
#endif

	if(*npts > 0) {
	    *tbeg = wfdisc->time + start/wfdisc->samprate;
	    *tdel = (pts_wanted < 1) ? 1./wfdisc->samprate : deci_dt;
	    *pdata = data;
	}
	else {
	    if(data != NULL) free(data);
	}
	return (err_msg[0] == '\0') ? 0 : -1;
}

static void
getCDName(const char *working_dir, const char *dir, const char *dfile,
		char *newpath)
{
	char buf[MAXPATHLEN+1];

	if(working_dir == NULL || (int)strlen(working_dir) < 1)
	{
	    snprintf(newpath, MAXPATHLEN, "%s/%s", dir, dfile);
	}
	else
	{
	    strncpy(buf, working_dir, MAXPATHLEN+1);
	    buf[MAXPATHLEN] = '\0';
	    snprintf(newpath, MAXPATHLEN, "%s%s/%s", strtok(buf, "gsett"),
			dir, dfile);
	}
}
	
static void
getDataFilename(const char *dir, const char *dfile, const char *working_dir,
		char *path)
{
	int n;
	const char *wdir, *sep;
#ifdef HAVE_LIBZ
        struct stat buf;
#endif

	wdir = (working_dir != NULL && working_dir[0] != '\0') ?
			working_dir : "./";
	n = (int)strlen(wdir);
	sep = (wdir[n-1] != '/') ? "/" : "";

	if((int)strlen(dir) > 0 && dir[0] != '-' && strcmp(dir, "."))
	{
	    if(dir[0] != '/') {
		/* use working directory
		 */
		snprintf(path, MAXPATHLEN+1,"%s%s%s/%s", wdir, sep, dir, dfile);
	    }
	    else {
		snprintf(path, MAXPATHLEN+1,"%s/%s", dir, dfile);
	    }
	}
	else {
	    if(dfile[0] != '/') {
		snprintf(path, MAXPATHLEN+1,"%s%s%s", wdir, sep, dfile);
	    }
	    else {
		strncpy(path, dfile, MAXPATHLEN+1);
		path[MAXPATHLEN] = '\0';
	    }
	}

#ifdef HAVE_LIBZ
	if(stat(path, &buf) != 0 && (int)strlen(path) < MAXPATHLEN-3)
	{
	    strcat(path, ".gz");
	}
#endif
}

/**
 * Get an error message set by cssio routines.
 */
const char *
cssioGetErrorMsg(void)
{
	return (err_msg[0] != '\0') ? err_msg : (char *)NULL;
}

void
#ifdef HAVE_STDARG_H
cssioSetErrorMsg(const char *format, ...)
#else
cssioSetErrorMsg(va_alist) va_dcl
#endif
{
	va_list	va;
#ifdef HAVE_STDARG_H
        va_start(va, format);
#else
        char *format;
	va_start(va);
	format = (char *)va_arg(va, char *);
#endif
	err_msg[0] = '\0';
	if(format != NULL) {
	    vsnprintf(err_msg, MAX_MSG, format, va);
	}
	va_end(va);
}

static int num_names = 0;
static char **tmp_names = NULL;

/**
 * Get a temporary file prefix. Returns NULL if the prefix is too long.
 */
const char *
cssioGetTmpPrefix(const string &dir, const string &prefix)
{
	char name[MAXPATHLEN+1];
	int fd;

	if((int)dir.length() + (int)prefix.length() + 1 + 6 > MAXPATHLEN) {
	    cssioSetErrorMsg("cssioGetTmpPrefix: prefix too long.");
	    return NULL;
	}
	snprintf(name, MAXPATHLEN, "%s/%sXXXXXX", dir.c_str(), prefix.c_str());
	fd = mkstemp(name);
	if(fd != -1) close(fd);

	cssioAddTmpPrefix(name);

	return tmp_names[num_names-1];
}

/**
 * Add a prefix to the list of temporary file prefixes.
 */
void
cssioAddTmpPrefix(const string &name)
{
	int len;

	if(tmp_names == NULL) {
	    tmp_names = (char **)malloc(sizeof(char *));
	}
	else {
	    tmp_names = (char **)realloc(tmp_names,
				(num_names+1)*sizeof(char *));
	}
	if(tmp_names == NULL) {
	    logErrorMsg(LOG_ERR, "cssioAddTmpPrefix: malloc failed.\n");
	    return;
	}
	len = (int)name.length()+1;
	tmp_names[num_names] = (char*)malloc(len);
	if(tmp_names[num_names] == NULL) {
	    logErrorMsg(LOG_ERR, "cssioAddTmpPrefix: malloc failed.\n");
	    return;
	}
	strncpy(tmp_names[num_names++], name.c_str(), len);
}

/**
 * Remove all temporary files with the input temporary file prefix.
 */
void
cssioDeleteTmpPrefix(const string &name)
{
    int i, j, n;

    for(i = num_names-1; i >= 0; i--) {
	n = strlen(tmp_names[i]);
	if(!name.compare(tmp_names[i]) || !name.compare(0,n-6,tmp_names[i])) {
	    unlinkTmpFile(name.c_str());
	    free(tmp_names[i]);
	    for(j = i; j < num_names-1; j++) {
		tmp_names[j] = tmp_names[j+1];
		num_names--;
	    }
	}
    }
}

/**
 * Remove all temporary files.
 */
void
cssioDeleteAllTmp()
{
	int i;

	for(i = 0; i < num_names; i++) {
	    unlinkTmpFile(tmp_names[i]);
	    free(tmp_names[i]);
	}
	num_names = 0;
}

static void
unlinkTmpFile(const char *name)
{
	char    path[MAXPATHLEN+1];

	unlink(name);
	snprintf(path, MAXPATHLEN+1, "%s.w", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.wfdisc", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.wfdisc.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.arrival", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.arrival.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.assoc", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.assoc.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.pick", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.pick.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.hydro_features", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.hydro_features.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.infra_features", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.infra_features.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.filter", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.filter.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.origin", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.origin.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.wftag", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.wftag.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.lastid", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.lastid.bak", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.origerr", name);
	unlink(path);
	snprintf(path, MAXPATHLEN+1, "%s.origerr.bak", name);
	unlink(path);
}

#ifdef _OLD__
/**
 * @param wfdisc The input wfdisc structure.
 * @param working_dir The working directory. The wfdisc dir is relative to this.
 * @param start_time The lower limit of the data times.
 * @param end_time The upper limit of the data times.
 * @param pts_wanted The maximum number of data values wanted. Set to 0 to \
 *		return all values.
 * @param npts The actual number of values returned (output).
 * @param tbeg The actual start time of the data values returned (output).
 * @param tdel The time increment of the data values returned (output).
 * @param pdata  The data values returned. (The array is allocated.)
 */
int
cssioReadSpectra(CssFsdiscClass fs, const char *working_dir, int *npts, float **pdata)
{
	int	fd, need;
	char	path[MAXPATHLEN+1];
	float	*data = NULL;
	int	start, rd_len;
	char	datatype[8];
	int	err_no;
#ifdef HAVE_LIBZ
        gzFile  zfd = Z_NULL;
#endif

	*npts = 0;
	*pdata = NULL;
	err_no = -1;
	err_msg[0] = '\0';

	getDataFilename(fs->dir, fs->dfile, working_dir, path);

	if((fd = open(path, O_RDONLY)) == -1)
	{
	    err_no = errno;
	    if(err_no >= 0) {
	        snprintf(err_msg, MAX_MSG, "Cannot open: %s\n%s",
			path, strerror(err_no));
	    }
	    else {
	        snprintf(err_msg, MAX_MSG, "Cannot open: %s", path);
	    }
	    return -1;
	}
#ifdef HAVE_LIBZ
	else if((int)strlen(path) > 3 && !strcmp(newpath+strlen(path)-3, ".gz")) {
	    zfd = gzdopen(fd, "rb");
	}
#endif

	need = fs->nf * 2;
	data = (float *)malloc(need*sizeof(float));

	if(data == NULL) {
	    snprintf(err_msg, MAX_MSG,
			"cssioReadData: out of memory. reading %s", path);
	    return -1;
	}

	/* The first data value to return ( >= 0 ). */
	start = 0;

	/* rd_len is used when reading compressed data */
	rd_len = 0;
	strncpy(datatype, "t4", sizeof(datatype));

	*npts = need;

#ifdef HAVE_LIBZ
	*npts = cssioReadDotw(path, zfd, fd, fs->foff, start, *npts, data, datatype,
				FLOAT_DATA, rd_len);
	if (zfd != Z_NULL) gzclose(zfd);
	else close(fd);
#else
	*npts = cssioReadDotw(path, fd, fs->foff, start, *npts, data, datatype,
				FLOAT_DATA, rd_len);
	close(fd);
#endif

	if (*npts > 0)
	{
	    *pdata = data;
	}
	else
	{
	    if (data != NULL) free(data);
	}
	
	return (err_msg[0] == '\0') ? 0 : -1;
}

#endif
