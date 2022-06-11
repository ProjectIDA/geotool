#ifndef	_TRAVEL_TIME_H_
#define	_TRAVEL_TIME_H_

#include <stdlib.h>
#include <string.h>
#include <string>
using namespace std;

/// @cond

#include "gobject++/CssTables.h"
extern "C" {
#include "crust.h"
#include "libtime.h"
#include "libgplot.h"
typedef int (*ComputeTTMethod)(double origin_time, double origin_lat,
		double origin_lon, double origin_depth, double delta,
		double esaz, const char *net, double lat, double lon,
		double elev, int num_phases, const char **phases, double *tt,
		double *slowness);
}

/** A class for all travel times for a station and source.
 *  @ingroup libgmethod
 */
class TrTm
{
    public:
	TrTm(void);
	~TrTm(void);

	const char *iaspeiPhase(int i);
	const char *jbPhase(int i);
	const char *regionalPhase(int i);
	const char *surfacePhase(int i);

	double iaspeiTime(const string &phase);
	double jbTime(const string &phase);
	double regionalTime(const string &phase);
	double surfaceTime(const string &phase);

	double tt(const string &phase) {
	    double d;
	    if((d = iaspeiTime(phase)) > 0.) return d;
	    if((d = jbTime(phase)) > 0.) return d;
	    if((d = regionalTime(phase)) > 0.) return d;
	    if((d = surfaceTime(phase)) > 0.) return d;
	    return -1.;
	}

	// slowness units are seconds/degree
	double iaspeiSlowness(const string &phase);
	double jbSlowness(const string &phase);
	double regionalSlowness(const string &phase);
	double surfaceSlowness(const string &phase);

	double slowness(const string &phase) {
	    double d;
	    if((d = iaspeiSlowness(phase)) > 0.) return d;
	    if((d = jbSlowness(phase)) > 0.) return d;
	    if((d = regionalSlowness(phase)) > 0.) return d;
	    if((d = surfaceSlowness(phase)) > 0.) return d;
	    return -1.;
	}

	int num_iaspei;
	int num_jb;
	int num_regional;
	int num_surface;

	char	net[10];
	char	sta[10];
	double	lat;
	double	lon;
	double	elev;
	double	delta;
	double	az;
	double	baz;
	double	otime;
	double	olat;
	double	olon;
	double	depth;
	double	infra_tt;

	float *iaspei_tt;
	float *iaspei_slow;
	float *jb_tt;
	float *jb_slow;
	float *regional_tt;
	float *regional_slow;
	float *surface_tt;
	float *surface_slow;
};

/** A class for computing travel times.
 *  @ingroup libgmethod
 */
class TravelTime
{
    public:
	TravelTime(const string &iaspei_file="", const string &jb_file="",
			ComputeTTMethod method=NULL);
	~TravelTime(void);

	// the origin time is used in these routines only to call
	// set_epoch_travel_time(origin_time). The origin time is only used
	// to compute seasonal corrections to acoustic travel times.
	// If no acoustic times are needed, the origin time doesn't matter

	double getTravelTime(const string &phase, double otime, double olat,
		double olon, double depth, double lat, double lon, double elev,
		const string &net, const string &sta, double *slowness,
		string &op);
	double getTravelTime(const string &phase, double olat, double olon,
		double depth, double lat, double lon, double elev,
		const string &net, const string &sta, double *slowness,
		string &op)
	{
	    return getTravelTime(phase, timeGetEpoch(), olat, olon, depth,
		lat, lon, elev, net, sta, slowness, op);
	}
	double getTravelTime(const string &phase, CssOriginClass *origin,
		double lat, double lon, double elev, const string &net,
		const string &sta, double *slowness, string &op)
	{
	    return getTravelTime(phase, origin->time, origin->lat, origin->lon,
		origin->depth, lat, lon, elev, net, sta, slowness, op);
	}
	bool firstArrival(double otime, double olat, double olon, double depth,
		double lat, double lon, double elev, const string &net,
		const string &sta, char type, string &phase,
		double *time, double *slowness);
	bool firstArrival(double olat, double olon, double depth, double lat,
		double lon, double elev, const string &net, const string &sta,
		char type, string &phase, double *time, double *slowness)
	{
	    return firstArrival(timeGetEpoch(), olat, olon, depth, lat, lon,
			elev, net, sta, type, phase, time, slowness);
	}
	bool firstArrival(CssOriginClass *origin, double lat, double lon,
		double elev, const string &net, const string &sta, char type,
		string &phase, double *time, double *slowness)
	{
	  if (origin == NULL) return False;
	  else return firstArrival(origin->time, origin->lat, origin->lon,
				   origin->depth, lat, lon, elev, net, sta,
				   type, phase, time, slowness);
	}
	void getAllTimes(TrTm *tr, double otime, double olat, double olon,
		double depth, double lat, double lon, double elev,
		const string &net, const string &sta);
	void getAllTimes(TrTm *tr, CssOriginClass *origin, double lat, double lon,
		double elev, const string &net, const string &sta)
	{
	    getAllTimes(tr, origin->time, origin->lat, origin->lon,
			origin->depth, lat, lon, elev, net, sta);
	}
	void setComputeMethod(ComputeTTMethod method) {
	    compute_tt_method = method;
	}

	void setLgVelocity(double lgvel);
	void setLqVelocity(double lqvel);
	void setLrVelocity(double lrvel);
	void setRgVelocity(double rgvel);
	void setTVelocity(double tvel);
	void setCelerity(double celerity);
	void setUseCelerity(bool use);
	void setStopPdiff(double max_degrees) { stop_Pdiff = max_degrees; }

	bool setCrust(CrustModel *crust_model);
	CrustModel getCrust(void) { return crust; }

	double getLgVelocity(void) { return lg_vel; }
	double getLqVelocity(void) { return lq_vel; }
	double getLrVelocity(void) { return lr_vel; }
	double getRgVelocity(void) { return rg_vel; }
	double getTVelocity(void) { return t_vel; }
	double getCelerity(void) { return infra_vel; }
	bool useCelerity(void) { return use_celerity; }
	double getStopPdiff(void) { return stop_Pdiff; }

	bool iaspei(const string &phase_name, float delta, float depth,
		float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp);
	const char *getJBFile(void);
	const char *getIaspeiPrefix(void);

	static int getIaspeiPhases(const char ***phase_list);
	static int phaseToCode(const string &phase);
//	static const char *getPhase(int code);
	static const char *iaspeiPhase(int i);
	static const char *jbPhase(int i);
	static const char *regionalPhase(int i);
	static const char *surfacePhase(int i);
	static bool openjb(const string &jb_file);
	static bool openIaspei(const string &iaspei_prefix);

	int num_iaspei;
	int num_jb;
	int num_regional;
	int num_surface;

    protected:
	double		stop_Pdiff;
	double		lg_vel;
	double		lq_vel;
	double		lr_vel;
	double		rg_vel;
	double		t_vel;
	double		infra_vel;
	double		infra_tt;
	float		last_depth;
	CrustModel	crust;
	string		jb_table;
	string		iaspei_table;

	bool		jb_first_warn;
	bool		use_celerity;

	TrTm		last_trtm;

	ComputeTTMethod compute_tt_method;

	void setIaspeiDepth(float depth);
	void initTr(TrTm *tr, double otime, double olat, double olon,
		double depth, double lat, double lon, double elev,
		const string &net, const string &sta);
	void getTimes(TrTm *lt);
	void getRegional(TrTm *tr);
	void getSurface(TrTm *tr);
};

/// @endcond
#endif
