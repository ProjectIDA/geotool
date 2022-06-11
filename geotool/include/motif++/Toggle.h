#ifndef TOGGLE_H
#define TOGGLE_H

#include "motif++/Component.h"

/** A class for the XmToggleButton widget.
 *  @ingroup libmotif
 */
class Toggle : public Component
{
    public:

	Toggle(const string &name, Component *parent, ActionListener *listener,
		bool one_of_many, Arg *args=NULL, int n=0);
	Toggle(const string &name, Component *parent,
		ActionListener *listener=NULL, Arg *args=NULL, int n=0);
	Toggle(const string &name, Component *parent, Arg *args, int n);
	Toggle(const string &name, Component *parent, int position,
		ActionListener *listener=NULL);
	
	~Toggle(void);

	void set(bool state, bool do_callbacks=false);
	bool state(void);
	void setLabel(const string &label);
        char *getLabel(void);

	virtual Toggle *getToggleInstance(void) { return this; }
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	static void parseHelp(const char *prefix);

    protected:
	void init(Component *parent, bool one_of_many, Arg *args, int n);

    private:
	static void valueChangedCallback(Widget, XtPointer, XtPointer);
};

#endif
