/** \file FileDialog.cpp
 *  \brief Defines class FileDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sys/param.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>

#include "motif++/FileDialog.h"
#include "motif++/RowColumn.h"
#include "motif++/Button.h"
#include "motif++/Toggle.h"
#include "motif++/Choice.h"
#include "motif++/MotifComp.h"
#include "motif++/Application.h"

typedef struct
{
    FileDialog	*fd;
    Widget	fsb;
} FsbList;

static int num_fsb_list = 0;
static FsbList *fsb_list = NULL;


/** Constructor.
 *  @param[in] name name given to this FileDialog instance.
 *  @param[in] parent the Component parent.
 *  @param[in] type the file or directory flag:
 *	- EXISTING_FILE_OR_DIR allow only an existing file or existing directory
 *	- EXISTING_FILE allow only an existing file
 *	- EXISTING_DIR allow only an existing directory
 *	- FILE_OR_DIR allow any file or directory
 *	- FILE_ONLY allow only a file
 *	- DIR_ONLY allow only a directory
 *  @param[in] dir the initial directory listed.
 *  @param[in] filter the file filter.
 *  @param[in] open_button_label the label for the Open button.
 */
FileDialog::FileDialog(const string &name, Component *parent, FileType type,
		const string &dir, const string &filter,
		const string &open_button_label) :
		TopWindow(name, parent, false, false)
{
    createInterface(dir, filter, open_button_label, type, 0, NULL, "");
}

/** Constructor.
 *  @param[in] name name given to this FileDialog instance.
 *  @param[in] parent the Component parent.
 *  @param[in] type the file or directory flag:
 *	- EXISTING_FILE_OR_DIR allow only an existing file or existing directory
 *	- EXISTING_FILE allow only an existing file
 *	- EXISTING_DIR allow only an existing directory
 *	- FILE_OR_DIR allow any file or directory
 *	- FILE_ONLY allow only a file
 *	- DIR_ONLY allow only a directory
 *  @param[in] dir the initial directory listed.
 *  @param[in] num_file_suffixes
 *  @param[in] file_suffixes
 *  @param[in] initial_file_suffix
 *  @param[in] open_button_label the label for the Open button.
 */
FileDialog::FileDialog(const string &name, Component *parent, FileType type,
		const string &dir, int num_file_suffixes,
		const char **file_suffixes, const string &initial_file_suffix,
		const string &open_button_label) :
		TopWindow(name, parent, false, false)
{
    createInterface(dir, "*", open_button_label, type,
		num_file_suffixes, file_suffixes, initial_file_suffix);
}

/** Create the interface.
 *  @param[in] dir the initial directory listed.
 *  @param[in] filter the file filter.
 *  @param[in] open_button_label the label for the Open button.
 *  @param[in] type the file or directory flag:
 *	- EXISTING_FILE_OR_DIR allow only an existing file or existing directory
 *	- EXISTING_FILE allow only an existing file
 *	- EXISTING_DIR allow only an existing directory
 *	- FILE_OR_DIR allow any file or directory
 *	- FILE_ONLY allow only a file
 *	- DIR_ONLY allow only a directory
 *  @param[in] num_file_suffixes
 *  @param[in] file_suffixes
 *  @param[in] initial_file_suffix
 */
void FileDialog::createInterface(const string &dir, const string &filter,
		const string &open_button_label, FileType type,
		int num_file_suffixes, const char **file_suffixes,
		const string &initial_file_suffix)
{
    int i, n, num_children, m;
    Widget dir_list, *children, w;
    XmString xm=NULL;
    Arg args[10];

    directory = (dir[0] != '\0') ?  strdup(dir.c_str()) : strdup(".");
    n = (int)strlen(directory);
    if(directory[n-1] == '/') directory[n-1] = '\0';
    file_filter = (filter[0] != '\0') ?  strdup(filter.c_str()) : strdup("*");
    file_type = type;
    filter_choice = NULL;

    append_file = false;

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;

    controls = new RowColumn("controls", this, args, n);
    close_button = new Button("Close", controls, this);

    if(file_type == WRITE_FILE) {
	open_button = new Button("Write File", controls, this);
    }
    else if(!open_button_label.empty()) {
	open_button = new Button(open_button_label, controls, this);
    }
    else {
	open_button = new Button("Open", controls, this);
    }
    if(file_type == EXISTING_FILE || file_type == FILE_ONLY
		|| file_type == WRITE_FILE)
    {
	open_button->setSensitive(false);
    }
    append_button = new Button("Append File", controls, this);
    append_button->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    form = new Form("form", rc);

    if(file_type != EXISTING_FILE) {
	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	selection_text = new TextField("Selection", form, this, args, n);
    }

    if(num_file_suffixes > 0)
    {
	if(file_suffixes) {
	    for(i = 0; i < num_file_suffixes; i++) {
		suffixes.push_back(strdup(file_suffixes[i]));
	    }
	}

	n = 0;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 5); n++;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	rc2 = new RowColumn("rc2", form, args, n);
	filter_label = new Label("Filter", rc2);

	filter_choice = new Choice("Filter Choice", rc2, this);
	filter_choice->addItem("All Types");
	for(i = 0; i < (int)suffixes.size(); i++) {
	    char *s = suffixes[i];
	    m = (int)strlen(s);
	    if(m > 3 && !strcasecmp(s+m-3, ".gz")) {
		s[m-3] = '\0';
		filter_choice->addItem(s);
		gz_suffixes.push_back(strdup(s));
		s[m-3] = '.';
	    }
	    else {
		filter_choice->addItem(s);
	    }
	}
	if((int)suffixes.size() > 0) {
	    filter_choice->addItem("*");
	}
/*
	else {
	    if(initial_file_suffix) {
		filter_choice->setChoice(initial_file_suffix);
	    }
	    else {
		initial_file_suffix = "All Types";
	    }
	}
*/

	n = 0;
	if(file_type == EXISTING_FILE) {
	    xm = createXmString("");
	    XtSetArg(args[n], XmNselectionLabelString, xm); n++;
	}
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, rc2->baseWidget()); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	if(file_type != EXISTING_FILE) {
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n],XmNbottomWidget,selection_text->baseWidget()); n++;
	}
	else {
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	}
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
	XtSetArg(args[n], XmNdoubleClickInterval, 500); n++;
	fsb = new FileSelectionBox("fsbox", form, args, n);
	if(file_type == EXISTING_FILE) XmStringFree(xm);
    }
    else {
	n = 0;
	if(file_type == EXISTING_FILE) {
	    xm = createXmString("");
	    XtSetArg(args[n], XmNselectionLabelString, xm); n++;
	}
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	if(file_type != EXISTING_FILE) {
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n],XmNbottomWidget,selection_text->baseWidget()); n++;
	}
	else {
	    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	}
	XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
	XtSetArg(args[n], XmNdoubleClickInterval, 500); n++;
	fsb = new FileSelectionBox("fsbox", form, args, n);
	if(file_type == EXISTING_FILE) XmStringFree(xm);
    }
    w = XmFileSelectionBoxGetChild(fsb->baseWidget(), XmDIALOG_FILTER_LABEL);
    XtUnmanageChild(w);
    fsb->setVisible(false);

    /* add a callback to the directory list so that a single
     * select will update the file list.
     */
    filter_button = fsb->getChild(XmDIALOG_APPLY_BUTTON);
    if(filter_button == NULL) {
	cerr << "FileDialog: XmDIALOG_APPLY_BUTTON = NULL." << endl;
	Application::getApplication()->stop(-1);
    }
    dir_list = fsb->getChild(XmDIALOG_DIR_LIST);
    if(dir_list == NULL) {
	cerr << "FileDialog: XmDIALOG_DIR_LIST = NULL." << endl;
	Application::getApplication()->stop(-1);
    }
    XtAddCallback(dir_list, XmNbrowseSelectionCallback, FileDialog::openDirCB,
			(XtPointer)this);

    n = 0;
    XtSetArg(args[n], XmNnumChildren, &num_children); n++;
    XtSetArg(args[n], XmNchildren, &children); n++;
    fsb->getValues(args, n);

    for(i = 0; i < num_children; i++) {
	if( XtClass(children[i]) == xmPushButtonGadgetClass ||
	    XtClass(children[i]) == xmSeparatorGadgetClass) {
		XtUnmanageChild(children[i]);
	}
    }

    fsb->setVisible(true);
    XtManageChild(base_widget);

    XtSetArg(args[0], XmNfileSearchProc, &defaultFileSearch);
    fsb->getValues(args, 1);

    if((w = fsb->getChild(XmDIALOG_FILTER_TEXT)) != NULL)
    {
	XmAnyCallbackStruct c;
	string path;

	if(num_file_suffixes > 0) {
	    m = (int)initial_file_suffix.length();

	    if(!initial_file_suffix.compare("*")) {
		path = string(directory) + "/*";
	    }
	    else if(!initial_file_suffix.compare("All Types")) {
		XmString xm;
		string sdir;
		XtSetArg(args[0], XmNfileSearchProc, fileSearch);
		fsb->setValues(args, 1);

		XtSetArg(args[0], XmNdirectory, &xm);
		fsb->getValues(args, 1);

		getXmString(xm, sdir);

		doFileSearch(sdir, "");

		path = string(directory) + "/(All Types)";
	    }
	    else if(m>3 && !strcasecmp(initial_file_suffix.c_str()+m-3, ".gz")){
		XmString xm;
		string sdir;
		XtSetArg(args[0], XmNfileSearchProc, fileSearch2);
		fsb->setValues(args, 1);

		XtSetArg(args[0], XmNdirectory, &xm);
		fsb->getValues(args, 1);

		getXmString(xm, sdir);

		doFileSearch(sdir, initial_file_suffix);

		gz_ftype = initial_file_suffix;
		path = string(directory) + "/*." +
			initial_file_suffix.substr(0, m-3);
	    }
	    else {
		path = string(directory) + "/*." + initial_file_suffix;
	    }
	}
	else {
	    path = string(directory) + "/" + file_filter;
	}
	XmTextSetString(w, (char *)path.c_str());
	c.reason = XmCR_ACTIVATE;
	c.event = NULL;
	XtCallCallbacks(filter_button, XmNactivateCallback, (XtPointer)&c);
	if(num_file_suffixes > 0) {
	    XtAddCallback(w, XmNactivateCallback, applyChoiceFilter,
			(XtPointer)this);
	}
	else {
	    XtAddCallback(w, XmNactivateCallback, applyFilter,
			(XtPointer)filter_button);
	}
    }

    if((w = fsb->getChild(XmDIALOG_LIST)) != NULL)
    {
	XtAddCallback(w, XmNbrowseSelectionCallback, fileSelect,
		(XtPointer)this);
	XtAddCallback(w, XmNextendedSelectionCallback, fileSelect,
		(XtPointer)this);
	XtAddCallback(w, XmNmultipleSelectionCallback, fileSelect,
		(XtPointer)this);
	XtAddCallback(w, XmNmultipleSelectionCallback, fileSelect,
		(XtPointer)this);
	XtAddCallback(w, XmNsingleSelectionCallback,fileSelect,(XtPointer)this);
    }
    Widget file_list = fsb->getChild(XmDIALOG_FILE_LIST);

    if(file_type == EXISTING_DIR || file_type == DIR_ONLY) {
	XtSetSensitive(file_list, false);
    }

    // record class pointer and fsb so we can get the class pointer from the
    // widget fsb in the fileSearch callback.
    if(!fsb_list) {
	fsb_list = (FsbList *)malloc(sizeof(FsbList));
    }
    else {
	fsb_list =(FsbList *)realloc(fsb_list,(num_fsb_list+1)*sizeof(FsbList));
    }
    fsb_list[num_fsb_list].fd = this;
    fsb_list[num_fsb_list].fsb = fsb->baseWidget();
    num_fsb_list++;

    if((w = fsb->getChild(XmDIALOG_TEXT)) != NULL) {
	XtUnmanageChild(w);
    }

    if(file_type == EXISTING_FILE) {
	int n;
	Arg args[2];
	Widget file_list = fsb->getChild(XmDIALOG_FILE_LIST);
	n = 0;
	XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
	XtSetValues(file_list, args, n);
	XtAddCallback(file_list, XmNextendedSelectionCallback, fileSelectCB,
			(XtPointer)this);
	XtSetValues(file_list, args, n);
//	n = 0;
//	XtSetArg(args[n], XmNeditable, False); n++;
//	selection_text->setValues(args, n);
    }

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XtNfileSelectCallback);

    current_directory = NULL;
    time_out_id = XtAppAddTimeOut(Application::getAppContext(), 1000,
		(XtTimerCallbackProc)checkForNewFiles, (XtPointer)this);

    // force the directory scrolled window to initialize correctly
    XtAppAddTimeOut(Application::getAppContext(), 1,
	(XtTimerCallbackProc)forceFilter, (XtPointer)this);
}

void FileDialog::setFileSuffixes(int num_file_suffixes,
		const char **file_suffixes, const string &initial_suffix)
{
    for(int i = 0; i < (int)suffixes.size(); i++) Free(suffixes[i]);
    suffixes.clear();
    gz_suffixes.clear();

    filter_choice->destroy();
    filter_choice = new Choice("Filter Choice", rc2, this);
    filter_choice->addItem("All Types");

    for(int i = 0; i < num_file_suffixes; i++) {
	char *s = strdup(file_suffixes[i]);
	int m = (int)strlen(s);
	if(m > 3 && !strcasecmp(s+m-3, ".gz")) {
	    s[m-3] = '\0';
	    filter_choice->addItem(s);
	    gz_suffixes.push_back(strdup(s));
	    s[m-3] = '.';
	}
	else {
	    filter_choice->addItem(s);
	}
	suffixes.push_back(s);
    }
    filter_choice->addItem("*");
    filter_choice->setChoice(initial_suffix, true);
}

/** Process ActionEvent callbacks. This Component is listening for actions from
 *  the Close and Open buttons.
 *  @param[in] action_event an ActionEvent instance.
 */
void FileDialog::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
        dialogCancel();
    }
    else if(comp == open_button) {
        dialogOpen();
    }
    else if(comp == append_button) {
        dialogAppend();
    }
    else if(!strcmp(cmd, "Filter Choice"))
    {
	doFilterChoice();
    }
    else if(!strcmp(cmd, "Selection")) {
	if(file_type != EXISTING_FILE) {
	    textCallback(selection_text->baseWidget());
	}
    }
}

void FileDialog::doFilterChoice(void)
{
    XmAnyCallbackStruct c;
    string dir, path;
    XmString xm;
    Arg args[1];
    Widget w;

    if(!filter_choice || !(w = fsb->getChild(XmDIALOG_FILTER_TEXT)) ) return;

    XtSetArg(args[0], XmNdirectory, &xm);
    fsb->getValues(args, 1);
    getXmString(xm, dir);

    if(!strcmp(filter_choice->getChoice(), "All Types")) {
	XtSetArg(args[0], XmNfileSearchProc, fileSearch);
	fsb->setValues(args, 1);
	path = dir + "/(All Types)";
    }
    else if(!strcmp(filter_choice->getChoice(), "*")) {
	XtSetArg(args[0], XmNfileSearchProc, defaultFileSearch);
	fsb->setValues(args, 1);
	path = dir + "/*";
    }
    else {
	int i;
	const char *c = filter_choice->getChoice();
	for(i = 0; i < (int)gz_suffixes.size() && strcmp(gz_suffixes[i],c);i++);
	if(i < (int)gz_suffixes.size()) {
	    gz_ftype = string(gz_suffixes[i]) + ".gz";
	    XtSetArg(args[0], XmNfileSearchProc, fileSearch2);
	}
	else {
	    XtSetArg(args[0], XmNfileSearchProc, defaultFileSearch);
	}
	fsb->setValues(args, 1);
	path = dir + "/*." + filter_choice->getChoice();
    }
    XmTextSetString(w, (char *)path.c_str());
    c.reason = XmCR_ACTIVATE;
    c.event = NULL;
    XtCallCallbacks(filter_button, XmNactivateCallback, (XtPointer)&c);
}

void FileDialog::textCallback(Widget w)
{
    char *file = XmTextGetString(w);

    if(file) {
	struct stat buf;
	if( !stat(file, &buf) ) { // file exists
	    DIR *dirp;
	    if( (dirp = opendir(file)) ) { // file is a directory
		closedir(dirp);
		if(file_type == EXISTING_FILE_OR_DIR ||
		   file_type == EXISTING_DIR ||
		   file_type == DIR_ONLY)
		{
		    open_button->setSensitive(true);
		}
		else {
		    open_button->setSensitive(false);
		    append_button->setSensitive(false);
		}
	    }
	    else { // file is not a directory
		if(file_type == EXISTING_FILE_OR_DIR ||
		   file_type == EXISTING_FILE || file_type == FILE_ONLY)
		{
		    open_button->setSensitive(true);
		    append_button->setSensitive(true);
		}
		else if(file_type == WRITE_FILE) {
		    open_button->setLabel("Overwrite File");
		    open_button->setSensitive(true);
		    append_button->setVisible(true);
		    append_button->setSensitive(true);
		}
		else {
		    open_button->setSensitive(false);
		    append_button->setSensitive(false);
		}
	    }
	}
	else if(file_type == FILE_OR_DIR || file_type == FILE_ONLY ||
		file_type == DIR_ONLY)
	{
	    open_button->setSensitive(true);
	    deselectFiles();
	}
	else if(file_type == WRITE_FILE) {
	    open_button->setLabel("Write File");
	    open_button->setSensitive(true);
	    append_button->setVisible(false);
	    deselectFiles();
	}
	else {
	    open_button->setSensitive(false);
	    deselectFiles();
	}
	XtFree(file);
    }
}

/** Motif Apply Filter callback.
 */
void FileDialog::applyFilter(Widget widget, XtPointer client, XtPointer data)
{
    Widget filter_button = (Widget)client;
    XmAnyCallbackStruct c;
    c.reason = XmCR_ACTIVATE;
    c.event = NULL;
    XtCallCallbacks(filter_button, XmNactivateCallback, (XtPointer)&c);
}

void FileDialog::applyChoiceFilter(Widget widget, XtPointer client,
			XtPointer data)
{
    FileDialog *fd = (FileDialog *)client;
    XmAnyCallbackStruct c;
    Widget w;
    c.reason = XmCR_ACTIVATE;
    c.event = NULL;

    if(!strcmp(fd->filter_choice->getChoice(), "All Types") &&
        (w = fd->fsb->getChild(XmDIALOG_FILTER_TEXT)) != NULL)
    {
	char *text = XmTextGetString(w);
	int n = strlen(text);
	int m = strlen("/(All Types)");
	if(n < m || strcmp(text+n-m, "/(All Types)")) {
	    Arg args[1];
	    XtSetArg(args[0], XmNfileSearchProc, fd->defaultFileSearch);
	    fd->fsb->setValues(args, 1);
	}
	XtFree(text);
    }

    XtCallCallbacks(fd->filter_button, XmNactivateCallback, (XtPointer)&c);
}

/** Get the file selection with optional wait. If the argument is true,
 *  this function does not return until either the open button or the close
 *  button is selected. If the argument is false, the function returns
 *  immediately with the string in the file selection text field.
 *  @param[in] wait if true, the function does not return until the op
 *  @returns the string from the file selection window or NULL if there is no
 *  selection.
 */
char * FileDialog::getFile(bool wait)
{
    if(file_type == EXISTING_FILE) {
	Widget file_list = fsb->getChild(XmDIALOG_FILE_LIST);
	int nitems=0;
	XmString *items;
	Arg args[2];
	XtSetArg(args[0], XmNselectedItems, &items);
	XtSetArg(args[1], XmNselectedItemCount, &nitems);

	if(wait && !waitForAnswer()) return NULL;

	XtGetValues(file_list, args, 2);
	return (nitems > 0) ? getXmString(items[0]) : NULL;
    }
    else {
	if(wait) {
	    return (waitForAnswer()) ? selection_text->getString() : NULL;
	}
	else {
	    return selection_text->getString();
	}
    }
}

int FileDialog::getFile(vector<string> &filenames, bool wait)
{
    filenames.clear();

    if(file_type == EXISTING_FILE) {
	Widget file_list = fsb->getChild(XmDIALOG_FILE_LIST);
	int nitems=0;
	XmString *items;
	Arg args[2];
	XtSetArg(args[0], XmNselectedItems, &items);
	XtSetArg(args[1], XmNselectedItemCount, &nitems);

	if(wait && !waitForAnswer()) return 0;

	XtGetValues(file_list, args, 2);
	for(int i = 0; i < nitems; i++) {
	    char *s = getXmString(items[i]);
	    filenames.push_back(string(s));
	    XtFree(s);
	}
	return (int)filenames.size();
    }
    else {
	if(wait && !waitForAnswer()) return 0;
	char *s = selection_text->getString();
	filenames.push_back(string(s));
	XtFree(s);
	return 1;
    }
}

/** Get the file selection. This function does not return until the open
 *  button is selected.
 *  @returns the string from the file selection window or NULL if there is no
 *  selection.
 */
bool FileDialog::getFile(const string &name, Component *parent, string &file,
		const string &dir, const string &filter,
		const string &open_button_label,
		bool *append, const string &selection_label)
{
    FileType ftype = (append) ? WRITE_FILE : FILE_ONLY;
    FileDialog *fd = new FileDialog(name, parent, ftype, dir, filter,
				open_button_label);
    if( !selection_label.empty() ) {
	Widget w;
	if( (w = fd->fsb->getChild(XmDIALOG_SELECTION_LABEL)) ) {
	    Arg args[1];
	    XmString xm = createXmString(selection_label);
	    XtSetArg(args[0], XmNlabelString, xm);
	    XtSetValues(w, args, 1);
	    XmStringFree(xm);
	}
    }
    fd->setVisible(true);
    char *f = fd->getFile();
    if(append)  *append = fd->append_file;
    if(fd->dialog_state == DIALOG_CANCEL) Free(f);
    fd->destroy();
    if(f) {
	file.assign(f);
	return true;
    }
    else {
	file.clear();
	return false;
    }
}

/** Get the file selection. This function does not return until the open
 *  button is selected.
 *  @returns the string from the file selection window or NULL if there is no
 *  selection.
 */
bool FileDialog::getDir(const string &name, Component *parent, string &file,
		const string &dir, const string &filter,
		const string &open_button_label,
		const string &selection_label)
{
    FileDialog *fd = new FileDialog(name, parent, DIR_ONLY, dir, filter,
				open_button_label);
    if( !selection_label.empty() ) {
	Widget w;
	if( (w = fd->fsb->getChild(XmDIALOG_SELECTION_LABEL)) ) {
	    Arg args[1];
	    XmString xm = createXmString(selection_label);
	    XtSetArg(args[0], XmNlabelString, xm);
	    XtSetValues(w, args, 1);
	    XmStringFree(xm);
	}
    }
    fd->setVisible(true);
    char *f = fd->getFile();
    if(fd->dialog_state == DIALOG_CANCEL) Free(f);
    fd->destroy();
    if(f) {
	file.assign(f);
	return true;
    }
    else {
	file.clear();
	return false;
    }
}

/** Destructor.
 */
FileDialog::~FileDialog(void)
{
    if(directory != NULL) XtFree(directory);
    if(file_filter != NULL) XtFree(file_filter);
    for(int i = 0; i < (int)suffixes.size(); i++) {
	Free(suffixes[i]);
    }
    for(int i = 0; i < (int)gz_suffixes.size(); i++) {
	Free(gz_suffixes[i]);
    }
    XtRemoveTimeOut(time_out_id);
    for(int i = 0; i < (int)files.size(); i++) Free(files[i]);
}

void FileDialog::setVisible(bool visible)
{
    TopWindow::setVisible(visible);

    if(!visible && dialog_state == DIALOG_CANCEL) {
	dialog_state = DIALOG_CANCEL;
	FileDialogStruct c;
	c.state = DIALOG_CANCEL;
	c.append = false;
	doCallbacks(base_widget, (XtPointer)&c, XmNactivateCallback);
    }
}

/** Set the state to DIALOG_CANCEL and hide the FileDialog window.
 */
void FileDialog::dialogCancel(void)
{
    dialog_state = DIALOG_CANCEL;
    setVisible(false);
}

/** Set the state to DIALOG_APPLY,append=false and hide the FileDialog window.
 */
void FileDialog::dialogOpen(void)
{
    dialog_state = DIALOG_APPLY;
    append_file = false;
    setVisible(false);
    FileDialogStruct c;
    c.state = DIALOG_APPLY;
    c.append = false;
    getFile(c.files, false);
    doCallbacks(base_widget, (XtPointer)&c, XmNactivateCallback);
}

/** Set the state to DIALOG_APPLY,append=true and hide the FileDialog window.
 */
void FileDialog::dialogAppend(void)
{
    dialog_state = DIALOG_APPLY;
    append_file = true;
    setVisible(false);
    FileDialogStruct c;
    c.state = DIALOG_APPLY;
    c.append = true;
    getFile(c.files, false);
    doCallbacks(base_widget, (XtPointer)&c, XmNactivateCallback);
}

/** Motif drectory list selection callback. This callback is from the
 *  XmDIALOG_DIR_LIST widget.
 */
void FileDialog::openDirCB(Widget widget, XtPointer client_data, XtPointer data)
{
    FileDialog *fd = (FileDialog *)client_data;
    XmListCallbackStruct *c = (XmListCallbackStruct *)data;
    c->reason = XmCR_ACTIVATE;
    XtCallCallbacks(fd->filter_button, XmNactivateCallback, (XtPointer)c);

    fd->setSelection(widget);
}

void FileDialog::setSelection(Widget widget)
{
    if(file_type == EXISTING_FILE) return;

    XmString *selectedItems;
    int num_selected;
    XtVaGetValues(widget, XmNselectedItems, &selectedItems,
		XmNselectedItemCount, &num_selected, NULL);
    if(num_selected > 0) {
	XmTextPosition pos;
	char *dir = getXmString(selectedItems[0]);
	int n = (int)strlen(dir);
	if(n > 0 && dir[n-1] == '.') dir[n-1] = '\0';
	selection_text->setString(dir);
	pos = (XmTextPosition)strlen(dir);
	XtVaSetValues(selection_text->baseWidget(), XmNcursorPosition,pos,NULL);
	XtFree(dir);
    }
}

/** Show Preview Area.
 */
void FileDialog::showPreviewArea()
{
    int nitems=0;
    XmString *items;
    Arg args[2];
    vector<Component *> *children = rc->getChildren();

    for(int i = 1; i < (int)children->size(); i++) {
	children->at(i)->setVisible(true);
    }
    delete children;

    Widget file_list = fsb->getChild(XmDIALOG_FILE_LIST);
    XtSetArg(args[0], XmNselectedItems, &items);
    XtSetArg(args[1], XmNselectedItemCount, &nitems);
    XtGetValues(file_list, args, 2);

    if(nitems > 0) {
	struct stat buf;
	char *file = getXmString(items[0]);
	if(!stat(file, &buf) && !S_ISDIR(buf.st_mode)) {
	    previewFile(file);
	}
	Free(file);
    }
}

/** Hide Preview Area.
 */
void FileDialog::hidePreviewArea()
{
    vector<Component *> *children = rc->getChildren();

    for(int i = 1; i < (int)children->size(); i++) {
	children->at(i)->setVisible(false);
    }
    delete children;
}

/** Motif file list selection callback. This callback is from the XmDIALOG_LIST
 *  widget.
 */
void FileDialog::fileSelect(Widget widget, XtPointer client, XtPointer data)
{
    FileDialog *fd = (FileDialog *)client;
    Widget file_list = fd->fsb->getChild(XmDIALOG_FILE_LIST);
    Widget text;
    char *file;
    XmTextPosition pos;

    if(fd->file_type == EXISTING_FILE) {
	int nitems=0;
	XmString *items;
	Arg args[2];
	XtSetArg(args[0], XmNselectedItems, &items);
	XtSetArg(args[1], XmNselectedItemCount, &nitems);
	XtGetValues(file_list, args, 2);
	if(nitems == 1) {
	    file = getXmString(items[0]);
	    fd->previewFile(file);
	    XtFree(file);
	}
    }
    else if((text = fd->fsb->getChild(XmDIALOG_TEXT)) != NULL
	&& (file = XmTextGetString(text)))
    {
	fd->selection_text->setString(file, true);
	pos = (XmTextPosition)strlen(file);
	XtVaSetValues(fd->selection_text->baseWidget(), XmNcursorPosition, pos,
			NULL);
	fd->previewFile(file);
	XtFree(file);
    }
    fd->doCallbacks((XtPointer)file_list, XtNfileSelectCallback);
}

void FileDialog::fileSearch(Widget w, XmFileSelectionBoxCallbackStruct *c)
{
    for(int i = 0; i < num_fsb_list; i++) {
	if(fsb_list[i].fsb == w) {
	    string dir;
	    getXmString(c->dir, dir);
	    fsb_list[i].fd->doFileSearch(dir, "");
	    return;
	}
    }
}

void FileDialog::fileSearch2(Widget w, XmFileSelectionBoxCallbackStruct *c)
{
    for(int i = 0; i < num_fsb_list; i++) {
	if(fsb_list[i].fsb == w) {
	    string dir;
	    getXmString(c->dir, dir);
	    fsb_list[i].fd->doFileSearch(dir, fsb_list[i].fd->gz_ftype);
	    return;
	}
    }
}

void FileDialog::doFileSearch(const string &dir, const string &ftype)
{
    Arg  args[4];
    int	 n, nlist, m;
    char type[200];
    DIR	 *dirp;
    struct dirent *dp;
    string path;
    XmString *list = NULL;

    if((dirp = opendir(dir.c_str())) == NULL)
    {
	n = 0;
	XtSetArg(args[n], XmNfileListItems, 0); n++;
	XtSetArg(args[n], XmNfileListItemCount, 0); n++;
	XtSetArg(args[n], XmNdirectoryValid, True); n++;
	XtSetArg(args[n], XmNlistUpdated, True); n++;
	fsb->setValues(args, n);
	return;
    }

    nlist = 0;
    if(!(list = (XmString *)mallocWarn(sizeof(XmString)))) return;

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	DIR *d;

	n = (int)dir.length();
	if(dir[n-1] != '/') {
	    path = dir + "/" + dp->d_name;
	}
	else {
	    path = dir + dp->d_name;
	}

	if((d = opendir(path.c_str())) != NULL) {
	    closedir(d);
	}
	if((int)ftype.length() == 0) {
	    n = (int)strlen(dp->d_name);
	    for(int i = 0; i < (int)suffixes.size(); i++)
	    {
		snprintf(type, sizeof(type), ".%s", suffixes[i]);
		m = (int)strlen(type);
		if(n >= m && !strcasecmp(dp->d_name+n-m, type)) {
		    if(!(list = (XmString *)reallocWarn(list,
					(nlist+1)*sizeof(XmString)))) return;
		    list[nlist++] = createXmString(path);
		}
		if(m > 4 && !strcasecmp(type+m-3, ".gz")) {
		    type[m-3] = '\0';
		    if(!strcasecmp(dp->d_name+n-m+3, type)) {
			if(!(list = (XmString *)reallocWarn(list,
				(nlist+1)*sizeof(XmString)))) return;
			list[nlist++] = createXmString(path);
		    }
		}
	    }
	}
	else {
	    n = (int)strlen(dp->d_name);
	    snprintf(type, sizeof(type), ".%s", ftype.c_str());
	    m = (int)strlen(type);
	    if(n >= m && !strcasecmp(dp->d_name+n-m, type)) {
		if(!(list = (XmString *)reallocWarn(list,
					(nlist+1)*sizeof(XmString)))) return;
		list[nlist++] = createXmString(path);
	    }
	    if(m > 4 && !strcasecmp(type+m-3, ".gz")) {
		type[m-3] = '\0';
		if(!strcasecmp(dp->d_name+n-m+3, type)) {
		    if(!(list = (XmString *)reallocWarn(list,
				(nlist+1)*sizeof(XmString)))) return;
		    list[nlist++] = createXmString(path);
		}
	    }
	}
    }
    closedir(dirp);

    n = 0;
    if(nlist > 0) {
	XtSetArg(args[n], XmNfileListItems, list); n++;
    }
    else {
	XtSetArg(args[n], XmNfileListItems, 0); n++;
    }
    XtSetArg(args[n], XmNfileListItemCount, nlist); n++;
    XtSetArg(args[n], XmNlistUpdated, True); n++;
    XtSetArg(args[n], XmNdirectoryValid, True); n++;
    fsb->setValues(args, n);

    for(int i = 0; i < nlist; i++) XmStringFree(list[i]);
    Free(list);
}

void FileDialog::checkForNewFiles(XtPointer client_data, XtIntervalId id)
{
    FileDialog *fd = (FileDialog *)client_data;
    fd->checkFiles();
    fd->time_out_id = XtAppAddTimeOut(Application::getAppContext(), 1000,
		(XtTimerCallbackProc)checkForNewFiles, client_data);
}

void FileDialog::checkFiles(void)
{
    Arg args[1];
    XmString xm;
    DIR *dirp;
    struct dirent *dp;

    XtSetArg(args[0], XmNdirectory, &xm);
    XtGetValues(fsb->baseWidget(), args, 1);

    char *dir = getXmString(xm);
    XmStringFree(xm);
    if(!current_directory || strcmp(dir, current_directory))
    {
	Free(current_directory);
	current_directory = strdup(dir);
	for(int i = 0; i < (int)files.size(); i++) Free(files[i]);
	files.clear();

	if( (dirp = opendir(current_directory)) )
	{
	    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	    {
		files.push_back(strdup(dp->d_name));
	    }
	    closedir(dirp);
	}
    }
    else {
	bool dir_changed = false;
	int nfiles = 0;
	if( (dirp = opendir(current_directory)) )
	{
	    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
	    {
		if(nfiles >= (int)files.size()
			|| strcmp(files[nfiles], dp->d_name))
		{
		    dir_changed = true;
		    break;
		}
		nfiles++;
	    }
	    if(dir_changed) {
		rewinddir(dirp);
		for(int i = 0; i < (int)files.size(); i++) Free(files[i]);
		files.clear();
		for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
		{
		    files.push_back(strdup(dp->d_name));
		}
		deselectFiles();
		XmAnyCallbackStruct c;
		c.reason = XmCR_ACTIVATE;
		c.event = NULL;
		XtCallCallbacks(filter_button, XmNactivateCallback,
			(XtPointer)&c);
	    }
	    closedir(dirp);
	}
    }
    Free(dir);
}

void FileDialog::fileSelectCB(Widget widget, XtPointer client, XtPointer data)
{
    FileDialog *fd = (FileDialog *)client;
    int num=0;
    Arg args[1];
    Widget file_list = fd->fsb->getChild(XmDIALOG_FILE_LIST);

    XtSetArg(args[0], XmNselectedItemCount, &num);
    XtGetValues(file_list, args, 1);

    fd->open_button->setSensitive(num > 0 ? true : false);
}

void FileDialog::deselectFiles(void)
{
    Widget w;
    if( (w = fsb->getChild(XmDIALOG_LIST)) ) {
	XmListDeselectAllItems(w);
    }
}

void FileDialog::forceFilter(XtPointer client_data, XtIntervalId id)
{
    FileDialog *fd = (FileDialog *)client_data;
    fd->doFilterChoice();
}

ParseVar FileDialog::parseVar(const string &name, string &value)
{
    if(parseCompare(name, "getSelection")) {
	getFile(value);
	return STRING_RETURNED;
    }
    return FormDialog::parseVar(name, value);
}
