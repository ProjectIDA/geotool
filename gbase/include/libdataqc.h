/*
 * Copyright 1994 Science Applications International Corporation.
 *
 * SCCS ID: @(#)libdataqc/libdataqc.h	107.1	05/14/96
 */


#ifndef LIBDATAQC_H
#define LIBDATAQC_H

#ifndef LIBWFM_H
#include "ibase/libwfm.h"	/* For Wfmem struct */
#endif

/* for qc_extended() type */
#define QC_EXTENDED_SS	   	0
#define QC_EXTENDED_ARRAY  	1
#define QC_EXTENDED_ALL		2

/* for def.spike_stat */
#define QC_SPIKE_STAT_AVG	0
#define QC_SPIKE_STAT_PER	1

/* for def.spike_dset */
#define QC_SPIKE_DSET_DATA	0
#define QC_SPIKE_DSET_1DIFF	1
#define QC_SPIKE_DSET_ALL	2

#define QC_ZERO_TOL	1.0e-6


typedef struct qc_def
{
	int		fix;		/* Fix the data ? */
	int		ntaper;		/* Number of point to taper outside
					   masked segments of length >=
					   drop_thr */
	int		drop_thr;	/* Number of consecutive equal-valued
					   samples to call a bad segment */
	double		single_trace_spike_thr;
	                                /* Amplitude value threshold for
					   single point spikes */
	int		niter;		/* Number of iterations to perform
					   extended qc */
	int		nsamp;		/* Number of samples in a time
					   interval for extended qc */
	int		nover;		/* Number of overlap samples 
					   for extended qc */
	double		spike_thr;	/* Amplitude value threshold for
					   extended qc spikes */
	int		spike_stat;	/* Statistic to use for measuring
					   spikes acros multiple data vectors*/
	double		spike_val;	/* Value to use for spike_stat */
	int		spike_npwin;	/* Number of points to focus in on
					   for single vector extended qc
					   spike detection */
	int		spike_dset;	/* Data set to use for extended
					   qc across multiple data vectors */
} QCDef, *QCDefP;


typedef struct qc_mask
{
	int	*start;	/* Start indices of masked segments */
	int	*end;	/* End indices of masked segments */
	int	nseg;	/* Number of masked segments */
	QCDef	def;	/* Mask definition */

} QCMask, *QCMaskP;


/* from qc.c */
extern	int qc_basic(float **data, int *npts, int ndata,
				QCDef *def, QCMask **mask);
extern 	int qc_extended(float **data, int *npts, int ndata,
				   QCDef *def, int type, QCMask **m);
extern int qc_fix_segments(float *data, int npts, QCMask *mask);
extern QCMask *qc_destroy_mask(QCMask *mask, int n);
extern QCMask *qc_copy_mask(QCMask *mask, int n);
extern QCMask *qc_create_empty_mask(int n);
extern int qc_all_masked(int n, QCMask *m);
extern int qc_all_valid(QCMask *m);
extern int qc_mask_interval(int npts, QCMask *mask, 
				      int istart, int iend, int relative,
				      QCMask *imask);
extern void qc_add_mask_offset(int offset, QCMask *mask);
extern void qc_demean(float *data, int npts, QCMask *mask, 
			       double mean);
extern void qc_mean(float *data, int npts, QCMask *mask, 
			     float *mean);
extern void qc_taper_segments(float *data, int npts, 
					QCMask *mask, int zp, int alter);
extern void qc_merge_masks(QCMask *m1, QCMask *m2, QCMask *m);
extern int qc_check_segment(int istart, int iend, QCMask *mask, 
				      int seglen);
extern int qc_check_mask(double tdata, double	tdataend,
				   double samprate, QCMask *mask,
				   double tstart, double  tend, double tlen);
extern int qc_count_mask_points(QCMask *mask);


extern int qc_wfm_basic(Wfmem *wfm, int nwfm, 
				  QCDef *def, QCMask **mask);
extern int qc_wfm_extended(Wfmem *wfm, int nwfm, 
				     QCDef *def, int type, 
				     QCMask **mask);
extern QCMask *qc_wfm_destroy_mask(QCMask *mask, int n);
extern int qc_wfm_check_mask(Wfmem *wfm, QCMask *mask,double tstart,
				       double tend, double tlen);
extern 	int qc_wfm_fix_segments(Wfmem *wfm, QCMask *mask);
extern 	void qc_wfm_mean(Wfmem *wfm, QCMask *mask, float *mean);
extern   void qc_wfm_demean(Wfmem *wfm, QCMask *mask, double mean);
extern 	void qc_wfm_taper_segments(Wfmem *wfm, QCMask *mask, 
					      int zp, int alter);
extern int qc_wfm_mask_interval(double tdata, double	tdataend,
					  double samprate, QCMask *mask,
					  double tstart, double  tend,
					  int relative, QCMask *imask);

#endif /* LIBDATAQC_H */



