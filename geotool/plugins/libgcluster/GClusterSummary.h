#ifndef _GCLUSTER_SUMMARY_H
#define _GCLUSTER_SUMMARY_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"


class Table;

namespace libgcluster {

class GClusterSummary : public ParamDialog
{
    public:
	GClusterSummary(const char *, Component *, ActionListener *);
	~GClusterSummary(void);
	void setSummary(int nclusters, int *matched, int *unmatched);

    protected:
	RowColumn *controls;
        Table *table;
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
