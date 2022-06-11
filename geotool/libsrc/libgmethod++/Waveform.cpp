/** \file Waveform.cpp
 *  \brief Defines class Waveform.
 *  \author Ivan Henson
 */
#include "config.h"
#include <strings.h>
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/CssTables.h"
#include "DataMethod.h"
#include "IIRFilter.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libtime.h"
static int sort_by_y0(const void *A, const void *B);
static int sort_by_sta(const void *A, const void *B);
static int sort_by_net(const void *A, const void *B);
static int sort_by_chan(const void *A, const void *B);
static int sort_by_chan2(const void *A, const void *B);
static int sort_by_time(const void *A, const void *B);
static int sort_by_dist(const void *A, const void *B);
static int sort_by_order(const void *A, const void *B);
static int sort_by_default_order(const void *A, const void *B);
}

using namespace std;

static char chan_sort_order0[50] = "bBeEsSlLmMhHuUvV";
static char chan_sort_order1[50] = "hHpPlL";
static char chan_sort_order2[50] = "bBzZnNeErRtTdDxXlL";

static int unique_id = 0;

Waveform::Waveform(GTimeSeries *timeseries) :
	cplot(NULL), ts(timeseries), visible(true), selected(true),
	scaled_x0(0.), scaled_y0(0.), fg(0), num_dp(0), dp(NULL), num_dw(0),
	dw(NULL), begSelect(NULL_TIME), endSelect(NULL_TIME), file_order(0),
	default_order(0), distance(0.), id(unique_id++)
{
    init();
}

Waveform::Waveform(void) :
	cplot(NULL), ts(NULL), visible(true), selected(true), scaled_x0(0.),
	scaled_y0(0.), fg(0), num_dp(0), dp(NULL), num_dw(0), dw(NULL),
	begSelect(NULL_TIME), endSelect(NULL_TIME), file_order(0),
	default_order(0), distance(0.), id(unique_id++)
{
    init();
}

Waveform::Waveform(const Waveform &w) :
	cplot(w.cplot), ts(w.ts), visible(w.visible), selected(w.selected),
	scaled_x0(w.scaled_x0), scaled_y0(w.scaled_y0), fg(w.fg),
	num_dp(0), dp(NULL), num_dw(0), dw(NULL), begSelect(w.begSelect),
	endSelect(w.endSelect), file_order(w.file_order),
	default_order(w.default_order), distance(w.distance),
	id(unique_id++)
{
    channel = w.channel;
    for(int i = 0; i < MAX_COMPONENTS; i++) c[i] = w.c[i];
    id = unique_id++;
}

Waveform & Waveform::operator=(const Waveform &w)
{
    if(this == &w) return *this;

    cplot = w.cplot;
    ts = w.ts;
    visible = w.visible;
    selected = w.selected;
    scaled_x0 = w.scaled_x0;
    scaled_y0 = w.scaled_y0;
    fg = w.fg;
    num_dp = 0;
    dp = NULL;
    num_dw = 0;
    dw = NULL;
    begSelect = w.begSelect;
    endSelect = w.endSelect;
    file_order = w.file_order;
    default_order = w.default_order;
    distance = w.distance;

    channel = w.channel;
    for(int i = 0; i < MAX_COMPONENTS; i++) c[i] = w.c[i];

    id = unique_id++;

    return *this;
}

void Waveform::init(void)
{
    for(int i = 0; i < MAX_COMPONENTS; i++) c[i] = NULL;
}

Waveform::~Waveform()
{
/*
    if(ts) ts->removeOwner(this);
    for(int i = 0; i < num_dp; i++) {
	dp[i]->removeOwner(this);
    }
    for(int i = 0; i < num_dw; i++) {
	dw[i].d1->removeOwner(this);
	dw[i].d2->removeOwner(this);
    }
    Free(dp);
    Free(dw);
*/
}

// static function
void Waveform::setChanSortOrder0(const string &chan_order)
{
    strncpy(chan_sort_order0, chan_order.c_str(), sizeof(chan_sort_order0));
    chan_sort_order0[sizeof(chan_sort_order0)-1] = '\0';
}

// static function
void Waveform::setChanSortOrder1(const string &chan_order)
{
    strncpy(chan_sort_order1, chan_order.c_str(), sizeof(chan_sort_order1));
    chan_sort_order1[sizeof(chan_sort_order1)-1] = '\0';
}

// static function
void Waveform::setChanSortOrder2(const string &chan_order)
{
    strncpy(chan_sort_order2, chan_order.c_str(), sizeof(chan_sort_order2));
    chan_sort_order2[sizeof(chan_sort_order2)-1] = '\0';
}

void Waveform::sortByY0(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_y0);
}

void Waveform::sortBySta(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_sta);
}

void Waveform::sortByNet(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_net);
}

void Waveform::sortByChan(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_chan);
}

void Waveform::sortByChan2(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_chan2);
}

void Waveform::sortByTime(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_time);
}

void Waveform::sortByDistance(gvector<Waveform *> &wvec,
			double lat, double lon)
{
    if(wvec.size() <= 0) return;

    gvector<Waveform *> w_dist, w_no_dist;

    for(int i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];
	if(w->lat() > -900. && w->lon() > -900.) {
	    double az, baz;
	    deltaz(lat, lon, w->lat(), w->lon(), &w->distance, &az, &baz);
	    w_dist.push_back(w);
	}
	else {
	    w_no_dist.push_back(w);
	}
    }
    w_dist.sort(sort_by_dist);
    wvec.clear();
    wvec.load(w_dist);

    // place w's with no distance at the end of wvec;
    for(int i = 0; i < w_no_dist.size(); i++) {
	wvec.push_back(w_no_dist[i]);
    }
}

void Waveform::sortByDistance(gvector<Waveform *> &wvec,
					double *lat, double *lon)
{
    if(wvec.size() <= 0) return;

    gvector<Waveform *> w_dist, w_no_dist;

    for(int i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];
	if(w->lat() > -900. && w->lon() > -900.
		&& lat[i]> -900. && lon[i]> -900.)
	{
	    double az, baz;
	    deltaz(lat[i], lon[i], w->lat(), w->lon(), &w->distance, &az, &baz);
	    w_dist.push_back(w);
	}
	else {
	    w_no_dist.push_back(w);
	}
    }
    w_dist.sort(sort_by_dist);
    wvec.clear();
    wvec.load(w_dist);

    // place ws with no distance at the end of wvec;
    for(int i = 0; i < w_no_dist.size(); i++) {
	wvec.push_back(w_no_dist[i]);
    }
}

void Waveform::sortByBaz(gvector<Waveform *> &wvec, double lat, double lon)
{
    if(wvec.size() <= 0) return;

    gvector<Waveform *> w_dist, w_no_dist;

    for(int i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];
	if(w->lat() > -900. && w->lon() > -900.) {
	    double az, dist;
	    deltaz(lat, lon, w->lat(), w->lon(), &dist, &az, &w->distance);
	    w_dist.push_back(w);
	}
	else {
	    w_no_dist.push_back(w);
	}
    }
    w_dist.sort(sort_by_dist);
    wvec.clear();
    wvec.load(w_dist);

    // place ws with no distance at the end of wvec;
    for(int i = 0; i < w_no_dist.size(); i++) {
	wvec.push_back(w_no_dist[i]);
    }
}

void Waveform::sortByBaz(gvector<Waveform *> &wvec, double *lat, double *lon)
{
    if(wvec.size() <= 0) return;

    gvector<Waveform *> w_dist, w_no_dist;

    for(int i = 0; i < wvec.size(); i++)
    {
	Waveform *w = wvec[i];
	if(w->lat() > -900. && w->lon() > -900.
		&& lat[i]> -900. && lon[i]> -900.)
	{
	    double az, dist;
	    deltaz(lat[i], lon[i], w->lat(), w->lon(), &dist, &az,&w->distance);
	    w_dist.push_back(w);
	}
	else {
	    w_no_dist.push_back(w);
	}
    }
    w_dist.sort(sort_by_dist);
    wvec.clear();
    wvec.load(w_dist);

    // place ws with no distance at the end of wvec;
    for(int i = 0; i < w_no_dist.size(); i++) {
	wvec.push_back(w_no_dist[i]);
    }
}

void Waveform::sortByFileOrder(gvector<Waveform *> &wvec)
{
    if(wvec.size() <= 0) return;

    gvector<Waveform *> w_dist, w_no_dist;

    for(int i = 0; i < wvec.size(); i++)
    {
	if(wvec[i]->ts->getValue("file order", &wvec[i]->file_order)) {
	    w_dist.push_back(wvec[i]);
	}
	else {
	    w_no_dist.push_back(wvec[i]);
	}
    }
    w_dist.sort(sort_by_order);
    wvec.clear();
    wvec.load(w_dist);

    // place ws with no distance at the end of wvec;
    for(int i = 0; i < w_no_dist.size(); i++) {
	wvec.push_back(w_no_dist[i]);
    }
}

void Waveform::sortByDefaultOrder(gvector<Waveform *> &wvec)
{
    wvec.sort(sort_by_default_order);
}

static int
sort_by_y0(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;

    if((*a)->scaled_y0 > (*b)->scaled_y0) return(1);
    else if((*a)->scaled_y0 < (*b)->scaled_y0) return(-1);
    else return(0);
}

static int
sort_by_sta(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->sta(), (*b)->sta())) ) return(cnd);

    return(sort_by_chan(a, b));
}

static int
sort_by_net(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd;

    if( (cnd = strcmp((*a)->net(), (*b)->net())) ) return(cnd);

    return(sort_by_chan2(a, b));
}

static int
sort_by_chan(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd, la, lb, len;
    register char *a_chan, *b_chan;

    la = (int)(*a)->channel.length();
    lb = (int)(*b)->channel.length();
    len = (la > lb) ? la-1 : lb-1;
    if(len < 0) len = 0;

    /* first sort without using the last letter of the chan
     */
    if(chan_sort_order0[0] == '\0')
    {
	if( (cnd=(*a)->channel.compare(0, len, (*b)->channel)) )
	{
	    return(cnd);
	}
    }
    else
    {
	a_chan = strchr(chan_sort_order0, (*a)->channel[0]);
	b_chan = strchr(chan_sort_order0, (*b)->channel[0]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    /* now sort by the last letter of chan.
     */
    la = (la > 0) ? la-1 : 0;
    lb = (lb > 0) ? lb-1 : 0;
    if(chan_sort_order2[0] == '\0')
    {
	return((*a)->channel.substr(la).compare((*b)->channel.substr(lb)));
    }
    else {
	a_chan = strchr(chan_sort_order2, (*a)->channel[la]);
	b_chan = strchr(chan_sort_order2, (*b)->channel[lb]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    if(la == 3 && lb == 3 && chan_sort_order1[0] != '\0')
    {
	a_chan = strchr(chan_sort_order1, (*a)->channel[1]);
	b_chan = strchr(chan_sort_order1, (*b)->channel[1]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    /*
     * who knows
     */
    return(strcmp((*a)->sta(), (*b)->sta()));
}

static int
sort_by_chan2(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    register int cnd, la, lb, len;
    register char *a_chan, *b_chan;

    la = (int)(*a)->channel.length();
    lb = (int)(*b)->channel.length();
    len = (la > lb) ? la-1 : lb-1;
    if(len < 0) len = 0;

    /* now sort by the last letter of chan.
     */
    la = (la > 0) ? la-1 : 0;
    lb = (lb > 0) ? lb-1 : 0;
    if(chan_sort_order2[0] == '\0')
    {
	return((*a)->channel.substr(la).compare((*b)->channel.substr(lb)));
    }
    else
    {
	a_chan = strchr(chan_sort_order2, (*a)->channel[la]);
	b_chan = strchr(chan_sort_order2, (*b)->channel[lb]);
	if( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    if(la == 3 && lb == 3 && chan_sort_order1[0] != '\0')
    {
	a_chan = strchr(chan_sort_order1, (*a)->channel[1]);
	b_chan = strchr(chan_sort_order1, (*b)->channel[1]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    /* first sort without using the last letter of the chan
     */
    if(chan_sort_order0[0] != '\0')
    {
	if( (cnd = (*a)->channel.compare(0, len, (*b)->channel)) ) return(cnd);
    }
    else
    {
	a_chan = strchr(chan_sort_order0, (*a)->channel[0]);
	b_chan = strchr(chan_sort_order0, (*b)->channel[0]);
	if ( (cnd = (long)a_chan - (long)b_chan)) return(cnd);
    }

    /*
     * who knows
     */
    return(strcmp((*a)->sta(), (*b)->sta()));
}

static int
sort_by_time(const void *A, const void *B)
{
    Waveform **a = (Waveform **)A;
    Waveform **b = (Waveform **)B;
    if((*a)->tbeg() != (*b)->tbeg())
    {
	return (*a)->tbeg() < (*b)->tbeg() ? -1:1;
    }
    return(sort_by_sta(a, b));
}

static int
sort_by_dist(const void *A, const void *B)
{
    Waveform *a = *(Waveform **)A;
    Waveform *b = *(Waveform **)B;
    if(a->distance != b->distance)
    {
	return(a->distance < b->distance ? -1 : 1);
    }
    return(sort_by_sta(A, B));
}

static int
sort_by_order(const void *A, const void *B)
{
    Waveform *a = *(Waveform **)A;
    Waveform *b = *(Waveform **)B;
    if(a->file_order != b->file_order)
    {
	return(a->file_order < b->file_order ? -1 : 1);
    }
    return 0;
}

static int
sort_by_default_order(const void *A, const void *B)
{
    Waveform *a = *(Waveform **)A;
    Waveform *b = *(Waveform **)B;
    if(a->default_order != b->default_order)
    {
	return(a->default_order < b->default_order ? -1 : 1);
    }
    return 0;
}

bool Waveform::changeMethod(DataMethod *dm)
{
    return DataMethod::changeMethod(dm, this);
}

bool Waveform::changeMethods(int num, DataMethod **dm)
{
    gvector<Waveform *> wvec;
    wvec.push_back(this);
    return DataMethod::changeMethods(num, dm, wvec);
}

bool Waveform::applyMethod(DataMethod *dm)
{
    gvector<Waveform *> wvec;
    wvec.push_back(this);
    return DataMethod::applyMethods(1, &dm, wvec);
}

bool Waveform::applyMethods(int num, DataMethod **dm)
{
    gvector<Waveform *> wvec;
    wvec.push_back(this);
    return DataMethod::applyMethods(num, dm, wvec);
}

static const char *names[] = {
	"Text Input", "Station", "Channel", "Distance(deg)", "Distance(km)",
	"Back Azimuth", "YYMONDD", "YYYYDDD", "HH:MM:SS", "Wfid",
	"Filter(low high)", "Filter(type)", "Filter(order)",
	"Filter(causal/zero phase)", "Instrument Name", "orid", "evid",
	"yy-mm-dd", "yyyy-mm-dd"
};

const char ** Waveform::memberNames()
{
    return names;
}

int Waveform::numMemberNames()
{
    return sizeof(names)/sizeof(char *);
}

char * Waveform::getTag(WaveformTag &tag, const string &sta,
	const string &channel, CssOriginClass *origin, double tbeg, double lat,
	double lon, GTimeSeries *ts)
{
    int	i, str_len, tag_len, size_this_line, max_line_size=10;
    char str[1000];
    char *tag_str;

    tag_str = (char *)malloc(10);
    tag_str[0] = '\0';

    size_this_line = 0;
    tag_len = 0;
    for(i = 0; i < (int)tag.members.size(); i++)
    {
	getPieceOfTag(tag, tag.members[i], sta, channel, origin, tbeg, lat,
			lon, ts, str, sizeof(str));
	str_len = strlen(str);
	if(str_len > 0)
	{
	    tag_str = (char *)realloc(tag_str, tag_len + str_len + 2);
	    tag_len += str_len + 2;

	    if(i > 0) {
		if(tag.members[i-1] == 1 && tag.members[i] == 2) {
		    strcat(tag_str, "/");
		    size_this_line += str_len + 1;
		}
		else if(size_this_line + str_len + 1 <= max_line_size) {
		    strcat(tag_str, " ");
		    size_this_line += str_len + 1;
		}
		else {
		    strcat(tag_str, "\n");
		    size_this_line = str_len;
		}
	    }
	    else {
		size_this_line = str_len;
	    }
	    strcat(tag_str, str);
	}
    }
    return(tag_str);
}

#define DGR2KM	111.1954	/* kilometers per degree */

/* Tag member IDs:
         0   user defined
         1   station
         2   channel
         3   distance deg
         4   distance km
         5   azimuth
         6   ddmonyy
         7   yyyyddd
         8   hh:mm:ss
         9   wfid
        10   filter hp lp
        11   filter type
        12   filter order
        13   filter phase/less
        14   instrument name
        15   orid
        16   evid
        17   yy-mm-dd
        18   yyy-mm-dd
 */

void Waveform::getPieceOfTag(WaveformTag &tag, int member_id, const string &sta,
	const string &channel, CssOriginClass *origin, double tbeg, double lat,
	double lon, GTimeSeries *ts, char *str, int str_size)
{
//    char **row = NULL;
    char c[100];
    int i, j;
    DateTime dt;
    double delta, az, baz;
    gvector<DataMethod *> *methods;

    str[0] = '\0';
    str[str_size-1] = '\0';

    if(member_id == 0)
    {
	if( !tag.ud_string.empty() ) {
	    stringcpy(str, tag.ud_string.c_str(), str_size);
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if(member_id == 1) {
	stringcpy(str, sta.c_str(), str_size);
    }
    else if(member_id == 2) {
	stringcpy(str, channel.c_str(), str_size);
    }
    else if(member_id == 9)
    {
	stringcpy(str, "NA", str_size);
	if(ts) {
	    cvector<CssWfdiscClass> *v = ts->getWfdiscs();
	    if(v) {
		int len = 0;
		for(i = 0; i < v->size(); i++) {
		    if(i == 0) {
			snprintf(str+len, str_size-len, "%ld", v->at(i)->wfid);
		    }
		    else {
			snprintf(str+len, str_size-len, ",%ld", v->at(i)->wfid);
		    }
		    len = (int)strlen(str);
		}
		delete v;
	    }
	}
    }
    else if(member_id == 10)
    {
	if(ts == NULL) {
	    stringcpy(str, "NA", str_size);
	    return;
	}
	methods = ts->dataMethods();
//	stringcpy(str, "NA", str_size);
	for(i = j = 0; i < (int)methods->size(); i++)
	{
	    IIRFilter *iir = methods->at(i)->getIIRFilterInstance();
	    if( iir )
	    {
//		if(!strcmp(str, "NA")) str[0] = '\0';
		if(iir->getFhigh() <= 1.0 &&
			    (iir->getFlow() > 0.0 && iir->getFhigh() > 0.0))
		{
		    snprintf(c, 100,"%.2f %.2f",iir->getFlow(),iir->getFhigh());
		}
		else {
		    snprintf(c, 100,"%.1f %.1f",iir->getFlow(),iir->getFhigh());
		}
		if(j++ > 0) strncat(str, "  ", str_size-strlen(str));
		strncat(str, c, str_size-strlen(str));
	    }
	}
	delete methods;
    }
    else if(member_id == 11)
    {
	if(ts == NULL) {
	    stringcpy(str, "NA", str_size);
	    return;
	}
	methods = ts->dataMethods();
	stringcpy(str, "NA", str_size);
	for(i = j = 0; i < (int)methods->size(); i++) {
	    IIRFilter *iir = methods->at(i)->getIIRFilterInstance();
	    if( iir ) {
		if(!strncmp(str, "NA", str_size)) str[0] = '\0';
		strncat(str, iir->getType(), str_size-strlen(str));
		if(j++ > 0) strncat(str, "  ", str_size-strlen(str));
	    }
	}
	delete methods;
    }
    else if(member_id == 12)
    {
	if(ts == NULL) {
	    stringcpy(str, "NA", str_size);
	    return;
	}
	methods = ts->dataMethods();
	stringcpy(str, "NA", str_size);
	for(i = j = 0; i < (int)methods->size(); i++) {
	    IIRFilter *iir = methods->at(i)->getIIRFilterInstance();
	    if( iir ) {
		if(!strcmp(str, "NA")) str[0] = '\0';
		snprintf(c, 100, "%d", iir->getOrder());
		if(j++ > 0) strncat(str, "  ", str_size-strlen(str));
		strncat(str, c, str_size-strlen(str));
	    }
	}
	delete methods;
    }
    else if(member_id == 13)
    {
	if(ts == NULL) {
	    stringcpy(str, "NA", str_size);
	    return;
	}
	methods = ts->dataMethods();
	stringcpy(str, "NA", str_size);
	for(i = j = 0; i < (int)methods->size(); i++) {
	    IIRFilter *iir = methods->at(i)->getIIRFilterInstance();
	    if( iir ) {
		if(!strcmp(str, "NA")) str[0] = '\0';
		snprintf(c, 100, "%d", iir->getZeroPhase());
		if(j++ > 0) strncat(str, "  ", str_size-strlen(str));
		strncat(str, c, str_size - strlen(str));
	    }
	}
	delete methods;
    }
/* this and the next one should be removed, since it is a 2 digit year! */
    else if(member_id == 6)
    {
	timeEpochToDate(tbeg, &dt);
	snprintf(str, str_size, "%02d%s%02d", dt.year%100,
			timeMonthName(dt.month), dt.day);
    }
    else if(member_id == 17)
    {
	timeEpochToDate(tbeg, &dt);
	snprintf(str, str_size, "%02d-%02d-%02d",dt.year%100, dt.month, dt.day);
    }
    else if(member_id == 18)
    {
	timeEpochToString(tbeg, str, str_size, YMD);
    }
    else if(member_id == 7)
    {
	timeEpochToDate(tbeg, &dt);
	snprintf(str, str_size, "%d%03d", dt.year, timeDOY(&dt));
    }
    else if(member_id == 8)
    {
	timeEpochToString(tbeg, str, str_size, HMS2);
#ifdef OLD
	timeEpochToDate(tbeg, &dt);

	/* prevent 60 seconds from appearing */
	diff = 60.0 - dt.second;
	if (diff < 0.5) {
	    timeEpochToDate(tbeg+0.5, &dt);
	}

	snprintf(str, str_size, "%02d:%02d:%.0f",dt.hour, dt.minute, dt.second);
#endif
    }
    else if(member_id == 3)	/* DISTANCE deg */
    {
	if(origin != NULL) {
	    deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);
	    snprintf(str, str_size, "%.1f", delta);
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if(member_id == 4)	/* DISTANCE km */
    {
	if(origin != NULL) {
	    deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);
	    snprintf(str, str_size, "%.1f", delta*DGR2KM);
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if(member_id == 5)	/* AZIMUTH */
    {
	if(origin != NULL) {
	    deltaz(origin->lat, origin->lon, lat, lon, &delta, &az, &baz);
	    snprintf(str, str_size, "%.1f", baz);
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if(member_id == 14)
    {
	string s;
	if(ts->getInstype(s)) {
	    snprintf(str, str_size, "%s", s.c_str());
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if (member_id == 15) /* orid */
    {
	if(origin != NULL) {
	    snprintf(str, str_size, "%ld", origin->orid);
	}
	else {
	    stringcpy(str, "NA", str_size);
	}
    }
    else if (member_id == 16) /* evid */
    {
/* get evid from another place.
	if(origin != NULL) {
	    Attribute *attr[1], a;
	    attr[0] = &a;
	    row = (char **)mallocWarn(sizeof(char *));
	    row[0] = NULL;
	    a.name = "evid";
	    a.format[0] = '\0';
	    origin_table_row(w,origin, NULL, 0, NULL, NULL, 1, attr, row, 1,
				"r", True);
	    if (row[0] == NULL) {
		stringcpy(str, "NA", str_size);
	    }
	    else {
		stringcpy(str, row[0], str_size);
	    }
            Free(row);
	}
	else {
*/
	    stringcpy(str, "NA", str_size);
//	}
    }
    else {
	stringcpy(str, "?", str_size);
    }
}
