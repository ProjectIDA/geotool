#ifndef _LIB_GPARSEP_H_
#define _LIB_GPARSEP_H_

#include "libgparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#ifndef MAXPATHLEN
#include <sys/param.h>
#endif

#ifdef HAVE_LIBODBC
#include "libgODBC.h"
#endif

#include "gobject/cssObjects.h"
#include "libBeam.h"
#include "crust.h"
#include "gobject/CssTable.h"
#include "css/cssAll.h"
#include "sac.h"

#include "response.h"
#include "Attribute.h"
#include "gobject/GMethod.h"
#include "libgcss.h"
#include "libFFDB.h"
#include "libgdraw.h"
#include "libFK.h"
#include "libFT2.h"
#include "libstalta.h"

typedef void * Widget;

typedef struct
{
	char	color_code[20];
	int	data_height;
	int	data_separation;
	int	initial_align;
	int	initial_sort;
	int	display_components;
	int	display_arrivals;
	bool	display_tags;
	bool	display_amplitude_scale;
} Properties;

typedef struct crustalModel
{
	char *name;
	float h[3];
	float vp[4];
	float vs[4];
} CrustalModel;

#define CHANGE_TIME             1
#define CHANGE_AMP_PER          2
#define CHANGE_PHASE_NAME       4
#define CHANGE_STASSID		8
#define CHANGE_AZIMUTH		16
#define CHANGE_SLOW		32


typedef void (*SaveModule)(Widget w, FILE *fp);

typedef struct
{
	char		*name;		/* ie FK, FT, Spectrogram, etc. */
	SaveModule	writeModule;
	SaveModule	readModule;
	int		num_instances;
} Module;

typedef struct
{
	int	dc, id, row, column;
	double	reset_time, new_time;
} Retime;


typedef struct
{
        int     	dc;
        int     	id;
	CPlotData	*cd;
	GObject		s;
} UniqueId;

typedef struct
{
        char    sta[8];
        char    phase[9];
        long    orid;
        int     assoc_index;
        int     arr_index;
        int     arr_id;
        int     arr_dc;
        double  delta;
        double  time;
} ArrAssocSort;

typedef struct
{
	char	*source;
	char	**dest;
	int	ndest;
} Accel;

typedef struct
{
	GObjectPart	core;

        /* RecentInput members */
	bool		read_all;
	char		format[50];
	int		nchan;
	char		**chan_list;
	int		nfiles;
	char		**files;
	int		nwavs;
	int		*wavs;
	int		num_cd;
	int		*cd_ids;
	CPlotData	**cd_list;
	CssOrigin	origin;
} _RecentInput, *RecentInput;


typedef struct
{
	int	nfiles;
	char	**files;
	char	format[50];
} RecentListing;

typedef struct
{
	int	dc, id, row, lat_column, lon_column;
	double	reset_lat, reset_lon;
        double	new_lat, new_lon;
} Relocate;


typedef struct
{
	char		popup[64];
	char		tag[256];
} TagStore;

typedef struct
{
	int		type;
	char		*label;
	Hashtable	hashtable;
} UndoAction;

#define UNDO_ARRIVAL_DELETE	1
#define UNDO_ARRIVAL_EDIT	2
#define UNDO_ORIGIN_DELETE	3
#define UNDO_ORIGIN_EDIT	4
#define UNDO_AMPLITUDE_EDIT	5

typedef struct
{
	char	*file;
} RecentProject;

typedef struct
{
	char 	*name;
	int	ndays;
	int	*day;
} DBSummary;

typedef struct
{
	char		*path;
	int		nsta;
	DBSummary	*staDB;

	int		nauth;
	DBSummary	*authDB;
} FileDB;

typedef struct
{
	int		type;
	int		row;
	CssTable	previous;
	Vector		cut_records;
	int		*cut_rows;
	int		num_paste;
	bool		saved;
} UndoRecord;

typedef bool (*PrintMethod)(Widget w, FILE *fp, char *printDialog,
			PrintStruct *p);

typedef struct
{
	char		*name;
	PrintMethod	printMethod;
} PrintMethodStruct;

typedef struct
{
	unsigned char	n;
	char		*line;
} GeotoolResource;

typedef struct
{
	char	*popup;
	int	start_line;
	int	required;
	char	*depends;
} ResourceFile;

typedef struct
{
	Vector		tables;
	Vector		waveforms;
} GeotoolStruct;

typedef struct
{
	char		*name;
	GMethodCreate	methodCreate;
} GMethod_Type;

#define DATA_SOURCE_NONE	0
#define DATA_SOURCE_PREFIX	1
#define DATA_SOURCE_FFDB	2
#define DATA_SOURCE_ODBC	3

typedef struct GParseEnv_struct
{
	int		source_type;
	FFDatabase	ffdb;
#ifdef HAVE_LIBODBC
	SQLHDBC		hdbc;
#endif
	GeotoolStruct	*geotools;
	int		num_geotools;
	int		current_g;
	int		data_source;
	const char	*args;

	int		verbose;
	int		num_tags;
	int		num_formats;
	FormatStruct	*formats;
	int		working_dialog_threshold;
	int		filter_dialog_threshold;
	int		low_swap;
	int		num_waveform_colors;
	char		**waveform_colors;
	char		*priority_file;
	char		*geo_table_dir;
	char		*tmp_dir;
	char		*backup_file;
	int		num_arrival_colors;
	int		*arrival_fg;
	char		*arrival_colors;
	char		*author_names;
	char		**authors;
	int		num_authors;
	int		sound;
	int		max_recent_input;

	int		num_crusts;
	CrustModel	*cm;

	char		*crust_models;
	char		*recipe_dir;
	char		*recipe_dir2;
	char		*lock_position;
	char		*chan_sort_order0;	/* first char  */
	char		*chan_sort_order1;	/* middle char  */
	char		*chan_sort_order2;      /* last  char  */
	char		*tag_members;
	char		**tags;
	char		*format_list;

	char		*iaspei_table;
	char		*jb_table;
	char		*map_dir;

	char		*file_listing_sort;
	char		*arrival_listing_sort;
	char		*amp_per_listing_sort;
	char		*origin_listing_sort;
	char		*stassoc_listing_sort;
	char		*hydro_features_listing_sort;
	char		*infra_features_listing_sort;
	char		*amplitude_listing_sort;
	char		*ampdescript_listing_sort;
	char		*filter_listing_sort;

	char		*arrival_attribute_list;
	char		*amp_per_attribute_list;
	char		*origin_attribute_list;
	char		*stassoc_attribute_list;
	char		*hydro_features_attribute_list;
	char		*infra_features_attribute_list;
	char		*amplitude_attribute_list;
	char		*ampdescript_attribute_list;
	char		*filter_attribute_list;
	char		*arrival_attributes;
	char		*amp_per_attributes;
	char		*origin_attributes;
	char		*stassoc_attributes;
	char		*hydro_features_attributes;
	char		*infra_features_attributes;
	char		*amplitude_attributes;
	char		*ampdescript_attributes;
	char		*filter_attributes;

	char		*affiliation_table;
	char		*arrival_table;
	char		*amplitude_table;
	char		*ampdescript_table;
	char		*assoc_table;
	char		*instrument_table;
	char		*instrument_table2;
	char		*lastid_table;
	char		*origerr_table;
	char		*origin_table;
	char		*sensor_table;
	char		*site_table;
	char		*sitechan_table;
	char		*staconf_table;
	char		*sensor_table2;
	char		*site_table2;
	char		*sitechan_table2;
	char		*stassoc_table;
	char		*hydro_features_table;
	char		*infra_features_table;
	char		*stamag_table;
	char		*netmag_table;
	char		*wftag_table;


	char		*fm_list;
	char		*clip_list;
	char		*qual_list;
	char		*stype_list;
	char		*timedef_list;
	char		*azdef_list;
	char		*slodef_list;

	char		*initial_alignment;

	char		*browser;

	char		start_phase[12];
	char		end_phase[12];
	double		start_phase_lead;
	double		end_phase_lag;
	double		start_time;
	double		end_time;
	double		window_length;

	bool		do_auto_amp_per;
	bool		preview_arr;
	bool		join_all_segs;
	bool		warn_if_hidden;
	bool		arr_sta_to_net;

	int		num_dynamic_libs;
	int		*dynamic_libs;

	double		join_time_limit;
	double		overlap_limit;
	double		sp_arrival_window;
	double		lp_arrival_window;

	double		default_deltim;
	double		default_delaz;
	double		default_delslo;

	double		last_iaspei_depth;

	int		num_origin_recipes;
	BeamRecipe	*origin_recipes;
	int		num_detection_recipes;
	BeamRecipe	*detection_recipes;
	int		num_groups;
	BeamGroup	*groups;

	FTParam		ft_param;

	FKParam		fk_param;
	StaLtaParam	stalta_param;

	int		num_modules;
	Module		*modules;

	int		num_methods;
	GMethod_Type	*methods;

	int 		num_print_methods;
	PrintMethodStruct *print_methods;

	bool		import_line;
	int		num_import_tables;
	char		**import_names;
	int		num_import_members;
	char		**import_members;
	int		num_import_ids;
	char		**import_ids;

	Vector		import_lines;

/* begin magic */
	int		top_n_requested;
	int		top_n_driver;
	int		top_n_to_consider;

        int             min_n_phase_for_event;  /* min number of phases needed to declare an event */
        int             n_driver_sta;           /* number of driver stations to consider when building events */

        double  min_lat;
        double  max_lat;
        double  min_lon;
        double  max_lon;

	double	azDevFactor;
	double	slowDevFactor;
	double	timeDevFactor;

        char    *ref_origin_table;
        char    *ref_arrival_table;
        char    *ref_assoc_table;
        char    *input_arrival_table;

        char    *cmp1_assoc_table;
        char    *cmp1_arrival_table;
        char    *cmp2_assoc_table;
        char    *cmp2_arrival_table;
        char    *cmp3_assoc_table;
        char    *cmp3_arrival_table;

        char    *cmp1_origin_table;
        char    *cmp2_origin_table;
        char    *cmp3_origin_table;

        char    *magic_input_file;
/* end */

} GParseEnvStruct;

PSWaveform
new_PSWaveform(TimeSeries ts, double y0, char *sta, char *chan, char *net,
		double lat, double lon);
void GParseFKInit(FKParam *p);
void GParseFTInit(FTParam *p);


#endif	/* _LIB_GPARSEP_H_ */
