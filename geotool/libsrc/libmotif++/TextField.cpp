/** \file TextField.cpp
 *  \brief Defines class TextField.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sys/param.h>
#include "motif++/TextField.h"
#include "motif++/Parse.h"
using namespace Parse;
extern "C" {
#include "libstring.h"
#include "libtime.h"
}

/** Constructor with X-resources and listener.
 *  @param[in] name the name given to this TextField instance.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNactivateCallback actions.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
TextField::TextField(const string &name, Component *parent,
		ActionListener *listener, Arg *args, int n)
		: Component(name, parent)
{
    init(parent, args, n);
    if(listener) addActionListener(listener, XmNvalueChangedCallback);
}

/** Constructor.
 *  @param[in] name the name given to this TextField instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
TextField::TextField(const string &name, Component *parent, Arg *args, int n) :
			Component(name, parent)
{
    init(parent, args, n);
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
void TextField::init(Component *parent, Arg *args, int n)
{
    if(!args) {
	base_widget = XtVaCreateManagedWidget(getName(),
			xmTextWidgetClass, parent->baseWidget(), NULL);
    }
    else {
	base_widget = XtCreateManagedWidget(getName(),
			xmTextWidgetClass, parent->baseWidget(), args, n);
    }
    installDestroyHandler();

    XtAddCallback(base_widget, XmNvalueChangedCallback,
		TextField::valueChangedCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNactivateCallback,
		TextField::activateCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNmodifyVerifyCallback,
		TextField::verifyCallback, (XtPointer)this);

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XmNvalueChangedCallback);
    enableCallbackType(XmNmodifyVerifyCallback);

    ignore_set_text = false;
    max_lines = 0;
}

/** Destructor. */
TextField::~TextField(void)
{
    if(base_widget) {
	XtRemoveAllCallbacks(base_widget, XmNactivateCallback);
	XtRemoveAllCallbacks(base_widget, XmNvalueChangedCallback);
	XtRemoveAllCallbacks(base_widget, XmNmodifyVerifyCallback);
    }
}

void TextField::valueChangedCallback(Widget w, XtPointer clientData,
				XtPointer data)
{
    TextField *text = (TextField *)clientData;

    if(!text->ignore_set_text) {
	text->doCallbacks(w, data, XmNvalueChangedCallback);
    }
}

void TextField::activateCallback(Widget w, XtPointer clientData, XtPointer data)
{
    TextField *text = (TextField *)clientData;

    if(!text->ignore_set_text) {
	text->doCallbacks(w, data, XmNactivateCallback);
    }
}

void TextField::verifyCallback(Widget w, XtPointer clientData, XtPointer data)
{
    TextField *text = (TextField *)clientData;

    if(text->ignore_set_text) return;

    text->doCallbacks(w, data, XmNmodifyVerifyCallback);

    if(!text->activation_keys.empty()) {
	XmTextVerifyCallbackStruct *c = (XmTextVerifyCallbackStruct *)data;

	if(c->event != NULL && c->event->type != KeyPress) return;

        if(c->text->length > 0 && c->text->ptr[0] == '\n')
	{
	    char *s = XmTextGetString(text->base_widget);
	    if(stringEndsWith(s, text->activation_keys.c_str())) {
		text->doCallbacks(w, data, XmNactivateCallback);
                /* don't allow '\n' to be printed after activation_keys */
                c->text->length = 0;
            }
	    if(s) free(s);
        }
    }
}

/** Set the text with a two double arguments.
 *  @param[in] format the format for printing the double arguments.
 *  @param[in] d1
 *  @param[in] d2
 */
void TextField::setString(const string &format, double d1, double d2)
{
    if(base_widget) {
	bool ignore = ignore_set_text;
	ignore_set_text = true;
	char text[50];
	snprintf(text, sizeof(text), format.c_str(), d1, d2);
	XmTextSetString(base_widget, text);
	ignore_set_text = ignore;
    }
}

/** Set the text with callback option.
 *  @param[in] s the new text.
 *  @param[in] do_callbacks if true, do callbacks to all listeners.
 */
void TextField::setString(const string &s, bool do_callbacks)
{
    if(base_widget) {
	bool ignore = ignore_set_text;
	ignore_set_text = !do_callbacks;
	XmTextSetString(base_widget, (char *)s.c_str());
	ignore_set_text = ignore;
    }
}

/** Set the text with double argument and callback option.
 *  @param[in] format the format for printing the double argument.
 *  @param[in] d the double value.
 *  @param[in] do_callbacks if true, do callbacks to all listeners.
 */
void TextField::setString(const string &format, double d, bool do_callbacks)
{
    if(base_widget) {
	bool ignore = ignore_set_text;
	ignore_set_text = !do_callbacks;
	char text[100];
	snprintf(text, sizeof(text), format.c_str(), d);
	XmTextSetString(base_widget, text);
	ignore_set_text = ignore;
    }
}

/** Set the text with an integer argument and callback option.
 *  @param[in] i the integer value to display.
 *  @param[in] do_callbacks if true, do callbacks to all listeners.
 */
void TextField::setString(long i, bool do_callbacks)
{
    if(base_widget) {
	bool ignore = ignore_set_text;
	ignore_set_text = !do_callbacks;
	char text[50];
	snprintf(text, sizeof(text), "%ld", i);
	XmTextSetString(base_widget, text);
	ignore_set_text = ignore;
    }
}

/** Get the text. White space is removed from both ends.
 *  @returns a pointer to the text string. Free the pointer when no longer
 *  needed.
 */
char *TextField::getString(void)
{
    if(base_widget) {
	char *s = XmTextGetString(base_widget);
	if(s) stringTrim(s);
	else s = strdup("");
	return s;
    }
    return strdup("");
}

/** Get the text. Fill the input string. White space is removed from both ends.
 *  @param[in,out] str a string of length len.
 *  @param[in] len the length of str.
 */
bool TextField::getString(char *str, int len)
{
    string s;
    getString(s);

    snprintf(str, len, "%s", s.c_str());
    return (str[0] != '\0') ? true : false;
}

/** Get the text. Assign the text to the input string. White space is removed
 *  from both ends.
 *  @param[in,out] str a string reference.
 */
bool TextField::getString(string &str)
{
    char *s = getString();

    str.assign(s);
    free(s);
    return (str.length() > 0) ? true : false;
}

/** Get the text. White space is not removed.
 *  @returns a pointer to the text string. Free the pointer when no longer
 *  needed.
 */
char *TextField::getFullString(void)
{
    if(base_widget) {
	return XmTextGetString(base_widget);
    }
    return NULL;
}

/** Append to the text.
 *  @param[in] s the string to append.
 */
void TextField::append(const string &s)
{
    XmTextPosition pos;

    if(!base_widget) return;

    bool ignore = ignore_set_text;
    ignore_set_text = true;
    if(max_lines > 0) {
	int num = 1;
	for(int i = (int)s.length()-1; i >= 0; i--) {
	    if(s[i] == '\n') num++;
	    if(num > max_lines) {
		cerr << "TextField:append warning: input string > maxLines()\n";
		XmTextSetString(base_widget, (char *)s.c_str()+i+1);
		return;
	    }
	}
	char *p = XmTextGetString(base_widget);
	for(int i = (int)strlen(p)-1; i >= 0; i--) {
	    if(p[i] == '\n') num++;
	    if(num > max_lines) {
		XmTextSetString(base_widget, p+i+1);
	    }
	}
	free(p);
    }
    pos = XmTextGetLastPosition(base_widget);
    XmTextInsert(base_widget, pos, (char *)s.c_str());
    ignore_set_text = ignore;
}

/** Set the maximum number of lines. When the number of lines reaches the
 *  maximum allowed, appending a new line forces the first line to be discarded.
 *  @param[in] num_lines the maximum number of lines allowed in the TextField
 */
void TextField::setMaxLines(int num_lines)
{
    if(!base_widget) return;

    if(num_lines <= 0) {
	max_lines = 0;
	return;
    }
    max_lines = num_lines;

    bool ignore = ignore_set_text;
    ignore_set_text = true;
    int num = 1;
    char *p = XmTextGetString(base_widget);
    for(int i = (int)strlen(p)-1; i >= 0; i--) {
	if(p[i] == '\n') num++;
	if(num > max_lines) {
	    XmTextSetString(base_widget, p+i+1);
	}
    }
    free(p);
    ignore_set_text = ignore;
}

/** Append to the text. Append ';\n' to the end of the current string before
 *  appending the input string.
 *  @param[in] s the string to append.
 */
void TextField::appendString(const string &s)
{
    if(base_widget) {
	bool ignore = ignore_set_text;
        XmTextPosition pos = XmTextGetLastPosition(base_widget);
        ignore_set_text = true;
        if(pos > 0) {
            XmTextInsert(base_widget, pos, (char *)";\n");
            pos += 2;
        }
        XmTextInsert(base_widget, pos, (char *)s.c_str());
        XmTextSetInsertionPosition(base_widget, 0);
        ignore_set_text = ignore;
    }
}

/** Get a double value.
 *  @param[out] d the double value.
 *  @returns true if the text was parsed as a double. returns false if the text
 *  is not a valid double value.
 */
bool TextField::getDouble(double *d)
{
    string s;
    getString(s);
    if(!s.empty() && stringToDouble(s.c_str(), d)) return true;
    return false;
}

/** Get an integer value.
 *  @param[out] i the integer value.
 *  @returns true if the text was parsed as an integer. returns false if the
 *  text is not a valid integer value.
 */
bool TextField::getInt(int *i)
{
    string s;
    getString(s);
    if(!s.empty() && stringToInt(s.c_str(), i)) return true;
    return false;
}

/** Get an long integer value.
 *  @param[out] i the long integer value.
 *  @returns true if the text was parsed as an integer. returns false if the
 *  text is not a valid integer value.
 */
bool TextField::getLong(long *i)
{
    string s;
    getString(s);
    if(!s.empty() && stringToLong(s.c_str(), i)) return true;
    return false;
}

/** Get a time value. Parse the text string as a time and return the epochal
 *  time value.
 *  @param[out] d the epochal time value.
 *  @returns true if the string parsed as a time value. return false if it
 *  could not be parsed.
 */
bool TextField::getTime(double *d)
{
    string s;
    getString(s);
    if(!s.empty() && timeParseString(s.c_str(), d) == 1) return true;
    return false;
}

/** Set activation characters. After this sequence of characters is typed into
 *  the text window, the XmNactivateCallback callbacks are called.
 *  @param[in] keys a sequence of one or more characters that triggers the
 *   XmNactivateCallback callbacks.
 */
void TextField::setActivationKeys(const string &keys)
{
    activation_keys.assign(keys);
}

bool TextField::equals(const string &s)
{
    string t;
    getString(t);
    return !t.compare(s) ? true : false;
}

/** Check for an empty TextField.
 *  @returns true if there are no characters in the TextField window.
 */
bool TextField::empty(void)
{
    string s;
    getString(s);
    return s.empty();
}	

/** Force tab-key focus transversal behavior when in multi-line-edit mode.
 *  The default behavior is for the tab-key to transfer focus only when the
 *  TextField editMode is SINGLE_LINE_EDIT. If editMode is MULTI_LINE_EDIT,
 *  the tab-key normally does not transfer the keyboard focus. This routine
 *  insures that the tab-binding for transfering focus is also in place
 *  when the edit mode is MULTI_LINE_EDIT.
 */
void TextField::tabKeyFocus(void)
{
    char trans[] = "<Key>Tab:next-tab-group()";
    XtTranslations translations = XtParseTranslationTable(trans);

    XtOverrideTranslations(base_widget, translations);
}

ParseCmd TextField::parseCmd(const string &cmd, string &msg)
{
    string s;
    int do_callback = 0;

    stringGetBoolArg(cmd.c_str(), "notify", &do_callback);

    if(parseGetArg(cmd, "text", s)) {
	setString(s, (bool)do_callback);
	return COMMAND_PARSED;
    }
    return ARGUMENT_ERROR;
}

ParseVar TextField::parseVar(const string &name, string &value)
{
    string s;
    getString(s);
    value.assign(s);
    return STRING_RETURNED;
}

// static
char * TextField::getAbsolutePath(const string &path)
{
    char *cwd;

    if((int)path.length() > 0 && path[0] == '/') {
	return cleanPath(path);
    }
    else if((cwd = (char *)getenv("PWD")) && cwd[0] != '\0') {
	return cleanPath(string(cwd) + "/" + path);
    }
    else {
	cerr << "getAbsolutePath: cannot get current working directory" << endl;
    }
    return NULL;
}

// static
char * TextField::cleanPath(const string &path)
{
    int i, j;
    char p[MAXPATHLEN+1];

    if((int)path.length() > 0 && path[0] != '/') {
	cerr << "cleanPath: path does not start with '/'" << endl;
	return NULL;
    }
    i = (int)sizeof(p)-1;
    strncpy(p, path.c_str(), i);
    p[i] = '\0';

/*
    // check that there are not more that two '.'s together, and
    // check that every '.' is preceded with '.' or '/'
    for(i = 1; p[i] != '\0'; i++) {
	if(p[i] == '.') {
	    if(p[i+1] == '.' && p[i+2] == '.') break;
	    if(p[i-1] != '.' && p[i-1] != '/') break;
	}
    }
    if(p[i] != '\0') {
	cerr << "cleanPath: invalid path: " << path << endl;
	return NULL;
    }
*/
	
    // /home/user/geotool/bin/../..//data/local.wfdisc
    // remove multiple '/'s
    j = 1;
    for(i = 1; p[i] != '\0'; i++) {
	if(p[i] != '/' || p[i-1] != '/') p[j++] = p[i];
    }
    p[j] = '\0';
    if(j == 0) return NULL; // can't really happen

    // remove all './' that are not '../'
    j = 1;
    for(i = 1; p[i] != '\0'; i++) {
	if(p[i-1] != '.' && p[i] == '.' && p[i+1] == '/') {
	    i++; // skip p[i] and p[i+1]
	}
	else {
	    p[j++] = p[i];
	}
    }
    p[j] = '\0';
    if(j == 0) return NULL; // can't really happen

    // remove all '../' from the path
    // /home/user/geotool/bin/../../data/local.wfdisc
    j = 0;
    for(i = 0; p[i] != '\0'; i++) {
	if(!strncmp(p+i, "/../", 4)) {
	    // go back to the previous '/'
	    j--;
	    while(j > 0 && p[j] != '/') j--;
	    if(j < 0) {
		cerr << "cleanPath: invalid path: " << path << endl;
		return NULL;
	    }
	    // j points to previous '/'

	    // skip over '/..'
	    i += 2;
	}
	else {
	    p[j++] = p[i];
	}
    }
    p[j] = '\0';
    return strdup(p);
}
