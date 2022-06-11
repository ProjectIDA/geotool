/** \file Map.cpp
 *  \brief Defines class Map.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include "widget/Map.h"
#include "motif++/Application.h"

using namespace std;

Map::Map(const string &name, Component *parent, Arg *args, int n) :
		AxesClass(mapPlotWidgetClass, name, parent, args, n)
{
    mp = (MapPlotWidget)base_widget;

    XtAddCallback(base_widget, XtNselectStationCallback,
		Map::selectStationCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNdragStationCallback,
		Map::dragStationCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectSourceCallback,
		Map::selectSourceCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNdragSourceCallback,
		Map::dragSourceCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNmapMeasureCallback,
		Map::mapMeasureCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectArcCallback,
		Map::selectArcCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectCircleCallback,
		Map::selectCircleCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNcursorMotionCallback,
		Map::cursorMotionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNshapeSelectCallback,
		Map::shapeSelectCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNsymbolSelectCallback,
		Map::symbolSelectCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNsymbolInfoCallback,
		Map::symbolInfoCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNutmCallback,
		Map::utmCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNpolarSelectCallback,
		Map::polarSelectCallback, (XtPointer)this);
    XtAddCallback(base_widget, XtNselectBarCallback,
		Map::selectBarCallback, (XtPointer)this);

    enableCallbackType(XtNselectStationCallback);
    enableCallbackType(XtNdragStationCallback);
    enableCallbackType(XtNselectSourceCallback);
    enableCallbackType(XtNdragSourceCallback);
    enableCallbackType(XtNmapMeasureCallback);
    enableCallbackType(XtNselectArcCallback);
    enableCallbackType(XtNselectCircleCallback);
    enableCallbackType(XtNcursorMotionCallback);
    enableCallbackType(XtNshapeSelectCallback);
    enableCallbackType(XtNsymbolSelectCallback);
    enableCallbackType(XtNsymbolInfoCallback);
    enableCallbackType(XtNutmCallback);
    enableCallbackType(XtNpolarSelectCallback);
    enableCallbackType(XtNselectBarCallback);

    MapPlotSetAppContext(mp, Application::getApplication()->appContext());
}

Map::~Map(void)
{
}

void Map::selectStationCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map *)client;
    map->doCallbacks(w, calldata, XtNselectStationCallback);
}

void Map::dragStationCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNdragStationCallback);
}

void Map::selectSourceCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNselectSourceCallback);
}

void Map::dragSourceCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNdragSourceCallback);
}

void Map::mapMeasureCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNmapMeasureCallback);
}

void Map::selectArcCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNselectArcCallback);
}

void Map::selectCircleCallback(Widget w, XtPointer client,XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNselectCircleCallback);
}

void Map::cursorMotionCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNcursorMotionCallback);
}

void Map::shapeSelectCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNshapeSelectCallback);
}

void Map::symbolSelectCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNsymbolSelectCallback);
}

void Map::symbolInfoCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNsymbolInfoCallback);
}

void Map::utmCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNutmCallback);
}

void Map::polarSelectCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNpolarSelectCallback);
}

void Map::selectBarCallback(Widget w, XtPointer client, XtPointer calldata)
{
    Map *map = (Map*)client;
    map->doCallbacks(w, calldata, XtNselectBarCallback);
}
