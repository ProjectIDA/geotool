/** \file WaveformTag.cpp
 *  \brief Defines class WaveformTag.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>

#include "gobject++/WaveformTag.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
} 

WaveformTag::WaveformTag(const WaveformTag &w) : members(), ud_string()
{
    members = w.members;
    ud_string = w.ud_string;
}

WaveformTag & WaveformTag::operator=(const WaveformTag &w)
{
    members = w.members;
    ud_string = w.ud_string;
    return *this;
}

void WaveformTag::setMembers(int num, const int *tag_members, string tag_string)
{
    members.clear();
    for(int i = 0; i < num; i++) {
	members.push_back(tag_members[i]);
    }
    ud_string.clear();

    if( !tag_string.empty() ) {
	for(int i = 0; i < (int)members.size(); i++)
	{
	    if(members[i] == 0) {
		ud_string = tag_string;
		break;
	    }
	}
    }

    /* printf("DEBUG/WaveformTag: %s\n", ud_string.c_str()); */
}
