#ifndef _ASC_SOURCE_H
#define _ASC_SOURCE_H

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include <string.h>
#include "TableSource.h"
#include "gobject++/SegmentInfo.h"
#include "gobject++/GTimeSeries.h"
#include "gobject++/CssTables.h"

enum AscMode {
    READING_WFDISC,
    READING_DATA,
    READING_TABLE,
    READING_NONE
};

class AscData;

/*
 *  @ingroup libgx
 */
class AscSource : public TableSource
{
    public:
	AscSource(const string &name, const string &file);
	~AscSource(void);

	// DataSource interface
	gvector<SegmentInfo *> *getSegmentList(void);
	bool makeTimeSeries(SegmentInfo *s, double tbeg, double tend,
		int pts, GTimeSeries **ts, const char **err_msg);
	bool reread(GTimeSeries *ts);
        int getTable(const string &cssTableName, gvector<CssTableClass *> &v);
        cvector<CssSiteClass> * getSiteTable(void);
        cvector<CssSitechanClass> * getSitechanTable(void);
        cvector<CssAffiliationClass> * getAffiliationTable(void);
	void removeDataReceiver(DataReceiver *owner);

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

	static bool isAscFile(const string &path);
	static bool output(const string &prefix, const string &access,
		gvector<Waveform *> &cd_list, const string &remark,
		bool raw);
	static bool write_ts(FILE *fp, GTimeSeries *ts, bool remove_calib);

    protected:
	string read_path;
#ifdef HAVE_LIBZ
	gzFile  zfp;
#else
	FILE *fp;
#endif
	enum AscMode mode;
	vector<AscData *> asc_data;
	gvector<gvector<CssTableClass *> *> asc_tables;
	cvector<CssSiteClass> sites;
	cvector<CssSitechanClass> sitechans;
	cvector<CssAffiliationClass> affiliations;
	long line_no;
	bool last_newline;
//	bool sac_ascii;
//	double time, samprate, lat, lon;

	bool openFile(const string &path);
	bool readFile(void);
	bool closeFile(void);
	void clear(void);
	bool getPhrase(char *phrase, int len);
	bool readWfdiscValue(const string &name, const string &value);
	bool readTableValue(CssTableClass *css, const string &name,
			const string &value);
	void storeDataValue(double d);
#ifdef HAVE_LIBZ
	int readChar(void) { return gzgetc(zfp); }
#else
	int readChar(void) { return getc(fp); }
#endif
	GSegment * readSeg(SegmentInfo *s, AscData *asc,
		double start_time, double end_time);

    private:

};

#endif
