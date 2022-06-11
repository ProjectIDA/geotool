#ifndef UNDO_BUTTON_H
#define UNDO_BUTTON_H

#include "motif++/Button.h"

/** A Button subclass for UndoActions
 *  @ingroup libmotif
 */
class UndoButton : public Button
{
    public:

	/** Constructor. */
	UndoButton(const string &name, Component *parent, Arg *args=NULL, int n=0);
	/** Constructor with accelerator and position. **/
	UndoButton(const string &name, const string &accel, Component *parent,
		int position=-1);

	/** Destructor. */
	~UndoButton(void);

    protected:

    private:
};

#endif
