#ifndef _CSS_TABLE_CLASS_H_
#define _CSS_TABLE_CLASS_H_

#include <stdlib.h>
#include <sys/stat.h>

#include "gobject++/Gobject.h"
#include "gobject++/gvector.h"
#include "gobject++/ghashtable.h"

extern "C" {
#include "libstring.h"
}

/**
 *  A CssTableClass is a GObject used to hold information about CSS tables.
 *
 *  @see GObject
 *
 */
#ifndef CSS_WRONG_FORMAT
#define CSS_WRONG_FORMAT 100
#endif

#ifndef CSS_MALLOC_ERROR
#define CSS_MALLOC_ERROR 101
#endif

#ifndef CSS_WRITE_ERROR
#define CSS_WRITE_ERROR  103
#endif

typedef struct
{
	int	start;
	int	end;
	char	name[32];
	char	format[8];
	int	offset;
	int	size;
	int	quark_offset;
	int	type;
	char	null_value[16];
} CssClassDescription;

typedef struct
{
	char	name[32];
	char	format[8];
	int	offset;
	int	size;
	int	type;
	char	null_value[16];
} CssClassExtra;

class DataSource;
class CssTableClass;

typedef CssTableClass * (*CreateCssTable)(void);

class CssTableClass : public Gobject
{
    protected:
	int		_name;
	int		_num_members;
	CssClassDescription	*_des;
	int		_num_extra;
	CssClassExtra	*_extra;
	int		_num_bytes;
	int		_line_length;

	DataSource	*_data_source;
	int		_dc;
	int		_id;
	int		_copy;
	int		_format;
	int		_odbc_source;
	int		_user;   // or parameter root
	int		_passwd; // or segment root
	int		_account;
	int		_directory_structure;
	double          _directory_duration;
	int		_table_name;
	int		_dir;
	int		_prefix;
	int		_file;
	int		_file_offset;
	bool		_selected;
	bool		_loaded;
	bool		_save_ds;

	ghashtable<string *>	shashtable;
	ghashtable<double>	dhashtable;
	ghashtable<long>	lhashtable;
	ghashtable<Gobject *>   vhashtable;

	CssTableClass(const string &table_name);
	CssTableClass(void) { init("CssTableClass"); }

	void init(const string &table_name);
	void initTable(void);
	void initDescription(void);
	void initExtra(void);

    public:
	~CssTableClass(void);
	bool operator==(const CssTableClass &t) { return  equals(t); }
	bool operator!=(const CssTableClass &t) { return !equals(t); }
/*
        CssTableClass & operator=(const CssTableClass &t) {
	    if(this == &t) return *this;
	    t.copyTo(this);
	    return *this;
	}
*/
	bool equals(const CssTableClass &t);
	bool nameIs(const string &s) { return !s.compare(getName()); }
	bool strictlyEquals(const CssTableClass &t);
	CssTableClass *clone(void) {
	    CssTableClass *t = createCssTable(getName());
	    copyTo(t);
	    return t;
	}
	bool copyTo(CssTableClass *dest, bool copy_source=true);
	const char *member(int index);
	int read(FILE *fp, const char **err_msg);
	int read_css_table(char *line);
	int write(FILE *fp, const char **err_msg);
	char *write_css_table(char *line);

	bool setDoubleMember(const string &member_name, double value);
	bool setMember(int i, const string &value);
	bool setExtra(int i, const string &value);

	int filePosition(void) { return _file_offset/(_line_length+1); }

	void setIds(int dc, int id);

	int memberIndex(const string &member_name);
	int memberOffset(const string &member_name);
	char *memberAddress(const string &member_name);
	int memberType(const string &member_name);
	char *toString(void);
	int memberToString(int member_index, char *buf, int buf_len);

	void setSource(int odbc_source, int user, int passwd) {
	    _odbc_source = odbc_source;
	    _user = user;
	    _passwd = passwd;
	}
	void getSource(int *odbc_source, int *user, int *passwd) {
	    *odbc_source = _odbc_source;
	    *user = _user;
	    *passwd = _passwd;
	}
	void setAccount(int account, int table_name) {
	    _account = account;
	    _table_name = table_name;
	}
	void getAccount(int *account, int *table_name) {
	    *account = _account;
	    *table_name = _table_name;
	}
	void setDirectoryStructure(int directory_structure,
			double directory_duration) {
	    _directory_structure = directory_structure;
	    _directory_duration = directory_duration;
	}
	void getDirectoryStructure(int *directory_structure,
			double *directory_duration) {
	    *directory_structure = _directory_structure;
	    *directory_duration = _directory_duration;
	}
	void copySourceTo(CssTableClass *dest, int id);

	const char *getName(void) { return quarkToString(_name); }

	int getNumMembers(void) { return _num_members; }
	CssClassDescription *description(void) { return _des; }

	int getNumExtra(void) { return _num_extra; }
	CssClassExtra *getExtra(void) { return _extra; }

	int	getType(void) { return _name; }
	int	getFormat(void) { return _format; }
	int	getNumBytes(void) { return _num_bytes; }
	int	getLineLength(void) { return _line_length; }
	int	getDC(void) { return _dc; }
	int	getID(void) { return _id; }
	int	getCopy(void) { return _copy; }
	int	getDir(void) { return _dir; }
	int	getPrefix(void) { return _prefix; }
	int	getFile(void) { return _file; }
	int	getFileOffset(void) { return _file_offset; }
	int	getSelected(void) { return _selected; }
	int	getLoaded(void) { return _loaded; }
	int	getSaveDS(void) { return _save_ds; }

	void	setDC(int dc) { _dc = dc; }
	void	setId(int id) { _id = id; }
	void	setFormat(int format) { _format = format; }
	void	setDir(int dir) { _dir = dir; }
	void	setPrefix(int prefix) { _prefix = prefix; }
	void	setFile(int file) { _file = file; }
	void	setFileOffset(int offset) { _file_offset = offset; }
	void	setSelected(bool selected) { _selected = selected; }
	void	setLoaded(bool loaded) { _loaded = loaded; }
	void	setSaveDS(bool save_ds) { _save_ds = save_ds; }

	void setDataSource(DataSource *ds);
	DataSource * getDataSource() { return _data_source; }

	/** Store a character string in the hashtable. The string is copied.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the character string.
	 */
	void putValue(const string &name, const string &value) {
	    shashtable.put(name, value);
	}
	/** Store a float value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the float value.
	 */
	void putValue(const string &name, float value) {
	    dhashtable.put(name, (double)value);
	}
	/** Store a double value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the double value.
	 */
	void putValue(const string &name, double value) {
	    dhashtable.put(name, value);
	}
	/** Store a long value in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the long value.
	 */
	void putValue(const string &name, long value) {
	    lhashtable.put(name, value);
	}
	/** Store a Gobject in the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[in] value the Gobject.
	 */
	void putValue(const string &name, Gobject *value) {
	    vhashtable.put(name, value);
	}

	/** Get a string value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the string value.
	 *  @param[in] len the length of value[].
	 *  @returns true if the name was found in the hashtable.
	 *  Returns false if not.
	 */
	bool getValue(const string &name, string &value)
	{
	    string *string_value;
	    if(shashtable.get(name, &string_value)) {
		value.assign(*string_value);
		return true;
	    }
	    return false;
	}

	/** Get a float value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the float value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, float *value)
	{
	    double d;
	    if(dhashtable.get(name, &d)) {
		*value = (float)d;
		return true;
	    }
	    return false;
	}

	/** Get a double value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the double value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, double *value)
	{
	    return dhashtable.get(name, value);
	}

	/** Get a long value from the hashtable.
	 *  @param[in] name the hashtable key.
	 *  @param[out] value the long value.
	 *  @returns true if the name was found in the hashtable. Returns false if not.
	 */
	bool getValue(const string &name, long *value)
	{
	    return lhashtable.get(name, value);
	}
        /** Get a Gobject from the hashtable.
         *  @param[in] name the hashtable key.
         *  @returns the Gobject or NULL if the name is not in the hashtable.
         */
	Gobject *getValue(const string &name)
	{
	    Gobject *o;
	    if(vhashtable.get(name, &o)) {
		return o;
	    }
	    return NULL;
	}
        /** Remove a value from the hastable.
         *  @param[in] name the hashtable key.
         */
	void removeValue(const string &name) {
	    shashtable.remove(name);
	    dhashtable.remove(name);
   	    lhashtable.remove(name);
	    vhashtable.remove(name);
	}

	int toBytes(char **bytes);

	static void predefineTables(void);
	static int define(const string &name, int num_members,
			CssClassDescription *des, int num_extra,
			CssClassExtra *extra, CreateCssTable create_table, int size);
	static CssTableClass *createCssTable(const string &cssTableName);
	static int getDescription(const string &name, CssClassDescription **des);
	static int getExtra(const string &name, CssClassExtra **extra);
	static int getAllNames(const char ***cssTableNames);
	static int readFile(const string &file, struct stat *old_stat,
		const string &table_name, gvector<CssTableClass *> &tables,
		const char **err_msg);
	static int readFile(const string &file, const string &table_name,
		gvector<CssTableClass *> &tables, const char **err_msg) {
	    return readFile(file, NULL, table_name, tables, err_msg);
	}
	static int sort(gvector<CssTableClass *> &tables, const string &member_name);
	static int sort(int num, CssTableClass **tables, const string &member_name);
	static CssTableClass *find(gvector<CssTableClass *> &tables,
		const string &member_name, long value);
	static int archive(CssTableClass ***t);
	static bool isTableName(const string &name);
	static bool isMember(const string &cssTableName,
		const string &memberName);
	static CssTableClass *fromBytes(int nbytes, char *bytes);
	static int redefine(const string &name, int num_members,
		CssClassDescription *des, int num_extra, CssClassExtra *extra,
		CreateCssTable create_table, int size);
	static char * getError();
	static void cleanUp(void);
};

typedef int (*ReadClassFunc)(CssTableClass *a);

#define CSS_STRING 0
#define CSS_DOUBLE 1
#define CSS_FLOAT  2
#define CSS_INT    3
#define CSS_LONG   4
#define CSS_TIME   5
#define CSS_DATE   6
#define CSS_LDDATE 7
#define CSS_QUARK  8
#define CSS_BOOL   9
#define CSS_JDATE  10

void stringToUpper(string &s);
int stringToQuark(const string &s);
int stringUpperToQuark(const string &s);

#endif
