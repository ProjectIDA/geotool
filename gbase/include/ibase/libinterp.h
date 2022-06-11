
/*
 * libintero.h include file.  Prototype definitions for routines called 
 * locally and outside of library, libinterp.
 *
 * SccsId:  @(#)libinterp.h	103.1	05 Oct 1994	Copyright 1994 Science Applications International Corporation.
 */

#ifndef _LIBINTERP_H_
#define _LIBINTERP_H_

#include "aesir.h"	/* For Proto definition. */


extern int interpolate_table_value(Bool	do_extrap,
				   Bool	in_hole,
				   Bool	do_we_need_z_derivs,
				   int	nx,
				   int	nz,
				   float	*x,
				   float	*z,
				   float	**xz,
				   float	dist,
				   float	depth,
				   float	*value,
				   float	*x_1st_deriv,
				   float	*z_1st_deriv,
				   float	*x_2nd_deriv,
				   float	*z_2nd_deriv,
				   int	*interp_err);

extern void holint2_(int	*phase_id,
		     int	*do_extrap,
		     int	*m,
		     int	*n,
		     float	*x,
		     float	*y,
		     float	*f,
		     int	*ldf,
		     float	*fbad,
		     float	*x0,
		     float	*y0,
		     float	*dcalx,
		     float	*fx0,
		     float	*fy0,
		     float	*fxy0,
		     int	*idist,
		     int	*idepth,
		     int	*ihole);

extern void brack_one_vector(float	*xx,
			     int	n,
			     float	x,
			     int	*j);

extern void brack_one_dvector(double	*xx,
			      int	n,
			      double	x,
			      int	*j);

extern void splie2(float	*xs,
		   float	*xf,
		   float	**ysf,
		   int	nslow,
		   int	nfast,
		   float	**y2sf);

extern void splin2(float	*xs,
		   float	*xf,
		   float	**ysf,
		   float	**y2sf,
		   int	nslow,
		   int	nfast,
		   float	rxs,
		   float	rxf,
		   float	*rysf,
		   float	*drysf,
		   float	*d2rysf);

extern void spline(float	*x,
		   float	*y,
		   int	n,
		   float	yp1,
		   float	ypn,
		   float	*y2);

extern void splint(float	*xa,
		   float	*ya,
		   float	*y2a,
		   int	n,
		   float	x,
		   float	*y);

extern void splint_deriv(float	*xa,
			 float	*ya,
			 float	*y2a,
			 int	n,
			 float	x,
			 float	*y,
			 float	*dy,
			 float	*d2y);

extern int ratint(float	*xa,
		  float	*ya,
		  int	n,
		  float	x,
		  float	*y,
		  float	*dy);

extern void nrerror(char *error_text);

extern float *vector(int nl, int nh);

extern double *dvector(int nl, int nh);

extern float **matrix(int nrl, int nrh, int ncl, int nch);

extern double **dmatrix(int nrl, int nrh, int ncl, int nch);

extern void free_vector(float *v, int nl, int nh);

extern void free_dvector(double *v, int nl, int nh);

extern void free_matrix(float **m,
			int nrl, int nrh, int ncl, int nch);

extern void free_dmatrix(double **m,
			 int nrl, int nrh, int ncl, int nch);

extern float  **convert_matrix(float *a,
			       int nrl, int nrh, int ncl, int nch);

extern void free_convert_matrix(float **b,
				int nrl, int nrh, int ncl, int nch);

#endif /* _LIBINTERP_H_ */

