#ifndef DETECTION30_H
#define DETECTION30_H

typedef struct {
	long	arid;
	long	jdate;
	double	time;
	char	sta[7];
	char	chan[9];
	char	bmtyp[5];
	long	sproid;
	double	cfreq;
	double	seaz;
	double	delaz;
	double	slow;
	double	delslo;
	double	snr;
	double	stav;
	double	fstat;
	double	deltim;
	double	bandw;
	long	fkqual;
	long	commid;
	char	lddate[18];
} DETECTION30;

#endif /* DETECTION30_H */
