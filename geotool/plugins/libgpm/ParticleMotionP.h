#ifndef	_PARTICLE_MOTIONP_H_
#define	_PARTICLE_MOTIONP_H_

#include <Xm/PrimitiveP.h>

#include "ParticleMotion.h"

/** 
 * 
 */
typedef struct
{
	Pixel		fg;
	GC		gc;
	GC		eraseGC;
	XFontStruct 	*font;
	Pixmap		pixmap;
	Pixmap		pixmap2;

	char		sta[10];
	int		depth;
	int		arrow_length;
	double		x0, y0, x1, y1, r_scale;

	int		npts;
	float		*x;
	float		*y;
	float		*z;
	float		*rx;
	float		*ry;
	float		*rz;

	double		alpha;
	double		beta;
	double		gamma;

	int		nsegs;
	XSegment	*segs;

} ParticleMotionPart;

/** 
 * 
 */
typedef struct
{
	int	empty;
} ParticleMotionClassPart;

/** 
 * 
 */
typedef struct _ParticleMotionRec
{
	CorePart		core;
	XmPrimitivePart		primitive;
	ParticleMotionPart	particle_motion;
} ParticleMotionRec;

/** 
 * 
 */
typedef struct _ParticleMotionClassRec
{
	CoreClassPart		core_class;
	XmPrimitiveClassPart	primitive_class;
	ParticleMotionClassPart	particle_motion_class;
} ParticleMotionClassRec;

extern ParticleMotionClassRec particleMotionClassRec;

#endif	/* _PARTICLE_MOTIONP_H_ */
