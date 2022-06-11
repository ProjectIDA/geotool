#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include <math.h>

#include "libcalib.h"
#include "libstring.h"
#include "logErrorMsg.h"

static int isBlank(char *line);
static int getNextLine(FILE *fp, char *line, int n, int *lineno);
static int nextLine(char *s, int *last, char *line, int len);


/* Read a calibration parameter file if CALEX5a format.
 *
 * Example file:
Calibration of the broadband (20 sec) seismometer STS-1 No. 14
1     alias
4     m
0     m0
0     m1
1     m2
48    maxit
1e-5  qac
1e-2  finac
1     ns1
0     ns2

amp   1.3	0.10
del   0.01	0.01
sub   0.	0.
til   0.	0.

bp2
per   20.	1.
bmp   0.7	0.1

end
 */

int
CalibReadPar(char *file, CalibParam *p)
{
	char line[200], *c, *last;
	char error[MAXPATHLEN+100];
	int l, n, ret, nline, lineno;
	FILE *fp;

	if(!(fp = fopen(file, "r"))) {
	    snprintf(error, sizeof(error), "Cannot open file %s\n%s",
		file, strerror(errno));
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}
	memset(p->parfile, 0, sizeof(p->parfile));
	strncpy(p->parfile, file, sizeof(p->parfile)-1);

	lineno = 0;

	/* The first non-blank line is the title
	 */
	nline = sizeof(line)-1;
	while((ret = stringGetLine(fp, line, nline)) != EOF && isBlank(line)) {
	    lineno++;
	}
	if(ret == EOF)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting title.", file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	stringTrim(line);
	memset(p->partitle, 0, sizeof(p->partitle));
	strncpy(p->partitle, line, sizeof(p->partitle)-1);

	/* read input file */
	if((ret=getNextLine(fp, line, nline, &lineno))
		|| !(c=strtok_r(line, "'`", &last)))
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting input file.", file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}
	memset(p->eing, 0, sizeof(p->eing));
	strncpy(p->eing, c, sizeof(p->eing)-1);
	
	/* read output file */
	if((ret=getNextLine(fp, line, nline, &lineno))
		|| !(c=strtok_r(line, "'`", &last)))
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting output file.",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}
	memset(p->ausg, 0, sizeof(p->ausg));
	strncpy(p->ausg, c, sizeof(p->ausg)-1);
	
	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%lf", &p->alias) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting alias value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->m) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting m value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->m0i) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting m0 value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->m1i) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting m1 value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->m2i) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting m2 value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->maxit) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting maxit value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%lf", &p->qac) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting qac value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%lf", &p->finac) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting finac value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->ns1) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting ns1 value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if((ret=getNextLine(fp, line, nline, &lineno)) ||
		sscanf(line, "%d", &p->ns2) != 1)
	{
	    snprintf(error, sizeof(error),
		"Error in file %s line %d: expecting ns2 value",file,lineno);
	    logErrorMsg(LOG_ERR, error);
	    fclose(fp);
	    return -1;
	}

	if(p->finac > 1.0) p->finac = 1.0;

	/* save raw input
	 */
	memset(p->raw, 0, sizeof(p->raw));

	for(l = 0; l < msys && !(ret = getNextLine(fp, line, nline, &lineno))
			&& strncasecmp(line, "end", 3); l++)
	{
	    p->lineno[l] = lineno;
	    stringTrim(line);
	    n = strlen(p->raw);
	    snprintf(p->raw+n, sizeof(p->raw)-n, "%s\n", line);
	}
	fclose(fp);

	if(ret != EOF && strncasecmp(line, "end", 3)) {
	    snprintf(error, sizeof(error),
		    "Parameter file %s was not read to end.", file);
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}

	return CalibGetStartParam(p);
}

int
CalibGetStartParam(CalibParam *p)
{
	char type[4], name[4];
	char error[MAXPATHLEN+100], line[100];
	char text[MAXPATHLEN+20];
	int l, nexttype, ipar, lineno, last;

	if(p->parfile[0] != '\0') {
	    snprintf(text, sizeof(text), "Error in file %s ", p->parfile);
	}
	else {
	    snprintf(text, sizeof(text), "Error ");
	}

	nexttype = 1;
	ipar = 0;
	type[0] = '\0';
	last = 0;
	for(l = 0; l < msys && nextLine(p->raw, &last, line, 100); l++)
	{
	    lineno = p->lineno[l];

	    if(l > 3 && nexttype) {
		type[0] = '\0';
		sscanf(line, "%10s", type);
		if(strcasecmp(type, "lp1") && strcasecmp(type, "hp1") &&
		   strcasecmp(type, "lp2") && strcasecmp(type, "bp2") &&
		   strcasecmp(type, "hp2"))
		{
		    snprintf(error, sizeof(error),
    "subsystem %s not recognized.\nMust be lp1, hp1, lp2, bp2, or hp2", type);
		    logErrorMsg(LOG_ERR, error);
		    return -1;
		}
		if(!nextLine(p->raw, &last, line, 100)
			|| !strncasecmp(line, "end", 3)) break;
	    }

	    switch(l) {
		case 0:
		    if(sscanf(line, "%3s %lf %lf", name, &p->x0[l],
			&p->rho[l]) != 3 || strcasecmp(name, "amp"))
		    {
			snprintf(error, sizeof(error),
				"%s line %d: expecting 'amp'", text, lineno);
			logErrorMsg(LOG_ERR, error);
			return -1;
		    }
		    break;
		case 1:
		    if(sscanf(line, "%3s %lf %lf", name, &p->x0[l],
			&p->rho[l]) != 3 || strcasecmp(name, "del"))
		    {
			snprintf(error, sizeof(error),
				"%s line %d: expecting 'del'", text, lineno);
			logErrorMsg(LOG_ERR, error);
			return -1;
		    }
		    break;
		case 2:
		    if(sscanf(line, "%3s %lf %lf", name, &p->x0[l],
			&p->rho[l]) != 3 || strcasecmp(name, "sub"))
		    {
			snprintf(error, sizeof(error),
				"%s line %d: expecting 'sub'", text, lineno);
			logErrorMsg(LOG_ERR, error);
			return -1;
		    }
		    break;
		case 3:
		    if(sscanf(line, "%3s %lf %lf", name, &p->x0[l],
			&p->rho[l]) != 3 || strcasecmp(name, "til"))
		    {
			snprintf(error, sizeof(error),
				"%s line %d: expecting 'til'", text, lineno);
			logErrorMsg(LOG_ERR, error);
			return -1;
		    }
		    break;
		default:
		    if(sscanf(line, "%3s %lf %lf", name, &p->x0[l],
			&p->rho[l]) != 3)
		    {
			snprintf(error, sizeof(error),
				"%s line %d: expecting 3 fields", text, lineno);
			logErrorMsg(LOG_ERR, error);
			return -1;
		    }
	    }

	    if(p->rho[l] != 0.0) {
		strcpy(p->name[ipar], name);
		p->x00[ipar] = p->x0[l];
		p->r00[ipar] = p->rho[l];
		ipar++;
	    }
	    if(strlen(type) == 3 && type[2] == '2') nexttype = !nexttype;
	    strcpy(p->typ[l], type);
	}

	if(ipar != p->m || ipar > mpar || l != 4+p->m1i+2*p->m2i) {
	    snprintf(error, sizeof(error),
		    "%s: wrong number of parameters.", text);
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}

	return 0;
}

static int
nextLine(char *s, int *last, char *line, int len)
{
	int i;

	while(s[*last] != '\0' && s[*last] == '\n') (*last)++;

	for(i = 0; i < len && s[*last] != '\0' && s[*last] != '\n'; i++) {
	    line[i] = s[*last];
	    (*last)++;
	}
	line[i] = '\0';

	return (line[0] != '\0') ? 1 : 0;
}

static int
isBlank(char *line)
{
	while(*line != '\0' && isspace((int)*line)) line++;
	return (*line == '\0') ? 1 : 0;
}

static int
getNextLine(FILE *fp, char *line, int n, int *lineno)
{
	int ret;

	while((ret = stringGetLine(fp, line, n)) != EOF
		&& (*line == '\0' || *line == ' ')) (*lineno)++;
	if(ret != EOF) (*lineno)++;
	return ret;
}

int
CalibReadSignal(char *file, int type, CalibSignal *cs)
{
	FILE *fp;
	int i, lineno, ret, npts;
	char line[2001];
	char error[MAXPATHLEN+100];
	CalibSignal cs_null = CALIB_SIGNAL_NULL;

	*cs = cs_null;

	if(!(fp = fopen(file, "r"))) {
	    snprintf(error, sizeof(error), "Cannot open file %s\n%s",
			file, strerror(errno));
	    logErrorMsg(LOG_ERR, error);
	    return -1;
	}
	cs->type = type;

	lineno = 1;
	if(stringGetLine(fp, cs->title, sizeof(cs->title)-1)) {
	    goto ERROR_RETURN;
	}
	lineno++;

	while((ret=stringGetLine(fp, line, 2000)) != EOF && line[0] == '%') {
	    lineno++;
	}
	if(ret == EOF) {
	    goto ERROR_RETURN;
	}
	if(sscanf(line, "%d %*s %lf %lf %lf",
		&npts, &cs->dt, &cs->tmin, &cs->tsec) != 4)
	{
	    goto ERROR_RETURN;
	}
	if(!(cs->x = (double *)mallocWarn(npts*sizeof(double)))) {
	    fclose(fp);
	    return -1;
	}

	for(i = 0; i < npts && fscanf(fp, "%lf", &cs->x[i]) == 1; i++);
	if(i < npts) {
	    snprintf(error, sizeof(error), "End of data encountered\nfile %s",
			file);
	    logErrorMsg(LOG_WARNING, error);
	}
	fclose(fp);

	cs->npts = i;

	return 0;

    ERROR_RETURN:
	fclose(fp);
	snprintf(error, sizeof(error), "Error reading file %s, line %d",
		file, lineno);
	logErrorMsg(LOG_ERR, error);
	return -1;
}
