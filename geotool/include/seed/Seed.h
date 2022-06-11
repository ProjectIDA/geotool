/** \file Seed.h
 *  \brief Declares the Seed class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_H_
#define _SEED_H_

#include <string>
#include "seed/FormatException.h"

class UnknownBlockette;
class Blockette5;
class Blockette8;
class Blockette10;
class Blockette11;
class Blockette12;
class Blockette30;
class Blockette31;
class Blockette32;
class Blockette33;
class Blockette34;
class Blockette35;
class Blockette41;
class Blockette42;
class Blockette43;
class Blockette44;
class Blockette45;
class Blockette46;
class Blockette47;
class Blockette48;
class Blockette50;
class Blockette51;
class Blockette52;
class Blockette53;
class Blockette54;
class Blockette55;
class Blockette56;
class Blockette57;
class Blockette58;
class Blockette59;
class Blockette60;
class Blockette61;
class Blockette62;
class Blockette70;
class Blockette71;
class Blockette72;
class Blockette73;
class Blockette74;
class Blockette100;
class Blockette200;
class Blockette201;
class Blockette300;
class Blockette310;
class Blockette320;
class Blockette390;
class Blockette395;
class Blockette400;
class Blockette405;
class Blockette500;
class Blockette1000;
class Blockette1001;
class Blockette2000;
class DataBlockette;
class DictionaryBlockette;
class DataRecord;
class SeedData;

/**
 * This abstract class is the superclass of all classes that represent
 * SEED objects. SEED objects include all types of blockettes (Volume,
 * Dictionary, Station, Time Span, Data), the Data Fixed Header and Data
 * Records.
 *
 */

class Seed
{
    public:
	Seed(const char *name) {
	    type.assign(name);
	}
	Seed & operator=(const Seed &s) {
	    if(this == &s) return *this;
	    return *this;
	}

	/** Loads the SEED object from a String. Subclassed must define
	 *  this method, which parses the input String for blockette
	 *  fields.
	 */
	virtual void load(const string &fields) throw(FormatException) { }
	virtual string str() { return string(""); }
	virtual string longStr() { return str(); }

	virtual ~Seed() {}

	int parseInt(const string &s, const string &name)
			throw(FormatException);
	double parseDouble(const string &s, const string &name)
			throw(FormatException);
	string &getType() { return type; }

	static int parseInt(const string &s) throw(FormatException);
	static double parseDouble(const string &s) throw(FormatException);

	static string variable(const string &s) {
	    size_t i;
	    if( (i = s.find('~')) != string::npos ) {
		return s.substr(0, i);
	    }
	    return s;
	}
	static string trim(const string &s) {
	    // Trim leading and trailing spaces
	    size_t startpos = s.find_first_not_of(" \t\n");
	    size_t endpos = s.find_last_not_of(" \t\n");

	    // if all spaces or empty return an empty string
	    if(( string::npos == startpos ) || ( string::npos == endpos )) {
		return string("");
	    }
	    else {
		return s.substr( startpos, endpos-startpos+1 );
	    }
	}

	virtual UnknownBlockette  * getUnknownBlockette(void) { return NULL; }
	virtual Blockette5  * getBlockette5(void)  { return NULL; }
	virtual Blockette8  * getBlockette8(void)  { return NULL; }
	virtual Blockette10 * getBlockette10(void) { return NULL; }
	virtual Blockette11 * getBlockette11(void) { return NULL; }
	virtual Blockette12 * getBlockette12(void) { return NULL; }
	virtual Blockette30 * getBlockette30(void) { return NULL; }
	virtual Blockette31 * getBlockette31(void) { return NULL; }
	virtual Blockette32 * getBlockette32(void) { return NULL; }
	virtual Blockette33 * getBlockette33(void) { return NULL; }
	virtual Blockette34 * getBlockette34(void) { return NULL; }
	virtual Blockette35 * getBlockette35(void) { return NULL; }
	virtual Blockette41 * getBlockette41(void) { return NULL; }
	virtual Blockette42 * getBlockette42(void) { return NULL; }
	virtual Blockette43 * getBlockette43(void) { return NULL; }
	virtual Blockette44 * getBlockette44(void) { return NULL; }
	virtual Blockette45 * getBlockette45(void) { return NULL; }
	virtual Blockette46 * getBlockette46(void) { return NULL; }
	virtual Blockette47 * getBlockette47(void) { return NULL; }
	virtual Blockette48 * getBlockette48(void) { return NULL; }
	virtual Blockette50 * getBlockette50(void) { return NULL; }
	virtual Blockette51 * getBlockette51(void) { return NULL; }
	virtual Blockette52 * getBlockette52(void) { return NULL; }
	virtual Blockette53 * getBlockette53(void) { return NULL; }
	virtual Blockette54 * getBlockette54(void) { return NULL; }
	virtual Blockette55 * getBlockette55(void) { return NULL; }
	virtual Blockette56 * getBlockette56(void) { return NULL; }
	virtual Blockette57 * getBlockette57(void) { return NULL; }
	virtual Blockette58 * getBlockette58(void) { return NULL; }
	virtual Blockette59 * getBlockette59(void) { return NULL; }
	virtual Blockette60 * getBlockette60(void) { return NULL; }
	virtual Blockette61 * getBlockette61(void) { return NULL; }
	virtual Blockette62 * getBlockette62(void) { return NULL; }
	virtual Blockette70 * getBlockette70(void) { return NULL; }
	virtual Blockette71 * getBlockette71(void) { return NULL; }
	virtual Blockette72 * getBlockette72(void) { return NULL; }
	virtual Blockette73 * getBlockette73(void) { return NULL; }
	virtual Blockette74 * getBlockette74(void) { return NULL; }
	virtual Blockette100 * getBlockette100(void) { return NULL; }
	virtual Blockette200 * getBlockette200(void) { return NULL; }
	virtual Blockette201 * getBlockette201(void) { return NULL; }
	virtual Blockette300 * getBlockette300(void) { return NULL; }
	virtual Blockette310 * getBlockette310(void) { return NULL; }
	virtual Blockette320 * getBlockette320(void) { return NULL; }
	virtual Blockette390 * getBlockette390(void) { return NULL; }
	virtual Blockette395 * getBlockette395(void) { return NULL; }
	virtual Blockette400 * getBlockette400(void) { return NULL; }
	virtual Blockette405 * getBlockette405(void) { return NULL; }
	virtual Blockette500 * getBlockette500(void) { return NULL; }
	virtual Blockette1000 * getBlockette1000(void) { return NULL; }
	virtual Blockette1001 * getBlockette1001(void) { return NULL; }
	virtual Blockette2000 * getBlockette2000(void) { return NULL; }
	virtual DictionaryBlockette * getDictionaryBlockette(void) {
		return NULL; }

	virtual DataRecord * getDataRecord(void) { return NULL; }
	virtual SeedData * getSeedData(void) { return NULL; }

	static Seed *createBlockette(const string &s);
	static DataBlockette *createDataBlockette(const string &s);
	static DataBlockette *createDataBlockette(DataBlockette &s);

    protected:
	/** The name of this SEED object. */
	string type;
};

#endif
