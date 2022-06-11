#ifndef _POLAR_PARAM_H
#define _POLAR_PARAM_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifClasses.h"

class Table;

namespace libgpolar {

class PolarParam : public ParamDialog
{
    public:
	PolarParam(const char *, Component *, ActionListener *);
	~PolarParam(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	bool getFilter(char *type, int *order, double *flo, double *fhi,
			int *zero_phase);
	bool filterOn() { return filter_on->state(); }

	Toggle *auto_param_toggle, *filter_on, *filter_off;
	TextField *window_length_text;

    protected:
	RowColumn *controls, *rc, *filter_rc;
	RadioBox *rb;
	Button *close_button, *compute_button, *help_button;
	Separator *sep;
	Label *window_length_label, *filter_label;
	Table *filter_table;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // namespace libgpolar

#endif
