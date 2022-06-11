/** \file WaveformConstraint.cpp
 *  \brief Defines class WaveformConstraint.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include "gobject++/SegmentInfo.h"
#include "WaveformConstraint.h"
#include "Iaspei.h"
#include "Waveform.h"
#include "gobject++/DataSource.h"

extern "C" {
#include "libstring.h"
#include "libtime.h"
static int sortByTSC(const void *A, const void *B);
static int sortBySC(const void *A, const void *B);
static int sortByC(const void *A, const void *B);
}


using namespace std;

WaveInput::WaveInput(double start_time, double end_time, double t0_time)
{
    start = start_time;
    end = end_time;
    t0 = t0_time;
}

WaveInput::~WaveInput(void)
{
}

WaveformConstraint::WaveformConstraint()
{
    overlap_limit = 30.;
    join_time_limit = 7200.;
    start_time = NULL_TIME;
    end_time = NULL_TIME;
    window_length = NULL_TIME;
    max_length = -1.;
    azCon = false;
    minAz = -999.;
    maxAz = -999.;
    memset((void *)startPhase, 0, sizeof(startPhase));
    memset((void *)endPhase, 0, sizeof(endPhase));
    start_phase_edge = 10.;
    end_phase_edge = 10.;
    waveform_limit = 0;

// put this memset in stringcpy
    memset((void *)chan_sort_order0, 0, sizeof(chan_sort_order0));
    memset((void *)chan_sort_order1, 0, sizeof(chan_sort_order1));
    memset((void *)chan_sort_order2, 0, sizeof(chan_sort_order2));
    stringcpy(chan_sort_order0, "bBeEsSlLmMhHuUvV", sizeof(chan_sort_order0));
    stringcpy(chan_sort_order1, "hHpPlL", sizeof(chan_sort_order1));
    stringcpy(chan_sort_order2, "bBzZnNeErRtTdDxXlL", sizeof(chan_sort_order2));
}

WaveformConstraint::~WaveformConstraint(void)
{
}

vector<WaveInput *> * WaveformConstraint::getTimeLimits(int num_selected,
		SegmentInfo **wav, vector<ArrivalInfo> &arr_info)
{
    int		i, j, k, n, nsegs;
    double	local_start_time, local_end_time, start, end, window;
    SegmentInfo **wav_sort, **wav_input;

    if(num_selected <= 0) return NULL;

    if(!(wav_input = (SegmentInfo **)mallocWarn(
			num_selected*sizeof(SegmentInfo *))))
    {
	return NULL;
    }
    if(!(wav_sort = (SegmentInfo **)mallocWarn(
			num_selected*sizeof(SegmentInfo *)))) {
	Free(wav_input);
	return NULL;
    }

    vector<WaveInput *> *wav_inputs = new vector<WaveInput *>;

    /* Each time only increment if wav[i] was used. The sort by time
     * of all same sta/chan can cause i+1 to be used before i, but
     * eventually i will be used.
     */
    for(i = 0; i < num_selected; i += !wav[i] ? 1 : 0) if(wav[i] != NULL)
    {
	if(azCon && !checkAz(wav[i], minAz, maxAz)) continue;

	/* collect all wavs with same sta/chan */
	n = 1;
	wav_sort[0] = wav[i];
	wav_sort[0]->wc = this;
	for(j = i+1; j < num_selected; j++)
	{
	    if(wav[j] != NULL && !strcmp(wav_sort[0]->sta,  wav[j]->sta)
			&& !strcmp(wav_sort[0]->chan, wav[j]->chan))
	    {
		    if(azCon && checkAz(wav[j], minAz, maxAz))
			continue;	
		    wav_sort[n] = wav[j];
		    wav_sort[n++]->wc = this;
	    }
	}
	
	/* sort these segments by time */
	qsort(wav_sort, n, sizeof(SegmentInfo *), sortByTSC);

	wav_input[0] = wav_sort[0];
	local_start_time = wav_input[0]->start;
	local_end_time = wav_input[0]->end;

	/* check if the overlaps and/or gaps allow these n wavs to be
	 * plotted as one.
	 */
	nsegs = 1;
	for(j = 1; j < n; j++)
	{
	    double gap = wav_sort[j]->start - wav_input[nsegs-1]->end;
	    if(gap > -overlap_limit && gap < join_time_limit)
	    {
		wav_input[nsegs++] = wav_sort[j];
		if(local_start_time > wav_sort[j]->start) {
			local_start_time = wav_sort[j]->start;
		}
		if(local_end_time < wav_sort[j]->end) {
			local_end_time = wav_sort[j]->end;
		}
	    }
	}

	/* NULL out the elements in wav that have been used
	 */
	for(j = 0; j < nsegs; j++) {
	    for(k = i; k < num_selected; k++) if(wav[k] == wav_input[j]) {
		wav[k] = NULL;
	    }
	}

	/* may need to modify the start time */
	if(start_time != NULL_TIME && window_length > 0)
	{
	    n =(int)((local_start_time - start_time)/window_length);
	    local_start_time = start_time + n*window_length;
	}

	if(start_time != NULL_TIME && local_start_time < start_time)
	{
	    local_start_time = start_time;
	}
	if(end_time != NULL_TIME && local_end_time > end_time) {
	    local_end_time = end_time;
	}
	if(max_length > 0. && local_end_time - local_start_time > max_length) {
	    local_end_time = local_start_time + max_length;
	}

	if((int)strlen(startPhase) > 0 && (int)strlen(endPhase) > 0
		&& wav_input[0]->origin_delta > 0.
		&& wav_input[0]->origin_time > NULL_TIME_CHECK)
	{
	    int s_stat, e_stat;
	    double t = phaseTime(startPhase, wav_input[0], arr_info,
				&s_stat);
	    if(s_stat) {
		local_start_time = t - start_phase_edge;
	    }
	    t = phaseTime(endPhase, wav_input[0], arr_info, &e_stat);
	    if(e_stat) {
		local_end_time = t + end_phase_edge;
	    }
	}

	if(window_length > 0) {
	    window = window_length;
	}
	else {
	    window = local_end_time - local_start_time;
	}

	for(start = local_start_time; start < local_end_time; start += window)
	{
	    end = start + window;
	    if(end > local_end_time) end = local_end_time;
		
	    WaveInput *w = new WaveInput(start, end, 0.);

	    for(j = 0; j < nsegs; j++) w->segments.push_back(wav_input[j]);
	    wav_inputs->push_back(w);

	    if(waveform_limit > 0 && (int)wav_inputs->size() >= waveform_limit)
	    {
		Free(wav_input); Free(wav_sort);
		return wav_inputs;
	    }
	}
    }
    Free(wav_input); Free(wav_sort);

    return wav_inputs;
}

// return true if it passes the test
bool WaveformConstraint::checkAz(SegmentInfo *wav, double minAz, double maxAz)
{
    if( (minAz > -1. && wav->origin_azimuth < minAz) ||
	(maxAz > -1. && wav->origin_azimuth > maxAz)) return(false);
    else return(true);
}


double WaveformConstraint::phaseTime(char *phase_code, SegmentInfo *wav,
			vector<ArrivalInfo> &arr_info, int *status)
{
    if(!strcmp(phase_code, "FirstObs") || !strcmp(phase_code, "LastObs"))
    {
	double time;
	firstObs(wav->net, phase_code, &time, arr_info);
	*status = 1;
	return (time);
    }
    else
    {
	float  delta, depth, tt;
	Iaspei iaspei;

	depth = (wav->origin_depth > -990.0) ?
                        (float)wav->origin_depth : 0.0;

	delta = wav->origin_delta;

	tt = iaspei.travelTime(phase_code, delta, depth);
	*status = (tt > 0.) ? 1 : 0;
	return(wav->origin_time + (double)tt);
    }
}

/* finds the times in the case of FirstObs
 */

int WaveformConstraint::firstObs(char *net, char *phase, double *time,
			vector<ArrivalInfo> &arr_info)
{
    int i, val;
    double first, last;

    val = 0;

    if(!strcmp(phase, "FirstObs"))
    {
	first = 9999999999.999;
	val = 1;

	for(i = 0; i < (int)arr_info.size(); i++) {
	    if(!strcmp(net, arr_info[i].net) && arr_info[i].time < first) {
		first = arr_info[i].time;
	    }
	}
	*time = first;
    }

    if(!strcmp(phase, "LastObs"))
    {
	last = -9999999999.999;
	val = 1;

	for(i = 0; i < (int)arr_info.size(); i++) {
	    if(!strcmp(net, arr_info[i].net) && arr_info[i].time > last) {
		last = arr_info[i].time;
	    }
	}
	*time = last;
    }
    return(val);
}

static int sortByTSC(const void *A, const void *B)
{
    SegmentInfo **a = (SegmentInfo **)A;
    SegmentInfo **b = (SegmentInfo **)B;

    if((*a)->start != (*b)->start)
    {
	return(((*a)->start < (*b)->start) ? -1 : 1);
    }
    return(sortBySC(a,b));

#ifdef OLD
    register char *a_chan, *b_chan;
    register int cnd, la, lb, len;
    if( (cnd = strcmp((*a)->sta,  (*b)->sta)) ) return(cnd);
    la = (int)strlen((*a)->chan);
    lb = (int)strlen((*b)->chan);
    len = (la > lb) ? la-1 : lb-1;
    if(len < 0) len = 0;
    /* first sort without using the last letter of the chan
     */
    if( (cnd = strncmp((*a)->chan, (*b)->chan, len)) ) return(cnd);

    /* now sort by the last letter of chan.
     */
    la = (la > 0) ? la-1 : 0;
    lb = (lb > 0) ? lb-1 : 0;
    if(app_data.chan_sort_order == NULL)
    {
	return(strcmp((*a)->chan+la, (*b)->chan+lb));
    }
    else
    {
	a_chan = strchr(app_data.chan_sort_order, (*a)->chan[la]);
	b_chan = strchr(app_data.chan_sort_order, (*b)->chan[lb]);
	return( (int)a_chan - (int)b_chan );
    }
#endif
}

static int sortBySC(const void *A, const void *B)
{
    SegmentInfo **a = (SegmentInfo **)A;
    SegmentInfo **b = (SegmentInfo **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->sta, (*b)->sta)) ) return(cnd);

    return(sortByC(a, b));
}

static int sortByC(const void *A, const void *B)
{
    SegmentInfo **a = (SegmentInfo **)A;
    SegmentInfo **b = (SegmentInfo **)B;
    register int cnd, la, lb, len;
    register char *a_chan, *b_chan;

    if(!strcasecmp((*a)->chan, (*b)->chan))
    {
	return(0);
    }

    la = (int)strlen((*a)->chan);
    lb = (int)strlen((*b)->chan);
    len = (la > lb) ? la-1 : lb-1;
    if(len < 0) len = 0;

    /* now sort by the last letter of chan.
     */
    la = (la > 0) ? la-1 : 0;
    lb = (lb > 0) ? lb-1 : 0;
    if((*a)->wc->chan_sort_order2[0] == '\0')
    {
	return(strcasecmp((*a)->chan+la, (*b)->chan+lb));
    }
    else
    {
	a_chan = strchr((*a)->wc->chan_sort_order2, (*a)->chan[la]);
	b_chan = strchr((*a)->wc->chan_sort_order2, (*b)->chan[lb]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    /* if it's a one char channel name, get out here */
    if (la == 0 || lb == 0)
    {
	if (la == lb) return(0);
	else if (la == 0) return(1);
	else if (lb == 0) return(-1);
    }

    /* first sort without using the last letter of the chan
     */
    if((*a)->wc->chan_sort_order0[0] == '\0')
    {
	if( (cnd = strncasecmp((*a)->chan, (*b)->chan, len)) ) return(cnd);
    }
    else
    {
	a_chan = strchr((*a)->wc->chan_sort_order0, (*a)->chan[0]);
	b_chan = strchr((*a)->wc->chan_sort_order0, (*b)->chan[0]);
	if( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }	

/*
    if (la == 3 && lb == 3 && ...
*/
    if(la == 2 && lb == 2 && (*a)->wc->chan_sort_order1[0] != '\0')
    {
	a_chan = strchr((*a)->wc->chan_sort_order1, (*a)->chan[1]);
	b_chan = strchr((*a)->wc->chan_sort_order1, (*b)->chan[1]);
	if( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }
    /*
     * who knows
     return(strcasecmp((*a)->sta, (*b)->sta));
     */
    return(strcasecmp((*a)->chan, (*b)->chan));
}

int WaveformConstraint::readWaveforms(DataSource *ds,
			gvector<SegmentInfo *> *segs,
			gvector<Waveform *> &wvec)
{
    const char *err_msg;
    int npts, pts = 0;
    bool preview_arr = false;
    GTimeSeries *ts;
    SegmentInfo **seginfo = NULL;
    vector<ArrivalInfo> arrival_info;

    seginfo = (SegmentInfo **)mallocWarn(
				(int)segs->size()*sizeof(SegmentInfo *));

    int num = 0;
    for(int i = 0; i < (int)segs->size(); i++) {
        SegmentInfo *s = segs->at(i);
        if(s->selected) {
            seginfo[num++] = s;
        }
    }

    vector<WaveInput *> *wav_inputs = getTimeLimits(num, seginfo, arrival_info);
    Free(seginfo);

    wvec.clear();
    if( (int)wav_inputs->size() == 0) {
	delete wav_inputs;
	return 0;
    }

    cvector<CssArrivalClass> arrivals;
    cvector<CssOriginClass> origins;
    cvector<CssOrigerrClass> origerrs;
    cvector<CssAssocClass> assocs;
    cvector<CssStassocClass> stassocs;
    cvector<CssHydroFeaturesClass> hydro_features;
    cvector<CssInfraFeaturesClass> infra_features;
    cvector<CssStamagClass> stamags;
    cvector<CssNetmagClass> netmags;
    cvector<CssWftagClass> wftags;
    cvector<CssAmplitudeClass> amplitudes;
    cvector<CssAmpdescriptClass> ampdescripts;
    cvector<CssParrivalClass> parrivals;

    num = (int)wav_inputs->size();

    for(int i = 0; i < num; i++)
    {
	arrivals.clear();
        origins.clear();
        origerrs.clear();
        assocs.clear();
        stassocs.clear();
        wftags.clear();
        hydro_features.clear();
        infra_features.clear();
        stamags.clear();
        netmags.clear();
        amplitudes.clear();
        ampdescripts.clear();
        parrivals.clear();

	WaveInput *w = wav_inputs->at(i);

	npts = ds->readData(&w->segments, w->start, w->end, pts,
			preview_arr, &ts, arrivals, origins, origerrs,
			assocs, stassocs, wftags, hydro_features,
			infra_features, stamags, netmags,
			amplitudes, ampdescripts, parrivals, &err_msg);

	if(err_msg != NULL) {
	    logErrorMsg(LOG_WARNING, err_msg);
	}
	if(npts <= 0) {
	    ts->deleteObject();
	    continue;
	}

	Waveform *wf = new Waveform(ts);
	wvec.push_back(wf);

	wf->ts->setSta(w->segments[0]->sta);
	wf->ts->setChan(w->segments[0]->chan);
	wf->ts->setNet(w->segments[0]->net);
	wf->ts->setLat(w->segments[0]->station_lat);
	wf->ts->setLon(w->segments[0]->station_lon);
	wf->ts->setElev(w->segments[0]->station_elev);
	delete w;
    }
    delete wav_inputs;

    return wvec.size();
}
