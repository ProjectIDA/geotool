#ifndef _UNDO_MANAGER_H
#define _UNDO_MANAGER_H

#include "motif++/Component.h"
#include "motif++/UndoAction.h"

class Button;

/** A class that manages UndoActions
 *  @ingroup libmotif
 */
class UndoManager : public Component
{
    public:
	UndoManager(const string &name);
	~UndoManager(void);

	void addUndoAction(UndoAction *undo);
	void undoLastAction(void);
	void addUndoButton(Button *button);
	void removeUndoButton(Button *button);
	void removeAll(void);
	void actionPerformed(ActionEvent *action_event);

    protected:
	vector<UndoAction *> undo_actions; //!< the UndoActions
	vector<Button *> undo_buttons; //!< the UndoButtons

    private:

};

#endif
