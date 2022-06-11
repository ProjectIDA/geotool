#ifndef _DATA_RECEIVER_H
#define _DATA_RECEIVER_H

#include "gobject++/DataSource.h"

class WaveformWindow;
class WaveformPlot;

/** An interface for classes that receive data.
 *  @ingroup libgmethod
 */
class DataReceiver
{
    public:
	virtual ~DataReceiver(void) {
	    if(data_source) data_source->removeDataReceiver(this);
	}

	virtual void setDataSource(DataSource *ds) {
	    if(ds != data_source) {
		if(data_source) data_source->removeDataReceiver(this);
		data_source = ds;
		if(data_source) data_source->addDataReceiver(this);
	    }
	}
	virtual void removeDataSource(DataSource *ds) {
	    if(data_source == ds) {
		data_source = NULL;
	    }
	}
	virtual DataSource *getDataSource(void) { return data_source; }
	virtual WaveformWindow *getWaveformWindowInstance(void) { return NULL; }
	virtual WaveformPlot *getWaveformPlotInstance(void) { return NULL; }

    protected:
	// can only be constructed by a subclass
	DataReceiver(void) : data_source(NULL) { }
	DataReceiver(const DataReceiver &d) : data_source(d.data_source) {
	    if(data_source) data_source->addDataReceiver(this);
	}
	DataReceiver & operator=(const DataReceiver &d) {
	    data_source = d.data_source;
	    return *this;
	}

	DataReceiver(DataSource *ds) : data_source(ds) {
	    if(data_source) data_source->addDataReceiver(this);
	}

	DataSource *data_source;
};

#endif
