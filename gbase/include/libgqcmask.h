#include "libstanoise.h"
#include "libdataqc.h"

typedef struct
{
  char  *out_dir;
  char  *qcpar;
  char  *qcfix;
  char  *station_name;
  char  **chan_list;
  double epochtime;
  int   nchan;
  int   qcdefid;
  double duration;
}QC_INPUT_PARAMETERS;

typedef struct
{
  int    perform_qc;
  double max_mask_fraction;
  char  *type_multi_component;
  int    min_multi_component;
  int    apply_extended;
  int    demean;
  int    fix;
  int    prefilter;
  double fhigh;
  int    gap_samples;
  double gap_taper_fraction;
  int    interval_samples;
  double interval_overlap_fraction;
  int    niter;
  double single_trace_spike_thresh;
  double spike_thresh;
  char  *spike_statistic;
  double spike_statistic_value;
  char  *spike_dataset;
  int    spike_window_samples;
  int    qcdefid;
  int    ondate;
  int    offdate;
} QCREC;

typedef struct
{
  double  start_time;
  double  end_time;
  int    *samples;
  double *sample_rate;
  char   *station_name;
  char  **sensor;
  char  **chan;
  int     nsensors;
  char   *shi_type;
  char   *name;
  char   *dir;
  char   *out_dir;
  char   *dfile;
  double  lat;
  double  lon;
  int     endian;
  QCREC  *qcrec;
  QCMask *qcmask;
}QCMASK_STRUCT;

int     CheckLib_gqcmask();
void    FreeQCMaskStruct(QCMASK_STRUCT *qcmask_struct);
QCDef  *GetQCDef(QCREC *qcrec);
QCMask *GetQCMask(WAVEDATA *wavedata,QCDef *qcdef);
STATE_INFO *GetStateQCMask(char *dir_filename,long *type_lookup);
int     QcFix(QCMask *qcmask,WAVEDATA *wavedata);
QCMASK_STRUCT *ReadQCMaskStructData(char *filename);
void    WriteQCMaskStructData(QCMASK_STRUCT *qcmask_struct,char *filename);

#ifdef HAVE_LIBODBC
QCREC  *GetQCREC(SQLHDBC hdbc,INPUT_PARAMETERS *ip);
int     UpdateQCtables(SQLHDBC hdbc,QCMASK_STRUCT *qcmask_struct);
#endif
