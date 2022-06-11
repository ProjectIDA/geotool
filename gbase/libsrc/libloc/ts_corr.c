
/*
 * Copyright (c) 1993-1996 Science Applications International Corporation.
 *

 * NAME
 *	read_ts_corr -- Read test-site correction table.
 *	get_ts_corr -- Get a single test-site correction for sta/phase/region.

 * FILE
 *	ts_corr.c

 * SYNOPSIS
 *	int
 *	read_ts_corr (default_vmodel_pathway, default_vmodel)
 *	char	*default_vmodel_pathway;(i) Directory pathway where default
 *					    T-T tables exist
 *	char	*default_vmodel;    	(i) Default velocity model name

 *	double
 *	get_ts_corr (ts_region, phase, sta, ts_corr_found)
 *	char	*ts_region;	(i) Test-site region requested
 *	char	*phase;		(i) Phase type for given region
 *	char	*sta;		(i) Station for given region
 *	Bool	*ts_corr_found	(o) Was a test-site correction found ?

 * DESCRIPTION
 *	-- read_ts_corr() reads the test-site correction table from files 
 *	into memory.  Each station encountered in an test-site correction 
 *	file will be read.

 *	-- get_ts_corr() gets the test-site correction given a station,
 *	phase and region ID assuming the tables have alread been read via
 *	a call from function, read_ts_corr().  Test-site correction 
 *	structure, ts_cor, is handled entirely within this file.

 * DIAGNOSTICS
 *	-- read_ts_corr() will return with an error code of ERR (-1) if input
 *	contains improperly formatted data.

 *	-- get_ts_corr() will return -999.0 if requested region ID is not 
 *	found.  It will simply return 0.0 if a station or phase is not
 *	found.

 * FILES
 *	-- read_ts_corr() reads input test-site correction file.  The file
 *	name MUST be "vmodel.ts_cor", and MUST also be located in the
 *	immediate default T-T table directory.

 * NOTES
 *	Test-site corrections are currently ADSN-specific.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy, 5/93,	Created.
 *	Walter Nagy, 11/16/95,	File re-named, ts_corr.c. 
 *	Walter Nagy, 11/30/95,	Added function, get_ts_corr(), which grabs 
 *				the test-site correction given a region, 
 *				station and phase type.
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "locp.h"
#include "ts_cor.h"

static	int	num_ts_regions = 0;
static	TS_Cor	*ts_cor = (TS_Cor *) NULL;


int
read_ts_corr (char *default_vmodel_pathway, char *default_vmodel)
{

	FILE	*ts_fp;
	int	i, j, sta_len;
	char	ts_file_name[FILENAMELEN];
	int     old_num_ts_regions = 0;


	strcpy (ts_file_name, default_vmodel_pathway);
	strcat (ts_file_name, "/");
	strcat (ts_file_name, default_vmodel);
	strcat (ts_file_name, ".ts_cor");

	/*
	 * Open test-site correction file, if it is available!  To be read
	 * this file MUST exist under the default velocity model T-T area
	 * as file, default_vmodel_pathway/vmodel.ts_cor,  It's OK to not
	 * have a test-site correction file, but obviously, no such 
	 * correction can then be applied.
	 */

	if ((ts_fp = fopen (ts_file_name, "r")) == NULL)
	{
	    fprintf (stderr, "Message: No test-site file exists in default T-T table area!\n");
	    ts_cor = (TS_Cor *) NULL;
	    return (OK);
	}

	/* 
	 * How many test-site regions are there ? 
	 */

	old_num_ts_regions = num_ts_regions;
	if (fscanf (ts_fp, "%d%*[^\n]", &num_ts_regions) != 1) return(ERR);

	/*
	 * If a previous test-site correction file existed, free its memory,
	 * then re-allocate, as necessary, ts_cor structure.
	 */

	if (ts_cor != (TS_Cor *) NULL)
	{
	    for (i = 0; i < old_num_ts_regions; i++)
	    {
		for (j = 0; j < ts_cor[i].num_sta; j++)
		{
		    UFREE (ts_cor[i].sta[j]);
		}
		UFREE (ts_cor[i].sta);
		UFREE (ts_cor[i].ts_corr);
	    }
	    UFREE (ts_cor);
	}

	ts_cor = UCALLOC (TS_Cor, num_ts_regions);

	/*
	 * Read test-site corrections themselves.
	 */

	for (i = 0; i < num_ts_regions; i++)
	{
	    if ((fscanf (ts_fp, "%d%s%s%d%*[^\n]", &ts_cor[i].reg_num,
			   ts_cor[i].reg_name_id, ts_cor[i].phase,
			   &ts_cor[i].num_sta)) != 4)
		return (ERR);
	    ts_cor[i].sta = UCALLOC (char *, ts_cor[i].num_sta);
	    ts_cor[i].ts_corr = UALLOC (double, ts_cor[i].num_sta);

	    /* Read actual test-site correction data */

	    for (j = 0; j < ts_cor[i].num_sta; j++)
	    {
		ts_cor[i].sta[j] = UCALLOC (char, 7);
		if (fscanf (ts_fp, "%6s %lf", ts_cor[i].sta[j], 
			    &ts_cor[i].ts_corr[j]) != 2) return(ERR);
		if ((strchr (ts_cor[i].sta[j], '*')) != NULL)
		{
		    sta_len = strlen (ts_cor[i].sta[j]);
		    strcpy (&ts_cor[i].sta[j][sta_len-1], "\0");
		}
	    }
	}

	fclose (ts_fp);
	return (OK);
}


double
get_ts_corr (char *ts_region, char *phase, char *sta, Bool *ts_corr_found)
{
	Bool	region_found = FALSE;
	int	i;
	int	ts_reg;
	int	ts_sta;


	ts_reg = -1;
	ts_sta = -1;
	*ts_corr_found = FALSE;

	for (i = 0; i < num_ts_regions; i++)
	{
	    if (STREQ (ts_region, ts_cor[i].reg_name_id))
	    {
		region_found = TRUE;
		if (STREQ (ts_cor[i].phase, phase))
		{
		    ts_reg = i;
		    break;
		}
	    }
	}

	if (!region_found)
	{
	    fprintf (stderr, " Error %d: Requested test-site region not available!\n", TSerror2);
	    return (-999.0);
	}
	if (ts_reg < 0)
	    return (0.0);

	for (i = 0; i < ts_cor[ts_reg].num_sta; i++)
	{
	    if (STREQ (sta, ts_cor[ts_reg].sta[i]))
	    {
		ts_sta = i;
		*ts_corr_found = TRUE;
		break;
	    }
	}
	if (*ts_corr_found)
	    return (ts_cor[ts_reg].ts_corr[ts_sta]);
	else
	    return (0.0);
}
