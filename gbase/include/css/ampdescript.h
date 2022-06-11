/**
 *      Ampdescript relation from CSS 3.0 table definitions.
 *      This table contains desciptions of how amplitude measurements in
 *	the amplitude table were made.
 */

#ifndef _AMPDESCRIPT_3_0_H_
#define _AMPDESCRIPT_3_0_H_

#define AMPDESCRIPT30_LEN   321

/**
 *  Ampdescript structure.
 *  @member amptype Ampltiude measure descriptor. Initial value = "-".
 *  @member toff Offset from theoretical or observed arrival time. Initial value = -999.
 *  @member tlen Duration of measurement window. Initial value = -1.
 *  @member gvlo Low group velocity for measurement window (km/sec). Initial value = -999.
 *  @member gvhi High group velocity for measurement window (km/sec). Initial value = -999.
 *  @member mtype Measurement type. Initial value = "-".
 *  @member descr Description.
 *  @member lddate Load date.
 */
typedef struct ampdescript {
	char	amptype[9];
	double	toff;
	double	tlen;
	double	gvlo;
	double	gvhi;
	char	mtype[9]; /* "peak", "stav", "rms", "peak2tr" or "1stpeak" */
	char	descr[256];
	char	lddate[18];
} AMPDESCRIPT30;

/* peak : maximum amplitude
 * stav : maximum short-term average amplitude
 * rms  : root-mean squared amplitude
 * 1stpeak : first motion amplitude
 */

#define AMPDESCRIPT_NULL \
{ \
"-",		/* amptype	*/ \
-999.,		/* toff		*/ \
-1,		/* tlen		*/ \
-999.,		/* gvlo		*/ \
-999.,		/* gvhi		*/ \
"-",		/* mtype	*/ \
"-",		/* descr	*/ \
"-",		/* lddate	*/ \
}

#endif /* _AMPDESCRIPT_3_0_H */
