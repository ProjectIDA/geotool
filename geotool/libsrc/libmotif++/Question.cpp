/** \file Question.cpp
 *  \brief Defines class Question.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/Question.h"
#include "motif++/Application.h"

/** Constructor for three buttons. If button3 is NULL, then display only two
 *  buttons. If button2 and button3 are NULL, then display only one button. If
 *  button1 is also NULL, then display one Button labelled "OK".
 *  @param[in] name the name given to this Question instance.
 *  @param[in] parent the Component parent.
 *  @param[in] question the text to display.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 *  @param[in] button3 the label for button 3.
 */
Question::Question(const string &name, Component *parent,const string &question,
		const string &button1, const string &button2,
		const string &button3) : FormDialog(name, parent, false, false)
{
    createInterface(question, button1, button2, button3);
}

/** Create the interface. If button3 is NULL, then display only two buttons.
 *  If button2 and button3 are NULL, then display only one button. If button1
 *  is also NULL, then display one Button labelled "OK".
 *  @param[in] question the text to display.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 *  @param[in] button3 the label for button 3.
 */
void Question::createInterface(const string &question, const string &button1,
		const string &button2, const string &button3)
{
    Arg args[10];
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

    if(!button1.empty()) {
	b1 = new Button(button1, controls, this);
    }
    else {
	b1 = new Button("OK", controls, this);
    }
    if(!button2.empty()) {
	b2 = new Button(button2, controls, this);
    }
    else b2 = NULL;
    if(!button3.empty()) {
	b3 = new Button(button3, controls, this);
    }
    else b3 = NULL;

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
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    label = new Label(question, this, args, n);
}

/** Process the actions from the buttons.
 */
void Question::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == b1) {
	answer = 1;
    }
    else if(comp == b2) {
	answer = 2;
    }
    else if(comp == b3) {
	answer = 3;
    }
    else {
	return;
    }
    setVisible(false);
}

/** Get the response from the user. This function does not return until one of
 *  the buttons is selected or the window is closed.
 *  @returns 1 if button 1 was selected, 2 for button 2, 3 for button 3 and 0
 *  if the Question dialog window was closed without selecting a button.
 */
int Question::getAnswer(void)
{
    XEvent event;
    Widget w;

    setVisible(true, CENTER_DIALOG);

    answer = 0;

    while(!answer)
    {
	if(!isVisible()) {
	    // the last event closed the window.
	    return 0;
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
    return answer;
}

/** Create a 1,2 or 3-button Question dialog and wait for the response. This
 *  function does not return until one of the buttons is selected or the window
 *  is closed.  If button3 is NULL, then display only two buttons. If button2
 *  and button3 are NULL, then display only one button. If button1 is also NULL,
 *  then display one Button labelled "OK".
 *  @param[in] name the name given to this Question instance.
 *  @param[in] parent the Component parent.
 *  @param[in] question the text to display.
 *  @param[in] button1 the label for button 1.
 *  @param[in] button2 the label for button 2.
 *  @param[in] button3 the label for button 3.
 *  @returns 1 if button 1 was selected, 2 for button 2, 3 for button 3 and 0
 *  if the Question dialog window was closed without selecting a button.
 */
int Question::askQuestion(const string &name, Component *parent,
		const string &question, const string &button1,
		const string &button2, const string &button3)
{
    Question *q = new Question(name, parent, question, button1,button2,button3);
    int answer = q->getAnswer();
    q->destroy();
    return answer;
}

/* Destructor */
Question::~Question(void)
{
}
