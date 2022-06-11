/*
 * NAME
 *      tt_info.c	
 * 
 * SYNOPSIS
 *	Reads the regional travel times for acoustic phases.
 *	Allows user to access the same.
 *
 * DESCRIPTION
 *
 *	read_HY_info
 *	  drives initialize_radial_2D_tt_tables.  Returns error code
 *	from the same.
 *
 *	char *list_file		Path to a file containing names of 
 *				stations for which travel times are
 *				to be calculated by radial_2D 
 * 				method.  Data files with the same 
 *				names as the stations must appear 
 *				in the same directory. 
 *
 *
 *	initialize_radial_2D_tt_tables
 *	  creates a single Station_Info structure that contains
 *	all the travel times for the radial_2D model.  Clears away
 *	any previously created data structures.  Returns error code
 *	if there are any reading problems, or OK is all is well.
 *	Typically will not be used as point-of-entry by user (see
 *	read_HY_info.
 *
 *	char *sta_names		Array of names of stations for which
 *				travel times will be calculated by
 *				radial_2D method.
 *	int num_sta_names	Number of elements in sta_names[].
 *	char *tt_path		Path to directory of radial_2D
 *				data files.  Only what appears to
 *				the left of the last / is used.
 *
 *	station_in_radial_2D_tables
 *	  accepts a single string as an argument and returns either
 *	HYDRO_INDEX or INFRA_INDEX if radial_2D data for the station
 *	of that name exists in the current Station_Info structure,
 *	FALSE otherwise.
 *
 *	get_hy_tt_pointer
 *	  returns a pointer to the hydroacoustic Period_Info structure created 
 *	by initialize_radial_2D_tt_tables.
 *
 *	get_infra_tt_pointer
 *	  returns a pointer to the infrasonic Period_Info structure created 
 *	by initialize_radial_2D_tt_tables.
 *
 *	get_acoustic_tt
 *	  Looks up and interpolates travel time from a location to
 *	a station.  If no data exists for the station in question, 
 *	-1.0 will be output as travel_time.  Travel time is 
 *	calculated by interpolating between four data table points.  
 *	The number of these table points that are blocked is output
 *	in the sixth argument, blocked.  If all four are blocked
 *	along the geodesic path, the anti-geodesic will be checked,
 *	and data output for that path, unless it is also blocked.
 *	In either case, and extrapolated travel time is output.
 *
 *	char *sta		The name of the station.
 *	double ev_lat		Event latitude.
 *	double ev_lon		Event longitude.
 *	double *travel_time	Travel time output.
 *	double *model_error	Modeling error output;
 *	int *blocked		Blockage output;
 * 
 *
 *      load_acoustic_tt
 *	  Module driven by get_acoustic_tt. Load neaded travel times for the radial_2D model.
 *
 * 	Period_Info* period
 *	int i
 *	int j
 *
 *
 *	look_up_ttime
 *	  Module driven by get_acoustic_tt.  Should not be accessed
 *	directly.
 *
 *	get_2D_model_error
 *	char *sta		Station name.
 *	double dist		Distance
 *	double azimuth 		Station-to-Event azimuth
 *
 *	get_currend_radial_2D_period_name
 *	  Return the name of the time period (month, season)
 *	to which the epoch time set in set_epoch_travel_time()
 *	corresponds for this station.
 *
 *	char *sta		Station name
 *
 * DIAGNOSTICS
 *	
 *	HYerror1:   Bad path name to radial_2D directory 
 *	HYerror2:   Logically meaningless parameter
 *	HYerror3:   Bad station file name in vmsf file
 *	HYerror4:   Memory allocation failure.
 *
 * FILES
 *
 * 	The complete path to the radial_2D station file is to be
 *	passed into read_HY_info.  This file contains simply a 
 *	list of station names.  Files of those same names must
 *	appear in the same directory as the station file. 
 *	Alternatively the user may choose circumvent read_HY_info
 *	and pass an array of station names and the path to the
 *	radial_2D directory directly into 
 *	initialize_radial_2D_tt_tables. 
 *	
 *
 * NOTES
 *	The capability to return attentuation is partially 
 *	implemented here.  This may not be a part of future 
 *	releases. 
 * 
 * AUTHOR
 *	Jerry A. Guern Sept 1997	
 *
 *	Guern 	June 1998	Modified to accomodate seasonally varying travel times and I phase.
 *
 *
 */

#include "config.h"
#include <string.h>
#include "libloc.h"
#include "tt_info.h"
#include "loc_info.h"
#include "libgeog.h"
#include "libaesir.h"
#include "libtime.h"
#define HYDRO_SEC_PER_DEG 	75.0
#define AZI_K 			period[i].station[j].azimuth[azi_index[k]]
#define PSA 			period[i].station[j].azimuth[k]
#define NUM_NOTICES		10

/*
 *   Create a table of tt and tl info, in the format
 *
 *   period[i].station[j].azimuth[k].tt[l] and
 *   period[i].station[j].azimuth[k].tl[l] 
 *
 *   where 0 < l < period[i].station[j].azimuth[k].nrad
 *   and   0 < k < period[i].station[j].num_azimuth
 *
 */

static 	Period_Info 	*hydro_period = (Period_Info *)(NULL);
static 	Period_Info 	*infra_period = (Period_Info *)(NULL);
static 	Period_Time 	*hydro_period_time = (Period_Time *)(NULL);
static 	Period_Time 	*infra_period_time = (Period_Time *)(NULL);
static	int		number_of_periods[] = {0,0};
static  Control_Flags  control = {20.0,0,0,0,0,0,0,HYDRO_INDEX};

int read_HY_info(char *list_file)
{
	/*
	 * Read the table of station files listed in the vmsf file.
	 * Drive initialize_radial_2D_tt_tables
	 */
	FILE *list_fp;
	FILE *sta_file;
	char **sta_names;
	char list_buffer[2000];
	char sta_buffer[2000];
	int num_stations;
	int i,hydro_return;	
	char path[2000];	
	char *ffp;

	if((list_fp = fopen (list_file, "r")) == NULL)
	{
		fprintf(stderr,"Could not open %s\n",list_file);
		fprintf(stderr,"Will use defaults or previously read radial_2D tables, if any.\n");
		return (OK); 
	}

	free_whole_table(hydro_period);
	free_whole_table(infra_period);
	control.current_hydro_per_index = 0;
	control.current_infra_per_index = 0;
	control.initiated		= 0;
	control.epoch_time_set		= 0;
	control.use_hydro_2D_table      = 0;
	control.use_infra_2D_table      = 0;
	control.informed		= 0;
	control.current_tech_index	= 0;

	/*first, confirm that there are two lists in the file.*/
	i=0;
	while(!(fscanf(list_fp,"%s",list_buffer)==EOF)) i++;
	fclose(list_fp);

	if(i != 2)
	{
	    fprintf(stderr,"ERROR:  %s contains %d lines.\n",list_file,i);
	    fprintf(stderr,"The file indicated on the second line of your vsmf file\n");
	    fprintf(stderr,"Should contain 2 lines; a hydro-acoustic station list file\n");
	    fprintf(stderr,"an infrasonic station list file.\n");
	    fprintf(stderr,"\nWill use default travel times instead of 2-D tables.\n\n");
	    return(OK);
	} 
	list_fp = fopen (list_file, "r");

	/*Make a copy of the input path name.  Do not write on the input string.*/ 
	strcpy(path,list_file);

        if ((ffp = strrchr (path, '/')) == NULL)
        {
            fprintf (stderr, "No / in radial_2D directory name!\n");
            return (HYerror1);
        }


	if (fscanf(list_fp,"%s",list_buffer) != 1) return (HYerror1);

	if(STREQ(list_buffer,"NONE"))
	    fprintf(stderr,"NONE specified as hydroacoustic radial_2D file path.  Default travel times will be used.\n");
	else
	{
	    strcpy (ffp+1, list_buffer);


	    if((sta_file = fopen (path, "r")) == NULL)
	    {
		fprintf(stderr,"Could not open %s\n",path);
                fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
                return (OK);
	    }

	    /*first, count how many stations are in the file*/
	    i=0;
	    while(!(fscanf(sta_file,"%s",sta_buffer)==EOF)) i++;
	    num_stations = i;
	    fclose(sta_file);

	    if(num_stations < 1)
	    {
		fprintf(stderr,"ERROR:  %s empty or not formatted correctly.\n",
			path);
		fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
		return(OK);
	    } 

	    sta_file = fopen (path, "r");
	    sta_names = UALLOC (char *, num_stations); 
	    for(i=0;i<num_stations;i++)
	    {
	      if (fscanf(sta_file,"%s\n",sta_buffer) != 1) return (HYerror1);
	      sta_names[i]=strdup(sta_buffer);
	    }
	    fclose(sta_file);

	    control.current_tech_index = HYDRO_INDEX;


	    /*  printf("DEBUG: path = %s\n", path); */
	    hydro_return = initialize_radial_2D_tables (sta_names, num_stations, path);

	    if(hydro_return != OK)
		return(hydro_return);
	}

	if (fscanf(list_fp,"%s",list_buffer) != 1) return (HYerror1);

        fclose(list_fp);

	if(STREQ(list_buffer,"NONE"))
	{
	    fprintf(stderr,"NONE specified as infrasonic radial_2D file path.  Default travel times will be used.\n");
	    return(OK);
	}

        strcpy (ffp+1, list_buffer);

        if((sta_file = fopen (path, "r")) == NULL)
        {
                fprintf(stderr,"Could not open %s\n",path);
                fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
                return (OK);
        }

        /*first, count how many stations are in the file*/
	i=0;
        while(!(fscanf(sta_file,"%s",sta_buffer)==EOF)) i++;
        num_stations = i;
        fclose(sta_file);

        if(num_stations < 1)
        {
            fprintf(stderr,"ERROR:  %s empty or not formatted correctly.\n",
			path);
            fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
            return(OK);
        }

        sta_file = fopen (path, "r");
        sta_names = UALLOC (char *, num_stations);
        for(i=0;i<num_stations;i++)
        {
	  if (fscanf(sta_file,"%s\n",sta_buffer) != 1) return (HYerror1);
	  sta_names[i]=strdup(sta_buffer);
        }
        fclose(sta_file);

	control.current_tech_index = INFRA_INDEX;

        return initialize_radial_2D_tables (sta_names, num_stations, path);
}


int initialize_radial_2D_tables (char *sta_names[], int num_sta_names, char *tt_path)
{
	/*
	 * Initialize the table structure and insert tt_tl information for the 
 	 * specified stations.  If a bin file corresponding to  
	 * a paricular station cannot be found in the specified *path, the 
	 * entire process stops and an error message is returned.
	 */ 

	Period_Info 	*period = (Period_Info *)(NULL);
	Period_Time 	*period_time = (Period_Time *)(NULL);
	char path[2000];	
	char buffer[2000];	
	char *ffp;
	int i,j,n;
	FILE *time_file;
	float doy;
	int num_period_file_lines;
	double ht_convert;

	if (num_sta_names<1)  
	{
		fprintf(stderr,"%d passed into initialize_Hydro_tt_tables as number of stations.\n",num_sta_names);
		return (HYerror2);
	}

	/*Make a copy, in case the user wanted to use the original later.*/ 
	/* printf("DEBUG: tt_path = %s\n", tt_path); */
	strcpy(path,tt_path);

        if ((ffp = strrchr (path, '/')) == NULL)
        {
            fprintf (stderr, "No / in radial_2D directory name!\n");
            return (HYerror1);
        }

	/*
	 *  Read the time_guide file.  This must be done before memory is 
	 *  allocated for the travel-time information so that we know how
	 *  many time periods there are.
	 */

	strcpy (ffp+1, "time_guide");

        /*first, count how many periods are in the file*/

        if((time_file = fopen (path, "r")) == NULL)
        {
                fprintf(stderr,"Could not open %s\n",path);
                fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
                return (OK);
        }
        for(i=0;(fscanf(time_file,"%s",buffer)!=EOF); i++);
        fclose(time_file);
	if(i%2 || i<2)
	{
	    fprintf(stderr,"%s has incorrect format.\n",path);
	    fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
                return (OK);
        }

	/*  Now allocate new memory. */
 
        num_period_file_lines = i/2;

	CHECK_MALLOC(period,Period_Info, num_period_file_lines);
	memset(period, 0, num_period_file_lines * sizeof(Period_Info));

	CHECK_MALLOC(period_time,Period_Time, num_period_file_lines);
	memset(period_time, 0, num_period_file_lines * sizeof(Period_Time));

        time_file = fopen (path, "r");

	if (fscanf(time_file,"%lf",&(ht_convert)) != 1) return (HYerror2);
	if(control.current_tech_index==HYDRO_INDEX) 
	    control.H_T_convert = ht_convert;

	i = 0;
	for(n = 0; n < num_period_file_lines; n++)
	{
	  if (fscanf(time_file,"%s\n",buffer) != 1) return (HYerror2);
	    if(n < (num_period_file_lines-1))
	    {	
	      if (fscanf(time_file,"%f",&doy) != 1) return (HYerror2);
	      period_time[n].last_doy = (int)(doy);
		if(n>0 && (period_time[n].last_doy <= period_time[n-1].last_doy))
		{
		    fprintf(stderr,"%s is not formatted correctly.\n",path);
		    fprintf(stderr,"Will use default travel times instead of 2-D tables.\n");
		    free_whole_table(period);
		    return (OK);
		}
	    }
	    for(j=0;j<i;j++)
		if(STREQ(buffer,period[j].per))
		    break;

	    period_time[n].index = j;

	    if(j<i)
		continue;  /* We've already loaded this period in. */

	    period[i].per = strdup(buffer);
	    period[i].num_station = num_sta_names;

	    CHECK_MALLOC(period[i].station,Station_Info, num_sta_names);
	    memset(period[i].station, 0, num_sta_names * sizeof(Station_Info));

	    for(j=0; j < num_sta_names; j++)
	    {
		period[i].station[j].sta = strdup(sta_names[j]);
		sprintf(buffer,"%s/%s",period[i].per,period[i].station[j].sta);
        	strcpy (ffp+1, buffer);
		period[i].station[j].num_azimuth = -999;
		period[i].station[j].file = strdup(path);
	    }  /* End loop over j, stations */
	    i++;  
	}      /* End loop over n, lines of time_guide */
	period_time[n-1].last_doy = 367;

        fclose(time_file);

	number_of_periods[control.current_tech_index-1] = i;

	if (i < num_period_file_lines) {
	  if((period = (Period_Info *) realloc (period,	i * sizeof (Period_Info))) == (Period_Info *)(NULL)) {
	    fprintf(stderr,"Memory reallocation failure in initialize_Hydro_tt_tables\n");
	    return (HYerror4);
	  }
	}
	

	for(j = 0; j<num_sta_names; j++) UFREE(sta_names[j]);
	UFREE(sta_names);

	if(control.current_tech_index==HYDRO_INDEX)
	{
	    /* These were the hydroacoustic tables */
	    control.use_hydro_2D_table = TRUE;
	    hydro_period = period;
	    hydro_period_time = period_time;
	}
	else	
	{
	    /* These were the infrasonic tables */
	    control.use_infra_2D_table = TRUE;
	    infra_period = period;
	    infra_period_time = period_time;
	}

	return OK;
}




int station_in_radial_2D_tables(char *sta)
{
    int i,j;
    i = control.current_hydro_per_index;
    if(control.use_hydro_2D_table && hydro_period!=NULL && hydro_period[i].station!=NULL)
	for(j=0;j<hydro_period[i].num_station;j++)
	    if(STREQ(hydro_period[i].station[j].sta,sta))
		return HYDRO_INDEX;

    i = control.current_infra_per_index;
    if(control.use_infra_2D_table && infra_period!=NULL && infra_period[i].station!=NULL)
	for(j=0;j<infra_period[i].num_station;j++)
	    if(STREQ(infra_period[i].station[j].sta,sta))
		return INFRA_INDEX;

    return FALSE;
}


Period_Info *get_hy_tt_pointer()
{
    return hydro_period;
}

Period_Info *get_infra_tt_pointer()
{
    return infra_period;
}

	
int load_acoustic_tt(Period_Info* period, int i, int j)
{
	char path[2000];	
	int  k;
	char *header= 	(char *)(NULL);
	FILE *bin_file;
	int nrad;
	int header_length;

	if (period[i].station[j].file == NULL) {
	  fprintf(stderr,"Azimuth file name is undefined (NULL)\n");
	  return (ERR);
	}
	strcpy(path, period[i].station[j].file);

	/* printf("DEBUG: Reading period %d, station %d\n", i, j); */

	if((bin_file = fopen (path, "r")) == NULL)
	{
	  if (!strcmp(path, "")) {
	    fprintf(stderr,"Could not open empty filename\n");
	  }
	  else {
	    fprintf(stderr,"Could not open %s\n", path);
	  }
	  return (ERR);
	}

	if (fread (&(period[i].station[j].lat), sizeof(float), 1, bin_file) != 1) {
	  fprintf(stderr, "Failed to read period %d, station %d latitude\n", i, j);
	  return(ERR);
	}
	if (fread (&(period[i].station[j].lon), sizeof(float), 1, bin_file) != 1) {
	  return(ERR);
	  fprintf(stderr, "Failed to read period %d, station %d longitude\n", i, j);
}
	if (fread (&header_length, sizeof(int), 1, bin_file) != 1) {
	  fprintf(stderr, "Failed to read period %d, station %d header length\n", i, j);
	  return(ERR); 
	}

#ifndef WORDS_BIGENDIAN
		endian_revert((char *) &(period[i].station[j].lat), 1, sizeof(float));
		endian_revert((char *) &(period[i].station[j].lon), 1, sizeof(float));
		endian_revert((char *) &header_length, 1, sizeof(int));
#endif

	CHECK_MALLOC(header, char, header_length+1);
	memset(header, 0, (header_length+1) * sizeof(char));
	if (fread (header, sizeof(char), header_length, bin_file) != header_length) {
	  fprintf(stderr, "Failed to read period %d, station %d header\n", i, j);
	  return(ERR);
	}
	/*sprintf(header+header_length,"\0");*/
        *(header+header_length) = '\0';
	UFREE(header);
	header=(char *)(NULL);

	if (period[i].station[j].azimuth == NULL) {
	  CHECK_MALLOC(period[i].station[j].azimuth, Azimuth_Info, 1);
	  memset(period[i].station[j].azimuth, 0, 1 * sizeof(Azimuth_Info));
	}
	k=0;
	while(feof(bin_file)==0)
	{
	  if (fread(&(PSA.azi), sizeof(float), 1, bin_file) != 1) {
	    /* Files are not corrupt, they always come with trailing garbage */
	    /* fprintf(stderr, "Failed to read period %d, station %d PSA azimuth (possibly corrupt file)\n", i, j); */
	    /* Do not return yet as it may be a corrupt file with trailing garbage */
	    /* return(ERR); */
	  }
#ifndef WORDS_BIGENDIAN
	  endian_revert((char *) &(PSA.azi), 1, sizeof(float));
#endif
	  /* Break on EOF */
	  if ( feof(bin_file) != 0 ) {
	    /*  fprintf(stderr, "EOF encountered and handled, corrupt file with trailing garbage: %s\n", path); */
	    break;
	  }

	    /* Skip trailing garbage */
            if ( PSA.azi >= 360 || PSA.azi < 0 ||
                  ( k > 0 && PSA.azi < period[i].station[j].azimuth[k-1].azi ) )
            {
             	fprintf(stderr,"<initialize_radial_2D_tables> Warning: Problem with hydro TT table format. \n");
             	fprintf(stderr,"         Period: %s , Station: %s , Previous radial: AZ = %7.2f deg.\n",
                                period[i].per, period[i].station[j].sta, period[i].station[j].azimuth[k-1].azi);
             	break;
            }

	    if (fread(&nrad, sizeof(int), 1, bin_file) != 1) {
	      fprintf(stderr, "Failed to read period %d, station %d number of radians\n", i, j);
	      return(ERR);
	    }

#ifndef WORDS_BIGENDIAN
		endian_revert((char *) &nrad, 1, sizeof(int));
#endif

	    PSA.nrad = nrad;
	    if (fread(&(PSA.delta_rad), sizeof(float), 1, bin_file) != 1) {
	      fprintf(stderr, "Failed to read period %d, station %d delta radians\n", i, j);
	      return(ERR);
	    }

#ifndef WORDS_BIGENDIAN
		endian_revert((char *) &(PSA.delta_rad), 1, sizeof(float));
#endif

	    if(nrad) 
	    {
		CHECK_MALLOC(PSA.tt,float,nrad);
		memset(PSA.tt, 0, nrad * sizeof(float));

		if (fread(PSA.tt, sizeof(float), nrad, bin_file) != nrad) {
		  fprintf(stderr, "Failed to read period %d, station %d PSA travel time\n", i, j);
		  return(ERR);
		}

#ifndef WORDS_BIGENDIAN
		endian_revert((char *) PSA.tt, nrad, sizeof(float));
#endif

		CHECK_MALLOC(PSA.tt_error,float,nrad);
		memset(PSA.tt_error, 0, nrad * sizeof(float));

		if (fread(PSA.tt_error, sizeof(float), nrad, bin_file) != nrad) {
		  fprintf(stderr, "Failed to read period %d, station %d PSA travel time error\n", i, j);
		  return(ERR);
		}

#ifndef WORDS_BIGENDIAN
		endian_revert((char *) PSA.tt_error, nrad, sizeof(float));
#endif			

	    }
	    else
	    {
		PSA.tt=(float *)(NULL);
		PSA.tt_error=(float *)(NULL);
	    }
	    k++;
	    if((period[i].station[j].azimuth=(Azimuth_Info *) realloc (period[i].station[j].azimuth,(k+1) * sizeof (Azimuth_Info))) == (Azimuth_Info *)(NULL)) 
	    {
		fprintf(stderr,"Memory reallocation failure in initialize_Hydro_tt_tables\n");
		return (ERR);
	    }
	    memset(period[i].station[j].azimuth+(k*sizeof(Azimuth_Info)),
		   0, (k+1) * sizeof(Azimuth_Info));
	}
		
	period[i].station[j].num_azimuth = k;
	fclose(bin_file);

	return (OK);
}
	

void get_acoustic_tt(char *sta, double ev_lat,double ev_lon,double *travel_time,
	double *model_error,int *blocked)
{
    Period_Info *period; 
    int i,j,tech_index;
    double esaz, seaz, dist;
    float azimuth, distance;
    double travel_time2,model_error2;
    int blocked2;

    tech_index = station_in_radial_2D_tables(sta);
    if(tech_index == HYDRO_INDEX)
    {
	period = hydro_period;
	i = control.current_hydro_per_index;
    }
    else if(tech_index == INFRA_INDEX)
    {
	period = infra_period;
	i = control.current_infra_per_index;
    }
    else
    {
	*travel_time = -1.0;
	return;
    }

    if(!control.initiated && control.informed < NUM_NOTICES)
    {
	fprintf(stderr,"ERROR!   Acoustic travel time requested without epoch time set.\n");
	fprintf(stderr,"Will use %s tables as default!\n",period[i].per);
	control.informed++ ;
    }

    j=0;
    while( (!STREQ(sta,period[i].station[j].sta)) && (j<period[i].num_station) ) j++;
    if(j == period[i].num_station)
    {
        *travel_time = -1.0;
        fprintf(stderr,"ERROR: Travel time requested for station not in 2-D acoustic tables.\n");
        return;
    }

    /*
     * If the travel time table is not loaded, get it. 
     */

    /*
    printf("DEBUG: period[i].station[j].file = %s\n",
	   (char *)period[i].station[j].file);
    */
    if(period[i].station[j].num_azimuth == -999) {
      if (load_acoustic_tt(period, i, j) == ERR) {
	fprintf(stderr, "Failed to load travel time table\n");
	return;
      }
    }

    ellip_dist(ev_lat, ev_lon, period[i].station[j].lat, period[i].station[j].lon, &dist, &esaz, &seaz, 0);

    distance=(float)(dist);
    azimuth=(float)(seaz);

    look_up_ttime(j,azimuth,distance,travel_time,model_error,blocked,tech_index);
    /* 
     * The variable 'blocked' contains the 0-4, the number of travel time table 
     * grid cells around the even location that are blocked from this station.
     */
    if (*blocked==4)
    {
	/* If the direct path is blocked, check the antigeodesic */
	look_up_ttime(j,azimuth-180.0,360.0-distance,&travel_time2, &model_error2,&blocked2,tech_index);
	if(blocked2<4)
	{
		/* The antigeodesic is unblocked */
		*travel_time = travel_time2;
		*model_error = model_error2;
		*blocked = blocked2;
	}
    }
}

void look_up_ttime(int station_index,float azimuth, float distance,double *travel_time,
	double *model_error,int *blocked, int tech_index) 
{
Period_Info *period;
int i,j,k,l,d;
float ttime[2][2];
float error[2][2];
int   azi_index[2];
float azi_interp;
float dist_interp[2];
float dist_deltarad;
int   steps_beyond_table;
int   nrad;
float delta_rad;

/*
 * Find the nearest table grid points to the event.  Interpolate or
 * extrapolate as necessary.
 *
 */

/* First, set i,j,k to the indices of the appropriate time period,
 * station, and nearby azimuth
 */

if(tech_index == HYDRO_INDEX)
{
    period = hydro_period;
    i = control.current_hydro_per_index;
}
else
{
    period = infra_period;
    i = control.current_infra_per_index;
}

j = station_index;

/* If the travel time table is not loaded, get it */

if(period[i].station[j].num_azimuth == -999) {
  if (load_acoustic_tt(period, i, j) == ERR) {
    fprintf(stderr, "Failed to load travel time table\n");
    return;
  }
}
while(azimuth<0.0) azimuth+=360.0;
while(azimuth>360.0) azimuth -= 360.0; 
k=0;
while((k < period[i].station[j].num_azimuth) && (PSA.azi<azimuth)) k++;

/* set azi_index[] to the azimuth indices that flank this event location */
if(k==0)
{
	/*Azimuth is less than lowest table value.*/
	azi_index[0]=period[i].station[j].num_azimuth - 1;
	azi_index[1]=0;
	azi_interp = (azimuth + 360.0 - period[i].station[j].azimuth[azi_index[0]].azi) /
	    (period[i].station[j].azimuth[0].azi + 360.0 - 
		period[i].station[j].azimuth[azi_index[0]].azi);
}
else if(k == period[i].station[j].num_azimuth)
{
	/*Azimuth is greater than highest table value.*/
	azi_index[0] = period[i].station[j].num_azimuth - 1;
	azi_index[1] = 0;
	azi_interp = (azimuth - period[i].station[j].azimuth[azi_index[0]].azi) /
	    (period[i].station[j].azimuth[0].azi + 360.0 - 
		period[i].station[j].azimuth[azi_index[0]].azi);
}
else 
{
	/*Azimuth is between two table values.*/
	azi_index[0]=k-1 ;
	azi_index[1]=k;
	azi_interp = (azimuth - period[i].station[j].azimuth[k-1].azi) /
	    (period[i].station[j].azimuth[k].azi - period[i].station[j].azimuth[k-1].azi);
}

*blocked=0;
/*
 * Find the travel time and modeling error at the four
 * travel time table grid points that surround the event. 
 */
for(k=0;k<2;k++)  /* Loop over the two enclosing values of azimuth */
{
	nrad = AZI_K.nrad;  
	delta_rad = AZI_K.delta_rad;
	dist_deltarad = distance / delta_rad;
	d=(int)(dist_deltarad); /* Index of maximum distance on the table short of the event */
	for(l=0;l<2;l++)  /* Loop over the two enclosing distance values */ 
	{
		steps_beyond_table = l + d - nrad;
		if ( steps_beyond_table > 0 )
		{
			/* This distance is beyond our table.  Extrapolate beyond
			 * the end of the table using a constant velocity 
			 */
			*blocked += 1;
			ttime[k][l] = HYDRO_SEC_PER_DEG * delta_rad * 
					(float)(steps_beyond_table);
			if(nrad)
				ttime[k][l] += AZI_K.tt[nrad-1];
			/* If this table didn't go out 5 deg for this azimuth,
			 * return a modeling error of 
			 * 5 seconds ; distance < 5 deg
			 * 1 sec/deg ; distance > 5 deg
			 */
			if((((float)(d+l))*delta_rad)>5.0) 
				error[k][l] = 5.0;
			else
				error[k][l] = ((float)(d+l))*delta_rad;	
		}
		else
		{
			if (d+l) /* The event is within the grid.  Interpolate*/
			{
				ttime[k][l] = AZI_K.tt[d+l-1];
				error[k][l] = AZI_K.tt_error[d+l-1];
			}
			else /*The event is closer than the first distance sample.*/
			{
				ttime[k][l] = 0.0;
				error[k][l] = 0.0;
			}
		}
	}
	dist_interp[k] = dist_deltarad-(float)(d);
}	

/* Interpolate between 4 points, which are either true grid points
 * or extrapolated values.
 */
*travel_time = (double)((ttime[0][0]*(1.0-dist_interp[0]) + 
		ttime[0][1]*dist_interp[0]) * (1.0-azi_interp));
*travel_time += (double)((ttime[1][0]*(1.0-dist_interp[1]) + 
		ttime[1][1]*dist_interp[1]) * azi_interp);
*model_error = (double)((error[0][0]*(1.0-dist_interp[0]) + 
		error[0][1]*dist_interp[0]) * (1.0-azi_interp));
*model_error += (double)((error[1][0]*(1.0-dist_interp[1]) + 
		error[1][1]*dist_interp[1]) * azi_interp);
}




void free_whole_table(Period_Info *period)
{
    int i,j,k;
    if (period != NULL) {
      for(i = 0; i < number_of_periods[control.current_tech_index-1]; i++) {
	if (period[i].station != NULL) {
	  for(j = 0; j < period[i].num_station; j++) {
	    if (period[i].station[j].azimuth != NULL) {
	      for(k=0; k < period[i].station[j].num_azimuth; k++) {
		UFREE(PSA.tt);
		UFREE(PSA.tt_error);
	      }
	    }
	    UFREE(period[i].station[j].azimuth);
	    UFREE(period[i].station[j].file);
	    UFREE(period[i].station[j].sta);
	  }
	}
	UFREE(period[i].station);
	UFREE(period[i].per);
      }
    }
    UFREE(period);
}

void set_epoch_travel_time(double etime)
{
/*
 * All applications using libloc MUST use this function.
 */
    int n=0,doy;
/*
    doy = (int)((stdtime_etoj(etime))%1000);
*/
    doy = (int)((timeEpochToJDate(etime))%1000);
    if(control.use_hydro_2D_table)
    {
	for(n=0; hydro_period_time[n].last_doy < doy; n++);
	control.current_hydro_per_index = hydro_period_time[n].index;
    }
    if(control.use_infra_2D_table)
    {
	for(n=0; infra_period_time[n].last_doy < doy; n++);
	control.current_infra_per_index = infra_period_time[n].index;
    }
    control.initiated         = TRUE;
}

char *get_current_radial_2D_period_name(char *sta)
{
  /* Return the time period name */
    int tech_index;
    tech_index = station_in_radial_2D_tables(sta);
    if(tech_index==HYDRO_INDEX)
	return hydro_period[control.current_hydro_per_index].per;
    if(tech_index==INFRA_INDEX)
	return infra_period[control.current_infra_per_index].per;
    return("ERROR");
}

double get_2D_model_error(char *sta, double dist, double azimuth)
{
    Period_Info *period;
    int blocked,tech_index,k,i=0,j=0;
    double time,error;
    tech_index = station_in_radial_2D_tables(sta);
    if(tech_index==HYDRO_INDEX || tech_index==INFRA_INDEX)
    {
	if(tech_index == HYDRO_INDEX)
	{
	    period = hydro_period;
	    i = control.current_hydro_per_index;
	}
	else
	{
	    period = infra_period;
	    i = control.current_infra_per_index;
	}
	/* find station_index j */

	for(k=0; k < period[i].num_station; k++)
	{
	    if(STREQ(sta, period[i].station[k].sta))
	    {
	        j = k;
		break;
	    }
	}
        if(j == period[i].num_station) 
	{
	    fprintf(stderr,"get_2D_model_error: Station %s is not in the list\n", sta);
	    error=1.0;  /* Default */
	    return error;
	}

	look_up_ttime(j, (float)(azimuth), (float)(dist), &time, &error, &blocked,tech_index);
    }   
    else 
	error=1.0;  /* Default */
    return error;
}    

double get_H_T_convert()
{
    return (control.H_T_convert);
}    
