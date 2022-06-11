/** \file SeedTime.h
 *  \brief Declares the SeedTime class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_TIME_H_
#define _SEED_TIME_H_

#include <sstream>
#include <string>
#include "Seed.h"
#include "FormatException.h"

using namespace std;

/**
 * This class represents a time value parsed from a SEED volume.
 * The time format is YYYY,DDD,HH,MM,SS,FFFF.
 */
class SeedTime
{
    public:
	int	year;
	int	doy;
	int	hour;
	int	minute;
	double	seconds;

	/** Create a SeedTime class initialized to 0000,001,00,00,00.0000 */
	SeedTime() : year(0), doy(1), hour(0), minute(0), seconds(0.) {}

	/** Parse SeedTime from a string.
	 *  The format is YYY,DDD,HH,MM,SS,FFFF
	 */
	SeedTime(const string &s, const string &name="") throw(FormatException);

	/** Create time from binary time format (BTIME)
	 */
	SeedTime(const string &bytes, int *so) throw(FormatException);

	/** Return a string with the time in format YYYY/DDD HH:MM:SS.SSSS */
	string str(void);

	/** Return epoch time. */
	double epoch() const;
};

#endif
