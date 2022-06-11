/** \file AddStation.cpp
 *  \brief Defines class AddStation.
 *  \author Vera Miljanovic
 */
#include "config.h"
#include "AddStation.h"
#include "motif++/MotifClasses.h"
extern "C" {
#include "libstring.h"
#include "libtime.h"
}
#include "gobject++/CssTables.h"
#include "ResponseFile.h"
#include "motif++/Choice.h"

static const int  num_channel_rows            = 10;
static const bool enable_instrument_selection = false;
static const bool enable_all_instruments      = false;  /* very, very slow */
static const bool enable_copy_station         = false;
static const bool enable_copy_template        = true;
static const bool enable_insert_records       = true;

static const int  num_template_sites          = 8;
static const int  num_template_sitechans      = 28;
static const int  num_template_affiliations   = 8;
static const int  num_template_sensors        = 0;

static const struct addstation_template_site_t template_sites[num_template_sites] = {
  {"STA01", 2000001, -1, 48.2328, 16.4152, 1.0000, "Example Station, 3 Component Broad Band",           "ss", "STA01",  0.0000,  0.0000},  
  {"STA02", 2000001, -1, 48.2328, 16.4152, 1.0000, "Example Station, 6 Component Short and Mid Period", "ss", "STA02",  0.0000,  0.0000},   
  {"STA03", 2000001, -1, 48.2328, 16.4152, 1.0000, "Example Station, 6 Component Short and Broad Band", "ss", "STA03",  0.0000,  0.0000},
  {"STA04", 2000001, -1, 48.2328, 16.4152, 1.0000, "Example Station, 3 Component Short Period",         "ss", "STA04",  0.0000,  0.0000},
  {"STA05", 2000001, -1, 48.2328, 16.4152, 1.0000, "Example Station, 3 Component BB and 1 Vertical",    "ss", "STA05",  0.0000,  0.0000},
  {"ARRAY", 2000001, -1, 50.7012, 29.2242, 0.1600, "Sample Array",                                      "ar", "ARRAY",  0.0000,  0.0000},
  {"ELB01", 2000001, -1, 50.6573, 29.2057, 0.1700, "Single Element of Sample Array - Broad Band",       "ss", "ARRAY", -4.9220, -1.1640},
  {"ELS01", 2000001, -1, 50.6573, 29.2057, 0.1700, "Single Element of Sample Array - Short Period",     "ss", "ARRAY", -4.9220, -1.1640}       
};

static const struct addstation_template_sitechan_t template_sitechans[num_template_sitechans] = {
  {"STA01", "BHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "broad band east"                   },
  {"STA01", "BHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "broad band north"                  },
  {"STA01", "BHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "broad band vertical"               },
  {"STA02", "SHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "short period east"                 },
  {"STA02", "SHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "short period north"                },
  {"STA02", "SHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "short period vertical"             },
  {"STA02", "MHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "mid period east"                   },
  {"STA02", "MHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "mid period north"                  },
  {"STA02", "MHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "mid period vertical"               },
  {"STA03", "SHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "short period east"                 },
  {"STA03", "SHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "short period north"                },
  {"STA03", "SHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "short period vertical"             },
  {"STA03", "BHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "broad band east"                   },
  {"STA03", "BHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "broad band north"                  },
  {"STA03", "BHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "broad band vertical"               },
  {"STA04", "SHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "short period east"                 },
  {"STA04", "SHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "short period north"                },
  {"STA04", "SHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "short period vertical"             },
  {"STA05", "BHE", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "broad band east"                   },
  {"STA05", "BHN", 2000001, -1, -1, "n", 0.030000,  0.0000, 90.0000, "broad band north"                  },
  {"STA05", "BHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "broad band vertical"               },
  {"STA05", "SHZ", 2000001, -1, -1, "n", 0.030000, 90.0000, 90.0000, "short period vertical"             },
  {"ARRAY", "cb",  2000001, -1, -1, "b", 0.000000, -1.0000, -1.0000, "coherent beam, 4-8 Hz, 8.1 km/s"   },
  {"ARRAY", "fkb", 2000001, -1, -1, "b", 0.000000, -1.0000, -1.0000, "detection beam, 0.8-3.5 Hz"        },
  {"ARRAY", "hb",  2000001, -1, -1, "i", 0.000000, -1.0000, -1.0000, "horizontal incoherent beam, 2-4 Hz"},
  {"ARRAY", "ib",  2000001, -1, -1, "i", 0.000000, -1.0000, -1.0000, "vertical incoherent beam, 2-4 Hz"  },
  {"ELB01", "BHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "broad band vertical element"       },
  {"ELS01", "SHZ", 2000001, -1, -1, "n", 0.030000, -1.0000,  0.0000, "short period vertical element"     }
};

static const struct addstation_template_affiliation_t template_affiliations[num_template_affiliations] = {
  {"STA01", "STA01"},
  {"STA02", "STA02"},        
  {"STA03", "STA03"},
  {"STA04", "STA04"},   
  {"STA05", "STA05"},
  {"ARRAY", "ARRAY"},
  {"ARRAY", "ELB01"},
  {"ARRAY", "ELS01"}
};

static const struct addstation_template_sensor_t template_sensors[num_template_sensors] = {
  //{"ARRAY", "cb", 946684800.0,  2145916800.0, 211636, -1, 2000001, 1.0, -1.0, 0.0, "y"}
};

AddStation::AddStation(const string &name, Component *parent,
		       TableViewer *table_viewer,
		       ActionListener *listener)
  : FormDialog(name, parent, false, false)
{
  tv = table_viewer;

  setCursor("hourglass");


  num_current_sites = tv->getTableRecords(cssSite, current_sites);
  num_sitechans = tv->getTableRecords(cssSitechan, sitechans);
  num_sensors = tv->getTableRecords(cssSensor, sensors);
  num_affiliations = tv->getTableRecords(cssAffiliation, affiliations);


  instruments_all.empty();
  instruments_choice.empty();

  if (enable_instrument_selection) {

    ResponseFile::getInstruments(instruments_all);

    /* pick instruments for dropdown */
    instruments_all.sortByMember("instype");
    for (int i = 0; i < instruments_all.size(); i++) {
      if (strcmp(instruments_all[i]->instype, "-")) {
	if (enable_all_instruments) {
	  /* complete choice - takes very long time to create interface */
	  instruments_choice.add(instruments_all[i]);
	}
	else {
	  /* limit choice - speed up creation of interface */
	  if ((i == 0) || (i > 0
			   && strcmp(instruments_all[i-1]->instype,
				     instruments_all[i]->instype))) {
	    instruments_choice.add(instruments_all[i]);
	  }
	}
      }
    }

  } /* endif enable_instrument_selection */

  /* define instruments dropdown text */
  instruments_choice_text = (char **)mallocWarn(instruments_choice.size()
						* sizeof(char *));
  instruments_choice.sortByMember("instype");
  for (int i = 0; i < instruments_choice.size(); i++) {
    instruments_choice_text[i] = (char *)mallocWarn(200*sizeof(char));
    snprintf(instruments_choice_text[i], 200,
	     "%s (%ld)",
	     instruments_choice[i]->instype,
	     instruments_choice[i]->inid);
  }


  /* allocate memory for channels */
  chan = (char **)mallocWarn(num_channel_rows*sizeof(char *));
  chan_ondate = (long *)mallocWarn(num_channel_rows*sizeof(long));
  chan_chanid = (long *)mallocWarn(num_channel_rows*sizeof(long));
  chan_offdate = (long *)mallocWarn(num_channel_rows*sizeof(long));
  chan_ctype = (char **)mallocWarn(num_channel_rows*sizeof(char *));
  chan_edepth = (double *)mallocWarn(num_channel_rows*sizeof(double));
  chan_hang= (double *)mallocWarn(num_channel_rows*sizeof(double));
  chan_vang= (double *)mallocWarn(num_channel_rows*sizeof(double));
  chan_descrip = (char **)mallocWarn(num_channel_rows*sizeof(char *));
  chan_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  ondate_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  offdate_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  ctype_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  edepth_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  hang_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  vang_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));
  descrip_text = (TextField **)mallocWarn(num_channel_rows*sizeof(TextField *));

  /* allocate memory for sensors */
  sensor_time = (double *)mallocWarn(num_channel_rows*sizeof(double));
  sensor_endtime = (double *)mallocWarn(num_channel_rows*sizeof(double));
  sensor_inid = (long *)mallocWarn(num_channel_rows*sizeof(long));
  sensor_jdate = (long *)mallocWarn(num_channel_rows*sizeof(long));
  sensor_calratio = (double *)mallocWarn(num_channel_rows*sizeof(double));
  sensor_calper = (double *)mallocWarn(num_channel_rows*sizeof(double));
  sensor_tshift = (double *)mallocWarn(num_channel_rows*sizeof(double));
  sensor_instant = (char **)mallocWarn(num_channel_rows*sizeof(char *));
  instrument = (Choice **)mallocWarn(num_channel_rows*sizeof(TextField *));

  createInterface();

  reset();

  addActionListener(listener);

  setCursor("default");
}

AddStation::~AddStation(void)
{
  Free(chan);
  Free(chan_ondate);
  Free(chan_chanid);
  Free(chan_offdate);
  Free(chan_ctype);
  Free(chan_edepth);
  Free(chan_hang);
  Free(chan_vang);
  Free(chan_descrip);
  Free(chan_text);
  Free(ondate_text);
  Free(offdate_text);
  Free(ctype_text);
  Free(edepth_text);
  Free(hang_text);
  Free(vang_text);
  Free(descrip_text);
  Free(instrument);
}

void AddStation::createInterface(void)
{
  Arg args[20], a[20];
  int n, m;

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  controls = new RowColumn("controls", this, args, n);

  add_button = new Button("Add", controls, this);
  cancel_button = new Button("Cancel", controls, this);
  reset_button = new Button("Reset", controls, this);
  validate_button = new Button("Validate", controls, this);
  if (enable_copy_station && num_current_sites > 0) {
    copy_station_button = new Button("Copy Station", controls, this);
    copy_stations = new Choice("copy_stations", controls, this);
    for (int i = 0; i < num_current_sites; i++) {
      if (((CssSiteClass *)current_sites[i])->offdate == -1) {
	copy_stations->addItem(((CssSiteClass *)current_sites[i])->sta);
      }
    }
  }
  if (enable_copy_template && num_template_sites > 0) {
    copy_template_button = new Button("Copy Template", controls, this);
    templates_label = new Label("Template Stations:", controls);
    copy_templates = new Choice("copy_templates", controls, this);
    for (int i = 0; i < num_template_sites; i++) {
      copy_templates->addItem(template_sites[i].staname);
    }
  }

  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
  XtSetArg(args[n], XmNbottomOffset, 10); n++;
  sep2 = new Separator("sep2", this, args, n);

  n = 0;
  XtSetArg(args[n], XtNheight, 400); n++;
  XtSetArg(args[n], XtNwidth, 700); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNbottomWidget, sep2->baseWidget()); n++;
  TabClass *tab = new TabClass("tab", this, args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopOffset, 5); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
  XtSetArg(args[n], XmNisAligned, True); n++;
  XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_END); n++;
  XtSetArg(args[n], XmNnumColumns, 2); n++;
  rc = new RowColumn("Station Parameters", tab, args, n);

  sta_label = new Label("Station code", rc);
  lat_label = new Label("Latitude", rc);
  lon_label = new Label("Longitude", rc);
  elev_label = new Label("Elevation (km)", rc);
  staname_label = new Label("Station description", rc);
  statype_label = new Label("Type", rc);
  refsta_label = new Label("Reference station", rc);
  dnorth_label = new Label("North offset from array reference (km)", rc);
  deast_label = new Label("East offset from array reference (km)", rc);
  net_label = new Label("Affiliation network identifier", rc);

  n = 0;
  XtSetArg(args[n], XmNcolumns, 15); n++;
  sta_text = new TextField("sta_text", rc, this, args, n);
  lat_text = new TextField("lat_text", rc, this, args, n);
  lon_text = new TextField("lon_text", rc, this, args, n);
  elev_text = new TextField("elev_text", rc, this, args, n);
  staname_text = new TextField("staname_text", rc, this, args, n);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
  statype_rb = new RadioBox("statype_rb", rc, args, n);

  n = 0;
  XtSetArg(args[n], XmNset, (statype == 0) ? 1 : 0); n++;
  XtSetArg(args[n], XmNshadowThickness, 0); n++;
  ss_toggle = new Toggle("ss", statype_rb, this, args, n);
  n = 0;
  XtSetArg(args[n], XmNset, (statype == 1) ? 1 : 0); n++;
  XtSetArg(args[n], XmNshadowThickness, 0); n++;
  ar_toggle = new Toggle("ar", statype_rb, this, args, n);

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  refsta_text = new TextField("refsta_text", rc, this, args, n);
  dnorth_text = new TextField("dnorth_text", rc, this, args, n);
  deast_text = new TextField("deast_text", rc, this, args, n);
  net_text = new TextField("net_text", rc, this, args, n);

  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopOffset, 5); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftOffset, 5); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightOffset, 5); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  channels = new Form("Channel Parameters", tab, args, n);
  tab->setOnTop(0);

  m = 0;
  XtSetArg(a[m], XmNwidth, 20); m++;

  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;

  RowColumn *c1 = new RowColumn("c1", channels, args, n);
  chan_label = new Label("Identifier", c1);
  for (int i = 0; i < num_channel_rows; i++) {
    chan_text[i] = new TextField("chan_text", c1, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, c1->baseWidget()); n++;
  RowColumn *c8 = new RowColumn("c8", channels, args, n);
  ondate_label = new Label("On date          ", c8);
  for (int i = 0; i < num_channel_rows; i++) {
    ondate_text[i] = new TextField("ondate_text", c8, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, c8->baseWidget()); n++;
  RowColumn *c9 = new RowColumn("c9", channels, args, n);
  offdate_label = new Label("Off date          ", c9);
  for (int i = 0; i < num_channel_rows; i++) {
    offdate_text[i] = new TextField("offdate_text", c9, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, c9->baseWidget()); n++;
  RowColumn *c2 = new RowColumn("c2", channels, args, n);
  ctype_label = new Label("Type", c2);
  for (int i = 0; i < num_channel_rows; i++) {
    ctype_text[i] = new TextField("ctype_text", c2, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftWidget, c2->baseWidget()); n++;
  RowColumn *c3 = new RowColumn("c3", channels, args, n);
  edepth_label = new Label("Emplacement depth (km)", c3);
  for (int i = 0; i < num_channel_rows; i++) {
    edepth_text[i] = new TextField("edepth_text", c3, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftWidget, c3->baseWidget()); n++;
  RowColumn *c4 = new RowColumn("c4", channels, args, n);
  hang_label = new Label("Horizontal angle", c4);
  for (int i = 0; i < num_channel_rows; i++) {
    hang_text[i] = new TextField("hang_text", c4, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftWidget, c4->baseWidget()); n++;
  RowColumn *c5 = new RowColumn("c5", channels, args, n);
  vang_label = new Label("Vertical angle", c5);
  for (int i = 0; i < num_channel_rows; i++) {
    vang_text[i] = new TextField("vang_text", c5, this, a, m);
  }
  n--;
  XtSetArg(args[n], XmNleftWidget, c5->baseWidget()); n++;
  RowColumn *c6 = new RowColumn("c6", channels, args, n);
  descrip_label = new Label("Channel description", c6);
  for (int i = 0; i < num_channel_rows; i++) {
    descrip_text[i] = new TextField("descrip_text", c6, this, NULL, 0);
  }
  n--;
  XtSetArg(args[n], XmNleftWidget, c6->baseWidget()); n++;
  RowColumn *c7 = new RowColumn("c7", channels, args, n);
  if (instruments_choice.size() > 0) {
    instrument_label = new Label("Instrument", c7);
  }

  n = 0;
  XtSetArg(args[n], XmNcolumns, 15); n++;
  for (int i = 0; i < num_channel_rows; i++) {
    if (instruments_choice.size() > 0) {
      instrument[i] = new Choice("instrument", c7, this);
      instrument[i]->addItem("-");
      for (int j = 0; j < instruments_choice.size(); j++) {
	instrument[i]->addItem(instruments_choice_text[j]);
      }
    }
  }

  enableCallbackType(XmNactivateCallback);
}

void AddStation::actionPerformed(ActionEvent *action_event)
{
  const char *cmd = action_event->getActionCommand();

  if(!strcmp(cmd, "Add")) {
    add();
  }
  else if(!strcmp(cmd, "Cancel")) {
    cancel();
  }
  else if(!strcmp(cmd, "Reset")) {
    reset();
  }
  else if(!strcmp(cmd, "Validate")) {
    validate();
  }
  else if(!strcmp(cmd, "Copy Station")) {
    copy_station();
  }
  else if(!strcmp(cmd, "Copy Template")) {
    copy_template();
  }
  /*
  else if (comp == sta_text || comp == lat_text || comp == lon_text
	   || comp == elev_text || comp == staname_text
	   || comp == ss_toggle || comp == ar_toggle || comp == dnorth_text
	   || comp == deast_text) {
    setButtonsSensitive();
  }
  */
  else {
    setButtonsSensitive();
  }
}

void AddStation::setButtonsSensitive(void)
{
  if (validateForm(false)) {
    add_button->setSensitive(true);
    validate_button->setSensitive(false);
  }
  else {
    add_button->setSensitive(false);
    validate_button->setSensitive(true);
  }
}

bool AddStation::validateForm(bool displayWarning)
{
  bool is_valid = true;
  char *s;
  double d;
  long l;

  num_current_sites = tv->getTableRecords(cssSite, current_sites);

  if ((s = sta_text->getString())) {
    if (strlen(s) <= 6) {
      bool is_existing = false;
      for (int i = 0; i < num_current_sites; i++) {
	if (!strcmp(s, ((CssSiteClass *)current_sites[i])->sta)) {
	  is_existing = true;
	  break;
	}
      }
      if (is_existing) {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid station name - already exists in %s table",
		      cssSite);
	}
      }
      else {
	if (sta != NULL ) {
	  free(sta); sta = NULL;
	}
	sta = s;
      }
    }
    else {
      free(s);
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid station name - maximum 6 characters");
      }
    }
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid station name");
    }
  }

  if (lat_text->getDouble(&d) && d >= -90.0 && d <= 90.0) {
    lat = d;
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid latitude");
    }
  }

  if (lon_text->getDouble(&d) && d >= -180.0 && d <= 180.0) {
    lon = d;
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid longitude");
    }
  }

  if (elev_text->getDouble(&d) && d >= -1.0 && d <= 15.0) {
    elev = d;
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid elevation");
    }
  }

  if ((s = staname_text->getString())) {
    if (strlen(s) <= 50) {
      if (staname != NULL ) {
	free(staname); staname = NULL;
      }
      staname = s;
    }
    else {
      free(s);
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid station description - maximum 50 characters");
      }
    }
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid station description");
    }
  }

  if (ss_toggle->state()) {
    statype = 0;
  }
  else if (ar_toggle->state()) {
    statype = 1;
  }

  if ((s = refsta_text->getString())) {
    if (strlen(s) <= 6) {
      if (refsta != NULL ) {
	free(refsta); refsta = NULL;
      }
      refsta = s;
    }
    else {
      free(s);
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid array reference station name - maximum 6 characters");
      }
    }
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid array reference station name");
    }
  }

  if (!strcmp(refsta, "")) {  /* if empty */
    is_valid = false;
    if (displayWarning) {
      showWarning("Array reference station name required");
    }
  }

  if (dnorth_text->getDouble(&d) && d >= -90.0 && d <= 90.0) {
    dnorth = d;
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid north offset from array reference");
    }
  }

  if (deast_text->getDouble(&d) && d >= -90.0 && d <= 90.0) {
    deast = d;
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid east offset from array reference");
    }
  }

  if ((s = net_text->getString())) {
    if (strlen(s) <= 8) {
      if (net != NULL ) {
	free(net); net = NULL;
      }
      net = s;
    }
    else {
      free(s);
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid array network identifier name - maximum 8 characters");
      }
    }
  }
  else {
    is_valid = false;
    if (displayWarning) {
      showWarning("Invalid array network identifier name");
    }
  }

  if (!strcmp(net, "")) {  /* if empty */
    is_valid = false;
    if (displayWarning) {
      showWarning("Network identifier name required");
    }
  }

  for (int i = 0; i < num_channel_rows; i++) {  /* foreach channel */
    
    if ((s = chan_text[i]->getString())) {
      if (strlen(s) <= 8) {
	if (chan[i] != NULL ) {
	  free(chan[i]); chan[i] = NULL;
	}
	chan[i] = s;
      }
      else {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid channel name - maximum 8 characters - for channel #%d", i+1);
	}
      }
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid channel name for channel #%d", i+1);
      }
    }

    if ((s = ondate_text[i]->getString())) {
      if (!strcmp(s, "-1")) {
	chan_ondate[i] = -1;
      }
      else if (timeParseJDate(s, &l) == 1) {
	chan_ondate[i] = l;
      }
      else {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid format for on date for channel #%d", i+1);
	}
      }
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid on date for channel #%d", i+1);
      }
    }

    if ((s = offdate_text[i]->getString())) {
      if (!strcmp(s, "-1")) {
	chan_offdate[i] = -1;
      }
      else if (timeParseJDate(s, &l) == 1) {
	chan_offdate[i] = l;
      }
      else {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid format for off date for channel #%d", i+1);
	}
      }
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid off date for channel #%d", i+1);
      }
    }

    if ((s = ctype_text[i]->getString())) {
      if (strlen(s) <= 4) {
	if (chan_ctype[i] != NULL ) {
	  free(chan_ctype[i]); chan_ctype[i] = NULL;
	}
	chan_ctype[i] = s;
      }
      else {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid channel type - maximum 4 characters - for channel #%d", i+1);
	}
      }
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid channel type for channel #%d", i+1);
      }
    }

    if (edepth_text[i]->getDouble(&d) && d >= 0.0 && d <= 2.0) {
      chan_edepth[i] = d;
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid emplacement depth for channel #%d", i+1);
      }
    }

    if (hang_text[i]->getDouble(&d) && d >= -1.0 && d < 360.0) {
      chan_hang[i] = d;
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid horizontal angle for channel #%d", i+1);
      }
    }

    if (vang_text[i]->getDouble(&d) && d >= -1.0 && d <= 90.0) {
      chan_vang[i] = d;
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid vertical angle for channel #%d", i+1);
      }
    }

    if ((s = descrip_text[i]->getString())) {
      if (strlen(s) <= 50) {
	if (chan_descrip[i] != NULL ) {
	  free(chan_descrip[i]); chan_descrip[i] = NULL;
	}
	chan_descrip[i] = s;
      }
      else {
	free(s);
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid channel description - maximum 50 characters - for channel #%d", i+1);
	}
      }
    }
    else {
      is_valid = false;
      if (displayWarning) {
	showWarning("Invalid channel description for channel #%d", i+1);
      }
    }

    if (instruments_choice.size() > 0) {
      if ((s = (char *)instrument[i]->getChoice())) {
	if (!strcmp(s, "-")) {
	  sensor_inid[i] = -1;
	}
	else {
	  for (int j = 0; j < instruments_choice.size(); j++) {
	    if (!strcmp(s, instruments_choice_text[j])) {
	      sensor_inid[i] = instruments_choice[j]->inid;
	      break;
	    }
	  }
	}
      }
      else {
	is_valid = false;
	if (displayWarning) {
	  showWarning("Invalid instrument for channel #%d", i+1);
	}
      }
    }

    if (strcmp(chan[i], "")) {  /* if non-empty */
      ondate_text[i]->setSensitive(true);
      offdate_text[i]->setSensitive(true);
      ctype_text[i]->setSensitive(true);
      edepth_text[i]->setSensitive(true);
      hang_text[i]->setSensitive(true);
      vang_text[i]->setSensitive(true);
      descrip_text[i]->setSensitive(true);
      if (instruments_choice.size() > 0) {
	instrument[i]->setSensitive(true);
      }
    }
    else {
      ctype_text[i]->setSensitive(false);
      ondate_text[i]->setSensitive(false);
      offdate_text[i]->setSensitive(false);
      edepth_text[i]->setSensitive(false);
      hang_text[i]->setSensitive(false);
      vang_text[i]->setSensitive(false);
      descrip_text[i]->setSensitive(false);
      if (instruments_choice.size() > 0) {
	instrument[i]->setSensitive(false);
      }
    }

  } /* end foreach channel */

  return is_valid;
}

void AddStation::validate(void)
{
  validateForm(true);
}

void AddStation::reset(void)
{
  /* default station values */
  sta = strdup("SIM01");
  ondate = 2000001;
  offdate = -1;
  lat = 48.2088;
  lon = 16.3726;
  elev = 0.171;
  staname = strdup("Simple Example Station");
  statype = 0;
  refsta = strdup(sta);
  dnorth = 0.0;
  deast = 0.0;

  /* default channel values */
  chan[0] = strdup("BHZ");
  chan_ondate[0] = 2000001;
  chan_chanid[0] = -1;  /* generate new unique id through standard interface */
  chan_offdate[0] = -1;
  chan_ctype[0] = strdup("n");
  chan_edepth[0] = 0.0;
  chan_hang[0] = 0.0;
  chan_vang[0] = 0.0;
  chan_descrip[0] = strdup("broadband vertical");
  for (int i = 1; i < num_channel_rows; i++) {
    chan[i] = strdup("");
    chan_ondate[i] = 2000001;
    chan_chanid[i] = -1;  /* generate new unique id through standard interface */
    chan_offdate[i] = -1;
    chan_ctype[i] = strdup("n");
    chan_edepth[i] = 0.0;
    chan_hang[i] = 0.0;
    chan_vang[i] = 0.0;
    chan_descrip[i] = strdup("");
  }

  /* default sensor values */
  for (int i = 0; i < num_channel_rows; i++) {
    sensor_time[i] = 946684800.0;  /* 2000-01-01 midnight */
    sensor_endtime[i] = 2145916800.0;  /* 2038-01-01 midnight */
    sensor_inid[i] = -1;
    sensor_jdate[i] = 2000001;
    sensor_calratio[i] = 1.0;
    sensor_calper[i] = -1.0;
    sensor_tshift[i] = 0.0;
    sensor_instant[i] = strdup("y");
  }

  /* default affiliation values */
  net = strdup(sta);

  refreshFormValues();

  setButtonsSensitive();
}

void AddStation::cancel(void)
{
  setVisible(false);
  reset();
}

void AddStation::add(void)
{
  CssSiteClass *new_site = NULL;
  CssSitechanClass *new_sitechan = NULL;
  CssSensorClass *new_sensor = NULL;
  CssAffiliationClass *new_affiliation = NULL;
  gvector<CssTableClass *> new_sites;
  gvector<CssTableClass *> new_sitechans;
  gvector<CssTableClass *> new_sensors;
  gvector<CssTableClass *> new_affiliations;

  if (!validateForm(true)) {
    setButtonsSensitive();
    return;
  }

  /* Set up station/site structure */
  new_site = new CssSiteClass();
  strcpy(new_site->sta, sta);
  new_site->ondate = ondate;
  new_site->offdate = offdate;
  new_site->lat = lat;
  new_site->lon = lon;
  new_site->elev = elev;
  strcpy(new_site->staname, staname);
  if (ss_toggle->state()) strcpy(new_site->statype, "ss");
  else if (ar_toggle->state()) strcpy(new_site->statype, "ar");
  strcpy(new_site->refsta, refsta);
  new_site->dnorth = dnorth;
  new_site->deast = deast;
  new_sites.add(new_site);

  /* Add station to site table */
  if (enable_insert_records) {
    tv->insertRecords(0, new_sites);
  }
  else {
    tv->addRecords(new_sites);
  }

  for (int i = 0; i < num_channel_rows; i++) {

    if (strcmp(chan[i], "")) {  /* if non-empty */

      if (chan_chanid[i] == -1) {
	/* Request new chanid through standard interface */
	chan_chanid[i] = tv->getId(cssLastid, "chanid");
      }

      /* Set up channel/sitechan structure */
      new_sitechan = new CssSitechanClass();
      strcpy(new_sitechan->sta, sta);
      strcpy(new_sitechan->chan, chan[i]);
      new_sitechan->ondate = chan_ondate[i];
      new_sitechan->chanid = chan_chanid[i];
      new_sitechan->offdate = chan_offdate[i];
      strcpy(new_sitechan->ctype, chan_ctype[i]);
      new_sitechan->edepth = chan_edepth[i];
      new_sitechan->hang = chan_hang[i];
      new_sitechan->vang = chan_vang[i];
      strcpy(new_sitechan->descrip, chan_descrip[i]);
      new_sitechans.add(new_sitechan);

      if (sensor_inid[i] != -1) {  /* if instrument chosen */

	/* Set up sensor structure */
	new_sensor = new CssSensorClass();
	strcpy(new_sensor->sta, sta);
	strcpy(new_sensor->chan, chan[i]);
	new_sensor->time = sensor_time[i];
	new_sensor->endtime = sensor_endtime[i];
	new_sensor->inid = sensor_inid[i];
	new_sensor->chanid = chan_chanid[i];
	new_sensor->jdate = sensor_jdate[i];
	new_sensor->calratio = sensor_calratio[i];
	new_sensor->calper = sensor_calper[i];
	new_sensor->tshift = sensor_tshift[i];
	strcpy(new_sensor->instant, sensor_instant[i]);
	new_sensors.add(new_sensor);

      }

    }

  }

  /* Add channels to sitechan table */
  if (enable_insert_records) {
    tv->insertRecords(0, new_sitechans);
  }
  else {
    tv->addRecords(new_sitechans);
  }

  /* Add sensors to sensor table */
  if (enable_insert_records) {
    tv->insertRecords(0, new_sensors);
  }
  else {
    tv->addRecords(new_sensors);
  }

  /* Set up affiliation structure */
  new_affiliation = new CssAffiliationClass();
  strcpy(new_affiliation->net, net);
  strcpy(new_affiliation->sta, sta);
  new_affiliations.add(new_affiliation);

  /* Add affiliations to affiliation table */
  if (enable_insert_records) {
    tv->insertRecords(0, new_affiliations);
  }
  else {
    tv->addRecords(new_affiliations);
  }

  doCallbacks(base_widget, (XtPointer)this, XmNactivateCallback);
  setVisible(false);
  setButtonsSensitive();
}

void AddStation::copy_station(void)
{
  char *copy_station = (char *)copy_stations->getChoice();
  int stanum = -1;
  int nc = 0;

  for (stanum = 0; stanum < num_current_sites; stanum++) {
    if ((!strcmp(copy_station, ((CssSiteClass *)current_sites[stanum])->sta))
	&& (((CssSiteClass *)current_sites[stanum])->offdate == -1)) {
      break;
    }
  }

  sta = strdup(((CssSiteClass *)current_sites[stanum])->sta);
  ondate = ((CssSiteClass *)current_sites[stanum])->ondate;
  offdate = ((CssSiteClass *)current_sites[stanum])->offdate;
  lat = ((CssSiteClass *)current_sites[stanum])->lat;
  lon = ((CssSiteClass *)current_sites[stanum])->lon;
  elev = ((CssSiteClass *)current_sites[stanum])->elev;
  staname = strdup(((CssSiteClass *)current_sites[stanum])->staname);
  if (!strcmp(((CssSiteClass *)current_sites[stanum])->statype, "ss")) {
    statype = 0;
  }
  else if (!strcmp(((CssSiteClass *)current_sites[stanum])->statype, "ar")) {
    statype = 1;
  }
  refsta = strdup(((CssSiteClass *)current_sites[stanum])->refsta);
  dnorth = ((CssSiteClass *)current_sites[stanum])->dnorth;
  deast = ((CssSiteClass *)current_sites[stanum])->deast;

  for (int i = 0; i < num_sitechans && nc < num_channel_rows; i++) {
    if (!strcmp(sta, ((CssSitechanClass *)sitechans[i])->sta)) {
      chan[nc] = strdup(((CssSitechanClass *)sitechans[i])->chan);
      chan_ondate[nc] = 2000001;
      chan_chanid[nc] = -1;  /* generate new unique id through standard interface */
      chan_offdate[nc] = -1;
      chan_ctype[nc] = strdup(((CssSitechanClass *)sitechans[i])->ctype);
      chan_edepth[nc] = ((CssSitechanClass *)sitechans[i])->edepth;
      chan_hang[nc] = ((CssSitechanClass *)sitechans[i])->hang;
      chan_vang[nc] = ((CssSitechanClass *)sitechans[i])->vang;
      chan_descrip[nc] = strdup(((CssSitechanClass *)sitechans[i])->descrip);
      nc++;
    }
  }

  for (int i = nc; i < num_channel_rows; i++) {
    chan[i] = strdup("");
    chan_ondate[i] = 2000001;
    chan_chanid[i] = -1;  /* generate new unique id through standard interface */
    chan_offdate[i] = -1;
    chan_ctype[i] = strdup("n");
    chan_edepth[i] = 0.0;
    chan_hang[i] = 0.0;
    chan_vang[i] = 0.0;
    chan_descrip[i] = strdup("");
  }

  for (int i = 0; i < num_channel_rows; i++) {
    sensor_time[i] = 946684800.0;  /* 2000-01-01 midnight */
    sensor_endtime[i] = 2145916800.0;  /* 2038-01-01 midnight */
    sensor_inid[i] = -1;
    sensor_jdate[i] = 2000001;
    sensor_calratio[i] = 1.0;
    sensor_calper[i] = -1.0;
    sensor_tshift[i] = 0.0;
    sensor_instant[i] = strdup("y");
  }

  if (instruments_choice.size() > 0) {
    for (int i = 0; i < nc; i++) {
      for (int j = 0; j < num_sensors; j++) {
	if ((!strcmp(sta, ((CssSensorClass *)sensors[j])->sta))
	    && (!strcmp(chan[i], ((CssSensorClass *)sensors[j])->chan))) {
	  sensor_inid[i] = ((CssSensorClass *)sensors[j])->inid;
	  break;
	}
      }
    }
  }

  net = strdup(sta);
  for (int i = 0; i < num_affiliations; i++) {
    if (!strcmp(sta, ((CssAffiliationClass *)affiliations[i])->sta)) {
      net = strdup(((CssAffiliationClass *)affiliations[i])->net);
    }
  }

  refreshFormValues();

  setButtonsSensitive();
}

void AddStation::copy_template(void)
{
  char *copy_template = (char *)copy_templates->getChoice();
  int stanum = -1;
  int nc = 0;

  for (stanum = 0; stanum < num_template_sites; stanum++) {
    if (!strcmp(copy_template, template_sites[stanum].staname)) {
      break;
    }
  }

  sta = strdup(template_sites[stanum].sta);
  ondate = template_sites[stanum].ondate;
  offdate = template_sites[stanum].offdate;
  lat = template_sites[stanum].lat;
  lon = template_sites[stanum].lon;
  elev = template_sites[stanum].elev;
  staname = strdup(template_sites[stanum].staname);
  if (!strcmp(template_sites[stanum].statype, "ss")) {
    statype = 0;
  }
  else if (!strcmp(template_sites[stanum].statype, "ar")) {
    statype = 1;
  }
  refsta = strdup(template_sites[stanum].refsta);
  dnorth = template_sites[stanum].dnorth;
  deast = template_sites[stanum].deast;

  for (int i = 0; i < num_template_sitechans && nc < num_channel_rows; i++) {
    if (!strcmp(sta, template_sitechans[i].sta)) {
      chan[nc] = strdup(template_sitechans[i].chan);
      chan_ondate[nc] = template_sitechans[i].ondate;
      chan_chanid[nc] = template_sitechans[i].chanid;
      chan_offdate[nc] = template_sitechans[i].offdate;
      chan_ctype[nc] = strdup(template_sitechans[i].ctype);
      chan_edepth[nc] = template_sitechans[i].edepth;
      chan_hang[nc] = template_sitechans[i].hang;
      chan_vang[nc] = template_sitechans[i].vang;
      chan_descrip[nc] = strdup(template_sitechans[i].descrip);
      nc++;
    }
  }

  for (int i = nc; i < num_channel_rows; i++) {
    chan[i] = strdup("");
    chan_ondate[i] = 2000001;
    chan_chanid[i] = -1;  /* generate new unique id through standard interface */
    chan_offdate[i] = -1;
    chan_ctype[i] = strdup("n");
    chan_edepth[i] = 0.0;
    chan_hang[i] = 0.0;
    chan_vang[i] = 0.0;
    chan_descrip[i] = strdup("");
  }

  for (int i = 0; i < num_channel_rows; i++) {
    sensor_time[i] = 946684800.0;  /* 2000-01-01 midnight */
    sensor_endtime[i] = 2145916800.0;  /* 2038-01-01 midnight */
    sensor_inid[i] = -1;
    sensor_jdate[i] = 2000001;
    sensor_calratio[i] = 1.0;
    sensor_calper[i] = -1.0;
    sensor_tshift[i] = 0.0;
    sensor_instant[i] = strdup("y");
  }

  if (instruments_choice.size() > 0) {
    for (int i = 0; i < nc; i++) {
      for (int j = 0; j < num_template_sensors; j++) {
	if ((!strcmp(sta, template_sensors[j].sta))
	    && (!strcmp(chan[i], template_sensors[j].chan))) {
	  sensor_inid[i] = template_sensors[j].inid;
	  break;
	}
      }
    }
  }

  net = strdup(sta);
  for (int i = 0; i < num_template_affiliations; i++) {
    if (!strcmp(sta, template_affiliations[i].sta)) {
      net = strdup(template_affiliations[i].net);
    }
  }

  refreshFormValues();

  setButtonsSensitive();
}

void AddStation::refreshFormValues(void)
{
  int y, m, d;
  char s[100];

  sta_text->setString(sta);
  lat_text->setString("%.4f", lat);
  lon_text->setString("%.4f", lon);
  elev_text->setString("%.6f", elev);
  staname_text->setString(staname);
  ss_toggle->set((statype == 0) ? 1 : 0);
  ar_toggle->set((statype == 1) ? 1 : 0);
  refsta_text->setString(refsta);
  dnorth_text->setString("%.4f", dnorth);
  deast_text->setString("%.4f", deast);

  for (int i = 0; i < num_channel_rows; i++) {
    chan_text[i]->setString(chan[i]);
    if (chan_ondate[i] == -1) {
      ondate_text[i]->setString("-1");
    }
    else {
      y = chan_ondate[i]/1000;
      timeMonthDay(y, chan_ondate[i] - (y * 1000), &m, &d);
      snprintf(s, sizeof(s), "%d-%02d-%02d", y, m, d);
      ondate_text[i]->setString(s);
    }
    if (chan_offdate[i] == -1) {
      offdate_text[i]->setString("-1");
    }
    else {
      y = chan_offdate[i]/1000;
      timeMonthDay(y, chan_offdate[i] - (y * 1000), &m, &d);
      snprintf(s, sizeof(s), "%d-%02d-%02d", y, m, d);
      offdate_text[i]->setString(s);
    }
    ctype_text[i]->setString(chan_ctype[i]);
    edepth_text[i]->setString("%.6f", chan_edepth[i]);
    hang_text[i]->setString("%.6f", chan_hang[i]);
    vang_text[i]->setString("%.6f", chan_vang[i]);
    descrip_text[i]->setString(chan_descrip[i]);
    if (instruments_choice.size() > 0) {
      instrument[i]->setChoice("-");
      for (int j = 0; j < instruments_choice.size(); j++) {
	if (sensor_inid[i] == instruments_choice[j]->inid) {
	  instrument[i]->setChoice(instruments_choice_text[j]);
	  break;
	}
      }
    }
  }

  net_text->setString(net);

  setButtonsSensitive();
}

ParseCmd AddStation::parseCmd(const string &cmd, string &msg)
{
  string c;

  if(parseCompare(cmd, "add")) {
    add();
  }
  else if(parseCompare(cmd, "cancel")) {
    cancel();
  }
  else if(parseCompare(cmd, "reset")) {
    reset();
  }
  else if(parseCompare(cmd, "validate")) {
    validate();
  }
  else if(parseCompare(cmd, "copy_station")) {
    copy_station();
  }
  else if(parseCompare(cmd, "copy_template")) {
    copy_template();
  }
  else if(parseCompare(cmd, "Help")) {
    char prefix[200];
    getParsePrefix(prefix, sizeof(prefix));
    parseHelp(prefix);
  }
  else {
    return FormDialog::parseCmd(cmd, msg);
  }
  return COMMAND_PARSED;
}

void AddStation::parseHelp(const char *prefix)
{
    printf("%sadd\n", prefix);
    printf("%scancel\n", prefix);
    printf("%sreset\n", prefix);
    printf("%svalidate\n", prefix);
    printf("%scopy_station\n", prefix);
    printf("%scopy_template\n", prefix);
}
