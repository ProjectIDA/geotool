/*	SccsId:	%W%	%G%	*/

#ifndef _LIB_FFDBP_H_
#define	_LIB_FFDBP_H_

#include <semaphore.h>
#include <stdarg.h>
#include "libFFDB.h"

/**
 * @private
 */
typedef struct
{
        char            *name;
	char		*path;
        struct stat     stat;
        Vector          records;
} StaticTable;

/**
 * @private
 */
typedef struct
{
	int		filename_q;
	struct stat	file_stat;
	FILE		*fp;
	Vector		records;
	int		pos;
	int		record_count;
} FFDB_FILE;

/**
 * @private
 */
typedef struct FFDB_Database
{
	char		*param_root;
	char		*seg_root;
	char		*directory_structure;
	int		num_static_tables;
	StaticTable	*static_tables;
	int		num_authors;
	AuthorStruct	*authors;
	int		default_author;
	double		directory_duration;
	double		tmin;
	double		tmax;
	int		num_mem_files;
	FFDB_FILE	*mem_files;
	int		mem_file_records;
	int		max_mem_file_records;
	int		read_global_tables;
} FFDatabasePart;

/**
 * @private
 */
typedef struct FFDB_QueryTable_Struct
{
	FFDatabase	ffdb;
	QParseStruct	parse_struct;
	char		*table_name;
	const char	*prefix;
	int		data_source_q;
	int		param_root_q;
	int		seg_root_q;
	int		format_q;
	int		buffer_limit;
	bool		reading_secondary;
	bool		search_error;
	int		num_fetched;
	Vector		records;
	sem_t		search_sem;
	sem_t		results_sem;
	pthread_t	thread;
} FFDB_QueryTablePart;

#ifdef HAVE_STDARG_H
void FFDBSetErrorMsg(int err, const char *format, ...);
#else
void FFDBSetErrorMsg();
#endif

FFDB_FILE *FFDBOpenFile(FFDatabase ffdb, const char *tableName,
		const char *path);
int FFDBCloseFile(FFDB_FILE *mf);
int FFDBReadFile(CssTable css, FFDB_FILE *mf, const char **err_msg);

#endif /* _LIB_FFDBP_H_ */
