/** \file DataHeader.h
 *  \brief Declares the DataHeader class members and methods.
 *  \author Ivan Henson
 */
#ifndef _DATA_HEADER_H_
#define _DATA_HEADER_H_

#include "seed/Seed.h"
#include "seed/SeedTime.h"

/** 
 * Fixed Section of Data Header (48 bytes)
 *
 *@see Seed
 *@see SeedStream
 *@see DataRecord
 */
class DataHeader : Seed
{
    public:
	int seqno;           //!< Sequence number.
	char dhqual;	     //!< Data header/quality indicator.
	string station;      //!< Station identifier code.
	string location;     //!< Location identifier.
	string channel;      //!< Channel identifier.
	string network;      //!< Network code.
	SeedTime start_time; //!< Record start time.
	short nsamples;      //!< Number of samples.
	/** Sample rate factor. > 0 : Samples/second, < 0 : Seconds/sample */
	short sample_rate;
	/**  Sample rate multiplier. > 0 : Multiplication factor,
	 	< 0 : Division factor */
	short multiplier;
	char activity;     //!< Activity flags.
	char io;           //!< I/O and clock flags.
	char quality;      //!< Data quality flags.
	int num;           //!< Number of blockettes that follow.
	int correction;    //!< Time correction in units of 0.0001 seconds
	SeedTime end_time; //!< Series end time.
	short offset;      //!< Beginning of data.
	short boffset;     //!< First blockette (offset to).

	/** Constructor
	 */
	DataHeader() : Seed("DataHeader"), seqno(0), dhqual('D'), nsamples(0),
		sample_rate(0), multiplier(0), activity(0), io(0), quality(0),
		num(0), correction(0), offset(0) { }

	/** Parse all DataHeader fields from the input char array.
	 * @param bytes 48 character header string.
	 * @wo byte order index array for 4-byte words.
	 * @so byte order index array for 2-byte words.
	 */
	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);

	double sampleRate();
	double startTime() {
	    if( !(activity & 0x02) ) {
		return start_time.epoch() + (double)correction/10000.;
	    }
	    return start_time.epoch();
	}

	double endTime() { return startTime() + (nsamples-1)/sampleRate(); }
};

#endif
