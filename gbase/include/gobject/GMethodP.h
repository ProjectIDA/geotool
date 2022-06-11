#ifndef _GMETHOD_P_H_
#define _GMETHOD_P_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "gobject/GMethod.h"

/** 
 *  GMethodPart of a GMethod structure. This is the private declaration of
 *  the GMethod structure.  A GMethod is a GObject used to hold information
 *  about a processing method for TimeSeries objects. The function protoype is
 *  GMethod_applyMethod. A second function prototype, GMethod_toStringMethod,
 *  is available to produce a character string method descriptor. A GMethod
 *  structure is any structure that has a GObjectPart as its first member and
 *  a GMethodPart as its second member. Create a GMethodPart with
 *  new_GMethodPart().
 *  <pre>
 *  typedef struct
 *  {
 *	GObjectPart	core;
 *	GMethodPart	gm;
 *  } _GMethod, *GMethod;
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/


/** 
 *  The GMethodPart structure definition. This is the private declaration of
 *  the GMethodPart structure.
 *
 *  @member method_type	Currently not defined.
 *  @member apply	A GMethod_applyMethod function pointer.
 *  @member toString	A GMethod_toStringMethod function pointer.
 *
 *  @see new_GMethod
 *  @see new_GObject
 *  @private
 */
typedef struct GMethod_struct
{
	/* GMethod members */
	int				method_type;
	GMethod_applyMethod		apply;
	GMethod_toStringMethod		toString;
} _GMethod_Part, *GMethod_Part;

#endif
