#ifndef FORM_DIALOG_H
#define FORM_DIALOG_H

#include "motif++/Component.h"

enum DialogPosition
{
    OFFSET_DIALOG,
    CENTER_DIALOG,
    NO_POSITION_DIALOG
};

enum ShowHelpRet
{
    HELP_SUCCESS,
    NO_HELP_KEY_FILE,
    HELP_KEY_NOT_FOUND,
    HELP_FORMAT_ERROR,
    INVALID_HELP_TYPE,
    NO_HELP_FOUND,
    HELP_FILE_NOT_FOUND
};

/** A subclass of Component based on the XmFormDialog widget. This class
 *  provides the basic functionality for popup windows.
 *  @ingroup libmotif
 */
class FormDialog : public Component
{
    public:

	FormDialog(const string &name, Component *parent, const string &title,
		bool focus_pointer=false, bool independent=true);
	FormDialog(const string &name, Component *parent,
		bool focus_pointer=false, bool independent=true);
	~FormDialog(void);

	void init(const string &title, bool pointer_focus, bool independent);
	void setVisible(bool);
	void setVisible(bool, DialogPosition);
	bool isVisible(void);
	bool isManaged(void) { return (bool)XtIsManaged(baseWidget()); }
	virtual void windowManagerClose(void) { setVisible(false); }
	void setTitle(const string &title);
	void destroy(void);
	void manage(void);
	void setSize(int width, int height);
	Position initialX(void) {return initial_x;}
	Position initialY(void) {return initial_y;}
	Dimension initialWidth(void) {return initial_width;}
	Dimension initialHeight(void) {return initial_height;}
	bool isIndependent(void) {return indep;}
	virtual FormDialog *getFormDialogInstance(void) { return this; }
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);
	void getParsePrefix(char *prefix, int prefix_len);
	ParseCmd parseChoiceQuestion(const string &s, string &msg);
	ParseCmd parseTextQuestion(const string &s, string &msg);
	ParseCmd parseListDialog(const string &cmd, string &msg);
	ParseCmd parseShowMessage(const string &cmd, string &msg);
	ParseCmd parseGetFile(bool get_file, const string &cmd, string &msg);

	ShowHelpRet showHelp(const string &key,
			const string &help_key_file="doc/help_key");
	void displayHelp(const string &message);

    protected:

#define DIALOG_WAITING	0
#define DIALOG_CANCEL	1
#define DIALOG_APPLY	2

	long dialog_state;
	bool indep;
	bool positioned;
	Position initial_x, initial_y;
	Dimension initial_width, initial_height;

	void positionOffset(void);
	void positionCenter(void);
	void setAutoUnmanage(bool state);
        void setResource(const char *res);
        void setResource(const char *format, const char *res);
	bool waitForAnswer(void);

    private:
	static void mapCallback(Widget, XtPointer, XtPointer);
	static void popdownCB(Widget, XtPointer, XtPointer);
	static void HandleWindowDel(Widget, XtPointer, XEvent *, Boolean *);
	static void getPositionCB(XtPointer data, XtIntervalId *id);
};
#endif
