#ifndef _PERIODS_H_
#define _PERIODS_H_

#include "gobject/GObject.h"
#include "gobject/Vector.h"

/* structure to hold objects that correspond to time periods.
 * Periods can overlap. Later periods added override earlier
 * periods when Periods_get is called.
 */
/**
 *  A GObject for holding objects that correspond to time periods.
 *  Periods can overlap. Later periods added override earlier
 *  periods when Periods_get() is called.
 *
 *  @see new_Periods
 *  @see Vector
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/

/** 
 *  The Periods structure definition.
 *
 *  @member numPeriods The number of time periods contained in the object.
 *  @member tbeg An array of beginning times for the periods.
 *  @member tend An array of ending times for the periods.
 *  @member v A Vector of numPeriods GObjects associated with the periods.
 */
typedef struct
{
	GObjectPart		core;

	/* Periods members */
	int			numPeriods;
	double			*tbeg;
	double			*tend;
	Vector			v;
} _Periods, *Periods;

Periods new_Periods(void);
void Periods_add(Periods p, double tbeg, double tend, GObject o);
GObject Periods_get(Periods p, double time);

#endif
