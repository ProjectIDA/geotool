/*
 * NAME
 *      routines for reading css "dotw" files.
 *
 * AUTHOR
 *      I. Henson
 */

/**
 *  Routines for reading and decompressing binary waveform data.
 */

#include "config.h"
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <vector>

#include "cssio.h"
extern "C" {
#include "logErrorMsg.h"
#include "canada_compress.h"
}

/**
 * @private
 */
typedef struct ieee_single {
        u_int   sign    : 1;
        u_int   exp     : 8;
        u_int   mantissa: 23;
} IEEE;

/**
 * @private
 */
typedef struct vax_single {
        u_int   mantissa2 : 16;
        u_int   sign      : 1;
        u_int   exp       : 8;
        u_int   mantissa1 : 7;
} VAX;

static void itoi(char *from, char *to, int num);
static int vftoif(VAX *from, IEEE *to, int num);
static ssize_t decompress_e(unsigned long *in, void *data, int outtype,
			    ssize_t start, ssize_t npts);
static void b_lshift(unsigned long *input, int nwords, int size);
static long b_lval(unsigned long input, int size);


static vector<int> files;

static void errorRead(const char *path, int seeking)
{
    int qfile = stringToQuark(path);

    // only warn one time per file
    for(int i = 0; i < (int)files.size(); i++) {
	if(files[i] == qfile) return;
    }
    files.push_back(qfile);

    if (seeking) {
	if(errno == 0) { 
	    fprintf(stderr, "Seek failed on file %s\n", path);
	} 
	else if(errno > 0) {
	    fprintf(stderr, "Seek failed on file %s: %s\n",
		    path, strerror(errno)); 
	}
    }
    else {
	if(errno == 0) { 
	    fprintf(stderr, "Read failed on file %s\n", path);
	} 
	else if(errno > 0) {
	    fprintf(stderr, "Read failed on file %s: %s\n",
		    path, strerror(errno)); 
	}
    }
}

#ifndef False
#define False 0
#endif

#ifndef True
#define True  1
#endif

#ifdef COMPRESSED_BUF_SIZE
#undef UNCOMPRESSED_BUF_SIZE
#endif
#define	COMPRESSED_BUF_SIZE 2048

#ifdef HAVE_LIBZ
ssize_t
cssioReadDotw(const char *path, gzFile zfd, int fd, off_t foff, ssize_t start,
	      ssize_t npts, void *data, const char *datatype, int outtype,
	      ssize_t rd_len)
#else /* HAVE_LIBZ */
/**
 * Read a waveform dotw file.
 * @param fd A file descriptor for an open data file.
 * @param foff The file offset at which to star the read.
 * @param start The first data value to return ( >= 0 ).
 * @param npts The number of data values to return.
 * @param data An array to receive the data values.
 * @param datatype The data compression type.
 * @param outtype The desired data type of the output data values (FLOAT_DATA \
 * 	or INT_DATA).
 * @param rd_len The read length (bytes) for "ca" datatype.
 */
ssize_t
cssioReadDotw(const char *path, int fd, off_t foff, ssize_t start, ssize_t npts,
	      void *data, const char *datatype, int outtype, ssize_t rd_len)
#endif /* HAVE_LIBZ */
{
	ssize_t k = 0, m = 0;
	ssize_t	lr, ns, data_size;
	ssize_t e_ret;
	float	*fdata = NULL;
	int	*idata;
	char	*cdata = NULL;
	off_t	offset = 0;
        ssize_t  sample;

	cssioSetErrorMsg("");

	/* printf("DEBUG: datatype = %s\n", datatype); */

	if(!strcmp(datatype, "s4") || !strcmp(datatype, "i4"))
	{
	    data_size = sizeof(int32_t);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		k = (ssize_t)gzread(zfd, data, data_size * npts);
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		k = read(fd, data, data_size * npts);
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    k = read(fd, data, data_size * npts);
#endif /* HAVE_LIBZ */
	    if (k == -1) {
	      errorRead(path, 0);
	      npts = 0;
	    }
	    else {
	      npts = k/data_size;
	    }
	    return cssioDecomp(data, 4*sizeof(float), data, npts, datatype, FLOAT_DATA);
	}
	else if(!strcmp(datatype, "t4") || !strcmp(datatype, "f4"))
	{
	    if(outtype != FLOAT_DATA) {
		cssioSetErrorMsg("t4 or f4 data cannot be read as int");
		return(0);
	    }

	    data_size = sizeof(float);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		m = (ssize_t)gzread(zfd, data, data_size * npts);
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		m = read(fd, data, data_size * npts);
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    m = read(fd, data, data_size * npts);
#endif /* HAVE_LIBZ */
	    if (m == -1) {
	      errorRead(path, 0);
	      /*   printf("DEBUG: shortread npts=%ld m=%ld\n", npts, m); */
	      npts = 0;
	    }
	    else {
	      npts = m/data_size;
	    }
	    return cssioDecomp(data, 4*sizeof(float), data, npts, datatype, FLOAT_DATA);
	}
	else if(!strcmp(datatype, "s2") || !strcmp(datatype, "i2"))
	{
	    data_size = sizeof(short);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		k = (ssize_t)gzread(zfd, data, data_size * npts);
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
		k = read(fd, data, data_size * npts);
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    k = read(fd, data, data_size * npts);
#endif /* HAVE_LIBZ */
	    if (k == -1) {
	      errorRead(path, 0);
	      npts = 0;
	    }
	    else {
	      npts = k/data_size;
	    }
	    return cssioDecomp(data, 4*sizeof(float), data, npts, datatype, FLOAT_DATA);
	}
	else if(!strcmp(datatype, "g2"))
	{
	    data_size = sizeof(short);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if(gzseek(zfd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
		m = (ssize_t)gzread(zfd, data, data_size*npts);
	    }
	    else {
		if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
		m = read(fd, data, data_size*npts);
	    }
#else /* HAVE_LIBZ */
	    if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    m = read(fd, data, data_size*npts);
#endif /* HAVE_LIBZ */
	    if (m == -1) {
	      errorRead(path, 0);
	      /*   printf("DEBUG: shortread npts=%ld m=%ld\n", npts, m); */
	      npts = 0;
	    }
	    else {
	      npts = m/data_size;
	    }
	    return cssioDecomp(data, 4*sizeof(float), data, npts, datatype, FLOAT_DATA);
	}
	else if(!strcmp(datatype, "s3") || !strcmp(datatype, "i3"))
	{
	    data_size = 3;
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if(gzseek(zfd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
		m = (ssize_t)gzread(zfd, data, data_size * npts);
	    }
	    else {
		if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
		m = read(fd, data, data_size * npts);
	    }
#else /* HAVE_LIBZ */
	    if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    m = read(fd, data, data_size * npts);
#endif /* HAVE_LIBZ */	
	    if (m == -1) {
	      errorRead(path, 0);
	      npts = 0;
	    }
	    else {
	      npts = m/data_size;
	    }
	    return cssioDecomp(data, 4*sizeof(float), data, npts, datatype, FLOAT_DATA);
	}
        else if(!strcmp(datatype, "c2") || !strcmp(datatype, "c4"))
        {
	    data_size = sizeof(char);
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if(gzseek(zfd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    }
	    else {
		if(lseek(fd, foff, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if(lseek(fd, foff, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    cdata = (char *)malloc(data_size*(start+npts)*2);
	    if (cdata == NULL) {
		  logErrorMsg(LOG_ERR,
			      "cssioReadDotw: couldn't allocate buffer");
		  return(0);
            }

#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		k = (ssize_t)gzread(zfd, cdata, data_size * (start + npts));
	    }
	    else {
		k = read(fd, cdata, data_size * (start + npts));
	    }
#else /* HAVE_LIBZ */
            k = read(fd, cdata, data_size * (start + npts));
#endif /* HAVE_LIBZ */
	    if (k == -1) {
	      errorRead(path, 0);
	      npts = 0;
	    }
	    else {
	      npts = (k - start)/data_size;
	      if(npts <= 0) {
		Free(cdata);
		return(0);
	      }
	    }
	    npts = cssioDecompC24(start, npts, cdata, (float *)data, datatype, outtype);
	    Free(cdata);
	    return npts;
	}
        else if(!strcmp(datatype, "e1"))
	{
	    long buf[COMPRESSED_BUF_SIZE];
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, foff, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, foff, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, foff, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */

	    sample = 0;
	    k = 0;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
	      while((ssize_t)gzread(zfd, buf, 8) == 8)
	      {
		ssize_t nbytes;
		int16_t i16;
//		lr = (ssize_t) ((int16_t *) (&buf[0]))[0];
//		ns = (ssize_t) ((int16_t *) (&buf[0]))[1];
		memcpy(&i16, buf, sizeof(i16));
		lr = (ssize_t)i16;
		memcpy(&i16, buf+2, sizeof(i16));
		ns = (ssize_t)i16;

		if(sample + ns < start)
		{
		    /* skip to the next record */
		  if (gzseek(zfd, lr - 8, SEEK_CUR) == -1) {
		    break;
		  }
		  sample += ns;
		  continue;
		}
		nbytes = lr - 8;
		m = (ssize_t)gzread(zfd, &buf[2], nbytes);
		if (m == -1) {
		  errorRead(path, 0);
		  npts = k;
		}
		else if (m != nbytes) {
		  npts = k;
		  break;
		}
		else {
		  /* Nothing */
		}
		if (outtype == FLOAT_DATA) {
		    fdata = (float *)data;
		    e_ret = decompress_e((unsigned long *)buf, fdata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if (e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}
		else {
		    idata = (int *) data;
		    e_ret = decompress_e((unsigned long *)buf, idata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if(e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}

		ns = (ssize_t)e_ret;

		k += (start > sample) ? ns - (start-sample) : ns;
		sample += ns;

		if(k >= npts) break;
	      }
	    }
	    else
	    {
	      while(read(fd, buf, 8) == 8)
	      {
		ssize_t nbytes;
		int16_t i16;
//		lr = (ssize_t) ((int16_t *) (&buf[0]))[0];
//		ns = (ssize_t) ((int16_t *) (&buf[0]))[1];
		memcpy(&i16, buf, sizeof(i16));
		lr = (ssize_t)i16;
		memcpy(&i16, buf+2, sizeof(i16));
		ns = (ssize_t)i16;

		if(sample + ns < start)
		{
		  // skip to the next record
		  if (lseek(fd, lr - 8, SEEK_CUR) == -1) {
		    break;
		  }
		  sample += ns;
		  continue;
		}
		nbytes = lr - 8;
		m = read(fd, &buf[2], nbytes);
		if (m == -1) {
		  errorRead(path, 0);
		  npts = k;
		}
		else if (m != nbytes) {
		  npts = k;
		  break;
		}
		else {
		  // Nothing
		}
		if (outtype == FLOAT_DATA) {
		    fdata = (float *)data;
		    e_ret = decompress_e((unsigned long *)buf, fdata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if (e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}
		else {
		    idata = (int *) data;
		    e_ret = decompress_e((unsigned long *)buf, idata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if(e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}

		ns = (ssize_t)e_ret;

		k += (start > sample) ? ns - (start-sample) : ns;
		sample += ns;

		if(k >= npts) break;
	      }
	    }
#else /* HAVE_LIBZ */
	    while(read(fd, buf, 8) == 8)
	    {
		ssize_t nbytes;
		lr = (ssize_t) ((int16_t *) (&buf[0]))[0];
		ns = (ssize_t) ((int16_t *) (&buf[0]))[1];

		if(sample + ns < start)
		{
		  // skip to the next record
		  if (lseek(fd, lr - 8, SEEK_CUR) == -1) {
		    break;
		  }
		  sample += ns;
		  continue;
		}
		nbytes = lr - 8;
		m = read(fd, &buf[2], nbytes);
		if (m == -1) {
		  errorRead(path, 0);
		  npts = k;
		}
		else if (m != nbytes) {
		  npts = k;
		  break;
		}
		else {
		  // Nothing
		}
		if (outtype == FLOAT_DATA) {
		    fdata = (float *)data;
		    e_ret = decompress_e((unsigned long *)buf, fdata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if (e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}
		else {
		    idata = (int *) data;
		    e_ret = decompress_e((unsigned long *)buf, idata+k,
					 outtype, start-sample,
					 npts-k);
		    if (e_ret < 0) {
		      if(e_ret == -1) {
			cssioSetErrorMsg("e1 format decompress error.");
		      }
		      else {
			cssioSetErrorMsg("decompress_e: malloc error.");
		      }
		      npts = k;
		      break;
		    }
		}

		ns = (ssize_t)e_ret;

		k += (start > sample) ? ns - (start-sample) : ns;
		sample += ns;

		if(k >= npts) break;
	    }
#endif /* HAVE_LIBZ */
	}
	else if(!strcmp(datatype, "ca"))
	{
	    char *comp = NULL;

	    offset = foff;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, 0) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, 0) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, 0) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    if ((comp = (char *) malloc((size_t)rd_len+80)) == NULL) {
		logErrorMsg(LOG_ERR,
			    "cssioReadDotw: couldn't allocate temporary buffer");
		return(0);
	    }

	    if (rd_len > (ssize_t)(npts * sizeof(int)))
	    {
		cssioSetErrorMsg("rd_len > (npts * sizeof(int))!");
	    }
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if((k = gzread(zfd, comp, rd_len)) != rd_len)
		{
		    errorRead(path, 0);
		    npts = k;
		}
	    }
	    else {
		if((k = read(fd, comp, rd_len)) != rd_len)
		{
		    errorRead(path, 0);
		    npts = k;
		}
	    }
#else /* HAVE_LIBZ */
            if((k = read(fd, comp, rd_len)) != rd_len)
            {
	      errorRead(path, 0);
	      npts = k;
	    }
#endif /* HAVE_LIBZ */

	    npts = cssioDecompCa((unsigned char*)comp, rd_len, data, npts, outtype);

	    free(comp);
	    return npts;
	}
	/* unknown data type */
	else
	{
	    npts = -1;
	}

	return(npts);
}

#define	READ_BUF_SIZ 2048

#ifdef HAVE_LIBZ
ssize_t
cssioReadDotwDeci(const char *path, gzFile zfd, int fd, off_t foff, ssize_t start,
		  ssize_t npts, float *data, const char *datatype, int outtype,
		  ssize_t num_per_interval)
#else /* HAVE_LIBZ */
/**
 * Read a waveform dotw file and decimate the waveform as it is read. The
 * decimation is performed by keeping the smallest and largest values in
 * each internal of length <b>num_per_interval</b>.
 * @param fd A file descriptor for an open data file.
 * @param foff The file offset at which to star the read.
 * @param start The first data value to return ( >= 0 ).
 * @param npts The number of data values to return.
 * @param data An array to receive the data values.
 * @param datatype The data compression type.
 * @param outtype The desired data type of the output data values (FLOAT_DATA \
 * 	or INT_DATA).
 * @param num_per_interval The decimation interval length.
 */
ssize_t
cssioReadDotwDeci(const char *path, int fd, off_t foff, ssize_t start,
		  ssize_t npts, float *data, const char *datatype, int outtype,
		  ssize_t num_per_interval)
#endif /* HAVE_LIBZ */
{
	int	s4_data, xlo, xhi;
	int	buf[READ_BUF_SIZ];
	ssize_t  i, k, l, m, n, need, read_npts, read_bytes, data_size;
        off_t	offset;
	int16_t	*s2;
	int32_t	*s4;
	float	*t4;
	double	interval_hi, interval_lo;
	float	fl_tmp;

	cssioSetErrorMsg("");

	if (outtype != FLOAT_DATA)
	{
	    cssioSetErrorMsg("cssioReadDotwDeci: outtype must be FLOAT_DATA.");
	    return 0;
	}

	read_bytes = READ_BUF_SIZ * sizeof(int);

	interval_hi = -1.e+60;
	interval_lo =  1.e+60;

	if(!strcmp(datatype, "s4"))
	{
	    data_size = sizeof(int32_t);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/data_size;
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;
#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, buf, data_size * need);
		    }
		    else {
			n = read(fd, buf, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, buf, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else if (n != (need * data_size)) {
		      npts = k;
		      break;
		    }
		    i = 0;
		}
		fl_tmp  = (float)buf[i++];
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "t4"))
	{
	    t4 = (float *)buf;
	    data_size = sizeof(float);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/data_size;
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;

#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, t4, data_size * need);
		    }
		    else {
			n = read(fd, t4, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, t4, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else {
		      npts = k;
		      break;
		    }
		    i = 0;
		}
		fl_tmp  = t4[i++];
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "s2"))
	{
	    s2 = (int16_t *)buf;
	    data_size = sizeof(int16_t);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if(gzseek(zfd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    }
	    else {
		if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if(lseek(fd, offset, SEEK_SET) == -1){ errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/data_size;
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;

#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, s2, data_size * need);
		    }
		    else {
			n = read(fd, s2, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, s2, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    } 
		    else {
		      npts = k;
		      break;
		    }
		    i = 0;
		}
		fl_tmp  = (float)s2[i++];
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "g2"))
	{
	    t4 = (float *)buf;
	    data_size = sizeof(int16_t);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/sizeof(float);
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;
#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, buf, data_size * need);
		    }
		    else {
			n = read(fd, buf, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, buf, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else if (n != (data_size * need)) {
		      npts = k;
		      break;
		    }
		    g2tofloat((unsigned char *)buf, t4, outtype, n);
		    i = 0;
		}
		fl_tmp = t4[i++];
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "s3"))
	{
	    s4 = (int32_t *)buf;
	    data_size = 3;
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/sizeof(float);
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;
#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, s4, data_size * need);
		    }
		    else {
			n = read(fd, s4, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, s4, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else if (n != (data_size * need)) {
		      npts = k;
		      break;
		    }
		    s3tos4(s4, n);
		    i = 0;
		}
		fl_tmp = (float)s4[i++];
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "i4"))
	{
	    data_size = sizeof(int32_t);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/data_size;
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;

#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, buf, data_size * need);
		    }
		    else {
			n = read(fd, buf, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, buf, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else if (n != (data_size * need)) {
		      npts = k;
		      break;
		    }
		    i = 0;
		}
		itoi((char *)&buf[i++], (char *)&s4_data, 1);
		fl_tmp = (float)s4_data;
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else if(!strcmp(datatype, "f4"))
	{
	    IEEE ieee;
	    t4 = (float *)buf;
	    data_size = sizeof(float);
	    offset = foff + data_size * start;
#ifdef HAVE_LIBZ
	    if(zfd != Z_NULL) {
		if (gzseek(zfd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
	    else {
		if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
	    }
#else /* HAVE_LIBZ */
	    if (lseek(fd, offset, SEEK_SET) == -1) { errorRead(path, 1); return(0); }
#endif /* HAVE_LIBZ */
	    read_npts = read_bytes/data_size;
	    i = read_npts;
	    xlo = xhi = 0;
	    for(k = 0, l = 0, m = 0; k < npts; k++)
	    {
		if(i == read_npts)
		{
		    need = (npts-k < read_npts) ? npts-k:read_npts;
#ifdef HAVE_LIBZ
		    if(zfd != Z_NULL) {
			n = (ssize_t)gzread(zfd, t4, data_size * need);
		    }
		    else {
			n = read(fd, t4, data_size * need);
		    }
#else /* HAVE_LIBZ */
		    n = read(fd, t4, data_size * need);
#endif /* HAVE_LIBZ */
		    if (n == -1) {
		      errorRead(path, 0);
		      npts = k;
		      break;
		    }
		    else if (n != (data_size * need)) {
		      npts = k;
		      break;
		    }
		    i = 0;
		}
		vftoif((VAX *)&t4[i++], &ieee, 1);
		memcpy(&fl_tmp, &ieee, sizeof(float));
		if(fl_tmp > interval_hi)
		{
		    xhi = k;
		    interval_hi = fl_tmp;
		}
		if(fl_tmp < interval_lo)
		{
		    xlo = k;
		    interval_lo = fl_tmp;
		}
		l++;
		if(l == num_per_interval)
		{
		    if(xlo < xhi)
		    {
			data[m] = interval_lo; m++;
			data[m] = interval_hi; m++;
		    }
		    else
		    {
			data[m] = interval_hi; m++;
			data[m] = interval_lo; m++;
		    }
		    interval_hi = -1.e+60;
		    interval_lo =  1.e+60;
		    l = 0;
		}
	    }
	}
	else
	{
	    return(0);
	}
	if(l != 0)
	{
	    data[m] = interval_hi; m++;
	    data[m] = interval_lo; m++;
	}
	return(m);
}

/*
 * itoi --
 *      IEEE int to VAX int or vice-versa
 */
static void
itoi(register char *from, register char *to, register int num)
{
        from += 3;
        for (;num--;from += 7) {
                *to++ = *from--;
                *to++ = *from--;
                *to++ = *from--;
                *to++ = *from;
        }
}

#define SGL_LIMITS      2       /* sgl_limts count */
#define VAX_SNG_BIAS    0x81    /* exponent offset */
#define IEEE_SNG_BIAS   0x7f


static struct sgl_limits {
        VAX     s;
        IEEE    ieee;
} sgl_limits[] = {
        {
           /*{ 0x3f, 0xf, 0x0, 0xff },*/	/* Max Vax */
           { 0x3f, 0x1, 0x0, 0x7f },	/* Max Vax */
           { 0x0, 0xff, 0x0 },		/* Max IEEE */
        },
        {
           { 0x0, 0x0, 0x0, 0x0 },	/* Min Vax */
           { 0x0, 0x0, 0x0 },		/* Min IEEE */
        },
};

/*
 * vftoif
 *      VAX float to IEEE float
 */
static int
vftoif(register VAX *from, register IEEE *to, register int num)
{
	register struct sgl_limits      *lim;
	register int    indx, cnt;
	float   *tbfr;
	int     overflow;

	if ((tbfr = (float *) malloc(num * sizeof(float))) == NULL) {
	    logErrorMsg(LOG_ERR, "vftoif: couldn't allocate temporary buffer.");
	    return(0);
	}

	itoi((char *)from,(char *)tbfr,num);
	from = (VAX *)tbfr;

	for(cnt = num; cnt--; ++from, ++to)
	{
	    overflow = False;
	    for (indx = 0,lim = sgl_limits;indx < SGL_LIMITS;++indx,++lim)
	    {
		if ((from->mantissa2 == lim->s.mantissa2) &&
		    (from->exp == lim->s.exp) &&
			(from->mantissa1 == lim->s.mantissa1))
		{
		    *to = lim->ieee;
		    overflow = True;
		    break;
		}
	    }
	    if (!overflow)
	    {
		to->exp = from->exp - VAX_SNG_BIAS + IEEE_SNG_BIAS;
		to->mantissa = (from->mantissa1<< 16) | from->mantissa2;
	    }
	    to->sign = from->sign;
	}
	free(tbfr);
	return(1);
}

static ssize_t samples_per_block[6] = { 7,  3, 4,  5, 4, 1 };
static ssize_t words_per_block[6]   = { 2,  1, 1,  2, 2, 1 };
static ssize_t bits_per_sample[6]   = { 9, 10, 7, 12, 15, 28 };
static int initial_shift[6]     = { 1,  2, 4,  4,  4,  4 };

static ssize_t
decompress_e(unsigned long *in, void *data, int outtype, ssize_t start,
	     ssize_t npts)
{
	ssize_t i, j, k, nd, lr;
	ssize_t ms;
	long check;
	unsigned long *out = NULL;
	float	*fdata;
	int	*idata;

	unsigned int code, idx;
	long *signed_out;

	lr   = (ssize_t) ((int16_t *) (&in[0]))[0] / 4;
	nd   = (ssize_t) ((char  *) (&in[1]))[0];
	
	b_lshift(&in[1], 1, 8);
	check = b_lval(in[1], 24);

	out = (unsigned long *)malloc(7*lr*sizeof(long));
	if(out == NULL) {
	  return(-2);
	}
	signed_out = (long *) out;

	ms   = 0;
	i    = 2;

	while( i < lr)
	{
	    code = in[i] >> 28;

	    if(code < 8) idx = 0;
	    else if (code < 0xc) idx = 1;
	    else if(code >= 0xc) idx = 2 + (code & 0x3);
		
	    b_lshift(&in[i], words_per_block[idx], initial_shift[idx]);

	    for(j = 0; j < samples_per_block[idx]; j++)
	    {
		out[ms++] = b_lval(in[i], bits_per_sample[idx]);

		b_lshift(&in[i], words_per_block[idx], bits_per_sample[idx]);
	    }
	    i += words_per_block[idx];
	}
	
	for(k = 0; k < nd; k ++)
	{
	  for(j = 1; j < (ssize_t)ms; signed_out[j] += signed_out[j-1], j++);
	}

	if(check != signed_out[ms-1])
	{
	    Free(out);
	    return(-1);
	}
	else
	{
	    if (outtype == FLOAT_DATA)
	    {
		fdata = (float *) data;

		for(i = 0; i < (ssize_t)ms && i < npts; i++)
		{
		    if(i >= start) *fdata++ = (float)signed_out[i];
		}
	    }
	    else if (outtype == INT_DATA)
	    {
		idata = (int *) data;

		for(i = 0; i < (ssize_t)ms && i < npts; i++)
		{
		    if(i >= start) *idata++ = (int)signed_out[i];
		}
	    }
	    Free(out);
	    return(ms);
	}
}

static void
b_lshift(unsigned long *input, int nwords, int size)
{
	register int  i;
	unsigned long ltmp, rtmp;
	int left    = size;
	int right   = sizeof(long) * 8 - size;
	
	ltmp = input[0] << left;

	for(i = 0; i < nwords - 1; i++)
	{
	    rtmp = ((unsigned) input[i+1]) >> right;
	    input[i] = ltmp | rtmp;
	    ltmp = (input[i+1]) << left;
	}
	
	input[nwords - 1] = ltmp;
	
	return;
}

static long
b_lval(unsigned long input, int size)
{
	unsigned long val = input >> (sizeof(long) * 8 - size);
	unsigned long emask  = 0xffffffff << (size -1);

	if(val & emask) val |= emask;
	
	return(val);
}

