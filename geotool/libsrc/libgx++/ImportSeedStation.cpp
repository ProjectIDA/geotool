/** \file ImportSeedStation.cpp
 *  \brief Defines class ImportSeedStation.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sys/stat.h>
#include <errno.h>
#include "ImportSeedStation.h"
#include "SeedToCss.h"


ImportSeedStation::ImportSeedStation(const string &name, Component *parent, ActionListener *listener)
	  : FormDialog(name, parent, false, false)
{
  createInterface();
  addActionListener(listener);
}

ImportSeedStation::~ImportSeedStation(void)
{
}

void ImportSeedStation::createInterface(void)
{
    Arg args[20];
    int n;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    file_label = new Label("Seed File", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    import_button = new Button("Import", controls, this);
    cancel_button = new Button("Cancel", controls, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 10); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    rb = new RadioBox("rb", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNset, true); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    update_toggle = new Toggle("Update Files", rb, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNset, false); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    overwrite_toggle = new Toggle("Overwrite Files", rb, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, file_label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, rb->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    rc = new RowColumn("Files", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNwidth, 10); n++;
    dir_label = new Label("table directory", rc, args, n);
    prefix_label = new Label("table prefix", rc, args, n);
    response_label = new Label("response directory", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNcolumns, 40); n++;
    dir_text = new TextField("prefix_dir", rc, this, args, n);
    prefix_text = new TextField("prefix_text", rc, this, args, n);
    response_text = new TextField("resp_text", rc, this, args, n);

    enableCallbackType(XmNactivateCallback);

    setDefaultFiles();
}

void ImportSeedStation::setDefaultFiles()
{
    string geo_table_dir, dir, respdir;

    getProperty("geoTableDir", geo_table_dir);

    dir = geo_table_dir + "/static";
    respdir = geo_table_dir + "/response";

    dir_text->setString(dir);
    prefix_text->setString("local");
    response_text->setString(respdir);
}

void ImportSeedStation::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Import")) {
	import();
    }
    else if(!strcmp(cmd, "Cancel")) {
	cancel();
    }
}

void ImportSeedStation::setSeedFile(string file) {
    seed_file = file;
    file_label->setLabel("Seed File: " + file);
}

void ImportSeedStation::import(void)
{
    if(seed_file.length() <= 0) return;

    string dir, respdir, prefix;
    bool update, getdata = false;

    if( seed_file.length() == 0  ) return;

    dir = dir_text->getString();
    prefix = prefix_text->getString();
    respdir = response_text->getString();
    update = update_toggle->state();

    if(dirOk("dir", dir) && dirOk("respdir", respdir)) {
	setCursor("hourglass");
	SeedToCss::convertToCss(seed_file, dir, prefix, respdir, update, getdata);
	setTableFiles2(dir, prefix, respdir);
	setCursor("default");
	setVisible(false);
    }
}

bool ImportSeedStation::dirOk(string name, string dir)
{
    struct stat buf;

    if( !stat(dir.c_str(), &buf) ) {
	if(!S_ISDIR(buf.st_mode)) {
	    showWarning("%s exists and is not a directory", dir.c_str());
	    return false;
	}
    }
    else if( !mkpath(dir) ) {
	showWarning("Cannot create %s\n%s", dir.c_str(), strerror(errno));
	return false;
    }
    return true;
}

bool ImportSeedStation::mkpath(string path)
{
    if(mkdir(path.c_str(), 0775) == -1)
    {
	if(errno == ENOENT) {
	    // no parent directory. try to create it
	    if( mkpath( path.substr(0, path.find_last_of('/')) ) ) {
		// Now, try to create again.
		return (mkdir( path.c_str(), 0775 ) == 0) ? true : false;
	    }
	    return false;
	}
	else if(errno == EEXIST) {
	    return true;
	}
	return false;
    }
    return true;
}

void ImportSeedStation::setTableFiles2(string dir, string prefix, string respdir)
{
    string prop, table;

    if(!getProperty("recipeDir", prop) || prop != respdir) {
	putProperty("recipeDir2", respdir, true, this);
    }
    table = dir + "/" + prefix + ".affiliation";
    if(!getProperty("affiliationTable", prop) || prop != table) {
	putProperty("affiliationTable2", table, true, this);
    }
    table = dir + "/" + prefix + ".instrument";
    if(!getProperty("instrumentTable", prop) || prop != table) {
	putProperty("instrumentTable2", table, true, this);
    }
    table = dir + "/" + prefix + ".sensor";
    if(!getProperty("sensorTable", prop) || prop != table) {
	putProperty("sensorTable2", table, true, this);
    }
    table = dir + "/" + prefix + ".site";
    if(!getProperty("siteTable", prop) || prop != table) {
	putProperty("siteTable2", table, true, this);
    }
    table = dir + "/" + prefix + ".sitechan";
    if(!getProperty("sitechanTable", prop) || prop != table) {
	putProperty("sitechanTable2", table, true, this);
    }
}

void ImportSeedStation::cancel(void)
{
    setVisible(false);
}
