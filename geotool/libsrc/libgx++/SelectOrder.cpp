/** \file SelectOrder.cpp
 *  \brief Defines class SelectOrderravelTimes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "SelectOrder.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

SelectOrder::SelectOrder(const string &name, Component *parent, int num_names,
                const char **select_names, int num_selected, int *selected,
		int width, int height) : FormDialog(name, parent, false, false)
{
    createInterface(num_names, select_names, num_selected, selected,
		width, height);
    close_when_apply = true;
    enableCallbackType(XmNactivateCallback);
}

void SelectOrder::createInterface(int num_names, const char **select_names,
		int num_selected, int *selected, int width, int height)
{
    Arg args[10];
    int i, n;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNwidth, width); n++;
    XtSetArg(args[n], XmNheight, height); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("sw", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    contents = new RowColumn("contents", sw, args, n);

    labels = (Label **)mallocWarn(num_names*sizeof(Label *));
    order = (int *)mallocWarn(num_names*sizeof(int));
    toggles = (Toggle **)mallocWarn(num_names*sizeof(Toggle *));
    names = (char **)mallocWarn(num_names*sizeof(char *));
    orderedNames = (char **)mallocWarn(num_names*sizeof(char *));
    rco = (RowColumn **)mallocWarn(num_names*sizeof(RowColumn *));
    num = num_names;

    for(i = 0; i < num; i++) order[i] = -1;
    n = 0;
    for(i = 0; i < num_selected; i++) {
	if(selected[i] >= 0 && selected[i] < num) {
	    order[selected[i]] = n++;
	}
    }

    for(i = 0; i < num; i++)
    {
	char label[20];
	bool set = false;

	n = 0;
	XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	rco[i] = new RowColumn("rco", contents, args, n);

	if(order[i] < 0 ) {
	    strcpy(label, "");
	}
	else if(order[i] < num ) {
	    snprintf(label, sizeof(label), "%d", order[i]+1);
	    set = true;
	}
	else {
	    cerr << "SelectOrder: order[" << i <<"] = " << order[i]
			<< " > num_names (" << num << ")" << endl;
	    strcpy(label, "");
	}
	n = 0;
	XtSetArg(args[n], XmNwidth, 15); n++;
	XtSetArg(args[n], XmNrecomputeSize, false); n++;
	labels[i] = new Label(label, rco[i], args, n);

	toggles[i] = new Toggle(select_names[i], rco[i], this);
	toggles[i]->set(set, false);
	names[i] = strdup(select_names[i]);
    }

    new Separator("sep1", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls1 = new RowColumn("controls1", rc, args, n);

    deselect_all = new Button("Deselect All", controls1, this);

    new Separator("sep2", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls2 = new RowColumn("controls2", rc, args, n);

    cancel = new Button("Cancel", controls2, this);
    apply = new Button("Apply", controls2, this);
}

void SelectOrder::resetNames(int num_names, const char **select_names,
			int num_selected, int *selected)
{
    int i, n;
    Arg args[10];

    for(i = 0; i < num; i++) Free(names[i]);

    if(num_names > num) {
	labels = (Label **)reallocWarn(labels, num_names*sizeof(Label *));
	order = (int *)reallocWarn(order, num_names*sizeof(int));
	toggles = (Toggle **)reallocWarn(toggles, num_names*sizeof(Toggle *));
	names = (char **)reallocWarn(names, num_names*sizeof(char *));
	orderedNames = (char **)reallocWarn(orderedNames,
				num_names*sizeof(char *));
	rco = (RowColumn **)reallocWarn(rco, num_names*sizeof(RowColumn *));
    }

    for(i = 0; i < num_names; i++) order[i] = -1;
    n = 0;
    for(i = 0; i < num_selected; i++) {
	if(selected[i] >= 0 && selected[i] < num_names) {
	    order[selected[i]] = n++;
	}
    }

    for(i = 0; i < num_names; i++)
    {
	char label[20];
	bool set = false;

	if(i >= num) {
	    n = 0;
	    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
	    rco[i] = new RowColumn("rco", contents, args, n);
	}

	if(order[i] < 0 ) {
	    strcpy(label, "");
	}
	else if(order[i] < num_names ) {
	    snprintf(label, sizeof(label), "%d", order[i]+1);
	    set = true;
	}
	else {
	    cerr << "SelectOrder: order[" << i <<"] = " << order[i]
			<< " > num_names (" << num_names << ")" << endl;
	    strcpy(label, "");
	}
	if(i >= num) {
	    n = 0;
	    XtSetArg(args[n], XmNwidth, 15); n++;
	    XtSetArg(args[n], XmNrecomputeSize, false); n++;
	    labels[i] = new Label(label, rco[i], args, n);
	    toggles[i] = new Toggle(select_names[i], rco[i], this);
	}
	else {
	    labels[i]->setLabel(label);
	    toggles[i]->setLabel(select_names[i]);
	}
	toggles[i]->set(set, false);
	names[i] = strdup(select_names[i]);
    }

    for(i = num_names; i < num; i++) rco[i]->setVisible(false);
    num = num_names;
}

SelectOrder::~SelectOrder(void)
{
    Free(labels);
    Free(order);
    Free(toggles);
    for(int i = 0; i < num; i++) Free(names[i]);
    Free(names);
    Free(orderedNames);
}

void SelectOrder::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Cancel")) {
	dialog_state = DIALOG_CANCEL;
        setVisible(false);
	doCallbacks(base_widget, (XtPointer)&dialog_state, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Apply")) {
	dialog_state = DIALOG_APPLY;
	if(close_when_apply) setVisible(false);
	doCallbacks(base_widget, (XtPointer)&dialog_state, XmNactivateCallback);
    }
    else if(comp->getToggleInstance()) {
	changeOrder(comp);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	for(int i = 0; i < num; i++) {
	    toggles[i]->set(false, true);
	}
    }
}

ParseCmd SelectOrder::parseCmd(const string &cmd, string &msg)
{
    if(parseCompare(cmd, "Cancel")) {
	dialog_state = DIALOG_CANCEL;
        setVisible(false);
	doCallbacks(base_widget, (XtPointer)&dialog_state, XmNactivateCallback);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "Apply")) {
	dialog_state = DIALOG_APPLY;
	if(close_when_apply) setVisible(false);
	doCallbacks(base_widget, (XtPointer)&dialog_state, XmNactivateCallback);
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "Deselect_All")) {
	for(int i = 0; i < num; i++) {
	    toggles[i]->set(false, true);
	}
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
	return COMMAND_PARSED;
    }
    else {
	for(int i = 0; i < num; i++) {
	    const char *c = toggles[i]->getName();
	    if( sameName(cmd, c, (int)strlen(c)) ) {
		toggles[i]->set(true, true);
		return COMMAND_PARSED;
	    }
	}
    }
    return FormDialog::parseCmd(cmd, msg);
}

void SelectOrder::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sTOGGLE_NAME\n", prefix);
}

void SelectOrder::changeOrder(Component *comp)
{
    int i, n = 0;

    for(i = 0; i < num; i++) {
	if(order[i] >= 0) n++;
    }
    for(i = 0; i < num && comp != toggles[i]; i++);
    if(i < num && toggles[i]->state()) {
	char label[20];
	order[i] = n;
	snprintf(label, sizeof(label), "%d", order[i]+1);
	labels[i]->setLabel(label);
    }
    if(i < num && !toggles[i]->state()) {
	char label[20];
	labels[i]->setLabel("");

	for(int j = 0; j < num; j++) {
	    if(order[j] > order[i]) {
		order[j]--;
		snprintf(label, sizeof(label), "%d", order[j]+1);
		labels[j]->setLabel(label);
	    }
	}
	order[i] = -1;
    }
//for(i = 0; i < num; i++) printf("%d %d\n", i, order[i]);
}

int SelectOrder::getOrder(int **o)
{
    int i, n;

    for(i = n = 0; i < num; i++) if(order[i] >= 0) n++;
    if(!n) { *o = NULL; return 0; }

    *o = (int *)mallocWarn(n*sizeof(int));

    for(i = 0; i < num; i++) {
	if(order[i] >= 0) {
	    (*o)[order[i]] = i;
	}
    }
    return n;
}


bool SelectOrder::applyChange(void)
{
    return waitForAnswer();
}

int SelectOrder::getNumSelected(void)
{
    int num_selected = 0;

    for(int i = 0; i < num; i++) {
	if(order[i] >= 0) num_selected++;
    }
    return num_selected;
}

char **SelectOrder::getNames(void)
{
    int num_selected = 0;

    for(int i = 0; i < num; i++) {
	if(order[i] >= 0) {
	    orderedNames[order[i]] = names[i];
	    num_selected++;
	}
    }
    int j = 0;
    for(int i = 0; i < num; i++) {
	if(order[i] < 0) {
	    orderedNames[num_selected+j++] = names[i];
	}
    }
    return orderedNames;
}

void SelectOrder::deselectAll(void)
{
    for(int i = 0; i < num; i++) {
	toggles[i]->set(false, true);
    }
}

void SelectOrder::select(const string &name)
{
    for(int i = 0; i < num; i++) {
	if(!name.compare(names[i])) {
	    toggles[i]->set(true, true);
	    break;
	}
    }
}
