#ifndef _LIB_GSE_H_
#define	_LIB_GSE_H_

#include <stdio.h>

#include "libgODBC.h"
#include "libgcss.h"

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */

typedef struct
{
	char	sta[9];
	char	chan[9];
	char	auxid[5];
	char	instype[9];
	long	nsamp;
	double	time;
	double	endtime;
	double	samprate;
	double	calib;
	double	calper;
	double	hang;
	double	vang;
	double	lat;
	double	lon;
} GSEParam;

/* ****** gse/checksum.c ********/
void compute_checksum(int *signal_int, int number_of_samples, int *_checksum);


/* ****** gse/dcomp.c ********/
void remdif1(int *data, int npts);
int dcomp6(char *buf, int **data);
int dcomp7(char *buf, int **data);
int dcomp8(char *buf, int **data);


/* ****** gse/get_gse_waveform.c ********/

#ifdef HAVE_LIBZ
int get_waveform(gzFile fp,
#else
int get_waveform(FILE *fp,
#endif
			int msg_version, int *dt_version, GSEParam *gp,
			int **dat, int *checksum, int *err);


/* ****** gse/gse.c ********/
int gseListData(const char *read_path, int format, int max_num,
			WaveformList ***pwavlist);
int gseReadData(int nsegs, WaveformList **wav_input, double start_time,
			double end_time, int pts, bool preview_arr,
			TimeSeries *ts, Vector arrivals,
			Vector origins, Vector origerrs, Vector assocs,
			Vector stassocs, Vector wftags, Vector hydro_features,
			Vector infra_features, Vector stamags, Vector netmags,
			Vector amplitudes, Vector ampdescripts, Vector parrivals,
			const char **err_msg);
int gseRereadData(TimeSeries ts, const char **err_msg);


/* ****** gse/return_data_type.c ********/
int return_data_type(char *line, int msg_version, int *type, int *dt_version);
int return_version (char *line, int *version);

/* ****** gse/return_data_type.c ********/
#ifdef HAVE_LIBODBC
int gse2odbc(SQLHDBC hdbc, const char *file, const char *wfdiscTable,
		const char *lastidTable, const char *sitechanTable,
		const char *dir, int verbose);
#endif /* HAVE_LIBODBC */

/*
  when reading a message, this is the datatype which is returned
*/

#define END		0
#define STOP		10
#define WAVEFORM	11
#define PURIFY		12
#define CHANNEL		13
#define STATION		14
#define BULLETIN	15
#define FTP_LOG		16
#define LOG		17
#define ERROR_LOG	18
#define IRIS_MSG	19
#define NO_DATA		20
#define PARSEFINGER	21
#define CRON		22
#define AT		23
#define WAVES4U		24

#define SAMPLEPHD	30
#define BLANKPHD	31
#define DETBKPHD	32
#define CALIBPHD	33
#define QCPHD		34
#define RMSSOH		35
#define MET		36
#define ALERT		37

#define UNKNOWN		99

/*
  message and data type versions
  can also use UNKNOWN 99
*/
#define GSE_20		1
#define IMS_10		2
#define RMS_20		3
#define RMS_30		4

/*
  possible problems when parsing
*/

#define NO_PROBLEM		0
#define NO_DATA_AVAIL		1
#define WAVE_LOCK		2
#define N_NE_OBS		3
#define UNK_FORMAT		4
#define MALLOC_FAILURE		5
#define NO_DAT_LINE		6
#define NO_CHK_LINE		7
#define ERR_DECODING_WAVEFORM	8
#define DATA_DELAYED		9


#endif /* _LIB_GSE_H_ */
