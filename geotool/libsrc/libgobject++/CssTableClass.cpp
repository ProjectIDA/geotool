/** \file CssTableClass.cpp
 *  \brief Defines class CssTableClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>

#include "gobject++/CssTableClass.h"
#include "gobject++/DataSource.h"
extern "C" {
#include "libtime.h"
#include "libstring.h"
#include "logErrorMsg.h"
}

/**
 * <h2>CssTableClass Class</h2>
 * <p>
 * The CssTableClass class contains routines that read, write and work with
 * tables (schema). Specific tables are represented by subclasses
 * of the CssTableClass base class. The class structure for CssTableClass is
 *  <pre>
 * typedef struct
 * {
 *      GObjectPart    core;
 *      CssTablePart   css;
 *
 * } *CssTableClass;
 * </pre>
 * Subclasses are defined by adding specific table members to this structure.
 * For example, the CssSensor class that represents the Sensor table is defined
 * as
 * <pre>
 * typedef struct
 * {
 *     GObjectPart     core;
 *     CssTablePart    css;
 *
 *     char    sta[7];         /\* station code         *\/
 *     char    chan[9];        /\* channel code         *\/
 *     double  time;           /\* epoch time of start of recording period *\/
 *     double  endtime;        /\* epoch time of end of recording period *\/
 *     long    inid;           /\* instrument id        *\/
 *     long    chanid;         /\* channel id           *\/
 *     long    jdate;          /\* julian date          *\/
 *     float   calratio;       /\* calibration          *\/
 *     float   calper;         /\* calibration period   *\/
 *     float   tshift;         /\* correction of data processing time *\/
 *     char    instant[2];     /\* (y,n) discrete/continuing snapshot *\/
 *     char    lddate[18];     /\* load date            *\/
 *
 *     /\* extra members for geotool internal use *\/
 *     int     sta_quark;
 *     int     chan_quark;
 * } *CssSensor;
 * </pre>
 * The Sensor table members are added to the class structure after CssTablePart.
 * Extra members can also be added to the structure, such as the sta_quark and
 * chan_quark members. These members are not part of the Sensor schema and
 * are not read from or written to the database or sensor file. They are
 * used only within the program and are placed in the Sensor subclass for
 * convenience. Here the sta_quark and chan_quark members are quarks of the
 * Sensor sta and chan strings.
 * </pre>
 *
 * The CssTablePart holds a description of the table; the table name, the
 * number of table members, the number of extra members, the printed line
 * length, etc. The CssClassDescription is a structure that describes each
 * table members. There is a CssClassDescription structure for each member.
 * The CssClassExtra is a structure that describes each extra member.
 * <p>
 * Many CSS table are predefined in the library (see cssObjects.c). Additional
 * table subclasses can be dynamically defined using the CssTableClass::define
 * routine. Create any CssTableClass subclass object with new_CssTableClass.
 */

#define MAX_ERROR_LEN 2048
static char error[MAX_ERROR_LEN] = "";

static CssTableClass **tables = NULL;
static int size_tables = 0;
static int num_tables = 0;

/**
 * @private
 */
typedef struct
{
	int		name;
	int		size;
	int		num_members;
	CssClassDescription	*des;
	int		num_extra;
	CssClassExtra	*extra;
	CreateCssTable	create_table;
} TableDefinition;

static int num_table_defs = 0;
static TableDefinition *table_defs = NULL;

static void trim(char *s);

extern "C" {
static int sort_names(const void *a, const void *b);
static int sort_by_string(const void *a, const void *b);
static int sort_by_date(const void *a, const void *b);
static int sort_by_long(const void *a, const void *b);
static int sort_by_int(const void *a, const void *b);
static int sort_by_double(const void *a, const void *b);
static int sort_by_float(const void *a, const void *b);
}
static void storeTable(CssTableClass *table);
static void removeTable(CssTableClass *table);


CssTableClass::CssTableClass(const string &table_name)
{
    init(table_name);

    storeTable(this);
}

void CssTableClass::init(const string &table_name)
{
    _name = stringToQuark(table_name);
    _num_members = 0;
    _des = NULL;
    _num_extra = 0;
    _extra = NULL;
    _num_bytes = 0;
    _line_length = 0;
    _data_source = NULL;
    _dc = 0;
    _id = 0;
    _copy = 0;
    _format = stringToQuark("css");
    int q = stringToQuark("");
    _odbc_source = q;
    _user = q;
    _passwd = q;
    _account = q;
    _directory_structure = q;
    _directory_duration = 1.e+60;
    _table_name = q;
    _dir = q;
    _prefix = q;
    _file = q;
    _file_offset = 0;
    _selected = false;
    _loaded = false;
    _save_ds = true;
}

CssTableClass::~CssTableClass(void)
{
    removeTable(this);
    if(_data_source) _data_source->removeOwner(this);
}

bool CssTableClass::isTableName(const string &name)
{
    int i, q = stringToQuark(name);

    predefineTables();

    for(i = 0; i < num_table_defs && table_defs[i].name != q; i++);

    return (i < num_table_defs) ? true : false;
}

bool CssTableClass::isMember(const string &cssTableName, const string &memberName)
{
    int i, j, q = stringToQuark(cssTableName);

    predefineTables();

    for(i = 0; i < num_table_defs && table_defs[i].name != q; i++);

    if(i == num_table_defs) {
	snprintf(error, sizeof(error),
		"CssTableClass::isMember: unknown table name %s",
		cssTableName.c_str());
	logErrorMsg(LOG_WARNING, error);
	return false;
    }

    for(j = 0; j < table_defs[i].num_members
	&& strcasecmp(table_defs[i].des[j].name, memberName.c_str()); j++);

    return (j < table_defs[i].num_members ? true : false);
}

int CssTableClass::getAllNames(const char ***cssTableNames)
{
    int i;
    const char **names=NULL;

    predefineTables();

    names = (const char **)malloc(num_table_defs*sizeof(char *));
    if(!names) {
	logErrorMsg(LOG_ERR, "CssTableClass::getAllName: malloc failed.");
	return -1;
    }
    for(i = 0; i < num_table_defs; i++) {
	names[i] = quarkToString(table_defs[i].name);
    }

    qsort(names, num_table_defs, sizeof(char *), sort_names);

    *cssTableNames = names;
    return num_table_defs;
}

CssTableClass * CssTableClass::createCssTable(const string &cssTableName)
{
    int i, q = stringToQuark(cssTableName);

    predefineTables();

    for(i = 0; i < num_table_defs && table_defs[i].name != q; i++);

    if(i == num_table_defs) {
	snprintf(error, sizeof(error),
	    "CssTableClass::createCssTable: unknown table name %s",
	    cssTableName.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }
    return (*table_defs[i].create_table)();
}

static int
sort_names(const void *a, const void *b)
{
    const char **na = (const char **)a;
    const char **nb = (const char **)b;
    return(strcmp(*na, *nb));
}

// static
void CssTableClass::cleanUp(void)
{
    int i;
//    Free(ids);
//    num_ids = 0;
    Free(tables)

    if(table_defs) {
	for(i = 0; i < num_table_defs; i++) {
	    Free(table_defs[i].des);
	    Free(table_defs[i].extra);
	}
	Free(table_defs);
    }
}

/**
 * Define a CssTableClass type. Define a new CssTableClass type. Call this routine once
 * for each new CssTableClass type. After a CssTableClass type (subclass) is defined
 * by this routine, CssTableClass objects of the type can be created with 
 * new_CssTableClass(type_name). The CssClassDescription that describes each table
 * member is
 * <pre>
 * typedef struct
 * {
 *    int     start;       /\* Character position in the printed line (>=1) *\/
 *    int     end;         /\* Character position in the printed line (>=1) *\/
 *    char    name[32];    /\* Member name. *\/
 *    char    format[8];   /\* Format for printing. *\/
 *    int     offset;      /\* Byte offset from the table structure pointer *\/
 *    int     size;        /\* Size in bytes *\/
 *    int     quark_offset;/\* Byte offset of an extra member that is a *\/
 *                                quark for this member (or -1) *\/
 *    int     type;        /\* Data type *\/
 *    char    null_value[16]; /\* Initial value *\/
 * } CssClassDescription;
 * </pre>
 * The CssClassExtra structure is used to describe "extra" table members that are
 * in the structure but are only used within the program. They are not read
 * from or written to a database or file. The CssClassExtra structure is
 * <pre>
 * typedef struct
 * {
 *    char    name[32];    /\* Member name. *\/
 *    char    format[8];   /\* Format for printing. *\/
 *    int     offset;      /\* Byte offset from the table structure pointer *\/
 *    int     size;        /\* Size in bytes *\/
 *    int     type;        /\* Data type *\/
 *    char    null_value[16]; /\* Initial value *\/
 * } CssClassExtra;
 * </pre>
 * The data types are
 * <pre>
 *     CSS_STRING for char *
 *     CSS_DOUBLE for double
 *     CSS_FLOAT  for float
 *     CSS_INT    for int
 *     CSS_LONG   for long
 *     CSS_TIME   for double epochal time.
 *     CSS_JDATE  for long jdate (yyyydoy)
 *     CSS_DATE   for DateTime date
 *     CSS_LDDATE for DateTime load date
 *     CSS_QUARK  for int quark
 *     CSS_BOOL   for bool
 * </pre>
 * For example, the code from cssObjects.c that defines the CssSensor class
 * is
 * <pre>
 *static bool
 *defineSensor()
 *{
 *#define offset(field)   Offset(CssSensor, field)
 *    CssClassDescription des[] = {
 *      {1,     6,      "sta",   "%s", offset(sta),  offset(sta_quark),\
 * CSS_STRING,  "-"},
 *      {8,    15,     "chan",   "%s", offset(chan),offset(chan_quark),\
 * CSS_STRING,  "-"},
 *      {17,   33,     "time","%.5lf", offset(time),                -1,\
 * CSS_TIME, T_NULL},
 *      {35,   51,  "endtime","%.5lf", offset(endtime),             -1,\
 * CSS_TIME, T_NULL},
 *      {53,   60,     "inid",  "%ld", offset(inid),                -1,\
 * CSS_LONG,   "-1"},
 *      {62,   69,   "chanid",  "%ld", offset(chanid),              -1,\
 * CSS_LONG,   "-1"},
 *      {71,   78,    "jdate",  "%ld", offset(jdate),               -1,\
 * CSS_JDATE,  "-1"},
 *      {80,   95, "calratio",   "%f", offset(calratio),            -1,\
 * CSS_FLOAT,  "0."},
 *      {97,  112,   "calper",   "%f", offset(calper),              -1,\
 * CSS_FLOAT, "-1."},
 *      {114, 119,   "tshift",   "%f", offset(tshift),              -1,\
 * CSS_FLOAT,  "0."},
 *      {121, 121,  "instant",   "%s", offset(instant),             -1,\
 * CSS_STRING,  "-"},
 *      {123, 139,   "lddate",   "%s", offset(lddate),              -1,\
 * CSS_LDDATE,    DTNULL},
 *    };
 *    CssClassExtra extra[] = {
 *         {offset(sta_quark),   CSS_INT, "0"},
 *         {offset(chan_quark),  CSS_INT, "0"},
 *    };
 *#undef offset
 *    CssSensor table;
 * 
 *    return (bool)CssTableClass::define("sensor", 12, des, 2, extra, sizeof(*table));
 *}
 * </pre>
 * 
 * @param name The table name.
 * @param num_members The number of members in the table.
 * @param des An array of num_members CssClassDescription structures.
 * @param num_extra The number of extra members.
 * @param extra An array of num_extra CssClassExtra structures.
 * @param size The total size of the subclass structure.
 * @no-sort
 */
int CssTableClass::define(const string &name, int num_members, CssClassDescription *des,
		int num_extra, CssClassExtra *extra, CreateCssTable create_table,
		int size)
{
    char msg[100];
    int i, j, name_q;

    name_q = stringToQuark(name);

    /* search for the table definition
     */
    for(i = 0; i < num_table_defs && name_q != table_defs[i].name; i++);
    if(i < num_table_defs)
    {
	return 0;
    }
    for(j = 0; j < num_members; j++) {
	if(j > 0 && des[j].start <= des[j-1].end) {
	    snprintf(msg, sizeof(msg),
		    "CssTableClass::define(%s): member %s has bad start position.",
			name.c_str(), des[j].name);
	    logErrorMsg(LOG_WARNING, msg);
	    return -1;
	}
	if(des[j].end < des[j].start) {
	    snprintf(msg, sizeof(msg),
		    "CssTableClass::define(%s): member %s has bad end position.",
			name.c_str(), des[j].name);
	    logErrorMsg(LOG_WARNING, msg);
	    return -1;
	}
    }
	    
    /* a new table type. store it in table_defs
     */
    if(table_defs == NULL) {
	table_defs = (TableDefinition *)malloc(sizeof(TableDefinition));
    }
    else {
	table_defs = (TableDefinition *)realloc(table_defs,
				(num_table_defs+1)*sizeof(TableDefinition));
    }
    if(!table_defs) {
	snprintf(msg, sizeof(msg), "CssTableClass::define(%s): malloc failed.",
			name.c_str());
	logErrorMsg(LOG_ERR, msg);
	return -1;
    }

    table_defs[i].name = name_q;
    table_defs[i].size = size;
    table_defs[i].num_members = num_members;
    table_defs[i].des =
		(CssClassDescription *)malloc(num_members*sizeof(CssClassDescription));
    if(!table_defs[i].des) {
	snprintf(msg, sizeof(msg), "CssTableClass::define(%s): malloc failed.",
		name.c_str());
	logErrorMsg(LOG_ERR, msg);
	return -1;
    }
    memcpy(table_defs[i].des, des, num_members*sizeof(CssClassDescription));

    table_defs[i].num_extra = num_extra;
    table_defs[i].extra = (CssClassExtra *)malloc(num_extra*sizeof(CssClassExtra));
    if(!table_defs[i].extra) {
	snprintf(msg, sizeof(msg), "CssTableClass::define(%s): malloc failed.",
		name.c_str());
	free(table_defs[i].des);
	logErrorMsg(LOG_ERR, msg);
	return -1;
    }
    memcpy(table_defs[i].extra, extra, num_extra*sizeof(CssClassExtra));

    table_defs[i].create_table = create_table;
    num_table_defs++;

    return 0;
}

/**
 * Get the description of a CssTableClass. Returns an array of CssClassDescription
 * structures. The CssClassDescription structure is defined as:
 * <pre>
 * typedef struct
 * {
 *       int     start;
 *       int     end;
 *       char    *name;
 *       char    *format;
 *       int     offset;
 *       int     quark_offset;
 *       int     type;
 *       char    *null_value;
 * } CssClassDescription;
 * </pre>
 * @param name The CssTableClass name.
 * @param des Returned pointer to an array of CssClassDescription structures. (Do
 * not free it.)
 * @return the number of table members and elements in des. Returns -1 if the \
 * name is not found.
 */
int CssTableClass::getDescription(const string &name, CssClassDescription **des)
{
    int i, q_name;

    /* search for the table definition
     */
    q_name = stringToQuark(name);
    for(i = 0; i < num_table_defs; i++) {
	if(q_name == table_defs[i].name) {
	    *des = table_defs[i].des;
	    return table_defs[i].num_members;
	}
    }

    predefineTables();

    /* try again */
    for(i = 0; i < num_table_defs; i++) {
	if(q_name == table_defs[i].name) {
	    *des = table_defs[i].des;
	    return table_defs[i].num_members;
	}
    }

    return -1;
}

int CssTableClass::getExtra(const string &name, CssClassExtra **extra)
{
    int i, q_name;

    /* search for the table definition
     */
    q_name = stringToQuark(name);
    for(i = 0; i < num_table_defs; i++) {
	if(q_name == table_defs[i].name) {
	    *extra = table_defs[i].extra;
	    return table_defs[i].num_extra;
	}
    }

    predefineTables();

    /* try again */
    for(i = 0; i < num_table_defs; i++) {
	if(q_name == table_defs[i].name) {
	    *extra = table_defs[i].extra;
	    return table_defs[i].num_extra;
	}
    }

    return -1;
}

static void
storeTable(CssTableClass *table)
{
    if(tables == NULL) {
	size_tables = 100;
	tables = (CssTableClass **)malloc(size_tables*sizeof(CssTableClass *));
	if(!tables) {
	    logErrorMsg(LOG_ERR, "storeTable: malloc failed.");
	    size_tables = 0;
	    return;
	}
	memset(tables, 0, size_tables*sizeof(CssTableClass *));
    }

    if(num_tables == size_tables) {
	size_tables += 100;
	tables = (CssTableClass **)realloc(tables, size_tables*sizeof(CssTableClass *));
	if(!tables) {
	    logErrorMsg(LOG_ERR, "storeTable: malloc failed.");
	    size_tables = 0;
	    return;
	}
	memset(tables+num_tables, 0, 100*sizeof(CssTableClass *));
    }
    tables[num_tables++] = table;
}

static void
removeTable(CssTableClass *table)
{
    int i;

    if(!tables) return;

    for(i = num_tables-1; i >= 0 && tables[i] != table; i--);
    if(i >= 0) {
	if(num_tables > 1) {
	    tables[i] = tables[num_tables-1];
	}
	num_tables--;
    }
}

/**
 * @private
 */
int CssTableClass::archive(CssTableClass ***t)
{
    *t = tables;
    return num_tables;
}

/**
 * Copy a CssTableClass. Copy the member values of this CssTableClass to another CssTableClass.
 * Copy <b>c2</b> member values to <b>c1</b> members.
 * @param dest A CssTableClass of the same type. Copy values from this to dest.
 * @return true for success, false for error (logErrorMsg is called).
 */
bool CssTableClass::copyTo(CssTableClass *dest, bool copy_source)
{
    int i;

    if(this == dest) {
	return true;
    }
    if(_name != dest->_name) {
	logErrorMsg(LOG_WARNING,
		"CssTableClass::copyTable: c1 and c2 are not the same table type.");
	return false;
    }
    for(i = 0; i < _num_members; i++) {
	char *member1 = (char *)dest + _des[i].offset;
	char *member2 = (char *)this + _des[i].offset;

	memcpy(member1, member2, _des[i].size);
    }

    for(i = 0; i < _num_extra; i++) {
	char *member1 = (char *)dest + _extra[i].offset;
	char *member2 = (char *)this + _extra[i].offset;
	memcpy(member1, member2, _extra[i].size);
    }

    if(copy_source)
    {
	dest->_format = _format;
	dest->_odbc_source = _odbc_source;
	dest->_user = _user;
	dest->_passwd = _passwd;
	dest->_table_name = _table_name;
	dest->_account = _account;
	dest->_directory_structure = _directory_structure;
	dest->_directory_duration = _directory_duration;
	dest->_dir = _dir;
	dest->_prefix = _prefix;
	dest->_file = _file;
	dest->_file_offset = _file_offset;
	dest->_selected = _selected;
//	dest->loaded = _loaded;
	dest->_save_ds = _save_ds;
	dest->setDataSource(_data_source);
    }
    return true;
}

/**
 * Comparison of two CssTables using member formats.
 * @return true if all members are equal, otherwise returns false.
 */
bool CssTableClass::equals(const CssTableClass &t)
{
    char buf1[100], buf2[100];
    bool compare_lddate = false;

    if(this == &t) return true;

    if(	_name != t._name || _num_members != t._num_members ||
	_num_bytes != t._num_bytes || _des != t._des)
    {
	return false;
    }

    for(int i = 0; i < _num_members; i++)
    {
	char *member1 = (char *)this + _des[i].offset;
	char *member2 = (char *)&t + t._des[i].offset;
	char *format = _des[i].format;

	switch(_des[i].type)
	{
	    case CSS_STRING:
		if(strcmp(member1, member2)) return false;
		break;
	    case CSS_DATE:
	    {
		DateTime *dt1 = (DateTime *)member1;
		DateTime *dt2 = (DateTime *)member2;
		if(dt1->year != dt2->year || dt1->month != dt2->month
			|| dt1->day != dt2->day) return false;
		break;
	    }
	    case CSS_LDDATE:
		if(compare_lddate)
		{
		    DateTime *dt1 = (DateTime *)member1;
		    DateTime *dt2 = (DateTime *)member2;
		    if(dt1->year != dt2->year || dt1->month != dt2->month
			|| dt1->day != dt2->day) return false;
		}
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		if(*(long *)member1 != *(long *)member2) return false;
		break;
	    case CSS_INT:
	    case CSS_QUARK:
		if(*(int *)member1 != *(int *)member2) return false;
		break;
	    case CSS_DOUBLE:
	    case CSS_TIME:
		snprintf(buf1, sizeof(buf1), format, *((double *)member1));
		snprintf(buf2, sizeof(buf2), format, *((double *)member2));
		if(strcmp(buf1, buf2)) return false;
		break;
	    case CSS_FLOAT:
		snprintf(buf1, sizeof(buf1), format, *((float *)member1));
		snprintf(buf2, sizeof(buf2), format, *((float *)member2));
		if(strcmp(buf1, buf2)) return false;
		break;
	    case CSS_BOOL:
		if(*(bool *)member1 != *(bool *)member2) return false;
		break;
	    default:
		return false;
	}
    }
    return true;
}

/**
 * Strict Comparison of two CssTables.
 * @return true if all members are equal, otherwise returns false.
 */
bool CssTableClass::strictlyEquals(const CssTableClass &t)
{
    if(this == &t) return true;
    if(_name != t._name) return false;

    for(int i = 0; i < _num_members; i++)
    {
	char *member1 = (char *)this + _des[i].offset;
	char *member2 = (char *)&t + _des[i].offset;

	switch(_des[i].type)
	{
	    case CSS_STRING:
		if(strcmp(member1, member2)) return false;
		break;
	    case CSS_DATE:
	    {
		DateTime *t1 = (DateTime *)member1;
		DateTime *t2 = (DateTime *)member2;
		if(t1->year != t2->year || t1->month != t2->month
			|| t1->day != t2->day) return false;
		break;
	    }
	    case CSS_LDDATE:
	    {
		DateTime *t1 = (DateTime *)member1;
		DateTime *t2 = (DateTime *)member2;
		if(t1->year != t2->year || t1->month != t2->month
			|| t1->day != t2->day) return false;
		break;
	    }
	    case CSS_LONG:
	    case CSS_JDATE:
		if(*(long *)member1 != *(long *)member2)
		    return false;
		break;
	    case CSS_INT:
	    case CSS_QUARK:
		if(*(int *)member1 != *(int *)member2) return false;
		break;
	    case CSS_DOUBLE:
		if(*(double *)member1 != *(double *)member2) return false;
		break;
	    case CSS_FLOAT:
		if(*(float *)member1 != *(float *)member2) return false;
		break;
	    case CSS_TIME:
		if(*(double *)member1 != *(double *)member2) return false;
		break;
	    case CSS_BOOL:
		if(*(bool *)member1 != *(bool *)member2) return false;
		break;
	    default:
		logErrorMsg(LOG_WARNING,
		    "CssTableClass::equals: unknown member type.");
		return false;
		break;
	}
    }
    return true;
}

/**
 * Get a CssTableClass member name from the member index.
 * @param o A CssTableClass object.
 * @param index The member index.
 * @return The member address. Returns NULL, if o is not a CssTableClass, \
 *		or the index is invalid. 
 */
const char * CssTableClass::member(int index)
{
    if(index < 0 || index > _num_members) {
	logErrorMsg(LOG_WARNING, "CssTableClass::member: invalid member index.");
	return NULL;
    }
    return (const char *)this + _des[index].offset;
}

/**
 *  @private
 *  Initialize the CssTableClass members to their default values.
 */
void CssTableClass::initTable(void)
{
    char *endptr, *q, msg[100];
    int i;

    initDescription();

    for(i = 0; i < _num_members; i++)
    {
	char *c = (char *)this + _des[i].offset;
	if(_des[i].type == CSS_STRING)
	{
	    strncpy(c, _des[i].null_value, (size_t)_des[i].size);
	    if(_des[i].size > 0) {
		c[_des[i].size-1] = '\0';
	    }
	    if(_des[i].quark_offset > 0)  {
		q = (char *)this + _des[i].quark_offset;
		*(int *)q = stringUpperToQuark(c);
	    }
	}
	else if(_des[i].type == CSS_DATE || _des[i].type == CSS_LDDATE)
	{
	    double epoch = NULL_TIME;
	    if(!strcmp(_des[i].null_value, "-")) {
		timeEpochToDate(timeGetEpoch(), (DateTime *)c);
	    }
	    else if(timeParseString(_des[i].null_value, &epoch)) {
		timeEpochToDate(epoch, (DateTime *)c);
	    }
	    else {
		snprintf(error, sizeof(error),
		    "CssTableClass::init: cannot parse date. %s.%s value %s",
			quarkToString(_name), _des[i].name, _des[i].null_value);
		logErrorMsg(LOG_WARNING, error);
	    }
	}
	else if(_des[i].type == CSS_DOUBLE)
	{
	    double d = strtod(_des[i].null_value, &endptr);
	    if(*endptr != '\0') {
		snprintf(msg, 100, 
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		logErrorMsg(LOG_WARNING, msg);
		d = -1;
	    }
	    memcpy(c, &d, sizeof(double));
	}
	else if(_des[i].type == CSS_TIME) {
	    double epoch = NULL_TIME;
	    if(!timeParseString(_des[i].null_value, &epoch)) {
		    snprintf(msg, 100,
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		    logErrorMsg(LOG_WARNING, msg);
		    epoch = NULL_TIME;
	    }
	    memcpy(c, &epoch, sizeof(double));
	}
	else if(_des[i].type == CSS_JDATE) {
	    long jdate;
	    if(!timeParseJDate(_des[i].null_value, &jdate)) {
		    snprintf(msg, 100,
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		    logErrorMsg(LOG_WARNING, msg);
		    jdate = -1;
	    }
	    memcpy(c, &jdate, sizeof(long));
	}
	else if(_des[i].type == CSS_FLOAT)
	{
	    float f = (float)strtod(_des[i].null_value, &endptr);
	    if(*endptr != '\0') {
		    snprintf(msg, 100, 
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		    logErrorMsg(LOG_WARNING, msg);
		    f = -1;
	    }
	    memcpy(c, &f, sizeof(float));
	}
	else if(_des[i].type == CSS_LONG)
	{
	    long l = strtol(_des[i].null_value, &endptr, 0);
	    if(*endptr != '\0') {
		    snprintf(msg, 100, 
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		    logErrorMsg(LOG_WARNING, msg);
		    l = -1;
	    }
	    memcpy(c, &l, sizeof(long));
	}
	else if(_des[i].type == CSS_INT)
	{
	    int n = (int)strtol(_des[i].null_value, &endptr, 0);
	    if(*endptr != '\0') {
		    snprintf(msg, 100, 
			"CssTableClass::init: bad null_value value for %s.%s\n",
			quarkToString(_name), _des[i].name);
		    logErrorMsg(LOG_WARNING, msg);
		    n = -1;
	    }
	    memcpy(c, &n, sizeof(int));
	}
	else if(_des[i].type == CSS_QUARK)
	{
	    int q = stringUpperToQuark(_des[i].null_value);
	    memcpy(c, &q, sizeof(int));
	}
	else if(_des[i].type == CSS_BOOL)
	{
	    bool b = false;
	    if(_des[i].null_value[0] == 'T' || _des[i].null_value[0] == 't'
			|| _des[i].null_value[0] == '1') b = true;
	    memcpy(c, &b, sizeof(bool));
	}
    }

    initExtra();
}

void CssTableClass::initDescription(void)
{
    int i;

    for(i = 0; i < num_table_defs && table_defs[i].name != _name; i++);

    if( i == num_table_defs ) {
	snprintf(error, sizeof(error),
	    "CssTableClass: %s has not been defined with CssTableClass::define()",
	    quarkToString(_name));
	logErrorMsg(LOG_ERR, error);
	return;
    }
    _num_members = table_defs[i].num_members;
    _des = table_defs[i].des;
    _num_extra = table_defs[i].num_extra;
    _extra = table_defs[i].extra;
    _num_bytes = table_defs[i].size;
    _line_length = _des[_num_members-1].end;
}

/**
 *  @private
 *  Initialize the CssTableClass members to their default values.
 */
void CssTableClass::initExtra(void)
{
    char *endptr, msg[100];
    int i;

    for(i = 0; i < _num_extra; i++)
    {
	char *c = (char *)this + _extra[i].offset;
	if(_extra[i].type == CSS_STRING)
	{
	    stringcpy(c, _extra[i].null_value, _extra[i].size);
	}
	else if(_extra[i].type == CSS_DATE || _extra[i].type == CSS_LDDATE)
	{
	    double epoch = NULL_TIME;
	    if(!strcmp(_extra[i].null_value, "-")) {
		timeEpochToDate(timeGetEpoch(), (DateTime *)c);
	    }
	    else if(timeParseString(_extra[i].null_value, &epoch)) {
		timeEpochToDate(epoch, (DateTime *)c);
	    }
	    else {
		snprintf(error, sizeof(error),
		    "CssTableClass::initExtra: cannot parse date. %s.%s value %s",
			getName(), _extra[i].name, _extra[i].null_value);
		logErrorMsg(LOG_WARNING, error);
	    }
	}
	else if(_extra[i].type == CSS_JDATE) {
	    long jdate;
	    if(!timeParseJDate(_extra[i].null_value, &jdate)) {
		snprintf(msg, 100,
			"CssTableClass::initExtra: bad null_value value for %s.%s\n",
			getName(), _extra[i].name);
		logErrorMsg(LOG_WARNING, msg);
		jdate = -1;
	    }
	    memcpy(c, &jdate, sizeof(long));
	}
	else if(_extra[i].type == CSS_DOUBLE)
	{
	    double d = strtod(_extra[i].null_value, &endptr);
	    if(*endptr != '\0') {
		snprintf(msg, 100, 
			"CssTableClass::initExtra: bad null_value value for %s\n",
			getName());
		logErrorMsg(LOG_WARNING, msg);
		d = -1;
	    }
	    memcpy(c, &d, sizeof(double));
	}
	else if(_extra[i].type == CSS_TIME) {
	    double epoch = NULL_TIME;
	    if(!timeParseString(_extra[i].null_value, &epoch)) {
		snprintf(msg, 100,
			"CssTableClass::initExtra: bad null_value value for %s\n",
			getName());
		logErrorMsg(LOG_WARNING, msg);
		epoch = NULL_TIME;
	    }
	    memcpy(c, &epoch, sizeof(double));
	}
	else if(_extra[i].type == CSS_FLOAT)
	{
	    float f = (float)strtod(_extra[i].null_value, &endptr);
	    if(*endptr != '\0') {
		snprintf(msg, 100, 
			"CssTableClass::initExtra: bad null_value value for %s\n",
			getName());
		logErrorMsg(LOG_WARNING, msg);
		f = -1;
	    }
	    memcpy(c, &f, sizeof(float));
	}
	else if(_extra[i].type == CSS_LONG)
	{
	    long l = strtol(_extra[i].null_value, &endptr, 0);
	    if(*endptr != '\0') {
		snprintf(msg, 100, 
			"CssTableClass::initExtra: bad null_value value for %s\n",
			getName());
		logErrorMsg(LOG_WARNING, msg);
		l = -1;
	    }
	    memcpy(c, &l, sizeof(long));
	}
	else if(_extra[i].type == CSS_INT)
	{
	    int n = (int)strtol(_extra[i].null_value, &endptr, 0);
	    if(*endptr != '\0') {
		snprintf(msg, 100, 
			"CssTableClass::initExtra: bad null_value value for %s\n",
			getName());
		logErrorMsg(LOG_WARNING, msg);
		n = -1;
	    }
	    memcpy(c, &n, sizeof(int));
	}
	else if(_extra[i].type == CSS_QUARK)
	{
	    int q = stringUpperToQuark(_extra[i].null_value);
	    memcpy(c, &q, sizeof(int));
	}
	else if(_extra[i].type == CSS_BOOL)
	{
	    char *s = _extra[i].null_value;
	    bool b = false;
	    if(s[0] == 'T' || s[0] == 't' || s[0] == '1') b = true;
	    memcpy(c, &b, sizeof(bool));
	}
    }
}

/**
 * Read a single CssTableClass from a file.
 * <pre>
 *           CSS_MALLOC_ERROR      malloc failed
 *	     CSS_WRONG_FORMAT      format error
 *           EOF               End of file encountered
 * </pre>
 * @param o A CssTableClass object.
 * @param fp A FILE pointer to an open file.
 * @param err_msg On error, if err_msg != NULL, *err_msg is set to a static \
 *		char error message.
 * @return 0 for success, CSS_MALLOC_ERROR, CSS_WRONG_FORMAT, EOF \
 *  		(logErrorMsg is called for a nonzero return)
 */
int CssTableClass::read(FILE *fp, const char **err_msg)
{
    char *line;
    int c, n;

    line = (char *)malloc((size_t)_line_length+1);
    if(line == NULL) {
	logErrorMsg(LOG_ERR, "CssTableClass::read: malloc failed.");
	if(err_msg) *err_msg = error;
	stringcpy(error, "malloc error.", (int)sizeof(error));
	return CSS_MALLOC_ERROR;
    }
    _file_offset = ftell(fp);
    /* read the next line_length characters up to the next '\n'
     */
    for(n = 0; (c = getc(fp)) != '\n' && c != EOF && n < _line_length; n++)
    {
	line[n] = c;
    }
    line[n] = '\0';
    if(c == EOF)
    {
	free(line);
	stringcpy(error, "format error: unexpected EOF.", (int)sizeof(error));
	if(err_msg) *err_msg = error;
	else logErrorMsg(LOG_WARNING,
			"CssTableClass::read: format error: unexpected EOF.");
	return(EOF);
    }
    if(c != '\n')
    {
	/* read to the next '\n'
	     */
	while((c = getc(fp)) != '\n' && c != EOF);
    }

    if(n < _line_length)
    {
	free(line);
	stringcpy(error, "format error: short record.", (int)sizeof(error));
	if(err_msg) *err_msg = error;
	else logErrorMsg(LOG_WARNING,
		"CssTableClass::read: format error: short record.");
	return(CSS_WRONG_FORMAT);
    }

    if(read_css_table(line))
    {
	if(err_msg) *err_msg = error;
	free(line);
	return(CSS_WRONG_FORMAT);
    }
    free(line);
    return 0;
}

/**
 * Write a single CssTableClass to a file. The error return codes are
 * <pre>
 *           CSS_MALLOC_ERROR      malloc failed
 *	     CSS_WRITE_ERROR       system write error
 * </pre>
 * @param o A CssTableClass object.
 * @param fp A FILE pointer to an open file.
 * @param err_msg On error, *err_msg is set to a static char error message.
 * @return 0 for success, nonzero for error. logErrorMsg is called before a \
 * 	nonzero return.
 */
int CssTableClass::write(FILE *fp, const char **err_msg)
{
    char *line;

    line = (char *)malloc((size_t)_line_length+1);
    if(line == NULL) {
	if(err_msg) *err_msg = error;
	stringcpy(error, "malloc error.", (int)sizeof(error));
	logErrorMsg(LOG_ERR, "CssTableClass::write: malloc failed.");
	return CSS_MALLOC_ERROR;
    }

    write_css_table(line);

    line[_line_length] = '\n';

    _file_offset = ftell(fp);
    if(fwrite(line, 1,(size_t)_line_length+1,fp) != (size_t)(_line_length+1))
    {
	snprintf(error, sizeof(error), "CssTableClass::write: write error. %s",
			strerror(errno));
	if(err_msg) *err_msg = error;
	else logErrorMsg(LOG_WARNING, error);
	free(line);
	return(CSS_WRITE_ERROR);
    }
    free(line);
    return 0;
}

int CssTableClass::read_css_table(char *line)
{
    for(int i = 0; i < _num_members; i++)
    {
	line[_des[i].end] = '\0';
	trim(&line[_des[i].start-1]);

	if( !setMember(i, &line[_des[i].start-1]) ) return 1;
    }
    return 0;
}

/**
 * Return a CssTableClass member index.
 * @param o A CssTableClass object.
 * @param member_name A member name.
 * @return The member index, or -1 if member_name is not found, \
 *		-2 if o is not a CssTableClass.
 */
int CssTableClass::memberIndex(const string &member_name)
{
    int i;

    for(i = 0; i < _num_members &&
		strcasecmp(member_name.c_str(), _des[i].name); i++);

    return (i < _num_members) ? i : -1;
}

/**
 * Return a CssTableClass member's offset from the CssTableClass pointer.
 * @param o A CssTableClass object.
 * @param member_name A member name.
 * @return The member offset, or -1 if member_name is not found, \
 *		-2 if o is not a CssTableClass.
 */
int CssTableClass::memberOffset(const string &member_name)
{
    int i;

    for(i = 0; i < _num_members &&
		strcasecmp(member_name.c_str(), _des[i].name); i++);

    return (i < _num_members) ? (int)_des[i].offset : -1;
}

/**
 * Return a CssTableClass member's address.
 * @param member_name A member name.
 * @return The member address, or NULL if member_name is not found \
 *		or o is not a CssTableClass.
 */
char * CssTableClass::memberAddress(const string &member_name)
{
    int i;
    for(i = 0; i < _num_members &&
		strcasecmp(member_name.c_str(), _des[i].name); i++);
    return (i < _num_members) ? (char *)this + _des[i].offset : NULL;
}

/**
 * Return a CssTableClass member data type.
 * @param member_name A member name.
 * @return The member type (CSS_STRING, CSS_LONG, CSS_DOUBLE, etc.), or -1 if \
 * member_name is not found or o is not a CssTableClass.
 */
int CssTableClass::memberType(const string &member_name)
{
    int i;
    for(i = 0; i < _num_members &&
		strcasecmp(member_name.c_str(), _des[i].name); i++);
    return (i < _num_members) ? _des[i].type : -1;
}

/**
 * Set the value of a CssTableClass member. The input string representation
 * of the member value is converted to the appropriate member data type.
 * If the member type is CSS_TIME, the value can be input in any format that
 * is recognized by timeParseString. 
 * @param o A CssTableClass object.
 * @param member_index A member index.
 * @param value A pointer to the new string value.
 * @return true for success, false if conversion from the string input fails.
 */
bool CssTableClass::setMember(int member_index, const string &value)
{
    int i = member_index;
    char *member_address, *endptr;

    if(i < 0 || i >= _num_members) return false;

    member_address = (char *)this + _des[i].offset;

    if(_des[i].type == CSS_STRING)
    {
	if(value.empty()) {
	    // null value
	    strncpy(member_address, "-", (size_t)_des[i].size);
	}
	else {
	    stringTrimCopy(member_address, value.c_str(), _des[i].size);
	}
	if(_des[i].quark_offset > 0)  {
	    char *q = (char *)this + _des[i].quark_offset;
	    *(int *)q = stringUpperToQuark(member_address);
	}
    }
    else if(_des[i].type == CSS_DATE || _des[i].type == CSS_LDDATE)
    {
	double epoch = NULL_TIME;
	if(!value.compare("-")) {
	    timeEpochToDate(epoch, (DateTime *)member_address);
	}
	else if(timeParseString(value.c_str(), &epoch)) {
	    timeEpochToDate(epoch, (DateTime *)member_address);
	}
	else {
	    snprintf(error, sizeof(error),
		    "CssTableClass::setMember: cannot parse date. %s.%s value %s",
			getName(), _des[i].name, value.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else if(_des[i].type == CSS_JDATE) {
	if(!value.compare("-")) {
	    *((long *)member_address) = strtol(_des[i].null_value, &endptr, 0);
	}
	else if(!timeParseJDate(value.c_str(), (long *)member_address)) {
	    snprintf(error, sizeof(error),
		    "CssTableClass::setMember: cannot parse jdate. %s.%s value %s",
			getName(), _des[i].name, value.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else if(_des[i].type == CSS_QUARK)
    {
	int q;
	if(value.empty()) {
	    q = stringToQuark("-");
	}
	else {
	    q = stringTrimToQuark(value.c_str());
	}
	memcpy(member_address, &q, sizeof(int));
    }
    else if(_des[i].type == CSS_BOOL)
    {
	bool b = false;
	if((int)value.length() > 0 && (value[0] == 't' || value[0] == 'T'
		|| value[0] == '1')) b = true;
	memcpy(member_address, &b, sizeof(bool));
    }
    else
    {
	double epoch;

	if(_des[i].type == CSS_DOUBLE) {
	    double d;
	    if(!value.compare("-")) {
		d = strtod(_des[i].null_value, &endptr);
	    }
	    else {
		d = strtod(value.c_str(), &endptr);
	    }
	    memcpy(member_address, &d, sizeof(double));
	}
	else if(_des[i].type == CSS_TIME) {
	    if(!value.compare("-")) {
		timeParseString(_des[i].null_value, &epoch);
		memcpy(member_address, &epoch, sizeof(double));
	    }
	    else if(timeParseString(value.c_str(), &epoch)) {
		memcpy(member_address, &epoch, sizeof(double));
		return true;
	    }
	    else {
		snprintf(error, sizeof(error),
			"format error: attribute name: %s", _des[i].name);
		logErrorMsg(LOG_WARNING, error);
		return false;
	    }
	}
	else if(_des[i].type == CSS_FLOAT) {
	    float f;
	    if(!value.compare("-")) {
		f = (float)strtod(_des[i].null_value, &endptr);
	    }
	    else {
		f = (float)strtod(value.c_str(), &endptr);
	    }
	    memcpy(member_address, &f, sizeof(float));
	}
	else if(_des[i].type == CSS_LONG) {
	    long l;
	    if(!value.compare("-")) {
		l = strtol(_des[i].null_value, &endptr, 0);
	    }
	    else {
		l = strtol(value.c_str(), &endptr, 0);
	    }
	    memcpy(member_address, &l, sizeof(long));
	}
	else if(_des[i].type == CSS_INT) {
	    int n;
	    if(!value.compare("-")) {
		n = (int)strtol(_des[i].null_value, &endptr, 0);
	    }
	    else {
		n = (int)strtol(value.c_str(), &endptr, 0);
	    }
	    memcpy(member_address, &n, sizeof(int));
	}
	else {
	    snprintf(error, sizeof(error),
			"Unknown type: %d, attribute name: %s",
			_des[i].type, _des[i].name);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}	
	if(*endptr != '\0') {    
	    snprintf(error, sizeof(error),
			"format error: attribute name: %s", _des[i].name);
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    return true;
}

bool CssTableClass::setExtra(int member_index, const string &value)
{
    int i = member_index, len;
    char *member_address;

    if(i < 0 || i >= _num_extra) return false;

    member_address = (char *)this + _extra[i].offset;

    if(_extra[i].type == CSS_STRING)
    {
	len = _extra[i].size;
	if(value.empty()) {
	    strncpy(member_address, "-", (size_t)len); /* null value */
	}
	else {
	    stringTrimCopy(member_address, value.c_str(), len);
	}
    }
    else if(_extra[i].type == CSS_DATE || _extra[i].type == CSS_LDDATE)
    {
	double epoch = NULL_TIME;
	if(timeParseString(value.c_str(), &epoch)) {
	    timeEpochToDate(epoch, (DateTime *)member_address);
	}
	else {
	    snprintf(error, sizeof(error),
		    "CssTableClass::setExtra: cannot parse date. %s.%s value %s",
			getName(), _extra[i].name, value.c_str());
	    logErrorMsg(LOG_WARNING, error);
	}
    }
    else if(_extra[i].type == CSS_JDATE) {
	if(!timeParseJDate(value.c_str(), (long *)member_address)) {
	    snprintf(error, 100,
		    "CssTableClass::setExtra: cannot parse jdate. %s.%s value %s",
			getName(), _extra[i].name, value.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return false;
	}
    }
    else if(_extra[i].type == CSS_QUARK)
    {
	int q;
	if(value.empty()) {
	    q = stringToQuark("-");
	}
	else {
	    q = stringTrimToQuark(value.c_str());
	}
	memcpy(member_address, &q, sizeof(int));
    }
    else if(_extra[i].type == CSS_BOOL)
    {
	bool b = false;
	if((int)value.length() > 0 && (value[0] == 't' || value[0] == 'T'
		|| value[0] == '1')) b = true;
	memcpy(member_address, &b, sizeof(bool));
    }
    else
    {
	char *endptr;
	double epoch;

	if(_extra[i].type == CSS_DOUBLE) {
	    double d = strtod(value.c_str(), &endptr);
	    memcpy(member_address, &d, sizeof(double));
	}
	else if(_extra[i].type == CSS_TIME) {
	    if(timeParseString(value.c_str(), &epoch)) {
		memcpy(member_address, &epoch, sizeof(double));
		return true;
	    }
	    else {
		snprintf(error, sizeof(error),
			"format error: extra member name: %s", _extra[i].name);
		return false;
	    }
	}
	else if(_extra[i].type == CSS_FLOAT) {
	    float f = (float)strtod(value.c_str(), &endptr);
	    memcpy(member_address, &f, sizeof(float));
	}
	else if(_extra[i].type == CSS_LONG) {
	    long l = strtol(value.c_str(), &endptr, 0);
	    memcpy(member_address, &l, sizeof(long));
	}
	else if(_extra[i].type == CSS_INT) {
	    int n = (int)strtol(value.c_str(), &endptr, 0);
	    memcpy(member_address, &n, sizeof(int));
	}
	else {
	    snprintf(error, sizeof(error),
			"Unknown type: %d, extra member name: %s",
			_extra[i].type, _extra[i].name);
	    return false;
	}	
	if(*endptr != '\0') {    
	    snprintf(error, sizeof(error),
			"format error: attribute name: %s", _extra[i].name);
	    return false;
	}
    }
    return true;
}

/**
 * Write the CssTableClass members as a character string. Free the string after use.
 * @param o A CssTableClass object.
 * @return a character string. Returns NULL if o is not a CssTableClass or malloc
 * fails. (logErrorMsg is called)
 */
char * CssTableClass::toString(void)
{
    char *line = (char *)malloc((size_t)_line_length+1);
    if(line == NULL) {
	logErrorMsg(LOG_ERR, "CssTableClass::toString: malloc error.");
	return NULL;
    }
    return write_css_table(line);
}

/**
 * Convert a CssTableClass member to a character string.
 * @param o A CssTableClass object.
 * @param member_index The member index.
 * @param buf A character array to hold the output.
 * @param buf_len The length of <b>buf</b>.
 * @return 0 for success, -1 for an invalid member index.
 */
int CssTableClass::memberToString(int member_index, char *buf, int buf_len)
{
    int k, n;
    char *member_address;

    if(member_index < 0 || member_index >= _num_members) {
	logErrorMsg(LOG_WARNING,
		"CssTableClass::memberToString invalid member_index.");
	return -1;
    }
    k = member_index;

    member_address = (char *)this + _des[k].offset;

    if(_des[k].type == CSS_DOUBLE || _des[k].type == CSS_TIME)
    {
	snprintf(buf, buf_len, _des[k].format, *((double *)member_address));
    }
    else if(_des[k].type == CSS_FLOAT)
    {
	snprintf(buf, buf_len, _des[k].format, *((float *)member_address));
    }
    else if(_des[k].type == CSS_LONG || _des[k].type == CSS_JDATE)
    {
	snprintf(buf, buf_len, _des[k].format, *((long *)member_address));
    }
    else if(_des[k].type == CSS_INT)
    {
	snprintf(buf, buf_len, _des[k].format, *((int *)member_address));
    }
    else if(_des[k].type == CSS_BOOL)
    {
	snprintf(buf, buf_len, _des[k].format, *((bool *)member_address));
    }
    else if(_des[k].type == CSS_QUARK)
    {
	const char *q = quarkToString(*((int *)member_address));
	n = _des[k].size - 1;
	if(n > buf_len) n = buf_len;
	strncpy(buf, q, n);
	buf[n] = '\0';
    }
    else
    {
	n = _des[k].size - 1;
	if(n > buf_len) n = buf_len;
	strncpy(buf, member_address, n);
	buf[n] = '\0';
    }
    return 0;
}

char * CssTableClass::write_css_table(char *line)
{
    int n, buflen=500;
    char *member_address, buf[501];
    int i, j, k, str;

    for(i = 0; i < _line_length; i++) line[i] = ' ';

    for(k = 0; k < _num_members; k++)
    {
	member_address = (char *)this + _des[k].offset;

	str = 0;
	if(_des[k].type == CSS_DOUBLE || _des[k].type == CSS_TIME)
	{
	    snprintf(buf, buflen,_des[k].format,*((double *)member_address));
	}
	else if(_des[k].type == CSS_FLOAT)
	{
	    snprintf(buf, buflen, _des[k].format,*((float *)member_address));
	}
	else if(_des[k].type == CSS_LONG || _des[k].type == CSS_JDATE)
	{
	    snprintf(buf, buflen, _des[k].format, *((long *)member_address));
	}
	else if(_des[k].type == CSS_INT)
	{
	    snprintf(buf, buflen, _des[k].format, *((int *)member_address));
	}
	else if(_des[k].type == CSS_BOOL)
	{
	    snprintf(buf, buflen, _des[k].format, *((bool *)member_address));
	}
	else if(_des[k].type == CSS_QUARK)
	{
	    const char *q = quarkToString(*((int *)member_address));
	    n = _des[k].end - _des[k].start + 1;
	    if(n > buflen) {
		snprintf(buf, buflen, 
			"write_css_table: %s.%s truncated to %d characters", 
			getName(), _des[k].name, buflen);
		logErrorMsg(LOG_WARNING, buf);
		n = buflen;
	    }
	    strncpy(buf, q, n);
	    buf[n] = '\0';
	    str = 1;
	}
	else if(_des[k].type == CSS_STRING)
	{
	    n = _des[k].end - _des[k].start + 1;
	    if(n > _des[k].size) n = _des[k].size;
	    if(n > buflen) {
		snprintf(buf, buflen, 
			"write_css_table: %s.%s truncated to %d characters", 
			getName(), _des[k].name, buflen);
		logErrorMsg(LOG_WARNING, buf);
		n = buflen;
	    }
	    strncpy(buf, member_address, n);
	    buf[n] = '\0';
	    str = 1;
	}
	else if(_des[k].type == CSS_DATE || _des[k].type == CSS_LDDATE)
	{
	    n = _des[k].end - _des[k].start + 1;
	    if(n >= 11) {
		strcpy(buf, timeDateString((DateTime *)member_address));
	    }
	    else {
		DateTime *dt = (DateTime *)member_address;
		snprintf(buf, sizeof(buf), "%04d%03d",dt->year,timeDOY(dt));
	    }
	}
	else  {
	    logErrorMsg(LOG_WARNING,
			"write_css_table: skipping unknown member type.\n");
	    continue;
	}
	if(!str)
	{
	    // right justify numbers
	    n = (int)strlen(buf);
	    if(n <= _des[k].end - _des[k].start + 1)
	    {
		j = _des[k].end - n;
		for(i = _des[k].start-1; i < j; i++) line[i]=' ';
		strncpy(line+j, buf, n);
	    }
	    else
	    {
		for(i = _des[k].start-1, j = 0; i < _des[k].end
					&& buf[j] != '\0'; i++, j++)
		{
		    line[i] = buf[j];
		}
	    }
	}
	else
	{
	    // left justify strings
	    for(i = _des[k].start-1, j = 0; i < _des[k].end
			&& buf[j] != '\0'; i++, j++)
	    {
		line[i] = buf[j];
	    }
	    for(; i < _des[k].end; i++) line[i] = ' ';
	}
	line[_des[k].end] = ' ';
    }
    line[_line_length] = '\0';
    return line;
}

static void
trim(char *s)
{
    int i, j, n;

    n = (int)strlen(s);
    for(i = n; i > 0 && s[i-1] == ' '; i--); s[i]='\0';
    for(i = 0; s[i] != '\0' && s[i] == ' '; i++);
    for(j = 0; s[i] != '\0'; i++, j++) s[j] = s[i];
    s[j] = '\0';
}

/**
 * Return an error message. (A static char string.)
 */
char * CssTableClass::getError()
{
	return error;
}

/**
 * @private
 * Set identification numbers for a CssTableClass object.
 */
void CssTableClass::setIds(int dc, int id)
{
    if(_dc != 0 || _id != 0) return;

    if(dc == 0 && id == 0) {
	logErrorMsg(LOG_WARNING, "CssTableClass::setIds: invalid ids.");
	return;
    }
   _dc = dc;
   _id = id;
   _copy = 1;
}

void CssTableClass::copySourceTo(CssTableClass *dest, int id)
{
    dest->_format		= _format;
    dest->_odbc_source		= _odbc_source;
    dest->_user			= _user;
    dest->_passwd		= _passwd;
    dest->_account		= _account;
    dest->_directory_structure	= _directory_structure;
    dest->_directory_duration  	= _directory_duration;
    dest->_table_name		= _table_name;
    dest->_dir			= _dir;
    dest->_prefix		= _prefix;
    dest->_file			= _file;
    dest->_file_offset		= _file_offset;
    dest->_dc 			= _dc;
    dest->_save_ds 		= _save_ds;

    if(_id > 0) {
	dest->_id = _id;
/*	CssTableClass::setIds(dest, source, id); */
    }
}

static int member_offset;

/**
 * Sort a Vector of CssTableClass objects by the specified member. Possible errors
 * are a bad Vector object, Vector objects are not all the same CssTableClass,
 * invalid member_name and unknown member type. logErrorMsg is called before
 * a error (nonzero) return.
 * @param tables a Vector containing CssTableClass objects.
 * @param member_name The table member that will be sorted.
 * @return 0 for success, nonzero for an error condition.
 */
int CssTableClass::sort(gvector<CssTableClass *> &tables, const string &member_name)
{
    int i, name, num = tables.size();
    bool quark = false;
    CssTableClass *table;
    CssClassDescription *des;

    if(num <= 0) return 0;

    table = tables[0];
    name = table->_name;
    for(i = 1; i < num; i++) {
	if(tables[i]->_name != name) {
	    logErrorMsg(LOG_WARNING,
		    "CssTableClass::sortTable: tables not all the same type.\n");
	    return -1;
	}
    }
    des = table->_des;

    for(i = 0; i < table->_num_members
		&& strcasecmp(member_name.c_str(), des[i].name); i++);

    if(i == table->_num_members)
    {
	// check for a quark reference
	char buf[100];
	int n = (int)member_name.length();
	if(n > 6 && !member_name.substr(n-6).compare("_quark"))
	{
	    stringcpy(buf, member_name.c_str(), 100);
	    buf[n-6] = '\0';

	    for(i = 0; i < table->_num_members
		    && strcasecmp(buf, des[i].name); i++);
	    if(i < table->_num_members && des[i].quark_offset > 0) {
		quark = true;
	    }
	}
	if(!quark) {
	    snprintf(error, sizeof(error),
			"CssTableClass::sort: invalid member_name: %s",
			member_name.c_str());
	    logErrorMsg(LOG_WARNING, error);
	    return -2;
	}
    }

    if(quark) {
	member_offset = des[i].quark_offset;
	tables.sort(sort_by_int);
    }
    else
    {
	member_offset = des[i].offset;

	switch(des[i].type)
	{
	    case CSS_STRING:
		tables.sort(sort_by_string);
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		tables.sort(sort_by_date);
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		tables.sort(sort_by_long);
		break;
	    case CSS_INT:
	    case CSS_QUARK:
		tables.sort(sort_by_int);
		break;
	    case CSS_DOUBLE:
	    case CSS_TIME:
		tables.sort(sort_by_double);
		break;
	    case CSS_FLOAT:
		tables.sort(sort_by_float);
		break;
	    default:
		snprintf(error, sizeof(error),
		    "CssTableClass::sort: unknown member type: %d\n", des[i].type);
		logErrorMsg(LOG_WARNING, error);
		return -3;
	}
    }
    return 0;
}

static int
sort_by_string(const void *a, const void *b)
{
    const char *sa = (const char *)(*(const char **)a + member_offset);
    const char *sb = (const char *)(*(const char **)b + member_offset);
    return(strcmp(sa, sb));
}

static int
sort_by_date(const void *a, const void *b)
{
    DateTime *ta = (DateTime *)(*(DateTime **)a + member_offset);
    DateTime *tb = (DateTime *)(*(DateTime **)b + member_offset);
    if(ta->year != tb->year) return(ta->year - tb->year);
    if(ta->month != tb->month) return(ta->month - tb->month);
    return(ta->day - tb->day);
}

static int
sort_by_long(const void *a, const void *b)
{
    const long *la = (const long *)(*(const char **)a + member_offset);
    const long *lb = (const long *)(*(const char **)b + member_offset);
    return(*la - *lb);
}

static int
sort_by_int(const void *a, const void *b)
{
    const int *ia = (const int *)(*(const char **)a + member_offset);
    const int *ib = (const int *)(*(const char **)b + member_offset);
    return(*ia - *ib);
}

static int
sort_by_double(const void *a, const void *b)
{
    const double *da = (const double *)(*(const char **)a + member_offset);
    const double *db = (const double *)(*(const char **)b + member_offset);
    return((int)(*da - *db));
}

static int
sort_by_float(const void *a, const void *b)
{
    const float *fa = (const float *)(*(const char **)a + member_offset);
    const float *fb = (const float *)(*(const char **)b + member_offset);
    return((int)(*fa - *fb));
}

#define member(i) *(long *)((char *)tables[i] + offset)

/**
 * Find a CssTableClass object that has the specified member value. The member
 * data type must be long. The input Vector must first be sorted by the member.
 * Possible errors are a bad object, invalid member_name, invalid member data
 * type.  logErrorMsg is called if an error occurred.
 * @param tables A Vector of CssTableClass objects.
 * @param member_name The table member that will be searched.
 * @param value The value that will be search for.
 * @return a CssTableClass for success, NULL if not found or an error occurred.
 */
CssTableClass * CssTableClass::find(gvector<CssTableClass *> &tables,
			const string &member_name, long value)
{
    int i, jl, ju, jm, offset;
    CssTableClass *table;

    if(tables.size() <= 0) return NULL;

    table = tables[0];
    for(i = 0; i < table->_num_members
	&& strcasecmp(member_name.c_str(), table->_des[i].name); i++);

    if(i == table->_num_members) {
	snprintf(error, sizeof(error),
		"CssTableClass::find: invalid member_name: %s", member_name.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }
    if(table->_des[i].type != CSS_LONG && table->_des[i].type != CSS_JDATE) {
	snprintf(error, sizeof(error),
		"CssTableClass::find: datatype for member %s is not long",
		member_name.c_str());
	logErrorMsg(LOG_WARNING, error);
	return NULL;
    }

    offset = table->_des[i].offset;

    if(value < member(0) || value > member(tables.size()-1)) return NULL;

    if(value == member(0)) return tables[0];

    jl = -1;
    ju = tables.size();

    while(ju - jl > 1)
    {
	jm = (ju + jl)/2;

	if(value > member(jm)) {
	    jl = jm;
	}
	else {
	    ju = jm;
	}
    }
    if(value == member(jl+1)) {
	return tables[jl+1];
    }
    else {
	return NULL;
    }
}

/**
 * Read a file of CSS table records. This routine reads all of the CSS formatted
 * records from a <b>file</b> and creates new CssTableClass objects. The new CssTableClass
 * objects are returned in the Vector <b>tables</b>. If the file stat structure
 * <b>old_stat</b> is not input as zeros, this routine first checks to see if
 * the <b>file</b> has been modified since the last call. If the <b>file</b>
 * has not been modified, it is not read. Input <b>old_stat.st_dev=0,
 * old_stat.st_ino=0, old_stat.st_mtime=0</b> to force the <b>file</b> to be
 * read (the first time). Repeated calls with the same <b>file</b> will only
 * read if the file stat structure has changed (the file has been modifed).
 * The return codes are:
 * <pre>
 *	 1	A successfull read. Records are returned.
 *	 0	The file has not been modified. No records are returned.
 *	-1	the system stat() routine failed. (<b>file</b> does not exist\
 *			or cannot be accessed.
 *      -2	the input <b>table_name</b> has not been defined as a CssTableClass
 *	-3	the <b>file</b> cannot be opened.
 *	-4	malloc failed.
 *	-5	new_CssTableClasss failed.
 *	-6	a read error occurred (e.g. a incorrectly formatted record).
 * </pre>
 * @param file The name of a file containing CSS formatted records.
 * @param old_stat The a file stat structure for this file.
 * @param table_name The CssTableClass type.
 * @param tables A Vector to receive the new CssTableClass objects.
 * @param err_msg *err_msg will point to a static error message, if there is \
 *	and error return.
 * @return The return code.
 * @see Vector
 */
int CssTableClass::readFile(const string &file, struct stat *old_stat,
		const string &table_name, gvector<CssTableClass *> &tables,
		const char **err_msg)
{
    int i, num, num_members, line_length, err;
    int prefix, dir, file_q;
    struct stat buf;
    CssTableClass *table;
    CssClassDescription *des;
    FILE *fp;

    *err_msg = NULL;

    if((num_members = getDescription(table_name, &des)) < 0) {
	*err_msg = error;
	snprintf(error, sizeof(error),
		"CssTableClass::readFile: unknown table name: %s",
		table_name.c_str());
	return -2;
    }
    /* Read the file only if it has been modifed. For the first call,
     * initialize old_stat.st_dev=0, old_stat.st_ino=0, old_sta.st_mtime=0,
     * or to force a read each call, set old_stat=NULL
     */
    if(!(err = stat(file.c_str(), &buf)) && old_stat != NULL &&
	buf.st_dev == old_stat->st_dev &&
	buf.st_ino == old_stat->st_ino &&
	buf.st_mtime == old_stat->st_mtime)
    {
	/* return 0 to indicate that the file has not been modified since
	 * the last read */
	return(0);
    }
    if(old_stat != NULL) {
	*old_stat = buf;
    }
    tables.clear();

    if(err) {
	*err_msg = error;
	stringcpy(error, strerror(errno), (int)sizeof(error));
	// force another read attempt on the next call
	if(old_stat != NULL) {
	    old_stat->st_dev = old_stat->st_ino = old_stat->st_mtime = 0;
	}
	return(-1);
    }
    if((fp = fopen(file.c_str(), "r")) == NULL)
    {
	if(errno > 0) {
	    snprintf(error, sizeof(error), "Cannot open %s\n%s",
			file.c_str(), strerror(errno));
	}
	else {
	    snprintf(error, sizeof(error), "Cannot open %s", file.c_str());
	}
	return(-3);
    }
    line_length = des[num_members-1].end;

    // compute the number of tables from the file length.
    num = buf.st_size/(line_length + 1);

    prefix = stringGetPrefix(file.c_str());
    dir = stringGetDir(file.c_str());
    file_q = stringToQuark(file);

    /* read num tables
     */
    for(i = err = 0; i < num; i++)
    {
	table = createCssTable(table_name);
	if( !(err = table->read(fp, err_msg)) ) {
	    table->_dir = dir;
	    table->_prefix = prefix;
	    table->_file = file_q;
	    tables.push_back(table);
	}
	else {
	    delete table;
	    break;
	}
    }
    fclose(fp);

    // return 1 to indicate a successfull read.
    return (err == 0 || err == EOF) ? 1 : -6;
}

int CssTableClass::toBytes(char **bytes)
{
    char *p;
    const char *nam;
    int i, name_len;

    int nbytes = 2*sizeof(int); // nbytes + strlen(_name)

    nam = getName();
    name_len = (int)strlen(nam);
    nbytes += name_len;

    for(i = 0; i < _num_members; i++) { // regular members
	nbytes += _des[i].size;
    }
    for(i = 0; i < _num_extra; i++) { // extra members
	nbytes += _extra[i].size;
    }
    nbytes += 11*sizeof(int) + sizeof(double) + sizeof(bool); // source info

    *bytes = (char *)malloc(nbytes);
    if(! *bytes ) {
	logErrorMsg(LOG_ERR, "CssTableClass::toBytes: malloc failed.");
	return 0;
    }

    p = *bytes;
    memcpy(p, &nbytes, sizeof(int));	p += sizeof(int);
    memcpy(p, &name_len, sizeof(int));	p += sizeof(int);
    memcpy(p, nam, name_len);		p += name_len;

    for(i = 0; i < _num_members; i++) {
	char *member = (char *)this + _des[i].offset;
	memcpy(p, member, _des[i].size);
	p += _des[i].size;
    }

    for(i = 0; i < _num_extra; i++) {
	char *member = (char *)this + _extra[i].offset;
	memcpy(p, member, _extra[i].size);
	p += _extra[i].size;
    }

    memcpy(p, &_format, sizeof(int));		p += sizeof(int);
    memcpy(p, &_odbc_source, sizeof(int));	p += sizeof(int);
    memcpy(p, &_user, sizeof(int));		p += sizeof(int);
    memcpy(p, &_passwd, sizeof(int));		p += sizeof(int);
    memcpy(p, &_account, sizeof(int));		p += sizeof(int);
    memcpy(p, &_directory_structure, sizeof(int)); p += sizeof(int);
    memcpy(p, &_directory_duration, sizeof(double)); p += sizeof(double);
    memcpy(p, &_table_name, sizeof(int));	p += sizeof(int);
    memcpy(p, &_dir, sizeof(int));		p += sizeof(int);
    memcpy(p, &_prefix, sizeof(int));		p += sizeof(int);
    memcpy(p, &_file, sizeof(int));		p += sizeof(int);
    memcpy(p, &_file_offset, sizeof(int));	p += sizeof(int);
    memcpy(p, &_selected, sizeof(bool));	p += sizeof(bool);
    memcpy(p, &_save_ds, sizeof(bool));		p += sizeof(bool);

    return nbytes;
}

CssTableClass * CssTableClass::fromBytes(int nbytes, char *bytes)
{
    CssTableClass *table;
    int i, name_len, nb;
    char *b, name[101];
    char *member;
    char error[200];

    if(nbytes < (signed int)(2*sizeof(int))) {
	logErrorMsg(LOG_ERR, "CssTableClass::fromBytes: invalid nbytes.");
	return NULL;
    }

    memcpy(&nb, bytes, sizeof(int));
    if(nb != nbytes) {
	logErrorMsg(LOG_ERR, "CssTableClass::fromBytes: invalid nbytes.");
	return NULL;
    }

    memcpy(&name_len, bytes+sizeof(int), sizeof(int));
    if(name_len > 100) {
	logErrorMsg(LOG_ERR, "CssTableClass::fromBytes: invalid name_len.");
	return NULL;
    }
    memcpy(name, bytes+2*sizeof(int), name_len);
    name[name_len] = '\0';
	
    if( !(table = createCssTable(name)) ) return NULL;

    nb = 2*sizeof(int) + name_len; // past nbytes, name_len and name

    for(i = 0; i < table->_num_members; i++) { // regular members
	nb += table->_des[i].size;
    }
    for(i = 0; i < table->_num_extra; i++) { // extra members
	nb += table->_extra[i].size;
    }
    nb += 11*sizeof(int) + sizeof(double) + sizeof(bool); // source info

    if(nb != nbytes) {
	delete table;
	snprintf(error, sizeof(error),
		"CssTableClass::fromBytes: invalid nbytes for %s.", name);
	logErrorMsg(LOG_ERR, error);
	return NULL;
    }

    b = bytes + 2*sizeof(int) + name_len; // start after nbytes and name
    for(i = 0; i < table->_num_members; i++) {
	member = (char *)table + table->_des[i].offset;
	memcpy(member, b, table->_des[i].size);
	b += table->_des[i].size;
    }

    for(i = 0; i < table->_num_extra; i++) {
	member = (char *)table + table->_extra[i].offset;
	memcpy(member, b, table->_extra[i].size);
	b += table->_extra[i].size;
    }

    memcpy(&table->_format, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_odbc_source, b, sizeof(int));b += sizeof(int);
    memcpy(&table->_user, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_passwd, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_account, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_directory_structure, b, sizeof(int)); b += sizeof(int);
    memcpy(&table->_directory_duration, b, sizeof(double)); b += sizeof(double);
    memcpy(&table->_table_name, b, sizeof(int));b += sizeof(int);
    memcpy(&table->_dir, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_prefix, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_file, b, sizeof(int));	b += sizeof(int);
    memcpy(&table->_file_offset, b, sizeof(int));b += sizeof(int);
    memcpy(&table->_selected, b, sizeof(bool));	b += sizeof(bool);
    memcpy(&table->_save_ds, b, sizeof(bool));	b += sizeof(bool);

    return table;
}

/**
 * Redefine a CssTableClass type. Use this routine to redefine the definition
 * of a table type. (CssTableClass::define can only be called once.) Do not redefine
 * a table type that is still being used by object instances. Delete all
 * objects of that type before redefining the table.
 * @param name The table name.
 * @param num_members The number of members in the table.
 * @param des An array of num_members CssClassDescription structures.
 * @param num_extra The number of extra members.
 * @param extra An array of num_extra CssClassExtra structures.
 * @param size The total size of the subclass structure.
 */
int CssTableClass::redefine(const string &name, int num_members, CssClassDescription *des,
		int num_extra, CssClassExtra *extra, CreateCssTable create_table,
		int size)
{
    char msg[100];
    int i, q_name;

    // search for the table definition
    q_name = stringToQuark(name);
    for(i = 0; i < num_table_defs && q_name != table_defs[i].name; i++);
    if(i == num_table_defs)
    {
	/* a new definition */
	return define(name, num_members, des, num_extra, extra, create_table,
			size);
    }
    table_defs[i].name = q_name;
    table_defs[i].size = size;
    if(table_defs[i].num_members != num_members)
    {
	if(table_defs[i].des) free(table_defs[i].des);
	table_defs[i].num_members = num_members;
	table_defs[i].des = (CssClassDescription *)malloc(
				num_members*sizeof(CssClassDescription));
	if(!table_defs[i].des) {
	    snprintf(msg, sizeof(msg), "CssTableClass::redefine(%s): malloc failed.",
			name.c_str());
	    logErrorMsg(LOG_ERR, msg);
	    return -1;
	}
    }
    memcpy(table_defs[i].des, des, num_members*sizeof(CssClassDescription));

    if(table_defs[i].num_extra != num_extra)
    {
	if(table_defs[i].extra) free(table_defs[i].extra);
	table_defs[i].num_extra = num_extra;
	if(num_extra > 0) {
	    table_defs[i].extra =(CssClassExtra *)malloc(num_extra*sizeof(CssClassExtra));
	    if(!table_defs[i].extra) {
		snprintf(msg,sizeof(msg),"CssTableClass::define(%s): malloc failed.",
			name.c_str());
		free(table_defs[i].des);
		logErrorMsg(LOG_ERR, msg);
		return -1;
	    }
	    memcpy(table_defs[i].extra, extra, num_extra*sizeof(CssClassExtra));
	}
    }
    return 0;
}

void CssTableClass::setDataSource(DataSource *ds)
{
    if(ds == _data_source || !_save_ds) return;

    if(_data_source) {
	_data_source->removeOwner(this);
    }
    _data_source = ds;
    if(_data_source) {
	_data_source->addOwner(this);
    }
}

void stringToUpper(string &s) {
    char *c = strdup(s.c_str());
    stringToUpper(c);
    s.assign(c);
    free(c);
}

int stringToQuark(const string &s)
{
    return stringToQuark(s.c_str());
}

int stringUpperToQuark(const string &s)
{
    return stringUpperToQuark(s.c_str());
}
