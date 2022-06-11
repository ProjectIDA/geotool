#ifndef _ADD_STATION_H
#define _ADD_STATION_H

#include "motif++/FormDialog.h"
#include "motif++/MotifClasses.h"
#include "TableViewer.h"
#include "gobject++/CssTableClass.h"
#include "gobject++/CssTables.h"
#include "CSSTable.h"

/**
 *  @ingroup libgx
 */
class AddStation : public FormDialog
{
 public:

  AddStation(const string &name, Component *parent,
	     TableViewer *table_viewer,
	     ActionListener *listener);
  ~AddStation(void);

  void actionPerformed(ActionEvent *action_event);

  ParseCmd parseCmd(const string &cmd, string &msg);
  static void parseHelp(const char *prefix);

 protected:

  TableViewer *tv;
  int num_current_sites; gvector<CssTableClass *> current_sites;
  int num_sitechans; gvector<CssTableClass *> sitechans;
  int num_affiliations; gvector<CssTableClass *> affiliations;
  int num_sensors; gvector<CssTableClass *> sensors;
  cvector<CssInstrumentClass> instruments_all, instruments_choice;
  char **instruments_choice_text;

  Label *station_label, *channel_label;
  Label *sta_label, *lat_label, *lon_label;
  Label *elev_label, *staname_label;
  Label *statype_label, *net_label, *refsta_label, *dnorth_label, *deast_label;
  Label *chan_label, *ondate_label, *offdate_label, *ctype_label;
  Label *edepth_label, *hang_label, *vang_label, *descrip_label;
  Label *instrument_label;
  Label *templates_label;
  RowColumn *rc, *controls;
  Form *channels;
  TextField *sta_text, *lat_text, *lon_text;
  TextField *elev_text, *staname_text;
  TextField *statype_text, *net_text, *refsta_text, *dnorth_text, *deast_text;
  TextField **chan_text, **ondate_text, **offdate_text, **ctype_text;
  TextField **edepth_text, **hang_text, **vang_text,**descrip_text;
  Choice **instrument;
  Separator *sep1, *sep2;
  RadioBox *statype_rb;
  Toggle *ss_toggle, *ar_toggle;
  Button *add_button, *cancel_button, *reset_button, *validate_button;
  Button *copy_station_button, *copy_template_button;
  Choice *copy_stations, *copy_templates;

  /* site */
  char *sta;
  long ondate;
  long offdate;
  double lat;
  double lon;
  double elev;
  char *staname;
  int statype;
  char *refsta;
  double dnorth;
  double deast;

  /* sitechan */
  char **chan;
  long *chan_ondate;
  long *chan_chanid;
  long *chan_offdate;
  char **chan_ctype;
  double *chan_edepth;
  double *chan_hang;
  double *chan_vang;
  char **chan_descrip;

  /* affiliation */
  char *net;

  /* sensor */
  double *sensor_time;
  double *sensor_endtime;
  long *sensor_inid;
  long *sensor_jdate;
  double *sensor_calratio;
  double *sensor_calper;
  double *sensor_tshift;
  char **sensor_instant;

  void createInterface(void);
  void setButtonsSensitive(void);
  bool validateForm(bool displayWarning);
  void validate(void);
  void reset(void);
  void cancel(void);
  void add(void);
  void copy_station(void);
  void copy_template(void);

 private:
  void refreshFormValues(void);
};

typedef struct addstation_template_site_t
{
	char	sta[7];		// station identifier
	long	ondate;		// Julian start date
	long	offdate;	// Julian off date
	double	lat;		// latitude
	double	lon;		// longitude
	double	elev;		// elevation
	char	staname[51];	// station description
	char	statype[5];	// station type: single station, virt array, etc
	char	refsta[7];	// reference station for array members
	double	dnorth;		// offset from array reference (km)
	double	deast;		// offset from array reference (km)
} addstation_template_site_t;

typedef struct addstation_template_sitechan_t
{
	char	sta[7];		// station identifier
	char    chan[9];	// channel identifier
	long	ondate;		// Julian start date
	long	chanid;		// channel id
	long	offdate;	// Julian off date
	char	ctype[5];	// channel type
	double	edepth;		// emplacement depth
	double	hang;		// horizontal angle
	double	vang;		// vertical angle
	char	descrip[51];	// channel description
} addstation_template_sitechan_t;

typedef struct addstation_template_affiliation_t
{
	char	net[9];		// unique network identifier
	char	sta[7];		// station identifier
} addstation_template_affiliation_t;

typedef struct addstation_template_sensor_t
{
 	char    sta[7];		// station code
	char    chan[9];	// channel code
	double  time;		// epoch time of start of recording period
	double  endtime;	// epoch time of end of recording period
	long	inid;		// instrument id
	long	chanid;		// channel id
	long	jdate;		// julian date
	double	calratio;	// calibration
	double	calper;		// calibration period
	double	tshift;		// correction of data processing time
	char	instant[2];	// (y,n) discrete/continuing snapshot{
} addstation_template_sensor_t;

#endif
