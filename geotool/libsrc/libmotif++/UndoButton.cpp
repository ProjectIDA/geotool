/** \file UndoButton.cpp
 *  \brief Defines class UndoButton.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "motif++/UndoButton.h"
#include "motif++/Application.h"

using namespace std;

/** Constructor with X-resources.
 *  @param[in] name the name given to this UndoButton instance. The name is
 * 		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
UndoButton::UndoButton(const string &name, Component *parent, Arg *args, int n)
		: Button(name, parent, args, n)
{
    Application::getApplication()->undoManager()->addUndoButton(this);
}

/** Constructor with accelerator and position.
 *  @param[in] name the name given to this UndoButton instance. The name is
 * 		the default action command string.
 *  @param[in] accel accelerator name.
 *  @param[in] parent the Component parent.
 *  @param[in] position the position of the UndoButton.
 */
UndoButton::UndoButton(const string &name, const string &accel,
		Component *parent, int position) :
		Button(name, accel, parent, position)
{
    Application::getApplication()->undoManager()->addUndoButton(this);
}

/** Destructor.
 */
UndoButton::~UndoButton(void)
{
    Application::getApplication()->undoManager()->removeUndoButton(this);
}
