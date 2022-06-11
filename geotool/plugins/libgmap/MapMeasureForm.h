#ifndef _MAP_MEASURE_FORM_H
#define _MAP_MEASURE_FORM_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "widget/Map.h"
#include "widget/Table.h"

namespace libgmap {

class MapMeasureForm : public Frame
{
    public:
	MapMeasureForm(const char *, Component *, Map *);
	~MapMeasureForm(void);

	void setVisible(bool visible);

    protected:
	// File menu
	Button *close_button;

	// View menu
	Button *clear_selected, *clear_all, *attributes_button;

	// Help menu
	Button *help_button;

	Table *table;
	Map *map;
	int num_ids;
	int *ids;
	bool added_crosshair;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *a);
	void list(void);
	void getRow(MapMeasure *m, int num_columns, TAttribute *a,
			char **row);
	void measure(MapMeasure *m);
	void deleteSelected(void);


    private:

};

} // namespace libgmap

#endif
