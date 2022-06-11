#ifndef _SEARCH_PARAM_H
#define _SEARCH_PARAM_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgarrival {

class SearchParam : public ParamDialog
{
    public:
	SearchParam(const char *name, Component *parent);
	~SearchParam(void);

    protected:

	RowColumn *controls;
	Separator *sep;
	Button	*close_button, *search_button;
	RowColumn *rc;
	Label *min_label, *max_label;
	TextField *min_text, *max_text;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:
};

} // namespace libgarrival

#endif
