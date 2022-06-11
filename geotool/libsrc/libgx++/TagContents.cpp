/** \file TagContents.cpp
 *  \brief Defines class TagContents.
 *  \author Ivan Henson
 */
#include "config.h"
#include "TagContents.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
}


TagContents::TagContents(const string &name, Component *parent, int num_names,
		const char **nams, int num_selected, int *selected,
		string user_string) : SelectOrder(name, parent, num_names,
		nams, num_selected, selected)
{
    Arg args[10];
    int n = 0;
    XtSetArg(args[n], XmNpositionIndex, 2); n++;
    ScrolledWindow *swindow = new ScrolledWindow("sw", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    XtSetArg(args[n], XmNrows, 3); n++;
    XtSetArg(args[n], XmNvalue, user_string.c_str()); n++;
    XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    user_text = new TextField("User Text", swindow, args, n);

    close_when_apply = false;
    cancel->setLabel("Close");

    n = 0;
    XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_NONE); n++;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    selected_only = new Toggle("Selected Only", controls2, args, n);
    selected_only->set(false);

    if(num_names > 0) {
	// the Text toggle
	toggles[0]->addActionListener(this, XmNvalueChangedCallback);
	user_text->setSensitive(toggles[0]->state());
    }
}

TagContents::~TagContents(void)
{
}

void TagContents::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    SelectOrder::actionPerformed(action_event);

    if(!strcmp(cmd, "Text Input")) {
	Toggle *t = comp->getToggleInstance();
	if(t) user_text->setSensitive(t->state());
    }
}

ParseCmd TagContents::parseCmd(const string &cmd, string &msg)
{
    string c, text;
    char *tok, *s, *last, *p;
    bool b, err, apply_tag=false;
    ParseCmd ret;

    if(parseFind(cmd, "set", msg, &err, "tag", false, "text", false,
		"selected_only", false))
    {
	if(parseGetArg(cmd, "tag", c)) {
	    for(int i = 0; i < num; i++) {
		toggles[i]->set(false, true);
	    }
	    p = strdup(c.c_str());
	    tok = p;
	    while((s = strtok_r(tok, ",", &last))) {
		tok = NULL;
		if((ret = SelectOrder::parseCmd(string(s), msg))
			!= COMMAND_PARSED)
		{
		    free(p);
		    return ret;
		}
	    }
	    free(p);
	    apply_tag = true;
	}

	if(parseString(cmd, "text", text)) {
	    for(int i = 0; i < num; i++) {
		if(!strcmp(toggles[i]->getName(), "Text Input")) {
		    toggles[i]->set(true, true);
		    break;
		}
	    }
	    user_text->setString(text);
	}
	b = false;
	parseGetArg(cmd, "set", msg, "selected_only", &b);
	if(!msg.empty()) return ARGUMENT_ERROR;
	selected_only->set(b, true);

	if(apply_tag) {
	    dialog_state = DIALOG_APPLY;
	    if(close_when_apply) setVisible(false);
	    doCallbacks(base_widget,
		(XtPointer)&dialog_state, XmNactivateCallback);
	}
    }
    else if(parseCompare(cmd, "Help")) {
        char prefix[200];
        getParsePrefix(prefix, sizeof(prefix));
        parseHelp(prefix);
    }
    else {
	return SelectOrder::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void TagContents::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%scancel\n", prefix);
    printf("%sdeselect_all\n", prefix);
    printf("%sTOGGLE_NANE (station,channel, etc)\n", prefix);
    printf("%stext=TEXT\n", prefix);
}
