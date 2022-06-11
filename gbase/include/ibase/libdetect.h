/*
 *
 *	Header file for libdetect
 *      (only those functions that are actually used by external users)
 */


#ifndef LIBDETECT_H
#define LIBDETECT_H


/* from avg.c */
int running_avg( float *data, int *state, int npts, 
		int len, int thresh, double (*fp)(double),
		float *avg, int *avg_state );




#endif /* LIBDETECT_H */

