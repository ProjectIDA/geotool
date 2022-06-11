/** \file Example.cpp
 *  \brief Defines class Example.
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#include "Example.h"		  // For this class definition
#include "motif++/MotifClasses.h" // For MenuBar, Menu, Button, etc.
#include "CPlotClass.h"
#include "PrintDialog.h"

// Example Frame subclass

Example::Example(const char *name, Component *parent) : Frame(name, parent)
{
    createInterface();
}

void Example::createInterface()
{
    int n;
    Arg args[20];

    // Set the initial width and height of the window.
    setSize(600, 400);

    // create a menu bar, tool bar, and info area (at the bottom)
    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    // create a File menu with two buttons
    file_menu = new Menu("File", menu_bar);
    print_button = new Button("Print...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    // create an Edit menu with one buttons
    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);

    // create an Option menu with two buttons
    option_menu = new Menu("Option", menu_bar);
    compute_button = new Button("Compute", option_menu, this);
    cursor_button = new Button("Cursor Position", option_menu, this);

    // create a Help menu with one button
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Example", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNinfoWidget, info_area->rightInfo()); n++;
    XtSetArg(args[n], XtNtitle, "Example CPlotClass"); n++;

    // create a CPlotClass object.
    plot = new CPlotClass("plot", frame_form, args, n);
    plot->addActionListener(this, XtNdoubleLineCallback);

    if(!tool_bar->loadDefaults()) {
	// Add Close and Compute to the tool bar
	tool_bar->add(close_button, "Close");
	tool_bar->add(compute_button, "Compute");
    }

    print_dialog = NULL;
}

/* Plug-in window destructor
 */
Example::~Example(void)
{
}

/* Handle ActionEvents (From Button, Toggle, etc.)
 */
void Example::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();
    const char *reason = action_event->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Compute")) {
	compute();
    }
    else if(!strcmp(cmd, "Clear")) {
	plot->clear();
    }
    else if(!strcmp(cmd, "Print...")) {
	printDialog();
    }
    else if(!strcmp(reason, XtNdoubleLineCallback)) {
	horizontalLine((AxesCursorCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Cursor Position")) {
	cursorPosition();
    }
    else if(comp == help_button) {
	displayHelp("Click the Compute button to draw a sin curve.");
    }
}

void Example::horizontalLine(AxesCursorCallbackStruct *p)
{
    printf("p->scaled_y1 = %.2lf  p->scaled_y2 = %.2lf\n",
		p->scaled_y1, p->scaled_y2);
}

void Example::cursorPosition(void)
{
    AxesCursorCallbackStruct *p;
    int n = plot->getDoubleLines(&p);

    for(int i = 0; i < n; i++) {
	printf("scaled_y1 = %.2lf  scaled_y2 = %.2lf\n",
		p[i].scaled_y1, p[i].scaled_y2);
    }
}

/* Handle string commands that are entered into the shell window.
 */
ParseCmd Example::parseCmd(const string &input, string &msg)
{
    string c;
    ParseCmd ret = COMMAND_PARSED;

    if(parseCompare(input, "Clear")) {
	clear_button->activate();
    }
    else if(parseString(input, "Print", c)) {
	if(print_dialog == NULL) {
	    print_dialog = new PrintDialog("Print Example", this, this);
	}
	ret = print_dialog->parseCmd(c, msg);
    }
    else if(parseCompare(input, "Compute")) {
	compute_button->activate();
    }
    else if(parseCompare(input, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = plot->parseCmd(input, msg)) == COMMAND_NOT_FOUND)
    {
	return FormDialog::parseCmd(input, msg);
    }
    return ret;
}

/* Print string commands.
 */
void Example::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scompute\n", prefix);
}

void Example::printDialog(void)
{
    if(print_dialog == NULL) {
	print_dialog = new PrintDialog("Print Example", this , this);
    }
    print_dialog->setVisible(true);
}

void Example::print(FILE *fp, PrintParam *p)
{
    AxesParm *a = plot->hardCopy(fp, p, NULL);
    Free(a);
}

void Example::compute(void)
{
    int n, npts = 100;
    double dx, x[100];
    float y[100];
    Arg args[2];

    dx = 360./(npts-1);
    for(int i = 0; i < npts; i++) {
	x[i] = i*dx;
	y[i] = sin(x[i]*M_PI/180.);
    }

    plot->clearWaveforms();

    n = 0;
    XtSetArg(args[n], XtNminXLab, 5); n++;
    XtSetArg(args[n], XtNmaxXLab, 5); n++;
    plot->setValues(args, n); // force 5 x-axis labels

    plot->addCurve(npts, x, y, CPLOT_CURVE, NULL, "sin", True,
                        CPLOT_ADJUST_LIMITS, 0);
    for(int i = 0; i < npts; i++) {
	x[i] = i*dx;
	y[i] = cos(x[i]*M_PI/180.);
    }
    plot->addCurve(npts, x, y, CPLOT_CURVE, NULL, "cos", True,
                        CPLOT_ADJUST_LIMITS, 0);
    plot->addHorDoubleLine();
}
