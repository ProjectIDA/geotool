#ifndef _DIAL_CLASS_H_
#define _DIAL_CLASS_H_

#include "motif++/Component.h"

extern "C" {
#include "widget/Dial.h"
}

/** This class is the interface to the Dial Widget.
 *
 *  @ingroup libwgets
 */
class DialClass : public Component
{
    public:

	DialClass(const char *name, Component *parent, Arg *args=NULL,int n=0)
		: Component(name, parent)
	{
	    initWidget(XsdialWidgetClass, name, parent, args, n);
	}
	~DialClass(void){}
	void destroy(void) {
	    XtRemoveCallback(base_widget, XtNselect, DialClass::selectCallback,
				(XtPointer)this);
	    this->Component::destroy();
	}

	void initWidget(WidgetClass widget_class, const char *name,
			Component *parent, Arg *args,int n)
	{
	    base_widget = XtCreateManagedWidget(getName(), widget_class,
				parent->baseWidget(), args, n);
	    installDestroyHandler();
	    dial = (XsDialWidget)base_widget;
	    XtAddCallback(base_widget, XtNselect, DialClass::selectCallback,
			(XtPointer)this);
	    enableCallbackType(XtNselect);
	}
	void setPosition(int position) {
	    Arg args[1];
	    XtSetArg(args[0], XtNposition, position);
	    setValues(args, 1);
	}

    protected:
	// use this constructor in a "Widget" subclass (ie CPlot, ConPlot, etc)
	DialClass(WidgetClass widget_class, const char *name, Component *parent,
		Arg *args, int n) : Component(name, parent)
	{
	    initWidget(widget_class, name, parent, args, n);
	}

	XsDialWidget	dial;

    private:

	static void selectCallback(Widget w, XtPointer client, XtPointer data) {
	    DialClass *dial = (DialClass *)client;
	    dial->doCallbacks(w, data, XtNselect);
	}
};

#endif
