
/*
 * Copyright (c) 1993-1994 Science Applications International Corporation.
 *

 * NAME
 *	init_aoi_grid \- Read area of interest (AOI) grid from file.
 * 	get_area_of_interest_string \- Get 4 character AOI string.

 * FILE 
 *	area_of_interest.c

 * SYNOPSIS
 *	int
 *	get_area_of_interest_string (lat, lon, aoi_string)
 *	double	lat;		(i) latitude to use for look-up. 
 *	double	lon;		(i) latitude to use for look-up. 
 *	char	*aoi_string;	(o) pointer to character string 
 *				    to contain result.

 * 	int
 *	init_aoi_grid (file)
 *	char	*file;	(i) Input path/filename containing grid information

 *	int
 *	get_aoi_info (lat, lon, aoi, hs, ss, ld)
 *	double	lat;	(i) Latitude of point of interest (deg)
 *	double	lon;	(i) Longitude of point of interest (deg)
 *	Bool	*aoi;	(o) Area of interest?
 *	Bool	*hs;	(o) High seismicity area?
 *	Bool	*ss;	(o) Shallow seismicity area?
 *	Bool	*ld;	(o) On land?

 *	static    
 *	int cell_compare (i, j)
 *	Cell *i, *j;	(i) Input cell structures 

 * DESCRIPTION
 *	Functions.  Set of routines for creating and accessing parameters 
 *	for cells of a grid of the Earth.  Parameters describe whether a cell
 *	of the grid is seismically "interesting".
 *
 *	get_area_of_interest_string () is a C callable function which uses 
 *	the input lat/lon to perform a look-up of the corresponding 
 *	area-of-interest data.  The data is returned as a four character string 
 *	describing area-of-interest, high-seismicity, shallow-seismicity and 
 *	land.  Upon successful completion, get_area_of_interest_striing () 
 *	returns OK, otherwise ERR.

 *	init_aoi_grid () initializes a static "area of interest" grid.  
 *	It opens the input file, sets up a dynamic array for grid cells, 
 *	reads data for one grid cell per line, determine a grid cell index
 *	for that cell according to grid parameters, and orders cells in 
 *	the grid by cell index.  Each line in file is assumed to have format:
 *
 *		lat lon area-of-interest hi-seismicity shallow-seismicity land 
 *
 *	where lat,lon 0<lat<180, 0<lon<360, and other parameters are strings
 *	either "T" or "F".  Assumed lat is defined as 0 at north pole.
 *	Parameters defining the grid are as follows:
 *
 *		Each grid cell is 1x1 degree in size
 *		There are 180 lat grid cells, 360 lon grid cells
 *		Grid re-maps lat,lon to -90<lat<90, -180<lon<180 with
 *			origin at lat=-90, lon=-180
 *		Grid defines cell indices ordered by latitude
 *	Returns OK when grid is set, returns ERR if error is encountered.

 * 	get_aoi_info () uses the grid initialized in init_aoi_grid. It
 *	initializes output to FALSE, determines grid cell of input lat,lon,
 * 	and searches the grid for this cell. If the cell is found, the
 *	grid values are set and OK is returned. If the cell is not found,
 *	ERR is returned.  Assumes -90<lat<90, -180<lon<180.

 *	cell_compare () used by qsort() for binary search.

 * DIAGNOSTICS
 *	None.

 * FILES
 *	init_aoi_grid () assumes input file format defined above.
 *	init_aoi_grid () uses dyn_array.h

 * AUTHORS
 *	Darrin Wahl	17 Jan 93	Created original files.
 *	Shawn Wass	28 Jan 93	Modified for use in ARS.
 *	Shawn Wass	 1 Apr 93	Modified so it knows if an AOI table 
 *					was previously read or not.  Changed 
 *					error reporting.
 *	Shawn Wass	 4 Jan 94	Removed the slib_err() calls since 
 *					this is typically called from the C
 *					level and it causes a core dump since 
 *					the stack was saved previously... it 
 *					can retore the correct stack (?).
 *	Walter Nagy	 5 Oct 94	Moved 4 of 6 functions from ARS src
 *					area (file, user_aoi.c) to libgeog.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef	SVR4
#include <string.h>
#include "libaesir.h"
#else
#include <strings.h>
#endif

#include "libgeog.h"
#include "aesir.h"
#include "dyn_array.h"

/* 
 *  Grid parameters definitions 
 *  If want to change default grid parameters, do it here.
 */

#define	LATINIT -90.0
#define	LONINIT -180.0
#define DLAT 	1.0
#define DLON 	1.0
#define NLAT 	180
#define NLON 	360
#define ORDERBY	"lat"

/* 
 *  Consistency checks for ranges of lat,lon 
 *  These checks must correspond to grid parameters defined above
 *  and input file definition
 */

#define	FILELATOK(a)	(((a) < 0.0 || (a) > 180.0) ? FALSE : TRUE)
#define	FILELONOK(a)	(((a) < 0.0 || (a) > 360.0) ? FALSE : TRUE)
#define	INPUTLATOK(a)	(((a) < -90.0 || (a) > 90.0) ? FALSE : TRUE)
#define	INPUTLONOK(a)	(((a) < -180.0 || (a) > 180.0) ? FALSE : TRUE)

/* 
 *  Map co-latitude to range -90<lat<90, longitude to range -180<lon<180 
 *  assuming input 0<lat<180, 0<lon<360, and lat=0 at north pole.
 *  This mapping must correspond to grid parameters defined above.
 */

#define LAT_MAP(a)	90.0 - (a)
#define LON_MAP(a)	((a) >= 180.0) ? (a) - 360.0 : (a)

/* 
 *  Test bit fields in Cell structure.
 */

#define BITSET(a)	((a) == 0) ? FALSE : TRUE

/* 
 *  One grid cell structure contains the grid cell index and 
 *  the information flags for that grid cell.
 *  Use fields one bit wide for memory conservation.
 */

typedef struct
{
	int	icell;		  /* Grid cell index based on grid params */
	short	is_area_of_interest : 1;	/* Area of interest? */
	short  	is_high_seismicity : 1;		/* High seismicity area? */
	short	is_shallow_seismicity : 1;	/* Shallow seismicity area? */
	short	is_land : 1;			/* On land? */
} Cell;

/* 
 *  Grid structure containing grid cells and parameters defining grid.
 */

typedef struct
{
	Cell	*cell;		/* Array of cells of grid */
	int	ncell;		/* Number of grid cells */
	float	lat_init;	/* Initial latitude defining grid origin(deg) */
	float	lon_init;	/* Initial longitude defining grid origin(deg) */
	float	dlat;		/* Latitude increment (deg) */
	float	dlon;		/* Longitude increment (deg) */
	int	nlat;		/* Number of latitude points in grid */
	int	nlon;		/* Number of longitude points in grid */
	char	orderby[5];	/* "lat" or "lon" for ordering cell indices */
} Grid;	


static int cell_compare(const void *a, const void *b);

/*
 *  External function declarations.
 */

extern int fclose(FILE *stream);

static	Grid	grid;			/* Current data grid */
static	Bool	table_read = FALSE;
static	char	*save_file = (char *) NULL;


/*
 * Initialize static area of interest grid.  Open input file, set up a dynamic 
 * array for grid cells, read data for one grid cell per line, 
 * determine grid cell index for that cell according to grid parameters,
 * order cells in grid by cell index.
 */

int
init_aoi_grid (char *file)
{
	Array	carray;		/* Dynamic array of cell structures */
	Cell	c;		/* Local cell structure */
	float 	lat, lon;	/* Local latitude, longitude (deg) */
	int	ilat, ilon;	/* Local integer lat, lon for icell compute */
	char 	areaint[2];	/* "T" or "F" for area-of-interest. */
	char 	high_seismicity[2];    /* "T" or "F" for high seismicity. */
	char 	shallow_seismicity[2]; /* "T" or "F" for shallow seismicity. */
	char 	land[2];	/* "T" or "F" for on land */
	int	retval = ERR;	/* Return value */
	FILE 	*fp;


	if (table_read && STREQ (file, save_file))
		return (OK);

	/* Initialize grid parameters and de-allocate previous grid cells */
	grid.lat_init = LATINIT;
	grid.lon_init = LONINIT;
	grid.dlat = DLAT;
       	grid.dlon = DLON;	
       	grid.nlat = NLAT;
	grid.nlon = NLON;
	sprintf (grid.orderby, ORDERBY);
	UFREE (grid.cell);


	/* Dynamic array for each grid cell */
	carray = array_create (sizeof (Cell));
	

	/* Open file for reading */
	if ((fp = fopen (file, "r")) == (FILE *) NULL)
	{
		fprintf (stderr, "Unable to open file \"%s\"\n", file);
		return retval;
	}
	
	fprintf (stdout, "Reading %s\n", file);

	/* Set up each grid cell based on data from file */
	while (fscanf (fp, "%f%f%s%s%s%s", &lat, &lon, areaint,
			   high_seismicity, shallow_seismicity, land) != EOF)
	{ 
		/* Consistency check */
		if (!FILELATOK (lat) || !FILELONOK (lon))
		{
			fprintf (stderr, "Bad lat %f or lon %f in %s\n", 
					 lat, lon, file);
			fclose (fp);
			return retval;
		}			

		/* Re-map input latitude, longitude */
		lat = LAT_MAP (lat);
		lon = LON_MAP (lon);

		/* Determine cell index based on grid parameters */
		ilat = (int) ((lat - grid.lat_init) / grid.dlat) + 1;
		ilon = (int) ((lon - grid.lon_init) / grid.dlon) + 1;
		c.icell = ilon + (ilat - 1) * grid.nlon;


		/* Set bits for cell depending on data */
		c.is_area_of_interest = (areaint[0] =='T') ? 1 : 0;
		c.is_high_seismicity = (high_seismicity[0] =='T') ? 1 : 0;
		c.is_shallow_seismicity = (shallow_seismicity[0] =='T') ? 1 : 0;
		c.is_land = (land[0] =='T') ? 1 : 0;

		/* Add cell to dynamic array */
		(void) array_add (carray, (caddr_t) &c);
	}
	fclose (fp);

	/* Get the grid cell array */
	grid.ncell = array_count (carray);
	grid.cell = (Cell *) array_list (carray);
	
	/* Order grid cells by icell for later binary search */
	(void) qsort ((char *) grid.cell, (size_t) grid.ncell, sizeof (Cell),
		      cell_compare);

	/* Free dynamic array */
	(void) array_free (carray);
	
	table_read = TRUE;
	UFREE (save_file);
	save_file = STRALLOC (file);

	/* No errors */
	return OK;

}	/* init_aoi_grid () */



/* 
 * Initialize output to FALSE, determine grid cell of input lat,lon,
 * search grid for this cell. If cell found, set return values and return
 * OK. If cell not found, error message printed and return ERR value.
 */

int
get_aoi_info (double lat, double lon, Bool *aoi, Bool *high_seismicity,
 Bool *shallow_seismicity, Bool *land)
     /*double	lat;*/
     /*double	lon;*/
     /*Bool	*aoi;*/
     /*Bool	*high_seismicity;*/
     /*Bool	*shallow_seismicity;*/
     /*Bool	*land;*/
{
	Bool	found;		/* Whether a lat,lon pair was in the grid */
	int	ilat, ilon;	/* Integer lat,lon (deg) for icell compute */
	int	icell;		/* Grid cell index of input lat,lon */
	int	lo, hi, mid; 	/* Used for binary search of grid */
	int	retval = ERR;	/* Return value */


	/* Initialize to FALSE */
	*aoi = FALSE;
	*high_seismicity = FALSE;
	*shallow_seismicity = FALSE;
	*land = FALSE;

	/* Consistency check */
	if (INPUTLATOK (lat) && INPUTLONOK (lon))
	{
		ilat = 1 + (int) ((lat - grid.lat_init) / grid.dlat);
		ilon = 1 + (int) ((lon - grid.lon_init) / grid.dlon);	
	}		
	else
	{
		fprintf (stderr, "Bad lat %f or lon %f\n", lat, lon);
		return retval;
	}			

	/* Determine grid cell index for input lat,lon */
	if (STREQ (grid.orderby, "lat"))
	{
		icell = ilon + (ilat - 1) * grid.nlon;
	}
	else if (STREQ (grid.orderby, "lon"))
	{
		icell = ilat + (ilon - 1) * grid.nlat;
	}
	else
	{
		fprintf(stderr, "Unrecognized order-by flag: %s", grid.orderby);
		return retval;
	}
	
	/*
	 *  Binary search for icell within grid.  There may not be an entry for
	 *  icell, which indicates an incomplete grid or point out of bounds.
	 */
	lo = 0;
	hi = grid.ncell - 1;
	found = FALSE;
	while (lo <= hi)
	{
		mid = (lo + hi)/2;
		
		if (icell < grid.cell[mid].icell)
		{
			hi = mid - 1;
		}
		else if (icell > grid.cell[mid].icell)
		{
			lo = mid + 1;
		}
		else 
		{
			*aoi = BITSET (grid.cell[mid].is_area_of_interest);

			*high_seismicity = 
				BITSET (grid.cell[mid].is_high_seismicity);

			*shallow_seismicity = 
				BITSET (grid.cell[mid].is_shallow_seismicity);

			*land = BITSET (grid.cell[mid].is_land);

			found = TRUE;

			break;
		}
	}

	/* No icell entry for this lat,lon pair */
	if (found == FALSE)
	{
		return retval;
	}
	

	/* No errors */
	return OK;

}	/* get_aoi_info () */


static int 
cell_compare (const void *a, const void *b)
{
	Cell 	*i = (Cell *) a; 
	Cell	*j = (Cell *) b;

	if (i->icell > j->icell)
		return (1);
	else if (i->icell < j->icell)
		return (-1);
	else
		return (0);
}	


/*
 *  Using the lat/lon pair, lookup the AOI information from the 
 *  grid and return it as a string.  Returns a string containing 
 *  a 4 character with the following character mappings for each 
 *  true or false value:
 *
 *         AOI    Seismic    Deep    Land
 *         ---    -------    ---     ----
 *  TRUE :  A        S        S       L
 *  FALSE:  O        A        D       O
 */

int
get_area_of_interest_string (double lat, double lon, char *aoi_string)
/*double	lat;*/		/*  (i) latitude to use for look-up. */
/*double	lon;*/		/*  (i) latitude to use for look-up. */
/*char	*aoi_string;*/	/* (o) pointer to character string to contain result. */
{
	Bool	aoi, high_seismicity, shallow_seismicity, land;
	
	if (!table_read)
		return ERR;

	if (!aoi_string)
		return ERR;

	if (lat == -999.0 && lon == -999.0)
	{
		/*
		 *  These are the NULL lat/lon values - this is probably 
		 *  a new origin with NULL values. 
		 */
		return ERR;
	}

	if (! INPUTLATOK (lat) || ! INPUTLONOK (lon))
	{
		return ERR;
	}
		
	/*
	 *  Perform lookup from table.
	 */
	if (get_aoi_info (lat, lon, 
			  &aoi, &high_seismicity, &shallow_seismicity, &land) 
	    == OK)
	{
		/*
		 *  Build the string containing the values.
		 */
		(void) sprintf (aoi_string, "%s%s%s%s", 
				(aoi == True) ? "A" : "O", 
				(high_seismicity == True) ? "S" : "A", 
				(shallow_seismicity == True) ? "S" : "D", 
				(land == True) ? "L" : "O");

		return OK;
	}

	return ERR;

} /* End get_area_of_interest_string. */


