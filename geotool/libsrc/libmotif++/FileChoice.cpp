/** \file FileChoice.cpp
 *  \brief Defines class FileChoice.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/FileChoice.h"
#include "motif++/FileDialog.h"

/** Constructor.
 *  @param[in] name the name given to this FileChoice instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] num_args the number of X-resource structures.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] choose_file true if the choice is for files; false if it is for
 *		directories.
 */
FileChoice::FileChoice(const string &name, Component *parent, Arg *args,
		int num_args, ActionListener *listener, bool choose_file) :
		Choice(name, parent, listener, args, num_args)
{
    addChooseFile(choose_file);
}

/** Destructor
 */
FileChoice::~FileChoice(void)
{
}

/** Add the "(choose file)" or "(choose directory)" choice to the menu.
 *  This choice will always be the last item in the menu.
 *  @param[in] choose_file if true, add "(choose file)"; if false add
 *		"(choose directory)"
 */
void FileChoice::addChooseFile(bool choose_file)
{
    if(choose_file) {
	Widget w = XtVaCreateManagedWidget("(choose file)",
				xmPushButtonWidgetClass,
				pulldown,
                                XmNmarginHeight, 0,
                                XmNhighlightThickness, 0,
                                NULL);
	XtAddCallback(w, XmNactivateCallback, chooseFileCB, (XtPointer)this);
    }
    else {
	Widget w = XtVaCreateManagedWidget("(choose directory)",
				xmPushButtonWidgetClass,
				pulldown,
                                XmNmarginHeight, 0,
                                XmNhighlightThickness, 0,
                                NULL);
	XtAddCallback(w, XmNactivateCallback, chooseDirCB, (XtPointer)this);
    }
}

/** Add another choice to the menu.
 *  The new item is added before the "(choose file)" or "(choose directory)"
 *  choice which is always the last item in the menu.
 *  @param[in] name the name of the new choice.
 */
void FileChoice::addItem(const string &name)
{
    Arg args[1];
    int num_children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtGetValues(pulldown, args, 1);

    // keep the (choose file) item the last (bottom) item
    short position = (num_children > 0) ? num_children-1 : 0;
    Widget w = XtVaCreateManagedWidget((char *)name.c_str(),
				xmPushButtonWidgetClass, pulldown,
				XmNmarginHeight, 0,
				XmNhighlightThickness, 0,
				XmNpositionIndex, position,
				NULL);
    XtAddCallback(w, XmNactivateCallback, Choice::activateCallback,
			(XtPointer)this);
    if(first_item) {
        first_item = false;
        XtVaSetValues(base_widget, XmNmenuHistory, w, NULL);
    }
}

/** Add a Separator.
 *  The Separator is added before the "(choose file)" or "(choose directory)"
 *  choice which is always the last item in the menu.
 */
void FileChoice::addSeparator(void)
{
    Arg args[1];
    int num_children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtGetValues(pulldown, args, 1);

    // keep the (choose file) item the last (bottom) item
    short position = (num_children > 0) ? num_children-1 : 0;
    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, pulldown,
				XmNorientation, XmHORIZONTAL,
				XmNpositionIndex, position,
				NULL);
}

/** Remove all choices
 *  The "(choose file)" or "(choose directory)" choice remains.
 */
void FileChoice::removeAllChoices(void)
{
    Arg args[2];
    int num_children;
    Widget *children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtSetArg(args[1], XmNchildren, &children);
    XtGetValues(pulldown, args, 2);

    // don't remove the last choice "(choose file)"
    for(int i = num_children-2; i >= 0; i--) {
        XtDestroyWidget(children[i]);
    }
}

void FileChoice::chooseFileCB(Widget w, XtPointer client, XtPointer data)
{
    FileChoice *f = (FileChoice *)client;
    f->chooseFile(true);
}

void FileChoice::chooseDirCB(Widget w, XtPointer client, XtPointer data)
{
    FileChoice *f = (FileChoice *)client;
    f->chooseFile(false);
}

/** Display the FileDialog window.
 *  If choose_file is true, the FileDialog::getFile function is called to get
 *  the user's file selection. If choose_file is false, the FileDialog::getDir
 *  function is called to get the user's directory selection.
 *  @param[in] choose_file
 */
void FileChoice::chooseFile(bool choose_file)
{
    string file;
    bool ret;
    Arg args[2];
    int num_children;
    Widget *children;

    XtSetArg(args[0], XmNnumChildren, &num_children);
    XtSetArg(args[1], XmNchildren, &children);
    XtGetValues(pulldown, args, 2);

    if(choose_file) {
	ret = FileDialog::getFile("Choose File", this, file, "./", "*",
				"Choose File");
    }
    else {
	ret = FileDialog::getDir("Choose Directory", this, file, "./", "*",
				"Choose Directory");
    }

    if(ret)
    {
	int i;
	for(i = 0; i < num_children && file.compare(XtName(children[i])); i++);

	if(i == num_children)
	{
	    // keep the (choose file) item at the bottom of the menu.
	    short position = (num_children > 0) ? num_children-1 : 0;
	    Widget w = XtVaCreateManagedWidget((char *)file.c_str(),
				xmPushButtonWidgetClass,
				pulldown,
				XmNmarginHeight, 0,
				XmNhighlightThickness, 0,
				XmNpositionIndex, position,
				NULL);
	    XtAddCallback(w, XmNactivateCallback, Choice::activateCallback,
			(XtPointer)this);
	    XtVaSetValues(base_widget, XmNmenuHistory, w, NULL);

	    XmPushButtonCallbackStruct c = {XmCR_ACTIVATE, NULL, 0};
	    doCallbacks(w, (XtPointer)&c);
	}
	else {
	    setChoice(file, true);
	}
    }
    else {
	XtVaSetValues(base_widget, XmNmenuHistory, children[0], NULL);
    }
}
