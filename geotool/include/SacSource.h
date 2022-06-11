#ifndef _SAC_SOURCE_H
#define _SAC_SOURCE_H

#include <string.h>
#include <inttypes.h>
#include "TableSource.h"
#include "gobject++/cvector.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

class SacData;

/** 
 *  @ingroup libgio
 */
class SacSource : public TableSource
{
    public:
	SacSource(const string &name, const string &file);
	~SacSource(void);

	// DataSource interface
	gvector<SegmentInfo *> *getSegmentList(void);
	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	bool reread(GTimeSeries *ts);

    protected:
	string read_path;
	bool css_arrival;
	vector<SacData *> sac_data;

	GSegment * readSeg(SegmentInfo *s, float *data, double calib,
		double start_time, double end_time);
};

#endif
