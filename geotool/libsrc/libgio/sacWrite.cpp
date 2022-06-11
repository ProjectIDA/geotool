/** \file sacWrite.cpp
 *  \brief SAC I/O routines
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#include "libgio.h"
#include "DataMethod.h"

extern "C" {
#include "libgmath.h"
#include "sac.h"
#include "libstring.h"
#include "libtime.h"
}

static bool write_ts(GTimeSeries *ts, Waveform *w, SAC_HEADER *s,
		CssOriginClass *origin, const char *wa, const char *prefix,
		char *sac_file, int raw);
static void newSacFile(const char *prefix, const char *sta, const char *chan,
		char *sac_file, size_t size);
static void stringToSac(char *a, const char *b, int n);
static void sacFlipHeader(SAC *sac);

bool
sacWriteCDC(const char *prefix, const char *wa, gvector<Waveform *> &wvec,
		const char *remark, int raw, CssOriginClass **origins)
{
    char	sac_file[MAXPATHLEN+1];
    int		i, j;
    int		inum;
    DateTime	dt;
    double	az, baz;
    gvector<DataMethod *> methods;
    SAC_HEADER	s, sac_header_null = SAC_HEADER_NULL;
    double	dlat, dlon, delta;
	
    for(i = 0; i < wvec.size(); i++)
    {
	memcpy(&s, &sac_header_null, sizeof(SAC_HEADER));

	stringToSac(s.b.kstnm, wvec[i]->sta(), 8);
	stringToSac(s.b.kcmpnm, wvec[i]->chan(), 8);
	s.a.delta = (float) wvec[i]->segment(0)->tdel();
	s.a.odelta = (float) wvec[i]->segment(0)->tdel();
	s.a.stla = (float) wvec[i]->lat();
	s.a.stlo =  (float) wvec[i]->lon();
	s.a.iftype = ITIME;
	s.a.internal4 = 6;	// nvhdr

	CssOriginClass *origin = (origins) ? origins[i] : NULL;
	if(origin)
	{
	    timeEpochToDate(origin->time, &dt);

	    /* if dt.second = 10.032999992370605, then
	       the logic below would record nzmsec incorrectly as 32, and not as 33.
	       Check for this case, and if found, add a correction factor to the
	       epoch time
	     */
	    s.a.nzsec = (int) dt.second;
	    s.a.nzmsec = (int)((dt.second - s.a.nzsec)*1000.);
	    inum = (int)((dt.second - s.a.nzsec)*10000.)  - (s.a.nzmsec * 10);
	    if (inum > 5)
	    {
	         timeEpochToDate(origin->time + 0.0005, &dt);
	    }

	    s.a.nzyear = (int) dt.year;
	    s.a.nzjday = (int) timeDOY(&dt);
	    s.a.nzhour = (int) dt.hour;
	    s.a.nzmin = (int) dt.minute;
	    s.a.nzsec = (int) dt.second;
	    s.a.nzmsec = (int)((dt.second - s.a.nzsec)*1000.);
	    s.a.o = 0.0;

	    s.a.evla = (float) origin->lat;
	    s.a.evlo = (float) origin->lon;
	    s.a.evdp = (float) origin->depth;

	    dlat = origin->lat;
	    dlon = origin->lon;
	    delta = -999.0;
	    deltaz(dlat, dlon, wvec[i]->lat(), wvec[i]->lon(),
			&delta, &az, &baz);
	    s.a.gcarc = (float) delta;
	}

	if(wvec[i]->num_dw <= 0)
	{
	    GTimeSeries *ts = new GTimeSeries(wvec[i]->ts);

	    if(raw) {
		ts->reread();
                wvec[i]->ts->getMethods("CutData", methods);
		// apply CutData, if found.
		for(int k = 0; k < methods.size(); k++) {
		    methods[k]->applyMethod(1, &ts);
		}
	    }
	    if(!write_ts(ts, wvec[i], &s, origin, wa, prefix, sac_file, raw))
	    {
		ts->deleteObject();
		return false;
	    }
	    ts->deleteObject();
	    continue;
	}

	for(j = 0; j < wvec[i]->num_dw; j++)
	{
	    GDataPoint *d1 = wvec[i]->dw[j].d1;
	    GDataPoint *d2 = wvec[i]->dw[j].d2;

	    GTimeSeries *ts = wvec[i]->ts->subseries(d1->time(), d2->time());
	    if(raw) {
		ts->reread();
                wvec[i]->ts->getMethods("CutData", methods);
		// apply CutData, if found.
		for(int k = 0; k < methods.size(); k++) {
		    methods[k]->applyMethod(1, &ts);
		}
	    }
	    if(!write_ts(ts, wvec[i], &s, origin, wa, prefix, sac_file, raw))
	    {
		ts->deleteObject();
		return false;
	    }
	    ts->deleteObject();
	}
    }
    return true;
}

static bool
write_ts(GTimeSeries *ts, Waveform *w, SAC_HEADER *s, CssOriginClass *origin,
	const char *wa, const char *prefix, char *sac_file, int raw)
{
    char error[MAXPATHLEN+100];
    int k, l, npts, err;
    int inum;
    double calib;
    bool calib_applied = false;
    FILE *fp;
    int flip_bytes;
    union
    {
	char    a[4];
	float   f;
	int    i;
	short   s;
    } e1, e2;
    e1.a[0] = 0; e1.a[1] = 0;
    e1.a[2] = 0; e1.a[3] = 1;
    flip_bytes = (e1.i == 1) ? 0 : 1;
flip_bytes = 0; // write in native byte order

    if(!raw) {
        /* Remove calib if is has been applied.
         */
        calib_applied = ts->getMethod("CalibData") ? true : false;
    }

    for(k = 0; k < ts->size(); k++)
    {
	GSegment *seg = ts->segment(k);
	calib = (seg->calib() != 0.) ? seg->calib() : 1.;
	s->a.scale = (float) calib;
	npts = seg->length();
	s->a.npts = npts;

	if(origin == NULL)
	{
	    DateTime dt;
	    timeEpochToDate(seg->tbeg(), &dt);

	    /* if dt.second = 10.032999992370605, then
	       the logic below would record nzmsec incorrectly as 32, and not as 33.
	       Check for this case, and if found, add a correction factor to the
	       epoch time
	     */
	    s->a.nzsec = (int) dt.second;
	    s->a.nzmsec = (int)((dt.second - s->a.nzsec)*1000.);
	    inum = (int)((dt.second - s->a.nzsec)*10000.)  - (s->a.nzmsec * 10);
	    if (inum > 5)
	    {
	         timeEpochToDate(seg->tbeg() + 0.0005, &dt);
	    }

	    s->a.nzyear = (int) dt.year;
	    s->a.nzjday = (int) timeDOY(&dt);
	    s->a.nzhour = (int) dt.hour;
	    s->a.nzmin = (int) dt.minute;
	    s->a.nzsec = (int) dt.second;
	    s->a.nzmsec = (int)((dt.second - s->a.nzsec)*1000.);
	    s->a.b = 0.;
	}
	else
	{
	    s->a.b = seg->tbeg() - origin->time;
	}
	s->a.e = s->a.b + seg->tend() - seg->tbeg();

	newSacFile(prefix, w->sta(), w->chan(), sac_file, MAXPATHLEN);

	if((fp = fopen(sac_file, wa)) == NULL) {
	    snprintf(error, sizeof(error),"sac: cannot open %s", sac_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	if(flip_bytes) sacFlipHeader((SAC *)s);
	err = fwrite(s, sizeof(SAC_HEADER), 1, fp);
	if(flip_bytes) sacFlipHeader((SAC *)s);

	if(err != 1)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error),
			"write error: %s\n%s", sac_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", sac_file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return false;
	}

	if(calib_applied && calib != 1.) {
	    for(l = 0; l < npts; l++) {
		seg->data[l] /= calib;
	    }
	}

	if(flip_bytes) {
	    for(l = 0; l < npts; l++) {
		e1.f = seg->data[l];
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		seg->data[l] = e2.f;
	    }
	}

	err = fwrite(seg->data, sizeof(float), npts, fp);

	if(err != npts)
	{
	    if(errno > 0) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
				sac_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", sac_file);
	    }
	    logErrorMsg(LOG_WARNING, error);
	    fclose(fp);
	    return false;
	}
	fclose(fp);
    }
    return true;
}

static void
newSacFile(const char *prefix, const char *sta, const char *chan,
		char *sac_file, size_t size)
{
    struct stat	buf;
    int	i;

    for(i = 0; ; i++)
    {
	snprintf(sac_file, size, "%s.%s.%s.%d.sac", prefix, sta, chan, i);

	if(stat(sac_file, &buf) == -1) {
	    return;
	}
    }
}

static void
stringToSac(char *a, const char *b, int n)
{
    int i;

    if(strcmp(b, "-")) {
	for(i = 0; b[i] != '\0' && i < n; i++) a[i] = b[i];
	while(i < n) a[i++] = ' ';
    }
}

static void
sacFlipHeader(SAC *sac)
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
    int n_floats = 70;
    int n_ints = 40;

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
