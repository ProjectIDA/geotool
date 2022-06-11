#include "qctool.h"
#include "libgparseP.h"
#include "libgODBC.h"
#include <time.h>
#include <unistd.h>

#define MAX_SUMM               (10000)

typedef struct
{
  double **psd_log;
  double **psd_log_std;
  double **freq;
  int    *samples;
  double *sample_rate;
  double  start_time;
  double  end_time;
  logical *good_sensor;
  char   *station_name;
  char  **sensor;
  char  **chan;
  int     nsensors;
  char   *psd_key;
  char   *shi_type;
  int     psd_id;
  char   *name;
  char   *dir;
  char   *out_dir;
  char   *dfile;
  double  lat;
  double  lon;
  int    *iterations;
  char   *window_type;
  double  enbw;
  double  main_window_length;
  double *sub_window_length;
  double  rov;
  int     *decon;
  int     endian;
  char   **decon_file;
  char   **recon_file;
  char   *local_day;
  char   *local_time;
  char   *units;
  double *calib;
  double *calper;
  char   *smoothing;
}PSD_STRUCT;

typedef struct
{
  char                   *sensor_name;
  char                   *chan;
  int                     f_bins;
  int                     p_bins;
  double                  *log_freq;
  double                  *log_psd;
  double                  *total_past;
  double                **psd_prob;
  logical                 good_sensor;
  int                     iterations;
  int                     decon;
  double                  sub_window_length;
}PSD_PROB_SENSOR_STRUCT;

typedef struct
{
  char                    *station_name;
  char                    *shi_type;
  char                    *window_type;
  double                   start_time;
  double                   end_time;
  double                   enbw;
  double                   main_window_length;
  int                      nsensors;
  int                      endian;
  double                   rov;
  PSD_PROB_SENSOR_STRUCT  *psd_prob_sensor;
}PSD_PROB_STRUCT;

typedef struct
{
  char  *in_dir;
  char  *sta;
  char  *job;
  char  *username;
  char  *passwd;
  char  *source;
  double start_time;
  int    number_of_intervals;
  double interval_length;
  double end_time;
  double main_window_length;
} SN_STATE_PARS;


typedef struct
{
  double *freq;
  double *dP;
  double *time;
  char   *chan;
  char   *sensor;
  int    *samples;
  int     j_sensor;
} SUMM_DAT;

typedef struct
{
  char *psd_file;
  int nsensors;
  int jdate;
  int hour;
  int min;
  int sec;
  double start_time;
  double end_time;
  SUMM_DAT *summ_dat;
}SUMM;

typedef struct
{
  char **sensor;
  char **chan;
} SC;

int         BigOrLittleEndian();
double     *CalculatePSD(complex *x1_t,int sample_points,double coh_power_gain,double incoh_power_gain,double enbw,double sample_rate);
int         CheckLib_stanoise();;
PSD_STRUCT *CreatePSDStruct(int nsensors);
logical     ContainsString(char *string1,char *string2);
char       *CopyAfterEqual(char *target,  char *source);
PSD_STRUCT *CopyPSDStruct(PSD_STRUCT *psd_struct_in);
char       *EpochtimeToJdate(double epochtime);
void        FreePSDStruct(PSD_STRUCT *psd_struct);
void        FreeStation(STATION *station,int nchan);
void        FreeWavedata(WAVEDATA *wavedata);
char      **GenerateYearDay_list(double start_jdate,double end_jdate,int number_of_days);
char      **GetChannels(char *chan_list,int *nchan);
complex   **GetDataBlock(double start_time,double end_time,WAVEDATA *wavedata);
WAVEDATA   *GetDotwAndWfdiscData(GeotoolStruct *gs,INPUT_PARAMETERS *ip, STATION *station,char *shi_type);
double      GetMaxSampleRate(PSD_STRUCT *psd_struct);
int         GetMaxSamples(PSD_STRUCT *psd_struct);
double     *GetNoDiffSampleRates(PSD_STRUCT *psd_struct);
char       *GetOutFileName(INPUT_PARAMETERS *ip,STATION *station);
void        GetSensorFromDataBlockTime(complex **a,complex *x,int i_sensor,int sample_points);
STATE_INFO *GetStateStaNoise(char *dir_filename,long *type_lookup,SN_STATE_PARS *sn_state_pars);
STATION    *GetStationInfo(GParseEnv gpe,INPUT_PARAMETERS *ip);
int         IsRecorded(STATION *station,char *sitechan_sta,char *sitechan_chan,int i_sensor);
double      JdateToEpoch(char *jdate_hour);
double      Maximum( double x, double y);
int         Maximum_int(int x,int y);
double      Minimum( double x, double y);
int         Minimum_int(int x,int y);
long        Minimum_long( long x, long y);
int         MkDir(char *dir);
long        NearestInteger(double x);
int         PlotProb(PSD_STRUCT *psd_struct,PSD_STRUCT *psd_struct_smooth, int *label, int *ordinal);
void        PsdProb(PSD_STRUCT *psd_struct);
PSD_STRUCT *ReadPSDData(char *in_name);
PSD_PROB_STRUCT *ReadPsdProb(char *psd_name);
void        RemoveTrend( complex y[], int n );
void        RunningMeanVarStd(double x, double *mean, double *var, double *std, int *m_n, double *m_oldM, double *m_oldS);
PSD_STRUCT *SanitizedPSDlog(PSD_STRUCT *psd_struct_in);
void        SavitzkyGolay(double **A,int n,int m,logical *good_sensor);
#ifdef HAVE_LIBODBC
int         GetChanid(SQLHDBC hdbc, char *sta, char *chan);
int        *SetState(SQLHDBC hdbc,double duration,double interval_length,double interval_start_time,
              int    number_of_intervals,
              long  *type_lookup,
              char  *sta,
              int   *state,
              int    nsensors,
              char **sensor,
              char **chan,
              char *idcx_account);
int         UpdateState(SQLHDBC hdbc,STATE_INFO *state_info,double duration,double interval_length,char *sta,long *type_lookup,char *idcx_account);
logical     UseDay(SQLHDBC hdbc,char *filename,char *sta,Vector vqcdata,long *type_lookup,char *idcx_account);
int         WriteWfdiscData(SQLHDBC hdbc,char *filename,WAVEDATA *wavedata);
#endif
char       *SHIStationType(char *station_name);
void        UpdateAveragePSD(PSD_STRUCT *psd_struct);
void        UpdateDayNightAveragePSD(PSD_STRUCT *psd_struct);
void        UpdateHighNoiseModel(PSD_STRUCT *psd_struct);
void        UpdateLowNoiseModel(PSD_STRUCT *psd_struct);
logical     UseFile(char *filename,char *station_name, Vector vqcdata,double start_time,double end_time,int offset,char *idcx_account);
void        Window_sn (complex x[], int n,char *window_type,double *coh_power_gain,double *incoh_power_gain,double *enbw,double *rov,double *ratio50);
void        WritePSDData(PSD_STRUCT *psd_struct);
int         WritePSD_NetCDF4(PSD_STRUCT *psd_struct);
void        check_err(const int stat, const int line, const char *file);
void        ZeroVector( complex x[], int n);
logical     file_exists (char * filename);
