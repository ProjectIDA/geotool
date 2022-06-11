#ifndef _MAP_THEME_COLORS_H
#define _MAP_THEME_COLORS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Map.h"
#include "widget/Table.h"

namespace libgmap {

class MapThemeColors : public FormDialog
{
    public:
	MapThemeColors(const char *name, Component *parent, ColorScale *c,
			Map *map, int id);
	~MapThemeColors(void);

	void loadColorTable(void);
	bool setColorScale(void);

	int		id;

    protected:
	ColorScale	*color_scale;
	Map		*map;
	Table		*table;

	RowColumn	*rc;
	Button		*close_button, *apply_button, *choose_color_button;
	Separator	*sep;

	Form		*form;
	Button		*color_plus_button, *color_minus_button;

	void actionPerformed(ActionEvent *a);
	void editColor(MmTableEditCallbackStruct *e);

    private:
};

} // namespace libgmap

#endif
