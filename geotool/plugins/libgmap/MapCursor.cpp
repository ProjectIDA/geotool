/** \file MapCursor.cpp
 *  \brief Defines class MapCursor.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sstream>

#include "MapCursor.h"
#include "MapWindow.h"
#include "WaveformWindow.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "ibase/libgeog.h"
}

using namespace libgmap;

MapCursor::MapCursor(const char *name, Component *parent, MapWindow *mapWindow,
		DataSource *ds) : FormDialog(name, parent, false, false)
{
    Arg args[20];
    int n;

    mw = mapWindow;
    map = mw->map;
    ww = ds->getWaveformWindowInstance();

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    sort_button = new Button("Sort Waveforms", controls, this);
    if(!ww) sort_button->setSensitive(false);
    new Space("space", controls, XmHORIZONTAL, 50);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_CENTER); n++;
    rc = new RowColumn("rc", this, args, n);

    label = new Label("Map Cursor", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc1 = new RowColumn("rc1", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, "0.0"); n++;
    latitude_text = new TextField("Latitude", rc1, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 1); n++;
    XtSetArg(args[n], XmNminimum, -900); n++;
    XtSetArg(args[n], XmNmaximum, 900); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    XmString xm = createXmString("Latitude");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    latitude_scale = new Scale("Latitude", rc1, this, args, n);
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc2 = new RowColumn("rc2", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, "0.0"); n++;
    longitude_text = new TextField("Longitude", rc2, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 1); n++;
    XtSetArg(args[n], XmNminimum, -1800); n++;
    XtSetArg(args[n], XmNmaximum, 1800); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    xm = createXmString("Longitude");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    longitude_scale = new Scale("Longitude", rc2, this, args, n);
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    rc3 = new RowColumn("rc3", rc);
    n = 0;
    XtSetArg(args[n], XmNcolumns, 10); n++;
    XtSetArg(args[n], XmNvalue, "0.0"); n++;
    depth_text = new TextField("Depth", rc3, this, args, n);
    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_RIGHT); n++;
    XtSetArg(args[n], XmNdecimalPoints, 1); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 800); n++;
    XtSetArg(args[n], XmNvalue, 0); n++;
    XtSetArg(args[n], XmNshowValue, False); n++;
    xm = createXmString("Depth");
    XtSetArg(args[n], XmNtitleString, xm); n++;
    depth_scale = new Scale("Depth", rc3, this, args, n);
    XmStringFree(xm);

    n = 0;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); n++;
    region_label = new Label("Seismic Region", rc, args, n);
    added_crosshair = false;
}

MapCursor::~MapCursor(void)
{
}

void MapCursor::setVisible(bool visible)
{
    FormDialog::setVisible(visible);
    if(visible) {
	map->addActionListener(this, XtNcrosshairCallback);
	map->addActionListener(this, XtNcrosshairDragCallback);
	added_crosshair = false;
	if( !map->hasCrosshair() ) {
	    added_crosshair = true;
	    map->addCrosshair();
	}
    }
    else {
	map->removeActionListener(this, XtNcrosshairCallback);
	map->removeActionListener(this, XtNcrosshairDragCallback);
	if(added_crosshair) {
	    map->deleteCrosshair();
	}
    }
}

void MapCursor::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(comp == latitude_text) {
	double lat;
	if(latitude_text->getDouble(&lat)) {
	    setScale(latitude_scale, lat);
	    positionCrosshair();
	}
    }
    else if(comp == longitude_text) {
	double lon;
	if(longitude_text->getDouble(&lon)) {
	    setScale(longitude_scale, lon);
	    positionCrosshair();
	}
    }
    else if(comp == longitude_text) {
	double depth;
	if(depth_text->getDouble(&depth)) {
	    setScale(depth_scale, depth);
	    positionCrosshair();
	}
    }
    else if(comp == latitude_scale) {
	latitude_text->setString("%.1f",
		(double)latitude_scale->getValue()/10.);
	positionCrosshair();
    }
    else if(comp == longitude_scale) {
	longitude_text->setString("%.1f",
			(double)longitude_scale->getValue()/10.);
	positionCrosshair();
    }
    else if(comp == depth_scale) {
	depth_text->setString("%.1f", (double)depth_scale->getValue()/10.);
    }
    else if(comp == map) { // crosshair callback
	crosshairCB();
    }
    else if(comp == sort_button) {
	double lat, lon;
	if(ww && latitude_text->getDouble(&lat) &&
		longitude_text->getDouble(&lon))
	{
	    ww->sortByDistanceFrom(lat, lon);
	}
    }
    else if(comp == help_button) {
	showHelp("Map Cursor Help");
    }
}

void MapCursor::setScale(Scale *scale, double value)
{
    int ivalue = (int)(10*value);
    if(ivalue < scale->getMinimum()) ivalue = scale->getMinimum();
    else if(ivalue > scale->getMaximum()) ivalue = scale->getMaximum();
    scale->setValue(ivalue);
}

void MapCursor::positionCrosshair(void)
{
    double lat, lon;

    if(latitude_text->getDouble(&lat) && longitude_text->getDouble(&lon))
    {
	map->positionCrosshair(lat, lon);
	seismicRegion(lat, lon);
    }
}

void MapCursor::crosshairCB(void)
{
    double lat, lon, delta, azimuth;

    if(!map->getCoordinates(&lat, &lon, &delta, &azimuth)) return;

    if(lon < -180) lon = -180;
    else if(lon > 180) lon = 180;
    setScale(longitude_scale, lon);
    longitude_text->setString("%.1f", lon);

    if(lat < -90) lat = -90;
    else if(lat > 90) lat = 90;
    setScale(latitude_scale, lat);
    latitude_text->setString("%.1f",lat);

    seismicRegion(lat, lon);
}

void MapCursor::seismicRegion(double lat, double lon)
{
    char region_name[40];
    int region_num = nmreg(lat, lon);
	
    sreg(gtos(region_num), region_name);

    region_label->setLabel(region_name);
}
