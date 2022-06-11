#ifndef _LOCSAT_H_
#define	_LOCSAT_H_

#include "gobject++/CssTables.h"
#include "gobject++/cvector.h"

class LocSAT : public CssTableClass
{
    public:
	bool	use_location;	/* Use lat_init,lon_init. Else use best guess */
	bool	fix_ot;		/* Constrain (fix) origin time */
	bool	fix_latlon;	/* Constrain (fix) lat/lon */
	bool	fix_depth;	/* Constrain (fix) depth */
	double	time_init;	/* origin time */
	float	lat_init;	/* First guess latitude (deg) */
	float	lon_init;	/* First guess longitude (deg) */
	float	depth_init;	/* Initial depth (km) */
	float	damp;	/* Percent damping relative to largest singular value */
	float	conf_level;	/* Confidence ellipse level */
	int	num_dof;	/* Number of degrees of freedom in sig0 */
	int	max_iter;	/* Maximum number of location iterations */
	bool	ignore_big_res;	/* Ignore large data residual during
				   construction of system matrix, where the
				   residual is > big_res_mult*data_std_err[] */
	float	big_res_mult;	/* Large residual multiplier factor. If
				   this factor times the data_std_err[] for any
				   phase is < data residual, then the phase will
				   not be used in the location during that
				   particular iteration. */
	int	ellip_cor_level;/* Ellipticity correction level */
	int	sssc_level;	/* Desired level for SSSC */
	bool	use_srst;	/* Apply SRST corrections for events with
				   stations encompasing a valid SRST region */
	bool	srst_var_wgt;	/* Apply SRST variance weighting for events
				   with stations encompasing a valid SRST region
				   when use_srst is turned on. */
	bool	dist_var_wgt; /* If set, pre-defined distance variance weighting
				 will be applied to data for calculation of the
				 error ellipses only.  This weighting will
				 over-ride the SRST variance weighting when
				 calculating these error ellipses. */
	bool	only_sta_w_corr;/* Use only arrival data with SRST corrections
				   available.  Only meaningful when use_srst is
				   turned on.  Data without SRST corrections
				   will be ignored. */
	float	user_var_wgt;	/* If > 0.0, use this user-defined variance
				   weighting in preference of all other variance
				   weighting. */
	long	orid;		/* orid of associated origin */
	char	lddate[18];


	/* extra members not written to the record
	 */
	int vmodel_spec_file;	/* quark */
	int sasc_dir_prefix;	/* quark */

	static CssTableClass *createLocSAT(void) {
	    return (CssTableClass *) new LocSAT();
	}
	LocSAT(void);
	LocSAT(LocSAT &a);
	LocSAT(int flag) { } // do not initialize members
};

extern "C" {
int locsat(CssTableClass *css, int num_phases, char **phases, 
	cvector<CssSiteClass> &css_site, cvector<CssArrivalClass> &css_arrival,
	cvector<CssAssocClass> &css_assoc, CssOriginClass *css_origin,
	CssOrigerrClass *css_origerr, char **msg);

void initLocSAT(void);
}

void DefineLocSATCB(void *widget, void *client, void *callback_data);


#endif /* _LOCSAT_H_ */
