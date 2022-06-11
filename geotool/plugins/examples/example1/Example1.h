#ifndef _EXAMPLE_1_H
#define _EXAMPLE_1_H

#include "motif++/MotifDecs.h"
#include "motif++/Frame.h"
#include "DataReceiver.h"

/* Change the following defines as desired. Make sure that they do not conflict
 * with another plug-in. PLUGIN_STRING must be "PLUGIN_NAME".
 */
#define PLUGIN_NAME		Example1
#define PLUGIN_STRING		"Example1"
#define PLUGIN_CLASS		Example1Plugin
#define PLUGIN_DESCRIPTION	"Table Example Plug-in"
#define PLUGIN_BUTTON_NAME	"Example 1 (Table)..."

class DataSource;
class Table;

/** Plug-in top window.
 */
class PLUGIN_NAME : public Frame, public DataReceiver
{
    public:
	PLUGIN_NAME(const char *, Component *, DataSource *);
	~PLUGIN_NAME(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

	void compute(void);
	void clear(void);
	void removeRow(void);
	void save(void);

    protected:
	// File menu
	Button *close_button, *output_button;

	// Edit menu
	Button *clear_button, *remove_row_button;

	// Option menu
	Button *compute_button;

	// Help menu
	Button *help_button;

	Table *table;
	FileDialog *file_dialog;

	void createInterface(void);
};

#endif
