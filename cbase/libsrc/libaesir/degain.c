/*
 * Copyright 1988-1994 Science Applications International Corporation.
 */
	
/*
 * .TH G2TOINT 3 "11 November 1988"
 * .SH NAME
 *	g2toint - convert g2 format data to host format integer
 *	.br
 *	inttog2 - convert host integer data to g2 format
 *	.br
 *	g2tofloat - convert g2 format data to host single precision
 *	.br
 *	floattog2 - convert host single precision to g2 format.
 *      .br
 *      i2toint   - convert host integer to host int format.
 *      .br
 *      t4toint   - convert Sun float data to host int format.
 *      .br
 *      a2tot4    - convert from aftac gain-ranged format to Sun float
 *      .br
 *      t4toa2    - convert Sun float to aftac gain-ranged format
 *      .br
 *      s3tos4    - convert Sun 3 byte integer to Sun 4 byte integer
 *      .br
 *      s4tos3    - convert Sun 4 byte integer to Sun 3 byte integer
 *
 * .SH FILE
 *	degain.c
 * .SH SYNOPSIS
 *	.nf
 *	void
 *	g2toint (from, to, num)
 *	unsigned char	*from;		(i) array with gainranged data
 *	int		*to;		(o) degained data
 *	int		num;		(i) number of entries
 *
 *	void
 *	inttog2 (from, to, num)
 *	int32_t		*from;		(i) array with i*4 data
 *	unsigned char	*to;		(o) gain ranged data
 *	int		num;		(i) number of entries
 *
 *	void
 *	g2tofloat (from, to, num)
 *	unsigned char	*from;		(i) array with gainranged data
 *	float		*to;		(o) degained data
 *	int		num;		(i) number of entries
 *
 *	void
 *	floattog2 (from, to, num)
 *	float		*from;		(i) array with i*4 data
 *	unsigned char	*to;		(o) gain ranged data
 *	int		num;		(i) number of entries
 *
 *	void
 *	i2toint (from, to, num)
 *	int16_t		*from;		(i) array with i*2 (not vax!!) data
 *	int         	*to;		(o) integer data
 *	int		num;		(i) number of entries
 *
 *	void
 *	t4toint (from, to, num)
 *	float		*from;		(i) array with Sun float data
 *	int    	        *to;		(o) integers
 *	int		num;		(i) number of entries
 *
 *	Fortran interface:
 *
 *	g2toint (from, to, num)
 *	integer*2	from(num)
 *	integer*4	to(num)
 *	integer		num
 *
 *	inttog2 (from, to, num)
 *	integer*4	from(num)
 *	integer*2	to(num)
 *	integer		num
 *
 *	g2tofloat (from, to, num)
 *	integer*2	from(num)
 *	real		to(num)
 *	integer		num
 *
 *	floattog2 (from, to, num)
 *	real		from(num)
 *	integer*4	to(num)
 *	integer		num
 *	.fi
 *
 * .SH DESCRIPTION
 *	.LP
 *	G2 format provides a representation with a manitissa ranging
 *	from -8192 to +8192 (8191 is the BIAS) and a multiplier of 1,
 *	4, 16, or 128 (the multiplier is 128/<gain-code>).
 *	Although the total dynamic range is +- 2**20,
 *	many numbers within this range are not representable.
 *	.LP
 *	To avoid the difficulty of having a vax and ieee versions of
 *	the g2 format, we define the g2 format as a byte stream.  Each
 *	value takes two bytes (bytes are 8 bits) with the high order
 *	two bits of byte[0] being the gain code, the low order 6 bits
 *	and the 8 bits of the second byte being the value (the 6 bits
 *	are the high order bits in the 14 bit value).
 *	.LP
 *	If value is less than BIAS, it is a negative number, if it is
 *	greater than BIAS it is a positive number.
 *	.LP
 *	Bit 15 and 16 (two upper bits) of the first byte in each
 *	sample contains the gain ranging value which determines the gain code:
 *	.nf
 *	00 -> gain code	= 128
 *	01 -> gain code 	= 32
 *	10 -> gain code  	= 8
 *	11 -> gain code  	= 1
 *	.fi
 *	.LP
 *	The formula for g2 to integer is:
 *	(value-BIAS)*(128/<gain code>).
 * .SH NOTES
 *	These routines can be used as inplace conversions.
 *	.LP
 *	Although g2 format offers less precision then integer format,
 *	in practice there is no difference since the original data from
 *	NORSAR is recorded in g2 format.  In other words, we have already
 *	lost the precision before the data reaches the processing stage.
 * .SH AUTHOR
 *	Pete Ware
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <inttypes.h>
#include <stdlib.h>

#include "libaesir.h"

/*
 * 128/gain code results in values to multiply sample value with.
 *
 * The values to multiply with are:
 *	gain bit 00 --> 128/128 =   1 no gain,0 bit shift
 *	gain bit 01 --> 128/32  =   4         2 bit shift
 *	gain bit 10 --> 128/8   =  16         4 bit shift
 *	gain bit 11 --> 128/1   = 128         7 bit shift
 *
 */

static int gain_value[4] = {0, 2, 4, 7}; 

/*
 * Array gain_range contains the maximum+1 value for each
 * gain bit. It is used when converting from i4 to g2 format.
 */

static int gain_range[4] = {1 << 13, 1 << 15, 1 << 17, 1 << 30};

#define TOP	0xc0			/* top two bits */
#define BOT	0x3f			/* bottom 6 bits */
#define BIAS	8191			/* offset */
#define BYTES_PER_G2	2		/* bytes in g2 sample.  Changing
					 this is not sufficient to change
					 the size of g2 format data */

/*
 * g2toint converts NORESS gain ranged data to host specific int format
 */


void
g2toint (unsigned char *from, int *to, int  num)
{
	int value;

	/*
	 * Start at the end and work backwords.  This allows
	 * inplace conversions, assuming there is enough space.
	 */

	for (to += (num - 1), from += (num-1) * BYTES_PER_G2;num-- ;
	     from -= BYTES_PER_G2, --to)
	{
		value = ((from[0] & BOT) << 8) | from[1];
		*to = (value - BIAS) << 
			gain_value[(unsigned char) (TOP & from[0]) >> 6];
	}
}

/*
 * inttog2 converts the hosts int data to NORESS gain ranged data.
 */

void
inttog2 (int *from, unsigned char *to, int num)
{
	int gain;
	int value;
	int neg;

	for (;num--; ++from)
	{
		value = *from;
		if (value < 0)
		{
			value = -value;
			neg = 1;
		}
		else 
		{
			neg = 0;
		}

		for (gain = 0; gain < (signed int)(sizeof(gain_range)/sizeof (int)); gain++)
			if (value < gain_range[gain])
				break;

		value = (value >> gain_value[gain]);
		if (neg)
		{
			value = -value + BIAS;
		}
		else
		{
			value += BIAS;
		}
		/*
		 * make high order two bits of to[0] be the gain and the low
		 * order 6 bits of to[0] be the high order 6 bits of value.
		 */
		*to++ = (gain << 6) | (0x3f & (value >> 8));
		/* make to[1] be the low order 8 bits of value */
		*to++ = value & 0xff;
	}
}

/*
 * floattog2 converts the hosts float data to NORESS gain ranged data.
 */

void
floattog2 (float *from, unsigned char *to, int num)
{
	int value;

	/*
	 * lets not duplicate all the effort in inttog2().
	 */

	while (num--)
	{
		value = (int)*from++;
		inttog2 (&value, to, 1);
		to += 2;
	}
}

/*
 *  s3tos4 converts 3 byte (24 bit) data to 4 byte data 
 */

void s4tos3(unsigned char *s4, unsigned char *s3, int n)
{
	unsigned char * p4;
	unsigned char * p3;
	int      i;
	
	for(i=0, p4=s4+1, p3=s3; 
	    i < n; 
	    p4+=4, p3+=3, i++)
	{
#ifdef __svr4__
		memmove (p3, p4, (size_t) 3);
#else
		bcopy (p4, p3, (size_t) 3);
#endif
	}
	return;
}

void s3tos4(unsigned char *s3,int *s4,int n)
{
        int           *  p4;
	unsigned char *  p3;
	int               i;

	p4 = s4 + (n-1);

	for(i = n, p3 = s3 + 3*(n-1);
	    i > 0;
	    i--, p4 -= 1, p3 -=3)
	{
#ifdef __svr4__
		memmove ((unsigned char *) p4 + 1, p3, (size_t) 3);
#else
		bcopy (p3, (unsigned char *) p4 + 1, (size_t) 3);
#endif
/*		memcpy((caddr_t) p4 + 1, p3, 3); */
		
		if(*p4 & 0x00800000) *p4 |= 0xff000000;
		
		else *p4 &= 0x00ffffff;
	}
	return;
}

/* 
 *  a2tot4 converts aftac gain-ranged format to Sun float
 */

void
a2tot4 (uint16_t *a2,float *t4,int n)
{
	float     *q4;
	uint16_t  *p2;
	int       i;
	int16_t   j;

	for(i = n, p2 = a2 + (n-1), q4 = t4 + (n-1);
	    i > 0;
	    i--, p2--, q4--)
	{
		j = *p2;
		
		if(j & 0x1000) j |= 0xf000;
		
		else j &= 0x1fff;

		*q4 = (double) (j << 11);

		*q4 /= (double)(1 << ((uint16_t) (*p2 & 0xe000) >> 12));

	}
}

/* 
 *  t4toa2 converts from Sun float aftac gain ranged format
 */

void
t4toa2 (float *t4, int16_t *a2, int n)
{
       /*  
	*  No checks here for valid inputs: check 'em before you get here 
	*/

	float           *q4;
	int16_t         *p2;
	int               i,j;
	int             l, m;
	static   uint16_t gain[8] = {
		0xe000, 0xc000, 0xa000, 0x8000, 0x6000, 0x4000,
		0x2000, 0x0000
		};

	for(i = 0, p2 = a2 , q4 = t4;
	    i < n;
	    i++, p2++, q4++)
	{
		m  = (int)(*q4 * 8);     /*  Convert to an integer */

		l  = abs(m);

		if(m) m = m/l;                 /*  get the sign */

		for(j = 0;
		    j < 8 && l & 0xfffff000;
		    j++, l >>=  2);

		*p2 = (int16_t) m * (int16_t) l ;            /* replace the sign */

		*p2 = (*p2 & 0x1fff) | (int16_t) gain[j];  /* merge with gain */
		
	}
}

/*
 * i2toint converts IDA/IRIS i2 data to host specific int format
 */

void
i2toint (int16_t *from, int *to, int num)
{

	/*
	 * Start at the end and work backwords.  This allows
	 * inplace conversions, assuming there is enough space.
	 */

	for (to += (num - 1), from += (num-1); num-- ;
	     from--, to--)
	{
		*to = *from;
	}
}

/*
 * t4toint converts host float data to host int integers.
 */
void
t4toint (float *from, int *to, int num)
{
	int i;

	for (i = 0; i < num; to[i] = (int) from[i], i++);
}

/*
 * i4toint converts from vax integer format to host int format
 */

void
i4toint (char *from, char *to, int num)
{

#ifdef vax
	if (from != to)
		bcopy (from, to, num * sizeof (int));
#else
	int		val;
	REG char	*ptr;
	
	for (ptr = (char *) &val; num-- > 0;
	     from += sizeof (int), to += sizeof (int))
	{
		ptr[0] = from[3];
		ptr[1] = from[2];
		ptr[2] = from[1];
		ptr[3] = from[0];
#ifdef __svr4__
		memmove (to, ptr, sizeof (int));
#else
		bcopy (ptr, to, sizeof (int));
#endif
	}
#endif
}


/*
 * s4toint converts from vax integer format to host int format
 */

void
s4toint (char *from, char *to, int num)
{

#ifdef sun
	if (from != to)
# ifdef __svr4__
		memmove (to, from, num * sizeof (int));
# else
		bcopy (from, to, num * sizeof (int));
# endif /* __svr4__ */
#else
	int		val;
	REG char	*ptr;
	
	for (ptr = (char *) &val; num-- > 0;
	     from += sizeof (int), to += sizeof (int))
	{
		ptr[0] = from[3];
		ptr[1] = from[2];
		ptr[2] = from[1];
		ptr[3] = from[0];
		bcopy (ptr, to, sizeof (int));
	}
#endif
}

/*
 * g2tofloat converts NORESS gain ranged data to host specific float format
 */

void
g2tofloat (unsigned char *from, float *to,int num)
{
	int value;

	/*
	 * Start at the end and work backwords.  This allows
	 * inplace conversions, assuming there is enough space
	 * and the user is not doing any tricky buffering.
	 */

	for (to += (num - 1), from += (num-1) * BYTES_PER_G2;num-- ;
	     from -= BYTES_PER_G2, --to)
	{
		value = ((from[0] & BOT) << 8) | from[1];
		*to = (value - BIAS) << 
			gain_value[(unsigned char) (TOP & from[0]) >> 6];
	}
}


/*
 * i2tofloat converts IDA/IRIS i2 data to host specific float format
 */
void
i2tofloat (int16_t *from, float *to, int num)
{

	/*
	 * Start at the end and work backwords.  This allows
	 * inplace conversions, assuming there is enough space.
	 */

	for (to += (num - 1), from += (num-1); num-- ;
	     from--, to--)
	{
		*to = (float) *from;
	}
}

/*
 * i4tofloat converts from vax integer format to host float format
 */

void
i4tofloat (int *from, float *to, int num)
{

#ifdef vax
	int	i;
	
	for ( i = 0; i < num; to[i] = (float) from[i], i++);
#else
	int		val;
	REG char	*ptr, *cp;

	for (ptr = (char *) &val; num-- > 0; from++, to++)
	{
		cp = (char *) from;
		ptr[0] = cp[3];
		ptr[1] = cp[2];
		ptr[2] = cp[1];
		ptr[3] = cp[0];
		*to = (float) val;
	}
#endif
}

/*
 * s4tofloat converts from vax integer format to host float format
 */

void
s4tofloat (int *from, float *to, int num)
{

#ifdef sun
	int	i;
	
	for (i = 0; i < num; to[i] = (float) from[i], i++);
#else
	int		val;
	REG char	*ptr, *cp;

	for (ptr = (char *) &val; num-- > 0; from++, to++)
	{
		cp = (char *) from;
		ptr[0] = cp[3];
		ptr[1] = cp[2];
		ptr[2] = cp[1];
		ptr[3] = cp[0];
		*to = (float) val;
	}
#endif
}

/*
 * t4tofloat converts host float data to host float integers.
 */
void
t4tofloat (float *from, float *to, int num)
{
	if (from != to)
#ifdef __svr4__
		memmove (to, from, num * sizeof (float));
#else
		bcopy ((char *) from, (char *) to, num * sizeof (float));
#endif
}

/*
 * Fortran interfaces to above routines
 */

void
g2toint_ (unsigned char *from, int *to, int *num)
{
	g2toint (from, to, *num);
}

void
g2tofloat_ (unsigned char *from, float *to, int *num)
{
	g2tofloat (from, to, *num);
}

void
inttog2_ (int *from, unsigned char *to, int *num)
{
	inttog2 (from, to, *num);
}

void
floattog2_ (float *from, unsigned char *to, int *num)
{
	floattog2 (from, to, *num);
}

void
i2toint_ (int16_t *from, int *to,int *num)
{
	i2toint (from, to, *num);
}

void
i2tofloat_ (int16_t *from, float *to, int *num)
{
	i2tofloat (from, to, *num);
}

void
i4tofloat_ (int *from, float *to, int *num)
{
	i4tofloat (from, to, *num);
}

void
s4tofloat_ (int *from,float *to,int *num)
{
	s4tofloat (from, to, *num);
}

void
t4tofloat_ (float *from,float *to,int *num)
{
	t4tofloat (from, to, *num);
}

void
a2tot4_ (int16_t *from,float *to, int *num)
{
	a2tot4 ((uint16_t *) from, to, *num);
}

void
t4toa2_ (float *from, int16_t *to, int *num)
{
	t4toa2 (from, to, *num);
}

void
s3tos4_ (char *from, int *to, int *num)
{
	s3tos4 ((unsigned char *) from, (int *) to, *num);
}

void
s4tos3_ (int *from,char *to, int *num)
{
	s4tos3 ((unsigned char *) from, (unsigned char *) to, *num);
}
