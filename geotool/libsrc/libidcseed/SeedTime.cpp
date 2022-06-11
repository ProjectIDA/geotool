/** \file SeedTime.cpp
 *  \brief Defines a class for Seed time fields.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

#include "seed/SeedTime.h"
#include "seed/Seed.h"
#include "seed/ByteOrder.h"

/**
 * This class represents a time value parsed from a SEED volume.
 * The time format is YYYY,DDD,HH,MM,SS,FFFF.
 */

/** Parse SeedTime from a string.
 *  The format is YYYY,DDD,HH,MM,SS,FFFF
 */
SeedTime::SeedTime(const string &s, const string &name) throw(FormatException)
{
    int n;

    year = 0; doy = 0; hour = 0; minute = 0; seconds = 0.;

    if((n = s.length()) == 0) return; // station offtime can be empty

    if(n < 4) {
	throw FormatException("short time string: " + name);
    }
    try {
	year = Seed::parseInt(s.substr(0, 4));
	if(n >= 8) {
	    doy = Seed::parseInt(s.substr(5, 3));
	}
	if(n >= 11) {
	    hour = Seed::parseInt(s.substr(9, 2));
	}
	if(n >= 14) {
	    minute = Seed::parseInt(s.substr(12, 2));
	}
	if(n >= 17) {
	    // allow ',' intead of '.'
	    string sec = s.substr(15);
	    if(sec[2] == '.') {
		sec = sec.substr(0, 2) + "." + sec.substr(3);
	    }
	    seconds = Seed::parseDouble(sec);
	}
    }
    catch ( FormatException &e ) {
	throw FormatException("time format error: " + name);
    }
}

SeedTime::SeedTime(const string &bytes, int *so) throw(FormatException)
{
    int i;
    UWORD w;

    year = 0; doy = 0; hour = 0; minute = 0; seconds = 0.;

    if((int)bytes.length() < 10) {
	throw FormatException("short time string.");
    }
    for(i = 0; i < 2; i++) w.a[so[i]] = bytes[i];
    year = w.s;

    for(i = 0; i < 2; i++) w.a[so[i]] = bytes[2+i];
    doy = w.s;

    hour = (int)((unsigned char )bytes[4]);
    minute = (int)((unsigned char )bytes[5]);
    seconds = (double)bytes[6];
    
    for(i = 0; i < 2; i++) w.a[so[i]] = bytes[8+i];
    seconds += (double)w.s/10000.;
}

/** Return a string with the time in format YYYY/DDD HH:MM:SS.SSSS
 */
string SeedTime::str()
{
    char s[50];

    snprintf(s, sizeof(s), "%04d/%03d %02d:%02d:%7.4lf",
		year, doy, hour, minute, seconds);

    return string(s);
}

#define isLeapYear(yr) ((yr%4)==0 && ((yr%100)!=0 || (yr%400)==0))

/** Return epoch time.
 */
double SeedTime::epoch(void) const
{
    int yr = year;
    double days = 0.;

    if(yr > 1970) {
	while(--yr >= 1970) days += isLeapYear(yr) ? 366 : 365;
    }
    else if(yr < 1970) {
	while(yr < 1970) {
	    days -= isLeapYear(yr) ? 366 : 365;
	    yr++;
	}
    }
    return (days + doy-1)*86400. + 3600.*hour + 60.*minute + seconds;
}
