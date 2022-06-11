#ifndef _BEAM_H
#define _BEAM_H

#include <string.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>

#include "gobject++/gvector.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "BeamSta.h"

class GTimeSeries;
class DataSource;

/** Functions for beam recipe I/O and beam computation.
 *  This class contains static functions for reading and writing beam recipes
 *  and creating waveform beams.
 *  @ingroup libgbeam
 */
class Beam
{
    public:
	Beam(const string &recipe_directory, const string &recipe_directory2)
	{
	    recipe_dir = recipe_directory;
	    recipe_dir2 = recipe_directory2;
	}
	Beam(const string &recipe_directory, const string &recipe_directory2,
	    const string &origin_recipes_selected,
	    const string &detection_recipes_selected)
	{
	    recipe_dir = recipe_directory;
	    recipe_dir2 = recipe_directory2;
	    origins_selected = origin_recipes_selected;
	    detections_selected = detection_recipes_selected;
	}

	string recipe_dir;
	string recipe_dir2;
	string origins_selected;
	string detections_selected;

	static BeamLocation getLocation(DataSource *ds,
		BeamLocation beam_location, gvector<Waveform *> &wvec,
		double *ref_lon, double *ref_lat);
	static bool getTimeLags(DataSource *ds,
		gvector<Waveform *> &wvec, double az, double slowness,
		BeamLocation beam_location, vector<double> &t_lags,
		double *beam_lat=NULL, double *beam_lon=NULL);
	static GTimeSeries *BeamTimeSeries(gvector<Waveform *> &wvec,
		vector<double> &t_lag, vector<double> &weights, bool coherent);
	static GTimeSeries *BeamTimeSeries(gvector<Waveform *> &wvec,
		vector<double> &t_lag, bool coherent)
	{
		vector<double> w;
		return BeamTimeSeries(wvec, t_lag, w, coherent);
	}
	static GTimeSeries *BeamSubSeries(gvector<Waveform *> &wvec,
		vector<double> &t_lag, vector<double> &weights,
		double tbeg, double tend, bool coherent);
	static GTimeSeries *BeamSubSeries(gvector<Waveform *> &wvec,
		vector<double> &t_lag, double tbeg, double tend, bool coherent)
	{
		vector<double> w;
		return BeamSubSeries(wvec, t_lag, w, tbeg, tend, coherent);
	}
	static bool getOrigin(const string &recipe_directory, const string &net,
		const string &phase, BeamRecipe *recipe);
	static int readFile(const string &recipe_directory, FILE *fp,
		const string &net, const char *path, int path_len,
		vector<BeamRecipe> &r);
	static int getGroup(BeamRecipe &recipe, vector<BeamSta> &beam_sta);
	static int readGroup(const string &recipe_directory, const string &net,
		const string &group, vector<BeamSta> &beam_sta);
	static int readOriginRecipes(const string &recipe_directory,
		const string &recipe_directory2, const string &selected,
		vector<BeamRecipe> &recipes);
	static int readDetectionRecipes(const string &recipe_directory,
		const string &recipe_directory2, const string &selected,
		vector<BeamRecipe> &recipes);
	static int readGroups(const string &recipe_directory,
		const string &recipe_directory2, vector<BeamGroup> &groups);
	static bool addGroup(const string &net, string &group,
		vector<BeamSta> &sta, const string &recipe_directory);
	static bool changeRecipe(BeamRecipe *recipe, bool delete_recipe);
	static bool addRecipe(const string &recipe_directory,
		BeamRecipe *recipe, bool origin_beam);
	static bool deleteGroup(BeamGroup *group);

        bool beamRecipe(const string &net, const string &chan, BeamRecipe &r);
        bool getSelectedOriginRecipe(const string &net, BeamRecipe &r);
        vector<BeamRecipe> * getOriginRecipes(bool force_read=false);
        vector<BeamRecipe> * getDetectionRecipes(bool force_read=false);

	static void ftrace(gvector<GTimeSeries *> &ts, double tmin,
		double tmax, double az, double slowness, double beam_lat,
		double beam_lon, int spts, int npols, double flow, double fhigh,
		bool zp, double snr, GTimeSeries *beam_ts, GTimeSeries *semb_ts,
		GTimeSeries *fst_ts, GTimeSeries *prob_ts);
	static void ftrace(gvector<Waveform *> &wvec, double tmin, double tmax,
		double az, double slowness, double beam_lat, double beam_lon,
		int spts, int npols, double flow, double fhigh, bool zp,
		double snr, GTimeSeries *beam_ts, GTimeSeries *semb_ts,
		GTimeSeries *fst_ts, GTimeSeries *prob_ts)
	{
	    gvector<GTimeSeries *> ts;
	    for(int i = 0; i < wvec.size(); i++) ts.push_back(wvec[i]->ts);
	    ftrace(ts, tmin, tmax, az, slowness, beam_lat, beam_lon, spts,
		npols, flow, fhigh, zp, snr, beam_ts, semb_ts, fst_ts, prob_ts);
	}
	static void ftrace(gvector<Waveform *> &wvec, double tmin, double tmax,
		vector<double > &tlags, int spts, int npols, double flow,
		double fhigh, bool zp, double snr, GTimeSeries *beam_ts,
		GTimeSeries *semb_ts, GTimeSeries *fst_ts, GTimeSeries *prob_ts)
	{
	    gvector<GTimeSeries *> ts;
	    for(int i = 0; i < wvec.size(); i++) ts.push_back(wvec[i]->ts);
	    ftrace(ts, tmin, tmax, tlags, spts, npols, flow, fhigh, zp, snr,
		beam_ts, semb_ts, fst_ts, prob_ts);
	}
	static void ftrace(gvector<GTimeSeries *> &ts, double tmin, double tmax,
		vector<double> &tlag, int spts, int npols, double flow,
		double fhigh, bool zp, double snr, GTimeSeries *beam_ts,
		GTimeSeries *semb_ts, GTimeSeries *fst_ts,GTimeSeries *prob_ts);
#ifdef HAVE_GSL
	static void filter(float *y, int npts, double tdel, int npols,
		double flow, double fhigh, bool zp);
	static void recbut(float *xr, int n, double dela, int nrdr, double lfr,
		double hfr);
	static void fstuff(int num, float *data[], int npts, double tdel,
		int spts, float snr, float lf, float hf, float *semb,
		float *fst, float *prob);
	static void shiftByFT(int npts, double *data, double t0);
#endif

    protected:
	
    private:
};
#endif
