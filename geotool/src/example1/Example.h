#ifndef _EXAMPLE_H
#define _EXAMPLE_H

#include "motif++/MotifDecs.h"
#include "motif++/Frame.h"
#include "PrintClient.h"
#include "CPlotClass.h"

class PrintDialog;

/** Example Frame subclass
 */
class Example : public Frame, public PrintClient
{
    public:
	Example(const char *, Component *);
	~Example(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	void printDialog(void);
        void print(FILE *fp, PrintParam *p);
	void compute(void);
	void horizontalLine(AxesCursorCallbackStruct *p);
	void cursorPosition(void);

    protected:
	// File menu
	Button *close_button, *print_button;

	// Edit menu
	Button *clear_button;

	// Option menu
	Button *compute_button, *cursor_button;

	// Help menu
	Button *help_button;

	CPlotClass *plot;
	PrintDialog *print_dialog;

	void createInterface(void);
};

#endif
