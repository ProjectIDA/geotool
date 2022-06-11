#ifndef _EXAMPLE_3_H
#define _EXAMPLE_3_h

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"

/* Change the following defines as desired. Make sure that they do not conflict 
 * with another plug-in. PLUGIN_STRING must be "PLUGIN_NAME".
 */
#define PLUGIN_NAME  		Example3
#define PLUGIN_STRING 		"Example3"
#define PLUGIN_CLASS  		Example3Plugin
#define PLUGIN_DESCRIPTION 	"Hilbert Transform Example Plug-in"
#define PLUGIN_BUTTON_NAME 	"Example 3 (Hilbert Transform)..."

class DataSource;

class PLUGIN_NAME : public FormDialog, public DataReceiver
{
    public:
	PLUGIN_NAME(const char *, Component *, DataSource *);
	~PLUGIN_NAME(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &err_msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

	void apply(void);
	void unfilter(void);

    protected:
	RowColumn *controls, *rc;
	Button *close_button, *apply_button, *unfilter_button, *help_button;
	Separator *sep;
	Label *label;
	RadioBox *input_rb;
	Toggle *all_toggle, *selected_toggle;

	void createInterface(void);
};

#endif
