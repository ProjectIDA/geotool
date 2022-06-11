/*	SccsId:	%W%	%G%	*/

#ifndef _POLARFILT_H_
#define	_POLARFILT_H_

/* ****** polarfilt.c ********/
int polarfilt(float *sz, float *se, float *sn, int npts, double dt,
                        double tcycles, double fc1, double fc2, double fcycles,
                        int norder, double apert, double rect, double thni,
                        double phi, float *y);


#endif /* _POLARFILT_H_ */
