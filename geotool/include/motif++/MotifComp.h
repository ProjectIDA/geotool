#ifndef MOTIF_COMP_H
#define MOTIF_COMP_H

#include "motif++/Component.h"
#include "motif++/Menu.h"
#include "motif++/Toggle.h"
#include "motif++/TopWindow.h"
#include "motif++/Parse.h"

/** A class for the XmForm widget
 *  @ingroup libmotif
 */
class Form : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this Form instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] manage if true, create and manage the Form. If false,
	 *	create the Form but do not manage it.
	 */
	Form(const string &name, Component *parent, bool manage_widget=true) :
		Component(name, parent)
	{
	    if(manage_widget) {
		base_widget = XtVaCreateManagedWidget(component_name.c_str(),
			xmFormWidgetClass, parent->baseWidget(), NULL);
	    }
	    else {
		base_widget = XtVaCreateWidget(component_name.c_str(),
			xmFormWidgetClass, parent->baseWidget(), NULL);
	    }
	    installDestroyHandler();
	}
	/** Constructor.
	 *  @param[in] name name given to this Form instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 *  @param[in] manage if true, create and manage the Form. If false,
	 *	create the Form but do not manage it.
	 */
	Form(const string &name, Component *parent, Arg *args, int n,
		bool manage=true) : Component(name, parent)
	{
	    if(manage) {
		base_widget = XtCreateManagedWidget(component_name.c_str(),
			xmFormWidgetClass, parent->baseWidget(), args, n);
	    }
	    else {
		base_widget = XtCreateWidget(component_name.c_str(),
			xmFormWidgetClass, parent->baseWidget(), args, n);
	    }
	    installDestroyHandler();
	}

	/** Destructor. */
	~Form(void){}
};

/** A class for the XmLabel widget.
 *  @ingroup libmotif
 */
class Label : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this Label instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	Label(const string &name, Component *parent, Arg *args=NULL, int n=0) :
		Component(name, parent)
	{
	    if(args && n) {
		base_widget = XtCreateManagedWidget(component_name.c_str(),
			xmLabelWidgetClass, parent->baseWidget(), args, n);
	    }
	    else {
		base_widget = XtVaCreateManagedWidget(component_name.c_str(),
			xmLabelWidgetClass, parent->baseWidget(), NULL);
	    }
	    installDestroyHandler();
	}

	~Label(void){}

	virtual Label *getLabelInstance(void) { return this; }

	/** Set the label string.
	 *  @param[in] s the label
	 */
	void setLabel(const string &s) {
	    Arg args[1];
	    XmString xm;
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(base_widget, args, 1);
	    char *c = getXmString(xm);
	    XmStringFree(xm);
	    if(s.compare(c)) {
		xm = createXmString(s.c_str());
		XtSetArg(args[0], XmNlabelString, xm);
		XtSetValues(base_widget, args, 1);
		XmStringFree(xm);
	    }
	    free(c);
	}
	/** Get the label string.
	 *  @returns the label string. Free it when it is no longer needed.
	 */
	char *getLabel(void) {
	    Arg args[1];
	    XmString xm;
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(base_widget, args, 1);
	    char *s =  getXmString(xm);
	    XmStringFree(xm);
	    return s;
	}

	void getLabel(string &s) {
	    Arg args[1];
	    XmString xm;
	    XtSetArg(args[0], XmNlabelString, &xm);
	    XtGetValues(base_widget, args, 1);
	    char *label =  getXmString(xm);
	    XmStringFree(xm);
	    s.assign(label);
	    free(label);
	}
};

/** A class for the XmMainWindow widget.
 *  @ingroup libmotif
 */
class MainWindow : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this MainWindow instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 *  @param[in] manage if true, create and manage the MainWindow. If
	 *	false, create the MainWindow but do not manage it.
	 */
	MainWindow(const string &name, Component *parent, Arg *args=NULL,
		int n=0, bool manage=true) : Component(name, parent)
	{
	    if(manage) {
		base_widget = XtCreateManagedWidget(component_name.c_str(),
			xmMainWindowWidgetClass, parent->baseWidget(), args, n);
	    }
	    else {
		base_widget = XtCreateWidget(component_name.c_str(),
			xmMainWindowWidgetClass, parent->baseWidget(), args, n);
	    }
	    installDestroyHandler();
	}

	~MainWindow(void){}
};

/** A class for the XmScrolledWindow widget.
 *  @ingroup libmotif
 */
class ScrolledWindow : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this ScrolledWindow instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 *  @param[in] manage if true, create and manage the ScrolledWindow. If
	 *	false, create the ScrolledWindow but do not manage it.
	 */
	ScrolledWindow(const string &name, Component *parent, Arg *args=NULL,
			int n=0, bool manage=true) : Component(name, parent)
	{
	    if(manage) {
		base_widget = XtCreateManagedWidget(component_name.c_str(),
		    xmScrolledWindowWidgetClass, parent->baseWidget(), args, n);
	    }
	    else {
		base_widget = XtCreateWidget(component_name.c_str(),
		    xmScrolledWindowWidgetClass, parent->baseWidget(), args, n);
	    }
	    installDestroyHandler();
	}

	~ScrolledWindow(void){}
};

/** A class for the XmSeparator widget.
 *  @ingroup libmotif
 */
class Separator : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this Separator instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	Separator(const string &name, Component *parent, Arg *args=NULL,
			int n=0) : Component(name, parent)
	{
	    base_widget = XtCreateManagedWidget(component_name.c_str(),
			xmSeparatorWidgetClass, parent->baseWidget(), args, n);
	    installDestroyHandler();
	}
	/** Constructor.
	 *  @param[in] name name given to this Separator instance.
	 *  @param[in] menu the Menu parent.
	 */
	Separator(const string &name, Menu *menu) : Component(name, menu)
	{
	    base_widget = XtVaCreateManagedWidget(component_name.c_str(),
			xmSeparatorWidgetClass, menu->baseWidget(), NULL);
//	    menu->items->addElement(this);
	    installDestroyHandler();
	}
};

/** A class for XmCreateRadioBox
 *  @ingroup libmotif
 */
class RadioBox : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this RadioBox instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	RadioBox(const string &name, Component *parent, Arg *args=NULL,
			int n=0) : Component(name, parent)
	{
	    base_widget = XmCreateRadioBox(parent->baseWidget(),
				(char *)component_name.c_str(), args, n);
	    XtManageChild(base_widget);
	    installDestroyHandler();
	}
	ParseCmd parseCmd(const string &cmd, string &msg)
	{
	    for(int i = 0; i < (int)comp_children.size(); i++) {
		if(Parse::parseCompare(cmd, comp_children[i]->getName())) {
		    ((Toggle *)comp_children[i])->set(true, true);
		    return COMMAND_PARSED;
		}
	    }
	    msg.assign(getName() + string(": invalid choice: ") + cmd);
	    return ARGUMENT_ERROR;
	}
};

/** A message dialog class that uses XmCreateMessageDialog. The dialog window
 *  is automatically destroyed when it is hidden.
 *  @ingroup libmotif
 */
class MessageDialog : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this MessageDialog instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] msg the message
	 */
	MessageDialog(const string &name, Component *parent, const string &msg)
			: Component(name, parent)
	{
	    XmString xm1 = createXmString(msg.c_str());
	    XmString xm2 = createXmString("Close");
	    XmString xm3 = createXmString(name);
	    Arg args[3];
	    XtSetArg(args[0], XmNmessageString, xm1);
	    XtSetArg(args[1], XmNokLabelString, xm2);
	    XtSetArg(args[2], XmNdialogTitle, xm3);

	    base_widget = XmCreateMessageDialog(parent->baseWidget(),
				(char *)component_name.c_str(), args, 3);
	    XmStringFree(xm1); XmStringFree(xm2); XmStringFree(xm3);
	    XtManageChild(base_widget);
	    Widget w;
	    if((w = XtNameToWidget(base_widget, "*Cancel"))) XtUnmanageChild(w);
	    if((w = XtNameToWidget(base_widget, "*Help"))) XtUnmanageChild(w);
	    
	    installDestroyHandler();
	}

	virtual void setVisible(bool visible) {
	    this->Component::setVisible(visible);
	    if(!visible) {
		destroy();
	    }
	}
};

/** A class for the XmPanedWindow widget
 *  @ingroup libmotif
 */
class Pane : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this Pane instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	Pane(const string &name, Component *parent, Arg *args=NULL, int n=0) :
			Component(name, parent)
	{
	    base_widget = XtCreateManagedWidget(component_name.c_str(),
		xmPanedWindowWidgetClass, parent->baseWidget(), args, n);
	    installDestroyHandler();
	}
};

/** A class for the XmFileSelectionBox widget
 *  @ingroup libmotif
 */
class FileSelectionBox : public Component
{
    public:
	FileSelectionBox(const string &name, Component *parent, Arg *args=NULL,
			int n=0) : Component(name, parent)
	{
	    base_widget = XtCreateManagedWidget(component_name.c_str(),
		xmFileSelectionBoxWidgetClass, parent->baseWidget(), args, n);
	    installDestroyHandler();
	}
	/** Get a widget child of the FileSelectedBox widget. Values for 
	 *  child_name are:
	 *	- XmDIALOG_APPLY_BUTTON
	 *	- XmDIALOG_CANCEL_BUTTON
	 *	- XmDIALOG_DEFAULT_BUTTON
	 *	- XmDIALOG_DIR_LIST
	 *	- XmDIALOG_DIR_LIST_LABEL
	 *	- XmDIALOG_FILTER_LABEL
	 *	- XmDIALOG_FILTER_TEXT
	 *	- XmDIALOG_HELP_BUTTON
	 *	- XmDIALOG_LIST
	 *	- XmDIALOG_LIST_LABEL
	 *	- XmDIALOG_OK_BUTTON
	 *	- XmDIALOG_SELECTION_LABEL
	 *	- XmDIALOG_SEPARATOR
	 *	- XmDIALOG_TEXT
	 *	- XmDIALOG_WORK_AREA
	 *  @param[in] child_name the child widget identifier
	 *  @returns the child widget.
	 */
	Widget getChild(unsigned int child_name) {
	    return XmFileSelectionBoxGetChild(base_widget, child_name);
	}
};

/** A class for the XmFileSelectionBox widget
 *  @ingroup libmotif
 */
class PopupMenu : public Component
{
    public:
	/** Constructor.
	 *  @param[in] name name given to this FileSelectionBox instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	PopupMenu(const string &name, Component *parent, Arg *args=NULL,
			int n=0) : Component(name, parent)
	{
	    base_widget = XmCreatePopupMenu(
				parent->topWindowParent()->baseWidget(),
				(char *)component_name.c_str(), args, n);
	    installDestroyHandler();
	}
	virtual PopupMenu *getPopupMenuInstance(void) { return this; }

	/** Position the popup menu.
	 *  @param[in] event position the popup menu at the event coordinates.
	 */
	void position(XButtonPressedEvent *event) {
	    XmMenuPosition(base_widget, event);
	}
};

/** A class that uses the XmSeparator widget to create a space.
 *  @ingroup libmotif
 */
class Space : public Component
{
    public:
	/** Constructor.
	 *  @param[in] name name given to this FileSelectionBox instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] orientation XmHORIZONTAL or XmVERTICAL
	 *  @param[in] width the width or height of the space.
	 */
	Space(const string &name, Component *parent, unsigned char orientation,
		int width) : Component(name, parent)
	{
	    if(orientation == XmHORIZONTAL) {
		base_widget = XtVaCreateManagedWidget(component_name.c_str(), 
				xmSeparatorWidgetClass, parent->baseWidget(),
				XmNorientation, XmVERTICAL,
				XmNseparatorType, XmNO_LINE,
				XmNwidth, width,
				NULL);
	    }
	    else {
		base_widget = XtVaCreateManagedWidget(component_name.c_str(), 
				xmSeparatorWidgetClass, parent->baseWidget(),
				XmNorientation, XmHORIZONTAL,
				XmNseparatorType, XmNO_LINE,
				XmNheight, width,
				NULL);
	    }
	}
};

/** A class for the XmFrame widget
 *  @ingroup libmotif
 */
class Group : public Component
{
    public:

	/** Constructor.
	 *  @param[in] name name given to this Group instance.
	 *  @param[in] parent the Component parent.
	 */
	Group(const string &name, Component *parent, bool manage_widget=false) : Component(name, parent)
	{
            if(manage_widget) {
                base_widget = XtVaCreateManagedWidget(component_name.c_str(),
                        xmFrameWidgetClass, parent->baseWidget(), NULL);
            } else {
	        base_widget = XtVaCreateWidget(component_name.c_str(),
			xmFrameWidgetClass, parent->baseWidget(), NULL);
            }
	    installDestroyHandler();
	}
	/** Constructor.
	 *  @param[in] name name given to this Group instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] n the number of X-resource structures.
	 */
	Group(const string &name, Component *parent, Arg *args, int n, 
		bool manage=false) : Component(name, parent)
	{
            if (manage) {
                base_widget = XtCreateManagedWidget(component_name.c_str(),
                        xmFrameWidgetClass, parent->baseWidget(), args, n);
            } else {
	        base_widget = XtCreateWidget(component_name.c_str(),
			xmFrameWidgetClass, parent->baseWidget(), args, n);
            }
	    installDestroyHandler();
	}

	/** Destructor. */
	~Group(void){}
};

#endif
