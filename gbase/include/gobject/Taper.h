#ifndef _Taper_H_
#define _Taper_H_

#include "gobject/GMethod.h"
#include "gobject/TimeSeries.h"

/**
 *  A GMethod object that specifies a taper operation. When a taper is
 *  applied to a waveform, a Taper object is created with the parameters
 *  applied and added to the waveforms methods-vector. Several types of
 *  tapers are currently available: hamming, hanning, cosine, parzen,
 *  welch, blackman or none.
 *
 *  @see new_Taper
 *  @see new_GObjectPart
 *  @see new_GMethodPart
 */

/** 
 *  A GMethod object that specifies a waveform taper.
 *
 *  @member type The type of taper.
 *  @member width The width of the taper (as a % of the waveform length), \
 *  		  if the type is "cosine".
 *  @member minpts The minimum length of the taper, if the type is "cosine".
 *  @member maxpts The maximum length of the taper, if the type is "cosine".
 */
typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	char			type[10];
	int			width;
	int			minpts;
	int			maxpts;
} _Taper, *Taper;

Taper new_Taper_fromString(const char *s);
Taper new_Taper(const char *type, int width, int minpts, int maxpts);
GMethod Taper_methodCreate(TimeSeries ts, const char *s);
void Taper_apply_ts(Taper taper, TimeSeries ts);

#endif
