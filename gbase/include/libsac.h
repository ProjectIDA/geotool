/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_SAC_H_
#define	_LIB_SAC_H_

#include <stdio.h>

#include "sac.h"

#include "libgODBC.h"
#include "libgcss.h"

/* ****** sac.c ********/
int sacListData(const char *read_path, int format, int max_num,
			WaveformList ***pwavlist);
int sacReadData(int nsegs, WaveformList **wav_input, double start_time,
			double end_time, int pts, bool preview_arr,
			TimeSeries *ts, Vector arrivals,
			Vector origins, Vector origerrs, Vector assocs,
			Vector stassocs, Vector wftags, Vector hydro_features,
			Vector infra_features, Vector stamags, Vector netmags,
			Vector amplitudes, Vector ampdescripts, Vector parrivals,
			const char **err_msg);
int sacRereadData(TimeSeries ts, const char **err_msg);
int sacArrivalAdd(CssArrival arr, TimeSeries ts, Password password);
int sacArrivalDelete(int n_arrivals, CssArrival *arrivals,
			int iarr, int n_assocs, CssAssoc *assocs);
int sacTableList(int num_tables, CssTable *tables, int num_extra,
			CssTable *extra, int num_attributes,
			Attribute **attributes, char **row, const char *rw,
			bool reopen);
int sacArrivalChange(CssArrival arr, TimeSeries ts, int mask);
int sacWriteCD(char *prefix, char *wa, int num_waveforms,
			CPlotData **cd_list, char *remark, int raw);
int read_ascii_sac_header(FILE *fp, SAC *s, int *lineno, const char **err_msg);
void sacFlipHeader(SAC *sac);
int sacReadHeader(FILE *fp, SAC *sac, int *binary, int *lineno,
			const char **err_msg);
int sacReadWaveform(FILE *fp, int binary, float *y, int *npts, SAC *sac);

/* ****** sac2odbc.c ********/
#ifdef HAVE_LIBODBC
int sac2odbc(SQLHDBC hdbc, const char *file, const char *wfdiscTable,
			const char *lastidTable, const char *sitechanTable,
			const char *dir, int verbose);
#endif /* HAVE_LIBODBC */


#endif /* _LIB_SAC_H_ */
