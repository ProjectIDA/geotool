
#ifndef _LIBGEOG_H_
#define _LIBGEOG_H_

#include	"aesir.h"	/* For Proto definition. */

extern int azimuth_cross_pt(double	alat1,
			    double	alon1,
			    double	aza,
			    double	alat2,
			    double	alon2,
			    double	azb,
			    double	*dista,
			    double	*distb,
			    double	*alat,
			    double	*alon);

extern void azimuth_cross_pt_(double	*flat1,
			      double	*flon1,
			      double	*fza,
			      double	*flat2,
			      double	*flon2,
			      double	*fzb,
			      double	*dista,
			      double	*distb,
			      double	*alat,
			      double	*alon,
			      int     *ierr);

extern void dist_azimuth(double	alat1,
			 double	alon1,
			 double	alat2,
			 double	alon2,
			 double	*delta,
			 double	*azi,
			 double	*baz,
			 int	phase_index);

extern void dist_azimuth_(double	*flat1,
			  double	*flon1,
			  double	*flat2,
			  double	*flon2,
			  double	*delta,
			  double	*azi,
			  double	*baz);

extern int read_ellip_corr_tables(char	*dir_prefix,
				  char	**phase_list,
				  int	num_phases);

extern double get_ellip_corr_from_table(double	delta,
					double	esaz,
					double	ev_geoc_co_lat,
					double	ev_depth,
					char	*phase);

extern double ellipticity_corr(double	delta,
			       double	esaz,
			       double	ecolat,
			       double	depth,
			       char	*phase);

extern int in_polygon(double	sample_lat,
		      double	sample_lon,
		      double	poly_data[][2],
		      int	num_poly_pairs);

extern void lat_lon(double alat1,
		    double	alon1,
		    double	delta,
		    double	azi,
		    double	*alat2,
		    double	*alon2);

extern void lat_lon_(double	*flat1,
		     double	*flon1,
		     double	*delta,
		     double	*azi,
		     double	*flat2,
		     double	*flon2);

extern double lat_conv(double	lat,
		       Bool	geog_to_geoc,
		       Bool	in_deg,
		       Bool	out_deg,
		       Bool	in_lat,
		       Bool	out_lat);

extern int greg(int	number,
		char	*rgnname);

extern int sreg(int	number,
		char	*rgnname);

extern int nmreg(double	latit,
		 double	longit);

extern int gtos(int grn);


extern void greg_(long *, char *, long);
extern void sreg_(long *, char *, long);
extern long nmreg_(float *lat, float *lon);
extern long gtos_(int *grn);

extern int small_circle_cross_pts(double	olat1,
				  double	olon1,
				  double	olat2,
				  double	olon2,
				  double	rsmall,
				  double	rlarge,
				  double	*xlat1,
				  double	*xlon1,
				  double	*xlat2,
				  double	*xlon2);

extern int init_aoi_grid(char *file);

extern int get_aoi_info(double	lat,
			double	lon,
			Bool	*aoi,
			Bool	*high_seismicity,
			Bool	*shallow_seismicity,
			Bool	*land);

extern int get_area_of_interest_string(double	lat,
				       double	lon,
				       char	*aoi_string);

extern int ellip_dist(double lat1, double lon1, double lat2, double lon2,
		      double *dist, double *faz, double *baz, int flag);

#endif /* _LIBGEOG_H_ */
