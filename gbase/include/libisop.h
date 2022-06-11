/* SccsId: @(#)libisop.h	1.1	05/24/00 */

#ifndef _LIBISOP_H_
#define	_LIBISOP_H_

void emdlv(float r, float *vs, float *vp);
void emdld(int *n, float *cpr, char *name);
int tabin(char *model);
void brnset(void);
int depset1(float dep);
int depset2(float dep);
int depset(float dep);
int DepSet(float dep);
void trtm(float delta, int *pn, float *tt, float *ray_p, float *dtdd,
			float *dtdh, float *dddp, char **phnm);
void trtm1(float delta, int *pn, float *tt, float *ray_p, float *dtdd,
			float *dtdh, float *dddp, char **phnm);
void trtm2(float delta, int *pn, float *tt, float *ray_p, float *dtdd,
			float *dtdh, float *dddp, char **phnm);
void Trtm(float delta, int *pn, float *tt, float *ray_p, float *dtdd,
			float *dtdh, float *dddp, char **phnm);
void get_seg(char *phase, int *npts, float *tt, float *delta, float *ray_p,
			int *n_branch);
void tauint(double ptk, double ptj, double pti, double zj, double zi,
			double *tau, double *x);
void tauspl(int i1, int i2, double *pt, double *c1, double *c2, double *c3,
			double *c4, double *c5);


#endif /* _LIBISOP_H_ */
