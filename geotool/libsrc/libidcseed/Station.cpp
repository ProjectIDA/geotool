/** \file Channel.cpp
 *  \brief Defines a class that holds station specific blockettes and
 *  a class that holds channel specific blockettes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <vector>
#include "seed/Station.h"
using namespace std;

/** Adds a Blockette of type 53-62 to this Station. */
void Channel::add(Seed *s)
{
    if( !s ) return;

    if(	s->getBlockette53() ) {
	response.push_back(new Blockette53(*s->getBlockette53()));
    }
    else if( s->getBlockette54() ) {
	response.push_back(new Blockette54(*s->getBlockette54()));
    }
    else if( s->getBlockette55() ) {
	response.push_back(new Blockette55(*s->getBlockette55()));
    }
    else if( s->getBlockette56() ) {
	response.push_back(new Blockette56(*s->getBlockette56()));
    }
    else if( s->getBlockette57() ) {
	response.push_back(new Blockette57(*s->getBlockette57()));
    }
    else if( s->getBlockette58() ) {
	response.push_back(new Blockette58(*s->getBlockette58()));
    }
    else if( s->getBlockette59() ) {
	response.push_back(new Blockette59(*s->getBlockette59()));
    }
    else if( s->getBlockette60() ) {
	response.push_back(new Blockette60(*s->getBlockette60()));
    }
    else if( s->getBlockette61() ) {
	response.push_back(new Blockette61(*s->getBlockette61()));
    }
    else if( s->getBlockette62() ) {
	response.push_back(new Blockette62(*s->getBlockette62()));
    }
}

/** Adds a Blockette of type 51-62 to this Station. */
void Station::add(Seed *s)
{
    if( !s ) return;

    if(s->getBlockette51()) {
	b51.push_back(*s->getBlockette51());
    }
    else if(s->getBlockette52()) {
	channels.push_back(new Channel(s->getBlockette52()));
    }
    else if(s->getBlockette53() || s->getBlockette54() ||
	    s->getBlockette55() || s->getBlockette56() ||
	    s->getBlockette57() || s->getBlockette58() ||
	    s->getBlockette59() || s->getBlockette60() ||
	    s->getBlockette61() || s->getBlockette62())
    {
	if(channels.size() == 0) {
	    cerr << "Blockette" << s->getType()
		<< " found before Blockette52" << endl;
	}
	channels.back()->add(s);
    }
}
