#ifndef _ROTATE_DATA_H
#define _ROTATE_DATA_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;
class Waveform;

enum RotateUpdate
{
    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z,
    ROTATE_XY,
    ROTATE_XYZ
};


/** A DataMethod subclass that applies a rotation two horizontal components.
 *  @ingroup libgmethod
 */
class RotateData : public DataMethod
{
    public:
	RotateData(double alpha_deg, double beta_deg, double gamma_deg,
		GTimeSeries *x_ts, GTimeSeries *y_ts, GTimeSeries *z_ts);
	RotateData(double azimuth, GTimeSeries *x_ts, GTimeSeries *y_ts);
	RotateData(double azimuth, GTimeSeries *x_ts, GTimeSeries *y_ts,
		GTimeSeries *z_ts);
	RotateData(double azimuth, double incidence, GTimeSeries *x_ts,
		GTimeSeries *y_ts, GTimeSeries *z_ts);
	RotateData(const RotateData &r);
	RotateData & operator=(const RotateData &r);
	~RotateData(void);

	const char *toString(void);
	Gobject *clone(void);

	virtual RotateData *getRotateDataInstance(void) { return this; }

	bool rotationCommutative(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	/** Get the rotation angle alpha.
	 *  @returns the rotation angle alpha in degrees.
	 */
	double getAlpha() { return alpha; }
	/** Get the rotation angle beta.
	 *  @returns the rotation angle beta in degrees.
	 */
	double getBeta() { return beta; }
	/** Get the rotation angle gamma.
	 *  @returns the rotation angle gamma in degrees.
	 */
	double getGamma() { return gamma; }

	static void rotate(GTimeSeries *ts_x, GTimeSeries *ts_y,
		GTimeSeries *ts_z, double alpha, double beta, double gamma,
		RotateUpdate update);
	static void rotate(float *x, float *y, int npts, double angle);
	static void rotate(float *x, float *y, float *z, int npts,
		double sta_alpha, double sta_beta, double sta_gamma,
		double alpha, double beta, double gamma);
	static double maxAngle(gvector<GTimeSeries *> &ts, double tmin,
		double tmax);
	static void maxAngles(gvector<GTimeSeries *> &ts, double tmin,
		double tmax, double *theta, double *phi);
	static int checkComponents(bool set_hang, double incidence,
		vector<int> &ncmpts, gvector<Waveform *> &wvec, string &errmsg);
	static bool getHang(GTimeSeries *ts, bool set_hang, double *hang,
		double *vang);
	static bool rotateWaveforms(double rotate_to_azimuth, Waveform *e_w,
		Waveform *n_w, string &errmsg);
	static bool rotateWaveforms(double rotate_to_azimuth,
		double rotate_to_incidence, Waveform *e_w, Waveform *n_w,
		Waveform *z_w, string &errmsg);

    protected:
	double	alpha; //!< the Euler rotation angle (degrees)
	double	beta; //!< the Euler rotation angle (degrees)
	double	gamma; //!< the Euler rotation angle (degrees)
	GTimeSeries *ts_x; //!< a horizontal component
	GTimeSeries *ts_y; //!< a horizontal component
	GTimeSeries *ts_z; //!< a vertical component

    private:

};

#endif
