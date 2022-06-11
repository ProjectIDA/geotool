/*
 * NAME
 * 
 *	running_avg
 *	recursive_avg
 *
 * FILE 
 *
 *	avg.c
 *
 * SYNOPSIS
 *
 *	int
 *	running_avg (data, state, npts, len, thresh, fp, avg, avg_state)
 *	float	*data;		(i) data for running average
 *	int	*state;		(i) state vector for data 
 *	int	npts;		(i) number of points in data,state
 *	int	len;		(i) average window length (samples)
 *	int	thresh;		(i) number of samples in a valid average window
 *	double	(*fp)();	(i) function to apply to data points
 *	float	*avg;		(o) running average vector
 *	int	*avg_state;	(o) state of running average
 *	
 *	int
 *	recursive_avg  (data, state, npts, prev_len, avg_len, thresh, fp, avg,
 *			avg_state)
 *	float	*data;		(i) data for running average
 *	int	*state;		(i) state vector for data 
 *	int	npts;		(i) number of points in data,state
 *	int	prev_len;	(i) recursion lookback length (samples)
 *	int	avg_len;	(i) average window length (samples)
 *	int	thresh;		(i) number of samples in a valid average window
 *	double	(*fp)();	(i) function to apply to data points
 *	float	*avg;		(o) recursive average vector
 *	int	*avg_state;	(o) state of recursive average
 *	
 *	static
 *	int
 *	init_avg  (data, state, npts, avg_len, fp, avg, avg_state)
 *	float	*data;		(i) data for running sums
 *	int	*state;		(i) state vector of data
 *	int	npts;		(i) number of points in data,state
 *	int	avg_len;	(i) length of averaging window (samples)
 *	double	(*fp)();	(i) function to apply to data points
 *	float	*avg;		(o) contains running sum of f(data)
 *	int	*avg_state;	(o) contains running sum of stata
 *	
 * DESCRIPTION
 *
 *	recursive_avg() computes the recursive average of
 *	a data set using state information about the data as
 *	
 *		avg[i] = avg[i-1] + (fp(data[i-M])-avg[i-1]) / N
 *
 *	for  i=[N/2, npts-N/2], N=avg_len, M=prev_len.
 *
 * 	Algorithm:
 *	
 *	1) Initialize running sum of data with function applied as
 *	well as running sum of state using init_avg().
 *	2) Compute the running average of the first half window of data.
 *	If the state of the running sum is greate than or equal to thresh,
 *	the running average is the running sum divided by the running state 
 *	sum, and the running state is set to one.  Otherwise, the running
 *	average and state are set to zero.
 * 	3) Compute the recursive average of the data. 
 * 	For each centered window, if the average needs to be 
 * 	initialized, and the running sum state is above the threshold,
 * 	re-initialize the recursive average as the centered running average 
 * 	and set the average state to one.  If the sum state is not
 * 	above the threshold, set the recursive average and state to zero.
 * 	If the average does not need to be initialized, then if
 * 	the state of the data at the recursion lookback (prev_len) is
 * 	one, compute the recursive average.  If the state at prev_len
 * 	not one, then if the running sum state is above the threshold,
 * 	carry the previous recursive average over.  Otherwise,
 * 	if the running sum state is below the threshold, the recursive
 * 	average needs to be re-initialized.
 *	4) Set the final half window to the final running average value
 *	5) Return -1 on error, 0 otherwise.
 *
 *	running_avg() computes the centered running (or sliding) average of
 *	a data set using state information about the data as 
 *
 *		avg[i] = avg[i-1] + (fp(data[i+N/2])-fp(data[i-N/2])) / N
 *
 *	for  i=[N/2, npts-N/2], N=len.
 *
 *	Using the state information, this becomes 
 *
 *			      1        i+N/2-1
 *		avg[i] = ------------  	sum    fp(data[j])*state[j]
 *			i+N/2-1        j=i-N/2
 *			 sum  state[k]      	
 *                      k=i-N/2
 *	
 * 	Algorithm:
 *	
 *	1) Initialize running sum of data with function applied as
 *	well as running sum of state using init_avg().
 *	2) Compute the running average of the first half window of data.
 *	If the state of the running sum is greate than or equal to thresh,
 *	the running average is the running sum divided by the running state 
 *	sum, and the running state is set to one.  Otherwise, the running
 *	average and state are set to zero.
 *	3) Compute the centered running average of the data in the same 
 *	manner.
 *	4) Set the final half window to the final running average value
 *	5) Return -1 on error, 0 otherwise.
 *	
 *
 *	init_avg() initializes a centered running sum of data and a centered 
 *	running state of the data state. 
 *
 *	Algorithm:
 * 
 *	1) Determine half the averaging window length as an odd number 
 *	of samples .
 *	2) Initialize the first half window of data as the sum of
 *	the first half window points, and likewise for the state sum.
 *	Apply the function to each data point and multiply by state.
 *	3) Compute running sums of the data and the state, all the
 *	while applying the function to the data and multiplying it
 *	by the state value at each point.
 *	4) Initialize the last half window of data as the value of
 *	the last sum of data and state.
 *	5) Return the index of the first centered average window.
 *	
 * DIAGNOSTICS
 *
 * FILES
 *
 * NOTES
 * 
 * SEE ALSO
 *
 * AUTHOR
 *
 *	Darrin Wahl
 *
 */

#include "config.h"
#include "libdetectP.h" 	/* For local protos */

/* Local function forward declaration */
static int init_avg( float *data, int *state, int npts, int avg_len, 
					 double (*fp)(double), float *avg, int *avg_state );


int
running_avg( float *data, int *state, int npts, int len, int thresh, 
			 double (*fp)(double), float *avg, int *avg_state )
{
	double	term;
	int	nm1;
	int	jm1;
	int	i, j, n;

	/* Error checks */
	if (len < 1)
		return (-1);

	/* Initialize running sum vectors in avg, avg_state */
	j = init_avg (data, state, npts, len, fp, avg, avg_state);
	if (j < 0)
		return (-1);

	/* Last valid centered avg */
	n = npts - j + 1;
	nm1 = n - 1;
	jm1 = j - 1;


	/* 
	 * Compute the running average of the first half window of data.
	 * If the state of the running sum is greater than or equal to thresh,
	 * the running average is the running sum divided by the running state 
	 * sum, and the running state is set to one.  Otherwise, the running
	 * average and state are set to zero
	 */
	term = (double) avg_state[jm1];
	if (term >= thresh)
	{
		for (i = 0; i < j; i++)
		{
			avg[i] /= term;
			avg_state[i] = 1;
		}
	}
	else 
	{
		for (i = 0; i < j; i++)
		{
			avg[i] = 0.0;
			avg_state[i] = 0;
		}
	}


	/* Compute the centered running average of the data */
	for (i = j; i < n; i++)
	{
		if (avg_state[i] >= thresh)
		{
			avg[i] /= (double) avg_state[i];
			avg_state[i] = 1;
		}
		else 
		{
			avg[i] = 0.0;
			avg_state[i] = 0;
		}
	}

	/* Set the final half window to the final running average value */
	for (i = n; i < npts; i++)
	{
		avg[i] = avg[nm1];
		avg_state[i] = avg_state[nm1];
	}

	return (0);
}


int
recursive_avg( float *data, int *state, int npts, int prev_len, int avg_len, 
			   int thresh, double (*fp)(double), float *avg, int *avg_state)
{
	double	one_on_len;
	double	term;
	int	im1;
	int	initialize;
	int	imp;
	int	jm1;
	int	i, j;


	/* Error checks */
	if (avg_len <= 0)
		return (-1);
	

	/* Compute inverse avg_len */
	one_on_len = 1.0 / (double) avg_len;
	

	/* Initialize running sum vectors in avg, avg_state */
	j = init_avg (data, state, npts, avg_len, fp, avg, avg_state);
	if (j < 0)
		return (-1);


	/* Last valid centered avg */
	jm1 = j - 1;
	

	/* 
	 * Compute the running average of the first half window of data.
	 * If the state of the running sum is greate than or equal to thresh,
	 * the running average is the running sum divided by the running state 
	 * sum, and the running state is set to one.  Otherwise, the running
	 * average and state are set to zero
	 * If we were able to initialize, set initialize flag to 0.
	 */
	term = (double) avg_state[jm1];
	if (term >= thresh)
	{
		initialize = 0;
		for (i = 0; i < j; i++)
		{
			avg[i] /= term;
			avg_state[i] = 1;
		}
	}
	else 
	{
		initialize = 1;
		for (i = 0; i < j; i++)
		{
			avg[i] = 0.0;
			avg_state[i] = 0;
		}
	}


	/* 
	 * Compute the recursive average of the data. 
	 * For each centered window, if the average needs to be 
	 * initialized, and the running sum state is above the threshold,
	 * re-initialize the recursive average as the centered running average 
	 * and set the average state to one.  If the sum state is not
	 * above the threshold, set the recursive average and state to zero.
	 * If the average does not need to be initialized, then if
	 * the state of the data at the recursion lookback (prev_len) is
	 * one, compute the recursive average.  If the state at prev_len
	 * not one, then if the running sum state is above the threshold,
	 * carry the previous recursive average over.  Otherwise,
	 * if the running sum state is below the threshold, the recursive
	 * average needs to be re-initialized.
	 */
	for (i = j; i < npts; i++)
	{
		im1 = i - 1;
		imp = i - prev_len;
		
		if (initialize)	
		{
			if (avg_state[i] >= thresh)	/* reinit */
			{	
				avg[i] /= (double) avg_state[i];
				avg_state[i] = 1;
				initialize = 0;
			}
			else
			{
				avg[i] = 0.0;
				avg_state[i] = 0;
			}
		}
		else
		{
			if (state[imp])
			{
				term = (*fp)(data[imp]) - avg[im1];
				avg[i] = avg[im1] + (term * one_on_len);
				avg_state[i] = 1;
			}
			else if (avg_state[i] >= thresh)	
			{	
				avg[i] = avg[im1];	/* carry */
				avg_state[i] = 1;
			}
			else
			{
				avg[i] = 0.0;
				avg_state[i] = 0;
				initialize = 1;
			}
		}
	}		

	return (0);
}

int
running_sum( int *data, int npts, int len, int *fdata )
{
	int	isum;
	int	start;
	int	end;
	int	i, j, n;
	
	if (len < 1)
		return (-1);
	

	/* Determine odd number of samples */
	start = end = len / 2;
	if (len % 2 == 0)
	{
		end -= 1;
	}


	/* Initialize */
	isum = 0;
	for (i = 0; i < len; i++)
	{
		isum += data[i];
	}

	j = start + 1;
	n = npts - end;
	for (i = 0; i < j; i++)
	{
		fdata[i] = isum;
	}
	for (i = j; i < n; i++)
	{
		isum += data[i + end] - data[i - j];
		fdata[i] = isum;
	}
	for (i = n; i < npts; i++)
	{
		fdata[i] = fdata[i - 1];
	}


	return (0);
}



static int
init_avg( float *data, int *state, int npts, int avg_len, double (*fp)(double), float *avg, int *avg_state )
{
	double	term;
	double	av;
	int	init_len;
	int	isum;
	int	im1;
	int	imj;
	int	ipe;
	int	start;
	int	end;
	int	ret;
	int	i, j, n;


	/* Determine odd number of samples for centered averages */
	start = end = avg_len / 2;
	if (avg_len % 2 == 0)
	{
		end -= 1;
	}


	/* 
	 * Compute index of first centered average point and number of
	 * points which will have centered averages
	 */
	j = start + 1;
	n = npts - end;


	/* 
	 * Initialize the first half window of data.  Apply the function
	 * and multiply by the data state as each point is summed.
	 * Sum the state values as well.  Store sums in av, isum.
	 */
	if (npts < avg_len)
	{
		init_len = npts;
	}
	else 
	{
		init_len = avg_len;
	}
	av = 0.0;
	isum = 0;
	for (i = 0; i < init_len; i++)
	{
		av += (*fp) (data[i]) * state[i];
		isum += state[i];
	}


	/* 
	 * Set the first half window data values to the sum av and
	 * the first half window data state values to the sum isum.
	 */
	if (npts < avg_len)
	{
		init_len = npts;
	}
	else
	{
		init_len = j;
	}
	for (i = 0; i < init_len; i++)
	{
		avg[i] = av;
		avg_state[i] = isum;
	}
	if (init_len == npts)
	{
		ret = npts;
		goto RETURN;
	}


	/* 
	 * Compute centered running sums for each data point and 
	 * each state point.  Remember to apply function and state
	 * to data points.
	 */
	for (i = j; i < n; i++)
	{
		ipe = i + end;
		imj = i - j;
		im1 = i - 1;
		
		isum += state[ipe] - state[imj];
		avg_state[i] = isum;
		
		term = (*fp) (data[ipe]) * state[ipe] - 
			(*fp) (data[imj]) * state[imj];
		avg[i] = avg[im1] + term;
	}


	/* Set last half window values to last sum value */
	av = avg[n - 1];
	isum = avg_state[n - 1];
	for (i = n; i < npts; i++)
	{
		avg[i] = av;
		avg_state[i] = isum;
	}


	/* Return first centered average point */
	ret = j;
	
 RETURN:

	return (ret);
}

