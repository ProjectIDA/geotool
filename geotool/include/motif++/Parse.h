#ifndef _PARSE_H_
#define _PARSE_H_

#include "gobject++/gvector.h"
#include "gobject++/CssTableClass.h"

typedef struct
{
    const char	*name;
    bool	required;
    bool	found;
} ParseArg;

/** 
 *
 *  @ingroup libmotif
 */
namespace Parse
{
    inline bool parseCompare(const string &cmd, const string &name) {
	return !strcasecmp(cmd.c_str(), name.c_str()); }
    inline bool parseCompare(const string &cmd, const string &name, int n) {
	return !strncasecmp(cmd.c_str(), name.c_str(), n); }
    bool parseArg(const string &s, const string &name, string &value);
    bool parseString(const string &s, const string &name, string &value);

    bool parseArray(const string &name, const string &array_name, int max_ndex,
		float *array, string &value, ParseVar *ret);
    bool parseArray(const string &name, const string &array_name, int max_ndex,
		double *array, string &value, ParseVar *ret);
    bool parseArray(const string &name, const string &array_name, int max_ndex,
		int *array, string &value, ParseVar *ret);
    bool parseArrayIndex(const string &name, const string &array_name,
		int max_ndex, int *ndex, int *nextc, string &value,
		ParseVar *ret);
    bool parseFuncArg(const string &c, int arg, const string &arg_name,
		double *d, string &value);
    bool parseFuncArg(const string &c, int arg, const string &arg_name, int *i,
		string &value);
    bool parseFuncArg(const string &c, int arg, const string &arg_name, long *i,
		string &value);
    bool parseFuncArg(const string &c, int arg, const string &arg_name,
		string &arg_value, string &var_value);
    ParseVar tableFindIndices(const string &name, gvector<CssTableClass *> &v,
		string &value);
    bool sameName(const string &s1, const string &s2, int n);
    bool sameName(const string &s1, const string &s2);

    bool parseGetArg(const string &c, const string &name, string &value);
    bool parseGetArg(const string &c, const string &cmd, string &msg,
		const string &name, int *value);
    bool parseGetArg(const string &c, const string &cmd, string &msg,
		const string &string, long *value);
    bool parseGetArg(const string &c, const string &cmd, string &msg,
		const string &name, double *value);
    bool parseGetArg(const string &c, const string &cmd, string &msg,
		const string &name, bool *value);
    inline bool parseGetArg(const string &cmd, const string &name,
		double *value) {
	return (stringGetDoubleArg(cmd.c_str(), name.c_str(), value) == 0);
    }
    inline bool parseGetArg(const string &cmd, const string &name,
		long *value) {
	return (stringGetLongArg(cmd.c_str(), name.c_str(), value) == 0);
    }
    inline bool parseGetArg(const string &cmd, const string &name,
		int *value) {
	long l;
	bool ret = (stringGetLongArg(cmd.c_str(), name.c_str(), &l) == 0);
	if(ret) *value = (int)l;
	return ret;
    }

    bool unknownArgs(const string &cmd, string &msg, int num_args,
		const char **args);
    bool missingArgs(const string &cmd, int num, const char **cmds,
		string &msg);
    bool parseFind(const string &cmd, const string &name, string &msg,bool *err,
		const char *arg0=NULL, bool required0=false,
		const char *arg1=NULL, bool required1=false,
		const char *arg2=NULL, bool required2=false,
		const char *arg3=NULL, bool required3=false,
		const char *arg4=NULL, bool required4=false,
		const char *arg5=NULL, bool required5=false,
		const char *arg6=NULL, bool required6=false,
		const char *arg7=NULL, bool required7=false,
		const char *arg8=NULL, bool required8=false,
		const char *arg9=NULL, bool required9=false);
    bool parseCheckArgs(const string &cmd, const string &name, int num_args,
		ParseArg *args, string &msg, bool *err);
    bool parseArgFound(int num_args, ParseArg *args);
    void parseTrim(string &s);
    inline void parsePrintDouble(string &msg, double d) {
		char s[30];
		snprintf(s, sizeof(s), "%.15g", d);
		msg.assign(s);
    }
    inline void parsePrintFloat(string &msg, float d) {
		char s[30];
		snprintf(s, sizeof(s), "%.7g", d);
		msg.assign(s);
    }
    inline void parsePrintInt(string &msg, long i) {
		char s[30];
		snprintf(s, sizeof(s), "%ld", i);
		msg.assign(s);
    }
};

#endif
