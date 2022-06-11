/** \file Details.cpp
 *  \brief Defines class Details.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
using namespace std;

#include "Details.h"
#include "motif++/MotifClasses.h"

using namespace libglc;

Details::Details(const char *name, Component *parent, char *text)
	: FormDialog(name, parent)
{
    createInterface();
    setText(text);
}

void Details::createInterface(void)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    save_button = new Button("Save...", controls, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, True); n++;
    XtSetArg(args[n], XmNrows, 25); n++;
    XtSetArg(args[n], XmNcolumns, 80); n++;
    XtSetArg(args[n], XmNscrollHorizontal, True); n++;
    XtSetArg(args[n], XmNscrollVertical, True); n++;
    XtSetArg(args[n], XtNbackground, stringToPixel("white")); n++;
    text_field = new TextField("Locate_Details_text", sw, args, n);
}

Details::~Details(void)
{
}

void Details::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Save...")) {
	save();
    }
}

void Details::setText(char *text)
{
    text_field->setString(text);
}

void Details::save(void)
{
    char *file;
    struct stat buf;
    FILE *fp;
    DIR *dirp;

    if( !(file = FileDialog::getFile("Save Location Details", this, "./", "*", "Save")))
    {
	return;
    }
    if(file[0] == '\0' || file[(int)strlen(file)-1] == '/') {
	showWarning("No file selected.");
	free(file);
	return;
    }
    if((dirp = opendir(file)) != NULL) {
	closedir(dirp);
	showWarning("%s is a directory.", file);
	free(file);
	return;
    }
    if(!stat(file, &buf)) {
	int answer = Question::askQuestion("Overwrite File", this,
				"Overwrite File?", "Write File", "Cancel");
	if(answer == 2) { // Cancel
	    free(file);
	    return;
	}
    }
    if((fp = fopen(file, "w")) == NULL)
    {
	showWarning("Cannot open %s\n%s", file, strerror(errno));
        free(file);
        return;
    }
    free(file);

    char *s = text_field->getString();
    for(int i = 0; s[i] != '\0'; i++) fputc(s[i], fp);
    free(s);

    fclose(fp);
}
