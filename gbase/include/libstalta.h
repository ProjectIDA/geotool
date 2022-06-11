#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gobject/GObject.h"

#ifndef _LIB_STALTA_H_
#define _LIB_STALTA_H_

typedef struct
{
        char    constraint[3];  /* 'z': zero phase, 'c': causal */
        char    ftype[3];       /* lp, hp, bp, or br            */
        int     forder;         /* filter order                 */
        float   lofreq;         /* low cut frequency (Hz)       */
        float   hifreq;         /* high cut frequency (Hz)      */
} FilterDef, *FilterDefP;

typedef struct staLta_def
{
        char    name[8];   	/* name of this recipe */
        double	staSec;       	/* duration short term average, in seconds */
        int	sta;       	/* duration short term average, in number of samples */
        double	ltaSec;		/* duration of long term average, in seconds */
        int	lta;		/* duration of long term average, in number of samples */
        double	bufSec;		/* separation between lta and sta, in seconds */
        int	buf;		/* separation between lta and sta, in number of samples */
        double  htrig;  	/* threshold level for trigger to be on (sta/lta) */
        double  ltrig;  	/* threshold level for trigger to be off (sta/lta) */
        double  snrThreshold;  	/* threshold level for accepting a trigger */
        double	wtrigSec;  	/* minimal trigger duration to become a trigger, in seconds */
        int	wtrig;  	/* minimal trigger duration to become a trigger, in number of samples */
        double	trgsepSec;      /* minimal time separation between end of trigger and start of the next trigger */
        int	trgsep;         /* minimal time separation between end of trigger and start of the next trigger */
        int     method;
	
	int     *ontime;
        int     *offtime;
        double  *ratio;
        double  *maxratio;
        float   *vsta;
        float   *vlta;

	char	beam_type[5];	/* coh or inc */
	char	group[16];	/* vertical or horizontal */
	int		nfilters;
	FilterDef	*f;
} StaLtaDef, *StaLtaDefP;


typedef struct
{
        char            net[10];
        char            sta[10];
	int		num_waveforms;
	bool		three_component;

        int             nbands;
	StaLtaDef	*s;

	/* parameters for grouping detections from the different bands */
} StaLtaParam;

int
allocStaltaSpace(StaLtaDef * s, int len);

void
freeStaltaSpace(StaLtaDef * s);

void
applyStaLtaSamprate(StaLtaDef * staltadef, double tdel);

int
stalta(StaLtaDef * s, int n, float *indata);

int
loadStaLtaRecipe(char *station, char *recipe_dir, StaLtaParam *p, const char **err_msg);


#endif /* _LIB_STALTA_H_ */
