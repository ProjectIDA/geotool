#ifndef _RESP_TAPERS_H
#define _RESP_TAPERS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

namespace libgrsp {

class RespTapers : public FormDialog
{
    public:
	RespTapers(const char *, Component *, double taper_secs,
		double low, double high, double cutoff);
	~RespTapers(void);

	void getParams(double *taper_secs, double *flo, double *fhi,
			double *cutoff);

	TextField *data_taper_text, *low_pass_text, *high_pass_text;
	TextField *amp_cutoff_text;
	Label *warn;
	void apply(void);
	bool params_ok;

    protected:
	RowColumn *controls, *rc;
	Separator *sep;
	Button *close_button, *apply_button, *help_button;
	Label *label, *label1, *label2, *label3, *label4;
	double data_taper, low_pass, high_pass, amp_cutoff;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void setButtonsSensitive(void);

    private:

};

} // namespace libgrsp

#endif
