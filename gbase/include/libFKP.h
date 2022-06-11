/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_FKP_H_
#define _LIB_FKP_H_

#include "libFK.h"
#include "libmath.h"

typedef struct
{
	int		*f_lo;
	int		*f_hi;
	int		t_size;
	int		taper_size;
	int		fsum_size;
	int		f_size;
	int		t0_size;
	int		csa_size;
	int		lags_size;

	FComplex	*t;
	float		*taper;
	float		*fsum;
	float		**f;
	CPlotData	**cd_list;
	double		*lat;
	double		*lon;
	double		*t0;
	double		*csa;
	double		*sna;
	double		*lags;
} WorkSpace;


typedef struct FKGram_struct
{
	GObjectPart	core;

	int		num_waveforms;
	int		*waveform_ids;
	int		windowed;
	int		n_slowness;
	double		d_slowness;
	double		slowness_min;
	double		slowness_max;
	int		nbands;
	double		fmin[MAXNUMFK];
	double		fmax[MAXNUMFK];
	double		window_length;
	double		window_overlap;

	int		n;
	int		nf;
	int		if1;
	int		if2;
	int		dk;
	int		window_width;
	int		window_npts;
	double		time0;
	double		t0_min;
	double		dt;
	double		df;
	double		domega;
	double		center_lat;
	double		center_lon;
	char		center_sta[10];
	bool		show_working;
	bool		*peakMask;

	WorkSpace	ws;
} FK_Gram;

typedef struct FKGram3C_struct
{
	GObjectPart	core;

	int		waveform_ids[3];
	int		windowed;
	int		n_slowness;
	double		d_slowness;
	double		slowness_min;
	double		slowness_max;
	int		nbands;
	double		fmin[MAXNUMFK];
	double		fmax[MAXNUMFK];
	double		window_length;
	double		window_overlap;

	double		p_site[3][3];
	float		*fptr[3];
	float		hang[3];
	float		vang[3];
	int		f_size;
	float		*f;
	int		dk;
	int		window_width;
	int		window_npts;
	double		total_power;
	double		tlen;
	double		time0;
	double		t0_min;
	double		dt;
	double		center_lat;
	double		center_lon;
	char		center_sta[10];
	CPlotData	*cd_list[3];

	bool		show_working;
} FK_Gram3C;

#endif
