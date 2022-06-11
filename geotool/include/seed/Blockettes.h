/** \file Blockettes.h
 *  \brief Defines the Seed blockette class members and methods.
 *  \author Ivan Henson
 */
#ifndef _BLOCKETTES_H_
#define _BLOCKETTES_H_
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <vector>
using namespace std;

#include "seed/Seed.h"
#include "seed/SeedTime.h"
#include "seed/FormatException.h"

class UnknownBlockette : public Seed
{
    public:
	string type;
	string fields;

	UnknownBlockette(const string &blockette_type,
			const string &blockette_fields) : Seed("unknown")
	{
	    type = blockette_type;
	    fields = blockette_fields;
	}
	~UnknownBlockette() {
	}
	UnknownBlockette * getUnknownBlockette(void) { return this; }
};

/** 
 * Field Volume Identifier Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette5 : public Seed
{
    public:
	string	version_of_format; //!< SEED version number ##.#.

	int logical_record_length; //!< Logical record length, expressed as power of 2.

	SeedTime beginning_time;   //!< beginning of volume data.

	/** Parse Blockette5 fields from a string. */
	Blockette5(const string &blockette) throw(FormatException) :
		Seed("005"), logical_record_length(0)
	{
	    load(blockette);
	}

	/** Create a Blockette5 with null fields. */
	Blockette5() : Seed("005"), logical_record_length(0) { }
	Blockette5(const Blockette5 &b) : Seed("005") { *this = b; }
	~Blockette5() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette5 * getBlockette5(void) { return this; }
};

/** 
 * Telemetry Volume Identifier Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette8 : public Seed
{
    public:
	string version_of_format;  //!< SEED version number ##.#
	int logical_record_length; //!< Logical record length, expressed as power of 2
	string station;		   //!< Station identifier.
	string location;	   //!< Location identifier.
	string channel;		   //!< Channel identifier.
	SeedTime beginning_time;   //!< beginning of volume data.
	SeedTime end_time;	   //!< end of volume data.
	SeedTime station_date;	   //!< Station information effective date.
	SeedTime channel_date;	   //!< Channel information effective date.
	string network;		   //!< Network code.

	/** Parse Blockette8 fields from a string. */
	Blockette8(const string &blockette) throw(FormatException) :
		Seed("008"), logical_record_length(0)
	{
	    load(blockette);
	}

	/** Create a Blockette8 with null fields. */
	Blockette8() : Seed("008"), logical_record_length(0) { }
	Blockette8(const Blockette8 &b) : Seed("008") { *this = b; }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette8 * getBlockette8(void) { return this; }
};

/** 
 * Volume Identifier Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette10 : public Seed
{
    public:
	string version_of_format;  //!< SEED version number ##.#
	int logical_record_length; //!< Logical record length, expressed as power of 2.
	SeedTime beginning_time;   //!< Beginning of volume data.
	SeedTime end_time;	   //!< End of volume data.
	SeedTime volume_time;	   //!< Creation date of volume.
	string organization;	   //!< Originating organization
	string label;		   //!< Optional volume label.

	/** Parse Blockette10 fields from a string. */
	Blockette10(const string &blockette) throw(FormatException) :
		Seed("010"), logical_record_length(0)
	{
	    load(blockette);
	}

	/** Create a Blockette10 with null fields. */
	Blockette10() : Seed("010"), logical_record_length(0) { }
	Blockette10(const Blockette10 &b) : Seed("010") { *this = b; }
	~Blockette10() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette10 * getBlockette10(void) { return this; }
};

/** 
 * Volume Station Header Index Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette11 : public Seed
{
    public:
	vector<string> station; //!< Station identification codes.
	vector<int> seqno;      //!< Sequence number of station header.

	/** Parse Blockette11 fields from a string. */
	Blockette11(const string &blockette) throw(FormatException) :Seed("011")
	{
	    load(blockette);
	}

	/** Create a Blockette11 with null fields. */
	Blockette11() : Seed("011") { }
	Blockette11(const Blockette11 &b) : Seed("011") { *this = b; }
	~Blockette11() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette11 * getBlockette11(void) { return this; }
};

/** 
 * Volume Time Span Index Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette12 : public Seed
{
    public:
	vector<SeedTime> beg; //!< Beginning of span.
	vector<SeedTime> end; //!< End of span.
	vector<int> seqno;    //!< Sequence number of time span header.

	/** Parse Blockette12 fields from a string. */
	Blockette12(const string &blockette) throw(FormatException) :Seed("012")
	{
	    load(blockette);
	}

	/** Create a Blockette12 with null fields. */
	Blockette12() : Seed("012") { }
	Blockette12(const Blockette12 &b) : Seed("012") { *this = b; }
	~Blockette12() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette12 * getBlockette12(void) { return this; }
};
/**
 * This abstract class is the superclass of all classes that represent
 * SEED Dictionary blockettes.
 *
 *@see Seed
 *@see SeedStream
 */
class DictionaryBlockette : public Seed
{
    public:
	DictionaryBlockette(const char *name) : Seed(name) { }
	int lookup_code; //!< Cross reference code, used in later blockettes.
	DictionaryBlockette * getDictionaryBlockette(void) { return this; }
};

/** 
 * Data Format Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette30 : public DictionaryBlockette
{
    public:
	string name;	     //!< Short descriptive name.
	int family_type;     //!< Data family type.
	vector<string> keys; //!< Decoder keys.

	/** Parse Blockette30 fields from a string. */
	Blockette30(const string &blockette) throw(FormatException) :
		DictionaryBlockette("030"), family_type(0)
	{
	    load(blockette);
	}

	/** Create a Blockette30 with null fields. */
	Blockette30() : DictionaryBlockette("030"), family_type(0) { }
	Blockette30(const Blockette30 &b) : DictionaryBlockette("030") {
		*this = b; }
	~Blockette30() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette30 * getBlockette30(void) { return this; }
};

/** 
 * Comment Description Blockette
 *
 *@see Seed
 *@see SeedStream
 */

class Blockette31 : public DictionaryBlockette
{
    public:
	string class_code; //!< Comment class code.
	string comment;    //!< Description of comment.
	int units;	   //!< Units of comment level.

	/** Parse Blockette31 fields from a string. */
	Blockette31(const string &blockette) throw(FormatException) :
		DictionaryBlockette("031"), units(0)
	{
	    load(blockette);
	}

	/** Create a Blockette31 with null fields. */
	Blockette31() : DictionaryBlockette("031"), units(0) { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette31 * getBlockette31(void) { return this; }
};

/** 
 * Cited Source Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette32 : public DictionaryBlockette
{
    public:
	string author;    //!< Name of publisher/author.
	string catalog;   //!< Date published/catalog.
	string publisher; //!< Publisher name.

	/** Parse Blockette32 fields from a string. */
	Blockette32(const string &blockette) throw(FormatException) :
		DictionaryBlockette("032")
	{
	    load(blockette);
	}

	/** Create a Blockette32 with null fields. */
	Blockette32() : DictionaryBlockette("032") { }
	Blockette32(const Blockette32 &b) : DictionaryBlockette("032") {
		*this = b; }
	~Blockette32() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette32 * getBlockette32(void) { return this; }
};

/** 
 * Generic Abbreviation Blockette
 *
 *@see Seed
 *@see SeedStream
 */

class Blockette33 : public DictionaryBlockette
{
    public:
	string description; //!< Abbreviation description.

	/** Parse Blockette33 fields from a string. */
	Blockette33(const string &blockette) throw(FormatException) :
		DictionaryBlockette("033")
	{
	    load(blockette);
	}

	/** Create a Blockette33 with null fields. */
	Blockette33() : DictionaryBlockette("033") { }
	Blockette33(const Blockette33 &b) : DictionaryBlockette("033") {
		*this = b; }
	~Blockette33() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette33 * getBlockette33(void) { return this; }
};

/** 
 * Units Abbreviations Blockette
 *
 *@see Seed
 *@see SeedStream
 */

class Blockette34 : public DictionaryBlockette
{
    public:
	string name;        //!< Unit name.
	string description; //!< Unit description.

	/** Parse Blockette34 fields from a string. */
	Blockette34(const string &blockette) throw(FormatException) :
		DictionaryBlockette("034")
	{
	    load(blockette);
	}

	/** Create a Blockette34 with null fields. */
	Blockette34() : DictionaryBlockette("034") { }
	Blockette34(const Blockette34 &b) : DictionaryBlockette("034") {
		*this = b; }
	~Blockette34() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette34 * getBlockette34(void) { return this; }
};

/** 
 * Beam Configuration Blockette
 *
 *@see Seed
 *@see SeedStream
 */

class Blockette35 : public DictionaryBlockette
{
    public:
	vector<string> station;  //!< Station identifier.
	vector<string> location; //!< Location identifier.
	vector<string> channel;  //!< Channel identifier.
	vector<int> subchannel;  //!< Sub-channel identifier.
	vector<double> weight;   //!< Component weight.

	/** Parse Blockette35 fields from a string. */
	Blockette35(const string &blockette) throw(FormatException) :
		DictionaryBlockette("035")
	{
	    load(blockette);
	}

	/** Create a Blockette35 with null fields. */
	Blockette35() : DictionaryBlockette("035") { }
	Blockette35(const Blockette35 &b) : DictionaryBlockette("035") {
		*this = b; }
	~Blockette35() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette35 * getBlockette35(void) { return this; }
};

/** 
 * FIR Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette41 : public DictionaryBlockette
{
    public:
	string name;	      //!< Response name.
	string symmetry_code; //!< Symmetry Code.
	int input_units;      //!< Signal In Units.
	int output_units;     //!< Signal Out Units.
	vector<double> coef;  //!< FIR Coefficient.

	/** Parse Blockette41 fields from a string. */
	Blockette41(const string &blockette) throw(FormatException) :
		DictionaryBlockette("041"), input_units(0), output_units(0)
	{
	    load(blockette);
	}

	/** Create a Blockette41 with null fields. */
	Blockette41() : DictionaryBlockette("041"), input_units(0),
		output_units(0) { }
	Blockette41(const Blockette41 &b) : DictionaryBlockette("041") {
		*this = b; }
	~Blockette41() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette41 * getBlockette41(void) { return this; }
};

/** 
 * Response (Polynomial) Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette42 : public DictionaryBlockette
{
    public:
	string name;	      //!< Response name.
	string transfer_type; //!< Transfer Function Type.
	int input_units;      //!< Signal In Units.
	int output_units;     //!< Signal Out Units.
	string poly_type;     //!< Polynomial Approximation Type
	string freq_units;    //!< Valid Frequency Units
	double min_freq;      //!< Lower Valid Frequency Bound
	double max_freq;      //!< Upper Valid Frequency Bound
	double min_approx;    //!< Lower Bound of Approximation
	double max_approx;    //!< Upper Bound of Approximation
	double max_error;     //!< Maximum Absolute Error
	vector<double> coef;  //!< Polynomial Coefficients
	vector<double> error; //!< Polynomial Coefficients Error

	/** Parse Blockette42 fields from a string. */
	Blockette42(const string &blockette) throw(FormatException) :
		DictionaryBlockette("042"), input_units(0), output_units(0),
		min_freq(0.), max_freq(0.), min_approx(0.), max_approx(0.),
		max_error(0.)
	{
	    load(blockette);
	}

	/** Create a Blockette41 with null fields. */
	Blockette42() : DictionaryBlockette("042"), input_units(0),
		output_units(0), min_freq(0.), max_freq(0.), min_approx(0.),
		max_approx(0.), max_error(0.) { }
	Blockette42(const Blockette42 &b) : DictionaryBlockette("042") {
		*this = b; }
	~Blockette42() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette42 * getBlockette42(void) { return this; }
};

/** 
 * Response (Poles & Zeros) Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette43 : public DictionaryBlockette
{
    public:
	string name;             //!< Response name.
	string type;             //!< Response type.
	int input_units;         //!< Stage signal input units.
	int output_units;        //!< Stage signal output units.
	double a0_norm;	         //!< AO normalization factor.  (1.0 if none)
	double norm_freq;        //!< Normalization frequency.
	vector<double> zr;       //!< Real zero.
	vector<double> zi;       //!< Imaginary zero.
	vector<double> zr_error; //!< Real zero error.
	vector<double> zi_error; //!< Imaginary zero error.
	vector<double> pr;       //!< Real pole.
	vector<double> pi;       //!< Imaginary pole.
	vector<double> pr_error; //!< Real pole error.
	vector<double> pi_error; //!< Imaginary pole error.

	/** Parse Blockette43 fields from a string. */
	Blockette43(const string &blockette) throw(FormatException) :
		DictionaryBlockette("043"), input_units(0), output_units(0),
		a0_norm(0.), norm_freq(0.)
	{
	    load(blockette);
	}

	/** Create a Blockette43 with null fields. */
	Blockette43() : DictionaryBlockette("043"), input_units(0),
			output_units(0), a0_norm(0.), norm_freq(0.) { }
	Blockette43(const Blockette43 &b) : DictionaryBlockette("043") {
		*this = b; }
	~Blockette43() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette43 * getBlockette43(void) { return this; }
};

/** 
 * Response (Coefficients) Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette44 : public DictionaryBlockette
{
    public:
	string name;                //!< Response name.
	string type;                //!< Response type.
	int input_units;            //!< Stage signal input units.
	int output_units;           //!< Stage signal output units.
	vector<double> numerator;   //!< Numerator coefficients.
	vector<double> nerror;	     //!< Numerator error.
	vector<double> denominator; //!< Denominator coefficients.
	vector<double> derror;      //!< Denominator error.

	/** Parse Blockette44 fields from a string. */
	Blockette44(const string &blockette) throw(FormatException) :
		DictionaryBlockette("044"), input_units(0), output_units(0)
	{
	    load(blockette);
	}

	/** Create a Blockette44 with null fields. */
	Blockette44() : DictionaryBlockette("044"), input_units(0),
			output_units(0) { }
	Blockette44(const Blockette44 &b) : DictionaryBlockette("044") {
			*this = b; }
	~Blockette44() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette44 * getBlockette44(void) { return this; }
};

/** 
 * Response List Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette45 : public DictionaryBlockette
{
    public:
	string name;                //!< Response name.
	int input_units;            //!< Stage signal input units.
	int output_units;           //!< Stage signal output units.
	vector<double> frequency;   //!< Frequency (Hz).
	vector<double> amplitude;   //!< Amplitude.
	vector<double> amp_error;   //!< Amplitude error.
	vector<double> phase;       //!< Phase angle (degrees).
	vector<double> phase_error; //!< Phase error (degrees).

	/** Parse Blockette45 fields from a string. */
	Blockette45(const string &blockette) throw(FormatException) :
		DictionaryBlockette("045"), input_units(0), output_units(0)
	{
	    load(blockette);
	}

	/** Create a Blockette45 with null fields. */
	Blockette45() : DictionaryBlockette("045"), input_units(0),
			output_units(0) { }
	Blockette45(const Blockette45 &b) : DictionaryBlockette("045") {
			*this = b; }
	~Blockette45() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette45 * getBlockette45(void) { return this; }
};

/** 
 * Generic Response Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette46 : public DictionaryBlockette
{
    public:
	string name;          //!< Response name.
	int input_units;      //!< Stage signal input units.
	int output_units;     //!< Stage signal output units.
	vector<double> corner_freq;  //!< Corner frequency (Hz).
	vector<double> corner_slope; //!< Corner slope (db/decade).

	/** Parse Blockette46 fields from a string. */
	Blockette46(const string &blockette) throw(FormatException) :
		DictionaryBlockette("046"), input_units(0), output_units(0)
	{
	    load(blockette);
	}

	/** Create a Blockette46 with null fields. */
	Blockette46() : DictionaryBlockette("046"), input_units(0),
			output_units(0) { }
	Blockette46(const Blockette46 &b) : DictionaryBlockette("046") {
			*this = b; }
	~Blockette46() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette46 * getBlockette46(void) { return this; }
};

/** 
 * Decimation Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette47 : public DictionaryBlockette
{
    public:
	string name;              //!< Response name.
	double input_sample_rate; //!< Input sample rate.
	int decimation_factor;    //!< Decimation factor.
	int decimation_offset;    //!< Decimation offset.
	double delay;             //!< Estimated delay (seconds).
	double correction;        //!< Correction applied (seconds).

	/** Parse Blockette47 fields from a string. */
	Blockette47(const string &blockette) throw(FormatException) :
		DictionaryBlockette("047"), input_sample_rate(0.),
		decimation_factor(0), decimation_offset(0), delay(0.),
		correction(0.)
	{
	    load(blockette);
	}

	/** Create a Blockette47 with null fields. */
	Blockette47() : DictionaryBlockette("047"), input_sample_rate(0.),
		decimation_factor(0), decimation_offset(0), delay(0.),
		correction(0.) { }
	Blockette47(const Blockette47 &b) : DictionaryBlockette("047") {
		*this = b; }
	~Blockette47() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette47 * getBlockette47(void) { return this; }
};

/** 
 * Channel Sensitivity/Gain Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette48 : public DictionaryBlockette
{
    public:
	string name;             //!< Response name.
	double sensitivity;      //!< Sensitivity.
	double frequency;        //!< Frequency (Hz).
	vector<double> cal_sensitivity; //!< Sensitivity for calibration.
	vector<double> cal_frequency; //!< Frequency of calibration sensitivity.
	vector<SeedTime> cal_time;      //!< Time of above calibration.

	/** Parse Blockette48 fields from a string. */
	Blockette48(const string &blockette) throw(FormatException) :
		DictionaryBlockette("048"), sensitivity(0.), frequency(0.)
	{
	    load(blockette);
	}

	/** Create a Blockette48 with null fields. */
	Blockette48() : DictionaryBlockette("048"), sensitivity(0.),
			frequency(0.) { }
	Blockette48(const Blockette48 &b) : DictionaryBlockette("048") {
			*this = b; }
	~Blockette48() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette48 * getBlockette48(void) { return this; }
};

/** 
 * Station Identifier Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette50 : public Seed
{
    public:
	string station;     //!< Station call letters.
	double latitude;    //!< Latitude (degrees).
	double longitude;   //!< Longitude (degrees).
	double elevation;   //!< Elevation (m).
	int num_channels;   //!< Number of channels.
	int num_comments;   //!< Number of station comments.
	string name;        //!< Site name.
	int network_id;     //!< Network identification code.
	string word_order;  //!< 32 bit word order.
	string short_order; //!< 16 bit word order.
	SeedTime start;     //!< Start effective date.
	SeedTime end;       //!< End effective date.
	string update;      //!< Update flag.
	string network;     //!< Network code.

	/** Parse Blockette50 fields from a string. */
	Blockette50(const string &blockette) throw(FormatException) :
		Seed("050"), latitude(0.), longitude(0.), elevation(0.),
		num_channels(0), num_comments(0), word_order("3210"),
		short_order("10")
	{
	    load(blockette);
	}

	/** Create a Blockette50 with null fields. */
	Blockette50() : Seed("050"), latitude(0.), longitude(0.), elevation(0.),
		num_channels(0), num_comments(0), word_order("3210"),
		short_order("10") { }
	Blockette50(const Blockette50 &b) : Seed("050") { *this = b; }
	~Blockette50() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette50 * getBlockette50(void) { return this; }
};

/** 
 * Station Comment Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette51 : public Seed
{
    public:
	SeedTime beg;      //!< Beginning effective time.
	SeedTime end;      //!< End effective time.
	int comment_code;  //!< Comment code key.
	int comment_level; //!< Comment level.

	/** Parse Blockette51 fields from a string. */
	Blockette51(const string &blockette) throw(FormatException) :
		Seed("051"), comment_code(0), comment_level(0)
	{
	    load(blockette);
	}

	/** Create a Blockette51 with null fields. */
	Blockette51() : Seed("051"), comment_code(0), comment_level(0) { }
	Blockette51(const Blockette51 &b) : Seed("051") { *this = b; }
	~Blockette51() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette51 * getBlockette51(void) { return this; }
};

/** 
 * Station Identifier Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette52 : public Seed
{
    public:
	string location;      //!< Location identifier.
	string channel;       //!< Channel identifier.
	int subchannel;       //!< Subchannel identifier.
	int instrument;       //!< Instrument identifier.
	string comment;       //!< Optional comment.
	int signal_units;     //!< Units of signal response.
	int calib_units;      //!< Units of calibration input.
	double latitude;      //!< Latitude (degrees).
	double longitude;     //!< Longitude (degrees).
	double elevation;     //!< Elevation (m).
	double local_depth;   //!< Local depth (m).
	double azimuth;       //!< Azimuth (degrees).
	double dip;           //!< Dip (degrees).
	int format_code;      //!< Data format identifier code.
	int reclen;	      //!< Data record length (as power of 2).
	double sample_rate;   //!< Sample rate (Hz).
	double clock_drift;   //!< Max clock drift (seconds).
	int num; 	      //!< Number of comments.
	string channel_flags; //!< Channel flags.
	SeedTime start;       //!< Start date.
	SeedTime end;         //!< End date.
	string update;        //!< Update flag.

	/** Parse Blockette52 fields from a string. */
	Blockette52(const string &blockette) throw(FormatException) :
		Seed("052"), subchannel(0), signal_units(0), calib_units(0),
		latitude(0.), longitude(0.), elevation(0.), local_depth(0.),
		azimuth(0.), dip(0.), format_code(0), reclen(0),sample_rate(0.),
		clock_drift(0.), num(0)
	{
	    load(blockette);
	}

	/** Create a Blockette52 with null fields. */
	Blockette52() : Seed("052"), subchannel(0), signal_units(0),
		calib_units(0), latitude(0.), longitude(0.), elevation(0.),
		local_depth(0.), azimuth(0.), dip(0.), format_code(0),
		reclen(0),sample_rate(0.), clock_drift(0.), num(0) { }
	Blockette52(const Blockette52 &b) : Seed("052") { *this = b; }
	~Blockette52() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette52 * getBlockette52(void) { return this; }
};

/** 
 * Response (Poles & Zeros) Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette53 : public Seed
{
    public:
	string type;      //!< Transfer function type.
	int stage;        //!< Stage sequence number.
	int input_units;  //!< Stage signal input units.
	int output_units; //!< Stage signal output units.
	double a0_norm;	  //!< AO normalization factor. (1.0 if none)
	double norm_freq; //!< Normalization frequency.
	vector<double> zr;       //!< Real zero.
	vector<double> zi;       //!< Imaginary zero.
	vector<double> zr_error; //!< Real zero error.
	vector<double> zi_error; //!< Imaginary zero error.
	vector<double> pr;       //!< Real pole.
	vector<double> pi;       //!< Imaginary pole.
	vector<double> pr_error; //!< Real pole error.
	vector<double> pi_error; //!< Imaginary pole error.
	bool from_b43;	  //!< True if this is created from blockette43

	/** Parse Blockette53 fields from a string. */
	Blockette53(const string &blockette) throw(FormatException) :
		Seed("053"), stage(0), input_units(0), output_units(0),
		a0_norm(0.), norm_freq(0.), from_b43(false)
	{
	    load(blockette);
	}

	/** Create a Blockette53 with null fields. */
	Blockette53() : Seed("053"), stage(0), input_units(0), output_units(0),
		a0_norm(0.), norm_freq(0.), from_b43(false) { }
	Blockette53(const Blockette53 &b) : Seed("053") { *this = b; }
	Blockette53(int stage_number, const Blockette43 &b);
	~Blockette53() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette53 * getBlockette53(void) { return this; }
};

/** 
 * Response (Coefficients) Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette54 : public Seed
{
    public:
	string type;         //!< Response type.
	int stage;           //!< Stage sequence number.
	int input_units;     //!< Stage signal input units.
	int output_units;    //!< Stage signal output units.
	vector<double> numerator;   //!< Numerator coefficients.
	vector<double> nerror;      //!< Numerator error.
	vector<double> denominator; //!< Denominator coefficients.
	vector<double> derror;      //!< Denominator error.
	bool from_b44;	     //!< True if this is created from blockette44

	/** Parse Blockette54 fields from a string. */
	Blockette54(const string &blockette) throw(FormatException) :
		Seed("054"), stage(0), input_units(0), output_units(0),
		from_b44(false)
	{
	    load(blockette);
	}

	/** Create a Blockette54 with null fields. */
	Blockette54() : Seed("054"),
		stage(0),input_units(0),output_units(0),from_b44(false) { }
	Blockette54(const Blockette54 &b) : Seed("054") { *this = b; }
	Blockette54(int stage_number, const Blockette44 &b);
	~Blockette54() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette54 * getBlockette54(void) { return this; }
};

/** 
 * Response List Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette55 : public Seed
{
    public:
	int stage;           //!< Stage sequence number.
	int input_units;     //!< Stage signal input units.
	int output_units;    //!< Stage signal output units.
	vector<double> frequency;   //!< Frequency (Hz).
	vector<double> amplitude;   //!< Amplitude.
	vector<double> amp_error;   //!< Amplitude error.
	vector<double> phase;       //!< Phase angle (degrees).
	vector<double> phase_error; //!< Phase error (degrees).
	bool from_b45;	     //!< True if this is created from blockette45

	/** Parse Blockette55 fields from a string. */
	Blockette55(const string &blockette) throw(FormatException) :
		Seed("055"), stage(0), input_units(0), output_units(0),
		from_b45(false)
	{
	    load(blockette);
	}

	/** Create a Blockette55 with null fields. */
	Blockette55() : Seed("055"),
		stage(0),input_units(0),output_units(0),from_b45(false) { }
	Blockette55(const Blockette55 &b) : Seed("055") { *this = b; }
	Blockette55(int stage_number, const Blockette45 &b);
	~Blockette55() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette55 * getBlockette55(void) { return this; }
};

/** 
 * Generic Response Dictionary Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette56 : public Seed
{
    public:
	int stage;            //!< Stage sequence number.
	int input_units;      //!< Stage signal input units.
	int output_units;     //!< Stage signal output units.
	vector<double> corner_freq;  //!< Corner frequency (Hz).
	vector<double> corner_slope; //!< Corner slope (db/decade).
	bool from_b46;	      //!< True if this is created from blockette46

	/** Parse Blockette56 fields from a string. */
	Blockette56(const string &blockette) throw(FormatException) :
		Seed("056"), stage(0), input_units(0), output_units(0),
		from_b46(false)
	{
	    load(blockette);
	}

	/** Create a Blockette56 with null fields. */
	Blockette56() : Seed("056"),
		stage(0),input_units(0),output_units(0),from_b46(false) { }
	Blockette56(const Blockette56 &b) : Seed("056") { *this = b; }
	Blockette56(int stage_number, const Blockette46 &b);
	~Blockette56() { }

	void load(const string &blockette) throw(FormatException);

	string str();
	string longStr();

	Blockette56 * getBlockette56(void) { return this; }
};

/** 
 * Decimation Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette57 : public Seed
{
    public:
	int stage;                //!< Stage sequence number.
	double input_sample_rate; //!< Input sample rate.
	int decimation_factor;    //!< Decimation factor.
	int decimation_offset;    //!< Decimation offset.
	double delay;             //!< Estimated delay (seconds).
	double correction;        //!< Correction applied (seconds).
	bool from_b47;	          //!< True if this is created from blockette47

	/** Parse Blockette57 fields from a string. */
	Blockette57(const string &blockette) throw(FormatException) :
		Seed("057"), stage(0), input_sample_rate(0.),
		decimation_factor(0), decimation_offset(0), delay(0.),
		correction(0.), from_b47(false)
	{
		load(blockette);
	}

	/** Create a Blockette57 with null fields. */
	Blockette57() : Seed("057"), stage(0), input_sample_rate(0.),
		decimation_factor(0), decimation_offset(0), delay(0.),
		correction(0.), from_b47(false) { }
	Blockette57(const Blockette57 &b) : Seed("057") { *this = b; }
	Blockette57(int stage_number, const Blockette47 &b);
	~Blockette57() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette57 * getBlockette57(void) { return this; }
};

/** 
 * Channel Sensitivity/Gain Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette58 : public Seed
{
    public:
	int stage;               //!< Stage sequence number.
	double sensitivity;      //!< Sensitivity.
	double frequency;        //!< Frequency (Hz).
	vector<double> cal_sensitivity; //!< Sensitivity for calibration.
	vector<double> cal_frequency; //!< Frequency of calibration sensitivity.
	vector<SeedTime> cal_time;    //!< Time of above calibration.
	bool from_b48;	         //!< True if this is created from blockette48

	/** Parse Blockette58 fields from a string. */
	Blockette58(const string &blockette) throw(FormatException) :
		Seed("058"), stage(0), sensitivity(0.), frequency(0.),
		from_b48(false)
	{
	    load(blockette);
	}

	/** Create a Blockette58 with null fields. */
	Blockette58() : Seed("058"),
		stage(0),sensitivity(0.),frequency(0.),from_b48(false) { }
	Blockette58(const Blockette58 &b) : Seed("058") { *this = b; }
	Blockette58(int stage_number, const Blockette48 &b);
	~Blockette58() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette58 * getBlockette58(void) { return this; }
};

/** 
 * Comment Description Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette59 : public Seed
{
    public:
	SeedTime beg;      //!< Beginning effective time.
	SeedTime end;      //!< End effective time.
	int comment_code;  //!< Comment code key.
	int comment_level; //!< Comment level.

	/** Parse Blockette59 fields from a string. */
	Blockette59(const string &blockette) throw(FormatException) :
		Seed("059"), comment_code(0), comment_level(0)
	{
	    load(blockette);
	}

	/** Create a Blockette59 with null fields. */
	Blockette59() : Seed("059"), comment_code(0), comment_level(0) { }
	Blockette59(const Blockette59 &b) : Seed("059") { *this = b; }
	~Blockette59() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette59 * getBlockette59(void) { return this; }
};

class ResponseStage
{
    public:
	int stage;	  //!< Stage sequence number
	vector<int> code; //!< Response lookup keys for each response
};

/** 
 * Response Reference Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette60 : public Seed
{
    public:
	vector<ResponseStage> response; //!< Responses for each stage

	/** Parse Blockette60 fields from a string. */
	Blockette60(const string &blockette) throw(FormatException) :Seed("060")
	{
	    load(blockette);
	}

	/** Create a Blockette60 with null fields. */
	Blockette60() : Seed("060") { }
	Blockette60(const Blockette60 &b) : Seed("060") { *this = b; }
	~Blockette60() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette60 * getBlockette60(void) { return this; }
};

/** 
 * FIR Response Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette61 : public Seed
{
    public:
	int stage;            //!< Stage sequence number.
	string name;          //!< Response name.
	string symmetry_code; //!< Symmetry Code.
	int input_units;      //!< Signal In Units.
	int output_units;     //!< Signal Out Units.
	vector<double> coef;  //!< FIR Coefficient.
	bool from_b41;	      //!< True if this is created from blockette41

	/** Parse Blockette61 fields from a string. */
	Blockette61(const string &blockette) throw(FormatException) :
		Seed("061"), stage(0), input_units(0), output_units(0),
		from_b41(false)
	{
	    load(blockette);
	}

	/** Create a Blockette61 with null fields. */
	Blockette61() : Seed("061"),
		stage(0),input_units(0),output_units(0),from_b41(false) { }
	Blockette61(const Blockette61 &b) : Seed("061") { *this = b; }
	Blockette61(int stage_number, const Blockette41 &b);
	~Blockette61() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette61 * getBlockette61(void) { return this; }
};

/** 
 * Response (Polynomial) Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette62 : public Seed
{
    public:
	string transfer_type; //!< Transfer Function Type.
	int stage;            //!< Stage sequence number.
	int input_units;      //!< Stage Signal Input Units.
	int output_units;     //!< Stage Signal Output Units.
	string poly_type;     //!< Polynomial Approximation Type.
	string freq_units;    //!< Valid Frequency Units.
	double min_freq;      //!< Lower Valid Frequency Bound.
	double max_freq;      //!< Upper Valid Frequency Bound.
	double min_approx;    //!< Lower Bound of Approximation.
	double max_approx;    //!< Upper Bound of Approximation.
	double max_error;     //!< Maximum Absolute Error.
	vector<double> coef;  //!< Polynomial Coefficients.
	vector<double> error; //!< Polynomial Coefficient Error.

	/** Parse Blockette62 fields from a string. */
	Blockette62(const string &blockette) throw(FormatException) :
		Seed("062"), stage(0), input_units(0), output_units(0),
		min_freq(0.), max_freq(0.), min_approx(0.), max_approx(0.),
		max_error(0.)
	{
	    load(blockette);
	}

	/** Create a Blockette62 with null fields. */
	Blockette62() : Seed("062"), stage(0), input_units(0), output_units(0),
		min_freq(0.), max_freq(0.), min_approx(0.), max_approx(0.),
		max_error(0.) { }

	Blockette62(const Blockette62 &b) : Seed("062") { *this = b; }
	~Blockette62() { }

	void load(const string &blockette) throw(FormatException);

	Blockette62 * getBlockette62(void) { return this; }
	string str();
	string longStr();
};

/** 
 * Volume Time Span Index Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette70 : public Seed
{
    public:
	string flag;  //!< Time span flag.
	SeedTime beg; //!< Beginning time of data span.
	SeedTime end; //!< End time of data span.

	/** Parse Blockette70 fields from a string. */
	Blockette70(const string &blockette) throw(FormatException) :Seed("070")
	{
	    load(blockette);
	}

	/** Create a Blockette70 with null fields. */
	Blockette70() : Seed("070") { }
	Blockette70(const Blockette70 &b) : Seed("070") { *this = b; }
	~Blockette70() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette70 * getBlockette70(void) { return this; }
};

/** 
 * Hypocenter Information Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette71 : public Seed
{
    public:
	SeedTime origin_time; //!< Origin time of event.
	int source_code;      //!< Hypocenter source identifier.
	double latitude;      //!< Latitude of event (degrees).
	double longitude;     //!< Longitude of event (degrees).
	double depth;         //!< Depth (km).
	vector<double> magnitude; //!< Magnitude.
	vector<string> mag_type;  //!< Magnitude type.
	vector<int> mag_source;   //!< Magnitude source.
	int seismic_region;   //!< Seismic region.
	int seismic_location; //!< Seismic location.
	string region_name;   //!< Region name.

	/** Parse Blockette71 fields from a string. */
	Blockette71(const string &blockette) throw(FormatException) :
		Seed("071"), source_code(0), latitude(0.), longitude(0.),
		depth(0.), seismic_region(0), seismic_location(0)
	{
	    load(blockette);
	}

	/** Create a Blockette71 with null fields. */
	Blockette71() : Seed("071"), source_code(0), latitude(0.),
		longitude(0.), depth(0.), seismic_region(0),
		seismic_location(0) { }
	Blockette71(const Blockette71 &b) : Seed("071") { *this = b; }
	~Blockette71() { }

	void load(const string &blockette) throw(FormatException);

	string str();
	string longStr();

	Blockette71 * getBlockette71(void) { return this; }
};

/** 
 * Event Phases Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette72 : public Seed
{
    public:
	string station;    //!< Station identifier.
	string location;   //!< Location identifier.
	string channel;    //!< Channel identifier.
	SeedTime time;	   //!< Arrival time of phase.
	double amplitude;  //!< Amplitude of signal.
	double period;     //!< Period of signal (seconds).
	double snr;        //!< Sginal-to-noise ratio.
	string phase_name; //!< Name of phase.
	int source;        //!< Source.
	string network;    //!< Network code.

	/** Parse Blockette72 fields from a string. */
	Blockette72(const string &blockette) throw(FormatException) :
		Seed("072"), amplitude(0.), period(0.), snr(0.), source(0)
	{
	    load(blockette);
	}

	/** Create a Blockette72 with null fields. */
	Blockette72() : Seed("072"), amplitude(0.), period(0.), snr(0.),
			source(0) { }
	Blockette72(const Blockette72 &b) : Seed("072") { *this = b; }
	~Blockette72() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette72 * getBlockette72(void) { return this; }
};

/** 
 * Time Span Data Start Index Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette73 : public Seed
{
    public:
	vector<string> station;  //!< Station identifier of data piece.
	vector<string> location; //!< Location identifier.
	vector<string> channel;  //!< Channel identifier.
	vector<SeedTime> time;   //!< Time of record.
	vector<int> seqno;       //!< Sequence number of first record.
	vector<int> subseqno;    //!< Sub-sequence number.

	/** Parse Blockette73 fields from a string. */
	Blockette73(const string &blockette) throw(FormatException) :Seed("073")
	{
	    load(blockette);
	}

	/** Create a Blockette73 with null fields. */
	Blockette73() : Seed("073") { }
	Blockette73(const Blockette73 &b) : Seed("073") { *this = b; }
	~Blockette73() { }

	void load(const string &blockette) throw(FormatException);
	string str();

	Blockette73 * getBlockette73(void) { return this; }
};

/** 
 * Time Series Index Blockette
 *
 *@see Seed
 *@see SeedStream
 */
class Blockette74 : public Seed
{
    public:
	string station;       //!< Station identifier.
	string location;      //!< Location identifier.
	string channel;       //!< Channel identifier.
	SeedTime start_time;  //!< Series start time.
	int start_seqno;      //!< Sequence number of first data.
	int start_subseqno;   //!< Sub-sequence number.
	SeedTime end_time;    //!< Series end time.
	int end_seqno;        //!< Sequence number of last record.
	int end_subseqno;     //!< Sub-sequence number.

	vector<SeedTime> accel_time; //!< Record start time.
	vector<int> accel_seqno;     //!< Sequence number of record.
	vector<int> accel_subseqno;  //!< Sub-sequence number.

	string network;       //!< Network code. */

	/** Parse Blockette74 fields from a string. */
	Blockette74(const string &blockette) throw(FormatException) :
		Seed("074"), start_seqno(0), start_subseqno(0), end_seqno(0),
		end_subseqno(0)
	{
	    load(blockette);
	}

	/** Create a Blockette74 with null fields. */
	Blockette74() : Seed("074"), start_seqno(0), start_subseqno(0),
			end_seqno(0), end_subseqno(0) { }
	Blockette74(const Blockette74 &b) : Seed("074") { *this = b; }
	~Blockette74() { }

	void load(const string &blockette) throw(FormatException);
	string str();
	string longStr();

	Blockette74 * getBlockette74(void) { return this; }
};

/**
 * This abstract class is the superclass of all classes that represent
 * SEED data blockettes.
 *
 *@see Seed
 *@see SeedStream
 */
class DataBlockette : public Seed
{
    public:
	DataBlockette(const char *name) : Seed(name) { }
	/** Returns the fixed header length of this SEED data blockette. */
	int length() { return len; }

	/** Loads the data blockette from a byte array */
	virtual void loadDB(const string &bytes, int *wo, int *so)
				throw(FormatException) = 0;
	
    protected:
	/** Fixed blockette length (bytes) */
	int len;		
};

/** 
 * Sample Rate Blockette  (12 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette100 : public DataBlockette
{
    public:
	int next;            //!< Next blockette's byte offset.
	double sample_rate; //!< Actual Sample rate.
	char flags;          //!< Flags.
	char reserved[3];    //!< Reserved byte.

	/** Parse Blockette100 fields from a byte array. */
	Blockette100(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("100"), next(0),
		sample_rate(0.), flags(0)
	{
	    len = 12;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette100 with null fields. */
	Blockette100() : DataBlockette("100"), next(0), sample_rate(0.),flags(0)
	{
	    len = 12;
	}
	Blockette100(const Blockette100 &b) : DataBlockette("100") {
		*this = b; }
	~Blockette100() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette100 * getBlockette100(void) { return this; }
};

/** 
 * Generic Event Detection Blockette (52 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette200 : public DataBlockette
{
    public:
	int next;         //!< Next blockette's byte offset.
	float amplitude;  //!< Signal amplitude.
	float period;     //!< Signal period.
	float background; //!< Background estimate.
	char flags;       //!< Event detection flags.
	char reserved;    //!< Reserved byte.
	SeedTime time;    //!< Signal onset time.
	string name;	  //!< Detector Name.

	/** Parse Blockette200 fields from a byte array. */
	Blockette200(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("200"), next(0),
		amplitude(0.), period(0.), background(0.), flags(0), reserved(0)
	{
	    len = 52;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette200 with null fields. */
	Blockette200() : DataBlockette("200"), next(0), amplitude(0.),
			period(0.), background(0.), flags(0), reserved(0)
	{
	    len = 52;
	}

	Blockette200(const Blockette200 &b) : DataBlockette("200") { *this = b;}
	~Blockette200() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette200 * getBlockette200(void) { return this; }
};

/** 
 * Murdock Event Detection Blockette (60 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette201 : public DataBlockette
{
    public:
	int next;         //!< Next blockette's byte offset.
	float amplitude;  //!< Signal amplitude.
	float period;     //!< Signal period.
	float background; //!< Background estimate.
	char flags;       //!< Event detection flags.
	char reserved;    //!< Reserved byte.
	SeedTime time;    //!< Signal onset time.
	char snr[6];      //!< Signal-to-noise ratio values
	char look_back;   //!< Lookback value
	char algorithm;   //!< Pick algorithm
	string name;	  //!< Detector Name.

	/** Parse Blockette201 fields from a byte array. */
	Blockette201(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("201"), next(0),
		amplitude(0.), period(0.), background(0.), flags(0),
		reserved(0), look_back(0), algorithm(0)
	{
	    len = 60;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette201 with null fields. */
	Blockette201() : DataBlockette("201"), next(0), amplitude(0.),
			period(0.), background(0.), flags(0), reserved(0),
			look_back(0), algorithm(0)
	{
	    len = 60;
	}

	Blockette201(const Blockette201 &b) : DataBlockette("201") { *this = b;}
	~Blockette201() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette201 * getBlockette201(void) { return this; }
};

/** 
 * Step Calibration Blockette (60 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette300 : public DataBlockette
{
    public:
	int next;              //!< Next blockette's byte offset.
	SeedTime time;         //!< Beginning of calibration time.
	int num_steps;         //!< Number of step calibrations.
	char flags;            //!< Calibration flags.
	unsigned int step;     //!< Step duration.
	unsigned int interval; //!< Interval duration.
	float amplitude;       //!< Calibration signal amplitude.
	string channel;	       //!< Channel with calibration input.
	char reserved;         //!< Reserved byte.
	unsigned int ref_amp;  //!< Reference amplitude.
	string coupling;       //!< Coupling
	string rolloff;        //!< Rolloff

	/** Parse Blockette300 fields from a byte array. */
	Blockette300(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("300"), next(0),
		num_steps(0), flags(0), step(0), interval(0), amplitude(0.),
		reserved(0), ref_amp(0)
	{
	    len = 60;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette300 with null fields. */
	Blockette300() : DataBlockette("300"), next(0), num_steps(0), flags(0),
		step(0), interval(0), amplitude(0.), reserved(0), ref_amp(0)
	{
	    len = 60;
	}

	Blockette300(const Blockette300 &b) : DataBlockette("300") { *this = b;}
	~Blockette300() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette300 * getBlockette300(void) { return this; }
};

/** 
 * Sine Calibration Blockette (60 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette310 : public DataBlockette
{
    public:
	int next;              //!< Next blockette's byte offset.
	SeedTime time;         //!< Beginning of calibration time.
	char reserved1;        //!< Reserved byte.
	char flags;            //!< Calibration flags.
	unsigned int duration; //!< Calibration duration.
	float period;          //!< Period of signal (seconds).
	float amplitude;       //!< Amplitude of signal.
	string channel;	       //!< Channel with calibration input.
	char reserved2;        //!< Reserved byte.
	unsigned int ref_amp;  //!< Reference amplitude.
	string coupling;       //!< Coupling
	string rolloff;        //!< Rolloff

	/** Parse Blockette310 fields from a byte array. */
	Blockette310(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("310"), next(0),
		reserved1(0), flags(0), duration(0), period(0), amplitude(0.),
		reserved2(0), ref_amp(0)
	{
	    len = 60;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette310 with null fields. */
	Blockette310() : DataBlockette("310"), next(0), reserved1(0), flags(0),
		duration(0), period(0), amplitude(0.), reserved2(0), ref_amp(0)
	{
	    len = 60;
	}

	Blockette310(const Blockette310 &b) : DataBlockette("310") { *this = b;}
	~Blockette310() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette310 * getBlockette310(void) { return this; }
};

/** 
 * Pseudo-random Calibration Blockette (64 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette320 : public DataBlockette
{
    public:
	int next;              //!< Next blockette's byte offset.
	SeedTime time;         //!< Beginning of calibration time.
	char reserved1;        //!< Reserved byte.
	char flags;            //!< Calibration flags.
	unsigned int duration; //!< Calibration duration.
	float amplitude;       //!< Peak-to-peak amplitude of steps
	string channel;	       //!< Channel with calibration input.
	char reserved2;        //!< Reserved byte.
	unsigned int ref_amp;  //!< Reference amplitude.
	string coupling;       //!< Coupling
	string rolloff;        //!< Rolloff
	string noise;          //!< Noise type.

	/** Parse Blockette320 fields from a byte array. */
	Blockette320(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("320"), next(0),
		reserved1(0), flags(0), duration(0), amplitude(0.),reserved2(0),
		ref_amp(0)
	{
	    len = 64;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette320 with null fields. */
	Blockette320() : DataBlockette("320"), next(0), reserved1(0), flags(0),
		duration(0), amplitude(0.), reserved2(0), ref_amp(0)
	{
	    len = 64;
	}

	Blockette320(const Blockette320 &b) : DataBlockette("320") { *this = b;}
	~Blockette320() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette320 * getBlockette320(void) { return this; }
};

/** 
 * Generic Calibration Blockette (28 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette390 : public DataBlockette
{
    public:
	int next;              //!< Next blockette's byte offset.
	SeedTime time;         //!< Beginning of calibration time.
	char reserved1;        //!< Reserved byte.
	char flags;            //!< Calibration flags.
	unsigned int duration; //!< Calibration duration.
	float amplitude;       //!< Peak-to-peak amplitude of steps
	string channel;	       //!< Channel with calibration input.
	char reserved2;        //!< Reserved byte.

	/** Parse Blockette390 fields from a byte array. */
	Blockette390(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("390"), next(0),
		reserved1(0), flags(0), duration(0), amplitude(0.), reserved2(0)
	{
	    len = 28;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette390 with null fields. */
	Blockette390() : DataBlockette("390"), next(0), reserved1(0), flags(0),
			duration(0), amplitude(0.), reserved2(0)
	{
	    len = 28;
	}

	Blockette390(const Blockette390 &b) : DataBlockette("390") { *this = b;}
	~Blockette390() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette390 * getBlockette390(void) { return this; }
};

/** 
 * Calibration Abort Blockette (16 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette395 : public DataBlockette
{
    public:
	int next;          //!< Next blockette's byte offset.
	SeedTime end_time; //!< End of calibration time.
	char reserved[2];  //!< Reserved bytes.

	/** Parse Blockette395 fields from a byte array. */
	Blockette395(const string &bytes, int *wo, int *so)
			throw(FormatException) : DataBlockette("395")
	{
	    len = 16;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette395 with null fields. */
	Blockette395() : DataBlockette("395")
	{
	    len = 16;
	}

	Blockette395(const Blockette395 &b) : DataBlockette("395") { *this = b;}
	~Blockette395() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette395 * getBlockette395(void) { return this; }
};

/** 
 * Beam Blockette (16 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette400 : public DataBlockette
{
    public:
	int next;         //!< Next blockette's byte offset.
	float azimuth;    //!< Beam azimuth (degrees).
	float slowness;   //!< Beam slowness (sec/degrees).
	int config;       //!< Beam configuration. (Blockette 35 look_up)
	char reserved[2]; //!< Reserved bytes.

	/** Parse Blockette400 fields from a byte array. */
	Blockette400(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("400"), next(0),
		azimuth(0.), slowness(0.)
	{
	    len = 16;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette400 with null fields. */
	Blockette400() : DataBlockette("400"), next(0), azimuth(0.),slowness(0.)
	{
	    len = 16;
	}

	Blockette400(const Blockette400 &b) : DataBlockette("400") { *this = b;}
	~Blockette400() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette400 * getBlockette400(void) { return this; }
};

/** 
 * Beam /elay Blockette (6 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette405 : public DataBlockette
{
    public:
	int next;  //!< Next blockette's byte offset.
	int delay; //!< Array of delay values (in .0001 second ticks).

	/** Parse Blockette405 fields from a byte array. */
	Blockette405(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("405"), next(0)
	{
	    len = 6;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette405 with null fields. */
	Blockette405() : DataBlockette("405"), next(0)
	{
	    len = 6;
	}

	Blockette405(const Blockette405 &b) : DataBlockette("405") { *this = b;}
	~Blockette405() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette405 * getBlockette405(void) { return this; }
};

/** 
 * Timing Blockette (200 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */
class Blockette500 : public DataBlockette
{
    public:
	int next;           //!< Next blockette's byte offset.
	float correction;   //!< VCO correction
	SeedTime time;	    //!< Time of exception.
	int micro_sec;      //!< Microsecond.
	int quality;	    //!< Reception quality.
	unsigned int count; //!< Exception count.
	string type;	    //!< Exception type.
	string model;	    //!< Clock model.
	string status;	    //!< Clock status.

	/** Parse Blockette500 fields from a byte array. */
	Blockette500(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("500"), next(0),
		correction(0.), micro_sec(0), quality(0), count(0)
	{
	    len = 200;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette500 with null fields. */
	Blockette500() : DataBlockette("500"), next(0), correction(0.),
			micro_sec(0), quality(0), count(0)
	{
	    len = 200;
	}

	Blockette500(const Blockette500 &b) : DataBlockette("500") { *this = b;}
	~Blockette500() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette500 * getBlockette500(void) { return this; }
};

/** 
 * Data Only SEED Blockette (8 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */

class Blockette1000 : public DataBlockette
{
    public:
	int next;        //!< Next blockette's byte offset.
	char format;     //!< Encoding format.
	char word_order; //!< Word order (0-little, 1-big indian).
	int lreclen;     //!< Data record length as a power of 2.
	char reserved;   //!< Reserved byte.

	/** Parse Blockette1000 fields from a byte array. */
	Blockette1000(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("1000"), next(0),
		format(0), word_order(0), lreclen(0), reserved(0)
	{
	    len = 8;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette1000 with null fields. */
	Blockette1000() : DataBlockette("1000"), next(0), format(0),
			word_order(0), lreclen(0), reserved(0)
	{
	    len = 8;
	}
	Blockette1000(const Blockette1000 &b) : DataBlockette("1000") {
		*this = b; }
	~Blockette1000() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette1000 * getBlockette1000(void) { return this; }
};

/** 
 * Data Extension Blockette (8 bytes)
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */

class Blockette1001 : public DataBlockette
{
    public:
	int next;      //!< Next blockette's byte offset.
	int timing;    //!< Timing quality.
	int micro_sec; //!< Microsecond.
	char reserved; //!< Reserved.
	int count;     //!< Frame count.

	/** Parse Blockette1001 fields from a byte array. */
	Blockette1001(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("1001"), next(0),
		timing(0), micro_sec(0), reserved(0), count(0)
	{
	    len = 8;
	    loadDB(bytes, wo, so);
	}

	/** Create a Blockette1001 with null fields. */
	Blockette1001() : DataBlockette("1001"), next(0), timing(0),
			micro_sec(0), reserved(0),count(0)
	{
	    len = 8;
	}
	Blockette1001(const Blockette1001 &b) : DataBlockette("1001") {
			*this = b; }
	~Blockette1001() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette1001 * getBlockette1001(void) { return this; }
};

/** 
 * Variable Length Opaque Data Blockette
 *
 *@see Seed
 *@see DataBlockette
 *@see SeedStream
 */

class Blockette2000 : public DataBlockette
{
    public:
	int next;            //!< Next blockette's byte offset.
	int length;          //!< Total blockette length in bytes.
	int offset;          //!< Offset to Opaque Data
	unsigned int record; //!< Record number
	char big_endian;     //!< Data word order.
	char flags;	     //!< Opaque Data flags
	int num_fields;      //!< Number of Opaque Header fields.
	string fields;       //!< Opaque Data Header fields.
	string data;	     //!< opaque data

	/** Parse Blockette2000 fields from a byte array. */
	Blockette2000(const string &bytes, int *wo, int *so)
		throw(FormatException) : DataBlockette("2000"), next(0),
		length(0), offset(0), record(0), big_endian(0), flags(0),
		num_fields(0), fields(NULL), data(NULL)
	{
	    loadDB(bytes, wo, so);
	    len = length;
	}

	/** Create a Blockette2000 with null fields. */
	Blockette2000() : DataBlockette("2000"), next(0), length(0), offset(0),
		record(0), big_endian(0), flags(0), num_fields(0), fields(NULL),
		data(NULL)
	{
	}
	Blockette2000(const Blockette2000 &b) : DataBlockette("2000") {
			*this = b; }
	~Blockette2000() { }

	void loadDB(const string &bytes, int *wo, int *so) throw(FormatException);
	string str();

	Blockette2000 * getBlockette2000(void) { return this; }
};

#endif
