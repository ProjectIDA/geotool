#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <time.h>

#include "libtime.h"


#ifdef TEST
/**
 * @private
 */
int
main(int argc, char **argv)
{
//	char *string_time = "2004Jul01 18:03:59.61050";
//	char *string_time = "19941227 09:22:53";
//	char *string_time = "03-01-25 13:43";
//	char *string_time = "03-01-85 13:43";
	char *string_time = "22-AUG-2005";
	double	time;
	DateTime dt;
char *c;
struct tm tm;

	if(!timeParseString(string_time, &time)) {
//	    fprintf(stderr, "timeParseString failed for %s.\n", string_time);
//	    return 0;
	}

//if(!(c = strptime(string_time, "%Y%m%d%n%H:%M:%S", &tm))) {
if(!(c = strptime(string_time, "%y-%m-%d%n%H:%M", &tm))) { /* 03-01-25 hh:mm */
printf("strptime failed at:%s\n", c);
}
else if(*c != '\0') {
printf("extra characters: %s\n", c);
}

	printf("time: %s\n", string_time);

	printf("time: %.5lf\n", time);

	timeEpochToDate(time, &dt);

	printf("year: %d\n", dt.year);
	printf("month: %d\n", dt.month);
	printf("day: %.d\n", dt.day);
	printf("seconds: %.5lf\n", dt.second);

	return 0;
}
#endif
