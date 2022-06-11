/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_ASCII_H_
#define	_LIB_ASCII_H_

#include "libgcss.h"
#include "gobject/TimeSeries.h"

/* ****** ascii.c ********/
int asciiListData(const char *read_path, int format, int max_num,
			WaveformList ***pwavlist);
int asciiReadData(int nsegs, WaveformList **wav_input, double start_time,
			double end_time, int pts, bool preview_arr,
			TimeSeries *ts, Vector arrivals,
			Vector origins, Vector origerrs, Vector assocs,
			Vector stassocs, Vector wftags, Vector hydro_features,
			Vector infra_features, Vector stamags, Vector netmags,
			Vector amplitudes, Vector ampdescripts, Vector parrivals,
			const char **err_msg);
int asciiRereadData(TimeSeries ts, const char **err_msg);
int asciiWriteCD(const char *prefix, const char *wa, int num_waveforms,
			CPlotData **cd_list, const char *remark, int raw);

#endif /* _LIB_ASCII_H_ */
