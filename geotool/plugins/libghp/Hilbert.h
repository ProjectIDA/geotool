#ifndef _HILBERT_H_
#define _HILBERT_H_

#include "DataMethod.h"

class GTimeSeries;
class GSegment;

namespace libghp {

class Hilbert : public DataMethod
{
    public:
	Hilbert(void);
	~Hilbert(void);

	Gobject *clone();
	const char *toString(void);

	bool applyMethod(int num_waveforms, GTimeSeries **ts);

    protected:

	static void applySegment(GSegment *s);

    private:

};

} // namespace libghp

#endif
