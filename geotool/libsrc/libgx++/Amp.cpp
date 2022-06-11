/** \file Amp.cpp
 *  \brief Defines amplitude mesaurement routines.
 *  \author Petr Firbas
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

#include "Amp.h"
#include "IIRFilter.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/DataSource.h"
#include "BasicSource.h"
#include "motif++/Application.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
#include "tapers.h"
}

static void local_warn(char *routine, char *message);
static void filter2(double samprate, int nsamp, double calib, float *data,
	double flow, double fhigh, int order, const string &ftype, int zp);

/*
 * Local defines (temporary)
 */

#define MAX_POINTS 20000
#define TEST_CODE 1

/* Temporary patch some .h file should be used! */
#define NA_AMPLITUDE -1.0
#define NA_PERIOD -1.0


#if !defined(ERR) || ((ERR) != 1)
#define ERR (1)
#endif

#if !defined(OK) || ((OK) != 0)
#define OK (0)
#endif


#define TRUE 1
#define FALSE 0

/*
 * This is here temporarily - should go to input part if 
 * allowed_filter_correction is decided to be input parameter
 */

#define MIN_ALLOWED_FILTER_CORRECTION 0.99

#ifdef TEST_CODE

#define MAX_ALLOWED_FILTER_CORRECTION 1000000.0
#define ALLOWED_FILTER_CORRECTION 1000000.0 

#else

#define MAX_ALLOWED_FILTER_CORRECTION 500.0

#define ALLOWED_FILTER_CORRECTION 500.0
/* This limit should avoid filter corrections (to be applied later)
   which are excessively large. In these cases mostly either period
   estimation is in error or the period is too far outside of the
   filter band. Mostly there could not be enough energy in signal to 
   allow for as high correction
*/
#endif

#ifdef OLD_DEFINES
#define DEFAULT_FILTER_TYPE_AMPLITUDE "BP"	/* Filter type ("BP") */
#define DEFAULT_HICUT_FREQ_AMPLITUDE 4.5	/* Upper corner frequency */
#define DEFAULT_LOWCUT_FREQ_AMPLITUDE 0.6	/* Lower corner frequency */
#define DEFAULT_ZERO_PHASE_FILTER_AMPLITUDE 1	/* Use zero phase filtering */
#define DEFAULT_FILTER_ORDER_AMPLITUDE 3	/* Butterworth filter, 3 poles
						   (twice)
						 */
/* Percentage of beam window tapered. */
#define DEFAULT_FILTER_TAPER_FRACT_AMPLITUDE 0.05

 /* Percent of max beam peak to though amplitude to use as threshold to omit
    small swings */
#define DEFAULT_AUTOMB_THRESHOLD1 15.0

/* Pre and post time where reading cannot occur as the filtering does not
   provide stable results here */
#define DEFAULT_PRE_POST_TIME_AMPLITUDE 10.0

/* Part of the amplitude window before the pick */
#define DEFAULT_PREPICK_WINDOW_AMPLITUDE 0.5

/* Part of the amplitude window after the pick */
#define DEFAULT_POSTPICK_WINDOW_AMPLITUDE 7.0

#endif

/*
 * Global Variables
 */


static int amp_err = 0;
static char amp_err_msg[1024];

#ifdef _MAIN

static float beam[MAX_POINTS]; /* processed signal - channel/beam */

main()
{
    double samprate = 50.0; /* Sample rate */
    int nsamples = MAX_POINTS; /* Number of points in beam buffer */
    double waveform_start_time = 0.0; /* Waweform start time */

    double max_allowed_filter_correction = ALLOWED_FILTER_CORRECTION; 
    /* This limit should avoid filter corrections (to be applied later)
     * which are excessively large. In these cases mostly either period
     * estimation is in error or the period is too far outside of the
     * filter band. Mostly there could not be enough energy in signal to 
     *  allow for as high correction
     */

 
    int i_frequency; /* Index of frequency used for testing */
    /* Current signal frequency used for synthetic signal generation */
    float signal_freq;
    int i_sample; /* Loop index */
    double time_of_the_pick; /* Time of the pick */
    double time_of_max_amplitude; /* Time of the max. amplitude */
    double max_amplitude; /* Max. amplitude in nm */
    double period; /* Period in seconds */
    double bandw; /* bandwidth used for measurement */
    int number_of_full_periods; /* Use to generate the maximum at correct pt */

    /*
     * Test on usable threshold for maximum filter correction 
     * (to be applied later) allowed.
     */

    if(max_allowed_filter_correction > MAX_ALLOWED_FILTER_CORRECTION) 
    {
	fprintf(stderr,
	    "Allowed filter correction=%.4f. Should be between %.4f and %.4f ",
	    max_allowed_filter_correction, MIN_ALLOWED_FILTER_CORRECTION, 
	    MAX_ALLOWED_FILTER_CORRECTION);

	fprintf(stderr, "Allowed filter correction set to %.4f ", 
	    MAX_ALLOWED_FILTER_CORRECTION);

	max_allowed_filter_correction = MAX_ALLOWED_FILTER_CORRECTION;
    }

    if(max_allowed_filter_correction < MIN_ALLOWED_FILTER_CORRECTION) 
    {
	fprintf(stderr,
	    "Allowed filter correction=%.4f. Should be between %.4f and %.4f ",
	    max_allowed_filter_correction, MIN_ALLOWED_FILTER_CORRECTION, 
	    MAX_ALLOWED_FILTER_CORRECTION);

	fprintf(stderr, "Allowed filter correction set to %.4f ", 
	    MIN_ALLOWED_FILTER_CORRECTION);

	max_allowed_filter_correction = MIN_ALLOWED_FILTER_CORRECTION;
    }

    fprintf(stderr, "Sampling frequency used: %f \n", samprate);


    /* 
     * Temporarily - loop over many signal frequencies 
     * step = 0.5 * sample interval 
     */

    /* for(i_frequency = 66; i_frequency < 67; i_frequency++) */
    for(i_frequency = 9; 
	samprate / (2 + i_frequency * 0.5) >= 1.0 / 2.6 && i_frequency < 163; 
	i_frequency++) 
    {
	signal_freq = samprate / (2 + i_frequency * 0.5); 

	/*
	 * Set time of the pick 
	 */

	number_of_full_periods = (int) ((DEFAULT_PRE_POST_TIME_AMPLITUDE +
			DEFAULT_PREPICK_WINDOW_AMPLITUDE) * signal_freq);

	/*
	 * For sin()
	 */

 	time_of_the_pick = waveform_start_time +
				(number_of_full_periods + 1.25) / signal_freq; 

	/*
	 * For sinc()
	 */

/*
	time_of_the_pick = waveform_start_time +
				(number_of_full_periods + 1.5);
*/


	fprintf(stderr, 
   "\nUsing sinusoid with frequency = %f, i.e. period = %f freq. index = %d\n", 
		signal_freq, 1.0 / signal_freq, i_frequency); 

/*
	fprintf(stderr, 
    "\nUsing sinc(x) with frequency = %f, i.e. period = %f freq. index = %d\n", 
		signal_freq, 1.0 / signal_freq, i_frequency);
*/

	/* 
	 * Sinusoid or sinc(x) generation
	 */
 
	for(i_sample = 0; i_sample < nsamples; i_sample++)
	{
/***
	    beam[i_sample] = ALLOWED_FILTER_CORRECTION * 1.0 * 
		sin(2.0 * M_PI * signal_freq * (float) i_sample / samprate);
***/
	    beam[i_sample] = 1.0 * 1.0 * 
		sin(2.0 * M_PI * signal_freq * (float) i_sample / samprate);
 
/***
	    if((float)i_sample/samprate == time_of_the_pick) {
		    beam[i_sample] = 1.0 * 1.0;
	    }
	    else {
		beam[i_sample] = 1.0 * 1.0 * sin(2.0*M_PI*signal_freq*
			((float)i_sample/samprate-time_of_the_pick))/
			(2.0*M_PI*signal_freq*((float)i_sample/
				samprate-time_of_the_pick));
	    } 
****/
	}

	/*
	 * Compute the maximum amplitude in nms corrected for filter and
	 * instrument response. Get also time of max. amplitude and the best
	 * guess for period Put the pick into a maximum to make the testing
	 * easy.
	 */

	if(measure_SP_pp_amplitude(beam, samprate, nsamples,
		waveform_start_time, time_of_the_pick, &max_amplitude, &period,
		&time_of_max_amplitude, &bandw) != OK)
	{
	    fprintf(stderr, "Max. amplitude determination failed.\n");
	    continue;
	}
	else
	{
	    double fract;
	    double rest;
	    rest = modf(fabs(2.0*(time_of_max_amplitude-time_of_the_pick)
			*signal_freq),&fract);

	    if((fabs(max_amplitude - 1.0) > 0.01 ||
		fabs(period * signal_freq - 1.0) > 0.01 ||
		(fabs(rest) > 0.01 && fabs(fabs(rest) - 1.0) > 0.01)) || 
		fabs((time_of_max_amplitude-time_of_the_pick)*signal_freq)>3.0)
	    {
		fprintf(stderr, "Amplitude: %f Time: %f Period: %f\n",
		    max_amplitude, time_of_max_amplitude, period);
		fprintf(stderr,
		    "Rel. amplitude: %f Rel. period: %f Diff. Time: %f Number of full periods: %f \n",
		    max_amplitude / 1.0, period * signal_freq,
		    time_of_max_amplitude - time_of_the_pick, 
		    (time_of_max_amplitude - time_of_the_pick) * signal_freq);
	    }
	}
    }
}
#endif

/** Measure amplitude and period.
 *  @param[in] ts the waveform data.
 *  @param[in] amptype the amptype for the CssAmplitude record.
 *  @param[in] time the arrival or measurement time.
 *  @param[in] mb_allow_counts if true, the units of the amplitude will be
 *	counts when the instrument response is not available.
 *  @param[in] mb_counts_amptype the amptype when the units of the amplitude
 *	are counts. Only used if mb_allow_counts is true and the instrument
 *	response is not available.
 *  @param[in,out] amp an amplitude record whose amptype, chan, start_time,
 *	duration, amp, and per values are set.
 *  @returns true for success or false for an error. Use Amp::getAmpError() to
 *	retrieve the error message.
 */
bool Amp::autoMeasureAmpPer(GTimeSeries *ts, const string &amptype,
			double time, bool mb_allow_counts,
			const string &mb_counts_amptype, CssAmplitudeClass *amp)
{
    GSegment *s;
    double max_amplitude; 
    double period; 
    double time_of_max_amplitude;
    DateTime dt2;

    strncpy(amp->amptype, amptype.c_str(), sizeof(amp->amptype));
    strncpy(amp->chan, ts->chan(), sizeof(amp->chan));
    amp->chan_quark = stringToQuark(amp->chan);

    amp->start_time = ts->tbeg();
    amp->duration = ts->tend() - ts->tbeg();

    s = ts->segment(0);

    /*	the max_amplitude returned by measure_SP_pp_amplitude is
	half peak to peak, in normalized nms
    */
    if(measure_SP_pp_amplitude(s->data, 1./s->tdel(), s->length(), s->tbeg(),
	time, &max_amplitude, &period, &time_of_max_amplitude, &amp->bandw)
		!= OK)
    {
	timeEpochToDate(time, &dt2);
	amp_err = 1;
	snprintf(amp_err_msg, sizeof(amp_err_msg),
	    "Max. amplitude determination failed.\nsta=%s time=%02d:%02d:%4.1f",
	    ts->sta(), dt2.hour, dt2.minute, dt2.second);
    }
    else if(max_amplitude > NA_AMPLITUDE)
    {
	amp->amp_cnts = max_amplitude;
	amp->amp_Nnms = amp->amp_cnts * s->calib();

	/* convert counts to nms
	 */
	if( !BasicSource::cts2nms(ts, s, max_amplitude, period, &amp->amp) )
	{
	    if(max_amplitude > 0.0 && amp->amp < 0.0 &&
		BasicSource::newMissingResp(stringUpperToQuark(ts->sta()),
					stringUpperToQuark(ts->chan())))
	    {
		amp_err = 1;
		snprintf(amp_err_msg, sizeof(amp_err_msg),
		    "Instrument correction could not be applied for %s/%s",
		    ts->sta(), ts->chan());
	    }
	    if(mb_allow_counts) {
		amp->amp = max_amplitude;
		strncpy(amp->amptype, mb_counts_amptype.c_str(), sizeof(amp->amptype));
		amp->amp_nms = amp->amp;
		amp->per = period;
		amp->amptime = time_of_max_amplitude;
	    }
	}
	else {
	    amp->amp_nms = amp->amp;
	    amp->per = period;
	    amp->amptime = time_of_max_amplitude;
	}
    }

    if(amp->amp < 0.) {
	return false;
    }
    return true;
}

/** Measure short period peak-to-peak amplitude. The following program
 *  properties are used by this function.
    <TABLE>
    <TR><TD><B>Property Name</B></TD> <TD><B>Description</B></TD>
		<TD><B>Default value</B></TD>
    <TR><TD>mb_filter_type</TD>	  <TD>filter type</TD>	           <TD>BP</TD>
    <TR><TD>mb_filter_zp</TD>	  <TD>zero phase filter</TD>	   <TD>1</TD>
    <TR><TD>mb_filter_locut</TD>	  <TD>filter low cut frequency</TD> <TD>0.6</TD>
    <TR><TD>mb_filter_hicut</TD>	<TD>filter high cut frequency</TD> <TD>4.5</TD>
    <TR><TD>mb_filter_order</TD>   <TD>filter order</TD>		   <TD>3</TD>
    <TR><TD>mb_taper_frac</TD> <TD>Fraction of data window tapered</TD>	   <TD>0.05</TD>
    <TR><TD>mb_amp_theshold1</TD> <TD>Percent of max signal peak to trough
	amplitude to use as threshold to omit small swings </TD>    <TD>15</TD>
    <TR><TD>filter_margin</TD>	  <TD>Initial and ending time period where
	reading cannot occur as the filtering does not provide stable results
	here</TD> <TD>10.</TD>
    <TR><TD>mb_lead</TD>	  <TD>Part of the amplitude window before the
		pick </TD> <TD>0.5</TD>
    <TR><TD>mb_length</TD>   <TD>The length of the measurement window</TD> <TD>7.0</TD>
    </TABLE>
 *  @param[in] beam the waveform signal.
 *  @param[in] samprate the sample rate.
 *  @param[in] nsamples the number of data values in the waveform.
 *  @param[in] waveform_start_time epochal start time.
 *  @param[in] time_of_the_pick epochal time of the pick.
 *  @param[out] max_amplitude the maximum amplitude in nm.
 *  @param[out] period the period in seconds.
 *  @param[out] time_of_max_amplitude the epochal time of the maximum amplitude.
 *  @param[out] bandw the bandwidth of the filter.
 */
int Amp:: measure_SP_pp_amplitude(float *beam, double samprate, int nsamples,
		double waveform_start_time, double time_of_the_pick, 
		double *max_amplitude, double *period, 
		double *time_of_max_amplitude, double *bandw)
{
    /*
     * External resources
     */

    /* Filter type ("BP") */
    string filt_type;

    /* Upper corner frequency */
    double hiCut;

    /* Lower corner frequency */
    double loCut;

    /* Use zero phase filtering */
    int zero_phase;

    /* Butterworth filter, 3 poles (twice) */
    int filt_order;

    /* Percentage of beam window tapered.  */
    double taper_fract;

    /* Percent of max beam peak to though amplitude to use as threshold to
     * omit small swings
     */
    double mb_amp_threshold1;

    /*	This limit should avoid filter corrections (to be applied later)
	which are excessively large. In these cases mostly either period
	estimation is in error or the period is too far outside of the filter
	band. Mostly there could not be enough energy in signal to allow for
	as high correction
    */
    double max_allowed_filter_correction = MAX_ALLOWED_FILTER_CORRECTION;

    /* Pre and post time where reading cannot occur as the filtering does not
     * provide stable results here
     */
    double mb_filter_margin;

    /* Part of the amplitude window before the pick */
    double mb_lead;

    /* The length of the amplitude window */
    double mb_length;

   /* Start time of the window to scanned for max. amplitudes */
    double window_start_time = NULL_TIME;


    int istart; /* First point of the amplitude window */
    int nsamples_in_window; /* Total number of points in the window */

    /* abs (peak-to-trough distance).  i.e. 2.0 * uncorrected max. amplitude */
    double amb_SigAmp;

    /* Peak to trough separation in sampling intervals - "float" number */
    double amb_f_sighalfper;

    /*	Position in beam of peak or trough, whichever is sooner, of the start
	of the interval where maximum amplitude is measured
     */
    double amb_f_sigtime;


    /*
     * Initialize
     */
    zero_phase = Application::getProperty("mb_filter_zp", true);
    loCut = Application::getProperty("mb_filter_locut", 0.8);
    hiCut = Application::getProperty("mb_filter_hicut", 4.5);
    *bandw = hiCut - loCut;
    filt_order = Application::getProperty("mb_filter_order", 3);
    taper_fract = Application::getProperty("mb_taper_frac", 0.05);
    mb_amp_threshold1 = Application::getProperty("mb_amp_theshold1", 15.);
    mb_filter_margin = Application::getProperty("mb_filter_margin", 10.);
    mb_lead = Application::getProperty("mb_lead", 0.5);
    mb_length = Application::getProperty("mb_length", 7.0);

    *max_amplitude = NA_AMPLITUDE;
    *period = NA_PERIOD;
    *time_of_max_amplitude = NULL_TIME;

/* zz
    fprintf(stderr, "\nBefore taper\n");
    for(i_sample = 100; i_sample < 150; i_sample++)
    {
	fprintf(stderr, "%.3f ", beam[i_sample]);
    }
    fprintf(stderr, "\n");
zz */
    if( !Application::getProperty("mb_filter_type", filt_type) ) {
	filt_type.assign("BP");
    }

    /*
     * Filter the signal based on a defined procedure
     */

    if(filt_order)
    {
	double tbeg = time_of_the_pick - mb_lead - mb_filter_margin;
    	double tend = time_of_the_pick - mb_lead + mb_length + mb_filter_margin;
	/*
	 * Check available window size
	 */

	if(tbeg < waveform_start_time ||
	   tend > waveform_start_time + (nsamples - 1) / samprate)
	{
	    return ERR;
	}
 
	/* Apply the taper (return variable currently ignored)
	 *
	 * Note: cosine_taper returns in fact a float:
	 *
	 * if(taperArea) return((float) unitArea / taperArea);
	 * else return(1.0);
	 *
	 * ????
	 */

	taper_fract = mb_filter_margin / ((nsamples - 1) / samprate);

	(void) Taper_cosine (beam, nsamples, taper_fract, taper_fract);

/* zz
	fprintf(stderr, "\nAfter taper\n");
	for(i_sample = 100; i_sample < 150; i_sample++)
	{
	    fprintf(stderr, "%.3f ", beam[i_sample]);
	}
	fprintf(stderr, "\n");
zz */

	/* Filter the signal to standard band calib is not used by filter2,
	 * so use -1.0 ....?
	 */

	(void) filter2(samprate, nsamples, -1.0, beam, loCut, hiCut,
			filt_order, filt_type, zero_phase);
/* zz
	fprintf(stderr, "\nAfter filter\n");
	for(i_sample = 100; i_sample < 150; i_sample++)
	{
	    fprintf(stderr, "%.3f ", beam[i_sample]);
	}
	fprintf(stderr, "\n");
zz */
    }
    else
    {
	if(time_of_the_pick - mb_lead < waveform_start_time ||
	   time_of_the_pick - mb_lead + mb_length > waveform_start_time +
			(nsamples - 1) / samprate)
	{
	    return ERR;
	}
    }


/*
    fprintf(stderr, "\nAfter filter\n");
    for(i_sample = 100; i_sample < 150; i_sample++)
    {
	fprintf(stderr, "%.3f ", beam[i_sample]);
    }
    fprintf(stderr, "\n");
*/


    /*
     * Calculate the index of the first point of the amplitude window and
     * the number of points in the amplitude window
     */

    window_start_time = time_of_the_pick - mb_lead;
    istart = (int) ((window_start_time - waveform_start_time) * samprate);
    nsamples_in_window = (int)(mb_length*samprate)+1;

    /*
     * Get the raw amplitude, (half) period ...
     */

    if(automb_new2(beam, &istart, &nsamples_in_window, &nsamples, filt_order,
	zero_phase, hiCut, loCut, samprate, &amb_SigAmp, &amb_f_sighalfper,
	&amb_f_sigtime, &mb_amp_threshold1, max_allowed_filter_correction)
		!= OK)
    {
	return ERR;
    }

    /*
     * Test on returned N/A values - should be caught in fact already above
     */

    if(amb_SigAmp == NA_AMPLITUDE || amb_f_sighalfper == NA_PERIOD || 
	amb_f_sigtime == NULL_TIME)
    {
	return ERR;
    }

    /*
     * Test on zero amplitude
     */

    if(amb_SigAmp == 0.0) {
	fprintf(stderr,"Warning: Zero amplitude.\n");
	return ERR;
    } 

    /*
     * Compute signal frequency
     */

    *period = (2 * amb_f_sighalfper) / samprate;

    /*
     * Apply new filter correction
     */

    if(filt_order)
    {
	if(apply_filt_resp_new2(filt_order, zero_phase, 1.0 / *period, 
		hiCut, loCut, samprate, &amb_SigAmp,
		max_allowed_filter_correction) != OK) 
	{
	    return ERR;
	}
    }

    /* 
     * To correct for filter response and instrument response
     * we must have a frequency
     */

#ifdef TEST_CODE

    /*
     * Disable instrument correction for time testing
     */

/*
    fprintf(stderr, "Instrument Correction disabled!\n");
*/

#else
 
    /* 
     * Here or inside of apply_inst_resp should be test on number of LSBs
     * forming the maximum aplitude; if not so in extreme cases
     * even electronic noise can be considered to be maximum !!
     */

/*!!
    if(apply_inst_resp(recipe->arrsta, recipe->vertComp, amb_SigTime,
	ampmeaspar.sensor_table, ampmeaspar.instrument_table,
	ampmeaspar.response_type, ampmeaspar.response_units,
	amb_SigPer, &amb_SigAmp) != OK)
    { 

!!*/
	/* 
	 * Error: No Instrument Correction 
	 */
/*!!
	fprintf(stderr, "No Instrument Correction for %s/%s",
		recipe->arrsta,recipe->vertComp);
    }
!!*/

#endif

    /* Convert 2*max amplitude, half period, and Maximum "start time" into
     * proper units, Note that N/A values are caught above 
     */

    *max_amplitude = amb_SigAmp / 2.0;
    *time_of_max_amplitude = waveform_start_time + amb_f_sigtime / samprate;

    return OK;
} 

/* --------------------------------------------------------- */

/*
 * New version PF
 */


/*
"defines" for testing
------------------------------------------------------------
*/

/* #define TEST_PRINT Debug print, low verbosity */
/* #define TEST_PRINT2 Debug print, high verbosity */
/* #define TEST_PRINT3 Debug print, very high verbosity */
/* define LAGRANGE Use Lagrange not cosine fit */
/* #define OLD_CODE Will compute also old peaks/troughs 
 and do a comparison test */
/* #define USE_GEOMETRICAL_MEAN Use geometrical averaging */


/*
"defines" as temporary patches
------------------------------------------------------------
*/

#define DB_ERRORMSG_SIZE_BIG 1024






/*
---------------------------------------------------------------
   Thresholds ... should be perhaps mostly external/input values
   and these values might be considered defaults
*/
#define THRESHOLD2 25.0
/* Value to recognize substantial side peaks As a recomended value one can
   take 50+threshold1.  You may use lower number, say 50 if the Lagrange
   computation for forward/backward peaks/troughs is also activated 
   see #define LAGRANGE_AS_HELP above

 !! now LAGRANGE_AS_HELP is activated
*/

#define THRESHOLD3 80.0
/* Percent of (0.0 to Nyquist frequency) band used by the digitizer */

#define THRESHOLD4 2.0
/* Maximum ratio between found half periods allowed before hypothesis of
   missed peak/trough pair prevails and interefrence effects may be seen
*/
 /*CHANGED 2.2 -> 2.0*/
#define THRESHOLD5 10.0
/* maximum allowed mean value of the signal in the window compared to the
   found peak-trough difference Maxamp; expressed in percent
*/
#define THRESHOLD6 97.0
/* In the process of maximum peak - trough pair selection a pair with lower
   than maximum aplitude up to THRESHOLD6 limit would be prefered if it is
   closer to the beginning of the window to the realy absolute maximum,
   but somewhere far from the "onset"
*/
#define ALLOWED_HP_RATIO 2.0
/* Allowed ratio between the determined signal frequency and the high cut
   frequency
*/
#define ALLOWED_LP_RATIO 1.2
/* Allowed ratio between the low cut frequency and the determined signal
frequency
*/
 /*CHANGED 2.5 -> 1.0 as we do not want to let microseisms to be measured */

/*
--------------------------------------------------------------
*/


/*
    These parameters are used to configure the code. They may remain here or
    be converted to input parameters
*/
/*#define DO_NOT_USE_SINGLE */
/* Produce N/A values if only one half period reading is available after all
   tests
*/
#define LAGRANGE_AS_HELP
/* Use Lagrange fit in case cosine is not suitable, do so only for
   peaks/troughs forward/backward
*/

/*
   Limits for limits for the thresholds - this block should go away with
   tests for allowed parameter value tests to the input part of the program
*/
#define MIN_THRESHOLD1 0.0
#define MAX_THRESHOLD1 50.0

#ifdef LAGRANGE_AS_HELP
#define MIN_THRESHOLD2 25.0
#define MAX_THRESHOLD2 80.0
#else
#define MIN_THRESHOLD2 50.0
#define MAX_THRESHOLD2 80.0
#endif

#define MIN_THRESHOLD3 40.0
#define MAX_THRESHOLD3 80.0
#define MIN_THRESHOLD4 1.5
#define MAX_THRESHOLD4 2.5
#define MIN_THRESHOLD5 5.0
#define MAX_THRESHOLD5 15.0
#define MIN_THRESHOLD6 95.0
#define MAX_THRESHOLD6 100.0
#define MIN_ALLOWED_HP_RATIO 1.0
#define MAX_ALLOWED_HP_RATIO 4.0
#define MIN_ALLOWED_LP_RATIO 1.0
#define MAX_ALLOWED_LP_RATIO 2.0



/*
Local defines
*/
#define PEAK 2		/* Search mode looking for peak */
#define TROUGH 1	/* Search mode looking for trough */
#define NEITHER 0	/* Search mode looking for either peak or trough*/

/**
 * Obtains a measure of the body wave amplitude in the given
 * beam waveform. Modifies automb algorithm as originally
 * proposed by H. Swanger (in program design spec).
 * Substantially modified by Petr Firbas to avoid some weak points
 * and to make the amplitude/period pair estimate more robust and precise.
 *
 * For the given beam waveform, the following algorithm is executed:
 *
 * (1) Identify all peak and trough indices. Form a list of this indices. 
 * Do not allow to assign end points of the window peak or trough 
 * as these points cannot be proven to be truly peaks or troughs 
 * and subsequently wrong period and amplitude would be used.
 * Handle cases when more subsequent points have identical value.
 * (Handle case when waveform contains only zeroes first.)
 *
 * (2) Iteratively, find the smallest peak to trough amplitude and
 * eliminate it from the peak-trough index list. Use "threshold1"
 * to identify peak-trough pair which can be eliminated.
 *
 * (3) Perform (2) until all "internal" peak-trough index pairs that had 
 * the amplitude distance less than a threshold1 % of max amplitude 
 * have been eliminated.
 *
 * (4) Identify the largest remaining adjacent peak-trough pair.
 * Take the abs(peak-trough) difference as uncorrected
 * "2* maximum amplitude". 
 *
 * (4a)
 * To avoid sitauation that a time of maximum is determined
 * for a peak-trough pair quite far in the window nad 
 * at the same time a very similar peak-trough separation 
 * can be found closer to the begining of the search window,
 * give preference to closer peak-trough pair if reaching
 * threshol6 (typically 95-99%)
 *
 * (5) Try to identify a peak/trough backwards and a peak/trough 
 * forwards if any in the window. If found test whether these peaks
 * have substantial amplitude compared to the central peak/trough 
 * pair (threshold2 is used).
 *
 * (6) Estimate the position of all these (2-4) peaks/troughs more precisely
 * using three relevant neighboring points (in fact those defining 
 * the peaks/troughs) using cosine function fit. For "side" peaks 
 * the robust Lagrange approach if needed. 
 *
 * (7) Estimate also more precisely the signal values at the central 
 * peak-trough pair. Compute more precise "2 * maximum amplitude" 
 * value.
 *
 * (8) Compute more precise estimate of the "start of the peak". Take
 * the first peak/trough defining the central peak-trough pair.
 *
 * (9) Use all 1-3 half period estimates as input for
 * more reliable and robust period determination.
 * 
 * (10) Quality check for non-reasonable values of the individual 
 * half period readings:
 * 
 * a) Test that period fits into (0.0 - Nyquist_frequency * threshold3)
 * band - test for useful band for the digitizer
 * (parameter threshold3)
 * 
 * b) Test that determined period is reasonable compared to the
 * window length (period determined shouldn't be longer then 1/5 of
 * the used window to keep the algorithm properly running)
 *
 * c) Test whether the period found is not too far from the
 * high pass corner frequency (allowed_hp_ratio parameter)
 *
 * d) Test whether the period found is not too far from the
 * low pass corner frequency (allowed_lp_ratio parameter)
 *
 * e) Test on maximum allowed filter correction to be applied 
 * later in apply_filter_response. (Do not allow to amplify
 * noise.) (Parameter: max_allowed_filter_correction)
 *
 * (11) Quality check for non-reasonable values which can be detected 
 * by comparison of the half period readings:
 *
 * a) Test others against the minimum half period found
 * (parameter threshold4) first. Exclude a too high value.
 * 
 * b) Test others against the maximum half period found
 * (parameter threshold4). Exclude a too small value.
 *
 * c) Get rid of one of two remaining values in case that
 * they do not seem to be consistent (also uses parameter
 * threshold4).
 *
 * (12) Compute the "best" estimate of the half period.
 * Based on DO_NOT_USE_SINGLE use/not use single half period 
 * reading.
 *
 * (13) Return abs (peak to trough distance = 2 * max amplitude), 
 * half period in samples (float value), and 
 * (float) "index" corresponding to the start of half period interval 
 * with maximum peak to trough distance
 *
 * @param[in] beam waveform data 
 * @param[in] istart_inp the first index in beam to use 
 * @param[in] nsamples_in_window_inp the number of points to use after istart 
 * @param[in] nsamples_inp the total number of points passed in beam 
 * @param[in] filter_order the filter order; for zero phase it will be doubled
 * @param[in] zero_phase flag for zero phase filter; true for non causal
 * @param[in] hicut high cut frequency of the filter
 * @param[in] lowcut low cut frequency of the filter
 * @param[in] samprate signal sample rate 
 * @param[out] amb_SigAmp  the absolute value of the peak-to-trough measurement,
 * 	i.e. 2.0 * uncorrected maximum amplitude 
 * @param[out] amb_f_sighalfper the peak to trough separation in sampling
 *	intervals - float number 
 * @param[out] amb_f_sigtime the position in beam of peak or trough, whichever
 *	is sooner, relative to the first point in the beam 
 * @param[in] threshold1_inp the percent of max beam amp to use as threshold 
 * @param[in] max_allowed_filter_correction maximum allowed filter correction. 
 * This limit should avoid filter corrections (to be applied later) which are
 * excessively large. In these cases mostly either period estimation is in
 * error or the period is too far outside of the filter band. Mostly there
 * could not be enough energy in signal to allow for as high correction
 * @returns OK if successful, ERR otherwise.  In case of failure returns -1.0
 *  in amb_SigAmp, amb_f_sighalfper, NULL_TIME in amb_f_sigtime
 * @author Petr Firbas
 * @version Feb 1995 test version, , Jan 1996 samplerate changed to double
 */
int Amp::automb_new2(float *beam, int *istart_inp, int *nsamples_in_window_inp, 
		int *nsamples_inp, int filter_order, int zero_phase, 
		double hicut, double lowcut, double samprate, 
		double *amb_SigAmp, double *amb_f_sighalfper, 
		double *amb_f_sigtime, double *threshold1_inp, 
		double max_allowed_filter_correction)
{

    /*
     * Temporary patch, as DB_ERRORMSG_SIZE = 80 was too small 
     */
    char error_string[DB_ERRORMSG_SIZE_BIG + 1];
    static char function_name[] = "automb_new2";


    int istart;
    int nsamples_in_window;
    int nsamples;
    double threshold1; /* Percent of max beam amp to use as threshold */
    double threshold2; /* Value to recognize substantial side peaks */

    /* Per cent of (0.0 to Nyquist frequency) band used by the digitizer */
    double threshold3;

    /* Maximum ratio between found half periods allowed before hypothesis of
     * missed peak/trough pair prevails and interefrence effects may be seen
     */
    double threshold4;

    /* maximum allowed mean value of the signal in the window compared to the
     * found peak-trough difference Maxamp; expressed in percent
     */
    double threshold5;

    /* peak-trough pair with separation lower than true max. peak-trough pair
     * will be prefered if closer to the begining of the window; expressed in
     * percent
     */
    double threshold6;

    /* Allowed ratio between the determined signal frequency and the high cut
     * frequency
     */
    double allowed_hp_ratio;

    /* Allowed ratio between the low cut frequency and the determined signal
     * frequency
     */
    double allowed_lp_ratio;

    /* Array which contains eithet PEAK, TROUGH or NEITHER value for every
     * data point
     */
    double *point_clasification;

    int mode = NEITHER; /* incline or decline search mode */
    float last_data; /* last point looked at in search */
    int i_pts; /* index into vectors */
    int i_hp; /* index into array of half period readings */
    int i; /* index into vectors */

    /* Calclulated Threshold for maxima/minima acceptance */
    float CalcThreshold1 = 0.0;

    /* Value used to recognize "substantial" side peaks */
    float CalcThreshold2 = 1.0;

    /* Value used not to allow values close to Nyquist frequency */
    float min_half_period_threshold;

    /* Value used to local_warn about too short window used */
    float max_half_period_threshold;

    double expected_correction; /* Expected correction for filter */
    double freq; /* measured signal frequency */
    float Maxamp = 0.0; /* largest max to min value in beam */
    float minamp = 0.0; /* current minimum peak-trough amp */
    float maxamp = 0.0; /* largest peak to trough amp so far */
    float max_f[4]; /* (float) "indexes" of corrected PEAKS & TROUGHS*/ 
    float best_half_period = -1.0; /* best value found */
    float min_half_period; /* max value found */
    float max_half_period; /* min value found */
    float half_period_value[3]; /* candidate values for the best half period */
    int highPt = 0; /* index of highest point in beam */
    int lowPt = 0; /* index of lowest point in beam */
    int min[2]={0,0}; /* indices of points in minimum peak-trough pair */
    int max[2]; /* indices of pionts in maximum peak-trough pair */
    int max_backward = -1; /* previous maximumu/minimum */
    int max_forward = -1; /* previous maximumu/minimum */
    int half_period_readings = 0; /* number of half periods selected */
    int REndPt=0; /* Rightmost end-point in buffer during decimation*/
    int LEndPt=0; /* Leftmost end-point in buffer during decimation*/
    int n_of_peaks = 0; /* Counter of peaks */
    int n_of_troughs = 0; /* Counter of troughs */
    int constant_value = TRUE; /* Flag for constant value trace */
    int first_i = 1; /* index of the first peak/trough */
    int last_i = 1; /* index of the last peak/trough */
    int first_internal_i = 2; /* index of the first "internal" peak/trough */
    float signal_mean = 0.0; /* computed signal average in the window */

    /* currenly found peak_to_trough_amplitude */
    float peak_to_trough_amplitude = 0.0;

/*
 .. #ifdef LAGRANGE or #ifdef LAGRANGE_AS_HELP
*/
    /* largest peak to trough amp computed from corrected (float) "indexes" */
    float lagrange_ampmax[4];
/*
 .. #else
*/
    /* largest peak to trough amp computed from corrected (float) "indexes" */
    float cos_ampmax[4];

    /* shift in samples determined by cosine function fitting */
    float cos_shift_samples[4];

    /* half period in samples determined by cosine function fitting */
    float cos_half_period[4];
/*
 .. #endif
*/

#ifdef TEST_PRINT
    float amb_f_sighalfper_sum = 0.0; /* sum of durations of 
    detected half periods */
    int number_of_cosine_readings = 0;
    float sum_of_cosine_readings = 0.0;
#endif 

#ifdef OLD_CODE
    int old_max[2]; /* indices of pionts in maximum peak-trough pair */
    int FirstPt=0; /* First point in calc'ing Peak-trough amp */
    float Maxamp_old = 0.0; /* largest max to min value in beam */
    float minamp_old = 0.0; /* current minimum peak-trough amp */
    float maxamp_old = 0.0; /* largest peak to trough amp so far */
#endif
 



    /* switch input parameters to local variables 
     */
    istart = *istart_inp;
    nsamples_in_window = *nsamples_in_window_inp;
    nsamples = *nsamples_inp;
    threshold1 = *threshold1_inp;

    /* Temporarily set the thesholds here; may be changed to input parameters
     */
    threshold2 = Application::getProperty("mb_amp_threshold2", THRESHOLD2);
    threshold3 = Application::getProperty("mb_amp_threshold3", THRESHOLD3);
    threshold4 = Application::getProperty("mb_amp_threshold4", THRESHOLD4);
    threshold5 = Application::getProperty("mb_amp_threshold5", THRESHOLD5);
    threshold6 = Application::getProperty("mb_amp_threshold6", THRESHOLD6);
    allowed_hp_ratio = Application::getProperty("allowed_hp_ratio", ALLOWED_HP_RATIO);
    allowed_lp_ratio = Application::getProperty("allowed_lp_ratio", ALLOWED_LP_RATIO);

    /* Initialization
     * All output values:
     * amb_SigAmp. amb_f_sighalfper, amb_f_sigtime have to be set to N/A values 
     * at initialization. 
     */

    *amb_f_sighalfper = -1.0;
    *amb_SigAmp = -1.0;
    *amb_f_sigtime = NULL_TIME;

    /* Check for good data conditions 
     *  There must be at least 4 data point to allow for at least one PEAK
     * and one THROUGH to be asssigned
     */
    if(beam == NULL || nsamples_in_window <= 4) 
    {
	snprintf(error_string, sizeof(error_string),
			"Error: Wrong data supplied.");
	local_warn(function_name, error_string);
	return ERR;
    }

    /* Test on input parameters consistency
     */
    if(istart < 0 || istart+nsamples_in_window > nsamples)
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: Wrong window: Start=%d End=%d Beam duration=%d samples.",
	    istart, istart+nsamples_in_window, nsamples);
	local_warn(function_name, error_string);
	return ERR;
    }

    /* Tests on usable values for the threshold ... should go somewhere 
     * up in the program if thresholds are implemented as input parameters
     */

    /* Test on usable threshold1 -
     * Percent of max beam amp to use as threshold
     */
    if(threshold1 < MIN_THRESHOLD1)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold1: %.2f. Should be between %.2f and %.2f ",
	    threshold1, MIN_THRESHOLD1, MAX_THRESHOLD1);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	    "Threshold1 (=Percent of max beam amplitude) set to %.2f ",
	    MIN_THRESHOLD1);
	local_warn(function_name, error_string);
	threshold1 = MIN_THRESHOLD1;
    }

    if(threshold1 > MAX_THRESHOLD1)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold1: %.2f. Should be between %.2f and %.2f ",
	    threshold1, MIN_THRESHOLD1, MAX_THRESHOLD1);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	    "Threshold1 (=Percent of max beam amplitude) set to %.2f ",
	    MAX_THRESHOLD1);
	local_warn(function_name, error_string);
	threshold1 = MAX_THRESHOLD1;
    }

    /* Test on usable threshold2 - Percent value to recognize substantial
     * side peaks
     */
    if(threshold2 < MIN_THRESHOLD2)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold2=%.2f. Should be between %.2f and %.2f ",
	    threshold2, MIN_THRESHOLD2, MAX_THRESHOLD2);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold2 (=Percent value to recognize substantial side peak) set to %.2f ",
	    MIN_THRESHOLD2);
	local_warn(function_name, error_string);
	threshold2 = MIN_THRESHOLD2;
    }

    if(threshold2 > MAX_THRESHOLD2)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold2=%.2f. Should be between %.2f and %.2f ",
	    threshold2, MIN_THRESHOLD2, MAX_THRESHOLD2);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold2 (=Percent value to recognize substantial side peak) set to %.2f ",
	    MAX_THRESHOLD2);
	local_warn(function_name, error_string);
	threshold2 = MAX_THRESHOLD2;
    }

    /* Test on usable threshold3 - Percent of (0.0 to Nyquist frequency) band
     * used by the digitizer and as such allowed here as "maximum allowed
     * frequency
     */
    if(threshold3 < MIN_THRESHOLD3)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold3=%.2f. Should be between %.2f and %.2f ",
	    threshold3, MIN_THRESHOLD3, MAX_THRESHOLD3);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	  "Threshold3 (=Percent of the Nyquist frequency allowed) set to %.2f ",
	    MIN_THRESHOLD3);
	local_warn(function_name, error_string);
	threshold3 = MIN_THRESHOLD3;
    }

    if(threshold3 > MAX_THRESHOLD3)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold3=%.2f. Should be between %.2f and %.2f ",
	    threshold3, MIN_THRESHOLD3, MAX_THRESHOLD3);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	  "Threshold3 (=Percent of the Nyquist frequency allowed) set to %.2f ",
	    MAX_THRESHOLD3);
	local_warn(function_name, error_string);
	threshold3 = MAX_THRESHOLD3;
    }

    /* Test on usable threshold4 -
     * Maximum ratio between found half periods allowed before hypothesis of
     * missed peak/trough pair prevails and interefrence effects may be highly
     * expected
     */
     if(threshold4 < MIN_THRESHOLD4)
     {
	snprintf(error_string, sizeof(error_string),
	    "Threshold4=%.2f. Should be between %.2f and %.2f ",
	    threshold4, MIN_THRESHOLD4, MAX_THRESHOLD4);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold4 (=Ratio of periods to test for interference/missed phases) set to %.2f ",
	    MIN_THRESHOLD4);
	local_warn(function_name, error_string);
	threshold4 = MIN_THRESHOLD4;
    }

    if(threshold4 > MAX_THRESHOLD4)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold4=%.2f. Should be between %.2f and %.2f ",
	    threshold4, MIN_THRESHOLD4, MAX_THRESHOLD4);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold4 (=Ratio of periods to test for interference/missed phases) set to %.2f ",
	    MAX_THRESHOLD4);
	local_warn(function_name, error_string);
	threshold4 = MAX_THRESHOLD4;
    }

    /* Test on usable threshold5 - maximum allowed mean value of the signal
     * in the window compared to the found peak-trough difference Maxamp;
     * expressed in percent 
     */
    if(threshold5 < MIN_THRESHOLD5)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold5=%.2f. Should be between %.2f and %.2f ",
	    threshold5, MIN_THRESHOLD5, MAX_THRESHOLD5);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold5 (=Allowed mean value of the signal in percent of Max.) set to %.2f ",
	    MIN_THRESHOLD5);
	local_warn(function_name, error_string);
	threshold5 = MIN_THRESHOLD5;
    }

    if(threshold5 > MAX_THRESHOLD5)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold5=%.2f. Should be between %.2f and %.2f ",
	    threshold5, MIN_THRESHOLD5, MAX_THRESHOLD5);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold5 (=Allowed mean value of the signal in percent of Max.) set to %.2f ",
	    MAX_THRESHOLD5);
	local_warn(function_name, error_string);
	threshold5 = MAX_THRESHOLD5;
    }

    /* Test on usable threshold6 - peak-trough pair with separation lower than
     * true max. peak-trough pair will be prefered if closer to the begining
     * of the window;; expressed in percent 
     */
    if(threshold6 < MIN_THRESHOLD6)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold6=%.2f. Should be between %.2f and %.2f ",
	    threshold6, MIN_THRESHOLD6, MAX_THRESHOLD6);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold6 (=Allowed preference for smaller/closer peak in percent of Max.) set to %.2f ",
	    MIN_THRESHOLD6);
	local_warn(function_name, error_string);
	threshold6 = MIN_THRESHOLD6;
    }

    if(threshold6 > MAX_THRESHOLD6)
    {
	snprintf(error_string, sizeof(error_string),
	    "Threshold6=%.2f. Should be between %.2f and %.2f ",
	    threshold6, MIN_THRESHOLD6, MAX_THRESHOLD6);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
 "Threshold6 (=Allowed preference for smaller/closer peak in percent of Max.) set to %.2f ",
	    MAX_THRESHOLD6);
	local_warn(function_name, error_string);
	threshold6 = MAX_THRESHOLD6;
    }

    /* Test on usable threshold for taking care of periods outside of the used
     * filter band
     */
    if(allowed_hp_ratio < MIN_ALLOWED_HP_RATIO)
    {
	snprintf(error_string, sizeof(error_string),
	"Frequency ratio threshold (HP)=%.2f. Should be between %.2f and %.2f ",
	    allowed_hp_ratio, MIN_ALLOWED_HP_RATIO, MAX_ALLOWED_HP_RATIO);

	local_warn(function_name, error_string);
	snprintf(error_string, sizeof(error_string),
	    "HP_ratio (=Ratio of signal frequency/high cut) set to %.2f ", 
	    MIN_ALLOWED_HP_RATIO);
	local_warn(function_name, error_string);
	allowed_hp_ratio = MIN_ALLOWED_HP_RATIO;
    }

    if(allowed_hp_ratio > MAX_ALLOWED_HP_RATIO)
    {
	snprintf(error_string, sizeof(error_string),
	 "Frequency ratio threshold(HP)=%.2f. Should be between %.2f and %.2f ",
	    allowed_hp_ratio, MIN_ALLOWED_HP_RATIO, MAX_ALLOWED_HP_RATIO);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	    "HP_ratio (=Ratio of signal frequency/high cut) set to %.2f ", 
	    MAX_ALLOWED_HP_RATIO);
	local_warn(function_name, error_string);
	allowed_hp_ratio = MAX_ALLOWED_HP_RATIO;
    }

    if(allowed_lp_ratio < MIN_ALLOWED_LP_RATIO)
    {
	snprintf(error_string, sizeof(error_string),
	"Frequency ratio threshold (LP)=%.2f. Should be between %.2f and %.2f ",
	    allowed_lp_ratio, MIN_ALLOWED_LP_RATIO, MAX_ALLOWED_LP_RATIO);

	local_warn(function_name, error_string);
	snprintf(error_string, sizeof(error_string),
	    "LP_ratio (=Ratio of signal frequency/high cut) set to %.2f ", 
	    MIN_ALLOWED_LP_RATIO);
	local_warn(function_name, error_string);
	allowed_lp_ratio = MIN_ALLOWED_LP_RATIO;
    }

    if(allowed_lp_ratio > MAX_ALLOWED_LP_RATIO)
    {
	snprintf(error_string, sizeof(error_string),
	"Frequency ratio threshold(LP)=%.2f. Should be between %.2f and %.2f ",
	    allowed_lp_ratio, MIN_ALLOWED_LP_RATIO, MAX_ALLOWED_LP_RATIO);
	local_warn(function_name, error_string);

	snprintf(error_string, sizeof(error_string),
	    "LP_ratio (=Ratio of signal frequency/high cut) set to %.2f ", 
	    MAX_ALLOWED_LP_RATIO);
	local_warn(function_name, error_string);
	allowed_lp_ratio = MAX_ALLOWED_LP_RATIO;
    }


    /* Make threshold assignments
     */

    /* Threshold for testing against the Nyuquist frequency
     */
    min_half_period_threshold = 1.0 / (threshold3 / 100.0);

    /* To be able to find 4 peaks/troughs the window should be able to
     * accomodate at least 4-6 half periods for any allowed period, so test
     * for 5.
     */
    max_half_period_threshold = ((float)nsamples_in_window - 5.0) / 5.0;
    if(max_half_period_threshold <= min_half_period_threshold || 
       max_half_period_threshold < 1.0 / (lowcut / allowed_lp_ratio))
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: threshold3 or window length are not reasonable.");
	local_warn(function_name, error_string);

	return ERR;
    }


    /* Test code :
     * It is not expected that besides of a constant value channel that there
     * might be more that two subsequent samples with exactly the same values -
     * it is expected that a filter running before this algorithm lowers
     * probability of such situations. However, for testing purposes this part
     * of code is added to see whether 3 or more such samples appear in data. 
     */
    {
	int n_of_identical = 0;

	for(i_pts = 1; i_pts < nsamples_in_window; i_pts++)
	{
	    if(beam[istart+i_pts] == beam[istart+i_pts-1]) {
		n_of_identical++;
	    }
	    else 
	    {
		if(n_of_identical > 2) {
		    snprintf(error_string, sizeof(error_string),
				"Sequence of %d identical samples\
 %.2f detected starting at sample %d in the window.",
			n_of_identical, beam[istart+i_pts-1],
			i_pts - n_of_identical);
		    local_warn(function_name, error_string);

		    n_of_identical = 0;
		}
		else {
		    n_of_identical = 0;
		}
	    }
	}
    }


    /* Make peak-trough vector 
     */
    if((point_clasification = (double *) malloc((nsamples_in_window) *
				sizeof(double))) == (double *) NULL)
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: point_clasification array cannot be allocated.");
	local_warn(function_name, error_string);
	return ERR;
    }


#ifdef OLD_CODE
    /* Calculate the peak/troughs positions using the old code to see the
     * comparison
     */
    {
	mode = NEITHER;
	minamp_old=0.0;
	Maxamp_old =0.0;
	highPt = 0;
	lowPt = 0;
	old_max[0] = 1; 
	old_max[1] = 1;
	min[0] = 0; 
	min[1] = 0;
	LEndPt = 0;
	REndPt = 0;
	last_data = (double) beam[istart];

	/* start at 2nd point */
	for(i_pts = 1; i_pts < nsamples_in_window; i_pts++)
	{
	    /* Search for High and Low point in beam */
	    if(beam[i_pts+istart] > beam[istart+highPt]) highPt = i_pts;
	    if(beam[i_pts+istart] < beam[istart+lowPt]) lowPt = i_pts;

	    /* Assign Peak, Trough, or Neither markers to all points in
	     * point_clasification
	     */
	    if((mode == TROUGH) && (beam[istart+i_pts] > last_data)) {
		point_clasification[i_pts-1] = TROUGH; mode = PEAK;
	    }
	    else if((mode == TROUGH) && (beam[istart+i_pts] == last_data)) {
		point_clasification[i_pts-1] = TROUGH; mode = NEITHER;
	    }
	    else if((mode == TROUGH) && (beam[istart+i_pts] < last_data)) {
		point_clasification[i_pts-1] = NEITHER; mode = TROUGH;
	    }
	    else if((mode == NEITHER) && (beam[istart+i_pts] > last_data)) {
		point_clasification[i_pts-1] = TROUGH; mode = PEAK;
	    }
	    else if((mode == NEITHER) && (beam[istart+i_pts] == last_data)) {
		point_clasification[i_pts-1] = NEITHER; mode =NEITHER;
	    }
	    else if((mode == NEITHER) && (beam[istart+i_pts] < last_data)) {
		point_clasification[i_pts-1] = PEAK; mode = TROUGH;
	    }
	    else if((mode == PEAK) && (beam[istart+i_pts] > last_data)) {
		point_clasification[i_pts-1] = NEITHER; mode = PEAK;
	    }
	    else if((mode == PEAK) && (beam[istart+i_pts] == last_data)) {
		point_clasification[i_pts-1] = PEAK; mode = NEITHER;
	    }
	    else if((mode == PEAK) && (beam[istart+i_pts] < last_data)) {
		point_clasification[i_pts-1] = PEAK; mode = TROUGH;
	    }

	    last_data = beam[istart+i_pts];
	}
	point_clasification[i_pts-1] = mode; /* Qualify Last point in vector */

	Maxamp_old = beam[istart+highPt] - beam[istart+lowPt];
	CalcThreshold1 = (threshold1/100.0) * Maxamp_old;

#ifdef TEST_PRINT2
	/* Test print of peaks/troughs 
	 */
	{
	    int lines = 1;
	    int last_ipts = 10;
	    int line;
	    if(max_forward != -1) last_ipts = max_forward + 1;
	    else last_ipts = nsamples_in_window - 1;

	    lines = (last_ipts +1) / 5 + 1;
	    for(line = 0; line < lines; line++)
	    {
		for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
		{
		    fprintf(stderr, "S(%2d)=%10.2g", i_pts,beam[istart+i_pts]);

		    if(point_clasification[i_pts] == PEAK)
			fprintf(stderr, "++P");
		    else if(point_clasification[i_pts] == TROUGH)
			fprintf(stderr, "++T");
		    else if(point_clasification[i_pts] == NEITHER)
			fprintf(stderr, " ");
		    else fprintf(stderr, "++!");

		    fprintf(stderr, " \t");
		}
		fprintf(stderr, "\n");
	    }
	    fprintf(stderr, "\n");
	}
#endif

	maxamp_old = 0.0;

	while(1)
	{
	    /* Reset minamp_old to find new minimum this pass */
	    minamp_old = Maxamp_old;

	    /* find first PEAK or TROUGH point, initialize it as FirstPt */
	    for(i = 0; i < nsamples_in_window; i++)
	    {
		if(point_clasification[i] == NEITHER) continue;
		else 
		{
		    FirstPt = REndPt = i;
		    break;
		}
	    }
	    /* Loop thru remaining points and find min and max amplitude
	     * indices
	     */
	    for(i_pts = i+1; i_pts < nsamples_in_window; i_pts++)
	    {
		/* Find Next Peak or Trough, skip if NEITHER */
		if(point_clasification[i_pts] == NEITHER) continue;
		else LEndPt = i_pts; /* assign this point as left-most Pt */
 
		/* Take abs of amplitude and see if its an acceptable min */
		peak_to_trough_amplitude =
		    fabs(beam[istart+FirstPt] - beam[istart+LEndPt]);

		if(peak_to_trough_amplitude < CalcThreshold1 &&
		   peak_to_trough_amplitude < minamp_old)
		{
		    /* found new acceptable min */
		    min[0] = FirstPt; /* store these points */
		    min[1] = LEndPt;

		    minamp_old = peak_to_trough_amplitude; /* set new minimum */
		}

		/* Find max amp and points too */
		if(peak_to_trough_amplitude > maxamp_old) 
		{
		    old_max[0] = FirstPt; 
	  	    old_max[1] = LEndPt;
		    maxamp_old = peak_to_trough_amplitude; 
		}
 
		/* Re-assign FirstPt as Left-Most end point */
		FirstPt = LEndPt;
	    }
	    /* Annihilate current minimum or break out if no more mins qualify*/
	    if(minamp_old < CalcThreshold1)
	    {

#ifdef TEST_PRINT3
		/* Test print after any peak-trough pair was declared to be
		 * annihilated
		 */
		{
		    int lines = 1;
		    int last_ipts = 10;
		    int line;

		    if(max_forward != -1) last_ipts = max_forward + 1;
		    else last_ipts = nsamples_in_window - 1;

		    lines = (last_ipts +1) / 5 + 1;
		    for(line = 0; line < lines; line++)
		    {
			for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
			{
			    fprintf(stderr, "S(%2d)=%10.2g",
				i_pts,beam[istart + i_pts]);

			    if(point_clasification[i_pts] == PEAK) {
				if(i_pts == min[0] || i_pts == min[1]) {
				    fprintf(stderr, "**p");
				}
				else {
				    fprintf(stderr, "++P");
				}
			    }
			    else if(point_clasification[i_pts] == TROUGH) {
				if(i_pts == min[0] || i_pts == min[1]) {
				    fprintf(stderr, "**t");
				}
				else {
				    fprintf(stderr, "++T");
				}
			    }
			    else if(point_clasification[i_pts] == NEITHER) {
				fprintf(stderr, " ");
			    }
			    else {
				fprintf(stderr, "++!");
			    }
			    fprintf(stderr, " \t");
			}
			fprintf(stderr, "\n");
		    }
		    fprintf(stderr, "\n");
		}
#endif
		/* annihilate PEAKS and TROUGH at this min; try again */
		if(min[0] == REndPt) {
		    point_clasification[ REndPt ] = NEITHER;
		}
		else if(min[1] == LEndPt) {
		    point_clasification[ LEndPt ] = NEITHER;
		}
		else {
		    point_clasification[ min[0] ] = NEITHER;
		    point_clasification[ min[1] ] = NEITHER; 
		}
	    }
	    else {
		/* stable condition */
		break;
	    }
	}
#ifdef TEST_PRINT2
	/* Test print after all the small peaks were anihilated
	 */
	{
	    int lines = 1;
	    int last_ipts = 10;
	    int line;

	    if(max_forward != -1) last_ipts = max_forward + 1;
	    else last_ipts = nsamples_in_window - 1;

	    lines = (last_ipts +1) / 5 + 1;
	    for(line = 0; line < lines; line++)
	    {
		for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
		{
		    fprintf(stderr, "S(%2d)=%10.2g",i_pts,beam[istart + i_pts]);

		    if(point_clasification[i_pts] == PEAK)
			fprintf(stderr, "++P");
		    else if(point_clasification[i_pts] == TROUGH)
			fprintf(stderr, "++T");
		    else if(point_clasification[i_pts] == NEITHER)
			fprintf(stderr, " ");
		    else fprintf(stderr, "++!");

		    if(i_pts == old_max[0]) fprintf(stderr, "1");
		    else if(i_pts == old_max[1]) fprintf(stderr, "2");
		    else fprintf(stderr, " ");

		    fprintf(stderr, "\t");
		}
		fprintf(stderr, "\n");
	    }
	    fprintf(stderr, "\n");
	}
#endif

/* OLD_CODE block ends here
*/
#endif


    /* Calculate the peak/troughs positions
     */ 

    /* First and last point need special handling as they cannot by definition
     * become maximum or minimum. This avoids very strange amplitudes to appear
     * based on incomplete half periods including cases when a "ramp" is
     * considered half period.  By definition if there is/are point(s)
     * following the first point or preceeding the last point of the interval,
     * neither of these can be marked as PEAK or TROUGH. 
     */

    point_clasification[0] = NEITHER;
    last_data = (double)beam[istart];

    i_pts = 1;
    if(beam[istart+i_pts] > last_data) {
	mode = PEAK;
	constant_value = FALSE;
    }
    else if(beam[istart+i_pts] == last_data) {
	mode = NEITHER;
    }
    else if(beam[istart+i_pts] < last_data) {
	mode = TROUGH;
	constant_value = FALSE;
    }
    last_data = beam[istart+i_pts];
 
    /* Assign TROUGH,NEITHER,PEAK attributes to internal points and store
     * it in pts; start at 3rd point and look back.
     */
    for(i_pts = 2; i_pts < nsamples_in_window; i_pts++) 
    {
	if(mode == TROUGH)
	{
	    if(beam[istart+i_pts] < last_data) {
		point_clasification[i_pts-1] = NEITHER; 
		mode = TROUGH;
		constant_value = FALSE;
	    }
	    else if(beam[istart+i_pts] > last_data) {
		point_clasification[i_pts-1] = TROUGH; 
		mode = PEAK; 
		n_of_troughs++;
		constant_value = FALSE;
	    }
	    else
	    {
		/* To avoid having more consequtive point labeled as TROUGHs
		 * change it here to NEITHER point_clasification[i_pts-1] =
		 * TROUGH
		 */
		point_clasification[i_pts-1] = NEITHER; 
		mode = NEITHER; 
		n_of_troughs++;
	    }
	}
	else if(mode == PEAK)
	{
	    if(beam[istart+i_pts] > last_data) {
		point_clasification[i_pts-1] = NEITHER; 
		mode = PEAK;
		constant_value = FALSE;
	    }
	    else if(beam[istart+i_pts] < last_data) {
		point_clasification[i_pts-1] = PEAK; 
		mode = TROUGH; 
		n_of_peaks++;
		constant_value = FALSE;
	    }
	    else {
		/* To avoid having more consequtive point labeled as PEAKSs
		 * change it here to NEITHER point_clasification[i_pts-1] =
		 * PEAK; 
		 */
		point_clasification[i_pts-1] = NEITHER; 
		mode = NEITHER; 
		n_of_peaks++;
	    }
	}
	/* else mode == NEITHER */
	else
	{
	    if(beam[istart+i_pts] > last_data) {
		point_clasification[i_pts-1] = TROUGH; 
		mode = PEAK; 
		n_of_troughs++;
		constant_value = FALSE;
	    }
	    else if(beam[istart+i_pts] < last_data) {
		point_clasification[i_pts-1] = PEAK; 
		mode = TROUGH; 
		n_of_peaks++;
		constant_value = FALSE;
	    }
	    else {
		point_clasification[i_pts-1] = NEITHER; 
		mode = NEITHER;
	    }
	}
	last_data = beam[istart+i_pts];
    }


    /* Qualify last point in vector:
     * Last point can neither be TROUGH nor PEAK as we do not know how the
     * signal continues and last point thus can be characterized only as
     * NEITHER
     */
    point_clasification[nsamples_in_window-1] = NEITHER;

#ifdef TEST_PRINT2
    /* Test print after the Peaks and troughs were initially assigned
     */
    {
	int lines=1;
	int last_ipts=10;
	int line;
	if(max_forward != -1) last_ipts = max_forward + 1;
	else last_ipts = nsamples_in_window - 1;

	lines = (last_ipts +1) / 5 + 1;
	for(line = 0; line < lines; line++)
	{
	    for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
	    {
		fprintf(stderr, "S(%2d)=%10.2g", i_pts,beam[istart + i_pts]);

		if(point_clasification[i_pts] == PEAK) fprintf(stderr, "--P");
		else if(point_clasification[i_pts] == TROUGH)
			fprintf(stderr, "--T");
		else if(point_clasification[i_pts] == NEITHER)
			fprintf(stderr, " ");
		else fprintf(stderr, "--!");

		fprintf(stderr, " \t");
	    }
	    fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n");
    }
#endif

    /* For constant value signal return N/A values both for amplitude and
     * period.  This case should not happen with good data, but in principle
     * it might appear when detection is ok, but subsequent amplitude/period
     * measurement is permormed in a window where the the seismoc signal already
     * disappeared.
     */
    if(constant_value == TRUE)
    {
	snprintf(error_string, sizeof(error_string),
		"Warning: Constant value in the measurement window");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }

    /* Test on # of PEAKS and TROUGHS
     * There must be at least one PEAK and at least one TROUGH in the interval 
     * scanned, otherwise no amplitude/period can be determined
     */

    if(n_of_peaks == 0 || n_of_troughs == 0)
    {
	snprintf(error_string, sizeof(error_string),
			"Warning: No PEAK-TROUGH pair found");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }

#ifdef DO_NOT_USE_SINGLE
    /* If DO_NOT_USE_SINGLE is used, one reading does not qualify and so
     * at least 3 peaks/troughs must be found
     */
    if(n_of_peaks + n_of_troughs < 3)
    {
	snprintf(error_string, sizeof(error_string),
    "Warning: Only one PEAK-TROUGH pair found. One reading does not qualify.");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }
#endif


    /* Find the highest PEAK and the deepest TROUGH.
     * Start at the second point, end at the last but one point.
     */
    highPt = 1;
    lowPt = 1;
    for(i_pts = 1; i_pts < nsamples_in_window-1; i_pts++) 
    {
	if(point_clasification[i_pts] == PEAK && 
	    beam[i_pts+istart] > beam[istart+highPt]) highPt = i_pts;

	if(point_clasification[i_pts] == TROUGH &&
	    beam[i_pts+istart] < beam[istart+lowPt]) lowPt = i_pts;
    }

    /* Determine Maxamp as maximum difference in the maxima in the signal.
     */
    Maxamp = beam[istart+highPt] - beam[istart+lowPt];


    /* Loop for determining significant PEAKS and TROUGHS
     */

    /* Derive CalcThreshold1 for testing of extremes to be neglected
     */
    CalcThreshold1 = (threshold1 / 100.0) * Maxamp;

    /* Verify that the signal used has really zero or small mean value
     */
    signal_mean = 0.0;
    for(i_pts = 0; i_pts < nsamples_in_window-1; i_pts++) {
	signal_mean += beam[i_pts+istart];
    }
    signal_mean /= nsamples_in_window;
    i_pts=abs(i_pts);
    if(100.0 * fabs(signal_mean) / Maxamp > threshold5)
    {
	snprintf(error_string, sizeof(error_string),
"Signal was not detrended. Mean is %.2f per cent of max amp; Max %.2f allowed.",
	    100.0 * fabs(signal_mean) / Maxamp, threshold5);
	local_warn(function_name, error_string);

	goto Exit_automb;
    } 


    /* Determine the maximum PEAK - TROUGH pair, delete all unimportatnt
     * extremes (below CalcThreshold1). Do it in iterative way - delete
     * only one unimportatnt extreme per a loop. Only this ensures that
     * the process converges to the expected maximum PEAK - TROUGH pair.
     * However deleting has sense only if number of peaks&troughs is
     * at least four as only then there exists an "internal" peak/trough pair
     * which might be deleted
     */

    if(n_of_peaks + n_of_troughs >= 4)
    {

	/* Find first PEAK or TROUGH point.
	 * There must be at least one as other cases were excluded above.
	 * Also keep its index as first_i
	 */
	for(i = 1; i < nsamples_in_window-1; i++)
	{
	    if(point_clasification[i] != NEITHER) {
		first_i = i;
		break;
	    }
	}

	/* Find last PEAK or TROUGH point.
	 * There must be at least one as other cases were excluded above.
	 * Also keep its index as last_i
	 */
	for(i = nsamples_in_window-1; i > first_i; i--)
	{
	    if(point_clasification[i] != NEITHER) {
		last_i = i;
		break;
	    }
	}

	while(TRUE)
	{
	    /* Reset minamp to find new minimum this pass 
	     */
	    minamp = Maxamp; 

	    /* Find first internal PEAK or TROUGH point and assign it as the
	     * leftmost point LEndPt.  There must be at least one as other
	     * cases were excluded above.
	     */
	    for(i = first_i + 1; i < last_i; i++)
	    {
		if(point_clasification[i] != NEITHER) {
		    LEndPt = i;
		    first_internal_i = i;
		    break;
		}
	    }

	    max[0] = 1; 
	    max[1] = 1;

	    /* Loop through remaining points and find min amplitude indices to
	     * annihilate unimportant peak-trough pairs; do not allow to
	     * annihilate the first and last peak/trough
	     */
	    for(i_pts = first_internal_i + 1; i_pts < last_i; i_pts++)
	    {
		/* Find next PEAK or TROUGH, skip if NEITHER. Assign this point
		 * as right-most REnPt.
		 */
		if(point_clasification[i_pts] == NEITHER) {
		    continue; 
		}
		else {
		    REndPt = i_pts;
		}
 		/* Take abs of amplitude and see if its an acceptable minimum
		 * that can be potentially deleted 
		 */
		peak_to_trough_amplitude = fabs(beam[istart+LEndPt] -
					beam[istart+REndPt]);

		if(peak_to_trough_amplitude < CalcThreshold1 &&
		   peak_to_trough_amplitude < minamp) 
		{
		    min[0] = LEndPt; /* store these points */
		    min[1] = REndPt;
		    minamp = peak_to_trough_amplitude; /* set new minimum */
		}
 
		/* Re-assign LEndPt as Left-Most end point
		 */
		LEndPt = REndPt;
	    }

	    /* Annihilate current minimum or break out if no more mins qualify 
	     */
	    if(minamp < CalcThreshold1)
	    { 
#ifdef TEST_PRINT3
		/* Test print before a peak/trough selected for annihilation
		 * is really annihilated
		 */
		{
		    int lines=1;
		    int last_ipts=10;
		    int line;
		    if(max_forward != -1) last_ipts = max_forward + 1;
		    else last_ipts = nsamples_in_window - 1;

		    lines = (last_ipts +1) / 5 + 1;
		    for(line = 0; line < lines; line++)
		    {
			for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
			{
			    fprintf(stderr, "S(%2d)=%10.2g",
				i_pts,beam[istart + i_pts]);

			    if(point_clasification[i_pts] == PEAK) {
				if(i_pts == min[0] || i_pts == min[1]) {
				    fprintf(stderr, "**p");
				}
				else {
				    fprintf(stderr, "--P");
				}
			    }
			    else if(point_clasification[i_pts] == TROUGH) {
				if(i_pts == min[0] || i_pts == min[1]) {
				    fprintf(stderr, "**t");
				}
				else {
				    fprintf(stderr, "--T");
				}
			    }
			    else if(point_clasification[i_pts] == NEITHER) {
				fprintf(stderr, " ");
			    }
			    else {
				fprintf(stderr, "--!");
			    }
			    fprintf(stderr, " \t");
			}
			fprintf(stderr, "\n");
		    }
		    fprintf(stderr, "\n");
		}
#endif

		/* Annihilate both the PEAK and TROUGH at this min; try again.
		 * We do not have to consider the option of annihilate
		 * only either PEAK or TROUGH as the smalles min is now
		 * always internal and never the MAX PEAK or MIN TROUGH
		 * can be annihilated this way.
		 */
		point_clasification[ min[0] ] = NEITHER;
		point_clasification[ min[1] ] = NEITHER;
	    }
	    else {
		/* stable condition, no more mins under CalcThreshold1 */
		break;
	    }
	}
    }

    /* Loop through remaining indices for the max peak_to_trough_amplitude pair
     * Take into account here also the peaks/troughs at the margins of the 
     * intervals
     */
    LEndPt = first_i;
    maxamp = 0.0;
    max[0] = 1; 
    max[1] = 1;

    for(i_pts = first_i; i_pts <= last_i; i_pts++)
    {
	/* Find next PEAK or TROUGH, skip if NEITHER, Assign this point as
	 * right-most Pt REndPt
	 */
	if(point_clasification[i_pts] == NEITHER) {
	    continue;
	}
	else {
	    REndPt = i_pts;
	}
	peak_to_trough_amplitude = fabs(beam[istart+LEndPt] -
					beam[istart+REndPt]);
 
	if(peak_to_trough_amplitude > maxamp) {
	    max[0] = LEndPt; 
	    max[1] = REndPt;
	    maxamp = peak_to_trough_amplitude; 
	}
 
	/* Re-assign LEndPt as Left-Most end point 
	 */
	LEndPt = REndPt;
    }

    /* The above loop finds the absolutely largest peak_to_trough_amplitude.
     * For signals which have several same amplitude swings it may mean 
     * that the peak_to_trough_amplitude found is somewhere close
     * to the end of the window. In such a case there might be
     * a very similar amplitude just around the pick. IN interactive
     * work one would be suprised to see the maximum amplitude picked so far 
     * ahead in the signal once just may be invisibly smaller peaks would
     * be analyst manual choice.
     * To avoid this go once more through and change the select the first 
     * peak-trough pair closes to the beginning of the interval even being
     * a bit smaller (by threshold6).
     * Loop through remaining indices for the max peak_to_trough_amplitude pair
     * Take into account here also the peaks/troughs at the margins of the 
     * intervals
     */
    LEndPt = first_i;

    for(i_pts = first_i; i_pts <= last_i; i_pts++)
    {
	/* Find next PEAK or TROUGH, skip if NEITHER, Assign this point as
	 * right-most Pt REndPt
	 */
	if(point_clasification[i_pts] == NEITHER) {
	    continue;
	}
	else {
	    REndPt = i_pts;
	}
	peak_to_trough_amplitude = fabs(beam[istart+LEndPt] -
					beam[istart+REndPt]);
 
	if(peak_to_trough_amplitude > maxamp * threshold6 / 100.0) {
	    max[0] = LEndPt; 
	    max[1] = REndPt;
	    maxamp = peak_to_trough_amplitude; 
 
	    /* If any found - just break
	     */
	    break;
	}
	/* Re-assign LEndPt as Left-Most end point 
	 */
	LEndPt = REndPt;
    }


#ifdef OLD_CODE
    /* We can test whether the OLD_CODE and the new one provide the same
     * position of maximum
     */
     if(old_max[0] != max[0] || old_max[1] != max[1])
     {
	if(old_max[1] == 0) {
	    fprintf (stderr, 
 "!!! Max/min selected erroneously at the start of window by the old approach.\n");
	}
	if(old_max[1] == nsamples_in_window - 1) {
	    fprintf (stderr, 
 "!!! Max/min selected erroneously at the end of window by the old approach.\n");
	}

	fprintf(stderr,
	    "!!! The old code found maxima at: %d,%d the new one at: %d,%d \n",
	    old_max[0], old_max[1], max[0], max[1]);
	fprintf(stderr, "!!! The old code found maxima: %f the new one: %f \n",
	    maxamp_old, maxamp);
    }
#endif

    /* Now find additional 2 maxima/minima, if any, one forward and one backward
     * 
     * Derive CalcThreshold2 for testing of PEAKS/TROUGHS to be neglected
     */
    CalcThreshold2 = (threshold2 / 100.0) * maxamp;

    /* Go back and try to find previous maximum/minimum. 
     * If not found indicate it by -1
     */
    max_backward = -2;
    for(i = max[0] - 1; i > 0; i--)
    {
	if(point_clasification[i] == NEITHER)  {
	    continue;
	}
	else 
	{
	    max_backward = i;

	    /* Once the PEAK or TROUGH is found, test whether it is
	     * a substantial one. 
	     * If the amplitude difference between this one and
	     * the one forming max.amplitude is less than CalcThreshold2
	     * then ignore it.
	     */
	    if(fabs(beam[istart + max_backward] - beam[istart + max[0] ]) 
			< CalcThreshold2)
	    {
		snprintf(error_string, sizeof(error_string),
"Backward PEAK/TROUGH point ignored.\
 \n ... Only %.2f per cent of max amp; %.2f requested.",
		    100.0 * fabs(beam[istart + max_backward] -
		    beam[istart + max[0] ]) / maxamp, threshold2);
		local_warn(function_name, error_string);

		max_backward = -1;
	    }
	    break;
	}
    }

    if(max_backward == -2) {
	max_backward = -1;

	snprintf(error_string, sizeof(error_string),
			"Backward PEAK/TROUGH point not found.");
	local_warn(function_name, error_string);
    }


    /* Go forward and try to find previous maximum/minimum
     * If not found indicate it by -1
     */
    max_forward = -2;
    for(i = max[1] + 1; i < nsamples_in_window - 1; i++) {
	if(point_clasification[i] == NEITHER) {
	    continue;
	}
	else {
	    max_forward = i;

	    /* Once the PEAK or TROUGH is found, test whether it is a
	     * substantial one. If the amplitude difference between this one
	     * and the one forming max.amplitude is less than CalcThreshold2
	     * then ignore it.
	     */
	    if(fabs(beam[istart + max_forward] - beam[istart + max[1] ]) 
			< CalcThreshold2)
	    {
		snprintf(error_string, sizeof(error_string),
"Forward PEAK/TROUGH point ignored.\
 \n ... Only %.2f per cent of max amp; %.2f requested.",
		    100.0 * fabs(beam[istart + max_forward] -
		    beam[istart + max[1] ]) / maxamp, threshold2);
		local_warn(function_name, error_string);

		max_forward = -1;
	    }
	    break;
	}
    }

    if(max_forward == -2) {
	max_forward = -1;

	snprintf(error_string, sizeof(error_string),
		"Forward PEAK/TROUGH point not found.");
	local_warn(function_name, error_string);
    }


#ifdef TEST_PRINT2
    /* Test print
     */
    {
	int lines=1;
	int last_ipts=10;
	int line;
	if(max_forward != -1) last_ipts = max_forward + 1;
	else last_ipts = nsamples_in_window - 1;

	lines = (last_ipts +1) / 5 + 1;
	for(line = 0; line < lines; line++)
	{
	    for(i_pts = 5 * line; i_pts < 5 * line + 5; i_pts++)
	    {
		fprintf(stderr, "S(%2d)=%10.2g", i_pts,beam[istart + i_pts]);

		if(point_clasification[i_pts] == PEAK) fprintf(stderr, "--P");
		else if(point_clasification[i_pts] == TROUGH)
		    fprintf(stderr, "--T");
		else if(point_clasification[i_pts] == NEITHER)
		    fprintf(stderr, " ");
		else fprintf(stderr, "--!");

		if(i_pts == max_forward) fprintf(stderr, "F");
		else if(i_pts == max[0]) fprintf(stderr, "1");
		else if(i_pts == max[1]) fprintf(stderr, "2");
		else if(i_pts == max_backward) fprintf(stderr, "B");
		else fprintf(stderr, " ");

		fprintf(stderr, "\t");
	    }
	    fprintf(stderr, "\n");
	}
    }
#endif

    /* Make a test on a properly selected window:
     * The values at the beginning and at the end of the interval may be 
     * in abs value larger than the found maximum peak/trough
     * Allow for the tolerance specified by threshold6
     */

    if( fabs(beam[istart] - beam[istart + max[0]]) > 100.0 * maxamp/threshold6
     || fabs(beam[istart + nsamples_in_window-1] - beam[istart + max[0]]) >
	    100.0 * maxamp / threshold6
     || fabs(beam[istart] - beam[istart + max[1]]) > 100.0 * maxamp/threshold6
     || fabs(beam[istart + nsamples_in_window-1] - beam[istart + max[1]]) >
	    100.0 * maxamp / threshold6)
    {
	snprintf(error_string, sizeof(error_string),
	    "Window unproperly selected - max/min found at its start or end.");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }
 
#ifdef TEST_PRINT2

    /* Now compute the average without computing more accurate positions
     * - test code only -
     */

    half_period_readings = 1;
    amb_f_sighalfper_sum = (float) (max[1] - max[0]);

    if(max_backward != -1) {
	half_period_readings++;
	amb_f_sighalfper_sum += (float) (max[0] - max_backward);
    }

    if(max_forward != -1) {
	half_period_readings++;
	amb_f_sighalfper_sum += (float) (max_forward - max [1]);
    }

    *amb_f_sighalfper = amb_f_sighalfper_sum / (float)half_period_readings;



    fprintf(stderr, "Old differences: %.3f %.3f %.3f \n", 
	(max_backward != -1) ? (float)(max[0] - max_backward) : -1.0, 
	(float)(max[1] - max[0]) , 
	(max_forward != -1) ? (float)(max_forward - max[1]) : -1.0);
    fprintf(stderr, "Integer: Length: %.3f Readings: %d Average: %.3f \n",
	amb_f_sighalfper_sum, half_period_readings, *amb_f_sighalfper);

#endif 

    /* Use Lagrange interpolation
     */ 
#ifdef LAGRANGE
    /* Now compute the more accurate positions of the PEAKS/TROUGHS 
     * using Langrange approach
     * Use Lagrange aproximation from three points only.
     * These 3 points must always satisfy requested conditions
     * as they are those which defined the PEAK/TROUGH
     */

    /* Assign values which will be updated in lagrange3.
     * All values initialized.
     */

    if(max_backward != -1) {
	max_f[0] = (float)max_backward; 
	lagrange_ampmax[0] = beam[istart + max_backward];
    }
    else {
	max_f[0] = -1.0; 
	lagrange_ampmax[0] = -1.0;
    }

    max_f[1] = (float)max[0];
    lagrange_ampmax[1] = beam[istart + max[0] ];

    max_f[2] = (float)max[1];
    lagrange_ampmax[2] = beam[istart + max[1] ];

    if(max_forward != -1) {
	max_f[3] = (float)max_forward;
	lagrange_ampmax[3] = beam[istart + max_forward];
    }
    else {
	max_f[3] = -1.0; 
	lagrange_ampmax[3] = -1.0;
    }

    /* Handle the central PEAK - TROUGH pair first
     */
    if( lagrange3(beam[istart + max[0] - 1], beam[istart + max[0] ], 
	    beam[istart + max[0] + 1], &max_f[1], &lagrange_ampmax[1]) 
     || lagrange3( beam[istart + max[1] - 1], beam[istart + max[1] ], 
	    beam[istart + max[1] + 1], &max_f[2], &lagrange_ampmax[2]))
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: lagrange3 failed for central PEAK - TROUGH pair.");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }
    else
    {
	/* Use the "start of maximum amplitude reading" amb_f_sigtime
	 * and 2*max.amplitude in any case; eventhough the half period
	 * derived from the central PEAK - TROUGH pair might be 
	 * not used for any reason
	 */
	*amb_f_sigtime = (float)istart + max_f[1];
	*amb_SigAmp = fabs(lagrange_ampmax[1] - lagrange_ampmax[2]);
    }


    /* If the PEAK/TROUGH backwards was found
    */
    if(max_backward != -1)
    {
	if( lagrange3( beam[istart + max_backward - 1], 
		beam[istart + max_backward], beam[istart + max_backward + 1],
		&max_f[0], &lagrange_ampmax[0]))
	{
	    snprintf(error_string, sizeof(error_string),
	"Error: lagrange3 failed for backward PEAK - TROUGH pair. Ignore it.");
	    local_warn(function_name, error_string);

	    max_backward = -1; 
	} 
    }


    /* If the PEAK/TROUGH forwards was found
     */
    if(max_forward != -1)
    {
	if( lagrange3( beam[istart + max_forward - 1], 
		beam[istart + max_forward], beam[istart + max_forward + 1],
		&max_f[3], &lagrange_ampmax[3]))
	{
	    snprintf(error_string, sizeof(error_string),
	"Error: lagrange3 failed for forward PEAK - TROUGH pair. Ignore it");
	    local_warn(function_name, error_string);

	    max_forward = -1; 
	} 
    }


#ifdef TEST_PRINT

    /* Test print
     */
    fprintf(stderr, "New - Lagrange differences: %.3f %.3f %.3f \n", 
	(max_backward != -1) ? max_f[1] - max_f[0] : -1.0, max_f[2] - max_f[1],
	(max_forward != -1) ? max_f[3] - max_f[2] : -1.0);

#endif


    /* Use cosine function fitting
    */
#else	/* not LAGRANGE */

    /* Now compute the more accurate positions of the PEAKS/TROUGHS 
     * using cosine function fitting
     * Use only three points only for this fit.
     * These 3 points must always satisfy requested conditions
     * as they are those which defined the PEAK/TROUGH
     */

    /* Assign values which will be updated in cos_parameters
     * All values initialized.
     */

    if(max_backward != -1) {
	max_f[0] = (float)max_backward; 
	cos_ampmax[0] = beam[istart + max_backward];
    }
    else {
	max_f[0] = -1.0; 
	cos_ampmax[0] = -1.0;
    }
    cos_half_period[0] = -1.0;
    cos_shift_samples[0] = 0.0;

    max_f[1] = (float)max[0];
    cos_ampmax[1] = beam[istart + max[0] ];
    cos_half_period[1] = -1.0;
    cos_shift_samples[1] = 0.0;

    max_f[2] = (float)max[1];
    cos_ampmax[2] = beam[istart + max[1] ];
    cos_half_period[2] = -1.0;
    cos_shift_samples[2] = 0.0;

    if(max_forward != -1) {
	max_f[3] = (float)max_forward;
	cos_ampmax[3] = beam[istart + max_forward];
    }
    else {
	max_f[3] = -1.0; 
	cos_ampmax[3] = -1.0;
    }
    cos_half_period[3] = -1.0;
    cos_shift_samples[3] = 0.0;


#ifdef TEST_PRINT3
    {
	int i1;
	float bias;
	float point0;
	float point1;
	float point2;
	float mean;
	for(i1 =0; i1 < nsamples_in_window; i1++) {
	    mean += beam[istart + i1];
	}
	mean /= nsamples_in_window;
	fprintf(stderr, "Mean level %f , i.e %f percent of Maxamp \n",
	    mean, 100.0*mean/Maxamp);

	for(i1=-20; i1 < 52; i1 += 2)
	{
	    bias = mean/10.0 * (float)i1;
	    point0= beam[istart + max[0] - 1] + bias;
	    point1= beam[istart + max[0] ] + bias;
	    point2= beam[istart + max[0] + 1] + bias;
	    cos_parameters( point0, point1, point2,
		&cos_shift_samples[1], &cos_ampmax[1], &cos_half_period[1]) ;
	    fprintf(stderr,
		"Bias level: %d Shift: %f Max: %f HalfPer: %f samples.\n",
		i1, cos_shift_samples[1], cos_ampmax[1], cos_half_period[1]);
	}
	fprintf(stderr, "Mean level %f , i.e %f percent of new max \n",
	    mean, 100.0*mean/fabs(cos_ampmax[1]));
    }
#endif
 
    /* Handle the central PEAK - TROUGH pair first
     */
    if( cos_parameters( beam[istart + max[0] - 1], beam[istart + max[0] ], 
	    beam[istart + max[0] + 1], &cos_shift_samples[1], &cos_ampmax[1],
	    &cos_half_period[1]) 
     || cos_parameters( beam[istart + max[1] - 1], beam[istart + max[1] ], 
	    beam[istart + max[1] + 1], &cos_shift_samples[2], &cos_ampmax[2],
	    &cos_half_period[2]))
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: cos_parameters failed for central PEAK - TROUGH pair.");
	local_warn(function_name, error_string);

	goto Exit_automb;
    }
    else
    {
	/* add the computed shift
	 */
	max_f[1] += cos_shift_samples[1];
	max_f[2] += cos_shift_samples[2];

	/* Use the "start of maximum amplitude reading" amb_f_sigtime
	 * and 2*max.amplitude in any case; eventhough the half period
	 * derived from the central PEAK - TROUGH pair might be 
	 * not used for any reason
	 */
	*amb_f_sigtime = (float)istart + max_f[1];
	*amb_SigAmp = fabs(cos_ampmax[1] - cos_ampmax[2]);
    }


    /* If the PEAK/TROUGH backwards was found
     */
    if(max_backward != -1)
    {
	if( cos_parameters( beam[istart + max_backward - 1], 
		beam[istart + max_backward], beam[istart + max_backward + 1],
		&cos_shift_samples[0], &cos_ampmax[0], &cos_half_period[0]))
	{
#ifdef LAGRANGE_AS_HELP 
	    /* Use Lagrange fit in case cosine is not suitable 
	     */
#ifdef TEST_PRINT
	    fprintf(stderr, "Lagrange fit used.\n");
#endif
	    /* note that lagrange3 contrary to cos_parameters updates
	     * internally max_f directly
	     */
	    if( lagrange3( beam[istart + max_backward - 1], 
		    beam[istart + max_backward], beam[istart + max_backward +1],
		    &max_f[0], &lagrange_ampmax[0]))
	    {
		snprintf(error_string, sizeof(error_string),
    "Error: lagrange3 also failed for backward PEAK - TROUGH pair. Ignore it.");
		local_warn(function_name, error_string);

		max_backward = -1;
	    }
	    else
	    {
		/* only to keep reasonable value here 
		 */
		cos_half_period[0] =
		    (cos_half_period[1] + cos_half_period[2]) / 2.0;
	    }
#else	/* not LAGRANGE_AS_HELP */
	    snprintf(error_string, sizeof(error_string),
 "Error: cos_parameters failed for backward PEAK - TROUGH pair. Ignore it.");
		local_warn(function_name, error_string);

	    max_backward = -1; 
#endif
	} 
	else
	{
	    /* add the computed shift
	     */
	    max_f[0] += cos_shift_samples[0];
	} 
    }


    /* If the PEAK/TROUGH forwards was found
     */
    if(max_forward != -1)
    {
	if( cos_parameters( beam[istart + max_forward - 1], 
		beam[istart + max_forward], beam[istart + max_forward + 1],
		&cos_shift_samples[3], &cos_ampmax[3], &cos_half_period[3]))
	{
#ifdef LAGRANGE_AS_HELP 
	    /* Use Lagrange fit in case cosine is not suitable 
	     */
#ifdef TEST_PRINT
	    fprintf(stderr, "Lagrange fit used. \n");
#endif
	    /* note that lagrange3 contrary to cos_parameters updates
	     * internally max_f directly
	     */
	    if( lagrange3( beam[istart + max_forward - 1], 
		    beam[istart + max_forward], beam[istart + max_forward + 1],
		    &max_f[3], &lagrange_ampmax[3]))
	    {
		snprintf(error_string, sizeof(error_string),
     "Error: lagrange3 also failed for forward PEAK - TROUGH pair. Ignore it.");
		local_warn(function_name, error_string);

		max_forward = -1;
	    }
	    else
	    {
		/* only to keep reasonable value here 
		 */
		cos_half_period[3] =
		    (cos_half_period[1] + cos_half_period[2])/2.0;
	    }
#else	/* not LAGRANGE_AS_HELP */
	    snprintf(error_string, sizeof(error_string),
    "Error: cos_parameters failed for forward PEAK - TROUGH pair.Ignore it.");
	    local_warn(function_name, error_string);

	    max_forward = -1; 
#endif
	} 
	else {
	    /* add the computed shift
	     */
	    max_f[3] += cos_shift_samples[3];
	} 
    }


#ifdef TEST_PRINT
#define RATIO_COS_DIFF 2.0

    /* Test print
     */
	fprintf(stderr, 
"New Peak/trough differences after fitting a cosine function: %.3f %.3f %.3f \n", 
	    (max_backward != -1) ? max_f[1] - max_f[0] : -1.0, 
	    max_f[2] - max_f[1], 
	    (max_forward != -1) ? max_f[3] - max_f[2] : -1.0);

	fprintf(stderr, 
"New half periods from fitting peaks with the cosine functions: %.3f %.3f %.3f %.3f samples\n", 
	    (max_backward != -1) ? cos_half_period[0] : -1.0, 
	    cos_half_period[1], cos_half_period[2], 
	    (max_forward != -1) ? cos_half_period[3] : -1.0);
 
	number_of_cosine_readings = 0;
	sum_of_cosine_readings = 0.0;

	if(max_backward != -1) 
	{
	    if(cos_half_period[0] / (max_f[1] - max_f[0]) > RATIO_COS_DIFF || 
	       cos_half_period[0] / (max_f[1] - max_f[0]) < 1.0 /RATIO_COS_DIFF)
	    {
		fprintf(stderr, "Warning: Ratio of half periods from cosine\
 fitting and peak/trough differences (Backward): %f \n",
		    cos_half_period[0] / (max_f[1] - max_f[0]));
	    }
	    else {
		number_of_cosine_readings++;
		sum_of_cosine_readings += cos_half_period[0];
	    }
	}

	if(cos_half_period[1] / cos_half_period[2] > RATIO_COS_DIFF || 
	   cos_half_period[1] / cos_half_period[2] < 1.0 / RATIO_COS_DIFF)
	{
	    fprintf(stderr, "Warning: Ratio of half periods from cosine\
 fitting the central PEAK/TROUGH: %f \n",
		cos_half_period[1] / cos_half_period[2]);
	}
	else if((cos_half_period[1] + cos_half_period[2]) /
		    (max_f[2] - max_f[1]) / 2.0 > RATIO_COS_DIFF || 
		(cos_half_period[1] + cos_half_period[2]) /
		    (max_f[2] - max_f[1]) / 2.0 < 1.0 / RATIO_COS_DIFF)
	{
	    fprintf(stderr, "Warning: Ratio of half periods from cosine\
 fitting of central peaks and peak/trough differences: %f \n",
		(cos_half_period[1] + cos_half_period[2]) /
		    (max_f[2] - max_f[1]) / 2.0);
	}
	else {
	    number_of_cosine_readings++;
	    sum_of_cosine_readings += cos_half_period[1];
	    number_of_cosine_readings++;
	    sum_of_cosine_readings += cos_half_period[2];
	}

	if(max_forward != -1) 
	{
	    if(cos_half_period[3] / (max_f[3] - max_f[2]) > RATIO_COS_DIFF || 
	       cos_half_period[3] / (max_f[3] - max_f[2]) < 1.0 /RATIO_COS_DIFF)
	    {
		fprintf(stderr, "Warning: Ratio of half periods from cosine\
 fitting and peak/trough differences (Forward): %f \n",
		    cos_half_period[3] / (max_f[3] - max_f[2]));
		}
		else {
		    number_of_cosine_readings++;
		    sum_of_cosine_readings += cos_half_period[3];
		}
	    }

	    if(number_of_cosine_readings > 0) {
		fprintf(stderr, "Number of useful cosine fits of the peaks:\
 %d Average half period %.3f samples \n",
		    number_of_cosine_readings,
		    sum_of_cosine_readings / number_of_cosine_readings);
	    }
#endif /* TEST_PRINT */

#endif /* not LAGRANGE */


    /* Get useful readings, put them into array
     */
    if(max_backward != -1) { 
	half_period_value[0] = max_f[1] - max_f[0];
    }
    else {
	half_period_value[0] = -1.0;
    }

    half_period_value[1] = max_f[2] - max_f[1];

    if(max_forward != -1) {
	half_period_value[2] = max_f[3] - max_f[2];
    }
    else {
	half_period_value[2] = -1.0;
    }

    /* Now make various tests on the obtained half periods
     */

    /* Test against:
     * a) Nyquist frequency proximity
     * b) Relation between used window length and detected half period
     * b) Distance from the corner frequencies 
     * c) Allowed maximum filter response correction (to be applied later)
     * ------------------------------------------------------------------------
     * Newer allow that half period is less or equal 1 sample interval
     * or better 1.0/threshold3, typically 1.0/0.8 as real digital
     * equipment can't use full band up to Nyquist frequency.
     * The non-integer sample values are used here as the positions
     * of peaks were refined in the "lagrange3" or "cos_parameters" function.
     * Maximum half period value allowed here is derived from
     * window length (nsamples_in_window) taking into account that the first
     * and the last point can't be assigned as a peak or trough and that the
     * algorithm implicitly requests (should it work best) if at least 
     * three half periods fit into the window
     */

    for(i_hp = 0; i_hp < 3; i_hp++)
    {
	/* Test on Nyquist frequency proximity
	 */ 
	if(half_period_value[i_hp] != -1.0)
	{
	    if(half_period_value[i_hp] < min_half_period_threshold)
	    {
		snprintf(error_string, sizeof(error_string),
"Half period[%1d]=%.3f seconds is is too\
 small. Not used. \n ... Only %.2f percent of fN=%.2f allowed",
		    i_hp, half_period_value[i_hp] / samprate, threshold3,
		    samprate/2.0);
		local_warn(function_name, error_string);

		half_period_value[i_hp] = -1.0;
	    }
	}

	/* Test on relation between used window length and detected half period
	 */
	if(half_period_value[i_hp] != -1.0)
	{
	    if(half_period_value[i_hp] > max_half_period_threshold)
	    {
		snprintf(error_string, sizeof(error_string),
"Half period[%1d]=%.3f seconds is too\
 large. Not used.\n ... Maximum allowed period for window selected is %f seconds.",
		    i_hp, half_period_value[i_hp] / samprate,
		    2.0 * max_half_period_threshold / samprate);
		local_warn(function_name, error_string);

		half_period_value[i_hp] = -1.0;
	    }
	}

	/* Test on distance from the corner frequencies, first higpass corner
	 * frequency, then lowpass corner frequency, do it only if hicut > 0.0,
	 * ie.  some high pass filtering took place or lowcut > 0.0
	 */

	if(hicut > 0.0)
	{
	    if(half_period_value[i_hp] != -1.0)
	    {
		if(1.0 / (2.0 * half_period_value[i_hp] / samprate) >
		    hicut * allowed_hp_ratio)
		{
		    snprintf(error_string, sizeof(error_string),
"Half period[%1d]=%.3f seconds too\
 far from highpass corner frequency %.2f. Not used. ",
			i_hp,half_period_value[i_hp] / samprate, hicut);
		    local_warn(function_name, error_string);

		    half_period_value[i_hp] = -1.0;
		}
	    }
	}

	if(lowcut > 0.0)
	{
	    if(half_period_value[i_hp] != -1.0)
	    {
		if(1.0 / (2.0 * half_period_value[i_hp] / samprate) <
		    lowcut / allowed_lp_ratio)
		{
		    snprintf(error_string, sizeof(error_string),
"Half period[%1d]=%.3f seconds too\
 far from lowpass corner frequency %.2f. Not used. ",
			i_hp,half_period_value[i_hp] / samprate, lowcut);
		    local_warn(function_name, error_string);

		    half_period_value[i_hp] = -1.0;
		}
	    }
	}

	/* Test on allowed maximum filter response correction (to be applied
	 * later) Use the apply_filter_response_subroutine setting the
	 * amplitude to a unit
	 */
	if(half_period_value[i_hp] != -1.0)
	{
	    expected_correction = 1.0;
	    freq = 1.0 / (2.0 * half_period_value[i_hp] / samprate);

	    if(apply_filt_resp_new2(filter_order, zero_phase, freq, hicut,
		lowcut, samprate, &expected_correction,
		max_allowed_filter_correction) == ERR) 
	    {
		snprintf(error_string, sizeof(error_string),
	 	    "Error: Usage of filter correction function failed");
		local_warn(function_name, error_string);

		goto Exit_automb;
	    }
 
	    if(expected_correction >= max_allowed_filter_correction)
	    {
		snprintf(error_string, sizeof(error_string),
"Half period[%1d]=%.3f seconds would require too high correction %.2f. Not used. ",
		    i_hp, half_period_value[i_hp] / samprate,
		    expected_correction);
		local_warn(function_name, error_string);

		half_period_value[i_hp] = -1.0;
	    }
	}
    }


    /* Order the found half periods and delete those which seem to be too large
     * compared to the smalest one 
     * -----------------------------------------------------------------------
     * Algorithm depends on number of readings available
     */

    /* Count number of available half period readings
     */
    half_period_readings = 0;
    for(i_hp = 0; i_hp < 3; i_hp++) {
	if(half_period_value[i_hp] != -1.0) {
	    half_period_readings++;
	}
    }

    /* Case 0 - no half period readings available at this point
     * algorithm failed - return only -1.0s
     */
    if(half_period_readings == 0)
    {
	snprintf(error_string, sizeof(error_string),
"None of selected half periods did pass all tests. No amplitude/period determined.");
	local_warn(function_name, error_string);
 
	goto Exit_automb;
    }
    /* Case 1 - one half period reading available at this point
    */
    else if(half_period_readings == 1)
    {
	/* Move the valid value down in the array 
	 */
	if(half_period_value[2] != -1.0) {
	    half_period_value[0] = half_period_value[2];
	    half_period_value[2] = -1.0;
	}
	if(half_period_value[1] != -1.0) {
	    half_period_value[0] = half_period_value[1];
	    half_period_value[1] = -1.0;
	}
    }
    /* Case 2 - two half period readings available at this point
    */
    else if(half_period_readings == 2)
    {
	/* Move the valid values down in the array 
	 */
	if(half_period_value[0] == -1.0) {
	    half_period_value[0] = half_period_value[1];
	    half_period_value[1] = half_period_value[2];
	    half_period_value[2] = -1.0;
	}
	else if(half_period_value[1] == -1.0) {
	    half_period_value[1] = half_period_value[2];
	    half_period_value[2] = -1.0;
	}

	/* Order the array
	 */
	if(half_period_value[1] < half_period_value[0]) {
	    min_half_period = half_period_value[1];
	    max_half_period = half_period_value[0];
	    half_period_value[0] = min_half_period;
	    half_period_value[1] = max_half_period;
	}
	else {
	    min_half_period = half_period_value[0];
	    max_half_period = half_period_value[1];
	}

	/* If the max_half_period/min_half_period ratio is greater than
	 * threshold4 then expect that max_half_period may be wrong, e.g.
	 * some peak/trough pair was deleted and now max_half_period is
	 * overestimating the true value; interference effects may play
	 * also important role ...  To select the one which might be used
	 * as a reference compare which one is more out from the selected
	 * filter band, if both are within the band than delete the larger one
	 */

	if(max_half_period / min_half_period > threshold4)
	{
	    snprintf(error_string, sizeof(error_string),
"Half period %.3f samples seems to be too large compared to %.3f. Not used.",
		max_half_period, min_half_period);
		local_warn(function_name, error_string);

		half_period_value[1] = -1.0;
		half_period_readings = 1;
	}
    }
    /* Case 3 - three half period readings available at this point
     * In this case make comparison with the minimum value found.
     * So first order the three values, then remove those which
     * are too large - see Case 2 above.
     */
    else if(half_period_readings == 3)
    {
	/* Order the array
	 */
	if(half_period_value[2] < half_period_value[1]) {
	    min_half_period = half_period_value[2];
	    half_period_value[2] = half_period_value[1];
	    half_period_value[1] = min_half_period;
	}

	if(half_period_value[1] < half_period_value[0]) {
	    min_half_period = half_period_value[1];
	    half_period_value[1] = half_period_value[0];
	    half_period_value[0] = min_half_period;
	}

	if(half_period_value[2] < half_period_value[1]) {
	    max_half_period = half_period_value[1];
	    half_period_value[1] = half_period_value[2];
	    half_period_value[2] = max_half_period;
	}

	/* Set the minimum and maximum found period
	 */
	min_half_period = half_period_value[0];
	max_half_period = half_period_value[2];

	/* Minimum found period will be used as a base for comparison,
	 * however, to remain really robust check whether this minimum
	 * half period found is not too strange.
	 * Use comparison with the medium value. And if found it is well above
	 * discard the lowest and use the medium value as a base
	 */
	if(min_half_period < half_period_value[1] / threshold4) 
	{
	    snprintf(error_string, sizeof(error_string),
"Shortest half period %.3f samples seems\
 to be too small \n ... compared to other values %.3f %.3f. Not used.",
		min_half_period, half_period_value[1], half_period_value[2]);
	    local_warn(function_name, error_string);

	    half_period_readings = 2;
	    half_period_value[0] = half_period_value[1];
	    half_period_value[1] = half_period_value[2];
	    half_period_value[2] = -1.0;
	    min_half_period = half_period_value[0];
	}
 

	/* Don't use the third - maximum ?, if there are still three available:
	 */
	if(half_period_readings == 3)
	{
	    if(max_half_period / min_half_period > threshold4)
	    {
		snprintf(error_string, sizeof(error_string),
"Longest half period %.3f samples seems\
 to be too large compared to %.3f. Not used.",
		    max_half_period, min_half_period);
		local_warn(function_name, error_string);

		half_period_readings = 2;
		max_half_period = half_period_value[1];
		half_period_value[2] = -1.0;
	    }
	}
	/* Get rid also of the originally medium value ? if there are still
	 * two available:
	 */
	else if(half_period_readings == 2)
	{
	    if(half_period_value[1] / min_half_period > threshold4)
	    {
		snprintf(error_string, sizeof(error_string),
"Second longest half period %.3f samples\
 seems to be to large compared to %.3f. Not used.",
		    half_period_value[1], min_half_period);
		local_warn(function_name, error_string);

		half_period_readings = 1;
		max_half_period = half_period_value[0];
		half_period_value[1] = -1.0;
	    }
	}
    }


#ifdef TEST_PRINT
    fprintf(stderr, 
   "New differences (half periods) in samples after testing: %.3f %.3f %.3f \n",
	half_period_value[0], half_period_value[1], half_period_value[2]);
#endif


    /* Compute the best half_period
     * ------------------------------
     */
    if(half_period_readings == 1)
#ifdef DO_NOT_USE_SINGLE
    {
	snprintf(error_string, sizeof(error_string),
      "Single period reading doesn't qualify. No amplitude/period determined.");
	local_warn(function_name, error_string);

	goto Exit_automb; 
     }
#else
     {
	best_half_period = half_period_value[0];
     }
#endif

     else if(half_period_readings == 2)
     {
#ifdef USE_GEOMETRICAL_MEAN
	best_half_period = sqrt(half_period_value[0] * half_period_value[1]);
#else
	best_half_period = (half_period_value[0] + half_period_value[1]) / 2.0;
#endif
     }
     else if(half_period_readings == 3)
     {
#ifdef USE_GEOMETRICAL_MEAN
	best_half_period = 
	cbrt(half_period_value[0] * half_period_value[1] *half_period_value[2]);
#else
	best_half_period = (half_period_value[0] + half_period_value[1] +
		half_period_value[2]) / 3.0;
#endif
    }
    else if(half_period_readings == 0)
    {
	snprintf(error_string, sizeof(error_string),
	    "Error: No half period qualifies. No amplitude/period determined.");
	local_warn(function_name, error_string);

	goto Exit_automb; 
    }
    /* Case - Error - should not appear, ...
     */
    else {
	snprintf(error_string, sizeof(error_string),
		"Error: No half periods. Error in program.");
	local_warn(function_name, error_string);

	goto Exit_automb; 
    }


#ifdef TEST_PRINT2

    /* test code for comparison
     */
    amb_f_sighalfper_sum = half_period_value[0];
    if(half_period_readings >= 2)
	amb_f_sighalfper_sum += half_period_value[1];
    if(half_period_readings == 3)
	amb_f_sighalfper_sum += half_period_value[2];

    *amb_f_sighalfper = amb_f_sighalfper_sum / (float)half_period_readings;

 
    fprintf(stderr, "Float: Length: %.3f samples Readings: %d Average: %.3f \n",
	amb_f_sighalfper_sum,half_period_readings,*amb_f_sighalfper);

#endif

    /* Set the best half_period
     */ 
    *amb_f_sighalfper = best_half_period;


#ifdef TEST_PRINT

    fprintf(stderr,"After precise fit: Readings: %d Best value: %.3f samples\n",
    half_period_readings,*amb_f_sighalfper);

    fprintf(stderr, 
	"Without precise fit: 2*maxamp=%.2g With precise fit: 2*maxamp=%.2g \n",
    maxamp, *amb_SigAmp);

    if(number_of_cosine_readings > 0)
    {
	fprintf(stderr, "Average half period from cosine period fit: %.3f\
 samples. \n ... Ratio of values derived from differences to cosine fit %.3f\n",
	    sum_of_cosine_readings / number_of_cosine_readings,
	    best_half_period/sum_of_cosine_readings *number_of_cosine_readings);
    }


    if(*amb_SigAmp/ maxamp > 1.5 || *amb_SigAmp/ maxamp < 0.99) {
	fprintf(stderr, "!#! Ratio of new_precise/old_raw amplitude is: %f \n",
	    *amb_SigAmp/ maxamp);
    }
#endif

 
    /* Clean up
     */
    free((void *) point_clasification);

    return OK;


    /* Error exit *************** Error exit *************** Error exit
    */
    Exit_automb:

    /* All output values:
     * amb_SigAmp. amb_f_sighalfper, amb_f_sigtime have to be set to N/A values 
     * as at initialization. 
     */

    *amb_f_sighalfper = -1.0;
    *amb_SigAmp = -1.0;
    *amb_f_sigtime = NULL_TIME;

    /* Clean up
     */
    free((void *) point_clasification);

    return OK; 
}


/**
 * Compute better aproximation of the maxima/minima time/position
 * from three points. Use robust Lagrange approach. Time coordinates
 * are considered one sample interval apart (i.e. one unit).
 * Compute also the value at maximum/minimum
 * found. This subroutine should be used preferably for PEAKS/TROUGHS
 * detected at the central point.
 * 
 * @param[in] y_1 the value for point at -1
 * @param[in] y0 the value for point at 0
 * @param[in] y1 the value for point at 1
 * @param[in,out] xmax the updated position of maximum found
 * @param[out] ymax the value at maximum found
 * @author Petr Firbas
 * @version Feb 1995 test version
 */
int Amp::lagrange3(float y_1, float y0, float y1, float *xmax, float *ymax)
{
    static char function_name[] = "lagrange3";
    char error_string[DB_ERRORMSG_SIZE_BIG + 1]; 

    float shift; /* shift of maximum in sample intervals */

    shift = 0.0;
    *ymax = y0;


    /* With current PEAK and TROUGH definition this should not ever happen
     */ 
    if((y_1 - 2 * y0 + y1) == 0.0)
    {
	snprintf(error_string, sizeof(error_string),
		"PEAK or TROUGH? Collinear samples detected.");
	local_warn(function_name, error_string);

	return ERR; 
    }
    else {
	/* Else determine PEAK/TROUGH position
	 */
	shift = (y_1 - y1) / (y_1 - 2 * y0 + y1) / 2.0;
     }

    /* Test again: Is the "shift" is in bounds - 
     * i.e. less than 1 sample interval?
     */
    if(fabs(shift) >= 1.0) 
    {
	snprintf(error_string,  sizeof(error_string),
	    "Values: %.2g %.2g %.2g do not define PEAK or TROUGH.", y_1,y0,y1);
	local_warn(function_name, error_string);

	*ymax = y0;

	return ERR; 
    }

    /* Compute the approximation of the PEAK/TROUGH value 
     */
    *ymax = (shift * (shift - 1.0)) * y_1 / 2.0 -
	    ((shift + 1.0) * (shift - 1.0)) * y0 +
	    (shift * (shift + 1.0)) * y1 / 2.0;

    /* calculate maximum/minimum position
     */
    *xmax = *xmax + shift;

    return OK;
}

/**
 * Compute better aproximation of the maxima/minima time/position
 * from three points. Fit a cosine curve. Time coordinates
 * are considered one sample interval apart (i.e. one unit).
 * Compute also the value at maximum/minimum
 * found. This subroutine should be used preferably for PEAKS/TROUGHS
 * detected at the central point.
 * 
 * @param[in] y_1 the value for point at -1
 * @param[in] y0 the value for point at 0
 * @param[in] y1  vthe alue for point at 1
 * @param[out] *shift_samples the position of maximum found (with fraction of
	sample interval)
 * @param[out] *amp the value at maximum found
 * @param[out] *half_period_samples the half period found (with fraction of
 *		sample interval)
 * @author Petr Firbas
 * @version March 1995 test version
 */
int Amp::cos_parameters(float y_1, float y0, float y1, float *shift_samples,
		float *amp, float *half_period_samples)
{
    float b, c;
    double temp;
    int peak = 1;

    static char function_name[] = "cos_parameters";
    char error_string[DB_ERRORMSG_SIZE_BIG + 1]; 

    /* With current PEAK and TROUGH definition this should not ever happen
     */ 
    if((y_1 - 2 * y0 + y1) == 0.0)
    {
	snprintf(error_string, sizeof(error_string),
		"PEAK or TROUGH? Collinear samples detected.");
	local_warn(function_name, error_string); 

	return ERR; 
    }

    /* For trough use input values * -1.0 and set variable "peak"
     */
    if(y_1 > y0) {
	peak = 0;
    }
    else if(y_1 == y0) {
	if(y1 > y0) {
	    peak = 0;
	}
    }

    if(peak == 0) {
	y_1 = -1.0 * y_1;
	y0 = -1.0 * y0;
	y1 = -1.0 * y1;
    }

    /* compute the half period in samples
     */
    temp = ((double)y1 + (double)y_1) / 2.0 / (double)y0;
    if(fabs(temp) >= 1.0)
    {
#ifdef TEST_PRINT
	/* Error messages handled in automb_new
	 */
	snprintf(error_string, sizeof(error_string),
	    "Cosine model not able to fit strange data. Giving up.");
	local_warn(function_name, error_string); 
#endif

	return ERR; 
    }

    b = acos(temp);
    *half_period_samples = M_PI / b;
 
    /* compute time shift of peak/trough as a fraction of sample interval
     */
    c = atan(1.0 / tan (b) * (y1 - y_1) / (y1 + y_1));
    *shift_samples = fabs(*half_period_samples / M_PI * c);

    /* determine the sign
     */
    if(y_1 > y1) {
	*shift_samples = -1.0 * *shift_samples;
    }

    /* determine the amplitude
     */
    *amp = fabs (y0 / cos(c));
    if(peak == 0) *amp = -1.0 * *amp;

    return OK; 
}

/*
 * New version PF
 */
/* #define TEST_PRINT */

#define DB_ERRORMSG_SIZE_BIG 1024

/* these defines should be put out of this function
 */
#define MAX_FILTER_ORDER 10
/* introduce this limit to aviod erroneous input */

#ifdef TEST_PRINT

#define MAX_HIGH_CUT 80.0
/* specifies the maximum allowed high cut frequncy with respect to
 *Nyquist frequency; value in percent
 */

#endif
 
/**
 * Applies the butterworth filter response corresponding to the input
 * frequency and band cut off frequency(ies).
 * It does not allow the correction to grow over a specified limit.
 *
 * Used in the libampmeas library of the DAP program
 * for body wave amplitude measurements.
 *
 * Warning message for no filter requested.
 *
 * param[in] filter_order the filter order
 * param[in] zero_phase true for non causal
 * param[in] freq the frequency in cycles/sec
 * param[in] hicut the band high cut off frequency in cycles/sec
 * param[in] lowcut the band low cut off frequency in cycles/sec
 * param[in] samprate the sampling frequncy in cycles/sec 
 * param[in,out] *amp the amplitude adjusted for filter response
 * param[in] max_allowed_filter_correction the maximum allowed filter
 * correction. This limit should avoid filter corrections (to be applied
 * later) which are excessively large. In these cases mostly either period
 * estimation is in error or the period is too far outside of the filter band.
 * Mostly there could not be enough energy in signal to allow for as high
 * correction.
 * @author Petr Firbas
 * @version Feb 1995 test version, Jan 1996 samplerate changed to double
 *
 */
int Amp::apply_filt_resp_new2(
    int filter_order, /* filter order */
    int zero_phase,   /* flag for zero phase filter */
    double freq,      /* signal frequency */
    double hicut,     /* high cut frequency of the filter */
    double lowcut,    /* low cut frequency of the filter */
    double samprate,  /* signal sample rate */
    double *amp,      /* i/o adjusted for instrument response */

    double max_allowed_filter_correction)
		/* This limit should avoid filter corrections (to be applied
		 * later) which are excessively large. In these cases mostly
		 * either period estimation is in error or the period is too
		 * far outside of the filter band. Mostly there could not be
		 * enough energy in signal to allow for as high correction
		 */
{

    static char function_name[] = "apply_filt_resp_new2";
    char error_string[DB_ERRORMSG_SIZE_BIG + 1];
 
    double sample_int;	     /* sample interval */
    double butterworth = 1.0;/* filter correction */
    double freq_a;	     /* signal frequency transformed to analog domain */
    double hicut_a;	     /* hicut frequency transformed to analog domain */
    double lowcut_a;         /* lowcut frequency transformed to analog domain */
    double omega_norm_LP;    /* frequency normalized to standard LP */
 
    /* Test on input frequency ... this test may go somewhere up in the program
     */
    if(samprate <= 0.0) 
    {
	snprintf(error_string, sizeof(error_string),
		"Wrong signal sampling rate: %f.", samprate);
	local_warn(function_name, error_string); 

	return ERR; 
    }

    /* Test on input frequency ... this test may go somewhere up in the program
     */
    if(freq <= 0.0 || freq >= samprate / 2.0) 
    {
	snprintf(error_string, sizeof(error_string),
		"Wrong signal frequency: %f.", freq);
	local_warn(function_name, error_string);

	return ERR; 
    }
 

#ifdef TEST_PRINT
    /* Test on reasonable high cut frequency with respect to 
     * the sampling rate/Nyquist frequency and the allowed MAX_HIGH_CUT in %
     * ... this test may go somewhere up in the program
     */
    if(hicut > ((float)MAX_HIGH_CUT / 100.0) * (samprate / 2.0))
    {
	snprintf(error_string, sizeof(error_string),
	    "High cut frequency %.2f > %f.2 = fNyquist * %.2f.",
	    hicut, (MAX_HIGH_CUT / 100.0) * (samprate / 2.0),
	    (float)MAX_HIGH_CUT);
	local_warn(function_name, error_string);

	return ERR; 
    }
#endif


    /* Test on filter order ... this test may go somewhere up in the program
     */
    if(  filter_order > MAX_FILTER_ORDER || 
	(filter_order < 1 && (lowcut > 0.0 || hicut > 0.0)) ||
	(filter_order < 0 && (lowcut > 0.0 && hicut > 0.0)))
    {
	snprintf(error_string, sizeof(error_string),
	    "Filter order: %d out of limits. Max order allowed is %d", 
	    filter_order, MAX_FILTER_ORDER);
	local_warn(function_name, error_string);

	return ERR; 
    }

    /* Use sample interval
     */
    sample_int = 1.0 / samprate;

    /* For a bandpass filter
     */ 
    if((lowcut > 0.0) && (hicut > 0.0))
    {
	/* Convert the "digital frequencies to "analog" ones 
	 */
	hicut_a = tan ((double)(M_PI * hicut * sample_int)) / 
			(M_PI * sample_int);
	lowcut_a = tan ((double)(M_PI * lowcut * sample_int)) / 
			(M_PI * sample_int);
	freq_a = tan ((double)(M_PI * freq * sample_int)) / 
			(M_PI * sample_int);

	/* Convert the signal "analog" frequency to that 
	 * of standard (normalized) LP filter 
	 */ 
	omega_norm_LP = (freq_a * freq_a - hicut_a * lowcut_a) / 
			freq_a / (hicut_a - lowcut_a);

	/* Compute the amplitude adjustment factor 
	 */
	butterworth = sqrt(fabs(1.0 / 
	    (1.0 + (double)pow(omega_norm_LP, (double)(2.0 * filter_order)))));
    }
    /* For a highpass filter
     */ 
    else if((lowcut > 0.0) && (hicut <= 0.0))
    {
	/* Convert the "digital frequencies to "analog" ones 
	 */
	lowcut_a = tan ((double)(M_PI * lowcut * sample_int)) / 
			(M_PI * sample_int);
	freq_a = tan ((double)(M_PI * freq * sample_int)) / 
			(M_PI * sample_int);

	/* Convert the signal frequency to that of standard LP filter 
	 */ 
	omega_norm_LP = -1.0 * lowcut_a / freq_a;

	/* Compute the amplitude adjustment factor 
 	 */
	butterworth = sqrt(fabs(1.0 / 
	    (1.0 + (double)pow(omega_norm_LP, (double)(2.0 * filter_order)))));
    }
    /* For a lowpass filter
     */ 
    else if((lowcut <= 0.0) && (hicut > 0.0))
    {
	/* Convert the "digital frequencies to "analog" ones 
	 */
	hicut_a = tan ((double)(M_PI * hicut * sample_int)) / 
			 (M_PI * sample_int);
	freq_a = tan ((double)(M_PI * freq * sample_int)) / 
			(M_PI * sample_int);

	/* Convert the signal frequency to that of standard LP filter 
	 */ 
	omega_norm_LP = freq_a / hicut_a;

	/* Compute the amplitude adjustment factor 
	 */
	butterworth = sqrt(fabs(1.0 / 
	    (1.0 + (double)pow(omega_norm_LP, (double)(2.0 * filter_order)))));
    }
    /* Test on "no filter requested" 
     * - not considered error here, but local_warning is issued
     */
    else
    {
	/* i.e. if((lowcut <= 0.0) && (hicut <= 0.0))
	 */
	butterworth = 1.0;
 
	snprintf(error_string, sizeof(error_string),
			"No filter correction requested.");
	local_warn(function_name, error_string);
    }
 
    /* For zero phase filter use the square of the correction coefficient
     */
    if(zero_phase) {
	butterworth = butterworth * butterworth;
    }

#ifdef TEST_PRINT
    /* Do not allow too high correction coeficient, if it is computed to be too
     * high there is a danger of amplification of seismic/electronic noise
     * instead of signal. So do not use such a high value.
     */
    if(1.0 / butterworth > max_allowed_filter_correction)
    {
	snprintf(error_string, "Filter correction too high: %f. Reset to %f.",
		1.0 / butterworth, max_allowed_filter_correction);
	local_warn(function_name, error_string);

	butterworth = 1.0 / max_allowed_filter_correction;
    }
#else
    if(1.0 / butterworth > max_allowed_filter_correction)
    {
	return ERR;
    }
#endif

    /* Apply amplitude adjustment 
     */
    *amp = *amp / butterworth;
 
    return OK;
}

static void
local_warn(char *routine, char *message)
{
/*
    fprintf (stderr, "Warning in %s: %s\n", routine, message);
*/
}

/* added to be compatable with Petr's amplitude measurement function */

static void
filter2(double samprate, int nsamp, double calib, float *data, double flow,
		double fhigh, int order, const string &ftype, int zp)
{
	double dt = 1./samprate;

	IIRFilter *iir = new IIRFilter(order, ftype, flow, fhigh, dt, 1);
	iir->applyMethod(data, nsamp, 1);
	delete iir;
}

/** Get the Amp class error message.
 *  @returns the error message or NULL.
 */
char * Amp::getAmpError(void)
{
    if(amp_err) {
	amp_err = 0;
	return amp_err_msg;
    }
    return NULL;
}
