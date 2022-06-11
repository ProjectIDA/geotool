#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#include <vector>
#include <iostream>
using namespace std;

#include "motif++/ActionEvent.h"
#include "motif++/ActionListener.h"
#include "gobject++/ghashtable.h"
extern "C" {
#include "libstring.h"
}

/** @defgroup libmotif library libmotif++
 *  C++ classes for Motif Widgets.
 *  This library is a C++ class interface to the Motif X11 Widget set. The
 *  library contains C++ classes for many of the Motif widgets. It also contains
 *  additional classes that utilize the Motif widgets to construct utility
 *  dialog windows (ParamDialog, Question, StatusDialog, TextQuestion) and
 *  widget containers (ToolBar, Frame). The library also defines classes for
 *  shared library plugins (PlugIn, PlugInManager) and undo-actions (UndoAction,
 *  UndoManager).
 *
 *  The classes can be grouped into four catagories, classes that are
 *  components, classes that are containers, classes that are dialog windows
 *  and classes that are non-graphical. These groups are shown below.
 *
 *  Classes that are components:
 *  <table>
 * <tr><td><b>Class</b></td> <td><b>widget class or creation function</b></td></tr>
 * <tr><td>ArrowButton</td> <td>XmArrowButton</td></tr>
 * <tr><td>Button</td> <td>XmPushButton</td></tr>
 * <tr><td>Choice</td> <td>XmCreateOptionMenu</td></tr>
 * <tr><td>FileChoice</td> <td>XmCreateOptionMenu</td></tr>
 * <tr><td>Component</td> <td></td>base class</tr>
 * <tr><td>Label</td> <td>XmLabel</td></tr>
 * <tr><td>List</td> <td>XmList</td></tr>
 * <tr><td>Scale</td> <td>XmScale</td></tr>
 * <tr><td>ScrollBar</td> <td>XmScrollBar</td></tr>
 * <tr><td>Separator</td> <td>XmSeparator</td></tr>
 * <tr><td>Space</td> <td>XmSeparator</td></tr>
 * <tr><td>TextField</td> <td>XmText</td></tr>
 * <tr><td>Toggle</td> <td>XmToggleButton</td></tr>
 * </table>
 *
 * Classes that are containers:
 * <table>
 * <tr><td><b>Class</b></td> <td><b>widget class or creation function</b></td></tr>
 * <tr><td>FileSelectionBox</td> <td>XmFileSelectionBox</td></tr>
 * <tr><td>Form</td> <td>XmForm</td></tr>
 * <tr><td>InfoClass</td> <td>XmForm</td></tr>
 * <tr><td>Menu</td> <td>XmCreatePulldownMenu</td></tr>
 * <tr><td>MenuBar</td> <td>XmCreateMenuBar</td></tr>
 * <tr><td>Pane</td> <td>XmPane</td></tr>
 * <tr><td>RadioBox</td> <td>XmCreateRadioBox</td></tr>
 * <tr><td>RowColumn</td> <td>XmRowColumn</td></tr>
 * <tr><td>ScrolledWindow</td> <td>XmScrolledWindow</td></tr>
 * <tr><td>ToolBar</td> <td>XmForm</td></tr>
 * </table>
 *
 * Dialog window classes 
 * <table>
 * <tr><td><b>Class</b></td> <td><b>widget class or creation function</b></td></tr>
 * <tr><td>FileDialog</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>FormDialog</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>Frame</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>MainWindow</td> <td>XmMainWindow</td></tr>
 * <tr><td>MessageDialog</td> <td>XmCreateMessageDialog</td></tr>
 * <tr><td>ParamDialog</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>PopupMenu</td> <td>XmCreatePopupMenu</td></tr>
 * <tr><td>Question</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>StatusDialog</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>TextQuestion</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>TopWindow</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>Warn</td> <td>XmCreateFormDialog</td></tr>
 * <tr><td>Warnings</td> <td>XmCreateFormDialog</td></tr>
 * </table>
 *
 * Classes without a graphical interface.
 * <table>
 * <tr><td>ActionEvent</td></tr>
 * <tr><td>Application</td></tr>
 * <tr><td>PlugIn</td></tr>
 * <tr><td>PlugInManager</td></tr>
 * <tr><td>UndoAction</td></tr>
 * <tr><td>UndoManager</td></tr>
 * </table>
 */

#define XtNsetSensitiveCallback (char *)"setSensitiveCallback"
#define XtNsetVisibleCallback (char *)"setVisibleCallback"
#define XtNsetLabelCallback (char *)"setLabelCallback"
#define XtNchangePropertyCallback (char *)"changePropertyCallback"

typedef struct
{
    /** The parse_file that is currently being parsed. */
    int parse_file_q;
    /** The directory that contains the parse_file that is currently
     *  being parsed.
     */
    int parse_dir_q;
    int line_number;
    /** If true, file paths inside a parse file are relative to the
     *  parse file's directory. Otherwise, they are relative to the
     *  directory where the program was executed.
     */
    bool relative_parse_paths;
} ParseFileInfo;


class Application;
class AxesClass;
class Button;
class CssFileDialog;
class FormDialog;
class Frame;
class InfoArea;
class Label;
class Menu;
class PopupMenu;
class RowColumn;
class Table;
class TableViewer;
class TextField;
class Toggle;
class TopWindow;
class Warn;
class WaveformPlot;
class WaveformView;
class WaveformWindow;

/** The base class for all libmotif++ graphical classes.
 *  The Component class is the base class for all of the libmotif++ classes
 *  that represent a graphical window. It contains the functionality that is
 *  common to all graphical components, such as setting the visiblity, the
 *  size, the sensitivity, etc. of components. It also contains the event
 *  callback interface, the accelerator interface and the interface to the
 *  graphical component tree.
 *  
 *  Component class callbacks:
 *      - XtNsetSensitiveCallback called when the sensitivity is changed.
 *      - XtNsetVisibleCallback called when the visibility is changed.
 *  @ingroup libmotif
 */
class Component : public Gobject, public ActionListener
{
    protected:
	/** The component constructor is accessible only from a subclass. */
	Component(const string &cname, Component *cparent);
        Component(const Component &c) :
	    comp_parent(c.comp_parent), real_comp_parent(c.real_comp_parent),
	    base_widget(c.base_widget), component_name(c.component_name),
	    command_string(c.command_string), callback_types(c.callback_types),
	    listeners(c.listeners), scripts(c.scripts),
	    comp_children(c.comp_children),
	    redirect_warnings(c.redirect_warnings)
	{ }

        Component & operator=(const Component &c)
	{
	    comp_parent = c.comp_parent;
	    real_comp_parent = c.real_comp_parent;
	    base_widget = c.base_widget;
	    component_name = c.component_name;
	    command_string = c.command_string;
	    callback_types = c.callback_types;
	    listeners = c.listeners;
	    scripts = c.scripts;
	    comp_children = c.comp_children;
	    redirect_warnings = c.redirect_warnings;
	    return *this;
	}

    public:
	/** @name Instance name and Class name */
	//@{
	/** Get the Component name.
	 *  @returns the name given to a Component instance.
	 */
	const char *getName(void) { return component_name.c_str(); }
	//@}

	/** @name Visibility and Sensitivity functions */
	//@{
	/** Display or hide the Component. */
	virtual void setVisible(bool);
	/** Returns true if the Component is visible (Managed). */
	virtual bool isVisible(void);
	/** Manage the base_widget of the Component. This function manages the
	 *  base_widget of the Component with XtManageChild, but does not insure
	 *  that the Component is visible. (XtMappedWhenManaged is false for
	 *  some classes.) The function setVisible is adequate for most
	 *  situations.
	 */
	virtual void manageBase(void) { XtManageChild(base_widget); }
	/** Set the Component's sensitivity. */
	void setSensitive(bool sensitive);
	/** Returns true if the component is currently sensitive to user input.
	 */
	virtual bool isSensitive(void) { return XtIsSensitive(base_widget); }
	//@}

	/** @name Size functions */
	//@{
	void setSize(int width, int height);
	void setWidth(int width);
	void setHeight(int height);
	void getSize(int *width, int *height);
	int getX(void);
	int getY(void);
	int getWidth(void);
	int getHeight(void);
	//@}

	/** @name Destruction functions */
	//@{
	virtual ~Component(void);
	virtual void destroy(void);
	//@}

	/** @name Event callback interface */
	//@{
	virtual void addActionListener(ActionListener *listener,
		const string &action_type=string(XmNactivateCallback));
	virtual void removeActionListener(ActionListener *listener,
		const string &action_type=string(XmNactivateCallback));
	virtual void removeAllListeners(const string &action_type);
	virtual vector<ActionListener *> *getActionListeners(
		const string &action_type=string(XmNactivateCallback));
	/** Set the action command string. The action command string is used to
	 *  identify the source of the action. It is recorded in the
	 *  ActionEvent argument to the listener component's actionPerformed
	 *  function. If setActionString is never called, the action command
	 *  string defaults to the name of the Component instance.
	 *  @param[in] command_string
	 */
	void setCommandString(const string &command) {
	    command_string.assign(command);
	}
//	void setCommandString(const string &command) {
//	    command_string.assign(command);
//	}
	/** Get the action command string. The action command string is used to
	 *  identify the source of the action. It is recorded in the
	 *  ActionEvent argument to the listener component's actionPerformed
	 *  function, that is called for each action. If this function is never
	 *  called, the action command string defaults to the name of the
	 *  Component instance.
	 *  @returns the action command string.
	 */
	const char *getCommandString(void) {
	    return command_string.c_str();
	}
	void getCommandString(string &s) { s.assign(command_string); }
	/** The action (event) callback function. A Component can "listen" for
	 * action events from another Component (or from itself), by registering
	 * itself as a listener with the other Component. Use the
	 * addActionListener function to register one Component as a listener
	 * for the actions on another Component. When an action occurs on a
	 * Component, that Component calls the actionPerformed function
	 * belonging to all of the Components that have registered as listeners
	 * for that type of action. The ActionEvent class contains the action
	 * command string, the source Component of the action, the reason for
	 * the action, and the callback data associated with the action.
	 * @param[in] action_event a pointer to an ActionEvent object.
	 * @sa addActionListener
	 */
	virtual void actionPerformed(ActionEvent *action_event) {}

	virtual void addScriptCallback(const string &s,
				const string &action_type);
	//@}

	/** @name Component/widget tree interface */
	//@{
	/** Get the base or root Widget of this Component */
	Widget baseWidget(void) { return base_widget; }
	/** Get the Widget parent of the base Widget of this Component */
	Widget widgetParent(void) {
	    return base_widget ? XtParent(base_widget) : NULL;}
	Widget shellWidget(void);
	vector<Component *> *getChildren(void);
	int numChildren(void) { return (int)comp_children.size(); }
	void positionLastChild(int new_pos);
	/** Get the Component parent. */
	Component *getParent(void) { return comp_parent; }

	Button *findButton(const string &name, bool star=true);
	InfoArea *findInfoArea(const string &name, bool star=true);
	Label *findLabel(const string &name, bool star=true);
	Menu *findMenu(const string &name, bool star=true);
	RowColumn *findRowColumn(const string &name, bool star=true);
	Table *findTable(const string &name, bool star=true);
	TextField *findTextField(const string &name, bool star=true);
	Toggle *findToggle(const string &name, bool star=true);

	TopWindow *topWindowParent(void);
	FormDialog *formDialogParent(void);

	virtual Application *getApplicationInstance(void) { return NULL; }
	virtual AxesClass *getAxesClassInstance(void) { return NULL; }
        virtual Button *getButtonInstance(void) { return NULL; }
	virtual CssFileDialog *getCssFileDialogInstance(void) { return NULL; }
        virtual FormDialog *getFormDialogInstance(void) { return NULL; }
        virtual Frame *getFrameInstance(void) { return NULL; }
        virtual InfoArea *getInfoAreaInstance(void) { return NULL; }
        virtual Label *getLabelInstance(void) { return NULL; }
        virtual Menu *getMenuInstance(void) { return NULL; }
        virtual PopupMenu *getPopupMenuInstance(void) { return NULL; }
        virtual RowColumn *getRowColumnInstance(void) { return NULL; }
        virtual Table *getTableInstance(void) { return NULL; }
	virtual TableViewer *getTableViewerInstance(void) { return NULL; }
        virtual TextField *getTextFieldInstance(void) { return NULL; }
        virtual Toggle *getToggleInstance(void) { return NULL; }
        virtual TopWindow *getTopWindowInstance(void) { return NULL; }
	virtual WaveformPlot *getWaveformPlotInstance(void) { return NULL; }
	virtual WaveformView *getWaveformViewInstance(void) { return NULL; }
	virtual WaveformWindow *getWaveformWindowInstance(void) { return NULL; }


	/** Reposition children. This implementation simply unmanages and
	 *  manages the base_widget.
	 */
	virtual void layout(void) {
	    if(base_widget) {
		XtUnmanageChild(base_widget); XtManageChild(base_widget);
	    }
	}
	//@}

	/** @name Error messages */
	//@{
#ifdef __STDC__
        void showWarning(const char *format, ...) throw(int);
        void putWarning(const char *format, ...) throw(int);
        static void printLog(const char *format, ...) throw(int);
#else
        void showWarning(va_alist);
        void putWarning(va_alist);
        static void printLog(va_alist);
#endif
	void showWarning(const string &s) { showWarning(s.c_str()); }

	/** Display the last error message.
	 */
	void showErrorMsg(void) {
	    char *msg = errorMsg();
	    if(msg) showWarning(msg);
	}
	/** Set the redirect state. If true warnings are printed to stdout only.
	 */
	void setRedirectWarnings(bool state) { redirect_warnings = state; }
	//@}

	/** @name X-resource interface */
	//@{
	/** Set X-resource values. Set X-resource values on the Component's
	 *  base widget.
	 *  @param[in] args resource Arg structures.
	 *  @param[in] n the number of Arg structures.
	 */
	virtual void setValues(Arg *args, int n) {
		XtSetValues(base_widget, args, n); }
	/** Get X-resource values. Get X-resource values from the Component's
	 *  base widget.
	 *  @param[out] args resource Arg structures.
	 *  @param[in] n the number of Arg structures.
	 */
	virtual void getValues(Arg *args, int n) {
		XtGetValues(base_widget, args, n); }

	/** @name Accelarator functions */
	//@{
	void installAccelerators(void);
	void installAccelerators(Component *comp);
	void installAccelerators(Widget source);
	void installAccelerators(Widget destination, Widget source);
	//@}

	/** @name Pixel utility functions */
	//@{
	Pixel stringToPixel(const string &color_name);
	Pixel pixelBrighten(Pixel pixel, double percent);
	void setCursor(const string &type);
	//@}

	/** @name XmString utility functions */
	//@{
	static char *getXmString(XmString xm);
	static void getXmString(XmString xm, string &s) {
	    char *xms = getXmString(xm);
	    s.clear();
	    if(xms) { s.assign(xms); XtFree(xms); }
	}
	static XmString createXmString(const string &s);
	//@}

	/** @name Selection communication functions */
	//@{
//	virtual void copy(int selection_type, Time time) { }
//	virtual void cut(int selection_type, Time time) { }
//	virtual void paste(Time time) { }
	//@}

	/** @name Callback functions */
	//@{
	/** Enable action callbacks of the specified type. A subclass of
	 *  Component must enable callback types (once only) before it's 
	 *  addActionListener function is called and before it's doCallbacks
	 *  function is called with the specified action type.
	 *  @param[in] action_type
	 */
	void enableCallbackType(const string &action_type);
	void doCallbacks(Widget source, XtPointer callback_data,
		const string &action_type=string(XmNactivateCallback));
	void doCallbacks(Component *, XtPointer callback_data,
		const string &action_type=string(XmNactivateCallback));
	void doCallbacks(XtPointer callback_data,
		const string &action_type=string(XmNactivateCallback)) {
	    doCallbacks(this, callback_data, action_type);
	}
	//@}

	virtual ParseCmd parseCmd(const string &cmd, string &msg);
	virtual ParseVar parseVar(const string &name, string &value);

	static void putProperty(const string &name, const string &value,
			bool permanent=true, Component *comp=NULL);
	static void removeProperty(const string &name);
	static char * getProperty(const string &name);
	static bool getProperty(const string &name, string &prop) {
	    char *p = getProperty(name);
	    prop.clear();
	    if(p) {prop.assign(p); free(p); return true;}
	    return false;
	}
	static int getProperty(const string &name, int default_value) {
	    char *prop = getProperty(name);
	    if(prop) {
		int i;
		if(stringToInt(prop, &i)) { free(prop); return i; }
		free(prop);
	    }
	    return default_value;
	}
	static long getProperty(const string &name, long default_value) {
	    char *prop = getProperty(name);
	    if(prop) {
		long i;
		if(stringToLong(prop, &i)) { free(prop); return i; }
		free(prop);
	    }
	    return default_value;
	}
	static double getProperty(const string &name, double default_value) {
	    char *prop = getProperty(name);
	    if(prop) {
		double d;
		if(stringToDouble(prop, &d)) { free(prop); return d; }
		free(prop);
	    }
	    return default_value;
	}
	static bool getProperty(const string &name, bool default_value) {
	    char *prop = getProperty(name);
	    if(prop) {
		int b;
		if(stringToBool(prop, &b)) { free(prop); return (bool)b; }
		free(prop);
	    }
	    return default_value;
	}

    protected:
	Component	*comp_parent;	//!< The Component parent
	/** The widget-tree Component parent. In most cases the base_widget of
	 * the comp_parent is the widget parent of the base_widget of the
	 * Component child, ie. comp_parent->base_widget ==
	 * XtParent(base_widget). In situations where this is not true, the
	 * real_comp_parent is the component whose base_widget is the widget
	 * parent.  real_comp_parent->base_widget == XtParent(base_widget) is
	 * always true.
	 */
	Component	*real_comp_parent;
	Widget		base_widget;	 //!< The base widget of this Component
	string		component_name;  //!< The Component instance name
	string		command_string;  //!< The action command string
	vector<int>	callback_types;  //!< The list of callback types
	/** The list of action listeners */
	ghashtable<vector<ActionListener *> *> listeners;
	typedef struct {
	    string script;
	    vector<ParseFileInfo> parse_file;
	} ScriptCallback;
	ghashtable<vector<ScriptCallback> *> scripts;
	vector<Component *> comp_children; //!< The children of this Component
	/** If true, print warnings from this Component to stdout */
	bool		redirect_warnings;

	void installDestroyHandler(void);

    private:
	void addChild(Component *comp) throw(int);
	void removeChild(Component *comp);
	void findRealParent(void);

	// XmNdestroyCallback
	static void widgetDestroyedCB(Widget, XtPointer, XtPointer);
	static void closeWarnCallback(Widget, XtPointer, XtPointer);
	static void warnCallback(Widget, XtPointer, XtPointer);

};

#endif
