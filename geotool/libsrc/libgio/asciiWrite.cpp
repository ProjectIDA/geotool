/** \file asciiWrite.cpp
 *  \brief Defines asciiWriteCDC.
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "libgio.h"
#include "DataMethod.h"
#include "libmath.h"

extern "C" {
#include "libstring.h"
}

static bool write_ts(GTimeSeries *ts, Waveform *w, const char *wa,
		const char *prefix, char *ascii_file, int file_len, int raw);

static void newAsciiFile(const char *prefix, const char *sta,
		const char *chan, char *ascii_file, int file_len);


bool
asciiWriteCDC(const char *prefix, const char *wa,
	gvector<Waveform *> &wvec, const char *remark, int raw)
{
    char	ascii_file[MAXPATHLEN+1];
    int		i, j;
    gvector<DataMethod *> methods;

    for(i = 0; i < wvec.size(); i++)
    {
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
	    if(!write_ts(ts, wvec[i], wa, prefix, ascii_file,
				sizeof(ascii_file), raw))
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
	    if(!write_ts(ts, wvec[i], wa, prefix, ascii_file,
				sizeof(ascii_file), raw))
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
write_ts(GTimeSeries *ts, Waveform *w, const char *wa,
		const char *prefix, char *ascii_file, int file_len, int raw)
{
    int i, j, n;
    char s[50];
    double calib, calper;
    bool calib_applied = false;
    FILE *fp;

    if (!raw) {
        calib_applied = ts->getMethod("CalibData") ? true : false;
    }

    for(int k = 0; k < ts->size(); k++)
    {
	GSegment *seg = ts->segment(k);
	newAsciiFile(prefix, w->sta(), w->chan(), ascii_file, file_len);

	if((fp = fopen(ascii_file, wa)) == NULL) {
	    char error[MAXPATHLEN+100];
	    snprintf(error, sizeof(error), "ascii: cannot open %s", ascii_file);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}

	calib = (seg->calib() != 0.) ? seg->calib() : 1.;
	calper = (seg->calper() != 0.) ? seg->calper() : 1.;
	if(calib_applied && calib != 1.) {
	    n = seg->length();
	    for(int l = 0; l < n; l++) {
		seg->data[l] /= calib;
	    }
	}

	fprintf(fp, "sta=%s chan=%s time=%.5f samprate=%.5f calib=%.5f calper=%.5f \n",
		w->sta(), w->chan(), seg->tbeg(), 1./seg->tdel(), calib,calper);

	fprintf(fp, "data\n");

	n = seg->length();
	for(i = 0; i < n; i++)
	{
	    snprintf(s, sizeof(s), "%f", seg->data[i]);
	    for(j = (int)strlen(s)-1; j > 0 && s[j] == '0'; j--);
	    if(s[j] == '.' && j > 0 && isdigit((int)s[j-1])) s[j] = '\0';
	    fprintf(fp, "%s\n", s);
	}

	if(i != seg->length())
	{
	    char error[MAXPATHLEN+100];
	    if(errno > 0) {
		snprintf(error, sizeof(error), "write error: %s\n%s",
			ascii_file, strerror(errno));
	    }
	    else {
		snprintf(error, sizeof(error), "write error: %s", ascii_file);
	    }
	    fclose(fp);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
	fclose(fp);
    }
    return true;
}

static void
newAsciiFile(const char *prefix, const char *sta, const char *chan,
		char *ascii_file, int file_len)
{
    struct stat buf;

    for(int i = 0; ; i++)
    {
	snprintf(ascii_file, file_len, "%s.%s.%s.%d.ascii", prefix, sta,chan,i);

	if(stat(ascii_file, &buf) == -1) {
	    return;
	}
    }
}
