#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "motif++/Component.h"

/** A class for the XmText widget.
 *  @ingroup libmotif
 */
class TextField : public Component
{
    public:

	TextField(const string &name, Component *parent,
		ActionListener *listener=NULL, Arg *args=NULL, int n=0);
	TextField(const string &name, Component *parent, Arg *args, int n);
	~TextField(void);

	void setString(const string &, bool do_callback=false);
	void setString(const string &, double, bool do_callback=false);
	void setString(const string &, double, double);
	void setString(long l, bool do_callbacks=false);
	void append(const string &s);
	void appendString(const string &s);
	char *getString(void);
	bool getString(char *str, int len);
	bool getString(string &str);
	char *getFullString(void);
	bool getDouble(double *d);
	bool getInt(int *i);
	bool getLong(long *l);
	bool getTime(double *d);
	void setActivationKeys(const string &keys);
	bool equals(const string &s);
	int maxLines(void) { return max_lines; }
	void setMaxLines(int num);
	void tabKeyFocus(void);

	/** Force the text at the charactor position to be displayed.
	 *  @param[in] position the charactoer position to be displayed.
	 */
	void showPosition(int position) {
	    XmTextShowPosition(base_widget, (XmTextPosition)position); }
	bool empty(void);

	void disableRedisplay() { XmTextDisableRedisplay(base_widget); }
	void enableRedisplay() { XmTextEnableRedisplay(base_widget); }

	virtual TextField *getTextFieldInstance(void) { return this; }

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);

	static char *getAbsolutePath(const string &path);
	static char *cleanPath(const string &path);
	static bool getAbsolutePath(const string &path, string &newpath) {
	    char *p = getAbsolutePath(path);
	    if(!p) return false;
	    newpath.assign(p);
	    free(p);
	    return true;
	}
	static bool cleanPath(const string &path, string &newpath) {
	    char *p = cleanPath(path);
	    if(!p) return false;
	    newpath.assign(p);
	    free(p);
	    return true;
	}

    protected:
	bool ignore_set_text;
	int max_lines;
	string activation_keys;

	void init(Component *, Arg *args, int n);

    private:
	static void valueChangedCallback(Widget, XtPointer, XtPointer);
	static void activateCallback(Widget, XtPointer, XtPointer);
	static void verifyCallback(Widget, XtPointer, XtPointer);
};

#endif
