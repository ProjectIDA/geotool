#ifndef _RESIDUALS_H
#define _RESIDUALS_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"

class Table;

namespace libglc {

class Residuals : public Frame
{
    public:
	Residuals(const char *, Component *);
	~Residuals(void);

    protected:
	Button *close_button;
	RowColumn *rc;
	Button *more_button, *less_button;
	Table *table;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *a);
	void moreResiduals(void);
	void lessResiduals(void);

    private:

};

} // namespace libglc

#endif
