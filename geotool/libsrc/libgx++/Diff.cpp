/** \file Diff.cpp
 *  \brief Defines class Diff.
 *  \author Vera Miljanovic
 */
#include "config.h"
#include <iostream>
#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */
using namespace std;

#include "Diff.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "DataMethod.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
#include "libtime.h"
#include "libstring.h"
static int sortByY(const void *A, const void *B);
}

static void hmsTime(double time, char *c, int len);

Diff::Diff(const string &name, Component *parent,
	WaveformPlot *wave_plot) : FormDialog(name, parent, false, false)
{
    wp = wave_plot;
    createInterface();
}

void Diff::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XtNwidth, 850); n++;
    const char *column_labels[] = {
	"sta", "chan", "time", "epoch time", "diff"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
//    XtSetArg(args[n], XtNdisplayMenuButton, True); n++;
    XtSetArg(args[n], XtNtableTitle, "Waveform Difference"); n++;
    table = new Table("table", this, args, n);
/*
    table->setAttributes(
"sta,%s,chan,%s,time,%t,epoch time,%t,diff,%s");
*/
    table->addActionListener(this, XtNattributeChangeCallback);
    wp->addActionListener(this, XtNdataChangeCallback);

    int col_align[11] = { LEFT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
		RIGHT_JUSTIFY, RIGHT_JUSTIFY};
    table->setAlignment(5, col_align);

    remove_cursor = false;
}

Diff::~Diff(void)
{
}

void Diff::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(action_event->getSource() == wp) { // cursor callback
	list();
    }
    else if(!strcmp(cmd, "Close"))
    {
	setVisible(false);
    }
    else if(!strcmp(action_event->getReason(), XtNattributeChangeCallback)) {
	list();
    }
}

void Diff::setVisible(bool visible)
{
    FormDialog::setVisible(visible);
    if(visible) {
	displayDataCursor();
    }
    else {
	removeDataCursor();
    }
}

void Diff::displayDataCursor(void)
{
   if(!wp) return;

    wp->addActionListener(this, XtNlineCallback);
    wp->addActionListener(this, XtNlineDragCallback);

    double xmin, xmax, x, ymin, ymax;

    wp->getLimits(&xmin, &xmax, &ymin, &ymax);

    x = xmin + (xmax-xmin)/5.;

    if( !wp->lineIsDisplayed("A") ) {
	remove_cursor = true;
        wp->positionLine("A", x, true);
    }
    else {
        remove_cursor = false;
    }
}

void Diff::removeDataCursor(void)
{
    if(!wp) return;

    wp->removeActionListener(this, XtNlineCallback);
    wp->removeActionListener(this, XtNlineDragCallback);

    if(remove_cursor) {
	wp->deleteLine();
    }
}

typedef struct
{
    int		pos;
    double	y;
} Order;

void Diff::list(void)
{
    int		i, num_waveforms, nrows, n;
    double	time;
    char	t_minus_origin[50], t_minus_start[50];
    char	time_string[50], counts_string[50], value_string[50];
    char	epoch_string[50], seg_index[20], index[20], index0[20];
    const char	*row[5];
    float	value, counts;
    CssOriginClass	*origin;
    Order	*order = NULL;
    gvector<Waveform *> wvec;
    Waveform *w;

    if( (num_waveforms = wp->getWaveforms(wvec)) <= 0) {
	table->removeAllRows();
	return;
    }
    nrows = table->numRows();

    if(num_waveforms < nrows) {
	for(i = nrows-1; i >= num_waveforms; i--) {
	    table->removeRow(i);
	}
    }

    order = (Order *)mallocWarn(num_waveforms*sizeof(Order));
	
    for(i = 0; i < num_waveforms; i++) {
	order[i].pos = i;
	order[i].y = wvec[i]->scaled_y0;
    }

    qsort(order, num_waveforms, sizeof(Order), sortByY);

    for(i = 0; i < num_waveforms; i++) if(wvec[order[i].pos]->num_dp > 0)
    {
	w = wvec[order[i].pos];
	time = w->dp[0]->time();

	if(w->ts->getMethod("CalibData"))
	{
	    value = w->dp[0]->segment()->data[w->dp[0]->index()];
	    counts = value;
	    if(w->dp[0]->segment()->calib() != 0.) {
		counts /= w->dp[0]->segment()->calib();
	    }
	}
	else {
	    counts = w->dp[0]->segment()->data[w->dp[0]->index()];
	    value = counts;
	    if(w->dp[0]->segment()->calib() != 0.) {
		value *= w->dp[0]->segment()->calib();
	    }
	}

	timeEpochToString(time, time_string, 50, YMONDHMS2);

	snprintf(epoch_string, sizeof(epoch_string), "%.4lf", time);

	if((origin=wp->getPrimaryOrigin(w)) != NULL && time > origin->time)
	{
	    hmsTime(time - origin->time, t_minus_origin, 50);
	}
	else {
	    strcpy(t_minus_origin, "             -");
	}
	hmsTime(time - w->tbeg(), t_minus_start, 50);

	if(finite(value))
	{
	    ftoa(counts, 4, 1, counts_string, sizeof(counts_string));
	    ftoa(value, 4, 1, value_string, sizeof(value_string));
	}
	else
	{
	    strcpy(counts_string, "-");
	    strcpy(value_string, "-");
	}
	snprintf(seg_index, sizeof(seg_index), "%d", w->dp[0]->segmentIndex());
	snprintf(index, sizeof(index), "%d", w->dp[0]->index());
	n = 0;
	for(int j = 0; j < w->dp[0]->segmentIndex()-1; j++) {
	    n += w->dp[0]->timeSeries()->segment(j)->length();
	}
	n += w->dp[0]->index();
	snprintf(index0, sizeof(index0), "%d", n);
	
	row[0] = w->sta();
	row[1] = w->chan();
	row[2] = time_string;
	row[3] = epoch_string;
	row[4] = "yes";

	if(i < nrows) {
	    table->setRow(i, row);
	}
	else {
	    table->addRow(row, true);
	}
    }
    Free(order);

    table->adjustColumns();
}

static int
sortByY(const void *A, const void *B)
{
    Order *a = (Order *)A;
    Order *b = (Order *)B;
    return((a->y < b->y) ? -1 : 1);
}

static void
hmsTime(double time, char *c, int len)
{
    int hrs, mins;
    double secs;

    hrs = (int)(time/3600.);
    mins = (int)((time - 3600.*hrs)/60.);
    secs = time - 3600.*hrs - 60.*mins;

    if(hrs == 0) {
	snprintf(c, len, "    %02d:%07.4f", mins, secs);
    }
    else {
	snprintf(c, len, "%03d%02d:%07.4f", hrs, mins, secs);
    }
}
