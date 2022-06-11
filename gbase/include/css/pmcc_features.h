/*	SccsId:	%W%	%G%	*/
/**
 *	Pmcc_features relation from CSS 3.0 table definitions.
 */
#ifndef _PMCC_FEATURES_3_0_H
#define _PMCC_FEATURES_3_0_H

#define PMCC_FEATURES30_LEN	147

/** 
 *  Pmcc_features structure.
 *  @member arid Arrival id. Initial value = -1.
 *  @member eng_time Initial value = -9999999999.999.
 *  @member eng_dur Initial value = -999.
 *  @member eng_deldur Initial value = -1.
 *  @member coh_time Initial value = -9999999999.999.
 *  @member coh_dur Initial value = -999.
 *  @member coh_deldur Initial value = -1.
 *  @member coinc_time Initial value = -9999999999.999.
 *  @member coinc_dur Initial value = -999.
 *  @member ninc_deldur Initial value = -1.
 *  @member ford Initial value = 0.
 *  @member zrcr_freq Initial value = -1.
 *  @member zrcr_delfreq Initial value = -1.
 *  @member crnr_freq Initial value = -1.
 *  @member crnr_delfreq Initial value = -1.
 *  @member coh_per Initial value = -999.
 *  @member coh_snr Initial value = -1.
 *  @member total_energy Initial value = -1.
 *  @member auth Initial value = "-".
 *  @member commid Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
        char    sta[7];         /* station                      */
        long	arid;           /* arrival id                   */
        double  initial_time;
        float   duration;
        float   consistency;
        float   correlation;
        long	famsize;
        float   minfreq;
        float   maxfreq;
        float   cfreq;
        float   sigmafreq;
	float	rmsamp;
        char    auth[16];
 	long	commid;
	char	lddate[18];	/* load date			*/
} PMCC_FEATURES30;

/* these need to be corrected

#define PMCC_FEATURES_RCS30 "%8ld%*c%8ld%*c%6c%*c%8c%*c%4f%*c%8f%*c%7f%*c%7f%*c%8f%*c%1c%*c%7f%*c%1c%*c%7f%*c%1c%*c%7f%*c%6f%*c%15c%*c%8ld%*c%17c"

#define PMCC_FEATURES_RVL30(SP) \
&(SP)->arid, &(SP)->orid, (SP)->sta, (SP)->phase, &(SP)->belief, &(SP)->delta,\
&(SP)->seaz, &(SP)->esaz, &(SP)->timeres, (SP)->timedef, &(SP)->azres,\
(SP)->azdef, &(SP)->slores, (SP)->slodef, &(SP)->emares, &(SP)->wgt, \
(SP)->vmodel, &(SP)->commid, (SP)->lddate

*/

#define PMCC_FEATURES_WCS30 "%8ld %17.5lf %7.2f %7.2f %17.5lf %7.2f %7.2f %17.5lf %7.2f %7.2f %4d %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %9.4f %-17.17s %8ld %-17.17s\n"

#define PMCC_FEATURES_WVL30(SP) \
(SP)->arid, (SP)->eng_time, (SP)->eng_dur, (SP)->eng_deldur, \
(SP)->coh_time, (SP)->coh_dur, \
(SP)->coh_deldur, (SP)->coinc_time, (SP)->coinc_dur, (SP)->coninc_deldur, \
(SP)->ford, (SP)->zrcr_freq, (SP)->zrcr_delfreq, (SP)->crnr_freq, \
(SP)->crnr_delfreq, (SP)->coh_per, \
(SP)->coh_snr, (SP)->total_energy, (SP)->auth, \
(SP)->commid,  (SP)->lddate


#define PMCC_FEATURES_NULL30 \
{ \
"-",			/* sta		*/ \
-1,			/* arid		*/ \
-9999999999.999,	/* initial_time	*/ \
-1.0,			/* duration	*/ \
-1.0,			/* consistency	*/ \
-1.0,			/* correlation  */ \
0,			/* famsize	*/ \
-1.0,			/* minfreq	*/ \
-1.0,			/* maxfreq	*/ \
-1.0,			/* cfreq	*/ \
-1.0,			/* sigmafreq	*/ \
-1.0,			/* rmsamp 	*/ \
"-",			/* auth		*/ \
-1,			/* commid	*/ \
"-",			/* lddate	*/ \
}

#endif /* _PMCC_FEATURES_3_0_H */

