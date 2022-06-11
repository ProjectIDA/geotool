#ifndef _LIB_CALIB_H_
#define _LIB_CALIB_H_
#include <sys/param.h>

typedef struct
{
	char	title[200];
	int	npts;
	int	type;
	double	tmin;
	double	tsec;
	double	dt;
	double	*x;
} CalibSignal;

#define INPUT_SIGNAL	0
#define OUTPUT_SIGNAL	1

#define CALIB_SIGNAL_NULL \
{ \
	"",		/* title */ \
	INPUT_SIGNAL,	/* type */ \
	0,		/* npts */ \
	0.,		/* tmin */ \
	0.,		/* tsec */ \
	0.,		/* dt */ \
	NULL,		/* x */ \
}

#define msys 25
#define mpar 12

typedef struct CalibOut_s
{
	int	npts;
	int	iter;
	int	converged;
	int	return_x;
	double	dt;
	double	*ausf;
	double	*einf;
	double	*synt1;
	double	*synt2;
	double	*rest;

	int	npar;
	double	initial_rms;
	double	final_rms;
	double	final_par[mpar];

	double	*x;
} CalibOut;

#define CALIB_OUT_NULL \
{ \
	0,		/* npts */ \
	0,		/* iter */ \
	0,		/* converged */ \
	1,		/* return_x */ \
	0.,		/* dt */ \
	NULL,		/* ausf */ \
	NULL,		/* einf */ \
	NULL,		/* synt1 */ \
	NULL,		/* synt2 */ \
	NULL,		/* rest */ \
	0,		/* npar */ \
	0.,		/* initial_rms */ \
	0.,		/* final_rms */ \
	{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},	/* final_par */ \
	NULL,		/* x */ \
}

typedef struct CalibParam_s
{
	char	parfile[MAXPATHLEN+1];
	char	partitle[200];
	char	eing[MAXPATHLEN+1];
	char	ausg[MAXPATHLEN+1];
	double	alias;
	int	m;
	int	m0i;
	int	m1i;
	int	m2i;
	int	maxit;
	double	qac;
	double	finac;
	int	ns1;
	int	ns2;

	double	x0[msys];
	double	rho[msys];
	char	typ[msys][4];

	char	name[mpar][4];
	double	x00[mpar];
	double	r00[mpar];

	char	raw[msys*50];
	int	lineno[msys];
} CalibParam;


typedef struct
{
	double	syspar[msys+1];
	double	sr[msys];
	double	si[msys];
	double	resid[msys];
	double	residi[msys];
	double	d[mpar];
	int	m0, m1, m2;

	double	step;
	double	axi;
	double	qn;
	int	init;

	double	dt;

	double	*aus;
	double	*sum;
	double	*einf;
	double	*sta;
} CalibSys;


/* ****** calex.c ********/
int CalibCompute(CalibParam *p, double dt, int npts, double *eing, double *ausg,
		CalibOut *co);
void CalibSetParam(CalibParam *p, double dt);
int CalibReadSignal(char *file, int type, CalibSignal *cs);
int CalibCongrd(CalibSys *s, CalibParam *p, double *x, double *q,
		double *gnorm, int iter, double *ein);
int CalibQuadr(CalibSys *s, CalibParam *p, double *x, double *quad,double *ein);
int CalibOutput(char *file, char *text, double *x, int npts, double dt,
		double tmin, double tsec);


/* ****** inpar.c ********/
int CalibReadPar(char *file, CalibParam *p);
int CalibGetStartParam(CalibParam *p);


#endif
