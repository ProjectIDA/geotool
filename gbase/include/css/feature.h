/*	SccsId:	%W%	%G%	*/
/**
 *	Feature relation from CSS 3.0 table definitions.
 */
#ifndef _FEATURE_3_0_H
#define _FEATURE_3_0_H

#define FEATURE30_LEN	256

/** 
 *  Feature structure.
 *  @member arid Arrival id. Initial value = -1.
 *  @member peak_time Initial value = -1.
 *  @member peak_level Initial value = -1.
 *  @member total_energy Initial value = -1.
 *  @member mean_arrival_time Initial value = -1.
 *  @member time_spread Initial value = -1.
 *  @member onset_time Initial value = -1.
 *  @member termination_time Initial value = -1.
 *  @member total_time Initial value = -1.
 *  @member num_cross Initial value = -1.
 *  @member ave_noise Initial value = -1.
 *  @member skewness Initial value = -1.
 *  @member kurtosis Initial value = -1.
 *  @member cep_var_signal Initial value = -1.
 *  @member cep_delay_time_signal Initial value = -1.
 *  @member cep_peak_std_signal Initial value = -1.
 *  @member cep_var_trend Initial value = -1.
 *  @member cep_delay_time_trend Initial value = -1.
 *  @member cep_peak_std_trend Initial value = -1.
 *  @member low_cut Initial value = -1.
 *  @member high_cut Initial value = -1.
 *  @member ford Initial value = -1.
 *  @member lddate Load date. Initial value = "-".
 */
typedef struct
{
	long	arid;		/* arrival id */
 	double	peak_time;
 	double	peak_level;
 	double	total_energy;
 	double	mean_arrival_time;
 	double	time_spread;
 	double	onset_time;
 	double	termination_time;
 	double	total_time;
 	int	num_cross;
 	double	ave_noise;
 	double	skewness;
 	double	kurtosis;
 	double	cep_var_signal;
 	double	cep_delay_time_signal;
 	double	cep_peak_std_signal;
 	double	cep_var_trend;
 	double	cep_delay_time_trend;
 	double	cep_peak_std_trend;
 	double	low_cut;
 	double	high_cut;
 	int	ford;
	char	lddate[18];	/* load date */
} FEATURE30;

/* these need to be corrected

#define FEATURE_RCS30 "%8ld%*c%8ld%*c%6c%*c%8c%*c%4f%*c%8f%*c%7f%*c%7f%*c%8f%*c%1c%*c%7f%*c%1c%*c%7f%*c%1c%*c%7f%*c%6f%*c%15c%*c%8ld%*c%17c"

#define FEATURE_RVL30(SP) \
&(SP)->arid, &(SP)->orid, (SP)->sta, (SP)->phase, &(SP)->belief, &(SP)->delta,\
&(SP)->seaz, &(SP)->esaz, &(SP)->timeres, (SP)->timedef, &(SP)->azres,\
(SP)->azdef, &(SP)->slores, (SP)->slodef, &(SP)->emares, &(SP)->wgt, \
(SP)->vmodel, &(SP)->commid, (SP)->lddate

*/

#define FEATURE_WCS30 "%8ld %17.5lf %7.3lf %7.3lf %17.5lf %7.3lf %17.5lf %17.5lf %17.5lf %8d %7.3lf %10.2lf %12.2lf %10.4le %7.3lf %7.3lf %10.4le %7.3lf %7.3lf %7.3lf %7.3lf %4d %-17.17s\n"

#define FEATURE_WVL30(SP) \
(SP)->arid, (SP)->peak_time, (SP)->peak_level, (SP)->total_energy, \
(SP)->mean_arrival_time, (SP)->time_spread, \
(SP)->onset_time, (SP)->termination_time, (SP)->total_time, (SP)->num_cross, \
(SP)->ave_noise, (SP)->skewness, (SP)->kurtosis, (SP)->cep_var_signal, \
(SP)->cep_delay_time_signal, (SP)->cep_peak_std_signal, \
(SP)->cep_var_trend, (SP)->cep_delay_time_trend, (SP)->cep_peak_std_trend, \
(SP)->low_cut, (SP)->high_cut, (SP)->ford, (SP)->lddate


#define FEATURE_NULL30 \
{ \
-1,			/* arid		*/ \
-1.0,			/* peak_time	*/ \
-1.0,			/* peak_level	*/ \
-1.0,			/* total_energy	*/ \
-1.0,			/* mean_arrival_time	*/ \
-1.0,			/* time_spread	*/ \
-1.0,			/* onset_time	*/ \
-1.0,			/* termination_time	*/ \
-1.0,			/* total_time	*/ \
-1,			/* num_cross	*/ \
-1.0,			/* ave_noise	*/ \
-1.0,			/* skewness	*/ \
-1.0,			/* kurtosis	*/ \
-1.0,			/* cep_var_signal	*/ \
-1.0,			/* cep_delay_time_signal	*/ \
-1.0,			/* cep_peak_std_signal	*/ \
-1.0,			/* cep_var_trend	*/ \
-1.0,			/* cep_delay_time_trend	*/ \
-1.0,			/* cep_peak_std_trend	*/ \
-1.0,			/* low_cut	*/ \
-1.0,			/* high_cut	*/ \
-1,			/* ford	*/ \
"-",			/* lddate	*/ \
}
 
#endif /* _FEATURE_3_0_H */

