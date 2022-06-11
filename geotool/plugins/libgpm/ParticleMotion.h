#ifndef	_PARTICLE_MOTION_H_
#define	_PARTICLE_MOTION_H_

#define XtNarrowLength 			(char *)"arrowLength"

#define XtCArrowLength 			(char *)"ArrowLength"

#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

typedef struct _ParticleMotionClassRec	*ParticleMotionWidgetClass;
typedef struct _ParticleMotionRec	*ParticleMotionWidget;

extern WidgetClass		particleMotionWidgetClass;

void ParticleMotionInput(ParticleMotionWidget w, int npts, float *x,
			double x_mean, float *y, double y_mean, float *z,
			double z_mean, const char *sta);
void ParticleMotionAzimuth(ParticleMotionWidget w, double angle);
void ParticleMotionIncidence(ParticleMotionWidget w, double angle);
double ParticleMotionGetIncidence(ParticleMotionWidget w);
double ParticleMotionGetAzimuth(ParticleMotionWidget w);
void ParticleMotionGetSta(ParticleMotionWidget w, char *sta);
Widget ParticleMotionCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);
void
ParticleMotionGetData(ParticleMotionWidget w, int *npts, float **x, float **y,
			float **z, double *az, double *incidence, char *sta,
			int sta_size);

#endif	/* _PARTICLE_MOTION_H_ */
