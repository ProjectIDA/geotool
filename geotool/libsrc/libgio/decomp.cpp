/*
 * NAME
 *      Routines for decompressing data.
 *
 * AUTHOR
 *      I. Henson
 */

#include "config.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>

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

/**
 * @private
 */
typedef union
{
	char	a[4];
	int	i;
} Word4;

/**
 * @private
 */
typedef union
{
	char	a[4];
	short	s;
} Word2;

static int bigEndian(void);
static int vftoif(VAX *from, IEEE *to, int num);

#define False 0
#define True  1

/**
 * Decompress a waveform.
 * @param data_in The compressed data.
 * @param data_in_size The number of bytes in data_in
 * @param data_out An integer or float array of length npts to receive the data values.
 * @param npts The number of data values to return.
 * @param datatype The data compression type. ("s4", "i4", "s3", "i3", "s2", "i2", t4", "f4", "v4", "g2", \
 *	"c2", or "c4")
 * @param outtype The desired data type of the output values (FLOAT_DATA or INT_DATA).
 */
int
cssioDecomp(void *data_in, int data_in_size, void *data_out, int npts, const char *datatype, int outtype)
{
	if(npts <= 0) return 0;

	if(!strcasecmp(datatype, "s4")) // big-endian integer
	{
	    return cssioDecompInt4(npts, (int *)data_in, data_out, outtype, !bigEndian());
	}
	else if(!strcasecmp(datatype, "i4")) // little-endian integer
	{
	    return cssioDecompInt4(npts, (int *)data_in, data_out, outtype, bigEndian());
	}
	else if(!strcmp(datatype, "s3")) // big-endian 3-byte integer
	{
	    return cssioDecompS3(npts, (int *)data_in, data_out, outtype);
	}
	else if(!strcmp(datatype, "i3")) // little-endian 3-byte integer
	{
	    return cssioDecompI3(npts, (int *)data_in, data_out, outtype);
	}
	else if(!strcasecmp(datatype, "s2")) // big-endian 2-byte integer
	{
	    return cssioDecompInt2(npts, (short *)data_in, data_out, outtype, !bigEndian());
	}
	else if(!strcasecmp(datatype, "i2")) // little-endian 3-byte integer
	{
	    return cssioDecompInt2(npts, (short *)data_in, data_out, outtype, bigEndian());
	}
	else if(!strcasecmp(datatype, "t4")) // big-endian float
	{
	    /* Can only request float output.
	     */
	    if(outtype != FLOAT_DATA)
	    {
		cssioSetErrorMsg("t4 data cannot be read as int");
		return(0);
	    }
	    return cssioDecompFloat(npts, (float *)data_in, (float *)data_out, !bigEndian());
	}
	else if(!strcasecmp(datatype, "f4")) // little-endian 4-byte float
	{
	    /* Can only request float output.
	     */
	    if(outtype != FLOAT_DATA)
	    {
		cssioSetErrorMsg("f4 data cannot be read as int");
		return(0);
	    }
	    return cssioDecompFloat(npts, (float *)data_in, (float *)data_out, bigEndian());
	}
	else if(!strcasecmp(datatype, "v4"))
	{
	    /* Can only request float output.
	     */
	    if(outtype != FLOAT_DATA)
	    {
		cssioSetErrorMsg("v4 data cannot be read as int");
		return(0);
	    }
	    return cssioDecompV4(npts, (float *)data_in, (float *)data_out);
	}
	else if(!strcmp(datatype, "g2"))
	{
	    return cssioDecompG2(npts, (unsigned char *)data_in, data_out, outtype);
	}
        else if(!strcmp(datatype, "c2") || !strcmp(datatype, "c4"))
        {
	    return cssioDecompC24(0, npts, (char *)data_in, (float *)data_out, datatype, outtype);
	}
        else if(!strcmp(datatype, "ca")) {
	    return cssioDecompCa((unsigned char *)data_in, data_in_size, data_out, npts, outtype);
	}
	else
	{
	    /* unknown data type */
	    cssioSetErrorMsg("cssioDecomp: unknown datatype: %s", datatype);
	    return -1;
	}
}

int
cssioDecompInt4(int npts, int *data_in, void *data_out, int outtype, bool swap_bytes)
{
	/* The input array data_in is 4-byte integer (big- or little-endian). The output
	 * array is either integer or float with the correct byte order for the
	 * current machine. data_out can point to the same memory as data_in
	 */

	/* Swap the byte order, if necessary
	 */
	if( swap_bytes )
	{
	    Word4 e1, e2;

	    if(outtype == FLOAT_DATA) /* also convert int to float */
	    {
		float *fdata = (float *)data_out;
		for(int k = 0; k < npts; k++)
		{
		    e1.i = data_in[k];
		    e2.a[0] = e1.a[3];
		    e2.a[1] = e1.a[2];
		    e2.a[2] = e1.a[1];
		    e2.a[3] = e1.a[0];
		    fdata[k] = (float)e2.i;
		}
	    }
	    else { // integer
		int *idata = (int *)data_out;
		for(int k = 0; k < npts; k++)
		{
		    e1.i = data_in[k];
		    e2.a[0] = e1.a[3];
		    e2.a[1] = e1.a[2];
		    e2.a[2] = e1.a[1];
		    e2.a[3] = e1.a[0];
		    idata[k] = e2.i;
		}
	    }
	}
	else if(outtype == FLOAT_DATA) /* convert int to float */
	{
	    float *fdata = (float *)data_out;
	    for(int k = 0; k < npts; k++) {
		fdata[k] = (float)data_in[k];
	    }
	}
	else if(data_out != (void *)data_in) /* just copy data */
	{
	    memcpy(data_out, data_in, npts*sizeof(int));
	}
	return npts;
}

int
cssioDecompS3(int npts, int *data_in, void *data_out, int outtype)
{
	Word4 e1, e2;
	int *idata = (int *)data_out;

	/* The input array data_in is big-endian 3-byte int. The output array
	 * is either 4-byte int or 4-byte float with the correct byte order for
	 * the current machine.
	 */
	memcpy(idata, data_in, npts*3);

	s3tos4(idata, npts); // convert to four byte int in place

	if(outtype == FLOAT_DATA)
	{
	    float *fdata = (float *)data_out;
		
	    if( !bigEndian() )
	    {
		for(int k = 0; k < npts; k++)
		{
		    memcpy(&e1.i, &idata[k], sizeof(int));
		    e2.a[0] = e1.a[3];
		    e2.a[1] = e1.a[2];
		    // since s3tos4 uses a short in the convert from 3 to 4 byte words, the two most significant
		    // bytes will be out of big-endian order if we are on a little-endian machine.
		    e2.a[2] = e1.a[0];
		    e2.a[3] = e1.a[1];
		    fdata[k] = (float)e2.i;
		}
	    }
	    else {
		for(int k = 0; k < npts; k++) fdata[k] = (float)idata[k];
	    }
	}
	else if( !bigEndian() )
	{
	    int *idata = (int *)data_out;
	    for(int k = 0; k < npts; k++)
	    {
		memcpy(&e1.i, &idata[k], sizeof(int));
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		// since s3tos4 uses a short in the convert from 3 to 4 byte words, the two most significant
		// bytes will be out of big-endian order if we are on a little-endian machine.
		e2.a[2] = e1.a[0];
		e2.a[3] = e1.a[1];
		idata[k] = e2.i;
	    }
	}
	return npts;
}

int
cssioDecompI3(int npts, int *data_in, void *data_out, int outtype)
{
	/* The input array data_in is little-endian 3-byte int. The output array
	 * is either 4-byte int or 4-byte float with the correct byte order for
	 * the current machine.
	 */
	char *a = (char *)data_in;
	int *b = (int *)data_out;

	for(int i = npts-1; i >= 0; i--) {
	    b[i] = 0;
	    memcpy(b+i, a+i*3, 3);
	}

	/* Swap the byte order, if running on a big-endian machine
	 */
	if( bigEndian() )
	{
	    Word4 e1, e2;

	    if(outtype == FLOAT_DATA) // Swap bytes and convert short to float
	    {
		float *fdata = (float *)data_out;
		for(int k = npts-1; k >= 0; k--)
		{
		    e1.i = b[k];
		    e2.a[0] = e1.a[3];
		    e2.a[1] = e1.a[2];
		    e2.a[2] = e1.a[1];
		    e2.a[3] = e1.a[0];
		    fdata[k] = (float)e2.i;
		}
	    }
	    else {
		int *idata = (int *)data_out;
		for(int k = npts-1; k >= 0; k--)
		{
		    e1.i = b[k];
		    e2.a[0] = e1.a[3];
		    e2.a[1] = e1.a[2];
		    e2.a[2] = e1.a[1];
		    e2.a[3] = e1.a[0];
		    idata[k] = e2.i;
		}
	    }
	}
	else if(outtype == FLOAT_DATA)
	{
	    // convert int to float
	    float *fdata = (float *)data_out;
	    for(int k = 0; k < npts; k++)
	    {
		fdata[k] = (float)b[k];
	    }
	}
	return npts;
}

int
cssioDecompInt2(int npts, short *data_in, void *data_out, int outtype, bool swap_bytes)
{
	/* The input array data_in is 2-byte short. The output array
	 * is either 4-byte int or 4-byte float with the correct byte order for
	 * the current machine.
	 */

	/* Swap the byte order
	 */
	if( swap_bytes )
	{
	    Word2 e1, e2;

	    if(outtype == FLOAT_DATA) // Swap bytes and convert short to float
	    {
		float *fdata = (float *)data_out;
		for(int k = npts-1; k >= 0; k--)
		{
		    e1.s = data_in[k];
		    e2.a[0] = e1.a[1];
		    e2.a[1] = e1.a[0];
		    fdata[k] = (float)e2.s;
		}
	    }
	    else { // Swap bytes and convert short to int
		int *idata = (int *)data_out;
		for(int k = npts-1; k >= 0; k--)
		{
		    e1.s = data_in[k];
		    e2.a[0] = e1.a[1];
		    e2.a[1] = e1.a[0];
		    idata[k] = (int)e2.s;
		}
	    }
	}
	else if(outtype == FLOAT_DATA)
	{
	    // convert short to float
	    float *fdata = (float *)data_out;
	    for(int k = npts-1; k >= 0; k--)
	    {
		fdata[k] = (float)data_in[k];
	    }
	}
	else {
	    // convert short to int
	    int *idata = (int *)data_out;
	    for(int k = npts-1; k >= 0; k--)
	    {
		idata[k] = (int)data_in[k];
	    }
	}
	return npts;
}

int
cssioDecompFloat(int npts, float *data_in, float *data_out, bool swap_bytes)
{
	/* The input array data_in is big-endian 4-byte float. The output array
	 * is float with the correct byte order for the current machine.
	 * data_out can point to the same memory as data_in
	 */
	if( swap_bytes )
	{
	    union
	    {
		char	a[4];
		float	f;
	    } e1, e2;

	    for(int k = 0; k < npts; k++)
	    {
		e1.f = *data_in++;
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		*data_out++ = e2.f;
	    }
	}
	else if(data_out != (void *)data_in) /* just copy data */
	{
	    memcpy(data_out, data_in, npts*sizeof(float));
	}
	return npts;
}

int
cssioDecompG2(int npts, unsigned char *data_in, void *data_out, int outtype)
{
	/* The input array data_in g2 compressed data. The output data
	 * is either 4-byte int or float with the correct byte order for
	 * the current machine.
	 */
	g2tofloat(data_in, data_out, outtype, npts);
	return npts;
}

int
cssioDecompV4(int npts, float *data_in, float *data_out)
{
	/* The input array data_in is VAX 4-byte float. The output array is
	 * IEEE 4-byte float with the correct byte order for the current machine.
	 */

	for(int k = 0; k < npts; k++)
	{
	    VAX f4_data;
	    IEEE ieee;
	    memcpy(&f4_data, &data_in[k], sizeof(float));
	    if(!vftoif(&f4_data, &ieee, 1))
	    {
		npts = k;
		break;
	    }
	    memcpy(&data_out[k], &ieee, sizeof(float));
	}

	if( !bigEndian() )
	{
	    union
	    {
		char	a[4];
		float	f;
	    } e1, e2;

	    for(int k = 0; k < npts; k++)
	    {
		memcpy(&e1.f, &data_out[k], sizeof(float));
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		data_out[k] = e2.f;
	    }
	}
	return npts;
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
 *	this might not work on a little-endian machine
 */
static int
vftoif(register VAX *from, register IEEE *to, register int num)
{
	register struct sgl_limits      *lim;
	register int    indx, cnt;
	float   *tbfr;
	int     overflow;
	Word4 e1,e2;

	if ((tbfr = (float *) malloc(num * sizeof(float))) == NULL) {
	    logErrorMsg(LOG_ERR, "vftoif: couldn't allocate temporary buffer.");
	    return(0);
	}

	for(int k = 0; k < num; k++)
	{
	    memcpy(&e1.i, &from[k], sizeof(int));
	    e2.a[0] = e1.a[3];
	    e2.a[1] = e1.a[2];
	    e2.a[2] = e1.a[1];
	    e2.a[3] = e1.a[0];
	    memcpy(&tbfr[k], &e2.i, sizeof(int));
	}
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


int
cssioDecompC24(int start, int npts, char *data_in, void *data_out,
		const char *datatype, int outtype)
{
	int	k;
	long	longcheck, decompnum;
	long	n28m1 = 134217727;

	/* The input array data_in is (start+npts) bytes of compressed
	 * data. The output array is either 4-byte int or float with the
	 * correct byte order for the current machine.
	 */
	char *cdata = data_in;
	char *bufout = (char *)malloc(sizeof(long) * (start+npts + 10));
	short *shortout = (short *)bufout;
	long *longout = (long *)bufout;

	if(!bufout) {
	    cssioSetErrorMsg("cssioDecomp: malloc failed.");
	    logErrorMsg(LOG_ERR, "cssioDecomp: malloc failed.");
	    return 0;
	}
		
	decompnum = longdcpress(start+npts, cdata, longout);

	decompnum--;

	rmfdif(decompnum, longout, longout);
	rmfdif(decompnum, longout, longout);

	longcheck = 0;
	for(k = 0; k < decompnum; k++) longcheck += longout[k];

	longcheck = longcheck & n28m1;

	if(start + npts > decompnum) {
	    npts = decompnum - start;
	    if(npts < 0) npts = 0;
	}

	if(!strcmp(datatype, "c2"))
	{
	    for(k = 0; k < decompnum; k++) {
		shortout[k] = (short)longout[k];
	    }
		
	    if(outtype == FLOAT_DATA)
	    {
		float *fdata = (float *)data_out;
                for(k = 0; k < npts; k++) {
		    fdata[k] = (float)shortout[start+k];
		}
	    }
	    else
	    {
		int *idata = (int *)data_out;
		for(k = 0; k < npts; k++) {
		    idata[k] = (int)shortout[start+k];
		}
	    }
	}
	else if(!strcmp(datatype, "c4"))
	{
	    if(outtype == FLOAT_DATA)
	    {
		float *fdata = (float *)data_out;
		for(k = 0; k < npts; k++) {
		    fdata[k] = (float)longout[start+k];
		}
	    }
	    else
	    {
		int *idata = (int *)data_out;
		for(k = 0; k < npts; k++) {
		    idata[k] = (int)longout[start+k];
		}
	    }
	}
	free(bufout);
	npts = k;

	return npts;
}


int
cssioDecompCa(unsigned char *data_in, int data_in_len, void *data_out, int npts,
		int outtype)
{
	int	status;
	long	v0;

	status = canada_uncompress(data_in, (uint32_t *)data_out, &data_in_len,
					npts, (uint32_t *)&v0);

	if(status != CANCOMP_SUCCESS)
	{
	    if(status == CANCOMP_ERR) {
		cssioSetErrorMsg("unrecoverable error (malloc fails)");
	    }
	    else if(status == CANCOMP_NOT_20) {
		cssioSetErrorMsg("number of samples not divisible by 20");
	    }
	    else if(status == CANCOMP_CORRUPT) {
		cssioSetErrorMsg("corrupted call");
	    }
	    else if(status == CANCOMP_EXCEED) {
		cssioSetErrorMsg(
"number of bytes available in compressed data exceeded during decompression");
	    }
	    return(0);
	}

	if( !bigEndian() )
	{
	    int i, *idata;
	    union
	    {
		char	a[4];
		int	i;
	    } e1, e2;

	    idata = (int *)data_out;
	    for(i = 0; i < npts; i++)
	    {
		memcpy(&e1.i, &idata[i], sizeof(int));
		e2.a[0] = e1.a[3];
		e2.a[1] = e1.a[2];
		e2.a[2] = e1.a[1];
		e2.a[3] = e1.a[0];
		idata[i] = e2.i;
	    }
	}

	if(outtype == FLOAT_DATA)
	{
	    int *idata = (int *)data_out;
	    float *fdata = (float *)data_out;

	    for(int k = 0; k < npts; k++) {
		fdata[k] = (float)idata[k];
	    }
	}
	return npts;
}


// check if the byte order is big-endian, least significant byte is last
static int
bigEndian(void)
{
	union
	{
	    char	a[4];
	    int		i;
	} e;

        e.a[0] = 0; e.a[1] = 0;
        e.a[2] = 0; e.a[3] = 1;
	return (e.i == 1) ? 1 : 0;
}
