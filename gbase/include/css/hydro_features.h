/*	SccsId:	%W%	%G%	*/
/**
 *	Hydro_features structure from CSS 3.0 table definitions.
 */
#ifndef _HYDRO_FEATURES_3_0_H
#define _HYDRO_FEATURES_3_0_H

#define HYDRO_FEATURES30_LEN	298

/** 
 *  Hydro_features structure.
 *  @member arid Arrival id. Initial value = -1.
 *  @member peak_time Initial value = -999.
 *  @member peak_level Initial value = -999.
 *  @member total_energy Initial value = -999.
 *  @member mean_arrival_time Initial value = -999.
 *  @member time_spread Initial value = -999.
 *  @member onset_time Initial value = -999.
 *  @member termination_time Initial value = -999.
 *  @member total_time Initial value = -999.
 *  @member num_cross Initial value = -1.
 *  @member ave_noise Initial value = -999.
 *  @member skewness Initial value = -999.
 *  @member kurtosis Initial value = -999.
 *  @member cep_var_signal Initial value = -999.
 *  @member cep_delay_time_signal Initial value = -999.
 *  @member cep_peak_std_signal Initial value = -999.
 *  @member cep_var_trend Initial value = -999.
 *  @member cep_delay_time_trend Initial value = -999.
 *  @member cep_peak_std_trend Initial value = -999.
 *  @member low_cut Initial value = -999.
 *  @member high_cut Initial value = -999.
 *  @member ford Initial value = -1.
 *  @member ftype Initial value = "-".
 *  @member fzp Initial value = -1.
 *  @member prob_weight_time Initial value = -999.
 *  @member sigma_time Initial value = -999.
 *  @member lddate[18] Load date. Initial value = "-".
 */
typedef struct
{
	long	arid;		/* arrival id			*/
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
	char	ftype[3];
	int	fzp;
	double	prob_weight_time;
	double	sigma_time;
	char	lddate[18];	/* load date			*/
} HYDRO_FEATURES30;

/* these need to be corrected

#define HYDRO_FEATURES_RCS30 "%8ld%*c%8ld%*c%6c%*c%8c%*c%4f%*c%8f%*c%7f%*c%7f%*c%8f%*c%1c%*c%7f%*c%1c%*c%7f%*c%1c%*c%7f%*c%6f%*c%15c%*c%8ld%*c%17c"

#define HYDRO_FEATURES_RVL30(SP) \
&(SP)->arid, &(SP)->orid, (SP)->sta, (SP)->phase, &(SP)->belief, &(SP)->delta,\
&(SP)->seaz, &(SP)->esaz, &(SP)->timeres, (SP)->timedef, &(SP)->azres,\
(SP)->azdef, &(SP)->slores, (SP)->slodef, &(SP)->emares, &(SP)->wgt, \
(SP)->vmodel, &(SP)->commid, (SP)->lddate

*/

#define HYDRO_FEATURES_WCS30 "%8ld %17.5lf %7.3lf %7.3lf %17.5lf %7.3lf %17.5lf %17.5lf %17.5lf %8d %7.3lf %10.2lf %12.2lf %10.4le %7.3lf %7.3lf %10.4le %7.3lf %7.3lf %7.3lf %7.3lf %4d %-2.2s %2d %17.5lf %17.5lf %-17.17s\n"

#define HYDRO_FEATURES_WVL30(SP) \
(SP)->arid, (SP)->peak_time, (SP)->peak_level, (SP)->total_energy, \
(SP)->mean_arrival_time, (SP)->time_spread, \
(SP)->onset_time, (SP)->termination_time, (SP)->total_time, (SP)->num_cross, \
(SP)->ave_noise, (SP)->skewness, (SP)->kurtosis, (SP)->cep_var_signal, \
(SP)->cep_delay_time_signal, (SP)->cep_peak_std_signal, \
(SP)->cep_var_trend, (SP)->cep_delay_time_trend, (SP)->cep_peak_std_trend, \
(SP)->low_cut, (SP)->high_cut, (SP)->ford, (SP)->ftype, (SP)->fzp, \
(SP)->prob_weight_time, (SP)->sigma_time, (SP)->lddate


#define HYDRO_FEATURES_NULL30 \
{ \
-1,			/* arid		*/ \
-999.0,			/* peak_time	*/ \
-999.0,			/* peak_level	*/ \
-999.0,			/* total_energy	*/ \
-999.0,			/* mean_arrival_time	*/ \
-999.0,			/* time_spread	*/ \
-999.0,			/* onset_time	*/ \
-999.0,			/* termination_time	*/ \
-999.0,			/* total_time	*/ \
-1,			/* num_cross	*/ \
-999.0,			/* ave_noise	*/ \
-999.0,			/* skewness	*/ \
-999.0,			/* kurtosis	*/ \
-999.0,			/* cep_var_signal	*/ \
-999.0,			/* cep_delay_time_signal	*/ \
-999.0,			/* cep_peak_std_signal	*/ \
-999.0,			/* cep_var_trend	*/ \
-999.0,			/* cep_delay_time_trend	*/ \
-999.0,			/* cep_peak_std_trend	*/ \
-999.0,			/* low_cut	*/ \
-999.0,			/* high_cut	*/ \
-1,			/* ford	*/ \
"-",   			/* ftype	*/ \
-1,			/* fzp		*/ \
-999.0,			/* prob_weight_time */ \
-999.0,			/* sigma_time	*/ \
"-",			/* lddate	*/ \
}
 
#endif /* _HYDRO_FEATURES_3_0_H */

