#ifndef INFO_AREA_H
#define INFO_AREA_H

#include "motif++/MotifComp.h"
#include "motif++/Component.h"
#include "motif++/Button.h"

extern "C" {
#include "motif++/Info.h"
}

/** A class for the Info widget. The Info widget is used for displaying text
 *  and for drawing.
 *  @ingroup libmotif
 */
class InfoClass : public Component
{
    public:
	/** Constructor.
	 *  @param[in] name name given to this InfoClass instance.
	 *  @param[in] parent the Component parent.
	 *  @param[in] args X-resource structures.
	 *  @param[in] num the number of X-resource structures.
	 */
	InfoClass(const string &name, Component *parent, Arg *args, int num)
                : Component(name, parent)
        {
	    base_widget = XtCreateManagedWidget(name.c_str(), infoWidgetClass,
				 parent->baseWidget(), args, num);
	}
	/** Display the input text.
	 *  @param[in] s the text to display.
	 */
	void setText(const string &s) {
		InfoSetText(base_widget, (char *)s.c_str()); }
	char *getText(void) { return InfoGetText(base_widget); }

};

enum InfoType {
    INFO_NORMAL,
    INFO_LEFT_ONLY
};

enum InfoAreaSide {
    INFO_AREA_LEFT,
    INFO_AREA_RIGHT
};

class Frame;

/** A class that contains two InfoClass components arranged horizontally. The
 *  InfoArea class is optionally displayed at the bottom of the Frame. It is
 *  typically used to display short messages and cursor related information.
 */
class InfoArea : public Component
{
    public:

	InfoArea(const string &name, Component *parent, InfoType=INFO_NORMAL);
	InfoArea(const string &name, Frame *frame, InfoType=INFO_NORMAL);

	/** Get the base widget of the left InfoClass child. */
	Widget leftInfo(void) { return left_info->baseWidget(); }
	/** Get the base widget of the right InfoClass child. */
	Widget rightInfo(void) {
	    return (right_info) ? right_info->baseWidget() : NULL; }
	/** Get the Form parent of the left InfoClass child. */
	Form *leftArea(void) { return left_area; }
	/** Get the Form parent of the right InfoClass child. */
	Form *rightArea(void) { return right_area; }

	/** Set the text in one of the InfoClass children.
	 *  @param[in] side INFO_AREA_LEFT or INFO_AREA_RIGHT
	 *  @param[in] s the text string.
 	 */
	void setText(InfoAreaSide side, const string &s) {
	    if(side == INFO_AREA_LEFT) left_info->setText(s.c_str());
	    else if(right_info) right_info->setText(s.c_str());
	}

	/** Get the text from one of the InfoClass children.
	 *  @param[in] side INFO_AREA_LEFT or INFO_AREA_RIGHT
	 *  @returns the text string. (Free the pointer when no longer needed.)
 	 */
	char * getText(InfoAreaSide side) {
	    if(side == INFO_AREA_LEFT) return left_info->getText();
	    else if(right_info) right_info->getText();
	    return NULL;
	}

	virtual InfoArea *getInfoAreaInstance(void) { return this; }

	void warningButtonOn(void);
	void actionPerformed(ActionEvent *action_event);

    protected:
	//! The type of this InfoArea INFO_NORMAL or INFO_LEFT_ONLY
	InfoType info_type;
	Form *left_area; //!< The Form parent of the left InfoClass
	Form *right_area; //!< The Form parent of the right InfoClass
	InfoClass *left_info; //!< The left InfoClass child.
	InfoClass *right_info; //!< The right InfoClass child.
	Button *warning; //!< The warning button.

	void init(Component *, InfoType);
};

#endif
