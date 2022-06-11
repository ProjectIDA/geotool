/** \file ListDialog.cpp
 *  \brief Defines class ListDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/ListDialog.h"
#include "motif++/Application.h"

/** Constructor.
 *  @param[in] name the name given to this ListDialog instance.
 *  @param[in] parent the Component parent.
 */
ListDialog::ListDialog(const string &name, Component *parent, int num,
		char **items) : FormDialog(name, parent, false, false)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    title = new Label(name, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, title->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNselectionPolicy, XmMULTIPLE_SELECT); n++;
    XtSetArg(args[n], XmNvisibleItemCount, num); n++;
    list = new List("list", this, args, n);
    for(int i = 0; i < num; i++) {
	list->addItem(items[i]);
    }
    answer = false;
}

void ListDialog::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == apply_button) {
	setVisible(false);
	answer = true;
    }
}

/** Destructor
 */
ListDialog::~ListDialog(void)
{
}

/** Get the response from the user. This function does not return until one of
 *  the buttons is selected or the window is closed.
 *  @returns 1 if button 1 was selected, 2 for button 2, 3 for button 3 and 0
 *  if the Question dialog window was closed without selecting a button.
 */
int ListDialog::getSelection(char ***selected_items)
{
    XEvent event;
    Widget w;

    setVisible(true, CENTER_DIALOG);

    answer = false;

    while(!answer)
    {
        if(!isVisible()) {
            // the last event closed the window.
            break;
        }

        XtAppNextEvent(Application::getApplication()->appContext(), &event);

        if(event.xany.type == ButtonPress || event.xany.type == ButtonRelease)
        {
            /* only dispatch button events in the ListDialog
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

    return list->getSelectedItems(selected_items);
}

int ListDialog::getSelectedItems(const string &name, Component *parent, int num,
			char **items, char ***selected_items)
{
    ListDialog *l = new ListDialog(name, parent, num, items);
    int num_selected = l->getSelection(selected_items);
    l->destroy();
    return num_selected;
}
