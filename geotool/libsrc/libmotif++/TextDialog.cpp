/** \file TextDialog.cpp
 *  \brief Defines class TextDialog.
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

#include "motif++/MotifClasses.h"


TextDialog::TextDialog(const string &name, Component *parent, int max_lines,
	const string &text) : FormDialog(name, parent, false, false)
{
    createInterface();
    text_field->setMaxLines(max_lines);
    setText(text);
}

TextDialog::TextDialog(const string &name, Component *parent,
	const string &text) : FormDialog(name, parent, false, false)
{
    createInterface();
    setText(text);
}

void TextDialog::createInterface(void)
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
    text_field = new TextField("Locate_TextDialog_text", sw, args, n);
}

TextDialog::~TextDialog(void)
{
}

void TextDialog::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Save...")) {
	save();
    }
}

void TextDialog::setText(const string &text)
{
    text_field->setString(text);
}

void TextDialog::append(const string &text)
{
    text_field->append(text);
}

void TextDialog::save(void)
{
    string file;
    struct stat buf;
    FILE *fp;
    DIR *dirp;

    if( !FileDialog::getFile("Save Messages", this, file, "./", "*"))
    {
	return;
    }
    if(file.empty() || file[(int)file.length()-1] == '/') {
	showWarning("No file selected.");
	return;
    }
    if((dirp = opendir(file.c_str())) != NULL) {
	closedir(dirp);
	showWarning(file + " is a directory.");
	return;
    }
    if(!stat(file.c_str(), &buf)) {
	int answer = Question::askQuestion("Overwrite File", this,
				"Overwrite File?", "Write File", "Cancel");
	if(answer == 2) { // Cancel
	    return;
	}
    }
    if((fp = fopen(file.c_str(), "w")) == NULL)
    {
	showWarning("Cannot open %s\n%s", file.c_str(), strerror(errno));
        return;
    }

    char *s = text_field->getString();
    for(int i = 0; s[i] != '\0'; i++) fputc(s[i], fp);
    free(s);

    fclose(fp);
}
