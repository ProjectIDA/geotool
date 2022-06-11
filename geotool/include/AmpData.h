#ifndef _AMP_DATA_H
#define _AMP_DATA_H

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

/** A DataMethod subclass that multiplies the data values of a GTimeSeries
 *  object by a constant factor.
 *  @ingroup libgmethod
 */
class AmpData : public DataMethod
{
    public:
	AmpData(const string &s) throw(int);
	AmpData(double amp_factor, const string &cmt) : DataMethod("AmpData"),
		factor(amp_factor), comment(cmt)
	{ }
	~AmpData(void);

	const char *toString(void);

	Gobject *clone(void);

	virtual AmpData *getAmpDataInstance(void) { return this; }

	bool canAppend(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyToSegment(GSegment *s) { ampSegment(s, factor); }

	/** Get the factor.
	 *  @returns the value of the factor.
	 */
	double getFactor() { return factor; }
	/** Get the comment.
	 *  @returns the comment.
	 */
	const char *getComment() { return comment.c_str(); }

	bool Equals(AmpData *a) { return factor == a->factor; }

	static AmpData *create(const string &args);

    protected:
	double	factor;  //!< the constant amplitude factor
	string	comment; //!< the comment.

	static void ampSegment(GSegment *s, double amp_factor);

    private:

};

#endif
