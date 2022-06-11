
/*
 * Copyright (c) 1995 Science Applications International Corporation.
 *

 * NAME
 *	read_LP_info -- Read long-period (LR & LQ) grid and phase velocity info
 *	get_LP_velocity -- Get LP phase velocity at a given period
 *	LP_trace_ray -- Do actual tracing of long-period rays thru grid
 *	LP_trace_rays -- Do actual tracing of long-period rays thru grid for multiple periods
 *	get_LP_grid_index -- Get LP grid indice (not currently called anywhere)
 *	set_LP_vel_files -- Replace default group velocity files with alternate files.
 *	set_LP_grid_files -- Replace default grid files with alternate files.

 * FILE
 *	LP_trace_ray.c

 * SYNOPSIS
 *	int
 *	read_LP_info (lp_pathway)
 *	char	*lp_pathway;	(i) Directory pathway containing LP tables

 *	double
 *	get_LP_velocity (ilat, ilon, period, ph_index)
 *	int	ilat;		(i) Latitude index
 *	int	ilon;		(i) Longitude index
 *	double	period;		(i) Period for which to get velocity (sec.)
 *	int	ph_index;	(i) LP phase index (0: LR; 1: LQ)

 *	double
 *	LP_trace_ray (ev_lat, ev_lon, sta_lat, sta_lon, ph_index, period)
 *	double	ev_lat;		(i) Geographic event latitude (deg.)
 *	double	ev_lon;		(i) Geographic event longitude (deg.)
 *	double	sta_lat;	(i) Geographic station latitude (deg.)
 *	double	sta_lon;	(i) Geographic station longitude (deg.)
 *	int	ph_index;	(i) LP phase index (0: LR; 1: LQ)
 *	double	period;		(i) Period for which to get velocity (sec.)

 *	double *
 *	LP_trace_rays (ev_lat, ev_lon, sta_lat, sta_lon, ph_index, period, nperiod)
 *	double	ev_lat;		(i) Geographic event latitude (deg.)
 *	double	ev_lon;		(i) Geographic event longitude (deg.)
 *	double	sta_lat;	(i) Geographic station latitude (deg.)
 *	double	sta_lon;	(i) Geographic station longitude (deg.)
 *	int	ph_index;	(i) LP phase index (0: LR; 1: LQ)
 *	double	*period;	(i) Periods for which to get velocity (sec.)
 *	long	nperiod;	(i) Number of elements in period array.

 *	int
 *	get_LP_grid_index (geoc_co_lat, east_lon, ph_index, azimuth)
 *	double	*geoc_co_lat;		(i/o) Geocentric co-latitude (deg.)
 *	double	east_lon;		(i) East longitude (deg.)
 *	int	ph_index;		(i) LP phase index (0: LR; 1: LQ)
 *	double	azimuth;		(i) Forward azimuth (rad.)

 *	void
 *	set_LP_vel_files(LR_file,LQ_file)
 *	char *LR_file;		(i) New LR velocity file
 *	char *LQ_file;		(i) New LQ velocity file

 *	void
 *	set_LP_grid_files(LR_file,LQ_file)
 *	char *LR_file;		(i) New LR grid file
 *	char *LQ_file;		(i) New LQ grid file
 *
 * New functions to support variable number of files and compiled files

 * void set_LP_grid_file( char *grid_file, char *vel_file)
 *	grid_file:	(i) First grid file
 *	vel_file:	(i) First velocity file corresponding to grid_file
 *
 * void set_LP_comp_file( char *LR_file) {}
 *	LR_file:	(i) Compiled grid+velocity file
 * 
 * void add_LP_grid_file( char *grid_file, char *vel_file) {}
 *	grid_file:	(i) Additional grid file
 *	vel_file:	(i) Additional velocity file corresponding to grid_file
 *
 * void add_LP_comp_file( char *LR_file) {}
 *	LR_file:	(i) Compiled grid+velocity file
 *  
 * int
 * read_compiled_file( char * lp_pathway, char *LR_file, int ph_index)
 *	lp_pathway:	(i) Directory of compiled files
 *	LR_file:	(i) Compiled grid+velocity file to read
 *	ph_index:	(i) Array index
 *
 * int
 * write_compiled_file( char * lp_pathway, char *LR_file, int ph_index)
 *	lp_pathway:	(i) Directory of compiled files
 *	LR_file:	(i) Compiled grid+velocity file to write
 *	ph_index:	(i) Array index
 *

 * DESCRIPTION
 *	Functions.  All handling of long-period (LP) grid and phase velocity
 *	tables is handled here.  Specifically, all functionality local to the 
 *	structure, lp_data, is handled within this file.
 
 *	-- read_LP_info() reads long-period (LR & LQ) grid and phase velocity
 *	tables.  Files "must" be specified (named) as follows:
 *	    LR grid file:	'LP_grid.LR'
 *	    LQ grid file:	'LP_grid.LQ'
 *	    LR velocity file:	'LP_vel.LR'
 *	    LQ velocity file:	'LP_vel.LQ'

 *	-- get_LP_velocity() interogates the lp_data structures for the 
 *	long-period phase velocity for a given input period.

 *	-- LP_trace_ray() traces a long-period ray thru grid of phase
 *	velocities (as read by, function, read_LP_info(), mapped on a 
 *	sphere, to obtain the travel-time (sec.).  See ALGORITHM section
 *	below for details.

 *	-- LP_trace_rays() traces long-period rays for multiple periods.

 *	-- get_LP_grid_index() interogates the lp_data structures for the
 *	grid index for an input geocentric co-latitude and east longitude.
 *	Input geocentric co-latitude can only be updated if, and only if,
 *	sample latitude and longitude fall right on a grid corner.

 * ALGORITHM
 *	This section provides a general algorithmic description for tracing
 *	long-period (LP) rays over a grid mapped onto the Earth's surface 
 *	containing different phase velocity vs. period measures.  The main
 *	guts of this algorithm are contained in function, LP_trace_ray(),
 *	while phase velocity data is extracted from a world-wide grid of
 *	indexes to phases velocity, via function, get_LP_velocity().  The
 *	general ray tracing task is accomplished as follows:

 *	   1.	Given an event and station location (lat/lon; deg.) along
 *		with the desired period (sec.), we determine which point
 *		is the westernmost so that we always trace from west-to-
 *		east.  This simplifies the overall approach, since we know
 *		rays can never exit from the west side of a grid boundary.

 *	   2.	Our first ray will almost always emanate from "within" a
 *		grid cell.  Calculate distance and azimuth from westernmost
 *		to easternmost ray points.  This is defined as the "current"
 *		distance and azimuth.
 
 *		Start of main iterative loop over all grid cells between
 *		station and event or vice-a-versa.
 *	   3.	We begin ray tracing from current starting point by first
 *		calculating the distance and azimuth from this point to 
 *		both the NE and SE corners of the current grid cell.  
 
 *	   4.	If the current azimuth lies between the computed NE and SE 
 *		azimuths, then we know the ray exits from the eastern 
 *		boundary.  We then calculate the distance and new position 
 *		to the east longitude grid boundary based on its latitude 
 *		intersection.  This is our new starting point for the ray.  
 *		Go to 6.

 *	   5.	If the current azimuth lies between the SE corner and 180
 *		deg. (i.e., directly due south), then our exit point will
 *		be defined along the southern boundary of the current
 *		grid cell.  Elsewise, the exit point will be the northern
 *		boundary.  We then calculate the distance and new position
 *		to the northern or southern latitude grid boundary based 
 *		its longitude intersection.  Note, since we insist that the 
 *		ray be traced from west-to-east, it cannot exit via the 
 *		western boundary of the grid cell.

 *	   6.	If the distance to the exit boundary is greater than the
 *		"current" distance, then the final grid cell has been 
 *		found.

 *	   7.	Compute travel-time spent within this segment (grid cell)
 *		of ray path.  The velocity is obtained by a call to 
 *		function, get_LP_velocity () for the given input period.
 *		Add this travel-time to the overall travel-time for the 
 *		path computed thus far.  If this is the final grid cell, 
 *		then go to 9.

 *	   8.	Calculate a new "current" azimuth and distance from our
 *		new starting point to the final (easternmost) point.  This 
 *		will provide the distance for testing step 7 during the
 *		next iteration.  Go to 3.

 *	   9.	Ray path completely traced.  Check that sum of distances
 *		for each individual grid cell is nearly equal (within 0.1%) 
 *		to the original distance computed between station and event.
 *		This check is just a verification that the ray was traced 
 *		properly.  Return travel-time (sec.).

 * DIAGNOSTICS
 *	-- read_LP_info() returns an integer argument to inform the user
 *	about local conditions encountered while attempting to read LP
 *	tables (if they are available):
 
 *	    0:	LP table were successfully read.
 *	    1:	No LP tables available.
 *	    2:	LP directory is empty.
 *	   -1:	Bogus input format encountered.
 *	   -2:	Problems encountered trying to open file.
 *	   -3:	Error allocating memory for given structure or array.

 *	A negative return code indicates a clear error condition.  A
 *	positive return code simply indicates that no LP grid and velocity
 *	files could be found, so that the default tables found in the 
 *	normal T-T directory area will be used instead.

 *	-- get_LP_velocity() will return with a negative velocity (namely,
 *	-1.0) if an invalid period or latitude/longitude index is specified.
 *	Note: change 1/28/97: period out of range returns the value at the
	closest end point.

 *	-- LP_trace_ray() will return the long-period ray travel-time (sec.)
 *	computed over entire grid.

 * FILES
 *	-- read_LP_info() reads LP grid and phase velocity files.

 * NOTES
 *	A file must exists within the main travel-time table directory 
 *	of the form, 'tab.lp_dir', where tab represents the travel-time 
 *	table prefix name.  This pointer file specifies the directory
 *	where the actual table are located.  get_LP_grid_index() is not
 *	currently called anywhere, but might it be useful in the future.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, June 22, 1995	Created.
 *	J. Stevens, November 29, 1995	Modified to do multiple periods and
 *					to use different data file names.
 *	J. Stevens, January 23, 1997	Modified to read/write binary files.
 */


#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "libLP.h"
#include "libgeog.h"
#include "lp_data.h"
#include "libaesir.h"
#define	TWO_PI		2.0*M_PI
#define	RAD_TO_DEG	(180.0/M_PI)
#define	DEG_TO_RAD	(M_PI/180.0)
#define	KM_TO_DEG	(1.0/111.195)
#define	RADIUS_EARTH	6371.0
#define	ONE_KM_IN_RAD	(1.0/RADIUS_EARTH)
#define	SMALL_DIST	0.01*ONE_KM_IN_RAD	/* 10 meters (radians) */

#define	LR	0
#define	LQ	1

#define	INSIDE	1
#define	EAST	2
#define	NORTH	3
#define	SOUTH	4

#define	NE	0		/* Northeast corner of grid (NE) */
#define	SE	1		/* Southeast corner of grid (SE) */
#define	WNS	2		/* West side of grid (NW or SW)  */

/* Error reporting */
#define	BOGUS_FORMAT	-1
#define	FILE_MISSING	-2
#define	MEM_ALLOC_ERR	-3

#define	READ_ERR(a)	{ fprintf (stderr, "\nread_LP_info: Error reading %s in file: %s!\n", (a), file_name); \
			}
#define	CALLOC_ERR(a)	{ fprintf (stderr, "\nread_LP_info: Error allocating space for %s in file: %s\n", (a), file_name); \
			}

static	LP_Data	*lp_data=NULL;

static	char	*def_LP_grid_file[] = { "LP_grid.LR", "LP_grid.LQ" };
static	char	*def_LP_vel_file[]  = { "LP_vel.LR",  "LP_vel.LQ"  };

static	int	nread_file = 0; /* Number of files successfully read */
static	int	ngrid_file = 0;
static	int	ncomp_file = 0;
static 	char	** LP_grid_file=NULL;
static 	char	** LP_vel_file=NULL;
static 	char	** LP_comp_file=NULL;
static int check_bounds(int ilat, int ilon, int ph_index);

int
read_LP_info (char *lp_pathway)
{

	FILE	**gfp=NULL;
	FILE 	**vfp=NULL;
	int	i, j, k;
	int	nlat, nlon;
	int	nindex, nper;
	double	spacing;
	char	file_name[FILENAMELEN];

	static	char	routine[]="read_LP_info";

	/*
	 * If previous LP tables have already been read, then free old tables
	 * before re-allocating new memory.
	 */

	if (nread_file > 0)
	{
	    for (i = 0; i < nread_file; i++)
	    {
		for (j = 0; j < lp_data[i].num_lat_grids; j++)
		    UFREE (lp_data[i].grid_indice[j]);
		UFREE (lp_data[i].grid_indice);
		for (j = 0; j < lp_data[i].num_indexes; j++)
		    UFREE (lp_data[i].velocity[j]);
		UFREE (lp_data[i].velocity);
		UFREE (lp_data[i].period_samples);
	    }
	    free(lp_data);
	    lp_data=NULL;
	    nread_file=0;
	}


	/* Check for input files */
	if (ncomp_file > 0) {
		/* Read compiled files */
		lp_data=(LP_Data *) calloc(ncomp_file,sizeof(LP_Data));
		for(k=0;k<ncomp_file;k++,nread_file++) {
			if(!LP_comp_file || !LP_comp_file[k]) 
				return(FILE_MISSING);
			if(!read_compiled_file( lp_pathway, LP_comp_file[k], k)) {
				return(BOGUS_FORMAT);
			}
		}
		return(OK);
	}
	if(ngrid_file<=0) {
		/* Do the old way */
		set_LP_grid_file(def_LP_grid_file[0],def_LP_vel_file[0]);
		add_LP_grid_file(def_LP_grid_file[1],def_LP_vel_file[1]);
	}
	/* Read formatted files */
	gfp=(FILE **) calloc(ngrid_file,sizeof(FILE *));
	vfp=(FILE **) calloc(ngrid_file,sizeof(FILE *));
	lp_data=(LP_Data *) calloc(ngrid_file,sizeof(LP_Data));

	/*
	 * Open grid indice files and set file pointers, gfp[].
	 * All files must exist!
	 */
	for(k=0;k<ngrid_file;k++,nread_file++) {
		strcpy (file_name, lp_pathway);
		strcat (file_name, "/");
		strcat (file_name, LP_grid_file[k]);
		if ((gfp[k] = fopen (file_name, "r")) == NULL)
		{
	    		fprintf (stderr, "Error: %s: File: %s is missing!\n",
			     routine, file_name);
			for(i=0;i<k-1;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k-1;i++) fclose(vfp[i]); /* Close all open files */
	    		return (FILE_MISSING);
		}
	/*
	 * Open LP velocity files and set file pointers, vfp[].
	 * All files must exist!
	 */

		strcpy (file_name, lp_pathway);
		strcat (file_name, "/");
		strcat (file_name, LP_vel_file[k]);
		if ((vfp[k] = fopen (file_name, "r")) == NULL)
		{
	    		fprintf (stderr, "Error: %s: File: %s is missing!\n",
			     routine, file_name);
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k-1;i++) fclose(vfp[i]); /* Close all open files */
	    		return (FILE_MISSING);
		}
	
	    /* Initialize lp_data structure */

	    	lp_data[k].num_lat_grids = 0;
	    	lp_data[k].num_lon_grids = 0;
	    	lp_data[k].latlon_spacing = 0.0;
	    	lp_data[k].grid_indice = (short **) NULL;
	    	lp_data[k].num_periods = 0;
	    	lp_data[k].num_indexes = 0;
	    	lp_data[k].period_samples = (double *) NULL;
	    	lp_data[k].velocity = (double **) NULL;

	    /*
	     * Begin reading grid indice information
	     */

	    	strcpy (file_name, LP_grid_file[k]);

	    	if (fscanf (gfp[k], "%*[^\n]\n%d%d%lf%*[^\n]", 
				&nlat, &nlon, &spacing) != 3)
	    	{
			READ_ERR("sampling info from grid indice file");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (BOGUS_FORMAT);
	    	}
	    	lp_data[k].num_lat_grids = nlat;
	    	lp_data[k].num_lon_grids = nlon;
	    	lp_data[k].latlon_spacing = spacing;


	    	if ((lp_data[k].grid_indice = (short **) 
			calloc (nlat, sizeof (short *))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].grid_indice");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (MEM_ALLOC_ERR);
	    	}
	    	for (i = 0; i < nlat; i++)
	    	{
			if ((lp_data[k].grid_indice[i] = (short *)
				calloc (nlon, sizeof (short))) == NULL)
			{
		    	CALLOC_ERR ("lp_data[].grid_indice[]");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
		    	return (MEM_ALLOC_ERR);
			}
	    	}

	    /*
	     * Do actual reading of LP grid indices here!
	     */

	    	for (j = 0; j < nlat; j++)
	    	{
			/* skip the comment and blank lines */
			while (getc(gfp[k]) != '\n');
			while (getc(gfp[k]) != '#');
			while (getc(gfp[k]) != '\n');
			for (i = 0; i < nlon; i++)
			{
		    	if (fscanf (gfp[k], "%d", &nindex) != 1)
		    	{
				READ_ERR("grid indice values");
				for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
				for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
				return (BOGUS_FORMAT);
		    	}
		    	lp_data[k].grid_indice[j][i] = (short) nindex;
			}
	    	}

	    /*
	     * Now read index/velocity tables.
	     */

	    	strcpy (file_name, LP_vel_file[k]);

	    	if (fscanf (vfp[k], "%*[^\n]\n%d%*[^\n]", &nindex) != 1)
	    	{
			READ_ERR("number of index samples");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (BOGUS_FORMAT);
	    	}
	    	if (fscanf (vfp[k], "%d%*[^\n]", &nper) != 1)
	    	{
			READ_ERR("number of period samples");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (BOGUS_FORMAT);
	    	}
	    	lp_data[k].num_indexes = nindex;
	    	lp_data[k].num_periods = nper;


	    	if ((lp_data[k].period_samples = 
			(double *) calloc (nper, sizeof (double))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].period_samples");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (MEM_ALLOC_ERR);
	    	}

	    /*
	     * Read period samples for velocity file here.
	     */

	    	for (i = 0; i < nper; i++)
	    	{
			if (fscanf (vfp[k], "%lf", &lp_data[k].period_samples[i]) != 1)
			{
		    	READ_ERR("period sample value");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
		    	return (BOGUS_FORMAT);
			}
	    	}

	    	if ((lp_data[k].velocity = (double **) calloc (nindex,
						sizeof (double *))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].velocity");
			for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
			for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
			return (MEM_ALLOC_ERR);
	    	}
	    	for (i = 0; i < nindex; i++)
	    	{
			if ((lp_data[k].velocity[i] = (double *) calloc (nper,
						sizeof (double))) == NULL)
			{
		    		CALLOC_ERR ("lp_data[].velocity[]");
				for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
				for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
		    		return (MEM_ALLOC_ERR);
			}
	    	}

	    	/*
	     	 * Finally, read actual long-period phase velocities for all
	     	 * indexes and periods.
	     	*/

	    	for (j = 0; j < nindex; j++)
	    	{
			/* skip the comment line */
			while (getc(vfp[k]) != '#');
			while (getc(vfp[k]) != '\n');
			for (i = 0; i < nper; i++)
			{
		    		if (fscanf (vfp[k], "%lf", &lp_data[k].velocity[j][i]) != 1)
		    		{
					READ_ERR("velocity");
					for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
					for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
					return (BOGUS_FORMAT);
		    		}
			}
	    	}
	}

	for(i=0;i<k;i++) fclose(gfp[i]); /* Close all open files */
	for(i=0;i<k;i++) fclose(vfp[i]); /* Close all open files */
	free(gfp); gfp=NULL;
	free(vfp); vfp=NULL;

	return (OK);
}


double
get_LP_velocity (int ilat, int ilon, double period, int ph_index)
{

	int	i;
	int	iper, igrid;
	int	num_per;
	double	lower_period_bound, upper_period_bound;
	double	ratio, vel_1, vel_2;
	double	velocity = -1.0;


	/*
	 * If bad latitude or longitude index is entered, return a value of
	 * -1.0, indicating that a problem was encountered.
	 *
	 * Except if ilon=-999 and ilat>0 interpret ilat as igrid. This is
	 * necessary because we need a way to return a value for a specific
	 * model.  We will do this a better way later - JLS, 2/15/97.
	 */

	if(ilon==-999 && ilat>=0) {
		igrid=ilat;
	} else {
	  if (ilat < 0 || ilon < 0 || ilat >= lp_data[ph_index].num_lat_grids ||
	    ilon >= lp_data[ph_index].num_lon_grids)
	    return (-1.0);

	  /*
	   * Get grid index!
	   */

	  igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
	}

	/*
	 * If input period is outside of valid range, then return with
	 * a negative velocity to indicate the problem.  Also print any
	 * such error to stderr.
	 *
	 *	num_per = lp_data[ph_index].num_periods;
	 *	if (period < lp_data[ph_index].period_samples[0] ||
	 *	    period > lp_data[ph_index].period_samples[num_per-1])
	 *	{
	 *	    fprintf (stderr, "%s: Input period of %f sec. is out-of-range!\n",
	 *			     routine, period);
	 *	    return (-1.0);
	 *	}
	 */
	/*
	 * Note significant change: 1/28/97
	 * Period out of range now returns the end point value.
	 * This is necessary because calculation of a full path correction
	 * at all frequencies will always exceed the data range.
	 */

	 num_per = lp_data[ph_index].num_periods;
	 if (period < lp_data[ph_index].period_samples[0])
		period = lp_data[ph_index].period_samples[0];
	 if ( period > lp_data[ph_index].period_samples[num_per-1])
	 	period = lp_data[ph_index].period_samples[num_per-1];
	/*
	 * Determine upper period index for interpolation purposes.
	 */

	for (i = 0, iper = 0; i < num_per; i++)
	{
	    if (period < lp_data[ph_index].period_samples[i])
	    {
		iper = i;
		break;
	    }
	}

	/*
	 * Interpolate period data to get phase velocity.
	 */

	if (iper > 0)
	{
	    upper_period_bound = lp_data[ph_index].period_samples[iper];
	    lower_period_bound = lp_data[ph_index].period_samples[iper-1];
	    ratio = (period - lower_period_bound) /
			(upper_period_bound - lower_period_bound);
	    vel_1 = lp_data[ph_index].velocity[igrid][iper-1];
	    vel_2 = lp_data[ph_index].velocity[igrid][iper];
	    velocity = vel_1 + ratio * (vel_2 - vel_1);
	}
	else
	    /* Special case: Input period == largest period sample */

	    velocity = lp_data[ph_index].velocity[igrid][num_per-1];

	/*
	printf ("igrid:%3d  ilat:%4d  ilon:%4d  iper:%3d  vel1:%6.3f  vel2:%6.3f  vel:%6.3f\n",
		igrid, ilat, ilon, iper, vel_1, vel_2, velocity);
	 */

	return (velocity); 
}


double *
LP_trace_rays (double ev_lat, double ev_lon, double sta_lat, double sta_lon,
int ph_index, double *period, long nperiod)
{

	Bool	final_grid_found = FALSE;
	int	ilat, ilon, nlat, nlon;
	int	grid_position;
	double	co_lat, lon;
	double	lon_360;
	double	start_lon, end_lon;
	double	start_co_lat, end_co_lat;
	double	ev_co_lat, sta_co_lat;
	double	remainder_lat, remainder_lon;
	double	cur_lat_bounds[2], cur_lon_bounds[2];
	double	cur_azi, cur_baz, cur_delta;
	double	azi[3], baz[3], delta[3];
	double	spacing, tmp, vel;
	double	delta_to_edge, new_co_lat, new_lon;
	double	delta_diff, orig_delta;
	double	sum_of_deltas = 0.0;
	double	*total_trv_time=0;
	long 	iper;

	if(!(total_trv_time=(double *)calloc(nperiod,sizeof(double))))
		fprintf (stderr,"Error allocating %ld elements for total_trv_time\n",nperiod);
	/*
	 * To simplify matters we will always trace from west to east.
	 * Therefore, set western and eastern longitude bounds here, 
	 * start_lon and end_lon, respectively.  First, convert 
	 * geographic event and station latitudes (deg.) into geocentric 
	 * co-latitude (deg.).
	 */

	ev_co_lat = lat_conv (ev_lat, TRUE, TRUE, TRUE, TRUE, FALSE);
	sta_co_lat = lat_conv (sta_lat, TRUE, TRUE, TRUE, TRUE, FALSE);

	/*
	 * To avoid crossing exactly across North or South Poles add a
	 * small ammount to one of the longitudes.  If ray passes exactly
	 * thru one of the poles, then this can create some numerical 
	 * and book-keeping headaches which are not worth the trouble.
	 */

	if (ev_lon != sta_lon && fabs (ev_lon - sta_lon) == 180.0)
	    sta_lon += 0.01;

	/*
	 * Compute distance, azimuth and back-azimuth starting with event
	 * and ending with station.  This will be re-ordered if the
	 * station is found to be west of the event.
	 */

	geoc_distaz (ev_co_lat*DEG_TO_RAD, ev_lon*DEG_TO_RAD,
		     sta_co_lat*DEG_TO_RAD, sta_lon*DEG_TO_RAD,
		     &cur_delta, &cur_azi, &cur_baz, 0);

	orig_delta = cur_delta;

	if (cur_azi > M_PI)
	{
	    start_co_lat = sta_co_lat;
	    start_lon = sta_lon;
	    end_co_lat = ev_co_lat;
	    end_lon = ev_lon;
	    tmp = cur_azi;
	    cur_azi = cur_baz;
	    cur_baz = tmp;
	}
	else
	{
	    start_co_lat = ev_co_lat;
	    start_lon = ev_lon;
	    end_co_lat = sta_co_lat;
	    end_lon = sta_lon;
	}

	spacing = lp_data[ph_index].latlon_spacing;
	nlat = (int)(180.0/spacing + 0.001); /* add small amount to avoid roundoff */
	nlon = (int)(360.0/spacing + 0.001);

	/*
	 * Get LP grid index for first grid cell (starting point).
	 * Start by represeting all longitude information from
	 * 0 to 360 deg.
	 */

	if (start_lon < 0.0)
	    lon_360 = start_lon + 360.0;
	else
	    lon_360 = start_lon;

	/*
	 * Set latitude and longitude indexes, that is, ilat and ilon, 
	 * respectively.  Note that input co-latitudes and longitudes 
	 * which fall along a boundary will be put into the next 
	 * co-latitude or longitude grid cell.  
	 */

	remainder_lat = modf ((start_co_lat/spacing), &co_lat);
	remainder_lon = modf ((lon_360/spacing), &lon);
	ilat = (int) co_lat;
	ilon = (int) lon;

	/*
	 * Special case: If both co-latitude and longitude fall on a 
	 * boundary (i.e., a corner), determine whether ray is moving
	 * into southerly (will stay in prescribed grid) or northerly
	 * (will move into latitude grid immediately above).  For stability,
	 * either add or subtract a small distance (10 m) to or from the 
	 * starting co-latitude.
	 */

	if (remainder_lat < DBL_EPSILON && remainder_lon < DBL_EPSILON)
	{
	    if (cur_azi < M_PI_2)
	    {
		--ilat;
		start_co_lat -= 0.0001;
		co_lat -= spacing;
	    }
	    else
		start_co_lat += 0.0001;
	}

	/*
	 * Save current lower (north; 0) and upper (south; 1) co-latitude
	 * and lower (west; 0) and upper (east; 1) longitude values of grid
	 * in static arrays, cur_lat_bounds and cur_lon_bounds (in radians).
	 * Also convert spacing into radians for the rest of this routine.
	 */

	cur_lat_bounds[0] = co_lat * spacing * DEG_TO_RAD;
	cur_lon_bounds[0] = lon * spacing * DEG_TO_RAD;

	spacing *= DEG_TO_RAD;

	cur_lat_bounds[1] = cur_lat_bounds[0] + spacing;
	cur_lon_bounds[1] = cur_lon_bounds[0] + spacing;
	if (cur_lon_bounds[0] > M_PI)
	    cur_lon_bounds[0] -= TWO_PI;
	if (cur_lon_bounds[1] > M_PI)
	    cur_lon_bounds[1] -= TWO_PI;


	/*
	 * Main iterative loop.  Rays will be trace from grid cell boundary
	 * to grid cell boundary until the complete ray path is traversed.
	 * Since the eastern boundary (EAST) will almost always be longer
	 * than the northern (NORTH) or southern (SOUTH) boundaries, we will
	 * check for it first.  Fortunately, some economies can be made
	 * here as well.  We will always start at the NE corner and inspect
	 * the SE second.  This will tells us quickly whether or not our
	 * exit ray will least the eastern boundary.  Please note that all
	 * directions are given relative to the "previous" grid cell exit
	 * points.  Therefore, an EAST grid position is actually emanating
	 * from the western edge for the current cell.  When a new edge is
	 * determined, then the direction is strictly true.  One way to 
	 * understand this choice is by realizing the direction indicates
	 * the general direction of the ray itself (specifically, whether
	 * it is more NORTH than SOUTH).  All rays are more EAST than WEST,
	 * by definition.  Please try to keep aware of this fact!
	 */


	/* Initialize starting point. */

	grid_position = INSIDE;
	co_lat = start_co_lat*DEG_TO_RAD;
	lon    = start_lon*DEG_TO_RAD;
	end_co_lat = end_co_lat*DEG_TO_RAD;
	end_lon = end_lon*DEG_TO_RAD;

	while (final_grid_found != TRUE)
	{
	    /* Always start with NE corner, then SE */
	    geoc_distaz (co_lat, lon, cur_lat_bounds[0], cur_lon_bounds[1],
	    		 &delta[NE], &azi[NE], &baz[NE], 0);
	    if (azi[NE] > M_PI)
	    {
		azi[NE] = 0.0;
		baz[NE] = M_PI;
	    }
	    geoc_distaz (co_lat, lon, cur_lat_bounds[1], cur_lon_bounds[1],
	    		 &delta[SE], &azi[SE], &baz[SE], 0);
	    if (azi[SE] > M_PI)
	    {
		azi[SE] = M_PI;
		baz[SE] = 0.0;
	    }
	    if (cur_azi > azi[NE] && cur_azi <= azi[SE])
	    {
		grid_position = EAST;

		/*
		 * Since the eastern longitude boundary defines a great
		 * circle, we only need subtract exactly 180 deg. from
		 * the back-azimuth (baz).  Note that we could avoid a 
		 * call to geoc_lat_lon() by just computing the side 
		 * coincident with the longitude great circle at the 
		 * eastern boundary.  Unfortunately, we still need to 
		 * calculate the distance from our current working 
		 * point to the edge to test if this is the final grid 
		 * cell.
		 */

		delta_to_edge =
		dist_given_2angles_plus_side (baz[NE]-M_PI, cur_azi-azi[NE],
					      delta[NE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final travel-time segment.
		     */
		    for(iper=0;iper<nperiod;iper++) {
		    	if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
				free(total_trv_time); return(0);
			} else {
		    		total_trv_time[iper] += (cur_delta*RAD_TO_DEG) / (vel*KM_TO_DEG);
			}
		    }
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add travel-time for this segment
		 */
		for(iper=0;iper<nperiod;iper++) {
		    if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
			free(total_trv_time); return(0);
		    } else {
			total_trv_time[iper] += (delta_to_edge*RAD_TO_DEG) / (vel*KM_TO_DEG);
		    }
		}
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lon_bounds[0] = new_lon;
		cur_lon_bounds[1] = new_lon + spacing;
		if (cur_lon_bounds[1] > M_PI)
		    cur_lon_bounds[1] -= TWO_PI;
		++ilon;
		if (ilon >= nlon)
		    ilon -= nlon;
		continue;
	    }

	    /*
	     * Ray must exit from NORTH or SOUTH co-latitude boundaries!

	     * If emanating from INSIDE or EAST boundary, then determine
	     * whether exit ray will leave NORTH or SOUTH boundary.  Note
	     * that EAST exit test has already been done.
	     */

	    if (grid_position == INSIDE || grid_position == EAST)
	    {
		if (cur_azi > azi[SE] && cur_azi <= M_PI)
		    grid_position = SOUTH;		/* Will exit to SOUTH */
		else
		    grid_position = NORTH;		/* Will exit to NORTH */
	    }

	    if (grid_position == SOUTH)
	    {
		geoc_distaz (cur_lat_bounds[1], cur_lon_bounds[1], 
			     cur_lat_bounds[1], lon, &delta[WNS], 
			     &azi[WNS], &baz[WNS], 0);

		delta_to_edge =
		dist_given_2angles_plus_side (baz[SE]-azi[WNS], cur_azi-azi[SE],
					      delta[SE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final travel-time segment.
		     */
		    for(iper=0;iper<nperiod;iper++) {
		    	if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
				free(total_trv_time); return(0);
			} else {
		    		total_trv_time[iper] += (cur_delta*RAD_TO_DEG) / (vel*KM_TO_DEG);
			}
		    }
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add travel-time for this segment
		 */
		for(iper=0;iper<nperiod;iper++) {
		    if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
			free(total_trv_time); return(0);
		    } else {
			total_trv_time[iper] += (delta_to_edge*RAD_TO_DEG) / (vel*KM_TO_DEG);
		    }
		}
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lat_bounds[0] = new_co_lat;
		cur_lat_bounds[1] = new_co_lat + spacing;
		++ilat;
		if (cur_lat_bounds[1] > M_PI)
		{
		    cur_lat_bounds[0] = M_PI - spacing;
		    cur_lat_bounds[1] = M_PI;
		    if(ilat>nlat-1) ilat=nlat-1;
		    /* --ilat;	Error! */	/* Wrap around S. Pole */
		}
		continue;
	    }
	    else	/* Exits to NORTH */
	    {
		geoc_distaz (cur_lat_bounds[0], cur_lon_bounds[1], 
			     cur_lat_bounds[0], lon, &delta[WNS], 
			     &azi[WNS], &baz[WNS], 0);

		delta_to_edge =
		dist_given_2angles_plus_side (azi[WNS]-baz[NE], azi[NE]-cur_azi,
					      delta[NE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final travel-time segment.
		     */
		    for(iper=0;iper<nperiod;iper++) {
		    	if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
				free(total_trv_time); return(0);
			} else {
		    		total_trv_time[iper] += (cur_delta*RAD_TO_DEG) / (vel*KM_TO_DEG);
			}
		    }
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add travel-time for this segment
		 */
		for(iper=0;iper<nperiod;iper++) {
		    if ((vel = get_LP_velocity (ilat, ilon, period[iper], ph_index)) < 0.0) {
			free(total_trv_time); return(0);
		    } else {
			total_trv_time[iper] += (delta_to_edge*RAD_TO_DEG) / (vel*KM_TO_DEG);
		    }
		}
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lat_bounds[1] = new_co_lat;
		cur_lat_bounds[0] = new_co_lat - spacing;
		--ilat;
		if (cur_lat_bounds[0] < 0.0)
		{
		    cur_lat_bounds[0] = 0.0;
		    cur_lat_bounds[1] = spacing;
		    /* ++ilat;	Error! */	/* Wrap around N. Pole */
		    if(ilat>0) ilat=0;
		}
		continue;
	    }

	}	/* End of main iterative loop over grid cells */


	/*
	 * If sum of all individual distance segments is different from
	 * original distance calculation by more than 0.1%, then print
	 * an warning message.
	 */

	delta_diff = fabs (orig_delta - sum_of_deltas);

	if (delta_diff > 0.001 * orig_delta)
	    fprintf (stderr, "Warning: Sum of individual LP segments differ by > 0.1%% from original distance!\n");

	return (total_trv_time);
}

int
get_LP_grid_index_int(int ilat,int ilon,int ph_index)
{
	int igrid;
	igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
	return(igrid);
}

int
get_LP_grid_index (double *geoc_co_lat, double east_lon, int ph_index,
		   double azimuth)
{

	int	ilat, ilon;
	int	igrid;
	double	co_lat, lon, lon_360;
	double	remainder_lat, remainder_lon;
	double	cur_lon_bounds[2];
	double	spacing;


	/*
	 * Represent all longitude information from 0 to 360 deg.
	 */

	if (east_lon < 0.0)
	    lon_360 = east_lon + 360;
	else
	    lon_360 = east_lon;

	spacing = lp_data[ph_index].latlon_spacing;

	/*
	 * Set latitude and longitude indexes, that is, ilat and ilon, 
	 * respectively.  This will give us the required grid index, igrid.
	 * Note that input co-latitudes and longitudes which fall along a 
	 * boundary will be put into the next co-latitude or longitude
	 * grid cell.  
	 */

	remainder_lat = modf ((*geoc_co_lat/spacing), &co_lat);
	remainder_lon = modf ((lon_360/spacing), &lon);
	ilat = (int) co_lat;
	ilon = (int) lon;
	igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];

	/*
	 * Special case: If both co-latitude and longitude fall on a 
	 * boundary (i.e., a corner), determine whether ray is moving
	 * into southerly (will stay in prescribed grid) or northerly
	 * (will move into latitude grid immediately above).  For stability,
	 * either add or subtract a small distance (10 m) to or from the 
	 * starting co-latitude.
	 */

	if (remainder_lat < DBL_EPSILON && remainder_lon < DBL_EPSILON)
	{
	    if (azimuth < M_PI_2)
	    {
		igrid = (int) lp_data[ph_index].grid_indice[ilat-1][ilon];
		    if(ilat>0) ilat=0;
		*geoc_co_lat -= 0.0001;
		co_lat -= spacing;
	    }
	    else
		*geoc_co_lat += 0.0001;
	}

	/*
	 * Save current lower (north; 0) and upper (south; 1) co-latitude
	 * and lower (west; 0) and upper (east; 1) longitude values of grid
	 * in static arrays, cur_lat_bounds and cur_lon_bounds (in radians).
	 */

	cur_lon_bounds[0] = lon * DEG_TO_RAD;
	cur_lon_bounds[1] = (lon + spacing) * DEG_TO_RAD;
	if (cur_lon_bounds[0] > M_PI)
	    cur_lon_bounds[0] -= TWO_PI;
	if (cur_lon_bounds[1] > M_PI)
	    cur_lon_bounds[1] -= TWO_PI;

	return (igrid); 
}

double
LP_trace_ray (double ev_lat, double ev_lon, double sta_lat, double sta_lon,
int ph_index, double period)
{
	double total_travel_time=0.;
	double *total_travel_times=0;
	long ntimes=1;

	total_travel_times = LP_trace_rays(ev_lat, ev_lon, sta_lat, sta_lon, ph_index, &period, ntimes);
	if(total_travel_times) {
		total_travel_time=*total_travel_times;
		free(total_travel_times);
	}
	return(total_travel_time);
}

void
set_LP_vel_files(char *LR_file,char *LQ_file)
{
	char * str;
	str=(char*)calloc(strlen(LR_file)+1,sizeof(char));
	strcpy(str,LR_file);
	LP_vel_file[LR]=str;
	str=(char*)calloc(strlen(LQ_file)+1,sizeof(char));
	strcpy(str,LQ_file);
	LP_vel_file[LQ]=str;
}

void
set_LP_grid_files(char *LR_file,char *LQ_file)
{
	char * str;
	str=(char*)calloc(strlen(LR_file)+1,sizeof(char));
	strcpy(str,LR_file);
	LP_grid_file[LR]=str;
	str=(char*)calloc(strlen(LQ_file)+1,sizeof(char));
	strcpy(str,LQ_file);
	LP_grid_file[LQ]=str;
}

double *
get_LP_velocities(double lat, double lon, int ph_index, double *period, long nperiod)
{
//	double spacing;
	long iper;
	int ilat,ilon;
	double co_lat;
//	double lon_360;
//	double remainder_lat, remainder_lon;
	double *values=0;

//	spacing = lp_data[ph_index].latlon_spacing;

	co_lat = lat_conv (lat, TRUE, TRUE, TRUE, TRUE, FALSE);
	/*
	 * Get LP grid index for first grid cell (starting point).
	 * Start by representing all longitude information from
	 * 0 to 360 deg.
	 */

/*
	if (lon < 0.0)
	    lon_360 = lon + 360.0;
	else
	    lon_360 = lon;
*/

	/*
	 * Set latitude and longitude indexes, that is, ilat and ilon, 
	 * respectively.  Note that input co-latitudes and longitudes 
	 * which fall along a boundary will be put into the next 
	 * co-latitude or longitude grid cell.  
	 */

//	remainder_lat = modf ((co_lat/spacing), &co_lat);
//	remainder_lon = modf ((lon_360/spacing), &lon);
	ilat = (int) co_lat;
	ilon = (int) lon;

	/*
	 * Get values for this grid region for all periods
	 */
	values=(double *)calloc(nperiod,sizeof(double));
	for(iper=0;iper<nperiod;iper++) {
		values[iper]=get_LP_velocity(ilat,ilon,period[iper],ph_index);
	}
	return(values);
}

double *
get_LP_periods(int ph_index,long *nperiod)
{
	double * period;
	long nper;
	nper=lp_data[ph_index].num_periods;
	period=(double *) calloc(nper,sizeof(double));
	memcpy((char *)period,(char *)lp_data[ph_index].period_samples,nper*sizeof(double));
	*nperiod=nper;
	return(period);
}

RAY_Data *
LP_trace_ray_grid (double ev_lat, double ev_lon, double sta_lat, double sta_lon,
int ph_index, long *ngrid, double period)
{

	Bool	final_grid_found = FALSE;
	int	ilat, ilon, nlat, nlon;
	int	grid_position;
	double	co_lat, lon;
	double	lon_360;
	double	start_lon, end_lon;
	double	start_co_lat, end_co_lat;
	double	ev_co_lat, sta_co_lat;
	double	remainder_lat, remainder_lon;
	double	cur_lat_bounds[2], cur_lon_bounds[2];
	double	cur_azi, cur_baz, cur_delta;
	double	azi[3], baz[3], delta[3];
	double	spacing, tmp;
	double	delta_to_edge, new_co_lat, new_lon;
	double	delta_diff, orig_delta;
	double	sum_of_deltas = 0.0;
	RAY_Data *ray_data=0;
	long	nraygrid=0;
	long	maxraygrid=lp_data[ph_index].num_lon_grids+lp_data[ph_index].num_lat_grids;

	if(!(ray_data=(RAY_Data *)calloc(maxraygrid,sizeof(RAY_Data))))
		fprintf (stderr,"Error allocating %ld elements for ray_data\n",maxraygrid);
	/*
	 * To simplify matters we will always trace from west to east.
	 * Therefore, set western and eastern longitude bounds here, 
	 * start_lon and end_lon, respectively.  First, convert 
	 * geographic event and station latitudes (deg.) into geocentric 
	 * co-latitude (deg.).
	 */

	ev_co_lat = lat_conv (ev_lat, TRUE, TRUE, TRUE, TRUE, FALSE);
	sta_co_lat = lat_conv (sta_lat, TRUE, TRUE, TRUE, TRUE, FALSE);

	/*
	 * To avoid crossing exactly across North or South Poles add a
	 * small ammount to one of the longitudes.  If ray passes exactly
	 * thru one of the poles, then this can create some numerical 
	 * and book-keeping headaches which are not worth the trouble.
	 */

	if (ev_lon != sta_lon && fabs (ev_lon - sta_lon) == 180.0)
	    sta_lon += 0.01;

	/*
	 * Compute distance, azimuth and back-azimuth starting with event
	 * and ending with station.  This will be re-ordered if the
	 * station is found to be west of the event.
	 */

	geoc_distaz (ev_co_lat*DEG_TO_RAD, ev_lon*DEG_TO_RAD,
		     sta_co_lat*DEG_TO_RAD, sta_lon*DEG_TO_RAD,
		     &cur_delta, &cur_azi, &cur_baz, 0);

	orig_delta = cur_delta;

	if (cur_azi > M_PI)
	{
	    start_co_lat = sta_co_lat;
	    start_lon = sta_lon;
	    end_co_lat = ev_co_lat;
	    end_lon = ev_lon;
	    tmp = cur_azi;
	    cur_azi = cur_baz;
	    cur_baz = tmp;
	}
	else
	{
	    start_co_lat = ev_co_lat;
	    start_lon = ev_lon;
	    end_co_lat = sta_co_lat;
	    end_lon = sta_lon;
	}

	spacing = lp_data[ph_index].latlon_spacing;
	nlat = (int)(180.0/spacing);
	nlon = (int)(360.0/spacing);

	/*
	 * Get LP grid index for first grid cell (starting point).
	 * Start by represeting all longitude information from
	 * 0 to 360 deg.
	 */

	if (start_lon < 0.0)
	    lon_360 = start_lon + 360.0;
	else
	    lon_360 = start_lon;

	/*
	 * Set latitude and longitude indexes, that is, ilat and ilon, 
	 * respectively.  Note that input co-latitudes and longitudes 
	 * which fall along a boundary will be put into the next 
	 * co-latitude or longitude grid cell.  
	 */

	remainder_lat = modf ((start_co_lat/spacing), &co_lat);
	remainder_lon = modf ((lon_360/spacing), &lon);
	ilat = (int) co_lat;
	ilon = (int) lon;

	/*
	 * Special case: If both co-latitude and longitude fall on a 
	 * boundary (i.e., a corner), determine whether ray is moving
	 * into southerly (will stay in prescribed grid) or northerly
	 * (will move into latitude grid immediately above).  For stability,
	 * either add or subtract a small distance (10 m) to or from the 
	 * starting co-latitude.
	 */

	if (remainder_lat < DBL_EPSILON && remainder_lon < DBL_EPSILON)
	{
	    if (cur_azi < M_PI_2)
	    {
		--ilat;
		start_co_lat -= 0.0001;
		co_lat -= spacing;
	    }
	    else
		start_co_lat += 0.0001;
	}

	/*
	 * Save current lower (north; 0) and upper (south; 1) co-latitude
	 * and lower (west; 0) and upper (east; 1) longitude values of grid
	 * in static arrays, cur_lat_bounds and cur_lon_bounds (in radians).
	 * Also convert spacing into radians for the rest of this routine.
	 */

	cur_lat_bounds[0] = co_lat * spacing * DEG_TO_RAD;
	cur_lon_bounds[0] = lon * spacing * DEG_TO_RAD;

	spacing *= DEG_TO_RAD;

	cur_lat_bounds[1] = cur_lat_bounds[0] + spacing;
	cur_lon_bounds[1] = cur_lon_bounds[0] + spacing;
	if (cur_lon_bounds[0] > M_PI)
	    cur_lon_bounds[0] -= TWO_PI;
	if (cur_lon_bounds[1] > M_PI)
	    cur_lon_bounds[1] -= TWO_PI;


	/*
	 * Main iterative loop.  Rays will be trace from grid cell boundary
	 * to grid cell boundary until the complete ray path is traversed.
	 * Since the eastern boundary (EAST) will almost always be longer
	 * than the northern (NORTH) or southern (SOUTH) boundaries, we will
	 * check for it first.  Fortunately, some economies can be made
	 * here as well.  We will always start at the NE corner and inspect
	 * the SE second.  This will tells us quickly whether or not our
	 * exit ray will least the eastern boundary.  Please note that all
	 * directions are given relative to the "previous" grid cell exit
	 * points.  Therefore, an EAST grid position is actually emanating
	 * from the western edge for the current cell.  When a new edge is
	 * determined, then the direction is strictly true.  One way to 
	 * understand this choice is by realizing the direction indicates
	 * the general direction of the ray itself (specifically, whether
	 * it is more NORTH than SOUTH).  All rays are more EAST than WEST,
	 * by definition.  Please try to keep aware of this fact!
	 */


	/* Initialize starting point. */

	grid_position = INSIDE;
	co_lat = start_co_lat*DEG_TO_RAD;
	lon    = start_lon*DEG_TO_RAD;
	end_co_lat = end_co_lat*DEG_TO_RAD;
	end_lon = end_lon*DEG_TO_RAD;

	while (final_grid_found != TRUE)
	{
	    /* Always start with NE corner, then SE */
	    geoc_distaz (co_lat, lon, cur_lat_bounds[0], cur_lon_bounds[1],
	    		 &delta[NE], &azi[NE], &baz[NE], 0);
	    if (azi[NE] > M_PI)
	    {
		azi[NE] = 0.0;
		baz[NE] = M_PI;
	    }
	    geoc_distaz (co_lat, lon, cur_lat_bounds[1], cur_lon_bounds[1],
	    		 &delta[SE], &azi[SE], &baz[SE], 0);
	    if (azi[SE] > M_PI)
	    {
		azi[SE] = M_PI;
		baz[SE] = 0.0;
	    }
	    if (cur_azi > azi[NE] && cur_azi <= azi[SE])
	    {
		grid_position = EAST;

		/*
		 * Since the eastern longitude boundary defines a great
		 * circle, we only need subtract exactly 180 deg. from
		 * the back-azimuth (baz).  Note that we could avoid a 
		 * call to geoc_lat_lon() by just computing the side 
		 * coincident with the longitude great circle at the 
		 * eastern boundary.  Unfortunately, we still need to 
		 * calculate the distance from our current working 
		 * point to the edge to test if this is the final grid 
		 * cell.
		 */

		delta_to_edge =
		dist_given_2angles_plus_side (baz[NE]-M_PI, cur_azi-azi[NE],
					      delta[NE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final distance segment.
		     */
		    if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		    ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		    ray_data[nraygrid].ilat=ilat;
		    ray_data[nraygrid].ilon=ilon;
		    ray_data[nraygrid].dist=cur_delta*RAD_TO_DEG/KM_TO_DEG;
		    if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		    nraygrid++;
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add distance for this segment
		 */
		if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		ray_data[nraygrid].ilat=ilat;
		ray_data[nraygrid].ilon=ilon;
		ray_data[nraygrid].dist=delta_to_edge*RAD_TO_DEG/KM_TO_DEG;
		if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		nraygrid++;
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lon_bounds[0] = new_lon;
		cur_lon_bounds[1] = new_lon + spacing;
		if (cur_lon_bounds[1] > M_PI)
		    cur_lon_bounds[1] -= TWO_PI;
		++ilon;
		if (ilon >= nlon)
		    ilon -= nlon;
		continue;
	    }

	    /*
	     * Ray must exit from NORTH or SOUTH co-latitude boundaries!

	     * If emanating from INSIDE or EAST boundary, then determine
	     * whether exit ray will leave NORTH or SOUTH boundary.  Note
	     * that EAST exit test has already been done.
	     */

	    if (grid_position == INSIDE || grid_position == EAST)
	    {
		if (cur_azi > azi[SE] && cur_azi <= M_PI)
		    grid_position = SOUTH;		/* Will exit to SOUTH */
		else
		    grid_position = NORTH;		/* Will exit to NORTH */
	    }

	    if (grid_position == SOUTH)
	    {
		geoc_distaz (cur_lat_bounds[1], cur_lon_bounds[1], 
			     cur_lat_bounds[1], lon, &delta[WNS], 
			     &azi[WNS], &baz[WNS], 0);

		delta_to_edge =
		dist_given_2angles_plus_side (baz[SE]-azi[WNS], cur_azi-azi[SE],
					      delta[SE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final distance segment.
		     */
		    if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		    ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		    ray_data[nraygrid].ilat=ilat;
		    ray_data[nraygrid].ilon=ilon;
		    ray_data[nraygrid].dist=cur_delta*RAD_TO_DEG/KM_TO_DEG;
		    if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		    nraygrid++;
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add distance for this segment
		 */
		if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		ray_data[nraygrid].ilat=ilat;
		ray_data[nraygrid].ilon=ilon;
		ray_data[nraygrid].dist=delta_to_edge*RAD_TO_DEG/KM_TO_DEG;
		if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		nraygrid++;
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lat_bounds[0] = new_co_lat;
		cur_lat_bounds[1] = new_co_lat + spacing;
		++ilat;
		if (cur_lat_bounds[1] > M_PI)
		{
		    cur_lat_bounds[0] = M_PI - spacing;
		    cur_lat_bounds[1] = M_PI;
		    /* --ilat; Error! */	/* Wrap around S. Pole */
		    if(ilat > nlat-1) ilat=nlat-1;
		}
		continue;
	    }
	    else	/* Exits to NORTH */
	    {
		geoc_distaz (cur_lat_bounds[0], cur_lon_bounds[1], 
			     cur_lat_bounds[0], lon, &delta[WNS], 
			     &azi[WNS], &baz[WNS], 0);

		delta_to_edge =
		dist_given_2angles_plus_side (azi[WNS]-baz[NE], azi[NE]-cur_azi,
					      delta[NE]);
		if ((cur_delta - delta_to_edge) < SMALL_DIST)
		{
		    /*
		     * Final cell found!!!  We only need to compute
		     * the final distance segment.
		     */
		    if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		    ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		    ray_data[nraygrid].ilat=ilat;
		    ray_data[nraygrid].ilon=ilon;
		    ray_data[nraygrid].dist=cur_delta*RAD_TO_DEG/KM_TO_DEG;
		    if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		    nraygrid++;
		    sum_of_deltas += cur_delta;
		    break;
		}

		geoc_lat_lon (co_lat, lon, delta_to_edge, cur_azi, 
			      &new_co_lat, &new_lon);

		/*
		 * Add distance for this segment
		 */
		if(!check_bounds(ilat, ilon, ph_index)) {free(ray_data); *ngrid=0; return(0);}
		ray_data[nraygrid].igrid = (int) lp_data[ph_index].grid_indice[ilat][ilon];
		ray_data[nraygrid].ilat=ilat;
		ray_data[nraygrid].ilon=ilon;
		ray_data[nraygrid].dist=delta_to_edge*RAD_TO_DEG/KM_TO_DEG;
		if(period>0.) ray_data[nraygrid].value=get_LP_velocity (ilat, ilon, period, ph_index);
		nraygrid++;
		sum_of_deltas += delta_to_edge;

		/*
		 * Always re-compute new forward azimuth (cur_azi) for each
		 * new starting point.
		 */

		geoc_distaz (new_co_lat, new_lon, end_co_lat, end_lon,
			     &cur_delta, &cur_azi, &cur_baz, 0);

		if (cur_delta < SMALL_DIST)
		    break;	/* Special case: distance < 10 meters */

		co_lat = new_co_lat;
		lon = new_lon;
		cur_lat_bounds[1] = new_co_lat;
		cur_lat_bounds[0] = new_co_lat - spacing;
		--ilat;
		if (cur_lat_bounds[0] < 0.0)
		{
		    cur_lat_bounds[0] = 0.0;
		    cur_lat_bounds[1] = spacing;
		    if(ilat<0) ilat=0;
		    /* ++ilat;	Error! */	/* Wrap around N. Pole */
		}
		continue;
	    }

	}	/* End of main iterative loop over grid cells */


	/*
	 * If sum of all individual distance segments is different from
	 * original distance calculation by more than 0.1%, then print
	 * an warning message.
	 */

	delta_diff = fabs (orig_delta - sum_of_deltas);

	if (delta_diff > 0.001 * orig_delta)
	    fprintf (stderr, "Warning: Sum of individual LP segments differ by > 0.1%% from original distance!\n");

	*ngrid=nraygrid;
	return (ray_data);
}

void
get_LP_grid_limits(int ph_index,int *nlat,int *nlon,int *nperiod,int *nmodel)
{
        *nlat=lp_data[ph_index].num_lat_grids;
        *nlon=lp_data[ph_index].num_lon_grids;
	*nperiod=lp_data[ph_index].num_periods;
	*nmodel=lp_data[ph_index].num_indexes;
}

/* New functions to support variable number of files and compiled files */

static void free_LP_files()
{
	int i;
	for(i=0;i<ngrid_file;i++) {
		free(LP_grid_file[i]);
		free(LP_vel_file[i]);
	}
	if(LP_grid_file) free(LP_grid_file); LP_grid_file=0;
	if(LP_vel_file) free(LP_vel_file); LP_vel_file=0;
	ngrid_file=0;
	for(i=0;i<ncomp_file;i++) {
		free(LP_comp_file[i]);
	}
	if(LP_comp_file) free(LP_comp_file); LP_comp_file=0;
	ncomp_file=0;
	nread_file=0;
}

void set_LP_grid_file( char *grid_file, char *vel_file) 
{
	free_LP_files();
	LP_grid_file=(char **) calloc(1,sizeof(char *));
	LP_grid_file[0]=(char *) calloc(strlen(grid_file)+1,sizeof(char));
	strcpy(LP_grid_file[0],grid_file);
	LP_vel_file=(char **) calloc(1,sizeof(char *));
	LP_vel_file[0]=(char *) calloc(strlen(vel_file)+1,sizeof(char));
	strcpy(LP_vel_file[0],vel_file);
	ngrid_file=1;
}

void set_LP_comp_file( char *comp_file)
{
	free_LP_files();
	LP_comp_file=(char **) calloc(1,sizeof(char *));
	LP_comp_file[0]=(char *) calloc(strlen(comp_file)+1,sizeof(char));
	strcpy(LP_comp_file[0],comp_file);
	ncomp_file=1;
}

void add_LP_grid_file( char *grid_file, char *vel_file) 
{
	if(ngrid_file<=0) {
		set_LP_grid_file(grid_file,vel_file);
	} else {
		LP_grid_file=(char **) realloc(LP_grid_file,(ngrid_file+1)*sizeof(char *));
		LP_grid_file[ngrid_file]=(char *) calloc(strlen(grid_file)+1,sizeof(char));
		strcpy(LP_grid_file[ngrid_file],grid_file);
		LP_vel_file=(char **) realloc(LP_vel_file,(ngrid_file+1)*sizeof(char *));
		LP_vel_file[ngrid_file]=(char *) calloc(strlen(vel_file)+1,sizeof(char));
		strcpy(LP_vel_file[ngrid_file],vel_file);
		ngrid_file++;
	}
}

void add_LP_comp_file( char *comp_file)
{
	if(ncomp_file<=0) {
		set_LP_comp_file(comp_file);
	} else {
		LP_comp_file=(char **) realloc(LP_comp_file,(ncomp_file+1)*sizeof(char *));
		LP_comp_file[ncomp_file]=(char *) calloc(strlen(comp_file)+1,sizeof(char));
		strcpy(LP_comp_file[ncomp_file],comp_file);
		ncomp_file++;
	}
}

int
read_compiled_file(char * lp_pathway, char *LR_file, int ph_index)
{
	char	file_name[FILENAMELEN];
	FILE	*cfp=NULL;
	int	i, j, k;
	int	nlat, nlon;
	int	nindex, nper;
	double	spacing;
	static	char	routine[]="read_compiled_file";

	/*
	 * Open binary file and set file pointer cfp
	 */
		strcpy (file_name, lp_pathway);
		strcat (file_name, "/");
		strcat (file_name, LR_file);
		if ((cfp = fopen (file_name, "r")) == NULL)
		{
	    		fprintf (stderr, "Error: %s: File: %s is missing!\n",
			     routine, file_name);
	    		return (0);
		}
	
	    /* Initialize lp_data structure */

		k = ph_index;
	    	lp_data[k].num_lat_grids = 0;
	    	lp_data[k].num_lon_grids = 0;
	    	lp_data[k].latlon_spacing = 0.0;
	    	lp_data[k].grid_indice = (short **) NULL;
	    	lp_data[k].num_periods = 0;
	    	lp_data[k].num_indexes = 0;
	    	lp_data[k].period_samples = (double *) NULL;
	    	lp_data[k].velocity = (double **) NULL;

	    /*
	     * Begin reading grid indice information
	     */
		if (fread(&nlat,sizeof(nlat),1,cfp) != 1)  return (0);
#ifndef WORDS_BIGENDIAN
                endian_revert((char *)&nlat, 1, sizeof(nlat));
#endif
		if (fread(&nlon,sizeof(nlon),1,cfp) != 1)  return (0);
#ifndef WORDS_BIGENDIAN
                endian_revert((char *)&nlon, 1, sizeof(nlon));
#endif
		if (fread(&spacing,sizeof(spacing),1,cfp) != 1)	return (0);
#ifndef WORDS_BIGENDIAN
                endian_revert((char *)&spacing, 1, sizeof(spacing));
#endif
	    	lp_data[k].num_lat_grids = nlat;
	    	lp_data[k].num_lon_grids = nlon;
	    	lp_data[k].latlon_spacing = spacing;

	    	if ((lp_data[k].grid_indice = (short **) 
			calloc (nlat, sizeof (short *))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].grid_indice");
			fclose(cfp);
			return (0);
	    	}
	    	for (i = 0; i < nlat; i++)
	    	{
			if ((lp_data[k].grid_indice[i] = (short *)
				calloc (nlon, sizeof (short))) == NULL)
			{
		    	CALLOC_ERR ("lp_data[].grid_indice[]");
			fclose(cfp);
		    	return (0);
			}
	    	}

	    /*
	     * Do actual reading of LP grid indices here!
	     */
	    	for (j = 0; j < nlat; j++)
	    	{
		  if (fread( lp_data[k].grid_indice[j],sizeof(short),nlon,cfp) != nlon) return (0);
#ifndef WORDS_BIGENDIAN
                        endian_revert((char *)lp_data[k].grid_indice[j], nlon, sizeof(short));
#endif
	    	}
	    /*
	     * Now read index/velocity tables.
	     */
		if (fread(&nindex,sizeof(nindex),1,cfp) != 1) return (0);
#ifndef WORDS_BIGENDIAN
                endian_revert((char *)&nindex, 1, sizeof(nindex));
#endif
		if (fread(&nper,sizeof(nper),1,cfp) != 1) return (0);
#ifndef WORDS_BIGENDIAN
                endian_revert((char *)&nper, 1, sizeof(nper));
#endif

	    	lp_data[k].num_indexes = nindex;
	    	lp_data[k].num_periods = nper;

	    	if ((lp_data[k].period_samples = 
			(double *) calloc (nper, sizeof (double))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].period_samples");
			fclose(cfp);
			return (0);
	    	}
	    /*
	     * Read period samples for velocity file here.
	     */
		if (fread( lp_data[k].period_samples,sizeof(double),nper,cfp) != nper) return (0);
#ifndef WORDS_BIGENDIAN
		endian_revert((char *)lp_data[k].period_samples, nper, sizeof(double));
#endif

	    	if ((lp_data[k].velocity = (double **) calloc (nindex,
						sizeof (double *))) == NULL)
	    	{
			CALLOC_ERR ("lp_data[].velocity");
			fclose(cfp);
			return (0);
	    	}
	    	for (i = 0; i < nindex; i++)
	    	{
			if ((lp_data[k].velocity[i] = (double *) calloc (nper,
						sizeof (double))) == NULL)
			{
		    		CALLOC_ERR ("lp_data[].velocity[]");
				fclose(cfp);
		    		return (0);
			}
	    	}

	    	/*
	     	 * Finally, read actual long-period phase velocities for all
	     	 * indexes and periods.
	     	*/

	    	for (j = 0; j < nindex; j++)
	    	{
		  if (fread( lp_data[k].velocity[j],sizeof(double),nper,cfp) != nper) return(0);
#ifndef WORDS_BIGENDIAN
                        endian_revert((char *)lp_data[k].velocity[j], nper, sizeof(double));
#endif
	    	}
	fclose(cfp);
	return(1);
}

int
write_compiled_file(char * lp_pathway, char *LR_file, int ph_index)
{
	char	file_name[FILENAMELEN];
	FILE	*cfp=NULL;
	int	j, k;
	int	nlat, nlon;
	int	nindex, nper;
	double	spacing;
	static	char	routine[]="write_compiled_file";

	/*
	 * Check to see if there is data to write out
	 */
	if(ph_index > nread_file-1) {
		fprintf(stderr, "Error: %s: no data to write - beyond end of array\n",routine);
		return 0;
	}
	if(!lp_data) {
		fprintf(stderr, "Error: %s: no data to write - null lp_data\n",routine);
		return 0;
	}
	/*
	 * Open binary file and set file pointer cfp
	 */
		strcpy (file_name, lp_pathway);
		strcat (file_name, "/");
		strcat (file_name, LR_file);
		if ((cfp = fopen (file_name, "w")) == NULL)
		{
	    		fprintf (stderr, "Error: %s: File: %s is missing!\n",
			     routine, file_name);
	    		return (0);
		}
	    /*
	     * Begin writing grid indice information
	     */
		k = ph_index;
	    	nlat = lp_data[k].num_lat_grids;
	    	nlon = lp_data[k].num_lon_grids;
	    	spacing = lp_data[k].latlon_spacing;
		fwrite(&nlat,sizeof(nlat),1,cfp);
		fwrite(&nlon,sizeof(nlon),1,cfp);
		fwrite(&spacing,sizeof(spacing),1,cfp);
	    /*
	     * Do actual writing of LP grid indices here!
	     */
	    	for (j = 0; j < nlat; j++)
	    	{
			fwrite( lp_data[k].grid_indice[j],sizeof(short),nlon,cfp);
	    	}
	    /*
	     * Now write index/velocity tables.
	     */
	    	nindex = lp_data[k].num_indexes;
	    	nper = lp_data[k].num_periods;
		fwrite(&nindex,sizeof(nindex),1,cfp);
		fwrite(&nper,sizeof(nper),1,cfp);
	    /*
	     * Write period samples for velocity file here.
	     */
		fwrite( lp_data[k].period_samples,sizeof(double),nper,cfp);

	    	/*
	     	 * Finally, write actual long-period phase velocities for all
	     	 * indexes and periods.
	     	 */

	    	for (j = 0; j < nindex; j++)
	    	{
			fwrite( lp_data[k].velocity[j],sizeof(double),nper,cfp);
	    	}
	fclose(cfp);
	return(1);
}

static int check_bounds(int ilat, int ilon, int ph_index)
{
	int inbounds=1;
	if (ilat < 0 || ilon < 0 || ilat >= lp_data[ph_index].num_lat_grids ||
		ilon >= lp_data[ph_index].num_lon_grids) inbounds=0;
	return inbounds;
}
