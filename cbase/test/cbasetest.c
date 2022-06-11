
/*******************************************************/
/*  Comprehensive Test Ban Treaty Organization         */
/*  Vienna                                             */
/*  Austria                                            */
/*******************************************************/

/**
 * @author    Gerald Klinkl
 *
 * $Log: cbasetest.c,v $
 *
 * Revision 1.1  2013-11-19 gerald
 * Remove stdtime_local_now() and stdtime_local_double() tests
 * as requested by HE
 *
 * Revision 1.1  2013-10-03 gerald
 * Add stdtime_local_now() and stdtime_local_double() tests
 * Add epoch to jdate function tests
 *
 */

#include <config.h>
#ifdef S_SPLINT_S
#include <netinet/in.h>  /* Need this to parse code with splint */
#endif /* S_SPLINT_S */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "libstdtime.h"


/**
 * @short Print usage message
 *
 * @description This function prints a usage message
 *
 * @param argv  Vector with CLI arguments
 * @return Nothing
 */

static void
usage(char *argv[])
{
   printf("usage: %s", argv[0]);
   exit(EXIT_FAILURE);
}

/**
 * @short Check command line arguments
 *
 * @description This function checks the command line arguments
 *
 * @param argc  Number of CLI arguments
 * @param argv  Vector with CLI arguments
 * @return Nothing
 */

static void
checkCommandLine(int argc, char *argv[])
{
   /* Check if 3 arguments are provided */
   if (argc < 1 || argc > 1) usage(argv);
}

static int
testLogLdate()
{
   int failed  = 0;
   typedef struct _ltc
   {
      double epoch;
      char *expResult;
      char *tzStr;
      int gmtOrLocal;
   } ltc_t;
   char *result;
   size_t ii;
   static ltc_t ltcTestCases[] =
   {
       { 1372600200.0, "20130630155000", "CEST", STDTIME_LOCAL_TIME },
       { 1384527600.0, "20131115160000", "CET ", STDTIME_LOCAL_TIME },
       { 1372600200.0, "20130630135000", "UTC ", STDTIME_GMT_TIME },
       { 1384527600.0, "20131115150000", "UTC ", STDTIME_GMT_TIME },
   };
   static ltc_t ltcTestCases1[] =
   {
       { 1372607400.0, "20130630 15:50:00", "CEST", STDTIME_LOCAL_TIME },
   };
   static ltc_t ltcTestCases2[] =
   {
       { 1372607400.0, "20130630155000", "UTC ", STDTIME_LOCAL_TIME },
       { 1384531200.0, "20131115160000", "UTC ", STDTIME_LOCAL_TIME },
       { 1372607400.0, "20130630155000", "UTC ", STDTIME_GMT_TIME },
       { 1384531200.0, "20131115160000", "UTC ", STDTIME_GMT_TIME },
   };
   char lddate[STDTIME_LDDATE_SIZE + 1];


   /*
   printf("result from stdtime_log_lddate with GMT is [%s]\n",
         stdtime_log_lddate(STDTIME_GMT_TIME));
   printf("result from stdtime_log_lddate with LOCAL is [%s]\n",
         stdtime_log_lddate(STDTIME_LOCAL_TIME));
   */

   /* CEST ist not a time zone, but CET is */
   if (setenv("TZ", "CET", 1))
   {
      fprintf(stderr, "Can't set timezone to CET");
      exit(1);
   }
   tzset();

   printf("\nTimezone set to CET\n");
   for (ii = 0; ii < sizeof(ltcTestCases)/sizeof(ltc_t); ii++)
   {
      /* check result with 'date -u -d@<epoch>' and 'date -d@<expResult>'
         They should return the same date and time string,
         except the timezone abbreviation */
      result = stdtime_format_gol(ltcTestCases[ii].epoch,
               "%Y\045m\045d\045H\045M\045S", ltcTestCases[ii].gmtOrLocal);
      printf("Epoch time [%ld] is %s local time: %s (%s) ...... ",
            (long)ltcTestCases[ii].epoch, ltcTestCases[ii].tzStr,
            result, ltcTestCases[ii].expResult );

      if (strncmp(result, ltcTestCases[ii].expResult,
                  strlen(ltcTestCases[ii].expResult)) == 0) printf("passed\n");
      else
      {
         printf("failed\n");
         failed = 1;
      }
   }

   /*
    * Returns always UTC!!! date
   (void) stdtime_get_lddate(lddate);
   printf("result from stdtime_get_lddate with local time is [%s]\n",
          lddate);
   */

   printf("\nBackward compatibility test\n");
   for (ii = 0; ii < sizeof(ltcTestCases1)/sizeof(ltc_t); ii++)
   {
      /* Keep in mind that stdtime_format_r() converts epoch time
         always mit gmtime_r(), which means the time zone is NOT
         considered. AND there is nothing like a "local" epoch time */
      (void) stdtime_format_r(ltcTestCases1[ii].epoch, "%L", lddate,
                             STDTIME_LDDATE_SIZE);
      printf("Epoch time [%ld] is %s date: %s (%s) ...... ",
            (long)ltcTestCases1[ii].epoch, ltcTestCases1[ii].tzStr,
            lddate, ltcTestCases1[ii].expResult );

      if (strncmp(lddate, ltcTestCases1[ii].expResult,
                  strlen(ltcTestCases1[ii].expResult)) == 0) printf("passed\n");
      else
      {
         printf("failed\n");
         failed = 1;
      }
   }

   /* Set UTC now */
   if (setenv("TZ", "UTC", 1))
   {
      fprintf(stderr, "Can't set timezone to UTC");
      exit(1);
   }
   tzset();

   printf("\nTimezone set to UTC\n");

   for (ii = 0; ii < sizeof(ltcTestCases2)/sizeof(ltc_t); ii++)
   {
      result = stdtime_format_gol(ltcTestCases2[ii].epoch,
               "%Y\045m\045d\045H\045M\045S", ltcTestCases2[ii].gmtOrLocal);
      printf("Epoch time [%ld] is %s local time: %s (%s) ...... ",
            (long)ltcTestCases2[ii].epoch, ltcTestCases2[ii].tzStr,
            result, ltcTestCases2[ii].expResult );

      if (strncmp(result, ltcTestCases2[ii].expResult,
                  strlen(ltcTestCases2[ii].expResult)) == 0) printf("passed\n");
      else
      {
         printf("failed\n");
         failed = 1;
      }
   }
   return(failed);
}

/**
 * @short Test epoch to julian date functions
 *
 * @description This function tests epoch to julian date functions
 *
 * @return 1 on failed, 0 otherwise
 */

static int
testE2J()
{
   int failed  = 0;
   typedef struct _ltc
   {
      jdate_t expResult;
      epoch_t epoch;
   } ltc_t;
   jdate_t result;
   size_t ii;
   static ltc_t ltcTestCases[] =
   {
       { 2013181, 1372600200.0 },
       { 2013319, 1384527600.0 },
   };

   for (ii = 0; ii < sizeof(ltcTestCases)/sizeof(ltc_t); ii++)
   {
       /* check result with 'date -u -d@<epoch>' and 'date -d@<expResult>'
          They should return the same date and time string,
          except the timezone abbreviation */
       result = stdtime_etoj(ltcTestCases[ii].epoch);
       printf("Epoch time [%ld] is julian date: %d ...... ",
             (long)ltcTestCases[ii].epoch, result);

      if (result == ltcTestCases[ii].expResult) printf("passed\n");
      else
      {
         printf("failed\n");
         failed = 1;
      }
   }
   return(failed);
}

/**
 * @short cbase test main function
 *
 * @description This function handles the CLI arguments and calls
 * @description the appropriate functions
 *
 * @param argc  Number of CLI arguments
 * @param argv  Vector with CLI arguments
 *
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */

int
main (int argc, char *argv[])
{
   /* Check command line argument(s) */
   (void) checkCommandLine(argc, argv);

   (void) testE2J();
   (void) testLogLdate();
   exit(EXIT_SUCCESS);
}
