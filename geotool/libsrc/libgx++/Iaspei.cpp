/** \file Iaspei.cpp
 *  \brief Defines class Iaspei.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;
#include <sys/param.h>

#include "Iaspei.h"
#include "motif++/Application.h"

extern "C" {
#include "libisop.h"
#include "libstring.h"
}

static string last_filename;
static float source_depth = -9999.;

#define DGR2KM  111.1954        /* kilometers per degree */


Iaspei::Iaspei()
{
    string name;

    if(!Application::getProperty("iaspeiTable", name)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    name.assign(c + string("/tables/models/iasp91"));
	}
    }
    if(!name.empty()) {
	init(name, 0.0);
    }
    else {
	fprintf(stderr, "Iaspei: property iaspeiFilePrefix not specified.\n");
    }
}

Iaspei::Iaspei(const string &iaspei_file_prefix)
{
    init(iaspei_file_prefix, -9999.);
}

Iaspei::Iaspei(const string &iaspei_file_prefix, double depth)
{
    init(iaspei_file_prefix, depth);
}

void Iaspei::init(const string &iaspei_file_prefix, double depth)
{
    if(last_filename.compare(iaspei_file_prefix)) {
	last_filename = iaspei_file_prefix;
	if(tabin((char *)last_filename.c_str())) {
	    fprintf(stderr, "Cannot open iaspei tables:\n%s.hed\n%s.tbl",
			iaspei_file_prefix.c_str(), iaspei_file_prefix.c_str());
	}
    }
    setDepth(depth);

    stop_Pdiff = 120.0;
    t_vel = 1.485;
    lr_vel = 3.0;
    rg_vel = 3.0;
    lg_vel = 3.4;
    lq_vel = 3.2;
}

Iaspei::~Iaspei(void)
{
}

void Iaspei::setDepth(float depth)
{
    if(depth < -999.) {
	fprintf(stderr, "Iaspei: source depth not set.\n");
	return;
    }
    if(depth != source_depth) {
	source_depth = depth;
	depset(source_depth);
    }
}

float Iaspei::travelTime(const string &phase_name, float delta)
{
    return travelTime(phase_name, delta, source_depth);
}

float Iaspei::travelTime(const string &phase_name, float delta, float depth)
{
    float tt, ray_p, dtdd, dtdh, dddp;

    if(travelTime(phase_name, delta, depth, &tt, &ray_p, &dtdd, &dtdh, &dddp)) {
	return tt; 
    }
    else {
	return -1.;
    }
}

int Iaspei::travelTime(const string &phase_name, float delta, float depth,
	float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp)
{
    const char **first_phase;
    int i, j, n;
    char *phcd[200], phasecd[2000];
    float tts[200], ps[200], dtdds[200], dtdhs[200], dddps[200];

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
    *ray_p = -1.;
    *dtdd = -1.;
    *dtdh = -1.;
    *dddp = -1.;

    setDepth(depth);

    for(j = 0; j < 200; j++) phcd[j] = phasecd+j*10;

    trtm(delta, &n, tts, ps, dtdds, dtdhs, dddps, phcd);

    if(!phase_name.compare("FirstP") || !phase_name.compare("FirstS"))
    {
	first_phase = (!phase_name.compare("FirstP")) ? first_p:first_s;

	for(i = 0; first_phase[i] != NULL; i++)
	{
	    for(j = 0; j < n; j++) if(!strcmp(first_phase[i], phcd[j]))
	    {
		if(delta > stop_Pdiff &&
                        strstr(first_phase[i], "Pdiff") != NULL) break;

		if (*tt < 0 || *tt > tts[j])
		{
		    *tt = tts[j];
		    /* convert ray_p to sec/km */
		    *ray_p = ps[j] / (6371. - depth);
		    *dtdd = dtdds[j];
		    *dtdh = dtdhs[j];
		    *dddp = dddps[j];
		}
	    }
	}
    }
    else if (!phase_name.compare("T")) {
	*tt = delta*DGR2KM/t_vel;
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
		*tt = tts[j];
		/* convert ray_p to sec/km */
		*ray_p = ps[j] / (6371. - depth);
		*dtdd = dtdds[j];
		*dtdh = dtdhs[j];
		*dddp = dddps[j];
		break;
	    }
	}
    }
    return( *tt > 0 ? 1 : 0);
}

int Iaspei::getCurve(const string &phase, double depth, int *npts, float *tt,
			float *dist, float *ray_p)
{
    setDepth((float)depth);
    int n_branches = 0;
    get_seg((char *)phase.c_str(), npts, tt, dist, ray_p, &n_branches);
    return n_branches;
}

