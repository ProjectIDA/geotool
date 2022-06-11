
/*
 * Copyright (c) 1995-1996 Science Applications International Corporation.
 *

 * NAME
 *	read_ec_table -- Read an ellipticity correction table
 *	get_ec_from_table -- Get ellipticity correction from tables.
 
 * FILE
 *	ellipticity_corr.c

 * SYNOPSIS
 *	int
 *	read_ec_table (file_name, ec_table_ptr)
 *	char	 *file_name;	 (i) Current ellipticity corr. file name
 *	EC_Table **ec_table_ptr; (o) Return current ec_table pointer

 *	double
 *	get_ec_from_table (ec_table, delta, esaz, ev_geoc_co_lat, ev_depth)
 *	EC_Table *ec_table;	(i) Ellipticity correction table for given phase
 *	double	 delta;		(i) Event to station distance (deg.)
 *	double	 esaz;		(i) Event to station azimuth from North (deg.)
 *	double	 ev_geoc_co_lat;(i) Event geocentric co-latitude (radians)
 *	double	 ev_depth;	(i) Event depth (km.)

 * DESCRIPTION
 *	Functions.  All ellipticity corrections manipulating tables of
 *	2-D distance/depth tau coefficients are handled here.  Specifically,
 *	all functionality local to the structure, ec_table, is handled
 *	within this file.

 *	-- read_ec_table() reads an ellipticity correction table for a
 *	given phase.  An individual call is made to read_ec_table() for
 *	each and every phase.

 *	-- get_ec_from_table() grabs the necessary ellipticity correction 
 *	coefficients from the ellipticity correction tables and computes 
 *	the source/station specific ellipticity correction itself.

 * DIAGNOSTICS
 *	-- read_ec_table() returns an integer argument to inform the user
 *	about local conditions encountered while attempting to read the 
 *	ellipticity correction tables (if they are available):

 *	    0:	Ellipticity correction tables were successfully read.
 *	    1:	No ellipticity correction tables available for this phase.
 *	   -1:	Bogus input format encountered.
 *	   -2:	Error allocating memory for given structure or array.

 *	A negative return code indicates a clear error condition.  A 
 *	positive return code simply indicates that no ellipticity 
 *	corrections could be found for given phase.  If no ellipticity
 * 	correction tables are found, then the default ellipticity
 *	corrections hard-coded into the function, ellipticity_corr() 
 *	[libgeog(3)], are to be used instead.

 *	-- get_ec_from_table() will return a correction of 0.0 sec. if no
 *	ellipticity correction table exists for given input phase.

 * FILES
 *	-- read_ec_table() reads ellipticity correction distance/depth tables.

 * NOTES
 *	If no ellipticity correction tables exist, then only the default
 *	corrections specified in function, ellipticity_corr(), defined in
 *	libgeog(3), can be applied.  A file must exists within the main
 *	travel-time table directory of the form, 'tab.elcor_dir', where
 *	tab represent the travel-time table prefix name.  This pointer
 *	file specifies the directory where the actual table are located.
 
 * SEE ALSO
 *	Complementary, but less accurate, ellipticity correction routine,
 *	ellipticity_corr(), located in file ellipticity_corr.c [libgeog(3)].
 *	These will be applied when no ellipticity corrections tables exist
 *	for a given set of travel-time tables (typically for T-T tables 
 *	prior to ak135 model).

 * AUTHOR
 *	Walter Nagy,  6/ 6/95	Created.
 *	Walter Nagy,  9/ 1/95	Re-written to hang directly off of travel-time
 *				handling structure, tt_table.
 */


#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "locp.h"
#include "tt_table.h"

/* Error report */
#define READ_ERR(a)	{ fprintf (stderr, "\nread_ec_table: Error reading %s in file: %s\n", (a), file_name); \
			}
#define CALLOC_ERR(a)	{ fprintf (stderr, "\nread_ec_table: Error allocating space for %s, in file: %s\n", (a), file_name); \
			}

#define	DEG_TO_RAD	(M_PI/180.0)
#define	SQRT3_OVER2	0.866025404


/* file_name: Current ellipticity corr. file name
 * ec_table_ptr: Pointer to ellipticity table for phase
 */
int
read_ec_table (char *file_name, EC_Table **ec_table_ptr)
{
	FILE	*tfp;
	int	i, j;	
	int	ntbd, ntbz;
	EC_Table *ec_table = (EC_Table *) NULL;


	/*
	 * Does ellipticity correction file exist for given phase ?  If so,
	 * open file pointer; else return with code = 1.
	 */

	if ((tfp = fopen (file_name, "r")) == NULL)
	    return (1);

	if ((ec_table = (EC_Table *) calloc (1, sizeof (EC_Table))) == NULL)
	{
	    fclose (tfp);
	    CALLOC_ERR ("structure, ec_table");
	    return (-2);
	}

	/* 
	 * Initialize ec_table structure for this phase 
	 */

	ec_table->num_dists = 0;
	ec_table->num_depths = 0;
	ec_table->dist_samples = (float *) NULL;
	ec_table->depth_samples = (float *) NULL;
	ec_table->t0 = (float **) NULL;
	ec_table->t1 = (float **) NULL;
	ec_table->t2 = (float **) NULL;

	/*
	 * Begin reading ellipticity correction info!
	 *   Start with depth sampling
	 */

	if (fscanf (tfp, "%*[^\n]\n%d%*[^\n]", &ntbz) != 1)
	{
	    READ_ERR("number of depth samples");
	    fclose (tfp);
	    return (-1);
	}
	ec_table->num_depths = ntbz;

	if ((ec_table->depth_samples =
		(float *) calloc (ntbz, sizeof (float))) == NULL)
	{
	    CALLOC_ERR ("ec_table->depth_samples");
	    fclose (tfp);
	    return (-2);
	}

	for (i = 0; i < ntbz; i++)
	{
	    if (fscanf (tfp, "%f", &ec_table->depth_samples[i]) != 1)
	    {
		READ_ERR("depth sample value");
		fclose (tfp);
		return (-1);
	    }
	}

	/*
	 * Next read depth sampling info
	 */

	if (fscanf (tfp, "%d%*[^\n]", &ntbd) != 1)
	{
	    READ_ERR("number of distance samples");
	    fclose (tfp);
	    return (-1);
	}
	ec_table->num_dists = ntbd;

	if ((ec_table->dist_samples =
		(float *) calloc (ntbd, sizeof (float))) == NULL)
	{
	    CALLOC_ERR ("ec_table->dist_samples");
	    fclose (tfp);
	    return (-2);
	}

	for (i = 0; i < ntbd; i++)
	{
	    if (fscanf (tfp, "%f", &ec_table->dist_samples[i]) != 1)
	    {
		READ_ERR("distance sample value");
		fclose (tfp);
		return (FALSE);
	    }
	}

	/*
	 * Read actual ellipticity cooeficients here.  First allocate
	 * necessary memory.
	 */

	if ((ec_table->t0 = (float **) calloc (ntbz, sizeof (float *))) == NULL)
	{
	    CALLOC_ERR ("ec_table->t0");
	    fclose (tfp);
	    return (-2);
	}
	if ((ec_table->t1 = (float **) calloc (ntbz, sizeof (float *))) == NULL)
	{
	    CALLOC_ERR ("ec_table->t1");
	    fclose (tfp);
	    return (-2);
	}
	if ((ec_table->t2 = (float **) calloc (ntbz, sizeof (float *))) == NULL)
	{
	    CALLOC_ERR ("ec_table->t2");
	    fclose (tfp);
	    return (-2);
	}
	for (i = 0; i < ntbz; i++)
	{
	    if ((ec_table->t0[i] = (float *) calloc (ntbd,
					sizeof (float))) == NULL)
	    {
		CALLOC_ERR ("ec_table->t0[]");
		fclose (tfp);
		return (-2);
	    }
	    if ((ec_table->t1[i] = (float *) calloc (ntbd,
					sizeof (float))) == NULL)
	    {
		CALLOC_ERR ("ec_table->t1[]");
		fclose (tfp);
		return (-2);
	    }
	    if ((ec_table->t2[i] = (float *) calloc (ntbd,
					sizeof (float))) == NULL)
	    {
		CALLOC_ERR ("ec_table->t2[]");
		fclose (tfp);
		return (-2);
	    }
	}

	for (j = 0; j < ntbz; j++)
	{
	    /* skip the comment line */
	    while (getc(tfp) != '#');
	    while (getc(tfp) != '\n');
	    for (i = 0; i < ntbd; i++)
	    {
		if (fscanf (tfp, "%f%f%f", &ec_table->t0[j][i],
					   &ec_table->t1[j][i],
					   &ec_table->t2[j][i]) != 3)
		{
		    READ_ERR("ellipticity coefficient");
		    fclose (tfp);
		    return (-1);
		}
	    }
	}

	fclose (tfp);

	*ec_table_ptr = ec_table;

	return (OK);	
} 


/* delta: Event to station distance (deg.)
 * esaz: Event to station azimuth from North (deg.)
 * ev_geoc_co_lat: Event geocentric co-latitude (radians)
 * ev_depth: Event depth (km.)
 * ec_table: Pointer to current ellip corr table
 */
double
get_ec_from_table (EC_Table *ec_table, double delta, double esaz, 
		   double ev_geoc_co_lat, double ev_depth)
{
	int	i, n;
	int	i1, i2, j1, j2;
	double	ellip_corr = 0.0;
	double	azim, dist_fac, depth_fac;
	double	a, b, c, d;
	double	tau0, tau1, tau2;

	static	double	sc0, sc1, sc2;
	static	double	save_ev_colat = 99.0;


	azim  = esaz*DEG_TO_RAD;	/* Event to station azimuth (rad.) */

	/*
	 * Set up reference constants, if event co-latitude has changed.
	 */

	if (ev_geoc_co_lat != save_ev_colat)
	{
	    sc0 = 0.25*(1.0 + 3.0*cos(2.0*ev_geoc_co_lat));
	    sc1 = SQRT3_OVER2*sin(2.0*ev_geoc_co_lat);
	    sc2 = SQRT3_OVER2*sin(ev_geoc_co_lat)*sin(ev_geoc_co_lat);
	    save_ev_colat = ev_geoc_co_lat;
	}

	/*
	 * Find high-ends of both distance (i2) and depth (j2) indexes.
	 */

	n = ec_table->num_dists;
	for (i2 = n-1, i = 1; i < n; i++)
	{
	    if (delta < ec_table->dist_samples[i])
	    {
		i2 = i;
		break;
	    }
	}
	n = ec_table->num_depths;
	for (j2 = n-1, i = 1; i < n; i++)
	{
	    if (ev_depth < ec_table->depth_samples[i])
	    {
		j2 = i;
		break;
	    }
	}
	i1 = i2 - 1;	/* Low-end distance index */
	j1 = j2 - 1;	/* Low-end depth index */

	dist_fac  = (delta - ec_table->dist_samples[i1]) /
		    (ec_table->dist_samples[i2] - ec_table->dist_samples[i1]);
	depth_fac = (ev_depth - ec_table->depth_samples[j1]) /
		    (ec_table->depth_samples[j2] - ec_table->depth_samples[j1]);

	/*
	 * Compute tau coefficients of Dziewonski and Gilbert (1976).
	 */

	/* tau0 */
	a = ec_table->t0[j1][i1];
	b = ec_table->t0[j2][i1];
	c = a + (ec_table->t0[j1][i2]-a)*dist_fac;
	d = b + (ec_table->t0[j2][i2]-b)*dist_fac;
	tau0 = c + (d-c)*depth_fac;

	/* tau1 */
	a = ec_table->t1[j1][i1];
	b = ec_table->t1[j2][i1];
	c = a + (ec_table->t1[j1][i2]-a)*dist_fac;
	d = b + (ec_table->t1[j2][i2]-b)*dist_fac;
	tau1 = c + (d-c)*depth_fac;

	/* tau2 */
	a = ec_table->t2[j1][i1];
	b = ec_table->t2[j2][i1];
	c = a + (ec_table->t2[j1][i2]-a)*dist_fac;
	d = b + (ec_table->t2[j2][i2]-b)*dist_fac;
	tau2 = c + (d-c)*depth_fac;

	/*
	 * Compute ellipticity correction via equations (22) and (26)
	 * of Dziewonski and Gilbert (1976).
	 */

	ellip_corr = sc0*tau0 + sc1*cos(azim)*tau1 + sc2*cos(2.0*azim)*tau2;

	return (ellip_corr);
}
