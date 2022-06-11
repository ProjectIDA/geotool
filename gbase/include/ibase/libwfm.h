
/*
 * Copyright 1991 Science Applications International Corporation.
 *
 * 
 * FILE 
 *	libwfm.h
 *
 * Header file for waveform library routines.
 *
 */
/*
 * SCCSId:	@(#)libwfm/libwfm.h	120.1 05/22/97
 *
 */

/*======================================================================*/
/* INCLUDES           							*/
/*======================================================================*/
#ifndef LIBWFM_H
#define	LIBWFM_H

#include <stdio.h>
#include <string.h>
#ifndef _AESIR_H_
#include "aesir.h"      /* For Proto macro */
#endif
#ifndef DB_WFDISC_H
#include "db_wfdisc.h"	/* For wfdisc struct */
#endif

/*======================================================================*/
/* TYPEDEFS           							*/
/*======================================================================*/
typedef struct par_struct
{
	char   *in_db;
	char   *out_db;
	char   *server;
	int     port;
	char   *in_table;
	char   *sitechan_table;
	char   *in_affiliation;
	char   *out_table;
	char   *in_file;
	char   *out_file;
	char   *net;
	char   *sta_list;
	char  **sta;
	int	nsta;
	char   *chan_list;
	char  **chan;
	int	nchan;
	double	start_time;
	double	end_time;
	double	duration;
	double	extension_time;
	int	one_wfile;
	int	one_wfile_per_stachan;
	int	one_wfile_per_seg;
	char   *wfdisc_select;
	int	maxrec;
	double  fill_value;
	double	gapepsilon;
	char   *datatype;
	char   *wdirectory;
	char   *wfile_base_name;
	int	compress;         /* Begining of flags. */
	int     join;
	int	shrink_window;
	int     verbose;
	int     wfd_to_wfm;
	int     wfd_to_file;
	int     wfm_to_file;
	int     wfd_to_db;
	int     wfm_to_db;
	int     remote;
	int     sac_out;
	int     connect_to_db;
	char   *remote_log_file;
} Wfmparameter, *WfmparameterP;

typedef struct libwfm_struct
 {
 	long   ngroup;         /* Total number of groups for entire run. */
 	struct index *index;   /* Pointer to first index structure */
 	long   nwfdisc;        /* Total number of wfdiscs */
 	struct wfdisc *wfdisc; /* Pointer to first wfdisc structure. */
} Wfmxref, *WfmxrefP;

typedef struct index
{
	struct wfdisc *wfd;    /* Pointer to 1st wfdisc element of group. */
	long   nwfd;           /* Number of wfdisc tuples in group. */
	char   sta[7];         /* Station name of group. */
	char   chan[9];        /* Channel name of group. */
	struct wfmap *wfmap;   /* Pointer to wfm-to-wfd map for group. */
	long   nwfm;           /* Number of wfmem's required for group. */
	struct wfmem *wfm;     /* Pointer to first wfmem of group.*/
} Wfmindex, *WfmindexP;

typedef struct wfmap
{
	struct wfdisc * *wfd;  /* Pointer to a wfdisc structure. */
} Wfmap, *WfmapP;

typedef struct wfmem
{
	char	sta[7];
	char	chan[9];
	double	time;
	long	chanid;
	long	jdate;       
	double	endtime;
	long	nsamp;
	double	samprate;
	double	calib;
	double	calper;
	char	instype[7];
	char	segtype[2];
	char	datatype[3];

	char	clip[2];
	long	foff;
	char    *data;          /* Pointer to data stored in memory. */
} Wfmem, *WfmemP;

/*======================================================================*/
/* DEFINES           							*/
/*======================================================================*/
#define BUFSIZE 4096           /* read data buffer size */
#define ERROR -1
#define  EXT_WFDISC_FORMAT "%6c %8c %lf %ld %ld %ld %lf %ld %lf %lf %lf %6c %1c %2c %1c %s %s %ld %ld"
#define FALSE 0
#define G2 "g2"
#define MAX_SAMP 512           /* tied to BUFSIZE */
#define N2 2
#define NO_ERROR 0
#define NUM_WFDISC_FIELDS 19
#define PMODE 0644
#define S1 "s1"
#define SAC_OFFSET 632         /* size of SAC header */
#define S2 "s2"
#define S3 "s3"
#define S4 "s4"
#define T4 "t4"
#define T8 "t8"
#define TRUE 1
#define LIBWFM_SLACK_SAMPS 5


/*======================================================================*/
/* FUNCTION PROTOTYPES	(externally callable functions from libwfm	*/
/*======================================================================*/
/* From clip_wfm.c */
extern int clip_wfm(double start_time,
		    double end_time,
		    struct wfmem *wfm,
		    int nwfd);

/* From free_libwfm1.c */
extern int free_libwfm1(struct libwfm_struct **libwfm_struct);
			

/* From free_libwfm2.c */
extern int free_libwfm2(struct libwfm_struct **libwfm_struct);

/* From getwfm1.c */
extern int getwfm1(struct par_struct * par,
                   struct libwfm_struct **libwfm_struct);
			
/* From getwfm2.c */
extern int getwfm2(Wfmparameter * par,
		   Wfdisc * wfdisc,
		   int nwfdisc,
		   Wfmxref ** libwfm_struct);

/* From read_libwfm_par.c */
extern struct par_struct *read_libwfm_par(int argc,
					  char ** argv);

/* From wrtwave.c */
extern int wrtwave(Wfmparameter *par,
		   Wfmxref * libwfm_struct);

/* From open_tmp_wfdisc_file.c */
extern FILE *open_tmp_wfdisc_file(char * filename);

/* From write_sac_header.c */
extern int write_sac_header(int fdwr,
			    struct wfdisc * wfd);

/* from build_index.c */
extern int build_index(struct wfdisc * wfdisc,
		       int nwfdisc,
		       struct index ** index);

/* From num_wfm.c */
extern int  num_wfm(int    join,
		    double gapepsilon,
		    Wfdisc *wfd,
		    int    nwfd);

/* From write_tup_2db.c */
extern int write_tup_2db(struct par_struct *par,
			 struct index *index,
			 struct wfdisc *wfd,
			 int    ntup,
			 int   *last_wfid);

/* From write_tup_2file.c */
extern int write_tup_2file(FILE * fp,
			   struct wfdisc *wfd);

/* From new_w_name.c */
extern int new_w_name(char *wfile_base_name,
		      int   wfid,
		      char *wfile,
		      char *New);

/* From strip.c */
extern int split(char *lstr,
		 char *rstr,
		 char  token[1],
		 int   fwd_rev);

/* From open_wfile.c */
extern int open_wfile(struct par_struct *par,
		      int    wfid,
		      double time1,
		      int   *wfir,
		      char  *wdir,
		      char  *owfile);

extern int prep_wfile(struct par_struct *par,
		      int    wfid,
		      double time1,
		      char  *wdir,
		      char  *tempfile);

/* From rd_wr.c */
extern int write_r(int     fd,
		   char    *buffer,
		   int     n,
		   int     timeout);

extern int read_r(int     fd,
		  char    *buffer,
		  int     n,
		  int     timeout);

/* From compfuncs.c */
/* conflicts with compress in /usr/include/zlib.h
extern int  compress(int * in,
                     char * out,
		     int num);
*/

extern void mydiff2(int * in,
		    int * out,
		    int n,
		    int init);

extern int dcompress(char * in,
		     int * out,
		     int num);

extern void int2(int * in,
		 int * out,
		 int n,
		 int init);

/* From cnt_nwfd.c */
extern int cnt_nwfd(Wfmap wfmap);

/* From window_extremes.c */
extern int window_extremes(struct  par_struct *par,
			   Wfmap wfmap,
			   int     nwfd,
			   double *t1,
			   double *t2);

/* From cp_wfd_wfd.c */
extern void cp_wfd_wfd(struct wfdisc * wfd,
		       struct wfdisc * wfdout);

/* From update_wfd.c */
extern void update_wfd(double start,
		       double end,
		       int    nsamp,
		       char   out_datatype[3],
		       double  calib,
		       char   dir[65],
		       char   dfile[33],
		       int    wfid,
		       int    foff,
		       struct wfdisc *wfdout);

/* From fill.c */
extern int fill(char * buff,
		int numwds,
		char * datatype,
		double fill_value);

/* from calc_nsamp.c */
extern int calc_nsamp(double tdiff,
		      double samprate);

extern double  calc_endtime(double tdiff,
			    int nsamp,
			    double samprate);
extern int	nsamp_overlap(double tdiff,
			      double samprate);
extern int	nsamp_clip(double tdiff,
			   double samprate);
extern int	nsamp_end_gap(double tdiff,
			      double samprate);
extern int	nsamp_mid_gap(double tdiff,
			      double samprate);


/* From read_wfdisc.c */
extern int   read_wfdisc(Wfdisc *wfd,
			 char   *ptr,
			 int    sampin,
			 int    nsamp,
			 char   *datatype,
			 int    do_calib);


/* from wfdutils.c */
extern void sort_wfdisc_by_time(Wfdisc *wfdisc,
				int    nwfdisc);
extern void  sort_wfdisc_by_interval(Wfdisc *wfdisc,
				 int    nwfdisc,
				 double t1, 
				 double	t2);
extern int  wfdisc_sta_chan_subarray(Wfdisc *wfdisc,
				     int    nwfdisc,
				     char   *sta,
				     char   *chan,
				     Wfdisc **subarray);
extern int  wfdisc_interval_subarray(Wfdisc *wfdisc,
				     int    nwfdisc,
				     char   *sta,
				     char   *chan,
				     double t1, 
				     double t2,
				     Wfdisc **subarray);


/*======================================================================*/
/* FUNCTION PROTOTYPES	(externally callable funcs from old libwfdisc	*/
/*======================================================================*/

/* from bit_shift.c */
extern void  b_rshift(unsigned long * input,
		      int nwords,
		      int size);

extern void b_lshift(unsigned long * input,
		     int nwords,
		     int size);

extern long b_rval(unsigned long input,
		   int size);

extern long  b_lval(unsigned long input,
		    int size);

/* from cf_get_buff.c */
extern int cf_get_buff(char    *buff,
		       char    *buff_dattyp,
		       char    *data,
		       char    *data_dattyp,
		       int     numwds,
		       double  mult);
                           
/* From e_format.c */
extern int e_decompress(unsigned long *in,
			unsigned long *out);
extern int e_compress(long *in, int len, unsigned long *out, int outlen,
		      int last, int *ncomp, long *wbuf);

/* From float.c */
extern void  h_itof(register long* from,
		    register float* to,
		    register int num);

extern void h_htof(register short* from,
		   register float* to,
		   register int num);

extern int  h_dtof(register double* from,
		   register float* to,
		   register int num);

/* From fortran_int_.c */
extern void  wfopen_(int     *fd,
		     char    sta[],
		     char    chan[],
		     double  *time1,
		     long    *wfid,
		     long    *chanid,
		     long    *jdate,
		     double  *endtime,
		     long    *nsamp,
		     float   *samprate,
		     float   *calib,
		     float   *calper,
		     char    instype[],
		     char    segtype[],
		     char    datatype[],
		     char    clip[],
		     char    dir[],
		     char    dfile[],
		     long    *foff,
		     long    *commid,
		     char    lddate[],
		     char    type[],
		     char    mode[],
		     int     len_sta,
		     int     len_chan,
		     int     len_instype,
		     int     len_segtype,
		     int     len_datatype,
		     int     len_clip,
		     int     len_dir,
		     int     len_dfile,
		     int     len_lddate,
		     int     len_type,
		     int     len_mode);

extern void   wfseek_(int     *fd,
		      int *seek_to,
		      int *how,
		      int *ierr);

extern void  wfread_(int     *fd,
		     char buffer[],
		     int *nsamp,
		     int *ngot,
		     int len_buffer);

extern void  wfclose_(int     *fd,
		      int 	*ierr);

extern void   wfsetdatatype_(int     *fd,
			     char datatype[],
			     int *ierr,
			     int len_datatype);

extern void  wfsetcalib_ (int     *fd,
			  double *calib);

extern int  wfwrite_(int     *fd,
		     char buffer[],
		     int *nsamp,
		     int *ngot,
		     int len_buffer);

extern void wfgeterror_(char buf[],
			int * bufsiz,
			int 	*ierr);

extern int   wfdisctruncate_(int     *fd,
			     int 	*ierr);

extern void  wfgetwfdisc_(int     *fd,
			  char    sta[],
			  char    chan[],
			  double  *time1,
			  long    *wfid,
			  long    *chanid,
			  long    *jdate,
			  double  *endtime,
			  long    *nsamp,
			  float   *samprate,
			  float   *calib,
			  float   *calper,
			  char    instype[],
			  char    segtype[],
			  char    datatype[],
			  char    clip[],
			  char    dir[],
			  char    dfile[],
			  long    *foff,
			  long    *commid,
			  char    lddate[],
			  int     *ierr,
			  int     len_sta,
			  int     len_chan,
			  int     len_instype,
			  int     len_segtype,
			  int     len_datatype,
			  int     len_clip,
			  int     len_dir,
			  int     len_dfile,
			  int     len_lddate);

/* From get_bpw.c */
extern int    get_bpw(char * type_string);

extern int get_equivalent_datatype(char * type_string, char *eq_type);

/* From io.c */
extern int  get_index(int fd);

extern void  wfdisc_io_init(void);

extern int  w_open(struct wfdisc *wfdisc_p,
		   char	* type,
		   int	mode);

extern int   w_seek(int fd,
		    int seek_to,
		    int how);

extern int  w_read(int fd,
		   char * buffer,
		   int nsamp);

extern int  w_close(int fd);

extern int   w_set_datatype(int fd,
			    char * datatype);

extern void  w_set_calib(int fd,
			 double calib);

extern int  w_write(int fd,
		    char * buffer,
		    int nsamp);

extern int  get_w_error(char * w_err_text,
			int bufsiz);

extern int  wfdisc_truncate(int fd);

extern Wfdisc *w_get_wfdisc(int fd);


/* From short.c */
extern void vhtoih(register char	*from,
		   register char	*to,
		   register int	num);

extern void ihtovh(register char	*from,
		   register char	*to,
		   register int 	num);

/* From long.c */
extern void  vitoii(register char* from,
		    register char* to,
		    register int num);

extern void  iitovi(register char* from,
		    register char* to,
		    register int num);

/* From float.c */
extern void  vftoif(register void* from,
		    register void* to,
		    register int num);

extern void iftovf(register void* from,
		   register void* to,
		   register int num);


/* From double.c */
extern void  vdtoid(register void* from,
		    register void* to,
		    register int num);

extern void idtovd(register void* from,
		   register void* to,
		   register int num);

/* From unlink_wfmxref.c */ 
extern int unlink_wfmxref(Wfmxref * wfmxref,
			  Wfmem ** wfmem);  

extern Wfmem *destroy_wfmem(int    nwf,
			    Wfmem *wfmem);


	/*
	 *  Prototypes for various data format conversion functions. 
	 */


extern void  wfm_g2toint(
			 unsigned char	*from, 
			 long		*to, 
			 int		num 
			 );

extern void   wfm_inttog2(
			  long		*from, 
			  unsigned char	*to, 
			  int		num
			  );

extern void	wfm_floattog2(
			      float		*from, 
			      unsigned char	*to, 
			      int			num
			      );

extern void	wfm_s3tos4(
			   unsigned char   *s3,
			   int     	*s4,
			   int        	n
			   );

extern void	wfm_s4tos3(
			   unsigned char   *s4, 
			   unsigned char   *s3, 
			   int		n
			   );

extern void	wfm_a2tot4(
			   unsigned short	*a2, 
			   float		*t4, 
			   int		n
			   );

extern void	wfm_t4toa2(
			   float  *t4, 
			   short  *a2, 
			   int      n
			   );

extern void	wfm_i2toint(
			    short	*from, 
			    long	*to, 
			    int	num
			    );

extern void	wfm_t4toint(
			    float	*from, 
			    long	*to, 
			    int	num
			    );

extern void	wfm_i4toint(
			    char	*from, 
			    char	*to, 
			    int	num
			    );

extern void	wfm_s4toint(
			    char	*from, 
			    char	*to,  
			    int	num   
			    );

extern void	wfm_g2tofloat(
			      unsigned char	*from, 
			      float		*to, 
			      int		 num 
			      );

extern void	wfm_i2tofloat(
			      short	*from,
			      float	*to, 
			      int		num
			      );

extern void	wfm_i4tofloat(
			      int		*from, 
			      float	*to, 
			      int		num 
			      );

extern void	wfm_s4tofloat(
			      int		*from, 
			      float	*to, 
			      int		num
			      );

extern void	wfm_t4tofloat(
			      float	*from, 
			      float       *to, 
			      int		num
			      );

/*
 * Fortran interfaces to above routines
 */

extern void	wfg2toint_(
			   unsigned char	*from, 
			   long		*to, 
			   int		*num
			   );


extern void	wfg2tofloat_(
			     unsigned char	*from, 
			     float		*to, 
			     int		*num
			     );

extern void	wfinttog2_(
			   long			*from, 
			   unsigned char	*to, 
			   int			*num
			   );

extern void	wffloattog2_(
			     float		*from, 
			     unsigned char	*to, 
			     int		*num
			     );

extern void	wfi2toint_(
			   short	*from, 
			   long		*to, 
			   int		*num
			   );

extern void	wfi2tofloat_(
			     short	*from, 
			     float	*to, 
			     int	*num
			     );

extern void	wfi4tofloat_(
			     int	*from, 
			     float	*to, 
			     int	*num
			     );

extern void	wfs4tofloat_(
			     int	*from, 
			     float	*to, 
			     int	*num
			     );

extern void	wft4tofloat_(
			     float	*from, 
			     float	*to, 
			     int	*num
			     );

extern void	wfa2tot4_(
			  short	*from, 
			  float	*to, 
			  int	*num
			  );

extern void	wft4toa2_(
			  float	*from, 
			  short	*to, 
			  int	*num
			  );

extern void	wfs3tos4_(
			  char	*from,
			  long	*to, 
			  int	*num
			  );

extern void	wfs4tos3_(
			  long	*from, 
			  char	*to, 
			  int	*num
			  );

#endif /* LIBWFM_H */
