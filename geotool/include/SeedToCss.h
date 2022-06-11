/** \file SeedToCss.h
 *  \brief Declares the SeedToCss class members and methods.
 *  \author Ivan Henson
 */
#ifndef _SEED_TO_CSS_H
#define _SEED_TO_CSS_H

#include "config.h"
#include <string>
#include <map>
#include "gobject++/CssTables.h"
#include "gobject++/cvector.h"
#include "gobject++/GSegment.h"
#include "seed/Blockettes.h"
#include "seed/SeedData.h"
using namespace std;

class Resp
{
    public:
    Blockette50 b50;
    Blockette52 b52;
    CssInstrumentClass instrument;
    string css_resp;
    Resp() {}
    Resp(const Resp &r) {
	b50 = r.b50;
	b52 = r.b52;
	instrument = r.instrument;
    }
};

/**
 *  A SeedToCss is a class for converting Seed information to the CSS file format.
 *
 *@see Seed
 */
class SeedToCss
{
    public:
	static bool convertToCss(string seedfile, string dir, string prefix, string respdir, bool update, bool getdata) {
		return convertToCss(seedfile, dir, prefix, respdir, "", update, getdata);
	}
	static bool convertToCss(string seedfile, string dir, string prefix, string respdir, string geotabledir, bool update, bool getdata);

	static bool getCssTables(string read_path, cvector<CssAffiliationClass> &affiliations, cvector<CssSiteClass> &sites,
			cvector<CssSitechanClass> &sitechans, cvector<CssSensorClass> &sensor, vector<Resp> &responses,
			cvector<CssOriginClass> &origins, cvector<CssArrivalClass> &arrivals);
	static bool getWaveforms(string read_path, cvector<CssWfdiscClass> &wfdiscs, vector<GSegment *> &segments);
	static GSegment * readSegment(SeedData *sd, string read_path, double start_time, double end_time);
	static GSegment * readSegment(SeedData *sd, string read_path) {
		return readSegment(sd, read_path, -1.e+60, 1.e+60);
	}

    protected:
	static bool writeTable(gvector<CssTableClass *> &tables, string file);
	static bool writeTable(vector<Resp> &v, string file, string respdir);
	static bool addAffiliations(cvector<CssAffiliationClass> &affiliations, gvector<CssTableClass *> &tables);
	static bool addSites(cvector<CssSiteClass> &sites, gvector<CssTableClass *> &tables);
	static bool addSitechans(cvector<CssSitechanClass> &sitechans, gvector<CssTableClass *> &tables, map<int, int> &chanids, long max_chanid);
	static bool addSensors(cvector<CssSensorClass> &sensors, gvector<CssTableClass *> &tables, map<int, int> &chanids,
			map<int, int> &inids, long max_inid);
	static bool addInstruments(vector<Resp> &responses, gvector<CssTableClass *> &tables, string respdir, map<int, int> &inids);
};

#endif
