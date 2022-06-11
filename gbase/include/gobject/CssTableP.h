#ifndef _CSS_TABLE_P_H_
#define _CSS_TABLE_P_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "gobject/CssTable.h"
#include "gobject/Hashtable.h"

/** 
 *  CssTablePart of a CssTable structure. This is the private declaration of
 *  the CssTable structure.  A CssTable is a GObject used to hold information
 *  about a processing methods for CssTable objects.  A CssTable
 *  structure is any structure that has a GObjectPart as its first member and
 *  a CssTablePart as its second member. Create a CssTablePart with
 *  new_CssTablePart().
 *  <pre>
 *  typedef struct
 *  {
 *	GObjectPart	core;
 *	CssTablePart	css;
 *  } _CssTable, *CssTable;
 *
 *  @see new_CssTablePart
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/


/** 
 *  The CssTablePart structure definition. This is the private declaration of
 *  the CssTablePart structure.
 *
 *  @member name	The table name.
 *  @member size	The number of elements in the table and in des.
 *  @member des		A CssDescription structure for the table.
 *  @member line_length The character line length for the table.
 *
 *  @see new_CssTable
 *  @see new_GObject
 *  @private
 */
typedef struct CssTable_struct
{
	/* CssTable members */
	int		css_table_flag;
	int		type;
	int		name;
	int		num_members;
	CssDescription	*des;
	int		num_extra;
	CssExtra	*extra;
	int		num_bytes;
	int		line_length;

	Hashtable	database;
	int		dc;
	int		id;
	int		copy;
	int		format;

	int		data_source;
	int		user;	/* or parameter root */
	int		passwd;	/* or segment root */
	int		account;
	int		directory_structure;
	double		directory_duration;
	int		table_name;
	int		dir;
	int		prefix;
	int		file;
	int		file_offset;
	bool		selected;
	bool		loaded;
	GObject		io;
} CssTable_Part;

#define CSSTABLE_getType(a) ((CssTable_Part *)(((CssTable)a)->css))->type
#define CSSTABLE_getName(a) quarkToString(((CssTable_Part *)(((CssTable)a)->css))->name)
#define CSSTABLE_numMembers(a) \
        ((CssTable_Part *)(((CssTable)a)->css))->num_members
#define CSSTABLE_des(a) ((CssTable_Part *)(((CssTable)a)->css))->des
#define CSSTABLE_lineLength(a) \
        ((CssTable_Part *)(((CssTable)a)->css))->line_length

#define CSSTABLE_dc(a) ((CssTable_Part *)(((CssTable)a)->css))->dc
#define CSSTABLE_id(a) ((CssTable_Part *)(((CssTable)a)->css))->id
#define CSSTABLE_copy(a) ((CssTable_Part *)(((CssTable)a)->css))->copy
#define CSSTABLE_format(a) ((CssTable_Part *)(((CssTable)a)->css))->format

#define CSSTABLE_dir(a) ((CssTable_Part *)(((CssTable)a)->css))->dir
#define CSSTABLE_prefix(a) ((CssTable_Part *)(((CssTable)a)->css))->prefix
#define CSSTABLE_file(a) ((CssTable_Part *)(((CssTable)a)->css))->file
#define CSSTABLE_fileOffset(a) \
        ((CssTable_Part *)(((CssTable)a)->css))->file_offset
#define CSSTABLE_selected(a) ((CssTable_Part *)(((CssTable)a)->css))->selected
#define CSSTABLE_loaded(a) ((CssTable_Part *)(((CssTable)a)->css))->loaded
#define CSSTABLE_IO(a) ((CssTable_Part *)(((CssTable)a)->css))->io

#define CSSTABLE_setId(a,b) ((CssTable_Part *)(((CssTable)a)->css))->id = b
#define CSSTABLE_setCopy(a,b) ((CssTable_Part *)(((CssTable)a)->css))->copy = b
#define CSSTABLE_setFormat(a,b) \
        ((CssTable_Part *)(((CssTable)a)->css))->format = b

#define CSSTABLE_setDir(a,b) ((CssTable_Part *)(((CssTable)a)->css))->dir = b
#define CSSTABLE_setPrefix(a,b) \
        ((CssTable_Part *)(((CssTable)a)->css))->prefix = b
#define CSSTABLE_setFile(a,b) ((CssTable_Part *)(((CssTable)a)->css))->file = b
#define CSSTABLE_setFileOffset(a,b) \
        ((CssTable_Part *)(((CssTable)a)->css))->file_offset = b
#define CSSTABLE_setSelected(a,b) \
        ((CssTable_Part *)(((CssTable)a)->css))->selected = b
#define CSSTABLE_setLoaded(a,b) \
        ((CssTable_Part *)(((CssTable)a)->css))->loaded = b

void predefineTables(void);

#define CSS_TABLE_FLAG 89467634

#define is_CssTable(a) \
(a && ((CssTable_Part *)(((CssTable)a)->css))->css_table_flag == CSS_TABLE_FLAG)

#endif
