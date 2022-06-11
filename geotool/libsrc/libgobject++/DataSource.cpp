/** \file DataSource.cpp
 *  \brief Defines class DataSource.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>

#include "gobject++/DataSource.h"
#include "DataReceiver.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GSourceInfo.h"

//static
/** Copy the source information from a GTimeSeries to a CssTableClass.
 *  @param[in,out] table a CssTableClass object
 *  @param[in] ts a GTimeSeries object whose data source is copied to the table.
 */
void DataSource::copySourceInfo(CssTableClass *table, GTimeSeries *ts)
{
    DataSource *ds = ts->getDataSource();
    if(!ds) {
	logErrorMsg(LOG_WARNING, "DataSource.copySourceInfo: null DataSource.");
    }
    else {
	table->setDataSource(ds);
    }
    ts->source_info.copySource(table);
}

//static
/** Copy the source information from a CssTableClass to a GTimeSeries.
 *  @param[in,out] ts a GTimeSeries object.
 *  @param[in,out] table a CssTableClass object whose data source is copied to the
 *	GTimeSeries object.
 */
void DataSource::copySourceInfo(GTimeSeries *ts, CssTableClass *table)
{
    DataSource *ds = table->getDataSource();
    if(!ds) {
	logErrorMsg(LOG_WARNING, "DataSource.copySourceInfo: null DataSource.");
    }
    else {
	ts->setDataSource(ds);
    }
    ts->source_info.setSource(table);
}

//static
/** Copy the source information from one CssTableClass to another CssTableClass.
 *  @param[in,out] table_dest a GTimeSeries object.
 *  @param[in,out] table_src CssTableClass object whose data source is copied to the
 *	table_dest object.
 */
void DataSource::copySourceInfo(CssTableClass *table_dest, CssTableClass *table_src)
{
    DataSource *ds = table_src->getDataSource();
    if(!ds) {
	logErrorMsg(LOG_WARNING, "DataSource.copySourceInfo: null DataSource.");
    }
    else {
	table_dest->setDataSource(ds);
    }
    table_src->copySourceTo(table_dest, 0);
}

void DataSource::addOwner(Gobject *owner)
{
    if( !allow_owners || !owner ) return;

    int i;
    /* don't register an owner twice */
    for(i = 0; i < (int)owners.size(); i++) {
	if(owners[i] == owner) break;
    }
    if(i == (int)owners.size()) {
	owners.push_back(owner);
    }
}

void DataSource::removeOwner(Gobject *owner)
{
    if((int)owners.size() == 0) return;

    /* remove owner from this->owners
     */
    for(int i = 0; i < (int)owners.size(); i++) {
	if(owners[i] == owner) {
	    owners.erase(owners.begin()+i);
	    break;
        }
    }
    if((int)owners.size() == 0) delete this;
}

// destructor
DataSource::~DataSource(void)
{
    for(int i = 0; i < (int)receivers.size(); i++) {
	receivers.at(i)->removeDataSource(this);
    }
    if((int)owners.size() > 0) {
	cerr << "Error: deleting DataSource object that has owners." << endl;
    }
}

/** 
 *  Add a DataReceiver to this DataSource. The DataReceiver will be notified
 *  when this DataSource is deleted.
 *  @param[in] r a DataReceiver instance.
 */
void DataSource::addDataReceiver(DataReceiver *r)
{
    if(r == NULL) return;

    int i;
    for(i = 0; i < (int)receivers.size() && receivers.at(i) != r; i++);
    if(i == (int)receivers.size()) {
	receivers.push_back(r);
    }
}

/** 
 *  Remove a DataReceiver from this DataSource.
 *  @param[in] r a DataReceiver.
 */
void DataSource::removeDataReceiver(DataReceiver *r)
{
    if(r == NULL) return;

    int i;
    for(i = 0; i < (int)receivers.size() && receivers.at(i) != r; i++);
    if(i < (int)receivers.size()) {
	receivers.erase(receivers.begin()+i);
    }
}

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void DataSource::
#ifdef __STDC__
ShowWarning(const char *format, ...)
#else
ShowWarning(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    int n;
    if( !format || (n = (int)strlen(format)) <= 0) return;

    vfprintf(stderr, format, va);
    if(format[n-1] != '\n') fprintf(stderr, "\n");
    va_end(va);
}

void DataSource::
#ifdef __STDC__
PutWarning(const char *format, ...)
#else
ShowWarning(va_alist) va_dcl
#endif
{
    va_list     va;
#ifdef __STDC__
    va_start(va, format);
#else
    char *format = (char *)va_arg(va, char *);
#endif

    int n;
    if( !format || (n = (int)strlen(format)) <= 0) return;

    vfprintf(stderr, format, va);
    if(format[n-1] != '\n') fprintf(stderr, "\n");
    va_end(va);
}

/** Get the data change state.
 *  @returns true if any of the data change fields are true. The change fields
 *	are:
 *	- select_waveform the selected state of a waveform has been changed.
 *	- sort_waveforms the waveforms have been sorted.
 *	- select_arrival the selected state of an arrival has been changed.
 *	- select_origin the selected state of an origin has been changed.
 *	- waveform a waveform has been added or removed.
 *	- arrival a arrival has been added or removed.
 *	- assoc a assoc has been added or removed.
 *	- origin a origin has been added or removed.
 *	- origerr a origerr has been added or removed.
 *	- stassoc a stassoc has been added or removed.
 *	- stamag a stamag has been added or removed.
 *	- netmag a netmag has been added or removed.
 *	- hydro a hydro_feature has been added or removed.
 *	- infra a infra_feature has been added or removed.
 *	- wftag a wftag has been added or removed.
 *	- amplitude a amplitude has been added or removed.
 *	- ampdescript a ampdescript has been added or removed.
 *	- filter a filter has been added or removed.
 *	- parrival a parrival has been added or removed.
 *	- unknown an unknown change.
 *	- unknown_select an unknown selection.
 *	- primary_origin the primary origin was set for a waveform.
 *	- working_orid the working origin has been changed.
 */
bool DataSource::dataChange(void)
{
    return (
	change.select_waveform ||
	change.sort_waveforms ||
	change.select_arrival ||
	change.select_origin ||
	change.waveform ||
	change.arrival ||
	change.assoc ||
	change.origin ||
	change.origerr ||
	change.stassoc ||
	change.stamag ||
	change.netmag ||
	change.hydro ||
	change.infra ||
	change.wftag ||
	change.amplitude ||
	change.ampdescript ||
	change.filter ||
	change.parrival ||
	change.unknown ||
	change.unknown_select ||
	change.primary_origin ||
	change.working_orid);
}

/** Compare channel names. If one channel name has three characters and the
 *  other has only two characters, the second character of the three-character
 *  name is ignored. Otherwise, if the channel names have the same number of
 *  characters, all characters are compared. The comparison is case insensitive.
 *  @param[in] c1 a channel name
 *  @param[in] c2 a channel name
 *  @returns true if the channel names are the "same".
 */
bool DataSource::compareChan(const string &c1, const string &c2)
{
    char tmp[3];

    int n1 = (int)c1.length();
    int n2 = (int)c2.length();

    if(n1 == 3 && n2 == 2) {
	tmp[0] = c1[0];
	tmp[1] = c1[2];
	tmp[2] = '\0';
	return !strcasecmp(tmp, c2.c_str()) ? true : false;
    }
    else if(n1 == 2 && n2 == 3) {
	tmp[0] = c2[0];
	tmp[1] = c2[2];
	tmp[2] = '\0';
	return !strcasecmp(c1.c_str(), tmp) ? true : false;
    }
    return !strcasecmp(c1.c_str(), c2.c_str()) ? true : false;
}
