/*	SccsId:	%W%	%G%	*/
/*
 *	Response structure defined.
 */
#ifndef _RESPONSE_STRUCT_
#define _RESPONSE_STRUCT_

#include "gobject/GObject.h"
#include "gobject/Vector.h"
#include "gobject/TimeSeries.h"
#include "libtime.h"
#include "css/wfdisc.h"
#include "css/instrument.h"

typedef struct complex
{
	float re;
	float im;
} ComPlex;

typedef struct
{
	char	sta[6];
	char	chan[4];
	char	auxid[5];
	char	instype[7];
	double	calib;
	double	calper;
	double	samprat;
	DateTime ondate;
	DateTime offdate;
} GSE_CAL;

typedef struct
{
	GObjectPart	core;

	char		source[13];	/* "theoretical" or "measured"	*/
	int		seq_num;	/* sequence number		*/
	char		des[13];	/* description			*/
	char		type[7];	/* fap, paz, or fir		*/
	char		units[2];	/* d, v, or a			*/
	char		author[45];	/* author or information source	*/
	double		a0;	/* normalization for paz / gain for fir */
	int		npoles;		/* number of poles for paz type */
	int		nzeros;		/* number of zeros for paz type */
	ComPlex		*pole;		/* poles for paz type		*/
	ComPlex		*pole_err;	/* poles for paz type		*/
	ComPlex		*zero;		/* zeros for paz type		*/
	ComPlex		*zero_err;	/* zeros for paz type		*/
	int		nfap;		/* number of fap triplets	*/
	float		*f;		/* frequency for fap type	*/
	float		*a;		/* amplitude for fap type	*/
	float		*p;		/* phase for fap type		*/
	float		*a_error;	/* amplitude error for fap type */
	float		*p_error;	/* phase error for fap type	*/
	float		samprate;	/* samples/sec for fir type	*/
	int		num_n;		/* # numerator coeffs. for fir	*/
	int		num_d;		/* # denominator coeffs. for fir*/
	float		*n;		/* numerator for fir type	*/
	float		*n_error;	/* numerator error for fir type	*/
	float		*d;		/* denominator for fir type	*/
	float		*d_error;	/* denominator error for fir 	*/

	GSE_CAL		*cal;		/* associated CAL for GSE format */
} _Response, *Response;

typedef struct
{
	GObjectPart	core;

	char		*file;		/* file name		*/
	Vector		responses;	/* all responses in file */
} _ResponseFile, *ResponseFile;

typedef struct
{
	int		file_index;
	INSTRUMENT30	in;
} _Instrument, *Instrument;


/* ****** response.c ********/
ResponseFile new_ResponseFile(const char *file);
Response new_Response(void);
const char *Response_getErrMsg(void);

int Response_convolve(Vector resp, int direction, float *data, int npts,
			double dt, double flo, double fhi, double calib,
			double calper, int remove_fir_time_shift,
			const char **err_msg);
void Response_paz(double omega, ComPlex *pole, int npoles, ComPlex *zero,
			int nzeros, double a0, double *real, double *imag);
int Response_fap(Response resp, double lofreq, double hifreq, int nf,
			double *real, double *imag);
int Response_fir(Response resp, double lofreq, double hifreq, int nf,
			double *real, double *imag);
int Response_compute(Vector resp, double lofreq, double hifreq, double calib,
			double calper, bool remove_fir_time_shift,
			int nf, double *real, double *imag);

void Response_unwrap(float *p, int n);
int Response_modified(TimeSeries ts);


/* ****** response.c ********/
int Response_getFile(TimeSeries ts, char *insname, int insname_len,
			char *file, int file_len, const char **err_msg);
int Response_getInstrument(WFDISC30 *wf, const char *path, const char *prefix,
			char *insname, int insname_len, char *file,
			int file_len, const char **err_msg);


/* ****** responseFiles.c ********/
void LoadInstrumentTable(const char *file);
void ReadAllInstruments(void);
ResponseFile AddResponse(const char *filename, bool warn);
int responseReadFiles(int nfiles, char **files, Instrument *last_instrument);
Instrument getInstrument(ResponseFile rf);
ResponseFile getResponseFile(Instrument n, bool warn);
Vector GetResponse(TimeSeries ts, bool print_err);
bool new_missing_rsp(int sta, int chan);
bool cts2nms(TimeSeries ts, double ampcts, double period, double *ampnms);
bool nms2cts(TimeSeries ts, double ampnms, double period, double *ampcts);
int RespGetInstruments(Instrument **instruments);
char *RespGetInstrumentPath(int file_index);

#endif
