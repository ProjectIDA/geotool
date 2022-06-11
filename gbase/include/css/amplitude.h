/**
 *      Amplitude relation from CSS 3.0 table definitions.
 *      This table contains arrival-based and origin-based amplitude
 *      measurements. The amplitude measurement is described in ampdescript.
 */

#ifndef _AMPLITUDE_3_0_H
#define _AMPLITUDE_3_0_H

#define AMPLITUDE30_LEN 181

/**
 *  Amplitude structure.
 *  @member ampid  Amplitude identifier. Every amplitude measure is assigned a unique positive integer that identifies it in the database. If an associated stamag record exists, then ampid links it to amplitude
 *  @member arid Arrival identifier. Each arrival is assigned a unique positive integer identifying it with a unique sta, chan and time.
 *  @member parid Every event-based parrival measure is assigned a unique positive integer that identifies it in the database. If an associated amplitude record exists, the parid links it to parrival.
 *  @member chan Channel identifier.
 *  @member amp Measured amplitude defined by amptype.
 *  @member per Measured period at the time of the amplitude measurement.
 *  @member snr Signal-to-noise ratio. This is an estimate of the ration of the amplitude of the signal to the amplitude of the noise immediately preceding it.
 *  @member amptime Epoch time of amplitude measure.
 *  @member start_time Epoch start time of the data interval.
 *  @member duration Total duration of amplitude window.
 *  @member bandw Frequency bandwidth.
 *  @member amptype Amplitude measure descriptor.
 *  @member units Units of amplitude measure.
 *  @member clip Clipped data flag. The value is a single-character flag to indicate whether (c) of not (n) the data were clipped.
 *  @member inarrival Flag to indicate whether of not amp is the same as it is in the arrival table.
 *  @member auth Author.
 *  @member lddate Load date.
 */

typedef struct amplitude {
	long	ampid;		/* amplitude identifier */
	long	arid;		/* arrival identifier */
	long	parid;		/* predicted arrival identifier */
	char	chan[9];	/* channel code */
	double	amp;		/* amplitude (nm) */
	double	per;		/* period (s) */
	double	snr;		/* signal-to-noise ratio */
	double	amptime;	/* time of amplitude measure */
	double	start_time;	/* start time of measurement window */
	double	duration;	/* duration of measurement window */
	double	bandw;		/* bandwidth */
	char	amptype[9];	/* amplitude measure descriptor */
	char	units[16];	/* units */
	char	clip[2];	/* clipping flag */
	char	inarrival[2];	/* "y" or "n" flag indicating if amp is the
				 * same as the amp in arrival table */
	char	auth[16];	/* author */
	char	lddate[18];	/* load date */
} AMPLITUDE30;

#endif /* _AMPLITUDE_3_0_H */
