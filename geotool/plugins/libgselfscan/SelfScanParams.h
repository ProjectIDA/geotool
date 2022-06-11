#ifndef _SELFSCAN_PARAMS_H
#define _SELFSCAN_PARAMS_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgselfscan {

#define SELFSCAN_PARAM_NULL \
{ \
	10.0,	/* windowlength */ \
	4,	/* nclusters */ \
        0.70,   /* min_corr_th */ \
}

class SelfScanParams : public ParamDialog
{
    public:
	SelfScanParams(const char *, Component *, ActionListener *);
	~SelfScanParams(void);

    protected:
	RowColumn *controls, *rc;
	Button *close_button, *help_button;
	Separator *sep;
	Label *label[1];
	TextField *text[1];

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // namespace libgselfscan

#endif
