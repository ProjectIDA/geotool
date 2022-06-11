/** \file SegmentInfo.cpp
 *  \brief Defines class SegmentInfo.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <strings.h>
using namespace std;

#include "gobject++/SegmentInfo.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

SegmentInfo::SegmentInfo() : path(stringToQuark("")), id(-1), format(0),
    file_order(-1), read_order(-1), start(NULL_TIME), end(NULL_TIME),
    input_start(NULL_TIME), input_end(NULL_TIME), nsamp(0), samprate(-1.),
    jdate(0), nass(0), ndef(0), station_lat(-999.), station_lon(-999.),
    station_elev(-999.), station_depth(-999.), dnorth(-999.), deast(-999.),
    selected(false), origin_id(-1), origin_time(NULL_TIME), origin_lat(-999.),
    origin_lon(-999.), origin_depth(-999.), origin_delta(-999.),
    origin_azimuth(-999.), component(0), hang(-999.), vang(-999.), alpha(-999.),
    beta(-999.), gamma(-999.), arrivals(), wc(NULL), w(NULL)
{
    memset((void *)sta, 0, sizeof(sta));
    memset((void *)chan, 0, sizeof(chan));
    memset((void *)net, 0, sizeof(net));
    memset((void *)refsta, 0, sizeof(refsta));
    memset((void *)x_chan, 0, sizeof(x_chan));
    memset((void *)y_chan, 0, sizeof(y_chan));
    memset((void *)z_chan, 0, sizeof(z_chan));
}

SegmentInfo::SegmentInfo(const SegmentInfo &s) : path(s.path), id(s.id),
    format(s.format), file_order(s.file_order), read_order(s.read_order),
    start(s.start), end(s.end), input_start(s.input_start),
    input_end(s.input_end), nsamp(s.nsamp), samprate(s.samprate),
    jdate(s.jdate), nass(s.nass), ndef(s.ndef), station_lat(s.station_lat),
    station_lon(s.station_lon), station_elev(s.station_elev),
    station_depth(s.station_depth), dnorth(s.dnorth), deast(s.deast),
    selected(s.selected), origin_id(s.origin_id), origin_time(s.origin_time),
    origin_lat(s.origin_lat), origin_lon(s.origin_lon),
    origin_depth(s.origin_depth), origin_delta(s.origin_delta),
    origin_azimuth(s.origin_azimuth), component(s.component), hang(s.hang),
    vang(s.vang), alpha(s.alpha), beta(s.beta), gamma(s.gamma), arrivals(),
    wc(NULL), array_elements(s.array_elements), w(NULL)
{
    strcpy(sta, s.sta);
    strcpy(chan, s.chan);
    strcpy(net, s.net);
    strcpy(refsta, s.refsta);
    strcpy(x_chan, s.x_chan);
    strcpy(y_chan, s.y_chan);
    strcpy(z_chan, s.z_chan);
    if(s.w) {
	w = s.w;
	w->addOwner(this);
    }
}

SegmentInfo::~SegmentInfo(void)
{
    arrivals.clear();
    if(w) w->removeOwner(this);
    array_elements.removeAll();
}

Gobject * SegmentInfo::clone(void)
{
    SegmentInfo *s = new SegmentInfo();
    s->copy(this);
    return (Gobject *)s;
}

void SegmentInfo::copy(const SegmentInfo *from)
{
    path = from->path;
    id = from->id;
    format = from->format;
    file_order = from->file_order;
    read_order = from->read_order;
    strcpy(sta, from->sta);
    strcpy(chan, from->chan);
    strcpy(net, from->net);
    strcpy(refsta, from->refsta);
    start = from->start;
    end = from->end;
    input_start = from->input_start;
    input_end = from->input_end;
    nsamp = from->nsamp;
    samprate = from->samprate;
    jdate = from->jdate;
    nass = from->nass;
    ndef = from->ndef;
    station_lat = from->station_lat;
    station_lon = from->station_lon;
    station_elev = from->station_elev;
    station_depth = from->station_depth;
    dnorth = from->dnorth;
    deast = from->deast;
    selected = from->selected;

    origin_id = from->origin_id;
    origin_time = from->origin_time;
    origin_lat = from->origin_lat;
    origin_lon = from->origin_lon;
    origin_depth = from->origin_depth;
    origin_delta = from->origin_delta;
    origin_azimuth = from->origin_azimuth;

    component = from->component;
    hang = from->hang;
    vang = from->vang;
    alpha = from->alpha;
    beta = from->beta;
    gamma = from->gamma;
    strcpy(x_chan, from->x_chan);
    strcpy(y_chan, from->y_chan);
    strcpy(z_chan, from->z_chan);

    wc = NULL;

    if(from->w) setWfdisc(from->w);

    for(int i = 0; i < from->array_elements.size(); i++) {
	SegmentInfo *s = ((SegmentInfo *)from)->array_elements[i];
	array_elements.add((SegmentInfo *)s->clone());
    }
}
