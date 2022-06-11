#ifndef _ITERATION_H
#define _ITERATION_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

class Table;

namespace libgcal {

class Iteration : public FormDialog
{
    public:
	Iteration(const char *, Component *);
	~Iteration(void);

	Table *table;

    protected:
	RowColumn *controls;
	Button *close_button;
	Separator *sep;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // libgcal

#endif
