/** \file ScrolledPane.cpp
 *  \brief Defines class ScrolledPane.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/ScrolledPane.h"
#include "motif++/ScrollBar.h"
extern "C" {
#include "X11/IntrinsicP.h" // for XtConfigureWidget
}

/** Constructor with X-resources.
 *  @param[in] name the name given to this ScrolledPane instance.
 *  @param[in] parent the Component parent.
 *  @param[in] num_children_visible the number of visible Component children.
 *  @param[in] child_border_width the border width for the Component children.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
ScrolledPane::ScrolledPane(const string &name, Component *parent,
			int num_children_visible, int child_border_width,
			Arg *args, int n)
			: Form(name, parent, args, n)
{
    num_visible = num_children_visible;
    if(num_visible < 1) num_visible = 1;
    border_width = child_border_width;
    init();
}

/** Destructor. */
ScrolledPane::~ScrolledPane(void)
{
}

/** Initialize.
 */
void ScrolledPane::init(void)
{
    int n;
    Arg args[20];
    int form_height = getHeight();
    XtPointer cb_data = (XtPointer)this;

    child_current_height = childHeight();
    current_form_height = form_height;

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_DOWN); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    more_arrow = new ArrowButton("more", this, args, n, this);
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, more_arrow->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNarrowDirection, XmARROW_UP); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    less_arrow = new ArrowButton("less", this, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, less_arrow->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    scrollbar = new ScrollBar("scrollbar", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, scrollbar->baseWidget()); n++;
    XtSetArg(args[n], XmNmarginWidth, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNresizeWidth, True); n++;
    XtSetArg(args[n], XmNresizeHeight, True); n++;
    form = new Form("form", this, args, n);

    XtAddEventHandler(scrollbar->baseWidget(), ExposureMask|VisibilityChangeMask
			|ResizeRedirectMask, False, resizeCB, cb_data);

    enableCallbackType(XmNresizeCallback);
}

/** The Resize callback from Xlib.
 */
void ScrolledPane::resizeCB(Widget widget, XtPointer client_data,
			XEvent *event, Boolean *cont)
{
    ScrolledPane *sp = (ScrolledPane *)client_data;
    sp->scroll(NULL);
}

void ScrolledPane::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == more_arrow) {
	if(num_visible < form->numChildren()) {
	    num_visible++;
	    scroll(NULL);
	}
    }
    else if(comp == less_arrow) {
	if(num_visible > 1) {
	    num_visible--;
	    scroll(NULL);
	}
    }
    else if(comp == scrollbar) {
	scroll((XmScrollBarCallbackStruct *)action_event->getCalldata());
    }
}

/** Update the display.
 *  @param[in] s A ScrollBar callback structure, or NULL.
 */
void ScrolledPane::scroll(XmScrollBarCallbackStruct *s)
{
    vector<Component *> *children = form->getChildren();
    int i, n, min, max, slider_size, value;
    Dimension total_height, form_width, form_height;
    Dimension scroll_width, child_width, child_height;
    Position top, child_top;
    bool do_callback=false;
    Arg args[10];

    n = 0;
    XtSetArg(args[n], XmNheight, &form_height); n++;
    XtSetArg(args[n], XmNwidth, &form_width); n++;
    XtGetValues(base_widget, args, n);

    if(form_height != current_form_height) {
	do_callback = true;
    }
    current_form_height = form_height;

    n = 0;
    XtSetArg(args[n], XmNminimum, &min); n++;
    XtSetArg(args[n], XmNmaximum, &max); n++;
    XtSetArg(args[n], XmNsliderSize, &slider_size); n++;
    XtSetArg(args[n], XmNvalue, &value); n++;
    XtSetArg(args[n], XmNwidth, &scroll_width); n++;
    scrollbar->getValues(args, n);

    max -= slider_size;

    child_height = childHeight();

    total_height = 0;
    for(i = 0; i < (int)children->size(); i++) {
	if( children->at(i)->isVisible() ) {
	    total_height += child_height + 2*border_width;
	}
    }

    if(total_height <= form_height) {
	top = 0;
    }
    else if(max > min) {
	top = -(int)(((double)value/(max-min))*(total_height-form_height));
    }
    else return;

    if(top && s && s->reason == XmCR_PAGE_INCREMENT) {
	// position the next plot at the top of the form.
	n = abs(top)/(child_height+2*border_width);
	if(n*(child_height+2*border_width) != abs(top)) n++;
	top = -n*(child_height+2*border_width);
	if(total_height != form_height) {
	    value = -top*(max-min)/(total_height-form_height);
	    if(value > max) {
		value = max;
		top= -(int)(((double)max/(max-min))*(total_height-form_height));
	    }
	    XtSetArg(args[0], XmNvalue, value);
	    scrollbar->setValues(args, 1);
	}
    }
    else if(top && s && s->reason == XmCR_PAGE_DECREMENT) {
	// position the previous plot at the top of the form.
	n = abs(top)/(child_height+2*border_width);
	top = -n*(child_height+2*border_width);
	if(total_height != form_height) {
	    value = -top*(max-min)/(total_height-form_height);
	    XtSetArg(args[0], XmNvalue, value);
	    scrollbar->setValues(args, 1);
	}
    }

    child_width = form_width - scroll_width - 2*border_width;

    child_top = top;
    for(i = 0; i < (int)children->size(); i++) {
	if( children->at(i)->isVisible() ) {
	    XtConfigureWidget(children->at(i)->baseWidget(), 0, child_top,
				child_width, child_height, border_width);
	    child_top += child_height + 2*border_width;
	}
    }
    delete children;

    if(child_current_height != child_height) do_callback = true;
    child_current_height = child_height;

    if(do_callback) doCallbacks(this, NULL, XmNresizeCallback);
}

/** Get the child Component height. All children have the same height. The
 *  sizebar controls this height, which can vary between child_min_height
 *  and the height of the ScrolledPane.
 *  @returns the current child Component height.
 */
Dimension ScrolledPane::childHeight(void)
{
    int height = getHeight()/num_visible;

    return height - 2*border_width;
/*
    vector<Component *> *children = form->getChildren();
    int n, min, max, slider_size, value, min_child_height, max_child_height;
    Dimension child_height;
    int form_height = getHeight();
    Arg args[4];

    n = 0;
    XtSetArg(args[n], XmNminimum, &min); n++;
    XtSetArg(args[n], XmNmaximum, &max); n++;
    XtSetArg(args[n], XmNsliderSize, &slider_size); n++;
    XtSetArg(args[n], XmNvalue, &value); n++;
    XtGetValues(sizebar, args, n);

    max -= slider_size;

    int num = (int)children->size();
    for(int i = 0; i < (int)children->size(); i++) {
	if( !children->at(i)->isVisible() ) num--;
    }
    delete children;

    min_child_height = (num > 0) ? form_height/num - 2*border_width : 0;
    if(min_child_height < child_min_height) {
	min_child_height = child_min_height;
    }
    max_child_height = form_height - 2*border_width;

    if(max <= min) {
	child_height = child_min_height;
    }
    else {
	child_height = min_child_height + (int)( ((double)value/(max-min))
		*(max_child_height - min_child_height) );
    }
    if(child_height < min_child_height) child_height = min_child_height;
    return child_height;
*/
}
