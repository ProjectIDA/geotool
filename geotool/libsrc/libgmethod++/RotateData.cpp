/** \file RotateData.cpp
 *  \brief Defines class RotateData.
 *  \author Ivan Henson
 */
#include "config.h"
#include <math.h>
#include "RotateData.h"
#include "Waveform.h"
#include "motif++/Component.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GCoverage.h"
#include "gobject++/GArray.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


/** Constructor. Create a DataMethod that will convert three component data to
 *  a rotated coordinate system. The Euler angles alpha, beta, and gamma rotate
 *  the coordinate axis from the original system to the new system. The
 *  vertical component ts_z can be NULL if beta is zero, in which case only
 *  the horizontal components are rotated.
 *  @param[in] alpha_deg the rotation Euler angle (degrees).
 *  @param[in] beta_deg the rotation Euler angle (degrees).
 *  @param[in] gamma_deg the rotation Euler angle (degrees).
 *  @param[in] x_ts the x horizontal component (normally East).
 *  @param[in] y_ts the y horizontal component (normally North).
 *  @param[in] z_ts the other vertical component or NULL.
 */
RotateData::RotateData(double alpha_deg, double beta_deg, double gamma_deg,
		GTimeSeries *x_ts, GTimeSeries *y_ts, GTimeSeries *z_ts)
		: DataMethod("RotateData"),
		alpha(alpha_deg), beta(beta_deg), gamma(gamma_deg),
		ts_x(x_ts), ts_y(y_ts), ts_z(z_ts)
{
    ts_x->addOwner(this);
    ts_y->addOwner(this);
    if( !z_ts ) {
	if(beta != 0.) {
	    logErrorMsg(LOG_WARNING,
		"RotateData: ts_z=null with non-null beta.");
	    beta = 0.;
	}
	ts_z = NULL;
    }
    else {
	ts_z = z_ts;
	ts_z->addOwner(this);
    }
}

/** Constructor. Create a DataMethod that will convert two horizontal components
 *  to a rotated coordinate system. The angle alpha is counter-clockwise.
 *  @param[in] azimuth the rotation azimuth (angle from North to the x axis,
 *		clockwise, in degrees).
 *  @param[in] x_ts the x horizontal component (normally East).
 *  @param[in] y_ts the y horizontal component (normally North).
 */
RotateData::RotateData(double azimuth, GTimeSeries *x_ts, GTimeSeries *y_ts)
		: DataMethod("RotateData"),
		alpha(90.0-azimuth), beta(0.), gamma(0.),
		ts_x(x_ts), ts_y(y_ts), ts_z(NULL)
{
    ts_x->addOwner(this);
    ts_y->addOwner(this);
}

/** Constructor. Create a DataMethod that will convert two horizontal components
 *  to a rotated coordinate system. The angle alpha is counter-clockwise.
 *  @param[in] azimuth the rotation azimuth (angle from North to the x axis,
 *		clockwise, in degrees).
 *  @param[in] x_ts the x horizontal component (normally East).
 *  @param[in] y_ts the y horizontal component (normally North).
 *  @param[in] z_ts the z vertical component.
 */
RotateData::RotateData(double azimuth, GTimeSeries *x_ts, GTimeSeries *y_ts,
			GTimeSeries *z_ts) : DataMethod("RotateData"),
		alpha(90.0-azimuth), beta(0.), gamma(0.),
		ts_x(x_ts), ts_y(y_ts), ts_z(z_ts)
{
    ts_x->addOwner(this);
    ts_y->addOwner(this);
    ts_z->addOwner(this);
}

/** Constructor. Create a DataMethod that will convert two horizontal components
 *  to a rotated coordinate system. The angle alpha is counter-clockwise.
 *  @param[in] azimuth the rotation azimuth (angle from North to the x axis,
 *		clockwise, in degrees).
 *  @param[in] incidence the rotation incidence (angle from vertical to the
 *		x axis, in degrees).
 *  @param[in] x_ts the x horizontal component (normally East).
 *  @param[in] y_ts the y horizontal component (normally North).
 *  @param[in] z_ts the z vertical component.
 */
RotateData::RotateData(double azimuth, double incidence, GTimeSeries *x_ts,
			GTimeSeries *y_ts, GTimeSeries *z_ts)
			: DataMethod("RotateData"),
		alpha(90.0-azimuth), beta(incidence-90.0), gamma(0.),
		ts_x(x_ts), ts_y(y_ts), ts_z(z_ts)
{
    ts_x->addOwner(this);
    ts_y->addOwner(this);
    ts_z->addOwner(this);
}

RotateData::RotateData(const RotateData &r) : DataMethod(r),
		alpha(r.alpha), beta(r.beta), gamma(r.gamma),
		ts_x(r.ts_x), ts_y(r.ts_y), ts_z(r.ts_z)
{
    ts_x->addOwner(this);
    ts_y->addOwner(this);
    if(ts_z) ts_z->addOwner(this);
}

RotateData & RotateData::operator=(const RotateData &r)
{
    if(this == &r) return *this;

    ts_x->removeOwner(this);
    ts_y->removeOwner(this);
    if(ts_z) ts_z->removeOwner(this);

    alpha = r.alpha;
    beta = r.beta;
    gamma = r.gamma,
    ts_x = r.ts_x;
    ts_y = r.ts_y;
    ts_z = r.ts_z;
    ts_x->addOwner(this);
    ts_y->addOwner(this);
    if(ts_z) ts_z->addOwner(this);
    return *this;
}

Gobject * RotateData::clone()
{
    return (Gobject *) new RotateData(alpha, beta, gamma, ts_x, ts_y, ts_z);
}

/** Destructor. */
RotateData::~RotateData(void)
{
    ts_x->removeOwner(this);
    ts_y->removeOwner(this);
    if(ts_z) ts_z->removeOwner(this);
}

bool RotateData::applyMethod(int n, GTimeSeries **ts)
{
    if(n <= 0) return true;

    if(ts == NULL) {
	logErrorMsg(LOG_WARNING, "RotateData.apply: ts=NULL");
	return false;
    }

    for(int i = 0; i < n; i++)
    {
	if(ts[i] == ts_x) {
	    rotate(ts_x, ts_y, ts_z, alpha, beta, gamma, ROTATE_X);
	}
	else if(ts[i] == ts_y) {
	    rotate(ts_x, ts_y, ts_z, alpha, beta, gamma, ROTATE_Y);
	}
	else if(ts[i] == ts_z) {
	    rotate(ts_x, ts_y, ts_z, alpha, beta, gamma, ROTATE_Z);
	}
	else {
	    logErrorMsg(LOG_WARNING,
		"RotateData.apply: ts is not a component.");
	    return false;
	}
    }
    return true;
}

/** Rotate a set of TimeSeries. This function has the option to change the
 *  data in only one of the components. The Euler angles alpha, beta, and gamma
 *  rotate the coordinate axis from the E,N,UP system to the new system. The
 *  update argument indicates which component or components are actually changed
 *  by the rotation. ts_z can be NULL if beta == 0.
 *  @param[in,out] ts_x The x component TimeSeries (normally East).
 *  @param[in,out] ts_y The y component TimeSeries (normally North).
 *  @param[in,out] ts_z The z component TimeSeries (normally Vertical).
 *  @param[in] alpha Euler angle
 *  @param[in] beta Euler angle
 *  @param[in] gamma Euler angle
 *  @param[in] update ROTATE_X, ROTATE_Y, ROTATE_Z, ROTATE_XY, ROTATE_XYZ.
 */
void RotateData::rotate(GTimeSeries *ts_x, GTimeSeries *ts_y, GTimeSeries *ts_z,
		double alpha, double beta, double gamma, RotateUpdate update)
{	    
    GCoverage *o;
    int i;
    double x_mean, y_mean, z_mean=0., a_mean, calib, calper;
    gvector<GTimeSeries *> ts;
	
    ts.push_back(new GTimeSeries());
    ts.push_back(new GTimeSeries());
    if(ts_z) {
	ts.push_back(new GTimeSeries());
    }

    int n = 0;
    calib = 0.;
    calper = 0.;
    for(i = 0; i < ts_x->size(); i++) {
	if(i == 0) {
	    calib = ts_x->segment(0)->calib();
	    calper = ts_x->segment(0)->calper();
	    n++;
	}
	ts[0]->addSegment(ts_x->segment(i));
    }

    for(i = 0; i < ts_y->size(); i++) {
	if(i == 0) { calib += ts_y->segment(0)->calib(); n++; }
	ts[1]->addSegment(ts_y->segment(i));
    }

    if(ts_z) {
	for(i = 0; i < ts_z->size(); i++) {
	    if(i == 0) { calib += ts_z->segment(0)->calib(); n++; }
	    ts[2]->addSegment(ts_z->segment(i));
	}
    }

    calib /= (double)n; // use the average calib for the rotated segments

    // get the current orientation
    double cur_alpha=0., cur_beta=0., cur_gamma=0.;

    if(update == ROTATE_X || update == ROTATE_XY || update == ROTATE_XYZ) {
	cur_alpha = ts_x->currentAlpha();
	cur_beta = ts_x->currentBeta();
	cur_gamma = ts_x->currentGamma();
	ts_x->removeAllSegments();
    }
    if(update == ROTATE_Y || update == ROTATE_XY || update == ROTATE_XYZ) {
	cur_alpha = ts_y->currentAlpha();
	cur_beta = ts_y->currentBeta();
	cur_gamma = ts_y->currentGamma();
	ts_y->removeAllSegments();
    }
    if(ts_z && (update == ROTATE_Z || update == ROTATE_XYZ)) {
	cur_alpha = ts_z->currentAlpha();
	cur_beta = ts_z->currentBeta();
	cur_gamma = ts_z->currentGamma();
	ts_z->removeAllSegments();
    }
    if(cur_alpha < -900.) cur_alpha = 0.;
    if(cur_beta < -900.) cur_beta = 0.;
    if(cur_gamma < -900.) cur_gamma = 0.;

    x_mean = ts[0]->mean();
    y_mean = ts[1]->mean();
    if( !ts_z ) {
	a_mean = (x_mean + y_mean)/2.;
    }
    else {
	z_mean = ts[2]->mean();
	a_mean = (x_mean + y_mean + z_mean)/3.;
    }

    for(o = new GCoverage(ts); o && o->hasMoreElements(); )
    {
	GArray *array = (GArray *)o->nextElement();
	int npts = array->length();
	
	for(i = 0; i < npts; i++) array->data[0][i] -= x_mean;
	for(i = 0; i < npts; i++) array->data[1][i] -= y_mean;

	if( !ts_z ) {
	    rotate(array->data[0], array->data[1], npts,
			-(cur_alpha-cur_gamma) + alpha-gamma);
	}
	else {
	    for(i =0; i < npts; i++) array->data[2][i] -= z_mean;
	    rotate(array->data[0], array->data[1], array->data[2],
			npts, cur_alpha, cur_beta, cur_gamma,
			alpha, beta, gamma);
	}

	if(update == ROTATE_X || update == ROTATE_XY || update == ROTATE_XYZ) {
	    // add the average mean
	    for(i=0; i < npts; i++) array->data[0][i] += a_mean;
	    ts_x->addSegment(new GSegment(array->data[0], npts,
				array->tbeg(), array->tdel(), calib, calper));
	    ts_x->setCurrentAlpha(alpha);
	    ts_x->setCurrentBeta(beta);
	    ts_x->setCurrentGamma(gamma);
	}
	if(update == ROTATE_Y || update == ROTATE_XY || update == ROTATE_XYZ) {
	    // add the average mean
	    for(i=0; i < npts; i++) array->data[1][i] += a_mean;
	    ts_y->addSegment(new GSegment(array->data[1], npts,
				array->tbeg(), array->tdel(), calib, calper));
	    ts_y->setCurrentAlpha(alpha);
	    ts_y->setCurrentBeta(beta);
	    ts_y->setCurrentGamma(gamma);
	}
	if(ts_z && (update == ROTATE_Z || update == ROTATE_XYZ)) {
	    // add the average mean
	    for(i=0; i < npts; i++) array->data[2][i] += a_mean;
	    ts_z->addSegment(new GSegment(array->data[2], npts,
				array->tbeg(), array->tdel(), calib, calper));
	    ts_z->setCurrentAlpha(alpha);
	    ts_z->setCurrentBeta(beta);
	    ts_z->setCurrentGamma(gamma);
	}
	array->deleteObject();
    }
    o->deleteObject();
}

/** Rotate a pair of float arrays.
 *  @param[in] x The East-West component.
 *  @param[in] y The North-South component.
 *  @param[in] npts The length of x[] and y[].
 *  @param[in] angle The rotation angle in degrees counter-clockwise.
 */
void RotateData::rotate(float *x, float *y, int npts, double angle)
{
    double  rangle, sin_rangle, cos_rangle;
    double  xx, yy;

    /* angle is the angle from the old x-axis to the new x-axis.
     * a positive angle is a counter-clockwise rotation.
     */

    rangle = angle*M_PI/180.0;
    sin_rangle = sin(rangle);
    cos_rangle = cos(rangle);

    for(int i = 0; i < npts; i++)
    {
	xx =   x[i]*cos_rangle + y[i]*sin_rangle;
	yy =  -x[i]*sin_rangle + y[i]*cos_rangle;
	x[i] = xx;
	y[i] = yy;
    }
}

/** Rotate three orthgonal arrays.
 *  @param[in] x The East component.
 *  @param[in] y The North component.
 *  @param[in] z The Vertical component.
 *  @param[in] npts The length of x[], y[], and z[].
 *  @param[in] cur_alpha The Euler angle from the E,N,UP to the current
 *  @param[in] cur_beta The Euler angle from the E,N,UP to the current
 *  @param[in] cur_gamma The Euler angle from the E,N,UP to the current
 *  @param[in] alpha The Euler angle from the E,N,UP to the new system.
 *  @param[in] beta The Euler angle from the E,N,UP to the new system.
 *  @param[in] gamma The Euler angle from the E,N,UP to the new system.
 */
void RotateData::rotate(float *x, float *y, float *z, int npts,
	double cur_alpha, double cur_beta, double cur_gamma,
	double alpha, double beta, double gamma)
{
    double a, b, g, sina, sinb, sing, cosa, cosb, cosg;
    double xx, yy, zz;
    double c[3][3], d[3][3], e[3][3];

    // first rotate from the current system to the E,N,UP coordinate system
    a = -cur_gamma*M_PI/180.0;
    b = -cur_beta*M_PI/180.0;
    g = -cur_alpha*M_PI/180.0;

    sina = sin(a);
    cosa = cos(a);
    sinb = sin(b);
    cosb = cos(b);
    sing = sin(g);
    cosg = cos(g);

    c[0][0] = cosa*cosb*cosg - sina*sing;
    c[0][1] = sina*cosb*cosg + cosa*sing;
    c[0][2] = -sinb*cosg;

    c[1][0] = -cosa*cosb*sing - sina*cosg;
    c[1][1] = -sina*cosb*sing + cosa*cosg;
    c[1][2] = sinb*sing;

    c[2][0] = cosa*sinb;
    c[2][1] = sina*sinb;
    c[2][2] = cosb;

    // rotate from E,N,UP to the new system.
    a = alpha*M_PI/180.0;
    b = beta*M_PI/180.0;
    g = gamma*M_PI/180.0;

    sina = sin(a);
    cosa = cos(a);
    sinb = sin(b);
    cosb = cos(b);
    sing = sin(g);
    cosg = cos(g);

    d[0][0] = cosa*cosb*cosg - sina*sing;
    d[0][1] = sina*cosb*cosg + cosa*sing;
    d[0][2] = -sinb*cosg;

    d[1][0] = -cosa*cosb*sing - sina*cosg;
    d[1][1] = -sina*cosb*sing + cosa*cosg;
    d[1][2] = sinb*sing;

    d[2][0] = cosa*sinb;
    d[2][1] = sina*sinb;
    d[2][2] = cosb;

    // multiply the two rotations.  e[] = d[] * c[]
    for(int i = 0; i < 3; i++) {
	for(int j = 0; j < 3; j++) {
	    double sum = 0.;
	    for(int k = 0; k < 3; k++) sum += d[i][k] * c[k][j];
	    e[i][j] = sum;
	}
    }

    for(int i = 0; i < npts; i++)
    {
	xx =   x[i]*e[0][0] + y[i]*e[0][1] + z[i]*e[0][2];
	yy =   x[i]*e[1][0] + y[i]*e[1][1] + z[i]*e[1][2];
	zz =   x[i]*e[2][0] + y[i]*e[2][1] + z[i]*e[2][2];
	x[i] = xx;
	y[i] = yy;
	z[i] = zz;
    }
}

/* 
 * Find the horizontal rotation angle (clockwise) which maximizes the
 * radial-component.
 * If x' and y' are the rotated coordinates, then
 * 	x'[i] = cos(theta)*x[i] - sin(theta)*y[i];
 * 	y'[i] = sin(theta)*x[i] + cos(theta)*y[i];
 *
 *	maximize { sum( y'[i] * y'[i] ) - sum(x'[i] * x'[i]) }
 *
 *	x'*x' = cos*cos*x*x + sin*sin*y*y - 2*sin*cos*x*y
 *	y'*y' = sin*sin*x*x + cos*cos*y*y + 2*sin*cos*x*y
 *
 *	y'*y' - x'*x' = cos(2*theta)*(y*y-x*x) + 2*sin(2*theta)*x*y
 *
 *	partial derivative = -2*sin(2*theta)*(y*y-x*x) + 4*cos(2*theta)*x*y = 0
 *
 *		tan(2*theta) = 2*sum(x[i]*y[i])/sum(y[i]*y[i]-x[i]*x[i])
 *
 *	max_theta = .5*atan2(2*sum(x[i]*y[i]), sum(y[i]*y[i]-x[i]*x[i]))
 */
/** Find the rotation angle (clockwise) which maximizes the radial-component.
 *  There is an analytical solution to this problem. If \f$x_i\f$ and
 *  \f$y_i\f$ are the East and North components and \f$\theta\f$ is the
 *  rotation angle, then the rotated components \f$r_i\f$ and
 *  \f$t_i\f$ are
\f{eqnarray*}
r_i &=& sin(\theta)*x_i + cos(\theta)*y_i \\
t_i &=& cos(\theta)*x_i - sin(\theta)*y_i
\f}
    The maximization of the difference between the radial-component signal
    power and the transverse-component signal power over a time window can be
    expressed as the maximization of the function \f$f(\theta)\f$
\f{eqnarray*}
	f(\theta) &=& \sum_i ( r_i )^2 - \sum_i ( t_i )^2
\f}
    Using 
\f{eqnarray*}
	( t_i )^2 &=& cos^{2}(\theta)*{x_i}^2 +
	sin^{2}(\theta)*{y_i}^{2} - 2*sin(\theta)*cos(\theta)*x_i*y_i \\
	( r_i )^2 &=& sin^{2}(\theta)*{x_i}^2 +
	cos^{2}(\theta)*{y_i}^2 + 2*sin(\theta)*cos(\theta)*x_i*y_i,
\f}
   the formula for \f$f(\theta)\f$ simplifies to
\f{eqnarray*}
	f(\theta) &=& \sum_i cos(2\theta)*(y_i*y_i-x_i*x_i) +
			2*sin(2\theta)*x_i*y_i
\f}
The derivative of \f$f\f$ with respect to the rotation angle \f$\theta\f$ is
\f{eqnarray*}
	\frac{df(\theta)}{d\theta} &=& \sum_i -2*sin(2\theta)*({y_i}^2-{x_i}^2)
		 + 4*cos(2\theta)*x_i*y_i
\f}
   Setting this derivative equal to zero gives the solution for the rotation
   angle that maximizes the difference between the signal powers of the radial
   component and the transverse component
\f{eqnarray*}
	\theta &=& .5*tan^{-1} \left( \frac{2*\sum_i {(x_i*y_i)}}
		{\sum_i {({y_i}^2 - {x_i}^2})} \right)
\f}
 *  
 *  @param[in] ts an array of two GTimeSeries objects, ordered ts[0]=East and
 *		ts[1]=North.
 *  @param[in] tmin the beginning time of the data window.
 *  @param[in] tmax the end time of the data window.
 *  @returns the computed rotation angle (clockwise from North) that
 *		maximizes the power of ts[1] between tmin and tmax.
 */
double RotateData::maxAngle(gvector<GTimeSeries *> &ts, double tmin,double tmax)
{
    double sum1, sum2, max_theta;
    double e_mean = ts[0]->mean();
    double n_mean = ts[1]->mean();
    GCoverage *o;

    max_theta = 0.;
    sum1 = sum2 = 0;

    for(o = new GCoverage(ts, tmin, tmax); o->hasMoreElements();)
    {
	GArray *array = (GArray *)o->nextElement();
	float *x = array->data[0];
	float *y = array->data[1];
	int npts = array->length();
	for(int i = 0; i < npts; i++) x[i] -= e_mean;
	for(int i = 0; i < npts; i++) y[i] -= n_mean;

	for(int i = 0; i < npts; i++)
	{
	    sum1 += x[i]*y[i];
	    sum2 += (y[i]*y[i] - x[i]*x[i]);
	}
	array->deleteObject();
    }
    o->deleteObject();

    if(sum1 == 0. && sum2 == 0.) {
	max_theta = 0.;
    }
    else {
	max_theta = .5*atan2(2*sum1, sum2);
    }
    max_theta *= 180./M_PI;
    return max_theta;
}

/* 
 *  Find the horizontal rotation angle (clockwise from North towards East)
 *  which maximizes the radial-component and find the second vertical rotation
 *  angle (from z towards the radial (new y) component) that maximizes the z
 *  component.
 *  @param[in] ts an array of three GTimeSeries objects, ordered ts[0]=East,
 *	ts[1]=North and ts[2]=Up.
 *  @param[in] tmin the beginning time of the data window.
 *  @param[in] tmax the end time of the data window.
 *  @paran[out] theta the computed horizontal rotation angle in (clockwise from
 *		North) that maximizes the power of ts[1] between tmin and tmax.
 *  @paran[out] phi the computed vertical rotation angle (clockwise from Up)
 *		that maximizes the power of ts[2] between tmin and tmax.
 */
void RotateData::maxAngles(gvector<GTimeSeries *> &ts, double tmin, double tmax,
			double *theta, double *phi)
{
    double theta_sum1, theta_sum2, max_theta, phi_sum1, phi_sum2, max_phi;
    double e_mean = ts[0]->mean();
    double n_mean = ts[1]->mean();
    double z_mean = ts[2]->mean();
    GCoverage *o;

    max_theta = 0.;
    theta_sum1 = theta_sum2 = 0;

    for(o = new GCoverage(ts, tmin, tmax); o->hasMoreElements();)
    {
	GArray *array = (GArray *)o->nextElement();
	int npts = array->length();
	float *x = array->data[0];
	float *y = array->data[1];

	for(int i = 0; i < npts; i++) x[i] -= e_mean;
	for(int i = 0; i < npts; i++) y[i] -= n_mean;

	for(int i = 0; i < npts; i++)
	{
	    theta_sum1 += x[i]*y[i];
	    theta_sum2 += (y[i]*y[i] - x[i]*x[i]);
	}
	array->deleteObject();
    }
    o->deleteObject();

    if(theta_sum1 == 0. && theta_sum2 == 0.) {
	max_theta = 0.;
    }
    else {
	max_theta = .5*atan2(2*theta_sum1, theta_sum2);
    }

    double sn = sin(max_theta);
    double cs = cos(max_theta);

    max_phi = 0.;
    phi_sum1 = phi_sum2 = 0;

    for(o = new GCoverage(ts, tmin, tmax); o->hasMoreElements();)
    {
	GArray *array = (GArray *)o->nextElement();
	int npts = array->length();
	float *x = array->data[0];
	float *y = array->data[1];
	float *z = array->data[2];

	for(int i = 0; i < npts; i++) x[i] -= e_mean;
	for(int i = 0; i < npts; i++) y[i] -= n_mean;
	for(int i = 0; i < npts; i++) z[i] -= z_mean;

	for(int i = 0; i < npts; i++)
	{
	    double r = sn*x[i] + cs*y[i];
	    phi_sum1 += r*z[i];
	    phi_sum2 += (z[i]*z[i] - r*r);
	}
	array->deleteObject();
    }
    o->deleteObject();

    if(phi_sum1 == 0. && phi_sum2 == 0.) {
	max_phi = 0.;
    }
    else {
	max_phi = .5*atan2(2*phi_sum1, phi_sum2);
    }

/*
    if(max_phi < 0.) {
	if(max_theta > 0.) max_theta -= 180.;
	else max_theta += 180.;
	max_phi = -max_phi;
    }
    if(max_phi > 0.) {
	if(theta_sum1 == 0. && theta_sum2 == 0.) {
	    max_theta = 0.;
	}
	else {
	    max_theta = .5*atan2(-2*theta_sum1, -theta_sum2);
	}
	max_phi = -max_phi;
    }
*/
    max_theta *= 180./M_PI;
    max_phi *= 180./M_PI;

    if(max_theta > 0.) max_theta -= 180.;
    else max_theta += 180.;

    *theta = max_theta;
    *phi = max_phi;
}

const char * RotateData::toString(void)
{
    char c[100];
    // convert alpha to the station to source azimuth 
    snprintf(c, sizeof(c), "RotateData: %.2f %.2f %.2f", alpha, beta, gamma);
    string_rep.assign(c);
    return string_rep.c_str();
}

#define sameSamprates(rate1,rate2) (fabs(rate1-rate2) < rate1*0.0001)

// static
int RotateData::checkComponents(bool set_hang, double incidence,
		vector<int> &ncmpts, gvector<Waveform *> &wvec, string &errstr)
{
    char s[50];
    int i, k, m, n;
    vector<int> ncomp;
    gvector<Waveform *> cds(wvec);
    double n_hang, e_hang, n_vang, e_vang;
    double e_samprate, n_samprate;
    double alpha, beta, gamma;

    errstr.clear();
    n = 0;
    for(m = 0; m < (int)ncmpts.size(); m++)
    {
	i = n;
	n += ncmpts[m];

	getHang(wvec[i]->ts, set_hang, &e_hang, &e_vang);
	getHang(wvec[i+1]->ts, set_hang, &n_hang, &n_vang);

	if( ncmpts[m] == 2 && (e_vang != 90. || n_vang != 90.
		|| incidence != 90.) )
	{
	    snprintf(s, sizeof(s), "%s: need vertical component.",
			wvec[i]->sta());
	    if(!errstr.empty()) errstr.append("\n");
	    errstr.append(s);
	    ncomp.push_back(-ncmpts[m]);
	}
	else if( (alpha = wvec[i]->currentAlpha()) > -900. &&
	    (beta = wvec[i]->currentBeta()) > -900. &&
	    (gamma = wvec[i]->currentGamma()) > -900.)
	{
	    ncomp.push_back(ncmpts[m]);
	    continue; // good
	}
	else
	{
	    if(e_hang == -999. || n_hang == -999.) {
		snprintf(s, sizeof(s), "%s/%s, %s/%s: no hang.",
		    wvec[i]->sta(), wvec[i]->chan(),
		    wvec[i+1]->sta(), wvec[i+1]->chan());
		if(!errstr.empty()) errstr.append("\n");
		errstr.append(s);
		ncomp.push_back(-ncmpts[m]);
		continue;
	    }
	    if(e_vang != 90. || n_vang != 90.) {
		snprintf(s, sizeof(s), "%s/%s, %s/%s: vang != 90.",
			wvec[i]->sta(), wvec[i]->chan(),
			wvec[i+1]->sta(), wvec[i+1]->chan());
		if(!errstr.empty()) errstr.append("\n");
		errstr.append(s);
		ncomp.push_back(-ncmpts[m]);
		continue;
	    }

	    double ex, ey, nx, ny;

	    ex = sin(M_PI*e_hang/180.); // e_hang is measured clockwise from y
	    ey = cos(M_PI*e_hang/180.);
	    nx = sin(M_PI*n_hang/180.);
	    ny = cos(M_PI*n_hang/180.);

	    if(fabs(ex*nx + ey*ny) > .02) { // more than one degree off
		snprintf(s, sizeof(s), "%s/%s, %s/%s: right-handed orthogonal \
components only.\nn_hang: %7.2f, e_hang: %7.2f",
			wvec[i]->sta(), wvec[i]->chan(),
			wvec[i+1]->sta(), wvec[i+1]->chan(), n_hang, e_hang);
		if(!errstr.empty()) errstr.append("\n");
		errstr.append(s);
		ncomp.push_back(-ncmpts[m]);
		continue;
	    }

	    n_samprate = 1./wvec[i]->segment(0)->tdel();
	    e_samprate = 1./wvec[i+1]->segment(0)->tdel();

	    if(!sameSamprates(n_samprate, e_samprate)) {
		if(!errstr.empty()) errstr.append("\n");
		errstr.append("Variable samprate.");
		ncomp.push_back(-ncmpts[m]);
	    }
	    else {
		ncomp.push_back(ncmpts[m]);
	    }
	}
    }

    ncmpts.clear();
    wvec.clear();
    for(m = k = 0;  m < (int)ncomp.size(); m++) {
	if(ncomp[m] > 0) {
	    ncmpts.push_back(ncomp[m]);
	    for(int j = 0; j < ncomp[m]; j++) {
		wvec.push_back(cds[k+j]);
	    }
	}
	k += abs(ncomp[m]);
    }
    return ncmpts.size();
}

bool RotateData::getHang(GTimeSeries *ts, bool set_hang, double *hang,
			double *vang)
{
    char c;
    string chan;

    *hang = ts->hang();
    *vang = ts->vang();

    if( *hang < -900.)
    {
	if(set_hang)
	{
	    ts->setCurrentAlpha(0.);
	    ts->setCurrentBeta(0.);
	    ts->setCurrentGamma(0.);

	    ts->getChan(chan);
	    int j = (int)chan.length();
	    c = (j > 0) ? chan[j-1] : '\0';

	    if(c == 'n' || c == 'N') {
		*hang = 0.;
		*vang = 90.;
	    }
	    else if(c == 'e' || c == 'E') {
		*hang = 90.;
		*vang = 90.;
	    }
	    else {
		return false;
	    }
	}
	else {
	    return false;
	}
    }
    return true;
}

// static
bool RotateData::rotateWaveforms(double rotate_to_azimuth, Waveform *e_w,
				Waveform *n_w, string &errstr)
{
    int i;
    GTimeSeries *ts_x, *ts_y;
    RotateData *rotate;
    double n_samprate, e_samprate;
    gvector<DataMethod *> *methods;

    e_samprate = 1./e_w->segment(0)->tdel();
    n_samprate = 1./n_w->segment(0)->tdel();

    errstr.clear();
    if( !sameSamprates(n_samprate, e_samprate) )
    {
	errstr.assign("Variable samprate.");
	return false;
    }

    // add a Rotate method to each component

    e_w->reread();
    n_w->reread();

    ts_x = new GTimeSeries();
    for(i = 0; i < e_w->size(); i++) {
	ts_x->addSegment(e_w->segment(i));
    }
    ts_y = new GTimeSeries();
    for(i = 0; i < n_w->size(); i++) {
	ts_y->addSegment(n_w->segment(i));
    }

    rotate = new RotateData(rotate_to_azimuth, e_w->ts, ts_y);

    /* always make the rotation the first operation
     */
    methods = e_w->dataMethods();
    if((int)methods->size() > 0 && methods->at(0)->getRotateDataInstance())
    {
	methods->set(rotate, 0);
    }
    else {
	methods->insert(rotate, 0);
    }
    e_w->setDataMethods(methods);
    delete methods;

    rotate = new RotateData(rotate_to_azimuth, ts_x, n_w->ts);

    /* always make the rotation the first operation
     */
    methods = n_w->dataMethods();
    if((int)methods->size() > 0 && methods->at(0)->getRotateDataInstance())
    {
	methods->set(rotate, 0);
    }
    else {
	methods->insert(rotate, 0);
    }
    n_w->setDataMethods(methods);
    delete methods;

    DataMethod::update(e_w);
    DataMethod::update(n_w);

    if((i = (int)e_w->channel.length()) > 0) 
    {
	if(isupper((int)e_w->channel[0])) {
	    e_w->channel[i-1] = 'R';
	}
	else {
	    e_w->channel[i-1] = 'r';
	}
    }

    if((i = (int)n_w->channel.length()) > 0)
    {
	if(isupper((int)n_w->channel[0])) {
	    n_w->channel[i-1] = 'T';
	}
	else {
	    n_w->channel[i-1] = 't';
	}
    }

    return true;
}

// static
bool RotateData::rotateWaveforms(double rotate_to_azimuth,
		double rotate_to_incidence, Waveform *e_w,
		Waveform *n_w, Waveform *z_w, string &errstr)
{
    int i;
    GTimeSeries *ts_x, *ts_y, *ts_z;
    RotateData *rotate;
    double n_samprate, e_samprate, z_samprate;
    gvector<DataMethod *> *methods;

    e_samprate = 1./e_w->segment(0)->tdel();
    n_samprate = 1./n_w->segment(0)->tdel();
    z_samprate = 1./z_w->segment(0)->tdel();

    errstr.clear();
    if(!sameSamprates(n_samprate, e_samprate) ||
	!sameSamprates(n_samprate, z_samprate) )
    {
	errstr.assign("Variable samprate.");
	return false;
    }

    // add a Rotate method to each component

    e_w->reread();
    n_w->reread();
    z_w->reread();

    ts_x = new GTimeSeries();
    for(i = 0; i < e_w->size(); i++) {
	ts_x->addSegment(e_w->segment(i));
    }
    ts_y = new GTimeSeries();
    for(i = 0; i < n_w->size(); i++) {
	ts_y->addSegment(n_w->segment(i));
    }
    ts_z = new GTimeSeries();
    for(i = 0; i < z_w->size(); i++) {
	ts_z->addSegment(z_w->segment(i));
    }

    rotate = new RotateData(rotate_to_azimuth, rotate_to_incidence, e_w->ts,
				ts_y, ts_z);

    /* always make the rotation the first operation
     */
    methods = e_w->dataMethods();
    if((int)methods->size() > 0 && methods->at(0)->getRotateDataInstance())
    {
	methods->set(rotate, 0);
    }
    else {
	methods->insert(rotate, 0);
    }
    e_w->setDataMethods(methods);
    delete methods;

    rotate = new RotateData(rotate_to_azimuth, rotate_to_incidence, ts_x,
				n_w->ts, ts_z);

    /* always make the rotation the first operation
     */
    methods = n_w->dataMethods();
    if((int)methods->size() > 0 && methods->at(0)->getRotateDataInstance())
    {
	methods->set(rotate, 0);
    }
    else {
	methods->insert(rotate, 0);
    }
    n_w->setDataMethods(methods);
    delete methods;

    rotate = new RotateData(rotate_to_azimuth, rotate_to_incidence, ts_x,
				ts_y, z_w->ts);

    /* always make the rotation the first operation
     */
    methods = z_w->dataMethods();
    if((int)methods->size() > 0 && methods->at(0)->getRotateDataInstance())
    {
	methods->set(rotate, 0);
    }
    else {
	methods->insert(rotate, 0);
    }
    z_w->setDataMethods(methods);
    delete methods;

    DataMethod::update(e_w);
    DataMethod::update(n_w);
    DataMethod::update(z_w);

    if((i = (int)e_w->channel.length()) > 0) 
    {
	if(isupper((int)e_w->channel[0])) {
	    e_w->channel[i-1] = 'R';
	}
	else {
	    e_w->channel[i-1] = 'r';
	}
    }

    if((i = (int)n_w->channel.length()) > 0)
    {
	if(isupper((int)n_w->channel[0])) {
	    n_w->channel[i-1] = 'T';
	}
	else {
	    n_w->channel[i-1] = 't';
	}
    }

    if((i = (int)z_w->channel.length()) > 0)
    {
	if(isupper((int)z_w->channel[0])) {
	    z_w->channel[i-1] = 'V';
	}
	else {
	    z_w->channel[i-1] = 'v';
	}
    }

    return true;
}
