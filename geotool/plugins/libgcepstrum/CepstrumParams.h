#ifndef _CEPSTRUM_PARAMS_H
#define _CEPSTRUM_PARAMS_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgcepstrum {

class CepstrumParams : public ParamDialog
{
    public:
	CepstrumParams(const char *, Component *, ActionListener *);
	~CepstrumParams(void);

    protected:
	RowColumn *controls, *rc;
	Button *close_button, *compute_button, *help_button;
	Separator *sep;
	Label *label[12];
	TextField *text[12];

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // namespace libgcepstrum

#endif
