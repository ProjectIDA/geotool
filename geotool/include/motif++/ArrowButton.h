#ifndef ARROWBUTTON_H
#define ARROWBUTTON_H

#include "motif++/Component.h"

/** A class for the XmArrowButton widget.
 *  @ingroup libmotif
 */
class ArrowButton : public Component
{
    public:

	/** Constructor. */
	ArrowButton(const string &name, Component *parent, int arrow_direction,
			ActionListener *listener=NULL);
	/** Constructor with X-resources and listener. */
	ArrowButton(const string &name, Component *parent, Arg *args, int num,
			ActionListener *listener=NULL);
	/** Constructor with default arrow direction. */
	ArrowButton(const string &name, Component *parent);

	/** Destructor. */
	~ArrowButton(void);

    protected:
	/** the arrow direction: XmARROW_UP, XmARROW_DOWN, XmARROW_LEFT,
	 * or XmARROW_RIGHT
	 */
	int direction;

	virtual void init(Component *);

    private:
	static void activateCallback(Widget, XtPointer, XtPointer);
};

#endif
