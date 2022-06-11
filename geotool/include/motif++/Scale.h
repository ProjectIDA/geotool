#ifndef SCALE_H
#define SCALE_H

#include "motif++/Component.h"

/** A class for the XmScale widget class.
 *  @ingroup libmotif
 */
class Scale : public Component
{
    public:

	Scale(const string &name, Component *parent,
		ActionListener *listener=NULL, Arg *args=NULL, int n=0);
	Scale(const string &name, Component *parent, Arg *args, int n);
	~Scale(void);

	/** Set the value.
	 *  @param[in] value
	 */
	void setValue(int value) { XmScaleSetValue(base_widget, value); }
	/** Get the value.
	 *  @returns the value.
	 */
	int getValue(void) { int value; XmScaleGetValue(base_widget, &value);
		return value; }
	int getMaximum(void);
	int getMinimum(void);
	void setMaximum(int maximum);
	void setMinimum(int minimum);

    protected:

	void init(Component *parent, Arg *args, int n);

    private:

	static void valueChangedCallback(Widget, XtPointer, XtPointer);
	static void dragCallback(Widget, XtPointer, XtPointer);
};

#endif
