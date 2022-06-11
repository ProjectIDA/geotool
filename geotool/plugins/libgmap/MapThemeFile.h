#ifndef _MAP_THEME_FILE_H_
#define _MAP_THEME_FILE_H_

#include "gobject++/Gobject.h"
#include "widget/Map.h"
#include "widget/Table.h"
#include "MapThemeColors.h"

class Table;

namespace libgmap {

class MapWindow;
class MapThemes;
class ShapeTable;

#define THEME_IMAGE_FILE        1
#define THEME_SHAPE_FILE        2
#define THEME_POLAR_FILE        3

class MapThemeFile : public Gobject
{
    friend class MapThemes;

    public:
	MapThemeFile(MapWindow *mapWindow, int themeType, const char *themeName,
		const char *filePath, int nShapeType, int nEntities,bool first);
	~MapThemeFile(void);

	void createShapeTable(void);
	void colorColumn(char *colorColumn);
	void initThemeColorScale(MapPlotTheme *t, double min, double max);
	void showColorTable(Component *parent);
	void setColorScale(void);
	int getId(void) { return id; }
	void setId(int win_id) {
	    this->id = win_id;
	    if(theme_colors_window) theme_colors_window->id = win_id;
	}

//	void MapColorBar(int row, Boolean state);

	MapWindow	*mw;

    protected:
	int		id;
	int		theme_type;
	int		shape_type;
	int		nshapes;
	bool		display;
	bool		cursor_info;
	bool		bndry;
	bool		fill;
	bool		color_bar;
	char		*theme_name;
	char		*path;
	char		*bndry_color;
	char		*fill_color;
	Pixel		bndry_fg;
	Pixel		fill_fg;
	bool		bndry_color_changed;
	bool		fill_color_changed;
	bool		on_delta_changed;
	bool		off_delta_changed;
	ShapeTable	*info_window;
	Table		*shape_table;
	MapThemeColors	*theme_colors_window;
	int		label_column;
	int		cursor_column;
	int		color_column;
	int		sym_type;
	int		sym_size;
	double		on_delta;
	double		off_delta;
	ColorScale	color_scale;

	int getColorBounds(double min, double max);
	void createColorTable(void);

    private:
	static void sortStrings(int lo0, int hi0, vector<const char *> &s,
				int *sort_order);

};

} // namespace libgmap

#endif
