#ifndef _GSE_SOURCE_H
#define _GSE_SOURCE_H

#include <string.h>
#include "TableSource.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

class GseData;

/*
 *  @ingroup libgio
 */
class GseSource : public TableSource
{
    public:
	GseSource(const string &name, const string &file);
	~GseSource(void);

	// DataSource interface
	gvector<SegmentInfo *> *getSegmentList(void);
	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	bool reread(GTimeSeries *ts);
	GSegment * readSegment(const string &full_path, int offset,
		double start_time, double end_time, int pts_wanted);

    protected:
	string read_path;
	vector<GseData *> gse_data;

    private:

};

#endif
