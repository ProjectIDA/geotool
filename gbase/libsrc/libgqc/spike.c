/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * NAME
 * 
 *	find_ss_spikes
 *	find_mda_spikes
 *
 * FILE 
 *
 *      spike.c
 *
 * SYNOPSIS
 *
 *	void
 *	find_ss_spikes (data, npts, mask, index, n_index)
 *	float	*data;		(i) Data vector to search
 *	int	npts;		(i) Number of elements of data
 *	QCMask	*mask;		(i) Mask with spike detection criteria
 *	int	**index;	(o) Indices of spikes
 *	int	*n_index;	(o) Number of elements of index
 *
 *	void
 *	find_mda_spikes (data, npts, ndata, mask, index, n_index)
 *	float	**data;		(i) Data vectors to search
 *	int	*npts;		(i) Vector of number of points in each data
 *	int	ndata;		(i) Number of data vectors 
 *	QCMask	*mask;		(i) Masks with spike detection criteria
 *	int	**index;	(o) Indices of spikes for each data vector
 *	int	*n_index;	(o) Number of elements of each index
 *
 *	static
 *	void
 *	compute_stats (data, npts, diff, maxdata, maxdiff)
 *	float	*data;		(i) Data vector
 *	int	npts;		(i) Number of elements of data
 *	float	*diff;		(i/o) Vector of minimum of forward,backward 
 *				    absolute first differences 
 *	float	*maxdata;	(o) Maximum value in data
 *	float	*max;		(o) Maximum value in diff
 *
 *	static
 *	void
 *	compute_valid_stats (data, npts, diff, mask, maxdata, maxdiff)
 *	float	*data;		(i) Data vector
 *	int	npts;		(i) Number of elements of data
 *	float	*diff;		(i/o) Vector of minimum of forward,backward 
 *				    absolute first differences 
 *	QCMask	*mask;		(i) Mask of data
 *	float	*maxdata;	(o) Maximum value in data
 *	float	*max;		(o) Maximum value in diff
 *
 *	static
 *	void
 *	compute_avg (d, npts, avg)
 *	float	*d;		(i) Data vector
 *	int	npts;		(i) Number of elements of d
 *	double	*avg;		(o) Average value of d
 *
 * DESCRIPTION
 *
 *	find_ss_spikes() saves the indices where a data point
 *	exceeds spike_level times the percentile of the first 
 *	differences of the data set and the point exceeds spike_level 
 *	times the percentile of the first differences of a window of length
 *	npwin centered around the point.  The spike_level and the npwin
 *	are input in the mask structure. Assumes non-NULL data pointer
 *	on input.  If npwin > npts, npts is used in place of npwin.
 *
 *	find_mda_spikes() takes a number of data vectors on input and 
 *	saves the indices of those data points which are considered spikes
 *	using statistics from the entire set of data vectors.  Either the first
 *	differences of the data or the data itself can be used to search
 *	for spikes.  Also, either the average maximum absolute amplitude or
 *	a percentile of the maximum absolute amplitude of all the data
 *	vectors can be used to set the spike detection threshold. 
 *	All data vectors are assumed to begin at the same time.
 *	All data vectors are assumed to have the same number of elements.
 *	Assumes that not all of the data vectors are completely masked.
 *
 *	compute_stats() returns a list of the minimum of the forward and
 *	backward absolute first differences of the input data, the
 *	maximum absolute data value, and the max element in diff.
 *	Assumes non-NULL data pointer on input.
 *
 *	compute_valid_stats() returns a list of the minimum of the forward and
 *	backward absolute first differences of those values in the input data
 *	which are not masked, a vector of the state of these differences,
 *	the maximum absolute valid data value, and 
 *	the max element in diff.  Wrapper for compute_stats().
 *	Assumes non-NULL data pointer on input, and that not all data is
 *	masked.
 *
 *	compute_avg() computes the average value of a data set.
 *	Assumes non-NULL data pointer on input.
 *
 *
 * DIAGNOSTICS
 *
 *	Output from find_mda_spikes()
 * FILES
 *
 * NOTES
 * 
 *	Calling routine must free output arrays from find_ss_spikes(),
 *	find_mda_spikes().
 *
 * SEE ALSO
 *
 *	Single data vector spike detection adapted from ssclean.f in 
 *	SigPro program.  Multiple data spike detection adapted from
 *	RONAPP routines in SigPro program.
 *
 * AUTHOR
 *	Darrin Wahl
 *	Jeff Given
 *
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>	/* for fabs() */
#include "dyn_array.h"
#include "libdataqcp.h"
#include "libstring.h"
#include "logErrorMsg.h"

#define FREE(a) if(a) {free(a); a = NULL;}

/* Local functions forward definition */
static void compute_avg(float *data, int npts, double *avg);

static void compute_stats(float *data, int npts, float *diff, double *maxdata,
			double *maxdiff);
static void compute_valid_stats(float *data, int npts, float *diff,
			QCMask *mask, double *maxdata, double *maxdiff);


void
find_ss_spikes(float *data, int npts, QCMask *mask, int **index, int *n_index)
{
	Array	a = NULL;
	Array	da = NULL;
	QCMask	tmask;
	float	*diff = NULL;
	float	*d = NULL;
	int	ndiff;
	double	perc;
	double	thr;
	double	dum;
	int	istart;
	int	iend;
	double	p = 0.0;
	double	spike_local = 0.0;
	double	spike_global = 0.0;
	int   	nwindow = 0;
	int	nwin2 = 0;
	int	i, j;
	
	*n_index = 0;
	*index = NULL;

	if(qc_all_masked (npts, mask) || npts < 3) return;
	
	/* Get parameters */
	thr = mask->def.spike_thr;
	perc = mask->def.spike_val;

	/* Determine odd number of samples in data window */
	nwindow = MIN (mask->def.spike_npwin, npts);
	if(nwindow % 2 == 0)
	{
	    nwindow -= 1;
	}
	nwin2 = nwindow / 2;

	/* Allocate storage for first differences */
	if(!(diff = (float *)mallocWarn(npts*sizeof(float))));
	memset((char *)diff, 0, npts * sizeof(float));

	/* Initialize local temporary mask */
	memset((char *)&tmask, 0, sizeof(QCMask));

	/* Compute first differences using only valid data */
	compute_valid_stats (data, npts, diff, mask, &dum, &dum);

	da = array_create (sizeof (float));
	for (i = 0; i < npts; i++)
	{
	    if(!qc_check_segment(i, i, mask, 1) && diff[i] > 0.0)
	    {
		array_add (da, (char *) &diff[i]);
	    }
	}
	ndiff = array_count(da);
	d = (float *)array_list(da);
	
	/* Determine global percentile of all differences */
	find_percentile(perc, d, ndiff, &p);
	array_free(da);

	/* Global spike value */
	spike_global = p * thr;

	/* 
	 * Look for spikes.  Difference values are spikes if
	 * the value is a spike relative to the entire data set and 
	 * relative to a small window around the value.
	 * Store indices in a dynamic array.
	 */
	a = array_create (sizeof (int));

	/* 
	 * Look for data spikes using first differences at beginning of 
	 * array only if there is valid data in the interval 
	 */
	istart = 0;
	iend = nwindow - 1;
	(void) qc_mask_interval (npts, mask, istart, iend, 0, &tmask);

	if(!qc_all_masked(nwindow, &tmask))
	{
	    ndiff = 0;
	    for (i = istart; i <= iend; i++)
	    {
		if (!qc_check_segment (i, i, &tmask, 1) && diff[i]>0.0)
		{
		    d[ndiff++] = diff[i];
		}
	    }
	    find_percentile (perc, d, ndiff, &p);

	    spike_local = p * thr;
	    for (i = 0; i < nwin2; i++)
	    {
		if (!qc_check_segment (i, i, &tmask, 1) &&
		    diff[i] > spike_global && diff[i] > spike_local)
		{
		    (void) array_add (a, (char *) &i);
		}
	    }
	}
	FREE(tmask.start);
	FREE(tmask.end);
	
	/* 
	 * Look for data spikes using first differences in middle of
	 * array only if there is valid data in the interval 
	 */
	for (i = nwin2; i < npts - nwin2; i++)
	{
	    if (diff[i] > spike_global)
	    {
		istart = i - nwin2;
		iend = istart + nwindow - 1;
			
		/* Find interval mask and offset mask by istart */
		qc_mask_interval(npts, mask, istart, iend, 0, &tmask);

		if(!qc_all_masked(nwindow, &tmask))
		{
		    ndiff = 0;
		    for(j = istart; j <= iend; j++)
		    {
			if(!qc_check_segment (j,j,&tmask,1) && diff[j] > 0.0)
			{
			    d[ndiff++] = diff[j];
			}
		    }
		    find_percentile(perc, d, ndiff, &p);

		    /* If spike is still apparent, flag data */
		    if(!qc_check_segment(i, i, &tmask, 1) && diff[i] > p * thr)
		    {
			array_add (a, (char *) &i);
		    }
		}
		FREE(tmask.start);
		FREE(tmask.end);
	    }
	}

	/* 
	 * Look for data spikes using first differences at end of 
	 * array only if there is valid data in the interval 
	 */
	istart = npts - nwindow;
	iend = npts - 1;
	(void) qc_mask_interval (npts, mask, istart, iend, 0, &tmask);

	if(!qc_all_masked(nwindow, &tmask))
	{
	    ndiff = 0;
	    for(i = istart; i <= iend; i++)
	    {
		if(!qc_check_segment(i, i, &tmask, 1) && diff[i] > 0.0)
		{
		    d[ndiff++] = diff[i];
		}
	    }
	    find_percentile(perc, d, ndiff, &p);
		
	    spike_local = p * thr;
	    for(i = npts - nwin2; i < npts; i++)
	    {
		if(!qc_check_segment (i, i, &tmask, 1) &&
			diff[i] > spike_global && diff[i] > spike_local)
		{
		    array_add(a, (char *) &i);
		}
	    }
	}
	FREE(tmask.start);
	FREE(tmask.end);

	*index = (int *) array_list(a);
	*n_index = array_count(a);

	array_free (a);
	FREE(diff);
	FREE(d);

	return;

}	/* find_ss_spikes */


void
find_mda_spikes(float **data, int *npts, int ndata, QCMask *mask,
		int **index, int *n_index)
{
	Array	a = NULL;	/* Dynamic array */
	QCMask	*m = NULL;

	float	**diff = NULL;	/* Min absolute values of first differences */

	double	thr;
	int	det_stat;
	double	det_val;
	int	det_dset;

	float	*adat = NULL;
	float	*adif = NULL;

	float	*dp = NULL;	/* Pointer to data */
	float	*df = NULL;	/* Pointer to diff */
	double	diff_sts;      	/* Spike threshold stat */
	double	data_sts;      	/* Spike threshold stat */
	double	max_diff; 	/* Maximimum first diff across data vectors */
	double	max_data; 	/* Maximimum value across data vectors */
	int	i, j, n;

	/* Assume some of the data are good */

	/* Assume all masks have same def for mda spike detection */
	thr = mask[0].def.spike_thr;
	det_stat = mask[0].def.spike_stat;
	det_val = mask[0].def.spike_val;
	det_dset = mask[0].def.spike_dset;


	/* Check if we can perform the masking */
	if (det_stat == QC_SPIKE_STAT_PER && ndata < 3)
	{
	    logErrorMsg(LOG_WARNING,
"find_mda_spikes: Not enough data vectors to compute percentile.  No array masking performed.");
	    return;
	}
	else if (det_stat == QC_SPIKE_STAT_AVG && ndata < 2)
	{
	    logErrorMsg(LOG_WARNING,
"find_mda_spikes: Not enough data vectors to compute average.  No array masking performed.");
	    return;
	}

	/* Allocate storage for statistic */
	if(!(adat = (float *)mallocWarn(ndata*sizeof(float))));
	memset((void *)adat, 0, ndata*sizeof(float));
	if(!(adif = (float *)mallocWarn(ndata*sizeof(float))));
	memset((void *)adif, 0, ndata*sizeof(float));

	/* Allocate storage for first difference pointers */
	if(!(diff = (float **)mallocWarn(ndata*sizeof(float *))));
	memset((void *)diff, 0, ndata*sizeof(float *));

	/* 
	 * Compute the maximum statistic across all data vectors for the 
	 * desired data set. If using first differences, store them for 
	 * later use in spike detection.
	 */
	j = 0;
	for (i = 0; i < ndata; i++)
	{
	    index[i] = NULL;
	    n_index[i] = 0;

	    dp = data[i];
	    n = npts[i];
	    m = &mask[i];

	    /* 
	     * If no data or all data is masked, go to next vector
	     * If not enough data to compute stats, go to next vector 
	     */
	    if(!dp || qc_all_masked (n, m) || n < 2) continue;

	    /* Allocate storage for differences */
	    if(!(diff[i] = (float *)mallocWarn(npts[i]*sizeof(float))));
	    memset((char *)diff[i], 0, npts[i]*sizeof(float));
	    df = diff[i];

	    /* 
	     * Compute minimum absolute first differences 
	     * (minimum of backward and forward diffs)
	     * and store value for next iteration.  Determine
	     * maximum first difference across all data vectors.
	     * Determine maximum absolute value across all data 
	     * Use only valid data
	     */
	    compute_valid_stats(dp, n, df, m, &max_data, &max_diff);

	    adat[j] = max_data;
	    adif[j++] = max_diff;
	}

	/* Compute the maximum statistic across all data arrays */
	if(det_stat == QC_SPIKE_STAT_PER)
	{
	    find_percentile(det_val, adat, j, &max_data);
	    find_percentile(det_val, adif, j, &max_diff);
	}
	else if(det_stat == QC_SPIKE_STAT_AVG)
	{
	    compute_avg(adat, j, &max_data);
	    compute_avg(adif, j, &max_diff);
	}
	FREE(adat);
	FREE(adif);
	
	/* 
	 * For each data vector, compare each point with spike threshold 
	 * statistic sts.  Determine if absolute value is above sts.
	 * Save points which are spikes in dynamic array in ind.
	 */
	diff_sts = max_diff * thr;
	data_sts = max_data * thr;
	for(i = 0; i < ndata; i++)
	{
	    a = array_create (sizeof (int));
	    dp = data[i];
	    df = diff[i];
	    m = &mask[i];

	    /* 
	     * Check for spikes if there is data 
	     */
	    if(det_dset == QC_SPIKE_DSET_ALL && dp && df)
	    {
		for (j = 0; j < npts[i]; j++)
		{
		    if ( !qc_check_segment (j,j,m,1) &&
			    ( (fabs ((double) df[j]) > diff_sts) ||
			      (fabs ((double) dp[j]) > data_sts)))
		    {
			array_add (a, (char *) &j);
		    }
		}
	    }
	    else if (det_dset == QC_SPIKE_DSET_1DIFF && df)
	    {
		for (j = 0; j < npts[i]; j++)
		{
		    if ( !qc_check_segment (j,j,m,1) &&
			    (fabs ((double) df[j]) > diff_sts))
		    {
			array_add (a, (char *) &j);
		    }
		}
	    }
	    else if (det_dset == QC_SPIKE_DSET_DATA && dp)
	    {
		for (j = 0; j < npts[i]; j++)
		{
		    if ( !qc_check_segment (j,j,m,1) &&
			    (fabs ((double) dp[j]) > data_sts))
		    {
			array_add (a, (char *) &j);
		    }
		}
	    }
	    index[i] = (int *) array_list (a);
	       n_index[i] = array_count (a);
	    array_free (a);
	}


	/* Done, now free memory */
	for(i = 0; i < ndata; i++)
	{
	    FREE(diff[i]);
	}
	FREE(diff);	

	return;
}	/* find_mda_spikes */

static void
compute_valid_stats(float *data, int npts, float *diff, QCMask *mask,
			double *maxdata, double *maxdiff)
{
	int	*start = NULL;
	int	*end = NULL;
	int	nseg;
	double	mdata;
	double	mdiff;
	int	i, j, n;
	
	if(qc_all_valid (mask))
	{
	    compute_stats(data, npts, diff, maxdata, maxdiff);
	    return;
	}

	start = mask->start;
	end = mask->end;
	nseg = mask->nseg;

	j = 0;
	n = start[0];
	compute_stats(&data[j], n, &diff[j], maxdata, maxdiff);

	for (i = 1; i < nseg; i++)
	{
	    j = end[i - 1] + 1;
	    n = start[i] - j;
	    compute_stats(&data[j],n,&diff[j],&mdata,&mdiff);

	    *maxdiff = MAX(*maxdiff, mdiff);
	    *maxdata = MAX(*maxdata, mdata);		
	}

	j = end[nseg - 1] + 1;
	n = npts - j;
	compute_stats(&data[j],n,&diff[j],&mdata, &mdiff);

	*maxdiff = MAX(*maxdiff, mdiff);
	*maxdata = MAX(*maxdata, mdata);		
	
	return;

}	/* compute_valid_stats */


static void
compute_stats(float *data, int npts, float *df, double *maxdata,
		double *maxdiff)
{
	double 	dfpl1;
	double	dfmin1;
	double	mf;
	double	md;
        double	d;
	int	i;
	
	*maxdiff = 0;
	*maxdata = 0;

	if(npts < 1)
	{
	    return;
	}
	else if (npts < 2)
	{
	    *maxdiff = 0;
	    *maxdata = fabs ((double) data[0]);
	    return;
	}

	/* Compute min abs differences, max difference, max abs data */
	md = fabs((double) data[0]);
	df[0] = dfmin1 = mf = fabs ((double) (data[1] - data[0]));
	for (i = 1; i < npts - 1; i++)
	{
	    dfpl1 = fabs ((double) (data[i] - data[i + 1]));
	    df[i] = MIN (dfmin1, dfpl1);
	    mf = MAX (mf, df[i]);
	    dfmin1 = dfpl1;

	    d = fabs ((double) data[i]);
	    md = MAX (md, d);
	}
	df[npts - 1]= MIN (df[npts - 2],
			   fabs((double)(data[npts-2]-data[npts-1])));

	d = fabs ((double) data[npts-1]);

	*maxdata = MAX (md, d);
	*maxdiff = MAX (mf, df[npts - 1]);
	
	return;
}	/* compute_stats */

	
static
void compute_avg(float *d, int npts, double *avg)
{
	double	a;
	int	i;
	
	*avg = 0.0;

	if(npts < 1) return;
	
	a = 0.0;
	for (i = 0; i < npts; i++)
	{
	    a += fabs ((double) d[i]);
	}
	a /= (double) npts;
	
	*avg = a;

	return;

}
