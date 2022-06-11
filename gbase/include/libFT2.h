
#ifndef _LIB_FT2_H_
#define _LIB_FT2_H_



/**
 *  these are the output values from each spectrum which is calculated, i.e., what is plotted 
 *  in the FT window in geotool. It is NOT each of the small windows which can go into the 
 *  result shown in the FT window
 */
typedef struct
{

	float		*t;		/* ? 1783 2142 2337 doubtful that it is used ? */
	float		*amp;		/* ? doubtful that it is used ? */

	float		*phase;		/* phase */
	float		*x;		/* freq values for the spectra */
	float		*y;		/* spectra in the units specified by the user  */
	float		*xPow;		/* power spectra, output from fft */

	double		mean;	
} FTVals;

/* +++++++++++++++++++++++++++++++ FT "data" +++++++++++++++++++++++++++++++++++++= */

typedef struct
{
	GObjectPart	core;

	char		sta[8];
	char		chan[10];
	double		time;		/* start time of waveform data */
	double		calper;
	double		calib;
	Vector		rsp;
	int		data_length;    /* total number of input points */
	int		npts;           /* same as winpts? */
        int             winpts;         /* number of samples for each small window */
	int		np2;		/* npts raised to the power of 2 */
	int		nf;		/* number of frequencies */


	long		wfid;		/* waveform id */
	double		dt;
	double		df;

        int             nFT;            /* number of output FT's */
        FTVals          *FT;            /* FT output data */

	double		amp_y_min, amp_y_max;
	double		phase_y_min, phase_y_max;
} _FTData, *FTData;


typedef struct
{
	double	        winlensec;      /* this is the full length of the processing window. 
                                         * if it is -1., then the whole input window is used
                                         */
	int		nwindows;       /* number of small windows to process inside winlensec */
	double	        overlapPercent; /* percent overlap between adjacent small windows */
        double          nonOverlapFraction;

	int		taper;
	float           begTaper;
	float           endTaper;
	double	        smoothvalue;
	bool		do_rsp;

	/* output in log format? */
	/* mode, der, units */
        bool            logX;   /* if true, x units are in log scale. otherwise, linear */
        int             modeY;  /* if 0 -> amp, if 1 -> power */
        int             derY;   /* if 0 -> displacement, if 1 -> velocity, if 2 -> acceleration */
        int             unitsY;


        /* the results of processing each segment of data is in one ftdata[] */
        int             num_ftdata;
        FTData          *ftdata;
} FTParam;

#ifndef FT_AMP

#define FT_AMP          0
#define FT_POWER        1
#define FT_PHASE        2

#define FT_DB_RE_NM     0
#define FT_DB_RE_M      1
#define FT_NM           2
#define FT_M            3
#define FT_LOG_NM       4
#define FT_LOG_M        5

#define FT_DISP         0
#define FT_VEL          1
#define FT_ACC          2

#endif /* FT_AMP */


#endif /* _LIB_FT2_H_ */
