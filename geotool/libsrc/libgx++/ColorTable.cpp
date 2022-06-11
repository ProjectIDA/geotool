/** \file ColorTable.cpp
 *  \brief Defines class ColorTable.
 *  \author Ivan Henson
 */
#include "config.h"
#include "ColorTable.h"
#include "WaveformPlot.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}

static int num_rgb = -1;
typedef struct
{
    short r, g, b;
    int color_name;
} RGB;

static RGB *rgb = NULL;
static void readRGB(void);

static void msgHandler(
    String	name,
    String	type,
    String	class_name,
    String	default_name,
    String*	params,
    Cardinal*	num_params)
{
}

static void msgHandler2(
    String	name,
    String	type,
    String	class_name,
    String	default_name,
    String*	params,
    Cardinal*	num_params)
{
    if(*num_params == 0) {
	fprintf(stderr, default_name);
    }
    else if(*num_params == 1) {
	fprintf(stderr, default_name, params[0]);
    }
    else if(*num_params == 2) {
	fprintf(stderr, default_name, params[0], params[1]);
    }
    else if(*num_params == 3) {
	fprintf(stderr, default_name, params[0], params[1], params[2]);
    }
}

ColorTable::ColorTable(const string &name, Component *parent) :
			FormDialog(name, parent)
{
    Arg args[20];
    int n, ncols, alignment[5];
    const char *labels[5];
    bool col_editable[5];

    ncols = 5;
    labels[0] = "color name";
    labels[1] = "red";
    labels[2] = "green";
    labels[3] = "blue";
    labels[4] = "color";
    alignment[0] = LEFT_JUSTIFY;
    alignment[1] = RIGHT_JUSTIFY;
    alignment[2] = RIGHT_JUSTIFY;
    alignment[3] = RIGHT_JUSTIFY;
    alignment[4] = LEFT_JUSTIFY;
    col_editable[0] = true;
    col_editable[1] = true;
    col_editable[2] = true;
    col_editable[3] = true;
    col_editable[4] = false;

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    close_button = new Button("Close", rc, this);
    apply_button = new Button("Apply", rc, this);
    apply_button->setSensitive(false);
    default_button = new Button("Default", rc, this);

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
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNcolumns, ncols); n++;
    XtSetArg(args[n], XtNcolumnLabels, labels); n++;
    XtSetArg(args[n], XtNwidth, 310); n++;
    XtSetArg(args[n], XtNheight, 350); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);

    table->setAlignment(ncols, alignment);
    table->setColumnEditable(col_editable);

    num_colors = 0;
    num_tmp_colors = 0;
    pixels = NULL;
    tmp_pixels = NULL;

    loadTable();

    readRGB();
}

ColorTable::~ColorTable(void)
{
    Free(pixels);
    Free(tmp_pixels);
}

void ColorTable::loadTable(void)
{
    Widget w = base_widget;
    const char *row[5];
    char **color_name=NULL;

    Free(pixels);
    Free(tmp_pixels);
    num_colors = num_tmp_colors = 0;

    if( (num_colors = getProperty("num_waveform_colors", 0)) > 0)
    {
	pixels = (Pixel *)mallocWarn(num_colors*sizeof(Pixel));
	color_name = (char **)mallocWarn(num_colors*sizeof(char *));

	for(int i = 0; i < num_colors; i++) {
            char s[20];
            snprintf(s, sizeof(s), "waveform_color%d", i+1);
            if( (color_name[i] = getProperty(s)) ) {
		pixels[i] = comp_parent->stringToPixel(color_name[i]);
            }
            else {
		pixels[i] = comp_parent->stringToPixel("black");
		color_name[i] = strdup("black");
            }
        }
    }
    else {
	num_colors = 12;
	pixels = (Pixel *)mallocWarn(num_colors*sizeof(Pixel));
	color_name = (char **)mallocWarn(num_colors*sizeof(char *));

	color_name[0]  = strdup("forest green");
	color_name[1]  = strdup("sky blue");
	color_name[2]  = strdup("orange");
	color_name[3]  = strdup("brown");
	color_name[4]  = strdup("MediumOrchid");
	color_name[5]  = strdup("red");
	color_name[6]  = strdup("thistle");
	color_name[7]  = strdup("sea green");
	color_name[8]  = strdup("tan");
	color_name[9]  = strdup("maroon");
	color_name[10] = strdup("slate blue");
	color_name[11] = strdup("grey");
	for(int i = 0; i < num_colors; i++) {
	    pixels[i] = comp_parent->stringToPixel(color_name[i]);
	}
    }

    table->removeAllRows();

    for(int i = 0; i < num_colors; i++)
    {
	char red[20], green[20], blue[20];
	XColor color;

	color.pixel = pixels[i];
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	XQueryColor(XtDisplay(w), XDefaultColormapOfScreen(XtScreen(w)),&color);

	snprintf(red, sizeof(red), "%d", color.red/256);
	snprintf(green, sizeof(green), "%d", color.green/256);
	snprintf(blue, sizeof(blue), "%d", color.blue/256);

	row[0] = isdigit(color_name[i][0]) ? (char *)"" : color_name[i];
	row[1] = red;
	row[2] = green;
	row[3] = blue;
	row[4] = (char *)"";

	table->addRow(row, false);
    }
    for(int i = 0; i < num_colors; i++) {
	table->fillCell(i, 4, pixels[i], false);
    }
    table->adjustColumns();

    tmp_pixels = (Pixel *)mallocWarn(num_colors*sizeof(Pixel));
    num_tmp_colors = num_colors;
    for(int i = 0; i < num_colors; i++) {
	tmp_pixels[i] = pixels[i];
    }
}

void ColorTable::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
	loadTable();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNvalueChangedCallback))
    {
	editColor((MmTableEditCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Apply")) 
    {
	saveColors();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Default")) 
    {
	putProperty("num_waveform_colors", "0");
	loadTable();
	saveColors();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "+"))
    {
	addRow();
    }
    else if(!strcmp(cmd, "-"))
    {
	int nrows = table->numRows();
	    
	if(nrows > 1) {
	    table->removeRow(nrows-1);
	    num_tmp_colors--;
	    setButtonsSensitive();
	}
    }
}

void ColorTable::addRow(void)
{
    vector<const char *> row;
    int nrows = table->numRows();
    Pixel pix;
	    
    table->getRow(nrows-1, row);
    table->addRow(row, true);

    pix = table->getCellPixel(nrows-1, 4);
    table->fillCell(nrows, 4, pix, true);

    num_tmp_colors++;
    tmp_pixels = (Pixel *)reallocWarn(tmp_pixels, num_tmp_colors*sizeof(Pixel));
    tmp_pixels[num_tmp_colors-1] = pix;
    setButtonsSensitive();
}

void ColorTable::editColor(MmTableEditCallbackStruct *e)
{
    Widget w = base_widget;
    int i;
    const char *name;
    XColor color;

    if(e->column > 0 && e->column < 4) {	// rgb value
	if(!stringToInt(e->new_string, &i)) {
	    apply_button->setSensitive(false);
	}
	else {
	    color.pixel = tmp_pixels[e->row];
	    XQueryColor(XtDisplay(w), XDefaultColormapOfScreen(XtScreen(w)),
			&color);
	    color.flags = DoRed | DoGreen | DoBlue;
	    if(e->column == 1) {
		color.red = i*256;
	    }
	    else if(e->column == 2) {
		color.green = i*256;
	    }
	    else if(e->column == 3) {
		color.blue = i*256;
	    }
	    
	    if(XAllocColor(XtDisplay(w), DefaultColormap(XtDisplay(w),
			DefaultScreen(XtDisplay(w))), &color))
	    {
		table->fillCell(e->row, 4, color.pixel, true);
		tmp_pixels[e->row] = color.pixel;
		if( (name = rgbToName(
				(short)(color.red/256),
				(short)(color.green/256),
				(short)(color.blue/256)) ) )
		{
		    table->setField(e->row, 0, name, true);
		}
		else {
		    table->setField(e->row, 0, "", true);
		}
		setButtonsSensitive();
	    }
	    else {
	        apply_button->setSensitive(false);
		table->setField(e->row, 0, "", true);
	    }
	}
    }
    else if(e->column == 0) {	// color name
	XrmValue from, to;
	to.addr = NULL;
	to.size = 0;
	from.addr = e->new_string;
	from.size = strlen(e->new_string) + 1;

	// avoid warning messages about colors not defined
	XtAppSetWarningMsgHandler(Application::getAppContext(), msgHandler);

        if(XtConvertAndStore(w, XtRString, &from, XtRPixel, &to) && to.addr)
        {
	    char s[20];
	    tmp_pixels[e->row] = *(Pixel *) to.addr;
	    color.pixel = tmp_pixels[e->row];
	    XQueryColor(XtDisplay(w), XDefaultColormapOfScreen(XtScreen(w)),
			&color);
	    snprintf(s, sizeof(s), "%d", (int)(color.red/256));
	    table->setField(e->row, 1, s, true);
	    snprintf(s, sizeof(s), "%d", (int)(color.green/256));
	    table->setField(e->row, 2, s, true);
	    snprintf(s, sizeof(s), "%d", (int)(color.blue/256));
	    table->setField(e->row, 3, s, true);
	    table->fillCell(e->row, 4, color.pixel, true);
	    setButtonsSensitive();
        }
	else {
	    apply_button->setSensitive(false);
	    return;
	}
	XtAppSetWarningMsgHandler(Application::getAppContext(), msgHandler2);
    }
}

void ColorTable::saveColors(void)
{
    char prop[50], value[50];
    vector<const char *> name, red, green, blue;

    Free(pixels);
    num_colors = num_tmp_colors;
    pixels = (Pixel *)mallocWarn(num_colors*sizeof(Pixel));
    for(int i = 0; i < num_colors; i++) {
	pixels[i] = tmp_pixels[i];
    }
    table->getColumn(0, name);
    table->getColumn(1, red);
    table->getColumn(2, green);
    table->getColumn(3, blue);
    snprintf(value, sizeof(value), "%d", num_colors);
    putProperty("num_waveform_colors", value);

    for(int i = 0; i < num_colors; i++)
    {
	snprintf(prop, sizeof(prop), "waveform_color%d", i+1);
	if(name[i][0] != '\0') {
	    snprintf(value, sizeof(value), "%s", name[i]);
	}
	else {
	    snprintf(value, sizeof(value), "%s,%s,%s", red[i],green[i],blue[i]);
	}
	putProperty(prop, value);
    }

    WaveformPlot::resetWaveformColors();
}

static void
readRGB(void)
{
    if(num_rgb >= 0) return;
    num_rgb = 0;

    FILE *fp = fopen("/usr/lib/X11/rgb.txt", "r");
    if( !fp ) return;

    char line[200], name[100], *c, *last;
    int r, g, b;

    rgb = (RGB *)mallocWarn(sizeof(RGB));

    while( !stringGetLine(fp, line, sizeof(line)) )
    {
	stringTrim(line);
	if(line[0] != '!')
	{
	    if(	(c = strtok_r(line, " \t", &last)) && stringToInt(c, &r) &&
		(c = strtok_r(NULL, " \t", &last)) && stringToInt(c, &g) &&
		(c = strtok_r(NULL, " \t", &last)) && stringToInt(c, &b))
	    {
		name[0] = '\0';
		while((c = strtok_r(NULL, " \t", &last))) {
		    int len = (int)strlen(name);
		    snprintf(name+len, sizeof(name)-len, "%s ", c);
		    stringTrim(name);
		}
		if(name[0] != '\0') {
		    rgb = (RGB *)reallocWarn(rgb, (num_rgb+1)*sizeof(RGB));
		    rgb[num_rgb].r = (short)r;
		    rgb[num_rgb].g = (short)g;
		    rgb[num_rgb].b = (short)b;
		    rgb[num_rgb].color_name = stringToQuark(name);
		    num_rgb++;
		}
	    }
	}
    }
    fclose(fp);
}

// static
const char *
ColorTable::rgbToName(short r, short g, short b)
{
    if(num_rgb < 0) readRGB();

    for(int i = 0; i < num_rgb; i++) {
	if(rgb[i].r == r && rgb[i].g == g && rgb[i].b == b) {
	    return quarkToString(rgb[i].color_name);
	}
    }
    return NULL;
}

void ColorTable::setButtonsSensitive(void)
{
    if(num_colors != num_tmp_colors) {
	apply_button->setSensitive(true);
	close_button->setLabel("Cancel");
	return;
    }
    for(int i = 0; i < num_colors; i++) {
	if(pixels[i] != tmp_pixels[i]) {
	    apply_button->setSensitive(true);
	    close_button->setLabel("Cancel");
	    return;
	}
    }
    apply_button->setSensitive(false);
    close_button->setLabel("Close");
}

ParseCmd ColorTable::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "apply")) {
	saveColors();
	setButtonsSensitive();
    }
    else if(parseCompare(cmd, "default")) {
	putProperty("num_waveform_colors", "0");
	loadTable();
	saveColors();
	setButtonsSensitive();
    }
    else if(parseGetArg(cmd, "colors", c)) {
	if(!setColors(c, msg)) {
	    return ARGUMENT_ERROR;
	}
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

bool ColorTable::setColors(const string &names, string &msg)
{
    int i, j;
    char *c, *s, *tok, *last, *r, *b, *g;

    i = 0;
    s = strdup(names.c_str());
    tok = s;
    while((c = strtok_r(tok, ",", &last))) {
	tok = NULL;
	if(i >= table->numRows()) {
	    addRow();
	}
	if(stringToInt(c, &j)) {
	    r = c;
	    if(!(g = strtok_r(NULL, ",", &last)) || !stringToInt(g, &j) ||
		!(b = strtok_r(NULL, ",", &last)) || !stringToInt(b, &j))
	    {
		msg.assign("colors: cannot interpret r,g,b");
		free(s);
		return false;
	    }
	    table->setFieldWithCB(i, 1, r);
	    table->setFieldWithCB(i, 2, g);
	    table->setFieldWithCB(i, 3, b);
	}
	else {
	    table->setFieldWithCB(i, 0, c);
	}
	i++;
    }
    free(s);
    num_tmp_colors = table->numRows();
    if(i > 0) {
	while(table->numRows() > i) {
	    table->removeRow(table->numRows()-1);
	    num_tmp_colors--;
	}
    }
    saveColors();
    setButtonsSensitive();
    return true;
}
