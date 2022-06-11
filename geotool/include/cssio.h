#ifndef _CSSIO_H_
#define	_CSSIO_H_

#include <stdio.h>

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */

#include "stdarg.h"

#include "gobject++/CssTables.h"

/* ****** cssio/CheckWfdisc.c ********/
int cssioCheckWfdisc(CssWfdiscClass *wf, const string &wdir);

/* ****** cssio/decomp.c ********/
int cssioDecomp(void *data_in, int data_in_size, void *data_out, int npts, const char *datatype, int outtype);
int cssioDecompInt4(int npts, int *data_in, void *data_out, int outtype, bool swap_bytes);
int cssioDecompS3(int npts, int *data_in, void *data_out, int outtype);
int cssioDecompI3(int npts, int *data_in, void *data_out, int outtype);
int cssioDecompInt2(int npts, short *data_in, void *data_out, int outtype, bool swap_bytes);
int cssioDecompFloat(int npts, float *data_in, float *data_out, bool swap_bytes);
int cssioDecompG2(int npts, unsigned char *data_in, void *data_out, int outtype);
int cssioDecompV4(int npts, float *data_in, float *data_out);
int cssioDecompC24(int start, int npts, char *data_in, void *data_out, const char *datatype, int outtype);
int cssioDecompCa(unsigned char *data_in, int data_in_len, void *data_out, int npts, int outtype);


/* ****** cssio/dcpress.c ********/
int longdcpress(long numin, const char *in, long *out);
void rmfdif(int num, long *in, long *out);


/* ****** cssio/g2tofloat.c ********/
void g2tofloat(unsigned char *from, void *to, int outtype, int num);


/* ****** cssio/read_dotw.c ********/
ssize_t
#ifdef HAVE_LIBZ
cssioReadDotw(const char *path, gzFile zfd, int fd, off_t foff, ssize_t start,
	      ssize_t npts, void *data, const char *datatype, int outtype,
	      ssize_t rd_len);
#else /* HAVE_LIBZ */
cssioReadDotw(const char *path, int fd, off_t foff, ssize_t start, ssize_t npts,
	      void *data, const char *datatype, int outtype, ssize_t rd_len);
#endif /* HAVE_LIBZ */
ssize_t
#ifdef HAVE_LIBZ
cssioReadDotwDeci(const char *path, gzFile zfd, int fd, off_t foff, ssize_t start,
		  ssize_t npts, float *data, const char *datatype, int outtype,
		  ssize_t num_per_interval);
#else /* HAVE_LIBZ */
cssioReadDotwDeci(const char *path, int fd, off_t foff, ssize_t start,
		  ssize_t npts, float *data, const char *datatype, int outtype,
		  ssize_t num_per_interval);
#endif /* HAVE_LIBZ */


/* ****** cssio/cssio.c ********/
int cssioReadData(CssWfdiscClass *wfd, const string &working_dir, double start_time,
			double end_time, int pts_wanted, int *npts,
			double *tbeg, double *tdel, float **pdata);
const char *cssioGetErrorMsg(void);
#ifdef HAVE_STDARG_H
void cssioSetErrorMsg(const char *format, ...);
#else
void cssioSetErrorMsg();
#endif
const char *cssioGetTmpPrefix(const string &dir, const string &prefix);
void cssioAddTmpPrefix(const string &name);
void cssioDeleteTmpPrefix(const string &name);
void cssioDeleteAllTmp(void);

/* ****** cssio/s3s4.c ********/
void s3tos4(void *byte, register int n);
void s4tos3(void *byte, register int n);

#define FLOAT_DATA	2
#define INT_DATA	3


#endif	/* _CSSIO_H_ */
