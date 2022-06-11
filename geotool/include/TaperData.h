#ifndef _TAPER_DATA_H
#define _TAPER_DATA_H

#include "DataMethod.h"

class GTimeSeries;

/** A DataMethod subclass that tapers the data values of a GTimeSeries.
 *  The following types of tapers are included: Hamming, Hanning, Cosine,
 *  Cosine at Beginning only, Parzen, Welch, Blackman, None.
 *  @ingroup libgmethod
 */
class TaperData : public DataMethod
{
    public:
	TaperData(const string &s);
	TaperData(const string &taper_type, int taper_width, int taper_minpts,
			int taper_maxpts);
	~TaperData(void);

	const char *toString(void);

        Gobject *clone(void);

	virtual TaperData *getTaperDataInstance(void) { return this; }

	bool canAppend(void) { return !type.compare("cosineBeg"); }
	bool rotationCommutative(void) { return true; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	bool applyMethod(GTimeSeries *ts);
	/** Get the taper type.
	 *  @returns the taper type. Valid types are:
	 *   - hamming
	 *   - hanning
	 *   - cosine  (apply a cosine taper to the beginning and end of the
	 *		waveform)
	 *   - cosineBeg (apply a cosine taper to the beginning only)
	 *   - parzen
	 *   - welch
	 *   - blackman
	 *   - none
	 */ 
	const char *getType() { return type.c_str(); }
	/** Get the width of the cosine taper or the cosineBeg taper. The width
	 *  is a percent of the waveform length.
	 *  @returns the taper width.
	 */
	int getWidth() { return width; }
	/** Get the minimum points for the cosine taper or the cosineBeg taper.
	 *  @returns the minimum points.
	 */
	int getMinpts() { return minpts; }
	/** Get the maximum points for the cosine taper or the cosineBeg taper.
	 *  @returns the maximum points.
	 */
	int getMaxpts() { return maxpts; }

	bool Equals(TaperData *t) {
	    if(!type.compare(t->type)) {
		if(!type.compare("cosine")) {
		    return (width == t->width && minpts == t->minpts
				&& maxpts == t->maxpts);
		}
		else return true;
	    }
	    return false;
	}

	static TaperData *create(const string &args);

    protected:
	/** the taper type. Valid types are
	 *   - hamming
	 *   - hanning
	 *   - cosine
	 *   - cosineBeg
	 *   - parzen
	 *   - welch
	 *   - blackman
	 *   - none
	 */ 
	string	type;
	/** the width of the cosine taper as a % of the waveformlength */
	int	width;
	/** The minimum length of the cosine or cosineBeg taper. */
	int	minpts;
	/** The maximum length of the cosine or cosineBeg taper. */
	int	maxpts;

	void init(const string &type, int width, int minpts, int maxpts);

    private:

};

#endif
