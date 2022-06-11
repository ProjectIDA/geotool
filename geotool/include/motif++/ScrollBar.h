#ifndef SCALE_BAR_H
#define SCALE_BAR_H

#include "motif++/Component.h"

/** A class for the XmScrollBar widget.
 *  @ingroup libmotif
 */
class ScrollBar : public Component
{
    public:

	ScrollBar(const string &name, Component *parent,
		ActionListener *listener=NULL, Arg *args=NULL, int n=0);
	~ScrollBar(void);

	int getMaximum(void);
	int getMinimum(void);
	int getValue(void);
	void setValue(int value);

    protected:

	void init(Component *parent, Arg *args, int n);

    private:

	static void valueChangedCallback(Widget, XtPointer, XtPointer);
	static void dragCallback(Widget, XtPointer, XtPointer);
	static void incrementCallback(Widget, XtPointer, XtPointer);
	static void decrementCallback(Widget, XtPointer, XtPointer);
	static void pageIncrementCallback(Widget, XtPointer, XtPointer);
	static void pageDecrementCallback(Widget, XtPointer, XtPointer);
};

#endif
