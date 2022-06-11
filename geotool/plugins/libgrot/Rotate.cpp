/** \file Rotate.cpp
 *  \brief Defines class Rotate.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>

#include "Rotate.h"
#include "RotateData.h"
#include "AmpData.h"
#include "CalibData.h"
#include "TaperData.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void scaleWaveforms(int npts, float *z, float *n, float *e);

#define sameSamprates(rate1,rate2) (fabs(rate1-rate2) < rate1*0.0001)

static bool get_resources = true;

typedef struct
{
    Boolean	set_hang;
    Pixel	azimuth_color;
} SubData, *SubDataPtr;

static SubData	sub_data;

#define XtNsetHang              "setHang"
#define XtNazimuthColor         "azimuthColor"

#define XtCSetHang              "SetHang"
#define XtCAzimuthColor         "AzimuthColor"

#define offset(field)   XtOffset(SubDataPtr, field)
static XtResource	resources[] =
{
	{(char *)XtNsetHang, (char *)XtCSetHang, XtRBoolean, sizeof(Boolean),
	    offset(set_hang), XtRString, (XtPointer)"True"},
	{(char *)XtNazimuthColor, (char *)XtCAzimuthColor, XtRPixel,
	    sizeof(Pixel), offset(azimuth_color), XtRString, (XtPointer)"red"},
};
#undef offset

static void
GetResources(Widget w, XtPointer subdata)
{
    XtGetSubresources(w, subdata, "data", "Data", resources,
			XtNumber(resources), NULL, 0);
    get_resources = False;
}

using namespace libgrot;


Rotate::Rotate(const char *name, Component *parent, DataSource *ds)
		: FormDialog(name, parent, false), DataReceiver(ds)
{
    createInterface();
    init();
}

void Rotate::createInterface(void)
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
    apply_button = new Button("Apply", controls, this);
    unrotate_button = new Button("Unrotate", controls, this);
    save_button = new Button("Save", controls, this);
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
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 2); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 2); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_BOTTOM); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNsliderSize, 100); n++;
    XtSetArg(args[n], XmNincrement, 5); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    scrollbar = new ScrollBar("scrollbar", this, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 20); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, scrollbar->baseWidget()); n++;
    XtSetArg(args[n], XmNrightOffset, 20); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    rc = new RowColumn("rc", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    dial_rc = new RowColumn("dial_rc", rc, args, n);

    label = new Label("Station to Source\nAzimuth", dial_rc);

    form = new Form("form", dial_rc);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
    W_label = new Label("W", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    E_label = new Label("E", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    N_label = new Label("N", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    S_label = new Label("S", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, N_label->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, S_label->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, W_label->baseWidget()); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, E_label->baseWidget()); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNmarkers, 24); n++;
    XtSetArg(args[n], XtNindicatorColor, stringToPixel("Red")); n++;
    XtSetArg(args[n], XtNarrow1Color, stringToPixel("Blue")); n++;
    XtSetArg(args[n], XtNarrow2Color, stringToPixel("Black")); n++;
    XtSetArg(args[n], XtNposition, 270); n++;
    dial = new DialClass("dial", form, args, n);
    dial->addActionListener(this, XtNselect);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    rc2 = new RowColumn("rc2", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNforeground, stringToPixel("Red")); n++;
    azimuth_label = new Label("Azimuth", rc2, args, n);
    incidence_label = new Label("Incidence", rc2);
    n = 0;
    XtSetArg(args[n], XmNforeground, stringToPixel("Blue")); n++;
    maximum_button = new Button("Maximum", rc2, args, n, this);
    origin_button = new Button("Origin", rc2, this);
    phase_choice = new Choice("phase_choice", rc2, this);
    phase_choice->addItem("P-dist");
    phase_choice->addItem("S-dist");

    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "270.0"); n++;
    azimuth_text = new TextField("Azimuth", rc2, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, True); n++;
    XtSetArg(args[n], XmNvalue, "90.0"); n++;
    incidence_text = new TextField("Incidence", rc2, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    maximum_text = new TextField("maximum", rc2, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    origin_text = new TextField("origin", rc2, args, n);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 5); n++;
    XtSetArg(args[n], XmNeditable, False); n++;
    distance_text = new TextField("distance", rc2, args, n);
}

void Rotate::init(void)
{
    same_methods = NULL;
    last_rotated[0] = NULL;
    last_rotated[1] = NULL;
    memset(map_sta, 0, sizeof(map_sta));
    sta_lat = -999.;
    sta_lon = -999.;
    map_azimuth = -999.;
    map_distance = -999.;

    sta_to_src_azimuth = -90.;
    dial->setPosition(270);
    reverse_azimuth =  90.; // 90. degrees from North to the x-axis
    distance = -999.;
    incidence = 90.; // 90. degrees from Vertical to the x-axis

    if(get_resources) {
	GetResources(base_widget, (XtPointer)&sub_data);
    }

    map = Application::getMap(this);
    if(map) {
	map->addActionListener(this, XtNsetVisibleCallback);
    }
    else {
	Application::addMapListener(this, getParent());
    }

    iaspei = new Iaspei();
    id = 0;
    p_npts = 0;
    s_npts = 0;

    getRayParameters(0.);
}

Rotate::~Rotate(void)
{
    if(data_source) data_source->removeDataReceiver(this);
    Free(same_methods);
}

void Rotate::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    const char *reason = action_event->getReason();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(comp == dial) {
        dialAction((DialCallbackStruct *)action_event->getCalldata());
    }
    else if(comp == scrollbar) {
	scrollAction((XmScrollBarCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Azimuth")) {
	setAzimuth();
	rotate();
    }
    else if(!strcmp(cmd, "Incidence")) {
	setIncidence();
	rotate();
    }
    else if(!strcmp(cmd, "phase_choice")) {
	setIncidence();
    }
    else if(!strcmp(cmd, "Maximum")) {
	rotateToMax();
    }
    else if(!strcmp(cmd, "Origin")) {
	rotateToOrigin();
    }
    else if(!strcmp(cmd, "Unrotate")) {
	unrotate();
    }
    else if(!strcmp(cmd, "Apply")) {
	rotate();
    }
    else if(!strcmp(cmd, "Save")) {
	saveAzInc();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Rotation Help");
    }
    else if(!strcmp(reason, XtNmapCallback)) {
	if(!map) {
	    map = (BasicMap *)comp;
	    map->addActionListener(this, XtNsetVisibleCallback);
	    drawOnMap(map_sta, sta_lat, sta_lon, map_azimuth, map_distance);
	}
    }
    else if(!strcmp(reason, XtNsetVisibleCallback)) {
	if(comp == map && map->isVisible()) {
	    drawOnMap(map_sta, sta_lat, sta_lon, map_azimuth, map_distance);
	}
    }
}

ParseCmd Rotate::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseArg(cmd, "azimuth", c)) {
	azimuth_text->setString(c, true);
    }
    else if(parseArg(cmd, "incidence", c)) {
	incidence_text->setString(c, true);
    }
    else if(parseCompare(cmd, "maximum")) {
	rotateToMax();
    }
    else if(parseCompare(cmd, "origin")) {
	rotateToOrigin();
    }
    else if(parseCompare(cmd, "unrotate")) {
	unrotate();
    }
    else if(parseCompare(cmd, "apply")) {
	rotate();
    }
    else if(parseCompare(cmd, "rotate", 6)) {
	if(parseGetArg(cmd, "azimuth", c)) {
	    azimuth_text->setString(c, true);
	}
	if(parseGetArg(cmd, "incidence", c)) {
	    incidence_text->setString(c, true);
	}
	rotate();
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

ParseVar Rotate::parseVar(const string &name, string &value)
{
    double d;

    if(parseCompare(name, "azimuth")) {
	if(azimuth_text->getDouble(&d)) {
	    parsePrintDouble(value, d);
	}
	else {
	    value.assign("-999");
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "incidence")) {
	if(incidence_text->getDouble(&d)) {
	    parsePrintDouble(value, d);
	}
	else {
	    value.assign("-999");
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "P_dist")) {
	phase_choice->setChoice("P-dist");
	if(distance_text->getDouble(&d)) {
	    parsePrintDouble(value, d);
	}
	else {
	    value.assign("-999");
	}
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "S_dist")) {
	phase_choice->setChoice("S-dist");
	if(distance_text->getDouble(&d)) {
	    parsePrintDouble(value, d);
	}
	else {
	    value.assign("-999");
	}
	return STRING_RETURNED;
    }
    return FormDialog::parseVar(name, value);
}

void Rotate::parseHelp(const char *prefix)
{
    printf("%sazimuth=AZIMUTH\n", prefix);
    printf("%sincidence=INCIDENCE\n", prefix);
    printf("%sapply\n", prefix);
    printf("%srotate\n", prefix);
    printf("%smaximum\n", prefix);
    printf("%sorigin\n", prefix);
    printf("%sunrotate\n", prefix);
}

void Rotate::dialAction(DialCallbackStruct *s)
{
    string errstr;
    int i, j, k, n;

    sta_to_src_azimuth = s->position;
    reverse_azimuth = sta_to_src_azimuth + 180.;
    if(reverse_azimuth > 360.) reverse_azimuth -= 360.;

    dial->setPosition(s->position);
    azimuth_text->setString("%.1f", sta_to_src_azimuth);

    if(s->event->type == ButtonPress)
    {
	if( data_source->getSelectedComponents(num_cmpts, wlist) <= 0)
	{
	    putWarning("No orthogonal waveform components selected.");
	    return;
 	}

	if( !RotateData::checkComponents(sub_data.set_hang, incidence,
				num_cmpts, wlist, errstr) )
	{
	    if( !errstr.empty() ) putWarning(errstr.c_str());
	    return;
	}
	if( !errstr.empty() ) putWarning(errstr.c_str());
	n = 0;
	for(i = 0; i < (int)num_cmpts.size(); i++) {
	    if(num_cmpts[i] == 2) {
		RotateData::rotateWaveforms(reverse_azimuth, wlist[n],
				wlist[n+1], errstr);
	    }
	    else if(num_cmpts[i] == 3) {
		RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wlist[n], wlist[n+1], wlist[n+2], errstr);
	    }
	    if( !errstr.empty() ) putWarning(errstr.c_str());
	    n += num_cmpts[i];
	}

	if((int)num_cmpts.size() == 0) return;

	Free(same_methods);

	if( !(same_methods = (bool *)mallocWarn(num_cmpts.size()*sizeof(bool))) ) {
	    return;
	}
	/* check if we can rotate without re-reading and processing data.
	 */
	n = 0;
	for(i = 0; i < (int)num_cmpts.size(); i++)
	{
	    same_methods[i] = false;

	    gvector<DataMethod *> *m[3];
	    m[0] = wlist[n]->dataMethods();
	    m[1] = NULL;
	    m[2] = NULL;

	    for(k = 1; k < num_cmpts[i]; k++) {
		m[k] = wlist[n+k]->dataMethods();
		if(m[k]->size() != m[0]->size()) break;
	    }
	    n += num_cmpts[i];

	    // if the number of methods applied to each waveform is not the same
	    if(k != num_cmpts[i]) {
		for(k = 0; k<3; k++) if(m[k]) delete m[k];
		continue;
	    }

	    /* The first method will be the Rotate just applied above by
	     * rotateWaveforms. Skip it.
	     */
	    for(j = 1; j < (int)m[0]->size(); j++)
	    {
		DataMethod *d[3];
		for(k = 0; k < num_cmpts[i]; k++) {
		    d[k] = m[k]->at(j);
		}

		for(k = 0; k < num_cmpts[i]; k++) {
		    if( !d[k]->getCopyDataInstance() ) break;
		}
		if(k == num_cmpts[i]) continue; // all methods are CopyData

		for(k = 0; k < num_cmpts[i]; k++) {
		    if( !d[k]->getCutDataInstance() ) break;
		}
		if(k == num_cmpts[i]) continue; // all methods are CutData

		// otherwise the methods must be the same for each waveform
		AmpData *a;
		CalibData *c;
		IIRFilter *iir;
		TaperData *t;
		if((a = d[0]->getAmpDataInstance())) {
		    for(k = 1; k < num_cmpts[i]; k++) {
			if( !a->Equals(d[k]->getAmpDataInstance()) ) break;
//			if( *d[k] != *d[0] ) break;
		    }
		    if(k < num_cmpts[i]) break;
		}
		else if((c = d[0]->getCalibDataInstance())) {
		    for(k = 1; k < num_cmpts[i]; k++) {
			if( !c->Equals(d[k]->getCalibDataInstance()) ) break;
		    }
		    if(k < num_cmpts[i]) break;
		}
		else if((iir = d[0]->getIIRFilterInstance())) {
		    for(k = 1; k < num_cmpts[i]; k++) {
			if( !iir->Equals(d[k]->getIIRFilterInstance()) ) break;
		    }
		    if(k < num_cmpts[i]) break;
		}
		else if((t = d[0]->getTaperDataInstance())) {
		    for(k = 1; k < num_cmpts[i]; k++) {
			if( !t->Equals(d[k]->getTaperDataInstance()) ) break;
		    }
		    if(k < num_cmpts[i]) break;
		}
	    }
	    if(j < (int)m[0]->size()) {
		for(k = 0; k<3; k++) if(m[k]) delete m[k];
		continue;
	    }

	    // Check if all previously applied methods are from the classes
	    // listed below. If so, then the RotateData method can be
	    // be applied without reapplying the previous methods.
	    for(k = 0; k < num_cmpts[i]; k++) {
		for(j = 0; j < (int)m[k]->size(); j++) {
		    DataMethod *d = (DataMethod *)m[k]->at(j);
		    if( !d->rotationCommutative() ) break;
		}
		if(j < (int)m[k]->size()) break;
	    }
	    if(k == num_cmpts[i]) {
		same_methods[i] = true;
	    }
	    for(k = 0; k<3; k++) if(m[k]) delete m[k];
	}
    }
    else if(s->event->type == MotionNotify)
    {
	n = 0;
	for(i = 0; i < (int)num_cmpts.size(); i++)
	{
	    if(same_methods[i])
	    {
		/* apply rotation to those waveforms that have the same methods,
		 * and the rotation is commutative with the other methods.
		 */
		if(num_cmpts[i] == 2) {
		    RotateData::rotate(wlist[n]->ts, wlist[n+1]->ts, NULL,
				90.-reverse_azimuth, incidence-90., 0.,
				ROTATE_XY);
		}
		else if(num_cmpts[i] == 3) {
		    RotateData::rotate(wlist[n]->ts, wlist[n+1]->ts,
			wlist[n+2]->ts, 90.-reverse_azimuth, incidence-90.,
			0., ROTATE_XYZ);
		}
	    }
	    else {
		if(num_cmpts[i] == 2) {
		    RotateData::rotateWaveforms(reverse_azimuth, wlist[n],
					wlist[n+1], errstr);
		}
		else if(num_cmpts[i] == 3) {
		    RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wlist[n], wlist[n+1], wlist[n+2], errstr);
		}
		if( !errstr.empty() ) putWarning(errstr.c_str());
	    }
	    n += num_cmpts[i];
	}
    }
    else if(s->event->type == ButtonRelease)
    {
	Free(same_methods);
	/* just to update the rotation objects
	*/
	n = 0;
	for(i = 0; i < (int)num_cmpts.size(); i++) {
	    if(num_cmpts[i] == 2) {
		RotateData::rotateWaveforms(reverse_azimuth, wlist[n],
				wlist[n+1], errstr);
	    }
	    else if(num_cmpts[i] == 3) {
		RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wlist[n], wlist[n+1], wlist[n+2], errstr);
	    }
	    if( !errstr.empty() ) putWarning(errstr.c_str());
	    n += num_cmpts[i];
	}
	data_source->modifyWaveforms(wlist);
	return;
    }
 
    data_source->modifyWaveforms(wlist);

    if((int)num_cmpts.size() == 1)
    {
	mapAzimuth(wlist[0], sta_to_src_azimuth);
	if(wlist[0]!=last_rotated[0] || wlist[1]!=last_rotated[1])
	{
	    Arg args[2];
	    maximum_text->setString("");
	    origin_text->setString("");
	    XtSetArg(args[0], XtNarrow1Visible, FALSE);
	    XtSetArg(args[1], XtNarrow2Visible, FALSE);
	    dial->setValues(args, 2);
	}
	last_rotated[0] = wlist[0];
	last_rotated[1] = wlist[1];
    }
}

void Rotate::setAzimuth(void)
{
    Arg args[2];
    int position;
    double az;

    if(!azimuth_text->getDouble(&az)) {
	return;
    }
    sta_to_src_azimuth = az;
    reverse_azimuth = sta_to_src_azimuth + 180.;
    if(reverse_azimuth > 360.) reverse_azimuth -= 360.;

    position = (int)az;
    XtSetArg(args[0], XtNposition, position);
    dial->setValues(args, 1);
}

void Rotate::rotate(void)
{
    int i, n;
    string errstr;
    vector<int> ncmpts;
    gvector<Waveform *> wvec;

    if( data_source->getSelectedComponents(ncmpts, wvec) <= 0)
    {
	putWarning("No orthogonal waveform components selected.");
	return;
    }

    if( !RotateData::checkComponents(sub_data.set_hang, incidence,
			ncmpts, wvec, errstr) )
    {
	if( !errstr.empty() ) putWarning(errstr.c_str());
	return;
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());

//    setCursor("hourglass");

    n = 0;
    for(i = 0; i < (int)ncmpts.size(); i++) {
	if(ncmpts[i] == 2) {
	    RotateData::rotateWaveforms(reverse_azimuth, wvec[n],
				wvec[n+1], errstr);
	}
	else if(ncmpts[i] == 3) {
	    RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wvec[n], wvec[n+1], wvec[n+2], errstr);
	}
	if( !errstr.empty() ) putWarning(errstr.c_str());
	n += ncmpts[i];
    }

    data_source->modifyWaveforms(wvec);

    if(ncmpts.size() == 1)
    {
	mapAzimuth(wvec[0], sta_to_src_azimuth);
	if(wvec[0] != last_rotated[0] || wvec[1] != last_rotated[1])
	{
	    Arg args[2];
	    maximum_text->setString("");
	    origin_text->setString("");
	    XtSetArg(args[0], XtNarrow1Visible, FALSE);
	    XtSetArg(args[1], XtNarrow2Visible, FALSE);
	    dial->setValues(args, 2);
	}
	last_rotated[0] = wvec[0];
	last_rotated[1] = wvec[1];
    }

//    setCursor("default");
}

void Rotate::setIncidence(void)
{
    Arg args[3];
    int min, max, slider_size, value;
    double angle;

    if(!incidence_text->getDouble(&angle)) {
	return;
    }
    incidence = angle;
    if(incidence < 0.) {
	incidence_text->setString("0.");
	incidence = 0.;
    }
    else if(incidence > 90.) {
	incidence_text->setString("90.");
	incidence = 90.;
    }
    XtSetArg(args[0], XmNminimum, &min);
    XtSetArg(args[1], XmNmaximum, &max);
    XtSetArg(args[2], XmNsliderSize, &slider_size);
    scrollbar->getValues(args, 3);

    value = 2*(int)((90.-incidence)*(max-slider_size-min)/180.) + min;
    scrollbar->setValue(value);

    computeDistance(incidence);
}

void Rotate::getRayParameters(double depth)
{
    int i, n;

    for(i = 0; i < 500; i++) {
	p_ray_p[i] = -1.;
	s_ray_p[i] = -1.;
    }
    n = 0;
    iaspei->getCurve("Pn", depth, &n, p_tt, p_dist, p_ray_p);
    for(i = 0; i < n; i++) {
        if(p_ray_p[i] >= 0. && p_dist[i] > 20.) break;
    }
    n = i;
    iaspei->getCurve("P", depth, &p_npts, p_tt+n, p_dist+n, p_ray_p+n);
    p_npts += n;

    n = 0;
    iaspei->getCurve("Sn", depth, &n, s_tt, s_dist, s_ray_p);
    for(i = 0; i < n; i++) {
        if(s_ray_p[i] >= 0. && s_dist[i] > 20.) break;
    }
    n = i;
    iaspei->getCurve("S", depth, &s_npts, s_tt+n, s_dist+n, s_ray_p+n);
    s_npts += n;
}

void Rotate::computeDistance(double inc)
{
    double diff, dmin, rayp;

    // find the ray parameters closest to this incidence
    distance = -999.;
    if(!strcmp(phase_choice->getChoice(), "P-dist"))
    {
	rayp = 6371.*sin(inc*PI/180.)/5.80;
	if(p_npts > 0) {
	    dmin = fabs(rayp - p_ray_p[0]);
	    distance = p_dist[0];
	}
	for(int i = 1; i < p_npts; i++) {
	    // isop_c get_seg() uses set_fnan to denote a break in the curve
//	    if(!fNaN(p_dist[i])) {
	    if(p_ray_p[i] >= 0.) {
		diff = fabs(rayp - p_ray_p[i]);
		if(dmin > diff) {
		    dmin = diff;
		    distance = p_dist[i];
		}
	    }
	}
    }
    else
    {
	rayp = 6371.*sin(inc*PI/180.)/3.36;
	if(s_npts > 0) {
	    dmin = fabs(rayp - s_ray_p[0]);
	    distance = s_dist[0];
	}
        for(int i = 1; i < s_npts; i++) {
	    // isop_c get_seg() uses set_fnan to denote a break in the curve

//	    if(!fNaN(s_dist[i])) {
	    if(s_ray_p[i] >= 0.) {
		diff = fabs(rayp - s_ray_p[i]);
		if(dmin > diff) {
		    dmin = diff;
		    distance = s_dist[i];
		}
	    }
	}
    }
    if(distance > 0.) {
	distance_text->setString("%.2f", distance);
    }
    else {
	distance_text->setString("");
    }
}

void Rotate::scrollAction(XmScrollBarCallbackStruct *s)
{
    int min, max, slider_size;
    Arg args[3];

    XtSetArg(args[0], XmNminimum, &min);
    XtSetArg(args[1], XmNmaximum, &max);
    XtSetArg(args[2], XmNsliderSize, &slider_size);
    scrollbar->getValues(args, 3);

    incidence = (double)(s->value-min)*180./(max-slider_size-min);
    incidence = ((int)(incidence+.5))*.5;
    if(incidence < 0.) incidence = 0.;
    if(incidence > 90.) incidence = 90.;
    incidence = 90. - incidence;
    incidence_text->setString("%.1f", incidence);

    rotate();

    computeDistance(incidence);
}

/** rotate to the azimuth and incidence of maximum rectilinearity
 */
void Rotate::rotateToMax(void)
{
    int i, n;
    string errstr;
    vector<int> ncmpts;
    bool windowed;
    gvector<Waveform *> wvec, ws;

    windowed = true;
    data_source->getSelectedComponents("a", ncmpts, wvec);
    if(ncmpts.size() <= 0 && !data_source->dataWindowIsDisplayed("a"))
    {
	windowed = false;
	data_source->getSelectedComponents(ncmpts, wvec);
    }

    if( !RotateData::checkComponents(sub_data.set_hang, incidence,
				ncmpts, wvec, errstr) )
    {
	if( !errstr.empty() ) putWarning(errstr.c_str());
	return;
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());
    if(ncmpts.size() <= 0) {
	putWarning("No orthogonal waveform components selected.");
	return;
    }

    n = 0;
    for(i = 0; i < (int)ncmpts.size(); i++)
    {
	ws.clear();
	for(int j = 0; j < ncmpts[i]; j++) ws.push_back(wvec[n+j]);
	rotateToMax(windowed, ws);
	n += ncmpts[i];
    }

    if((int)ncmpts.size() == 1)
    {
	Arg args[2];
	XtSetArg(args[0], XtNarrow1Visible, TRUE);
	XtSetArg(args[1], XtNarrow1Position, (int)sta_to_src_azimuth);
	dial->setValues(args, 2);

	XtSetArg(args[0], XtNposition, (int)sta_to_src_azimuth);
	dial->setValues(args, 1);

	if(wvec[0] != last_rotated[0] || wvec[1] != last_rotated[1])
	{
	    origin_text->setString("");
	    XtSetArg(args[0], XtNarrow2Visible, FALSE);
	    dial->setValues(args, 1);
	}
	last_rotated[0] = wvec[0];
	last_rotated[1] = wvec[1];

	mapAzimuth(wvec[0], sta_to_src_azimuth);
    }
}

void Rotate::rotateToMax(bool windowed, gvector<Waveform *> &wvec)
{
    int i, npts;
    string errstr;
    double diff, az, inc, mean[3];
    double s[9], d[3], o[3], v[9];
    float *e=NULL, *n=NULL, *z=NULL;

    if(windowed)
    {
	GDataPoint *d1 = wvec[0]->dw[0].d1;
	GDataPoint *d2 = wvec[0]->dw[0].d2;
	GDataPoint *dlast;

	if(d1->segmentIndex() != d2->segmentIndex()) {
	    showWarning("Data gap: %s/%s", wvec[0]->sta(), wvec[0]->chan());
	    return;
	}
	npts = d2->index() - d1->index() + 1;

	dlast = wvec[0]->dw[0].d1;
	for(i = 1; i < wvec.size(); i++)
	{
	    d1 = wvec[i]->dw[0].d1;
	    diff = fabs(dlast->segment()->tdel() - d1->segment()->tdel());
	    dlast = d1;

	    if(diff/d1->segment()->tdel() > .01) {
		showWarning("variable sample interval.");
		return;
	    }
	    d2 = wvec[i]->dw[0].d2;

            if(d1->segmentIndex() != d2->segmentIndex()) {
                showWarning("Data gap: %s/%s",wvec[i]->sta(), wvec[i]->chan());
                return;
            }
            if(npts > d2->index() - d1->index() + 1) {
                npts = d2->index() - d1->index() + 1;
            }
	}
    }
    else
    {
	for(i = 0; i < wvec.size(); i++) {
            if(wvec[i]->size() > 1) {
		showWarning("Data gap: %s/%s",wvec[i]->sta(), wvec[i]->chan());
		return;
	    }
	}

	npts = wvec[0]->length();
	for(i = 1; i < wvec.size(); i++)
        {
	    diff = fabs(wvec[i]->segment(0)->tdel() -
			wvec[0]->segment(0)->tdel());
	    if(diff/wvec[i]->segment(0)->tdel() > .01) {
		showWarning("non constant sample rate.");
		return;
	    }
	    if(npts > wvec[i]->length()) {
		npts = wvec[i]->length();
	    }
	}
    }

    if( !(z = (float *)mallocWarn(npts*sizeof(float))) ||
	!(n = (float *)mallocWarn(npts*sizeof(float))) ||
	!(e = (float *)mallocWarn(npts*sizeof(float))))
    {
	Free(z); Free(n); Free(e);
	return;
    }
    // First rotate the waveforms to the local E,N,Z coordinate system
    if(wvec.size() == 2) {
	RotateData::rotateWaveforms(90., wvec[0], wvec[1], errstr);
    }
    else if(wvec.size() == 3) {
	RotateData::rotateWaveforms(90., 90., wvec[0], wvec[1], wvec[2],errstr);
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());
    mean[0] = mean[1] = mean[2] = 0.;
    for(i = 0; i < wvec.size(); i++) {
	mean[i] = wvec[i]->mean();
    }

    if(windowed) {
	GDataPoint *d1 = wvec[0]->dw[0].d1;
	GDataPoint *d2 = wvec[1]->dw[0].d1;
	GSegment *seg;
	seg = wvec[0]->segment(d1->segmentIndex());
	memcpy(e, seg->data + d1->index(), npts*sizeof(float));
	seg = wvec[1]->segment(d2->segmentIndex());
	memcpy(n, seg->data + d2->index(), npts*sizeof(float));

	if(wvec.size() == 3) {
	    GDataPoint *d3 = wvec[2]->dw[0].d1;
	    seg = wvec[2]->segment(d3->segmentIndex());
	    memcpy(z, seg->data + d3->index(), npts*sizeof(float));
	}
	else {
	    for(i = 0; i < npts; i++) z[i] = 1.;
	}
    }
    else {
	memcpy(e, wvec[0]->segment(0)->data, npts*sizeof(float));
	memcpy(n, wvec[1]->segment(0)->data, npts*sizeof(float));
	if(wvec.size() == 3) {
	    memcpy(z, wvec[2]->segment(0)->data, npts*sizeof(float));
	}
	else {
	    for(i = 0; i < npts; i++) z[i] = 1.;
	}
    }

    for(i = 0; i < npts; i++) {
	e[i] -= mean[0];
	n[i] -= mean[1];
	z[i] -= mean[2];
    }

    scaleWaveforms(npts, z, n, e);

    covar(npts, e, n, z, s);

    Free(e); Free(n); Free(z);

    tred2(3, s, d, o, v);
    tql2(3, d, o, v);
    for(i = 0; i < 3; i++) if(d[i] < 0.) d[i] = 0.;
    d[0] = sqrt(d[0]);      // make units amp instead of power
    d[1] = sqrt(d[1]);
    d[2] = sqrt(d[2]);

/*
    if(d[2] == 0.) {
	rectilinearity = 0.;
    }
    else {
	rectilinearity = 1. - .5*(d[0] + d[1])/d[2];
    }
*/
    i = (v[8] > 0.) ? -1 : 1;
    az = 180.*atan2(i*v[6], i*v[7])/M_PI;
    inc = 180.*acos(fabs(v[8]))/M_PI;

    azimuth_text->setString("%.2f", az);
    maximum_text->setString("%.2f", az);
    setAzimuth();

    incidence_text->setString("%.2f", inc);
    setIncidence();

    if(wvec.size() == 2) {
	RotateData::rotateWaveforms(reverse_azimuth, wvec[0], wvec[1], errstr);
    }
    else if(wvec.size() == 3) {
	RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wvec[0], wvec[1], wvec[2], errstr);
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());

    data_source->modifyWaveforms(wvec);
}

static void
scaleWaveforms(int npts, float *z, float *n, float *e)
{
    int i;
    double d, scale;

    scale = 0.;
    for(i = 0; i < npts; i++)
    {
	d = fabs(z[i]);
	if(d > scale) scale = d;
	d = fabs(e[i]);
	if(d > scale) scale = d;
	d = fabs(n[i]);
	if(d > scale) scale = d;
    }
    if(scale != 0.) {
	scale = 1./scale;
	for(i = 0; i < npts; i++) {
	    z[i] *= scale;
	    n[i] *= scale;
	    e[i] *= scale;
	}
    }
}

void Rotate::rotateToOrigin(void)
{
    int i, m, n;
    string errstr;
    vector<int> ncmpts;
    gvector<Waveform *> wvec, ws;
    Arg args[3];
    double delta, az, baz;
    CssOriginClass *origin;

    if( data_source->getSelectedComponents(ncmpts, wvec) <= 0) {
	putWarning("No orthogonal waveform components selected.");
	return;
    }

    if( !RotateData::checkComponents(sub_data.set_hang, incidence,
				ncmpts, wvec, errstr) )
    {
	if( !errstr.empty() ) putWarning(errstr.c_str());
	return;
    }
    if( !errstr.empty() ) putWarning(errstr.c_str());

    if(ncmpts.size() == 1
	    && (origin = data_source->getPrimaryOrigin(wvec[0])) != NULL
	    && origin->lat > -900. && origin->lon > -900.
	    && wvec[0]->lat() > -900. && wvec[0]->lon() > -900.)
    {
	deltaz(origin->lat, origin->lon, wvec[0]->lat(), wvec[0]->lon(),
			&delta, &az, &baz);
	if(baz < -998)
	{
	    XtSetArg(args[0], XtNarrow2Visible, FALSE);
	    dial->setValues(args, 1);
	    return;
	}
	if(baz < 0) {
	    i = -(int)(baz/360) + 1;
	    baz += i*360.;
	}
	else if(baz > 360.) {
	    i = (int)(baz/360);
	    baz -= i*360.;
	}

  	XtSetArg(args[0], XtNarrow2Visible, True);
  	XtSetArg(args[1], XtNarrow2Position, (int)baz);
  	XtSetArg(args[2], XtNposition, (int)baz);
  	dial->setValues(args, 3);
	azimuth_text->setString("%.1f", baz);
	origin_text->setString("%.1f", baz);
	setAzimuth();

	if(wvec[0] != last_rotated[0] || wvec[1] != last_rotated[1]) {
	    maximum_text->setString("");
	    XtSetArg(args[0], XtNarrow1Visible, FALSE);
	    dial->setValues(args, 1);
	}
	last_rotated[0] = wvec[0];
	last_rotated[1] = wvec[1];
    }
    else {
	azimuth_text->setString("");
	XtSetArg(args[0], XtNarrow2Visible, FALSE);
	dial->setValues(args, 1);
    }

    incidence_text->setString("90.");
    setIncidence();
		
    setCursor("hourglass");

    n = 0;
    for(m = 0; m < (int)ncmpts.size(); m++)
    {
	i = n;
	n += ncmpts[m];
	if((origin = data_source->getPrimaryOrigin(wvec[i])) == NULL
		|| origin->lat < -900. || origin->lon < -900.)
	{
	    showWarning("%s/%s: No origin information.",
				wvec[i]->sta(), wvec[i]->chan());
	    continue;
	}
	if(wvec[i]->lat() < -900. || wvec[i]->lon() < -900.)
	{
	    showWarning("%s/%s: No station location.",
			wvec[i]->sta(), wvec[i]->chan());
	    continue;
	}
	deltaz(origin->lat, origin->lon, wvec[i]->lat(), wvec[i]->lon(),
			&delta, &az, &baz);

	sta_to_src_azimuth = baz;
	reverse_azimuth = baz + 180.;
	if(reverse_azimuth > 360.) reverse_azimuth -= 360.;

        if(ncmpts[m] == 2) {
            RotateData::rotateWaveforms(reverse_azimuth, wvec[i], wvec[i+1],
				errstr);
	    ws.push_back(wvec[i]);
	    ws.push_back(wvec[i+1]);
        }
        else if(ncmpts[m] == 3) {
            RotateData::rotateWaveforms(reverse_azimuth, incidence,
				wvec[i], wvec[i+1], wvec[i+2], errstr);
	    ws.push_back(wvec[i]);
	    ws.push_back(wvec[i+1]);
	    ws.push_back(wvec[i+2]);
        }
	if( !errstr.empty() ) putWarning(errstr.c_str());
	if(ncmpts.size() == 1) {
	    mapAzimuth(wvec[0], sta_to_src_azimuth);
	}
    }

    data_source->modifyWaveforms(ws);

    setCursor("default");
}

void Rotate::unrotate(void)
{
    Arg args[3];
    int n;
    vector<int> ncmpts;
    gvector<Waveform *> wvec;

    n = 0;
    XtSetArg(args[n], XtNposition, -90); n++;
    XtSetArg(args[n], XtNarrow1Visible, FALSE); n++;
    XtSetArg(args[n], XtNarrow2Visible, FALSE); n++;
    dial->setValues(args, n);
    azimuth_text->setString("270.");
    incidence_text->setString("90.");
    setIncidence();
    maximum_text->setString("");
    origin_text->setString("");

    if( data_source->getSelectedComponents(ncmpts, wvec) <= 0) {
	showWarning("No orthogonal waveform components selected.");
	return;
    }

    setCursor("hourglass");

    DataMethod::remove("RotateData", wvec);

    for(int i = 0; i < wvec.size(); i++) {
	wvec[i]->ts->getChan(wvec[i]->channel);
    }

    data_source->modifyWaveforms(wvec);

    mapAzimuth(wvec[0], -999.);

    setCursor("default");
}

void Rotate::mapAzimuth(Waveform *w, double az)
{
    drawOnMap(w->sta(), w->lat(), w->lon(), az, distance);
}

void Rotate::drawOnMap(const char *sta, double lat, double lon, double az,
			double dist)
{
    int i, ident, nsta;
    MapPlotArc arc;
    MapPlotDelta delta;
    MapPlotStation *station = NULL;
    LineInfo line_info_init = LINE_INFO_INIT;

    map_azimuth = az;
    map_distance = dist;
    strcpy(map_sta, sta);
    sta_lat = lat;
    sta_lon = lon;

    if(!map || map_sta[0] == '\0' || sta_lat < -900. || sta_lon < -900.) return;

    if(!map->getStaArc((char *)sta, (char *)"rotate", &arc))
    {
	arc.label = (char *)"rotate";
	arc.lat = sta_lat;
	arc.lon = sta_lon;
	arc.az = az;
	arc.del = 180.;
	memcpy(&arc.line, &line_info_init, sizeof(LineInfo));
	arc.line.fg = sub_data.azimuth_color;
	ident = map->addArc(&arc, true);
	nsta = map->getStations(&station);

	for(i = 0; i < nsta; i++) {
	    if(station[i].label != NULL && !strcmp(station[i].label, sta))
	    break;
	}
	if(i < nsta) map->assoc(ident, station[i].id);
	Free(station);
    }
    else
    {
	arc.az = az;
	map->change((MapObject *)&arc, true);
    }

    if(!map->getStaDelta((char *)sta, (char *)"rotate", &delta))
    {
	delta.label = (char *)"rotate";
	delta.lat = lat;
	delta.lon = lon;
	delta.del = dist;
	memcpy(&delta.line, &line_info_init, sizeof(LineInfo));
	delta.line.fg = sub_data.azimuth_color;
//	delta.line.xorr = (reason == 1) ? True : False;
	delta.line.display = MAP_ON;
	ident = map->addDelta(&delta, true);

	nsta = map->getStations(&station);

	for(i = 0; i < nsta; i++) {
	    if(station[i].label != NULL && !strcmp(station[i].label, sta))
	    break;
	}
	if(i < nsta) map->assoc(ident, station[i].id);
	Free(station);
    }
    else
    {
	delta.del = dist;
//	arc.line.xorr = (reason == 1) ? True : False;
	arc.line.xorr = False;
//	arc.line.display = display;
	arc.line.display = MAP_ON;
	map->change((MapObject *)&delta, true);
    }
}

void Rotate::saveAzInc(void)
{
    vector<int> sel;
    int changed[2] = {10, 14};
    vector<const char *> row;
    double azimuth, incidence;
    cvector<CssArrivalClass> arr;
    CssArrivalClass *a;
    DataSource *ds;

    if(!data_source) return;

    data_source->getSelectedTable(arr);
    if(arr.size() <= 0) {
        showWarning("No arrivals selected.");
        return;
    }
    else if(arr.size() > 1) {
        showWarning("More than one arrival selected.");
        return;
    }
    if( !(ds = arr[0]->getDataSource()) ) {
        showWarning("Cannot save changes.");
        return;
    }

    if(!azimuth_text->getDouble(&azimuth)) {
	showWarning("Invalid azimuth value.");
	return;
    }
    else if(!incidence_text->getDouble(&incidence)) {
	showWarning("Invalid incidence value.");
	return;
    }

    a = new CssArrivalClass(*arr[0]);

    a->azimuth = azimuth;
    a->ema = incidence;
    ds->changeTable(arr[0], a);
    arr[0]->azimuth = a->azimuth;
    arr[0]->ema = a->ema;
    TableListener::doCallbacks(arr[0], this, 2, changed);
}

