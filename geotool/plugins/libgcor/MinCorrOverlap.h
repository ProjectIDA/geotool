#ifndef _MIN_CORR_OVERLAP_H
#define _MIN_CORR_OVERLAP_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

namespace libgcor {

class MinCorrOverlap : public FormDialog
{
    public:
	MinCorrOverlap(const char *name, Component *parent);
	~MinCorrOverlap(void);

	void setVisible(bool visible);
	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);
	double		min_overlap;

    protected:

	RowColumn	*rc, *controls;
	Button		*close_button, *apply_button, *help_button;
	Separator	*sep;
	Label		*label;
	TextField	*text;

	void createInterface(void);
	void apply(void);

    private:
};

} // namespace libgcor

#endif
