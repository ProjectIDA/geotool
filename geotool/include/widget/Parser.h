#ifndef _PARSER_H_
#define _PARSER_H_

#include "gobject++/DataSource.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/gvector.h"
#include "Waveform.h"
#include "gobject++/CssTableClass.h"

/** 
 *
 *  @ingroup libwgets
 */
class Parser : public DataSource
{
    public:

    Parser(void);
    ~Parser(void);

    static ParseVar getTSMember(const char *c, Waveform *w, string &value);
    static ParseVar getSegInfo(GSegment *seg, const char *c, string &value);
    protected:

    ParseCmd dataParseCmd(const char *cmd, string &msg);

    ParseVar getWaveMember(const string &name, string &value);
    ParseVar dataParse(const string &name, string &value);
    ParseVar getWaveformLoop(const char *name, string &value);
    ParseVar getWaveformCmd(const char *name, string &value);
    ParseVar getTableLoop(const char *name);
    ParseVar getTableCmd(const char *name, string &value);
    ParseVar getTableMember(const char *name, string &value);
    bool getTabMember(const char *c, CssTableClass *o, string &value);
    ParseVar waveFindIndices(const char *name, string &value);

    class WaveformParseStruct {
	public:
	int i, num;
	char name[20];
	gvector<Waveform *> wvec;
	WaveformParseStruct() {
	    i = 0;
	    num = 0;
	    memset(name, 0, sizeof(name));
	}
	~WaveformParseStruct() {
	}
    };
    WaveformParseStruct wps, wps_sel;

    class TableParseStruct {
	public:
	int i;
	bool selected;
	char name[20];
	char table_name[20];
	gvector<CssTableClass *> v;
	TableParseStruct() {
	    i = 0;
	    selected = false;
	    memset(name, 0, sizeof(name));
	    memset(table_name, 0, sizeof(table_name));
	}
	~TableParseStruct() {
	}
    };
    vector<TableParseStruct *> tps;

    private:

};

#endif
