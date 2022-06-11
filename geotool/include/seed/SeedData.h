/** \file SeedData.h
 *  \brief Declares the SeedData class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_DATA_H_
#define _SEED_DATA_H_

#include <vector>
#include "seed/Blockettes.h"
#include "seed/DataRecord.h"
#include "seed/Station.h"

/**
 * This class holds DataRecord objects that can be concatenated into a
 * continuous timeseries.
 */
class SeedData : public Seed
{
    public:
	SeedData() : Seed("SD"), calib(0.), calper(0.), channel(NULL) {}

	SeedData(DataRecord *dr) : Seed("SeedData"), calib(0.), calper(0.),
		channel(NULL) { records.push_back(*dr); }
	SeedData(const SeedData &s);
	SeedData & operator=(const SeedData &d);
	~SeedData() { delete channel; }

	SeedData * getSeedData(void) { return this; }

	int nsamples() {
	    int n=0;
	    for(int i = 0; i < (int)records.size(); i++) {
		n += records[i].header.nsamples;
	    }
	    return n;
	}
	double samprate() {
	    if((int)records.size() > 0) {
		return records[0].samprate;
	    }
	    else return 0.;
	}
	double startTime() {
	    if((int)records.size() > 0) {
		return records[0].header.startTime();
	    }
	    else return 0.;
	}
	double endTime() {
	    double rate = samprate();
	    if(rate != 0.) return startTime() + (nsamples()-1)/rate;
	    return 0.;
	}

	int readData(istream *in, float *data, int nsamp);

	double calib;
	double calper;
	Channel *channel;
	vector<DataRecord> records;
};

#endif
