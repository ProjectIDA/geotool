#ifndef _CSS_TABLE_H_
#define _CSS_TABLE_H_

#include <stdlib.h>
#include <sys/stat.h>

#include "gobject/GObject.h"
#include "gobject/Vector.h"

/**
 *  A CssTable is a GObject used to hold information about a processing
 *  CSS tables.
 *
 *  @see GObject
 *
 */

/** 
 *  CssTablePart structure. This is an opaque structure pointer here to protect
 *  its contents, which should not be changed after creation by
 *  new_CssTablePart().
 *  @see new_CssTablePart
 */
typedef struct CssTable_struct	*CssTablePart;

/** 
 *  CssTable structure. A CssTable is any structure that has a GObjectPart as
 *  its first member and a CssTablePart as its second member.
 */
typedef struct
{
	GObjectPart	core;
	CssTablePart	css;
} _CssTable, *CssTable;

#define CSS_WRONG_FORMAT 100
#define CSS_MALLOC_ERROR 101
#define CSS_BAD_OBJECT	 102
#define CSS_WRITE_ERROR  103

typedef struct
{
	int		start;
	int		end;
	char		name[32];
	char		format[8];
	int		offset;
	int		size;
	int		quark_offset;
	int		type;
	char		null_value[16];
} CssDescription, *CssDescriptionPtr;

typedef struct
{
	char		name[32];
	char		format[8];
	int		offset;
	int		size;
	int		type;
	char		null_value[16];
} CssExtra;


int	CssTable_define(const char *name, int num_members,
			CssDescription *des, int num_extra, CssExtra *extra,
			int size);
int	CssTable_redefine(const char *name, int num_members,
			CssDescription *des, int num_extra, CssExtra *extra,
			int size);
int	CssTable_getDescription(const char *name, CssDescription **des);
int	CssTable_getExtra(const char *name, CssExtra **extra);
int	CssTable_getAllNames(const char ***cssTableNames);
CssTable new_CssTable(const char *name);
int	new_CssTables(const char *name, int num, CssTable *tables);
bool	CssTable_copyTable(CssTable c1, CssTable c2, bool copy_source);
char   *CssTable_member(CssTable o, int index);

int	CssTable_read(CssTable css, FILE *fp, const char **err_msg);
int	CssTable_write(CssTable css, FILE *fp, const char **err_msg);
int	CssTable_readFile(const char *file, struct stat *old_stat,
		const char *table_name, Vector tables, const char **err_msg);

bool	CssTable_setDoubleMember(CssTable o, const char *member_name,
		double value);
bool	CssTable_setMember(CssTable o, int i, const char *value);
bool	CssTable_setExtra(CssTable o, int i, const char *value);
char   *CssTable_getError(void);
int	CssTable_getSize(CssTable css);
int	CssTable_getCssSize(CssTable css);
int	CssTable_filePosition(CssTable table);

void	CssTable_setIds(CssTable css, int dc, int id);
void	CssTable_freeIds(CssTable table);

int	CssTable_sort(Vector tables, const char *member_name);
int	CssTable_sortTable(CssTable *tables, int num,
			const char *member_name);
int	CssTable_memberIndex(CssTable o, const char *member_name);
int	CssTable_memberOffset(CssTable o, const char *member_name);
char	*CssTable_memberAddress(CssTable o, const char *member_name);
int	CssTable_memberType(CssTable o, const char *member_name);
char	*CssTable_toString(CssTable o);
int	CssTable_memberToString(CssTable o, int member_index, char *buf,
		int buf_len);

CssTable CssTable_find(Vector tables, const char *member_name, long value);

void	CssTable_setSource(CssTable table, int data_source, int user,
			int passwd);
void	CssTable_getSource(CssTable table, int *data_source, int *user,
			int *passwd);
void	CssTable_setAccount(CssTable table, int account, int table_name);
void	CssTable_getAccount(CssTable table, int *account, int *table_name);
void	CssTable_setDirectoryStructure(CssTable table, int directory_structure,
			double directory_duration);
void	CssTable_getDirectoryStructure(CssTable table, int *directory_structure,
			double *directory_duration);
void	CssTable_copySource(CssTable dest, CssTable src, int id);


int	CssTable_getType(CssTable css);
const char   *CssTable_getName(CssTable css);
int	CssTable_numMembers(CssTable css);
CssDescription *CssTable_description(CssTable o);

int	CssTable_lineLength(CssTable table);
int	CssTable_dc(CssTable css);
int	CssTable_id(CssTable css);
int	CssTable_copy(CssTable css);
int	CssTable_format(CssTable css);
int	CssTable_dir(CssTable table);
int	CssTable_prefix(CssTable table);
int	CssTable_file(CssTable table);
int	CssTable_fileOffset(CssTable table);
int	CssTable_selected(CssTable table);
int	CssTable_loaded(CssTable table);
GObject CssTable_IO(CssTable css);
void	CssTable_setId(CssTable css, int id);
void	CssTable_setFormat(CssTable css, int format);
void	CssTable_setDir(CssTable table, int dir);
void	CssTable_setPrefix(CssTable table, int dir);
void	CssTable_setFile(CssTable table, int dir);
void	CssTable_setFileOffset(CssTable table, int offset);
void	CssTable_setSelected(CssTable table, bool b);
void	CssTable_setLoaded(CssTable table, bool b);
void	CssTable_setIO(CssTable css, GObject io);
void	CssTable_init(CssTable css);
bool	CssTable_isCssTable(GObject o);
bool	CssTable_isTableName(char *name);
bool	CssTable_isMember(char *cssTableName, char *memberName);

int CssTables_archive(CssTable **t);

int CssTable_putValue(CssTable a, const char *name, const char *member_type,
			void *member_addr);
int CssTable_putBlock(CssTable a, const char *name, const char *member_type,
			int member_size, void *member_addr);
int CssTable_getValue(CssTable a, const char *name, const char *member_type,
			void *member_addr);
int CssTable_getStringValue(CssTable a, const char *name, const char *spec,
			char *buf, int buflen);
int CssTable_replaceValue(CssTable a, const char *name, const char *buf);
void CssTable_removeValue(CssTable a, const char *name);

void CssTable_terminate(void);

int CssTable_toBytes(CssTable c, char **bytes);
CssTable CssTable_fromBytes(int nbytes, char *bytes);


typedef int (*ReadFunc)(CssTable a);

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

#endif
