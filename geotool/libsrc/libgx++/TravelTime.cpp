/** \file TravelTime.cpp
 *  \brief Defines class TravelTime.
 *  \author Ivan Henson
 */
#include "config.h"
#include <math.h>
#include <sys/types.h>
#include <sys/param.h>
#include <strings.h>
#include <iostream>
#include <sstream>
#include "TravelTime.h"
#include "motif++/Application.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "libisop.h"
}

using namespace std;

#define Free(a) {if(a) free((void *)a); a = NULL; }
#define DGR2KM  111.1954        // kilometers per degree

static string iaspei_table_gb;
static string jb_table_gb;

typedef struct
{
    const char  *phase;
    int		phase_q;
    bool        use_internal_tt;
} PhaseList;

static int findPhase(const string &phase, int num, PhaseList *phase_list);

extern "C" {
static int
computeTTMethod(double origin_time, double origin_lat, double origin_lon,
	double origin_depth, double delta, double esaz, const char *net,
	double station_lat, double station_lon, double station_elev,
	int num_phases, const char **phases, double *tt, double *slowness);
}

#define NUM_IASPEI      84
#define NUM_JB          13
#define NUM_REGIONAL    10
#define NUM_SURFACE      6

#define NUM_TOTAL      113

/*  These phases are not in the CTBTO travel time table files.
 *  PbPb, PgPg, SbSb, Sg, SgSg, P'P'df, S'S'df, P'P'ab, SPg, PgS, SPn, PnS,
 *  P'P'bc, S'S'ac, pPn, sPn, pPb, sPb, sPg, sSn, sSb
 */

static PhaseList iaspei_phases[NUM_IASPEI] =
{
    {"Pn",     0, false}, {"PnPn",   0, false},
    {"Pb",     0, false}, {"PbPb",   0,  true},
    {"Pg",     0, false}, {"PgPg",   0,  true},
    {"Sn",     0, false}, {"SnSn",   0, false},
    {"Sb",     0, false}, {"SbSb",   0,  true},
    {"Sg",     0,  true}, {"SgSg",   0,  true},
    {"PcP",    0, false}, {"PcS",    0, false},
    {"ScP",    0, false}, {"ScS",    0, false},
    {"PKiKP",  0, false}, {"SKiKP",  0, false},
    {"PKKPdf", 0, false}, {"PKKSdf", 0, false},
    {"SKKPdf", 0, false}, {"SKKSdf", 0, false},
    {"P'P'df", 0,  true}, {"S'S'df", 0,  true},
    {"P'P'ab", 0,  true}, {"P",      0, false},
    {"S",      0, false}, {"SPg",    0,  true},
    {"PgS",    0,  true}, {"PP",     0, false},
    {"SS",     0, false}, {"SPn",    0,  true},
    {"PnS",    0,  true}, {"P'P'bc", 0,  true},
    {"SKSac",  0, false}, {"SKKSac", 0, false},
    {"S'S'ac", 0,  true}, {"PKKPbc", 0, false},
    {"SKKPbc", 0, false}, {"PKKSbc", 0, false},
    {"PS",     0, false}, {"SP",     0, false},
    {"Pdiff",  0, false}, {"Sdiff",  0, false},
    {"SKSdf",  0, false}, {"PKKPab", 0, false},
    {"SKPdf",  0, false}, {"PKSdf",  0, false},
    {"PKPdf",  0, false}, {"PKSbc",  0, false},
    {"SKPbc",  0, false}, {"SKPab",  0, false},
    {"PKSab",  0, false}, {"PKKSab", 0, false},
    {"SKKPab", 0, false}, {"PKPbc",  0, false},
    {"PKPab",  0, false}, {"pPn",    0,  true},
    {"sPn",    0,  true}, {"pPb",    0,  true},
    {"sPb",    0,  true}, {"sPg",    0,  true},
    {"sSn",    0,  true}, {"sSb",    0,  true},
    {"pPKiKP", 0, false}, {"sPKiKP", 0, false},
    {"pP",     0, false}, {"sP",     0, false},
    {"sS",     0, false}, {"pS",     0, false},
    {"pSKSac", 0, false}, {"sSKSac", 0, false},
    {"pPdiff", 0, false}, {"sPdiff", 0, false},
    {"pSdiff", 0, false}, {"sSdiff", 0, false},
    {"pSKSdf", 0, false}, {"sSKSdf", 0, false},
    {"pPKPdf", 0, false}, {"sPKPdf", 0, false},
    {"pPKPbc", 0, false}, {"sPKPbc", 0, false},
    {"pPKPab", 0, false}, {"sPKPab", 0, false},
};
static PhaseList jb_phases[NUM_JB] =
{
    {"JP",     0, false}, {"JPP",    0, false},
    {"JS",     0, false}, {"JSS",    0, false},
    {"JPcP",   0, false}, {"JpP",    0, false},
    {"JsP",    0, false}, {"JScS",   0, false},
    {"JPKPab", 0, false}, {"JPKPbc", 0, false},
    {"JPKPdf", 0, false}, {"JSKSac", 0, false},
    {"JSKSdf", 0, false},
};
static int jb_code[NUM_JB] =
{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 17,
};
static PhaseList regional_phases[NUM_REGIONAL] =
{
    {"RPg",   0, false}, {"RPn",  0, false},
    {"RSg",   0, false}, {"RSn",  0, false},
    {"RP*",   0, false}, {"RS*",  0, false},
    {"RPx",   0, false}, {"RSx",  0, false},
    {"RPIP",  0, false}, {"RPMP", 0, false},
};
/*
 * A good average hydrocoustic velocity for is 1.48 km/sec.
 * An average infrasonic velocity is 300 m/sec.
*/
static PhaseList surface_phases[NUM_SURFACE] =
{
    {"LR",  0,  true}, {"LQ",  0,  true},
    {"Lg",  0, false}, {"T",   0, false},
    {"I",   0, false}, {"Rg",  0, false},
};


TrTm::TrTm(void) :
	num_iaspei(NUM_IASPEI), num_jb(NUM_JB), num_regional(NUM_REGIONAL),
	num_surface(NUM_SURFACE), lat(0.), lon(0.), elev(0.), delta(0.),
	az(0.), baz(0.), otime(0.), olat(0.), olon(0.), depth(0.),
	infra_tt(0.), iaspei_tt(NULL), iaspei_slow(NULL), jb_tt(NULL),
	jb_slow(NULL), regional_tt(NULL), regional_slow(NULL), surface_tt(NULL),
	surface_slow(NULL)
{
    memset((void *)net, 0, sizeof(net));
    memset((void *)sta, 0, sizeof(sta));

    iaspei_tt = new float[NUM_IASPEI];
    iaspei_slow = new float[NUM_IASPEI];
    jb_tt = new float[NUM_JB];
    jb_slow = new float[NUM_JB];
    regional_tt = new float[NUM_REGIONAL];
    regional_slow = new float[NUM_REGIONAL];
    surface_tt = new float[NUM_SURFACE];
    surface_slow = new float[NUM_SURFACE];

    for(int i = 0; i < NUM_IASPEI; i++) {
	iaspei_tt[i] = 0.;
	iaspei_slow[i] = 0.;
    }
    for(int i = 0; i < NUM_JB; i++) {
	jb_tt[i] = 0.;
	jb_slow[i] = 0.;
    }
    for(int i = 0; i < NUM_REGIONAL; i++) {
	regional_tt[i] = 0.;
	regional_slow[i] = 0.;
    }
    for(int i = 0; i < NUM_SURFACE; i++) {
	surface_tt[i] = 0.;
	surface_slow[i] = 0.;
    }
}

TrTm::~TrTm(void)
{
    delete [] iaspei_tt;
    delete [] iaspei_slow;
    delete [] jb_tt;
    delete [] jb_slow;
    delete [] regional_tt;
    delete [] regional_slow;
    delete [] surface_tt;
    delete [] surface_slow;
}

const char * TrTm::iaspeiPhase(int i)
{
    if(i >= 0 && i < NUM_IASPEI) {
	return iaspei_phases[i].phase;
    }
    return NULL;
}
const char * TrTm::jbPhase(int i)
{
    if(i >= 0 && i < NUM_JB) {
	return jb_phases[i].phase;
    }
    return NULL;
}
const char * TrTm::regionalPhase(int i)
{
    if(i >= 0 && i < NUM_REGIONAL) {
	return regional_phases[i].phase;
    }
    return NULL;
}
const char * TrTm::surfacePhase(int i)
{
    if(i >= 0 && i < NUM_SURFACE) {
	return surface_phases[i].phase;
    }
    return NULL;
}

double TrTm::iaspeiTime(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_IASPEI; i++) {
	if(q == iaspei_phases[i].phase_q) {
	    return iaspei_tt[i];
	}
    }
    return -1.;
}
double TrTm::jbTime(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_JB; i++) {
	if(q == jb_phases[i].phase_q) {
	    return jb_tt[i];
	}
    }
    return -1.;
}
double TrTm::regionalTime(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_REGIONAL; i++) {
	if(q == regional_phases[i].phase_q) {
	    return regional_tt[i];
	}
    }
    return -1.;
}
double TrTm::surfaceTime(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_SURFACE; i++) {
	if(q == surface_phases[i].phase_q) {
	    return surface_tt[i];
	}
    }
    return -1.;
}

double TrTm::iaspeiSlowness(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_IASPEI; i++) {
	if(q == iaspei_phases[i].phase_q) {
	    return iaspei_slow[i];
	}
    }
    return -1.;
}
double TrTm::jbSlowness(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_JB; i++) {
	if(q == jb_phases[i].phase_q) {
	    return jb_slow[i];
	}
    }
    return -1.;
}
double TrTm::regionalSlowness(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_REGIONAL; i++) {
	if(q == regional_phases[i].phase_q) {
	    return regional_slow[i];
	}
    }
    return -1.;
}
double TrTm::surfaceSlowness(const string &phase)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < NUM_SURFACE; i++) {
	if(q ==  surface_phases[i].phase_q) {
	    return surface_slow[i];
	}
    }
    return -1.;
}

TravelTime::TravelTime(const string &iaspei_prefix, const string &jb_file,
			ComputeTTMethod method) :
	num_iaspei(NUM_IASPEI), num_jb(NUM_JB), num_regional(NUM_REGIONAL),
	num_surface(NUM_SURFACE), stop_Pdiff(120.), lg_vel(3.4), lq_vel(3.2),
	lr_vel(3.0), rg_vel(3.0), t_vel(1.485), infra_vel(.320),
	infra_tt(-1.), last_depth(-999.), crust(), jb_table(""),
	iaspei_table(""), jb_first_warn(true), use_celerity(false),
	last_trtm(), compute_tt_method(method)
{
    const char *c;

    iaspei_table.assign(iaspei_prefix);
    jb_table.assign(jb_file);

    if( iaspei_table.empty() &&
		(c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	iaspei_table = c + string("/tables/models/iasp91");
    }

    if( jb_table.empty() &&
		(c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	jb_table = c + string("/tables/models/jbtable");
    }

    if(!compute_tt_method) {
	compute_tt_method = computeTTMethod;
    }

    crust.h[0] = 0.;
    crust.h[1] = 0.;
    crust.vp[0] = -1.;
    crust.vp[1] = -1.;
    crust.vp[2] = -1.;
    crust.vs[0] = -1.;
    crust.vs[1] = -1.;
    crust.vs[2] = -1.;
    memset((void *)crust.name, 0, sizeof(crust.name));
    memset((void *)crust.full_name, 0, sizeof(crust.full_name));

    if(iaspei_phases[0].phase_q == 0) {
	for(int i = 0; i < NUM_IASPEI; i++) {
	    iaspei_phases[i].phase_q = stringToQuark(iaspei_phases[i].phase);
	}
	for(int i = 0; i < NUM_JB; i++) {
	    jb_phases[i].phase_q = stringToQuark(jb_phases[i].phase);
	}
	for(int i = 0; i < NUM_REGIONAL; i++) {
	    regional_phases[i].phase_q =stringToQuark(regional_phases[i].phase);
	}
	for(int i = 0; i < NUM_SURFACE; i++) {
	    surface_phases[i].phase_q = stringToQuark(surface_phases[i].phase);
	}
    }
}

TravelTime::~TravelTime(void)
{
}

int TravelTime::getIaspeiPhases(const char ***phase_list)
{
    const char **plist = NULL;

    plist = (const char **)malloc(NUM_IASPEI*sizeof(const char *));
    for(int i = 0; i < NUM_IASPEI; i++) plist[i] = iaspei_phases[i].phase;
    *phase_list = plist;
    return NUM_IASPEI;
}

void TravelTime::setLgVelocity(double lgvel)
{
    if(lg_vel != lgvel) {
	if(lgvel <= 0.) {
	    cerr << "TravelTime: invalid lgvel=" << lgvel << endl;
	}
	else {
	    lg_vel = lgvel;
	}
    }
}

void TravelTime::setLqVelocity(double lqvel)
{
    if(lq_vel != lqvel) {
	if(lqvel <= 0.) {
	    cerr << "TravelTime: invalid lqvel=" << lqvel << endl;
	}
	else {
	    lq_vel = lqvel;
	}
    }
}

void TravelTime::setLrVelocity(double lrvel)
{
    if(lr_vel != lrvel) {
	if(lrvel <= 0.) {
	    cerr << "TravelTime: invalid lrvel=" << lrvel << endl;
	}
	else {
	    lr_vel = lrvel;
	}
    }
}

void TravelTime::setRgVelocity(double rgvel)
{
    if(rg_vel != rgvel) {
	if(rgvel <= 0.) {
	    cerr << "TravelTime: invalid rgvel=" << rgvel << endl;
	}
	else {
	    rg_vel = rgvel;
	}
    }
}

void TravelTime::setTVelocity(double tvel)
{
    if(t_vel != tvel) {
	if(tvel <= 0.) {
	    cerr << "TravelTime: invalid tvel=" << tvel << endl;
	}
	else {
	    t_vel = tvel;
	}
    }
}

void TravelTime::setCelerity(double celerity)
{
    if(infra_vel != celerity) {
	if(celerity <= 0.) {
	    cerr << "TravelTime: invalid celerity=" << celerity << endl;
	}
	else {
	    infra_vel = celerity;
	}
    }
}

void TravelTime::setUseCelerity(bool use)
{
    if(use_celerity != use) {
	use_celerity = use;
    }
}

double TravelTime::getTravelTime(const string &phase, double otime, double olat,
		double olon, double depth, double lat, double lon, double elev,
		const string &net, const string &sta, double *slowness,
		string &op)
{
    char type;
    double tt;

    if(!strcasecmp(phase.c_str(), "firstp") ||
		!strcasecmp(phase.c_str(), "firsts"))
    {
	if(!strcasecmp(phase.c_str(), "firstp")) {
	    type = 'P';
	}
	else {
	    type = 'S';
	}
	if(firstArrival(otime, olat, olon, depth, lat, lon, elev, net, sta,
			type, op, &tt, slowness))
	{
	    return tt;
	}
	return -1.;
    }
    else {
	TrTm tr;
	getAllTimes(&tr, otime, olat, olon, depth, lat, lon, elev, net, sta);
	tt = tr.tt(phase);
	*slowness = tr.slowness(phase.c_str());
	op.assign(phase);
	return tt;
    }
}

/** 
 *  Get all the travel times for an origin and a station location.
 *  The travel times for all phases are computed for a CssOrigin and
 *  a station location. A TrTm structure is returned that holds the
 *  phase codes and travel times.
 *  <pre>
 *  typedef struct
 *  {
 *	char	sta[10]; - The network name
 *	float	delta;	 - The distance from the origin to lat,lon.
 *	float	olat;	 - The source latitude.
 *	float	olon;	 - The source longitude.
 *	float	depth;   - The depth of the origin.
 *	float	tt[NUM_TOTAL]; - An array of travel times in seconds.
 *  } TrTm;
 *  </pre>
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] origin A CssOrigin object.
 *  @param[in] lat The station latitude in degrees.
 *  @param[in] lon The station longitude in degrees.
 *  @param[in] elev The station elevation in meters.
 *  @param[in] net The network name.
 *  @param[in] sta The station name.
 *  @returns A TrTm class pointer, or NULL if an error occured.
p
 */
void TravelTime::getAllTimes(TrTm *tr, double otime, double olat, double olon,
		double depth, double lat, double lon, double elev,
		const string &net, const string &sta)
{
    int	j;
    bool same=false;
    TrTm *lt = &last_trtm;

    initTr(tr, otime, olat, olon, depth, lat, lon, elev, net, sta);

    if(compute_tt_method == computeTTMethod)
    {
	// tt's depend on source and station location
	// hydro and infra can depend on otime (seasonal correction)
	// Day-of-year resolution
	int tr_doy = (int)(timeEpochToJDate(tr->otime)%1000);
	int doy = (int)(timeEpochToJDate(otime)%1000);
	if( tr_doy == doy &&
	    fabs(tr->olat - lt->olat) <= .01 &&
	    fabs(tr->olon - lt->olon) <= .01 &&
	    fabs(tr->depth - lt->depth) <= .01 &&
	    fabs(tr->lat - lt->lat) <= .01 &&
	    fabs(tr->lon - lt->lon) <= .01 &&
	    fabs(tr->elev - lt->elev) <= .01 &&
	    !strcmp(tr->net, lt->net) &&
	    !strcmp(tr->sta, lt->sta))
	{
	    same = true;
	}
    }
    else if(compute_tt_method == NULL)
    {
	if( fabs(tr->delta - lt->delta) <= .01 &&
	    fabs(tr->depth - lt->depth) <= .01)
	{
	    same = true;
	}
    }

    if( !same )
    {
	initTr(lt, otime, olat, olon, depth, lat, lon, elev, net, sta);
	getTimes(lt);
    }

    // copy results from last_trtm
    for(j = 0; j < NUM_IASPEI; j++) {
	tr->iaspei_tt[j] = lt->iaspei_tt[j];
	tr->iaspei_slow[j] = lt->iaspei_slow[j];
    }
    for(j = 0; j < NUM_JB; j++) {
	tr->jb_tt[j] = lt->jb_tt[j];
	tr->jb_slow[j] = lt->jb_slow[j];
    }
    for(j = 0; j < NUM_REGIONAL; j++) {
	tr->regional_tt[j] = lt->regional_tt[j];
	tr->regional_slow[j] = lt->regional_slow[j];
    }
    for(j = 0; j < NUM_SURFACE; j++) {
	tr->surface_tt[j] = lt->surface_tt[j];
	tr->surface_slow[j] = lt->surface_slow[j];
    }

    // Always compute the regional tt in case the crust has changed.
    getRegional(tr);

    // Always compute the surface tt in case the velocities have changed.
    getSurface(tr);
}

void TravelTime::getTimes(TrTm *lt)
{
    int	i, j, k, n;
    const char *phcd[200];
    char phasecd[2000];
    float tt[200], p[200], dtdd[200], dtdh[200], dddp[200];
    double ttd[200], slowness[200];
    Derivatives der;

    for(i = 0; i < 200; i++) {
	tt[i] = 0.;
	p[i] = 0.;
	dtdd[i] = 0.;
	dtdh[i] = 0.;
	dddp[i] = 0.;
	ttd[i] = 0.;
	slowness[i] = 0.;
    }

    if(compute_tt_method != NULL)
    {
	// use internal method below to get phases that are not included here.
	int np = 0;
	for(j = 0; j < NUM_IASPEI; j++) {
	    if( !iaspei_phases[j].use_internal_tt ) {
		phcd[np++] = iaspei_phases[j].phase;
	    }
	}
	for(j = 0; j < NUM_SURFACE; j++) {
	    if( !surface_phases[j].use_internal_tt ) {
		phcd[np++] = surface_phases[j].phase;
	    }
	}

	/* Pass the net name instead of sta, since tt-corrections
	 * key on it. delta/az is to the station.
	 */
	(*(compute_tt_method))(lt->otime, lt->olat, lt->olon, lt->depth,
		lt->delta, lt->az, lt->net, lt->lat, lt->lon, lt->elev, np,
		phcd, ttd, slowness);

	np = 0;
	for(j = 0; j < NUM_IASPEI; j++) {
	    if( !iaspei_phases[j].use_internal_tt ) {
		lt->iaspei_slow[j] = slowness[np];
		lt->iaspei_tt[j] = ttd[np++];
	    }
	}
	for(j = 0; j < NUM_SURFACE; j++) {
	    if( !surface_phases[j].use_internal_tt ) {
		lt->surface_slow[j] = slowness[np];
		lt->surface_tt[j] = ttd[np++];
	    }
	}
	lt->infra_tt = lt->surface_tt[4]; // "I" phase

	if(!strncmp(crust.name, "iasp", 4))
	{
	    /* for iaspei crust, use these times instead of the
	     * regional computation below.
	     */
	    for(k = 0; k < NUM_REGIONAL; k++) {
		for(j = 0; j < NUM_IASPEI && strcmp(iaspei_phases[j].phase,
			regional_phases[k].phase+1); j++);
		if(j < NUM_IASPEI && ttd[j] > 0.) {
		    lt->regional_slow[k] = slowness[j];
		    lt->regional_tt[k] = ttd[j];
		}
	    }
	}
	// get times for use_internal_tt phases
	if(openIaspei(iaspei_table.c_str()))
	{
	    for(j = 0; j < 200; j++) phcd[j] = phasecd+j*10;
	    setIaspeiDepth(lt->depth);
	    trtm2(lt->delta, &n, tt, p, dtdd, dtdh, dddp, (char **)phcd);

	    for(i = 0; i < n; i++) {
		j = findPhase(phcd[i], NUM_IASPEI, iaspei_phases);
		if(j >= 0 && iaspei_phases[j].use_internal_tt) {
		    lt->iaspei_tt[j] = tt[i];
		    lt->iaspei_slow[j] = dtdd[i];
		}
	    }
	}
    }
    else
    {
	if(openIaspei(iaspei_table.c_str()))
	{
	    for(j = 0; j < 200; j++) phcd[j] = phasecd+j*10;
	    setIaspeiDepth(lt->depth);
	    trtm2(lt->delta, &n, tt, p, dtdd, dtdh, dddp, (char **)phcd);

	    for(i = 0; i < n; i++) {
		j = findPhase(phcd[i], NUM_IASPEI, iaspei_phases);
		if(j >= 0) {
		    lt->iaspei_tt[j] = tt[i];
		    lt->iaspei_slow[j] = dtdd[i];
		}
	    }
	}
    }
    if( !jb_table.empty() && openjb(jb_table.c_str()) )
    {
	float t;
	for(j = 0; j < NUM_JB; j++)
	{
	    if(!jbsim(jb_code[j], lt->delta, lt->depth, &t, &der))
	    {
		lt->jb_tt[j] = t;
		lt->jb_slow[j] = der.dtdd; // sec/deg
	    }
	}
    }
}

void TravelTime::initTr(TrTm *tr, double otime, double olat, double olon,
		double depth, double lat, double lon, double elev,
		const string &net, const string &sta)
{
    tr->num_iaspei = NUM_IASPEI;
    tr->num_jb = NUM_JB;
    tr->num_regional = NUM_REGIONAL;
    tr->num_surface = NUM_SURFACE;

    snprintf(tr->net, sizeof(tr->net), "%s", net.c_str());
    snprintf(tr->sta, sizeof(tr->sta), "%s", sta.c_str());
    tr->lat = lat;
    tr->lon = lon;
    tr->elev = elev;

    tr->otime = otime;
    tr->olat = olat;
    tr->olon = olon;
    tr->depth = (depth < 0.) ? 0. : depth;
    deltaz(olat, olon, lat, lon, &tr->delta, &tr->az, &tr->baz);

    tr->infra_tt = -1.;
    for(int j = 0; j < NUM_IASPEI; j++) {
	tr->iaspei_tt[j] = -1.;
	tr->iaspei_slow[j] = -1.;
    }
    for(int j = 0; j < NUM_JB; j++) {
	tr->jb_tt[j] = -1.;
	tr->jb_slow[j] = -1.;
    }
    for(int j = 0; j < NUM_REGIONAL; j++) {
	tr->regional_tt[j] = -1.;
	tr->regional_slow[j] = -1.;
    }
    for(int j = 0; j < NUM_SURFACE; j++) {
	tr->surface_tt[j] = -1.;
	tr->surface_slow[j] = -1.;
    }
}

void TravelTime::getRegional(TrTm *tr)
{
    Derivatives der;

    if(tr->delta < 20.)
    {
	for(int i = 0; i < NUM_REGIONAL; i++)
	{
	    float tt;
	    if(!regional(&crust, regional_phases[i].phase+1, tr->delta,
				tr->depth, &tt, &der))
	    {
		// check if we don't already have this from above
		if(tr->regional_tt[i] < 0.) {
		    tr->regional_tt[i] = tt;
		    tr->regional_slow[i] = der.dtdd; // sec/deg
		}
	    }
	}
    }
}

void TravelTime::getSurface(TrTm *tr)
{
    tr->surface_tt[0] = tr->delta*DGR2KM/lr_vel;
    tr->surface_tt[1] = tr->delta*DGR2KM/lq_vel;

    tr->surface_slow[0] = lr_vel/DGR2KM; // vel in sec/deg
    tr->surface_slow[1] = lq_vel/DGR2KM; // vel in sec/deg

    if(compute_tt_method == NULL)
    {
	tr->surface_tt[2] = tr->delta*DGR2KM/lg_vel;
	tr->surface_tt[3] = tr->delta*DGR2KM/t_vel;
	tr->surface_tt[4] = tr->delta*DGR2KM/infra_vel;
	tr->surface_tt[5] = tr->delta*DGR2KM/rg_vel;

	tr->surface_slow[2] = lq_vel/DGR2KM; // vel in sec/deg
	tr->surface_slow[3] = t_vel/DGR2KM; // vel in sec/deg
	tr->surface_slow[4] = infra_vel/DGR2KM; // vel in sec/deg
	tr->surface_slow[5] = rg_vel/DGR2KM; // vel in sec/deg
    }
    if(use_celerity) {
	tr->surface_tt[4] = tr->delta*DGR2KM/infra_vel;
	tr->surface_slow[4] = infra_vel/DGR2KM; // vel in sec/deg
    }
}

void TravelTime::setIaspeiDepth(float depth)
{
    if(fabs(depth - last_depth) > 0.01) {
	last_depth = depth;
	depset2(depth);
    }
}

/** 
 *  Get the travel time, ray parameter and derivatives for a phase, depth
 *  and distance.
 *  
 */
bool TravelTime::iaspei(const string &phase_name, float delta, float depth,
	float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp)
{
    const char **first_phase;
    int i, j, n;
    char *phcd[200], phasecd[2000];
    float ttf[200], p[200], dtddf[200], dtdhf[200], dddpf[200];
 
    const char *first_p[] =
    {
	"Pn", "PnPn", "Pb", "PbPb", "Pg", "PgPg", "PcP", "ScP",
	"PKiKP", "SKiKP", "PKKPdf", "SKKPdf", "P'P'df", "P'P'ab",
	"P", "SPg", "PP", "SPn", "P'P'bc", "PKKPbc", "SKKPbc", "SP",
	"Pdiff", "PKKPab", "SKPdf", "PKPdf", "SKPbc", "SKPab",
	"SKKPab", "PKPbc", "PKPab", "pPn", "sPn", "pPb", "sPb", "sPg",
	"pPKiKP", "sPKiKP", "pP", "sP", "pPdiff", "sPdiff", "pPKPdf",
	"sPKPdf", "pPKPbc", "sPKPbc", "pPKPab", "sPKPab", NULL,
    };
    const char *first_s[] =
    {
	"Sn", "SnSn", "Sb", "SbSb", "Sg", "SgSg", "PcS", "ScS",
	"PKKSdf", "SKKSdf", "S'S'df", "S", "PgS", "SS", "PnS", "SKSac",
	"SKKSac", "S'S'ac", "PKKSbc", "PS", "Sdiff", "SKSdf", "PKSdf",
	"PKSbc", "PKSab", "PKKSab", "sSn", "sSb", "sS", "pS", "pSKSac",
	"sSKSac", "pSdiff", "sSdiff", "pSKSdf", "sSKSdf", NULL,
    };
 
    *tt = -1.;
 
    if(!openIaspei(iaspei_table.c_str())) return false;

    setIaspeiDepth(depth);

    for(j = 0; j < 200; j++) phcd[j] = phasecd+j*10;
 
    trtm2(delta, &n, ttf, p, dtddf, dtdhf, dddpf, phcd);
 
    if (!phase_name.compare("FirstP") || !phase_name.compare("FirstS"))
    {
	first_phase = (!phase_name.compare("FirstP")) ? first_p : first_s;
 
	for(i = 0; first_phase[i] != NULL; i++)
	{
	    for(j = 0; j < n; j++) if (!strcmp(first_phase[i], phcd[j]))
	    {
		if(delta > stop_Pdiff && strstr(first_phase[i], "Pdiff")) break;
 
		if (*tt < 0 || *tt > ttf[j])
		{
		    *tt = ttf[j];
		    // convert ray_p to sec/km
		    *ray_p = p[j] / (6371. - depth);
		    *dtdd = dtddf[j];
		    *dtdh = dtdhf[j];
		    *dddp = dddpf[j];
		}
	    }
	}
    }
    else if (!phase_name.compare("T")) {
	*tt = delta*DGR2KM/t_vel;
    }
    else if (!phase_name.compare("I")) {
	*tt = delta*DGR2KM/infra_vel;
    }
    else if (!phase_name.compare("LR")) {
	*tt = delta*DGR2KM/lr_vel;
    }
    else if (!phase_name.compare("Rg")) {
	*tt = delta*DGR2KM/rg_vel;
    }
    else if (!phase_name.compare("Lg")) {
	*tt = delta*DGR2KM/lg_vel;
    }
    else if (!phase_name.compare("LQ")) {
	*tt = delta*DGR2KM/lq_vel;
    }
    else
    {
	for(j = 0; j < n; j++)
	{
	    if(!phase_name.compare(phcd[j]))
	    {
		*tt = ttf[j];
		// convert ray_p to sec/km
		*ray_p = p[j] / (6371. - depth);
		*dtdd = dtddf[j];
		*dtdh = dtdhf[j];
		*dddp = dddpf[j];
		break;
	    }
	}
    }
    return( *tt > 0 ? true : false);
}

/** 
 *  Get the phase integer code for a phase name.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] phase A phase name.
 *  @returns the phase code ( > = 0), or -1 if no phase code exists for the
 *		input phase name.
 */
/*
int TravelTime::phaseToCode(const char *phase)
{
    int i;
    int code = 0;

    for(i = 0; i < NUM_IASPEI; i++, code++) {
	if(!strcmp(iaspei_phases[i].phase, phase)) return(code);
    }
    for(i = 0; i < NUM_JB; i++, code++) {
	if(!strcmp(jb_phases[i].phase, phase)) return(code);
    }
    for(i = 0; i < NUM_REGIONAL; i++, code++) {
	if(!strcmp(phase, regional_phases[i].phase+1)) return(code);
    }
    for(i = 0; i < NUM_SURFACE; i++, code++) {
	if(!strcmp(phase, surface_phases[i].phase)) return(code);
    }
    return -1;
}
*/

/** 
 *  Get the string phase name for an integer phase code.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] code A phase code.
 *  @returns the phase name or NULL for an invalid code
 */
/*
const char * TravelTime::getPhase(int code)
{
    int n;

    if(code >= 0 && code < NUM_IASPEI) {
	return iaspei_phases[code].phase;
    }

    n = NUM_IASPEI;
    if(code >= n && code < n + NUM_JB) {
	return jb_phases[code-n].phase;
    }

    n += NUM_JB;
    if(code >= n && code < n + NUM_REGIONAL) {
	return regional_phases[code-n].phase;
    }

    n += NUM_REGIONAL;
    if(code >= n && code < n + NUM_SURFACE) {
	return surface_phases[code-n].phase;
    }
    return (char *)NULL;
}
*/

const char * TravelTime::iaspeiPhase(int i)
{
    if(i >= 0 && i < NUM_IASPEI) {
	return iaspei_phases[i].phase;
    }
    return (const char *)NULL;
}

const char * TravelTime::jbPhase(int i)
{
    if(i >= 0 && i < NUM_JB) {
	return jb_phases[i].phase;
    }
    return (const char *)NULL;
}

const char * TravelTime::regionalPhase(int i)
{
    if(i >= 0 && i < NUM_REGIONAL) {
	return regional_phases[i].phase;
    }
    return (const char *)NULL;
}

const char * TravelTime::surfacePhase(int i)
{
    if(i >= 0 && i < NUM_SURFACE) {
	return surface_phases[i].phase;
    }
    return (const char *)NULL;
}

/** 
 *  Set the crustal model. This will be used in regional travel time
 *  computations.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] crust The crustal model.
 */
bool TravelTime::setCrust(CrustModel *crust_model)
{
    // check if this is a new crust model
    if( crust.h[0]  ==crust_model->h[0] &&  crust.h[1] == crust_model->h[1] &&
 	crust.vp[0] ==crust_model->vp[0] && crust.vp[1] == crust_model->vp[1] &&
	crust.vp[2] ==crust_model->vp[2] && crust.vs[0] == crust_model->vs[0] &&
	crust.vs[1] ==crust_model->vs[1] && crust.vs[2] == crust_model->vs[2])
    {
	// just in case the name changed
	memcpy(&crust, crust_model, sizeof(CrustModel));
	return false;
    }

    memcpy(&crust, crust_model, sizeof(CrustModel));

    return true;
}

/** 
 *  Get the phase name and travel time of the first arrival. For
 *  an origin location and a station location, return the phase name and
 *  travel time of the first arrival.
 *  @param[in] w A TtPlotWidget pointer.
 *  @param[in] origin A CssOrigin.
 *  @param[in] lat The station latitude in degrees.
 *  @param[in] lon The station longitude in degrees.
 *  @param[in] elev The station elevation in meters.
 *  @param[in] net The network name.
 *  @param[in] sta The station name.
 *  @param[in] type 'P' for the first P phase, or 'S' for the first S phase.
 *  @param[out] phase The returned phase name.
 *  @param[out] time The computed travel time.
 *  @param[out] slowness The computed slowness (sec/deg).
 *  @returns true for success, false for failure.
 */
bool TravelTime::firstArrival(double otime, double olat, double olon,
	double depth, double lat, double lon, double elev, const string &net,
	const string &sta, char type, string &phase, double *time,
	double *slowness)
{
    const char **first_phase;
    TrTm tr;
    double tmin=0., tt;
    const char *first_p[] =
    {
	"Pn", "PnPn", "Pb", "PbPb", "Pg", "PgPg", "PcP", "ScP",
	"PKiKP", "SKiKP", "PKKPdf", "SKKPdf", "P'P'df", "P'P'ab",
	"P", "SPg", "PP", "SPn", "P'P'bc", "PKKPbc", "SKKPbc", "SP",    
	"Pdiff", "PKKPab", "SKPdf", "PKPdf", "SKPbc", "SKPab",
	"SKKPab", "PKPbc", "PKPab", "pPn", "sPn", "pPb", "sPb", "sPg",
	"pPKiKP", "sPKiKP", "pP", "sP", "pPdiff", "sPdiff", "pPKPdf",
	"sPKPdf", "pPKPbc", "sPKPbc", "pPKPab", "sPKPab", NULL,
    };
    const char *first_s[] =
    {
	"Sn", "SnSn", "Sb", "SbSb", "Sg", "SgSg", "PcS", "ScS", 
	"PKKSdf", "SKKSdf", "S'S'df", "S", "PgS", "SS", "PnS", "SKSac",
	"SKKSac", "S'S'ac", "PKKSbc", "PS", "Sdiff", "SKSdf", "PKSdf",
	"PKSbc", "PKSab", "PKKSab", "sSn", "sSb", "sS", "pS", "pSKSac",
	"sSKSac", "pSdiff", "sSdiff", "pSKSdf", "sSKSdf", NULL,
    };

    getAllTimes(&tr, otime, olat, olon, depth, lat, lon, elev, net, sta);

    *time = -1.;
    *slowness = -1.;
    phase.clear();

    first_phase = (type == 'P') ? first_p : first_s;

    for(int i = 0; first_phase[i] != NULL; i++)
    {
	tt = tr.tt(first_phase[i]);

	if(tt > 0.)
	{
	    if(!strstr(first_phase[i], "Pdiff") || tr.delta <= stop_Pdiff) {
		if(tmin == 0. || tmin > tt) {
		    *time = tt;
		    *slowness = tr.slowness(first_phase[i]);
		    phase.assign(first_phase[i]);
		    tmin = *time;
		}
	    }
	}
    }
    return (*time > 0.) ? true : false;
}

/*
#define JB_TABLE_INCLUDED
*/
bool TravelTime::openjb(const string &jb_file)
{
#ifndef JB_TABLE_INCLUDED
    if(jb_file.compare(jb_table_gb))
    {
	if(jbopen((char *)jb_file.c_str())) {
	    cerr << "Cannot open jbTable: " << jb_file << endl;
	    return false;
	}
	jb_table_gb = jb_file;
    }
#endif
    return true;
}
 
bool TravelTime::openIaspei(const string &iaspei_prefix)
{
    if(iaspei_prefix.compare(iaspei_table_gb))
    {
	if(tabin((char *)iaspei_prefix.c_str()))
	{
	    fprintf(stderr, "Cannot open iaspei tables:\n%s.hed\n%s.tbl",
			iaspei_prefix.c_str(), iaspei_prefix.c_str());
	    return false;
	}
	iaspei_table_gb = iaspei_prefix;
    }
    return true;
}

const char * TravelTime::getJBFile(void)
{
    return jb_table.c_str();
}

const char * TravelTime::getIaspeiPrefix(void)
{
    return iaspei_table.c_str();
}

static int
findPhase(const string &phase, int num, PhaseList *phase_list)
{
    int q = stringToQuark(phase);
    for(int i = 0; i < num; i++) {
	if(q == phase_list[i].phase_q) return(i);
    }
    return -1;
}

extern "C" {
#include "ibase/libloc.h"
#include "ibase/loc_params.h"
#include "ibase/site_Astructs.h"
}

/*
 * lat,lon: degrees
 * depth: km
 * elev: meters
 * tt: seonds
 * slowness: sec/deg
 */
static int
computeTTMethod(double origin_time, double origin_lat, double origin_lon,
	double origin_depth, double delta, double esaz, const char *net,
	double station_lat, double station_lon, double station_elev,
	int num_phases, const char **phases, double *tt, double *slowness)
{
    Site site;
    Ar_Info ar_info;
    string vmodel_file, sasc_dir;
    char *geotool_home = NULL;
    Locator_params locator_params;
    Bool extrapolate = FALSE;
    int i;
    static string last_sasc_dir;

    for(i = 0; i < num_phases; i++) tt[i] = -1;

    snprintf(site.sta, sizeof(site.sta), "%s", net);
    site.lat = station_lat;
    site.lon = station_lon;
    site.elev = station_elev;

    if(!Application::getProperty("vmodel_spec_file", vmodel_file) &&
       !Application::getProperty("locate.LocSAT.vmodel_spec_file", vmodel_file)
	&& (geotool_home = (char *)getenv("GEOTOOL_HOME")) )
    {
	vmodel_file = geotool_home + string("/tables/data/TT/vmsf/idc.defs");
    }
    if( vmodel_file.empty() ) {
	fprintf(stderr, "Cannot find vmodel_spec_file.\n");
	return 0;
    }

    /*
     * Read travel-time tables
     */
    if(setup_tt_facilities((char *)vmodel_file.c_str(), (char **)phases,
			num_phases, &site, 1) != OK)
    {
	fprintf(stderr, "Error reading T-T tables.\n");
	return 0;
    }
    // set time for acoustic seasonal correction
    set_epoch_travel_time(origin_time);

    /*
     * Read slowness/azimuth station correction (SASC) tables
     */
    if(  !Application::getProperty("sasc_dir_prefix", sasc_dir) &&
         !Application::getProperty("locate.LocSAT.sasc_dir_prefix", sasc_dir)
      && (geotool_home = (char *)getenv("GEOTOOL_HOME")) )
    {
        sasc_dir = geotool_home + string("/tables/data/SASC/sasc");
    }
    if( sasc_dir.empty() ) {
        fprintf(stderr, "Cannot find sasc_dir_prefix.\n");
    }
    else if(last_sasc_dir.compare(sasc_dir))
    {
	last_sasc_dir = sasc_dir;
	if(read_sasc((char *)last_sasc_dir.c_str()) != OK)
	{
	    fprintf(stderr, "Error while trying to read SASC tables.\n");
	}
    }

    /* Specify static parameters */

    locator_params.refill_if_loc_fails	= 0;
    locator_params.use_elev_corr	= 1;
    locator_params.est_std_error	= 1.0;
    locator_params.verbose		= '4';
    locator_params.use_location		= 0;
    locator_params.fix_origin_time	= 0;
    locator_params.fix_lat_lon		= 0;
    locator_params.fix_depth		= 0;
    locator_params.origin_time_init	= 0.;
    locator_params.lat_init		= 0.;
    locator_params.lon_init		= 0.;
    locator_params.depth_init		= 0.;
    locator_params.damp			= -1.;
    locator_params.conf_level		= .90;
    locator_params.num_dof		= 999999;
    locator_params.max_iterations	= 20;
    locator_params.ignore_large_res	= 0;
    locator_params.large_res_mult	= 3.;
    locator_params.ellip_cor_type	= 2;
    locator_params.sssc_level		= 1;
    locator_params.use_test_site_corr	= (int) FALSE;
    locator_params.use_srst		= 0;
    locator_params.srst_var_wgt		= 0;
    locator_params.use_only_sta_w_corr	= 0;
    locator_params.dist_var_wgt		= 1;
    locator_params.user_var_wgt		= -1.;
    strcpy(locator_params.test_site_region, "-");
    locator_params.prefix		= (char *)vmodel_file.c_str();
    locator_params.outfile_name		= NULL;

    for(i = 0; i < num_phases; i++) {
	tt[i] = compute_ttime_w_corrs (&locator_params, &site,
		extrapolate, origin_lat, origin_lon, origin_depth, delta,
		esaz, (char *)phases[i], 0, &ar_info, &slowness[i]);
    }

    return 1;
}
