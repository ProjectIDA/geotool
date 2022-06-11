/** \file RowColumn.cpp
 *  \brief Defines class RowColumn.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/RowColumn.h"
#include "motif++/Button.h"
#include "motif++/Application.h"

/** Constructor.
 *  @param[in] name the name given to this RowColumn instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 *  @param[in] manage if true, manage the widget. if false, do not manage it.
 */
RowColumn::RowColumn(const string &name, Component *parent, Arg *args, int n,
		bool manage) : Component(name, parent)
{
    if(manage) {
	base_widget = XtCreateManagedWidget((char *)getName(),
				xmRowColumnWidgetClass,
				parent->baseWidget(), args, n);
    }
    else {
	base_widget = XtCreateWidget((char *)getName(), xmRowColumnWidgetClass,
				parent->baseWidget(), args, n);
    }
    installDestroyHandler();
    warning_button = NULL;
}

/** Set a Help Component. Sets XmNmenuHelpWidget.
 *  @param[in] comp the help Component.
 */
void RowColumn::setHelp(Component *comp) {
    Arg args[1];
    XtSetArg(args[0], XmNmenuHelpWidget, comp->baseWidget());
    setValues(args, 1);
}

/** Make a warning button visible. A Button labeled " ! " is inserted in the
 *  last position of the RowColumn.
 */
void RowColumn::warningButtonOn(void)
{
    if(!warning_button) {
	Arg args[1];
	short pos = 0;

	XtSetArg(args[0], XmNpositionIndex, pos);
	warning_button = new Button(" ! ", this, args, 1, this);
    }
    else {
	warning_button->setVisible(true);
    }
}

/** Process the warning_button action. When the warning_button is selected,
 *  display the Application warning window and hide the warning_button.
 */
void RowColumn::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == warning_button) {
	Application::displayWarnings();
	warning_button->setVisible(false);
    }
}

