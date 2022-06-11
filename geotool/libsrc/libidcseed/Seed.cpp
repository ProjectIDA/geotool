/** \file Seed.cpp
 *  \brief Defines the base class for all Seed objects.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <iostream>
using namespace std;
#include "seed/Seed.h"

int Seed::parseInt(const string &s, const string &name) throw(FormatException)
{
    long l;
    char *endptr;

    string t = trim(s);

    if(t.empty()) {
	return -1; // some field can be blank, 52.subchannel
    }

    l = strtol(t.c_str(), &endptr, 10);

    if( (int)(endptr - t.c_str()) != (int)t.length() ) {
	throw(FormatException(s, type, name));
    }
    return (int)l;
}

int Seed::parseInt(const string &s) throw(FormatException)
{
    long l;
    char *endptr;

    string t = trim(s);

    if(t.empty()) throw FormatException(s);

    l = strtol(t.c_str(), &endptr, 10);

    if( (int)(endptr - t.c_str()) != (int)t.length() ) {
	throw(FormatException(s));
    }
    return (int)l;
}

double Seed::parseDouble(const string &s, const string &name)
		throw(FormatException)
{
    double d;
    char *endptr;

    string t = trim(s);

    if(t.empty()) throw FormatException(s, type, name);

    d = strtod(t.c_str(), &endptr);

    if( (int)(endptr - t.c_str()) != (int)t.length() ) {
	throw FormatException(s, type, name);
    }
    return d;
}

double Seed::parseDouble(const string &s) throw(FormatException)
{
    double d;
    char *endptr;

    string t = trim(s);

    if(t.empty()) { throw FormatException(s); }

    d = strtod(t.c_str(), &endptr);

    if( (int)(endptr - t.c_str()) != (int)t.length() ) {
	throw FormatException(s);
    }
    return d;
}
