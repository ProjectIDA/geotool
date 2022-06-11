/** \file WaveformColor.cpp
 *  \brief Defines class WaveformColor.
 *  \author Ivan Henson
 */
#include "config.h"
#include "WaveformColor.h"
#include "ColorTable.h"
#include "motif++/MotifClasses.h"

WaveformColor::WaveformColor(const string &name, Component *parent,
	ActionListener *listener) : FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Color Code", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    colors_button = new Button("Colors...", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 6); n++;
    list = new List("list", this, args, n, this);
    const char *items[] = {
	"uniform", "unique", "station", "channel", "network", "origin",
    };
    list->addItems(items, 6);

    color_table = NULL;

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener, XmNactivateCallback);
}

WaveformColor::~WaveformColor(void)
{
}

void WaveformColor::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Colors...")) {
	if( !color_table ) {
	    color_table = new ColorTable("Waveform Color List", this);
	}
	color_table->setVisible(true);
    }
    else if(!strcmp(cmd, "list")) {
	int n;
	char **s=NULL;
	if( (n = list->getSelectedItems(&s)) > 0) {
	    doCallbacks(action_event->getSource()->baseWidget(),(XtPointer)s[0],
				XmNactivateCallback);
	    for(int i = 0; i < n; i++) Free(s[i]);
	    Free(s);
	}
    }
}

ParseCmd WaveformColor::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret = list->parseCmd(cmd, msg);

    if( ret != COMMAND_NOT_FOUND) {
	return ret;
    }
    else if(parseCompare(cmd, "Help")) {
        char prefix[200];
        getParsePrefix(prefix, sizeof(prefix));
        parseHelp(prefix);
	return COMMAND_PARSED;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
}

void WaveformColor::parseHelp(const char *prefix)
{
    vector<string> items;

    list->getItems(items);

    for(int i = 0; i < (int)items.size(); i++) {
	printf("%s%s\n", prefix, items[i].c_str());
    }
}

void WaveformColor::setColor(const string &color)
{
    vector<string> items;

    list->getItems(items);

    for(int i = 0; i < (int)items.size(); i++) {
	if(!color.compare(items[i])) {
	    list->select(color);
	}
    }
}
