#ifndef _BUTTON_H
#define _BUTTON_H

#include "motif++/Component.h"

/** A class for the XmPushButton widget.
 *  @ingroup libmotif
 */
class Button : public Component
{
    public:

	/** Constructor. */
	Button(const string &name, Component *parent,
		ActionListener *listener=NULL);
	/** Constructor with X-resources. */
	Button(const string &name, Component *parent, Arg *args, int n,
                ActionListener *listener=NULL, int position=0);
	/** Constructor with accelerator. */
	Button(const string &name, const string &accel, Component *parent,
		ActionListener *listener=NULL);
	/** Constructor with position. */
	Button(const string &name, Component *parent, int position,
		ActionListener *listener=NULL);
	/** Constructor with accelerator and position. */
	Button(const string &name, const string &accel, Component *parent,
		int position, ActionListener *listener=NULL);
	~Button(void);

	virtual Button *getButtonInstance(void) { return this; }

	ParseCmd parseCmd(const string &cmd, string &msg);

	void activate(void);
	/** Change the Button label. */
	void setLabel(const string &label);

    protected:

	virtual void init(Component *parent);
	virtual void init(Component *parent, const string &accel);
	virtual void init(Component *parent, Arg *args, int n, int position);
	virtual void init(Component *parent, int position);
	virtual void init(Component *parent, const string &accel, int position);

    private:
	static void activateCallback(Widget, XtPointer, XtPointer);
};

#endif
