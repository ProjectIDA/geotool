#ifndef _TAPER_WINDOW_H
#define _TAPER_WINDOW_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

/**
 *  @ingroup libgx
 */
class TaperWindow : public FormDialog
{
    public:
	TaperWindow(const string &name, Component *parent, int width_percent=5,
		int min_pts=5, int max_pts=0);
	~TaperWindow(void) {}
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);
	TextField *width, *min_points, *max_points;
    protected:
	RowColumn *rc, *controls;
	Label *width_label, *min_label, *max_label;
	Separator *sep;
	Button *close_button;
	void actionPerformed(ActionEvent *action_event);
};

#endif
