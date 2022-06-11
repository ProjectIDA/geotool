/** \file Dictionary.h
 *  \brief Declares the Dictionalry class members and methods.
 *  \author Ivan Henson
 */
#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <vector>
#include "seed/Blockettes.h"

/**
 * This class holds all the blockettes of type 30-48 which are
 * associated with an abbreviation dictionary
 */
class Dictionary : public Seed
{
    public:
	Dictionary() : Seed("Dictionary") { }
	~Dictionary() { clear(); }

	/** Adds a DictionaryBlockette (30-34, 41-48) */
	void add(DictionaryBlockette *s);

	void clear();

	Blockette30 * getB30(int lookup_code);
	Blockette31 * getB31(int lookup_code);
	Blockette32 * getB32(int lookup_code);
	Blockette33 * getB33(int lookup_code);
	Blockette34 * getB34(int lookup_code);
	Blockette41 * getB41(int lookup_code);
	Blockette42 * getB42(int lookup_code);
	Blockette43 * getB43(int lookup_code);
	Blockette44 * getB44(int lookup_code);
	Blockette45 * getB45(int lookup_code);
	Blockette46 * getB46(int lookup_code);
	Blockette47 * getB47(int lookup_code);
	Blockette48 * getB48(int lookup_code);

	vector<Blockette30 *> b30;
	vector<Blockette31 *> b31;
	vector<Blockette32 *> b32;
	vector<Blockette33 *> b33;
	vector<Blockette34 *> b34;
	vector<Blockette41 *> b41;
	vector<Blockette42 *> b42;
	vector<Blockette43 *> b43;
	vector<Blockette44 *> b44;
	vector<Blockette45 *> b45;
	vector<Blockette46 *> b46;
	vector<Blockette47 *> b47;
	vector<Blockette48 *> b48;

};

#endif
