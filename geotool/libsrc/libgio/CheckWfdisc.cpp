/*
 * NAME
 *      cssioCheckWfdisc
 *
 * AUTHOR
 *      I. Henson
 *      Teledyne Geotech
 */
#include "config.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>
#include <errno.h>

#include "cssio.h"
#include "gobject++/CssTables.h"

/**
 * Check the nsamp, samprate and endtime values in a wfdisc. If the nsamp
 * value is <= 0, compute it from the datatype, samprate and dotw file size.
 * Check for samprate > 0 and compute the endtime.
 * @param wf Input wfdisc structure to check.
 * @param wdir The working directory.
 * @return 0 for success, nonzero for error. Use cssioGetErrorMsg.
 */
int
cssioCheckWfdisc(CssWfdiscClass *wf, const string &wdir)
{
    int		data_size;
    struct stat	buf;
    char	path[MAXPATHLEN+1];
    char	msg[MAXPATHLEN+100];

    if(!strcmp(wf->datatype, "s4") || !strcmp(wf->datatype, "i4")) {
	data_size = sizeof(int);
    }
    else if(!strcmp(wf->datatype, "s3") || !strcmp(wf->datatype, "i3")) {
	data_size = 3;
    }
    else if(!strcmp(wf->datatype, "s2") || !strcmp(wf->datatype, "i2")) {
	data_size = sizeof(short);
    }
    else if(!strcmp(wf->datatype, "t4") || !strcmp(wf->datatype, "f4")) {
	data_size = sizeof(float);
    }
    else if(!strcmp(wf->datatype, "v4")) {
	data_size = sizeof(float);
    }
    else if(!strcmp(wf->datatype, "g2")) {
	data_size = sizeof(short);
    }
    else if(!strcmp(wf->datatype, "c2")) {
	data_size = sizeof(long);
    }
    else if(!strcmp(wf->datatype, "c4")) {
	data_size = sizeof(long);
    }
    else if(!strcmp(wf->datatype, "sd")) {
	data_size = sizeof(long);
    }
    else if(!strcmp(wf->datatype, "e1") || !strcasecmp(wf->datatype, "ca")) {
	return(0); /* can't easily check this type */
    }
    else {
	snprintf(msg, sizeof(msg), "unknown datatype: %s", wf->datatype);
	cssioSetErrorMsg(msg);
	return(-1);
    }
    if(wf->nsamp <= 0)	/* should check for -1 (CSS_NULL) only */
    {
	int len = (int)wdir.length();
	const char *sep = (len > 0 && wdir[len-1] == '/') ? "" : "/";

	path[0] = '\0';
	if((int)strlen(wf->dir) > 0 && wf->dir[0] != '-' && strcmp(wf->dir,"."))
	{
	    if(wf->dir[0] != '/') {
		snprintf(path, sizeof(path), "%s%s%s/%s",
				wdir.c_str(), sep, wf->dir, wf->dfile);
	    }
	     else {
		snprintf(path, sizeof(path), "%s/%s", wf->dir, wf->dfile);
	    }
	}
	else
	{
	    if(wf->dfile[0] != '/') {
		snprintf(path, sizeof(path), "%s%s%s",
				wdir.c_str(), sep, wf->dfile);
	    }
	}
	if(stat(path, &buf) < 0)
	{
	    snprintf(msg, sizeof(msg), "cannot stat %s: %s",
			path, strerror(errno));
	    cssioSetErrorMsg(msg);
	    return(-2);
	}
	wf->nsamp = buf.st_size/data_size;
    }
    if(wf->samprate <= 0)
    {
	cssioSetErrorMsg("samprate <= 0.");
	return(-3);
    }
    wf->endtime = wf->time + (wf->nsamp-1)/wf->samprate;
    return(0);
}
