/** \file UndoManager.cpp
 *  \brief Defines class UndoManager.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/UndoManager.h"
#include "motif++/Button.h"

/** Constructor
 */
UndoManager::UndoManager(const string &name) : Component(name, NULL)
{
}

/** Destructor. */
UndoManager::~UndoManager(void)
{
}

/** Add an UndoAction to the stack.
 */
void UndoManager::addUndoAction(UndoAction *undo)
{
    undo_actions.push_back(undo);
    for(int i = 0; i < (int)undo_buttons.size(); i++) {
	Button *b = undo_buttons.at(i);
	string label;
	undo->getLabel(label);
	b->setLabel(label);
	b->setSensitive(true);
    }
}

/** Undo the last action.
 */
void UndoManager::undoLastAction(void)
{
    if((int)undo_actions.size() > 0) {
	UndoAction *undo = undo_actions.back();
	undo->undo();
	delete undo;
	undo_actions.pop_back();
    }
    else return;

    if((int)undo_actions.size() > 0) {
	UndoAction *undo = undo_actions.back();

	for(int i = 0; i < (int)undo_buttons.size(); i++) {
	    Button *b = undo_buttons.at(i);
	    string label;
	    undo->getLabel(label);
	    b->setLabel(label);
	    b->setSensitive(true);
	}
    }
    else {
	for(int i = 0; i < (int)undo_buttons.size(); i++) {
	    Button *b = undo_buttons.at(i);
	    b->setLabel("Undo");
	    b->setSensitive(false);
	}
    }
}

/** Add a Button
 */
void UndoManager::addUndoButton(Button *button)
{
    int i;
    for(i = 0; i < (int)undo_buttons.size(); i++) {
	if(undo_buttons.at(i) == button) break;
    }
    if( i == (int)undo_buttons.size() )
    {
	button->addActionListener(this);
	undo_buttons.push_back(button);
	if((int)undo_actions.size() > 0) {
	    UndoAction *undo = undo_actions.back();
	    string label;
	    undo->getLabel(label);
	    button->setLabel(label);
	    button->setSensitive(true);
	}
	else {
	    button->setSensitive(false);
	}
    }
}

/** Remove a Button
 */
void UndoManager::removeUndoButton(Button *button)
{
    for(int i = 0; i < (int)undo_buttons.size(); i++) {
	if(undo_buttons.at(i) == button) {
	    undo_buttons.erase(undo_buttons.begin()+i);
	    return;
	}
    }
}

void UndoManager::removeAll(void)
{
    for(int i = 0; i < (int)undo_actions.size(); i++) {
        delete undo_actions[i];
    }
    undo_actions.clear();

    for(int i = 0; i < (int)undo_buttons.size(); i++) {
	Button *b = undo_buttons.at(i);
	b->setLabel("Undo");
	b->setSensitive(false);
    }
}

void UndoManager::actionPerformed(ActionEvent *action_event)
{
    undoLastAction();
}
