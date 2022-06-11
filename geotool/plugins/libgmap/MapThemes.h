#ifndef _MAPTHEMES_H
#define _MAPTHEMES_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/gvector.h"
#include "widget/Map.h"
#include "widget/Table.h"

class PrintDialog;

namespace libgmap {

class MapOverlay;
class MapThemeFile;
class MapWindow;

class MapThemes : public FormDialog
{
    friend class MapThemeFile;

    public:
	MapThemes(const char *name, Component *parent, MapWindow *mapWindow);
	~MapThemes(void);

	MapWindow	*mw;
	PrintDialog	*print_window;

	void setProjection(const char *name);
	void setColorMap(int num, Pixel *pixels);
	Pixel getColor(const char *name);

	bool readShapeFile(const char *path);
	void readNetCDF(const char *path);
	void readGriddedData(const char *path);
	void readPolar(const char *path);
	void readInitialThemes(void);
	void addCursorCallbacks(Map *map);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	static int get_line(FILE *fp, char *line, int len, int *line_no);

    protected:

	gvector<MapThemeFile *>	*theme_files;
	Button		*close_button;
	Button		*shape_table_button;
	Button		*color_table_button;
	Button		*remove_button;
	Button		*apply_button;
	Button		*save_button;
	Button		*help_button;
	Separator	*sep;
	RowColumn	*controls, *updn, *rc;
	ArrowButton	*up_arrow, *down_arrow;
	Label		*label;
	Table		*table;


	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void selectRow(void);
	void changeThemeLevel(bool raise_level);
	void themesChoice(MmTableEditCallbackStruct *c);
	void mapColorBar(MapThemeFile *tf, int row, Boolean state);
	void themesEdit(MmTableEditCallbackStruct *c);
	void applyChange(void);
	bool isTrue(char *s);
	void addThemeRow(MapThemeFile *tf, bool new_theme);
	char *getThemeName(const char *path);
	void displayShapeTable(void);
	void displayColorTable(void);
	void saveThemes(void);
	void removeThemes(void);
	void readPolarFile(FILE *fp, MapPlotTheme *theme);

	static void cursorMotionCB(Widget w, XtPointer client, XtPointer data);
	static void shapeSelectCB(Widget w, XtPointer client, XtPointer data);

    private:
	int last_theme_id;
	int last_shape_index;
};

} // namespace libgmap

#endif
