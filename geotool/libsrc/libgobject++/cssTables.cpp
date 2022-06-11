/** \file cssTables.cpp
 *  \brief Defines schema subclasses of CssTableClass.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include "gobject++/CssTables.h"

#define T_NULL	"none"
#define DTNULL	"-"

#define offset(field) ((unsigned int)(((const char *)&a.field) - ((const char *)&a))), sizeof(a.field)
#define OffSet(field) ((unsigned int)(((const char *)&a.field) - ((const char *)&a)))


/*---------------------------------------------------------------------------*/
/**
 * Define CssArrival.
 */
static void defineArrival(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssArrivalClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",    "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    24,     "time", "%.5lf", offset(time),    0, CSS_TIME, T_NULL},
    {26,   33,     "arid",   "%ld", offset(arid),    0, CSS_LONG,   "-1"},
    {35,   42,    "jdate",   "%ld", offset(jdate),   0, CSS_JDATE,  "-1"},
    {44,   51,  "stassid",   "%ld", offset(stassid), 0, CSS_LONG,   "-1"},
    {53,   60,   "chanid",   "%ld", offset(chanid),  0, CSS_LONG,   "-1"},
    {62,   69,     "chan",    "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {71,   78,   "iphase",    "%s", offset(iphase),  0, CSS_STRING, "-"},
    {80,   80,    "stype",    "%s", offset(stype),   0, CSS_STRING, "-"},
    {82,   87,   "deltim", "%.3lf", offset(deltim),  0, CSS_DOUBLE, "-1."},
    {89,   95,  "azimuth", "%.2lf", offset(azimuth), 0, CSS_DOUBLE, "-1."},
    {97,  103,    "delaz", "%.2lf", offset(delaz),   0, CSS_DOUBLE, "-1."},
    {105, 111,     "slow", "%.2lf", offset(slow),    0, CSS_DOUBLE, "-1."},
    {113, 119,   "delslo", "%.2lf", offset(delslo),  0, CSS_DOUBLE, "-1."},
    {121, 127,      "ema", "%.2lf", offset(ema),     0, CSS_DOUBLE, "-1."},
    {129, 135,     "rect", "%.3lf", offset(rect),    0, CSS_DOUBLE, "-1."},
    {137, 146,      "amp",   "%lf", offset(amp),     0, CSS_DOUBLE, "-1."},
    {148, 154,      "per", "%.2lf", offset(per),     0, CSS_DOUBLE, "-1."},
    {156, 162,    "logat",   "%lf", offset(logat),   0, CSS_DOUBLE, "-1."},
    {164, 164,     "clip",    "%s", offset(clip),    0, CSS_STRING, "-"},
    {166, 167,       "fm",    "%s", offset(fm),	     0, CSS_STRING, "-"},
    {169, 178,      "snr",   "%lf", offset(snr),     0, CSS_DOUBLE, "-1."},
    {180, 180,     "qual",    "%s", offset(qual),    0, CSS_STRING, "-"},
    {182, 196,     "auth",    "%s", offset(auth),    0, CSS_STRING, "-"},
    {198, 205,   "commid",   "%ld", offset(commid),  0, CSS_LONG,   "-1"},
    {207, 223,   "lddate",    "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"ts_dc",         "%d", offset(ts_dc),	 CSS_INT,	"-1"},
    {"ts_id",         "%d", offset(ts_id),	 CSS_INT,	"-1"},
    {"ts_copy",       "%d", offset(ts_copy),	 CSS_INT,	"-1"},
    {"phase",         "%s", offset(phase),	 CSS_STRING,	"-"},
    {"amp_cnts",   "%.2lf", offset(amp_cnts),	 CSS_DOUBLE,	"-1."},
    {"amp_Nnms",   "%.2lf", offset(amp_Nnms),	 CSS_DOUBLE,	"-1."},
    {"amp_nms",    "%.2lf", offset(amp_nms),	 CSS_DOUBLE,	"-1."},
    {"zp_Nnms",    "%.2lf", offset(zp_Nnms),	 CSS_DOUBLE,	"-1."},
    {"period",     "%.2lf", offset(period),	 CSS_DOUBLE,	"-1."},
    {"box_location",  "%d", offset(box_location),CSS_BOOL,	"false"},
    {"boxtime",    "%.5lf", offset(boxtime),	 CSS_TIME,	T_NULL},
    {"boxmin",     "%.2lf", offset(boxmin),	 CSS_DOUBLE,	"0"},
    {"atype",         "%d", offset(atype),	 CSS_INT,	"-1"},
    {"value",      "%.2lf", offset(value),	 CSS_DOUBLE,	"0."},
    {"pick_file",     "%d", offset(pick_file),	 CSS_QUARK,	""},
    {"wftag_file",    "%d", offset(wftag_file),	 CSS_QUARK,	""},
    {"filter_file",   "%d", offset(filter_file), CSS_QUARK,	""},
    {"pick_index",    "%d", offset(pick_index),	 CSS_INT,	"-1"},
    {"wftag_index",   "%d", offset(wftag_index), CSS_INT,	"-1"},
    {"filter_index",  "%d", offset(filter_index),CSS_INT,	"-1"},
    {"sta_quark",     "%d", offset(sta_quark),	 CSS_QUARK,	""},
    {"chan_quark",    "%d", offset(chan_quark),	 CSS_QUARK,	""},
    {"net_quark",     "%d", offset(net_quark),	 CSS_QUARK,	""},
    };
    CssTableClass::define(cssArrival, sizeof(des)/sizeof(CssClassDescription), des,
		sizeof(extra)/sizeof(CssClassExtra), extra,
		CssArrivalClass::createArrival, sizeof(CssArrivalClass));
  }
}

CssArrivalClass::CssArrivalClass(void) : CssTableClass(cssArrival)
{
    defineArrival();
    initTable();
}

CssArrivalClass::CssArrivalClass(CssArrivalClass &a) : CssTableClass(cssArrival)
{
    defineArrival();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssWfdiscClass.
 */
static void defineWfdisc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssWfdiscClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   33,     "time","%.5lf", offset(time),    0, CSS_TIME,	T_NULL},
    {35,   42,     "wfid",  "%ld", offset(wfid),    0, CSS_LONG,	"-1"},
    {44,   51,   "chanid",  "%ld", offset(chanid),  0, CSS_LONG,	"-1"},
    {53,   60,    "jdate",  "%ld", offset(jdate),   0, CSS_JDATE,	"-1"},
    {62,   78,  "endtime","%.5lf", offset(endtime), 0, CSS_TIME,	T_NULL},
    {80,   87,    "nsamp",  "%ld", offset(nsamp),   0, CSS_LONG,	"-1"},
    {89,   99, "samprate","%.7lf", offset(samprate),0, CSS_DOUBLE,	"-1."},
    {101, 116,    "calib","%.6lf", offset(calib),   0, CSS_DOUBLE,	"0."},
    {118, 133,   "calper","%.6lf", offset(calper),  0, CSS_DOUBLE,	"-1."},
    {135, 140,  "instype",   "%s", offset(instype), 0, CSS_STRING,	"-"},
    {142, 142,  "segtype",   "%s", offset(segtype), 0, CSS_STRING,	"-"},
    {144, 145, "datatype",   "%s", offset(datatype),0, CSS_STRING,	"-"},
    {147, 147,     "clip",   "%s", offset(clip),    0, CSS_STRING,	"-"},
    {149, 212,      "dir",   "%s", offset(dir),     0, CSS_STRING,	"-"},
    {214, 245,    "dfile",   "%s", offset(dfile),   0, CSS_STRING,	"-"},
    {247, 256,     "foff",  "%ld", offset(foff),    0, CSS_LONG,	"0"},
    {258, 265,   "commid",  "%ld", offset(commid),  0, CSS_LONG,	"-1"},
    {267, 283,   "lddate",   "%s", offset(lddate),  0, CSS_LDDATE,	DTNULL},
    };
    CssClassExtra extra[] = {
        {"sta_quark", "%d", offset(sta_quark),  CSS_QUARK, ""},
        {"chan_quark", "%d", offset(chan_quark), CSS_QUARK, ""},
    };
    CssTableClass::define(cssWfdisc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssWfdiscClass::createWfdisc, sizeof(CssWfdiscClass));
  }
}

CssWfdiscClass::CssWfdiscClass(void) : CssTableClass(cssWfdisc)
{
    defineWfdisc();
    initTable();
}

CssWfdiscClass::CssWfdiscClass(CssWfdiscClass &a) : CssTableClass(cssWfdisc)
{
    defineWfdisc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/*
 * define CssHistoryClass
 */

static void defineHistory(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssHistoryClass a(0);
    CssClassDescription des[] = {
    {1,     64,      "dir",    "%s",  offset(dir),     0, CSS_STRING,  "-"},
    {66,    97,    "dfile",    "%s",  offset(dfile)  , 0, CSS_STRING,  "-"},
    {99,   108,  "binfoff",   "%ld",  offset(binfoff), 0, CSS_LONG,    "0"},
    {110,  119,  "clffoff",   "%ld",  offset(clffoff), 0, CSS_LONG,    "0"},
    {121,  130,  "foffdff",   "%ld",  offset(foffdff), 0, CSS_LONG,    "0"},
    {132,  148,     "time", "%.5lf",  offset(time),    0, CSS_TIME, T_NULL},
    {150,  159,      "pid",   "%ld",  offset(pid),     0, CSS_LONG,    "0"},
    {161,  184,     "host",    "%s",  offset(host),    0, CSS_STRING,  "-"},
    {186,  201,   "lddate",    "%s",  offset(lddate),  0, CSS_LDDATE,  DTNULL},
    };
    CssTableClass::define(cssHistory, sizeof(des)/sizeof(CssClassDescription), des, 0,
		NULL, CssHistoryClass::createHistory, sizeof(CssHistoryClass));
  }
}

CssHistoryClass::CssHistoryClass(void) : CssTableClass(cssHistory)
{
    defineHistory();
    initTable();
}

CssHistoryClass::CssHistoryClass(CssHistoryClass &a) : CssTableClass(cssHistory)
{
    defineHistory();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssChannameClass.
 */
static void defineChanname(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssChannameClass a(0);
    CssClassDescription des[] = {
    {1, 6,       "station",  "%s", offset(station),     0, CSS_STRING, "-"},
    {8, 13,       "stream",  "%s", offset(stream),      0, CSS_STRING, "-"},
    {15, 20,  "extern_sta",  "%s", offset(extern_sta),  0, CSS_STRING, "-"},
    {22, 29, "extern_chan",  "%s", offset(extern_chan), 0, CSS_STRING, "-"},
    {31, 36,  "intern_sta",  "%s", offset(intern_sta),  0, CSS_STRING, "-"},
    {38, 45, "intern_chan",  "%s", offset(intern_chan), 0, CSS_STRING, "-"},
    {47, 50,  "capability", "%ld", offset(capability),  0, CSS_LONG,   "0"},
    {52, 55,    "position", "%ld", offset(position),    0, CSS_LONG,   "0"},
    {57, 60,    "revision", "%ld", offset(revision),    0, CSS_LONG,   "0"},
    {62, 78,      "lddate",  "%s", offset(lddate),      0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssChanname, sizeof(des)/sizeof(CssClassDescription), des, 0,
		NULL, CssChannameClass::createChanname, sizeof(CssChannameClass));
  }
}

CssChannameClass::CssChannameClass(void) : CssTableClass(cssChanname)
{
    defineChanname();
    initTable();
}

CssChannameClass::CssChannameClass(CssChannameClass &a) : CssTableClass(cssChanname)
{
    defineChanname();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFileproductClass.
 */
static void defineFrameProduct(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFrameProductClass a(0);
    CssClassDescription des[] = {
    {1,     10,     "frid",   "%ld", offset(frid),     0, CSS_LONG,    "0"},
    {12,    21,   "typeid",  " %ld", offset(type_id),  0, CSS_LONG,    "0"},
    {23,    86,      "dir",    "%s", offset(dir),      0, CSS_STRING,  "-"},
    {88,   119,    "dfile",    "%s", offset(dfile),    0, CSS_STRING,  "-"},
    {121,  130,     "foff",   "%ld", offset(foff),     0, CSS_LONG,    "0"},
    {132,  141,    "dsize",   "%ld", offset(dsize),    0, CSS_LONG,    "0"},
    {143,  159,     "time", "%.5lf", offset(time),     0, CSS_TIME, T_NULL},
    {161,  177,  "endtime", "%.5lf", offset(endtime),  0, CSS_TIME, T_NULL},
    {179,  184,      "sta",    "%s", offset(sta),      0, CSS_STRING,  "-"},
    {186,  193,     "chan",    "%s", offset(chan),     0, CSS_STRING,  "-"},
    {195,  210,   "author",    "%s", offset(author),   0, CSS_STRING,  "-"},
    {212,  228,  "version", "%.2lf", offset(version),  0, CSS_DOUBLE, "0."},
    {230,  233, "revision",   "%ld", offset(revision), 0, CSS_LONG,    "0"},
    {235,  235, "obsolete",   "%ld", offset(obsolete), 0, CSS_LONG,    "0"},
    {237,  253,   "lddate",    "%s", offset(lddate),   0, CSS_LDDATE,  DTNULL},
    };
    CssTableClass::define(cssFrameProduct, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssFrameProductClass::createFrameProduct,
		sizeof(CssFrameProductClass));
  }
}

CssFrameProductClass::CssFrameProductClass(void) : CssTableClass(cssFrameProduct)
{
    defineFrameProduct();
    initTable();
}

CssFrameProductClass::CssFrameProductClass(CssFrameProductClass &a) : CssTableClass(cssFrameProduct)
{
    defineFrameProduct();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFpDescriptionClass.
 */
static void defineFpDescription(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFpDescriptionClass a(0);
    CssClassDescription des[] = {
    {1,     10,      "typeid", "%ld", offset(type_id),    0, CSS_LONG,   "0"},
    {12,    23,    "prodtype",  "%s", offset(prodtype),   0, CSS_STRING, "-"},
    {25,    88,        "name",  "%s", offset(name),       0, CSS_STRING, "-"},
    {90,   105,    "msgdtype",  "%s", offset(msgdtype),   0, CSS_STRING, "-"},
    {107,  114,  "msgdformat",  "%s", offset(msgdformat), 0, CSS_STRING, "-"},
    {116,  125, "header_fpid", "%ld", offset(header_fpid),0, CSS_LONG,   "0"},
    {127,  143,      "lddate",  "%s", offset(lddate),     0, CSS_LDDATE,DTNULL},
    };
    CssTableClass::define(cssFpDescription, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssFpDescriptionClass::createFpDescription,
		sizeof(CssFpDescriptionClass));
  }
}

CssFpDescriptionClass::CssFpDescriptionClass(void) : CssTableClass(cssFpDescription)
{
    defineFpDescription();
    initTable();
}

CssFpDescriptionClass::CssFpDescriptionClass(CssFpDescriptionClass &a) :
				CssTableClass(cssFpDescription)
{
    defineFpDescription();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssMd5DigestClass.
 */
static void defineMd5Digest(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssMd5DigestClass a(0);
    CssClassDescription des[] = {
    {1,     64,     "dir",   "%s", offset(dir),    0, CSS_STRING, "-"},
    {66,    97,   "dfile",   "%s", offset(dfile),  0, CSS_STRING, "-"},
    {99,   108,   "inode",  "%ld", offset(inode),  0, CSS_LONG,   "0"},
    {110,  141,  "digest",   "%s", offset(digest), 0, CSS_STRING, "-"},
    {143,  159,  "lddate",   "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssMd5_digest, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssMd5DigestClass::createMd5Digest,
		sizeof(CssMd5DigestClass));
  }
}

CssMd5DigestClass::CssMd5DigestClass(void) : CssTableClass(cssMd5_digest)
{
    defineMd5Digest();
    initTable();
}

CssMd5DigestClass::CssMd5DigestClass(CssMd5DigestClass &a) : CssTableClass(cssMd5_digest)
{
    defineMd5Digest();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssClfClass.
 */
static void defineClf(void)
{
  static bool defined = false;
  if( ! defined ) {
    defined = true;
    CssClfClass a(0);
    CssClassDescription des[] = {
    {1,    10,        "frid",   "%ld", offset(frid),        0, CSS_LONG, "0"},
    {12,   49,     "frameid",    "%s", offset(frameid),     0, CSS_STRING, "-"},
    {51,   67,        "time", "%.5lf", offset(time),        0, CSS_TIME,T_NULL},
    {69,   85,   "timestamp", "%.5lf", offset(timestamp),   0, CSS_TIME,T_NULL},
    {87,  103, "receivespan", "%.5lf", offset(receivespan), 0, CSS_DOUBLE, "0"},
    {105, 114,        "foff",   "%ld", offset(foff),        0, CSS_LONG, "0"},
    {116, 125,      "length",   "%ld", offset(length),      0, CSS_LONG, "0"},
    {127, 133,    "duration",   "%ld", offset(duration),    0, CSS_LONG, "0"},
    {135, 136,   "frametype",    "%s", offset(frametype),   0, CSS_STRING, "-"},
    {138, 141, "verifstatus",    "%s", offset(verifstatus), 0, CSS_STRING, "-"},
    {143, 150,       "pcode",    "%s", offset(pcode),       0, CSS_STRING, "-"},
    {152, 215,        "mask",    "%s", offset(mask),        0, CSS_STRING, "-"},
    {217, 220,    "revision",   "%ld", offset(revision),    0, CSS_LONG, "-1"},
    {222, 238,      "lddate",    "%s", offset(lddate),      0, CSS_LDDATE, "-"},
    };
    CssTableClass::define(cssClf, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssClfClass::createClf, sizeof(CssClfClass));
  }
}

CssClfClass::CssClfClass(void) : CssTableClass(cssClf)
{
    defineClf();
    initTable();
}

CssClfClass::CssClfClass(CssClfClass &a) : CssTableClass(cssClf)
{
    defineClf();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssWaveIntervalClass.
 */
static void defineWaveInterval(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssWaveIntervalClass a(0);
    CssClassDescription des[] = {
    {1,    10,        "wintid",   "%ld", offset(wintid),        0, CSS_LONG, "0"},
    {12,   17,       "station",    "%s", offset(station),       0, CSS_STRING, "-"},
    {19,   35,          "time", "%.5lf", offset(time),          0, CSS_TIME, T_NULL},
    {37,   53,       "endtime", "%.5lf", offset(endtime),       0, CSS_TIME, T_NULL},
    {55,   61,   "percentdata",  "%.3f", offset(percentdata),   0, CSS_FLOAT, "0."},
    {63,   69,  "percentchans",  "%.3f", offset(percentchans),  0, CSS_FLOAT, "0."},
    {71,   77, "percentusable",  "%.3f", offset(percentusable), 0, CSS_FLOAT, "0."},
    {79,   85, "percenttimely",  "%.3f", offset(percenttimely), 0, CSS_FLOAT, "0."},
    {87,   93,        "status",    "%s", offset(status),        0, CSS_STRING, "-"},
    {95,  158,          "mask",    "%s", offset(mask),          0, CSS_STRING, "-"},
    {160, 176,    "createdate",    "%s", offset(createdate),    0, CSS_LDDATE, DTNULL},
    {178, 194,        "lddate",    "%s", offset(lddate),        0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssWaveinterval, sizeof(des)/sizeof(CssClassDescription), des,
	0, NULL, CssWaveIntervalClass::createWaveInterval, sizeof(CssWaveIntervalClass));
  }
}

CssWaveIntervalClass::CssWaveIntervalClass(void) : CssTableClass(cssWaveinterval)
{
    defineWaveInterval();
    initTable();
}

CssWaveIntervalClass::CssWaveIntervalClass(CssWaveIntervalClass &a) : CssTableClass(cssWaveinterval)
{
    defineWaveInterval();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssOutageClass.
 */
static void defineOutage(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssOutageClass a(0);
    CssClassDescription des[] = {
    {1,     10,     "otgid",    "%ld", offset(otgid),     0, CSS_LONG,    "0"},
    {12,    17,       "sta",     "%s", offset(sta),       0, CSS_STRING,  "-"},
    {19,    26,      "chan",     "%s", offset(chan),      0, CSS_STRING,  "-"},
    {28,    31,     "auxid",     "%s", offset(auxid),     0, CSS_STRING,  "-"},
    {33,    49,      "time",  "%.5lf", offset(time),      0, CSS_TIME, T_NULL},
    {51,    67,   "endtime",  "%.5lf", offset(endtime),   0, CSS_TIME, T_NULL},
    {69,   100,    "status",     "%s", offset(status),    0, CSS_STRING,  "-"},
    {102,  116,      "auth",     "%s", offset(auth),      0, CSS_STRING,  "-"},
    {118,  118, "available",     "%s", offset(available), 0, CSS_STRING,  "-"},
    {120,  127,    "commid",    "%ld", offset(commid),    0, CSS_LONG,    "0"},
    {129,  145,    "lddate",     "%s", offset(lddate),    0, CSS_LDDATE,DTNULL},
    };
    CssTableClass::define(cssOutage, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssOutageClass::createOutage, sizeof(CssOutageClass));
  }
}

CssOutageClass::CssOutageClass(void) : CssTableClass(cssOutage)
{
    defineOutage();
    initTable();
}

CssOutageClass::CssOutageClass(CssOutageClass &a) : CssTableClass(cssOutage)
{
    defineOutage();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssOriginClass.
 */
static void defineOrigin(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssOriginClass a(0);
    CssClassDescription des[] =
    {
    {1,     9,      "lat", "%.4lf",offset(lat),   0, CSS_DOUBLE,  "-999."},
    {11,   19,      "lon", "%.4lf",offset(lon),   0, CSS_DOUBLE,  "-999."},
    {21,   29,    "depth", "%.4lf",offset(depth), 0, CSS_DOUBLE,  "-999."},
    {31,   47,     "time", "%.5lf",offset(time),  0, CSS_TIME, T_NULL},
    {49,   56,     "orid",  "%ld", offset(orid),  0, CSS_LONG,   "-1"},
    {58,   65,     "evid",  "%ld", offset(evid),  0, CSS_LONG,   "-1"},
    {67,   74,    "jdate",  "%ld", offset(jdate), 0, CSS_JDATE,   "-1"},
    {76,   79,     "nass",  "%ld", offset(nass),  0, CSS_LONG,   "-1"},
    {81,   84,     "ndef",  "%ld", offset(ndef),  0, CSS_LONG,   "-1"},
    {86,   89,      "ndp",  "%ld", offset(ndp),   0, CSS_LONG,   "-1"},
    {91,   98,      "grn",  "%ld", offset(grn),   0, CSS_LONG,   "-1"},
    {100, 107,      "srn",  "%ld", offset(srn),   0, CSS_LONG,   "-1"},
    {109, 115,    "etype",   "%s", offset(etype), 0, CSS_STRING,   "-"},
    {117, 125,    "depdp", "%.4lf",offset(depdp), 0, CSS_DOUBLE,   "-999."},
    {127, 127,    "dtype",   "%s", offset(dtype), 0, CSS_STRING,  "-"},
    {129, 135,       "mb", "%.2lf",offset(mb),    0, CSS_DOUBLE,   "-999."},
    {137, 144,     "mbid",  "%ld", offset(mbid),  0, CSS_LONG,   "-1"},
    {146, 152,       "ms", "%.2lf",offset(ms),    0, CSS_DOUBLE,   "-999."},
    {154, 161,     "msid",  "%ld", offset(msid),  0, CSS_LONG,   "-1"},
    {163, 169,       "ml", "%.2lf",offset(ml),    0, CSS_DOUBLE,   "-999."},
    {171, 178,     "mlid",  "%ld", offset(mlid),  0, CSS_LONG,   "-1"},
    {180, 194,"algorithm",   "%s", offset(algorithm),0, CSS_STRING,"-"},
    {196, 210,     "auth",   "%s", offset(auth),  0, CSS_STRING,   "-"},
    {212, 219,   "commid",  "%ld", offset(commid),0, CSS_LONG,   "-1"},
    {221, 237,   "lddate",   "%s", offset(lddate),0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"wftag_file",    "%d", offset(wftag_file),    CSS_QUARK, ""},
    {"origerr_file",  "%d", offset(origerr_file),  CSS_QUARK, ""},
    {"wftag_index",   "%d", offset(wftag_index),   CSS_INT, "-1"},
    {"origerr_index", "%d", offset(origerr_index), CSS_INT, "-1"},
    };
    CssTableClass::define(cssOrigin, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssOriginClass::createOrigin, sizeof(CssOriginClass));
  }
}

CssOriginClass::CssOriginClass(void) : CssTableClass(cssOrigin)
{
    defineOrigin();
    initTable();
}

CssOriginClass::CssOriginClass(CssOriginClass &a) : CssTableClass(cssOrigin)
{
    defineOrigin();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssOrigerrClass.
 */
static void defineOrigerr(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssOrigerrClass a(0);
    CssClassDescription des[] = {
    {1,     8,   "orid",   "%ld",  offset(orid),   0, CSS_LONG,   "-1"},
    {10,   24,    "sxx", "%.4lf",  offset(sxx),	   0, CSS_DOUBLE, "-1."},
    {26,   40,    "syy", "%.4lf",  offset(syy),	   0, CSS_DOUBLE, "-1."},
    {42,   56,    "szz", "%.4lf",  offset(szz),	   0, CSS_DOUBLE, "-1."},
    {58,   72,    "stt", "%.4lf",  offset(stt),	   0, CSS_DOUBLE, "-1."},
    {74,   88,    "sxy", "%.4lf",  offset(sxy),	   0, CSS_DOUBLE, "-1."},
    {90,  104,    "sxz", "%.4lf",  offset(sxz),	   0, CSS_DOUBLE, "-1."},
    {106, 120,    "syz", "%.4lf",  offset(syz),	   0, CSS_DOUBLE, "-1."},
    {122, 136,    "stx", "%.4lf",  offset(stx),	   0, CSS_DOUBLE, "-1."},
    {138, 152,    "sty", "%.4lf",  offset(sty),	   0, CSS_DOUBLE, "-1."},
    {154, 168,    "stz", "%.4lf",  offset(stz),	   0, CSS_DOUBLE, "-1."},
    {170, 178,  "sdobs", "%.4lf",  offset(sdobs),  0, CSS_DOUBLE, "-1."},
    {180, 188, "smajax", "%.4lf",  offset(smajax), 0, CSS_DOUBLE, "-1."},
    {190, 198, "sminax", "%.4lf",  offset(sminax), 0, CSS_DOUBLE, "-1."},
    {200, 205, "strike", "%.2lf",  offset(strike), 0, CSS_DOUBLE, "-1."},
    {207, 215, "sdepth", "%.4lf",  offset(sdepth), 0, CSS_DOUBLE, "-1."},
    {217, 224,  "stime", "%.2lf",  offset(stime),  0, CSS_DOUBLE, "-1."},
    {226, 230,   "conf", "%.3lf",  offset(conf),   0, CSS_DOUBLE, "0."},
    {232, 239, "commid",   "%ld",  offset(commid), 0, CSS_LONG,   "-1"},
    {241, 257, "lddate",    "%s",  offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssOrigerr, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssOrigerrClass::createOrigerr,
		sizeof(CssOrigerrClass));
  }
}

CssOrigerrClass::CssOrigerrClass(void) : CssTableClass(cssOrigerr)
{
    defineOrigerr();
    initTable();
}

CssOrigerrClass::CssOrigerrClass(CssOrigerrClass &a) : CssTableClass(cssOrigerr)
{
    defineOrigerr();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssOrigauxClass.
 */
static void defineOrigaux(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssOrigauxClass a(0);
    CssClassDescription des[] = {
    {1,     8,   "event",    "%s",  offset(event),  0, CSS_STRING, "-"},
    {10,   10,  "otfixf",    "%s",  offset(otfixf), 0, CSS_STRING, "-"},
    {12,   12,  "epfixf",    "%s",  offset(epfixf), 0, CSS_STRING, "-"},
    {14,   21,    "nsta",   "%ld",  offset(nsta),   0, CSS_LONG, "-1"},
    {23,   25,     "gap",   "%ld",  offset(gap),    0, CSS_LONG, "-1"},
    {27,   34,   "ident",    "%s",  offset(ident),  0, CSS_STRING, "-"},
    {36,   41, "mindist", "%.2lf",  offset(mindist),0, CSS_DOUBLE, "-1."},
    {43,   48, "maxdist", "%.2lf",  offset(maxdist),0, CSS_DOUBLE, "-1."},
    {50,   50,  "antype",    "%s",  offset(antype), 0, CSS_STRING, "-"},
    {52,   59,    "evid",   "%ld",  offset(evid),   0, CSS_LONG, "-1"},
    {61,   68,    "orid",   "%ld",  offset(orid),   0, CSS_LONG, "-1"},
    {70,   86,  "lddate",    "%s",  offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssOrigaux, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssOrigauxClass::createOrigaux, sizeof(CssOrigauxClass));
  }
}

CssOrigauxClass::CssOrigauxClass(void) : CssTableClass(cssOrigaux)
{
    defineOrigaux();
    initTable();
}

CssOrigauxClass::CssOrigauxClass(CssOrigauxClass &a) : CssTableClass(cssOrigaux)
{
    defineOrigaux();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssLastidClass.
 */
static void defineLastid(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssLastidClass a(0);
    CssClassDescription des[] = {
    {1,    15,  "keyname",  "%s", offset(keyname),  0, CSS_STRING,	"-"},
    {17,   24, "keyvalue", "%ld", offset(keyvalue), 0, CSS_LONG,	"-1"},
    {26,   42,   "lddate",  "%s", offset(lddate),   0, CSS_LDDATE,    DTNULL},
    };
    CssTableClass::define(cssLastid, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssLastidClass::createLastid, sizeof(CssLastidClass));
  }
}

CssLastidClass::CssLastidClass(void) : CssTableClass(cssLastid)
{
    defineLastid();
    initTable();
}

CssLastidClass::CssLastidClass(CssLastidClass &a) : CssTableClass(cssLastid)
{
    defineLastid();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssGregionClass.
 */
static void defineGregion(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGregionClass a(0);
    CssClassDescription des[] = {
    {1,    3,      "grn",  "%ld", offset(grn),      0, CSS_LONG,        "-1"},
    {5,   44,    "grname",  "%s", offset(grname),   0, CSS_STRING,  	  "-"},
    {46,   54,    "lddate",   "%s", offset(lddate),  0, CSS_STRING,   "-"},
    };
    CssTableClass::define(cssGregion, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGregionClass::createGregion, sizeof(CssGregionClass));
  }
}

CssGregionClass::CssGregionClass(void) : CssTableClass(cssGregion)
{
    defineGregion();
    initTable();
}

CssGregionClass::CssGregionClass(CssGregionClass &a) : CssTableClass(cssGregion)
{
    defineGregion();
    initTable();
    a.copyTo(this);
}



/*---------------------------------------------------------------------------*/
/** Define CssSensorClass.
 */
static void defineSensor(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssSensorClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   33,     "time","%.5lf", offset(time),    0, CSS_TIME, T_NULL},
    {35,   51,  "endtime","%.5lf", offset(endtime), 0, CSS_TIME, T_NULL},
    {53,   60,     "inid",  "%ld", offset(inid),    0, CSS_LONG,   "-1"},
    {62,   69,   "chanid",  "%ld", offset(chanid),  0, CSS_LONG,   "-1"},
    {71,   78,    "jdate",  "%ld", offset(jdate),   0, CSS_JDATE,  "-1"},
    {80,   95, "calratio",  "%lf", offset(calratio),0, CSS_DOUBLE,  "0."},
    {97,  112,   "calper",  "%lf", offset(calper),  0, CSS_DOUBLE,  "-1."},
    {114, 119,   "tshift",  "%lf", offset(tshift),  0, CSS_DOUBLE,  "0."},
    {121, 121,  "instant",   "%s", offset(instant), 0, CSS_STRING, "-"},
    {123, 139,   "lddate",   "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),   CSS_QUARK, ""},
    {"chan_quark", "%d", offset(chan_quark),  CSS_QUARK, ""},
    };
    CssTableClass::define(cssSensor, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssSensorClass::createSensor, sizeof(CssSensorClass));
  }
}

CssSensorClass::CssSensorClass(void) : CssTableClass(cssSensor)
{
    defineSensor();
    initTable();
}

CssSensorClass::CssSensorClass(CssSensorClass &a) : CssTableClass(cssSensor)
{
    defineSensor();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssInstrumentClass.
 */
static void defineInstrument(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssInstrumentClass a(0);
    CssClassDescription des[] = {
    {1,     8,     "inid", "%ld", offset(inid),    0, CSS_LONG,   "-1"},
    {10,   59,  "insname",  "%s", offset(insname), 0, CSS_STRING, "-"},
    {61,   66,  "instype",  "%s", offset(instype), 0, CSS_STRING, "-"},
    {68,   68,     "band",  "%s", offset(band),    0, CSS_STRING, "-"},
    {70,   70,  "digital",  "%s", offset(digital), 0, CSS_STRING, "-"},
    {72,   82, "samprate", "%lf", offset(samprate),0, CSS_DOUBLE,  "-1."},
    {84,   99,   "ncalib", "%lf", offset(ncalib),  0, CSS_DOUBLE,  "0."},
    {101, 116,  "ncalper", "%lf", offset(ncalper), 0, CSS_DOUBLE,  "-1."},
    {118, 181,      "dir",  "%s", offset(dir),     0, CSS_STRING, "-"},
    {183, 214,    "dfile",  "%s", offset(dfile),   0, CSS_STRING, "-"},
    {216, 221,  "rsptype",  "%s", offset(rsptype), 0, CSS_STRING, "-"},
    {223, 239,   "lddate",  "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssInstrument, sizeof(des)/sizeof(CssClassDescription), des, 0,
		NULL, CssInstrumentClass::createInstrument, sizeof(CssInstrumentClass));
  }
}

CssInstrumentClass::CssInstrumentClass(void) : CssTableClass(cssInstrument)
{
    defineInstrument();
    initTable();
}

CssInstrumentClass::CssInstrumentClass(CssInstrumentClass &a) : CssTableClass(cssInstrument)
{
    defineInstrument();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssSitechanClass.
 */
static void defineSitechan(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssSitechanClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",  "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",  "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   24,   "ondate", "%ld", offset(ondate), 0, CSS_JDATE,  "-1"},
    {26,   33,   "chanid", "%ld", offset(chanid), 0, CSS_LONG,   "-1"},
    {35,   42,  "offdate", "%ld", offset(offdate),0, CSS_JDATE,  "-1"},
    {44,   47,    "ctype",  "%s", offset(ctype),  0, CSS_STRING, "-"},
    {49,   57,   "edepth", "%lf", offset(edepth), 0, CSS_DOUBLE,  "-999."},
    {59,   64,     "hang", "%lf", offset(hang),   0, CSS_DOUBLE,  "-999."},
    {66,   71,     "vang", "%lf", offset(vang),   0, CSS_DOUBLE,  "-999."},
    {73,  122,  "descrip",  "%s", offset(descrip),0, CSS_STRING, "-"},
    {124, 140,   "lddate",  "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),   CSS_QUARK, ""},
    {"chan_quark", "%d", offset(chan_quark),  CSS_QUARK, ""},
    {"chan_alt", "%d", offset(chan_alt),  CSS_QUARK, ""},
    };
    CssTableClass::define(cssSitechan, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssSitechanClass::createSitechan, sizeof(CssSitechanClass));
  }
}

CssSitechanClass::CssSitechanClass(void) : CssTableClass(cssSitechan)
{
    defineSitechan();
    initTable();
}

CssSitechanClass::CssSitechanClass(CssSitechanClass &a) : CssTableClass(cssSitechan)
{
    defineSitechan();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssSiteClass.
 */
static void defineSite(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssSiteClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",    "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,   "ondate",   "%ld", offset(ondate),  0, CSS_JDATE, "-1"},
    {17,   24,  "offdate",   "%ld", offset(offdate), 0, CSS_JDATE, "-1"},
    {26,   34,      "lat", "%.4lf", offset(lat),     0, CSS_DOUBLE,"-999."},
    {36,   44,      "lon", "%.4lf", offset(lon),     0, CSS_DOUBLE,"-999."},
    {46,   54,     "elev", "%.4lf", offset(elev),    0, CSS_DOUBLE,"-999."},
    {56,   105, "staname",    "%s", offset(staname), 0, CSS_STRING, "-"},
    {107,  110, "statype",    "%s", offset(statype), 0, CSS_STRING, "-"},
    {112,  117,  "refsta",    "%s", offset(refsta), OffSet(refsta_quark), CSS_STRING, "-"},
    {119,  127,  "dnorth", "%.4lf", offset(dnorth),  0, CSS_DOUBLE, "0."},
    {129,  137,   "deast", "%.4lf", offset(deast),   0, CSS_DOUBLE, "0."},
    {139,  155,  "lddate",    "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),    CSS_QUARK, ""},
    {"refsta_quark", "%d", offset(refsta_quark), CSS_QUARK, ""},
    };
    CssTableClass::define(cssSite, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssSiteClass::createSite, sizeof(CssSiteClass));
  }
}

CssSiteClass::CssSiteClass(void) : CssTableClass(cssSite)
{
    defineSite();
    initTable();
}

CssSiteClass::CssSiteClass(CssSiteClass &a) : CssTableClass(cssSite)
{
    defineSite();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssWftagClass.
 */
static void defineWftag(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssWftagClass a(0);
    CssClassDescription des[] = {
    {1,     8,  "tagname",  "%s", offset(tagname), 0, CSS_STRING, "-"},
    {10,   17,    "tagid", "%ld", offset(tagid),   0, CSS_LONG,  "-1"},
    {19,   26,     "wfid", "%ld", offset(wfid),    0, CSS_LONG,  "-1"},
    {28,   44,   "lddate",  "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssWftag, sizeof(des)/sizeof(CssClassDescription),
			  des, 0, NULL, CssWftagClass::createWftag, sizeof(CssWftagClass));
  }
}

CssWftagClass::CssWftagClass(void) : CssTableClass(cssWftag)
{
    defineWftag();
    initTable();
}

CssWftagClass::CssWftagClass(CssWftagClass &a) : CssTableClass(cssWftag)
{
    defineWftag();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssXftagClass.
 */
static void defineXtag(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssXtagClass a(0);
    CssClassDescription des[] = {
    {1,     8,    "thisid",   "%ld", offset(thisid),   0, CSS_LONG, "-1"},
    {10,   17,    "thatid",   "%ld", offset(thatid),   0, CSS_LONG, "-1"},
    {19,   26,  "thisname",    "%s", offset(thisname), 0, CSS_STRING, "-"},
    {28,   35,  "thatname",    "%s", offset(thatname), 0, CSS_STRING, "-"},
    {37,   62,    "dbname",    "%s", offset(dbname),   0, CSS_STRING, "-"},
    {64,   81,    "lddate",    "%s", offset(lddate),   0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"tagid",     "%ld", offset(tagid),		CSS_LONG,	"0"},
    {"wfid",      "%ld", offset(wfid), 		CSS_LONG,	"0"},
    {"minTime", "%.2lf", offset(mintime), 	CSS_TIME,	T_NULL},
    {"maxTime", "%.2lf", offset(maxtime), 	CSS_TIME,	T_NULL},
    {"sta",        "%s", offset(sta),	 	CSS_STRING,	""},
    {"dir",        "%s", offset(dir), 		CSS_STRING,	""},
    };
    CssTableClass::define(cssXtag, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssXtagClass::createXtag, sizeof(CssXtagClass));
  }
}

CssXtagClass::CssXtagClass(void) : CssTableClass(cssXtag)
{
    defineXtag();
    initTable();
}

CssXtagClass::CssXtagClass(CssXtagClass &a) : CssTableClass(cssXtag)
{
    defineXtag();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFsdiscClass.
 */
static void defineFsdisc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFsdiscClass a(0);
    CssClassDescription des[] = {
    {1,     8,    "jdate",  "%ld", offset(jdate),   0, CSS_JDATE, "-1"},
    {10,   26,     "time","%.5lf", offset(time),    0, CSS_TIME, T_NULL},
    {28,   33,     "tlen",  "%lf", offset(tlen),    0, CSS_DOUBLE, "-1."},
    {35,   40,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {42,   49,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {51,   54,   "fstype",   "%s", offset(fstype),  0, CSS_STRING, "-"},
    {56,   63,     "arid",  "%ld", offset(arid),    0, CSS_LONG, "-1"},
    {65,   73,     "maxf",  "%lf", offset(maxf),    0, CSS_DOUBLE,  "-1."},
    {75,   82,       "nf",  "%ld", offset(nf),      0, CSS_LONG,    "-1"},
    {84,   94, "samprate",  "%lf", offset(samprate),0, CSS_DOUBLE,  "-1."},
    {96,  103,   "chanid",  "%ld", offset(chanid),  0, CSS_LONG, "-1"},
    {105, 112,     "wfid",  "%ld", offset(wfid),    0, CSS_LONG, "-1"},
    {114, 121,    "fsrid",  "%ld", offset(fsrid),   0, CSS_LONG, "-1"},
    {123, 130,     "fsid",  "%ld", offset(fsid),    0, CSS_LONG, "-1"},
    {132, 133, "datatype",   "%s", offset(datatype),0, CSS_STRING, "-"},
    {135, 198,      "dir",   "%s", offset(dir),     0, CSS_STRING, "-"},
    {200, 231,    "dfile",   "%s", offset(dfile),   0, CSS_STRING, "-"},
    {233, 242,     "foff",  "%ld", offset(foff),    0, CSS_LONG, "-1"},
    {244, 251,   "commid",  "%ld", offset(commid),  0, CSS_LONG, "-1"},
    {253, 269,   "lddate",   "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK, ""},
    {"chan_quark", "%d", offset(chan_quark),CSS_QUARK, ""},
    };
    CssTableClass::define(cssFsdisc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssFsdiscClass::createFsdisc, sizeof(CssFsdiscClass));
  }
}

CssFsdiscClass::CssFsdiscClass(void) : CssTableClass(cssFsdisc)
{
    defineFsdisc();
    initTable();
}

CssFsdiscClass::CssFsdiscClass(CssFsdiscClass &a) : CssTableClass("arrival")
{
    defineFsdisc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFsrecipeClass.
 */
static void defineFsrecipe(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFsrecipeClass a(0);
    CssClassDescription des[] = {
    {1,     8,      "fsrid","%ld", offset(fsrid),      0, CSS_LONG, "-1"},
    {10,   24,     "fsdesc", "%s", offset(fsdesc),     0, CSS_STRING, "-"},
    {26,   33,      "taper", "%s", offset(taper),      0, CSS_STRING, "-"},
    {35,   37, "taperstart","%ld", offset(taperstart), 0, CSS_LONG, "-1"},
    {39,   41,   "taperend","%ld", offset(taperend),   0, CSS_LONG, "-1"},
    {43,   51,     "winlen","%lf", offset(winlen),     0, CSS_DOUBLE, "-1."},
    {53,   55,    "overlap","%ld", offset(overlap),    0, CSS_LONG, "-1"},
    {57,   64,       "nfft","%ld", offset(nfft),       0, CSS_LONG, "-1"},
    {66,   70,"smoothvalue","%lf", offset(smoothvalue),0, CSS_DOUBLE, "-1."},
    {72,   72,   "response", "%s", offset(response),   0, CSS_STRING, "-"},
    {74,   90,     "lddate", "%s", offset(lddate),     0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssFsrecipe, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssFsrecipeClass::createFsrecipe, sizeof(CssFsrecipeClass));
  }
}

CssFsrecipeClass::CssFsrecipeClass(void) : CssTableClass(cssFsrecipe)
{
    defineFsrecipe();
    initTable();
}

CssFsrecipeClass::CssFsrecipeClass(CssFsrecipeClass &a) : CssTableClass(cssFsrecipe)
{
    defineFsrecipe();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFstagClass.
 */
static void defineFstag(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFstagClass a(0);
    CssClassDescription des[] = {
    {1,     8,    "afsid", "%ld", offset(afsid), 0, CSS_LONG, "-1"},
    {10,   17,     "fsid", "%ld", offset(fsid),  0, CSS_LONG, "-1"},
    {19,   35,   "lddate",  "%s", offset(lddate),0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssFstag, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssFstagClass::createFstag, sizeof(CssFstagClass));
  }
}

CssFstagClass::CssFstagClass(void) : CssTableClass(cssFstag)
{
    defineFstag();
    initTable();
}

CssFstagClass::CssFstagClass(CssFstagClass &a) : CssTableClass(cssFstag)
{
    defineFstag();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssSpdiscClass.
 */
static void defineSpdisc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssSpdiscClass a(0);
    CssClassDescription des[] = {
    {1,     8,   "jdate",  "%ld", offset(jdate),  0, CSS_JDATE, "-1"},
    {10,   26,    "time","%.5lf", offset(time),  0, CSS_TIME,T_NULL},
    {28,   33,    "tlen",   "%f", offset(tlen),    0, CSS_DOUBLE, "-1."},
    {35,   40,     "sta",   "%s", offset(sta),OffSet(sta_quark), CSS_STRING,"-"},
    {42,   49,   "chan",    "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {51,   58,  "winpts",  "%ld", offset(winpts),  0, CSS_LONG,    "-1"},
    {60,   62, "overlap",  "%ld", offset(overlap), 0, CSS_LONG,    "-1"},
    {64,   68,    "nwin",  "%ld", offset(nwin),    0, CSS_LONG,    "-1"},
    {70,   75,  "lofreq",  "%lf", offset(lofreq),  0, CSS_DOUBLE, "-1."},
    {77,   82,  "hifreq",  "%lf", offset(hifreq),  0, CSS_DOUBLE, "-1."},
    {84,   88,      "nf",  "%ld", offset(nf),      0, CSS_LONG,    "-1"},
    {90,   97,    "spid",  "%ld", offset(spid),    0, CSS_LONG,   "-1"},
    {99,  100,"datatype",   "%s", offset(datatype),0, CSS_STRING, "-"},
    {102, 165,     "dir",   "%s", offset(dir),     0, CSS_STRING, "-"},
    {167, 198,   "dfile",   "%s", offset(dfile),   0, CSS_STRING, "-"},
    {200, 209,    "foff",  "%ld", offset(foff),    0, CSS_LONG,  "-1"},
    {211, 218,  "commid",  "%ld", offset(commid),  0, CSS_LONG,  "-1"},
    {220, 236,  "lddate",   "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark",  "%d", offset(sta_quark),  CSS_QUARK,	""},
    {"chan_quark", "%d", offset(chan_quark), CSS_QUARK,	""},
    };
    CssTableClass::define(cssSpdisc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssSpdiscClass::createSpdisc, sizeof(CssSpdiscClass));
  }
}

CssSpdiscClass::CssSpdiscClass(void) : CssTableClass(cssSpdisc)
{
    defineSpdisc();
    initTable();
}

CssSpdiscClass::CssSpdiscClass(CssSpdiscClass &a) : CssTableClass(cssSpdisc)
{
    defineSpdisc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssDervdiscClass.
 */
static void defineDervdisc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssDervdiscClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   33,     "time","%.5lf", offset(time),      0, CSS_TIME,T_NULL},
    {35,   45,     "tlen",  "%lf", offset(tlen),      0, CSS_DOUBLE, "-1"},
    {47,   54,      "net",   "%s", offset(net),       0, CSS_STRING, "-"},
    {56,   63,   "dervid",  "%ld", offset(dervid),    0, CSS_LONG,   "-1"},
    {65,   72,    "recid",  "%ld", offset(recid),     0, CSS_LONG,   "-1"},
    {75,   91,   "method",   "%s", offset(method),    0, CSS_STRING, "-"},
    {93,   94, "datatype",   "%s", offset(datatype),  0, CSS_STRING, "-"},
    {96,   99, "dervtype",   "%s", offset(dervtype),  0, CSS_STRING, "-"},
    {101, 108,    "jdate",  "%ld", offset(jdate),     0, CSS_JDATE, "-1"},
    {110, 173,      "dir",   "%s", offset(dir),       0, CSS_STRING, "-"},
    {175, 206,    "dfile",   "%s", offset(dfile),     0, CSS_STRING, "-"},
    {208, 217,     "foff",  "%ld", offset(foff),      0, CSS_LONG,  "-1"},
    {219, 226,   "commid",  "%ld", offset(commid),    0, CSS_LONG,  "-1"},
    {228, 244,   "lddate",   "%s", offset(lddate),    0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark",  "%d", offset(sta_quark),   CSS_QUARK, ""},
    {"chan_quark", "%d", offset(chan_quark),  CSS_QUARK, ""},
    };
    CssTableClass::define(cssDervdisc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssDervdiscClass::createDervdisc, sizeof(CssDervdiscClass));
  }
}

CssDervdiscClass::CssDervdiscClass(void) : CssTableClass(cssDervdisc)
{
    defineDervdisc();
    initTable();
}

CssDervdiscClass::CssDervdiscClass(CssDervdiscClass &a) : CssTableClass(cssDervdisc)
{
    defineDervdisc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssPmccRecipeClass.
 */
static void definePmccRecipe(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssPmccRecipeClass a(0);
    CssClassDescription des[] = {
    {1,     8,    "pmccrecid",  "%ld", offset(pmccrecid),    0, CSS_LONG, "-1"},
    {10,   11,        "deflt",  "%ld", offset(deflt),        0, CSS_LONG,    "-1"},

    {13,   20,       "fGroup",  "%ld", offset(fGroup),       0, CSS_LONG, "-1"},
    {22,   28,       "winlen",  "%lf", offset(winlen),       0, CSS_DOUBLE, "-1"},
    {30,   36,       "wingap",  "%lf", offset(wingap),       0, CSS_DOUBLE, "-1"},
    {38,   44,   "threshCons",  "%lf", offset(threshCons),   0, CSS_DOUBLE, "-1"},
    {46,   49,  "threshNsens",  "%ld", offset(threshNsens),  0, CSS_LONG,"-1"},
    {51,   56,      "qFactor",  "%lf", offset(qFactor),      0, CSS_DOUBLE, "-1"},

    {58,   64,       "pmcc3D",  "%ld", offset(pmcc3D),       0, CSS_LONG,"-1"},
    {66,   74,  "sound_speed",  "%lf", offset(sound_speed),  0, CSS_DOUBLE,"-1"},
    {76,   84, "elevation_angle","%lf", offset(elevation_angle), 0, CSS_DOUBLE,"-1"},

    {86,   93, "threshFamLen",  "%lf", offset(threshFamLen), 0, CSS_DOUBLE,"-1"},
    {95,  100, "threshFamMin",  "%ld", offset(threshFamMin), 0, CSS_LONG,"-1"},
    {102, 109, "threshFamMax",  "%ld", offset(threshFamMax), 0, CSS_LONG,"-1"},

    {111, 118,"speedTransition","%lf", offset(speedTransition),0, CSS_DOUBLE, "-1"},
    {120, 127,      "timeTol",  "%lf", offset(timeTol),   0, CSS_DOUBLE, "-1"},
    {129, 136,      "freqTol",  "%lf", offset(freqTol),   0, CSS_DOUBLE, "-1"},
    {138, 145,    "speedTol1",  "%lf", offset(speedTol1), 0, CSS_DOUBLE, "-1"},
    {147, 154,    "speedTol2",  "%lf", offset(speedTol2), 0, CSS_DOUBLE, "-1"},
    {156, 163,       "azTol1",  "%lf", offset(azTol1),    0, CSS_DOUBLE, "-1"},
    {165, 172,       "azTol2",  "%lf", offset(azTol2),    0, CSS_DOUBLE, "-1"},

    {174, 188,         "auth",   "%s", offset(auth),      0, CSS_STRING, "-"},

    {190, 206,       "lddate",  "%s", offset(lddate),     0, CSS_LDDATE,DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssPmccRecipe, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssPmccRecipeClass::createPmccRecipe, sizeof(CssPmccRecipeClass));
  }
}

CssPmccRecipeClass::CssPmccRecipeClass(void) : CssTableClass(cssPmccRecipe)
{
    definePmccRecipe();
    initTable();
}

CssPmccRecipeClass::CssPmccRecipeClass(CssPmccRecipeClass &a) : CssTableClass(cssPmccRecipe)
{
    definePmccRecipe();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssPmccFeaturesClass.
 */
static void definePmccFeatures(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssPmccFeaturesClass a(0);
    CssClassDescription des[] = {
    {1,     6,         "sta",    "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,         "arid",  "%ld", offset(arid),       0, CSS_LONG,    "-1"},
    {17,   33, "initial_time","%.5lf", offset(initial_time),0,CSS_TIME,T_NULL},
    {35,   42,     "duration",  "%lf", offset(duration),   0, CSS_DOUBLE, "-1."},
    {44,   51,  "consistency",  "%lf", offset(consistency),0, CSS_DOUBLE,"-1."},
    {53,   60,  "correlation",  "%lf", offset(correlation),0, CSS_DOUBLE,"-1."},
    {62,   66,      "famsize",  "%ld", offset(famsize),    0, CSS_LONG,    "-1"},
    {68,   73,      "minfreq",  "%lf", offset(minfreq),    0, CSS_DOUBLE, "-1."},
    {75,   80,      "maxfreq",  "%lf", offset(maxfreq),    0, CSS_DOUBLE, "-1."},
    {82,   87,        "cfreq",  "%lf", offset(cfreq),      0, CSS_DOUBLE, "-1."},
    {89,   94,    "sigmafreq",  "%lf", offset(sigmafreq),  0, CSS_DOUBLE, "-1."},
    {96,  104,       "rmsamp",  "%lf", offset(rmsamp),     0, CSS_DOUBLE, "-1."},
    {106, 120,         "auth",   "%s", offset(auth),       0, CSS_STRING, "-"},
    {122, 129,       "commid",  "%ld", offset(commid),     0, CSS_LONG,    "-1"},
    {131, 147,       "lddate",   "%s", offset(lddate),     0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssPmccFeatures, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssPmccFeaturesClass::createPmccFeatures, sizeof(CssPmccFeaturesClass));
  }
}

CssPmccFeaturesClass::CssPmccFeaturesClass(void) : CssTableClass(cssPmccFeatures)
{
    definePmccFeatures();
    initTable();
}

CssPmccFeaturesClass::CssPmccFeaturesClass(CssPmccFeaturesClass &a) : CssTableClass(cssPmccFeatures)
{
    definePmccFeatures();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssPickClass.
 */
static void definePick(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssPickClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   24,   "chanid",  "%ld", offset(chanid),   0, CSS_LONG,   "-1"},
    {26,   42,     "time","%.5lf", offset(time),     0, CSS_TIME,T_NULL},
    {44,   51,     "arid",  "%ld", offset(arid),     0, CSS_LONG,  "-1"},
    {53,   55,  "amptype",   "%s", offset(amptype),  0, CSS_STRING, "-"},
    {57,   66, 	    "amp",  "%lf", offset(amp),      0, CSS_DOUBLE, "-999."},
    {68,   74,      "per",  "%lf", offset(per),      0, CSS_DOUBLE, "-1."},
    {76,   85,    "calib",  "%lf", offset(calib),    0, CSS_DOUBLE,  "0."},
    {87,   93,   "calper",  "%lf", offset(calper),   0, CSS_DOUBLE, "-1."},
    {95,  104, "ampcalib",  "%lf", offset(ampcalib), 0, CSS_DOUBLE,  "0."},
    {106, 115,   "ampmin",  "%lf", offset(ampmin),   0, CSS_DOUBLE, "-1."},
    {117, 124,   "commid",  "%ld", offset(commid),   0, CSS_LONG,   "-1"},
    {126, 142,   "lddate",   "%s", offset(lddate),   0, CSS_LDDATE,  DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    {"chan_quark", "%d", offset(chan_quark),CSS_QUARK,	""},
    };
    CssTableClass::define(cssPick, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssPickClass::createPick, sizeof(CssPickClass));
  }
}

CssPickClass::CssPickClass(void) : CssTableClass(cssPick)
{
    definePick();
    initTable();
}

CssPickClass::CssPickClass(CssPickClass &a) : CssTableClass(cssPick)
{
    definePick();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssFilterClass.
 */
static void defineFilter(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssFilterClass a(0);
    CssClassDescription des[] = {
    {1,     6,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {8,    15,     "chan",   "%s", offset(chan),OffSet(chan_quark),CSS_STRING,"-"},
    {17,   24,   "chanid",  "%ld", offset(chanid),  0, CSS_LONG,  "-1"},
    {26,   33,     "arid",  "%ld", offset(arid),    0, CSS_LONG,  "-1"},
    {35,   42,     "wfid",  "%ld", offset(wfid),    0, CSS_LONG,  "-1"},
    {44,   45,     "band",   "%s", offset(band),    0, CSS_STRING, "-"},
    {47,   47,    "ftype",   "%s", offset(ftype),   0, CSS_STRING, "-"},
    {49,   52,   "forder",  "%ld", offset(forder),  0, CSS_LONG,   "-1"},
    {54,   62,   "lofreq","%.4lf", offset(lofreq),  0, CSS_DOUBLE, "-1."},
    {64,   72,   "hifreq","%.4lf", offset(hifreq),  0, CSS_DOUBLE, "-1."},
    {74,  103,     "algo",   "%s", offset(algo),    0, CSS_STRING, "-"},
    {105, 119,  "program",   "%s", offset(program), 0, CSS_STRING, "-"},
    {121, 137,   "lddate",   "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    {"chan_quark", "%d", offset(chan_quark),CSS_QUARK,	""},
    };
    CssTableClass::define(cssFilter, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssFilterClass::createFilter, sizeof(CssFilterClass));
  }
}

CssFilterClass::CssFilterClass(void) : CssTableClass(cssFilter)
{
    defineFilter();
    initTable();
}

CssFilterClass::CssFilterClass(CssFilterClass &a) : CssTableClass(cssFilter)
{
    defineFilter();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssAssocClass.
 */
static void defineAssoc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssAssocClass a(0);
    CssClassDescription des[] = {
    {1,     8,     "arid",  "%ld", offset(arid),   0, CSS_LONG, "-1"},
    {10,   17,     "orid",  "%ld", offset(orid),   0, CSS_LONG, "-1"},
    {19,   24,      "sta",   "%s", offset(sta), OffSet(sta_quark),CSS_STRING,"-"},
    {26,   33,    "phase",   "%s", offset(phase),  0, CSS_STRING, "-"},
    {35,   38,   "belief","%.2lf", offset(belief), 0, CSS_DOUBLE, "-1."},
    {40,   47,    "delta","%.3lf", offset(delta),  0, CSS_DOUBLE, "-1."},
    {49,   55,     "seaz","%.2lf", offset(seaz),   0, CSS_DOUBLE, "-999."},
    {57,   63,     "esaz","%.2lf", offset(esaz),   0, CSS_DOUBLE, "-999."},
    {65,   72,  "timeres","%.3lf", offset(timeres),0, CSS_DOUBLE, "-999."},
    {74,   74,  "timedef",   "%s", offset(timedef),0, CSS_STRING, "-"},
    {76,   82,    "azres","%.1lf", offset(azres),  0, CSS_DOUBLE, "-999."},
    {84,   84,    "azdef",   "%s", offset(azdef),  0, CSS_STRING, "-"},
    {86,   92,   "slores","%.2lf", offset(slores), 0, CSS_DOUBLE, "-999."},
    {94,   94,   "slodef",   "%s", offset(slodef), 0, CSS_STRING, "-"},
    {96,  102,   "emares","%.1lf", offset(emares), 0, CSS_DOUBLE, "-999."},
    {104, 109,      "wgt","%.3lf", offset(wgt),    0, CSS_DOUBLE, "-1."},
    {111, 125,   "vmodel",   "%s", offset(vmodel), 0, CSS_STRING, "-"},
    {127, 134,   "commid",  "%ld", offset(commid), 0, CSS_LONG, "-1"},
    {136, 152,   "lddate",   "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssAssoc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssAssocClass::createAssoc, sizeof(CssAssocClass));
  }
}

CssAssocClass::CssAssocClass(void) : CssTableClass(cssAssoc)
{
    defineAssoc();
    initTable();
}

CssAssocClass::CssAssocClass(CssAssocClass &a) : CssTableClass(cssAssoc)
{
    defineAssoc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssStassocClass.
 */
static void defineStassoc(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssStassocClass a(0);
    CssClassDescription des[] = {
    {1,     8,  "stassid",  "%ld", offset(stassid), 0, CSS_LONG,      "-1"},
    {10,   15,      "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {17,   23,    "etype",   "%s", offset(etype),   0, CSS_STRING,     "-"},
    {25,   56, "location",   "%s", offset(location),0, CSS_STRING,     "-"},
    {58,   64,     "dist",  "%lf", offset(dist),    0, CSS_DOUBLE,    "-1."},
    {66,   72,  "azimuth","%.2lf", offset(azimuth), 0, CSS_DOUBLE,    "-1."},
    {74,   82,      "lat",   "%f", offset(lat),     0, CSS_DOUBLE,  "-999."},
    {84,   92,      "lon",   "%f", offset(lon),     0, CSS_DOUBLE,  "-999."},
    {94,  102,    "depth","%.4lf", offset(depth),   0, CSS_DOUBLE,  "-999."},
    {104, 120,     "time","%.5lf", offset(time),    0, CSS_TIME,   T_NULL},
    {122, 128,      "imb",  "%lf", offset(imb),     0, CSS_DOUBLE,  "-999."},
    {130, 136,      "ims",  "%lf", offset(ims),     0, CSS_DOUBLE,  "-999."},
    {138, 144,      "iml",  "%lf", offset(iml),     0, CSS_DOUBLE,  "-999."},
    {146, 160,     "auth",   "%s", offset(auth),    0, CSS_STRING,     "-"},
    {162, 169,   "commid",  "%ld", offset(commid),  0, CSS_LONG,      "-1"},
    {171, 187,   "lddate",   "%s", offset(lddate),  0, CSS_LDDATE,  DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssStassoc, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssStassocClass::createStassoc, sizeof(CssStassocClass));
  }
}

CssStassocClass::CssStassocClass(void) : CssTableClass(cssStassoc)
{
    defineStassoc();
    initTable();
}

CssStassocClass::CssStassocClass(CssStassocClass &a) : CssTableClass(cssStassoc)
{
    defineStassoc();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssAffiliationClass.
 */
static void defineAffiliation(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssAffiliationClass a(0);
    CssClassDescription des[] = {
    {1,     8,      "net", "%s", offset(net),OffSet(net_quark), CSS_STRING,"-"},
    {10,   15,      "sta", "%s", offset(sta),OffSet(sta_quark), CSS_STRING,"-"},
    {17,   33,   "lddate", "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"net_quark", "%d", offset(net_quark),	CSS_QUARK,	""},
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssAffiliation, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssAffiliationClass::createAffiliation, sizeof(CssAffiliationClass));
  }
}

CssAffiliationClass::CssAffiliationClass(void) : CssTableClass(cssAffiliation)
{
    defineAffiliation();
    initTable();
}

CssAffiliationClass::CssAffiliationClass(CssAffiliationClass &a) : CssTableClass(cssAffiliation)
{
    defineAffiliation();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssStanetClass.
 */
static void defineStanet(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssStanetClass a(0);
    CssClassDescription des[] = {
    {1,     8,      "net", "%s", offset(net),OffSet(net_quark), CSS_STRING,"-"},
    {10,   15,      "sta", "%s", offset(sta),OffSet(sta_quark), CSS_STRING,"-"},
    {17,   33,   "lddate", "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"net_quark", "%d", offset(net_quark),	CSS_QUARK,	""},
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssStanet, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssStanetClass::createStanet, sizeof(CssStanetClass));
  }
}

CssStanetClass::CssStanetClass(void) : CssTableClass(cssStanet)
{
    defineStanet();
    initTable();
}

CssStanetClass::CssStanetClass(CssStanetClass &a) : CssTableClass(cssStanet)
{
    defineStanet();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssHydroFeaturesClass.
 */
static void defineHydroFeatures(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssHydroFeaturesClass a(0);
    CssClassDescription des[] = {
    {1,     8,                  "arid",   "%ld", offset(arid),                  0, CSS_LONG, "-1"},
    {10,   26,             "peak_time", "%.5lf", offset(peak_time),             0, CSS_TIME, T_NULL},
    {28,   34,            "peak_level", "%.4lf", offset(peak_level),            0, CSS_DOUBLE,"-1."},
    {36,   42,          "total_energy", "%.4lf", offset(total_energy),          0, CSS_DOUBLE,"-1."},
    {44,   60,     "mean_arrival_time", "%.5lf", offset(mean_arrival_time),     0, CSS_TIME,  T_NULL},
    {62,   68,           "time_spread", "%.4lf", offset(time_spread),           0, CSS_DOUBLE,"-1."},
    {70,   86,            "onset_time", "%.5lf", offset(onset_time),            0, CSS_TIME,  T_NULL},
    {88,  104,      "termination_time", "%.5lf", offset(termination_time),      0, CSS_TIME,  T_NULL},
    {106, 122,            "total_time", "%.5lf", offset(total_time),            0, CSS_DOUBLE,"-1."},
    {124, 131,             "num_cross",   "%ld", offset(num_cross),             0, CSS_LONG,  "-1"},
    {133, 139,             "ave_noise", "%.4lf", offset(ave_noise),             0, CSS_DOUBLE,"-1."},
    {141, 150,              "skewness", "%.4lf", offset(skewness),              0, CSS_TIME,   T_NULL},
    {152, 163,              "kurtosis", "%.4lf", offset(kurtosis),              0, CSS_TIME,   T_NULL},
    {165, 174,        "cep_var_signal", "%.4lf", offset(cep_var_signal),        0, CSS_DOUBLE,"-1."},
    {176, 182, "cep_delay_time_signal", "%.4lf", offset(cep_delay_time_signal), 0, CSS_DOUBLE,"-1."},
    {184, 190,   "cep_peak_std_signal", "%.4lf", offset(cep_peak_std_signal),   0, CSS_DOUBLE,"-1."},
    {192, 201,         "cep_var_trend", "%.4lf", offset(cep_var_trend),         0, CSS_DOUBLE,"-1."},
    {203, 209,  "cep_delay_time_trend", "%.4lf", offset(cep_delay_time_trend),  0, CSS_DOUBLE,"-1"},
    {211, 217,    "cep_peak_std_trend", "%.4lf", offset(cep_peak_std_trend),    0, CSS_DOUBLE,"-1."},
    {219, 225,               "low_cut", "%.4lf", offset(low_cut),               0, CSS_DOUBLE,"-1."},
    {227, 233,              "high_cut", "%.4lf", offset(high_cut),              0, CSS_DOUBLE,"-1."},
    {235, 238,                  "ford",   "%ld", offset(ford),                  0, CSS_LONG,  "0"},
    {240, 241,                 "ftype",    "%s", offset(ftype),                 0, CSS_STRING,"-"},
    {243, 244,                   "fzp",   "%ld", offset(fzp),                   0, CSS_LONG,  "0"},
    {246, 262,      "prob_weight_time", "%.5lf", offset(prob_weight_time),      0, CSS_DOUBLE,"-999."},
    {264, 280,            "sigma_time", "%.5lf", offset(sigma_time),            0, CSS_DOUBLE,"-999."},
    {282, 298,                "lddate",    "%s", offset(lddate),                0, CSS_LDDATE,DTNULL},
    };
    CssTableClass::define(cssHydroFeatures, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssHydroFeaturesClass::createHydroFeatures,
		sizeof(CssHydroFeaturesClass));
  }
}

CssHydroFeaturesClass::CssHydroFeaturesClass(void) : CssTableClass(cssHydroFeatures)
{
    defineHydroFeatures();
    initTable();
}

CssHydroFeaturesClass::CssHydroFeaturesClass(CssHydroFeaturesClass &a) : CssTableClass(cssHydroFeatures)
{
    defineHydroFeatures();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssInfraFeaturesClass.
 */
static void defineInfraFeatures(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssInfraFeaturesClass a(0);
    CssClassDescription des[] = {
    {1,     8, "arid", 	        "%ld", offset(arid),        0, CSS_LONG,  "-1"},
    {10,   26, "eng_time",    "%.5lf", offset(eng_time),    0, CSS_TIME,  T_NULL},
    {28,   34, "eng_dur",     "%.2lf", offset(eng_dur),     0, CSS_DOUBLE,"-999."},
    {36,   42, "eng_deldur",  "%.2lf", offset(eng_deldur),  0, CSS_DOUBLE,"-1."},
    {44,   60, "coh_time",    "%.5lf", offset(coh_time),    0, CSS_TIME,  T_NULL},
    {62,   68, "coh_dur",     "%.2lf", offset(coh_dur),     0, CSS_DOUBLE,"-999."},
    {70,   86, "coh_deldur",  "%.2lf", offset(coh_deldur),  0, CSS_DOUBLE,"-1."},
    {88,  104, "coinc_time",    "%lf", offset(coinc_time),  0, CSS_DOUBLE,"-1."},
    {106, 122, "coinc_dur",   "%.2lf", offset(coinc_dur),   0, CSS_DOUBLE,"-999."},
    {124, 131, "coinc_deldur","%.2lf", offset(coinc_deldur),0, CSS_DOUBLE,"-1."},
    {133, 139, "ford",          "%ld", offset(ford),        0, CSS_LONG,  "0"},
    {141, 150, "zrcr_freq",   "%.2lf", offset(zrcr_freq),   0, CSS_DOUBLE,"-1."},
    {152, 163, "zrcr_delfreq","%.2lf", offset(zrcr_delfreq),0, CSS_DOUBLE,"-1."},
    {165, 174, "crnr_freq",   "%.2lf", offset(crnr_freq),   0, CSS_DOUBLE,"-1."},
    {176, 182, "crnr_delfreq","%.2lf", offset(crnr_delfreq),0, CSS_DOUBLE,"-1."},
    {184, 190, "coh_per",     "%.2lf", offset(coh_per),     0, CSS_DOUBLE,"-999."},
    {192, 201, "coh_snr",     "%.2lf", offset(coh_snr),     0, CSS_DOUBLE,"-1."},
    {203, 209, "total_energy","%.4lf", offset(total_energy),0, CSS_DOUBLE,"-1."},
    {211, 217, "auth",           "%s", offset(auth),        0, CSS_STRING,"-"},
    {219, 226, "commid",        "%ld", offset(commid),      0, CSS_LONG,  "-1"},
    {228, 244, "lddate",         "%s", offset(lddate),      0, CSS_LDDATE,DTNULL},
    };
    CssTableClass::define(cssInfraFeatures, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssInfraFeaturesClass::createInfraFeatures,
		sizeof(CssInfraFeaturesClass));
  }
}

CssInfraFeaturesClass::CssInfraFeaturesClass(void) : CssTableClass(cssInfraFeatures)
{
    defineInfraFeatures();
    initTable();
}

CssInfraFeaturesClass::CssInfraFeaturesClass(CssInfraFeaturesClass &a) :
			CssTableClass(cssInfraFeatures)
{
    defineInfraFeatures();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssStamagClass.
 */
static void defineStamag(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssStamagClass a(0);
    CssClassDescription des[] = {
    {1,     8,       "magid",  "%ld", offset(magid),      0, CSS_LONG, "-1"},
    {10,   17,       "ampid",  "%ld", offset(ampid),      0, CSS_LONG, "-1"},
    {19,   24,         "sta",   "%s", offset(sta),OffSet(sta_quark),CSS_STRING,"-"},
    {26,   33,        "arid",  "%ld", offset(arid),       0, CSS_LONG, "-1"},
    {35,   42,        "orid",  "%ld", offset(orid),       0, CSS_LONG, "-1"},
    {44,   51,        "evid",  "%ld", offset(evid),       0, CSS_LONG, "-1"},
    {53,   60,       "phase",   "%s", offset(phase),      0, CSS_STRING, "-"},
    {62,   69,       "delta", "%.3lf",offset(delta),      0, CSS_DOUBLE, "-1."},
    {71,   76,     "magtype",   "%s", offset(magtype),    0, CSS_STRING, "-"},
    {78,   84,   "magnitude", "%.2lf",offset(magnitude),  0,CSS_DOUBLE,"-999."},
    {86,   92, "uncertainty", "%.2lf",offset(uncertainty),0, CSS_DOUBLE, "-1."},
    {94,  100,      "magres", "%.2lf",offset(magres),     0,CSS_DOUBLE,"-999."},
    {102, 102,      "magdef",   "%s", offset(magdef),     0, CSS_STRING, "-"},
    {104, 118,      "mmodel",   "%s", offset(mmodel),     0, CSS_STRING, "-"},
    {120, 134,        "auth",   "%s", offset(auth),       0, CSS_STRING, "-"},
    {136, 143,      "commid",  "%ld", offset(commid),     0, CSS_LONG, "-1"},
    {145, 161,      "lddate",   "%s", offset(lddate),     0, CSS_LDDATE,DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark", "%d", offset(sta_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssStamag, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssStamagClass::createStamag, sizeof(CssStamagClass));
  }
}

CssStamagClass::CssStamagClass(void) : CssTableClass(cssStamag)
{
    defineStamag();
    initTable();
}

CssStamagClass::CssStamagClass(CssStamagClass &a) : CssTableClass(cssStamag)
{
    defineStamag();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssNetmagClass.
 */
static void defineNetmag(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssNetmagClass a(0);
    CssClassDescription des[] = {
    {1,     8,        "magid","%ld", offset(magid),       0, CSS_LONG, "-1"},
    {10,   17,         "net",  "%s", offset(net),OffSet(net_quark),CSS_STRING,"-"},
    {19,   26,        "orid", "%ld", offset(orid),        0, CSS_LONG, "-1"},
    {28,   35,        "evid", "%ld", offset(evid),        0, CSS_LONG, "-1"},
    {37,   42,     "magtype",  "%s", offset(magtype),     0, CSS_STRING, "-"},
    {44,   51,        "nsta", "%ld", offset(nsta),        0, CSS_LONG, "-1"},
    {53,   59,   "magnitude","%.2lf",offset(magnitude),   0, CSS_DOUBLE, "-999."},
    {61,   67, "uncertainty","%.2lf",offset(uncertainty), 0, CSS_DOUBLE, "-1."},
    {69,   83,        "auth",  "%s", offset(auth),        0, CSS_STRING, "-"},
    {85,   92,      "commid", "%ld", offset(commid),      0, CSS_LONG, "-1"},
    {94,  110,      "lddate",  "%s", offset(lddate),      0, CSS_LDDATE,DTNULL},
    };
    CssClassExtra extra[] = {
    {"net_quark", "%d", offset(net_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssNetmag, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssNetmagClass::createNetmag, sizeof(CssNetmagClass));
  }
}

CssNetmagClass::CssNetmagClass(void) : CssTableClass(cssNetmag)
{
    defineNetmag();
    initTable();
}

CssNetmagClass::CssNetmagClass(CssNetmagClass &a) : CssTableClass(cssNetmag)
{
    defineNetmag();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssAmpdescriptClass.
 */
static void defineAmpdescript(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssAmpdescriptClass a(0);
    CssClassDescription des[] = {
    {  1,   8, "amptype",    "%s", offset(amptype), 0, CSS_STRING, "-"},
    { 10,  15,    "toff", "%.2lf", offset(toff),    0, CSS_DOUBLE, "-999."},
    { 17,  26,    "tlen", "%.3lf", offset(tlen),    0, CSS_DOUBLE, "-1."},
    { 28,  32,    "gvlo", "%.2lf", offset(gvlo),    0, CSS_DOUBLE, "-999."},
    { 34,  38,    "gvhi", "%.2lf", offset(gvhi),    0, CSS_DOUBLE, "-999."},
    { 40,  47,   "mtype",    "%s", offset(mtype),   0, CSS_STRING, "-"},
    { 49, 303,   "descr",    "%s", offset(descr),   0, CSS_STRING, "-"},
    {305, 321,  "lddate",    "%s", offset(lddate),  0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssAmpdescript, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssAmpdescriptClass::createAmpdescript,
		sizeof(CssAmpdescriptClass));
  }
}

CssAmpdescriptClass::CssAmpdescriptClass(void) : CssTableClass(cssAmpdescript)
{
    defineAmpdescript();
    initTable();
}

CssAmpdescriptClass::CssAmpdescriptClass(CssAmpdescriptClass &a) : CssTableClass(cssAmpdescript)
{
    defineAmpdescript();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssAmplitudeClass.
 */
static void defineAmplitude(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssAmplitudeClass a(0);
    CssClassDescription des[] = {
    {1,     8,       "ampid",   "%ld", offset(ampid),      0, CSS_LONG, "-1"},
    {10,   17,        "arid",   "%ld", offset(arid),       0, CSS_LONG, "-1"},
    {19,   26,       "parid",   "%ld", offset(parid),      0, CSS_LONG, "-1"},
    {28,   35,        "chan",    "%s", offset(chan),OffSet(chan_quark),CSS_STRING, "-"},
    {37,   47,         "amp", "%.2lf", offset(amp),        0, CSS_DOUBLE, "-1."},
    {49,   55,         "per", "%.2lf", offset(per),        0, CSS_DOUBLE, "-999."},
    {57,   66,         "snr", "%.2lf", offset(snr),        0, CSS_DOUBLE, "-1."},
    {68,   84,     "amptime", "%.5lf", offset(amptime),    0, CSS_TIME, T_NULL},
    {86,   102, "start_time", "%.5lf", offset(start_time), 0, CSS_TIME, T_NULL},
    {104,  110,   "duration", "%.2lf", offset(duration),   0, CSS_DOUBLE, "-999"},
    {112,  118,      "bandw", "%.3lf", offset(bandw),      0, CSS_DOUBLE, "-1."},
    {120,  127,    "amptype",    "%s", offset(amptype),    0, CSS_STRING, "-"},
    {129,  143,      "units",    "%s", offset(units),      0, CSS_STRING, "-"},
    {145,  145,       "clip",    "%s", offset(clip),       0, CSS_STRING, "-"},
    {147,  147,  "inarrival",    "%s", offset(inarrival),  0, CSS_STRING, "y"},
    {149,  163,       "auth",    "%s", offset(auth),       0, CSS_STRING, "-"},
    {165,  181,     "lddate",    "%s", offset(lddate),     0,CSS_LDDATE,DTNULL},
    };
    CssClassExtra extra[] = {
    {"chan_quark",    "%d", offset(chan_quark),	CSS_QUARK,	""},
    {"amp_cnts",   "%.2lf", offset(amp_cnts),	CSS_DOUBLE,	"-1."},
    {"amp_Nnms",   "%.2lf", offset(amp_Nnms),	CSS_DOUBLE,	"-1."},
    {"amp_nms",    "%.2lf", offset(amp_nms),	CSS_DOUBLE,	"-1."},
    {"zp_Nnms",    "%.2lf", offset(zp_Nnms),	CSS_DOUBLE,	"-1."},
    {"box_location",  "%d", offset(box_location),CSS_BOOL,	"false"},
    {"boxtime",    "%.5lf", offset(boxtime),	CSS_TIME,	T_NULL},
    {"boxmin",     "%.2lf", offset(boxmin),	CSS_DOUBLE,	"0"},
    };
    CssTableClass::define(cssAmplitude, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssAmplitudeClass::createAmplitude, sizeof(CssAmplitudeClass));
  }
}

CssAmplitudeClass::CssAmplitudeClass(void) : CssTableClass(cssAmplitude)
{
    defineAmplitude();
    initTable();
}

CssAmplitudeClass::CssAmplitudeClass(CssAmplitudeClass &a) : CssTableClass(cssAmplitude)
{
    defineAmplitude();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssStaconfClass.
 */
static void defineStaconf(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssStaconfClass a(0);
    CssClassDescription des[] = {
    {1,    6,    "staname", "%s", offset(staname), OffSet(staname_quark), CSS_STRING, "-"},
    {8,   15,    "statype", "%s", offset(statype), OffSet(statype_quark), CSS_STRING, "-"},
    {17,  22,    "refsite", "%s", offset(refsite), OffSet(refsite_quark), CSS_STRING, "-"},
    {24,  31,    "refchan", "%s", offset(refchan), OffSet(refchan_quark), CSS_STRING, "-"},
    {33,  38, "threecsite", "%s", offset(threecsite), OffSet(threecsite_quark), CSS_STRING, "-"},
    {40,  47, "threecband", "%s", offset(threecband), OffSet(threecband_quark), CSS_STRING, "-"},
    {49,  54,     "lpsite", "%s", offset(lpsite), OffSet(lpsite_quark), CSS_STRING, "-"},
    {56,  73,     "lpband", "%s", offset(lpband), OffSet(lpband_quark), CSS_STRING, "-"},
    {75,  91,     "lddate", "%s", offset(lddate), 0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"staname_quark",    "%d", offset(staname_quark),   CSS_QUARK,	""},
    {"statype_quark",    "%d", offset(statype_quark),   CSS_QUARK,	""},
    {"refsite_quark",    "%d", offset(refsite_quark),   CSS_QUARK,	""},
    {"refchan_quark",    "%d", offset(refchan_quark),	CSS_QUARK,	""},
    {"threecsite_quark", "%d", offset(threecsite_quark),CSS_QUARK,	""},
    {"threecband_quark", "%d", offset(threecband_quark),CSS_QUARK,	""},
    {"lpsite_quark",     "%d", offset(lpsite_quark),	CSS_QUARK,	""},
    {"lpband_quark",     "%d", offset(lpband_quark),	CSS_QUARK,	""},
    };
    CssTableClass::define(cssStaconf, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssStaconfClass::createStaconf, sizeof(CssStaconfClass));
  }
}

CssStaconfClass::CssStaconfClass(void) : CssTableClass(cssStaconf)
{
    defineStaconf();
    initTable();
}

CssStaconfClass::CssStaconfClass(CssStaconfClass &a) : CssTableClass(cssStaconf)
{
    defineStaconf();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/** Define CssParrivalClass.
 */
static void defineParrival(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssParrivalClass a(0);
    CssClassDescription des[] = {
    {1,    8,       "parid",   "%ld", offset(parid),      0, CSS_LONG, "-1"},
    {10,  17,        "orid",   "%ld", offset(orid),       0, CSS_LONG, "-1"},
    {19,  26,        "evid",   "%ld", offset(evid),       0, CSS_LONG, "-1"},
    {28,  33,         "sta",    "%s", offset(sta), OffSet(sta_quark), CSS_STRING, "-"},
    {35,  51,        "time", "%.5lf", offset(time),       0, CSS_TIME, T_NULL},
    {53,  59,     "azimuth", "%.2lf", offset(azimuth),    0, CSS_DOUBLE,    "-1."},
    {61,  67,        "slow", "%.2lf", offset(slow),       0, CSS_DOUBLE,    "-1."},
    {69,  76,       "phase",    "%s", offset(phase),      0, CSS_STRING,      "-"},
    {78,  85,       "delta", "%.2lf", offset(delta),      0, CSS_DOUBLE,    "-1."},
    {87, 101,      "vmodel",    "%s", offset(vmodel),     0, CSS_STRING, "-"},
    {103,119 ,     "lddate",    "%s", offset(lddate),     0, CSS_LDDATE, DTNULL},
    };
    CssClassExtra extra[] = {
    {"sta_quark",    "%d", offset(sta_quark),   CSS_QUARK,	""},
    };
    CssTableClass::define(cssParrival, sizeof(des)/sizeof(CssClassDescription),
		des, sizeof(extra)/sizeof(CssClassExtra), extra,
		CssParrivalClass::createParrival, sizeof(CssParrivalClass));
  }
}

CssParrivalClass::CssParrivalClass(void) : CssTableClass(cssParrival)
{
    defineParrival();
    initTable();
}

CssParrivalClass::CssParrivalClass(CssParrivalClass &a) : CssTableClass(cssParrival)
{
    defineParrival();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
static void defineDynamic5(void) {
    static bool defined = false;
    if( !defined ) {
	defined = true;
	CssTableClass::define("dynamic5", 0, NULL, 0, NULL,
		CssDynamic5Class::createDynamic5, sizeof(CssDynamic5Class));
    }
}
CssDynamic5Class::CssDynamic5Class(void) : CssTableClass("dynamic5") {
    defineDynamic5();
    initDescription();
    for(int i = 0; i < 5; i++) data[i].l_data = 0;
}
CssDynamic5Class::CssDynamic5Class(CssDynamic5Class &a) : CssTableClass("dynamic5") {
    defineDynamic5();
    initDescription();
    memcpy((void *)data, (void *)a.data, sizeof(data));
}

static void defineDynamic10(void) {
    static bool defined = false;
    if( !defined ) {
	defined = true;
	CssTableClass::define("dynamic10", 0, NULL, 0 , NULL,
		CssDynamic10Class::createDynamic10, sizeof(CssDynamic10Class));
    }
}
CssDynamic10Class::CssDynamic10Class(void) : CssTableClass("dynamic10") {
    defineDynamic10();
    initDescription();
    for(int i = 0; i < 10; i++) data[i].l_data = 0;
}
CssDynamic10Class::CssDynamic10Class(CssDynamic10Class &a) : CssTableClass("dynamic10") {
    defineDynamic10();
    initDescription();
    memcpy((void *)data, (void *)a.data, sizeof(data));
}

static void defineDynamic20(void) {
    static bool defined = false;
    if( !defined ) {
	defined = true;
	CssTableClass::define("dynamic20", 0, NULL, 0,NULL,
		CssDynamic20Class::createDynamic20, sizeof(CssDynamic20Class));
    }
}
CssDynamic20Class::CssDynamic20Class(void) : CssTableClass("dynamic20") {
    defineDynamic20();
    initDescription();
    for(int i = 0; i < 20; i++) data[i].l_data = 0;
}
CssDynamic20Class::CssDynamic20Class(CssDynamic20Class &a) : CssTableClass("dynamic20") {
    defineDynamic20();
    initDescription();
    memcpy((void *)data, (void *)a.data, sizeof(data));
}

static void defineDynamic40(void) {
    static bool defined = false;
    if( !defined ) {
	defined = true;
	CssTableClass::define("dynamic40", 0, NULL, 0, NULL,
		CssDynamic40Class::createDynamic40, sizeof(CssDynamic40Class));
    }
}
CssDynamic40Class::CssDynamic40Class(void) : CssTableClass("dynamic40") {
    defineDynamic40();
    initDescription();
    for(int i = 0; i < 40; i++) data[i].l_data = 0;
}
CssDynamic40Class::CssDynamic40Class(CssDynamic40Class &a) : CssTableClass("dynamic40") {
    defineDynamic40();
    initDescription();
    memcpy((void *)data, (void *)a.data, sizeof(data));
}

static void defineDynamic60(void) {
    static bool defined = false;
    if( !defined ) {
	defined = true;
	CssTableClass::define("dynamic60", 0, NULL, 0, NULL,
		CssDynamic60Class::createDynamic60, sizeof(CssDynamic60Class));
    }
}
CssDynamic60Class::CssDynamic60Class(void) : CssTableClass("dynamic60") {
    defineDynamic60();
    initDescription();
    for(int i = 0; i < 60; i++) data[i].l_data = 0;
}
CssDynamic60Class::CssDynamic60Class(CssDynamic60Class &a) : CssTableClass("dynamic60") {
    defineDynamic60();
    initDescription();
    memcpy((void *)data, (void *)a.data, sizeof(data));
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgEnergyCal.
 */
static void defineGardsBgEnergyCal(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgEnergyCalClass a(0);
    CssClassDescription des[] = {
    {1,     4,    "sample_id", "%ld", offset(sample_id),    0, CSS_LONG,  "-1"},
    {6,    29,  "beta_coeff1", "%lf", offset(beta_coeff1),  0, CSS_DOUBLE, "0"},
    {31,   54,  "beta_coeff2", "%lf", offset(beta_coeff2),  0, CSS_DOUBLE, "0"},
    {56,   79,  "beta_coeff3", "%lf", offset(beta_coeff3),  0, CSS_DOUBLE, "0"},
    {81,  104, "gamma_coeff1", "%lf", offset(gamma_coeff1), 0, CSS_DOUBLE, "0"},
    {106, 129, "gamma_coeff2", "%lf", offset(gamma_coeff2), 0, CSS_DOUBLE, "0"},
    {131, 154, "gamma_coeff3", "%lf", offset(gamma_coeff3), 0, CSS_DOUBLE, "0"},
    {156, 172,      "moddate",  "%s", offset(moddate),      0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgEnergyCal, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgEnergyCalClass::createGardsBgEnergyCal,
		sizeof(CssGardsBgEnergyCalClass));
  }
}

CssGardsBgEnergyCalClass::CssGardsBgEnergyCalClass(void) : CssTableClass(cssGardsBgEnergyCal)
{
    defineGardsBgEnergyCal();
    initTable();
}

CssGardsBgEnergyCalClass::CssGardsBgEnergyCalClass(CssGardsBgEnergyCalClass &a) :
		CssTableClass(cssGardsBgEnergyCal)
{
    defineGardsBgEnergyCal();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgProcParamsClass.
 */
static void defineGardsBgProcParams(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgProcParamsClass a(0);
    CssClassDescription des[] = {
    {1,     4,      "sample_id", "%ld",offset(sample_id),      0, CSS_LONG, "-1"},
    {6,    29,    "lc_abscissa", "%lf",offset(lc_abscissa),    0,CSS_DOUBLE,"0"},
    {31,   34,         "method", "%ld",offset(method),         0, CSS_LONG, "0"},
    {36,   39, "det_bkgnd_used", "%ld",offset(det_bkgnd_used), 0, CSS_LONG, "0"},
    {41,   44, "gas_bkgnd_used", "%ld",offset(gas_bkgnd_used), 0, CSS_LONG, "0"},
    {46,   49, "beta_ecr_order", "%ld",offset(beta_ecr_order), 0, CSS_LONG, "0"},
    {51,   54,"gamma_ecr_order", "%ld",offset(gamma_ecr_order),0, CSS_LONG, "0"},
    {56,   59,     "max_qc_dev", "%ld",offset(max_qc_dev),     0, CSS_LONG, "0"},
    {61,   64,          "qc_id", "%ld",offset(qc_id),          0, CSS_LONG, "0"},
    {66,   89,      "xe_in_air", "%lf",offset(xe_in_air),      0,CSS_DOUBLE,"0"},
    {91,   94,   "det_bkgnd_id", "%ld",offset(det_bkgnd_id),   0, CSS_LONG, "0"},
    {96,   99,   "gas_bkgnd_id", "%ld",offset(gas_bkgnd_id),   0, CSS_LONG, "0"},
    {101, 104, "qc_b_threshold", "%ld",offset(qc_b_threshold), 0, CSS_LONG, "0"},
    {106, 109,       "bin_rows", "%ld",offset(bin_rows),       0, CSS_LONG, "0"},
    {111, 114,  "bin_min_count", "%ld",offset(bin_min_count),  0, CSS_LONG, "0"},
    {116, 119,"bin_gamma_start", "%ld",offset(bin_gamma_start),0, CSS_LONG, "0"},
    {121, 124, "bin_beta_start", "%ld",offset(bin_beta_start), 0, CSS_LONG, "0"},
    {126, 129, "bin_max_vector_size", "%ld", offset(bin_max_vector_size), 0, CSS_LONG, "0"},
    {131, 147,      "moddate", "%s",offset(moddate), 0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgProcParams, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgProcParamsClass::createGardsBgProcParams,
		sizeof(CssGardsBgProcParamsClass));
  }
}

CssGardsBgProcParamsClass::CssGardsBgProcParamsClass(void) : CssTableClass(cssGardsBgProcParams)
{
    defineGardsBgProcParams();
    initTable();
}

CssGardsBgProcParamsClass::CssGardsBgProcParamsClass(CssGardsBgProcParamsClass &a) :
		CssTableClass(cssGardsBgProcParams)
{
    defineGardsBgProcParams();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgProcParamsRoi.
 */
static void defineGardsBgProcParamsRoi(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgProcParamsRoiClass a(0);
    CssClassDescription des[] = {
    {1,   4,    "sample_id", "%ld", offset(sample_id),    0, CSS_LONG,    "0"},
    {6,   9,          "roi", "%ld", offset(roi),          0, CSS_LONG,    "0"},
    {11, 14, "halflife_sec", "%ld", offset(halflife_sec), 0, CSS_LONG,    "0"},
    {16, 39,    "abundance", "%lf", offset(abundance),    0, CSS_DOUBLE, "0"},
    {41, 44,   "nuclide_id", "%ld", offset(nuclide_id),   0, CSS_LONG,    "0"},
    {46, 62,      "moddate",  "%s", offset(moddate),      0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgProcParamsRoi, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgProcParamsRoiClass::createGardsBgProcParamsRoi,
		sizeof(CssGardsBgProcParamsRoiClass));
  }
}

CssGardsBgProcParamsRoiClass::CssGardsBgProcParamsRoiClass(void) :
		CssTableClass(cssGardsBgProcParamsRoi)
{
    defineGardsBgProcParamsRoi();
    initTable();
}

CssGardsBgProcParamsRoiClass::CssGardsBgProcParamsRoiClass(CssGardsBgProcParamsRoiClass &a) :
			CssTableClass(cssGardsBgProcParamsRoi)
{
    defineGardsBgProcParamsRoi();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsRoiChannelsClass.
 */
static void defineGardsRoiChannels(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsRoiChannelsClass a(0);
    CssClassDescription des[] = {
    {1,   4,    "sample_id", "%ld", offset(sample_id),    0, CSS_LONG,  "0"},
    {6,   9,          "roi", "%ld", offset(roi),          0, CSS_LONG,  "0"},
    {11, 14, "b_chan_start", "%ld", offset(b_chan_start), 0, CSS_LONG,  "0"},
    {16, 19,  "b_chan_stop", "%ld", offset(b_chan_stop),  0, CSS_LONG,  "0"},
    {21, 24, "g_chan_start", "%ld", offset(g_chan_start), 0, CSS_LONG,  "0"},
    {26, 29,  "g_chan_stop", "%ld", offset(g_chan_stop),  0, CSS_LONG,  "0"},
    {31, 47,      "moddate",  "%s", offset(moddate),      0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsRoiChannels, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsRoiChannelsClass::createGardsRoiChannels,
		sizeof(CssGardsRoiChannelsClass));
  }
}

CssGardsRoiChannelsClass::CssGardsRoiChannelsClass(void) : CssTableClass(cssGardsRoiChannels)
{
    defineGardsRoiChannels();
    initTable();
}

CssGardsRoiChannelsClass::CssGardsRoiChannelsClass(CssGardsRoiChannelsClass &a) :
			CssTableClass(cssGardsRoiChannels)
{
    defineGardsRoiChannels();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgRoiConcsClass.
 */
static void defineGardsBgRoiConcs(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgRoiConcsClass a(0);
    CssClassDescription des[] = {
    {1,     4, "sample_id",  "%ld", offset(sample_id), 0, CSS_LONG,   "0"},
    {6,     9,       "roi",  "%ld", offset(roi),       0, CSS_LONG,   "0"},
    {11,   34,      "conc",  "%lf", offset(conc),      0, CSS_DOUBLE, "0"},
    {36,   59,  "conc_err",  "%lf", offset(conc_err),  0, CSS_DOUBLE, "0"},
    {61,   84,       "mdc",  "%lf", offset(mdc),       0, CSS_DOUBLE, "0"},
    {86,   89,  "nid_flag",  "%ld", offset(nid_flag),  0, CSS_LONG,   "0"},
    {91,  114,        "lc",  "%lf", offset(lc),        0, CSS_DOUBLE, "0"},
    {116, 139,        "ld",  "%lf", offset(ld),        0, CSS_DOUBLE, "0"},
    {141, 157,   "moddate",   "%s", offset(moddate),   0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgRoiConcs, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgRoiConcsClass::createGardsBgRoiConcs,
		sizeof(CssGardsBgRoiConcsClass));
  }
}

CssGardsBgRoiConcsClass::CssGardsBgRoiConcsClass(void) : CssTableClass(cssGardsBgRoiConcs)
{
    defineGardsBgRoiConcs();
    initTable();
}

CssGardsBgRoiConcsClass::CssGardsBgRoiConcsClass(CssGardsBgRoiConcsClass &a) :
			CssTableClass(cssGardsBgRoiConcs)
{
    defineGardsBgRoiConcs();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgRoiCountsClass.
 */
static void defineGardsBgRoiCounts(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgRoiCountsClass a(0);
    CssClassDescription des[] = {
    {1,     4,           "sample_id", "%ld", offset(sample_id),           0, CSS_LONG,   "0"},
    {6,     9,                 "roi", "%ld", offset(roi),                 0, CSS_LONG,   "0"},
    {11,   34,               "gross", "%lf", offset(gross),               0, CSS_DOUBLE, "0"},
    {36,   59,           "gross_err", "%lf", offset(gross_err),           0, CSS_DOUBLE, "0"},
    {61,   84,     "gas_bkgnd_gross", "%lf", offset(gas_bkgnd_gross),     0, CSS_DOUBLE, "0"},
    {86,  109,     "gas_bkgnd_count", "%lf", offset(gas_bkgnd_count),     0, CSS_DOUBLE, "0"},
    {111, 134, "gas_bkgnd_count_err", "%lf", offset(gas_bkgnd_count_err), 0, CSS_DOUBLE, "0"},
    {136, 159,     "det_bkgnd_count", "%lf", offset(det_bkgnd_count),     0, CSS_DOUBLE, "0"},
    {161, 184, "det_bkgnd_count_err", "%lf", offset(det_bkgnd_count_err), 0, CSS_DOUBLE, "0"},
    {186, 209,           "net_count", "%lf", offset(net_count),           0, CSS_DOUBLE, "0"},
    {211, 234,       "net_count_err", "%lf", offset(net_count_err),       0, CSS_DOUBLE, "0"},
    {236, 259,   "critical_lev_samp", "%lf", offset(critical_lev_samp),   0, CSS_DOUBLE, "0"},
    {261, 284,    "critical_lev_gas", "%lf", offset(critical_lev_gas),    0, CSS_DOUBLE, "0"},
    {286, 302,             "moddate", "%s",  offset(moddate),             0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgRoiCounts, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgRoiCountsClass::createGardsBgRoiCounts,
		sizeof(CssGardsBgRoiCountsClass));
  }
}

CssGardsBgRoiCountsClass::CssGardsBgRoiCountsClass(void) : CssTableClass(cssGardsBgRoiCounts)
{
    defineGardsBgRoiCounts();
    initTable();
}

CssGardsBgRoiCountsClass::CssGardsBgRoiCountsClass(CssGardsBgRoiCountsClass &a) :
			CssTableClass(cssGardsBgRoiCounts)
{
    defineGardsBgRoiCounts();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgIsotopeConcsClass.
 */
static void defineGardsBgIsotopeConcs(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgIsotopeConcsClass a(0);
    CssClassDescription des[] = {
    {1,     4,  "sample_id", "%ld", offset(sample_id),  0, CSS_LONG,   "0"},
    {6,     9, "nuclide_id", "%ld", offset(nuclide_id), 0, CSS_LONG,   "0"},
    {11,   34,       "conc", "%lf", offset(conc),       0, CSS_DOUBLE, "0"},
    {36,   59,   "conc_err", "%lf", offset(conc_err),   0, CSS_DOUBLE, "0"},
    {61,   84,        "mdc", "%lf", offset(mdc),        0, CSS_DOUBLE, "0"},
    {86,   89,   "nid_flag", "%ld", offset(nid_flag),   0, CSS_LONG,   "0"},
    {91,  114,         "lc", "%lf", offset(lc),         0, CSS_DOUBLE, "0"},
    {116, 139,         "ld", "%lf", offset(ld),         0, CSS_DOUBLE, "0"},
    {141, 157,    "moddate",  "%s", offset(moddate),    0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgIsotopeConcs, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgIsotopeConcsClass::createGardsBgIsotopeConcs,
		sizeof(CssGardsBgIsotopeConcsClass));
  }
}

CssGardsBgIsotopeConcsClass::CssGardsBgIsotopeConcsClass(void) :
			CssTableClass(cssGardsBgIsotopeConcs)
{
    defineGardsBgIsotopeConcs();
    initTable();
}

CssGardsBgIsotopeConcsClass::CssGardsBgIsotopeConcsClass(CssGardsBgIsotopeConcsClass &a) :
			CssTableClass(cssGardsBgIsotopeConcs)
{
    defineGardsBgIsotopeConcs();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsSampleStatusClass.
 */
static void defineGardsSampleStatus(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsSampleStatusClass a(0);
    CssClassDescription des[] = {
    {1,   4,       "sample_id", "%ld", offset(sample_id),      0, CSS_LONG, "0"},
    {6,  22,      "entry_date", " %s", offset(entry_date),     0, CSS_LDDATE, DTNULL},
    {24, 40,  "cnf_begin_date",  "%s", offset(cnf_begin_date), 0, CSS_LDDATE, DTNULL},
    {42, 58,    "cnf_end_date",  "%s", offset(cnf_end_date),   0, CSS_LDDATE, DTNULL},
    {60, 76,     "review_date",  "%s", offset(review_date),    0, CSS_LDDATE, DTNULL},
    {78, 81,     "review_time", "%ld", offset(review_time),    0, CSS_LONG, "0"},
    {83, 112,        "analyst",  "%s", offset(analyst),        0, CSS_STRING,"-"},
    {114, 114,        "status",  "%s", offset(status),         0, CSS_STRING,"-"},
    {116, 119,      "category", "%ld", offset(category),       0, CSS_LONG, "0"},
    {121, 124, "auto_category", "%ld", offset(auto_category),  0, CSS_LONG, "0"},
    {126, 142,  "release_date",  "%s", offset(release_date),   0, CSS_LDDATE, DTNULL},
    {144, 160,       "moddate",  "%s", offset(moddate),        0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsSampleStatus, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsSampleStatusClass::createGardsSampleStatus,
		sizeof(CssGardsSampleStatusClass));
  }
}

CssGardsSampleStatusClass::CssGardsSampleStatusClass(void) : CssTableClass(cssGardsSampleStatus)
{
    defineGardsSampleStatus();
    initTable();
}

CssGardsSampleStatusClass::CssGardsSampleStatusClass(CssGardsSampleStatusClass &a) :
			CssTableClass(cssGardsSampleStatus)
{
    defineGardsSampleStatus();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsSampleData.
 */
static void defineGardsSampleData(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsSampleDataClass a(0);
    CssClassDescription des[] = {
    {1,  12,          "site_det_code",  "%s", offset(site_det_code),        0, CSS_STRING, "-"},
    {14, 18,              "sample_id",  "%d", offset(sample_id),            0, CSS_LONG, "0"},
    {20, 24,             "station_id",  "%d", offset(station_id),           0, CSS_LONG, "0"},
    {26, 30,            "detector_id",  "%d", offset(detector_id),          0, CSS_LONG, "0"},
    {32, 288,       "input_file_name",  "%s", offset(input_file_name),      0, CSS_STRING, "-"},
    {290, 290,          "sample_type",  "%s", offset(sample_type),          0, CSS_STRING, "-"},
    {292, 292,            "data_type",  "%s", offset(data_type),            0, CSS_STRING, "-"},
    {294, 310,             "geometry",  "%s", offset(geometry),             0, CSS_STRING, "-"},
    {312, 316,   "spectral_qualifier",  "%s", offset(spectral_qualifier),   0, CSS_STRING, "-"},
    {318, 334,         "transmit_dtg",  "%s", offset(transmit_dtg),         0, CSS_LDDATE, DTNULL},
    {336, 352,        "collect_start",  "%s", offset(collect_start),        0, CSS_LDDATE, DTNULL},
    {354, 370,         "collect_stop",  "%s", offset(collect_stop),         0, CSS_LDDATE, DTNULL},
    {372, 388,    "acquisition_start",  "%s", offset(acquisition_start),    0, CSS_LDDATE, DTNULL},
    {390, 406,     "acquisition_stop",  "%s", offset(acquisition_stop),     0, CSS_LDDATE, DTNULL},
    {408, 417, "acquisition_real_sec", "%lf", offset(acquisition_real_sec), 0, CSS_DOUBLE, "0"},
    {419, 428, "acquisition_live_sec", "%lf", offset(acquisition_live_sec), 0, CSS_DOUBLE, "0"},
    {430, 437,             "quantity", "%lf", offset(quantity),             0, CSS_DOUBLE, "0"},
    {439, 455,              "moddate",  "%s", offset(moddate),              0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsSampleData, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsSampleDataClass::createGardsSampleData,
		sizeof(CssGardsSampleDataClass));
  }
}

CssGardsSampleDataClass::CssGardsSampleDataClass(void) : CssTableClass(cssGardsSampleData)
{
    defineGardsSampleData();
    initTable();
}

CssGardsSampleDataClass::CssGardsSampleDataClass(CssGardsSampleDataClass &a) :
			CssTableClass(cssGardsSampleData)
{
    defineGardsSampleData();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgStdSpectraResultClass.
 */
static void defineGardsBgStdSpectraResult(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgStdSpectraResultClass a(0);
    CssClassDescription des[] = {
    {1,     4,      "sample_id", "%ld", offset(sample_id),      0, CSS_LONG,   "0"},
    {6,     9, "std_spectra_id", "%ld", offset(std_spectra_id), 0, CSS_LONG,   "0"},
    {11,   34,    "beta_coeff1", "%lf", offset(beta_coeff1),    0, CSS_DOUBLE, "0"},
    {36,   59,    "beta_coeff2", "%lf", offset(beta_coeff2),    0, CSS_DOUBLE, "0"},
    {61,   84,    "beta_coeff3", "%lf", offset(beta_coeff3),    0, CSS_DOUBLE, "0"},
    {86,  109,   "gamma_coeff1", "%lf", offset(gamma_coeff1),   0, CSS_DOUBLE, "0"},
    {111, 134,   "gamma_coeff2", "%lf", offset(gamma_coeff2),   0, CSS_DOUBLE, "0"},
    {136, 159,   "gamma_coeff3", "%lf", offset(gamma_coeff3),   0, CSS_DOUBLE, "0"},
    {161, 184,       "estimate", "%lf", offset(estimate),       0, CSS_DOUBLE, "0"},
    {186, 209,          "error", "%lf", offset(error),          0, CSS_DOUBLE, "0"},
    {211, 227,        "moddate",  "%s", offset(moddate),        0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgStdSpectraResult,
	sizeof(des)/sizeof(CssClassDescription),
	des, 0, NULL, CssGardsBgStdSpectraResultClass::createGardsBgStdSpectraResult,
	sizeof(CssGardsBgStdSpectraResultClass));
  }
}

CssGardsBgStdSpectraResultClass::CssGardsBgStdSpectraResultClass(void) :
			CssTableClass(cssGardsBgStdSpectraResult)
{
    defineGardsBgStdSpectraResult();
    initTable();
}

CssGardsBgStdSpectraResultClass::CssGardsBgStdSpectraResultClass(
	CssGardsBgStdSpectraResultClass &a) : CssTableClass(cssGardsBgStdSpectraResult)
{
    defineGardsBgStdSpectraResult();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsBgQcResultClass.
 */
static void defineGardsBgQcResult(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsBgQcResultClass a(0);
    CssClassDescription des[] = {
    {1,     4, "sample_id", "%ld", offset(sample_id), 0, CSS_LONG,   "0"},
    {6,    29, "amplitude", "%lf", offset(amplitude), 0, CSS_DOUBLE, "0"},
    {31,   54,      "fwhm", "%lf", offset(fwhm),      0, CSS_DOUBLE, "0"},
    {56,   79,  "centroid", "%lf", offset(centroid),  0, CSS_DOUBLE, "0"},
    {81,  104,    "offset", "%lf", offset(offset),    0, CSS_DOUBLE, "0"},
    {106, 129,     "slope", "%lf", offset(slope),     0, CSS_DOUBLE, "0"},
    {131, 147,   "moddate",  "%s", offset(moddate),   0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsBgQcResult, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsBgQcResultClass::createGardsBgQcResult,
		sizeof(CssGardsBgQcResultClass));
  }
}

CssGardsBgQcResultClass::CssGardsBgQcResultClass(void) : CssTableClass(cssGardsBgQcResult)
{
    defineGardsBgQcResult();
    initTable();
}

CssGardsBgQcResultClass::CssGardsBgQcResultClass(CssGardsBgQcResultClass &a) :
			CssTableClass(cssGardsBgQcResult)
{
    defineGardsBgQcResult();
    initTable();
    a.copyTo(this);
}

/*---------------------------------------------------------------------------*/
/**
 * Define CssGardsRoiLimitsClass.
 */
static void defineGardsRoiLimits(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsRoiLimitsClass a(0);
    CssClassDescription des[] = {
    {1,     4,      "sample_id", "%ld", offset(sample_id),      0, CSS_LONG,  "0"},
    {6,     9,            "roi", "%ld", offset(roi),            0, CSS_LONG,  "0"},
    {11,   18, "b_energy_start", "%lf", offset(b_energy_start), 0, CSS_DOUBLE,"0"},
    {20,   27,  "b_energy_stop", "%lf", offset(b_energy_stop),  0, CSS_DOUBLE,"0"},
    {29,   36, "g_energy_start", "%lf", offset(g_energy_start), 0, CSS_DOUBLE,"0"},
    {38,   45,  "g_energy_stop", "%lf", offset(g_energy_stop),  0, CSS_DOUBLE,"0"},
    {47,   63,        "moddate",  "%s", offset(moddate),        0, CSS_LDDATE, DTNULL},
    };
    CssTableClass::define(cssGardsRoiLimits, sizeof(des)/sizeof(CssClassDescription),
		des, 0, NULL, CssGardsRoiLimitsClass::createGardsRoiLimits,
		sizeof(CssGardsRoiLimitsClass));
  }
}

CssGardsRoiLimitsClass::CssGardsRoiLimitsClass(void) : CssTableClass(cssGardsRoiLimits)
{
    defineGardsRoiLimits();
    initTable();
}

CssGardsRoiLimitsClass::CssGardsRoiLimitsClass(CssGardsRoiLimitsClass &a) :
			CssTableClass(cssGardsRoiLimits)
{
    defineGardsRoiLimits();
    initTable();
    a.copyTo(this);
}
/* End bg_analyze */


/*---------------------------------------------------------------------------*/
/* Start noble gas gui */
/**
 * Define CssGardsEfficiencyPairsClass.
 */
static void defineGardsEfficiencyPairs(void)
{
  static bool defined = false;
  if( !defined ) {
    defined = true;
    CssGardsEfficiencyPairsClass a(0);
    CssClassDescription des[] = {
    {1,     4,    "sample_id", "%ld", offset(sample_id),    0, CSS_LONG, "0"},
    {6,    13, "effic_energy", "%lf", offset(effic_energy), 0,CSS_DOUBLE,"0"},
    {15,   22,   "efficiency", "%lf", offset(efficiency),   0,CSS_DOUBLE,"0"},
    {23,   30, " effic_error", "%lf", offset(effic_error),  0,CSS_DOUBLE,"0"},
    };
    CssTableClass::define(cssGardsEfficiencyPairs,sizeof(des)/sizeof(CssClassDescription),
	des, 0, NULL, CssGardsEfficiencyPairsClass::createGardsEfficiencyPairs,
	sizeof(CssGardsEfficiencyPairsClass));
  }
}

CssGardsEfficiencyPairsClass::CssGardsEfficiencyPairsClass(void) :
			CssTableClass(cssGardsEfficiencyPairs)
{
    defineGardsEfficiencyPairs();
    initTable();
}

CssGardsEfficiencyPairsClass::CssGardsEfficiencyPairsClass(CssGardsEfficiencyPairsClass &a) :
			CssTableClass(cssGardsEfficiencyPairs)
{
    defineGardsEfficiencyPairs();
    initTable();
    a.copyTo(this);
}
/* End noble gas gui */


/*---------------------------------------------------------------------------*/
static bool tables_defined = false;

void CssTableClass::predefineTables(void)
{
    if( !tables_defined ) {
	tables_defined = true;
	defineArrival();
	defineWfdisc();
	defineHistory();
	defineChanname();
	defineFrameProduct();
	defineFpDescription();
	defineMd5Digest();
	defineClf();
	defineWaveInterval();
	defineOutage();
	defineOrigin();
	defineOrigerr();
	defineOrigaux();
	defineLastid();
	defineSensor();
	defineInstrument();
	defineGregion();
	defineSitechan();
	defineSite();
	defineWftag();
	defineXtag();
	defineFsdisc();
	defineFsrecipe();
	defineFstag();
	defineSpdisc();
	defineDervdisc();
	definePmccRecipe();
	definePmccFeatures();
	definePick();
	defineFilter();
	defineAssoc();
	defineStassoc();
	defineAffiliation();
	defineStanet();
	defineHydroFeatures();
	defineInfraFeatures();
	defineStamag();
	defineNetmag();
	defineAmpdescript();
	defineAmplitude();
	defineStaconf();
	defineParrival();

	defineDynamic5();
	defineDynamic10();
	defineDynamic20();
	defineDynamic40();
	defineDynamic60();

	// Start bg_analyze
	defineGardsBgEnergyCal();
	defineGardsBgProcParams();
	defineGardsBgProcParamsRoi();
	defineGardsRoiChannels();
	defineGardsBgRoiConcs();
	defineGardsBgRoiCounts();
	defineGardsBgIsotopeConcs();
	defineGardsSampleStatus();
	defineGardsSampleData();
	defineGardsBgStdSpectraResult();
	defineGardsBgQcResult();
	defineGardsRoiLimits();
	// End bg_analyze
  
	// start noble gas gui
	defineGardsEfficiencyPairs();
    }
}
