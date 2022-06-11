#ifndef _PARTICLE_MOTION_CLASS_H
#define _PARTICLE_MOTION_CLASS_H

#include "motif++/Component.h"

extern "C" {
#include "ParticleMotion.h"
}

namespace libgpm {

class ParticleMotionClass : public Component
{
    public:

	ParticleMotionClass(const char *name, Component *parent, Arg *args,
				int n) : Component(name, parent)
	{
	    initWidget(particleMotionWidgetClass, name, parent, args, n);
	}
	ParticleMotionClass(const char *name, Component *parent, Arg *args,
		int n, ActionListener *listener) : Component(name, parent)
	{
	    initWidget(particleMotionWidgetClass, name, parent, args, n);
	}
	~ParticleMotionClass(void) {}

	void input(int npts, float *x, double x_mean, float *y, double y_mean,
		float *z, double z_mean, const char *sta)
	{
	    ParticleMotionInput(pm, npts, x, x_mean, y, y_mean, z, z_mean, sta);
	}
	void setAzimuth( double angle) {
	    ParticleMotionAzimuth(pm, angle);
	}
	void setIncidence(double angle) {
	    ParticleMotionIncidence(pm, angle);
	}
	double getAzimuth(void) {
	    return ParticleMotionGetAzimuth(pm);
	}
	double getIncidence(void) {
	    return ParticleMotionGetIncidence(pm);
	}
	void getSta(char *sta) { ParticleMotionGetSta(pm, sta); }
	void getData(int *npts, float **x, float **y, float **z, double *az,
			double *incidence, char *sta, int sta_size)
	{
	    ParticleMotionGetData(pm, npts, x, y, z, az,incidence,sta,sta_size);
	}

    protected:
	// use this constructor in a "Widget" subclass
	ParticleMotionClass(WidgetClass widget_class, const char *name,
		Component *parent, Arg *args, int n) : Component(name, parent)
	{
	    initWidget(widget_class, name, parent, args, n);
	}
	void initWidget(WidgetClass widget_class, const char *name,
			Component *parent, Arg *args,int n)
	{
	    base_widget = XtCreateManagedWidget(getName(), widget_class,
				parent->baseWidget(), args, n);
	    installDestroyHandler();
	    pm = (ParticleMotionWidget)base_widget;
	}

	ParticleMotionWidget	pm;

    private:

};

} // namespace libgpm

#endif
