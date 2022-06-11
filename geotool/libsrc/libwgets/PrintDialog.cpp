/** \file PrintDialog.cpp
 *  \brief Defines class PrintDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sstream>
#include <sys/param.h>
#include <errno.h>

#include "widget/PrintDialog.h"
#include "PrintParam.h"
#include "PrintClient.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}


PrintDialog::PrintDialog(const char *name, Component *parent,
		PrintClient *comp) : FormDialog(name, parent, false, false)
{
    Arg args[20];
    int n;
    char ps_name[200];

    print_comp = comp;
    print_options = NULL;

    setResource("*%s*marginHeight: 1", name);
    setResource("*%s*highlightThickness: 0", name);
    setResource("*%s*traversalOn: False", name);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNtraversalOn, False); n++;
    rc1 = new RowColumn("rc1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    filename_label = new Label("Filename", rc1, args, n);

    snprintf(ps_name, sizeof(ps_name), "%s.ps",
		Application::getApplication()->applicationClass());

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    XtSetArg(args[n], XmNvalue, ps_name); n++;
    filename_text = new TextField("filename", rc1, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    command_label = new Label("Command", rc1, args, n);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    XtSetArg(args[n], XmNvalue, "lp"); n++;
    command_text = new TextField("command", rc1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    print_button = new Button("Print", controls, this);
    cancel_button = new Button("Cancel", controls, this);
    options = new Button("Options", controls, this);
//    Button *b = new Button("Help", controls, this);
//    controls->setHelp(b);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    rc2 = new RowColumn("rc2", this, args, n);


    n = 0;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    layout_choice = new Choice("Layout", rc2, this, args, n);
    layout_choice->addItem("Landscape");
    layout_choice->addItem("Portrait");

    n = 0;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    paper_choice = new Choice("Paper", rc2, this, args, n);
    paper_choice->addItem("Letter");
    paper_choice->addItem("Tabloid");
    paper_choice->addItem("Ledger");
    paper_choice->addItem("Legal");
    paper_choice->addItem("Executive");
    paper_choice->addItem("A3");
    paper_choice->addItem("A4");
    paper_choice->addItem("A5");
    paper_choice->addItem("B4");
    paper_choice->addItem("B5");
    paper_choice->setChoice("A4");

    units_choice = new Choice("Units", rc2, this, args, 1);
    units_choice->addItem("inches");
    units_choice->addItem("mm");
    units_choice->addItem("points");
    units_choice->setChoice("mm");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightOffset, 4); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    rc3 = new RowColumn("rc3", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    left_label = new Label("left", rc3, args, n);
    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    right_label = new Label("right", rc3, args, n);
    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    width_label = new Label("width", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    left_text = new TextField("left_text", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    right_text = new TextField("right_text", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    width_text = new TextField("width_text", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    top_label = new Label("top", rc3, args, n);
    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    bottom_label = new Label("bottom", rc3, args, n);
    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    height_label = new Label("height", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    top_text = new TextField("top_text", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    bottom_text = new TextField("bottom", rc3, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 6); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    height_text = new TextField("height_text", rc3, args, n);

    printInit();

    print_options = new PrintOptions("Print Options", this);
}

PrintDialog::~PrintDialog(void)
{
}

void PrintDialog::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Cancel")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Print")) {
	if(print()) setVisible(false);
    }
    else if(!strcmp(cmd, "Options")) {
	print_options->setVisible(true);
    }
    else if(!strcmp(cmd, "Layout") ||!strcmp(cmd, "Units") ||
		!strcmp(cmd, "Paper"))
    {
	printInit();
    }
}

ParseCmd PrintDialog::parseCmd(const string &cmd, string &msg)
{
    string c;
    bool err;
    int num;
    ParseCmd ret;
    ParseArg args[] = { {"layout", false, false}, {"paper", false, false},
	{"units", false, false}, {"filename", false, false},
	{"command", false, false}, {"left", false, false},
	{"right", false, false}, {"top", false, false},
	{"bottom", false, false}, {"width", false, false},
	{"height", false, false}, {"ink", false, false},
	{"arrival_placement", false, false}, {"line_width", false, false},
	{"waveform_scale", false, false}, {"tag_placement", false, false},
	{"axes_labels_font", false, false}, {"axes_labels_size", false, false},
	{"axes_labels_type", false, false}, {"tick_labels_font", false, false},
	{"tick_labels_size", false, false}, {"tick_labels_type", false, false},
	{"tag_labels_font", false, false}, {"tag_labels_size", false, false},
	{"tag_labels_type", false, false}, {"arrival_labels_font", false,false},
	{"arrival_labels_size", false, false},
	{"arrival_labels_type", false, false},{"main_titLe_font", false, false},
	{"main_title_size", false, false}, {"main_title_type", false, false}
    };

    num = (int)sizeof(args)/(int)sizeof(ParseArg);
    if(parseCheckArgs(cmd, "set", num, args, msg, &err))
    {
	if(err) return ARGUMENT_ERROR;
	if(!parseArgFound(num, args)) {
	    msg.assign("print_window.set: missing argument");
	    return ARGUMENT_ERROR;
	}
	if(parseGetArg(cmd, "layout", c)) {
	    layout_choice->setChoice(c, true, true);
	}
	if(parseGetArg(cmd, "paper", c)) {
	    paper_choice->setChoice(c, true, true);
	}
	if(parseGetArg(cmd, "units", c)) {
	    units_choice->setChoice(c, true, true);
	}
	if(parseGetArg(cmd, "filename", c)) {
	    string path;
	    Application::getApplication()->getFullPath(c, path);
	    filename_text->setString(path);
	}
	if(parseGetArg(cmd, "command", c)) {
	    command_text->setString(c);
	}
	if(parseGetArg(cmd, "left", c)) {
	    left_text->setString(c);
	}
	if(parseGetArg(cmd, "right", c)) {
	    right_text->setString(c);
	}
	if(parseGetArg(cmd, "top", c)) {
	    top_text->setString(c);
	}
	if(parseGetArg(cmd, "bottom", c)) {
	    bottom_text->setString(c);
	}
	if(parseGetArg(cmd, "width", c)) {
	    width_text->setString(c);
	}
	if(parseGetArg(cmd, "height", c)) {
	    height_text->setString(c);
	}
	ret = print_options->parseCmd(cmd, msg);
	if(ret == ARGUMENT_ERROR) return ret;
    }
    else if(parseCompare(cmd, "print")) {
	if(print()) setVisible(false);
    }
    else if(parseCompare(cmd, "cancel")) {
	setVisible(false);
    }
    else if(parseString(cmd, "options", c)) {
	return print_options->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char *s, prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	// remove the name after print (ie print waveforms, print map, etc).
	// so the command is also something.print
	if( (s = strstr(prefix, "print ")) ) {
	    s[5] = '.';
	    s[6] = '\0';
	}
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar PrintDialog::parseVar(const string &name, string &value)
{
    string c;

    if(parseCompare(name, "layout")) {
	value.assign(layout_choice->getChoice());
    }
    else if(parseCompare(name, "paper")) {
	value.assign(paper_choice->getChoice());
    }
    else if(parseCompare(name, "units")) {
	value.assign(units_choice->getChoice());
    }
    else if(parseCompare(name, "command")) {
	command_text->getString(value);
    }
    else if(parseCompare(name, "left")) {
	left_text->getString(value);
    }
    else if(parseCompare(name, "right")) {
	right_text->getString(value);
    }
    else if(parseCompare(name, "top")) {
	top_text->getString(value);
    }
    else if(parseCompare(name, "bottom")) {
	bottom_text->getString(value);
    }
    else if(parseCompare(name, "width")) {
	width_text->getString(value);
    }
    else if(parseCompare(name, "height")) {
	height_text->getString(value);
    }
    else if(parseString(name, "options", c)) {
	return print_options->parseVar(name, value);
    }
    else {
	return FormDialog::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void PrintDialog::parseHelp(const char *prefix)
{
    printf("%slayout=(portrait,landscape)\n", prefix);
    printf("%spaper=(letter,tabloid,ledger,legal,executive,a3,a4,a5,b4,b5)\n",
			prefix);
    printf("%sunits=(inches,mm,points)\n", prefix);
    printf("%sfilename=FILENAME\n", prefix);
    printf("%scommand=COMMAND\n", prefix);
    printf("%sleft=NUMBER\n", prefix);
    printf("%sright=NUMBER\n", prefix);
    printf("%stop=NUMBER\n", prefix);
    printf("%sbottom=NUMBER\n", prefix);
    printf("%swidth=NUMBER\n", prefix);
    printf("%sheight=NUMBER\n", prefix);
    printf("%scancel\n", prefix);
    printf("%sprint\n", prefix);
    printf("%soptions.help\n", prefix);
}

void PrintDialog::printInit(void)
{
    static int npapers = 10;
    static struct {
	const char *type;
	struct {
	    double width;
	    double height;
	} units[3];
    } papers[] =
	    {		 /* inches	 millimeters	  points */
	{"Letter",	{{8.5, 11.0},	{215.9, 279.4},	{612., 792.}} },
	{"Tabloid",	{{11., 17.0},	{279.4, 431.8},	{792., 1224.}} },
	{"Ledger",	{{17., 11.0},	{431.8, 279.4},	{1224., 792.}} },
	{"Legal", 	{{8.5, 14.0},	{215.9, 355.6},	{612., 1008.}} },
	{"Executive",{{7.5, 10.0},	{190.5, 254.0},	{540., 720.}} },
	{"A3",	{{11.69, 16.53},{297.0, 419.8},	{842., 1190.}} },
	{"A4",	{{8.26, 11.69},	{209.9, 297.0},	{595., 842.}} },
	{"A5",	{{5.83, 8.26},	{148.2, 209.9},	{420., 595.}} },
	{"B4",	{{9.83, 13.93},	{249.8, 353.8},	{708., 1003.}} },
	{"B5",	{{7.17, 10.12},	{182.0, 257.2},	{516., 729.}} },
    };
    static double horizontal_margin = .5, vertical_margin = .5;
    const char *print_layout, *print_paper, *print_units;
    int i;
    double left=0., right=0., bottom=0., top=0., width=0., height=0.;

    print_layout = layout_choice->getChoice();
    print_paper = paper_choice->getChoice();
    print_units = units_choice->getChoice();

    for(i = 0; i < npapers; i++)
    {
	if(!strcmp(print_paper, papers[i].type))
	{
	    width = papers[i].units[0].width;
	    height = papers[i].units[0].height;
	    left = horizontal_margin;
	    right = width - horizontal_margin;
	    bottom = vertical_margin;
	    top = height - vertical_margin;
	    break;
	}
    }
    if(i == npapers) {
	showWarning("Error: Unknown paper type: %s\n", print_paper);
	return;
    }
    if(strcmp(print_units, "inches"))	/* not equal to inches */
    {
	if(!strcmp(print_units, "mm"))
	{
	    double inches_to_mm = 215.9/8.5;
	    left = (int)(left*inches_to_mm + .5);
	    right = (int)(right*inches_to_mm + .5);
	    bottom = (int)(bottom*inches_to_mm + .5);
	    top = (int)(top*inches_to_mm + .5);
	    width = papers[i].units[1].width;
	    height = papers[i].units[1].height;
	}
	else if(!strcmp(print_units, "points"))
	{
	    double inches_to_points = 72.;
	    left = (int)(left*inches_to_points + .5);
	    right = (int)(right*inches_to_points + .5);
	    bottom = (int)(bottom*inches_to_points + .5);
	    top = (int)(top*inches_to_points + .5);
	    width = papers[i].units[2].width;
	    height = papers[i].units[2].height;
	}
	else {
	    showWarning("Error: Unknown paper units: %s\n", print_units);
	    return;
	}
    }
    if(!strcmp(print_layout, "Landscape"))
    {
	double x;
	x = left;
	left = bottom;
	bottom = x;
	x = right;
	right = top;
	top = x;
	x = width;
	width = height;
	height = x;
    }
    if(!strcmp(print_units, "points"))
    {
	char buf[50];
	snprintf(buf, sizeof(buf), "%d", (int)(left+.5));
	left_text->setString(buf);
	snprintf(buf, sizeof(buf), "%d", (int)(right+.5));
	right_text->setString(buf);
	snprintf(buf, sizeof(buf), "%d", (int)(bottom+.5));
	bottom_text->setString(buf);
	snprintf(buf, sizeof(buf), "%d", (int)(top+.5));
	top_text->setString(buf);
	snprintf(buf, sizeof(buf), "%d", (int)(width+.5));
	width_text->setString(buf);
	snprintf(buf, sizeof(buf), "%d", (int)(height+.5));
	height_text->setString(buf);
    }
    else {
	char buf[50];
	snprintf(buf, sizeof(buf), "%.2f", left);
	left_text->setString(buf);
	snprintf(buf, sizeof(buf), "%.2f", right);
	right_text->setString(buf);
	snprintf(buf, sizeof(buf), "%.2f", bottom);
	bottom_text->setString(buf);
	snprintf(buf, sizeof(buf), "%.2f", top);
	top_text->setString(buf);
	snprintf(buf, sizeof(buf), "%.2f", width);
	width_text->setString(buf);
	snprintf(buf, sizeof(buf), "%.2f", height);
	height_text->setString(buf);
    }
}

bool PrintDialog::print(void)
{
    PrintParam p;
    char *file, *cmd;
    const char *print_units;
    char sys[1024];
    FILE *fp;

    p.top_title_user = false;
    p.top_title = NULL;
    p.x_axis_label = NULL;
    p.y_axis_label = NULL;

    p.portrait = !strcasecmp(layout_choice->getChoice(), "Portrait") ?
			True : False;
    p.center = !p.portrait;

    print_units = units_choice->getChoice();

    if(!left_text->getDouble(&p.left)) {
	showWarning("Print: invalid left field.");
	return false;
    }
    if(!right_text->getDouble(&p.right)) {
	showWarning("Print: invalid right field.");
	return false;
    }
    if(!top_text->getDouble(&p.top)) {
	showWarning("Print: invalid top field.");
	return false;
    }
    if(!bottom_text->getDouble(&p.bottom)) {
	showWarning("Print: invalid bottom field.");
	return false;
    }

    if(!strcmp(print_units, "mm")) {
	double mm_to_inches = 8.5/215.9;
	p.left *= mm_to_inches;
	p.right *= mm_to_inches;
	p.bottom *= mm_to_inches;
	p.top *= mm_to_inches;
    }
    else if(!strcmp(print_units, "points")) {
	double points_to_inches = 1./72.;
	p.left *= points_to_inches;
	p.right *= points_to_inches;
	p.bottom *= points_to_inches;
	p.top *= points_to_inches;
    }

    strcpy(p.fonts.title_font, "Helvetica");
    strcpy(p.fonts.label_font, "Helvetica");
    strcpy(p.fonts.axis_font, "Helvetica");
    strcpy(p.fonts.tag_font, "Helvetica");
    strcpy(p.fonts.arr_font, "Helvetica");
    p.fonts.title_fontsize = 16;
    p.fonts.label_fontsize = 12;
    p.fonts.axis_fontsize = 12;
    p.fonts.tag_fontsize = 12;
    p.fonts.arr_fontsize = 12;
    strcpy(p.mainFontStyle, "Plain");
    strcpy(p.labelsFontStyle, "Plain");
    strcpy(p.axesFontStyle, "Plain");
    strcpy(p.tagFontStyle, "Plain");
    strcpy(p.arrFontStyle, "Plain");
    p.color = False;
    p.arrPlace = True;
    p.tagPlace = True;
    p.full_scale = True;
    p.data_linewidth = 2;

    if(print_options) {
	print_options->fillPrintStruct(&p);
    }

    file = filename_text->getString();
    if((fp = open_ps(file, p.color, p.portrait)) == NULL) return false;

    setCursor("hourglass");

    print_comp->print(fp, &p);

    fprintf(fp, "1 slw\n");
    if(p.color) fprintf(fp, "grestore\n");
    fprintf(fp, "showpage\n");
    fclose(fp);

    cmd  = command_text->getString();
    if(strlen(stringTrim(cmd)) > 0) {
	strcpy(sys, cmd);
	strcat(sys, " ");
	strcat(sys, file);
	system(sys);
    }
    XtFree(file);
    XtFree(cmd);
    setCursor("default");

    return true;
}

FILE * PrintDialog::open_ps(char *filename, bool do_color, bool do_portrait)
{
    FILE *fp;

    if((fp = fopen(filename, "w")) == NULL)
    {
	if(errno > 0) {
	    showWarning("Cannot open %s\n%s", filename, strerror(errno));
	}
	else {
	    showWarning("Cannot open %s", filename);
	}
	    return NULL;
    }
    /* Output ps prologue.
     */
    fprintf(fp, "%%!\n");
    fprintf(fp, "/S { currentpoint stroke moveto } def\n");
    fprintf(fp, "/M { moveto } def\n");
    fprintf(fp, "/d { lineto } def\n");
    fprintf(fp, "/c { copy } def\n");
    fprintf(fp, "/I { index } def\n");
    fprintf(fp, "/L { lineto } def\n");
    fprintf(fp, "/D { lineto currentpoint stroke moveto } def\n");
    fprintf(fp, "/m { rmoveto } def\n");
    fprintf(fp, "/l { rlineto } def\n");
    fprintf(fp, "/s { scale } def\n");
    fprintf(fp,"/PR { 1 index dup scale dup stringwidth pop neg 0 rmoveto");
    fprintf(fp, " dup false charpath pathbbox exch pop 1 index sub 2 div");
    fprintf(fp, " sub newpath moveto show 1. exch div dup scale } def\n");
    fprintf(fp,"/PL { 1 index dup scale dup stringwidth pop dup div 0 rmoveto");
    fprintf(fp, " dup false charpath pathbbox exch pop 1 index sub 2 div");
    fprintf(fp, " sub newpath moveto show 1. exch div dup scale } def\n");

    fprintf(fp, "/PC { dup dup scale exch dup stringwidth pop 2 div neg");
    fprintf(fp, " 0 rmoveto show 1 exch div dup scale } def\n");
    fprintf(fp,"/P { dup dup scale exch show 1 exch div dup scale } def\n");
    fprintf(fp, "/C { closepath } def\n");
    fprintf(fp, "/N { newpath } def\n");
    fprintf(fp, "/r { rotate } def\n");
    fprintf(fp, "/slw { setlinewidth } def\n");
    fprintf(fp, "%%%%EndProlog\n");

    fprintf(fp, "1 setlinecap\n");
    fprintf(fp, "%.5f %.5f scale N 0 slw [ ] 0 setdash\n",
			POINTS_PER_DOT, POINTS_PER_DOT);

    if(do_color) fprintf(fp, "gsave\n");

    if(!do_portrait)
    {
	int i = (int)(-8.5*DOTS_PER_INCH);
	fprintf(fp, "90 rotate 0 %d translate\n", i);
    }
    return fp;
}

#ifdef ZZZ
static void
get_print_option(Widget widget, char *str, char *text_wid, char *om_wid)
{
    Arg args[1];
    Widget	wid, w;

    strcpy(str, "");
    if((wid = widgetId(text_wid)) == NULL) {
	showWarning(widget, "get_print_option: can't find %s", text_wid);
	return;
    }
    ui_getValue(wid, str);

    if ((int) strlen(str) < 1)
    {
	if((wid = widgetId(om_wid)) == NULL) {
	    showWarning(widget, "get_print_option: can't find %s", om_wid);
	    return;
	}
	w = NULL;
	XtSetArg(args[0], XmNmenuHistory, &w);
	XtGetValues(wid, args, 1);
	if(w == NULL) {
	    showWarning(widget,
			"get_print_option: %s XmNmenuHistory = NULL.", om_wid);
	    return;
	}
	strcpy(str, w->core.name);
    }
}

static int
is_number(char *msg)
{
    int i, n;

    n = (int) strlen(msg);
    for (i=0; i<n; i++)
	if (!isdigit((int)msg[i]))
    {
	break;
    }
	
    return ( i < n ? 0 : 1);
}


void
AddFontStyle(char *font, char *fontStyle)
{
    if(!strcmp(fontStyle, "Plain"))
    {
	if(!strcmp(font, "Times")) strcpy(font, "Times-Roman");
    }
    else if(!strcmp(fontStyle, "Bold"))
    {
	strcat(font, "-Bold");
    }
    else if(!strcmp(fontStyle, "Italic") && !strcmp(font, "Times"))
    {
	strcat(font, "-Italic");
    }
    else if(!strcmp(fontStyle, "Italic"))
    {
	strcat(font, "-Oblique");
    }
}
#endif
