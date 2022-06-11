#ifndef _SEED_SOURCE_H
#define _SEED_SOURCE_H

#include <string.h>
#include "TableSource.h"
#include "seed/SeedData.h"
#include "seed/SeedInput.h"
#include "gobject++/SegmentInfo.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

class ChannelResponse;

/*
 *  @ingroup libgx
 */
class SeedSource : public TableSource
{
    public:
	SeedSource(const string &name, const string &file);
	~SeedSource(void);

	// DataSource interface
	gvector<SegmentInfo *> *getSegmentList(void);
	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	bool reread(GTimeSeries *ts);
        int getTable(const string &cssTableName, gvector<CssTableClass *> &v);

	vector<int> selected;
	vector<int> selected_records;
/*
        cvector<CssSiteClass> * getSiteTable(void);
        cvector<CssSitechanClass> * getSitechanTable(void);
        cvector<CssAffiliationClass> * getAffiliationTable(void);

	int getTable(cvector<CssAffiliationClass> &v) {
		return getTable(cssAffiliation, v); }
	int getTable(cvector<CssAmpdescriptClass> &v) {
		return getTable(cssAmpdescript, v); }
	int getTable(cvector<CssAmplitudeClass> &v) {
		return getTable(cssAmplitude, v); }
	int getTable(cvector<CssArrivalClass> &v) {
		return getTable(cssArrival, v); }
	int getTable(cvector<CssAssocClass> &v) {
		return getTable(cssAssoc, v); }
	int getTable(cvector<CssInfraFeaturesClass> &v) {
		return getTable(cssInfraFeatures, v); }
	int getTable(cvector<CssHydroFeaturesClass> &v) {
		return getTable(cssHydroFeatures, v); }
	int getTable(cvector<CssOriginClass> &v) {
		return getTable(cssOrigin, v); }
	int getTable(cvector<CssOrigerrClass> &v) {
		return getTable(cssOrigerr, v); }
	int getTable(cvector<CssParrivalClass> &v) {
		return getTable(cssParrival, v); }
	int getTable(cvector<CssNetmagClass> &v) {
		return getTable(cssNetmag, v); }
	int getTable(cvector<CssSiteClass> &v) {
		return getTable(cssSite, v); }
	int getTable(cvector<CssSitechanClass> &v) {
		return getTable(cssSitechan, v); }
	int getTable(cvector<CssStamagClass> &v) {
		return getTable(cssStamag, v); }
	int getTable(cvector<CssStassocClass> &v) {
		return getTable(cssStassoc, v); }
	int getTable(cvector<CssWfdiscClass> &v) {
		return getTable(cssWfdisc, v); }
	int getTable(cvector<CssWftagClass> &v) {
		return getTable(cssWftag, v); }
	int getTable(cvector<CssXtagClass> &v) {
		return getTable(cssXtag, v); }
*/

	void listWaveforms();
	void listRecords();
	void listBlockettes();
	void removeDataReceiver(DataReceiver *owner);

	void saveAsCSS(string file);

/*
	vector<Response *> * channelResponse(GTimeSeries *ts,
			bool print_err=true);
*/

	static bool isSeedFile(const string &path);

    protected:
	string read_path;
	vector<SeedData *> seed_data;
	gvector<gvector<CssTableClass *> *> seed_tables;

/*
	cvector<CssSiteClass> sites;
	cvector<CssSitechanClass> sitechans;
	cvector<CssAffiliationClass> affiliations;
*/
	vector<ChannelResponse *> responses;

	void clear(void);

	void addSeg(gvector<SegmentInfo *> *segs, SeedData *sd, int path_quark);
	void listSeedData(SeedData &sd);
	void listBlocketteInfo(SeedInput &in, Seed *o);
	void listStation(SeedInput &in, Blockette50 *b50);
	void listChannel(SeedInput &in, const string &sta, Blockette52 *b52);
	void listComment(SeedInput &in, Blockette31 *b31);
	void listAbbreviation(SeedInput &in, Blockette33 *b33);
	void listUnit(SeedInput &in, Blockette34 *b34);
	void listDataRec(DataRecord &dr);

	void storeResponse(Station &sta, Channel &chan, Dictionary &d, const string &path);
	void saveResponse(Station &sta, Channel &chan, Dictionary &d, const string &path);
	void saveStaAsCss(Station &sta, Channel &chan, Dictionary &d, const string &path);

	void process53(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette53 &b53, double *calib);
	void process54(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette54 &b54,
			Blockette54 *b54b, Channel &chan);
	void process55(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette55 &b55,
			double norm_freq, double *calib);
	void process57(Dictionary &d, ostringstream &os_header, Blockette57 &b57);
	void process58(ostringstream &os_start_header, Blockette58 &b58, int stage, double *scaled_sens,
			double sensitivity_freq);
	void process61(Dictionary &d, ostringstream &os_header, ostringstream &os_data, Blockette61 &b61,
			string b53_type, Channel &chan);

    private:

};

#endif
