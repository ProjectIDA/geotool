#ifndef _LOCATE_H
#define _LOCATE_H

#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "widget/Parser.h"
#include "motif++/MotifDecs.h"
#include "widget/TabClass.h"
#include "widget/Table.h"
#include "widget/TableListener.h"
#include "Details.h"
#include "MultiTable.h"
#include "gobject++/gvector.h"
#include "gobject++/CssTables.h"

/** @defgroup libglc plugin Locate Event
 */

typedef void (*LocInitFunction)(void);

typedef int (*LocFunction)(CssTableClass *css, int num_phases, char **phases,
	cvector<CssSiteClass> &css_site, cvector<CssArrivalClass> &css_arrival,
	cvector<CssAssocClass> &css_assoc, CssOriginClass *css_origin,
	CssOrigerrClass *css_origerr, char **details);

class TableAttributes;
class TableViewer;

namespace libglc {

class Residuals;

class OriginData : public Gobject
{
    public:
	OriginData(CssOriginClass *o, CssOrigerrClass *oerr, char *s) {
	    origin = o;
	    origerr = oerr;
	    details = s;
	    details_dialog = NULL;
	    saved = false;
	    origin->addOwner(this);
	    if(origerr) origerr->addOwner(this);
	}
	~OriginData() {
	    free(details);
	    origin->removeOwner(this);
	    if(origerr) origerr->removeOwner(this);
	    if(details_dialog) {
		details_dialog->setVisible(false);
		details_dialog->destroy();
	    }
	}

	CssOriginClass	*origin;
	CssOrigerrClass	*origerr;
	char		*details;
	Details		*details_dialog;
	bool		saved;
};

class ArrivalData : public Gobject
{
    public :
	ArrivalData(CssArrivalClass *arr) {
	    arrival = arr;
	    site = NULL;
	    arrival->addOwner(this);

	    memset(phase, 0, sizeof(phase));
	    memset(timedef, 0, sizeof(timedef));
	    memset(azdef, 0, sizeof(azdef));
	    memset(slodef, 0, sizeof(slodef));
	}
	~ArrivalData() {
	    arrival->removeOwner(this);
	}

	CssArrivalClass	*arrival;
	CssSiteClass		*site;
	cvector<CssAssocClass> assocs;
	char		phase[9];
	char		timedef[2];
	char		azdef[2];
	char		slodef[2];

	void putAssoc(CssAssocClass *assoc) {
	    assocs.push_back(assoc);
	}
};

/** Locate window.
 *  @ingroup libglc
 */
class Locate : public Frame, public DataReceiver, public Parser
{
    public:
	Locate(const char *, Component *, DataSource *);
	Locate(const char *, Component *, WaveformPlot *);
	Locate(const char *, Component *, TableViewer *);
	~Locate(void);

	void setDataSource(DataSource *ds);
	void setVisible(bool visible);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseVar parseArrivalData(const string &name, string &value);
	void parseHelp(const char *prefix);

	// DataSource routines
 	int getTable(const string &cssTableName, gvector<CssTableClass *> &v) {
	    if(!cssTableName.compare(cssOrigin)) {
		return getTable((cvector<CssOriginClass> &)v);
	    }
	    else if(!cssTableName.compare(cssOrigerr)) {
		return getTable((cvector<CssOrigerrClass> &)v);
	    }
	    else if(!cssTableName.compare(cssArrival)) {
		return getTable((cvector<CssArrivalClass> &)v);
	    }
	    else if(!cssTableName.compare(cssSite)) {
		return getTable((cvector<CssSiteClass> &)v);
	    }
	    return 0;
	}
	int getTable(cvector<CssOriginClass> &v) {
	    v.clear();
	    v.ensureCapacity(origin_data.size());
	    for(int i = 0; i < origin_data.size(); i++) {
		v.push_back(origin_data[i]->origin);
	    }
	    return v.size();
	}
	int getTable(cvector<CssOrigerrClass> &v) {
	    v.clear();
	    v.ensureCapacity(origin_data.size());
	    for(int i = 0; i < origin_data.size(); i++) {
		v.push_back(origin_data[i]->origerr);
	    }
	    return v.size();
	}
	int getTable(cvector<CssArrivalClass> &v) {
	    v.clear();
	    v.ensureCapacity(arrival_data.size());
	    for(int i = 0; i < arrival_data.size(); i++) {
		v.push_back(arrival_data[i]->arrival);
	    }
	    return v.size();
	}
	int getTable(cvector<CssSiteClass> &v) {
	    v.clear();
	    v.ensureCapacity(sites.size());
	    for(int i = 0; i < sites.size(); i++) {
		v.push_back(sites[i]);
	    }
	    return v.size();
	}
	int getTable(cvector<CssAffiliationClass> &v) { return 0; }
	int getTable(cvector<CssAmpdescriptClass> &v) { return 0; }
	int getTable(cvector<CssAmplitudeClass> &v) { return 0; }
	int getTable(cvector<CssAssocClass> &v) { return 0; }
	int getTable(cvector<CssParrivalClass> &v) { return 0; }
	int getTable(cvector<CssNetmagClass> &v) { return 0; }
	int getTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
	int getTable(cvector<CssStamagClass> &v) { return 0; }
	int getTable(cvector<CssStassocClass> &v) { return 0; }
	int getTable(cvector<CssSitechanClass> &v) { return 0; }
	int getTable(cvector<CssWfdiscClass> &v) { return 0; }
	int getTable(cvector<CssWftagClass> &v) { return 0; }
	int getTable(cvector<CssXtagClass> &v) { return 0; }

	int getSelectedTable(cvector<CssOriginClass> &v) {
	    vector<int> rows;
	    int num = origin_table->getSelectedRows(rows);
	    v.clear();
	    v.ensureCapacity(num);
	    for(int i = 0; i < num; i++) {
		v.push_back(origin_data[rows[i]]->origin);
	    }
	    return v.size();
	}
	int getSelectedTable(cvector<CssArrivalClass> &v) {
	    vector<int> rows;
	    int num = arrival_table->getSelectedRows(rows);
	    v.clear();
	    v.ensureCapacity(num);
	    for(int i = 0; i < num; i++) {
		v.push_back(arrival_data[rows[i]]->arrival);
	    }
	    return v.size();
	}
	int getSelectedTable(cvector<CssAffiliationClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAmpdescriptClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAmplitudeClass> &v) { return 0; }
	int getSelectedTable(cvector<CssAssocClass> &v) { return 0; }
	int getSelectedTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
	int getSelectedTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
	int getSelectedTable(cvector<CssNetmagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssOrigerrClass> &v) { return 0; }
	int getSelectedTable(cvector<CssParrivalClass> &v) { return 0; }
	int getSelectedTable(cvector<CssStamagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssStassocClass> &v) { return 0; }
	int getSelectedTable(cvector<CssSiteClass> &v) { return 0; }
	int getSelectedTable(cvector<CssSitechanClass> &v) { return 0; }
	int getSelectedTable(cvector<CssWfdiscClass> &v) { return 0; }
	int getSelectedTable(cvector<CssWftagClass> &v) { return 0; }
	int getSelectedTable(cvector<CssXtagClass> &v) { return 0; }
	int getSelectedTable(const string &cssTableName,gvector<CssTableClass *> &v){
	    if(!cssTableName.compare(cssOrigin)) {
		return getSelectedTable((cvector<CssOriginClass> &)v);
	    }
	    else if(!cssTableName.compare(cssArrival)) {
		return getSelectedTable((cvector<CssArrivalClass> &)v);
	    }
	    return 0;
	}
	int getPathInfo(PathInfo **p);
	void addDataListener(Component *comp) {
	    addActionListener(comp, XtNdataChangeCallback);
	}

    protected:
	TableViewer	*tv;

	// File menu
	Button *locate_button, *new_locate_button, *save_button, *close_button;
	Button *reload_button;
	Toggle *auto_load;

	// Edit menu
	Button *remove_origins, *remove_arrivals;

	// View menu
	Button *origin_attributes_button, *arrival_attributes_button;
	Button *location_details;
	Menu *select_menu, *sort_menu;
	Button *select_all_button, *deselect_all_button, *distance_time;
	Button *orid_distance_time, *time_button, *sta_time_button;
	Button *sort_by_column;

	// Option menu
	Menu *defining_menu, *method_menu;
	Button *time_all_button, *time_none_button, *az_all_button;
	Button *az_none_button, *slow_all_button, *slow_none_button;
	Toggle *locsat_toggle;
	Button *colors_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	MultiTable *origin_table;
	Table *arrival_table;
	Form *form;
	Label *label;
	TabClass *tab;
	TableAttributes *arrival_attributes;
	Residuals *residuals_window;
	vector <Locate *> windows;

	gvector<OriginData *> origin_data;
	gvector<ArrivalData *> arrival_data;

	cvector<CssSiteClass> sites;

	typedef struct
	{
	    char		*name;
	    Toggle		*toggle;
	    char		*shared_lib;
	    char		*function_name;
	    char		*init_function_name;
	    LocFunction		locFunction;
	    LocInitFunction	locInitFunction;
	    Table		*param_table;
	} LocationMethod;

	int num_methods;
	LocationMethod *methods;
	int location_status;

	void init(void);
	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void list(void);
	void getDynamicMethods(void);
	bool getLocFunctions(LocationMethod *method);
	void createParamTab(LocationMethod *method);
	void getDefaultParams(CssTableClass *css);
	void addParamLine(CssTableClass *css, int orid);
	void listOriginData(void);
	void listArrivalData(void);
	LocationMethod *getMethod(void) {
	    Widget w;
	    if((w = tab->getTabOnTop()) != NULL) {
		for(int i = 0; i < num_methods; i++) {
		    if(methods[i].param_table->baseWidget() == w) {
			return &methods[i];
		    }
		}
	    }
	    return NULL;
	}

	void locateEvents(bool selected);
	bool getDefining(vector<const char *> &phase,
		vector<const char *> &timedef, vector<const char *> &azdef,
		vector<const char *> &slodef);
	void showDetails(void);
	void initializeParams(void);
	void loadData(void);
	void loadFromTableQuery(TableViewer *tv);
	int loadFromOriginTab(TableViewer *tv, cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssArrivalClass> &arrivals);
	void loadTables(void);
	void loadStructs(cvector<CssOriginClass> &origins,
		cvector<CssOrigerrClass> &origerrs, cvector<CssAssocClass> &assocs,
		cvector<CssArrivalClass> &arrivals);
	bool haveArid(int arid);
	bool haveOrid(int orid);

	void sortArrivals(const char *sort);
	void selectOrigin(void);
	void updateInitialLocation(void);
	void arrivalChange(TableListenerCallback *tc);
	void paramChoice(Table *table, MmTableEditCallbackStruct *c);
	void removeSelectedOrigin(void);
	void removeSelectedArrival(void);
	void setDefining(int col_index, const char *defining);
	void columnMoved(void);
	void saveOrigins(void);
	void reload(void);
	void loadResources(void);
	Table *findTable(const char *name);

    private:

};

} // namespace libglc

#endif
