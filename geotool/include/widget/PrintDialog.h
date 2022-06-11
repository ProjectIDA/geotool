#ifndef _PRINT_DIALOG_H_
#define _PRINT_DIALOG_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/PrintOptions.h"

class PrintClient;

/**
 *  @ingroup libwgets
 */
class PrintDialog : public FormDialog
{
    public:
	PrintDialog(const char *, Component *, PrintClient *);
	~PrintDialog(void);

	void actionPerformed(ActionEvent *action_event);
	bool print(void);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	static void parseHelp(const char *prefix);

    protected:
	PrintClient *print_comp;
	PrintOptions *print_options;
	RowColumn *rc1, *rc2, *rc3, *controls;
	Separator *sep1, *sep2;
	Label *filename_label, *command_label;
	TextField *filename_text, *command_text;
	Button *cancel_button, *print_button, *options;
	Label *left_label, *top_label, *right_label, *bottom_label;
	TextField *left_text, *top_text, *right_text, *bottom_text;
	Label *width_label, *height_label;
	TextField *width_text, *height_text;
	Choice *layout_choice, *units_choice, *paper_choice;

	void printInit(void);
	FILE *open_ps(char *, bool, bool);
};

#endif
