#ifndef _QC_DATA_H
#define _QC_DATA_H

#include "DataMethod.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libdataqc.h"
}

class Table;

namespace libgdataqc {

class QCData : public DataMethod
{
    public:
	QCData(const char *s);
	QCData(bool extended_type, QCDef *);
	QCData(void);
	~QCData(void);

        Gobject *clone(void);
	const char *toString(void);

	bool applyMethod(int num_waveforms, GTimeSeries **ts);
	void applyqc(GTimeSeries *ts, Table *table=NULL);

    protected:
	bool extended;
	QCDef qcdef;

	void init(const char *s);

    private:

};

} // namespace libgdataqc

#endif
