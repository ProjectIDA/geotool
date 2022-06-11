/** \file GClusterSummary.cpp
 *  \brief Defines class GClusterSummary
 *  \author Vera Miljanovic
 */
#include "config.h"
#include <iostream>

using namespace std;

#include "GClusterSummary.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"

using namespace libgcluster;


GClusterSummary::GClusterSummary(const char *name, Component *parent,
		ActionListener *listener) : ParamDialog(name, parent)
{
    createInterface();

    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void GClusterSummary::createInterface()
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 4); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 4); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, 3); n++;
    const char *col_labels[] = {"cluster", "matched waveforms", "unmatched waveforms"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    table = new Table("table", this, args, n);
    table->setAttributes("cluster,%d,matched waveforms,%d,unmatched waveforms,%d");
    int alignment[3] = {LEFT_JUSTIFY, LEFT_JUSTIFY, LEFT_JUSTIFY};
    table->setAlignment(3, alignment);
    table->addActionListener(this, XtNattributeChangeCallback);

}

GClusterSummary::~GClusterSummary(void)
{
}

void GClusterSummary::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("GCluster Summary");
    }
}

void GClusterSummary::setSummary(int nclusters, int *matched, int *unmatched)
{
    const char *row[3];
    char clusterid_string[20], matched_string[20], unmatched_string[20];

    table->removeAllRows();

    int num_columns = table->numColumns();

    for (int i = 0; i < nclusters; i++) {

      for(int j = 0; j < num_columns; j++) {
	TAttribute a = table->getAttribute(j);

	if(!strcmp(a.name, "cluster")) {
	  snprintf(clusterid_string, sizeof(clusterid_string), a.format, i);
	  row[j] = clusterid_string;
	}
	else if(!strcmp(a.name, "matched waveforms")) {
	  snprintf(matched_string, sizeof(matched_string), a.format,
		   matched[i]);
	  row[j] = matched_string;
	}
	else if(!strcmp(a.name, "unmatched waveforms")) {
	  snprintf(unmatched_string, sizeof(unmatched_string), a.format,
		   unmatched[i]);
	  row[j] = unmatched_string;
	}
      }

      table->addRow(row, false);
    }

    /* adjust table columns */
    table->adjustColumns();
}
