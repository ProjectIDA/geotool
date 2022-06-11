/** \file SeedInput.h
 *  \brief Declares the SeedInput class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_INPUT_H_
#define _SEED_INPUT_H_

#include <iostream>
#include <istream>
#include <vector>
#include "seed/Seed.h"
#include "seed/Blockettes.h"
#include "seed/DataRecord.h"
#include "seed/Station.h"
#include "seed/SeedData.h"
#include "seed/Dictionary.h"

#define DATABIT  0x01
#define RAWBIT   0x02

#define FMTBIT   0x01
#define HDRBIT   0x02
#define LENBIT   0x04
#define SEQNOBIT 0x08

/**
 *  A SeedInput is a class for reading Seed objects.
 *
 *@see Seed
 */
class SeedInput
{
    public:
	/** Creates a Seed input stream.
	 */
	SeedInput(istream *is, const char mode_bits=0,
			const char exception_bits=0) :
		lreclen(4096), logical_offset(0), record_index(0),
		ctrl_type("VAST"), data_quality("DRQM"), record_type(0),
		record_continuation(0), mode(mode_bits), bits(exception_bits),
		state(0), record_seqno(0), reading_data(false),
		next(), in(is)
	{
	}

	virtual ~SeedInput() {
	    for(int i = 0; i < (int)next.size(); i++) delete next[i];
	    for(int i = 0; i < (int)stations.size(); i++) delete stations[i];
	}

	/** Read the next Seed object. */
	Seed *readSeed();

	/** Set Control header types (default "VAST") */
	void setCtrlTypes(const string &header_types) {
		ctrl_type = header_types; }

	/** Set Control header types (default "ARQM") */
	void setDataTypes(const string &data_types) {
		data_quality = data_types; }

	/** Set exceptions that will be thrown. */
	void exceptions(const char exception_bits) { bits = exception_bits; }

	/** Returns the logical record length. */
	int logicalRecordLength() { return lreclen; }

	/** Returns the byte offset within the current logical record. */
	int logicalOffset() { return logical_offset; }

	/** Returns the SEED current record type. */
	char recordType() { return record_type; }

	/** Returns the SEED current record continuation flag. */
	char recordContinuation() { return record_continuation; }

	/** Returns the SEED current record sequence number. */
	int recordSeqenceNumber() { return record_seqno; }

	/** Skip to the next logical record. */
	void skipRec() {
	    int n = lreclen - logical_offset;
	    if(n > 0) {
		in->seekg(n, ios_base::cur);
	    }
	    logical_offset = 0;
	}
	char getState() { return state; }
	bool good() { return (state == 0); }

	vector<Station *> stations;
	Dictionary dictionary;

    protected:
	int lreclen;         //!< logical record length
	int logical_offset;  //!< byte offset in current logical record
	int record_index;    //!< current logical record.
	string ctrl_type;    //!< Control header types "VAST"
	string data_quality; //!< data header/quality indicators "DRQM"

	char record_type;
	char record_continuation;
	char mode;
	char bits;
	char state;
	int record_seqno;
	bool reading_data;
	bool read_data;
	vector<Seed *> next;
	istream *in;

	Seed * readSeedObject();
	void readVolumeHeader();
	DataRecord * readDataRecord(const string &s);

	/** Read a String from the next num_bytes of input.
	 */
	string readString(int num_bytes);

	/** Read bytes directly from the stream.
	 */
	void readBytes(char *bytes, int len);
	Seed * getNextObject();
	void getWordOrder(string &sta, string &net, string *work_order,
			string *short_order);
	Seed * getSeedData(DataRecord *dr);
	void getFormat(DataRecord *dr, Blockette1000 *b1000);
	void getCalib(SeedData *sd);
	Seed * convertBlockette60(Blockette60 *b60);
};

#endif
