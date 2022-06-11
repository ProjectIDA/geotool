#include "libstanoise.h"

typedef struct
{
  char   *out_dir;
  char   *station_name;
  char   *qc_corrected;
  char   *resp_out;
  char  **chan_list;
  int     nchan;
  double  duration;
  double  epochtime;
}SS_INPUT_PARAMETERS;

typedef struct
{
  int     endian;
  int    *samples;
  double *sample_rate;
  double  start_time;
  double  end_time;
  char   *station_name;
  char  **sensor;
  char  **chan;
  char   *shi_type;
  int     nsensors;
  char   *name;
  char   *dfile;
  char   *dir;
  char   *out_dir;
}SS_STRUCT;

int CheckLib_standardsensor();

#ifdef HAVE_LIBODBC
int GetInid(SQLHDBC hdbc, char *resp_file,double start_time,double end_time);
#endif
