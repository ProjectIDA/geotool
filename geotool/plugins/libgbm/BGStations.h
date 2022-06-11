#ifndef _B_G_STATIONS_H
#define _B_G_STATIONS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "Beam.h"

class Table;

namespace libgbm {

class BGStations : public FormDialog
{
    public:
	BGStations(const char *, Component *);
	~BGStations(void);

	void list(BeamGroup *bg);

    protected:
	RowColumn *controls;
	Button *close_button;
	Separator *sep;
	Table *table;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // libgbm

#endif
