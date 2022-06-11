/** \file Fonts.cpp
 *  \brief Defines class Fonts.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sstream>

extern "C" {
#include "libstring.h"
#include "X11/IntrinsicP.h"
}

#include "Fonts.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "widget/CPlotClass.h"

typedef struct
{
    string menus;
    string toolbars;
    string infobars;
    string tables;
    string axes;
    string waves;
    string amps;
    string arrivals;
} Tags;

static void putFontResources(Tags *tag, Widget widget);
static void saveFontProperties(Tags *tag);
static void getOption(string &tag, Choice *type, Choice *size);
static void getFontLine(string &font, const char *name, char *line,int line_size);
static void setFont(Widget w, Tags *tag);
static void shakeAllWindows(Widget w);
static bool inToolBar(Widget w);
static bool setFontValue(Widget w, char *name, string &tag);
static void resizeWidget(Widget widget, Dimension width, Dimension height,
			Dimension border_width, bool dir);

// only one Fonts window
static Fonts *fonts = NULL;

Fonts::Fonts(const string &name, Component *parent) :
			FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Fonts", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);
    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNnumColumns, 3); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    rc = new RowColumn("rc", this, args, n);

    lab1 = new Label("Menus", rc);
    lab2 = new Label("ToolBars", rc);
    lab3 = new Label("InfoBars", rc);
    lab4 = new Label("Tables", rc);
    lab5 = new Label("Axes Labels", rc);
    lab6 = new Label("Waveform Labels", rc);
    lab7 = new Label("Waveform Amplitudes", rc);
    lab8 = new Label("Arrival Labels", rc);

    type1 = new Choice("Menus Font Type", rc, this);
    type1->addItem("Medium");
    type1->addItem("Bold");
    type2 = new Choice("ToolBars Font Type", rc, this);
    type2->addItem("Medium");
    type2->addItem("Bold");
    type3 = new Choice("InfoBars Font Type", rc, this);
    type3->addItem("Medium");
    type3->addItem("Bold");
    type4 = new Choice("Tables Font Type", rc, this);
    type4->addItem("Medium");
    type4->addItem("Bold");
    type5 = new Choice("Axes Labels Font Type", rc, this);
    type5->addItem("Medium");
    type5->addItem("Bold");
    type6 = new Choice("Waveform Labels Font Type", rc, this);
    type6->addItem("Medium");
    type6->addItem("Bold");
    type7 = new Choice("Waveform Amplitudes Font Type", rc, this);
    type7->addItem("Medium");
    type7->addItem("Bold");
    type8 = new Choice("Arrival Labels Font Type", rc, this);
    type8->addItem("Medium");
    type8->addItem("Bold");

    size1 = new Choice("Menus Font Size", rc, this);
    size1->addItem("8"); size1->addItem("10"); size1->addItem("12");
    size1->addItem("14"); size1->addItem("18"); size1->addItem("24");
    size2 = new Choice("ToolBars Font Size", rc, this);
    size2->addItem("8"); size2->addItem("10"); size2->addItem("12");
    size2->addItem("14"); size2->addItem("18"); size2->addItem("24");
    size3 = new Choice("InfoBars Font Size", rc, this);
    size3->addItem("8"); size3->addItem("10"); size3->addItem("12");
    size3->addItem("14"); size3->addItem("18"); size3->addItem("24");
    size4 = new Choice("Tables Font Size", rc, this);
    size4->addItem("8"); size4->addItem("10"); size4->addItem("12");
    size4->addItem("14"); size4->addItem("18"); size4->addItem("24");
    size5 = new Choice("Axes Labels Font Size", rc, this);
    size5->addItem("8"); size5->addItem("10"); size5->addItem("12");
    size5->addItem("14"); size5->addItem("18"); size5->addItem("24");
    size6 = new Choice("Waveform Labels Font Size", rc, this);
    size6->addItem("8"); size6->addItem("10"); size6->addItem("12");
    size6->addItem("14"); size6->addItem("18"); size6->addItem("24");
    size7 = new Choice("Waveform Amplitudes Font Size", rc, this);
    size7->addItem("8"); size7->addItem("10"); size7->addItem("12");
    size7->addItem("14"); size7->addItem("18"); size7->addItem("24");
    size8 = new Choice("Arrival Labels Font Size", rc, this);
    size8->addItem("8"); size8->addItem("10"); size8->addItem("12");
    size8->addItem("14"); size8->addItem("18"); size8->addItem("24");

    fontsInit();

    setFontMenus();

    XtManageChild(base_widget);
}

Fonts::~Fonts(void)
{
}

void Fonts::setVisible(bool visible)
{
    if(visible) {
	setFontMenus();
    }
    FormDialog::setVisible(visible);
}

void Fonts::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	setFonts();
    }
}

//static
void Fonts::showFonts(void)
{
    if(!fonts) {
	Application *app = Application::getApplication();
	vector<Component *> *v = app->getWindows();
	if((int)v->size() > 0) {
	    fonts = new Fonts("Fonts", v->front());
	}
	else {
	    fonts = new Fonts("Fonts", app);
	}
    }
    fonts->setVisible(true);
}

ParseCmd Fonts::parse(const string &cmd, string &msg)
{
    if(!fonts) {
	Application *app = Application::getApplication();
	vector<Component *> *v = app->getWindows();
	if((int)v->size() > 0) {
	    fonts = new Fonts("Fonts", v->front());
	}
	else {
	    fonts = new Fonts("Fonts", app);
	}
    }
    return fonts->parseCmd(cmd, msg);
}

ParseCmd Fonts::parseCmd(const string &cmd, string &msg)
{
    char *name, *type, *size, *last, buf[100];
    stringcpy(buf, cmd.c_str(), sizeof(buf));

    name = strtok_r(buf, "=:, \t", &last);
    if(!strcasecmp(name, "Menus")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type1->parseCmd(type, msg) != COMMAND_PARSED)
	{ 
	    return ARGUMENT_ERROR;
	}
	if(!size || size1->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "ToolBars")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type2->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size2->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "InfoBars")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type3->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size3->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Tables")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type4->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size4->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Axes_Labels")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type5->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size5->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Waveform_Labels")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type6->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size6->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Waveform_Amplitudes")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type7->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size7->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Arrival_Labels")) {
	type = strtok_r(NULL, "=:, \t", &last);
	size = strtok_r(NULL, "=:, \t", &last);
	if(!type || type8->parseCmd(type, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
	if(!size || size8->parseCmd(size, msg) != COMMAND_PARSED)
	{
	    return ARGUMENT_ERROR;
	}
    }
    else if(!strcasecmp(name, "Apply")) {
	setFonts();
    }
    else if(parseCompare(cmd, "Help")) {
        char prefix[200];
        getParsePrefix(prefix, sizeof(prefix));
        parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void Fonts::parseHelp(const char *prefix)
{
    printf("%smenus=(medium or bold) (8,10,12,14,18, or 24)\n", prefix);
    printf("%stoolbars=(medium or bold) (8,10,12,14,18, or 24)\n", prefix);
    printf("%sinfobars=(medium or bold) (8,10,12,14,18, or 24)\n", prefix);
    printf("%stables=(medium or bold) (8,10,12,14,18, or 24)\n", prefix);
    printf("%saxes_labels=(medium or bold) (8,10,12,14,18, or 24)\n", prefix);
    printf("%swaveform_labels=(medium or bold) (8,10,12,14,18, or 24)\n",
		prefix);
    printf("%swaveform_amplitudes=(medium or bold) (8,10,12,14,18, or 24)\n",
		prefix);
    printf("%sarrival_labels=(medium or bold) (8,10,12,14,18, or 24)\n",prefix);
    printf("%sapply\n", prefix);
}

#define XtNdefaultRenderTable "defaultRenderTable"
#define XtCDefaultRenderTable "DefaultRenderTable"

typedef struct
{
	char		name[4];
	XmFontList	list;
	XFontStruct	*font;
} NamedFont;

typedef struct
{
	int		num_fonts;
	NamedFont	fonts[12];
} fontData, *fontDataPtr;

#define offset(field)	XtOffset(fontDataPtr, field)
static XtResource resources[] =
{
	{(char *)"fontM8", (char *)"FontM8", XmRFontList, sizeof(XmFontList),
		offset(fonts[0].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-8-*-*-*-*-*-*-*"},
	{(char *)"fontM10", (char *)"FontM10", XmRFontList, sizeof(XmFontList),
		offset(fonts[1].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*"},
	{(char *)"fontM12", (char *)"FontM12", XmRFontList, sizeof(XmFontList),
		offset(fonts[2].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"},
	{(char *)"fontM14", (char *)"FontM14", XmRFontList, sizeof(XmFontList),
		offset(fonts[3].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-14-*-*-*-*-*-*-*"},
	{(char *)"fontM18", (char *)"FontM18", XmRFontList, sizeof(XmFontList),
		offset(fonts[4].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-18-*-*-*-*-*-*-*"},
	{(char *)"fontM24", (char *)"FontM24", XmRFontList, sizeof(XmFontList),
		offset(fonts[5].list), XtRString,
		(XtPointer)"-adobe-helvetica-medium-r-*-*-24-*-*-*-*-*-*-*"},
	{(char *)"fontB8", (char *)"FontB8", XmRFontList, sizeof(XmFontList),
		offset(fonts[6].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-8-*-*-*-*-*-*-*"},
	{(char *)"fontB10", (char *)"FontB10", XmRFontList, sizeof(XmFontList),
		offset(fonts[7].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"},
	{(char *)"fontB12", (char *)"FontB12", XmRFontList, sizeof(XmFontList),
		offset(fonts[8].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*"},
	{(char *)"fontB14", (char *)"FontB14", XmRFontList, sizeof(XmFontList),
		offset(fonts[9].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-14-*-*-*-*-*-*-*"},
	{(char *)"fontB18", (char *)"FontB18", XmRFontList, sizeof(XmFontList),
		offset(fonts[10].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-18-*-*-*-*-*-*-*"},
	{(char *)"fontB24", (char *)"FontB24", XmRFontList, sizeof(XmFontList),
		offset(fonts[11].list), XtRString,
		(XtPointer)"-adobe-helvetica-bold-r-*-*-24-*-*-*-*-*-*-*"},
};
#undef offset

static fontData font_data;

void Fonts::fontsInit(void)
{
    Tags tag;
    XmFontContext context;
    XmStringCharSet charset;

    Application *application = Application::getApplication();

    XtGetApplicationResources(application->baseWidget(), &font_data, resources,
				XtNumber(resources), NULL, 0);
    font_data.num_fonts = 12;
    stringcpy(font_data.fonts[0].name, "M8",  sizeof(font_data.fonts[0].name));
    stringcpy(font_data.fonts[1].name, "M10", sizeof(font_data.fonts[1].name));
    stringcpy(font_data.fonts[2].name, "M12", sizeof(font_data.fonts[2].name));
    stringcpy(font_data.fonts[3].name, "M14", sizeof(font_data.fonts[3].name));
    stringcpy(font_data.fonts[4].name, "M18", sizeof(font_data.fonts[4].name));
    stringcpy(font_data.fonts[5].name, "M24", sizeof(font_data.fonts[5].name));
    stringcpy(font_data.fonts[6].name, "B8",  sizeof(font_data.fonts[6].name));
    stringcpy(font_data.fonts[7].name, "B10", sizeof(font_data.fonts[7].name));
    stringcpy(font_data.fonts[8].name, "B12", sizeof(font_data.fonts[8].name));
    stringcpy(font_data.fonts[9].name, "B14", sizeof(font_data.fonts[9].name));
    stringcpy(font_data.fonts[10].name,"B18", sizeof(font_data.fonts[10].name));
    stringcpy(font_data.fonts[11].name,"B24", sizeof(font_data.fonts[11].name));

    for(int i = 0; i < 12; i++)
    {
	font_data.fonts[i].font = NULL;
	if(!XmFontListInitFontContext(&context, font_data.fonts[i].list)
		|| !XmFontListGetNextFont(context, &charset,
			&font_data.fonts[i].font))
	{
	    showWarning("fontsInit: invalid fontList for M8");
	    return;
	}
	XtFree(charset);
	XmFontListFreeFontContext(context);
    }


    if(!getProperty("menuFont", tag.menus))	  tag.menus.assign("B12");
    if(!getProperty("toolbarFont", tag.toolbars)) tag.toolbars.assign("M14");
    if(!getProperty("infobarFont", tag.infobars)) tag.infobars.assign("M12");
    if(!getProperty("tableFont", tag.tables))	  tag.tables.assign("M12");
    if(!getProperty("axesFont", tag.axes))	  tag.axes.assign("B12");
    if(!getProperty("waveLabelFont", tag.waves))  tag.waves.assign("B12");
    if(!getProperty("waveAmpFont", tag.amps))	  tag.amps.assign("M10");
    if(!getProperty("arrivalFont", tag.arrivals)) tag.arrivals.assign("B12");

    putProperty("menuFont", tag.menus, false);
    putProperty("toolbarFont", tag.toolbars, false);
    putProperty("infobarFont", tag.infobars, false);
    putProperty("tableFont", tag.tables, false);
    putProperty("axesFont", tag.axes, false);
    putProperty("waveLabelFont", tag.waves, false);
    putProperty("waveAmpFont", tag.amps, false);
    putProperty("arrivalFont", tag.arrivals, false);

    putFontResources(&tag, base_widget);
}

void Fonts::setFontMenus(void)
{
    string prop;
    const char *type;
    if(getProperty("menuFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type1->setChoice(type);
	size1->setChoice(prop.substr(1));
    }
    if(getProperty("toolbarFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type2->setChoice(type);
	size2->setChoice(prop.substr(1));
    }
    if(getProperty("infobarFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type3->setChoice(type);
	size3->setChoice(prop.substr(1));
    }
    if(getProperty("tableFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type4->setChoice(type);
	size4->setChoice(prop.substr(1));
    }
    if(getProperty("axesFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type5->setChoice(type);
	size5->setChoice(prop.substr(1));
    }
    if(getProperty("waveLabelFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type6->setChoice(type);
	size6->setChoice(prop.substr(1));
    }
    if(getProperty("waveAmpFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type7->setChoice(type);
	size7->setChoice(prop.substr(1));
    }
    if(getProperty("arrivalFont", prop) && (int)prop.length() > 1) {
	type = (prop[0] == 'B') ? "Bold" : "Medium";
	type8->setChoice(type);
	size8->setChoice(prop.substr(1));
    }
}

void Fonts::setFonts(void)
{
    Tags tag;

    getOption(tag.menus, type1, size1);
    getOption(tag.toolbars, type2, size2);
    getOption(tag.infobars, type3, size3);
    getOption(tag.tables, type4, size4);
    getOption(tag.axes, type5, size5);
    getOption(tag.waves, type6, size6);
    getOption(tag.amps, type7, size7);
    getOption(tag.arrivals, type8, size8);

    putFontResources(&tag, base_widget);

    Application *application = Application::getApplication();
    setFont(application->baseWidget(), &tag);

    shakeAllWindows(application->baseWidget());

    saveFontProperties(&tag);
}

static void
putFontResources(Tags *tag, Widget widget)
{
	char line[1024];
	XrmDatabase database;

        database = XrmGetDatabase(XtDisplay(widget));

	getFontLine(tag->menus, "fontList", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->axes, "TtPlot.axesFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->axes, "CPlot.axesFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->axes, "Axes.axesFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->axes, "axesFont", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->waves, "TtPlot.tagFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->waves, "CPlot.tagFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->waves, "tagFont", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->arrivals, "TtPlot.arrivalFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->arrivals, "CPlot.arrivalFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->arrivals, "arrivalFont", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->amps, "TtPlot.ampFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->amps, "CPlot.ampFont", line, sizeof(line));
	XrmPutLineResource(&database, line);
	getFontLine(tag->amps, "ampFont", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->toolbars, "toolbar*fontList", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->tables, "MmTable.font", line, sizeof(line));
	XrmPutLineResource(&database, line);

	getFontLine(tag->infobars, "Info.font", line, sizeof(line));
	XrmPutLineResource(&database, line);
}

static void
saveFontProperties(Tags *tag)
{
    Application::putProperty("menuFont", tag->menus);
    Application::putProperty("toolbarFont", tag->toolbars);
    Application::putProperty("infobarFont", tag->infobars);
    Application::putProperty("tableFont", tag->tables);
    Application::putProperty("axesFont", tag->axes);
    Application::putProperty("waveLabelFont", tag->waves);
    Application::putProperty("waveAmpFont", tag->amps);
    Application::putProperty("arrivalFont", tag->arrivals);

    Application::writeApplicationProperties();
}

static void
getOption(string &tag, Choice *type, Choice *size)
{
    char s[50];
    snprintf(s, sizeof(s), "%c%s", *(type->getChoice()), size->getChoice());
    tag.assign(s);
}

static void
getFontLine(string &font, const char *name, char *line, int line_size)
{
    int size;
    const char *prog_name = Application::getApplication()->resourceName();

    sscanf(font.c_str()+1, "%d", &size);
    if(font[0] == 'M') {
	snprintf(line, line_size,
	    "%s*%s:-adobe-helvetica-medium-r-*-*-%d-*-*-*-*-*-*-*",
		prog_name, name, size);
    }
    else {
	snprintf(line, line_size,
	    "%s*%s:-adobe-helvetica-bold-r-*-*-%d-*-*-*-*-*-*-*",
		prog_name, name, size);
    }
}

static void
setFont(Widget w, Tags *tag)
{
    Arg args[2];
    int i, num;
    Widget *children;

    if(XtIsSubclass(w, compositeWidgetClass))
    {
	Dimension h, hmax_before=0, hmax_after, h_before=0;
	bool menu = False;

	if(XmIsRowColumn(w)) {
	    unsigned char type;
	    XtSetArg(args[0], XmNrowColumnType, &type);
	    XtGetValues(w, args, 1);
	    if(type != XmWORK_AREA && type != XmMENU_BAR) menu = True;
	}
	XtSetArg(args[0], XmNnumChildren, &num);
	XtSetArg(args[1], XmNchildren, &children);
	XtGetValues(w, args, 2);

	if(!menu)
	{
	    hmax_before = 0;
	    h_before = w->core.height;
	    for(i = 0; i < num; i++) {
		if((XtIsSubclass(children[i], xmLabelWidgetClass) &&
			!XmIsCascadeButton(children[i])) ||
			XmIsText(children[i]))
		{
		    h = children[i]->core.y + children[i]->core.height;
		    if(h > hmax_before) hmax_before = h;
		}
	    }
	}

	for(i = 0; i < num; i++) {
	    setFont(children[i], tag);
	}

	if(!menu)
	{
	    hmax_after = 0;
	    for(i = 0; i < num; i++) {
		if((XtIsSubclass(children[i], xmLabelWidgetClass) &&
			!XmIsCascadeButton(children[i])) ||
			XmIsText(children[i]))
		{
		    h = children[i]->core.y + children[i]->core.height;
		    if(h > hmax_after) hmax_after = h;
		}
	    }
	    if(hmax_after != hmax_before &&
			!strcmp(w->core.name, "toolBar"))
	    {
		if(h_before + hmax_after-hmax_before > 0)
		{
		    bool dir = (hmax_after > hmax_before) ? True : False;
		    resizeWidget(w, w->core.width,
				h_before + hmax_after - hmax_before,
				w->core.border_width, dir);
		}
	    }
	}
	num = w->core.num_popups;
	children = w->core.popup_list;
	for(i = 0; i < num; i++) {
	    setFont(children[i], tag);
	}
    }
    else if(XmIsCascadeButton(w))
    {
	Widget pulldown;

	XtSetArg(args[0], XmNsubMenuId, &pulldown);
	XtGetValues(w, args, 1);

	if(pulldown != NULL) {
	    XtSetArg(args[0], XmNnumChildren, &num);
	    XtSetArg(args[1], XmNchildren, &children);
	    XtGetValues(pulldown, args, 2);

	    for(i = 0; i < num; i++) {
		setFont(children[i], tag);
	    }
	}
    }

    if(!XtIsRealized(w)) return;

    if(inToolBar(w)) {
	if(setFontValue(w, XtNfont, tag->toolbars)) return;
    }
    if(XtIsSubclass(w, infoWidgetClass)) {
	if(setFontValue(w, XtNfont, tag->infobars)) return;
    }
    if(XtIsSubclass(w, mmTableWidgetClass)) {
	if(setFontValue(w, XtNfont, tag->tables)) return;
    }
    if(XtIsSubclass(w, cPlotWidgetClass)) {
	setFontValue(w, XtNtagFont, tag->waves);
	setFontValue(w, XtNarrivalFont, tag->arrivals);
	setFontValue(w, XtNampFont, tag->amps);
    }
/*
    if(XtIsSubclass(w, mapPlotWidgetClass))
    {
	setFontValue(w, XtNlabelFont, map_tag);
    }
*/
    if(XtIsSubclass(w, axesWidgetClass)) {
	if(setFontValue(w, XtNaxesFont, tag->axes)) return;
    }

    if(!setFontValue(w, XmNfont, tag->menus)) return;

    if(XtIsSubclass(w, xmTextWidgetClass))
    {
	char *text;
		
	if((text = XmTextGetString(w)) != NULL) {
	    XmTextSetString(w, (char *)"");
	    XmTextSetString(w, text);
	    XtFree(text);
	}
    }
    else if(XtIsSubclass(w, xmTextFieldWidgetClass))
    {
	char *text;

	XtSetArg(args[0], XmNvalue, &text);
	XtGetValues(w, args, 1);
	XtSetArg(args[0], XmNvalue, "");
	XtSetValues(w, args, 1);
	XtSetArg(args[0], XmNvalue, text);
	XtSetValues(w, args, 1);
	XtFree(text);
    }
}

static void
shakeAllWindows(Widget w)
{
    if(XtIsSubclass(w, applicationShellWidgetClass))
    {
	XtResizeWidget(w, w->core.width+1, w->core.height+1,
				w->core.border_width);
	XtResizeWidget(w, w->core.width-1, w->core.height-1,
				w->core.border_width);
    }
    else if(w->core.parent != NULL && XmIsDialogShell(w->core.parent))
    {
	XtResizeWidget(w->core.parent, w->core.width+1, w->core.height+1,
				w->core.border_width);
	XtResizeWidget(w->core.parent, w->core.width-1, w->core.height-1,
				w->core.border_width);
    }
    if(XtIsSubclass(w, compositeWidgetClass))
    {
	Arg args[2];
	int i, num;
	Widget *children;

	XtSetArg(args[0], XmNnumChildren, &num);
	XtSetArg(args[1], XmNchildren, &children);
	XtGetValues(w, args, 2);

	for(i = 0; i < num; i++) {
	    shakeAllWindows(children[i]);
	}

	num = w->core.num_popups;
	children = w->core.popup_list;
	for(i = 0; i < num; i++) {
	    shakeAllWindows(children[i]);
	}
    }
}

static bool
inToolBar(Widget w)
{
    while(XtParent(w) != NULL) {
	w = XtParent(w);
	if(!strcmp(w->core.name, "toolbar")) return true;
    }
    return false;
}

static bool
setFontValue(Widget w, char *name, string &tag)
{
    int i;
    Arg args[2];
    bool b = False;
    XmFontList fontList = NULL;
    XFontStruct *font = NULL;

    for(i = 0; i < font_data.num_fonts; i++) {
	if(!tag.compare(font_data.fonts[i].name)) {
	    font = font_data.fonts[i].font;
	    fontList = font_data.fonts[i].list;
	    break;
	}
    }
    if(font != NULL) {
	XtSetArg(args[0], name, font);
	XtSetArg(args[1], XmNfontList, fontList);
	XtSetValues(w, args, 2);
	b = True;
    }
    return b;
}

static void
resizeWidget(Widget widget, Dimension width, Dimension height,
			Dimension border_width, bool dir)
{
    int n;
    Dimension wid, h;
    Widget w, parents[100];

    n = 0;
    for(w = widget; w->core.parent != NULL &&
	!XmIsDialogShell(w->core.parent); w = w->core.parent)
    {
	if(dir) {
	    if(w->core.parent->core.height < height) {
		parents[n++] = w->core.parent;
	    }
	}
	else {
	    if(w->core.parent->core.height > height &&
		w->core.parent->core.height - height < 20) {
		    parents[n++] = w->core.parent;
	    }
	}
    }
    for(--n; n >= 0; n--) {
	XtMakeResizeRequest(parents[n], parents[n]->core.width, height,&wid,&h);
    }
    XtMakeResizeRequest(widget, width, height, &wid, &h);
}
