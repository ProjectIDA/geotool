#ifndef _ROW_COLUMN_H
#define _ROW_COLUMN_H

#include "motif++/Component.h"

class Button;

/** A class for the XmRowColumn widget.
 *  @ingroup libmotif
 */
class RowColumn : public Component
{
    public:

	RowColumn(const string &name, Component *parent, Arg *args=NULL,
		int n=0, bool manage=true);
	~RowColumn(void) {}

	virtual RowColumn *getRowColumnInstance(void) { return this; }

	void setHelp(Component *comp);
	void warningButtonOn(void);
	void actionPerformed(ActionEvent *action_event);

    protected :
	Button *warning_button;
};

#endif
