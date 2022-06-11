#ifndef _AXES_LABELS_H
#define _AXES_LABELS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

class AxesClass;

/** A FormDialog subclass for setting plot labels. This interface allows the
 *  x-axis label, the y-axis label, and the plot title to be set for an AxesClass
 *  object.
 *  \image html AxesLabels.gif "The Axes Labels Window"
 *  \image latex AxesLabels.eps "The Axes Labels Window" width=3in
 *  @ingroup libgx
 */
class AxesLabels : public FormDialog
{
    public:
	AxesLabels(const string &name, Component *parent, AxesClass *axes);
	~AxesLabels(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:

	RowColumn	*rc, *controls;
	Button		*close_button, *apply_button, *help_button;
	Separator	*sep;
	AxesClass	*axes;
	Label		*title_label, *x_axis_label, *y_axis_label;
	TextField	*title_text, *x_axis_text, *y_axis_text;

	void createInterface(void);
	void setTextFields(void);
	void apply(void);

    private:
	string last_title_string, last_x_axis_string, last_y_axis_string;
};

#endif
