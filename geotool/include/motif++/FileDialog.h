#ifndef _FILE_DIALOG_H_
#define _FILE_DIALOG_H_

#include "motif++/TopWindow.h"

#define XtNfileSelectCallback	(char *)"fileSelectCallback"

class Button;
class Toggle;
class RowColumn;
class Separator;
class FileSelectionBox;
class Form;
class Label;
class Choice;
class TextField;

enum FileType {
    EXISTING_FILE_OR_DIR,
    EXISTING_FILE,
    EXISTING_DIR,
    FILE_OR_DIR,
    FILE_ONLY,
    DIR_ONLY,
    WRITE_FILE
};

/** The FileDialog XmNactivateCallback structure.
 */
typedef struct
{
    int	 state;	//!< DIALOG_APPLY or DIALOG_CANCEL
    bool append;
    vector<string> files; //!< the selected files
} FileDialogStruct;

/** A subclass of TopWindow for the selection of files or directories. This
 *  class utilitzes the Motif FileSelectionBox interface. It changes the
 *  behavior and adds a couple of options. A single click on the directory list
 *  updates the file list. A directory-only mode is available which allows
 *  only directories to be selected. An optional Preview Toggle will display a
 *  preview area to the right of the File Selection Box for use by subclasses.
 *
 *  There are several methods of getting the file or directory selection from
 *  the FileDialog. The first method is illustrated in the example code below.
 *  The static function FileDialog::getFile(const string &, Component *, char *,
 *  char *, char *) creates a new FileDialog window each time it is called and
 *  does not return until the Open or Close button is selected. The FileDialog
 *  window is destroyed when getFile returns.
 *  \include motif++/FileDialogExample1.cpp
 *
 *  A variation of the method above is shown below, where the FileDialog is
 *  first created once and set visible when needed. The function getFile(void)
 *  does not return until either the Open or Close button is selected.
 *  \include motif++/FileDialogExample2.cpp
 *
 *  The final method is illustrated in the example code below. The FileDialog is
 *  created once and set visible when needed. But instead of using the
 *  getFile(void) function, the Example class listens to the file_dialog
 *  instance for an action by the user. The actionPerformed function handles
 *  the callback. The main difference in this method is that the rest of a
 *  program's interface is not blocked while the FileDialog is visible.
 *  \include motif++/FileDialogExample3.cpp
 *  @ingroup libmotif
 */
class FileDialog : public TopWindow
{
    public:
	FileDialog(const string &name, Component *parent, FileType type,
		const string &dir, const string &filter,
		const string &open_button_label="Open");
	FileDialog(const string &name, Component *parent, FileType type,
		const string &dir, int num_file_suffixes,
		const char **file_suffixes, const string &initial_file_suffix,
		const string &open_button_label);

	~FileDialog(void);
	void setVisible(bool visible);

	char *getFile(bool wait=true);

	bool getFile(string &file, bool wait=true) {
	    char *s = getFile(wait);
	    if(s) { file.assign(s); free(s); return true; }
	    else { file.assign(""); return false; }
	}
	int getFile(vector<string> &files, bool wait=true);

	/** Get the controls RowColumn
	 *  @returns the controls RowColumn Component
	*/
	RowColumn *getControls(void) { return controls; }
	/** Get the Open Button
	 *  @returns the Open Button Component
	*/
	Button *getOpenButton(void) { return open_button; }
	/** Get the preview Toggle
	 *  @returns the Preview Toggle
	 */
	RowColumn *previewParent() { return rc; }
	/** Preview the selected file. This function should be reimplemented in
	 *  subclasses to display a preview of the file in the preview area.
	 */
	virtual bool previewFile(const string &file) { return true; }
	virtual void showPreviewArea(void);
	virtual void hidePreviewArea(void);

	void actionPerformed(ActionEvent *action_event);
	ParseVar parseVar(const string &name, string &value);

	/** Get the file selection for writing.
	 *  @param[in] name name given to this FileDialog instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] dir the initial directory listed.
	 *  @param[in] filter the file filter.
	 *  @param[in] open_button_label the open buttob label.
	 *  @param[out] append if true, append to an existing file. If false,
	 *		write a new file.
	 *  @param[in] selection_label selection label.
	 */
	static bool getFile(const string &name, Component *parent, string &file,
		const string &dir="./", const string &filter="*",
		const string &open_button_label="Open", bool *append=NULL,
		const string &selection_label="");

	/** Get the directory selection. Only directories can be selected. The
	 *  open button label is "Open".
	 *  @param[in] name name given to this FileDialog instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] dir the initial directory listed.
	 *  @param[in] filter the file filter.
	 *  @param[in] open_button_label the label for the Open button.
	 *  @param[in] selection_label selection label.
	 */
	static bool getDir(const string &name, Component *parent, string &file,
		    const string &dir="./", const string &filter="*",
		    const string &open_button_label="Open",
		    const string &selection_label="");

	static char * getFile(const string &name, Component *parent,
		const string &dir="./", const string &filter="*",
		const string &open_button_label="Open", bool *append=NULL,
		const string &selection_label="")
	{
	    string file;
	    if(getFile(name, parent, file, dir, filter, open_button_label,
			append, selection_label)) return strdup(file.c_str());
	    return NULL;
	}

    protected:
	/** @name Interface Components */
	//@{
	RowColumn	*rc;	//!< Contains form and optional preview area
	RowColumn	*rc2;
	RowColumn	*controls;	//!< Contains Close, Open and Preview
	Button		*open_button;	//!< Open Button
	Button		*append_button;	//!< Append Button
	Button		*close_button;	//!< Close Button
	Separator	*sep;		//!< Separator above controls
	Form		*form;		//!< Contains FileSelectionBox fsb
	FileSelectionBox *fsb;		//!< FileSelectionBox Component
	Label		*filter_label;
	Choice		*filter_choice;
	TextField	*selection_text;
	//@}

	char		*directory;	//!< Initial directory
	char		*file_filter;	//!< File filter
	FileType	file_type;	//!< The allowed file or directory flag
	Widget		filter_button;
	XmSearchProc	defaultFileSearch;
	vector<char *>	suffixes, gz_suffixes;
	bool		append_file;
	string		gz_ftype;

	void createInterface(const string &dir, const string &filter,
		const string &open_button_label, FileType type,
		int num_file_suffixes, const char **file_suffixes,
		const string &initial_file_suffix);

	void setFileSuffixes(int num_file_suffixes, const char **suffixes,
		const string &initial_suffix);
	static void fileSearch(Widget w, XmFileSelectionBoxCallbackStruct *c);
	static void fileSearch2(Widget w, XmFileSelectionBoxCallbackStruct *c);
	void doFilterChoice(void);
	void doFileSearch(const string &dir, const string &ftype);
	void deselectFiles(void);
	static void fileSelectCB(Widget w, XtPointer client, XtPointer data);

    private:
	static void openDirCB(Widget, XtPointer, XtPointer);
	static void applyFilter(Widget, XtPointer, XtPointer);
	static void applyChoiceFilter(Widget, XtPointer, XtPointer);
	static void fileSelect(Widget, XtPointer, XtPointer);
	static void checkForNewFiles(XtPointer client_data, XtIntervalId id);
	static void forceFilter(XtPointer client_data, XtIntervalId id);

	void checkFiles(void);
	void textCallback(Widget w);
	void dialogCancel(void);
	void dialogOpen(void);
	void dialogAppend(void);
	void setSelection(Widget widget);

	XtIntervalId time_out_id;
	vector<char *> files;
	char *current_directory;
};
#endif
