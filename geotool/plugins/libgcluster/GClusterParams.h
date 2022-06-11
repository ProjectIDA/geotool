#ifndef _GCLUSTER_PARAMS_H
#define _GCLUSTER_PARAMS_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgcluster {

#define GCLUSTER_PARAM_NULL \
{ \
	4,	/* nclusters */ \
        0.75,   /* min_xcorr_th */ \
}

class GClusterParams : public ParamDialog
{
    public:
	GClusterParams(const char *, Component *, ActionListener *);
	~GClusterParams(void);

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

} // namespace libgcluster

#endif
