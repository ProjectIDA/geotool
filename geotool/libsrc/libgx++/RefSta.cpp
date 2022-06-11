/** \file RefSta.cpp
 *  \brief Defines class RefSta.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "RefSta.h"
#include "motif++/MotifClasses.h"
#include "gobject++/DataSource.h"
#include "widget/Table.h"

extern "C" {
#include "libstring.h"
}

RefSta::RefSta(const string &name, Component *parent, const string &net,
		DataSource *ds) : FormDialog(name, parent), DataReceiver(NULL)
{
    network.push_back(stringUpperToQuark(net));

    createInterface();
    setDataSource(ds);
    enableCallbackType(XmNactivateCallback);
}

RefSta::RefSta(const string &name, Component *parent, vector<int> &nets,
		DataSource *ds) : FormDialog(name, parent), DataReceiver(NULL)
{
    network.clear();
    for(int i = 0; i < (int)nets.size(); i++) network.push_back(nets[i]);

    createInterface();
    setDataSource(ds);
    enableCallbackType(XmNactivateCallback);
}

void RefSta::createInterface(void)
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
    apply_button = new Button("Apply", controls, this);
    make_perm_button = new Button("Make Permanent", controls, this);

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
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XtNwidth, 250); n++;
    XtSetArg(args[n], XtNvisibleRows, 25); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNdragSelect, False); n++;
    XtSetArg(args[n], XtNtableTitle, "Select Reference Station"); n++;
    XtSetArg(args[n], XtNcolumns, 3); n++;
    table = new Table("Select Reference Station", this, args, n);
    table->setAttributes("Network,%s,Station,%s,Refsta,%s");

    table->addActionListener(this, XtNattributeChangeCallback);
    table->addActionListener(this, XtNselectRowCallback);
}

void RefSta::setDataSource(DataSource *ds, vector<int> &nets)
{
    network.clear();
    for(int i = 0; i < (int)nets.size(); i++) network.push_back(nets[i]);
    setDataSource(ds);
}

void RefSta::setDataSource(DataSource *ds)
{
    if(data_source != ds) {
	if(data_source) data_source->removeDataReceiver(this);
	data_source = ds;
	data_source->addDataReceiver(this);
    }
    if(!data_source) return;

    const char *row[3];
    TAttribute a[3];
    GStation **stations=NULL;

    table->removeAllRows();
    int num_columns = table->numColumns();

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    for(int k = 0; k < (int)network.size(); k++)
    {
	char *net = (char *)quarkToString(network[k]);
	int num_stations = data_source->getNetwork(net, &stations);
	int num_rows = table->numRows();

	for(int i = 0; i <num_stations; i++)
	{
	    for(int j = 0; j < num_columns; j++)
	    {
		row[j] = (char *)"-";
		if(!strcmp(a[j].name, "Network")) {
		    row[j] = (char *)quarkToString(stations[i]->net);
		}
		else if(!strcmp(a[j].name, "Station")) {
		    row[j] = (char *)quarkToString(stations[i]->sta);
		}
		else if(!strcmp(a[j].name, "Refsta")) {
		    row[j] = (char *)quarkToString(stations[i]->refsta);
		}
	    }
	    table->addRow(row, false);
	}
	for(int i = 0; i < num_stations; i++) {
	    if(stations[i]->sta == stations[i]->refsta) {
		table->selectRow(num_rows+i, true);
	    }
	}
	Free(stations);
    }
    table->adjustColumns();
    table->sortByColumnLabels("Network,Station");

    apply_button->setSensitive(false);
    make_perm_button->setSensitive(false);
    applied = false;
}

RefSta::~RefSta(void)
{
    if(data_source) data_source->removeDataReceiver(this);
}

void RefSta::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
        setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	dialog_state = DIALOG_APPLY;
	apply(false);
    }
    else if(!strcmp(cmd, "Make Permanent")) {
	apply(true);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectStation((MmTableSelectCallbackStruct *)action_event->getCalldata());
    }
}

void RefSta::selectStation(MmTableSelectCallbackStruct *c)
{
    if(c->nchanged_rows == 0 || !data_source) return;

    if(!c->states[c->changed_rows[0]]) {
	apply_button->setSensitive(false);
	return;
    }

    vector<const char *> row;
    table->getRow(c->changed_rows[0], row);
    int q_net = stringUpperToQuark(row[0]);

    // make sure that only one station is selected in each network.
    for(int i = 0; i < c->nrows; i++)
	if(c->states[i] && i != c->changed_rows[0])
    {
	table->getRow(i, row);
	if(q_net == stringUpperToQuark(row[0])) {
	    table->selectRow(i, false);
	    c->states[i] = false;
	}
    }

    int i;
    for(i = 0; i < c->nrows; i++) if(c->states[i])
    {
	table->getRow(i, row);
	if(strcmp(row[1], row[2])) { // if selected row station != refsta
	    break;
	}
    }
    if(i < c->nrows) {
	apply_button->setSensitive(true);
	make_perm_button->setSensitive(true);
    }
    else {
	apply_button->setSensitive(false);
	if(!applied) make_perm_button->setSensitive(false);
    }
}

void RefSta::apply(bool permanent)
{
    if(!data_source) return;
    vector<bool> states;
    vector<const char *> net, sta;
    int nrows = table->getRowStates(states);
    int num_refsta=0;
    GStation **stations=NULL;

    table->getColumn(0, net);
    apply_button->setSensitive(false);
    applied = true;

    const char **refsta = NULL;
    if(!(refsta = (const char **)mallocWarn(nrows*sizeof(char *)))) return;

    table->getColumn(1, sta);
    for(int i = 0; i < nrows; i++) if(states[i])
    {
	int q_net = stringUpperToQuark(net[i]);
	int q_sta = stringUpperToQuark(sta[i]);
	int num_stations = data_source->getNetwork((char *)net[i], &stations);
	for(int j = 0; j < num_stations; j++) {
	    if(q_net == stations[j]->net) {
		stations[j]->refsta = q_sta;
	    }
	}
	Free(stations);
	refsta[num_refsta++] = quarkToString(q_sta);
    }

    setDataSource(data_source);

    if(permanent) {
//	int i, num_refsta;
//	const char **refsta = NULL;

	make_perm_button->setSensitive(false);

/*
	for(i = num_refsta = 0; i < num_stations; i++) {
	    if(stations[i]->sta == stations[i]->refsta) num_refsta++;
	}
	if(!(refsta = (const char **)mallocWarn(num_refsta*sizeof(char *))))
	    return;

	for(i = num_refsta = 0; i < num_stations; i++) {
	    if(stations[i]->sta == stations[i]->refsta) {
		refsta[num_refsta++] = quarkToString(stations[i]->refsta);
	    }
	}
*/
	setCursor("hourglass");
	data_source->saveRefSta(num_refsta, refsta);
	setCursor("default");
//	Free(refsta);
	applied = false;
    }
    Free(refsta);
}

ParseCmd RefSta::parseCmd(const string &cmd, string &msg)
{
    if(parseCompare(cmd, "Apply")) {
	apply_button->activate();
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "Make_Permanent")) {
	make_perm_button->activate();
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "Select_Row", 10)) {
	return table->parseCmd(cmd, msg);
    }
    else if(parseCompare(cmd, "Select ", 7)) {
	string s = string("select_row ") + cmd.substr(7);
	return table->parseCmd(s, msg);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_NOT_FOUND;
}

void RefSta::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%smake_permanent\n", prefix);
    printf("%sselect [network=NAME] [station=NAME]\n", prefix);
}
