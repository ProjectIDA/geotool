#ifndef _LIBCSSIO_H_
#define	_LIBCSSIO_H_

#include <stdio.h>

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */

#include "stdarg.h"

#include "css/cssAll.h"
#include "gobject/cssObjects.h"

/* ****** cssio/CheckWfdisc.c ********/
int cssioCheckWfdisc(WFDISC30 *wf, const char *wdir);

/* ****** cssio/decomp.c ********/
int cssioDecomp(void *data_in, int data_in_len, void *data_out, int npts,
		const char *datatype, int outtype);
int cssioDecompS4(int npts, int *data_in, void *data_out, int outtype);
int cssioDecompT4(int npts, float *data_in, float *data_out);
int cssioDecompS2(int npts, short *data_in, void *data_out, int outtype);
int cssioDecompG2(int npts, unsigned char *data_in, void *data_out,int outtype);
int cssioDecompI4(int npts, int *data_in, void *data_out, int outtype);
int cssioDecompF4(int npts, float *data_in, float *data_out);
int cssioDecompS3(int npts, int *data_in, void *data_out, int outtype);
int cssioDecompC24(int start, int npts, char *data_in, void *data_out,
		const char *datatype, int outtype);
int cssioDecompCa(unsigned char *data_in, int data_in_len, void *data_out,
		int npts, int outtype);



/* ****** cssio/dcpress.c ********/
int longdcpress(long numin, const char *in, long *out);
void rmfdif(int num, long *in, long *out);


/* ****** cssio/g2tofloat.c ********/
void g2tofloat(unsigned char *from, void *to, int outtype, int num);


/* ****** cssio/readTables.c ********/
int cssioReadWfdisc		(FILE *fp, WFDISC30 	    *w);
int cssioWriteWfdisc		(FILE *fp, WFDISC30 	    *w);
int cssioReadArrival		(FILE *fp, ARRIVAL30 	    *w);
int cssioWriteArrival		(FILE *fp, ARRIVAL30 	    *w);
int cssioReadOrigin		(FILE *fp, ORIGIN30 	    *w);
int cssioWriteOrigin		(FILE *fp, ORIGIN30 	    *w);
int cssioReadOrigerr		(FILE *fp, ORIGERR30 	    *w);
int cssioWriteOrigerr		(FILE *fp, ORIGERR30 	    *w);
int cssioReadLastid		(FILE *fp, LASTID30 	    *w);
int cssioWriteLastid		(FILE *fp, LASTID30 	    *w);
int cssioReadSensor		(FILE *fp, SENSOR30 	    *w);
int cssioWriteSensor		(FILE *fp, SENSOR30 	    *w);
int cssioReadInstrument		(FILE *fp, INSTRUMENT30     *w);
int cssioWriteInstrument	(FILE *fp, INSTRUMENT30     *w);
int cssioReadSitechan		(FILE *fp, SITECHAN30	    *w);
int cssioWriteSitechan		(FILE *fp, SITECHAN30	    *w);
int cssioReadSite		(FILE *fp, SITE30	    *w);
int cssioWriteSite		(FILE *fp, SITE30	    *w);
int cssioReadWftag		(FILE *fp, WFTAG30 	    *w);
int cssioWriteWftag		(FILE *fp, WFTAG30 	    *w);
int cssioReadXtag		(FILE *fp, XTAG30 	    *w);
int cssioWriteXtag		(FILE *fp, XTAG30 	    *w);
int cssioReadFsdisc		(FILE *fp, FSDISC30 	    *w);
int cssioReadFsave		(FILE *fp, FSAVE30 	    *w);
int cssioReadFsrecipe		(FILE *fp, FSRECIPE30 	    *w);
int cssioReadFstag		(FILE *fp, FSTAG30 	    *w);
int cssioReadPick		(FILE *fp, PICK 	    *w);
int cssioWritePick		(FILE *fp, PICK 	    *w);
int cssioReadFilter		(FILE *fp, FILTER 	    *w);
int cssioWriteFilter		(FILE *fp, FILTER 	    *w);
int cssioReadAssoc		(FILE *fp, ASSOC30 	    *w);
int cssioWriteAssoc		(FILE *fp, ASSOC30 	    *w);
int cssioReadStassoc		(FILE *fp, STASSOC30	    *w);
int cssioWriteStassoc		(FILE *fp, STASSOC30	    *w);
int cssioReadAffiliation	(FILE *fp, AFFILIATION30    *w);
int cssioWriteAffiliation	(FILE *fp, AFFILIATION30    *w);
int cssioReadHydrofeatures	(FILE *fp, HYDRO_FEATURES30 *w);
int cssioWriteHydrofeatures	(FILE *fp, HYDRO_FEATURES30 *w);
int cssioReadInfrafeatures	(FILE *fp, INFRA_FEATURES30 *w);
int cssioWriteInfrafeatures	(FILE *fp, INFRA_FEATURES30 *w);
int cssioReadStamag		(FILE *fp, STAMAG30	    *w);
int cssioWriteStamag		(FILE *fp, STAMAG30	    *w);
int cssioReadNetmag		(FILE *fp, NETMAG30	    *w);
int cssioWriteNetmag		(FILE *fp, NETMAG30	    *w);
int cssioReadAmpdescript	(FILE *fp, AMPDESCRIPT30    *w);
int cssioWriteAmpdescript	(FILE *fp, AMPDESCRIPT30    *w);
int cssioReadAmplitude		(FILE *fp, AMPLITUDE30	    *w);
int cssioWriteAmplitude		(FILE *fp, AMPLITUDE30      *w);
int cssioReadSpdisc		(FILE *fp, SPDISC30	    *w);
int cssioWriteSpdisc		(FILE *fp, SPDISC30	    *w);
int cssioReadDervdisc		(FILE *fp, DERVDISC30	    *w);
int cssioWriteDervdisc		(FILE *fp, DERVDISC30	    *w);
int cssioReadPmccfeatures	(FILE *fp, PMCC_FEATURES30  *w);
int cssioWritePmccfeatures	(FILE *fp, PMCC_FEATURES30  *w);
int cssioReadPmccrecipe		(FILE *fp, PMCC_RECIPE30    *w);
int cssioWritePmccrecipe	(FILE *fp, PMCC_RECIPE30    *w);


/* ****** cssio/read_dotw.c ********/
int
#ifdef HAVE_LIBZ
cssioReadDotw(gzFile zfd, int foff, int start, int npts, void *data,
			const char *datatype, int outtype, int rd_len);
#else /* HAVE_LIBZ */
cssioReadDotw(int fd, int foff, int start, int npts, void *data,
			const char *datatype, int outtype, int rd_len);
#endif /* HAVE_LIBZ */
int
#ifdef HAVE_LIBZ
cssioReadDotwDeci(gzFile zfd, int foff, int start, int npts, float *data,
			char *datatype, int outtype, int num_per_interval);
#else /* HAVE_LIBZ */
cssioReadDotwDeci(int fd, int foff, int start, int npts, float *data,
			char *datatype, int outtype, int num_per_interval);
#endif /* HAVE_LIBZ */


/* ****** cssio/cssio.c ********/
int cssioReadData(WFDISC30 *wfd, const char *working_dir, double start_time,
			double end_time, int pts_wanted, int *npts,
			double *tbeg, double *tdel, float **pdata);
const char *cssioGetErrorMsg(void);
#ifdef HAVE_STDARG_H
void cssioSetErrorMsg(const char *format, ...);
#else
void cssioSetErrorMsg();
#endif
const char *cssioGetTmpPrefix(const char *dir, const char *prefix);
void cssioAddTmpPrefix(const char *name);
void cssioDeleteTmpPrefix(const char *name);
void cssioDeleteAllTmp(void);

int cssioReadSpectra(CssFsdisc fs, const char *working_dir, int *npts, float **pdata);

/* ****** cssio/s3s4.c ********/
void s3tos4(void *byte, register int n);
void s4tos3(void *byte, register int n);

#define FLOAT_DATA	2
#define INT_DATA	3


#endif	/* _LIBCSSIO_H_ */
