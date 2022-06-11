#ifndef _WARNINGS_H_
#define _WARNINGS_H_

#include "motif++/FormDialog.h"

class Component;
class RowColumn;

/** A FormDialog that displays all warning messages. This dialog window is
 *  displayed when the "More warnings" button of the Warn window is selected.
 *  \image html Warnings.gif "The Warnings window "
 *  \image latex Warnings.eps "The Warnings window" width=3in
 *  @ingroup libmotif
 */
class Warnings : public FormDialog
{
    friend class Component;

    public:
	Warnings(Component *parent);

	void actionPerformed(ActionEvent *action_event);

    protected:
	RowColumn	*controls;
	Widget		sw;
	Widget		text;
	Widget		close;
	Widget		redirect;

	Widget		show_warn;
	Widget		more_warnings;
	int		warn_no;
	int		last_n;
	char		*last;
	XmTextPosition  pos;

	static void redirectWarnings(Widget, XtPointer, XtPointer);
	void append(char *warning);
	void adjustScrollbar(void);

        // cannot actually copy the Warnings instance. This is just to avoid
        // compiler warnings from the -Weffc++ option on this file
        Warnings(const Warnings &);
        Warnings & operator=(const Warnings &a) { return *this; }
};

#endif
