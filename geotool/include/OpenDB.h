#ifndef _OPENDB_H
#define _OPENDB_H

#include "motif++/FormDialog.h"
#include "motif++/TextField.h"
#include "motif++/Choice.h"
#include "motif++/MotifDecs.h"
#include "widget/TabClass.h"
#include "widget/Table.h"
#include "TableViewer.h"
#include "FFDatabase.h"

#ifdef HAVE_LIBODBC
#include "FixBool.h"
#include "libgdb.h"
#endif

class CssFileDialog;
class TableQuery;

#define XtNconnectionCallback "connectionCallback"

class OpenDBStruct
{
    public :
	string	tabName;
	bool	get_origins;
	bool	get_arrivals;
	bool	get_wfdiscs;
	char	*origin_query;
	char	*arrival_query;
	char	*wfdisc_query;
	char	*general_query;
	double	tmin;
	double	tmax;
	char	*use;

	OpenDBStruct()
	{
	    get_origins = True;
	    get_arrivals = True;
	    get_wfdiscs = True;
	    origin_query = NULL;
	    arrival_query = NULL;
	    wfdisc_query = NULL;
	    general_query = NULL;
	    tmin = NULL_TIME;
	    tmax = NULL_TIME;
	    use = NULL;
	}

	~OpenDBStruct()
	{
	    Free(origin_query);
	    Free(arrival_query);
	    Free(wfdisc_query);
	    Free(general_query);
	}
};

/** Database Connection window.
 *  @ingroup libgtq
 */
class OpenDB : public FormDialog
{
    public:
	OpenDB(const string &name, Component *parent, bool auto_connect);
	OpenDB(const string &name, Component *parent, bool auto_connect,
		ActionListener *listener);
	OpenDB(const string &name, TableQuery *tq_parent, bool auto_connect,
		ActionListener *listener);
	~OpenDB(void);

	ConnectionType sourceType(void);
#ifdef HAVE_LIBODBC
	SQLHDBC	getHDBC(void) { return hdbc; }
#endif
	FFDatabase *getFFDB(void) { return ffdb; }
	string getMapping(const string &cssTableName);
	char *getAccount(void) { return dbAccount->getString(); }
	double queryBuffer(void) { return query_buffer; }
	string directoryStructure(void) { return directory_structure; }
	double directoryDuration(void) { return directory_duration; }
	bool arrivalTimeBefore(double *time_before);
	bool arrivalTimeAfter(double *time_after);
	char *arrivalChannels(void) { return arrival_channels->getString(); }
	char *getWritableAuthor(const string &param_root);
	void setMappingPrefix(char *s);
	void setMappingSuffix(char *s);
	void resetMapping(void);
	void saveMapping(void);

	void disconnect(void);
	void outputDisconnect(void);
	bool autoConnect(void);
	void autoConnectFlatFile(void);
	void autoConnectFilePrefix(const string &file="");
	void autoConnectODBC(const string &odbc_source);
	bool autoOutputConnect(void);
	void autoOutputConnectFlatFile(void);
	void autoOutputConnectFilePrefix(void);
	void autoOutputConnectODBC(const string &odbc_source);

	bool connectODBC(const string &odbc_source, const string &user,
			const string &password);
	void connectPrefix(const string &prefix);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseCmd parseMapping(const string &cmd, string &msg);
	void parseHelp(const char *prefix);
	bool outputConnected(void) { return output_connected; }
	string outputSource(void) { return output_data_source; }
	string outputAccount(void) { return output_data_account; }
	string outputPasswd(void) { return output_data_passwd; }
	string outputParamRoot(void) { return output_ffdb_param_root; }
	string outputSegRoot(void) { return output_ffdb_seg_root; }
	string outputPrefix(void) { return output_ffdb_file_prefix; }

	bool noTable(const string &name) {
	    if(!name.compare(cssSite)) return no_sites;
	    else if(!name.compare(cssSitechan)) return no_sitechans;
	    else if(!name.compare(cssAffiliation)) return no_affiliations;
	    return false;
	}
	void setNoTable(const string &name) {
	    if(!name.compare(cssSite)) no_sites = true;
	    else if(!name.compare(cssSitechan)) no_sitechans = true;
	    else if(!name.compare(cssAffiliation)) no_affiliations = true;
	}

    protected:

	Button 		*run_query_button;
	TabClass	*tab;
	TableQuery	*tq;
//	Widget  info;
 
        // Connect tab: makeConnectionTab()
	Label  		*source_label, *output_source_label;
	Label  		*account_label, *output_account_label;
	Label  		*passwd_label, *output_passwd_label;
        Choice		*source, *output_source;
        TextField	*account_text, *output_account_text;
        TextField	*password_text, *output_password_text;
        Button		*connect, *output_connect;
        Button		*create_param, *output_create_param;
        Button		*create_seg, *output_create_seg;
        TextField	*connection, *output_connection;
	Toggle		*save_source, *output_toggle;
	Button		*choose_file_button, *output_choose_file_button;
	CssFileDialog	*open_prefix_file;
	Form		*output_form;

	// Mapping tab: makeMappingTab()
	TextField	*prefix;
	Button		*prefix_button;
	TextField	*suffix;
	Button		*suffix_button;
	Table		*mapping_table;
	Button		*reset_mapping_button;
	Button		*make_mapping_permanent;
	char		**permanent_mapping;

	// Account tab: makeAccountTab()
	Label		*account_tab_label;
	TextField	*dbAccount;
	Choice		*author_choice;
	Toggle		*writable;
	Button		*account_create;
	Button		*account_permanent;
	Label		*dir_label;
	Table		*author_table;
	string		permanent_account;

	// Origin tab: makeOriginTab()
	TextField	*originId;
	TextField	*minLat;
	TextField	*maxLat;
	TextField	*minLon;
	TextField	*maxLon;
	TextField	*minDepth;
	TextField	*maxDepth;
	TextField	*minDefining;
	TextField	*maxDefining;
	TextField	*minNass;
	TextField	*maxNass;
	TextField	*minTime;
	TextField	*maxTime;
	TextField	*origin_query;
	Toggle		*origin_arrival_toggle;
	Toggle		*origin_wfdisc_toggle;
	Choice		*use;

	// Arrival tab: makeArrivalTab()
	TextField	*arrivalId;
	TextField	*arrival_start_time;
	TextField	*arrival_end_time;
	TextField	*arrival_stations;
	TextField	*arrival_channels;
	TextField	*arrival_query;
	Toggle		*arrival_origin_toggle;
	Toggle		*arrival_wfdisc_toggle;
	TextField	*arrival_time_before;
	TextField	*arrival_time_after;
	TextField	*waveform_channels;

	// Time tab: makeTimeTab()
	TextField	*time_start_time;
	TextField	*time_end_time;
	TextField	*time_stations;
	TextField	*time_channels;
	TextField	*time_network;
	Toggle		*time_origin_toggle;
	TextField	*time_origin_query;
	Toggle		*time_arrival_toggle;
	TextField	*time_arrival_query;
	Toggle		*time_wfdisc_toggle;
	TextField	*time_wfdisc_query;

	double	tmin, tmax;
	double	query_buffer;

	// General tab: makeGeneralTab()
	TextField	*general_query;

	enum InterfaceType {
	    ODBC_INTERFACE	= 1,
	    FFDB_INTERFACE	= 2,
	    PREFIX_INTERFACE	= 3
	} interface_type, output_interface_type;

	bool	connected, output_connected;
	bool	no_sites, no_sitechans, no_affiliations;
	string	ffdb_param_root, output_ffdb_param_root;
	string	ffdb_seg_root, output_ffdb_seg_root;
	string	ffdb_file_prefix, output_ffdb_file_prefix;
	string	directory_structure;
	double	directory_duration;
	string	data_source, output_data_source;
	string	data_account, output_data_account;
	string	data_passwd, output_data_passwd;
	string	working_author;

	bool	last_arid_input;

#ifdef HAVE_LIBODBC
	SQLHDBC	hdbc, output_hdbc;
#endif
	FFDatabase *ffdb, *output_ffdb;

	void createInterface();
	void actionPerformed(ActionEvent *action_event);
        void makeConnectionTab();
        void makeMappingTab();
        void makeAccountTab();
        void makeOriginTab();
        void makeArrivalTab();
        void makeTimeTab();
        void makeGeneralTab();
	void setODBCInterface();
	void setFFDBInterface();
	void setPrefixInterface();
	void setOutputODBCInterface();
	void setOutputFFDBInterface();
	void setOutputPrefixInterface();
	void accountInput();
	void outputAccountInput();
	void maskPassword(XtPointer data, string &passwd);
	bool createDir(const string &path);
	void turnOnTabs(bool state, bool general_tab);
	void turnOnButton(Button *b, TextField *text);
	bool connectButton(void);
	void outputConnectButton(void);
	bool OdbcConnect(void);
	bool OdbcConnectOutput(void);
	void flatFileConnect(void);
	void dbAccountInput(void);

	void timeFormQuery(void);
	void aridFormQuery(void);
	void arrivalFormQuery(void);
	void originFormQuery(void);
	bool originsFromTime(char *query, int query_size);
	bool arrivalsFromTime(const string &tabName, char *query,
				int query_size);
	bool arrivalsFromArid(char *query, int query_size);
	bool originsFromOrid(char *query, int query_size);
	bool wfdiscsFromTime(char *query, int query_size, double *tmin,
				double *tmax);
	bool tabOnTop(const string &tabName);
	void turnOnTimeQuery(void);
	void saveDataSource(void);
	void saveOutputDataSource(void);
	void setAuthorWritable(void);
	void setButtonsSensitive(void);
	void saveAccount(void);

	static void addClause(char *query, int query_size, char *And,
			int And_size, const char *name, char *min, char *max);

    private:
};

#endif
