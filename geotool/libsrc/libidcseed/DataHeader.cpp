/** \file DataHeader.cpp
 *  \brief Defines Data Header parsing method.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;
#include "seed/DataHeader.h"
#include "seed/ByteOrder.h"

/** Create a DataHeader class by parsing all fields from the input byte array.
 * @param bytes the last 40 bytes of the Data Header (after the 6-byte sequence
 *	number, 1-byte quality indicator and 1 reserved byte).
 * @wo word order indices to convert from data order to local order.
 * @so short word order indices to convert from data order to local order.
 */
void DataHeader::loadDB(const string &bytes, int *wo, int *so)
		throw(FormatException)
{
    int i;
    UWORD u;
    WORD w;
    LONG l;
    string v;

    type = "Data";

    if(bytes.length() < 40) {
	throw FormatException("DataHeader: short header.");
    }

    station = trim(bytes.substr(0, 5));
    location = trim(bytes.substr(5, 2));
    channel = trim(bytes.substr(7, 3));
    network = trim(bytes.substr(10, 2));

    start_time = SeedTime(bytes.substr(12), so);

    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[22+i];
    nsamples = u.s;
    for(i = 0; i < 2; i++) w.a[so[i]] = bytes[24+i];
    sample_rate = w.s;
    for(i = 0; i < 2; i++) w.a[so[i]] = bytes[26+i];
    multiplier = w.s;

    activity = bytes[28];
    io = bytes[29];
    quality = bytes[30];
    num = bytes[31];
    for(i = 0; i < 4; i++) l.a[wo[i]] = bytes[32+i];
    correction = l.i;

    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[36+i];
    offset = u.s;
    for(i = 0; i < 2; i++) u.a[so[i]] = bytes[38+i];
    boffset = u.s;
}

double DataHeader::sampleRate()
{
    double samprate;

    if(sample_rate == 0) {
	return 0.;
    }
    else if(sample_rate > 0) {
	samprate = (double)sample_rate;
    }
    else {
	samprate = -1./(double)sample_rate;
    }

    if(multiplier > 0) {
	samprate *= (double)multiplier;
    }
    else if(multiplier < 0) {
	samprate /= -(double)multiplier;
    }
    return samprate;
}
