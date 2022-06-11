#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "libstring.h"

#define PII 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679
#define MAX_CHARS 1024
#define Free_sn(ptr) do \
                  { \
                     if (ptr)\
                     {(void) free ((char *) (ptr));\
                                     (ptr) = NULL;\
                     } \
                  } while (0)

#define EPSILON_LONG_DOUBLE        1.0E-16
#define EPSILON_DOUBLE             1.0E-7
#define EPSILON_FLOAT              1.0E-4
#define EPSILON_LARGE              1.0E-2
#define LARGE_NEGATIVE_INT     -999999999
#define LARGE_POSITIVE_INT      999999999
#define LARGE_NEGATIVE_DOUBLE  -9999999999.0
#define LARGE_POSITIVE_DOUBLE   9999999999.0
#define LARGE_NEGATIVE_FLOAT   -9999999999.0
#define LARGE_POSITIVE_FLOAT    9999999999.0
#define DEGREES_PER_HOUR        15.0
#define KM_PER_DEGREE                 111.194926
#define M_PER_KM                     1000.0
#define SECONDS_PER_DAY             86400.0
#define SECONDS_PER_DAY_long        86400L
#define SECONDS_PER_YEAR_long       (SECONDS_PER_DAY_long*365L)
#define SECONDS_PER_LEAP_YEAR_long  (SECONDS_PER_DAY_long*366L)
#define EARTH_RADIUS               6371
#define RAD_PER_DEGREE             (PII/180.0)
#define DEGREE_PER_RAD             (180.0/PII)
#define NM_TO_M_LOG              9.0
#define INFRA_UNIT_CONVERT       0.0
#define HYDRO_UNIT_CONVERT       0.0
#define FIVE_PERCENTILE         1.645
#define TEN_PERCENTILE          1.282
#define PERCENT_5                (5.0)
#define PERCENT_95              (95.0)
#define PERCENT_10              (10.0)
#define PERCENT_90              (90.0)
#define mod(i,j) (i-((i)/(j))*(j))
#define JANUARY     (jday<32)
#define FEBRUARY  ( (jday>=32)  && (jday<60) )
#define MARCH     ( (jday>=60)  && (jday<91) )
#define APRIL     ( (jday>=91)  && (jday<121) )
#define MAY       ( (jday>=121) && (jday<152) )
#define JUNE      ( (jday>=152) && (jday<182) )
#define JULY      ( (jday>=182) && (jday<213) )
#define AUGUST    ( (jday>=213) && (jday<244) )
#define SEPTEMBER ( (jday>=244) && (jday<274) )
#define OCTOBER   ( (jday>=274) && (jday<305) )
#define NOVEMBER  ( (jday>=305) && (jday<335) )
#define DECEMBER  ( (jday>=335) )
#define STA_LEN                 7
#define CHAN_LEN                9
#define INS_LEN                 7
#define SEG_LEN                 2
#define TYP_LEN                 3
#define DIR_LEN                65
#define FILE_LEN               33
#define CLIP_LEN                2
#define MAX_SENSORS_PER_STATION 100
#define FREE_ARG                char*
#define NP   11
#define NL    5
#define NR    5
#define ORDER 6
#define BINS_PER_OCTAVE         8
#define SAN_BINS                100
#define SMOOTH_F                (BINS_PER_OCTAVE)
#define SMOOTH_P                (3)
#define A4_WIDTH                (21.0)
#define A4_HEIGHT               (29.7)
#define XOR            4.0
#define YOR            7.0
#define PH            10.0
#define PL            10.0
#define PLOT_F0        0.01
#define PLOT_F1        7.0
#define PLOT_LOG_F0  (log10(PLOT_F0))
#define PLOT_LOG_F1  (log10(PLOT_F1))
#define SEIS_PLOT_DB_P0 -200.0
#define SEIS_PLOT_DB_P1  -50.0
#define INFRA_PLOT_LOG_P0 -8.0
#define INFRA_PLOT_LOG_P1  2.0
#define HYDRO_PLOT_LOG_P0  40.0
#define HYDRO_PLOT_LOG_P1 190.0
#define HYDRO_ABSURD_P0 (HYDRO_PLOT_LOG_P0 - 9.0)
#define HYDRO_ABSURD_P1 (HYDRO_PLOT_LOG_P1 + 9.0)
#define INFRA_ABSURD_P0 (INFRA_PLOT_LOG_P0 - 4.0)
#define INFRA_ABSURD_P1 (INFRA_PLOT_LOG_P1 + 4.0)
#define SEIS_ABSURD_P0 (SEIS_PLOT_DB_P0 - 100.0)
#define SEIS_ABSURD_P1 (SEIS_PLOT_DB_P1 + 50.0)
#define FREE_ARG                char*
#define MAX_WFDISC_ROWS        1000
#define NM_TO_M_LOG             9.0

typedef enum {FALSE_sn,TRUE_sn} logical;

typedef struct
{
   double re;
   double im;
} complex;

typedef struct
{
  char   *station_name;
  double  lat;
  double  lon;
  char  **sta_list;
  char  **chan_list;
  char  **decon_file;
  char  **recon_file;
  char  **instype;
  int    *inid;
  int    *chanid;
  int     nsensors;
  double *calib;
  double *calper;
} STATION;

typedef struct
{
  double epochtime;
  double desired_sub_window_length;
  double *sub_window_length;
  double main_window_length;
  double lo_freq;
  double hi_freq;
  char  *qcpar;
  char  *qc_out_dir;
  char  *qcfix;
  char  *window_type;
  char  *out_dir;
  char  *station_name;
  char  *do_plot;
  char  *record;
  char  **chan_list;
  char  *convolve_resp_par;
  int   nchan;
  int   recorded;
  int   decon;
  int   deconvolve;
  int   qcdefid;
} INPUT_PARAMETERS;

typedef struct
{
  char *sta;
  char *chan;
  int   chanid;
} SITECHAN_FIELDS;

typedef struct
{
  int  num;
  char *sta;
  char *net;
  char *type;
} AFFILIATION_FIELDS;

typedef struct
{
  int inid;
  int chanid;
} SENSOR_FIELDS;

typedef struct
{
  char *dir;
  char *dfile;
  char *instype;
  int   inid;
} INSTRUMENT_FIELDS;

typedef struct
{
  char *sta;
  char *chan;
  double time;
  int wfid;
  int chanid;
  int jdate;
  double endtime;
  int nsamp;
  float samprate;
  float calib;
  float calper;
  char *instype;
  char *segtype;
  char *datatype;
  char *clip;
  char *dir;
  char *dfile;
  int foff;
  int commid;
} WFDISC_TABLE;

typedef struct
{
  complex     **dotw;
  complex     **response;
  WFDISC_TABLE *wfdisc;
  int           w_cols;
  int           w_rows;
  int           nsensors;
  double        data_start_time;
  double        data_end_time;
  long         *data_samples;
  double        signal_start_time;
  double        signal_end_time;
  long          signal_samples;
  long          signal_offset;
  double       *sample_rate;
  int          *data_present_flag;
  int          *decon;
} WAVEDATA;

typedef struct
{
  int            nsensors;
  char         **sensor;
  char         **chan;
  double         start_time;
  double         end_time;
  int            number_of_cells;
  long          *types;
  int           *state;
}STATE_INFO;

AFFILIATION_FIELDS *affiliation_fields;
int max_types;
double cell_length;
FILE *logfile_ptr;
