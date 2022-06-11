#ifndef _DEMEAN_H_
#define _DEMEAN_H_

#include "DataMethod.h"

class GTimeSeries;

/** A DataMethod subclass that removes the mean from a GTimeSeries object.
 *  @ingroup libgmethod
 */
class Demean : public DataMethod
{
    public:
	Demean(void);
	~Demean(void);

	const char *toString(void);

        Gobject *clone(void);

	virtual Demean *getDemeanInstance(void) { return this; }

	bool canAppend(void) { return false; }

	bool applyMethod(int num_waveforms, GTimeSeries **ts);

	static Demean *create(const string &args);


    private:

};

#endif
