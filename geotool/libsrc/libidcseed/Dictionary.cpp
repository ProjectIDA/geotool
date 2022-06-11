/** \file Dictionary.cpp
 *  \brief Defines a class for holding dictionary blockettes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <vector>
#include "seed/Dictionary.h"

void Dictionary::clear()
{
    int i;
    for(i = 0; i < (int)b30.size(); i++) delete b30[i];
    for(i = 0; i < (int)b31.size(); i++) delete b31[i];
    for(i = 0; i < (int)b32.size(); i++) delete b32[i];
    for(i = 0; i < (int)b33.size(); i++) delete b33[i];
    for(i = 0; i < (int)b34.size(); i++) delete b34[i];
    for(i = 0; i < (int)b41.size(); i++) delete b41[i];
    for(i = 0; i < (int)b42.size(); i++) delete b42[i];
    for(i = 0; i < (int)b43.size(); i++) delete b43[i];
    for(i = 0; i < (int)b44.size(); i++) delete b44[i];
    for(i = 0; i < (int)b45.size(); i++) delete b45[i];
    for(i = 0; i < (int)b46.size(); i++) delete b46[i];
    for(i = 0; i < (int)b47.size(); i++) delete b47[i];
    for(i = 0; i < (int)b48.size(); i++) delete b48[i];
}

/** Adds a DictionaryBlockette (30-34, 41-48) */
void Dictionary::add(DictionaryBlockette *s)
{
    if(	s->getBlockette30() ) {
	b30.push_back(new Blockette30(*s->getBlockette30()));
    }
    else if( s->getBlockette31() ) {
	b31.push_back(new Blockette31(*s->getBlockette31()));
    }
    else if( s->getBlockette32() ) {
	b32.push_back(new Blockette32(*s->getBlockette32()));
    }
    else if( s->getBlockette33() ) {
	b33.push_back(new Blockette33(*s->getBlockette33()));
    }
    else if( s->getBlockette34() ) {
	b34.push_back(new Blockette34(*s->getBlockette34()));
    }
    else if( s->getBlockette41() ) {
	b41.push_back(new Blockette41(*s->getBlockette41()));
    }
    else if( s->getBlockette42() ) {
	b42.push_back(new Blockette42(*s->getBlockette42()));
    }
    else if( s->getBlockette43() ) {
	b43.push_back(new Blockette43(*s->getBlockette43()));
    }
    else if( s->getBlockette44() ) {
	b44.push_back(new Blockette44(*s->getBlockette44()));
    }
    else if( s->getBlockette45() ) {
	b45.push_back(new Blockette45(*s->getBlockette45()));
    }
    else if( s->getBlockette46() ) {
	b46.push_back(new Blockette46(*s->getBlockette46()));
    }
    else if( s->getBlockette47() ) {
	b47.push_back(new Blockette47(*s->getBlockette47()));
    }
    else if( s->getBlockette48() ) {
	b48.push_back(new Blockette48(*s->getBlockette48()));
    }
}

Blockette30 * Dictionary::getB30(int lookup_code)
{
    for(int i = 0; i < (int)b30.size(); i++) {
	if(b30[i]->lookup_code == lookup_code) return b30[i];
    }
    return NULL;
}
Blockette31 * Dictionary::getB31(int lookup_code)
{
    for(int i = 0; i < (int)b31.size(); i++) {
	if(b31[i]->lookup_code == lookup_code) return b31[i];
    }
    return NULL;
}
Blockette32 * Dictionary::getB32(int lookup_code)
{
    for(int i = 0; i < (int)b32.size(); i++) {
	if(b32[i]->lookup_code == lookup_code) return b32[i];
    }
    return NULL;
}
Blockette33 * Dictionary::getB33(int lookup_code)
{
    for(int i = 0; i < (int)b33.size(); i++) {
	if(b33[i]->lookup_code == lookup_code) return b33[i];
    }
    return NULL;
}
Blockette34 * Dictionary::getB34(int lookup_code)
{
    for(int i = 0; i < (int)b34.size(); i++) {
	if(b34[i]->lookup_code == lookup_code) return b34[i];
    }
    return NULL;
}
Blockette41 * Dictionary::getB41(int lookup_code)
{
    for(int i = 0; i < (int)b41.size(); i++) {
	if(b41[i]->lookup_code == lookup_code) return b41[i];
    }
    return NULL;
}
Blockette42 * Dictionary::getB42(int lookup_code)
{
    for(int i = 0; i < (int)b42.size(); i++) {
	if(b42[i]->lookup_code == lookup_code) return b42[i];
    }
    return NULL;
}
Blockette43 * Dictionary::getB43(int lookup_code)
{
    for(int i = 0; i < (int)b43.size(); i++) {
	if(b43[i]->lookup_code == lookup_code) return b43[i];
    }
    return NULL;
}
Blockette44 * Dictionary::getB44(int lookup_code)
{
    for(int i = 0; i < (int)b44.size(); i++) {
	if(b44[i]->lookup_code == lookup_code) return b44[i];
    }
    return NULL;
}
Blockette45 * Dictionary::getB45(int lookup_code)
{
    for(int i = 0; i < (int)b45.size(); i++) {
	if(b45[i]->lookup_code == lookup_code) return b45[i];
    }
    return NULL;
}
Blockette46 * Dictionary::getB46(int lookup_code)
{
    for(int i = 0; i < (int)b46.size(); i++) {
	if(b46[i]->lookup_code == lookup_code) return b46[i];
    }
    return NULL;
}
Blockette47 * Dictionary::getB47(int lookup_code)
{
    for(int i = 0; i < (int)b47.size(); i++) {
	if(b47[i]->lookup_code == lookup_code) return b47[i];
    }
    return NULL;
}
Blockette48 * Dictionary::getB48(int lookup_code)
{
    for(int i = 0; i < (int)b48.size(); i++) {
	if(b48[i]->lookup_code == lookup_code) return b48[i];
    }
    return NULL;
}
