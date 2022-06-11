/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_GEO_H
#define	_LIB_GEO_H


/* ****** libgeo/nmrd_reg.c ********/

int greg(int number, char *rgnname);
void greg_(long int *number, char *rgnname, long int nmlnth);
int sreg(int number, char *rgnname);
void sreg_(long int *number, char *rgnname, long int nmlnth);
int nmreg(double latit, double longit);
long nmreg_(float *latit, float *longit);
int gtos(int grn);
long gtos_(int *grn);

#endif /* _LIB_GEO_H */
