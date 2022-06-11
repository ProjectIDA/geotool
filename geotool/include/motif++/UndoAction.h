#ifndef _UNDO_ACTION_H
#define _UNDO_ACTION_H

#include <string>
#include "gobject++/Gobject.h"

/** An interface for an "undo" operation.
 *  @ingroup libmotif
 */
class UndoAction : public Gobject
{
    public:
	virtual ~UndoAction(void) {}

	// defined by the subclass
	virtual bool undo(void) = 0; //!< Undo the operation.
	virtual void getLabel(string &label) = 0; //!< the undo label
	/** Get an error message. */
	virtual bool errorMsg(string &msg){ msg.clear(); return false; }

    protected:
	UndoAction(void) {}

    private:

};

#endif
