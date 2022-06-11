#ifndef _FK_PARAM_H_
#define _FK_PARAM_H_

#include <stdio.h>
#include "gobject++/Gobject.h"
#include "gobject++/GTimeSeries.h"
#include "FKData.h"
#include "BeamSta.h"

class DataSource;
class Waveform;

extern "C" {
#include "libstalta.h"
}

namespace libgfk {

class FKGram;
class FKGram3C;

#define FK_SEC_PER_DEG  0
#define FK_SEC_PER_KM   1

/** Holds information about an FK signal. The maximum FK value and its location
 *  in terms of azimuth and slowness or apparent velocity are stored for a
 *  series of FK computations in a sliding data window.
 * @ingroup libgfk
 */
class FKSignal
{
    public:
	FKSignal(void) {
	    snr = NULL;
	    fstat = NULL;
	    appvel = NULL;
	    azimuth = NULL;
	    slowness = NULL;
	    delaz = NULL;
	    delslo = NULL;
	}
	~FKSignal(void) {
	    Free(snr);
	    Free(fstat);
	    Free(appvel);
	    Free(azimuth);
	    Free(slowness);
	    Free(delaz);
	    Free(delslo);
	}

	float	*snr;		//!< the snr value.
	float	*fstat;		//!< the fstat value.
	float	*appvel;	//!< the apparent velocity of the maximum
	float	*azimuth;	//!< the azimuth of the maximum
	float	*slowness;	//!< the slowness of the maximum
	float	*delaz;		//!< delaz
	float	*delslo;	//!< delslo
};

/**
 * @ingroup libgfk
 */
class FKParam : public Gobject
{
    public:
	FKParam(void);
	~FKParam(void);

	void findMinMax(void);
	void allocSignal(void);
	void updateSignalMeasurements(DataSource *ds, bool unwrapAz,
		BeamLocation beam_location, double stav_len, double ltav_len,
		gvector<Waveform *> &wvec);
	bool computeSnr(DataSource *ds, double time, double stav_len,
		double ltav_len, BeamLocation beam_location,
		gvector<Waveform *> &wvec,
		double az, double slow_sec_per_km, double fmin, double fmax,
		float *snr);
	static double getSnr(GTimeSeries *ts, double time, double window_len,
		double stav_len, double ltav_len);

	void freeSpace(void);


	static void azSlowToSxSy(float obsAzimuth, float obsSlowness,
		float * sx, float *sy);
	static void computeAzSlow(int fk_units, double x, double y,
		double fstat, double cfreq, float *az, float *app_vel,
		float *slowness, float *delaz, float *delslo);
	static void crosshair_to_slow_az(int fk_units, double scaled_x,
			double scaled_y, double *sec_per_km, double *az);

	char		net[10];
	char		sta[10];
	char		chan[10];
	bool		three_component;
	int		nbands;
	int		fk_units;

	int		num_fkdata;
	int		size_fkdata;
	FKData		**fkdata;
	FKData		*single_fk;
	int		num_waveforms;
	int		*waveform_ids;
	float		min_fk[MAX_NBANDS];
	float		max_fk[MAX_NBANDS];
	double		window_length;
	double		window_overlap;
	bool		beam_input;

        /* signal measurements */
        FKSignal        sig[4];
	double		*x;

	FKGram		*fg;
	FKGram3C	*fg3C;
};

} // namespace libgfk

#endif
