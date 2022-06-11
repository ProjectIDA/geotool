/** \file Working.cpp
 *  \brief Defines class Working.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "Working.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

Working::Working(const string &name, Component *parent, int num,
		const char **labels) : FormDialog(name, parent, false, false)
{
    createInterface("Retrieving records", num, labels);
    enableCallbackType(XmNactivateCallback);
}

Working::Working(const string &name, Component *parent, int num,
		const char **labels, ActionListener *listener)
		: FormDialog(name, parent, false, false)
{
    createInterface("Retrieving records", num, labels);
    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

Working::Working(const string &name, Component *parent, const string &title,
		const string &update_message) :
		FormDialog(name, parent, false, false)
{
    update_msg = update_message;
    createInterface(title, 0, NULL);
    enableCallbackType(XmNactivateCallback);
}

Working::Working(const string &name, Component *parent, const string &title,
		const string &update_message, ActionListener *listener) :
		FormDialog(name, parent, false, false)
{
    update_msg = update_message;
    createInterface(title, 0, NULL);
    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void Working::createInterface(const string &title, int num, const char **labels)
{
    char name[100], type[50];
    int i, n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    label = new Label(title, this, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    stop = new Button("Stop", this, args, 4, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, stop->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);
    
    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++; 
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 2); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    rc = new RowColumn("rc", this, args, n);
        
    num_labels = num;
    if(num) label_comps = (Label **)mallocWarn(num*sizeof(Label *));
    else label_comps = NULL;
    for(i = 0; i < num; i++) {
        snprintf(type, sizeof(type), "%s records:", labels[i]);
	label_comps[i] = new Label(type, rc);
	label_comps[i]->setSensitive(false);
    }

    if(num) result_comps = (Label **)mallocWarn(num*sizeof(Label *));
    else result_comps = NULL;
    for(i = 0; i < num; i++) {
        snprintf(name, sizeof(name), "%s_result", labels[i]);
        result_comps[i] = new Label(name, rc);
        result_comps[i]->setSensitive(false);
    }
    if(!num) {
	result_comps = (Label **)mallocWarn(sizeof(Label *));
	snprintf(name, sizeof(name), "    0 %s", update_msg.c_str());
	result_comps[0] = new Label(name, rc);
    }
//    XtManageChild(base_widget);

    working = true;

    XFlush(XtDisplay(base_widget));
    XtAppContext app = Application::getApplication()->appContext();
    XEvent event;
    int check = 0;
    while((XtAppPending(app) & XtIMXEvent) && ++check < 1000)
    {   
	XtAppNextEvent(app, &event);
	XtDispatchEvent(&event);
    }
    XFlush(XtDisplay(base_widget));
}

Working::~Working(void)
{
    Free(label_comps);
    Free(result_comps);
}

void Working::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Stop")) {
	working = false;
	doCallbacks(comp->baseWidget(), NULL,(const char *)XmNactivateCallback);
    }
}

bool Working::update(const string &tableName, int num_records)
{   
    if(!working) return false;
    XtAppContext app = Application::getApplication()->appContext();

    int i;
    for(i=0; i < num_labels; i++) {
	char type[50];
        snprintf(type, sizeof(type), "%s records:", tableName.c_str());
	if(!strcmp(type, label_comps[i]->getName())) break;
    }
	
    if(i < num_labels)
    {
	XEvent event;
	char name[50];
    
	label_comps[i]->setSensitive(true);
	result_comps[i]->setSensitive(true);
        
        snprintf(name, sizeof(name), "%6d records retrieved", num_records);
	result_comps[i]->setLabel(name);
        
        XFlush(XtDisplay(base_widget));
	int check = 0;
        while((XtAppPending(app) & XtIMXEvent) && ++check < 1000)
        {   
	    XtAppNextEvent(app, &event);
            XtDispatchEvent(&event);
        }
        XFlush(XtDisplay(base_widget));
        return true;
    }
    return false;
}

bool Working::update(int num_records)
{   
    if(!working) return false;
    XtAppContext app = Application::getApplication()->appContext();
    XEvent event;
    char name[1000];
    
    snprintf(name, sizeof(name), "%6d %s", num_records, update_msg.c_str());
    result_comps[0]->setLabel(name);
        
    XFlush(XtDisplay(base_widget));
    int check = 0;
    while((XtAppPending(app)&XtIMXEvent) && ++check < 1000)
    {   
	XtAppNextEvent(app, &event);
	XtDispatchEvent(&event);
    }
    XFlush(XtDisplay(base_widget));
    return true;
}
