/** \file DataRecord.h
 *  \brief Declares the DataRecord class members and methods.
 *  \author Ivan Henson
 */
#ifndef _DATA_RECORD_H_
#define _DATA_RECORD_H_

#include <vector>
#include "seed/Seed.h"
#include "seed/SeedTime.h"
#include "seed/FormatException.h"
#include "seed/DataHeader.h"
#include "seed/Blockettes.h"

/**
 * Data Record
 *
 *@see DataHeader
 *@see DataBlockette
 *@see Seed
 *@see SeedInput
 */
class DataRecord : public Seed
{
    public:
	string word_order;    //!< 32 bit word order from blockette 50 or 1000
        string short_order;   //!< 16 bit word order from blockette 50 or 1000
	int wo[4];            //!< native_int_byte[wo[i]] = seed_byte[i]
	int so[2];            //!< native_short_byte[so[i]] = seed_byte[i]
	int reclen;           //!< Data record length from blockette 1000
	DataHeader header;    //!< Fixed section of data header.
	/**  An array of data_header->num data blockettes. */
	vector<DataBlockette *> blockettes;
	int record_offset;    //!< Offset to this record in the file.
	int data_file_offset; //!< Offset of the data bytes in the file.
	int data_length;      //!< Number of bytes of compressed data.
	string data;          //!< Compressed data.
	double samprate;      //!< Sample rate from DataHeader or Blockette100
	int format;	      //!< From Blockette1000 or Blockette30
	double clock_drift;   //!< From Blockette52

	/** Creates a DataRecord class from a bytes array. */
	DataRecord(const string &header_bytes, const string &wordorder,
		const string &shortorder) throw(FormatException) :
		Seed("DR"), word_order(wordorder),
		short_order(shortorder), reclen(0), data_file_offset(0),
		samprate(0.), format(11)
	{
	    // determine the swap order from the seed data to the native order
	    swapOrder();

	    header.loadDB(header_bytes, wo, so);
	}
	DataRecord(const DataRecord &d);
	DataRecord & operator=(const DataRecord &d);

	~DataRecord() {
	    for(int i = 0; i <(int)blockettes.size(); i++) delete blockettes[i];
	}

	void swapOrder();

	void resetWordOrder(const string &header_bytes, const string wordorder,
			const string shortorder)
	{
	    word_order = wordorder;
	    short_order = shortorder;
	    swapOrder();
	    header.loadDB(header_bytes, wo, so);
	}

	DataRecord * getDataRecord(void) { return this; }

        string str() {
	    ostringstream out;
	    out << header.station << " " << header.location << " "
		<< header.channel << " " << header.start_time.str() << " "
		<< header.nsamples;
            return out.str();
	}
};

#endif
