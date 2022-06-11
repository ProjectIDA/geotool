
/*
 * Copyright (c) 1995-1998 Science Applications International Corporation.
 *

 * NAME
 *	read_sssc -- Read SSSC tables based on sta_phase_model contents.
 *	read_single_sssc_file -- Read SSSC file(s) at a time from apply_sssc().
 *	apply_sssc -- Apply source-specific station correction adjustments.

 * FILE
 *	sssc.c

 * SYNOPSIS
 *	int
 *	read_sssc (model_descrip_ptr, num_models, sta_phase_model_ptr, num_spm)
 *	Model_Descrip	**model_descrip_ptr;	(i/o) Model description struct
 *	int		num_models;		(i)   Number of elements in
 *						      model_descrip structure
 *	Sta_Phase_Model	**sta_phase_model_ptr;	(i/o) Station/Phase/Vmodel 
 *						      structure
 *	int		num_spm;		(i)   Number of elements in
 *						      sta_phase_model structure

 *	int
 *	read_single_sssc_file (spm, sssc_level, verbose)
 *	Sta_Phase_Model spm;	(i)   Station/Phase/Vmodel structure
 *	int	sssc_level;	(i)   Level of SSSC
 *	char	verbose;	(i)   Verbosity level

 *	apply_sssc (ev_lat, ev_lon, spm, sssc_correct, me_factor, me_sssc, sssc_level, verbose)
 *	double	ev_lat;		(i) Event latitude (decmimal degrees)
 *	double	ev_lon;		(i) Event longitude (decmimal degrees)
 *	Sta_Phase_Model spm;	(i) Station/Phase/Vmodel structure
 *	double	*sssc_correct;	(o) SSSC correction [0]; Longitude deriv. [1];
 *				    Latitude deriv. [2];
 *	double	*me_factor;	(o) SSSC modelling error factor
 *	double	*me_sssc;	(o) SSSC modelling error at source, if available
 *	int	sssc_level;	(i) Level of SSSC
 *	char	verbose;	(i) Verbosity level

 * DESCRIPTION
 *	-- read_sssc() reads source-specific station-correction (SSSC) tables 
 *	from files into memory (only filenames now). Each station, correction-type, region 
 *	file encountered will be investigated for the existence of 
 *	a corresponding SSSC.  The filenames to be read have the form 
 *	TT.STA.PHASE.LEVEL.SRC, where TT indicates this is a travel-time
 *	SSSC file, STA is the station name, PHASE is the phase type (name),
 *	LEVEL indicates whether this is a regional or local level SSSC file,
 *	and SRC is the source region name. 

 *	-- read_single_sssc_file() reads, as the name implies, one or more SSSC 
 *	table for a given wave (phase), level, region and station, at a 
 *	time.  For each station, region and phase type, read SSSCs defined 
 *	at lat/lon nodes.  Origin of SSSC lies in the northwest corner of 
 *	the model.   This routine is called one time for regional and
 *	local corrections.

 *	-- apply_sssc() corrects for source-specific station effects in the
 *	observed datum given a station/phase-type/level/region group.
 *	Apply SSSC here to partially compensate for lateral velocity 
 *	heterogeneities. If needed SSSC file is not loaded into memory earlier,
 *	calls read_single_sssc_file.
 

 * DIAGNOSTICS
 *	-- read_sssc() will return an error code of ERR (-1) if a fatal
 *	error is encountered; else it will return OK (0).

 *	-- read_single_sssc_file() will return an error code of ERR (-1)
 *	if a fatal error is encountered.  A return code of ADJUST_STACOR (1)
 *	means that the SSSC was adjusted based on the difference noted 
 *	between the the bulk station station correction value in the
 *	velocity model specification file and the SSSC file.  The value
 *	contained in the velocity model specification file takes precedence.
 *	A return code of ADJUST_SED_VEL (2) means that the SSSC was 
 *	adjusted based on the difference noted between the the sedimentary 
 *	velocity specified in the velocity model specification file and 
 *	the SSSC file.  Again, the value contained in the velocity model 
 *	specification file takes precedence.

 *	-- apply_sssc() will complain if incompatabilities are encountered.
 *	On return will specify whether or not an SSSC was found via Boolean
 *	variable.  If a positive value of me_sssc is returned, this indicates
 *	a source-dependent modeling error exists and should override all other
 *	modeling error information, including me_factor.

 * FILES
 *	Reads all source-specific station correction files here.  Input 
 *	station correction file names are pre-pended here as well.

 * NOTES
 *	For now, depth attribute of sssc structure is always set to 0.0 km.

 * SEE ALSO
 *	Calling routine, setup_tttables(). 

 * AUTHOR
 *	Walter Nagy,  2/ 6/95,	Created.
 *	Walter Nagy, 11/30/95,	Modified to exploit efficiencies gained by 
 *				upgrading sssc handling.  
 *	Walter Nagy, 4/21/95,	A T-T derivative correction is now made to 
 *				when SSSC's are employed.
 *	Walter Nagy, 12/20/95,	All SSSC file and memory management is
 *				exploited using the dyn_array facililities.
 *	Walter Nagy, 12/28/95,	Data integrity enhanced by duplicating important
 *				variables from VMSF on first uncommented 
 *				line of SSSC file.  Sssc pointer now hangs 
 *				directly off sta_phase_model structure.
 *	Walter Nagy,  3/12/98,	Now source-dependent modeling errors can be 
 *				employed for SSSC's on exactly same grid used
 *				to store SSSC themselves.
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "libloc.h"
#include "locp.h"
#include "loc_defs.h"
#include "libinterp.h"
#include "libaesir.h"
#include "dyn_array.h"


#define	MAX_SSSC_STRING	5000
#define	GRID_CHAR_WIDTH	10
#define	SSSC_CHAR_WIDTH	8
#define	ADJUST_STACOR	1
#define	ADJUST_SED_VEL	2


int
read_sssc (Model_Descrip **model_descrip_ptr, int num_models,
   Sta_Phase_Model **sta_phase_model_ptr, int num_spm)
{
   
      FILE	*sssc_fp = NULL;
      Bool	regional_level_sssc_file_found = FALSE;
      int	i, j, k;
      int	fn_len;
      char	cor_file_name[FILENAMELEN];
      char	sssc_dir_pathway[FILENAMELEN], sssc_file_name[FILENAMELEN];
      Model_Descrip	*model_descrip;
      Sta_Phase_Model	*sta_phase_model;
   
      Sssc	*anchor=NULL, *cur=NULL, *prev=NULL, *sssc=NULL,
			*finer_detail=NULL;
   
      Array	list_of_files;
      int	*num_files;
      char	*ptr;
      char	***ptr_to_files = (char ***) NULL;
   
      DIR	*dirp;
      struct	dirent	*direntp;
      Bool	unique_name = FALSE;
   
      model_descrip = *model_descrip_ptr;
      sta_phase_model = *sta_phase_model_ptr;
   
   /*
    * If no SSSC directory exists for entire travel-time directory, then
    * some efficiencies can be gained.  Save list of all files found in
    * each SSSC directory.  Restrict search to valid SSSC files beginning
    * with "TT".
    */
   
      ptr_to_files = UALLOC (char **, num_models);
      num_files    = UALLOC (int, num_models);
      for (i = 0; i < num_models; i++)
      {
         num_files[i] = 0;
         ptr_to_files[i] = (char **) NULL;
         model_descrip[i].sssc_dir_exists = FALSE;
         strcpy (sssc_dir_pathway, model_descrip[i].dir_pathway);
         strcat (sssc_dir_pathway, "/SSSC");
         if ((dirp = opendir (sssc_dir_pathway)) != NULL)
         {
            list_of_files = array_create (sizeof (char *));
            while ((direntp = readdir (dirp)) != NULL)
            {
            /*
            * Allocate local list_of_files array here.  Note that
            * this array is only used in this function, then
            * memory is freed.
            */
            
               ptr = STRALLOCA (direntp->d_name);
               if (! strncmp (direntp->d_name, "TT", 2))
                  array_add (list_of_files, (caddr_t) &ptr);
            }
            model_descrip[i].sssc_dir_exists = TRUE;
            (void) closedir (dirp);
         
            ptr_to_files[i] = (char **) array_list (list_of_files);
            num_files[i] = array_count (list_of_files);
            array_free (list_of_files);
         }
      }
   
   /*
    * Now loop over all sta_phase_model info and look for related SSSC
    * files.
    */
   
      for (i = 0; i < num_spm; i++)
      {
         j = sta_phase_model[i].vel_index;
         if (! model_descrip[j].sssc_dir_exists || num_files[j] == 0)
            continue;	/* No SSSC directory exists, so skip !! */
         if (sta_phase_model[i].phase[0] == '*')
            continue;	/* Wild card phase type, so skip !! */
      
         strcpy (sssc_dir_pathway, model_descrip[j].dir_pathway);
         strcat (sssc_dir_pathway, "/SSSC/");
      
         strcpy (cor_file_name, "TT.");
         strcat (cor_file_name, sta_phase_model[i].sta);
         strcat (cor_file_name, ".");
         strcat (cor_file_name, sta_phase_model[i].phase);
         strcat (cor_file_name, ".reg.");

         unique_name = FALSE;

	 if (strcmp(sta_phase_model[i].source_region,"*") != 0)
	 {
	   if (!strcmp(sta_phase_model[i].source_region,"-"))
	     continue;	 /* "-" we don't need SSSC */
	   else
	   {
             strcat (cor_file_name, sta_phase_model[i].source_region); /* add suffix of SSSC file */
	     unique_name = TRUE;
	   }
	 }
         fn_len = strlen (cor_file_name);
      
       /*
        * Read regional level SSSC file first.  If none exists, no need
        * looking for local level SSSC files.
        */
      
         regional_level_sssc_file_found = FALSE;

         for (k = 0; k < num_files[j]; k++)
         {
            if (! strncmp (ptr_to_files[j][k], cor_file_name, fn_len))
            {
  	       if (unique_name &&  (strcmp(ptr_to_files[j][k], cor_file_name) != 0))
		  continue; /* if we have the suffix, name of file is cor_file_name und unique */

               regional_level_sssc_file_found = TRUE;
               strcpy (sssc_file_name, sssc_dir_pathway);
               strcat (sssc_file_name, ptr_to_files[j][k]);
           
            /*
            * Allocate memory for current sssc structure
            */
            
               if (sta_phase_model[i].sssc == (Sssc *) NULL)
               {
                  if ((sta_phase_model[i].sssc = 
                  (Sssc *) calloc (1, sizeof (Sssc))) == NULL)
                  {
                     fprintf (stderr, "Fatal error: While trying to allocate memory for sssc!!\n");
                     fclose (sssc_fp);
                     return (ERR);
                  }
                  sta_phase_model[i].sssc->next_src_region = (Sssc *) NULL;
                  sta_phase_model[i].sssc->finer_detail = (Sssc *) NULL;
                  anchor = sta_phase_model[i].sssc;
                  prev = sta_phase_model[i].sssc;
                  cur = sta_phase_model[i].sssc;
               }
               else
               {
                  if ((sssc = (Sssc *) calloc (1, sizeof (Sssc))) == NULL)
                  {
                     fprintf (stderr, "Fatal error: While trying to allocate memory for sssc!!\n");
                     fclose (sssc_fp);
                     return (ERR);
                  }
                  prev->next_src_region = sssc;
                  sssc->next_src_region = (Sssc *) NULL;
                  sssc->finer_detail = (Sssc *) NULL;
                  prev = sssc;
                  cur = sssc;
               }

	       cur->sssc_path=STRALLOC(sssc_file_name);
  
            }
         }
      
         if (! regional_level_sssc_file_found)
	 {	    
	    if (! unique_name) {
              continue;
	    }
	    else
      	    {
	      fprintf (stderr, "Error trying to find SSSC file: %s\n", cor_file_name);
	      continue;
	    }
	 }
       /*
        * If we have reached this point a regional level SSSC file has
        * been found.  Now look for any local level SSSC corrections.
        * Make sure they exist inside a previously defined regional
        * level SSSC area (currently doesn't work).  User's are only limited to 1 local
        * level SSSC description per regional level SSSC file if we have the region_source
	* (suffix) in vmsf file.
	*
	* No sssc_level available so we have to read local SSSC !!!
        */
      
         strcpy (cor_file_name, "TT.");
         strcat (cor_file_name, sta_phase_model[i].sta);
         strcat (cor_file_name, ".");
         strcat (cor_file_name, sta_phase_model[i].phase);
         strcat (cor_file_name, ".local.");
	 if (strcmp(sta_phase_model[i].source_region,"*") != 0)
	 {
	   if (!strcmp(sta_phase_model[i].source_region,"-"))
	     continue;	 /* "-" we don't need SSSC */
	   else
             strcat (cor_file_name, sta_phase_model[i].source_region); /* add suffix of SSSC file */
	 }
         fn_len = strlen (cor_file_name);
      
         for (k = 0; k < num_files[j]; k++)
         {
            if (! strncmp (ptr_to_files[j][k], cor_file_name, fn_len))
            {
  	       if (unique_name && (strcmp(ptr_to_files[j][k], cor_file_name) != 0))
		  continue; /* if we have the suffix, name of file is cor_file_name und unique */

               strcpy (sssc_file_name, sssc_dir_pathway);
               strcat (sssc_file_name, ptr_to_files[j][k]);
            
            /*
            * Allocate memory for current local (detail) sssc structure
            */
            
		if (anchor->finer_detail == (Sssc *) NULL)
		{
               		if ((finer_detail = 
		               (Sssc *) calloc (1, sizeof (Sssc))) == NULL)
               		{
		                  fprintf (stderr, "Fatal error: While trying to allocate memory for sssc!!\n");
                		  fclose (sssc_fp);
		                  return (ERR);
               		}
					anchor->finer_detail=finer_detail;
               		finer_detail->next_src_region = (Sssc *) NULL;
	                finer_detail->finer_detail = (Sssc *) NULL;
			finer_detail->sssc_path=STRALLOC(sssc_file_name);
			prev = finer_detail;
		}
		else
		{
			if ((sssc = (Sssc *) calloc (1, sizeof (Sssc))) == NULL)
			{
			   fprintf (stderr, "Fatal error: While trying to allocate memory for sssc!!\n");
		 	   fclose (sssc_fp);
	 	 	   return (ERR);
			}
				prev->next_src_region = sssc;
				sssc->next_src_region = (Sssc *) NULL;
				sssc->finer_detail = (Sssc *) NULL;
				sssc->sssc_path=STRALLOC(sssc_file_name);
				prev = sssc;
		}
           
            /*
            * Find regional level sssc structure for given station 
            * and phase.  If none is found, then skip this SSSC file 
            * and free its memory.  Otherwise, attached local level
            * SSSC file to its corresponding regional level SSSC file.
            * We must make sure that local level SSSC grid is fully
            * contained within associated regional level SSSC grid.
            */
            
               sssc = anchor;
               while (sssc != (Sssc *) NULL)
               {
                  sssc->finer_detail = finer_detail;
                  sssc = sssc->next_src_region;
               } 
            }
         }
      }
   /*
    * Lastly, free temporary filename arrays!
    */
   
      for (j = 0; j < num_models; j++)
         UFREE (ptr_to_files[j]);
      UFREE (ptr_to_files);
      UFREE (num_files);
   
      return (OK);
   }


int
read_single_sssc_file (Sta_Phase_Model spm, int sssc_level, char verbose)
{

      FILE	*sssc_fp=NULL, *new_fp=NULL;
      int	i, j, k, m;
      int	icode = OK;  /* If positve: a warning; if negative: an error */
      double	bulk_sta_corr, sed_vel;
      double	adj_stacor = 0.0;
      char	phase_type[9], regional_or_local[9];
      char	station_name[7], vmodel[16];
      char	input_string[MAX_SSSC_STRING];
      char	new_outfile_name[FILENAMELEN];
      Sssc	*cur;
		int	local_loaded = 0;

    cur = spm.sssc;
    while (cur != (Sssc *) NULL)
    {

      cur->sta_cor = (float **) NULL;
      cur->sssc_2nd_deriv = (float **) NULL;
      cur->mdl_err = (float **) NULL;

      if ((sssc_fp = fopen (cur->sssc_path, "r")) == NULL)
      {
	  fprintf (stderr, "Error trying to open SSSC file: %s\n", cur->sssc_path);
          UFREE(cur->sssc_path);
          cur->sssc_path=STRALLOC("ERR_OPEN_FILE!");
	  return (ERR);
      }
   
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            sscanf (input_string, "%s%s%s%s%s%lf%lf%lf", station_name,
               phase_type, cur->source_region, vmodel,
               regional_or_local, &bulk_sta_corr,
               &sed_vel, &cur->me_factor);
            break;
         }
      }
   
   /*
    * Do some error checking here to make sure SSSC file station name and
    * phase type are consistent with information described in velocity 
    * model specification file (i.e., sta_phase_model structure 
    * [spm here]) and the file name itself.
    */
   
      if (strcmp (station_name, spm.sta))
      {
         fprintf (stderr, "Fatal error: Station: %s in SSSC file is inconsistent with filename!!\n", station_name);
         return (ERR);
      }
      if (strcmp (phase_type, spm.phase))
      {
         fprintf (stderr, "Fatal error: Phase: %s in SSSC file is inconsistent with filename!!\n", phase_type);
         return (ERR);
      }
   
   /*
    * If an inconsistency is found with the bulk static station correction
    * (bulk_sta_corr) or the sedimentary velocity (sed_vel) we can correct,
    * but should also warn the user that this discrepency should be fixed
    * ASAP.  If icode == ADJUST_STACOR, then SSSC corrections must be 
    * adjusted for difference.  If icode == ADJUST_SED_VEL, then SSSC 
    * corrections must be adjusted for elevation correction relative to 
    * sedimentary velocity specified in station/phase/model file.
    */
   
      adj_stacor = 0.0;
      if (fabs(bulk_sta_corr - spm.bulk_sta_corr) > 0.0001)
      {
         adj_stacor = bulk_sta_corr - spm.bulk_sta_corr;
         icode = ADJUST_STACOR;
      }
   
      if (fabs(sed_vel - spm.sed_vel) > 0.0001)
      {
         icode = ADJUST_SED_VEL;
      }
   
      if (icode > OK)
      {
       /*
        * If SSSC grid had to be updated, then write a new file with
        * the _new post-fix added to its original name in the /tmp 
        * area.  Also make sure user know which velocity model this
        * SSSC file belongs to by attaching vmodel name, as well.  
        * Notify user by writing this info to stdout.
        */
      
         strcpy (new_outfile_name, "/tmp/TT.");
         strcat (new_outfile_name, station_name);
         strcat (new_outfile_name, ".");
         strcat (new_outfile_name, phase_type);
         strcat (new_outfile_name, ".");
         strcat (new_outfile_name, regional_or_local);
         strcat (new_outfile_name, ".");
         strcat (new_outfile_name, cur->source_region);
         strcat (new_outfile_name, "_new_");
         strcat (new_outfile_name, vmodel);
      
         if (icode == ADJUST_STACOR)
         {
            fprintf (stderr, "\nWarning: Bulk station correction in SSSC file is inconsistent with velocity\n");
            fprintf (stderr, "         model spec. file value for station:%s phase: %s\n", spm.sta, spm.phase);
            fprintf (stderr, "\nSSSC file has been updated for you and put in temporary (and volatile)\n");
            fprintf (stderr, " directory: %s\n", new_outfile_name);
         }
         if (icode == ADJUST_SED_VEL)
         {
            fprintf (stderr, "\nWarning: Sed. velocity in SSSC file is inconsistent with velocity\n");
            fprintf (stderr, "         model spec. file value for station:%s phase: %s\n", spm.sta, spm.phase);
            fprintf (stderr, "\nSSSC file has been updated for you and put in temporary (and volatile)\n");
            fprintf (stderr, " directory: %s\n", new_outfile_name);
         }
      
         new_fp = fopen (new_outfile_name, "w+");
      
         fprintf (new_fp, "# Station Phase    Source Region    Velocity Model   Level  StaCor  SedVel MEFac\n");
         fprintf (new_fp, "# ======= ======== ================ ================ =====  ======  ====== =====\n");
         fprintf (new_fp, "  %-6s  %-8s %-16s %-16s %-5s%8.3f%8.3f%6.2f\n",
            station_name, phase_type, cur->source_region, vmodel, 
            regional_or_local, spm.bulk_sta_corr, spm.sed_vel, 
            cur->me_factor);
         fprintf (new_fp, "# Number of Lat/Lon samples, respectively:\n");
      }
   
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            sscanf (input_string, "%d%d%*[^\n]", 
               &cur->nlats, &cur->nlons);
            if (icode > OK)
               fprintf (new_fp, "%5d%5d\n", cur->nlats, cur->nlons);
            break;
         }
      }
   
      cur->lat_grid_ref = UALLOC (float, cur->nlats);
      cur->lon_grid_ref = UALLOC (float, cur->nlons);
      cur->sta_cor = UALLOC (float *, cur->nlats);
      cur->sssc_2nd_deriv = UALLOC (float *, cur->nlats);
   
   /*
    * Read actual latitude and longitude coordinates.  We will adjust 
    * these to be relative to the NW origin a little later.
    */
   
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            for (i = 0, k = 0; i < cur->nlats; i++)
            {
               sscanf (&input_string[k], "%f", &cur->lat_grid_ref[i]);
               k += GRID_CHAR_WIDTH;
            }
            if (icode > OK)
            {
               fprintf (new_fp, "# Latitude samples (deg) from north to south:\n");
               for (i = 0; i < cur->nlats; i++)
                  fprintf (new_fp, "%10.3f", cur->lat_grid_ref[i]);
            }
            break;
         }
      }
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            for (i = 0, k = 0; i < cur->nlons; i++)
            {
               sscanf (&input_string[k], "%f", &cur->lon_grid_ref[i]);
               k += GRID_CHAR_WIDTH;
            }
            if (icode > OK)
            {
               fprintf (new_fp, "\n# Longitude samples (deg) from west to east:\n");
               for (i = 0; i < cur->nlons; i++)
                  fprintf (new_fp, "%10.3f", cur->lon_grid_ref[i]);
            }
            break;
         }
      }
   
   /*
    * Set up our lat/lon grid reference model where our lat/lon origin 
    * coordinate position we be (0.0, 0.0) and positively increase to 
    * the south and east.  This is an important distinction, since all 
    * internal handling of SSSC grid info is relative to this frame 
    * of reference, even though the user views the SSSC files in a 
    * more geographically understandable format.  This has been done 
    * to ensure no problems are encountered when crossing major 
    * lat/lon discontinuities (e.g., the -180/180 deg. longitude 
    * discontinuity).
    */
   
      cur->origin_nw_lat = cur->lat_grid_ref[0];
      cur->origin_nw_lon = cur->lon_grid_ref[0];
      cur->se_lat_bound  = cur->lat_grid_ref[cur->nlats-1];
      cur->se_lon_bound  = cur->lon_grid_ref[cur->nlons-1];
      cur->depth = 0.0;	/* Depth is always 0.0 km. for now */
   
      cur->lat_grid_ref[0] = 0.0;
      for (j = 1; j < cur->nlats; j++)
         cur->lat_grid_ref[j] = cur->origin_nw_lat - cur->lat_grid_ref[j];
      cur->lon_grid_ref[0] = 0.0;
      for (j = 1; j < cur->nlons; j++)
      {
         if (cur->lon_grid_ref[j] > cur->origin_nw_lon)
            cur->lon_grid_ref[j] -= cur->origin_nw_lon;
         else
         {
         /* We've crossed IDL (180.0/-180.0 lon. boundary) */
            cur->lon_grid_ref[j] += 360.0;
            cur->lon_grid_ref[j] -= cur->origin_nw_lon;
         }
      }
   
   /*
    * Now read grided SSSC's themselves
    */
   
      m = 0;
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            cur->sta_cor[m] = UALLOC (float, cur->nlons);
            cur->sssc_2nd_deriv[m] = UALLOC (float, cur->nlons);
            for (j = 0, k = 0; j < cur->nlons; j++)
            {
               sscanf (&input_string[k], "%f", &cur->sta_cor[m][j]);
               if (m != 0 && m != cur->nlats-1 && 
               j != 0 && j != cur->nlons-1)
                  cur->sta_cor[m][j] += adj_stacor;
               k += SSSC_CHAR_WIDTH;
            }
            ++m;
            if (m == cur->nlats)
               break;
         }
      }
   
   /*
    * Write new SSSC information to output file
    */
   
      if (icode > OK)
      {
         fprintf (new_fp, "\n# SSSC's in map view (origin is NW corner):");
         for (m = 0; m < cur->nlats; m++)
         {
            fprintf (new_fp, "\n");
            for (j = 0, k = 0; j < cur->nlons; j++)
               fprintf (new_fp, "%8.2f", cur->sta_cor[m][j]);
         }
         fprintf (new_fp, "\n");
      }
   
   /*
    * If available, read source-dependent SSSC modeling errors.  The
    * existence of these modeling errors will automatically negate the
    * simple distance-dependent modeling errors stored in the travel-time
    * tables.  Also, the me_factor field on the first line of the SSSC
    * file will be ignored.  These modeling errors must be defined on
    * exactly the same grid as the SSSC's themselves.
    */
   
      m = 0;
      while (fgets (input_string, MAX_SSSC_STRING, sssc_fp) != NULL)
      {
       /* Skip lines w/ '#' in 1st column */
         if (!strncmp (input_string, "#", 1))
            continue;
         else
         {
            if (m == 0)
               cur->mdl_err = UALLOC (float *, cur->nlats);
            cur->mdl_err[m] = UALLOC (float, cur->nlons);
            for (j = 0, k = 0; j < cur->nlons; j++)
            {
               sscanf (&input_string[k], "%f", &cur->mdl_err[m][j]);
               k += SSSC_CHAR_WIDTH;
            }
            ++m;
            if (m == cur->nlats)
               break;
         }
      }
   
   /*
    * Write SSSC model errors to output file
    */
   
      if (m > 0 && icode > OK)
      {
         fprintf (new_fp, "\n# SSSC modeling errors in map view (origin is NW corner):");
         for (m = 0; m < cur->nlats; m++)
         {
            fprintf (new_fp, "\n");
            for (j = 0, k = 0; j < cur->nlons; j++)
               fprintf (new_fp, "%8.2f", cur->mdl_err[m][j]);
         }
         fprintf (new_fp, "\n");
      }
   
   /*
    * Pre-compute 2nd derivatives
    */
   
      splie2 (cur->lat_grid_ref, cur->lon_grid_ref, cur->sta_cor, 
         cur->nlats, cur->nlons, cur->sssc_2nd_deriv);
      
      if (icode > OK)
         fclose (new_fp);
   
      fclose (sssc_fp);

      if (verbose == '3' || verbose == '4' || verbose == 'y' || verbose == 't')
        fprintf(stderr, "SSSC: %s loaded\n", cur->sssc_path);

        UFREE(cur->sssc_path);

      cur = cur->next_src_region;

/* Load local SSSC (sssc_level = 2)*/
      if (sssc_level == 2)
      {  
        if ((cur == (Sssc *) NULL) && local_loaded == 0)
        {
           cur = spm.sssc->finer_detail;
	   local_loaded = 1;
        }
      }
    }
      return (icode);
   }


Bool
apply_sssc (double ev_lat, double ev_lon, Sta_Phase_Model spm,
	double *sssc_correct, double *me_factor, double *me_sssc,
	int sssc_level, char verbose)
{
      Bool	in_sssc_grid = FALSE;
      int	n;
      int	lat1=0, lat2=0;
      float	a, b, c, corr, dlat=0, dlon=0, h, percent=0;
      float	lat_1st_deriv, lat_2nd_deriv;
      double	top_1st, bot_1st, val_1st;
      Sssc	*cur=NULL, *keep=NULL;
  
      float	b_corr = 0.001; /* to awoid boundary effects during comparison */  
      sssc_correct[0] = 0.0;	/* Actual SSSC to be returned */
      sssc_correct[1] = 0.0;	/* Longitude derviative of SSSC */
      sssc_correct[2] = 0.0;	/* Latitude derviative of SSSC */
      *me_factor	= 1.0;	/* SSSC modelling error factor */
      *me_sssc	= -1.0;	/* SSSC modelling error at source */
     
    /*
     * If spm.sssc->sssc_path != NULL, read SSSC file(s) for the phase of the
     * station.
    */
      if (spm.sssc->sssc_path != (char *) NULL)
      {
	if (!strcmp(spm.sssc->sssc_path, "ERR_OPEN_FILE!"))
		/* couldn't open file by read_single_sssc_file!*/
         return (FALSE);
	 
	if(read_single_sssc_file(spm, sssc_level, verbose))
         return (FALSE);
      }
   /*
    * Determine whether event is located within one of the regional
    * source regions -- If not, return w/o a correction. dlat and
    * dlon are measures relative to an origin coordinate at the 
    * northwesternmost corner of the SSSC grid, where latitude is 
    * positive to the south and longitude is positive to the east.
    * dlat and dlon are both 0.0 at the origin coordinate position.
    */
   
      cur = spm.sssc;
      while (cur != (Sssc *) NULL)
      {
         if (ev_lat < (cur->origin_nw_lat - b_corr) && ev_lat > (cur->se_lat_bound + b_corr) )
         {
            if (cur->origin_nw_lon < cur->se_lon_bound)
            {
               if (ev_lon > (cur->origin_nw_lon + b_corr) &&
               ev_lon < (cur->se_lon_bound - b_corr) )
               {
                  in_sssc_grid = TRUE;
                  dlon = ev_lon - cur->origin_nw_lon;
               }
            }
            else
            {
            /* Boundary data crosses -180/+180 longitude */
               if (ev_lon > 0.0 && ev_lon > (cur->origin_nw_lon + b_corr) )
               {
                  in_sssc_grid = TRUE;
                  dlon = ev_lon - cur->origin_nw_lon;
               }
               else if (ev_lon < 0.0 && ev_lon < (cur->se_lon_bound - b_corr) )
               {
                  in_sssc_grid = TRUE;
                  dlon = 360.0 + ev_lon - cur->origin_nw_lon;
               }
            }
            if (in_sssc_grid)
            {
               dlat = cur->origin_nw_lat - ev_lat;
               keep = cur;
               break;
            }
         }
         cur = cur->next_src_region;
      }
      if (! in_sssc_grid)
         return (FALSE);		/* Regional correction not found */
   
   /*
    * Regional source region found, now look for a local source region (sssc_level=2).
    * If one exists, set new sssc pointer.
    */

      if(sssc_level == 2)
      {
	if (spm.sssc->finer_detail != (Sssc *) NULL)
      	{
	  cur = spm.sssc->finer_detail;

          while (cur != (Sssc *) NULL)
          {
            if (ev_lat < cur->origin_nw_lat && ev_lat > cur->se_lat_bound)
       	    {
               if (cur->origin_nw_lon < cur->se_lon_bound)
       	       {
               	  if (ev_lon > cur->origin_nw_lon &&
                  ev_lon < cur->se_lon_bound)
       	          {
               	     in_sssc_grid = TRUE;
                     dlon = ev_lon - cur->origin_nw_lon;
       	          }
               }
       	       else
               {
       	       /* Boundary data crosses -180/+180 longitude */
               	  if (ev_lon > 0.0 && ev_lon > cur->origin_nw_lon)
                  {
       	             in_sssc_grid = TRUE;
               	     dlon = ev_lon - cur->origin_nw_lon;
                  }
       	          else if (ev_lon < 0.0 && ev_lon < cur->se_lon_bound)
               	  {
                     in_sssc_grid = TRUE;
       	             dlon = 360.0 + ev_lon - cur->origin_nw_lon;
               	  }
               }
       	       if (in_sssc_grid)
               {
       	          dlat = cur->origin_nw_lat - ev_lat;
               	  keep = cur;
                  break;
       	       }
            }
       	    cur = cur->next_src_region;
         }
        }
      }   
      cur = keep;
   
   /*
    * Determine SSSC and derivatives in latitude direction via 
    * bi-cubic spline interpolation at point (dlat, dlon)
    */
   
      splin2 (cur->lat_grid_ref, cur->lon_grid_ref, cur->sta_cor, 
         cur->sssc_2nd_deriv, cur->nlats, cur->nlons, dlat, dlon,
         &corr, &lat_1st_deriv, &lat_2nd_deriv);
   
      sssc_correct[0] = (double) corr;
      sssc_correct[2] = (double) lat_1st_deriv;
      *me_factor = cur->me_factor;
   
   /*
    * If source-dependent modeling error information has been defined
    * for this SSSC grid, then perform bi-linear interpolation and store 
    * in variable, me_sssc.  A positive value of me_sssc indicates that 
    * this value should be employed over-and-above all other modeling 
    * error information.
    */
   
      if (cur->mdl_err != (float **) NULL)
      {
         for (n = 1; n < cur->nlats; n++)
         {
            if (cur->lat_grid_ref[n] > dlat)
            {
               lat1 = n - 1;
               lat2 = n;
               percent = (dlat - cur->lat_grid_ref[n-1]) /
                  (cur->lat_grid_ref[n] - cur->lat_grid_ref[n-1]);
               break;
            }
         }
         for (n = 1; n < cur->nlons; n++)
         {
            if (cur->lon_grid_ref[n] > dlon)
            {
               h = cur->lon_grid_ref[n] - cur->lon_grid_ref[n-1];
               a = (dlon - cur->lon_grid_ref[n-1]) / h;
            
               b = cur->mdl_err[lat1][n-1] + a*(cur->mdl_err[lat1][n]-cur->mdl_err[lat1][n-1]);
               c = cur->mdl_err[lat2][n-1] + a*(cur->mdl_err[lat2][n]-cur->mdl_err[lat1][n-1]);
            
               *me_sssc = b + percent*(c - b);
               break;
            }
         }
      }
   
   /*
    * Approximate longitude derivative based on local 2nd derivatives in
    * the same direction.
    */
   
      for (n = 1; n < cur->nlats; n++)
      {
         if (cur->lat_grid_ref[n] > dlat)
         {
            lat1 = n - 1;
            lat2 = n;
            percent = (dlat - cur->lat_grid_ref[n-1]) /
               (cur->lat_grid_ref[n] - cur->lat_grid_ref[n-1]);
            break;
         }
      }
      for (n = 1; n < cur->nlons; n++)
      {
         if (cur->lon_grid_ref[n] > dlon)
         {
            h = cur->lon_grid_ref[n] - cur->lon_grid_ref[n-1];
            a = (cur->lon_grid_ref[n] - dlon) / h;
            b = (dlon - cur->lon_grid_ref[n-1]) / h;
            top_1st = ((cur->sta_cor[lat1][n]-cur->sta_cor[lat1][n-1])/h) - (((3.0*a*a-1.0)*h*cur->sssc_2nd_deriv[lat1][n-1])/6.0) + (((3.0*b*b-1.0)*h*cur->sssc_2nd_deriv[lat1][n])/6.0);
            bot_1st = ((cur->sta_cor[lat2][n]-cur->sta_cor[lat2][n-1])/h) - (((3.0*a*a-1.0)*h*cur->sssc_2nd_deriv[lat2][n-1])/6.0) + (((3.0*b*b-1.0)*h*cur->sssc_2nd_deriv[lat2][n])/6.0);
            val_1st = top_1st + percent*(bot_1st - top_1st);
            sssc_correct[1] = (val_1st * cos(ev_lat*DEG_TO_RAD));
            break;
         }
      }
   
      return (TRUE);
   }


