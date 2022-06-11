#ifndef _HILBERT_TRANSFORM_H
#define _HILBERT_TRANSFORM_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"

/** @defgroup libghp plugin Hilbert Transform
 */

class DataSource;
class TaperWindow;
class TaperData;

namespace libghp {

/** Hilbert Transform window.
 *  @ingroup libghp
 */
class HilbertTransform : public FormDialog, public DataReceiver
{
    public:
	HilbertTransform(const char *, Component *, DataSource *);
	~HilbertTransform(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	RowColumn *controls, *rc;
	Button *close_button, *apply_button, *unfilter_button, *help_button;
	Button *taper_button;
	Separator *sep;
	Label *label;
	RadioBox *input_rb;
	Toggle *all_toggle, *selected_toggle;
	TaperWindow *taper_window;

	void actionPerformed(ActionEvent *action_event);
	void createInterface(void);
	void apply(void);
	void unfilter(void);
	TaperData *getTaper(void);

    private:

};

} // namespace libghp

#endif
