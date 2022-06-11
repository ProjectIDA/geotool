#ifndef SCALED_PANE_H
#define SCALED_PANE_H

#include "motif++/MotifComp.h"
#include "motif++/ScrollBar.h"
#include "motif++/ArrowButton.h"

/** A container class for scrolling components. All Component children are
 *  displayed vertically with the same height. The chid Component height
 *  is adjustable between the minimum child height to the height of the
 *  ScrolledPane Component.
 *  @ingroup libmotif
 */
class ScrolledPane : public Form
{
    public:

	ScrolledPane(const string &name, Component *parent,
		int num_children_visible=1, int child_border_width=0,
		Arg *args=NULL, int n=0);
	~ScrolledPane(void);

	/** Get the Form Component.
	 *  @returns the Form Component that holds user added Components.
	 */
	Form *getForm(void) { return form; }
	/** Get the minimum child Component height.
	 *  @returns the minimum allowed height for Component children.
	 */
	int getChildHeight(void) { return child_current_height; }
	void actionPerformed(ActionEvent *);

    protected:
	ScrollBar *scrollbar;
	ArrowButton *more_arrow, *less_arrow;
	Form *form;
	int num_visible;
	int child_current_height;
	int current_form_height;
	int border_width;

	void init(void);
	void scroll(XmScrollBarCallbackStruct *s);
	Dimension childHeight(void);

    private:

	static void scrollCallback(Widget, XtPointer, XtPointer);
	static void resizeCB(Widget widget, XtPointer client_data,
				XEvent *event, Boolean *cont);


};

#endif
