/** \file SeedData.cpp
 *  \brief Defines a class for continuous data records for a single channel.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include "seed/SeedData.h"
#include "seed/Decoders.h"
using namespace std;

SeedData::SeedData(const SeedData &s) : Seed("SD")
{
    calib = s.calib;
    calper = s.calper;
    channel = (s.channel) ? new Channel(*s.channel) : NULL;
    records = s.records;
}

SeedData & SeedData::operator=(const SeedData &s)
{
    if(this == &s) return *this;
    delete channel;
    if(s.channel) channel = new Channel(*s.channel);
    return *this;
}

int SeedData::readData(istream *in, float *data, int nsamples)
{
    int n = 0, nbytes, nsamp = 0;
    char *bytes = NULL;

    for(int i = 0; i < (int)records.size(); i++)
    {
	in->seekg(records[i].data_file_offset, ios::beg);
	nbytes = records[i].data_length;
	if(!bytes) bytes = (char *)malloc(nbytes);
	else bytes = (char *)realloc(bytes, nbytes);
	in->read(bytes, nbytes);
	if(nsamp + records[i].header.nsamples > nsamples) break;

	n = Decoders::decode(records[i].format, bytes, nbytes, records[i].wo,
		records[i].so, records[i].header.nsamples, data+nsamp);

	if(n != records[i].header.nsamples) {
	    cerr << "Warning: decoded nsamples != header.nsamples. "
		<< "seqno: " << records[i].header.seqno
		<< " sta: " << records[i].header.station
		<< " chan: " << records[i].header.channel
		<< " loc: " << records[i].header.location << endl;
	}
	nsamp += n;
    }
    free(bytes);
    return nsamp;
}
