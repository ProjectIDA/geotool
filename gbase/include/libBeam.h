/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_BEAM_H
#define	_LIB_BEAM_H

#include <sys/param.h>
#include "libgcss.h"

typedef struct
{
	char	net[20];
	char	path[MAXPATHLEN+1];
	char	name[20];
	char	beam_type[10];
	char	rot[5];
	int	std;
	double	snr;
	double	azi;
	double	slow;
	char	phase[15];
	double	flo;
	double	fhi;
	int	ford;
	int	zp;
	char	ftype[5];
	char	group[30];
	bool	selected;
} BeamRecipe;

#define BEAM_RECIPE_NULL \
{ \
	"",	/* net */ \
	"",	/* path */ \
	"",	/* name */ \
	"",	/* beam_type */ \
	"",	/* rot */ \
	-1,	/* std */ \
	-1.,	/* snr */ \
	-999.,	/* azi */ \
	-999.,	/* slow */ \
	"",	/* phase */ \
	-1.,	/* flo */ \
	-1.,	/* fhi */ \
	3,	/* ford */ \
	0,	/* zp */ \
	"",	/* ftype */ \
	"",	/*group */ \
	False,	/*group */ \
}

typedef struct
{
	char	sta[10];
	char	chan[10];
	double	wgt;
} BeamSta;

typedef struct
{
	char	net[20];
	char	group[50];
        char    path[MAXPATHLEN+1];
	int	nsta;
	BeamSta	*beamSta;
} BeamGroup;

int BeamGetLocation(int beam_location, int num_waveforms, CPlotData **cd_list,
		double *ref_lon, double *ref_lat, const char **err_msg);
double *BeamGetTimeLags(int num_waveforms, CPlotData **cd_list, double az,
		double slowness, int beam_location);
TimeSeries BeamTimeSeries(int num_waveforms, CPlotData **cd_list,
		double *t_lag, double *weights, bool coherent);
TimeSeries BeamSubSeries(int num_waveforms, CPlotData **cd_list, double *t_lag,
                double *weights, double tbeg, double tend, bool coherent);

/* ****** cssio/beamRecipe.c ********/
int BeamGetOrigin(const char *recipe_dir, const char *net, const char *phase,
		BeamRecipe *recipe, const char **err_msg);
int BeamReadFile(FILE *fp, const char *net, const char *path,
		BeamRecipe **recipes);
int BeamGetGroup(BeamRecipe *recipe, BeamSta **beam_sta, const char **err_msg);
int BeamReadGroup(const char *recipe_dir, const char *net, const char *group,
		BeamSta **beam_sta, const char **err_msg);
int BeamReadOriginRecipes(const char *recipe_dir, const char *recipe_dir2,
		BeamRecipe **recipes);
int BeamReadDetectionRecipes(const char *recipe_dir, const char *recipe_dir2,
		BeamRecipe **recipes);
int BeamReadGroups(const char *recipe_dir, const char *recipe_dir2,
		BeamGroup **groups);
int BeamAddGroup(const char *net, const char *group, int nsta, BeamSta *sta,
		const char *recipe_dir);
int BeamChangeRecipe(BeamRecipe *recipe, bool Delete);
int BeamAddRecipe(const char *recipe_dir, BeamRecipe *recipe, bool origin_beam);
int BeamDeleteGroup(BeamGroup *group);

#endif /* _LIB_BEAM_H */
