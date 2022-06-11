#ifndef _IIR_FILTER_H
#define _IIR_FILTER_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

typedef struct
{
    double   r;
    double   i;
} Cmplx;

/** A DataMethod subclass that filters the data of a GTimeSeries object.
 *  This is a translation of the Fortran code for iir digital filters written
 *  by Dave Harris, Lawrence Livermore National Laboratory.
 *  @ingroup libgmethod
 */
class IIRFilter : public DataMethod
{
    public:
	IIRFilter(int order, const string &type, double fl, double fh,
			double tdel, int zero_phase) throw(int);
	IIRFilter(double tdel, const string &s) throw(int);
	IIRFilter(GTimeSeries *ts, const string &s) throw(int);
        IIRFilter(const IIRFilter &f);
        IIRFilter & operator=(const IIRFilter &f);
	~IIRFilter(void);

        Gobject *clone(void);

	virtual IIRFilter *getIIRFilterInstance(void) { return this; }

	void applyMethod(GTimeSeries *ts);
	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyMethod(GSegment *s, bool reset=true);
	void applyToSegment(GSegment *s) { applyMethod(s, true); }
	bool applyMethod(float *data, int data_length, bool reset=true);
	void continueMethod(GSegment *s) { applyMethod(s, false); }
	const char *toString(void);

	/** Get the filter order.
	 *  @returns the order of the filter.
	 */
	int getOrder() { return order; }
	/** Get the filter type.
	 *  @returns the type of the filter. "BP, "BR", "LP", "HP", or "NA"
	 */
	char *getType() { return type; }
	/** Get the filter low frequency cutoff.
	 *  @returns the low frequency filter cutoff.
	 */
	double getFlow() { return flow; }
	/** Get the filter high frequency cutoff.
	 *  @returns the high frequency filter cutoff.
	 */
	double getFhigh() { return fhigh; }
	/** Get the sample time increment.
	 *  @returns the sample time increment.
	 */
	double getTdel() { return tdel; }
	/** Get the zero-phase flag.
	 *  @returns 1 for a zero phase filter, 0 for a causal filter.
	 */
	int getZeroPhase() { return zero_phase; }

	bool canAppend(void) { return true; }
	bool rotationCommutative(void) { return true; }

	bool Equals(IIRFilter *iir) {
	    return (order == iir->order && !strcmp(type, iir->type) &&
		flow == iir->flow && fhigh == iir->fhigh &&
		tdel == iir->tdel && zero_phase == iir->zero_phase);
	}

	static IIRFilter *create(double tdel, const string &args);

    protected:
	int	order;	    //!< the order of the filter
	char	type[3];    //!< the type: "BP, "BR", "LP", "HP", or "NA"
	double	flow;       //!< the filter low frequency cutoff.
	double	fhigh;      //!< the high frequency filter cutoff.
	double	tdel;       //!< the sample time increment
	int	zero_phase; //!< the zero-phase flag.
	int	nsects;     //!< the number of second-order sections.
	int	nps;	    //!< the number of butter poles.
	double	*sn; //!< numerator polynomials for second order sections.
	double	*sd; //!< denominator polynomials for second order sections.
	double	*x1; //!< nsects coefficients needed for the recursive filter
	double	*x2; //!< nsects coefficients needed for the recursive filter
	double	*y1; //!< nsects coefficients needed for the recursive filter
	double	*y2; //!< nsects coefficients needed for the recursive filter

	void init(double tdel, const string &s) throw(int);
	void init(int iord, const char *ftype, double fl, double fh,
			double tdel, int zero_phase);
	void reverse(float *data, int data_length, bool reset);
	void Reset(void);
	void bilinear(void);
	int butterPoles(Cmplx *p, char *ptype, int iord);
	void cutoffAlter(double f);
	void lowpass(Cmplx *p, const char *ptype, int np);
	void LPtoBP(Cmplx *p, const char *ptype, int np, double fl,double fh);
	void LPtoBR(Cmplx *p, const char *ptype, int np, double fl,double fh);
	void LPtoHP(Cmplx *p, const char *ptype, int np);

	void applyFilter(float *data, int data_length, int nsects, double *sn,
                double *sd, double *x1, double *x2, double *y1, double *y2);
	void doReverse(float *data, int data_length, int nsects, double *sn,
                double *sd, double *x1, double *x2, double *y1, double *y2);
	double tangent_warp(double f, double t);
	Cmplx cmplx_add(Cmplx c1, Cmplx c2);
	Cmplx cmplx_sub(Cmplx c1, Cmplx c2);
	Cmplx cmplx_mul(Cmplx c1, Cmplx c2);
	Cmplx cmplx_div(Cmplx c1, Cmplx c2);
	Cmplx cmplx(double fr, double fi);
	Cmplx cmplx_conjg(Cmplx c);
	Cmplx cmplx_sqrt(Cmplx c);
	Cmplx real_cmplx_mul(double r, Cmplx c);
	double real_part(Cmplx c);

    private:

};

#endif
