#ifndef _CAL_PARAM_H
#define _CAL_PARAM_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgcal {

class CalParam : public ParamDialog
{
    public:
	CalParam(const char *, Component *, ActionListener *);
	~CalParam(void);

    protected:
	RowColumn *controls, *rc, *rc2;
	Button *close_button, *compute_button, *iteration_button, *help_button;
	Separator *sep;
	Form *form;
	Label *lab1, *lab2;
	Label *label[10];
	TextField *text[10];
	Label *input_label, *output_label;
	TextField *input_text, *output_text, *partitle_text;
	ScrolledWindow *sw;
	TextField *param_text;
	

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // libgcal

#endif
