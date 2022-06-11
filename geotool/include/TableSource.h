#ifndef _TABLE_SOURCE_H
#define _TABLE_SOURCE_H

#include <string.h>
#include <vector>
#include "BasicSource.h"
#include "FFDatabase.h"

#ifdef HAVE_LIBODBC
#include "FixBool.h"
#include "libgdb.h"
#endif

/** An interface for subclasses that support CSS table I/O.
 *  @ingroup libgx
 */
class TableSource : public BasicSource
{
    public:
	TableSource(const string &name);
	~TableSource(void);

	bool openPrefix(const string &prefix);
	bool openFFDB(const string &parameter_root, const string &segment_root,
		const string &directory_structure, double directory_duration);
	bool openODBC(const string &data_source, const string &user,
		const string &password);
	void closeConnection(void);
	virtual void query(const string &);
	void queryAllPrefixTables(const string &wfdisc_query="");
	bool queryPrefixTables(const string &css_table_name, const string &query);
	bool queryFFDBTables(const string &css_table_name, const string &query);
	bool queryODBCTables(const string &css_table_name, const string &query);
	void clearTable(const string &table_name);

	int getTable(const string &cssTableName, gvector<CssTableClass *> &table);
	int getTable(cvector<CssAffiliationClass> &v) { return getTable(cssAffiliation, v); }
	int getTable(cvector<CssAmpdescriptClass> &v) { return getTable(cssAmpdescript, v); }
	int getTable(cvector<CssAmplitudeClass> &v) { return getTable(cssAmplitude, v); }
	int getTable(cvector<CssArrivalClass> &v) { return getTable(cssArrival, v); }
	int getTable(cvector<CssAssocClass> &v) { return getTable(cssAssoc, v); }
	int getTable(cvector<CssGregionClass> &v) { return getTable(cssGregion, v); }
	int getTable(cvector<CssInfraFeaturesClass> &v) { return getTable(cssInfraFeatures, v); }
	int getTable(cvector<CssHydroFeaturesClass> &v) { return getTable(cssHydroFeatures, v); }
	int getTable(cvector<CssOriginClass> &v) { return getTable(cssOrigin, v); }
	int getTable(cvector<CssOrigerrClass> &v) { return getTable(cssOrigerr, v); }
	int getTable(cvector<CssParrivalClass> &v) {return getTable(cssParrival, v);}
	int getTable(cvector<CssNetmagClass> &v) { return getTable(cssNetmag, v); }
	int getTable(cvector<CssSiteClass> &v) { return getTable(cssSite, v); }
	int getTable(cvector<CssSitechanClass> &v) {return getTable(cssSitechan, v);}
	int getTable(cvector<CssStamagClass> &v) { return getTable(cssStamag, v); }
	int getTable(cvector<CssStassocClass> &v) { return getTable(cssStassoc, v); }
	int getTable(cvector<CssWfdiscClass> &v) { return getTable(cssWfdisc, v); }
	int getTable(cvector<CssWftagClass> &v) { return getTable(cssWftag, v); }
	int getTable(cvector<CssXtagClass> &v) { return getTable(cssXtag, v); }

	int getSelectedTable(const string &cssTableName,gvector<CssTableClass *> &table);
	int getSelectedTable(cvector<CssAffiliationClass> &v) { return getSelectedTable(cssAffiliation, v); }
	int getSelectedTable(cvector<CssAmpdescriptClass> &v) { return getSelectedTable(cssAmpdescript, v); }
	int getSelectedTable(cvector<CssAmplitudeClass> &v) { return getSelectedTable(cssAmplitude, v); }
	int getSelectedTable(cvector<CssArrivalClass> &v) { return getSelectedTable(cssArrival, v); }
	int getSelectedTable(cvector<CssAssocClass> &v) { return getSelectedTable(cssAssoc, v); }
	int getSelectedTable(cvector<CssGregionClass> &v) { return getSelectedTable(cssGregion, v); }
	int getSelectedTable(cvector<CssOriginClass> &v) { return getSelectedTable(cssOrigin, v); }
	int getSelectedTable(cvector<CssOrigerrClass> &v) { return getSelectedTable(cssOrigerr, v); }
	int getSelectedTable(cvector<CssParrivalClass> &v) { return getSelectedTable(cssParrival, v); }
	int getSelectedTable(cvector<CssNetmagClass> &v) { return getSelectedTable(cssNetmag, v); }
	int getSelectedTable(cvector<CssSiteClass> &v) { return getSelectedTable(cssSite, v); }
	int getSelectedTable(cvector<CssSitechanClass> &v) { return getSelectedTable(cssSitechan, v); }
	int getSelectedTable(cvector<CssStamagClass> &v) { return getSelectedTable(cssStamag, v); }
	int getSelectedTable(cvector<CssStassocClass> &v) { return getSelectedTable(cssStassoc, v); }
	int getSelectedTable(cvector<CssWfdiscClass> &v) { return getSelectedTable(cssWfdisc, v); }
	int getSelectedTable(cvector<CssWftagClass> &v) { return getSelectedTable(cssWftag, v); }

	void selectTables(const string &cssTableName, vector<int> &indices);

	cvector<CssAffiliationClass> * getAffiliationTable(void) {
		return (cvector<CssAffiliationClass> *)findTable(cssAffiliation); }
	cvector<CssSiteClass> * getSiteTable(void) {
		return (cvector<CssSiteClass> *)findTable(cssSite); }
	cvector<CssSitechanClass> * getSitechanTable(void) {
		return (cvector<CssSitechanClass> *)findTable(cssSitechan); }

	void getNetworkTables(void) {
	    if(!findTable(cssAffiliation)) {
		query("affiliation select * from affiliation");
	    }
	    if(!findTable(cssSitechan)) {
		query("sitechan select * from sitechan");
	    }
            if(!findTable(cssSite)) {
		query("site select * from site");
	    }
	}
	string getNet(const string &sta);
	CssSiteClass *getSite(const string &sta, int jdate);
	CssSitechanClass *getSitechan(const string &sta, const string &chan, int jdate);
	void removeDataReceiver(DataReceiver *owner);

	void copyFrom(TableSource *from);

	static TableSource * defaultSource(void);

	bool verbose;

    protected:
	gvector<CssTableClass *> * findTable(const string &css_table_name);
	char *checkPrefix(const string &file_prefix);
	void storeRecords(gvector<CssTableClass *> &v);
	bool addTables(gvector<CssTableClass *> &v);

	gvector<gvector<CssTableClass *> *> tables;

    private:
	void setFFDBIds(gvector<CssTableClass *> &records);
	void setODBCIds(gvector<CssTableClass *> &records);

	enum SourceType {
	    DATA_SOURCE_NONE,
	    DATA_SOURCE_PREFIX,
	    DATA_SOURCE_FFDB,
	    DATA_SOURCE_ODBC
	};
	SourceType	source_type;
	FFDatabase	*ffdb;
#ifdef HAVE_LIBODBC
	SQLHDBC		hdbc;
#endif
};

#endif
