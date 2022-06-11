/** \file ArrowButton.cpp
 *  \brief Defines class ArrowButton.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>
#include "motif++/ArrowButton.h"

/** Constructor. The Component listener is notified of any
 *  button events on the ArrowButton via the listener's actionPerformed
 *  function. The ActionEvent reason is XmNactivateCallback. The ActionEvent
 *  callback data is a pointer to the XmArrowButtonCallbackStruct structure:
\verbatim
       typedef struct
       {
               int reason;
               XEvent * event;
               int click_count;
       } XmArrowButtonCallbackStruct;\endverbatim
 *  @param[in] name the name given to this ArrowButton instance. The name is
 *		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] arrow_direction the arrow direction: XmARROW_UP, XmARROW_DOWN,
 *		XmARROW_LEFT, or XmARROW_RIGHT.
 *  @param[in] listener for XmNactivateCallback actions.
 */
ArrowButton::ArrowButton(const string &name, Component *parent,
	int arrow_direction, ActionListener *listener) : Component(name, parent)
{
    direction = arrow_direction;
    init(parent);
    if(listener) addActionListener(listener, XmNactivateCallback);
}

/** Create the ArrowButon widget.
 *  @param[in] parent
 */
void ArrowButton::init(Component *parent)
{
    comp_parent = parent;
    base_widget = XtVaCreateManagedWidget(getName(),
			xmArrowButtonWidgetClass, parent->baseWidget(),
			XmNarrowDirection, direction,
			NULL);
    installDestroyHandler();
    XtAddCallback(base_widget, XmNactivateCallback,
		ArrowButton::activateCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
}

/** Constructor with default arrow direction.
 *  @param[in] name the name given to this ArrowButton instance. The name is
 *		the default action command string.
 *  @param[in] parent the Component parent.
 */
ArrowButton::ArrowButton(const string &name, Component *parent)
		: Component(name, parent)
{
    comp_parent = parent;
    base_widget = XtVaCreateManagedWidget(name.c_str(),xmArrowButtonWidgetClass,
			parent->baseWidget(), NULL);
    installDestroyHandler();
    XtAddCallback(base_widget, XmNactivateCallback,
		ArrowButton::activateCallback, (XtPointer)this);
    enableCallbackType(XmNactivateCallback);

    Arg args[1];
    XtSetArg(args[0], XmNarrowDirection, &direction);
    getValues(args, 1);
}

/** Constructor with X-resources and listener.
 *  @param[in] name the name given to this ArrowButton instance. The name is
 *		the default action command string.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] num the number of X-resource structures.
 *  @param[in] listener for XmNactivateCallback actions.
 */
ArrowButton::ArrowButton(const string &name, Component *parent, Arg *args,
		int num, ActionListener *listener) : Component(name, parent)
{
    comp_parent = parent;
    base_widget = XtCreateManagedWidget(name.c_str(), xmArrowButtonWidgetClass,
			parent->baseWidget(), args, num);
    installDestroyHandler();
    XtAddCallback(base_widget, XmNactivateCallback,
		ArrowButton::activateCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener, XmNactivateCallback);
}

/** Destructor.
 */
ArrowButton::~ArrowButton(void)
{
    if(base_widget) {
	XtRemoveAllCallbacks(base_widget, XmNactivateCallback);
    }
}

void ArrowButton::activateCallback(Widget w, XtPointer clientData,
				XtPointer calldata)
{
    ArrowButton *button = (ArrowButton *)clientData;

    button->doCallbacks(w, calldata);
}
