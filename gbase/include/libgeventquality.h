#include "libstanoise.h"

typedef struct
{
  double a;
  double b;
  long   type;
  int    points;
  char  *plot_type;
} EVDATA;

typedef struct
{
  double  start_time;
  double  end_time;
  char   *evquality_key;
  char   *station_name;
  char   *shi_type;
  char   *account;
  int     nsensors;
  char   *name;
  char   *dir;
  char   *out_dir;
  char   *dfile;
  char   *data_source;
  EVDATA *evdata;
}EVQUALITY_STRUCT;

typedef struct
{
  int    arid;
  char  *chan;
} ARRIVAL_FIELDS;

typedef struct
{
  double magres;
  int    orid;
  int    magid;
  int    arid;
  char  *magtype;
  char  *sta;
  char  *phase;
} STAMAG_FIELDS;

typedef struct
{
  int    magid;
  int    nsta;
  double magnitude;
  int    orid;
} NETMAG_FIELDS;

typedef struct
{
  double azres;
  double seaz;
  int    orid;
  int    arid;
} ASSOC_FIELDS;

typedef struct
{
  int    orid;
  double time;
} ORIGIN_FIELDS;

int CheckLib_gevquality();
EVDATA *DoQueries(GParseEnv gpe);
void FreeEVQUALITYStruct(EVQUALITY_STRUCT *evquality_struct);
STATE_INFO *GetStateEvQuality(char *dir_filename,long *type_lookup);
EVQUALITY_STRUCT *ReadEvQuality(char *filename);
void  WriteEvQuality(EVQUALITY_STRUCT *evquality_struct,char *filename);
