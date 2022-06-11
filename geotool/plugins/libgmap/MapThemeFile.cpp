/** \file MapThemeFile.cpp
 *  \brief Defines class MapThemeFile.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

#include "MapThemeFile.h"
#include "MapWindow.h"
#include "MapThemeColors.h"
#include "ShapeTable.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

using namespace libgmap;

MapThemeFile::MapThemeFile(MapWindow *mapWindow, int themeType,
		const char *themeName, const char *filePath, int nShapeType,
		int nEntities, bool first) : Gobject()
{
    char name[1000];
    string prop;

    mw = mapWindow;
    id = 0;
    theme_type = themeType;
    shape_type = nShapeType;
    nshapes = nEntities;

    snprintf(name, sizeof(name), "theme.%s.display", themeName);
    display = Application::getProperty(name, true);

    snprintf(name, sizeof(name), "theme.%s.color_bar", themeName);
    color_bar = Application::getProperty(name, false);

    theme_name = strdup(themeName);
    if(nEntities > 30000) {
	on_delta = 20.0;
    }
    else {
	on_delta = 360.0;
    }
    snprintf(name, sizeof(name), "theme.%s.on_delta", theme_name);
    on_delta = Application::getProperty(name, on_delta);

    off_delta = 0.0;
    snprintf(name, sizeof(name), "theme.%s.off_delta", theme_name);
    off_delta = Application::getProperty(name, 0.0);

    snprintf(name, sizeof(name), "theme.%s.bndry", theme_name);
    bndry = Application::getProperty(name, true);

    fill = first ? true : false;
    snprintf(name, sizeof(name), "theme.%s.fill", theme_name);
    fill = Application::getProperty(name, fill);

    bndry_color = strdup("grey45");
    bndry_fg = mw->mainWindow()->stringToPixel("grey45");
    snprintf(name, sizeof(name), "theme.%s.bndry_color", theme_name);
    if(Application::getProperty(name, prop)) {
	bndry_fg = mw->mainWindow()->stringToPixel(prop.c_str());
    }
    if(nShapeType == SHPT_POLYGON) {
	fill_color = strdup("tan");
    }
    else {
	fill_color = strdup("grey45");
    }
    fill_fg = mw->mainWindow()->stringToPixel("tan");
    snprintf(name, sizeof(name), "theme.%s.fill_color", theme_name);
    if(Application::getProperty(name, prop)) {
	fill_fg = mw->mainWindow()->stringToPixel(prop.c_str());
    }

    snprintf(name, sizeof(name), "theme.%s.sym_type", theme_name);
    sym_type = Application::getProperty(name, CIRCLE);

    snprintf(name, sizeof(name), "theme.%s.sym_size", theme_name);
    sym_size = Application::getProperty(name, 2);

    path = strdup(filePath);
    info_window = NULL;
    shape_table = NULL;
    snprintf(name, sizeof(name), "theme.%s.labels", theme_name);
    label_column = Application::getProperty(name, -1);

    snprintf(name, sizeof(name), "theme.%s.cursor_column", theme_name);
    cursor_column = Application::getProperty(name, -1);

    if(cursor_column >= 0) {
	cursor_info = true;
    }
    else {
	cursor_info = first ? true : false;
    }
    snprintf(name, sizeof(name), "theme.%s.display", theme_name);
    cursor_info = Application::getProperty(name, cursor_info);

    snprintf(name, sizeof(name), "theme.%s.color_column", theme_name);
    color_column = Application::getProperty(name, -1);

    bndry_color_changed = false;
    fill_color_changed = false;
    on_delta_changed = false;
    off_delta_changed = false;
    theme_colors_window = NULL;
}

MapThemeFile::~MapThemeFile()
{
    free(theme_name);
    free(path);
    free(bndry_color);
    free(fill_color);
    Free(color_scale.pixels);
    Free(color_scale.lines);
}

void MapThemeFile::initThemeColorScale(MapPlotTheme *t, double min, double max)
{
    int i, n, num_colors = 14;
    int r[] = {111,119, 71, 38, 44, 44, 32,210,231,237,255,209,138, 41};
    int g[] = { 54,133,150,183,210,208,240,233,201,158,  0,  0,  0,  5};
    int b[] = {255,208,237,237,255, 26,  0, 11, 18,  0,  0,  0,  0,  0};
    Widget w = mw->mainWindow()->baseWidget();

    if(!(t->color_scale.pixels =
		(Pixel *)mallocWarn(num_colors*sizeof(Pixel)))) return;
    if(!(color_scale.pixels =
		(Pixel *)mallocWarn(num_colors*sizeof(Pixel)))) return;
    n = num_colors + 1;
    if(!(t->color_scale.lines = (double *)mallocWarn(n*sizeof(double)))) return;
    if(!(color_scale.lines = (double *)mallocWarn(n*sizeof(double)))) return;
    color_scale.num_labels = 0;
    color_scale.labels = NULL;
    color_scale.label_values = NULL;

    n = 0;
    for(i = 0; i < num_colors; i++)
    {
	XColor c;
	c.red   = r[i]*256;
	c.green = g[i]*256;
	c.blue  = b[i]*256;
	c.flags = DoRed | DoGreen | DoBlue;

	if(XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
		DefaultScreen(XtDisplay(w))), &c))
	{
	    color_scale.pixels[n] = c.pixel;
	    t->color_scale.pixels[n++] = c.pixel;
	}
    }
    t->color_scale.num_colors = n;
    color_scale.num_colors = n;
    getColorBounds(min, max);
    for(i = 0; i < num_colors+1; i++) {
	t->color_scale.lines[i] = color_scale.lines[i];
    }
}

void MapThemeFile::createShapeTable(void)
{
    if(info_window) return;

    DBFHandle dbf;
    int ncols;

    if((dbf = DBFOpen(path, "rb")) == NULL) {
	mw->showWarning("No theme table found.");
	return;
    }
    ncols = DBFGetFieldCount(dbf);
    if(ncols <= 0) {
	mw->showWarning("No information available.");
	DBFClose(dbf);
	return;
    }
    char title[MAXPATHLEN+20];
    snprintf(title, sizeof(title), "Theme: %s", path);
    info_window = new ShapeTable(title, mw, this, dbf, ncols);
    shape_table = info_window->shapeTable();
}

void MapThemeFile::showColorTable(Component *parent)
{
    if(!theme_colors_window) {
	char title[MAXPATHLEN+20];
	snprintf(title, sizeof(title), "Theme: %s", theme_name);
	theme_colors_window = new MapThemeColors(title, mw, &color_scale,
					mw->map, id);
    }
    theme_colors_window->setVisible(true);
}

void MapThemeFile::setColorScale(void)
{
    if(!theme_colors_window) {
	char title[MAXPATHLEN+20];
	snprintf(title, sizeof(title), "Theme: %s", theme_name);
	theme_colors_window = new MapThemeColors(title, mw, &color_scale,
					mw->map, id);
    }
    theme_colors_window->setColorScale();
}

int MapThemeFile::getColorBounds(double min, double max)
{
    int nlab, ndigit, ndeci, num;

    if(color_scale.num_colors <= 0) return 0;

    num = color_scale.num_colors + 1;

    if(min == max) {
	if(min != 0.) {
	    min *= .9;
	    max *= 1.1;
	}
	else {
	    min = -1;
	    max = 1;
	}
    }

    nicex(min, max, num, num, &nlab, color_scale.lines, &ndigit, &ndeci);
    color_scale.lines[0] = min;
    color_scale.lines[num-1] = max;

    if(!theme_colors_window) {
	char title[MAXPATHLEN+20];
	snprintf(title, sizeof(title), "Theme: %s", theme_name);
	theme_colors_window = new MapThemeColors(title, mw, &color_scale,
					mw->map, id);
    }
    else {
	theme_colors_window->loadColorTable();
    }

    return num;
}

void MapThemeFile::colorColumn(char *colorcolumn)
{
    if(!strcmp(colorcolumn, "off")) {
	color_column = -1;
	mw->map->themeFillColor(id, fill_color, true);
    }
    else if(!strcmp(colorcolumn, "sort-order"))
    {
	int i, nrows;
 	vector<int> row_order;
	double *values=NULL;

	nrows = shape_table->getRowOrder(row_order);
	if(!(values = (double *)mallocWarn(nrows*sizeof(double)))) return;
	for(i = 0; i < nrows; i++) values[i] = row_order[i] + 1;

	getColorBounds(1., (double)nrows);
	mw->map->shapeColor(id, nrows, values, color_scale.num_colors+1,
			color_scale.lines);
	Free(values);
    }
    else
    {
	vector<const char *> lab;
	int i, num, time_code, nrows, *sort_order=NULL;
	double *values=NULL, min=0., max=0.;

	num = shape_table->getColumnLabels(lab);
	for(i = 0; i < num && strcmp(colorcolumn, lab[i]); i++);
	if(i == num) return;

	color_column = i;
	nrows = shape_table->getColumn(i, lab);

	if(!(values = (double *)mallocWarn(nrows*sizeof(double)))) return;

	if((time_code = shape_table->getColumnTime(i)) >= 0)
	{
	    for(i = 0; i < nrows; i++) {
		values[i] = timeStringToEpoch(lab[i],
				(enum TimeFormat)time_code);
	    }
	}
	else
	{
	    for(i = 0; i < nrows && stringToDouble(lab[i],&values[i]); i++);
	    if(i < nrows)
	    {
		if(!(sort_order = (int *)mallocWarn(nrows*sizeof(int)))) return;

		for(i = 0; i < nrows; i++) sort_order[i] = i;

		sortStrings(0, nrows-1, lab, sort_order);

		if(nrows > 0) values[sort_order[0]] = 1.;
		for(i = 1; i < nrows; i++) {
		    if(!strcmp(lab[i], lab[i-1])) {
			values[sort_order[i]] = values[sort_order[i-1]];
		    }
		    else {
			values[sort_order[i]] = values[sort_order[i-1]] +1.;
		    }
		}
		Free(sort_order);
	    }
	}

	if(nrows > 0) {
	    min = max = values[0];
	}
	for(i = 1; i < nrows; i++) {
	    if(min > values[i]) min = values[i];
	    if(max < values[i]) max = values[i];
	}
	
	getColorBounds(min, max);
	mw->map->shapeColor(id, nrows, values, color_scale.num_colors+1,
			color_scale.lines);
	Free(values);
    }
}

void MapThemeFile::sortStrings(int lo0, int hi0, vector<const char *> &s,
			int *sort_order)
{
    int lo = lo0;
    int hi = hi0;
    const char *mid = s[(lo0+hi0)/2];

    if(lo0 >= hi0) return;

    while(lo <= hi) {
        while(lo < hi0 && strcmp(s[lo], mid) < 0) lo++;
        while(hi > lo0 && strcmp(s[hi], mid) > 0) hi--;

        if(lo <= hi){
            if(lo != hi) {
                const char *g = s[lo];
		int i = sort_order[lo];
                s[lo] = s[hi];
                s[hi] = g;
                sort_order[lo] = sort_order[hi];
                sort_order[hi] = i;
            }
            lo++;
            hi--;
        }
    }
    if(lo0 < hi) sortStrings(lo0, hi, s, sort_order);
    if(lo < hi0) sortStrings(lo, hi0, s, sort_order);
}
