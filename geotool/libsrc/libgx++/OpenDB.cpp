/** \file OpenDB.cpp
 *  \brief Defines class OpenDB.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
#include <errno.h>
#include <sys/param.h>
using namespace std;

#include "OpenDB.h"
#include "TableQuery.h"
#include "widget/Table.h"
#include "widget/TabClass.h"
#include "motif++/MotifClasses.h"
#include "CssFileDialog.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

#define PERIOD(a) (a[0] != '\0' ? "." : "")


OpenDB::OpenDB(const string &name, Component *parent, bool auto_connect) :
		FormDialog(name, parent)
{
    tq = NULL;
    createInterface();

    if(auto_connect) {
	autoConnect();
	autoOutputConnect();
    }
}

OpenDB::OpenDB(const string &name, Component *parent, bool auto_connect,
		ActionListener *listener) : FormDialog(name, parent)
{
    tq = NULL;
    createInterface();

    if(auto_connect) {
	autoConnect();
	autoOutputConnect();
    }

    addActionListener(listener);
    addActionListener(listener, XtNconnectionCallback);
}

OpenDB::OpenDB(const string &name, TableQuery *tq_parent, bool auto_connect,
		ActionListener *listener) : FormDialog(name, tq_parent)
{
    tq = tq_parent;
    createInterface();

    if(auto_connect) {
	autoConnect();
	autoOutputConnect();
    }

    addActionListener(listener);
    addActionListener(listener, XtNconnectionCallback);
}

void OpenDB::createInterface()
{
    Separator *sep;
    RowColumn *controls;
    Arg args[6];
    int n;

    setSize(450, 650);
    n = 0;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW); n++;
    setValues(args, n);

    interface_type = ODBC_INTERFACE;
    output_interface_type = ODBC_INTERFACE;

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    new Button("Close", controls, this);
    run_query_button = new Button("Run Query", controls, this);
    run_query_button->setSensitive(false);

    n = 0;
//    XtSetArg(args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    tab = new TabClass("tab", this, args, n);
    tab->addActionListener(this, XtNtabCallback);
    
    connected = false;
    no_sites = no_sitechans = no_affiliations = false;
    output_connected = false;
    if( !getProperty("directory_structure", directory_structure) ) {
	// empty is for stationyyyymmdd
//	directory_structure.assign("%Y/%m/%d");
    }

    directory_duration = 1.e+30;
    query_buffer = 144000.;
    last_arid_input = false;
#ifdef HAVE_LIBODBC
    hdbc = NULL;
    output_hdbc = NULL;
#endif
    ffdb = NULL;
    output_ffdb = NULL;

    makeConnectionTab();
    makeMappingTab();
    makeAccountTab();
    makeOriginTab();
    makeArrivalTab();
    makeTimeTab();
    makeGeneralTab();

    setFFDBInterface();

    turnOnTabs(false, false);

    enableCallbackType(XmNactivateCallback);
    enableCallbackType(XtNconnectionCallback);
}

bool OpenDB::autoConnect(void)
{
    string prop;

    if( !getProperty("data_source", prop) ) {
	const char *src = source->getChoice();

	if(!strcmp(src, "Flat-File-DB")) setFFDBInterface();
	else if(!strcmp(src, "File-Prefix")) setPrefixInterface();
	else setODBCInterface();
	return false;
    }
    else if(!prop.compare("Flat-File-DB")) {
	autoConnectFlatFile();
    }
    else if(!prop.compare("File-Prefix")) {
	autoConnectFilePrefix();
    }
    else if(!prop.compare(0, 5, "ODBC:")) {
	autoConnectODBC(prop.substr(5));
    }
    else {
	autoConnectODBC(prop);
    }
    return connected;
}

void OpenDB::autoConnectFlatFile(void)
{
    string prop;

    disconnect();
    source->setChoice("Flat-File-DB");
    setFFDBInterface();

    if(getProperty("parameter_root", prop)) {
	account_text->setString(prop);
    }
    if(getProperty("segment_root", prop)) {
	password_text->setString(prop);
    }
    connectButton();
}

void OpenDB::autoConnectFilePrefix(const string &file_prefix)
{
    string prop;

    disconnect();
    source->setChoice("File-Prefix");
    setPrefixInterface();

    if( file_prefix.empty() ) {
	if(getProperty("file_prefix", prop)) {
	    account_text->setString(prop);
	    account_text->showPosition((int)prop.length());
	}
    }
    else {
	account_text->setString(file_prefix);
	account_text->showPosition((int)file_prefix.length());
    }
    connectButton();
}

void OpenDB::autoConnectODBC(const string &odbc_source)
{
    if(source->setChoice(odbc_source, true, true))
    {
	string prop;

	disconnect();
	setODBCInterface();

	if(getProperty("odbc_user", prop)) {
	    account_text->setString(prop);
	}
	if(getProperty("odbc_password", prop)) {
	    data_passwd = prop;
	}
	connectButton();
    }
}

bool OpenDB::autoOutputConnect(void)
{
    string prop;

    if(getProperty("use_same_output", true)) {
	return false;
    }

    if(!getProperty("output_data_source", prop)) {
	const char *src = output_source->getChoice();

	if(!strcmp(src, "Flat-File-DB")) setOutputFFDBInterface();
	else if(!strcmp(src, "File-Prefix")) setOutputPrefixInterface();
	else setOutputODBCInterface();
	return false;
    }
    else if(!prop.compare("Flat-File-DB")) {
	autoOutputConnectFlatFile();
    }
    else if(!prop.compare("File-Prefix")) {
	autoOutputConnectFilePrefix();
    }
    else if(!prop.compare(0, 5, "ODBC:")) {
	autoOutputConnectODBC(prop.substr(5));
    }
    else {
	autoOutputConnectODBC(prop);
    }
    if(output_connected) {
	output_toggle->set(false, false);
    }
    return output_connected;
}

void OpenDB::autoOutputConnectFlatFile(void)
{
    string prop;

    output_source->setChoice("Flat-File-DB");
    setOutputFFDBInterface();

    if(getProperty("output_parameter_root", prop)) {
	output_account_text->setString(prop);
    }
    if(getProperty("output_segment_root", prop)) {
	output_password_text->setString(prop);
    }
    outputConnectButton();
}

void OpenDB::autoOutputConnectFilePrefix(void)
{
    string prop;

    output_source->setChoice("File-Prefix");
    setOutputPrefixInterface();

    if(getProperty("output_file_prefix", prop)) {
	output_account_text->setString(prop);
	output_account_text->showPosition((int)prop.length());
    }
    outputConnectButton();
}

void OpenDB::autoOutputConnectODBC(const string &odbc_source)
{
    if(output_source->setChoice(odbc_source, true, true))
    {
	string prop;

	setOutputODBCInterface();

	if(getProperty("output_odbc_user", prop)) {
	    output_account_text->setString(prop);
	}
	if(getProperty("output_odbc_password", prop)) {
	    output_data_passwd = prop;
	}
	outputConnectButton();
    }
}

bool OpenDB::connectODBC(const string &odbc_source, const string &user,
			const string &password)
{
    if(!source->setChoice(odbc_source, true, true)) {
	showWarning("Invalid Data Source: %s", odbc_source.c_str());
	return false;
    }
    else {
	setODBCInterface();

	account_text->setString(user);
	data_passwd = password;

	return connectButton();
    }
}

void OpenDB::connectPrefix(const string &file)
{
    if(source->setChoice("File-Prefix"))
    {
	setPrefixInterface();

	char *file_prefix = strdup(file.c_str());
	// get file_prefix
	int n = strlen(file_prefix);
	for(int i = n-1; i >= 0 && file_prefix[i] != '/'; i--) {
	    if(file_prefix[i] == '.') {
		file_prefix[i] = '\0';
		break;
	    }
	}

	account_text->setString(file_prefix);
	account_text->showPosition((int)strlen(file_prefix));
	Free(file_prefix);

	connectButton();
    }
}

ConnectionType OpenDB::sourceType(void)
{
    if(!connected) {
	return NO_CONNECTION;
    }
    if(interface_type == ODBC_INTERFACE) {
	return ODBC_CONNECTION;
    }
    else if(interface_type == FFDB_INTERFACE) {
	return FFDB_CONNECTION;
    }
    else if(interface_type == PREFIX_INTERFACE) {
	return PREFIX_CONNECTION;
    }
    return NO_CONNECTION;
}

void OpenDB::makeConnectionTab()
{
    Arg args[20];
    int n;
    Form *Connection;
    RowColumn *rc, *rc2, *rc3, *rc4;

    Connection = new Form("Connection", tab);

    setResource("*%s*rc*marginHeight: 0", getName());

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    rc = new RowColumn("rc", Connection, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    source_label = new Label("Data Source", rc, args, n);
    account_label = new Label("Account", rc, args, n);
    choose_file_button = new Button("Prefix Path (Choose File)",rc,args,n,this);
    choose_file_button->setVisible(false);
    passwd_label = new Label("Password", rc, args, n);

    source = new Choice("data source", rc, this);
#ifdef HAVE_LIBODBC
    int num;
    char **dataSources=NULL;
    if(ODBCDataSources(&num, &dataSources) == -1) {
        showWarning("OpenDBInit: %s", ODBCErrMsg());
    }
    else {
        for(int i = 0; i < num; i++) {
            source->addItem(dataSources[i]);
        }
	source->addSeparator();
    }
#endif
    source->addItem("Flat-File-DB");
    source->addItem("File-Prefix");

    XtSetArg(args[0], XmNcolumns, 20);
    account_text = new TextField("account", rc, args, 1);
    account_text->addActionListener(this, XmNactivateCallback);
    account_text->addActionListener(this, XmNvalueChangedCallback);

    password_text = new TextField("password", rc, args, 1);
    password_text->addActionListener(this, XmNactivateCallback);
    password_text->addActionListener(this, XmNvalueChangedCallback);
    password_text->addActionListener(this, XmNmodifyVerifyCallback);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    rc2 = new RowColumn("rc2", Connection, args, n);

    connect = new Button("Connect", rc2, this);
    connect->setSensitive(false);
    create_param = new Button("Create Parameter Root", rc2, this);
    create_param->setSensitive(false);
    create_param->setVisible(false);
    create_seg = new Button("Create Segment Root", rc2, this);
    create_seg->setSensitive(false);
    create_seg->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    save_source = new Toggle("Save Connection Information",
				Connection, this, args, n);
    save_source->set(true);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    XtSetArg(args[n], XmNskipAdjust, true); n++;
    XtSetArg(args[n], XmNwordWrap, False); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, False); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    connection = new TextField("connection", Connection, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, connection->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    Separator *sep = new Separator("sep", Connection, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    output_toggle = new Toggle("Use Same Connection for Output",
					Connection, this, args, n);
    output_toggle->set(true);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, output_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, save_source->baseWidget()); n++;
    output_form = new Form("output_form", Connection, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, output_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNspacing, 0); n++;
    rc3 = new RowColumn("rc3", output_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    output_source_label = new Label("Output", rc3, args, n);
    output_account_label = new Label("Account", rc3, args, n);
    output_choose_file_button = new Button("Prefix Path (Choose File)",
					rc3, args, n, this);
    output_choose_file_button->setCommandString("Output Choose File");
    output_choose_file_button->setVisible(false);
    output_passwd_label = new Label("Password", rc3, args, n);

    output_source = new Choice("data output", rc3, this);
    output_source->setSensitive(false);
#ifdef HAVE_LIBODBC
    for(int i = 0; i < num; i++) {
	output_source->addItem(dataSources[i]);
	free(dataSources[i]);
    }
    if(num) output_source->addSeparator();
    Free(dataSources);
#endif
    output_source->addItem("Flat-File-DB");
    output_source->addItem("File-Prefix");

    XtSetArg(args[0], XmNcolumns, 20);
    output_account_text = new TextField("output_account", rc3, args, 1);
    output_account_text->addActionListener(this, XmNactivateCallback);
    output_account_text->addActionListener(this, XmNvalueChangedCallback);
    output_account_text->setSensitive(false);

    output_password_text = new TextField("output_password", rc3, args, 1);
    output_password_text->addActionListener(this, XmNactivateCallback);
    output_password_text->addActionListener(this, XmNvalueChangedCallback);
    output_password_text->addActionListener(this, XmNmodifyVerifyCallback);
    output_password_text->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc3->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    rc4 = new RowColumn("rc4", output_form, args, n);

    output_connect = new Button("Connect", rc4, this);
    output_connect->setCommandString("Output Connect");
    output_connect->setSensitive(false);
    output_create_param = new Button("Create Parameter Root", rc4, this);
    output_create_param->setCommandString("Output Create Parameter Root");
    output_create_param->setSensitive(false);
    output_create_param->setVisible(false);
    output_create_seg = new Button("Create Segment Root", rc4, this);
    output_create_seg->setCommandString("Output Create Segment Root");
    output_create_seg->setSensitive(false);
    output_create_seg->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc4->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    XtSetArg(args[n], XmNskipAdjust, true); n++;
    XtSetArg(args[n], XmNwordWrap, False); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, False); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    output_connection = new TextField("output connection", output_form, args,n);

    open_prefix_file = NULL;
}

void OpenDB::makeMappingTab()
{
    Arg args[20];
    int n;
    Form *Mapping, *form, *form1;
    Separator *sep1, *sep2;

    Mapping = new Form("Mapping", tab);
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form1 = new Form("form1", Mapping, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    reset_mapping_button = new Button("Reset", form1, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, reset_mapping_button->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    make_mapping_permanent = new Button("Make Permanent", form1, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, form1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep1 = new Separator("sep1", Mapping, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form = new Form("form", Mapping, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    prefix_button = new Button("Set Prefix", form, args, n, this);
    prefix_button->setSensitive(false);
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, prefix_button->baseWidget()); n++;
    XtSetArg(args[n], XmNcolumns, 15); n++;
    prefix = new TextField("Prefix", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, prefix->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    suffix_button = new Button("Set Suffix", form, args, n, this);
    suffix_button->setSensitive(false);
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, suffix_button->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNcolumns, 15); n++;
    suffix = new TextField("Suffix", form, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep2 = new Separator("sep2", Mapping, args, n);

    const char *column_labels[2] = {"Table", "Database Name"};
    bool col_edit[2] = {false, true};
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
    XtSetArg(args[n], XtNtableTitle, "Table Name Mapping"); n++;
    XtSetArg(args[n], XtNeditable, true); n++;
    XtSetArg(args[n], XtNselectable, false); n++;
    XtSetArg(args[n], XtNcenterHorizontally, true); n++;
    XtSetArg(args[n], XtNcolumns, 2); n++;
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNcolumnEditable, col_edit); n++;
    mapping_table = new Table("mapping table", Mapping, args, n);
    mapping_table->addActionListener(this, XtNvalueChangedCallback);
    char **cssTableNames=NULL;
    int num = CssTableClass::getAllNames((const char ***)&cssTableNames);
    permanent_mapping = (char **)malloc(num*sizeof(char *));
    const char *row[2];
    string prop;
    for(int i = 0; i < num; i++) {
        row[0] = cssTableNames[i];
	if( getProperty(string("mapping.") + row[0], prop) ) {
	    row[1] = prop.c_str();
	}
	else {
	    row[1] = cssTableNames[i];
	}
	permanent_mapping[i] = strdup(row[1]);
        mapping_table->addRow(row, false);
    }
    Free(cssTableNames);
    mapping_table->adjustColumns();
    mapping_table->setVisible(true);
    setButtonsSensitive();
}

void OpenDB::makeAccountTab()
{
    Arg args[20];
    int n;
    Form *Account;
    RowColumn *rc;

    Account = new Form("Account", tab);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    rc = new RowColumn("rc", Account, args, n);

    n = 0;
    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    account_tab_label = new Label("Database Account", rc, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 15); n++;
    dbAccount = new TextField("dbAccount", rc, args, n);
    dbAccount->addActionListener(this, XmNvalueChangedCallback);
    getProperty("database.account", permanent_account);
    author_choice = new Choice("author", rc, this);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 2); n++;
    XtSetArg(args[n], XmNset, true); n++;
    writable = new Toggle("writable", rc, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    account_create = new Button("Create Author Directory", Account, args, n,
			this);
    account_create->setVisible(false);

    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, account_create->baseWidget()); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    dir_label = new Label("dir", Account, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, account_tab_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    account_permanent = new Button("Make Permanent", Account, args, n, this);
    account_permanent->setVisible(false);
    account_permanent->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, account_create->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNtableTitle, "Authors"); n++;
    XtSetArg(args[n], XtNeditable, false); n++;
    XtSetArg(args[n], XtNselectable, true); n++;
    XtSetArg(args[n], XtNcenterHorizontally, true); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XtNcolumns, 3); n++;
    const char *col_labels[] = {"Author", "First Day", "Last Day"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    author_table = new Table("author table", Account, args, n);
    author_table->setVisible(false);
}

void OpenDB::makeOriginTab()
{
    Arg args[20];
    int n;
    Label *label;
    Form *origin, *form;
    RowColumn *rc, *rc2;
    ScrolledWindow *sw;

    origin = new Form("origin", tab);

    n = 0;
//    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Origin Query Constraints", origin, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    rc = new RowColumn("rc", origin, args, n);

    new Label("Origin Id", rc);
    new Label("Min Latitude", rc);
    new Label("Max Latitude", rc);
    new Label("Min Longitude", rc);
    new Label("Max Longitude", rc);
    new Label("Min Depth", rc);
    new Label("Max Depth", rc);
    new Label("Min Defining", rc);
    new Label("Max Defining", rc);
    new Label("Min Associated", rc);
    new Label("Max Associated", rc);
    new Label("Min Time", rc);
    new Label("Max Time", rc);

    XtSetArg(args[0], XmNcolumns, 20);
    originId = new TextField("originId", rc, this, args, 1);
    originId->setCommandString("origin input");
    minLat = new TextField("minLat", rc, this, args, 1);
    minLat->setCommandString("origin input");
    maxLat = new TextField("maxLat", rc, this, args, 1);
    maxLat->setCommandString("origin input");
    minLon = new TextField("minLon", rc, this, args, 1);
    minLon->setCommandString("origin input");
    maxLon = new TextField("maxLon", rc, this, args, 1);
    maxLon->setCommandString("origin input");
    minDepth = new TextField("minDepth", rc, this, args, 1);
    minDepth->setCommandString("origin input");
    maxDepth = new TextField("maxDepth", rc, this, args, 1);
    maxDepth->setCommandString("origin input");
    minDefining = new TextField("minDefining", rc, this, args, 1);
    minDefining->setCommandString("origin input");
    maxDefining = new TextField("maxDefining", rc, this, args, 1);
    maxDefining->setCommandString("origin input");
    minNass = new TextField("minNass", rc, this, args, 1);
    minNass->setCommandString("origin input");
    maxNass = new TextField("maxNass", rc, this, args, 1);
    maxNass->setCommandString("origin input");
    minTime = new TextField("minTime", rc, this, args, 1);
    minTime->setCommandString("origin input");
    maxTime = new TextField("maxTime", rc, this, args, 1);
    maxTime->setCommandString("origin input");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form = new Form("form", origin, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    origin_query = new TextField("origin_query", sw, this, args, n);
    origin_query->tabKeyFocus();

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, sw->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    rc2 = new RowColumn("rc", form, args, n);

    n = 0;
    XmString xm = createXmString("Return Arrivals");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    origin_arrival_toggle = new Toggle("arrivalToggle", rc2, args, n);
    XmStringFree(xm);
    n = 0;
    xm = createXmString("Return Wfdiscs");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    origin_wfdisc_toggle = new Toggle("wfdiscToggle", rc2, args, n);
    XmStringFree(xm);
    use = new Choice("use", rc2, this);
    use->addItem("Use Arrivals");
    use->addItem("Use Wftags");
}

void OpenDB::makeArrivalTab()
{
    Arg args[20];
    int n;
    Label *label;
    ScrolledWindow *sw;
    Form *arrival, *form, *form2;
    RowColumn *rc;

    arrival = new Form("arrival", tab);
    n = 0;
//    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Arrival Query Constraints", arrival, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    rc = new RowColumn("rc", arrival, args, n);

    new Label("Arrival Id", rc);
    new Label("Or", rc);
    new Label("Start Time", rc);
    new Label("End Time", rc);
    new Label("Stations", rc);
    new Label("Channels", rc);

    XtSetArg(args[0], XmNcolumns, 20);
    arrivalId = new TextField("arrivalId", rc, this, args, 1);
    arrivalId->setCommandString("arid input");
    n = 0;
    XtSetArg(args[n], XmNseparatorType, XmNO_LINE); n++;
    new Separator("sep", rc, args, n);

    arrival_start_time = new TextField("start_time", rc, this);
    arrival_start_time->setCommandString("arrival input");
    arrival_end_time = new TextField("end_time", rc, this);
    arrival_end_time->setCommandString("arrival input");
    arrival_stations = new TextField("stations", rc, this);
    arrival_stations->setCommandString("arrival input");
    arrival_channels = new TextField("channels", rc, this);
    arrival_channels->setCommandString("arrival input");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form = new Form("form", arrival, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    arrival_query = new TextField("arrival_query", sw, this, args, n);
    arrival_query->tabKeyFocus();

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form2 = new Form("form2", arrival, args, n);

    n = 0;
    XmString xm = createXmString("Return Origins");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    arrival_origin_toggle = new Toggle("wfdiscToggle", form2, args, n);
    XmStringFree(xm);
    n = 0;
    xm = createXmString("Return Wfdiscs");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, arrival_origin_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    arrival_wfdisc_toggle = new Toggle("wfdiscToggle", form2, args, n);
    XmStringFree(xm);

    n = 0;
//    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Waveform Constraints", arrival, args, n);

/*
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form3 = new Form("form2", arrival, args, n);
*/
    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    rc = new RowColumn("rc", arrival, args, n);

    new Label("Time before", rc);
    new Label("Time after", rc);
    new Label("Channels", rc);
    XtSetArg(args[0], XmNcolumns, 20);
    arrival_time_before = new TextField("time_before", rc, args, 1);
    arrival_time_before->setString("60s");
    arrival_time_after = new TextField("time_after", rc, args, 1);
    arrival_time_after->setString("240s");
    waveform_channels = new TextField("waveform_channels", rc, args, 1);
}

void OpenDB::makeTimeTab()
{
    Arg args[20];
    int n;
    RowColumn *rc;
    Label *label;
    Form *time, *form1, *form2, *form3;
    ScrolledWindow *sw;

    time = new Form("time", tab);
    n = 0;
//    XtSetArg(args[n], XmNmarginHeight, 0); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Time Constraints", time, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, true); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    rc = new RowColumn("rc", time, args, n);

    new Label("Start Time", rc);
    new Label("End Time", rc);
    new Label("Stations", rc);
    new Label("Channels", rc);
    new Label("Network", rc);

    XtSetArg(args[0], XmNcolumns, 20);
    time_start_time = new TextField("time_start_time", rc, this, args, 1);
    time_start_time->setString("now - 20m");
    time_start_time->setCommandString("time input");
    time_end_time = new TextField("time_end_time", rc, this, args, 1);
    time_end_time->setString("now");
    time_end_time->setCommandString("time input");
    time_stations = new TextField("time_stations", rc, this, args, 1);
    time_stations->setString("sta like 'FIA0'");
    time_stations->setCommandString("time stations");
    time_channels = new TextField("time_channels", rc, this, args, 1);
    time_channels->setCommandString("time input");
    time_network = new TextField("time_network", rc, this, args, 1);
    time_network->setCommandString("time network");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form1 = new Form("form1", time, args, n);

    n = 0;
    XmString xm = createXmString("Return Origins");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    time_origin_toggle = new Toggle("time_origin_toggle", form1, this, args, n);
    time_origin_toggle->setCommandString("edit_time_queries");
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, time_origin_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    time_origin_query = new TextField("time_origin_query", sw, this, args, n);
    time_origin_query->setCommandString("edit_time_queries");
    time_origin_query->tabKeyFocus();

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form2 = new Form("form2", time, args, n);

    n = 0;
    xm = createXmString("Return Arrivals");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    time_arrival_toggle = new Toggle("time_arrival_toggle", form2, this,args,n);
    time_arrival_toggle->setCommandString("edit_time_queries");
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, time_arrival_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form2, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    time_arrival_query = new TextField("time_arrival_query", sw, this, args, n);
    time_arrival_query->setCommandString("edit_time_queries");
    time_arrival_query->tabKeyFocus();

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    form3 = new Form("form3", time, args, n);

    n = 0;
    xm = createXmString("Return Wfdiscs");
    XtSetArg(args[n], XmNlabelString, xm); n++;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    time_wfdisc_toggle = new Toggle("time_wfdisc_toggle", form3, this, args, n);
    time_wfdisc_toggle->setCommandString("edit_time_queries");
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, time_wfdisc_toggle->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", form3, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 5); n++;
    time_wfdisc_query = new TextField("time_wfdisc_query", sw, this, args, n);
    time_wfdisc_query->setCommandString("edit_time_queries");
    time_wfdisc_query->tabKeyFocus();

    timeFormQuery();
}

void OpenDB::makeGeneralTab()
{
    Arg args[20];
    int n;
    Form *general;
    Label *label;
    ScrolledWindow *sw;

    general = new Form("general", tab);
    n = 0;
//    XtSetArg(args[n], XmNmarginHeight, 0); n+;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Submit queries for full rows, partial rows or joins",
                        general, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 20); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_RIGHT); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAPPLICATION_DEFINED); n++;
    sw = new ScrolledWindow("sw", general, args, n);

    n = 0;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNrows, 10); n++;
    general_query = new TextField("general_query", sw, this, args, n);
}

OpenDB::~OpenDB(void)
{
    Free(permanent_mapping);
}

void OpenDB::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "tab")) {
	char *tabname = (char *)action_event->getCalldata();

	if(!strcmp(tabname, "Connection") || !strcmp(tabname, "Author") ||
		!strcmp(tabname, "Mapping"))
	{
	    run_query_button->setSensitive(false);
	}
	else if(!strcmp(tabname, "time")) {
	    turnOnTimeQuery();
	}
	else if(!strcmp(tabname, "arrival")) {
	    turnOnButton(run_query_button, arrival_query);
	}
	else if(!strcmp(tabname, "origin")) {
	    turnOnButton(run_query_button, origin_query);
	}
    }
    else if(!strcmp(cmd, "Reset")) {
	resetMapping();
    }
    else if(!strcmp(cmd, "Make Permanent")) {
	if(comp == make_mapping_permanent) {
	    saveMapping();
	}
	else {
	    saveAccount();
	}
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "account"))
    {
	if(!strcmp(action_event->getReason(), XmNactivateCallback))
	{
	    if(interface_type == ODBC_INTERFACE ||
		interface_type == FFDB_INTERFACE)
	    {
		if(password_text->isSensitive()) {
		    Widget w = password_text->baseWidget();
		    XSetInputFocus(XtDisplay(w), XtWindow(w),
			RevertToNone,CurrentTime);
		}
	    }
	    else {
		connectButton();
	    }
	}
	else {	// XmNvalueChangedCallback
	    accountInput();
	}
    }
    else if(!strcmp(cmd, "output_account"))
    {
	if(!strcmp(action_event->getReason(), XmNactivateCallback))
	{
	    if(output_interface_type == ODBC_INTERFACE ||
		output_interface_type == FFDB_INTERFACE)
	    {
		if(output_password_text->isSensitive()) {
		    Widget w = output_password_text->baseWidget();
		    XSetInputFocus(XtDisplay(w), XtWindow(w),
			RevertToNone, CurrentTime);
		}
	    }
	    else {
//		connectButton();
	    }
	}
	else {	// XmNvalueChangedCallback
	    outputAccountInput();
	}
    }
    else if(!strcmp(cmd, "password")) {
	if(interface_type == ODBC_INTERFACE) {
	    if(!strcmp(action_event->getReason(), XmNactivateCallback)) {
		connectButton();
	    }
	    else {
		maskPassword(action_event->getCalldata(), data_passwd);
	    }
	}
	else {
	    accountInput();
	}
    }
    else if(!strcmp(cmd, "output_password")) {
	if(output_interface_type == ODBC_INTERFACE) {
	    if(!strcmp(action_event->getReason(), XmNactivateCallback)) {
//		connectButton();
	    }
	    else {
		maskPassword(action_event->getCalldata(), output_data_passwd);
	    }
	}
	else {
	    outputAccountInput();
	}
    }
    else if(!strcmp(cmd, "data source")) {
	const char *src = source->getChoice();

	if(!strcmp(src, "Flat-File-DB")) {
	    setFFDBInterface();
	}
	else if(!strcmp(src, "File-Prefix")) {
	    setPrefixInterface();
	}
	else {
	    setODBCInterface();
	}
    }
    else if(!strcmp(cmd, "data output")) {
	const char *src = output_source->getChoice();

	if(!strcmp(src, "Flat-File-DB")) {
	    setOutputFFDBInterface();
	}
	else if(!strcmp(src, "File-Prefix")) {
	    setOutputPrefixInterface();
	}
	else {
	    setOutputODBCInterface();
	}
    }
    else if(!strcmp(cmd, "Prefix Path (Choose File)")) {
	char *file;
	if(!open_prefix_file){
	    open_prefix_file = new CssFileDialog("Choose File", this, FILE_ONLY,
					"./", (char *)"All Tables");
	}
	open_prefix_file->setVisible(true);

	if((file = open_prefix_file->getFile()) != NULL) {
	    // get prefix
	    int n = strlen(file);
	    for(int i = n-1; i >= 0 && file[i] != '/'; i--) {
		if(file[i] == '.') {
		    file[i] = '\0';
		    break;
		}
	    }
	    account_text->setString(file, true);
	    account_text->showPosition((int)strlen(file));
	    XtFree(file);
	}
    }
    else if(!strcmp(cmd, "Output Choose File")) {
	char *file;
	if(!open_prefix_file){
	    open_prefix_file = new CssFileDialog("Choose File", this, FILE_ONLY,
					"./", (char *)"All Tables");
	}
	open_prefix_file->setVisible(true);

	if((file = open_prefix_file->getFile()) != NULL) {
	    // get prefix
	    int n = strlen(file);
	    for(int i = n-1; i >= 0 && file[i] != '/'; i--) {
		if(file[i] == '.') {
		    file[i] = '\0';
		    break;
		}
	    }
	    output_account_text->setString(file, true);
	    output_account_text->showPosition((int)strlen(file));
	    XtFree(file);
	}
    }
    else if(!strcmp(cmd, "Create Parameter Root") ||
	    !strcmp(cmd, "Create Segment Root"))
    {
	string path;
	if(!strcmp(cmd, "Create Parameter Root")) {
	    account_text->getString(path);
	}
	else {
	    password_text->getString(path);
	}
	if(!createDir(path)) {
	    return;
	}
	comp->setSensitive(false);
	accountInput();
    }
    else if(!strcmp(cmd, "Output Create Parameter Root") ||
	    !strcmp(cmd, "Output Create Segment Root"))
    {
	string path;
	if(!strcmp(cmd, "Output Create Parameter Root")) {
	    output_account_text->getString(path);
	}
	else {
	    output_password_text->getString(path);
	}
	if(!createDir(path)) {
	    return;
	}
	comp->setSensitive(false);
	accountInput();
    }
    else if(!strcmp(cmd, "Connect")) {
	connectButton();
    }
    else if(!strcmp(cmd, "Output Connect")) {
	outputConnectButton();
    }
    else if(!strcmp(cmd, "Disconnect")) {
	disconnect();
    }
    else if(!strcmp(cmd, "Output Disconnect")) {
	outputDisconnect();
    }
    else if(!strcmp(cmd, "dbAccount")) {
	dbAccountInput();
    }
    else if(!strcmp(cmd, "origin_query")) {
	if(tabOnTop("origin")) {
	    turnOnButton(run_query_button, origin_query);
	}
    }
    else if(!strcmp(cmd, "arrival_query")) {
	if(tabOnTop("arrival")) {
	    turnOnButton(run_query_button, arrival_query);
	}
    }
    else if(!strcmp(cmd, "general_query")) {
	turnOnButton(run_query_button, general_query);
    }
    else if(!strcmp(cmd, "edit_time_queries")) {
	turnOnTimeQuery();
    }
    else if(!strcmp(cmd, "origin input")) {
	originFormQuery();
	turnOnButton(run_query_button, origin_query);
    }
    else if(!strcmp(cmd, "arrival input")) {
	arrivalFormQuery();
	turnOnButton(run_query_button, arrival_query);
    }
    else if(!strcmp(cmd, "arid input")) {
	aridFormQuery();
	turnOnButton(run_query_button, arrival_query);
    }
    else if(!strcmp(cmd, "time stations")) {
	time_network->setString("");
	timeFormQuery();
	turnOnTimeQuery();
    }
    else if(!strcmp(cmd, "time network")) {
	time_stations->setString("");
	timeFormQuery();
	turnOnTimeQuery();
    }
    else if(!strcmp(cmd, "time input")) {
	timeFormQuery();
	turnOnTimeQuery();
    }
    else if(!strcmp(cmd, "time_origin_toggle")) {
    }
    else if(!strcmp(cmd, "Set Suffix")) {
	char *s = suffix->getString();
	setMappingSuffix(s);
	Free(s);
    }
    else if(!strcmp(cmd, "Set Prefix")) {
	char *s = prefix->getString();
	setMappingPrefix(s);
	Free(s);
    }
    else if(!strcmp(cmd, "Run Query")) {
	Widget w;
	if((w = tab->getTabOnTop())) {
	    OpenDBStruct op;
	    op.tabName = XtName(w);
	    op.tmin = tmin;
	    op.tmax = tmax;
	    op.use = (char *)use->getChoice();
	    if(!op.tabName.compare("origin")) {
		op.get_origins = true;
		op.get_arrivals = origin_arrival_toggle->state();
		op.get_wfdiscs = origin_wfdisc_toggle->state();
		op.origin_query = origin_query->getString();
		op.arrival_query = strdup("");
		op.wfdisc_query = strdup("");
		op.general_query = strdup("");
	    }
	    else if(!op.tabName.compare("arrival")) {
		op.get_origins = arrival_origin_toggle->state();
		op.get_arrivals = true;
		op.get_wfdiscs = arrival_wfdisc_toggle->state();
		op.origin_query = strdup("");
		op.arrival_query = arrival_query->getString();
		op.wfdisc_query = strdup("");
		op.general_query = strdup("");
	    }
	    else if(!op.tabName.compare("time")) {
		op.get_origins = time_origin_toggle->state();
		op.get_arrivals = time_arrival_toggle->state();
		op.get_wfdiscs = time_wfdisc_toggle->state();
		op.origin_query = time_origin_query->getString();
		op.arrival_query = time_arrival_query->getString();
		op.wfdisc_query = time_wfdisc_query->getString();
		op.general_query = strdup("");
	    }
	    else if(!op.tabName.compare("general")) {
		op.get_origins = false;
		op.get_arrivals = false;
		op.get_wfdiscs = false;
		op.origin_query = strdup("");
		op.arrival_query = strdup("");
		op.wfdisc_query = strdup("");
		op.general_query = general_query->getString();
	    }

	    doCallbacks(w, &op, XmNactivateCallback);
	}
    }
    else if(!strcmp(cmd, "Save Source")) {
	saveDataSource();
	saveOutputDataSource();
    }
    else if(!strcmp(cmd, "writable")) {
	Toggle *t = comp->getToggleInstance();
	if( t && t->state() ) {
	    setAuthorWritable();
	}
    }
    else if(!strcmp(cmd, "mapping table")) {
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Prefix")) {
	char *s = prefix->getString();
	prefix_button->setSensitive((s && s[0] != '\0') ? true : false);
	Free(s);
    }
    else if(!strcmp(cmd, "Suffix")) {
	char *s = suffix->getString();
	suffix_button->setSensitive((s && s[0] != '\0') ? true : false);
	Free(s);
    }
    else if(!strcmp(cmd, "author")) {
	dbAccount->setString(author_choice->getChoice(), true);
    }
    else if(!strcmp(cmd, "Create Author Directory")) {
	char path[MAXPATHLEN+1];
	char *a = dbAccount->getString();
	snprintf(path, sizeof(path), "%s/%s", ffdb_param_root.c_str(), a);
	Free(a);
	if(createDir(path)) {
	    account_create->setSensitive(false);
//	    author_choice->addItem(authors[i]);
	    disconnect();
	    OdbcConnect();
	}
    }
    else if(!strcmp(cmd, "Use Same Connection for Output")) {
	bool use_same = output_toggle->state();
	if( use_same && output_connected ) {
	    outputDisconnect();
	}
	output_source->setSensitive(!use_same);
	output_account_text->setSensitive(!use_same);
	output_password_text->setSensitive(!use_same);
	output_connect->setSensitive(!use_same);
    }
}

ParseCmd OpenDB::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "select_tab", c)) {
	tab->setOnTop(c);
    }
    else if(parseArg(cmd, "data_source", c)) {
	if(connected) connect->activate(); // disconnects
	if( !source->setChoice(c, true, true) ) {
	    msg.assign(string("Cannot interpret data_source=") + c);
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "account", c) || parseArg(cmd, "user", c) ||
	parseArg(cmd, "prefix_path", c) || parseArg(cmd, "prefix", c) ||
	parseArg(cmd, "path", c) || parseArg(cmd, "parameter_root", c))
    {
	if(connected) connect->activate(); // disconnects
	account_text->setString(c, true);
	account_text->showPosition((int)c.length());
    }
    else if( parseArg(cmd, "password", c) )
    {
	if(connected) connect->activate(); // disconnects
	data_passwd.assign(c);
    }
    else if( parseArg(cmd, "segment_root", c))
    {
	if(connected) connect->activate(); // disconnects
	password_text->setString(c, true);
	password_text->showPosition((int)c.length());
    }
    else if(parseCompare(cmd, "connect")) {
	if(!connected) {
	    if( !connect->isSensitive() ) {
		msg.assign("Set connection parameters first.");
		return ARGUMENT_ERROR;
	    }
	    connect->activate();
	}
    }
    else if(parseCompare(cmd, "disconnect")) {
	if(connected) {
	    connect->activate(); // disconnects
	}
    }
    else if(parseCompare(cmd, "create_parameter_root")) {
	create_param->activate();
    }
    else if(parseCompare(cmd, "create_segment_root")) {
	create_seg->activate();
    }
    else if(parseArg(cmd, "mapping", c)) {
	string p;
	if(parseArg(c, "set_prefix", p)) {
	    prefix->setString(p, true);
	    prefix_button->activate();
	}
	else if(parseArg(c, "set_suffix", p)) {
	    suffix->setString(p, true);
	    suffix_button->activate();
	}
	else if(parseCompare(c, "reset")) {
	    reset_mapping_button->activate();
	}
	else if(parseCompare(c, "make_permanent")) {
	    make_mapping_permanent->activate();
	}
	else {
	    return parseMapping(c, msg);
	}
    }
    else if(parseArg(cmd, "database_account", c)) {
	dbAccount->setString(c, true);
    }
    else if(parseCompare(cmd, "account_make_permanent")) {
	account_permanent->activate();
    }
    else if(parseArg(cmd, cssOrigin, c))
    {
	string p;
	tab->setOnTop(cssOrigin);
	if(parseArg(c, "origin_id", p) || parseArg(c, "id", p)) {
	    originId->setString(p, true);
	}
	else if(parseArg(c, "min_latitude", p)) {
	    minLat->setString(p, true);
	}
	else if(parseArg(c, "max_latitude", p)) {
	    maxLat->setString(p, true);
	}
	else if(parseArg(c, "min_longitude", p)) {
	    minLon->setString(p, true);
	}
	else if(parseArg(c, "max_longitude", p)) {
	    maxLon->setString(p, true);
	}
	else if(parseArg(c, "min_depth", p)) {
	    minDepth->setString(p, true);
	}
	else if(parseArg(c, "max_depth", p)) {
	    maxDepth->setString(p, true);
	}
	else if(parseArg(c, "min_defining", p)) {
	    minDefining->setString(p, true);
	}
	else if(parseArg(c, "max_defining", p)) {
	    maxDefining->setString(p, true);
	}
	else if(parseArg(c, "min_associated", p)) {
	    minNass->setString(p, true);
	}
	else if(parseArg(c, "max_associated", p)) {
	    maxNass->setString(p, true);
	}
	else if(parseArg(c, "min_time", p)) {
	    minTime->setString(p, true);
	}
	else if(parseArg(c, "max_time", p)) {
	    maxTime->setString(p, true);
	}
	else if(parseArg(c, "return_arrivals", p)) {
	    return origin_arrival_toggle->parseCmd(c, msg);
	}
	else if(parseArg(c, "return_wfdiscs", p)) {
	    return origin_wfdisc_toggle->parseCmd(c, msg);
	}
	else if(parseCompare(c, "use_arrivals")) {
	    use->setChoice("Use Arrivals", true);
	}
	else if(parseCompare(c, "use_wftags")) {
	    use->setChoice("Use Wftags", true);
	}
	else if(parseCompare(c, "run_query")) {
	    tab->setOnTop(cssOrigin);
	    run_query_button->activate();
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "arrival", c))
    {
	string p;
	tab->setOnTop("arrival");
	if(parseArg(c, "arrival_id", p) || parseArg(c, "id", p)) {
	    arrivalId->setString(p, true);
	}
	else if(parseArg(c, "start_time", p)) {
	    arrival_start_time->setString(p, true);
	}
	else if(parseArg(c, "end_time", p)) {
	    arrival_end_time->setString(p, true);
	}
	else if(parseArg(c, "stations", p)) {
	    arrival_stations->setString(p, true);
	}
	else if(parseArg(c, "channels", p)) {
	    arrival_channels->setString(p, true);
	}
	else if(parseArg(c, "return_origins", p)) {
	    return arrival_origin_toggle->parseCmd(p, msg);
	}
	else if(parseArg(c, "return_wfdiscs", p)) {
	    return arrival_wfdisc_toggle->parseCmd(p, msg);
	}
	else if(parseArg(c, "time_before", p)) {
	    arrival_time_before->setString(p, true);
	}
	else if(parseArg(c, "time_after", p)) {
	    arrival_time_after->setString(p, true);
	}
	else if(parseArg(c, "waveform_channels", p)) {
	    waveform_channels->setString(p, true);
	}
	else if(parseCompare(c, "run_query")) {
	    tab->setOnTop("arrival");
	    run_query_button->activate();
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseArg(cmd, "time", c))
    {
	string p;
	tab->setOnTop("time");
	if(parseArg(c, "start_time", p)) {
	    time_start_time->setString(p, true);
	}
	else if(parseArg(c, "end_time", p)) {
	    time_end_time->setString(p, true);
	}
	else if(parseArg(c, "stations", p)) {
	    time_stations->setString(p, true);
	}
	else if(parseArg(c, "channels", p)) {
	    time_channels->setString(p, true);
	}
	else if(parseArg(c, "network", p)) {
	    time_network->setString(p, true);
	}
	else if(parseArg(c, "return_origins", p)) {
	    return time_origin_toggle->parseCmd(p, msg);
	}
	else if(parseArg(c, "return_arrivals", p)) {
	    return time_arrival_toggle->parseCmd(p, msg);
	}
	else if(parseArg(c, "return_wfdiscs", p)) {
	    return time_wfdisc_toggle->parseCmd(p, msg);
	}
	else if(parseCompare(c, "run_query")) {
	    tab->setOnTop("time");
	    run_query_button->activate();
	}
	else {
	    return ARGUMENT_ERROR;
	}
    }
    else if(parseCompare(cmd, "run_query")) {
	run_query_button->activate();
    }
    else if(parseArg(cmd, "save_source", c)) {
	return save_source->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "writable", c)) {
	return writable->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "help")) {
	char file_prefix[200];
	getParsePrefix(file_prefix, sizeof(file_prefix));
	parseHelp(file_prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd OpenDB::parseMapping(const string &cmd, string &msg)
{
    string c;
    vector<const char *> table_name;
    int i;

    mapping_table->getColumn(0, table_name);
    for(i = 0; i < (int)table_name.size(); i++) {
	if(parseArg(cmd, table_name[i], c)) {
	    mapping_table->setField(i, 1, c);
	    break;
	}
    }
    if(i == (int)table_name.size()) {
	msg.assign(string("mapping: ") + cmd + " not recognised");
    }
    return (i < (int)table_name.size()) ? COMMAND_PARSED : ARGUMENT_ERROR;
}

void OpenDB::parseHelp(const char *file_prefix)
{
    printf("%saccount ACCOUNT\n", file_prefix);
    printf("%saccount_make_permanent\n", file_prefix);
    printf("%screate_parameter_root\n", file_prefix);
    printf("%screate_segment_root\n", file_prefix);
    printf("%sconnect\n", file_prefix);
    printf("%sdatabase_account=ACCOUNT\n", file_prefix);
    printf("%sdata_source=DATA_SOURCE\n", file_prefix);
    printf("%sdisconnect\n", file_prefix);
    printf("%sselect_tab=TABNAME\n", file_prefix);

    printf("%sparameter_root=PARAM_ROOT\n", file_prefix);
    printf("%spassword=PASSWORD\n", file_prefix);
    printf("%sprefix_path=PATH\n", file_prefix);

    printf("%smapping.set_prefix=PREFIX\n", file_prefix);
    printf("%smapping.set_suffix=SUFFIX\n", file_prefix);
    printf("%smapping.reset\n", file_prefix);
    printf("%smapping.make_permanent\n", file_prefix);
    printf("%smapping.[table commands]\n", file_prefix);

    printf("%ssegment_root=SEG_ROOT\n", file_prefix);
    printf("%suser=USER\n", file_prefix);

    printf("%sarrival.arrival_id=ID\n", file_prefix);
    printf("%sarrival.start_time=TIME\n", file_prefix);
    printf("%sarrival.end_time=TIME\n", file_prefix);
    printf("%sarrival.stations=STATIONS\n", file_prefix);
    printf("%sarrival.channels=CHANNELS\n", file_prefix);
    printf("%sarrival.return_origins=(true,false)\n", file_prefix);
    printf("%sarrival.return_wfdiscs=(true,false)\n", file_prefix);
    printf("%sarrival.time_before=TIME\n", file_prefix);
    printf("%sarrival.time_after=TIME\n", file_prefix);
    printf("%sarrival.waveform_channels=CHANNELS\n", file_prefix);
    printf("%sarrival.run_query\n", file_prefix);

    printf("%sorigin.origin_id=ID\n", file_prefix);
    printf("%sorigin.min_latitude=LAT\n", file_prefix);
    printf("%sorigin.max_latitude=LAT\n", file_prefix);
    printf("%sorigin.min_longitude=LON\n", file_prefix);
    printf("%sorigin.max_longitude=LON\n", file_prefix);
    printf("%sorigin.min_depth=DEPTH\n", file_prefix);
    printf("%sorigin.max_depth=DEPTH\n", file_prefix);
    printf("%sorigin.min_defining=NUM\n", file_prefix);
    printf("%sorigin.max_defining=NUM\n", file_prefix);
    printf("%sorigin.min_associated=NUM\n", file_prefix);
    printf("%sorigin.max_associated=NUM\n", file_prefix);
    printf("%sorigin.min_time=TIME\n", file_prefix);
    printf("%sorigin.max_time=TIME\n", file_prefix);
    printf("%sorigin.return_arrivals=(true,false)\n", file_prefix);
    printf("%sorigin.return_wfdiscs=(true,false)\n", file_prefix);
    printf("%sorigin.use_arrivals\n", file_prefix);
    printf("%sorigin.use_wftags\n", file_prefix);
    printf("%sorigin.run_query\n", file_prefix);

    printf("%stime.start_time=TIME\n", file_prefix);
    printf("%stime.end_time=TIME\n", file_prefix);
    printf("%stime.stations=STATIONS\n", file_prefix);
    printf("%stime.channels=CHANNELS\n", file_prefix);
    printf("%stime.return_origins=(true,false)\n", file_prefix);
    printf("%stime.return_arrivals=(true,false)\n", file_prefix);
    printf("%stime.return_wfdiscs=(true,false)\n", file_prefix);
    printf("%stime.run_query\n", file_prefix);

    printf("%ssave_source=(true,false)\n", file_prefix);
    printf("%swritable=(true,false)\n", file_prefix);
}

void OpenDB::setODBCInterface(void)
{
    account_label->setLabel("Account");
    account_label->setVisible(true);
    choose_file_button->setVisible(false);

    passwd_label->setLabel("Password");
    passwd_label->setSensitive(true);

    account_text->setString("");
    account_text->setVisible(true);
    password_text->setString("");
    password_text->setSensitive(true);
    data_passwd[0] = '\0';

    tab->setLabel("Author", "Account");

    account_tab_label->setLabel("Database Account");
    account_permanent->setVisible(true);
    account_permanent->setSensitive(false);

    author_choice->setVisible(false);
    writable->setVisible(false);
    account_create->setVisible(false);
    dir_label->setVisible(false);

    connect->setSensitive(false);

    create_param->setVisible(false);
    create_seg->setVisible(false);

    if(interface_type == FFDB_INTERFACE) {
	dbAccount->setString("");
    }

    interface_type = ODBC_INTERFACE;
}

void OpenDB::setOutputODBCInterface(void)
{
    output_account_label->setLabel("Account");
    output_account_label->setVisible(true);
    output_choose_file_button->setVisible(false);

    output_passwd_label->setLabel("Password");
    output_passwd_label->setSensitive(true);

    output_account_text->setString("");
    output_account_text->setVisible(true);
    output_password_text->setString("");
    output_password_text->setSensitive(true);
    output_data_passwd[0] = '\0';

/*
    tab->setLabel("Author", "Account");

    account_tab_label->setLabel("Database Account");
    account_permanent->setVisible(true);
    account_permanent->setSensitive(false);

    author_choice->setVisible(false);
    writable->setVisible(false);
    account_create->setVisible(false);
    dir_label->setVisible(false);
*/
    output_connect->setSensitive(false);

    output_create_param->setVisible(false);
    output_create_seg->setVisible(false);

    if(output_interface_type == FFDB_INTERFACE) {
//	dbAccount->setString("");
    }

    output_interface_type = ODBC_INTERFACE;
}

void OpenDB::setFFDBInterface(void)
{       
    interface_type = FFDB_INTERFACE;
        
    account_label->setLabel("Parameter Root");
    account_label->setVisible(true);
    choose_file_button->setVisible(false);
        
    connect->setSensitive(false);
        
    create_param->setVisible(true);
    create_param->setSensitive(false);

    create_seg->setVisible(true);
    create_seg->setSensitive(false);
        
    passwd_label->setLabel("Segment Root");
    passwd_label->setSensitive(true);
        
    account_text->setString("");

    password_text->setSensitive(true);
    password_text->setString("");

    tab->setLabel("Account", "Author");

    account_tab_label->setLabel("Working Author");

    account_permanent->setVisible(false);
    author_choice->setVisible(true);
    writable->setVisible(true);
    account_create->setVisible(true);

    dir_label->setVisible(true);
}

void OpenDB::setOutputFFDBInterface(void)
{       
    output_interface_type = FFDB_INTERFACE;
        
    output_account_label->setLabel("Parameter Root");
    output_account_label->setVisible(true);
    output_choose_file_button->setVisible(false);

    output_connect->setSensitive(false);
        
    output_create_param->setVisible(true);
    output_create_param->setSensitive(false);

    output_create_seg->setVisible(true);
    output_create_seg->setSensitive(false);
        
    output_passwd_label->setLabel("Segment Root");
    output_passwd_label->setSensitive(true);
        
    output_account_text->setString("");

    output_password_text->setSensitive(true);
    output_password_text->setString("");

/*
    tab->setLabel("Account", "Author");

    account_tab_label->setLabel("Working Author");

    account_permanent->setVisible(false);
    author_choice->setVisible(true);
    writable->setVisible(true);
    account_create->setVisible(true);

    dir_label->setVisible(true);
*/
}

void OpenDB::setPrefixInterface(void)
{
    interface_type = PREFIX_INTERFACE;

    account_label->setVisible(false);
    choose_file_button->setVisible(true);

    account_text->setString("");

    connect->setSensitive(false);

    create_param->setVisible(false);
    create_seg->setVisible(false);
    passwd_label->setSensitive(false);
    password_text->setSensitive(false);
}

void OpenDB::setOutputPrefixInterface(void)
{
    output_interface_type = PREFIX_INTERFACE;

    output_account_label->setVisible(false);
    output_choose_file_button->setVisible(true);

    output_account_text->setString("");
    output_connect->setSensitive(false);

    output_create_param->setVisible(false);
    output_create_seg->setVisible(false);
    output_passwd_label->setSensitive(false);
    output_password_text->setSensitive(false);
}

void OpenDB::accountInput(void)
{
//    InfoSetText(info, "");

    if(interface_type == ODBC_INTERFACE) {
	connect->setSensitive(true);
    }
    else if(interface_type == FFDB_INTERFACE) {
	char *param_root, *seg_root;
	struct stat buf;
        bool param_exists, seg_exists;
        
	param_root = account_text->getString();
	seg_root = password_text->getString();
	param_exists = (!stat(param_root, &buf) && S_ISDIR(buf.st_mode));
	seg_exists = (!stat(seg_root, &buf) && S_ISDIR(buf.st_mode));
	create_param->setSensitive(!param_exists);
	create_seg->setSensitive(!seg_exists);

	if(param_exists && seg_exists) { 
	    connect->setSensitive(true);
	}
	else {
	    connect->setSensitive(false);
	}
	free(param_root);
	free(seg_root);
    }
    else // PREFIX_INTERFACE
    {
	char *file_prefix = account_text->getString();
	if(file_prefix[0] != '\0') {
	    connect->setSensitive(true);
	}
	else {
	    connect->setSensitive(false);
	}
	Free(file_prefix);
    }
}

void OpenDB::outputAccountInput(void)
{
    if(output_interface_type == ODBC_INTERFACE) {
	output_connect->setSensitive(true);
    }
    else if(output_interface_type == FFDB_INTERFACE) {
	char *param_root, *seg_root;
	struct stat buf;
        bool param_exists, seg_exists;
        
	param_root = output_account_text->getString();
	seg_root = output_password_text->getString();
	param_exists = (!stat(param_root, &buf) && S_ISDIR(buf.st_mode));
	seg_exists = (!stat(seg_root, &buf) && S_ISDIR(buf.st_mode));
	output_create_param->setSensitive(!param_exists);
	output_create_seg->setSensitive(!seg_exists);

	if(param_exists && seg_exists) { 
	    output_connect->setSensitive(true);
	}
	else {
	    output_connect->setSensitive(false);
	}
	free(param_root);
	free(seg_root);
    }
    else // PREFIX_INTERFACE
    {
	char *file_prefix = output_account_text->getString();
	if(file_prefix[0] != '\0') {
	    output_connect->setSensitive(true);
	}
	else {
	    output_connect->setSensitive(false);
	}
	Free(file_prefix);
    }
}

void OpenDB::maskPassword(XtPointer data, string &passwd)
{
    char pwd[200];
    XmTextVerifyCallbackStruct *c = (XmTextVerifyCallbackStruct *)data;
    char *s, key = '\0', buf[200];
    int i, j, n, m, len;

    len = sizeof(pwd);
    snprintf(pwd, sizeof(pwd), "%s", passwd.c_str());

    if(c->event != NULL && c->event->type == KeyPress) {
	XLookupString((XKeyEvent *)c->event, &key, 1, NULL, NULL);
    }

    if(c->reason == XmCR_MODIFYING_TEXT_VALUE)
    {
	n = (int)strlen(pwd);
	if(key == '\b') {
	    if(c->startPos >= 0 && c->startPos <= n &&
                    c->endPos >= 0 && c->endPos <= n)
	    {
		int max = len-1;
		for(i = c->startPos, j = c->endPos;
			i < max && j < max && j < n; i++, j++) {
		    pwd[i] = pwd[j];
		}
		n -= (c->endPos-c->startPos);
		if(n < 0) n = 0;
	    }
	}
	else if(c->startPos >= 0 && c->startPos <= n) {
	    int max = len-1;
	    s = c->text->ptr;
	    stringcpy(buf, pwd, sizeof(buf)); 
	    for(i = 0; s[i] != '\0' && c->startPos+i < max; i++, n++)
	    {   
		pwd[c->startPos+i] = s[i];
		s[i] = '*';
	    }
	    pwd[c->startPos+i] = '\0';
	    m = strlen(pwd) + strlen(buf+c->startPos);
	    if(m < len-1) {
		strcat(pwd, buf+c->startPos);
	    }
	}
	pwd[n] = '\0';
	passwd.assign(pwd);
    }
}

bool OpenDB::createDir(const string &path)
{       
    char *c, *last, *tok, *p;
    char dir[MAXPATHLEN+1];
    struct stat buf;

    p = strdup(path.c_str());
    stringTrim(p);

    dir[0] = (p[0] == '/') ? '/' : '\0';
    tok = p;
    while((c = strtok_r(tok, "/", &last)) != NULL)
    {
	tok = NULL;
	if(strlen(dir) + strlen(c) + 1 > MAXPATHLEN) {
	    showWarning("createDir: path too long: %s", path.c_str());
	    return false;
	}
	strcat(dir, c);
	strcat(dir, "/");
	if(!stat(dir, &buf)) {
	    if(!S_ISDIR(buf.st_mode)) {
	       showWarning("Cannot create %s exists and is not a directory",
			path.c_str());
		free(p);
		return false;
	    }
	}
	else {
	    if(mkdir(dir, 0755)) {
		showWarning("Cannot create %s\n%s", dir, strerror(errno));
		free(p);
		return false;
	    }
	}
    }
    free(p);
    return true;
}

bool OpenDB::connectButton(void)
{
    if( !OdbcConnect() ) return false;

    if(save_source->state()) {
	saveDataSource();
    }
    return true;
}

void OpenDB::outputConnectButton(void)
{
    if(OdbcConnectOutput() && save_source->state()) {
	saveOutputDataSource();
    }
}

void OpenDB::saveDataSource(void)
{
    if(interface_type == FFDB_INTERFACE) {
	putProperty("data_source", data_source);
	putProperty("parameter_root", ffdb_param_root);
	putProperty("segment_root", ffdb_seg_root);
    }
    else if(interface_type == PREFIX_INTERFACE) {
	putProperty("data_source", data_source);
	putProperty("file_prefix", ffdb_file_prefix);
    }
    else if(interface_type == ODBC_INTERFACE) {
	char c[100], *s;
	snprintf(c, sizeof(c), "ODBC:%s", source->getChoice());
	putProperty("data_source", c);
	s = account_text->getString();
	putProperty("odbc_user", s);
	free(s);
	putProperty("odbc_password", data_passwd);
    }
    Application::writeApplicationProperties();
}

void OpenDB::saveOutputDataSource(void)
{
    if(output_toggle->state()) {
	putProperty("use_same_output", "true");
    }
    else {
	putProperty("use_same_output", "false");
    }

    if(output_interface_type == FFDB_INTERFACE) {
	putProperty("output_data_source", output_data_source);
	putProperty("output_parameter_root", output_ffdb_param_root);
	putProperty("output_segment_root", output_ffdb_seg_root);
    }
    else if(output_interface_type == PREFIX_INTERFACE) {
	putProperty("output_data_source", output_data_source);
	putProperty("output_file_prefix", output_ffdb_file_prefix);
    }
    else if(output_interface_type == ODBC_INTERFACE) {
	char c[100], *s;
	snprintf(c, sizeof(c), "ODBC:%s", output_source->getChoice());
	putProperty("output_data_source", c);
	s = account_text->getString();
	putProperty("output_odbc_user", s);
	free(s);
	putProperty("output_odbc_password", output_data_passwd);
    }
    Application::writeApplicationProperties();
}

bool OpenDB::OdbcConnect(void)
{
    string dataSource, user, param_root, seg_root, file_prefix;
    char text[1024];
#ifdef HAVE_LIBODBC
    int auto_commit = 0;
#endif

//	clearAllTables(qt);

    if(interface_type == ODBC_INTERFACE)
    {
	source->getChoice(dataSource);
	account_text->getString(user);

#ifdef HAVE_LIBODBC
	if( !(hdbc = ODBCConnect(dataSource, user, data_passwd, auto_commit)) )
	{
	    showWarning("%s", ODBCErrMsg());
	    return false;
	}
	if(tq) {
	    tq->conn_handle.connection_type = ODBC_CONNECTION;
	    tq->conn_handle.hdbc = hdbc;
	    tq->conn_handle.data_source = dataSource;
	    tq->conn_handle.user = user;
	    tq->conn_handle.data_passwd = data_passwd;
	}
#endif
	turnOnTabs(true, true);
    }
    else if(interface_type == FFDB_INTERFACE) {
	account_text->getString(param_root);
	password_text->getString(seg_root);
	if( !(ffdb = FFDatabase::FFDBOpen(param_root, seg_root,
			directory_structure, directory_duration)) )
	{
	    showWarning("%s", FFDatabase::FFDBErrMsg());
	    return false;
	}
	if(tq) {
	    tq->conn_handle.connection_type = FFDB_CONNECTION;
	    tq->conn_handle.ffdb = ffdb;
	}
	turnOnTabs(true, false);
    }
    else {
	account_text->getString(file_prefix);
        int i;
        for(i = (int)file_prefix.length()-1; i >= 0; i--) {
            if(file_prefix[i] == '.' &&
			CssTableClass::isTableName(file_prefix.substr(i+1))) {
		file_prefix = file_prefix.substr(0, i);
		break;
	    }
        }

	if((ffdb = FFDatabase::FFDBOpenPrefix(file_prefix)) == NULL)
	{
	    showWarning("%s", FFDatabase::FFDBErrMsg());
	    return false;
	}
	if(tq) {
	    tq->conn_handle.connection_type = PREFIX_CONNECTION;
	    tq->conn_handle.ffdb = ffdb;
	    tq->conn_handle.prefix = file_prefix;
	}
	turnOnTabs(true, false);
	tab->setSensitive("Mapping", false);
	tab->setSensitive("Author", false);
    }
    connect->setLabel("Disconnect");
    connect->setSensitive(true);
    connect->setCommandString("Disconnect");

    source->setSensitive(false);
    choose_file_button->setSensitive(false);
    account_text->setSensitive(false);
    password_text->setSensitive(false);

    int i;
    char title[MAXPATHLEN+500];

    if(interface_type == ODBC_INTERFACE) {
	snprintf(text, sizeof(text),
	    "Connected\n\nData Source:\t%s\nAccount:\t\t%s\nPassword:",
		dataSource.c_str(), user.c_str());
	snprintf(title, sizeof(title), "%s: %s", dataSource.c_str(),
		user.c_str());
    }
    else if(interface_type == FFDB_INTERFACE) {
	snprintf(text, sizeof(text),
		"Connected\n\nParam Root:\t%s\nSeg Root:\t\t%s",
		param_root.c_str(), seg_root.c_str());
	for(i=(int)param_root.length()-1; i >= 0 && param_root[i] != '/'; i--);
	if(i >= 0) {
	    i++;
	    snprintf(title, sizeof(title), "%s (%s)", param_root.c_str()+i,
			param_root.c_str());
	}
	else {
	    snprintf(title, sizeof(title), "%s", param_root.c_str());
	}
    }
    else {
	snprintf(text, 1024,"Connected\n\nFile-Prefix: %s",file_prefix.c_str());
	for(i=(int)file_prefix.length()-1; i > 0 && file_prefix[i] != '/'; i--);
	if(i >= 0) {
	    i++;
	    snprintf(title, sizeof(title), "%s (%s)",
		file_prefix.c_str()+i, file_prefix.c_str());
	}
	else {
	    snprintf(title, sizeof(title), "%s", file_prefix.c_str());
	}
    }
    connection->setString(text);

    if(tq) {
	tq->setTitle(title);
    }

    if(interface_type == ODBC_INTERFACE)
    {
	data_source = dataSource;
	data_account = user;
	dbAccount->setString(permanent_account);
	putProperty("dbAccount", permanent_account, false);
    }
    else if(interface_type == FFDB_INTERFACE)
    {
	data_source.assign("Flat-File-DB");
	ffdb_param_root = param_root;
	ffdb_seg_root = seg_root;
//	qt->directory_duration = ffdb_data.directory_duration;
//	qt->directory_structure = strdup(ffdb_data.directory_structure);
	flatFileConnect();
    }
    else {
	data_source.assign("File-Prefix");
	ffdb_file_prefix = file_prefix;
	dbAccount->setString("");
    }
    connected = true;
    doCallbacks(base_widget, (XtPointer)connected, XtNconnectionCallback);
    return true;
}

bool OpenDB::OdbcConnectOutput(void)
{
    string dataSource, user, param_root, seg_root, file_prefix;
    char text[1024];
#ifdef HAVE_LIBODBC
    int auto_commit = 0;
#endif

//	clearAllTables(qt);

    if(output_interface_type == ODBC_INTERFACE)
    {
	output_source->getChoice(dataSource);
	output_account_text->getString(user);

#ifdef HAVE_LIBODBC
	if( !(output_hdbc = ODBCConnect(dataSource, user, output_data_passwd,
				auto_commit)) )
	{
	    showWarning("%s", ODBCErrMsg());
	    return false;
	}
#endif
//	turnOnTabs(true, true);
    }
    else if(output_interface_type == FFDB_INTERFACE) {
	output_account_text->getString(param_root);
	output_password_text->getString(seg_root);
	if((output_ffdb = FFDatabase::FFDBOpen(param_root, seg_root,
			directory_structure, directory_duration)) == NULL)
	{
	    showWarning("%s", FFDatabase::FFDBErrMsg());
	    return false;
	}
//	turnOnTabs(true, false);
    }
    else {
	output_account_text->getString(file_prefix);
	if((output_ffdb = FFDatabase::FFDBOpenPrefix(file_prefix)) == NULL)
	{
	    showWarning("%s", FFDatabase::FFDBErrMsg());
	    return false;
	}
//	turnOnTabs(true, false);
//	tab->setSensitive("Mapping", false);
//	tab->setSensitive("Author", false);
    }
    output_connect->setLabel("Disconnect");
    output_connect->setSensitive(true);
    output_connect->setCommandString("Output Disconnect");

    output_source->setSensitive(false);
    output_choose_file_button->setSensitive(false);
    output_account_text->setSensitive(false);
    output_password_text->setSensitive(false);

    int i;
    char title[MAXPATHLEN+500];

    if(output_interface_type == ODBC_INTERFACE) {
	snprintf(text, sizeof(text),
	    "Connected\n\nData Source:\t%s\nAccount:\t\t%s\nPassword:",
		dataSource.c_str(), user.c_str());
	snprintf(title, sizeof(title), "%s: %s", dataSource.c_str(),
		user.c_str());
    }
    else if(output_interface_type == FFDB_INTERFACE) {
	snprintf(text, sizeof(text),
		"Connected\n\nParam Root:\t%s\nSeg Root:\t\t%s",
		param_root.c_str(), seg_root.c_str());
	for(i=(int)param_root.length()-1; i >= 0 && param_root[i] != '/'; i--);
	if(i >= 0) {
	    i++;
	    snprintf(title, sizeof(title), "%s (%s)", param_root.c_str()+i,
			param_root.c_str());
	}
	else {
	    snprintf(title, sizeof(title), "%s", param_root.c_str());
	}
    }
    else {
	snprintf(text, sizeof(text), "Connected\n\nFile-Prefix: %s",
		file_prefix.c_str());
	for(i=(int)file_prefix.length()-1; i > 0 && file_prefix[i] != '/'; i--);
	if(i >= 0) {
	    i++;
	    snprintf(title, sizeof(title), "%s (%s)", file_prefix.c_str()+i,
			file_prefix.c_str());
	}
	else {
	    snprintf(title, sizeof(title), "%s", file_prefix.c_str());
	}
    }
    output_connection->setString(text);

    if(output_interface_type == ODBC_INTERFACE)
    {
	output_data_source = dataSource;
	output_data_account = user;
//	dbAccount->setString(permanent_account);
    }
    else if(output_interface_type == FFDB_INTERFACE)
    {
	output_data_source.assign("Flat-File-DB");
	output_ffdb_param_root = param_root;
	output_ffdb_seg_root = seg_root;
//	flatFileConnect();
    }
    else {
	output_data_source.assign("File-Prefix");
	output_ffdb_file_prefix = file_prefix;
//	dbAccount->setString("");
    }
    output_connected = true;
//    doCallbacks(base_widget, (XtPointer)connected, XtNconnectionCallback);
    return true;
}

void OpenDB::flatFileConnect(void)
{
    int i, num_authors;
    char **authors = NULL;
    const char *default_author = NULL;

    author_choice->removeAllChoices();

    if(!working_author.empty())
    {
	ffdb->setDefaultAuthor(working_author.c_str());
    }
    num_authors = ffdb->getAuthors(&authors);
    if(num_authors > 0)
    {
	author_choice->setVisible(false);
	if((default_author = ffdb->defaultAuthor()) != NULL)
	{
	    author_choice->addItem(default_author);
//		SetAuthorCB(button, NULL, NULL);
	    writable->set(true, false);
	}
	for(i = 0; i < num_authors; i++)
	    if(strcmp(authors[i], default_author))
	{
	    author_choice->addItem(authors[i]);
	}
	for(i = 0; i < num_authors; i++) free(authors[i]);
	Free(authors);
	if(default_author && working_author.empty()) {
	    working_author.assign(default_author);
	    dbAccount->setString(working_author);
	}
	author_choice->setVisible(true);
    }
    else {
	author_choice->setVisible(false);
	if( !working_author.empty() ) {
	    /* display working author in text field */
	    dbAccount->setString(working_author);
	}
	else {
	    dbAccount->setString("");
	}
    }
    dbAccountInput();
}

void OpenDB::disconnect(void)
{
    if( !connected ) return;

    if(interface_type == ODBC_INTERFACE) {
#ifdef HAVE_LIBODBC
/*
	for(i = 0; i < num_query_tables; i++)
	{
	    if(&query_tables[i] != qt && query_tables[i].hdbc == qt->hdbc) {
		    break;
	    }
	}
	if(i == num_query_tables) {
*/
	    ODBCDisconnect(hdbc);
	data_passwd[0] = '\0';
//	}
	hdbc = NULL;
#endif
    }
    else if(ffdb) {
	delete ffdb;
	ffdb = NULL;
    }

    source->setSensitive(true);
    choose_file_button->setSensitive(true);
    account_text->setSensitive(true);
    password_text->setSensitive(true);
    connection->setString("");

    connect->setLabel("Connect");
    connect->setCommandString("Connect");
    connect->setSensitive(true);

    turnOnTabs(false, false);

    run_query_button->setSensitive(false);

    if(tq) {
	tq->setTitle("TableQuery");
    }

    connected = false;
    no_sites = no_sitechans = no_affiliations = false;
    doCallbacks(base_widget, (XtPointer)connected, XtNconnectionCallback);
}

void OpenDB::outputDisconnect(void)
{
    if(output_interface_type == ODBC_INTERFACE) {
#ifdef HAVE_LIBODBC
	ODBCDisconnect(output_hdbc);
	output_data_passwd.clear();
	output_hdbc = NULL;
#endif
    }
    else if(output_ffdb) {
	delete output_ffdb;
	output_ffdb = NULL;
    }

    output_source->setSensitive(true);
    output_choose_file_button->setSensitive(true);
    output_account_text->setSensitive(true);
    output_password_text->setSensitive(true);
    output_connection->setString("");

    output_connect->setLabel("Connect");
    output_connect->setCommandString("Output Connect");
    output_connect->setSensitive(true);

//    turnOnTabs(false, false);

    output_connected = false;
}

void OpenDB::turnOnTabs(bool state, bool general_tab)
{       
    char **tab_labels;

    int num = tab->getLabels(&tab_labels);
                
    /* turn on/off all tabs except the Connection tab and optionally the
     * general tab
     */ 
    for(int i = 0; i < num; i++)
    {   
	if( !strcmp(tab_labels[i], "general") || 
	    !strcmp(tab_labels[i], "Tables"))
	{   
	    if(!state || general_tab) {
		tab->setSensitive(tab_labels[i], state);
	    }
	}
	else if(strcmp(tab_labels[i], "Connection")) {
	    tab->setSensitive(tab_labels[i], state);
	}
    }
    free(tab_labels);
}

void OpenDB::turnOnButton(Button *b, TextField *textField)
{
    string text;

    textField->getString(text);
    if( !text.empty() ) {
	b->setSensitive(true);
    }
    else {
	b->setSensitive(false);
    }
}

void OpenDB::turnOnTimeQuery(void)
{
    string text;
    bool on = false;

    if(!tabOnTop("time")) return;

    if(time_origin_toggle->state()) {
	time_origin_query->getString(text);
	if(!text.empty()) on = true;
    }

    if(time_arrival_toggle->state()) {
	time_arrival_query->getString(text);
	if(!text.empty()) on = true;
    }

    if(time_wfdisc_toggle->state()) {
	time_wfdisc_query->getString(text);
	if(!text.empty()) on = true;
    }

    run_query_button->setSensitive(on);
}

void OpenDB::dbAccountInput(void)
{
    char path[MAXPATHLEN+20];
    string author;

    if(interface_type == ODBC_INTERFACE)
    {
	timeFormQuery();
	originFormQuery();
	if(last_arid_input) {
	    aridFormQuery();
	}
	else {
	    arrivalFormQuery();
	}
	string s;
	dbAccount->getString(s);
	if( s.compare(permanent_account) ) {
	    account_permanent->setSensitive(true);
	    putProperty("dbAccount", s, false);
	}
 	else {
	    account_permanent->setSensitive(false);
	}
	return;
    }
    else if(!ffdb) {
	return;
    }
//    dbAccount->setString("");

/** use getcwd(path, MAXPATHLEN+1) if param_root[0] != '/' */

    if(dbAccount->getString(author) && !author.empty())
    {
	int i, num_authors;
	char **authors = NULL;

	num_authors = ffdb->getAuthors(&authors);
	for(i = 0; i < num_authors && author.compare(authors[i]); i++);
	if(i == num_authors) {
	    account_create->setSensitive(true);
	}
	else {
	    account_create->setSensitive(false);
	    author_choice->setChoice(authors[i]);
	}
	for(i = 0; i < num_authors; i++) free(authors[i]);
	if(authors) free(authors);
	snprintf(path, MAXPATHLEN+20,"path: %s/%s",
			ffdb_param_root.c_str(), author.c_str());
    }
    else {
	account_create->setSensitive(true);
	stringcpy(path, "path:", MAXPATHLEN+1);
    }

    dir_label->setLabel(path);
}

void OpenDB::timeFormQuery(void)
{
    char query[MAX_Q_LEN] = "";

    if(!originsFromTime(query, MAX_Q_LEN)) return;

    time_origin_query->setString(query);

    if(!arrivalsFromTime("time", query, MAX_Q_LEN)) return;

    time_arrival_query->setString(query);

    if(!wfdiscsFromTime(query, MAX_Q_LEN, &tmin, &tmax)) return;

    time_wfdisc_query->setString(query);
}

void OpenDB::aridFormQuery(void)
{
    char query[MAX_Q_LEN];

    if(arrivalsFromArid(query, MAX_Q_LEN)) {
	arrival_query->setString(query);
    }
}

void OpenDB::arrivalFormQuery(void)
{
    char query[MAX_Q_LEN];

    if(arrivalsFromTime("arrival", query, MAX_Q_LEN)) {
	arrival_query->setString(query);
    }
}

void OpenDB::originFormQuery(void)
{
    char query[MAX_Q_LEN];

    if(originsFromOrid(query, sizeof(query))) {
	origin_query->setString(query);
    }
}

bool OpenDB::originsFromTime(char *query, int query_size)
{
    int n;
    char *start_time = NULL, *end_time = NULL;
    char msg[1024];
    string origin_table;
    char *account, And[5];
    double start = NULL_TIME, end = NULL_TIME;
	
    start_time = time_start_time->getString();
    if(strlen(start_time) > 0)
    {
	if(timeParseString(start_time, &start) != 1)
	{
	    Free(start_time);
	    return false;
	}
    }
    Free(start_time);

    end_time = time_end_time->getString();
    if(strlen(end_time) > 0)
    {
	if(timeParseString(end_time, &end) != 1)
	{
	    Free(end_time);
	    return False;
	}
    }
    Free(end_time);

    if( !timeCheckTimes(start, end, msg, 1024) )
    {
	return False;
    }

    origin_table = getMapping(cssOrigin);
    if((account = dbAccount->getString()) == NULL) return false;

    snprintf(query, query_size, "select * from %s%s%s", account,
		PERIOD(account), origin_table.c_str());
    stringcpy(And, "", sizeof(And));
    Free(account);

    if(start > NULL_TIME_CHECK)
    {
	n = strlen(query);
	snprintf(query+n, query_size-n, " where %s time >= %.2f ",
			And, start - 7200.);
	stringcpy(And, "and", sizeof(And));
    }
    else stringcpy(And, " where", sizeof(And));

    if(end > NULL_TIME_CHECK)
    {
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s time <= %.2f ", And, end);
    }
    return true;
}

bool OpenDB::arrivalsFromTime(const string &tabName, char *query, int query_size)
{
    bool input;
    int n;
    char *start_time = NULL, *end_time = NULL, *c;
    char *stations = NULL, *channels = NULL, *network = NULL;
    char stations_clause[256]="", channels_clause[256]="";
    char msg[1024], *account, And[5];
    string arrival_table, affiliation_table;
    double start = NULL_TIME, end = NULL_TIME;
	
    if(!tabName.compare("arrival")) {
	start_time = arrival_start_time->getString();
	end_time = arrival_end_time->getString();
    }
    else {
	start_time = time_start_time->getString();
	end_time = time_end_time->getString();
    }
    if(strlen(start_time) > 0)
    {
	if(timeParseString(start_time, &start) != 1)
	{
	    Free(start_time);
	    Free(end_time);
	    return false;
	}
    }
    Free(start_time);

    if(strlen(end_time) > 0)
    {
	if(timeParseString(end_time, &end) != 1)
	{
	    Free(end_time);
	    return false;
	}
    }
    Free(end_time);

    if( !timeCheckTimes(start, end, msg, 1024) )
    {
	return false;
    }

    arrival_table = getMapping(cssArrival);
    affiliation_table = getMapping(cssAffiliation);
    if((account = dbAccount->getString()) == NULL) return false;

    if(!tabName.compare("arrival")) {
	stations = arrival_stations->getString();
	check_query_char(stations, stations_clause, 256, "sta", 0);
    }
    else {
	stations = time_stations->getString();
	network = time_network->getString();
	if(stations[0] != '\0') {
	    check_query_char(stations, stations_clause, 256, "sta", 0);
	}
	else if(network[0] != '\0') {
	    snprintf(stations_clause, sizeof(stations_clause), "net='%s'", 
			network);
	}
	Free(network);
    }
    Free(stations);

    input = false;
    if(stations_clause[0] != '\0') {
	input = true;
	snprintf(query, query_size,
"select distinct r.* from %s%s%s r, %s%s%s a where r.sta = a.sta and a.%s ",
		account, PERIOD(account), arrival_table.c_str(),
		account, PERIOD(account), affiliation_table.c_str(),
			stations_clause);
		stringcpy(And, "and", sizeof(And));
    }
    else {
	snprintf(query, query_size, "select * from %s%s%s where ",
			account, PERIOD(account), arrival_table.c_str());
	stringcpy(And, "", sizeof(And));
    }
    Free(account);

    if(!tabName.compare("arrival"))
    {
	channels = arrival_channels->getString();
	check_query_char(channels, channels_clause, 256, "chan", 2);
	Free(channels);

	if(channels_clause != NULL && (int)strlen(channels_clause) > 0) {
	    input = true;
	    n = strlen(query);
	    snprintf(query+n, query_size-n, "%s %s ", And, channels_clause);
	    stringcpy(And, "and", sizeof(And));
	}
    }

    if(start > NULL_TIME_CHECK)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s time >= %.2f ", And, start);
	stringcpy(And, "and", sizeof(And));
    }

    if(end > NULL_TIME_CHECK)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s time <= %.2f ", And, end);
    }
    if(!input) {
	if((c = strstr(query, " where")) != NULL) *c = '\0';
    }
    return true;
}

bool OpenDB::arrivalsFromArid(char *query, int query_size)
{
    char *arrival_id = NULL, *account;
    string arrival_table;
    long arid;
	
    arrival_id = arrivalId->getString();
    if(strlen(arrival_id) <= 0) {
	Free(arrival_id);
	arrival_id = NULL;
    }

    arrival_table = getMapping(cssArrival);
    if((account = dbAccount->getString()) == NULL) return False;

    if(!arrival_id) {
	snprintf(query, query_size, "select * from %s%s%s",
		account, PERIOD(account), arrival_table.c_str());
    }
    else if(stringToLong(arrival_id, &arid)) {  /* a single arid */
	snprintf(query, query_size, "select * from %s%s%s where arid=%s",
		account, PERIOD(account), arrival_table.c_str(), arrival_id);
    }
    else {
	snprintf(query,query_size,"select * from %s%s%s where arid in (%s)",
		account, PERIOD(account), arrival_table.c_str(), arrival_id);
    }
    XtFree(arrival_id);
    XtFree(account);

    return true;
}

bool OpenDB::originsFromOrid(char *query, int query_size)
{
    char *origin_id = NULL, *account, And[6], *min, *max;
    string origin_table;
    double d, t_min, t_max;
    int n;
    long orid, l;
	
    origin_id = originId->getString();
    if(strlen(origin_id) <= 0) {
	Free(origin_id);
	origin_id = NULL;
    }

    origin_table = getMapping(cssOrigin);
    if((account = dbAccount->getString()) == NULL) return False;

    if(!origin_id) {
	snprintf(query, query_size, "select * from %s%s%s",
		account, PERIOD(account), origin_table.c_str());
	stringcpy(And, "", sizeof(And));
    }
    else if(stringToLong(origin_id, &orid)) {  /* a single arid */
	snprintf(query, query_size, "select * from %s%s%s where orid=%s",
		account, PERIOD(account), origin_table.c_str(), origin_id);
	stringcpy(And, " and ", sizeof(And));
    }
    else {
	snprintf(query, query_size,
		"select * from %s%s%s where orid in (%s)",
		account, PERIOD(account), origin_table.c_str(), origin_id);
	stringcpy(And, " and ", sizeof(And));
    }
    XtFree(origin_id);
    XtFree(account);

    if((min = minLat->getString()) && strlen(min) > 0) {
	if(!stringToDouble(min, &d) || d < -90.01 || d > 90.01) {
	    showWarning("Cannot interpret Min Latitude");
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxLat->getString()) && strlen(max) > 0) {
	if(!stringToDouble(max, &d) || d < -90.01 || d > 90.01) {
	    showWarning("Cannot interpret Max Latitude");
	    XtFree(max);
	    max = NULL;
	}
    }
    addClause(query, query_size, And, sizeof(And), "lat", min, max);
    XtFree(min); XtFree(max);

    if((min = minLon->getString()) && strlen(min) > 0) {
	if(!stringToDouble(min, &d) || d < -180.01 || d > 180.01) {
	    showWarning("Cannot interpret Min Longitude");
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxLon->getString()) && strlen(max) > 0) {
	if(!stringToDouble(max, &d) || d < -180.01 || d > 180.01) {
	    showWarning("Cannot interpret Max Longitude");
	    XtFree(max);
	    max = NULL;
	}
    }
    addClause(query, query_size, And, sizeof(And), "lon", min, max);
    XtFree(min); XtFree(max);

    if((min = minDepth->getString()) && strlen(min) > 0){
	if(!stringToDouble(min, &d)) {
	    showWarning("Cannot interpret Min Depth");
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxDepth->getString()) && strlen(max) > 0){
	if(!stringToDouble(max, &d)) {
	    showWarning("Cannot interpret Max Depth");
	    XtFree(max);
	    max = NULL;
	}
    }
    addClause(query, query_size, And, sizeof(And), "depth", min, max);
    XtFree(min); XtFree(max);

    if((min = minDefining->getString()) && strlen(min) > 0) {
	if(!stringToLong(min, &l)) {
	    showWarning("Cannot interpret Min Defining");
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxDefining->getString()) && strlen(max) > 0) {
	if(!stringToLong(max, &l)) {
	    showWarning("Cannot interpret Max Defining");
	    XtFree(max);
	    max = NULL;
	}
    }
    addClause(query, query_size, And, sizeof(And), "ndef", min, max);
    XtFree(min); XtFree(max);

    if((min = minNass->getString()) && strlen(min) > 0) {
	if(!stringToLong(min, &l)) {
	    showWarning("Cannot interpret Min Associated");
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxNass->getString()) && strlen(max) > 0) {
	if(!stringToLong(max, &l)) {
	    showWarning("Cannot interpret Max Associated");
	    XtFree(max);
	    max = NULL;
	}
    }
    addClause(query, query_size, And, sizeof(And), "nass", min, max);
    XtFree(min); XtFree(max);

    if((min = minTime->getString()) && strlen(min) > 0) {
	if(timeParseString(min, &t_min) != 1) {
	    XtFree(min);
	    min = NULL;
	}
    }
    if((max = maxTime->getString()) && strlen(max) > 0) {
	if(timeParseString(max, &t_max) != 1) {
	    XtFree(max);
	    max = NULL;
	}
    }
    if(min != NULL && min[0] != '\0') {
	if(And[0] == '\0') {
	    if((int)strlen(query) + 8 > query_size) {
		showWarning("Query too long.");
		XtFree(min); XtFree(max);
		return false;
	    }
	    strcat(query, " where ");
	}
	n = strlen(query);
	snprintf(query+n, query_size-n, "%stime >= %.2f", And, t_min);
	stringcpy(And, " and ", sizeof(And));
    }
    if(max != NULL && max[0] != '\0') {
	if(And[0] == '\0') {
	    if((int)strlen(query) + 8 > query_size) {
		showWarning("Query too long.");
		XtFree(min); XtFree(max);
		return False;
	    }
	    strcat(query, " where ");
	}
	n = strlen(query);
	snprintf(query+n, query_size-n, "%stime <= %.2f", And, t_max);
    }
    XtFree(min); XtFree(max);

    return true;
}

void OpenDB::addClause(char *query, int query_size, char *And, int And_size,
		const char *name, char *min, char *max)
{
    char *q;
    int n;

    if((min == NULL || min[0] == '\0') && (max == NULL || max[0] == '\0')) {
	return;
    }

    if(And[0] == '\0') {
	if((int)strlen(query) + 8 < query_size) {
	    strcat(query, " where ");
	}
    }

    n = strlen(query);
    q = query + n;

    if(min != NULL && min[0] != '\0') {
	if(max != NULL && max[0] != '\0') {
	    snprintf(q, query_size-n, "%s%s between %s and %s",
				And, name,min,max);
	}
	else {
	    snprintf(q, query_size-n, "%s%s >= %s", And, name, min);
	}
    }
    else if(max != NULL && max[0] != '\0') {
	    snprintf(q, query_size-n, "%s%s <= %s", And, name, max);
    }
    stringcpy(And, " and ", And_size);
}

bool OpenDB::wfdiscsFromTime(char *query, int query_size, double *t_min,
			double *t_max)
{
    bool input;
    int n;
    char *start_time = NULL, *end_time = NULL, *c;
    char *stations = NULL, *channels = NULL, *network = NULL;
    char stations_clause[256]="", channels_clause[256]="";
    char msg[1024], *account, And[5];
    string wfdisc_table, affiliation_table;
    double start = NULL_TIME, end = NULL_TIME;
	
    start_time = time_start_time->getString();
    if(strlen(start_time) > 0)
    {
	if(timeParseString(start_time, &start) != 1) {
	    XtFree(start_time);
	    return false;
	}
    }
    XtFree(start_time);

    end_time = time_end_time->getString();
    if(strlen(end_time) > 0)
    {
	if(timeParseString(end_time, &end) != 1) {
	    XtFree(end_time);
	    return false;
	}
    }
    XtFree(end_time);

    if( !timeCheckTimes(start, end, msg, 1024) )
    {
	return false;
    }

    wfdisc_table = getMapping(cssWfdisc);
    affiliation_table = getMapping(cssAffiliation);
    if((account = dbAccount->getString()) == NULL) return False;

    stations = time_stations->getString();
    network = time_network->getString();
    if(stations[0] != '\0') {
	check_query_char(stations, stations_clause, 256, "sta", 0);
    }
    else if(network[0] != '\0') {
	snprintf(stations_clause, sizeof(stations_clause),"a.net='%s'",network);
    }
    Free(stations);
    Free(network);

    input = False;
    if(stations_clause[0] != '\0')
    {
	input = true;
	if(!strncmp(stations_clause, "sta ", 4)) {
	    snprintf(query, query_size,
"select distinct w.* from %s%s%s w, %s%s%s a where a.%s and w.sta=a.sta  ",
		    account, PERIOD(account), wfdisc_table.c_str(),
		    account, PERIOD(account), affiliation_table.c_str(),
		    stations_clause);
		    stringcpy(And, "and", sizeof(And));
	}
	else {
	    snprintf(query, query_size,
"select distinct w.* from %s%s%s w, %s%s%s a where %s and w.sta=a.sta  ",
		    account, PERIOD(account), wfdisc_table.c_str(),
		    account, PERIOD(account), affiliation_table.c_str(),
		    stations_clause);
		    stringcpy(And, "and", sizeof(And));
	}
    }
    else
    {
	snprintf(query, query_size, "select distinct * from %s%s%s where ",
		account, PERIOD(account), wfdisc_table.c_str());
	stringcpy(And, "", sizeof(And));
    }
    XtFree(account);

    channels = time_channels->getString();
    check_query_char(channels, channels_clause, 256, "chan", 2);
    XtFree(channels);

    if(channels_clause != NULL && (int)strlen(channels_clause) > 0)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s %s ", And, channels_clause);
	stringcpy(And, "and", sizeof(And));
    }

    if(start > NULL_TIME_CHECK && end > NULL_TIME_CHECK)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n,
		"%s endtime > %.2f and time between %.2f - %.2f and %.2f",
		And, start, start, query_buffer, end);
    }
    else if(start > NULL_TIME_CHECK)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s time >= %.2f ", And, start);
    }
    else if(end > NULL_TIME_CHECK)
    {
	input = true;
	n = strlen(query);
	snprintf(query+n, query_size-n, "%s time <= %.2f ", And, end);
    }
    if(!input) {
	if((c = strstr(query, " where")) != NULL) *c = '\0';
    }
    *t_min = start;
    *t_max = end;
    return true;
}

bool OpenDB::tabOnTop(const string &tabName)
{
    Widget w;
    if((w = tab->getTabOnTop()) && !tabName.compare(XtName(w))) {
	return true;
    }
    return false;
}

string OpenDB::getMapping(const string &cssTableName)
{
    vector<const char *> row;
    int nrows = mapping_table->numRows();

    for(int i = 0; i < nrows; i++) {
	mapping_table->getRow(i, row);
	if(!strcasecmp(row[0], cssTableName.c_str())) {
	    const char *c = row[1];
	    return string(c);
	}
    }
    return cssTableName;
}

bool OpenDB::arrivalTimeBefore(double *time_before)
{
    return arrival_time_before->getTime(time_before);
}

bool OpenDB::arrivalTimeAfter(double *time_after)
{
    return arrival_time_after->getTime(time_after);
}

void OpenDB::setAuthorWritable(void)
{
    const char *default_author = ffdb->defaultAuthor();

    ffdb->setAuthorWritable(default_author, True);

    int num_authors;
    char **authors = NULL;

    num_authors = ffdb->getAuthors(&authors);

    for(int i = 0; i < num_authors; i++) {
	if(strcmp(default_author, authors[i])) {
	    ffdb->setAuthorWritable(authors[i], False);
	}
	free(authors[i]);
    }
    Free(authors);
}

char * OpenDB::getWritableAuthor(const string &param_root)
{
    if(!ffdb_param_root.compare(param_root))
    {
	char *author = NULL;
	char **authors = NULL;
	int num_authors = ffdb->getAuthors(&authors);
	for(int i = 0; i < num_authors; i++) {
	    if(ffdb->getAuthorWritable(authors[i])) {
		author = strdup(authors[i]);
		break;
	    }
	}
	for(int i = 0; i < num_authors; i++) free(authors[i]);
	if(authors) free(authors);
	return author;
    }
    return NULL;
}

void OpenDB::setMappingPrefix(char *s)
{
    vector<const char *> row;
    int nrows = mapping_table->numRows();

    for(int i = 0; i < nrows; i++)
    {
	char name[100];
	mapping_table->getRow(i, row);
	snprintf(name, sizeof(name), "%s%s", s, row[0]);
	row[1] = name;
	mapping_table->setRow(i, row);
    }
    mapping_table->adjustColumns();
    setButtonsSensitive();
}

void OpenDB::setMappingSuffix(char *s)
{
    vector<const char *> row;
    int nrows = mapping_table->numRows();

    for(int i = 0; i < nrows; i++)
    {
	char name[100];
	mapping_table->getRow(i, row);
	snprintf(name, sizeof(name), "%s%s", row[0], s);
	row[1] = name;
	mapping_table->setRow(i, row);
    }
    mapping_table->adjustColumns();
    setButtonsSensitive();
}

void OpenDB::resetMapping(void)
{
    vector<const char *> row;
    int nrows = mapping_table->numRows();

    for(int i = 0; i < nrows; i++)
    {
	mapping_table->getRow(i, row);
	char name[200];
	strcpy(name, row[0]);
	row[1] = name;
	mapping_table->setRow(i, row);
    }
    mapping_table->adjustColumns();
    reset_mapping_button->setSensitive(false);
    setButtonsSensitive();
}

void OpenDB::saveMapping(void)
{
    char name[200];
    vector<const char *> row;
    int nrows = mapping_table->numRows();

    for(int i = 0; i < nrows; i++)
    {
	mapping_table->getRow(i, row);
	snprintf(name, sizeof(name), "mapping.%s", row[0]);
	if(strcmp(row[0], row[1])) {
	    putProperty(name, row[1]);
	}
	else {
	    removeProperty(name);
	}
	Free(permanent_mapping[i]);
	permanent_mapping[i] = strdup(row[1]);
    }
    Application::writeApplicationProperties();
    make_mapping_permanent->setSensitive(false);
}

void OpenDB::setButtonsSensitive(void)
{
    vector<const char *> col1, col2;
    int i, nrows;

    nrows = mapping_table->getColumn(0, col1);
    mapping_table->getColumn(1, col2);

    for(i = 0; i < nrows; i++) {
	if(strcmp(col1[i], col2[i])) break;
    }
    reset_mapping_button->setSensitive((i < nrows) ? true : false);

    for(i = 0; i < nrows; i++) {
	if(strcmp(permanent_mapping[i], col2[i])) break;
    }
    make_mapping_permanent->setSensitive((i < nrows) ? true : false);
}

void OpenDB::saveAccount(void)
{
    string s;
    dbAccount->getString(s);

    if(!s.empty()) {
	putProperty("database.account", s);
    }
    else {
	removeProperty("database.account");
    }
    permanent_account = s;
    Application::writeApplicationProperties();
    account_permanent->setSensitive(false);
}
