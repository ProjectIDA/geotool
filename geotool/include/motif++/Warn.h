#ifndef _WARN_H_
#define _WARN_H_

#include "motif++/FormDialog.h"

class Component;
class RowColumn;
class Button;
class Toggle;

/** A FormDialog that displays a warning message. This dialog window is
 *  displayed when the Component::showWarning function is called. If more that
 *  one warning message was generated, the "More warnings" button will be
 *  sensitive. It will display the Warnings window that lists all messages.
 *  \image html Warn.gif "The Warn window "
 *  \image latex Warn.eps "The Warn window" width=3in
 *  @ingroup libmotif
 */
class Warn : public FormDialog
{
    friend class Component;

    public:
	Warn(const string &msg, Component *parent);

	void actionPerformed(ActionEvent *action_event);

    protected:
	RowColumn *row_column;
	Button *close;
	Button *more_warnings;
	//!< if this toggle is set, future warnings go to stdout only.
	Toggle *redirect;
};

#endif
