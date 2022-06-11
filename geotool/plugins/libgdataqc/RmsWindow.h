#ifndef _RMS_WINDOW_H
#define _RMS_WINDOW_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "WaveformPlot.h"

namespace libgdataqc {


/** RMS window.
 *  @ingroup libgdataqc
 */
class RmsWindow : public FormDialog, public DataReceiver
{
    public:
	RmsWindow(const char *, Component *, DataSource *ds);
	~RmsWindow(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	WaveformPlot	*wp;

	Label *label;
	TextField *text;
	Separator *sep;
	RowColumn *controls;
	Button *apply_button, *unfilter_button, *close_button;
	Button *help_button;

	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void apply(void);
	void unfilter(void);

    private:

};

} // namespace libgdataqc

#endif
