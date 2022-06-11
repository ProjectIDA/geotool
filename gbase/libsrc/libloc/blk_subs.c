/* 
 *      Copyright (c) 1996 Science Applications Internaional Corporation.
 *                 

 * NAME
 *

 * FILE
 *	blk_defs.h
 * 
 * SYNOPSIS
 *
 * 	int 
 *	setup_blk_facilities 
 *	(char *blk_prefix, Site *site, int num_sites )
 *
 * 
 * 	Bool 
 * 	blocked_path
 *	(char *sta , double lat_source, 
 *	double lon_source, Bool *back_branch )
 *
 *      Bool 
 *      blocked_path_max_dist
 *      (char *sta , double lat_source, 
 *      double lon_source, double *dist , double *az)
 *
 *     
 *	Bool 
 *	blocked_ellipse
 *      (char *sta , double lat_source, double lon_source, 
 * 	double smajax, double sminax, double strike, 
 *      Bool *back_branch )
 * 
 *      Bool 
 *      blocked_ellipse_dist
 *      (char *sta , double lat_source, 
 *      double lon_source, double smajax, double sminax, double strike, 
 *      double *dist_center, double *dist_min, double *dist_max, 
 *      double *min_azi, double *max_azi) ;
 *
 *	int 
 *  	num_blocked_stations (void)
 *
 *	Site * get_blocked_site_pt(int sta )
 *
 *      set_blk_radius (void)
 *   
 *      get_blk_radius (void)
 *
 *
 * DESCRIPTION
 *
 *     	setup_blk_facilities
 *     	Initializes the data structures for blockage information for all
 *     	stations where it is available.  
 *     
 *
 *     	blocked_path
 *     	Return a Boolean variable. 
 *	TRUE if path is clear (non-blocked)
 * 	FALSE otherwise.
 *     	The back_branch boolean lets you know if the path is clear
 *     	on the longest path between the two points.
 *      If back_branch=TRUE, the clear path is on a back_branch.
 *
 *	blocked_path_max_dist
 *     	Return a Boolean variable. 
 *	TRUE if path is clear (non-blocked)
 * 	FALSE otherwise.
 *      The dist argument gives the actual path length
 *      if the path is unblocked. It returns -1 otherwise.
 *
 *    	blocked_ellipse
 *      Return a Boolean variable
 *	TRUE if there is a clear path 
 * 	FALSE if all paths are blocked
 *     	The back_branch boolean lets you know if the clear path
 *     	is on the longest path between the ellipse and the station.
 *      If back_branch=TRUE, the clear path is on a back_branch.
 *      smajax and sminax are in kilometers.
 *      strike is in degrees.
 * 
 *      Bool 
 *      blocked_ellipse_info
 *      Return a Boolean variable
 *	TRUE if there is a clear path 
 * 	FALSE if all paths are blocked
 *      Returns the actual path distance
 *      to the center and the min and max non-blocked distances
 *          
 * 
 *  	num_blocked_stations (void)
 *      int Returns the number of stations for which a 
 *      blockage grid is available.    
 * 
 *      get_blocked_site_pt(n)
 *      Returns a pointer to a site structure
 *      given a number between 0 and num_blocked_stations.
 *
 *      set_blk_radius (void)
 *      Set the value of the radius to be used for instance by the 
 *      locator to add some fuzziness to the exact location
 *      of a point on earth.
 *   
 *      get_blk_radius (void)
 *      Get the value of the radius to be used for instance by the 
 *      locator to add some fuzziness to the exact location
 *      of a point on earth.
 *
 * DIAGNOSTICS
 *	None.

 * FILES
 *	None.

 * NOTES
 *	None.

 * SEE ALSO
 *	

 * AUTHORS 
 *	Header and prototype                    RLB 09/12/96
 *	Changed informative message
 *      from stderr to stdout                   RLB 07/30/97
 *      Fix an ellipse parameterization
 *      bug (CMR-477)                           RLB 12/16/98
 * 
 */  



#include "config.h"
#include <stdio.h>
#include <string.h>

#include "aesir.h"
#include "site_Astructs.h"
#include "libloc.h"
#include "libgeog.h"
#include "loc_defs.h"
#include "blk_defs.h"

#define NPT_ELLIPSE 36

Blk_Inf blk_inf = { (Site_Blk *) NULL, 0 } ;

double blk_radius = DEG_TO_KM ;



int 
setup_blk_facilities 
#if UsePrototypes
(char *blk_prefix, Site *site, int num_sites)
#else
(blk_prefix, site, num_sites)
char    *blk_prefix ; 
Site    *site       ;
int     num_sites   ;
#endif
{
	char filename[FILENAMELEN+1] ;
        char input_string[BUFSIZ]    ;
	char staname[10]             ;

	FILE *fp     ;
        int i        ;
        int nsta_blk ;
	int naz      ;
	
	Site_Blk *blk_tmp = (Site_Blk *) NULL ;
	
	nsta_blk = 0 ;

        /*
	fprintf(stdout,
	"Found blockage grid file for stations: \n");
        */
	
        for(i = 0 ; i < num_sites ; i++)
	{

	/*
	 *  Check if there is a blockage grid 
	 *  file with this station name
	 *
	 */
		
		strcpy(filename, blk_prefix) ;
		strcat(filename, "/") ;
		strcat(filename, FILE_PREF) ;
		strcat(filename, site[i].sta) ;
		strcat(filename, FILE_SUFF) ;
		if((fp = fopen (filename, "r")) != NULL)
		{
        /* 
         *  RLB 6/22/00 commented out so it can be used by the web interface code and subscription system 
         *
   	 *         fprintf(stdout,
         *				" %s ",
         *				site[i].sta);
         */
			nsta_blk++;
			
			if(( blk_tmp=(Site_Blk*)realloc(blk_tmp, nsta_blk*sizeof(Site_Blk))) == NULL )
				fprintf(stderr,
					"Error in realloc function \
                                        \nin call to setup_blk_facilities\n") ;
			naz=0;
			blk_tmp[nsta_blk-1].rad = (Radplot *) NULL ;
			
			if(fgets (input_string, BUFSIZ, fp) != NULL)
			{
				sscanf (input_string, "%s" ,staname);
				if(strcmp (staname, site[i].sta))
					fprintf(stderr,
						"File name %s incompatible \
                                 with station name inside the file (%s)\n", 
						filename, staname );
			}
			
			while (fgets (input_string, BUFSIZ, fp) != NULL)
			{
				/* Skip lines w/ '#' in 1st column */
				if (!strncmp (input_string, "#", 1)) 
					continue;
				else
				{
					naz++;
					if(( blk_tmp[nsta_blk-1].rad=
				(Radplot*)realloc(blk_tmp[nsta_blk-1].rad, 
						    naz*sizeof(Radplot))) == NULL )
					fprintf(stderr,
						"Error in realloc function \
                                        \nin call to setup_blk_facilities\n") ;
					
					sscanf (input_string, "%lf %lf",  
						&(blk_tmp[nsta_blk-1].rad[naz-1].max_dist),
						&(blk_tmp[nsta_blk-1].rad[naz-1].azimuth)) ;
					continue;
				}
			}
			blk_tmp[nsta_blk-1].site = &site[i] ;
			blk_tmp[nsta_blk-1].naz  = naz      ;
			fclose(fp);
		}
	}
        /*
	fprintf(stdout,"\n");
        */
	blk_inf.site = blk_tmp ;
	blk_inf.num_sites = nsta_blk ;
	return (OK);
}


Bool 
blocked_path
#if UsePrototypes
(char *sta , double lat_source, 
double lon_source, Bool *back_branch )
#else
(sta, lat_source, lon_source, back_branch )
char    *sta       ;
double  lat_source ;
double  lon_source ;
Bool    *back_branch   ;
#endif
{
	Bool found_blk    ;

	Site_Blk *blk_tmp = NULL ;
	
	int i             ;
	
	double delta      ;
	double azi        ;
	double baz        ;
	double max_dist = 0.  ;

	
	found_blk = FALSE ;
	if(blk_inf.site == (Site_Blk *) NULL)
		return(FALSE) ;
	for(i=0 ; i < blk_inf.num_sites ; i++)
	{
		if(!strcmp(blk_inf.site[i].site->sta, sta))
		{
			found_blk = TRUE ;
			blk_tmp = &(blk_inf.site[i]);
			break ;
		}
	}

	/*
	 *  If no information is found for this station,
	 *  return and assume blockage.
	 */

	if(found_blk == FALSE)
		return(FALSE) ;
	
	else
	{
		/*
		 * Get azimuth betweeen station and event
		 */
		
		dist_azimuth (blk_tmp->site->lat, blk_tmp->site->lon, 
		      lat_source, lon_source, 
		      &delta, &azi, &baz, 0);

		/* 
		 * Get maximum distance of blockage for this azimuth 
		 * and the azimuth 180 degrees away from it.
		 */

		for(i=1 ; i < blk_tmp->naz ; i++)
		{
			max_dist = 0;
			if(blk_tmp->rad[i].azimuth   >= azi &&
			   blk_tmp->rad[i-1].azimuth <  azi )
			{
				max_dist = blk_tmp->rad[i-1].max_dist + 
					(blk_tmp->rad[i].max_dist - blk_tmp->rad[i-1].max_dist)*
					(azi - blk_tmp->rad[i-1].azimuth)/
						(blk_tmp->rad[i].azimuth - blk_tmp->rad[i-1].azimuth) ;
				break ;
			}
			
		}
		if(delta <= max_dist)
		{
			*back_branch = FALSE ;
			return(TRUE);
		}
		
		/*
		 * Shortest path is blocked, 
		 * now check on the longest path 
		 */

		else
		{
			if(azi < 180.) 
				azi += 180. ;
			else 
				azi -= 180. ;
			
			for(i=1 ; i < blk_tmp->naz ; i++)
			{
				max_dist = 0;
				if(blk_tmp->rad[i].azimuth   >= azi &&
				   blk_tmp->rad[i-1].azimuth <  azi )
				{
					max_dist = blk_tmp->rad[i-1].max_dist + 
						(blk_tmp->rad[i].max_dist - 
						 blk_tmp->rad[i-1].max_dist)*
							(azi - blk_tmp->rad[i-1].azimuth)/
								(blk_tmp->rad[i].azimuth - 
								 blk_tmp->rad[i-1].azimuth) ;
					break ;
				}
		
			}
		
			if((360.-delta) <= max_dist)
			{
				*back_branch = TRUE ;
				return(TRUE);
			}
		}

		/* 
		 * If we make it this far, the path must be blocked on 
		 * either the shortest or the longest path between the
		 * station and the source.
		 */	

		return(FALSE) ;
	}	
}


Bool 
blocked_ellipse
#if UsePrototypes
(char *sta , double lat_source, 
double lon_source, double smajax, double sminax, double strike, Bool *back_branch)
#else
(sta, lat_source, lon_source, smajax, sminax, strike, back_branch)
char    *sta       ;
double  lat_source ;
double  lon_source ;
double  smajax     ; 
double  sminax     ; 
double  strike     ;
Bool *back_branch  ;
#endif
{
	/* 
	 *  Discretize ellipse perimeter 10 degree/10 degree
	 *
	 */
	
	int i        ;

	double elat  ;
	double elon  ;
	double r     ;
	double azim  ;
	double arg   ;
	double ell_inc = 360.0/NPT_ELLIPSE;
	double rx    ;
	double ry    ;
	
	
	Bool   clear ; 	
	
	clear = FALSE ;
	

        for ( i=0 ; i < NPT_ELLIPSE ; i++ )
        {
                arg = i * ell_inc * DEG_TO_RAD ;
                rx = smajax * cos(arg);
                ry = sminax * sin(arg);
                r = sqrt(rx * rx + ry * ry) * KM_TO_DEG;
                azim = atan2(ry,rx) * RAD_TO_DEG;
                azim = strike - azim;
                lat_lon( lat_source, lon_source, r, azim, &elat, &elon);

		/*
		 *  If any point on the ellipse is clear,
		 *  return TRUE
		 */

		if( blocked_path(sta, elat, elon, back_branch ) == TRUE )
		{
			clear = TRUE ;
			return(clear);
		}
		
		
	}
	
	/*
	 * If no point of the ellipse is clear,
	 * return FALSE.
	 */

	return(clear) ;
		
	
}

Bool 
blocked_path_max_dist
#if UsePrototypes
(char *sta , double lat_source, 
double lon_source, double *dist, double *az)
#else
(sta, lat_source, lon_source, dist, az )
char    *sta       ;
double  lat_source ;
double  lon_source ;
double  *dist      ;
double  *az        ;

#endif
{
	Bool found_blk    ;

	Site_Blk *blk_tmp = NULL ;
	
	int i             ;
	
	double delta      ;
	double azi        ;
	double baz        ;
	double max_dist = 0. ;

	
	found_blk = FALSE ;
	*dist = -1.       ;
	*az   = -1.       ;
	
	if(blk_inf.site == (Site_Blk *) NULL)
		return(FALSE) ;
	for(i=0 ; i < blk_inf.num_sites ; i++)
	{
		if(!strcmp(blk_inf.site[i].site->sta, sta))
		{
			found_blk = TRUE ;
			blk_tmp = &(blk_inf.site[i]);
			break ;
		}
	}

	/*
	 *  If no information is found for this station,
	 *  return and assume blockage.
	 */

	if(found_blk == FALSE)
		return(FALSE) ;
	
	else
	{
		/*
		 * Get azimuth betweeen station and event
		 */
		
		dist_azimuth (blk_tmp->site->lat, blk_tmp->site->lon, 
		      lat_source, lon_source, 
		      &delta, &azi, &baz, 0);

		/* 
		 * Get maximum distance of blockage for this azimuth 
		 * and the azimuth 180 degrees away from it.
		 */

		for(i=1 ; i < blk_tmp->naz ; i++)
		{
			max_dist = 0;
			if(blk_tmp->rad[i].azimuth   >= azi &&
			   blk_tmp->rad[i-1].azimuth <  azi )
			{
				max_dist = blk_tmp->rad[i-1].max_dist + 
					(blk_tmp->rad[i].max_dist - blk_tmp->rad[i-1].max_dist)*
					(azi - blk_tmp->rad[i-1].azimuth)/
						(blk_tmp->rad[i].azimuth - blk_tmp->rad[i-1].azimuth) ;
				break ;
			}
			
		}

		*az = azi ;

		if(delta <= max_dist)
		{
			*dist = delta ;
			return(TRUE);
		}
		
		/*
		 * Shortest path is blocked, 
		 * now check on the longest path 
		 */

		else
		{
			if(azi < 180.) 
				azi += 180. ;
			else 
				azi -= 180. ;
			
			*az = azi ;

			for(i=1 ; i < blk_tmp->naz ; i++)
			{
				max_dist = 0;
				if(blk_tmp->rad[i].azimuth   >= azi &&
				   blk_tmp->rad[i-1].azimuth <  azi )
				{
					max_dist = blk_tmp->rad[i-1].max_dist + 
						(blk_tmp->rad[i].max_dist - 
						 blk_tmp->rad[i-1].max_dist)*
							(azi - blk_tmp->rad[i-1].azimuth)/
								(blk_tmp->rad[i].azimuth - 
								 blk_tmp->rad[i-1].azimuth) ;
					break ;
				}
		
			}
		
			if((360.-delta) <= max_dist)
			{
				*dist = 360.-delta;
				return(TRUE);
			}
		}

		/* 
		 * If we make it this far, the path must be blocked on 
		 * either the shortest or the longest path between the
		 * station and the source.
		 */	

		
		return(FALSE) ;
	}	
}



Bool 
blocked_ellipse_dist
#if UsePrototypes
(char *sta , double lat_source, 
double lon_source, double smajax, double sminax, double strike, double *dist_center,
double *dist_min, double *dist_max, double *az_min, double *az_max) 
#else
(sta, lat_source, lon_source, smajax, sminax, strike, dist_center, dist_min, dist_max,
az_min, az_max)
char    *sta       ;
double  lat_source ;
double  lon_source ;
double  smajax     ; 
double  sminax     ; 
double  strike     ;
double  *dist_center ;
double  *dist_min    ; 
double  *dist_max    ;
double  *az_min       ;
double  *az_max       ;
#endif
{
	/* 
	 *  Discretize ellipse perimeter 10 degree/10 degree
	 *
	 */
	
	int i        ;

	double elat  ;
	double elon  ;
	double r     ;
	double azim  ;
	double arg   ;
	double dist  ;
        double az    ;
	double ell_inc = 360.0/NPT_ELLIPSE;
	double rx    ;
	double ry    ;
	double az_center ;

	
	
	Bool   clear ; 	
	
	clear = FALSE ;

	*dist_min = 360. ;
	*dist_max = 0.   ;
	*az_min   = 360. ;
	*az_max   = 0.   ;
	
	
	for ( i=0 ; i < NPT_ELLIPSE ; i++ )
	{	
                arg = i * ell_inc * DEG_TO_RAD ;
                rx = smajax * cos(arg);
                ry = sminax * sin(arg);
                r = sqrt(rx * rx + ry * ry) * KM_TO_DEG;
                azim = atan2(ry,rx) * RAD_TO_DEG;
                azim = strike - azim;
		
	        lat_lon( lat_source, lon_source, r, azim, &elat, &elon ) ;

		/*
		 *  If any point on the ellipse is clear,
		 *  return TRUE
		 */

		if( blocked_path_max_dist( sta, elat, elon, 
					  &dist , &az) == TRUE )
		{
			if( dist < *dist_min ) *dist_min=dist ;
			if( dist > *dist_max ) *dist_max=dist ;
			if( az   < *az_min   ) *az_min = az   ;
			if( az   > *az_max   ) *az_max = az   ;
			
			clear = TRUE ;
		}
		
	}

	if(clear == FALSE)
	{
		*dist_min = -1.  ;
		*dist_max = -1.  ;
		*az_min   = -1.  ;
		*az_max   = -1.  ;
	}

	/* 
	 * Get distance to center
	 *
	 */	

	if( blocked_path_max_dist(sta, lat_source, lon_source, 
				  dist_center, &az_center ) == TRUE )
		clear = TRUE ;

	/*
	 * If no point of the ellipse is clear, and the center is blocked
         * as well
	 * return FALSE and set all distances to -1.
	 */

	return(clear) ;
}




int 
num_blocked_stations
#if UsePrototypes
(void)
#else
(void)
#endif
{
	int num;
	
	num = blk_inf.num_sites ;
	return(num);
}

Site * 
get_blocked_site_pt
#if UsePrototypes
(int sta )
#else
(sta)
int sta ;
#endif
{
	Site *pt ;
	pt = blk_inf.site[sta].site ;
	return(pt);
}

double
get_blk_radius
#if UsePrototypes
(void)
#else
(void)
#endif
{
        double rad       ;
       
        rad = blk_radius ;	
	return(rad)      ;
}



int
set_blk_radius
#if UsePrototypes
(double rad)
#else
(rad)
double rad ;
#endif
{
        blk_radius = rad ;
	return(OK)       ;
}
