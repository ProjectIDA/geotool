/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * NAME
 * 
 *	qc -\ Quality Control of Data 
 *
 * FILE 
 *
 *	qc.c
 *
 * SYNOPSIS
 *
 *	See also:  qc_wfm_<functionality>()
 *
 *	int 
 *	qc_basic (data, npts, ndata, def, mask)
 *	float 	**data;		(i) input data vector(s)
 *	int 	*npts;		(i) number of points in data vector(s)
 *	int 	ndata;		(i) number of data vectors
 *	QCDef	*def;		(i) input mask definition
 *	QCMask 	**mask;		(o) array of mask structure(s) for data
 *
 *	int 
 *	qc_extended (data, npts, ndata, def, type, mask)
 *	float 	**data;		(i) input data vector(s)
 *	int 	*npts;		(i) number of points in data vector(s)
 *	int 	ndata;		(i) number of data vectors
 *	QCDef	*def;		(i) input mask definition
 *	int	type;		(i) type of extended spike detection
 *					QC_EXTENDED_SS=single trace
 *					QC_EXTENDED_ARRAY=array
 *					QC_EXTENDED_ALL=both
 *	QCMask 	**mask;		(o) array of mask structure(s) for data
 *
 *	QCMask *
 *	qc_destroy_mask (mask, n)
 *	QCMask *mask;		(i) Array of masks to destroy
 *	int	n;		(i) Number of masks to destroy
 *
 *	int
 *	qc_all_masked (n, m)
 *	int	n;		(i) Number of data points mask covers
 *	QCMask	*m;		(i) Mask of data
 *	
 *	int
 *	qc_all_valid (m)
 *	QCMask	*m;		(i) Mask of data
 *
 *	int
 *	qc_mask_interval (npts, mask, istart, iend, relative, imask)
 *	int	npts;		(i) Number of points of data which mask covers
 *	QCMask	*mask;		(i) Mask of data
 *	int	istart;		(i) Index of first point in interval
 *	int	iend;		(i) Index of last point in interval
 *	int	relative;	(i) Set indices relative ?
 *	QCMask	*imask;		(i/o) Mask of input interval
 *
 *	void
 *	qc_add_mask_offset (offset, m)
 *	int	offset;		(i) Offset to add to mask
 *	QCMask	*m;		(i) Mask of data
 *
 *	void
 *	qc_merge_masks (m1, m2, m)
 *	QCMask	*m1;		(i) first input mask
 *	QCMask	*m2;		(i) second input mask
 *	QCMask	*m;		(i/o) mask containing merged info from m1,m2
 *
 *	void
 *	qc_mean (data, npts, mask, mean)
 *	float	*data;		(i) data 
 *	int	npts;		(i) number of elements in data
 *	QCMask	*mask;		(i) mask for data
 *      float	*mean;		(o) mean of unmasked data
 *
 *	void
 *	qc_demean (data, npts, mask, mean)
 *	float	*data;		(i/o) data to demean
 *	int	npts;		(i) number of elements in data
 *	QCMask	*mask;		(i) mask for data
 *      double	mean;		(i) mean of unmasked data
 *
 *	int
 *	qc_fix_segments (data, npts, mask)
 *	float 	*data;		(i) data
 *	int 	npts;		(i) number of elements of data
 *	QCMask 	*mask;		(i) mask for data
 *
 *	void
 *	qc_taper_segments (data, npts, mask, zp, alter)
 *	float	*data;		(i) data
 *	int	npts;		(i) number of elements of data
 *	QCMask	*mask;		(i) mask for data
 *	int	zp;		(i) zero phase flag (0=no, 1=yes)
 *	int	alter;		(i) alter mask flag (0=no, 1=yes)
 *
 *	int 
 *	qc_check_mask (tdata, tdataend, samprate, mask, tstart, tend, tlen)
 *	double	tdata;		(i) start time of entire data set (sec)
 *	double	tdataend;	(i) end time of entire data set (sec)
 *	double	samprate;	(i) sampling rate of data (samp/sec)
 *	QCMask 	*mask;       	(i) pointer to mask structure  
 *	double  tstart;      	(i) start time of window of interest (sec)
 *	double  tend;        	(i) end time of window of interest (sec)
 *	double	tlen;		(i) time length of bad segment
 *
 *	int
 *	qc_check_segment (istart, iend, mask, seglen)
 *	int	istart;		(i) start index of segment to check
 *	int	iend;		(i) end index of segment to check
 *	QCMask	*mask;		(i) pointer to mask 
 *	int	seglen;		(i) length of segments to check
 *
 *	int
 *	qc_count_mask_points (mask)
 *	QCMask *mask;		(i) mask to check
 *
 *	QCMask *
 *	qc_copy_mask (mask, n)
 *	QCMask	*mask;		(i) array of mask structures to copy
 *	int	n;		(i) number of mask structures to copy
 *
 *	QCMask *
 *	qc_create_empty_mask (n)
 *	int	n;		(i) number of empty mask structures to create
 *
 * DESCRIPTION
 *
 *	qc_basic() performs basic quality control on the input data.
 *	An array of QCMask structures is created which contains those 
 *	indices of the input data which are considered bad according to 
 *	the criteria in the QCDef structure. Basic quality control consists of 
 *	single point spike detection and gap detection.
 *	If a data vectors is not masked, its mask will be empty, and will need
 *	to freed as well. Returns -1 if there is invalid input, otherwise 0.
 *
 *	qc_extended() performs extended quality control on the input data.
 *	In addition to the operations performed in basic quality control
 *	(see qc_basic() ), extended checking for spikes is done
 *	either on individual data vectors, across all data vectors, or a 
 *	combination of both. Returns -1 if there is invalid input, otherwise 0.
 *
 *	qc_destroy_mask() frees memory associated with an array of
 *	QCMask structures.
 *
 *	qc_all_masked() returns 1 if all n data points are masked (ie
 *	there is one mask segment and it is of length n).  If the
 *	mask is NULL, or some of the data is valid, returns 0.
 *
 *	qc_all_valid() returns 1 if all data points are valid (ie the
 *	mask is empty) or if mask is NULL.
 *
 *	qc_mask_interval() fills a mask describing the state of the
 *	data in the specified interval.  It is assumed that the input
 *	mask is for a data set encompassing the specified 
 *	interval. If relative is set to 0, the mask indices are absolute
 *	with respect to the mask interval.  If relative is not 0, 
 *	the indices are relative to the original data set. The interval 
 *	mask contents must be freed by the calling routine.  
 *	Returns 0 on success, -1 on error.  
 *	
 *	qc_add_mask_offset() adds the offset to the mask indices.
 *
 *	qc_mean() computes the mean of data segments which are not
 *	masked. See qc_demean().
 *
 *	qc_demean() removes the mean value from data segments which
 *	are not masked.  See qc_mean().
 *
 *	qc_taper_segments() applies a cosine taper of length def->ntaper 
 *	points to data on either side of masked segments length >=
 *	def->drop_thr.	The data inside the masked segments is set to zero.
 *	If zp is 0, the taper is only applied to the data after the segment.
 *	If alter is 1, the mask is updated to include those data points 
 *	which were tapered.  No action is taken if there is no data or 
 *	the data is not masked.
 *
 *	qc_fix_segments() linearly interpolates mask segments of 
 *	length < def->drop_thr if def->fix is 1.  Otherwise, all masked
 *	data are set to 0.  This alters the input data.
 *	No action is taken if there is no data or data is not masked.
 *	Returns 0 on success, -1 on error.  
 *	
 *	
 *	qc_check_mask() returns 1 if the data between tstart and tend 
 *	has any masked segments of length tlen or greater.
 *
 *	qc_check_segment () returns 1 if the data between indices istart
 *	and iend has any masked indices in segments of length >= seglen.
 *
 *	qc_merge_masks () merges the masks m1,m2 and returns the results
 *	in mask m.  The contents of the output mask must be freed by
 *	the calling routine.
 *
 *	qc_count_mask_points() returns the number of masked points in the mask.
 *
 *	qc_copy_mask () returns a pointer to an array of n mask structures
 *	which are complete copies of the input array.
 *
 *	qc_create_empty_mask () creates an array of n empty mask structures.
 *
 *	qc_wfm_<name>() is the wfm-structure interface to qc_<name>().
 *
 * DIAGNOSTICS
 *
 * FILES
 *
 * SEE ALSO
 *
 *	libwfm(3), dyn_array(3)
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "libdataqcp.h"
#include "dyn_array.h"
#include "libstring.h"
#include "logErrorMsg.h"
#include "libFT.h"

#define FREE(a) if(a) {free(a); a = NULL;}

#define BETWEEN(a,x,y)  ((a) >= (x) && (a) <= (y))


/* Local functions forward declaration */
static void basic_qc(float *data, int npts, QCMask *mask);
static void extended_qc(float **data, int *npts, int ndata, 
		QCDef *def, int type, QCMask *imask, QCMask *mask);


int 
qc_basic(float **data, int *npts, int ndata, QCDef *def, QCMask **m)
{
	QCMask	*mask = NULL;		
	int	i;

	*m = NULL;

	if(ndata < 1 || data == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_basic: Bad data input.");
	    return -1;
	}
	else if(npts == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_basic: Bad npts input.");
	    return -1;
	}
	else if(def == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_basic: Bad def input.");
	    return -1;
	}

	/* Allocate temporary mask structures */
	if(!(mask = (QCMask *)mallocWarn(ndata*sizeof(QCMask)))) return -1;
	memset((void *) mask, 0, sizeof(QCMask) * ndata);

	/* 
	 * Perform basic quality control on data vectors
	 */
	for(i = 0; i < ndata; i++)
	{
	    /* Copy definition structure into mask */
	    memcpy((void *) &mask[i].def, (void *) def, sizeof(QCDef));

	    /* Don't attempt masking if there is no data */
	    if(!data[i]) continue;
		
	    basic_qc(data[i], npts[i], &mask[i]);
	}
	
	*m = mask;

	return 0;
}

static void
basic_qc(float *data, int npts, QCMask *mask)
{
	int	*s1 = NULL, *e1 = NULL, *s2 = NULL, *e2 = NULL;
	int	*start = NULL, *end = NULL;
	int	n1, n2, nseg;

	mask->start = NULL;
	mask->end = NULL;
	mask->nseg = 0;

	/* Find all bad single point data */
	find_points(data, npts,mask->def.single_trace_spike_thr,&s1,&e1,&n1);

	/* Find all bad dropout and dead sequences of length >= drop_thr */
	find_sequences(data, npts, mask->def.drop_thr, &s2, &e2, &n2);

	/*
	 * Merge bad point data with bad sequences and
	 * put results into mask.
	 */
	merge_segments(s1, e1, n1, s2, e2, n2, &start, &end, &nseg);

	mask->start = start;
	mask->end = end;
	mask->nseg = nseg;

	if(s1) free(s1);
	if(e1) free(e1);
	if(s2) free(s2);
	if(e2) free(e2);
}


int 
qc_extended(float **data, int *npts, int ndata, QCDef *def, int type,
		QCMask **mask)
{
	QCMask	*m1 = NULL;		/* Total mask */
	QCMask	*m2 = NULL;		/* Mask for extended qc */
	QCMask	*m3 = NULL;		/* Temporary mask */
	QCMask	*tmask = NULL;		/* Temporary mask */

	float	*d = NULL;		/* Local pointers */
	float	**dp = NULL;
	int	*np = NULL;
	float	*mean = NULL;

	int	interval;		/* Loop counter for data intervals */
	int	ninterval;		/* Number of data intervals */
	int	start;			/* Start index of an interval */
	int	nm1;			

	int	iter;			/* Loop counter for iterations */

	int	niter;			/* From def */
	int	nsamp;
	int	nover;
	int	ndiff;			/* nsamp - nover */

	int     ret = 0;
	int	i, j;
	char	*fname = "extended_qc";
	char	error[100];

	*mask = NULL;

	if(ndata < 1 || data == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_extended: Bad data input.");
	    return -1;
	}
	else if(npts == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_extended: Bad npts input.");
	    return -1;
	}
	else if(def == NULL) {
	    logErrorMsg(LOG_WARNING, "qc_extended: Bad def input.");
	    return -1;
	}

	/* 
	 * Check for special conditions.
	 *
	 * If we are not just doing single data array masking, check
	 * that all vectors have the same number of elements so we 
	 * can perform multiple data array masking.  If not, and requested 
	 * both multiple and single, perform only single; if user requested
	 * multiple only, return an error.
	 */
	if(type > 2 || type < 0)
	{
	    snprintf(error, sizeof(error), 
		"%s: Bad input type. Performing complete extended qc type==%d",
		fname, QC_EXTENDED_ALL);
	    logErrorMsg(LOG_WARNING, error);
	    type = QC_EXTENDED_ALL;
	}

	if(type != QC_EXTENDED_SS)
	{
	    j = npts[0];
	    for(i = 1; i < ndata; i++)
	    {
		if(npts[i] != j)
		{
		    logErrorMsg(LOG_WARNING,
"qc_extended: All data vectors must have the same number of elements to perform multiple data array masking.");
		    if(type == QC_EXTENDED_ALL)
		    {
			logErrorMsg(LOG_WARNING,
			    "Continuing with single data array masking only.");
			type = QC_EXTENDED_SS;
			break;
		    }
		    else if(type == QC_EXTENDED_ARRAY)
		    {
			logErrorMsg(LOG_WARNING, "No masking done.");
			ret = -1;
			goto RETURN;
		    }
		}
	    }			
	}		

	/* Check to make sure the def is valid */
	nsamp = def->nsamp;
	nover = def->nover;
	niter = def->niter;
	if(nover < def->drop_thr)
	{
	    nover = def->drop_thr;
	    snprintf(error, sizeof(error),
		"%s: Setting nover to drop_thr %d", fname, nover);
	    logErrorMsg(LOG_WARNING, error);
	}
	if(nover >= nsamp)
	{
	    nover = nsamp - 1;
	    snprintf(error, sizeof(error),
		"%s: Setting nover to nsamp-1 %d", fname, nover);
	    logErrorMsg(LOG_WARNING, error);
	}
	if(nsamp > npts[0])
	{
	    nsamp = npts[0];
	    snprintf(error, sizeof(error),
		"%s: Setting nsamp to npts %d\n", fname, nsamp);
	}
	if(type == QC_EXTENDED_SS && niter > 1)
	{
	    niter = 1;
	    logErrorMsg(LOG_WARNING,
		"qc_extended: Setting niter to 1 for SS type");
	}

	/* 
	 * Calculate the number of intervals which will fit
	 * into the data, including the last interval of length <=nsamp.
	 *
	 * ndiff is number of non-overlapping samples in each interval
	 * ninterval is number of intervals (integer division)
	 * nm1 is ninterval - 1
	 */
	ndiff = nsamp - nover;
	ninterval = 0;
	while(nsamp + (ninterval * ndiff) < npts[0])
	{
	    ninterval++;
	}
	ninterval++;
	nm1 = ninterval - 1;

	/* Allocate dp,np for data pointers, npts */
	if(!(dp = (float **)mallocWarn(ndata*sizeof(float *)))) goto RETURN;
	memset((void *)dp, 0, ndata*sizeof(float *));
	if(!(np = (int *)mallocWarn(ndata*sizeof(int)))) goto RETURN;
	memset((void *)np, 0, ndata*sizeof(int));

	/* Allocate vector of means */
	if(!(mean = (float *)mallocWarn(ndata*sizeof(float)))) goto RETURN;
	memset((void *)mean, 0, ndata*sizeof(float));

	/* Allocate masks m1 for output */
	if(!(m1 = (QCMask *)mallocWarn(ndata*sizeof(QCMask)))) goto RETURN;
	memset((void *)m1, 0, ndata*sizeof(QCMask));

	/* Allocate tmasks for mask intervals */
	if(!(tmask = (QCMask *)mallocWarn(ndata*sizeof(QCMask)))) goto RETURN;
	memset((void *)tmask, 0, ndata*sizeof(QCMask));

	/* 
	 * Fill mask def, interval mask def for each vector.
	 * Allocate float interval vector of length nsamp for each vector.
	 * Compute basic mask for vector, store in m1[i].
	 * Compute initial mean for vector, store in mean[i].
	 */
	for(i = 0; i < ndata; i++)
	{
	    /* Copy definition structure into mask */
	    memcpy((void *)&m1[i].def, (void *)def, (size_t)sizeof(QCDef));

	    /* Copy definition structure into mask */
	    memcpy((void *)&tmask[i].def, (void *)def, (size_t)sizeof(QCDef));

	    /* Don't attempt masking if there is no data */
	    if(!data[i]) continue;
		
	    if(!(dp[i] = (float *)mallocWarn(nsamp*sizeof(float)))) goto RETURN;
	    memset((void *)dp[i], 0, nsamp*sizeof(float));

	    basic_qc(data[i], npts[i], &m1[i]);

	    qc_mean(data[i], npts[i], &m1[i], &mean[i]);
	}
	
	/* 
	 * Iterate over extended quality control, re-computing mean
	 * after each iteration.
	 */
	for(iter = 0; iter < niter; iter++)
	{
	    /* 
	     * Loop over time segment intervals, creating a mask
	     * for all interval vectors for each interval.
	     * Merge each interval mask with each vector's overall
	     * mask in m1 on each loop.  Reset overall mask
	     * to result of merge, point back to it in m1.
	     */
	    for(interval = 0; interval < ninterval; interval++)
	    {
		/* Allocate temporary mask structures */
		if(!(m2 = (QCMask *)mallocWarn(ndata*sizeof(QCMask))))
		    goto RETURN;
		memset((void *)m2, 0, ndata*sizeof(QCMask));
		if(!(m3 = (QCMask *)mallocWarn(ndata*sizeof(QCMask))))
		    goto RETURN;
		memset((void *)m3, 0, ndata*sizeof(QCMask));

		/* 
		 * Point to start of data in interval
		 * and set number of points. Copy data in interval.
		 * Get interval mask from overall mask, and
		 * demean copy of data using interval mask and
		 * overall mean[i].
		 */
		start = interval * ndiff;
		for(i = 0; i < ndata; i++)
		{
		    d = data[i];

		    /* Don't attempt masking if there is no data*/
		    if(!d) continue;
				
		    /* Number of points in interval */
		    np[i] = (interval != nm1) ? nsamp : npts[i] - start;

		    /* Copy original data */
		    memcpy((void *)dp[i], (void *)&d[start],
				np[i]*sizeof(float));
		    /* 
		     * Find mask for interval.
		     * Adjust the indices of interval 
		     * mask for start offset.
		     */
		    qc_mask_interval(npts[i], &m1[i], start, start+np[i]-1, 0,
					 &tmask[i]);

		    /* Demean interval using overall mean */
		    qc_demean(dp[i], np[i], &tmask[i], mean[i]);
		}
		/* 
		 * Find new interval mask using
		 * extended qc on interval data 
		 */
		extended_qc(dp, np, ndata, def, type, tmask, m2);
			
		/* Merge interval masks with overall masks */
		for(i = 0; i < ndata; i++)
		{
		    /* Clean up this tmask */
		    FREE(tmask[i].start);
		    FREE(tmask[i].end);

		    /* 
		     * Adjust the indices of new interval 
		     * mask for start offset
		     */
		    qc_add_mask_offset(start, &m2[i]);

		    /* Merge m1,m2 into mask m3 */
		    qc_merge_masks(&m1[i], &m2[i], &m3[i]);
		}

		/* Destroy m1,m2 and reset m1 as m3 */
		m1 = qc_destroy_mask(m1, ndata);
		m2 = qc_destroy_mask(m2, ndata);
		m1 = m3;
	    } /* end of loop over interval */

	    /* 
	     * The mask m1 now contains the entire mask from 
	     * all intervals 
	     */

	    /* Re-calculate mean here using new m1 */
	    for(i = 0; i < ndata; i++)
	    {
		if(data[i]) qc_mean(data[i], npts[i], &m1[i], &mean[i]);
	    }
	} /* end of loop over iter */

	ret = 0;

 RETURN:
	if(ret < 0) {
	    qc_destroy_mask(m1, ndata);
	    *mask = (QCMask *) NULL;
	}
	else {
	    *mask = m1;
	}
	if(dp) {
	    for(i = 0; i < ndata; i++) FREE(dp[i]);
	}
	FREE(dp);
	FREE(np);
	FREE(mean);
	qc_destroy_mask(tmask, ndata);

	return (ret);
}	/* qc_extended */


static void
extended_qc(float **data, int *npts, int ndata, QCDef *def, int type,
		QCMask *imask, QCMask *mask)
{
	int	*start = NULL, *end = NULL;
	int	*s1 = NULL, *e1 = NULL, *s2 = NULL, *e2 = NULL;
	int	n1 = 0, n2 = 0, nseg = 0, all_masked = 0;

	int	**ss_ind = NULL;	/* ...for single data arrays... */
	int	*ss_nind = NULL;
	int	**mda_ind = NULL;	/* ...for multiple data arrays... */
	int	*mda_nind = NULL;
	
	float	*d = NULL;		
	int	i, n;

	if(type != QC_EXTENDED_ARRAY)
	{
	    /* Allocate memory for ss spike indices */
	    if(!(ss_ind = (int **)mallocWarn(ndata*sizeof(int *)))) goto RETURN;
	    memset((void *)ss_ind, 0, ndata*sizeof(int *));
	    if(!(ss_nind = (int *)mallocWarn(ndata*sizeof(int)))) goto RETURN;
	    memset((void *)ss_nind, 0, ndata*sizeof(int));
	}

	/* 
	 * Copy def structure into output mask.
	 * If all data is masked for a data vector, continue.
	 * Locate spike segments in each data array if there 
	 * is data. Store the segments in the single spike arrays.
	 */
	for(i = 0; i < ndata; i++)
	{
	    d = data[i];
	    n = npts[i];

	    /* Copy definition structure into mask */
	    memcpy((void *)&mask[i].def, (void *)def, sizeof(QCDef));

	    /* Don't attempt to mask if there is no good data */
	    if(qc_all_masked (n, &imask[i]))
	    {
		all_masked++;
		continue;
	    }
	    /* 
	     * Locate single data vector spikes if desired.
	     * Save the indices for merging with multiple data 
	     * array spike indices later. The ss_ind are allocated.
	     */
	    if(d && type != QC_EXTENDED_ARRAY)
	    {
		find_ss_spikes(d, n,&imask[i],&ss_ind[i],&ss_nind[i]);
	    }
	}
	
	/* 
	 * Only do array masking if some data vectors are not completely
	 * masked on input
	 */
	if(all_masked == ndata) goto RETURN;

	/* 
	 * Locate spikes across multiple data arrays if desired.
	 * Allocate memory for mda spike indices.
	 * Save the indices for merging with single data 
	 * arrays later. The mda_ind are allocated.
	 * Assumes all masks have same def as first mask.
	 */
	if(type != QC_EXTENDED_SS)
	{
	    if(!(mda_ind = (int **)mallocWarn(ndata*sizeof(int*)))) goto RETURN;
	    memset((void *)mda_ind, 0, ndata*sizeof(int *));
	    if(!(mda_nind = (int *)mallocWarn(ndata*sizeof(int)))) goto RETURN;
	    memset((void *)mda_nind, 0, ndata*sizeof (int));

	    find_mda_spikes(data, npts, ndata, imask, mda_ind, mda_nind);
	}

	/* Set up spike segments */
	for(i = 0; i < ndata; i++)
	{
	    mask[i].start = NULL;
	    mask[i].end = NULL;
	    mask[i].nseg = 0;

	    if(data[i])
	    {
		if(type == QC_EXTENDED_ALL)
		{
		    get_segments_from_indices(ss_ind[i], ss_nind[i],
						  &s1, &e1, &n1);

		    get_segments_from_indices(mda_ind[i], mda_nind[i],
						  &s2, &e2, &n2);

		    merge_segments(s1, e1, n1, s2, e2, n2, &start, &end, &nseg);
				
		    FREE(s1);
		    FREE(e1);
		    FREE(s2);
		    FREE(e2);
		}
		else if(type == QC_EXTENDED_ARRAY)
		{
		    get_segments_from_indices(mda_ind[i], mda_nind[i], 
						   &start, &end, &nseg);
		}
		else if(type == QC_EXTENDED_SS)
		{
		    get_segments_from_indices(ss_ind[i], ss_nind[i], 
						&start, &end, &nseg);
		}

		mask[i].start = start;
		mask[i].end = end;
		mask[i].nseg = nseg;
	    }
	}

 RETURN:

	if(ss_ind) for(i = 0; i < ndata; i++) FREE(ss_ind[i]);
	if(mda_ind) for(i = 0; i < ndata; i++) FREE(mda_ind[i]);
	FREE(ss_ind);
	FREE(ss_nind);
	FREE(mda_ind);
	FREE(mda_nind);

	return;
}	/* extended_qc */

int
qc_fix_segments(float *data, int npts, QCMask *mask)
{
	if(!data || npts < 1 || qc_all_valid(mask)) return (0);
	
	/* Fix sequences if desired */
	return(fix_segments(data, npts, mask->def.drop_thr, mask->def.fix, 
		      mask->start, mask->end, mask->nseg));
}


QCMask *
qc_destroy_mask(QCMask *m, int n)
{
	int	i;

	if(m)
	{
	    for(i = 0; i < n; i++) {
		FREE(m[i].start);
		FREE(m[i].end);	
	    }
	    UFREE(m);
	}
	return(m);
}


int
qc_all_masked(int n, QCMask *m)
{
	if(m && ((m->nseg == 1) && (m->end - m->start + 1) == n)) return (1);

	return (0);
}

int
qc_all_valid(QCMask *m)
{
	if(!m || m->nseg < 1) return (1);

	return (0);
}	

QCMask *
qc_copy_mask(QCMask *mask, int n)
{
	QCMask	*m = NULL;
	int	i;
	
	if(mask && n > 0)
	{
	    if(!(m = (QCMask *)mallocWarn(n*sizeof(QCMask)))) return NULL;

	    for(i = 0; i < n; i++)
	    {
		memset((void *)&m[i], 0, sizeof(QCMask));
		memcpy((void *)&m[i].def, (void *) &mask[i].def, sizeof(QCDef));
		if(!qc_all_valid(&mask[i]))
		{
		    m[i].nseg = mask[i].nseg;

	    	    if(!(m[i].start = (int *)mallocWarn(m[i].nseg*sizeof(int))))
			 return NULL;
		    memcpy((void *)m[i].start, (void *)mask[i].start, 
					m[i].nseg*sizeof(int));

	    	    if(!(m[i].end = (int *)mallocWarn(m[i].nseg*sizeof(int))))
			 return NULL;
		    memcpy((void *)m[i].end, (void *) mask[i].end, 
					m[i].nseg*sizeof(int));
		}
	    }				
	}
	return (m);
}

QCMask *
qc_create_empty_mask(int n)
{
	QCMask	*m = NULL;
	
	if(n > 0)
	{
	    if(!(m = (QCMask *)mallocWarn(n*sizeof(QCMask)))) return NULL;
	    memset((void *)m, 0, n * sizeof(QCMask));
	}
	return (m);
}

int
qc_mask_interval(int npts, QCMask *mask, int istart, int iend, int relative,
			QCMask *imask)
{
	Array	s = NULL;
	Array	e = NULL;
	int	*i1 = NULL;
	int	*i2 = NULL;
	int	i;

	/* Initialize imask */
	imask->start = NULL;
	imask->end = NULL;
	imask->nseg = 0;
	memcpy((void *) &imask->def, (void *) &mask->def, sizeof (QCDef));

	/* If no data masked, return. If invalid segment, return error */
	if(qc_all_valid(mask))
	{
	    return (0);
	}
	else if(iend < istart)
	{
	    return(-1);
	}

	istart = MAX(istart, 0);
	istart = MIN(istart, npts - 1);
	iend   = MAX(iend, 0);
	iend   = MIN(iend, npts - 1);

	/* Dynamic arrays for new interval mask */
	s = array_create(sizeof(int));
	e = array_create(sizeof(int));	

	/* Find masked points within window of interest */
	i1 = mask->start;
	i2 = mask->end;   
	for(i = 0; i < mask->nseg; i++)
	{
	    if(BETWEEN(istart, i1[i], i2[i]))
	    {
		array_add(s, (char *) &istart);
			
		if(BETWEEN(iend, i1[i], i2[i]))
		{
		    array_add(e, (char *) &iend);
		}
		else
		{
		    array_add(e, (char *) &i2[i]);
		}
	    }
	    else if BETWEEN(i1[i], istart, iend)
	    {
		array_add(s, (char *) &i1[i]);

		if(BETWEEN(i2[i], istart, iend))
		{
		    array_add(e, (char *) &i2[i]);
		}				
		else
		{
		    array_add(e, (char *) &iend);
		}				
	    }
	}     

	/* Set output */
	imask->nseg = array_count(s);
	imask->start = (int *) array_list(s);
	imask->end = (int *) array_list(e);	
	
	/* Clean up */
	array_free(s);
	array_free(e);

	/* Add offset to interval mask if desired */
	if(!relative)
	{
	    qc_add_mask_offset(-istart, imask);
	}
	
	return (0);
}	

void
qc_add_mask_offset(int offset, QCMask *m)
{
	int 	i;
	
	if(qc_all_valid(m)) return;
	
	/* Add offset to interval mask */
	for(i = 0; i < m->nseg; i++)
	{
	    m->start[i] += offset;
	    m->end[i] += offset;
	}
	return;
}	

void
qc_merge_masks(QCMask *m1, QCMask *m2, QCMask *m)
{
	QCMask	*tm = NULL;
	QCDef	def;
	int	*start = NULL;
	int	*end = NULL;
	int	nseg = 0;

	memset((void *)&def, 0, sizeof(QCDef));
	
	/* 
	 * If nothing masked in m1 or m2, return.
	 * Otherwise, if only one mask has masked data,	
	 * copy that one and return.
	 */
	if(qc_all_valid(m1) && qc_all_valid(m2))
	{
	    memcpy((void *) &def, (void *) &m1->def, sizeof(QCDef));
	}
	else if(qc_all_valid(m2))
	{
	    tm = qc_copy_mask(m1, 1);

	    start = tm->start;
	    end = tm->end;
	    nseg = tm->nseg;

	    memcpy((void *) &def, (void *) &m1->def, sizeof(QCDef));
	    FREE(tm);
	}
	else if(qc_all_valid(m1))
	{
	    tm = qc_copy_mask(m2, 1);

	    start = tm->start;
	    end = tm->end;
	    nseg = tm->nseg;

	    memcpy((void *) &def, (void *) &m2->def, sizeof(QCDef));
	    FREE(tm);
	}
	else 
	{
	    /* Merge masked segments from two masks */
	    merge_segments(m1->start, m1->end, m1->nseg, m2->start, m2->end,
				m2->nseg, &start, &end, &nseg);

	    memcpy((void *) &def, (void *) &m2->def, sizeof(QCDef));
	}
	
	/* Set up output */
	memcpy((void *) &m->def, (void *) &def, sizeof(QCDef));
	m->start = start;
	m->end = end;
	m->nseg = nseg;

	return;
}	/* qc_merge_masks */


void
qc_demean(float *data, int npts, QCMask *mask, double mean)
{
	int	*start;
	int	*end;
	int	nseg;
	int	i, j, k, n;
	
	/* 
	 * If no data or all data masked, return.
	 * If no data masked, compute mean directly 
	 */
	if(!data || qc_all_masked(npts, mask))
	{
	    return;
	}
	else if(qc_all_valid(mask))
	{
	    for(i = 0; i < npts; i++) data[i] -= mean;
	    return;
	}

	/* 
	 * Subtract mean from data segments which are not masked 
	 */
	start = mask->start;
	end = mask->end;
	nseg = mask->nseg;
	j = 0;
	n = start[0];
	for(k = 0; k < n; k++) data[j+k] -= mean;

	for(i = 1; i < nseg; i++)
	{
	    j = end[i - 1] + 1;
	    n = start[i] - j;
	    for(k = 0; k < n; k++) data[j+k] -= mean;
	}

	j = end[nseg - 1] + 1;
	n = npts - j;
	for(k = 0; k < n; k++) data[j+k] -= mean;

	return;
}	/* qc_demean */

void
qc_mean(float *data, int npts, QCMask *mask, float *m)
{
	int	*start;
	int	*end;
	int	nseg;
	double	mean = 0.0;
	double	sum = 0.0;
	int	cnt = 0;
	int	i, j, k, n;

	*m = 0.0;

	/* 
	 * If no data or all data masked, return.
	 * If no data masked, compute mean directly 
	 */
	if(!data || qc_all_masked(npts, mask))
	{
	    return;
	}
	else if(qc_all_valid(mask))
	{
	    mean = 0.;
	    for(k = 0; k < npts; k++) mean += data[k];
	    mean /= (double)npts;
	    *m = mean;
	    return;
	}
	/* 
	 * Compute the sum of each data segment which is not masked 
	 * and keep track of the count.
	 */
	start = mask->start;
	end = mask->end;
	nseg = mask->nseg;
	j = 0;
	n = start[0];
	sum = 0.;
	for(k = 0; k < n; k++) sum += data[j+k];
	mean += sum;
	cnt += n;
	for(i = 1; i < nseg; i++)
	{
	    j = end[i - 1] + 1;
	    n = start[i] - j;
	    sum = 0.;
	    for(k = 0; k < n; k++) sum += data[j+k];
	    mean += sum;
	    cnt += n;
	}

	j = end[nseg - 1] + 1;
	n = npts - j;
	sum = 0.;
	for(k = 0; k < n; k++) sum += data[j+k];
	mean += sum;
	cnt += n;

	mean /= (double) cnt;
	
	*m = mean;
	
	return;
}	/* qc_mean */


void
qc_taper_segments(float *data, int npts, QCMask *mask, int zp, int alter)
{
	QCMask	*m = NULL;
	QCMask	*new_mask = NULL;
	Array	s = NULL;
	Array	e = NULL;
	int	start;
	int	end;
	int	ntaper;
	int	seglen;
	int	i, j, k, n;
	
	if(!data || qc_all_valid(mask)) return;

	/* 
	 * Taper outside segments of length >= dropout threshold.
	 * Keep track of tapered segments for later inclusion in new mask
	 * if altering the mask is desired.
	 */
	if(alter)
	{
	    s = array_create(sizeof(int));
	    e = array_create(sizeof(int));	
	}
	ntaper = mask->def.ntaper;
	for(i = 0; i < mask->nseg; i++)
	{
	    start = mask->start[i];
	    end = mask->end[i];		

	    seglen = end - start + 1;
		
	    if(seglen < mask->def.drop_thr) continue;
		
	    /* Zero out the segment */
	    for(k = 0; k < seglen; k++) data[start+k] = 0.0;

	    /* Taper after segment */
	    j = end + 1;
	    n = ntaper;
	    if(j + n > npts)
	    {
		n = npts - j;
	    }
	    Taper_cosine(&data[j], n, 1.0, 0.0);


	    /* Add tapered segment to start, end arrays if desired */
	    if((n > 0) && alter)
	    {
		array_add(s, (char *) &j);
		j += n - 1;
		array_add(e, (char *) &j);
	    }

	    /* Don't taper before segment if not zp */
	    if(!zp) continue;
		
	    /* Taper before segment */
	    j = start - ntaper;
	    n = ntaper;
	    if(j < 0)
	    {
		j = 0;
		n = start;
	    }
	    Taper_cosine(&data[j], n, 0.0, 1.0);

	    /* Add tapered segment to start, end arrays if desired */
	    if((n > 0) && alter)
	    {
		array_add(s, (char *) &j);
		j += n - 1;
		array_add(e, (char *) &j);
	    }
	}

	/* If altering mask is not desired, we are done */
	if(!alter) 
	{
	    return;
	}

	/* Create new mask to include tapered segments */
	m = qc_create_empty_mask(1);
	m->nseg = array_count(s);
	m->start = (int *) array_list(s);
	m->end = (int *) array_list(e);	
	new_mask = qc_create_empty_mask(1);

	/* Merge masks */
	qc_merge_masks(mask, m, new_mask);

	/* Destroy old mask contents */
	FREE(mask->start);
	FREE(mask->end);

	/* The def is the same, so only reset the mask segments */
	mask->nseg = new_mask->nseg;
	mask->start = new_mask->start;
	mask->end = new_mask->end;

	/* Don't destroy contents of new mask, just structure */
	FREE(new_mask);

	/* Destroy local memory if allocated */
	qc_destroy_mask(m, 1);
	array_free(s);
	array_free(e);	

	return;
}

int 
qc_check_mask(double tdata, double tdataend, double samprate, QCMask *mask,
		double tstart, double tend, double tlen)
	/* tdata	start time of entire data set */
	/* tdataend	end time of entire data set */
	/* samprate	sampling rate of data */
	/* mask       	pointer to mask structure  */
	/* tstart      	start time of window of interest */
	/* tend        	end time of window of interest */
	/* tlen		time length of bad segments */
{
   int 	nsamp;  	/* number of samples in time interval */
   int 	istart;		/* index of start time of window of interest */
   int 	iend;		/* index of end time of window of interest */
   int	seglen;		/* num samps in bad segment */
   

   /* Determine end points of data interval within entire data set */
   istart = (int) rint((tstart - tdata) * samprate);
   iend   = (int) rint((tend   - tdata) * samprate);
   nsamp = (int) rint((tdataend - tdata) * samprate) + 1;
   seglen = (int) rint(tlen * samprate) + 1;

   istart = MAX(istart, 0);
   istart = MIN(istart, nsamp);
   iend   = MAX(iend, 0);
   iend   = MIN(iend, nsamp);

   return(qc_check_segment(istart, iend, mask, seglen));
   
}

int
qc_check_segment(int istart, int iend, QCMask *mask, int seglen)
{

	int	*i1 = NULL;
	int	*i2 = NULL;
	int     datalen;
	int	ilen;
	int	i;

	if(qc_all_valid(mask)) return (0);

	/* data interval length */

	datalen = iend - istart + 1;

	/* length must be at least size of mask segment check length */

	if(datalen < seglen) return(0);

	i1 = mask->start;
	i2 = mask->end;   

	/*
	 * check all mask segments, clipping segment lengths that
	 *  extend beyond the data interval.
	 */

	for(i = 0; i < mask->nseg; i++)
	{
	     if(BETWEEN(istart, i1[i], i2[i]))
	     {
		  if(i2[i] > iend)
		       ilen = datalen;
		  else
		       ilen = i2[i] - istart + 1;

		  if(ilen >= seglen)
		       return(1);
	     }
	     else if(BETWEEN(i1[i], istart, iend))
	     {
		  if(i2[i] > iend)
		       ilen = iend - i1[i] + 1;
		  else
		       ilen = i2[i] - i1[i] + 1;

		  if(ilen >= seglen)
		       return(1);
	     }
	}     
	return (0);

}	/* qc_check_segment */


int
qc_count_mask_points(QCMask *mask)
{
	int	i, n;
	
	if(qc_all_valid(mask)) return (0);

	n = 0;
	for(i = 0; i < mask->nseg; i++)
	{
	    n += (mask->end[i] - mask->start[i]) + 1;
	}

	return (n);
}

#ifdef HAVE_WFMEM_SUPPORT

int 
qc_wfm_basic (wfm, nwfm, def, mask)
Wfmem	*wfm;
int	nwfm;
QCDef	*def;
QCMask 	**mask;
{
	float	**data = NULL;
	int	*npts = NULL;
	int	ndata;
	int	i;
	int	ret;
	char	*fname = "qc_wfm_basic";
	
	
	MALLOC_CHECK (data = UALLOC (float *, nwfm));
	MALLOC_CHECK (npts = UALLOC (int, nwfm));
	ndata = nwfm;
	for (i = 0; i < nwfm; i++)
	{
		data[i] = (float *) wfm[i].data;
		npts[i] = wfm[i].nsamp;
	}		
	
	ret = qc_basic (data, npts, ndata, def, mask);

	UFREE (data);
	UFREE (npts);
	
	return (ret);

}	/* qc_wfm_basic */


int 
qc_wfm_extended (wfm, nwfm, def, type, mask)
Wfmem	*wfm;
int	nwfm;
QCDef	*def;
int	type;
QCMask 	**mask;
{
	float	**data = NULL;
	int	*npts = NULL;
	int	ndata;
	int	i;
	int	ret;
	char	*fname = "qc_wfm_extended";
      
	
	MALLOC_CHECK (data = UALLOC (float *, nwfm));
	MALLOC_CHECK (npts = UALLOC (int, nwfm));
	ndata = nwfm;
	for (i = 0; i < nwfm; i++)
	{
		data[i] = (float *) wfm[i].data;
		npts[i] = wfm[i].nsamp;
	}		
	
	ret = qc_extended (data, npts, ndata, def, type, mask);


	UFREE (data);
	UFREE (npts);
	
	return (ret);

}	/* qc_wfm_extended */


QCMask *
qc_wfm_destroy_mask (mask, n)
QCMask *mask;
int	n;
{
	return (qc_destroy_mask (mask, n));

}	/* qc_wfm_destroy_mask */


int
qc_wfm_check_mask (wfm, mask, tstart, tend, tlen)
Wfmem   *wfm;
QCMask 	*mask;   /* pointer to mask structure  */
double  tstart;  /* index of time window start */
double  tend;    /* index of time window end */
double	tlen;	 /* length of bad segment */      
{
	return (qc_check_mask (wfm->time, wfm->endtime, wfm->samprate,
			       mask, tstart, tend, tlen));

}	/* qc_wfm_check_mask */


int
qc_wfm_fix_segments (wfm, mask)
Wfmem	*wfm;
QCMask 	*mask;
{
	return(qc_fix_segments ((float *) wfm->data, wfm->nsamp, mask));

}	/* qc_wfm_fix_segments */


void
qc_wfm_mean (wfm, mask, mean)
Wfmem	*wfm;
QCMask 	*mask;
float	*mean;
{
	qc_mean ((float *) wfm->data, wfm->nsamp, mask, mean);

}	/* qc_wfm_mean */

void
qc_wfm_demean (wfm, mask, mean)
Wfmem	*wfm;
QCMask 	*mask;
double 	mean;
{
	qc_demean ((float *) wfm->data, wfm->nsamp, mask, mean);

}	/* qc_wfm_demean */

void
qc_wfm_taper_segments (wfm, mask, zp, alter)
Wfmem	*wfm;
QCMask 	*mask;
int	zp;
int	alter;
{
	qc_taper_segments ((float *) wfm->data, wfm->nsamp, mask, zp, alter);

}	/* qc_wfm_taper_segments */


int 
qc_wfm_mask_interval (tdata, tdataend, samprate, mask, tstart, tend, 
		      relative, imask)
double	tdata;		/* start time of entire data set */
double	tdataend;	/* end time of entire data set */
double	samprate;	/* sampling rate of data */
QCMask 	*mask;       	/* pointer to mask structure  */
double  tstart;      	/* start time of window of interest */
double  tend;        	/* end time of window of interest */
int	relative;	/* set mask relative ? */
QCMask	*imask;		/* interval mask */
{
   int 	nsamp;   	/* number of samples in time interval */
   int 	istart;		/* index of start time of window of interest */
   int 	iend;		/* index of end time of window of interest */
   

   /* Determine end points of data interval within entire data set */
   istart = (int) rint ((tstart - tdata) * samprate);
   iend   = (int) rint ((tend   - tdata) * samprate);
   nsamp = (int) rint ((tdataend - tdata) * samprate) + 1;

   return (qc_mask_interval (nsamp, mask, istart, iend, relative, imask));
   
}	/* qc_wfm_interval_mask */

#endif
