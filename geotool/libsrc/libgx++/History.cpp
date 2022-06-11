/** \file History.cpp
 *  \brief Defines class History.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "History.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "DataMethod.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"


History::History(const string &name, Component *parent, DataSource *ds)
                        : FormDialog(name, parent), DataReceiver(ds)
{
    createInterface();
    data_source->addDataListener(this);
}

void History::createInterface(void)
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
    remove_button = new Button("Remove Methods", controls, this);
    remove_button->setSensitive(false);

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
    label = new Label("Method History", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], XtNdisplayHorizontalScrollbar, False); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XtNwidth, 450); n++;
    const char *column_labels[] = {
	"No.", "Station", "Channel", "Seq. No.", "Method"};
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNsingleSelect, False); n++;
    table = new Table("table", this, args, n);

    table->addActionListener(this, XtNselectRowCallback);

    list();
}

History::~History(void)
{
}

void History::list(void)
{
    char number[10], seqno[10];
    const char *row[5];
    gvector<Waveform *> wvec;
    struct history_item item;

    table->removeAllRows();
    remove_button->setSensitive(false);

    data_source->getWaveforms(wvec, false);

    history.clear();

    for(int i = 0; i < wvec.size(); i++) if(wvec[i]->length() > 2)
    {
	gvector<DataMethod *> *methods = wvec[i]->dataMethods();

	snprintf(number, sizeof(number), "%d", i+1);
	row[0] = number;
	row[1] = wvec[i]->sta();
	row[2] = wvec[i]->chan();

	if((int)methods->size() == 0) {
	    row[3] = "-";
	    row[4] = "none";
	    table->addRow(row, true);
	    item.ts = wvec[i]->ts;
	    item.dm = NULL;
	    history.push_back(item);
	    delete methods;
	    continue;
	}
	for(int j = 0; j < (int)methods->size(); j++) {
	    DataMethod *dm = methods->at(j);
	    snprintf(seqno, sizeof(seqno), "%d", j+1);
	    row[3] = seqno;
	    row[4] = dm->toString();
	    table->addRow(row, true);
	    item.ts = wvec[i]->ts;
	    item.dm = dm;
	    history.push_back(item);
	}
	delete methods;
    }
    table->adjustColumns();
}

void History::setVisible(bool visible)
{
    FormDialog::setVisible(visible);
    if(visible) {
	list();
    }
}

void History::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close"))
    {
	setVisible(false);
    }
    else if(action_event->getSource() == table) { // select row
	vector<bool> states;
	int n = 0, nrows = table->getRowStates(states);
	if(nrows > (int)history.size()) nrows = history.size(); // should not be
	for(int i = 0; i < nrows; i++) if(states[i]) {
	    if(history[i].dm) n++;
	}
	if(n) {
	    remove_button->setSensitive(true);
	}
	else {
	    remove_button->setSensitive(false);
	}
    }
    else if(!strcmp(cmd, "Remove Methods"))
    {
	gvector<Waveform *> wvec;
	data_source->getWaveforms(wvec, false);
	remove(wvec);
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	DataChange *d = (DataChange *)action_event->getCalldata();
	if(d->waveform && isVisible()) {
	    list();
	}
    }
}

void History::remove(gvector<Waveform *> &wvec)
{
    vector<bool> states;
    vector<const char *> seqno;
    int j, k, nrows;
    gvector<Waveform *> ws;

    nrows = table->getRowStates(states);
    if(nrows > (int)history.size()) nrows = history.size(); // should not happen
    table->getColumn(3, seqno);

    setCursor("hourglass");
    for(j = 0; j < wvec.size(); j++)
    {
	bool removed_one = false;
	for(int i = nrows-1; i >= 0; i--) if(states[i] && history[i].dm)
	{
	    if(history[i].ts == wvec[j]->ts)
	    {
		sscanf(seqno[i], "%d", &k);
		k--;
		gvector<DataMethod *> *methods = history[i].ts->dataMethods();
		if(k < (int)methods->size() && history[i].dm == methods->at(k))
		{
		    if(history[i].dm->getRotateDataInstance())
		    {
			wvec[j]->ts->getChan(wvec[j]->channel);
		    }
		    methods->removeAt(k);
		    removed_one = true;
		    wvec[j]->setDataMethods(methods);
		}
		delete methods;
	    }
	}
	if(removed_one) {
	    DataMethod::update(wvec[j]);
	    ws.push_back(wvec[j]);
	}
    }

    data_source->modifyWaveforms(ws);

    setCursor("default");
}

ParseCmd History::parseCmd(const string &cmd, string &msg)
{
    gvector<Waveform *> wvec;

    if(parseCompare(cmd, "remove_all")) {
	data_source->getWaveforms(wvec, false);
        remove(wvec);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}
