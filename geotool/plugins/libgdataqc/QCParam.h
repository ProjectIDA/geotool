#ifndef _QC_PARAM_H
#define _QC_PARAM_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "WaveformPlot.h"
#include "libdataqc.h"

/** @defgroup libgdataqc plugin Data QC
 */

class Table;

namespace libgdataqc {

/** QC Parameters window.
 *  @ingroup libgdataqc
 */
class QCParam : public Frame, public DataReceiver
{
    public:
	QCParam(const char *, Component *, DataSource *ds);
	~QCParam(void);

	void setDataSource(DataSource *ds);

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	WaveformPlot	*wp;

	bool extended;
	QCDef qcdef;

	// File menu
	Button *save_button, *close_button;

	// Option menu
	Button *apply_button;
	Toggle *qc_input_toggle;

	// Help menu
	Button *help_button;

	Table *table, *gap_table;

	void createInterface(void);
	void list(void);
	void actionPerformed(ActionEvent *action_event);
	void apply(void);
	void apply(DataQCStruct *dq);
	bool getParam(void);
	void initParams(void);
	void save(void);

    private:

};

} // namespace libgdataqc

#endif
