/** \file ConvolveData.cpp
 *  \brief Defines class ConvolveData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <strings.h>
#include <iostream>
#include <sstream>
#include <math.h>

#include "ConvolveData.h"
#include "gobject++/GTimeSeries.h"
#include "Response.h"
#include "ResponseFile.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

/** Constructor for one or more instrument responses. Each response is
 *  sequentially convolved or deconvolved with the data.
 *  @param[in] operation_direction The direction of the operation: -1 to
 *	deconvolve, 1 to convolve.
 *  @param[in] resp A vector of Response objects.
 *  @param[in] instrument_type The instrument type (instype).
 *  @param[in] flow Apply a cosine taper from 0 to frequency flo.
 *  @param[in] fhigh Apply a cosine taper from frequency fhi to the nyquist
 *	frequency.
 *  @param[in] amp_cutoff low amplitude fraction cutoff for deconvolution.
 *  @param[in] calib normalize the response amplitude to calib at period calper
 *  @param[in] calper normalize the response amplitude to calib at period calper
 *  @param[in] remove_fir_time_shift Remove any fir response time shift.
 */
ConvolveData::ConvolveData(int operation_direction, vector<Response *> &resp,
		const string &instrument_type, double flow, double fhigh,
		double amp_cutoff, double calib, double calper,
		bool remove_fir_time_shift) : DataMethod("ConvolveData"),
	responses(), direction(operation_direction), flo(flow), fhi(fhigh),
	cutoff(amp_cutoff), ncalib(calib), ncalper(calper>0. ? calper:1.),
	remove_time_shift(remove_fir_time_shift)
{
    for(int i = 0; i < (int)resp.size(); i++) {
	responses.push_back(resp[i]);
	resp[i]->addOwner(this);
    }
    strncpy(instype, instrument_type.c_str(), 6);
    instype[6] = '\0';
}

/** Constructor for one instrument response. The response is convolved or
 *  deconvolved with the data.
 *  @param[in] operation_direction The direction of the operation: -1 to
 *	deconvolve, 1 to convolve.
 *  @param[in] resp A Response object.
 *  @param[in] instrument_type The instrument type (instype).
 *  @param[in] flow Apply a cosine taper from 0 to frequency flo.
 *  @param[in] fhigh Apply a cosine taper from frequency fhi to the nyquist
 *	frequency.
 *  @param[in] amp_cutoff low amplitude fraction cutoff for deconvolution.
 *  @param[in] calib normalize the response amplitude to calib at period calper
 *  @param[in] calper normalize the response amplitude to calib at period calper
 *  @param[in] remove_fir_time_shift Remove any fir response time shift.
 */
ConvolveData::ConvolveData(int operation_direction, Response *resp,
		const string &instrument_type, double flow, double fhigh,
		double amp_cutoff, double calib, double calper,
		bool remove_fir_time_shift) : DataMethod("ConvolveData"),
	responses(), direction(operation_direction), flo(flow), fhi(fhigh),
	cutoff(amp_cutoff), ncalib(calib), ncalper(calper>0. ? calper:1.),
	remove_time_shift(remove_fir_time_shift)
{
    responses.push_back(resp);
    resp->addOwner(this);
    strncpy(instype, instrument_type.c_str(), 6);
    instype[6] = '\0';
}

Gobject * ConvolveData::clone()
{
    return (Gobject *) new ConvolveData(direction, responses, instype,
			flo, fhi, cutoff, ncalib, ncalper, remove_time_shift);
}

/** Destructor. */
ConvolveData::~ConvolveData(void)
{
    for(int i = 0; i < (int)responses.size(); i++) {
	responses[i]->removeOwner(this);
    }
}

bool ConvolveData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0 || (int)responses.size() <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "ConvolveData.apply: ts=NULL.");
	return false;
    }
    for(int i = 0; i < n; i++)
    {
	applyMethod(ts[i]);
    }
    return true;
}

bool ConvolveData::applyMethod(GTimeSeries *ts)
{
    for(int i = 0; i < ts->size(); i++) {
	GSegment *s = ts->segment(i);
	float dt = s->tdel();

	if(! Response::convolve(&responses, direction, s->data, s->length(),
			dt, flo, fhi, cutoff, ncalib, ncalper,
			remove_time_shift) )
	{
	    logErrorMsg(LOG_WARNING, "ConvolveData: failed.");
	    break;
	}
	s->setCalibration(1., -1.);
    }
    ts->setInstype(instype);
    return true;
}

void ConvolveData::applyToSegment(GSegment *s)
{
    float dt = s->tdel();

    if(! Response::convolve(&responses, direction, s->data, s->length(),
			dt, flo, fhi, cutoff, ncalib, ncalper,
			remove_time_shift) )
    {
	logErrorMsg(LOG_WARNING, "ConvolveData: failed.");
    }
    s->setCalibration(1., -1.);
}

const char * ConvolveData::toString(void)
{
    char s_fhi[20], stages[225], c[500];

    if(fhi > 0.) {
	snprintf(s_fhi, 20, "%.2f", fhi);
    }
    else {
	strncpy(s_fhi, "nyquist", 20);
    }
    stages[0] = '\0';
    for(int i = 0; i < (int)responses.size() && i < 20; i++) {
	char buf[10];
	snprintf(buf, 10, "%d", responses[i]->stage);
	strcat(stages, buf);
	if(i < (int)responses.size()-1) strcat(stages, ",");
    }
    if(direction == 1) {
	snprintf(c, sizeof(c),
		"Convolve: %s seq_no=%s flo=%.2f fhi=%s calib=%.6f calper=%.4f remove_time_shift=%d",
		instype, stages, flo, s_fhi, ncalib, ncalper, remove_time_shift);
    }
    else {
	snprintf(c, sizeof(c),
		"Deconvolve: %s seq_no=%s flo=%.2f fhi=%s amp_cutoff=%2g calib=%.6f calper=%.4f remove_time_shift=%d",
		instype, stages, flo, s_fhi, cutoff, ncalib, ncalper, remove_time_shift);
    }
    string_rep.assign(c);
    return string_rep.c_str();
}

/** Constructor with character string argument. The format of the string is
 *  \code
	"Convolve: %s seq_no=%s flo=%.2f fhi=%s", instype, stages, flo, fhi);
    \endcode
 *  where the first token is either "Convolve:" or "Deconvolve:", and the
 *  stages string contains the comma delimited sequence numbers of the
 *  cascaded response members. For example, "1,3,4". Or the stages string
 *  can be set to "all" to indicate all cascaded members of the response. The
 *  fhi parameter can be either a number or "nyquist", which indicates a high
 *  pass filter.
 *  @param s the arguments as a character string.
 *  @throws GERROR_INVALID_ARGS if a parameter is not found or cannot be parsed,
 *   or the instrument responses cannot be obtained.
 */
ConvolveData::ConvolveData(const string &str) throw(int) : DataMethod("ConvolveData"),
	responses(), direction(1), flo(0.), fhi(0.), cutoff(0.), ncalib(1.),
	ncalper(1.), remove_time_shift(true)
{
    char *c, seq_no[100];
    const char *s;
    int i, num_resp;
    CssInstrumentClass *ins;
    cvector<CssInstrumentClass> v;
    ResponseFile *rf;

    memset(instype, 0, sizeof(instype));
    s = str.c_str();

    if(!strncmp(s, "Deconvolve:", 11)) {
	direction = -1;
	if(sscanf(s+11, "%5s", instype) != 1) return;
    }
    else if(!strncmp(s, "Convolve:", 9)) {
	direction = 1;
	if(sscanf(s+9, "%5s", instype) != 1) return;
    }
    else {
	GError::setMessage(
		"ConvolveData constructor: missing direction indicator.");
	throw GERROR_INVALID_ARGS;
    }
    if((c = strstr((char *)s, "seq_no=")) == NULL
		|| sscanf(c+7, "%s", seq_no) != 1)
    {
	GError::setMessage("ConvolveData constructor: missing seq_no.");
	throw GERROR_INVALID_ARGS;
    }
    if((c = strstr((char *)s, "flo=")) == NULL || sscanf(c+4, "%lf", &flo) != 1)
    {
	GError::setMessage("ConvolveData constructor: missing flo.");
	throw GERROR_INVALID_ARGS;
    }
    if((c = strstr((char *)s, "fhi=")) == NULL) {
	GError::setMessage("ConvolveData constructor: missing fhi.");
	throw GERROR_INVALID_ARGS;
    }
    if(!strcmp(c+4, "nyquist")) {
	fhi = 0.;
    }
    else if(sscanf(c+4, "%lf", &fhi) != 1) {
	GError::setMessage("ConvolveData constructor: invalid fhi.");
	throw GERROR_INVALID_ARGS;
    }

    if(direction == -1 && ((c = strstr((char *)s, "amp_cutoff=")) == NULL
	|| sscanf(c+11, "%lf", &cutoff) != 1)) {
	GError::setMessage("ConvolveData constructor: missing amp_cutoff.");
	throw GERROR_INVALID_ARGS;
    }
    if((c=strstr((char *)s,"calib=")) == NULL || sscanf(c+4,"%lf",&ncalib) != 1)
    {
	GError::setMessage("ConvolveData constructor: missing calib.");
	throw GERROR_INVALID_ARGS;
    }
    if((c=strstr((char *)s,"calper="))==NULL || sscanf(c+4,"%lf",&ncalper) != 1)
    {
	GError::setMessage("ConvolveData constructor: missing calper.");
	throw GERROR_INVALID_ARGS;
    }
    if((c = strstr((char *)s, "remove_time_shift=")) != NULL &&
		sscanf(c+18, "%d", &i) == 1) {
	remove_time_shift = (i == 1) ? true : false;
    }

    ResponseFile::getInstruments(v);
    ins = NULL;
    for(i = 0; i < v.size(); i++) {
	if(!strcmp(v[i]->instype, instype)) {
	    ins = v[i];
	    break;
	}
    }
    if( !ins ) {
	GError::setMessage("Cannot find instrument: %s", instype);
	throw GERROR_INVALID_ARGS;
    }

    if( !(rf = ResponseFile::readFile(ins, false)) ) {
	throw GERROR_RESPONSE_FILE_ERROR;
    }
    num_resp = (int)rf->responses.size();

    if(!strcasecmp(seq_no, "all")) {
	for(i = 0; i < num_resp; i++) {
	    responses.push_back(rf->responses[i]);
	    rf->responses[i]->addOwner(this);
	}
    }
    else {
	char *last;
	char *tok = seq_no;
	while((c = strtok_r(tok, ",", &last)) != NULL) {
	    tok = NULL;
	    if(sscanf(c, "%d", &i) == 1 && i < num_resp) {
		responses.push_back(rf->responses[i]);
		rf->responses[i]->addOwner(this);
	    }
	    else {
		GError::setMessage("Invalid seq_no parameter.");
		throw GERROR_INVALID_ARGS;
	    }
	}
    }
}
