/** \file ResponseFile.cpp
 *  \brief Defines class ResponseFile.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/param.h>
#include <errno.h>

#include "ResponseFile.h"

/*
 * Instrument response routines.
 *
 * AUTHOR
 *	Ivan Henson -- Nov 1991
 *	Teledyne Geotech
 */

#define MAX_LINE_LEN 128

#define GETLINE(fp,Line,n,line_num) \
{ \
    getNextLine(fp, Line, n, &line_num); \
    if(Line[0] == '\0')	\
    {	\
	GError::setMessage("format error line %d in %s",line_num,file.c_str());\
	return(NULL); \
    } \
}

#define MALLOC(ptr,num,type) \
    if((ptr = (type *)malloc((num)*sizeof(type))) == (type *)0 && num > 0)\
    { \
	fprintf(stderr, "ResponseFile malloc error.\n"); \
	GError::setMessage("ResponseFile malloc error."); \
	return NULL; \
    }

#define LINE_ERR \
{ \
    GError::setMessage("format error line %d of %s", line_num, file.c_str()); \
    fclose(fp); delete resp; \
    return false; \
}
#define LINE2_ERR \
{ \
    GError::setMessage("format error line %d of %s", *line_num, file.c_str()); \
    fclose(fp); \
    return NULL; \
}

static gvector<ResponseFile *> resp_files;

static int getNextLine(FILE *fp, char *line, int n, int *line_num);

#ifndef M_PI
#define	M_PI 3.14159265358979323846
#endif

/*
 * Open a response file. Return a ResponseFile object for a response file. The
 * ResponseFile's vector contains a Response object for each response in the
 * file. The Response class members are
 *
 *      string      source;         // "theoretical" or "measured"
 *      int         stage;          // stage number
 *      string      des;            // description
 *      string      type;           // fap, paz, or fir
 *      string      input_units;    // d, v, or a
 *      string      output_units;   // usually counts
 *      string      author;         // author or information source
 *      double      a0;             // normalization for paz / gain for fir
 *      double      b58_sensitivity;//!< calibration
 *      double      b58_frequency;  //!< calibration frequency
 *      int         npoles;         // number of poles for paz type
 *      int         nzeros;         // number of zeros for paz type
 *      FComplex    *pole;          // poles for paz type
 *      FComplex    *pole_err;      // poles for paz type
 *      FComplex    *zero;          // zeros for paz type
 *      FComplex    *zero_err;      // zeros for paz type
 *      int         nfap;           // number of fap triplets
 *      float       *fap_f;         // frequency for fap type
 *      float       *fap_a;         // amplitude for fap type
 *      float       *fap_p;         // phase for fap type
 *      float       *amp_error;     // amplitude error for fap type
 *      float       *phase_error;   // phase error for fap type
 *      float       input_samprate; // input samples/sec for fir type
 *      float       output_samprate;// output samples/sec
 *      int         num_n;          // # numerator coeffs. for fir
 *      int         num_d;          // # denominator coeffs. for fir
 *      float       *fir_n;         // numerator for fir type
 *      float       *fir_n_error;   // numerator error for fir type
 *      float       *fir_d;         // denominator for fir type
 *      float       *fir_d_error;   // denominator error for fir
 *      GSE_CAL     *cal;           // associated CAL for GSE format
 */

/** Constructor. Read all the responses from the input file, in either CSS or
 *  GSE2.0 format. Use the function ResponseFile instead of this constructor
 *  in situations where the same file could be read multiple times. This
 *  constructor will always read the file each time it is used. This
 *  ResponseFile function will not read the same file twice, but instead will
 *  return the ResponseFile object for a file that has previously been read.
 *  @param[in] filename the filename.
 *  the response file.
 */
ResponseFile::ResponseFile(const string &filename) :
		file(filename), responses(), io_error(false)
{
    if( !open() )  {
	io_error = true;
	return;
    }
}

/** Destructor. */
ResponseFile::~ResponseFile(void)
{
    responses.clear();
}

/** Open the file and read the response information into one or more Response
 *  objects. Both CSS formatted response files and GSE2.0 formatted files can
 *  be read. The function attempts to read the file as a CSS formatted file and
 *  it that fails, it attempts to read it as a GSE2.0 formatted file.
 */
bool ResponseFile::open(void)
{
    char line[MAX_LINE_LEN];
    FILE *fp;

    if(file.empty()) {
	GError::setMessage("ResponseFile: filename not specified.");
	return false;
    }
	
    if((fp = fopen(file.c_str(), "r")) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("cannot open %s\n%s", file.c_str(),
				strerror(errno));
	}
	else {
	    GError::setMessage("cannot open %s", file.c_str());
	}
	return false;
    }
    if(!stringGetLine(fp, line, MAX_LINE_LEN-1) &&
	(!strncasecmp(line, "# paz", 5) || !strncasecmp(line, "# fap", 5)))
    {
	fclose(fp);
	return(openTGFile()); // read old style Teledyne Geotech format
    }
    else
    {
	fclose(fp);
	if( openCssFile() ) {
	    return true;
	}
	else if((int)responses.size() <= 0) {
	    if( !readGseResponse() ) {
		return false;
	    }
	    else if((int)responses.size() <= 0) {
		return false;
	    }
	    return true;
	}
	else {
	    return false;
	}
    }
}

/** Read the file as a standard CSS formatted response file.
 *  @returns true for success. returns false if an error was encountered and
 */
bool ResponseFile::openCssFile(void)
{
    int line_num, ret;
    char line[MAX_LINE_LEN];
    FILE *fp;
    Response *resp;

    if((fp = fopen(file.c_str(), "r")) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("cannot open %s\n%s",
			file.c_str(), strerror(errno));
	}
	else {
	    GError::setMessage("cannot open %s", file.c_str());
	}
	return false;
    }

    resp = new Response();

    line_num = 0;
    while((ret = getNextLine(fp, line, MAX_LINE_LEN-1, &line_num)) != EOF)
    {
	if( determineType(line, resp, line_num) )
	{
	    if(!strcasecmp(resp->type.c_str(), "paz"))
	    {
		if( !readPAZ(fp, resp, &line_num) ) {
		    LINE_ERR;
		}
	    }
	    else if(!strcasecmp(resp->type.c_str(), "fap"))
	    {
		if( !readFAP(fp, resp, &line_num) ) {
		    LINE_ERR;
		}
	    }
	    else if(!strcasecmp(resp->type.c_str(), "fir"))
	    {
		if( !readFIR(fp, resp, &line_num) ) {
		    LINE_ERR;
		}
	    }
	    else {
		GError::setMessage(
			"Unrecognized response type: '%s'\nline %d of %s",
			resp->type.c_str(), line_num, file.c_str());
		fclose(fp);
		delete resp;
		return false;
	    }
	    responses.push_back(resp);
	    resp = new Response();
	}
    }
    fclose(fp);
    delete resp;
    return true;
}

/** Determine the response type ("paz", "fir", or  "fap") of the next response
 *  int the file. This function sets the following members of the Response
 *  object: source, des, type, author, and stage.
 *  @param[in] line the beginning line of a response.
 *  @param[in] resp a Response object to hold the response information.
 *  @param[in] line_num the line number for the input line.
 *  @returns true for success. Returns false if the type cannot be determined.
 */
bool ResponseFile::determineType(char *line, Response *resp, int line_num)
{
    char *s, *s1, *s2, *s3, *s4, *last;

    resp->source.clear();
    resp->des.clear();
    resp->type.clear();
    resp->author.clear();

    if( !(s1 = strtok_r(line, " \t", &last)) ||
	!(s2 = strtok_r(NULL, " \t", &last)) ||
	!(s3 = strtok_r(NULL, " \t", &last)) ||
	!(s4 = strtok_r(NULL, " \t", &last)) ||
	!stringToInt(s2, &resp->stage))
    {
	GError::setMessage("Warning: bad format: line %d of %s\n",
		line_num, file.c_str());
	return false;
    }
    if( strcasecmp(s4, "paz") && strcasecmp(s4, "fap") && strcasecmp(s4, "fir"))
    {
	GError::setMessage("unknown response type, line %d of %s",
		line_num, file.c_str());
	return false;
    }

    resp->source.append(s1);
    resp->des.append(s3);
    resp->type.append(s4);

    /* copy the remainder of the line to resp->author
     */
    while((s = strtok_r(NULL, " \t", &last))) {
	if(resp->author.length() > 0) {
	    resp->author.append(" ");
	}
	resp->author.append(s);
    }

    return true;
}

/** Read a PAZ response. The following members of the Response object are
 *  set by this function: a0, npoles, pole, pole_err, nzeros, zero, zero_err.
 *  The format of the PAZ response is illustrated by the example below.
 *  Each line consists of one or more free-formatted entries. Blank lines and
 *  lines that start with '#' are skipped. The entries of the PAZ response are
 *  annotated in the example.  The first line contains the source
 *  (theoretical, observed, etc), the stage number, the description, and
 *  the type (paz, fir, or fap). This function assumes that the first line has
 *  already been read, and looks first for the a0 entry.
 *
\code
# stage -1
#
 theoretical  1    instrument    paz 	// source stage des type
0.0903			// a0
4			// npoles
-0.02356 0.02356 0 0	// pole[0].re pole[0].im pole_err[0].re pole_err[0].im
-0.02356 -0.02356 0 0   // pole[1].re pole[1].im pole_err[1].re pole_err[1].im
-50 32.2 0 0 		// pole[2].re pole[2].im pole_err[2].re pole_err[2].im
-50 -32.2 0 0 		// pole[3].re pole[3].im pole_err[3].re pole_err[3].im

5			// nzeros
0 0 0 0			// zero[0].re zero[0].im zero_err[0].re zero_err[0].im
0 0 0 0			// zero[1].re zero[1].im zero_err[1].re zero_err[1].im
138 144 0 0		// zero[2].re zero[2].im zero_err[2].re zero_err[2].im
138 -144 0 0		// zero[3].re zero[3].im zero_err[3].re zero_err[3].im
0 0 0 0			// zero[4].re zero[4].im zero_err[4].re zero_err[4].im
\endcode
 *
 *  @param[in] fp a FiLE pointer.
 *  @param[in,out] resp a Response object
 *  @param[in,out] line_num the line number in the file.
 */
bool ResponseFile::readPAZ(FILE *fp, Response *resp, int *line_num)
{
    int i;
    char line[MAX_LINE_LEN];

    resp->type.assign("paz");

    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%le", &resp->a0) != 1)
    {
	return false;
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
	|| sscanf(line, "%d", &resp->npoles) != 1)
    {
	return false;
    }
    MALLOC(resp->pole, resp->npoles, FComplex);
    MALLOC(resp->pole_err, resp->npoles, FComplex);
    for(i = 0; i < resp->npoles; i++)
    {
	resp->pole_err[i].re = 0.;
	resp->pole_err[i].im = 0.;
	// read pole.re pole.im pole_err.re pole_err.im or if only two numbers
	// are found, read pole.re and pole.im
	if( getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    (sscanf(line, "%e %e %e %e", &resp->pole[i].re, &resp->pole[i].im,
		&resp->pole_err[i].re, &resp->pole_err[i].im) != 4 &&
	     sscanf(line, "%e %e", &resp->pole[i].re, &resp->pole[i].im) != 2) )
	{
	    return false;
	}
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF)
    {
	return true;
    }
    else if(sscanf(line, "%d", &resp->nzeros) != 1)
    {
	return false;
    }
    MALLOC(resp->zero, resp->nzeros, FComplex);
    MALLOC(resp->zero_err, resp->nzeros, FComplex);
    for(i = 0; i < resp->nzeros; i++)
    {
	resp->zero_err[i].re = 0.;
	resp->zero_err[i].im = 0.;
	// read zero.re zero.im zero_err.re zero_err.im or if only two numbers
	// are found, read zero.re and zero.im
	if( getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    (sscanf(line, "%e %e %e %e", &resp->zero[i].re, &resp->zero[i].im,
		&resp->zero_err[i].re, &resp->zero_err[i].im) != 4 &&
	     sscanf(line, "%e %e", &resp->zero[i].re, &resp->zero[i].im) != 2) )
	{
	    return false;
	}
    }
    return true;
}

/** Read a FAP response. The following members of the Response object are
 *  set by this function: nfap, fap_f, fap_a, fap_p, amp_error, and phase_error.
 *  The format of the FAP response is illustrated by the example below.
 *  Each line consists of one or more free-formatted entries. Blank lines and
 *  lines that start with '#' are skipped. The entries of the FAP response are
 *  annotated in the example.  The first line contains the source
 *  (theoretical, observed, etc), the stage number, the description, and
 *  the type (paz, fir, or fap). This function assumes that the first line has
 *  already been read, and looks first for the nfap entry, followed by nfap
 *  lines of fap_f, fap_a, fap_p, amp_error and phase_error.
\code
#
# Instrument response for WRA_bb.1
# In unit of displacement (count/nm)
#
#    source stage    des       type
  theoretical  0   instrument    fap   computed-by-Inst-response
104	// nfap
#  fap_f      fap_a      fap_p   amp_error  phase_error
 0.010000  4.346282e-02  178.6     0.0000    0.0000
 0.010766  4.894528e-02  172.7     0.0000    0.0000
 0.011590  5.484414e-02  167.1     0.0000    0.0000
 0.012478  6.118266e-02  161.7     0.0000    0.0000
 0.013434  6.798585e-02  156.7     0.0000    0.0000
 0.014463  7.528067e-02  151.8     0.0000    0.0000
 0.015570  8.309629e-02  147.3     0.0000    0.0000
 0.016763  9.146448e-02  142.9     0.0000    0.0000
...
\endcode
 *
 *  @param[in] fp a FiLE pointer.
 *  @param[in,out] resp a Response object
 *  @param[in,out] line_num the line number in the file.
 */
bool ResponseFile::readFAP(FILE *fp, Response *resp, int *line_num)
{
    char line[MAX_LINE_LEN];
    double degrees_to_radians;

    resp->type.assign("fap");
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
	|| sscanf(line, "%d", &resp->nfap) != 1)
    {
	return false;
    }
    MALLOC(resp->fap_f, resp->nfap, float);
    MALLOC(resp->fap_a, resp->nfap, float);
    MALLOC(resp->fap_p, resp->nfap, float);
    MALLOC(resp->amp_error, resp->nfap, float);
    MALLOC(resp->phase_error, resp->nfap, float);
    degrees_to_radians = acos(-1.)/180.;
    for(int i = 0; i < resp->nfap; i++)
    {
	
	resp->amp_error[i] = 0.;
	resp->phase_error[i] = 0.;

	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF)
	{
	    int n = resp->nfap - i;
	    if(n == 1) {
	       fprintf(stderr, "Missing 1 fap value in %s\n", file.c_str());
	    }
	    else {
	       fprintf(stderr, "Missing %d fap values in %s\n",n,file.c_str());
	    }
	    resp->nfap = i;
	    break;
	}
	else if(sscanf(line, "%e %e %e %e %e", &resp->fap_f[i], &resp->fap_a[i],
	    &resp->fap_p[i], &resp->amp_error[i], &resp->phase_error[i]) != 5
		&& sscanf(line, "%e %e %e", &resp->fap_f[i], &resp->fap_a[i],
			&resp->fap_p[i]) != 3)
	{
	    return false;
	}
	if(resp->fap_a[i] == 0.) resp->fap_a[i] = 1.e-30;
	resp->fap_p[i] *= degrees_to_radians;
    }
    return true;
}

/** Read a FIR response. The following members of the Response object are
 *  set by this function: samprate, num_n, fir_n, fir_n_error, num_d, fir_d,
 *  and fir_d_error. The format of the FAP response is illustrated by the
 *  example below. Each line consists of one or more free-formatted entries.
 *  Blank lines and lines that start with '#' are skipped. The entries of the
 *  FIR response are annotated in the example.  The first line contains the
 *  source (theoretical, observed, etc), the stage number, the description,
 *  and the type (paz, fir, or fap). This function assumes that the first line
 *  has already been read, and looks for the samprate entry.
\code
#
#  stage-4
#
 theoretical  4    digitizer    fir	// source stage des type
30000			// input samprate
34			// num_n
3.788770e-05  0		// fir_n[0] fir_n_error[0]
1.997270e-04  0		// fir_n[1] fir_n_error[1]
5.912770e-04  0		// fir_n[2] fir_n_error[2]
1.198340e-03  0		// fir_n[3] fir_n_error[3]
1.677200e-03  0		// fir_n[4] fir_n_error[4]
1.234440e-03  0		// fir_n[5] fir_n_error[5]
-1.158770e-03  0	// fir_n[6] fir_n_error[6]
-6.071730e-03  0	// fir_n[7] fir_n_error[7]
-1.261020e-02  0	// fir_n[8] fir_n_error[8]
-1.766690e-02  0	// fir_n[9] fir_n_error[9]
...			// fir_n[i] fir_n_error[i]
0			// num_d
\endcode
 *
 *  @param[in] fp a FiLE pointer.
 *  @param[in,out] resp a Response object
 *  @param[in,out] line_num the line number in the file.
 */
bool ResponseFile::readFIR(FILE *fp, Response *resp, int *line_num)
{
    int i;
    char line[MAX_LINE_LEN];

    resp->type.assign("fir");
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%le", &resp->input_samprate) != 1)
    {
	return false;
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%d", &resp->num_n) != 1)
    {
	return false;
    }
    MALLOC(resp->fir_n, resp->num_n, float);
    MALLOC(resp->fir_n_error, resp->num_n, float);
    for(i = 0; i < resp->num_n; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%e %e", &resp->fir_n[i], &resp->fir_n_error[i]) != 2)
	{
	    return false;
	}
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF)
    {
	return true;
    }
    else if(sscanf(line, "%d", &resp->num_d) != 1)
    {
	return false;
    }
    MALLOC(resp->fir_d, resp->num_d, float);
    MALLOC(resp->fir_d_error, resp->num_d, float);
    for(i = 0; i < resp->num_d; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%e %e", &resp->fir_d[i], &resp->fir_d_error[i]) != 2)
	{
	    return false;
	}
    }
    return true;
}

/** Read the response information from a GSE2.0 formatted file. PAZ, FIR and
 *  FAP response types are read.
 *  @return true for success. Returns false if the file cannot be read
 *	correctly.
 */
bool ResponseFile::readGseResponse(void)
{
/* NAME
 *	fills ResponseFile structure for the input GSE2.0 formatted file.
 *	(see response.h)
 *
 * AUTHOR
 *	Ivan Henson -- June 2000
 *	Multimax, Inc.
 */
    int line_num, ret;
    char line[MAX_LINE_LEN];
    FILE *fp;
    GSE_CAL cal;
    Response *resp;

    if((fp = fopen(file.c_str(), "r")) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("cannot open %s\n%s",
			file.c_str(), strerror(errno));
	}
	else {
	    GError::setMessage("cannot open %s", file.c_str());
	}
	return false;
    }

    line_num = 0;
    while((ret = getNextLine(fp, line, MAX_LINE_LEN-1, &line_num)) != EOF)
    {
	if(!strncasecmp(line, "CAL2", 4))
	{
	    if(!readCAL2(line, &cal, line_num, file.c_str()) ) {
		fclose(fp);
		return false;
	    }
	}
	else if(!strncasecmp(line, "PAZ2", 4))
	{
	    if((resp = readPAZ2(fp,line,&cal,&line_num,file.c_str())) == NULL) {
		fclose(fp);
		return false;
	    }
	    responses.push_back(resp);
	}
	else if(!strncasecmp(line, "FAP2", 4))
	{
	    if((resp = readFAP2(fp,line,&cal,&line_num,file.c_str())) == NULL) {
		fclose(fp);
		return false;
	    }
	    responses.push_back(resp);
	}
	else if(!strncasecmp(line, "FIR2", 4))
	{
	    if((resp = readFIR2(fp,line,&cal,&line_num,file.c_str())) == NULL) {
		fclose(fp);
		return false;
	    }
	    responses.push_back(resp);
	}
	// skip GEN2, DIG2
    }
    fclose(fp);
    return true;
}

/** Read a GSE2.0 CAL2 line.
 *  @param[in] line the CAL2 line
 *  @param[out] cal the CAL2 information.
 *  @param[in] line_num the position of the line in the input file.
 *  @param[in] filename the filename.
 *  @return true for success. Returns false if the line cannot be parsed.
 */
bool ResponseFile::readCAL2(char *line, GSE_CAL *cal, int line_num,
				const string &filename)
{
    int n;
    GSE_CAL cal_init = {
	"", "", "", "",		// sta, chan, auxid, instype
	-1., -1., -1.,		// calib, calper, samprat
	{0, 0, 0, 0, 0, 0.},	// ondate
	{0, 0, 0, 0, 0, 0.},	// offdate
    };

    *cal = cal_init;

    n = (int)strlen(line);
    if(n < 56) return false;

    line[10] = '\0';
    if(sscanf(line+5, "%s", cal->sta) != 1) {
	strncpy(cal->sta, "-", 6);
    }
    line[14] = '\0';
    if(sscanf(line+11, "%s", cal->chan) != 1) {
	strncpy(cal->chan, "-", 4);
    }
    line[19] = '\0';
    if(sscanf(line+15, "%s", cal->auxid) != 1) {
	strncpy(cal->auxid, "-", 5);
    }
    line[26] = '\0';
    if(sscanf(line+21, "%s", cal->instype) != 1) {
	strncpy(cal->instype, "-", 7);
    }
    line[37] = '\0';
    if(sscanf(line+27, "%lf", &cal->calib) != 1) {
	GError::setMessage("error reading CAL calib, line %d of %s",
		line_num, filename.c_str());
	return false;
    }
    line[45] = '\0';
    if(sscanf(line+38, "%lf", &cal->calper) != 1) {
	GError::setMessage("error reading CAL calper, line %d of %s",
		line_num, filename.c_str());
	return false;
    }
    line[56] = '\0';
    if(sscanf(line+46, "%lf", &cal->samprat) != 1) {
	GError::setMessage("error reading CAL samprat, line %d of %s",
		line_num, filename.c_str());
	return false;
    }
    if(n >= 67) {
	line[67] = '\0';
	if(sscanf(line+57, "%d%*c%d%*c%d", &cal->ondate.year,
		&cal->ondate.month, &cal->ondate.day) != 3) {
	    GError::setMessage("error reading CAL ondate, line %d of %s",
		    line_num, filename.c_str());
	    return false;
	}
    }
    if(n >= 73) {
	line[73] = '\0';
	if(sscanf(line+68, "%d%*c%d", &cal->ondate.hour, &cal->ondate.minute)
		!= 2) {
	    GError::setMessage("error reading CAL ontime, line %d of %s",
		    line_num, filename.c_str());
	    return false;
	}
    }
    if(n >= 84) {
	line[84] = '\0';
	if(sscanf(line+74, "%d%*c%d*c%d", &cal->offdate.year,
		&cal->offdate.month, &cal->offdate.day) != 3) {
	    GError::setMessage("error reading CAL offdate, line %d of %s",
		    line_num, filename.c_str());
	    return false;
	}
    }
    if(n >= 90) {
	line[90] = '\0';
	if(sscanf(line+85, "%d%*c%d", &cal->offdate.hour,
		&cal->offdate.minute) != 2) {
	    GError::setMessage("error reading CAL offtime, line %d of %s",
		    line_num, filename.c_str());
	    return false;
	}
    }
    return true;
}

/** Read a GSE2.0 PAZ2 response.
 *  @param[in] fp file pointer positioned at the beginning of a PAZ2 response.
 *  @param[in] line the PAZ2 line with stage number, units, a0, npoles,
 *	nzeros, etc.
 *  @param[in] cal the CAL2 information.
 *  @param[in,out] line_num the position in the file of the last line read.
 *  @param[in] filename the filename.
 *  @return a Response object or NULL if there was an error parsing the file.
 */
Response * ResponseFile::readPAZ2(FILE *fp, char *line, GSE_CAL *cal,
		int *line_num, const string &filename)
{
    char s[100];
    int i, n;
    Response *resp;

    resp = new Response();

    MALLOC(resp->cal, 1, GSE_CAL);
    *resp->cal = *cal;

    resp->input_samprate = cal->samprat;
    snprintf(s, sizeof(s), "%s/%s", cal->sta, cal->chan);
    resp->source.assign(s);
    resp->des.assign(cal->instype, 13);

    resp->type.assign("paz");
    n = (int)strlen(line);
    if(n < 55) {
	delete resp;
	return NULL;
    }

    line[7] = '\0';
    if(sscanf(line+5, "%d", &resp->stage) != 1) {
	GError::setMessage("error reading stage number, line %d of %s",
		*line_num, filename.c_str());
	delete resp;
	return NULL;
    }
    resp->input_units.assign(line+8, 1);
    resp->response_units.assign(line+8, 1);

    line[25] = '\0';
    if(sscanf(line+10, "%lf", &resp->a0) != 1) {
	GError::setMessage("error reading paz normalization, line %d of %s",
			*line_num, filename.c_str());
	delete resp;
	return NULL;
    }
    line[51] = '\0';
    if(sscanf(line+48, "%d", &resp->npoles) != 1) {
	GError::setMessage("error reading paz num poles, line %d of %s",
		*line_num, filename.c_str());
	delete resp;
	return NULL;
    }
    line[55] = '\0';
    if(sscanf(line+52, "%d", &resp->nzeros) != 1) {
	GError::setMessage("error reading paz num zeros, line %d of %s",
		*line_num, filename.c_str());
	delete resp;
	return NULL;
    }
    if(n > 59) {
	for(i = n-1; i > 59 && isspace((int)line[i]); i--);
	line[i+1] = '\0';
	for(i = 59; i < n-1 && isspace((int)line[i]); i++);
	resp->author.assign(line+i, 45);
    }

    MALLOC(resp->pole, resp->npoles, FComplex);
    MALLOC(resp->pole_err, resp->npoles, FComplex);
    for(i = 0; i < resp->npoles; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	   sscanf(line, "%15e %15e", &resp->pole[i].re, &resp->pole[i].im) != 2)
	{
	    delete resp;
	    return NULL;
	}
    }
    MALLOC(resp->zero, resp->nzeros, FComplex);
    MALLOC(resp->zero_err, resp->nzeros, FComplex);
    for(i = 0; i < resp->nzeros; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	   sscanf(line, "%15f %15f", &resp->zero[i].re, &resp->zero[i].im) != 2)
	{
	    delete resp;
	    return NULL;
	}
    }
    return resp;
}

/** Read a GSE2.0 FAP2 response.
 *  @param[in] fp file pointer positioned at the beginning of a PAZ2 response.
 *  @param[in] line the FAP2 line with stage number, nfap, units, etc.
 *  @param[in] cal the CAL2 information.
 *  @param[in,out] line_num the position in the file of the last line read.
 *  @param[in] filename the filename.
 *  @return a Response object or NULL if there was an error parsing the file.
 */
Response * ResponseFile::readFAP2(FILE *fp, char *line, GSE_CAL *cal,
			int *line_num, const string &filename)
{
    int i, n;
    char s[100];
    Response *resp;
    double degrees_to_radians;

    resp = new Response();

    MALLOC(resp->cal, 1, GSE_CAL);
    *resp->cal = *cal;

    resp->input_samprate = cal->samprat;
    snprintf(s, sizeof(s), "%s/%s", cal->sta, cal->chan);
    resp->source.assign(s);
    resp->des.assign(cal->instype);

    resp->type.assign("fap");
    n = (int)strlen(line);
    if(n < 27) {
	delete resp;
	return NULL;
    }

    line[7] = '\0';
    if(sscanf(line+5, "%d", &resp->stage) != 1) {
	GError::setMessage("error reading stage number, line %d of %s",
		*line_num, filename.c_str());
	delete resp;
	return NULL;
    }
    resp->input_units.assign(line+8, 1);
    resp->response_units.assign(line+8, 1);

    line[27] = '\0';
    if(sscanf(line+24, "%d", &resp->nfap) != 1) {
	GError::setMessage("error reading fap num triplets, line %d of %s",
		*line_num,filename.c_str());
	delete resp;
	return NULL;
    }
    if(n > 49) {
	for(i = n-1; i > 49 && isspace((int)line[i]); i--);
	line[i+1] = '\0';
	for(i = 49; i < n-1 && isspace((int)line[i]); i++);
	resp->author.assign(line+i, 45);
    }
    MALLOC(resp->fap_f, resp->nfap, float);
    MALLOC(resp->fap_a, resp->nfap, float);
    MALLOC(resp->fap_p, resp->nfap, float);

    degrees_to_radians = M_PI/180.;

    for(i = 0; i < resp->nfap; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%10f %15f %4f", &resp->fap_f[i], &resp->fap_a[i],
		&resp->fap_p[i]) != 3)
	{
	    GError::setMessage("format error line %d of %s",*line_num,
			filename.c_str());
	    delete resp;
	    return NULL;
	}
	if(resp->fap_a[i] == 0.) resp->fap_a[i] = 1.e-30;
	resp->fap_p[i] *= degrees_to_radians;
    }

    return resp;
}

/** Read a GSE2.0 FIR2 response.
 *  @param[in] fp file pointer positioned at the beginning of a PAZ2 response.
 *  @param[in] line the FIR2 line with stage number, a0, deci, symflag, etc.
 *  @param[in] cal the CAL2 information.
 *  @param[in,out] line_num the position in the file of the last line read.
 *  @param[in] filename the filename.
 *  @return a Response object or NULL if there was an error parsing the file.
 */
Response * ResponseFile::readFIR2(FILE *fp, char *line, GSE_CAL *cal,
		int *line_num, const string &filename)
{
    int i, j, n, deci;
    char symflag, s[100];
    Response *resp;
    const char *fn = filename.c_str();

    resp = new Response();

    MALLOC(resp->cal, 1, GSE_CAL);
    *resp->cal = *cal;

    resp->input_samprate = cal->samprat;
    snprintf(s, sizeof(s), "%s/%s", cal->sta, cal->chan);
    resp->source.assign(s);
    resp->des.assign(cal->instype, 13);

    resp->type.assign("fir");
    n = (int)strlen(line);
    if(n < 39) {
	GError::setMessage("format error line %d of %s", *line_num, fn);
	delete resp;
	return NULL;
    }

    line[7] = '\0';
    if(sscanf(line+5, "%d", &resp->stage) != 1) {
	GError::setMessage("error reading stage number, line %d of %s",
		*line_num, fn);
	delete resp;
	return NULL;
    }
    line[18] = '\0';
    if(sscanf(line+8, "%lf", &resp->a0) != 1) {
	GError::setMessage("error reading fir gain, line %d of %s",
		*line_num, fn);
	delete resp;
	return NULL;
    }
    line[23] = '\0';
    if(sscanf(line+19, "%d", &deci) != 1) {
	GError::setMessage("error reading fir deci, line %d of %s",
		*line_num, fn);
	delete resp;
	return NULL;
    }
    line[34] = '\0';
    if(sscanf(line+33, "%c", &symflag) != 1) {
	GError::setMessage("error reading fir symflag, line %d of %s",
		*line_num, fn);
	delete resp;
	return NULL;
    }
    line[39] = '\0';
    if(sscanf(line+35, "%d", &resp->num_n) != 1) {
	    GError::setMessage("error reading fir nfactor, line %d of %s",
		*line_num, fn);
	delete resp;
	return NULL;
    }
    resp->num_d = 0;
    if(n > 39) {
	for(i = n-1; i > 39 && isspace((int)line[i]); i--);
	line[i+1] = '\0';
	for(i = 40; i < n-1 && isspace((int)line[i]); i++);
	resp->author.assign(line+i, 45);
    }
    MALLOC(resp->fir_n, resp->num_n, float);

    n = resp->num_n/5;
    for(i = 0; i < n; i++) {
	j = i*5;
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%e %e %e %e %e",
		&resp->fir_n[j], &resp->fir_n[j+1], &resp->fir_n[j+2],
		&resp->fir_n[j+3], &resp->fir_n[j+4]) != 5)
	{
	    GError::setMessage("format error line %d of %s",*line_num,fn);
	    delete resp;
	    return NULL;
	}
    }
    j = n*5;
    if(resp->num_n - n*5 == 1) {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%e", &resp->fir_n[j]) != 1)
	{
	    GError::setMessage("format error line %d of %s",*line_num,fn);
	    delete resp;
	    return NULL;
	}
    }
    else if(resp->num_n - n*5 == 2) {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
		sscanf(line, "%e %e", &resp->fir_n[j], &resp->fir_n[j+1]) != 2)
	{
	    GError::setMessage("format error line %d of %s",*line_num,fn);
	    delete resp;
	    return NULL;
	}
    }
    else if(resp->num_n - n*5 == 3) {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%e %e %e", &resp->fir_n[j],
		&resp->fir_n[j+1], &resp->fir_n[j+2]) != 3)
	{
	    GError::setMessage("format error line %d of %s",*line_num,fn);
	    delete resp;
	    return NULL;
	}
    }
    else if(resp->num_n - n*5 == 4) {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
		|| sscanf(line, "%e %e %e %e", &resp->fir_n[j],
		&resp->fir_n[j+1], &resp->fir_n[j+2], &resp->fir_n[j+3]) != 3)
	{
	    GError::setMessage("format error line %d of %s",*line_num,fn);
	    delete resp;
	    return NULL;
	}
    }
    if(symflag == 'B' || symflag == 'b')
    {
	n = 2*resp->num_n - 1;
	if(!(resp->fir_n = (float *)realloc(resp->fir_n, n*sizeof(float))))
	{
	    fprintf(stderr, "readFir2: malloc error.");
	    delete resp;
	    return NULL;
	}
	for(i = 0; i < resp->num_n-1; i++) {
	    resp->fir_n[resp->num_n+i] = resp->fir_n[resp->num_n-2-i];
	}
	resp->num_n = n;
    }
    else if(symflag == 'C' || symflag == 'c')
    {
	n = 2*resp->num_n;
	if( !(resp->fir_n = (float *)realloc(resp->fir_n, n*sizeof(float))) )
	{
	    fprintf(stderr, "readFir2: malloc error.");
	    delete resp;
	    return NULL;
	}
	for(i = 0; i < resp->num_n; i++) {
	    resp->fir_n[resp->num_n+i] = resp->fir_n[resp->num_n-1-i];
	}
	resp->num_n = n;
    }
    else if(symflag != 'A' && symflag != 'a') {
	GError::setMessage("can't handle fir symflag: %c, line %d of %s",
		symflag, *line_num, fn);
	delete resp;
	return NULL;
    }

    return resp;
}

/** Read a Teledyne Geotech formatted response file.
 */
bool ResponseFile::openTGFile(void)
{
/* NAME
 *	fills ResponseFile structure for the input response file in tg format.
 *	(see response.h)
 *
 *	returns false for failure
 *
 * AUTHOR
 *	Ivan Henson -- Nov 1991
 *	Teledyne Geotech
 */
    int line_num;
    char line[MAX_LINE_LEN];
    Response *resp;
    FILE *fp;

    if((fp = fopen(file.c_str(), "r")) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("cannot open %s\n%s",
			file.c_str(), strerror(errno));
	}
	else {
	    GError::setMessage("cannot open %s", file.c_str());
	}
	return false;
    }
    if(stringGetLine(fp, line, MAX_LINE_LEN-1) == EOF)
    {
	GError::setMessage("format error, line 1: %s", file.c_str());
	fclose(fp); return false;
    }

    resp = new Response();

    resp->source.assign("geotech");

    line_num = 1;
    if(!strncasecmp(line, "# paz", 5)) {
	resp->type.assign("paz");
    }
    else if(!strncasecmp(line, "# fap", 5)) {
	resp->type.assign("fap");
    }
    else {
	GError::setMessage("error reading response type, line %d of %s",
			line_num, file.c_str());
	fclose(fp); delete resp; return false;
    }
    line_num++;
    if(stringGetLine(fp, line, MAX_LINE_LEN-1) == EOF)
    {
	GError::setMessage("format error, line %d: %s", line_num, file.c_str());
	fclose(fp); delete resp; return false;
    }
    if(!strncasecmp(line, "# dis", 5)) resp->input_units.assign("d");
    else if(!strncasecmp(line, "# vel", 5)) resp->input_units.assign("v");
    else if(!strncasecmp(line, "# acc", 5)) resp->input_units.assign("a");
    else
    {
	GError::setMessage("format error, line %d: %s", line_num, file.c_str());
	fclose(fp); delete resp; return false;
    }
    line_num++;
    if(stringGetLine(fp, line, MAX_LINE_LEN-1) == EOF)
    {
	GError::setMessage("format error, line %d: %s", line_num, file.c_str());
	fclose(fp); delete resp; return false;
    }

    if(!resp->type.compare("paz"))
    {
	if( !read_tgPaz(fp, resp, &line_num) ) {
	    fclose(fp); delete resp; return false;
	}
    }
    else if(!resp->type.compare("fap"))
    {
	if( !read_tgFap(fp, resp, &line_num) ) {
	    fclose(fp); delete resp; return false;
	}
    }
    fclose(fp);
    responses.push_back(resp);
    return true;
}

/** Read a Teledyne Geotech formatted PAZ response
 */
bool ResponseFile::read_tgPaz(FILE *fp, Response *resp, int *line_num)
{
    int i;
    char line[MAX_LINE_LEN];

    resp->a0 = 1.;
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
	|| sscanf(line, "%d", &resp->npoles) != 1)
    {
	GError::setMessage("format error line %d of %s",*line_num,file.c_str());
	return false;
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
	|| sscanf(line, "%d", &resp->nzeros) != 1)
    {
	GError::setMessage("format error line %d of %s",*line_num,file.c_str());
	return false;
    }
    if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF
	|| sscanf(line, "%le", &resp->a0) != 1)
    {
	GError::setMessage("format error line %d of %s",*line_num,file.c_str());
	return false;
    }
    MALLOC(resp->pole, resp->npoles, FComplex);
    MALLOC(resp->zero, resp->nzeros, FComplex);
    for(i = 0; i < resp->npoles; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%e %e", &resp->pole[i].re, &resp->pole[i].im) != 2)
	{
	    GError::setMessage("format error line %d of %s",
				*line_num, file.c_str());
	    return false;
	}
    }
    for(i = 0; i < resp->nzeros; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%e %e", &resp->zero[i].re, &resp->zero[i].im) != 2)
	{
	    GError::setMessage("format error line %d of %s",
				*line_num, file.c_str());
	    return false;
	}
    }
    return true;
}

/** Read a Teledyne Geotech formatted FAP response
 */
bool ResponseFile::read_tgFap(FILE *fp, Response *resp, int *line_num)
{
    int i;
    char line[MAX_LINE_LEN], ptype[4];
    float asymlo, asymhi, fnorm;
    double degrees_to_radians;

    if(!strncasecmp(line+15, "deg", 3)) {
	strncpy(ptype, "deg", 4);
    }
    else {
	strncpy(ptype, "rad", 4);
    }
    while(stringGetLine(fp, line, MAX_LINE_LEN-1) != EOF)
    {
	(*line_num)++;
	if(line[0] != '#') break;
	if(!strncasecmp(line, "# RESPONSE TYPE:", 16))
	{
	    line[16+44] = '\0';
	    resp->author.assign(line+16, 45);
	}
    }
    if(sscanf(line, "%d %e %e %e",&resp->nfap,&asymlo,&asymhi,&fnorm) != 4)
    {
	GError::setMessage("format error line %d of %s",*line_num,file.c_str());
	return false;
    }
    MALLOC(resp->fap_f, resp->nfap, float);
    MALLOC(resp->fap_a, resp->nfap, float);
    MALLOC(resp->fap_p, resp->nfap, float);

    for(i = 0; i < resp->nfap; i++)
    {
	if(getNextLine(fp, line, MAX_LINE_LEN-1, line_num) == EOF ||
	    sscanf(line, "%e %e %e", &resp->fap_f[i], &resp->fap_a[i],
		&resp->fap_p[i]) != 3)
	{
	    GError::setMessage("format error line %d of %s",
				*line_num, file.c_str());
	    return false;
	}
    }
    degrees_to_radians = M_PI/180.;
    if(!strcmp(ptype, "deg"))
    {
	for(i = 0; i < resp->nfap; i++)
	{
	    resp->fap_p[i] *= degrees_to_radians;
	}
    }
    return true;
}

static int
getNextLine(FILE *fp, char *line, int n, int *line_num)
{
    int i, m, ret;

    /* call stringGetLine, skip all blank lines and lines starting with '#'
     */

    for(;;)
    {
	(*line_num)++;
	ret = stringGetLine(fp, line, n);
	m = strlen(line);
	if(m <= 0)
	{
	    if(ret == EOF) break;
	}
	else {
	for(i = 0; i < m && isspace((int)line[i]); i++);
	if(i < m && line[i] != '#' && line[i] != '(') break;
/*
	for(i = 0; i < m && line[i] == ' '; i++);
	if(i < m) break;
*/
	}
    }
    return(ret);
}

/** Create a ResponseFile object for the input file. All responses are read
 *  from the file in either CSS or GSE2.0 format. Use this function instead of
 *  the ResponseFile constructor in situations where the same file could be
 *  read multiple times. This function will not read the same file twice, but
 *  instead will return the ResponseFile object for a file that has previously
 *  been read. The ResponseFile constructor will always read the file each
 *  time it is used.
 *  @param[in] filename a CSS formatted or GSE2.0 formatted response file.
 *  @param[in] warn if true, print a warning message if this function fails.
 *  @returns a ResponseFile object or NULL if the file could not be parsed.
 */
ResponseFile * ResponseFile::readFile(const string &filename, bool warn)
{
    ResponseFile *rf;

    for(int i = 0; i < (int)resp_files.size(); i++)
    {
	if(!resp_files[i]->file.compare(filename)) {
	    return resp_files[i];
	}
    }
    rf = new ResponseFile(filename);
    if(rf->ioError()) {
	if(warn) fprintf(stderr, "%s\n", GError::getMessage());
	delete rf;
	rf = NULL;
    }
    else {
	resp_files.push_back(rf);
    }
    return rf;
}
