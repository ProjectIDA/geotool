/** \file MapThemeColors.cpp
 *  \brief Defines class MapThemeColors.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "MapThemeColors.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

using namespace libgmap;

MapThemeColors::MapThemeColors(const char *name, Component *parent,
	ColorScale *cs, Map *map_src, int theme_id) : FormDialog(name, parent)
{
    Arg args[20];
    int n, ncols, alignment[5];
    const char *labels[5];
    bool col_editable[5];

    color_scale = cs;
    map = map_src;
    id = theme_id;

    ncols = 5;
    labels[0] = "red";
    labels[1] = "green";
    labels[2] = "blue";
    labels[3] = "color";
    labels[4] = "lower bound";
    alignment[0] = RIGHT_JUSTIFY;
    alignment[1] = RIGHT_JUSTIFY;
    alignment[2] = RIGHT_JUSTIFY;
    alignment[3] = LEFT_JUSTIFY;
    alignment[4] = RIGHT_JUSTIFY;
    col_editable[0] = true;
    col_editable[1] = true;
    col_editable[2] = true;
    col_editable[3] = false;
    col_editable[4] = true;

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    close_button = new Button("Close", rc, this);
    apply_button = new Button("Apply", rc, this);
    apply_button->setSensitive(false);
    choose_color_button = new Button("Choose Color...", rc, this);
    choose_color_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rc->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    color_plus_button = new Button("+", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, color_plus_button->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    color_minus_button = new Button("-", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNtableTitle, getName()); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNcolumns, ncols); n++;
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    XtSetArg(args[n], XtNwidth, 310); n++;
    XtSetArg(args[n], XtNheight, 390); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);

    table->setAlignment(ncols, alignment);
    table->setColumnEditable(col_editable);

    loadColorTable();
}

MapThemeColors::~MapThemeColors(void)
{
}

void MapThemeColors::loadColorTable(void)
{
    ColorScale *c = color_scale;
    Widget w = base_widget;
    int i, ndeci=0;
    const char *row[5];

    table->removeAllRows();

    if(c->num_colors > 0) {
	char lab[20], last_lab[20];
	int range;
	double dif;

	dif = c->lines[c->num_colors] - c->lines[0];
	if(dif == 0.) {
	    for(i = 0; i <= c->num_colors; i++) {
		c->lines[i] = i;
	    }
	}
	range = (int)log10(fabs(dif));
	ndeci = (range-2 > 0) ? 0 : -range+2;
	last_lab[0] = '\0';
	for(;;)
	{
	    for(i = 0; i <= c->num_colors; i++)
	    {
		ftoa(c->lines[i], ndeci, 1, lab, 20);
		if(!strcmp(lab, last_lab)) {
		    ndeci++;
		    last_lab[0] = '\0';
		    break;
		}
		stringcpy(last_lab, lab, sizeof(last_lab));
	    }
	    if(i > c->num_colors || ndeci >= 10) break;
	}
    }

    for(i = c->num_colors-1; i >= 0; i--)
    {
	char red[20], green[20], blue[20], bounds[20];
	XColor color;

	color.pixel = c->pixels[i];
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	XQueryColor(XtDisplay(w), XDefaultColormapOfScreen(XtScreen(w)),&color);

	snprintf(red, sizeof(red), "%d", color.red/256);
	snprintf(green, sizeof(green), "%d", color.green/256);
	snprintf(blue, sizeof(blue), "%d", color.blue/256);

	ftoa(c->lines[i], ndeci, 1, bounds, 20);

	row[0] = red;
	row[1] = green;
	row[2] = blue;
	row[3] = "";
	row[4] = bounds;

	table->addRow(row, False);
    }
    for(i = 0; i < c->num_colors; i++) {
	table->fillCell(c->num_colors-1-i, 3, c->pixels[i], False);
    }
    table->adjustColumns();
}

void MapThemeColors::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
//    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
	loadColorTable();
    }
    else if(!strcmp(reason, XtNvalueChangedCallback))
    {
	editColor((MmTableEditCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Apply")) 
    {
	if(setColorScale()) {
	    apply_button->setSensitive(false);
	}
    }
    else if(!strcmp(cmd, "+"))
    {
	vector<const char *> row;
	int nrows = table->numRows();
	Pixel pixel;
	    
	table->getRow(nrows-1, row);
	table->addRow(row, true);

	pixel = table->getCellPixel(nrows-1, 3);
	table->fillCell(nrows, 3, pixel, True);
    }
    else if(!strcmp(cmd, "-"))
    {
	int nrows = table->numRows();
	    
	if(nrows > 1) {
	    table->removeRow(nrows-1);
	}
    }
}

void MapThemeColors::editColor(MmTableEditCallbackStruct *e)
{
    int rgb;
    XColor color;
    ColorScale *c;
    c = color_scale;

    if(e->column >= 0 && e->column < 3) {
	if(!stringToInt(e->new_string, &rgb)) {
	    apply_button->setSensitive(false);
	    return;
	}
    }
    else if(e->column == 4) {
	double d;
	apply_button->setSensitive(stringToDouble(e->new_string, &d));
	return;
    }

    apply_button->setSensitive(true);

    color.pixel = c->pixels[c->num_colors-1-e->row];
    XQueryColor(XtDisplay(base_widget),
		XDefaultColormapOfScreen(XtScreen(base_widget)), &color);
    color.flags = DoRed | DoGreen | DoBlue;

    if(e->column == 0) {
	    color.red = rgb*256;
    }
    else if(e->column == 1) {
	    color.green = rgb*256;
    }
    else if(e->column == 2) {
	    color.blue = rgb*256;
    }
    if(XAllocColor(XtDisplay(base_widget),
			DefaultColormap(XtDisplay(base_widget),
			DefaultScreen(XtDisplay(base_widget))), &color))
    {
	table->fillCell(e->row, 3, color.pixel, True);
    }
}

bool MapThemeColors::setColorScale(void)
{
    XColor color;
    Widget w = map->baseWidget();
    ColorScale *c = color_scale;
    vector<const char *> row;
    int nrows;
    double top, bound;

    nrows = table->numRows();

    top = c->lines[c->num_colors];

    if(nrows <= 0) return true;

    color.flags = DoRed | DoGreen | DoBlue;

    for(int i = 0; i < nrows; i++) {
	table->getRow(i, row);
	if(!stringToUShort(row[0], &color.red) ||
	   !stringToUShort(row[1], &color.green) ||
	   !stringToUShort(row[2], &color.blue) ||
	   !stringToDouble(row[4], &bound))
	{
	    return false;
	}
    }
    if(nrows > c->num_colors) {
	Free(c->lines);
	Free(c->pixels);
	c->pixels = (Pixel *)mallocWarn(nrows*sizeof(Pixel));
	c->lines = (double *)mallocWarn((nrows+1)*sizeof(double));
    }
    c->num_colors = nrows;
    c->lines[c->num_colors] = top;

    for(int i = 0; i < nrows; i++) {
	table->getRow(i, row);
	stringToUShort(row[0], &color.red);
	stringToUShort(row[1], &color.green);
	stringToUShort(row[2], &color.blue);
	stringToDouble(row[4], &bound);

	color.red *= 256;
	color.green *= 256;
	color.blue *= 256;
	XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color);
	c->pixels[c->num_colors-1-i] = color.pixel;
	c->lines[c->num_colors-1-i] = bound;
    }
    map->themeColorScale(id, c);
    return true;
}
