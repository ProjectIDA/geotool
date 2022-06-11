#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "libtime.h"

#ifdef TEST

static void test_it(const char *s);

/**
 * @private
 */
int
main(int argc, char **argv)
{
	test_it("now");
	test_it(" 1998/12/03 12:23");
	test_it("       1997/03/12  12:23:39.7");
	test_it("2001-11-12 18:31:01");
	test_it("       2003/03/05  12:23:39");
	test_it("       2001/1/1  12:23:39.7");
	test_it("     1995/123  12:23:39.7");
	test_it("     1995/123  12:23:39.7 + 2h");
	test_it("now");
	test_it("now + 1.5d ");
	test_it("now -9d");
	test_it("now - 300");
	test_it("97464643. ");
	test_it("97464643. + 5s + 1h ");
	test_it("97464643. -7d ");
	test_it("03Jan25 12:23:39.7");
	test_it("2003Jan25 12:23:39.7");
	test_it("03Jan25");
	test_it("25-JAN-2003");

	return 0;
}

static void
test_it(const char *s)
{
	char str[100];
	double	time;
	DateTime dt;

	if(!timeParseString(s, &time)) {
	    fprintf(stderr, "timeParseString failed for: %s\n", s);
	    return;
	}
	timeEpochToDate(time, &dt);
	printf("%s   %.3f\n", s, time);
	printf("YMONDHMS: %s", timeEpochToString(time, str, 100, YMONDHMS));
	printf("   %.3f\n", timeStringToEpoch(str, YMONDHMS));
	printf("YMONDHMS2: %s", timeEpochToString(time, str, 100, YMONDHMS2));
	printf("   %.3f\n", timeStringToEpoch(str, YMONDHMS2));
	printf("YMOND: %s", timeEpochToString(time, str, 100, YMOND));
	printf("   %.3f\n", timeStringToEpoch(str, YMOND));
	printf("HMS: %s", timeEpochToString(time, str, 100, HMS));
	printf("   %.3f\n", timeStringToEpoch(str, HMS));
	printf("HMS2: %s", timeEpochToString(time, str, 100, HMS2));
	printf("   %.3f\n", timeStringToEpoch(str, HMS2));
	printf("GSE20: %s", timeEpochToString(time, str, 100, GSE20));
	printf("   %.3f\n", timeStringToEpoch(str, GSE20));
	printf("GSE21: %s", timeEpochToString(time, str, 100, GSE21));
	printf("   %.3f\n", timeStringToEpoch(str, GSE21));
	printf("MONTH_HOURS: %s", timeEpochToString(time, str,100,MONTH_HOURS));
	printf("   %.3f\n", timeStringToEpoch(str, MONTH_HOURS));
	printf("MONTH_HOURS2: %s",timeEpochToString(time, str, 100, MONTH_HOURS2));
	printf("   %.3f\n", timeStringToEpoch(str, MONTH_HOURS2));
	printf("EPOCH_DAYS: %s", timeEpochToString(time, str, 100, EPOCH_DAYS));
	printf("   %.3f\n", timeStringToEpoch(str, EPOCH_DAYS));
	printf("YMD: %s", timeEpochToString(time, str, 100, YMD));
	printf("   %.3f\n", timeStringToEpoch(str, YMD));
	printf("DHMS: %s", timeEpochToString(time, str, 100, DHMS));
	printf("   %.3f\n\n", timeStringToEpoch(str, DHMS));
	printf("HHMMSS: %s",timeEpochToString(time, str, 100, HHMMSS));
	printf("   %.3f\n", timeStringToEpoch(str, HHMMSS));
}
#endif
