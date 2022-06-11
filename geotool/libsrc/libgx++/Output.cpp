/** \file Output.cpp
 *  \brief Defines class Output.
 *  \author Ivan Henson
 */
#include "config.h"
#include "Output.h"
#include "motif++/MotifClasses.h"
#include "CssFileDialog.h"
#include "AscSource.h"
#include "libgio.h"

extern "C" {
#include "libstring.h"
}


Output::Output(const string &name, Component *parent, WaveformPlot *wave_plot)
		: FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    wp = wave_plot;

    wp->addDataListener(this); // listen for waveform selection

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    label = new Label("Save Selected Waveforms as...", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    tables_toggle =new Toggle("Output Tables", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, tables_toggle->baseWidget()); n++;
    rc = new RowColumn("rc", this, args, n);

    prefix_button = new Button("Choose Path/Prefix", rc, this);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    prefix_text = new TextField("prefix_text", rc, this, args, n);

    new Space("space", rc, XmVERTICAL, 10);

    remark_label = new Label("Remark", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    remark_text = new TextField("remark_text", rc, args, n);

    new Space("space", rc, XmVERTICAL, 10);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc2 = new RowColumn("rc2", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNspacing, 30); n++;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    rb1 = new RadioBox("rb1", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    append_toggle = new Toggle("Append", rb1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    overwrite_toggle = new Toggle("Overwrite", rb1, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    rb2 = new RadioBox("rb2", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    raw_toggle = new Toggle("Raw", rb2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    as_displayed_toggle = new Toggle("As Displayed", rb2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    rb3 = new RadioBox("rb3", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    all_toggle = new Toggle("All Waveforms", rb3, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    selected_toggle = new Toggle("Selected Waveforms", rb3, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNborderWidth, 1); n++;
    rb4 = new RadioBox("rb4", rc2, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    css_toggle = new Toggle("CSS", rb4, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    sac_toggle = new Toggle("SAC", rb4, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    ascii_toggle = new Toggle("ASCII", rb4, this, args, n);

    open_prefix_file = NULL;
}

Output::~Output(void)
{
}

void Output::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
    }
    else if(!strcmp(cmd, "Choose Path/Prefix")) {
	chooseFile();
    }
    else if(action_event->getSource() == prefix_text ||
	action_event->getSource() == wp ||
	!strcmp(cmd, "All Waveforms") || !strcmp(cmd, "Selected Waveforms"))
    {
	apply_button->setSensitive(!prefix_text->empty() && 
		((all_toggle->state() && wp->numWaveforms()) ||
		 (selected_toggle && wp->numSelected())) );
    }
    else if(!strcmp(cmd, "CSS") || !strcmp(cmd, "SAC") || !strcmp(cmd, "ASCII"))
    {
	if(css_toggle->state()) {
	    tables_toggle->setSensitive(true);
	}
	else {
	    tables_toggle->set(false, false);
	    tables_toggle->setSensitive(false);
	}
    }
}

ParseCmd Output::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseCompare(cmd, "append")) {
	append_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "overwrite")) {
	overwrite_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "raw")) {
	raw_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "as_Displayed")) {
	as_displayed_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "all_waveforms")) {
	all_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "selected_waveforms")) {
	selected_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "CSS")) {
	css_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "SAC")) {
	sac_toggle->set(true, true);
    }
    else if(parseCompare(cmd, "ASCII")) {
	ascii_toggle->set(true, true);
    }
    else if(parseArg(cmd, "remark", c)) {
	remark_text->setString(c, true);
    }
    else if(parseArg(cmd, "path", c) || parseArg(cmd, "prefix", c)) {
	char *full_path = Application::fullPath(c);
	prefix_text->setString(full_path, true);
	Free(full_path);
    }
    else if(parseCompare(cmd, "apply")) {
	apply();
    }
    else if(parseCompare(cmd, "close")) {
	setVisible(false);
    }
    else if(parseCompare(cmd, "help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void Output::parseHelp(const char *prefix)
{
    printf("%sappend\n", prefix);
    printf("%soverwrite\n", prefix);
    printf("%sprefix=FILEPATH\n", prefix);
    printf("%sraw\n", prefix);
    printf("%sall_waveforms\n", prefix);
    printf("%sselected_waveforms\n", prefix);
    printf("%sas_displayed\n", prefix);
    printf("%scss\n", prefix);
    printf("%ssac\n", prefix);
    printf("%sascii\n", prefix);
    printf("%spath=PATH\n", prefix);
    printf("%sremark=REMARK\n", prefix);
    printf("%sclose\n", prefix);
    printf("%sapply\n", prefix);
}

void Output::chooseFile(void)
{
    char *file;

    if(!open_prefix_file) {
	open_prefix_file = new CssFileDialog("Choose Prefix", this, FILE_ONLY,
				"./", (char *)"All Tables", "Choose Prefix");
    }
    open_prefix_file->setVisible(true);

    if((file = open_prefix_file->getFile()) != NULL)
    {
	// get prefix
	for(int i = (int)strlen(file)-1; i >= 0 && file[i] != '/'; i--) {
	    if(file[i] == '.' && CssTableClass::isTableName(file+i+1)) {
		file[i] = '\0';
		break;
	    }
	}
	prefix_text->setString(file, true);
	prefix_text->showPosition((int)strlen(file));
	Free(file);
    }
}

void Output::apply(void)
{
    gvector<Waveform *> wvec;
    int raw;
    char *prefix, *remark, wa[2];

    if(all_toggle->state()) {
	if(wp->getWaveforms(wvec) <= 0) {
	    showWarning("No Waveforms.");
	    return;
	}
    }
    else if(wp->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No Waveforms selected.");
	return;
    }

    prefix = prefix_text->getString();
    for(int i = (int)strlen(prefix)-1; i >= 0 && prefix[i] != '/'; i--) {
	if(prefix[i] == '.' && CssTableClass::isTableName(prefix+i+1)) {
	    prefix[i] = '\0';
	    break;
	}
    }
    if( prefix[0] == '\0' ) {
	return;
    }

    remark = remark_text->getString();

    wa[0] = append_toggle->state() ? 'a' : 'w';
    wa[1] = '\0';

    raw = raw_toggle->state() ? 1 : 0;

    setCursor("hourglass");

    if(css_toggle->state()) {
	cssWriteCDC(prefix, wa, wvec, remark, raw);
	if(tables_toggle->state()) {
	    wp->outputTables(prefix, wa);
	}
    }
    else if(sac_toggle->state()) {
	CssOriginClass **origins = new CssOriginClass * [wvec.size()];
	for(int i = 0; i < wvec.size(); i++) {
	    origins[i] = wp->getPrimaryOrigin(wvec[i]);
	}
	sacWriteCDC(prefix, wa, wvec, remark, raw, origins);
    }
    else if(ascii_toggle->state()) {
	GError::getMessage();
	AscSource::output(prefix, wa, wvec, remark, raw);
	char *c = GError::getMessage();
	if(c) {
	    showWarning(c);
	}
    }
    Free(prefix);
    Free(remark);

    setCursor("default");
}
