
#ifndef _LIBLP_H_
#define _LIBLP_H_

/*
 * SccsId:  %W%	%G%"
 */

#include "aesir.h"	/* For Proto definition. */


extern int read_LP_info(char *lp_pathway);

extern double get_LP_velocity(int	ilat,
			      int	ilon,
			      double	period,
			      int	ph_index);

extern double LP_trace_ray(double	ev_lat,
			   double	ev_lon,
			   double	sta_lat,
			   double	sta_lon,
			   int	ph_index,
			   double	period);

extern double *LP_trace_rays(double	ev_lat,
			     double	ev_lon,
			     double	sta_lat,
			     double	sta_lon,
			     int	ph_index,
			     double	*period,
			     long	nperiod);

extern int get_LP_grid_index(double	*geoc_co_lat,
			     double	east_lon,
			     int	ph_index,
			     double	azimuth);

extern int get_LP_grid_index_int(int ilat,
				 int ilon,
				 int ph_index);

extern double dist_given_2angles_plus_side(double	angle_wrt_to_baz,
					   double	angle_wrt_to_azi,
					   double	side_between_angles);

extern void geoc_distaz(double	alat1,
			double	alon1,
			double	alat2,
			double	alon2,
			double	*delta,
			double	*azi,
			double	*baz,
			int	phase_index);

extern void geoc_distaz_(double	*flat1,
			 double	*flon1,
			 double	*flat2,
			 double	*flon2,
			 double	*delta,
			 double	*azi,
			 double	*baz,
			 int	*ph_index);

extern void geoc_lat_lon(double	alat1,
			 double	alon1,
			 double	delta,
			 double	azi,
			 double	*alat2,
			 double	*alon2);

extern void geoc_lat_lon_(double *flat1,
			  double *flon1,
			  double *delta,
			  double *azi,
			  double *alat2,
			  double *alon2);

extern void set_LP_vel_files(char *LR_file,
			     char *LQ_file);

extern void set_LP_grid_files(	char *LR_file,
				char *LQ_file);

extern void set_LP_grid_file(char *grid_file, char *vel_file);
extern void set_LP_comp_file(char *comp_file);

extern void add_LP_grid_file(char *grid_file, char *vel_file);
extern void add_LP_comp_file(char *comp_file);

extern int read_compiled_file(char *lp_pathway, char *LR_file, int ph_index);
extern int write_compiled_file(char *lp_pathway, char *LR_file, int ph_index);

extern double *get_LP_velocities( double lat, 
				  double lon, 
				  int ph_index, 
				  double *period, 
				  long nperiod);

extern double *get_LP_periods(int ph_index, long *nperiod);

extern void get_LP_grid_limits(int ph_index,
			       int *nlat,
			       int *nlon,
			       int *nperiod,
			       int *nmodel);

typedef struct _RAY_Data {
	int ilat;
	int ilon;
	int igrid;	/* Grid (model) number */
	double dist;	/* Distance crossed in grid cell */
	double value;	/* Velocity or other parameter in grid cell */
} RAY_Data;

extern RAY_Data *LP_trace_ray_grid(double ev_lat, 
				   double ev_lon, 
				   double sta_lat, 
				   double sta_lon,
				   int ph_index, 
				   long *ngrid,
				   double period);

#endif /* _LIBLP_H_ */


