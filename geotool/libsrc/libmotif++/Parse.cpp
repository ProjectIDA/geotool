/** \file Parse.cpp
 *  \brief Defines namespace parse
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "motif++/Parse.h"
#include "gobject++/CssTables.h"
using namespace std;

extern "C" {
#include "libstring.h"
#include "libtime.h"
}

bool Parse::parseArray(const string &name, const string &array_name,
		int max_ndex, float *array, string &value, ParseVar *ret)
{
    int i, nextc;
    ostringstream os;

    if( parseCompare(name, array_name) ) {
	os << "@f" << (long)array << ":" << max_ndex;
	value.assign(os.str());
	*ret = STRING_RETURNED;
	return true;
    }
    if(parseArrayIndex(name, array_name, max_ndex, &i, &nextc, value, ret)) {
	if(*ret == STRING_RETURNED) {
	    parsePrintFloat(value, array[i]);
	}
	return true;
    }
    if( parseCompare(name, array_name + ".size_") ) {
	parsePrintInt(value, max_ndex);
	*ret = STRING_RETURNED;
	return true;
    }

    return false;
}

bool Parse::parseArray(const string &name, const string &array_name,
		int max_ndex, double *array, string &value, ParseVar *ret)
{
    int i, nextc;
    ostringstream os;

    if( parseCompare(name, array_name) ) {
	os << "@d" << (long)array << ":" << max_ndex;
	value.assign(os.str());
	*ret = STRING_RETURNED;
	return true;
    }
    if(parseArrayIndex(name, array_name, max_ndex, &i, &nextc, value, ret)) {
	if(*ret == STRING_RETURNED) {
            parsePrintDouble(value, array[i]);
	}
	return true;
    }
    if( parseCompare(name, array_name + ".size_") ) {
        parsePrintInt(value, max_ndex);
	*ret = STRING_RETURNED;
	return true;
    }

    return false;
}

bool Parse::parseArray(const string &name, const string &array_name,
		int max_ndex, int *array, string &value, ParseVar *ret)
{
    int i, nextc;
    ostringstream os;

    if( parseCompare(name, array_name) ) {
	os << "@i" << (long)array << ":" << max_ndex;
	value.assign(os.str());
	*ret = STRING_RETURNED;
	return true;
    }
    if(parseArrayIndex(name, array_name, max_ndex, &i, &nextc, value, ret)) {
	if(*ret == STRING_RETURNED) {
	    parsePrintInt(value, array[i]);
	}
	return true;
    }
    if( parseCompare(name, array_name + ".size_") ) {
	parsePrintInt(value, max_ndex);
	*ret = STRING_RETURNED;
	return true;
    }

    return false;
}

bool Parse::parseArrayIndex(const string &name, const string &array_name,
		int max_ndex, int *ndex, int *nextc, string &value,
		ParseVar *ret)
{
    string ndx;
    const char *c, *e;
    int i, n;

    *ndex = -1;
    *nextc = (int)name.length();

    n = (int)array_name.length() + 1;

    if( !parseCompare(name, array_name + "[", n) ) {
	*ret = VARIABLE_NOT_FOUND;
	return false;
    }

    c = name.c_str() + n;

    while(*c != '\0' && isspace((int)*c)) c++;
    if(*c == '\0') {
	value.assign("variable syntax error: " + name);
	*ret = VARIABLE_ERROR;
	return true;
    }
    e = c+1;
    while(*e != '\0' && *e != ']' && !isspace((int)*e)) e++;
    if(*e == '\0') {
	value.assign("variable syntax error: " + name);
	*ret = VARIABLE_ERROR;
	return true;
    }
    ndx.assign(c, e-c);
    
    if(!stringToInt(ndx.c_str(), &i) || i <= 0 || i > max_ndex) {
	value.assign("Invalid index: " + name);
	*ret = VARIABLE_ERROR;
	return true;
    }

    c = e;
    if(*c != ']') {
	c++;
	while(*c != '\0' && *c != ']') c++;
	if(*c == '\0') {
	    value.assign("variable syntax error: " + name);
	    *ret = VARIABLE_ERROR;
	    return true;
	}
    }
    c++;
    *ndex = i-1;
    *nextc = (int)(c - name.c_str());

    *ret = STRING_RETURNED;
    return true;
}

bool Parse::parseFuncArg(const string &name, int arg_pos,
			const string &arg_name, double *d, string &value)
{
    int ret;
    size_t i;
    ostringstream a;
    string func;

    if(arg_pos < 1) {
	cerr << "parseFuncArg: invalid argument index: " << arg_pos << endl;
	return false;
    }
    a << "arg" << arg_pos;
    if( !(ret = stringGetDoubleArg(name.c_str(), a.str().c_str(), d)) ) {
	return true;
    }

    func = ((i = name.find('_')) != string::npos) ? name.substr(0, i) : name;
    if(ret == -1) {
	value.assign(func + ": missing " + arg_name + " argument");
    }
    else if(ret == -2) {
	value.assign(func + ": invalid type for " + arg_name + " argument");
    }
    return false;
}

bool Parse::parseFuncArg(const string &name, int arg_pos,
		const string &arg_name, int *i, string &value)
{
    int ret;
    size_t j;
    ostringstream a;
    string func;

    if(arg_pos < 1) {
	cerr << "parseFuncArg: invalid argument index: " << arg_pos << endl;
	return false;
    }
    a << "arg" << arg_pos;
    if( !(ret = stringGetIntArg(name.c_str(), a.str().c_str(), i)) ) {
	return true;
    }
    func = ((j = name.find('_')) != string::npos) ? name.substr(0, j) : name;
    if(ret == -1) {
	value.assign(func + ": missing " + arg_name + " argument");
    }
    else if(ret == -2) {
	value.assign(func + ": invalid type for " + arg_name + " argument");
    }
    return false;
}

bool Parse::parseFuncArg(const string &name, int arg_pos,
		const string &arg_name, long *i, string &value)
{
    int ret;
    size_t j;
    ostringstream a;
    string func;

    if(arg_pos < 1) {
	cerr << "parseFuncArg: invalid argument index: " << arg_pos << endl;
	return false;
    }
    a << "arg" << arg_pos;
    if( !(ret = stringGetLongArg(name.c_str(), a.str().c_str(), i)) ) {
	return true;
    }
    func = ((j = name.find('_')) != string::npos) ? name.substr(0, j) : name;
    if(ret == -1) {
	value.assign(func + ": missing " + arg_name + " argument");
    }
    else if(ret == -2) {
	value.assign(func + ": invalid type for " + arg_name + " argument");
    }
    return false;
}

bool Parse::parseFuncArg(const string &name, int arg_pos,
		const string &arg_name, string &arg_value, string &var_value)
{
    size_t i;
    ostringstream a;
    string func;

    if(arg_pos < 1) {
	cerr << "parseFuncArg: invalid argument index: " << arg_pos << endl;
	return false;
    }
    a << "arg" << arg_pos;

    if( parseGetArg(name, a.str(), arg_value) ) return true;

    func = ((i = name.find('_')) != string::npos) ? name.substr(0, i) : name;
    var_value.assign(func + ": missing " + arg_name + " argument");
    return false;
}

ParseVar Parse::tableFindIndices(const string &name, gvector<CssTableClass *> &v,
				string &value)
{
    int i, j, num_col, num_members, mem[100];
    char *member_address, s[50];
    string member[100], values[100];
    long search_long[100];
    ostringstream os;
    CssClassDescription *des;

    if(v.size() == 0) {
	value.assign("0");
	return STRING_RETURNED;
    }

    j = 1;
    for(i = 0; i < 100; i++) {
	os.str("");
	os << "arg" << j++;
	if( !parseGetArg(name, os.str(), member[i]) ) break;

	os.str("");
	os << "arg" << j++;
	if( !parseGetArg(name, os.str(), values[i]) ) {
	    value.assign("find_indices: missing value for " + member[i]);
	    return VARIABLE_ERROR;
	}
    }
    num_col = i;

    if(num_col == 0) {
	value.assign("find_indices: missing arguments");
	return VARIABLE_ERROR;
    }

    num_members = v[0]->getNumMembers();
    des = v[0]->description();

    for(j = 0; j < num_col; j++) {
	for(i = 0; i < num_members && parseCompare(member[j],des[i].name); i++);
	if(i == num_members) {
	    value.assign("find_indices: name " + member[j] + " not found");
            return VARIABLE_ERROR;
        }
	if(des[i].type == CSS_LONG || des[i].type == CSS_JDATE || 
		des[i].type == CSS_INT || des[i].type == CSS_QUARK)
	{
	    if(!stringToLong(values[j].c_str(), &search_long[j])) {
		value.assign("find_indices: "+member[j]+" must be an integer");
		return VARIABLE_ERROR;
	    }
	}
	mem[j] = i;
    }

    value.clear();
    os.str("");

    for(i = 0; i < v.size(); i++)
    {
	CssTableClass *o = v[i];

	bool ok=true;
	for(j = 0; j < num_col && ok; j++)
	{
	    member_address = (char *)o + des[mem[j]].offset;
	    switch(des[mem[j]].type)
	    {
	    case CSS_STRING:
		if(parseCompare(values[j], member_address)) ok = false;
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		snprintf(s, sizeof(s), "%s",
			timeDateString((DateTime *)member_address));
		if(parseCompare(values[j], s)) ok = false;
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		if(search_long[j] != *(long *)member_address) ok = false;
		break;
	    case CSS_QUARK:
	    case CSS_INT:
		if(search_long[j] != *(int *)member_address) ok = false;
		break;
	    case CSS_DOUBLE:
	    case CSS_TIME:
		snprintf(s, sizeof(s), "%.15g", *(double *)member_address);
		if(parseCompare(values[j], s)) ok = false;
		break;
	    case CSS_FLOAT:
		snprintf(s, sizeof(s), "%.7g", *(float *)member_address);
		if(parseCompare(values[j], s)) ok = false;
		break;
	    default:
		ok = false;
            }
	}

	if(ok) {
	    os << i+1 << ",";
	}
    }
    value.assign(os.str());
    // don't take the last ","
    size_t n = value.length();
    if((int)n > 1) value.erase(n-1);

    return STRING_RETURNED;
}

/** Compare the first n characters of strings s1 and s2, case insensitive.
 *  Returns false is the length of s2 is not n. Do not compare characters
 *  that are not alphanumeric.
 */
bool Parse::sameName(const string &s1, const string &s2, int n)
{
    if((int)s2.length() != n || n <= 0) return false;

    if( !isalpha((int)s2[n-1]) && !isdigit((int)s2[n-1]) ) n--;

    if((int)s1.length() < n) return false;

    for(int i = 0; i < n; i++) {
	bool b1 = (isalpha((int)s1[i]) || isdigit((int)s1[i]));
	bool b2 = (isalpha((int)s2[i]) || isdigit((int)s2[i]));
	if(b1 != b2) return false;
	if(b1 && (tolower((int)s1[i]) != tolower((int)s2[i])) ) return false;
    }
    return true;
}

bool Parse::sameName(const string &s1, const string &s2) {
    int n = (int)s2.length();
    if((int)s1.length() != n) return false;
    return Parse::sameName(s1, s2, n);
}

bool Parse::parseString(const string &cmd, const string &name, string &value)
{
    const char *s = cmd.c_str();
    int n = (int)name.length();

    while(*s != '\0' && isspace((int)*s)) s++;

    if( !strncasecmp(s, name.c_str(), n) )
    {
	// the character after the name must be white space, '.', or '='.
	char *c = (char *)(s + n);
	if(!isspace((int)*c) && *c != '.' && *c != '=') return false;
	if(*c == '.') {
	    c++;
	    while(*c != '\0' && isspace((int)*c)) c++;
	}
	else {
	    while(*c != '\0' && isspace((int)*c)) c++;

	    if(*c == '=') {
		c++;
		while(*c != '\0' && isspace((int)*c)) c++;
	    }
	}
	value.assign(c);
	return true;
    }
    return false;
}

bool Parse::parseArg(const string &cmd, const string &name, string &value)
{
    const char *s = cmd.c_str();
    int n = (int)name.length();

    while(*s != '\0' && isspace((int)*s)) s++;

    if( !strncasecmp(s, name.c_str(), n) )
    {
	// the character after the name must be white space, '.', or '='.
	char *c = (char *)(s + n);
	if(!isspace((int)*c) && *c != '.' && *c != '=') return false;
	if(*c == '.') {
	    c++;
	    while(*c != '\0' && isspace((int)*c)) c++;
	}
	else {
	    while(*c != '\0' && isspace((int)*c)) c++;

	    if(*c == '=') {
		c++;
		while(*c != '\0' && isspace((int)*c)) c++;
	    }
	}
	n = (int)strlen(c);
	if(*c == '"' || *c == '\'' || *c == '`') {
	    char a = *c;
	    while(isspace((int)c[n-1]) && n > 1) n--;
	    if(n > 1 && c[n-1] == a) {
		c++;
		n -= 2;
	    }
	}
	value.assign(c, n);
	return true;
    }
    return false;
}

bool Parse::parseGetArg(const string &s, const string &name, string &value)
{
    int n;
    char a;
    const char *beg=NULL, *end, *c, *str;

    str = s.c_str();
    if((n = (int)name.length()) <= 0) return false;

    // find name offset with white space or '='

    beg = str;
    while(*beg != '\0' && isspace((int)*beg)) beg++;

    while(*beg != '\0') {
	if(!strncasecmp(beg, name.c_str(), n)) {
	    c = beg+n; // first character after the name
	    while(*c != '\0' && isspace((int)*c)) c++; // skip spaces
	    if(*c == '=') { // found equals. set beg to value
		beg = c+1;
		while(*beg != '\0' && isspace((int)*beg)) beg++;
		break;
	    }
	}
	// no match. find the next item preceded by a space
	while(*beg != '\0' && !isspace((int)*beg)) {
	    if(*beg == '"' || *beg == '\'' || *beg == '`') { // skip quotes
		a = *beg;
		beg++;
		while(*beg != '\0' && *beg != a) beg++;
		if(*beg == a) beg++;
	    }
	    else beg++;
	}
	while(*beg != '\0' && isspace((int)*beg)) beg++;
    }
    if(*beg == '\0') return false;
    
/*
    look = str;
    while((beg = strstr(look, name)) != NULL)
    {
	if( (     beg == str  || isspace((int)*(beg-1)) || *(beg-1)=='=') &&
	    (*(beg+n) == '\0' || isspace((int)*(beg+n)) || *(beg+n)=='=') )
	{
	    break;
	}
	look = beg+n;
    }
    if(beg == NULL) return NULL;

    // get the token following name
    beg += n;
    while(*beg != '\0' && (isspace((int)*beg) || *beg=='=')) beg++;
*/

    if(*beg == '"' || *beg == '\'' || *beg == '`')
    {
	// token is enclosed in quotes. Don't return the quotes.
	a = *beg;
	beg++;
	end = beg;
	while(*end != '\0' && *end != a) end++;
    }
    else if(*beg == '{')  // token is enclosed in {}. Don't return the brackets.
    {
	int num = 1;
	beg++;
	end = beg;
	while(num > 0) {    // takes care of embedded '{'s
	    while(*end != '\0' && *end != '}') {
		if(*end == '{') num++;
		end++;
	    }
	    num--;
	    if(num) end++; 
	}
    }
    else if(*beg == '(')    // token is enclosed in (). Return the ()'s.
    {
	int num = 1;
	end = beg;
	while(num > 0) {    // takes care of embedded '('s
	    while(*end != '\0' && *end != ')') {
		if(*end == '(') num++;
		end++;
	    }
	    num--;
	    end++;
	}
    }
    else if(*beg == '[')    // token is enclosed in []. Return the []'s.
    {
	int num = 1;
	end = beg;
	while(num > 0) {    // takes care of embedded '['s
	    while(*end != '\0' && *end != ']') {
		if(*end == '[') num++;
		end++;
	    }
	    num--;
	    end++;
	}
    }
    else
    {
	end = beg;
	while(*end != '\0' && !isspace((int)*end)) end++;
    }

    value.assign(beg, end-beg);

    return true;
}

bool Parse::parseGetArg(const string &s, const string &cmd, string &msg,
		const string &name, int *value)
{
    int ret = stringGetIntArg(s.c_str(), name.c_str(), value);
    if( !ret ) {
	return true;
    }
    else if( ret == -2 ) {
	msg.assign(cmd + ": expecting integer value for '" + name + "'");
    }
    return false;
}

bool Parse::parseGetArg(const string &s, const string &cmd, string &msg,
		const string &name, long *value)
{
    int ret = stringGetLongArg(s.c_str(), name.c_str(), value);
    if( !ret ) {
	return true;
    }
    else if( ret == -2 ) {
	msg.assign(cmd + ": expecting integer value for '" + name + "'");
    }
    return false;
}

bool Parse::parseGetArg(const string &s, const string &cmd, string &msg,
		const string &name, double *value)
{
    int ret = stringGetDoubleArg(s.c_str(), name.c_str(), value);
    if( !ret ) {
	return true;
    }
    else if( ret == -2 ) {
	msg.assign(cmd + ": expecting number value for '" + name + "'");
    }
    return false;
}

bool Parse::parseGetArg(const string &s, const string &cmd, string &msg,
		const string &name, bool *value)
{
    int i;
    int ret = stringGetBoolArg(s.c_str(), name.c_str(), &i);
    if( !ret ) {
	*value = (i != 0) ? true : false;
	return true;
    }
    else if( ret == -2 ) {
	msg.assign(cmd + ": expecting boolean value for '" + name + "'");
    }
    return false;
}

bool Parse::missingArgs(const string &cmd, int num, const char **cmds,
			string &msg)
{
    for(int i = 0; i < num; i++) if(!strcasecmp(cmd.c_str(), cmds[i])) {
        msg.assign(string(cmds[i]) + ": missing argument(s)");
        return true;
    }
    return false;
}

/*
 * cmd is the command line after the command
 */
bool Parse::unknownArgs(const string &cmd, string &msg, int num_args,
			const char **args)
{
    int i;
    char a, *b, c, *e, *s;

    s = strdup(cmd.c_str());
    b = s;

    while(*b != '\0') {
	while(isspace((int)*b)) b++;
	e = b; // b points to the name in name=value
	while(*e != '\0' && !isspace((int)*e) && *e != '=') e++;
	a = *e;
	*e = '\0';
	for(i = 0; i < num_args && strcasecmp(b, args[i]); i++);
	if(i == num_args) {
	    msg.assign("Unknown argument: " + string(b));
	    free(s);
	    return true;
	}
	if(a == '\0') break;
	b = e+1;
	while(isspace((int)*b)) b++;
	if(a == '=') { // skip value
	    if(*b  == '\'' || *b == '"' || *b == '`' || *b == '{') {
		c = (*b == '{') ? '}' : *b;
		b++;
		while(*b != '\0' && *b != c) b++; // skip the value
		if(*b != '\0') b++;
	    }
	    else {
		while(*b != '\0' && !isspace((int)*b)) b++;  // skip the value
	    }
	}
    }
    free(s);
    return false;
}

/* returns true if the command name matches
 * *err = false if the command name matches and the required arguments are
 *		found and there are no extra arguments
 * *err = true if the command name matches, but a required argument is missing
 *		or an extra argument was found
 */
bool Parse::parseFind(const string &cmd, const string &name,
		string &msg, bool *err,
		const char *arg0, bool required0,
		const char *arg1, bool required1,
		const char *arg2, bool required2,
		const char *arg3, bool required3,
		const char *arg4, bool required4,
		const char *arg5, bool required5,
		const char *arg6, bool required6,
		const char *arg7, bool required7,
		const char *arg8, bool required8,
		const char *arg9, bool required9)
{
    int i, n, num_args;
    ParseArg args[10];

    *err = false;
    if(arg0 == NULL) {
	if(parseCompare(cmd, name)) return true;
	n = (int)name.length();
	if(parseCompare(cmd, name, n) && (int)cmd.length() > n
		&& isspace((int)cmd[n])) {
	    msg.assign(name + ": unexpected argument(s): " + cmd.substr(n));
	    *err = true;
	    return true;
	}
    }
    i = 0;
    if(arg0) { args[i].name = arg0; args[i++].required = required0; }
    if(arg1) { args[i].name = arg1; args[i++].required = required1; }
    if(arg2) { args[i].name = arg2; args[i++].required = required2; }
    if(arg3) { args[i].name = arg3; args[i++].required = required3; }
    if(arg4) { args[i].name = arg4; args[i++].required = required4; }
    if(arg5) { args[i].name = arg5; args[i++].required = required5; }
    if(arg6) { args[i].name = arg6; args[i++].required = required6; }
    if(arg7) { args[i].name = arg7; args[i++].required = required7; }
    if(arg8) { args[i].name = arg8; args[i++].required = required8; }
    if(arg9) { args[i].name = arg9; args[i++].required = required9; }
    num_args = i;
    for(i = 0; i < num_args; i++) args[i].found = false;

    return parseCheckArgs(cmd, name, num_args, args, msg, err);
}

/* returns true if the command name matches
 * *err = false if the command name matches and the required arguments are
 *		found and there are no extra arguments
 * *err = true if the command name matches, but a required argument is missing
 *		or an extra argument was found
 */
bool Parse::parseCheckArgs(const string &cmd, const string &name, int num_args,
		ParseArg *args, string &msg, bool *err)
{
    char a, *b, c, *e, *s;
    int i, n;

    *err = false;
    n = (int)name.length();
    if(!parseCompare(cmd, name, n) || ((int)cmd.length() > n &&
		!isspace((int)cmd[n])) ) return false;

    s = strdup(cmd.c_str()+n);
    b = s;

    while(*b != '\0') {
	while(isspace((int)*b)) b++;
	e = b; // b points to the name in name=value or to just name
	while(*e != '\0' && !isspace((int)*e) && *e != '=') e++;
	a = *e;
	*e = '\0';
	for(i = 0; i < num_args && strcasecmp(b, args[i].name); i++);
	if(i == num_args) {
	    msg.assign(name + ": unexpected argument: " + b);
	    free(s);
	    *err = true;
	    return true;
	}
	if(a == '\0') break;
	b = e+1;
	while(isspace((int)*b)) b++;
	if(a == '=') { // skip value
	    if(*b != '\0') args[i].found = true;
	    if(*b  == '\'' || *b == '"' || *b == '`' || *b == '{') {
		c = (*b == '{') ? '}' : *b;
		b++;
		while(*b != '\0' && *b != c) b++; // skip the value
		if(*b != '\0') b++;
	    }
	    else {
		while(*b != '\0' && !isspace((int)*b)) b++;  // skip the value
	    }
	}
    }
    free(s);

    for(i = 0; i < num_args; i++) {
	if(args[i].required && !args[i].found) {
	    msg.assign(name + ": missing argument '" + args[i].name + "'");
	    *err = true;
	    return true;
	}
    }
    return true;
}

bool Parse::parseArgFound(int num_args, ParseArg *args)
{
    int i;
    for(i = 0; i < num_args && !args[i].found; i++);
    return (i < num_args) ? true : false;
}

void Parse::parseTrim(string &s)
{
    int i, n;

    n = (int)s.length();
    for(i = 0; i < n && isspace((int)s[i]); i++);
    if(i > 0) s.erase(0, i);
    n = (int)s.length();
    for(i = n-1; i >= 0 && isspace((int)s[i]); i--);
    if(i < n-1) s.erase(i+1);
}
