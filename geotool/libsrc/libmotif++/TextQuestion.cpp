/** \file TextQuestion.cpp
 *  \brief Defines class TextQuestion.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/TextQuestion.h"
#include "motif++/Application.h"

/** Constructor.
 *  @param[in] name the name given to this TextQuestion instance.
 *  @param[in] parent the Component parent.
 *  @param[in] question the text to display above the TextField.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 */
TextQuestion::TextQuestion(const string &name, Component *parent,
		const string &question, const string &button1,
		const string &button2) : FormDialog(name, parent, false, false)
{
    createInterface(question, button1, button2);
}

/** Initialize.
 *  @param[in] question the text to display above the TextField.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 */
void TextQuestion::createInterface(const string &question,const string &button1,
			const string &button2)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    title = new Label(getName(), this, args, n);

    n = 0;
    XtSetArg(args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, title->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep1 = new Separator("sep1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    b1 = new Button(button1, controls, this);
    b1->setSensitive(false);
    if(!button2.empty()) {
	b2 = new Button(button2, controls, this);
    }
    else b2 = NULL;

    n = 0;
    XtSetArg(args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep2 = new Separator("sep2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNcolumns, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    text = new TextField("text", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, text->baseWidget()); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    label = new Label(question, this, args, n);
}

/** Process the actions from the buttons.
 */
void TextQuestion::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == text) {
	char *c;
	if((c = text->getString()) && strlen(c) > 0) {
	    b1->setSensitive(true);
	    free(c);
	}
	else {
	    b1->setSensitive(false);
	}
    }
    if(comp == b1) {
	answer = 1;
    }
    else if(comp == b2) {
	answer = 2;
    }
    else {
	return;
    }
    setVisible(false);
}

/** Get the response from the user. The getAnswer function does not return
 *  until either one of the buttons has been selected or the window has been
 *  closed. If the first button is selected, a string value is returned.
 *  If the second button is selected or the window is closed, then NULL is
 *  returned.
 *  @returns the text string. Free the pointer when it is no longer needed.
 */
bool TextQuestion::getAnswer(string &ans)
{
    XEvent event;
    Widget w;

    setVisible(true);

    ans.clear();
    answer = 0;

    while(!answer)
    {
	if(!isVisible()) {
	    // the last event closed the window.
	    return false;
	}

	XtAppNextEvent(Application::getApplication()->appContext(), &event);

	if(event.xany.type == ButtonPress || event.xany.type == ButtonRelease)
	{
	    /* only dispatch button events in the QuestionDialog
	     */
	    w = XtWindowToWidget(XtDisplay(base_widget), event.xany.window);
	    while(w != NULL)
	    {
		if(w == base_widget) {
		    XtDispatchEvent(&event);
		    break;
		}
		w = XtParent(w);
	    }
	}
	else {
	    XtDispatchEvent(&event);
	}
    }
    if(answer == 1) {
	text->getString(ans);
	return true;
    }
    else {
	return false;
    }
}

/** Destructor.
 */
TextQuestion::~TextQuestion(void)
{
}

/** Create a TextQuestion dialog and wait for the response. This function does
 *  not return until one of the buttons has been selected or the window is
 *  closed. If the first button is selected, a string value is returned.
 *  If the second button is selected or the window is closed, then NULL is
 *  returned.
 *  @param[in] name the name given to this TextQuestion instance.
 *  @param[in] parent the Component parent.
 *  @param[in] question the text to display above the TextField.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 */
bool TextQuestion::askTextQuestion(const string &name, Component *parent,
		const string &question, string &ans, const string &button1,
		const string &button2)
{
    TextQuestion *q = new TextQuestion(name, parent, question, button1,button2);
    bool ret = q->getAnswer(ans);
    q->destroy();
    return ret;
}

/** Create a TextQuestion dialog and wait for the response. The default answer
 *  can be entered. This function does not return until one of the buttons has
 *  been selected or the window is closed. If the first button is selected, a
 *  string value is returned. If the second button is selected or the window is
 *  closed, then NULL is returned.
 *  @param[in] name the name given to this TextQuestion instance.
 *  @param[in] parent the Component parent.
 *  @param[in] question the text to display above the TextField.
 *  @param[in] default_ans the text to display in the TextField as the default
 *	answer.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 */
bool TextQuestion::askTextQuestion(const string &name, Component *parent,
		const string &question, string &ans, const string &default_ans,
		const string &button1, const string &button2)
{
    TextQuestion *q = new TextQuestion(name, parent, question, button1,button2);
    q->text->setString(default_ans, true);
    bool ret = q->getAnswer(ans);
    q->destroy();
    return ret;
}
