/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * NAME
 *	merge_segments
 * 
 * FILE 
 *	segments.c
 *
 * SYNOPSIS
 *
 *      void
 *      merge_segments (Is1, Ie1, Icnt1, Is2, Ie2, Icnt2, Ms, Me, Mcnt)
 *      int     *Is1,*Ie1,Icnt1;   (i) Start,end indices and number of
 *                                     segments in first mask
 *      int     *Is2,*Ie2,Icnt2;   (i) Start,end indices and number of
 *                                     segments in second mask
 *      int     **Ms;              (o) Output merged segments start indices
 *      int     **Me;              (o) Output merged segments end indices
 *      int     *Mcnt;             (o) Output number of merged segments
 *
 *	int
 *	in_segments(S, E, n, s, e, slen)
 *	int 	* S;		   (i) Start indices
 *	int 	* E;		   (i) End indices
 *	int 	n;		   (i) Number of indices
 *	int 	s;		   (i) start index of segment to check
 *	int 	e;		   (i) end index of segment to check
 *	int 	slen;		   (i) length of segments to check
 *
 *	static void
 *	find_insertion_index(S, E, n, ins_s, ins_e, i_i)
 *	int 	* S;		   (i) Start indices
 *	int 	* E;		   (i) End indices
 *	int 	n;		   (i) Number of indices
 *	int 	ins_s;		   (i) start indice to be inserted
 *	int 	ins_e;		   (i) end indice to be inserted
 *	int 	* i_i;		   (o) Insertion index/point to
 *				       begin merge/insert.
 *	void
 *	get_segments_from_indices (ind, nind, start, end, nseg)
 *	int	*ind;		(i) List of indices
 *	int	nind;		(i) Number of indices in ind
 *	int	**start;	(o) Start indices of segments
 *	int	**end;		(o) End indices of segments
 *	int	*nseg;		(o) Number of segments
 *
 *	int
 *	fix_segments (data, npts, thresh, ntaper, fix, start, end, nseg)
 *	float	*data;		(i/o) Data sequence
 *	int	npts;		(i) Number of points in data
 *	int	thresh;		(i) Number of consecutive equal values 
 *				    below which data should be fixed if
 *				    fix=1 or fix=-1
 *	int	ntaper;		(i) Number of points to taper before and
 *				    after segments of length >= thresh
 *	int	fix;		(i) Flag to fix data of length < thresh
 *	int	**start;	(i/o) Address of start indices of segments to 
 *				      fix
 *	int	**end;		(i/o) Address of end indices of segments to fix
 *	int	*nseg;		(i/o) Address of number of segments to fix
 *
 * DESCRIPTION
 *
 *      merge_segments() creates merged segments from the two sets
 *      of input segments.  If both input sets are empty or NULL,
 *      nm is zero and NULL pointers are output for sm, em.
 *      If only one input set is not empty or non-NULL, the input
 *      set of segments is copied and returned.  If both inputs sets
 *      are not empty and non-NULL, new segments containing unique
 *      indices are returned.  Input indices in the BOTH segment sets are
 *	ASSUMED to be in ascending order.  The algorithm includes
 *	an O(log n) search for finding the merge/insertion point for
 *	EACH indice pair in the second mask.
 *
 *	in_segments () returns 1 if the data between indices s and e
 *	has any masked indices in segments of length >= slen.  The
 *	function does an O(log n) search on the set of start/end
 *	indices, S and E, given that these array are sorted in
 *	ascending order. 
 *
 *	find_insertion_index () uses a O(log n) search on S and E
 *	array indices to find the insertion index/point, *i_i, to
 *	begin the insertion/merge of ins_s and ins_e, into S and E. 
 *	S and E must be sorted in ascending order.
 *
 *	get_segments_from_indices() creates lists of the start and end
 *	segments defined by the input indices.  If non-NULL input or
 *	no indices on input, returns nseg set to zero.
 *	The start,end arrays must be freed by calling routine.  
 *	
 *	fix_segments() will alter the input data according to the input
 *	parameters using the segment information.  Input data should be
 *	demeaned.  This routine assumes non-NULL data on input.
 *	If fix>0, those segments of length < 2 will be fixed by cubic fit, 
 *	and those of length >= 2 and < thresh will be fixed by interpolation. 
 *	All segments of length >= thresh will be set to zero. 
 *	If fix<=0, all segments will be set to zero.  This option is useful 
 *	for creating a binary mask of those points which are bad.
 *	Returns 0 on success, -1 on error.  
 *
 *	interpolate_segment() performs a linear interpolation on 
 *	a segment of data.
 *
 *
 * NOTES
 *
 * AUTHOR
 *	Darrin Wahl, Lance Al-Rawi
 */

#include "config.h"
#include <stdio.h>
#include <math.h>
#include "dyn_array.h"
#include "libdataqcp.h"
#include "libstring.h"
#include "logErrorMsg.h"


/* Local functions forward declaration */
static void find_insertion_index(int * S, int * E, int n, int ins_s,
				int ins_e, int * i_i); 
static void interpolate_segment(float *data, int npts, int start, int end);


void merge_segments(int *Is1, int *Ie1, int Icnt1, int *Is2, int *Ie2,
			int Icnt2, int **Ms, int **Me, int *Mcnt)
{

  int * Rs;			/* Local pointer to output arrays   */
  int * Re;
  int ii;			/* number of current input indice   */
  int ri;			/* number of current output indice  */
  int i_s, i_e;			/* current input indices 	    */
  int insert_s, insert_e;	/* current indices to insert        */
  int max_size;			/* Max # of merged indices.	    */
  int num_ind, num_bytes;
  int i;
 
  /* Initialize */

  *Mcnt = 0;
  *Ms = NULL;
  *Me = NULL;
  
  /*
   * If no indices in either set of segments, return
   */
  if((Icnt1 + Icnt2 < 1) || ((!Is1 || !Ie1) && (!Is2 || !Ie2))) return;

  /*
   * There must be at least one set of segments.
   * If no indices in one set of segments, copy the other
   * set of segments and return this as the merged segments.
   *
   * NOTE: Both sets of segments is assumed to be in ascending order.
   *
   */

  if((Icnt2 < 1) || !Is2 || !Ie2)
  {
	*Mcnt = Icnt1;

	if(!(Rs = (int *)mallocWarn(Icnt1*sizeof(int)))) return;
	memcpy((void *)Rs, Is1, Icnt1*sizeof(int));
	*Ms = Rs;
 
	if(!(Re = (int *)mallocWarn(Icnt1*sizeof(int)))) return;
	memcpy((void *)Re, Ie1, Icnt1*sizeof(int));
	*Me = Re;
 
	return;
  }
  else if ((Icnt1 < 1) || !Is1 || !Ie1)
  {
	*Mcnt = Icnt2;
 
	if(!(Rs = (int *)mallocWarn(Icnt2*sizeof(int)))) return;
	memcpy((void *)Rs, Is2, Icnt2*sizeof(int));
	  *Ms = Rs;
 
	if(!(Re = (int *)mallocWarn(Icnt2*sizeof(int)))) return;
	memcpy((void *)Re, Ie2, Icnt2*sizeof(int));
	*Me = Re;
 
	return;
  }

  /* Resulting/merged arrays cannot be larger than the size of
     both input arrays. */ 
	
  ri = max_size = Icnt1 + Icnt2;
  if(!(Rs = (int *)mallocWarn(max_size*sizeof(int)))) return;
  *Ms = Rs;
  if(!(Re = (int *)mallocWarn(max_size*sizeof(int)))) return;
  *Me = Re;

  /* Copy first mask into result/merge mask so that merging
     can be done in place. */

  num_bytes = Icnt1 * sizeof(int);
  memcpy(Rs, Is1, num_bytes);
  memcpy(Re, Ie1, num_bytes);


  /* Insert Mask 2 indice pairs into Mask 1, one at a time. */ 

  for(i=0; i < Icnt2; i++)
  {
    insert_s = Is2[i];
    insert_e = Ie2[i];

    /* find insertion point using a O(log N) search instead of O(n) */

    ii = 0;
    find_insertion_index(Rs, Re, Icnt1, insert_s, insert_e, &ii);
    ri = ii;


    /* Merge loop - insert/merge insert indices into Mask 1. 
       Loop until all Mask 1 indices have been searched 
       or moved. */

    while(ii != Icnt1)
    {
      /* current input start/end indices */

      i_s = Rs[ii];
      i_e = Re[ii];

    
      /* if insert start/end range does not overlap or abut REMAINING
         input indices, move remaining mask indices down one and 
	 then place insert indices in the open slot. */
	
      if((insert_e + 1) < i_s)
      {
        num_ind = Icnt1 - ii;
        num_bytes = num_ind * sizeof(int);
        if(num_ind)
        {
          memmove(&Rs[ri+1], &Rs[ii], num_bytes); 
          memmove(&Re[ri+1], &Re[ii], num_bytes);
        }

        Rs[ri] = insert_s;
        Re[ri] = insert_e;
	ri += num_ind + 1;

        break;
      }

    
      /* if insert start/end range does not overlap or abut CURRENT 
         input indices, and is greater than the current interval,
         then move current input indices into current insertion
	 point and increment both array counters. Indices to insert
         do NOT change.

         Otherwise, the input and insert indices OVERLAP, adjust 
         insert indices to encapsulate the merged interval, and 
         increment the input point array counter. */

      if(insert_s > (i_e + 1))
      {
        Rs[ri] = i_s;
        Re[ri] = i_e;
        ri++;
        ii++;
      }
      else
      {
        if(i_e > insert_e)
          insert_e = i_e;

        if(insert_s > i_s)
          insert_s = i_s;

        ii++;
      }
   
      if(ii == Icnt1)
      {
        Rs[ri] = insert_s;
        Re[ri] = insert_e;
        ri++;
      }
    }

    /* reset mask 1 count to count of resulting mask from last merge/insert */

    Icnt1 = ri;
  }

  /* Final mask will be smaller if any interval merging took place, if
     so clip merged mask with realloc */

  if(ri < (max_size))
  {
    if(!(*Ms = (int *)reallocWarn(Rs, ri*sizeof(int)))) return;
    if(!(*Me = (int *)reallocWarn(Re, ri*sizeof(int)))) return;
  }
      
  *Mcnt = ri;

  /* return success */

  return;
}
 
      
int
in_segments(int *S, int *E, int n, int s, int e, int slen)
{
  int ind;

  if(!S || !E) return(0);
    
  if(n)
  {
    if(n == 1)
    {
      if((e < S[0]) || (s > E[0]) || ((E[0] - S[0] + 1) < slen)) return(0);
      else return(1);
    }
      
    ind = n/2;

    if(e < S[ind])
    {
      return(in_segments(S, E, ind, s, e, slen));
    }
    else if(s > E[ind])
    {
      ind++;
      return(in_segments(&S[ind], &E[ind], n-ind, s, e, slen));
    }

    /* [s..e] contains at least one point in the current segment */
 
    if(((E[ind] - S[ind] + 1) >= slen)) return(1);
    else return(0);
  }

  return(0);
}


static void
find_insertion_index(int *S, int *E, int n, int ins_s, int ins_e, int *i_i)
{
  int ind;
  int i_s, i_e;

  if(!S || !E || !i_i) return;
    
  if(n > 1)
  {
    ind = n/2;
    i_s = S[ind];
    i_e = E[ind];

    if(ins_s >= i_s)
    {
      if(ins_s < (i_e + 2))
      {
	*i_i += ind;
        return; 
      }
      else
      {
	/* insertion point is one ahead (i.e. n/2 + 1) unless we
  	   are already at the end. */
 
	if(n == 2)
	{
	     *i_i += 1;
	     return;
	}
	
        *i_i += ind + 1;
        n = n - *i_i;
        find_insertion_index(&S[*i_i], &E[*i_i], n, ins_s, ins_e, i_i);
      }
    }
    else {
      find_insertion_index(S, E, ind, ins_s, ins_e, i_i);
    }
  }
  return;
}


void
get_segments_from_indices(int *ind, int nind, int **start, int **end, int *nseg)
{
	Array	a = NULL;
	Array	b = NULL;
	int	i, j;
	
	*start = NULL;
	*end = NULL;
	*nseg = 0;

	if (nind < 1 || !ind) return;

	/* 
	 * Now determine the combined segments' start and end indices
	 * and number of segments.
	 *
	 * If only one index, it is both start and end index of segment.
	 * If more than one index, compare current index-1 with previous
	 * index. If they are different, it is a new segment. Save the
	 * start and end indices of this segment.  Otherwise, continue
	 * until no other 
	 */
	a = array_create (sizeof (int));
	b = array_create (sizeof (int));
	j = ind[0];
	for (i = 1; i < nind; i++)
	{
	    if (ind[i] - 1 != ind[i - 1])
	    {
		array_add(a, (char *) &j);
		array_add(b, (char *) &ind[i - 1]);
		j = ind[i];
	    }
	}
	array_add (a, (char *) &j);
	array_add (b, (char *) &ind[nind - 1]);

	*nseg = array_count (a);
        *start = (int *) array_list (a);
	*end = (int *) array_list (b);	

	array_free (a);
	array_free (b);

	return;
}



int
fix_segments(float *data, int npts, int thresh, int fix, int *start,
		int *end, int nseg)
{
	int	s;
	int	e;
	int	seglen;
	int	i, j;
	char	*fname = "fix_segments";
	
	for (i = 0; i < nseg; i++)
	{
	    s = start[i];
	    e = end[i];
	    seglen = e - s + 1;

	    /* Do range checking on mask interval indices */

	    if (s < 0 || s > (npts - 1))
	    {
		char error[100];
		snprintf(error, sizeof(error),
		    "%s: Start index %d is invalid. Data range is 0 - %d\n",
		    fname, s, npts - 1);
		logErrorMsg(LOG_WARNING, error);
		return (-1);
	    }
		
	    if (e < 0 || e > (npts - 1))
	    {
		char error[100];
		snprintf(error, sizeof(error),
		    "%s: End index %d is invalid. Data range is 0 - %d\n",
		    fname, e, npts - 1);
		logErrorMsg(LOG_WARNING, error);
		return (-1);
	    }
		
	    /* 
	     * If fix>0 and the segment length is
	     * smaller than the threshold, interpolate
	     * the segment, otherwise fill it with zeroes.
	     */
	    if((fix > 0) && (seglen < thresh))
	    {
		interpolate_segment(data, npts, s, e);
	    }
	    else
	    {
		for(j = 0; j < seglen; j++) data[s+j] = 0.0;
	    }
	}
	return(0);
}


static void
interpolate_segment(float *data, int npts, int start, int end)
{
        float	*d = NULL;
        double	d0, dn, delta;
        int	seglen;
        int	s, e;		
	int	nm1;
	int	interp;		/* Can we interpolate? */
	int	i;        

	/* 
	 * We need one point beyond each end of the masked segment 
	 * to interpolate.  Check if we have them.  If we have none,
	 * zero out segment.  If we only have a point before the 
	 * segment, set the segment value to this value.
	 * Likewise, if we only have a point after the segment,
	 * set the segment value to this value.
	 */
	interp = 0;
	e = end + 1;
	s = start - 1;
	nm1 = npts - 1;
	d0 = 0.0;
	if((e > nm1) && (s < 0))
	{
	    d0 = 0.0;
	}
	else if(e > nm1)
	{
	    d0 = data[s];
	}
	else if(s < 0)
	{
	    d0 = data[e];		
	}
	else
	{
	    interp = 1;
	}

	
	/* If we can't interpolate, fill the segment with a constant */
	if(!interp)
	{
	    for (i = start; i <= end; i++) data[i] = d0;
	}
	else
	{
	    dn = data[e];
	    d0 = data[s];
	    seglen = e - s + 1;
	    delta = (dn - d0) / (double) (seglen - 1);

	    d = &data[s];
	    for (i = 1; i < seglen - 1; i++) d[i] = d0 + delta * i;
        }
	
	return;
}
