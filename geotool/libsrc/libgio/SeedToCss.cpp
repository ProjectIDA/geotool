/** \file SeedToCss.cpp
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <math.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>

#include "SeedToCss.h"
#include "SeedSource.h"
#include "seed/SeedInput.h"
#include "seed/Seed2CssResp.h"
#include "seed/Seed2CssResp.h"
#include "gobject++/CssTables.h"
#include "gobject++/GTimeSeries.h"
#include "Response.h"
#include "cssio.h"
#include <fstream>
#include <map>

extern "C" {
#include "libgmath.h"
#include "libtime.h"
#include "libstring.h"
}

bool SeedToCss::getCssTables(string seedfile,
		cvector<CssAffiliationClass> &affiliations,
		cvector<CssSiteClass> &sites,
		cvector<CssSitechanClass> &sitechans,
		cvector<CssSensorClass> &sensors,
		vector<Resp> &responses,
		cvector<CssOriginClass> &origins,
		cvector<CssArrivalClass> &arrivals)
{
    int i, chanid = 0;
    Seed *o;
    Blockette33 *b33;
    Blockette50 b50;
    Blockette52 b52;
    Blockette53 *b53;
    Blockette58 *b58;
    Blockette55 *b55;
    Blockette71 *b71;
    Blockette72 *b72;
    double a0_norm_freq = 1.0;
    double calib = 1.0;
    double scaled_sens = 1.0;
    ifstream ifs;
    bool first_b52 = false;
    string rsptype = "";
    map<int, string> units;

    ifs.open(seedfile.c_str());
    if( !ifs.good() ) {
        char error[MAXPATHLEN+100];
        snprintf(error, sizeof(error), "seed: cannot open %s", seedfile.c_str());
        logErrorMsg(LOG_WARNING, error);
        return false;
    }

    SeedInput in(&ifs);

    while( (o = in.readSeed()) ) {
	if( o->getBlockette50() ) {
	    b50 = *o->getBlockette50();
	    first_b52 = true;
	    int net_q = stringUpperToQuark(b50.network.c_str());
	    int sta_q = stringUpperToQuark(b50.station.c_str());

	    CssAffiliationClass *aff = new CssAffiliationClass();
	    stringcpy(aff->net, b50.network.c_str(), sizeof(aff->net));
	    stringcpy(aff->sta, b50.station.c_str(), sizeof(aff->sta));
	    aff->net_quark = net_q;
	    aff->sta_quark = sta_q;
	    affiliations.push_back(aff);

	    a0_norm_freq = 1.0;
	    calib = 1.0;
	    scaled_sens = 1.0;
	    rsptype = "";
	}
	else if( o->getBlockette52() ) {
	    b52 = *o->getBlockette52();
	    if(first_b52) {
		first_b52 = false;
		CssSiteClass *site = new CssSiteClass();
		stringcpy(site->sta, b50.station.c_str(), sizeof(site->sta));
		site->sta_quark = stringUpperToQuark(site->sta);
		site->ondate = 1000*b52.start.year + b52.start.doy;
		site->offdate = 1000*b52.end.year + b52.end.doy;
		if(site->ondate <= 0) site->ondate = -1;
		if(site->offdate <= 0) site->offdate = -1;
		site->lat = b52.latitude;
		site->lon = b52.longitude;
		site->elev = b52.elevation/1000.;
		stringcpy(site->staname, b50.name.c_str(), sizeof(site->staname));
		for(i = 0; i < (int)sites.size(); i++) {
		    if(site->sta_quark == sites[i]->sta_quark &&
			site->ondate == sites[i]->ondate) break;
		}
		if(i < (int)sites.size()) {
		    sites.removeAt(i);
//cout << "removing site " << site->sta << " " << site->ondate << endl;
		}
		sites.push_back(site);
	    }

	    CssSitechanClass *sitechan = new CssSitechanClass();
	    stringcpy(sitechan->sta, b50.station.c_str(),sizeof(sitechan->sta));
/*
	    snprintf(sitechan->chan, sizeof(sitechan->chan), "%s%s",
			b52.channel.c_str(), b52.location.c_str());
*/
	    snprintf(sitechan->chan, sizeof(sitechan->chan), "%s", b52.channel.c_str());
	    sitechan->sta_quark = stringUpperToQuark(sitechan->sta);
	    sitechan->chan_quark = stringUpperToQuark(sitechan->chan);
	    sitechan->ondate = 1000*b52.start.year + b52.start.doy;
	    chanid++;
	    sitechan->chanid = chanid;
	    sitechan->offdate = 1000*b52.end.year + b52.end.doy;
	    if(sitechan->ondate <= 0) sitechan->ondate = -1;
	    if(sitechan->offdate <= 0) sitechan->offdate = -1;
	    strcpy(sitechan->ctype, "n");
	    sitechan->edepth = b52.local_depth/1000.;
	    sitechan->hang = b52.azimuth;
	    sitechan->vang = b52.dip + 90.;
	    switch(sitechan->chan[0]) {
		case 'B': strcpy(sitechan->descrip, "broad-band"); break;
		case 'H': strcpy(sitechan->descrip, "high broad-band"); break;
		case 'E': strcpy(sitechan->descrip, "extremely high broad-band"); break;
		case 'M': strcpy(sitechan->descrip, "medium broad-band"); break;
		case 'S': strcpy(sitechan->descrip, "short period"); break;
		default:  strcpy(sitechan->descrip, "-"); break;
	    }
	    switch(sitechan->chan[2]) {
		case '1': strcat(sitechan->descrip, " one"); break;
		case '2': strcat(sitechan->descrip, " two"); break;
		case '3': strcat(sitechan->descrip, " tree"); break;
		case 'E': strcat(sitechan->descrip, " east"); break;
		case 'N': strcat(sitechan->descrip, " north"); break;
		case 'Z': strcat(sitechan->descrip, " vertical"); break;
	    }
	    if(b52.location.length() > 0) {
		int len = (int)strlen(sitechan->descrip);
		snprintf(sitechan->descrip+len, sizeof(sitechan->descrip)-len, " loc: %s", b52.location.c_str());
	    }
	    sitechan->putValue("location", b52.location);
	    sitechan->putValue("used", (long)0);
	    for(i = 0; i < (int)sitechans.size(); i++) {
		string location;
		sitechans[i]->getValue("location", location);
		if(!strcmp(sitechan->sta, sitechans[i]->sta) &&
		   !strcmp(sitechan->chan, sitechans[i]->chan) &&
		   !strcmp(b52.location.c_str(), location.c_str()) &&
		   sitechan->ondate == sitechans[i]->ondate) break;
	    }
	    if(i < (int)sitechans.size()) {
//cout << "removing sitechan " << sitechan->sta << " " << sitechan->chan << endl;
		sitechans.removeAt(i);
	    }
	    sitechans.push_back(sitechan);
	}
        else if( (b53 = o->getBlockette53()) ) {
	    a0_norm_freq = b53->norm_freq;
	    if(b53->zr.size() > 0) {
		Blockette34 *b34;
		if( (b34 = in.dictionary.getB34(b53->input_units)) && !strcasecmp(b34->name.c_str(), "M/S")) {
		    calib = 2.*M_PI*b53->norm_freq;
		}
		else if( (b34 = in.dictionary.getB34(b53->input_units)) && !strcasecmp(b34->name.c_str(), "M/S**2")) {
		    calib = 4.*M_PI*M_PI*b53->norm_freq*b53->norm_freq;
		}
		rsptype = "paz";
	    }
	}
        else if( (b55 = o->getBlockette55()) ) {
	    string input_units="", output_units="";
	    Blockette34 *b34;

	    if( (b34 = in.dictionary.getB34(b55->input_units)) ) input_units = b34->name;
	    if( (b34 = in.dictionary.getB34(b55->output_units)) ) output_units = b34->name;

	    if( !strcasecmp(input_units.c_str(), "M/S")) {
		calib = 2.*M_PI*a0_norm_freq;
	    }
	    else if( !strcasecmp(output_units.c_str(), "M/S**2")) {
		calib = 4.*M_PI*M_PI*a0_norm_freq*a0_norm_freq;
	    }
	    rsptype = "fap";
	}
        else if( (b58 = o->getBlockette58()) )
	{
	    if(b58->stage != 0) {
		scaled_sens *= b58->sensitivity;
	    }
	    else if(b58->stage == 0 ) {
		CssSensorClass *sensor = new CssSensorClass();
		stringcpy(sensor->sta, b50.station.c_str(), sizeof(sensor->sta));
		snprintf(sensor->chan, sizeof(sensor->chan), "%s", b52.channel.c_str());
		sensor->sta_quark = stringUpperToQuark(sensor->sta);
		sensor->chan_quark = stringUpperToQuark(sensor->chan);
		DateTime dt;
		dt.year = b52.start.year;
		dt.month = 1;
		dt.day = 1;
		dt.hour = 0;
		dt.minute = 0;
		dt.second = 0;
		sensor->time = timeDateToEpoch(&dt) + 24*60*(b52.start.doy-1);
		dt.year = b52.end.year;
		dt.month = 1;
		dt.day = 1;
		dt.hour = 0;
		dt.minute = 0;
		dt.second = 0;
		sensor->endtime = timeDateToEpoch(&dt) + 24*60*(b52.end.doy-1);
		sensor->chanid = chanid;
		sensor->inid = chanid;
		sensor->jdate = 1000*b52.end.year + b52.end.doy;
		if(sensor->jdate <= 0) sensor->jdate = -1;
		sensor->calratio = 1.0;
		sensor->calper = 1./a0_norm_freq;
		sensor->tshift = 0.0;
		strcpy(sensor->instant, "y");
		sensors.push_back(sensor);

		CssInstrumentClass instrument;
	    	instrument.inid = chanid;

		string insname = (b33 = in.dictionary.getB33(b52.instrument)) ? b33->description : "";
		stringcpy(instrument.insname, insname.c_str(), sizeof(instrument.insname));

		if(strstr(instrument.insname, "CMG-3T"))	strcpy(instrument.instype, "CMG-3T");
		else if(strstr(instrument.insname, "CMG3-T"))	strcpy(instrument.instype, "CMG3-T");
		else if(strstr(instrument.insname, "CMG3T"))	strcpy(instrument.instype, "CMG3T");
		else if(strstr(instrument.insname, "CMG40T"))	strcpy(instrument.instype, "CMG40T");
		else if(strstr(instrument.insname, "CMG-40T")) strcpy(instrument.instype, "CMG40T");
		else if(strstr(instrument.insname, "CMG3ESP_120"))   strcpy(instrument.instype, "CMG3E1");
		else if(strstr(instrument.insname, "CMG3ESP_60"))    strcpy(instrument.instype, "CMG3E6");
		else if(strstr(instrument.insname, "CMG3ESP_30"))    strcpy(instrument.instype, "CMG3E3");
		else if(strstr(instrument.insname, "KS-54000"))      strcpy(instrument.instype, "KS5400");
		else if(strstr(instrument.insname, "KS2000"))        strcpy(instrument.instype, "KS2000");
		else if(strstr(instrument.insname, "KS2K"))	      strcpy(instrument.instype, "KS2000");
		else if(strstr(instrument.insname, "Trillium 240"))  strcpy(instrument.instype, "Tril24");
		else if(strstr(instrument.insname, "Trillium 120P")) strcpy(instrument.instype, "Tril12");
		else if(strstr(instrument.insname, "STS2"))   strcpy(instrument.instype, "STS-2");
		else if(strstr(instrument.insname, "STS-2"))  strcpy(instrument.instype, "STS-2");
		else if(strstr(instrument.insname, "STS-1H")) strcpy(instrument.instype, "STS-1H");
		else if(strstr(instrument.insname, "STS-1V")) strcpy(instrument.instype, "STS-1V");
		else if(strstr(instrument.insname, "STS1"))   strcpy(instrument.instype, "STS1");

		instrument.band[0] = tolower(sensor->chan[0]);
		instrument.band[1] = '\0';
		strcpy(instrument.digital, "d");
		instrument.samprate = b52.sample_rate;
		if(calib*scaled_sens != 0.) {
		    instrument.ncalib = 1.e+09/(calib*scaled_sens);
		}
		else {
		    instrument.ncalib = 0.;
		}
		if(a0_norm_freq != 0.) {
		    instrument.ncalper = 1.0/a0_norm_freq;
		}
		else {
		    instrument.ncalper = -1.0;
		}
		strcpy(instrument.rsptype, rsptype.c_str());
		Resp resp;
		resp.b50 = b50;
		resp.b52 = b52;
		resp.instrument = instrument;
		responses.push_back(resp);
	    }
	}
        else if( (b71 = o->getBlockette71()) ) {
	    CssOriginClass *origin = new CssOriginClass();
	    origin->lat = b71->latitude;
	    origin->lon = b71->longitude;
	    origin->depth = b71->depth;
	    origin->time = b71->origin_time.epoch();
	    origin->jdate = timeEpochToJDate(origin->time);
	    origin->grn = b71->seismic_region;
	    origin->srn = b71->seismic_location;
	    for(i = 0; i < (int)b71->mag_type.size(); i++) {
		if(strstr(b71->mag_type[i].c_str(), "mb") ||
		    strstr(b71->mag_type[i].c_str(), "MB")) {
		    origin->mb = b71->magnitude[i];
		}
		else if(strstr(b71->mag_type[i].c_str(), "ms") ||
			strstr(b71->mag_type[i].c_str(), "MS")) {
		    origin->ms = b71->magnitude[i];
		}
		else if(strstr(b71->mag_type[i].c_str(), "ml") ||
			strstr(b71->mag_type[i].c_str(), "ML")) {
		    origin->ml = b71->magnitude[i];
		}
            }
	    origins.push_back(origin);
	}
	else if( (b72 = o->getBlockette72()) ) {
	    CssArrivalClass *arrival = new CssArrivalClass();
	    stringcpy(arrival->sta, b72->station.c_str(), sizeof(arrival->sta));
/*
            snprintf(arrival->chan, sizeof(arrival->chan), "%s%s",
                        b72->channel.c_str(), b72->location.c_str());
*/
	    snprintf(arrival->chan, sizeof(arrival->chan), "%s", b72->channel.c_str());
	    arrival->sta_quark = stringUpperToQuark(arrival->sta);
	    arrival->chan_quark = stringUpperToQuark(arrival->chan);
	    arrival->time = b72->time.epoch();
	    arrival->jdate = timeEpochToJDate(arrival->time);
	    stringcpy(arrival->iphase, b72->phase_name.c_str(), sizeof(arrival->iphase));
	    arrival->amp = b72->amplitude;
	    arrival->per = b72->period;
	    arrival->snr = b72->snr;
	    arrivals.push_back(arrival);
	}
	delete o;
    }

    for(i = 0; i < (int)in.stations.size(); i++) {
	for(int j = (int)in.stations[i]->channels.size()-1; j >= 0; j--) {
//printf("i = % d j = %d %s %s\n", i, j, in.stations[i]->b50.station.c_str(), in.stations[i]->channels[j]->b52.channel.c_str());
	    int julian_ondate = in.stations[i]->channels[j]->b52.start.year*1000 + in.stations[i]->channels[j]->b52.start.doy;
	    string s = Seed2CssResp::cssResponse(*in.stations[i], *in.stations[i]->channels[j], in.dictionary);
	    int chanid = -1;
	    for(int k = 0; k < (int)sitechans.size(); k++) {
		string location;
		sitechans[k]->getValue("location", location);
		if(!strcmp(in.stations[i]->b50.station.c_str(), sitechans[k]->sta) &&
		   !strcmp(in.stations[i]->channels[j]->b52.channel.c_str(), sitechans[k]->chan) &&
		   !strcmp(in.stations[i]->channels[j]->b52.location.c_str(), location.c_str()) &&
		   julian_ondate == sitechans[k]->ondate)
		{
		    long used = 0;
		    sitechans[k]->getValue("used", &used);
		    if(!used) {
			chanid = sitechans[k]->chanid;
			sitechans[k]->putValue("used", (long)1);
			break;
		    }
		}
	    }
	    if(chanid > -1) {
		int inid = -1;
		for(int k = 0; k < (int)sensors.size(); k++) {
		    if(sensors[k]->chanid == chanid) {
			inid = sensors[k]->inid;
		    }
		}
		if(inid != -1) {
		    for(int k = 0; k < (int)responses.size(); k++) {
			if(responses[k].instrument.inid == inid) {
			    responses[k].css_resp = s;
			}
		    }
		}
	    }
	}
    }
    return true;
}

bool SeedToCss::getWaveforms(string seedfile, cvector<CssWfdiscClass> &wfdiscs, vector<GSegment *> &segments)
{
    Seed *o;
    DataHeader *h;
    SeedData *sd;
    ifstream ifs;

    ifs.open(seedfile.c_str());
    if( !ifs.good() ) {
        char error[MAXPATHLEN+100];
        snprintf(error, sizeof(error), "SeedToCss::getWaveforms: cannot open %s", seedfile.c_str());
        logErrorMsg(LOG_WARNING, error);
        return false;
    }

    SeedInput in(&ifs);

    while( (o = in.readSeed()) ) {
	if( (sd = o->getSeedData()) ) {
	    if((int)sd->records.size() > 0) {
		GSegment *segment = SeedToCss::readSegment(sd, seedfile);
		CssWfdiscClass *w = new CssWfdiscClass();
		h = &sd->records[0].header;
		stringcpy(w->sta, h->station.c_str(), sizeof(w->sta));
		stringcpy(w->chan, h->channel.c_str(), sizeof(w->chan));
		w->sta_quark = stringUpperToQuark(w->sta);
		w->chan_quark = stringUpperToQuark(w->chan);
		w->time = h->startTime();
		w->endtime = sd->records.back().header.endTime();
		w->nsamp = sd->nsamples();
//w->samprate = h->sampleRate();
		w->samprate = sd->samprate();
		w->jdate = timeEpochToJDate(w->time);
		w->calib = sd->calib;
		w->calper = sd->calper;
		wfdiscs.push_back(w);
		segments.push_back(segment);
	    }
//	    delete sd;
	}
	delete o;
    }
    return true;
}

bool SeedToCss::convertToCss(string seedfile, string dir, string prefix, string respdir, string geotabledir, bool update, bool getdata)
{
    const char *err_msg;
    gvector<CssTableClass *> tables;
    string file, table_name;
    struct stat buf;

    cvector<CssAffiliationClass> affiliations;
    cvector<CssSiteClass> sites;
    cvector<CssSitechanClass> sitechans;
    cvector<CssSensorClass> sensors;
    vector<Resp> responses;
    cvector<CssOriginClass> origins;
    cvector<CssArrivalClass> arrivals;
    cvector<CssWfdiscClass> wfdiscs;
    vector<GSegment *> segments;
    map<int, int> chanids;
    map<int, int> inids;
    int wfid = 1;

    if(!stat(dir.c_str(), &buf)) {
	if(!S_ISDIR(buf.st_mode)) {
	    cerr << dir << " exists and is not a directory." << endl;
	    return false;
	}
    }
    else {
	if(mkdir(dir.c_str(), 0755)) {
	    cerr << "Cannot create " << dir << endl << strerror(errno) << endl;
	    return false;
	}
    }
    if(!stat(respdir.c_str(), &buf)) {
	if(!S_ISDIR(buf.st_mode)) {
	    cerr << respdir << " exists and is not a directory." << endl;
	    return false;
	}
    }
    else {
	if(mkdir(respdir.c_str(), 0755)) {
	    cerr << "Cannot create " << respdir << endl << strerror(errno) << endl;
	    return false;
	}
    }

    SeedToCss::getCssTables(seedfile, affiliations, sites, sitechans, sensors, responses, origins, arrivals);

    if(getdata) {
	if(!SeedToCss::getWaveforms(seedfile, wfdiscs, segments)) return false;
    }

    if((int)affiliations.size() > 0) {
	table_name = affiliations[0]->getName();
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	if(update) {
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret <= -2) {
		cerr << err_msg << endl;
		return false;
	    }
	    if( !addAffiliations(affiliations, tables) || !writeTable(tables, file)) return false;
	}
	else {
	    if( !writeTable(affiliations, file) ) return false;
	}
    }

    if((int)sites.size() > 0) {
	table_name = sites[0]->getName();
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	if(update) {
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret <= -2) {
		cerr << err_msg << endl;
		return false;
	    }
	    if( !addSites(sites, tables) || !writeTable(tables, file)) return false;
	}
	else {
	    if( !writeTable(sites, file) ) return false;
	}
    }
    tables.clear();

    if((int)sitechans.size() > 0) {
	table_name = sitechans[0]->getName();
	long max_chanid = 0;
	if( !geotabledir.empty() ) {
	    file = geotabledir + "/static." + table_name;
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret >= 0) {
		for(int i = 0; i < (int)tables.size(); i++) {
		    CssSitechanClass *s = (CssSitechanClass *)tables[i];
		    if(max_chanid < s->chanid) max_chanid = s->chanid;
		}
	    }
	}
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	if(update) {
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret <= -2) {
		cerr << err_msg << endl;
		return false;
	    }
	    if( !addSitechans(sitechans, tables, chanids, max_chanid) || !writeTable(tables, file) ) return false;
	}
	else {
	    if( !writeTable(sitechans, file) ) return false;
	}
    }
    tables.clear();

    if((int)sensors.size() > 0) {
	table_name = sensors[0]->getName();
	long max_inid = 0;
	if( !geotabledir.empty() ) {
	    file = geotabledir + "/static." + table_name;
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret >= 0) {
		for(int i = 0; i < (int)tables.size(); i++) {
		    CssSensorClass *s = (CssSensorClass *)tables[i];
		    if(max_inid < s->inid) max_inid = s->inid;
		}
	    }
	}
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	if(update) {
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret <= -2) {
		cerr << err_msg << endl;
		return false;
	    }
	    if( !addSensors(sensors, tables, chanids, inids, max_inid) || !writeTable(tables, file) ) return false;
	}
	else {
	    if( !writeTable(sensors, file) ) return false;
	}
    }
    tables.clear();

    if((int)responses.size() > 0) {
	table_name = responses[0].instrument.getName();
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	if(update) {
	    int ret = CssTableClass::readFile(file, table_name, tables, &err_msg);
	    if(ret <= -2) {
		cerr << err_msg << endl;
		return false;
	    }
	    if( !addInstruments(responses, tables, respdir, inids) || !writeTable(tables, file) ) return false;
	}
	else {
	    if( !writeTable(responses, file, respdir) ) return false;
	}
    }

    if(getdata && wfdiscs.size() > 0) {
	const char *err_msg;
	table_name = wfdiscs[0]->getName();
	string dfile = prefix + ".w";
	file = prefix + "." + table_name;
	if(dir.length() > 0) file = dir + "/" + file;

	FILE *fp;
	if((fp = fopen(file.c_str(), "w")) == NULL) {
	    fprintf(stderr, "Cannot open %s\n%s\n", file.c_str(), strerror(errno));
	    return 1;
	}
	FILE *fpw;
	file = prefix + ".w";
	if(dir.length() > 0) file = dir + "/" + file;
	if((fpw = fopen(file.c_str(), "w")) == NULL) {
	    fprintf(stderr, "Cannot open %s\n%s\n", file.c_str(), strerror(errno));
	    return 1;
	}
	union { char a[4]; int i; } e1;
	e1.a[0] = 0; e1.a[1] = 0;
	e1.a[2] = 0; e1.a[3] = 1;
	bool big_endian = (e1.i == 1) ? true : false;
	string datatype = big_endian ? "t4" : "f4";

	for(int i = 0; i < (int)wfdiscs.size(); i++) {
	    int j;
	    // fill in wfdisc.instype
	    for(j = 0; j < (int)sensors.size(); j++) {
		if(!strcmp(wfdiscs[i]->sta, sensors[j]->sta) && 
		   !strcmp(wfdiscs[i]->chan, sensors[j]->chan) &&
		    wfdiscs[i]->time >= sensors[j]->time &&
		    wfdiscs[i]->time <= sensors[j]->endtime) break;
	    }
	    if(j < (int)sensors.size()) {
		for(int k = 0; k < (int)responses.size(); k++) {
		    if(responses[k].instrument.inid == sensors[j]->inid) {
			strcpy(wfdiscs[i]->instype, responses[k].instrument.instype);
			break;
		    }
		}
		wfdiscs[i]->chanid = sensors[j]->chanid;
	    }
	    strcpy(wfdiscs[i]->datatype, datatype.c_str());
	    strcpy(wfdiscs[i]->dir, ".");
	    strcpy(wfdiscs[i]->dfile, dfile.c_str());
	    wfdiscs[i]->foff = ftell(fpw);
	    wfdiscs[i]->wfid = wfid++;

	    if(wfdiscs[i]->write(fp, &err_msg) != 0) {
		cerr << err_msg << endl;
		fclose(fp);
		for(int k = 0; k < (int)segments.size(); k++) delete segments[k];
		return false;
	    }
	    if(fwrite(segments[i]->data, sizeof(float), (size_t)wfdiscs[i]->nsamp, fpw) != (size_t)wfdiscs[i]->nsamp) {
		cerr << "write error: " << file << " " << strerror(errno) << endl;
		fclose(fp);
		fclose(fpw);
		for(int k = 0; k < (int)segments.size(); k++) delete segments[k];
		return false;
	    }
	}
	fclose(fp);
	fclose(fpw);
    }
    for(int k = 0; k < (int)segments.size(); k++) delete segments[k];

    return true;
}

bool SeedToCss::writeTable(gvector<CssTableClass *> &tables, string file)
{
    const char *err_msg;
    FILE *fp;

    if((int)tables.size() > 0) {
	if((fp = fopen(file.c_str(), "w")) == NULL) {
	    fprintf(stderr, "Cannot open %s\n%s\n", file.c_str(), strerror(errno));
	    return 1;
	}
	for(int i = 0; i < (int)tables.size(); i++) {
	    if(tables[i]->write(fp, &err_msg) != 0) {
		cerr << err_msg << endl;
		fclose(fp);
		return false;
	    }
	}
	fclose(fp);
    }
    return true;
}

bool SeedToCss::writeTable(vector<Resp> &v, string file, string respdir)
{
    FILE *fp, *resp_fp;
    const char *err_msg;
    char resp_file[MAXPATHLEN+1];
    char path[MAXPATHLEN+1];

    //resp.IU_TARA_BH1_10_2011262

    if((int)v.size() > 0) {
	if((fp = fopen(file.c_str(), "w")) == NULL) {
	    fprintf(stderr, "Cannot open %s\n%s\n", file.c_str(), strerror(errno));
	    return 1;
	}
	if((int)v.size() > 1) printf("writing: %d records to %s\n", (int)v.size(), file.c_str());
	else printf("writing: %d record  to %s\n", (int)v.size(), file.c_str());

	for(int i = 0; i < (int)v.size(); i++) {
	    if(v[i].instrument.write(fp, &err_msg) != 0) {
		cerr << err_msg << endl;
		fclose(fp);
		return false;
	    }

	    snprintf(resp_file, sizeof(resp_file), "resp.%s_%s_%s_%s_%4d%03d.txt",
		v[i].b50.network.c_str(), v[i].b50.station.c_str(), v[i].b52.channel.c_str(),
		v[i].b52.location.c_str(), v[i].b52.start.year, v[i].b52.start.doy);

	    snprintf(v[i].instrument.dir, sizeof(v[i].instrument.dir), "%s", respdir.c_str());
	    snprintf(v[i].instrument.dfile, sizeof(v[i].instrument.dfile), "%s", resp_file);

	    snprintf(path, sizeof(path), "%s/%s", respdir.c_str(), resp_file);

	    if((resp_fp = fopen(path, "w")) == NULL) {
		fprintf(stderr, "Cannot open %s\n%s\n", path, strerror(errno));
		fclose(fp);
		return 1;
	    }
	    fprintf(resp_fp, "%s\n", v[i].css_resp.c_str());
	    fclose(resp_fp);
	}
	fclose(fp);
    }
    return true;
}

bool SeedToCss::addAffiliations(cvector<CssAffiliationClass> &affiliations, gvector<CssTableClass *> &tables)
{
    for(int i = 0; i < (int)affiliations.size(); i++) {
	int j;
	// check if there is an existing affiliation with the same net and sta
	for(j = 0; j < (int)tables.size(); j++) {
	    CssAffiliationClass *a = (CssAffiliationClass *)tables[j];
	    if(affiliations[i]->net_quark == a->net_quark && affiliations[i]->sta_quark == a->sta_quark) break;
	}
	if(j == (int)tables.size()) {
	   // add affiliations[i] to the file
	    tables.push_back(affiliations[i]);
	}
    }
    return true;
}

bool SeedToCss::addSites(cvector<CssSiteClass> &sites, gvector<CssTableClass *> &tables)
{
    int year, doy, epoch_day;
    double epoch_time;

    for(int i = 0; i < (int)sites.size(); i++) {
	int j;
	// check if there is a site with the same sta and ondate
	for(j = 0; j < (int)tables.size(); j++) {
	    CssSiteClass *site = (CssSiteClass *)tables[j];
	    if(site->sta_quark == sites[i]->sta_quark && site->ondate == sites[i]->ondate) break;
	}
	if(j < (int)tables.size()) {
	    cerr << "Found conflicting site: sta: " << sites[i]->sta << " ondate: " << sites[i]->ondate << endl;
	    return false;
	}

	// check for a site in the table that has overlapping on-periods and change the offdate of the
	// existing or the new site record, if necessary
	for(j = 0; j < (int)tables.size(); j++)
	    if( ((CssSiteClass *)tables[j])->sta_quark == sites[i]->sta_quark )
	{
	    CssSiteClass *site = (CssSiteClass *)tables[j];

	    // check for overlapping on-periods
	    if(site->ondate <= sites[i]->ondate && (site->offdate <= 0 || site->offdate >= sites[i]->ondate))
	    {
		// set current site->offdate = new site->ondate - 1
		year = sites[i]->ondate/1000;
		doy = sites[i]->ondate - 1000*year;
		epoch_day = (year - 1970)*365 + doy - 1;
		epoch_time = (epoch_day-1)*24*60*60;
		site->offdate = timeEpochToJDate(epoch_time);
	    }
	    else if(site->ondate <= sites[i]->offdate && (site->offdate <= 0 || site->offdate >= sites[i]->offdate))
	    {
		// set new site->offdate = current site->ondate - 1
		year = site->ondate/1000;
		doy = site->ondate - 1000*year;
		epoch_day = (year - 1970)*365 + doy - 1;
		epoch_time = (epoch_day-1)*24*60*60;
		sites[i]->offdate = timeEpochToJDate(epoch_time);
	    }
	}
	// add sites[i] to the file
	tables.push_back(sites[i]);
    }
    return true;
}

bool SeedToCss::addSitechans(cvector<CssSitechanClass> &sitechans, gvector<CssTableClass *> &tables, map<int, int> &chanids, long max_chanid)
{
    int year, doy, epoch_day;
    double epoch_time;

    for(int i = 0; i < (int)sitechans.size(); i++) {
	int j;
	// check if there is a sitechan with the same sta,chan, ondate, offdate and descrip
	for(j = 0; j < (int)tables.size(); j++) {
	    CssSitechanClass *sitechan = (CssSitechanClass *)tables[j];
	    if(sitechan->sta_quark == sitechans[i]->sta_quark &&
		sitechan->chan_quark == sitechans[i]->chan_quark &&
		sitechan->ondate == sitechans[i]->ondate &&
		sitechan->offdate == sitechans[i]->offdate &&
		!strcmp(sitechan->descrip, sitechans[i]->descrip) ) break;
	}
	if(j < (int)tables.size()) {
	    cerr << "Found conflicting sitechan: sta: " << sitechans[i]->sta << " chan: " << sitechans[i]->chan
			<< " ondate: " << sitechans[i]->ondate << endl;
	    return false;
	}

	// check for a sitechan in the table that has overlapping on-periods and change the offdate of the
	// existing or the new sitechan record, if necessary
	for(j = 0; j < (int)tables.size(); j++)
	    if( ((CssSitechanClass *)tables[j])->sta_quark == sitechans[i]->sta_quark &&
		((CssSitechanClass *)tables[j])->chan_quark == sitechans[i]->chan_quark )
	{
	    CssSitechanClass *sitechan = (CssSitechanClass *)tables[j];

	    // check for overlapping on-periods
	    if(sitechan->ondate <= sitechans[i]->ondate && (sitechan->offdate <= 0 || sitechan->offdate >= sitechans[i]->ondate))
	    {
		// set current sitechan->offdate = new sitechan->ondate - 1
		year = sitechans[i]->ondate/1000;
		doy = sitechans[i]->ondate - 1000*year;
		epoch_day = (year - 1970)*365 + doy - 1;
		epoch_time = (epoch_day-1)*24*60*60;
		sitechan->offdate = timeEpochToJDate(epoch_time);
	    }
	    else if(sitechan->ondate <= sitechans[i]->offdate && (sitechan->offdate <= 0 
			|| sitechan->offdate >= sitechans[i]->offdate))
	    {
		// set new sitechan->offdate = current sitechan->ondate - 1
		year = sitechan->offdate/1000;
		doy = sitechan->offdate - 1000*year;
		epoch_day = (year - 1970)*365 + doy - 1;
		epoch_time = (epoch_day-1)*24*60*60;
		sitechans[i]->offdate = timeEpochToJDate(epoch_time);
	    }
	}

	// check if the new sitechan[i].chanid is unique
	for(j = 0; j < (int)tables.size(); j++) {
	    if( ((CssSitechanClass *)tables[j])->chanid == sitechans[i]->chanid ) break;
	}
	if(j < (int)tables.size() || max_chanid > 0) {
	    // need to change the new chanid, find the current maximum chanid
	    int chanid = max_chanid;
	    for(j = 0; j < (int)tables.size(); j++) {
		if( ((CssSitechanClass *)tables[j])->chanid > chanid) chanid = ((CssSitechanClass *)tables[j])->chanid;
	    }
	    chanid++;
	    // set new chanid and put it in the map to use in the sensor table
	    chanids[sitechans[i]->chanid] = chanid;
	    sitechans[i]->chanid = chanid;
	}

	// add sitechans[i] to the file
	tables.push_back(sitechans[i]);
    }
    return true;
}

bool SeedToCss::addSensors(cvector<CssSensorClass> &sensors, gvector<CssTableClass *> &tables, map<int, int> &chanids,
			map<int, int> &inids, long max_inid)
{
    map<int, int>::iterator it;
    int chanid;

    for(int i = 0; i < (int)sensors.size(); i++) {
	int j;
	chanid = sensors[i]->chanid;
	if((it = chanids.find(chanid)) != chanids.end()) chanid = (*it).second;
	    
	// check if there is a sensor with the same chanid
	// this should not happen, since we have already looked at all of the sitechan chanids
	for(j = 0; j < (int)tables.size(); j++) {
	    CssSensorClass *sensor = (CssSensorClass *)tables[j];
	    if(sensor->chanid == chanid) break;
	}
	if(j < (int)tables.size()) {
	    cerr << "Found conflicting sensor chanid: " << chanid << endl;
	    return false;
	}

	// set chanid to the map value that was set in add sitechans
	sensors[i]->chanid = chanid;

	// check if the new sensor[i].inid is unique
	for(j = 0; j < (int)tables.size(); j++) {
	    if( ((CssSensorClass *)tables[j])->inid == sensors[i]->inid ) break;
	}
	if(j < (int)tables.size() || max_inid > 0) {
	    // need to change the new inid, find the current maximum inid
	    int inid = max_inid;
	    for(j = 0; j < (int)tables.size(); j++) {
		if( ((CssSensorClass *)tables[j])->inid > inid) inid = ((CssSensorClass *)tables[j])->inid;
	    }
	    inid++;
	    // set new sensor.inid and put it in the map to use in the instrument table
	    inids[sensors[i]->inid] = inid;
	    sensors[i]->inid = inid;
	}

	// add sensor[i] to the file
	tables.push_back(sensors[i]);
    }
    return true;
}

bool SeedToCss::addInstruments(vector<Resp> &responses, gvector<CssTableClass *> &tables, string respdir, map<int, int> &inids)
{
    char resp_file[MAXPATHLEN+1];
    char path[MAXPATHLEN+1];
    map<int, int>::iterator it;
    int inid;
    FILE *resp_fp;

    if(respdir.length() == 0) respdir = ".";

    for(int i = 0; i < (int)responses.size(); i++) {
	int j;
	inid = responses[i].instrument.inid;
	if((it = inids.find(inid)) != inids.end()) inid = (*it).second;
	    
	// check if there is an instrument with the same inid
	// this should not happen, since we have already looked at all of the sensor inids
	for(j = 0; j < (int)tables.size(); j++) {
	    CssInstrumentClass *instrument = (CssInstrumentClass *)tables[j];
	    if(instrument->inid == inid) break;
	}
	if(j < (int)tables.size()) {
	    cerr << "Found conflicting instrument inid: " << inid << endl;
	    return false;
	}

	// set inid to the map value that was set in add sensors
	responses[i].instrument.inid = inid;

	// check if the new instrument[i].inid is unique
	for(j = 0; j < (int)tables.size(); j++) {
	    if( ((CssInstrumentClass *)tables[j])->inid == responses[i].instrument.inid ) break;
	}
	if(j < (int)tables.size()) {
	    // need to change the new inid, find the current maximum inid
	    int inid = 0;
	    for(j = 0; j < (int)tables.size(); j++) {
		if( ((CssInstrumentClass *)tables[j])->inid > inid) inid = ((CssInstrumentClass *)tables[j])->inid;
	    }
	    inid++;
	    // set new instrument.inid
	    inids[responses[i].instrument.inid] = inid;
	    responses[i].instrument.inid = inid;
	}

	// make the new response filename
	snprintf(resp_file, sizeof(resp_file), "resp.%s_%s_%s_%s_%4d%03d.txt", responses[i].b50.network.c_str(),
		responses[i].b50.station.c_str(), responses[i].b52.channel.c_str(),
                responses[i].b52.location.c_str(), responses[i].b52.start.year, responses[i].b52.start.doy);

	snprintf(responses[i].instrument.dir, sizeof(responses[i].instrument.dir), "%s", respdir.c_str());
	snprintf(responses[i].instrument.dfile, sizeof(responses[i].instrument.dfile), "%s", resp_file);

	snprintf(path, sizeof(path), "%s/%s", respdir.c_str(), resp_file);

	// write the response file
	if((resp_fp = fopen(path, "w")) == NULL) {
	    fprintf(stderr, "Cannot open %s\n%s\n", path, strerror(errno));
	    return false;
	}
	fprintf(resp_fp, "%s\n", responses[i].css_resp.c_str());
	fclose(resp_fp);

	// add the new instrument to the file
	CssInstrumentClass *ins = new CssInstrumentClass();
	responses[i].instrument.copyTo(ins);
	tables.push_back(ins);
    }
    return true;
}

GSegment * SeedToCss::readSegment(SeedData *sd, string read_path, double start_time, double end_time)
{
    int npts, start, n;
    double tbeg, tdel;
    float *data;
    GSegment *s;
    ifstream ifs;

    start = 0;
    if(start_time > sd->startTime()) {
	start = (int)((start_time - sd->startTime())*sd->samprate()+.5);
    }

    if(end_time > sd->endTime()) {
	npts = sd->nsamples() - start;
    }
    else {
	npts = (int)(((end_time - sd->startTime())*sd->samprate()+.5) - start + 1);
    }
    if(start + npts > sd->nsamples()) npts = sd->nsamples() - start;

    if(npts <= 0) return NULL;

    tdel = 1./sd->samprate();
    tbeg = sd->startTime() + start*tdel;

    ifs.open(read_path.c_str());

    if( !ifs.good() ) {
	char error[MAXPATHLEN+20];
	snprintf(error, sizeof(error),"seed: cannot open %s",read_path.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }

    data = (float *)malloc(sd->nsamples()*sizeof(float));

    n = sd->readData(&ifs, data, sd->nsamples());

    if(start+npts > n) {
	logErrorMsg(LOG_WARNING, "SeedToCss::readSegment error");
	npts = n - start;
	if(npts <= 0) {
	    free(data);
	    return NULL;
        }
    }

    s = new GSegment(data+start, npts, tbeg, tdel, sd->calib, sd->calper);
    free(data);
    return s;
}

