/** \file AddGroup.cpp
 *  \brief Defines class AddGroup
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
#include <math.h>

using namespace std;

#include "AddGroup.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "TableSource.h"

extern "C" {
#include "libstring.h"
}

using namespace libgbm;

AddGroup::AddGroup(const char *name, Component *parent,
	ActionListener *listener) : FormDialog(name, parent), DataReceiver()
{
    createInterface();
    setRecipeDirChoice();
    listNetworks();
    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
}

void AddGroup::createInterface()
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Add Beam Group", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    label2 = new Label("Recipe directory", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, label2->baseWidget()); n++;
    recipe_dir_choice = new FileChoice("recipe_dir", form , args, n, NULL, false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form2 = new Form("form2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    label3 = new Label("group-name", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, label3->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    group_text = new TextField("group_text", form2, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    add_button = new Button("Add", controls, this);
    add_button->setSensitive(false);
    cancel_button = new Button("Cancel", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    form3 = new Form("form3", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNsingleSelect, True); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNtableTitle, "Select a Network"); n++;
    XtSetArg(args[n], XtNcolumns, 1); n++;
    const char *col_labels[] = {"Network"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    net_table = new Table("net_table", form3, args, n);

    net_table->addActionListener(this, XtNselectRowCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, net_table->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNvisibleRows, 20); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNtableTitle, "Select Stations and Set Weight"); n++;
    XtSetArg(args[n], XtNcolumns, 3); n++;
    const char *collabels[] = {"Station", "Channel", "Weight"};
    XtSetArg(args[n], XtNcolumnLabels, collabels); n++;
    Boolean col_editable[] = {False, False, True};
    XtSetArg(args[n], XtNcolumnEditable, col_editable); n++;
    group_table = new Table("group_table", form3, args, n);

    group_table->addActionListener(this, XtNselectRowCallback);

    table_source = new TableSource("AddGroup");
    table_source->openPrefix("tmp");
    table_source->query("affiliation select * from affiliation");
    table_source->query("site select * from site");
    table_source->query("sitechan select * from sitechan");
    setDataSource(table_source);
}

AddGroup::~AddGroup(void)
{
}

void AddGroup::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Cancel")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Add")) {
	if(addGroup()) {
	    setVisible(false);
	    doCallbacks(base_widget, (XtPointer)NULL, XmNactivateCallback);
	}
    }
    else if(action_event->getSource() == net_table) { // select row
	selectNetwork();
    }
    else if(action_event->getSource() == group_table) { // select row
	bool set = (!group_text->empty() && group_table->numSelectedRows());
	add_button->setSensitive(set);
    }
    else if(comp == group_text) {
	bool set = (!group_text->empty() && group_table->numSelectedRows());
	add_button->setSensitive(set);
    }
}

void AddGroup::setRecipeDirChoice(void)
{
    string recipe_dir, recipe_dir2;

    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME"))) {
	    recipe_dir.assign(c+string("/tables/recipes"));
        }
    }
    recipe_dir_choice->removeAllChoices();

    if(!recipe_dir.empty()) {
	recipe_dir_choice->addItem(recipe_dir);
    }
    getProperty("recipeDir2", recipe_dir2);
    if(!recipe_dir2.empty()) {
	recipe_dir_choice->addItem(recipe_dir2);
    }
}

void AddGroup::listNetworks(void)
{
    if(!data_source) return;

    cvector<CssAffiliationClass> a;

    if( data_source->getTable(a) <= 0 ) return;

    const char *row[1];

    net_table->removeAllRows();

    for(int i = 0; i < a.size(); i++)
    {
	row[0] = a[i]->net;
	net_table->addRow(row, false);
    }

    vector<int> columns;
    columns.push_back(0);
    net_table->sortUnique(columns);
    net_table->adjustColumns();
    net_table->sortByColumnLabels("Network");
}

void AddGroup::selectNetwork(void)
{
    vector<int> rows;
    vector<const char *> row;
    GStation **stations=NULL;

    if( !net_table->getSelectedRows(rows) ) return;
    net_table->getRow(rows[0], row);
    int num_stations = data_source->getNetwork(row[0], &stations);

    group_table->removeAllRows();
    for(int i = 0; i < num_stations; i++) {
	const char *r[3];
	r[0] = (char *)quarkToString(stations[i]->sta);
	r[2] = "1.0";
	for(int j = 0; j < stations[i]->nchan; j++) {
	    r[1] = quarkToString(stations[i]->chan[j]);
	    group_table->addRow(r, false);
	}
    }
    group_table->adjustColumns();
}

bool AddGroup::addGroup()
{
    int i, nrows;
    vector<int> rows;
    string group;
    char net[50];
    const char *recipe_dir = NULL;
    vector<const char *> row;
    vector<BeamSta> sta;

    if(net_table->getSelectedRows(rows) <= 0) {
	showWarning("No network selected.");
	return false;
    }
    net_table->getRow(rows[0], row);
    stringcpy(net, row[0], sizeof(net));

    if(!group_text->getString(group)) {
	showWarning("no group-name.");
	return false;
    }
    rows.clear();
    if((nrows = group_table->getSelectedRows(rows)) <= 0) {
	showWarning("No stations selected.");
	return false;
    }

    for(i = 0; i < nrows; i++) {
	BeamSta s;
	group_table->getRow(rows[i], row);
	stringcpy(s.sta, row[0], sizeof(sta[i].sta));
	stringcpy(s.chan, row[1], sizeof(sta[i].chan));

	if(!stringToDouble(row[2], &s.wgt)) {
	    showWarning("Invalid weight for %s/%s", row[0], row[1]);
	    return false;
	}
	sta.push_back(s);
    }

    recipe_dir = recipe_dir_choice->getChoice();

    if( Beam::addGroup(stringToUpper(net), group, sta, recipe_dir) )
    {
	return true;
//	    BeamGroupListTable(gt);
    }
    return false;
}
