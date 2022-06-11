/** \file Station.h
 *  \brief Declares the Station and Channel class members and methods.
 *  \author Ivan Henson
 */
#ifndef _STATION_H_
#define _STATION_H_

#include <iostream>
#include <vector>
#include "seed/Blockettes.h"
using namespace std;

class Channel
{
    public:
	Channel(Blockette52 *blockette52) { b52 = *blockette52; }
        Channel(const Channel &c) {
	    b52 = c.b52;
	    for(int i = 0; i < (int)c.response.size(); i++) add(c.response[i]);
	}
	~Channel() { 
	    for(int i = 0; i < (int)response.size(); i++) delete response[i];
	}

	/** Adds a Blockette of type 53-62 to this Station. */
	void add(Seed *s);

	Blockette52 b52;
	vector<Seed *> response;
};

/**
 * This class holds all the blockettes of type 51-61 which are
 * associated with a single Blockette50.
 */
class Station : public Seed
{
    public:
	Blockette50 b50;
	vector<Blockette51> b51;
	vector<Channel *> channels;


	Station(Blockette50 *blockette50) : Seed("Station") {
		b50 = *blockette50; }
        Station(const Station &s) : Seed("Station") {
	    b50 = s.b50;
	    b51 = s.b51;
	    for(int i = 0; i < (int)s.channels.size(); i++) {
		channels.push_back(new Channel(*s.channels[i]));
	    }
	}

	~Station() {
	    for(int i = 0; i < (int)channels.size(); i++) delete channels[i];
	}

	/** Adds a Blockette of type 51-62 to this Station. */
	void add(Seed *s);
};

#endif
