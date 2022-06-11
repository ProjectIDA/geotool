#ifndef _GEO_CORE_H
#define _GEO_CORE_H

#include <stdlib.h>
#include <string.h>

#include "gobject/GObject.h"

/** 
 *  Waveform tag contents.
 */
typedef struct tag_members
{
	int	num_contrib;	/* number of contributors to label */
	int	contrib[40];	/* tells what makes the label */
	bool	user_defined;	/* T/F if we have it */
	bool	chan;		/* T/F if we have it */
	bool	filter;		/* T/F if we have it */
	bool	distance_deg;	/* T/F if we have it */
	bool	distance_km;	/* T/F if we have it */
	bool	azimuth;	/* T/F if we have it */
	bool	have_chan;	/* T/F if user wants it */
	char	*ud_string;	/* the string */
} TagMembers;

#define TAG_MEMBERS_INIT \
{ \
	0, 			/* num_contrib */ \
	{0,0,0,0,0,0,0,0,0,0, \
	 0,0,0,0,0,0,0,0,0,0, \
	 0,0,0,0,0,0,0,0,0,0, \
	 0,0,0,0,0,0,0,0,0,0,},	/* contrib[40] */ \
	False,			/* user_defined */ \
	False,			/* chan */ \
	False,			/* filter */ \
	False,			/* distance deg */ \
	False,			/* distance km */ \
	False,			/* azimuth */ \
	False,			/* have_chan */ \
	(char *)NULL,		/* ud_string */ \
}

/** 
 *  A GObject used to hold miscellaneous information about a waveform.
 *
 *  @see new_GObjectPart
 *  @see Response
 */ 
typedef struct
{
	GObjectPart		core;

	/* GeoCore members */
	int		file_order;		/* file order number */
	TagMembers	tag_members;		/* tag component structure */
} _GeoCore, *GeoCore;

GeoCore new_GeoCore(void);

#define getGeoCore(ts) ((GeoCore)Hashtable_get(ts->hashtable,"GeoCore"))
#define putGeoCore(ts,gc) Hashtable_put(ts->hashtable, "GeoCore", (GObject)gc)

#endif
