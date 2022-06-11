/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * Private header file for libdataqc
 *
 * SCCS ID: @(#)libdataqc/libdataqcp.h	107.1	05/14/96
 */


#ifndef LIBDATAQCP_H
#define LIBDATAQCP_H

#include "libdataqc.h"


#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define SQR(a)		((a) * (a))



/* from percentile.c */
extern void find_percentile(double percentile, float *data,
			             int npts, double *value);

/* from points.c */
extern void find_points(float *data, int npts, double thresh,
				  int **start, int **end, int *nseg);


/* from segments.c */
extern int in_segments(int * S, int * E, int n, int s, int e,
				 int slen);

extern void merge_segments(int *s1, int *e1, int n1,
				     int *s2, int *e2, int n2,
				     int **sm, int **em, int *nm);

extern void get_segments_from_indices(int *ind, int nind,
						int **start, int **end,
						int *nseg);

extern int fix_segments(float *data, int npts, int thresh, 
				   int fix, int *start, int *end, int nseg);


/* from sequences.c */
extern void find_sequences(float *data, int npts, int thresh,
				     int **start, int **end, int *nseg);


/* from spike.c */
extern void find_ss_spikes(float *data, int npts, 
				    QCMask *mask, int **index, int *n_index);

extern void find_mda_spikes(float **data, int *npts, int ndata,
				       QCMask *mask,int **index,int *n_index);



#endif /* LIBDATAQCP_H */


