/** \file QueryViews.cpp
 *  \brief Defines class QueryViews.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>

extern "C" {
#include "libstring.h"
}
#include "QueryViews.h"
#include "motif++/MotifClasses.h"

QueryViews::QueryViews(const string &name, Component *parent, Menu *vmenu)
		: FormDialog(name, parent)
{
    int n;
    Arg args[20];

    views_menu = vmenu;

    setSize(560, 650);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);
    cancel_button = new Button("Cancel", controls, this);
    default_button = new Button("Set Default", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
    sw = new ScrolledWindow("sw", this, args, n);

    work = new Form("work", sw);

    for(int i = 0; i < MAX_VIEWS; i++) {
	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 5); n++;
	if(i == 0) {
	    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	}
	else {
	    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	    XtSetArg(args[n], XmNtopWidget, query_sw[i-1]->baseWidget()); n++;
	}
	XtSetArg(args[n], XmNtopOffset, 5); n++;
	XtSetArg(args[n], XmNcolumns, 20); n++;
	query_label[i] = new TextField("text", work, args, n);

	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftOffset, 5); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget, query_label[i]->baseWidget()); n++;
	XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
	XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
	query_sw[i] = new ScrolledWindow("sw", work, args, n);

	n = 0;
	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
	XtSetArg(args[n], XmNwordWrap, True); n++;
	XtSetArg(args[n], XmNrows, 5); n++;
	XtSetArg(args[n], XmNcolumns, 60); n++;
	XtSetArg(args[n], XmNscrollHorizontal, False); n++;
	query_text[i] = new TextField("qtext", query_sw[i], args, n);
    }

    init();

    enableCallbackType(XmNactivateCallback);
}

QueryViews::~QueryViews(void)
{
}

void QueryViews::init(void)
{
    int i;
    char name[100];
    string version;

    getProperty("program_release", version);

    if(version.compare(Application::getRelease())) {
	removeProperty("nviews");
	for(i = 0; i < MAX_VIEWS; i++) {
	    snprintf(name, 20, "view%d.name", i+1);
	    removeProperty(name);
	    snprintf(name, 20, "view%d.query", i+1);
	    removeProperty(name);
	}
    }

    num_views = 0;
    for(i = 0; i < MAX_VIEWS; i++) {
	views[i].name = NULL;
	views[i].query = NULL;
	views[i].b = NULL;
    }

    getProperty("nviews", &num_views);

    if(num_views > MAX_VIEWS) num_views = MAX_VIEWS;

    for(i = 0; i < num_views; i++)
    {
	snprintf(name, 20, "view%d.name", i+1);
	if((views[i].name = getProperty(name)) == NULL) {
	    num_views = i;
	    break;
	}
	snprintf(name, 20, "view%d.query", i+1);
	if((views[i].query = getProperty(name)) == NULL) {
	    free(views[i].name);
	    num_views = i;
	    break;
	}
    }
    for(i = 0; i < MAX_VIEWS; i++) {
	snprintf(name, 20, "view %d", i);
	views[i].b = new Button(name, views_menu, comp_parent);
	views[i].b->setVisible(false);
    }
    views_menu->addSeparator("sep");
    edit_views = new Button("Edit Views", views_menu, this);

    if(num_views > 0) {
    	for(i = 0; i < num_views; i++) {
	    query_label[i]->setString(views[i].name);
	    query_text[i]->setString(views[i].query);
	}
    	for(i = num_views; i < MAX_VIEWS; i++) {
	    query_label[i]->setString("");
	    query_text[i]->setString("");
	}
    }
    else {
	setDefaultViews();
    }
}

void QueryViews::setDefaultViews(void)
{
    for(int i = 0; i < num_views; i++) {
	Free(views[i].name);
	Free(views[i].query);
    }
    num_views = 3;
    views[0].name = strdup("3c_sel_v");
    views[0].query = strdup(
"select * from wfdisc w, staconf s where time between origin_time - 14400 \
and origin_time + 3600 and s.statype = 's-3c' and s.threecsite = w.sta and \
s.threecband = substr(w.chan, 1, 1) and \
upper(substr(w.chan, length(w.chan), 1)) in ('Z','N','E')");
    views[1].name = strdup("hydro_sel_v");
    views[1].query = strdup(
"select * from wfdisc w, staconf s where time between origin_time - 14400 \
and origin_time + 3600 and s.statype = 'h-hydro' and s.refsite = w.sta and \
s.refchan= w.chan union select * from wfdisc w, staconf s where time between \
origin_time - 14400 and origin_time + 3600 and s.statype = 'h-tphase' and \
s.refsite = w.sta and upper(substr(w.chan, length(w.chan), 1)) \
in ('Z', 'N', 'E')");
    views[2].name = strdup("infra_sel_v");
    views[2].query = strdup(
"select * from wfdisc w, staconf s where time between origin_time - 14400 \
and origin_time + 3600 and s.statype = 'i-array' and s.refsite = w.sta and \
s.refchan= w.chan");
}

void QueryViews::saveViews(void)
{
    char name[100];

    snprintf(name, 20, "%d", num_views);
    putProperty("nviews", name);

    for(int i = 0; i < num_views; i++) {
        snprintf(name, 20, "view%d.name", i+1);
        putProperty(name, views[i].name);
        snprintf(name, 20, "view%d.query", i+1);
        putProperty(name, views[i].query);

	query_label[i]->setString(views[i].name);
	query_text[i]->setString(views[i].query);

	views[i].b->setLabel(views[i].name);
	views[i].b->setVisible(true);
    }
    for(int i = num_views; i < MAX_VIEWS; i++) {
	query_label[i]->setString("");
	query_text[i]->setString("");
	views[i].b->setVisible(false);
    }
}

void QueryViews::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Apply")) {
	setVisible(false);
	apply();
	doCallbacks(action_event->getSource()->baseWidget(), NULL,
			XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Cancel")) {
	setVisible(false);
	cancel();
    }
    else if(!strcmp(cmd, "Set Default")) {
	setDefaultViews();
	setVisible(false);
	doCallbacks(action_event->getSource()->baseWidget(), NULL,
			XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Edit Views")) {
	setVisible(true);
    }
}

void QueryViews::apply(void)
{
    int n = 0;
    for(int i = 0; i < MAX_VIEWS; i++)
    {
	char *name=NULL, *query=NULL;
	if( (name = query_label[i]->getString()) && strlen(name) > 0 &&
	    (query = query_text[i]->getString()) && strlen(query) > 0 )
	{
	    Free(views[n].name);
	    Free(views[n].query);
	    views[n].name = name;
	    views[n].query = query;
	    n++;
	}
	else {
	    Free(name);
	    Free(query);
	}
    }
    num_views = n;

    saveViews();
}

void QueryViews::cancel(void)
{
    for(int i = 0; i < num_views; i++) {
	query_label[i]->setString(views[i].name);
	query_text[i]->setString(views[i].query);
    }
    for(int i = num_views; i < MAX_VIEWS; i++) {
	query_label[i]->setString("");
	query_text[i]->setString("");
    }
}
