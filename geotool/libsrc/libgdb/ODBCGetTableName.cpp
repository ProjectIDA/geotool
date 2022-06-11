#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libgdb.h"

#define MAX_TABLES      20

typedef struct
{
        char            *author;        /* table author (or account) */
        char            *name;          /* table name (from the query) */
        char            *ref;           /* a query reference to the table */
} QTable;

/* The QTerm structure contains information about a single term of a "where"
 * constraint clause
 */
typedef struct
{
        char            *ref;   /* table reference ("a" in "affiliation a") */
        char            *member;/* member name ("sta" in "a.sta") */
        QTable          *table; /* table containing the member */
} QTerm;

typedef struct
{
        char *qbuf;
        int distinct;
        int num_ref, num_tables;
        QTerm refs[MAX_TABLES];
        QTable *tables[MAX_TABLES];
} QParseStruct;

static int parseQuery(const char *query, QParseStruct *p);
static int beforeWhere(QParseStruct *p, char **last, char **pos);
static int getPrimaryTable(QParseStruct *p);
static int get_selection(char *c, char **ref, char **member);
static int check_more(char *p);
static int preprocessQuery(const char *query, int n, char *q);

/**
 *  Get the table that a query will return. Parse the input query to get the
 *  author (account) and table name of the table that the query will return.
 *  @param query An SQL query that should return full table rows.
 *  @author A character array to receive the author name.
 *  @name A character array to receive the table name.
 *  @return 0 for success, -1 for failure. Retrieve the error \
 *  		number and error message with FFDBErrno() and FFDBErrMsg().
 */
int
ODBCGetTableName(const string &query, string &author, string &name)
{
	char *c, *last;
	int i, n;
	QParseStruct p;

	p.qbuf = NULL;
	p.distinct = 0;
	p.num_ref = 0;
	p.num_tables = 0;

	if(!parseQuery(query.c_str(), &p)) {
	    return -1;
	}
	Free(p.qbuf);
	for(i = 0; i < MAX_TABLES; i++) Free(p.tables[i]);

	if(query.empty() || query[0] == ',') {
	    return -1;
	}

	n = (int)query.length();
	p.qbuf = (char *)malloc(2*n);
	if(!p.qbuf) {
	    return -1;
	}

	/* Preprocess the query string to remove or insert spaces that will
 	 * make the parsing easier.
	 */
	if(!preprocessQuery(query.c_str(), n, p.qbuf)) {
	    Free(p.qbuf);
	    return -1;
	}
	for(i = 0; i < MAX_TABLES; i++) {
	    p.tables[i] = (QTable *)malloc(sizeof(QTable));
	    if(!p.tables[i]) {
		Free(p.qbuf);
		return -1;
	    }
	    p.tables[i]->author = NULL;
	    p.tables[i]->name = NULL;
	    p.tables[i]->ref = NULL;
	}

	/* Parse the query up to the "where" clause.
	 */
	if(!beforeWhere(&p, &last, &c))
	{
	    Free(p.qbuf);
	    for(i = 0; i < MAX_TABLES; i++) Free(p.tables[i]);
	    return -1;
	}
	if(!getPrimaryTable(&p)) {
	    Free(p.qbuf);
	    for(i = 0; i < MAX_TABLES; i++) Free(p.tables[i]);
	    return -1;
	}

	if(p.num_tables > 0) {
	    if(p.tables[0]->author == NULL) {
		author.assign("");
	    }
	    else {
		author.assign(p.tables[0]->author);
	    }
	    if(p.tables[0]->name == NULL) {
		name.assign("");
	    }
	    else {
		name.assign(p.tables[0]->name);
	    }
	}
	Free(p.qbuf);
	for(i = 0; i < MAX_TABLES; i++) Free(p.tables[i]);
	
	return 0;
}

/**
 * @private
 */
static int
parseQuery(const char *query, QParseStruct *p)
{
	char *c = NULL, *last = NULL;
	int i, n;

	if(query[0] == '\0' || query[0] == ',') {
	    return 0;
	}

	n = (int)strlen(query);
	p->qbuf = (char *)malloc(2*n);
	if(!p->qbuf) {
	    return 0;
	}

	/* Preprocess the query string to remove or insert spaces that will
 	 * make the parsing easier.
	 */
	if(!preprocessQuery(query, n, p->qbuf)) {
	    Free(p->qbuf);
	    return 0;
	}
	for(i = 0; i < MAX_TABLES; i++) {
	    p->tables[i] = (QTable *)malloc(sizeof(QTable));
	    if(!p->tables[i]) {
		Free(p->qbuf);
		return 0;
	    }
	    p->tables[i]->author = NULL;
	    p->tables[i]->name = NULL;
	    p->tables[i]->ref = NULL;
	}

	/* Parse the query up to the "where" clause.
	 */
	if(!beforeWhere(p, &last, &c))
	{
	    Free(p->qbuf);
	    for(i = 0; i < MAX_TABLES; i++) Free(p->tables[i]);
	    return 0;
	}
	if(p->num_tables == 0) {
	    Free(p->qbuf);
	    for(i = 0; i < MAX_TABLES; i++) Free(p->tables[i]);
	}
	
	if(p->num_tables > 4) {
	    Free(p->qbuf);
	    for(i = 0; i < MAX_TABLES; i++) Free(p->tables[i]);
	    return 0;
	}

	return getPrimaryTable(p);
}

/* Preprocess the query string to remove or insert spaces that will
 * make the parsing easier. Fails only if the query starts with an operator.
 * Take care of ';'
 */
static int
preprocessQuery(const char *query, int n, char *q)
{
	int i, j;

	if(query[0] == '\0' || query[0] == ',' || query[0] == '=' ||
	    query[0] == '<' || query[0] == '>' || query[0] == '+' ||
	    query[0] == '-') return 0;

	/* Remove spaces before commas, insure a space after each comma.
	 * Offset operators (=,>=,<=) with spaces.
	 */
	q[0] = query[0];
	for(i = j = 1; i < n; i++)
	{
	    if(query[i-1] == ',' && !isspace((int)query[i])) {
		q[j++] = ' ';
	    }
	    else if(query[i-1] == '=' && !isspace((int)query[i])) {
		q[j++] = ' ';
	    }
	    else if(query[i-1] == '<' && query[i] != '=' &&
			!isspace((int)query[i]))
	    {
		if(query[i] != '=') q[j++] = ' ';
	    }
	    else if(query[i-1] == '>' && query[i] != '=' &&
			!isspace((int)query[i]))
	    {
		if(query[i] != '=') q[j++] = ' ';
	    }
	    else if(query[i-1] == '+' && !isspace((int)query[i])) {
		q[j++] = ' ';
	    }
	    else if(query[i-1] == '-' && !isspace((int)query[i])) {
		q[j++] = ' ';
	    }

	    if(query[i] == '\'') {
		/* skip over character string */
		q[j++] = query[i++];
		while(i < n && query[i] != '\'') q[j++] = query[i++];
		if(i < n) q[j++] = query[i];
	    }
	    else if(query[i] == ',') {
		while(isspace((int)q[j-1])) j--;
		q[j++] = ',';
	    }
	    else if(query[i] == '=' && !isspace((int)query[i-1])) {
		if(query[i-1] != '<' && query[i-1] != '>') q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else if(query[i] == '>' && !isspace((int)query[i-1])) {
		q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else if(query[i] == '<' && !isspace((int)query[i-1])) {
		q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else if(query[i] == '+' && !isspace((int)query[i-1])) {
		q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else if(query[i] == '-' && !isspace((int)query[i-1])) {
		q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else if(query[i] == '(' && !isspace((int)query[i-1])) {
		q[j++] = ' ';
		q[j++] = query[i];
	    }
	    else {
		q[j++] = query[i];
	    }
	}
	q[j] = '\0';
	if(q[j-1] == ';') q[j-1] = '\0';

	return 1;
}

static int
beforeWhere(QParseStruct *p, char **last, char **pos)
{
	char *c = NULL;
	int more;
	int n;
	
	*pos = p->qbuf;
	if((c = (char *)strtok_r(p->qbuf, " \t\n", last)) == NULL) return 0;

	if(strcmp(c, "select")) return 0;

	if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) return 0;
	*pos = c;

	p->distinct = 0;
	if(!strcmp(c, "distinct")) {
	    p->distinct = 1;
	    if((c = (char *)strtok_r(NULL, " \t\n",last)) == NULL) return 0;
	    *pos = c;
	}
	if(!strcmp(c, "*")) {
	    /* "select *" */
	    p->num_ref = 0;
	    if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) return 0;
	    *pos = c;
	}
	else
	{
	    /* "select r.*" or "r.member, s.member" */
	    more = 1;
	    for(n = 0; n < MAX_TABLES && more; n++)
	    {
		more = get_selection(c, &p->refs[n].ref, &p->refs[n].member);
		if((c=(char *)strtok_r(NULL," \t\n",last)) ==NULL) return 0;
		*pos = c;
	    }
	    p->num_ref = n;
	}
	/* "select distinct r.* from" */
	if(strcmp(c, "from")) return 0;

	if((c = (char *)strtok_r(NULL, " \t\n", last)) == NULL) return 0;
	*pos = c;

	more = 1;
	for(n = 0; n < MAX_TABLES && more; n++)
	{
	    more = get_selection(c, &p->tables[n]->author, &p->tables[n]->name);
	    p->tables[n]->ref = NULL;

	    if(!more)
	    {
		if((c = (char *)strtok_r(NULL," \t\n",last)) != NULL)
		{
		    *pos = c;
		    if(strcasecmp(c, "where")) {
			/* not equal to "where", must be a reference */
			p->tables[n]->ref = c;
			more = check_more(c);
			if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) {
			    *pos = c;
			    more = 0;
			}
		    }
		}
	    }
	    else {
		if((c = (char *)strtok_r(NULL," \t\n",last)) == NULL) {
		    more = 0;
		}
		else {
		    *pos = c;
		    if(!strcasecmp(c, "where")) return 0;
		}
	    }
	}
	p->num_tables = n;
	*pos = c;
	return 1;
}

static int
getPrimaryTable(QParseStruct *p)
{
	char *r;
	int i, i_primary=0;
	QTable *primary_table;

	if(p->num_tables == 0)  {
	    return 0;
	}
	if(p->num_ref == 0) /* no secondary tables */
	{
	    if(p->num_tables > 1) {
		return 0;
	    }
	    i_primary = 0;
	}
	else if(p->num_ref == 1)
	{
	    if((r = p->refs[0].ref) == NULL) {
		return 0;
	    }
	    /* find the primary table */
	    for(i = 0; i < p->num_tables; i++) {
		if(p->tables[i]->ref != NULL && !strcmp(p->tables[i]->ref, r))
		    break;
	    }
	    if(i == p->num_tables) {
		return 0;
	    }
	    i_primary = i;
	}
	else if(p->num_ref > 1) {
	    return 0;
	}

	/* put the primary table first in tables[]
	 */
	primary_table = p->tables[i_primary];
	for(i = i_primary; i > 0; i--) {
	    p->tables[i] = p->tables[i-1];
	}
	p->tables[0] = primary_table;

	return 1;
}

static int
get_selection(char *c, char **ref, char **member)
{
	char *p = NULL;

	/* "select distinct r.member" */

	if(c[0] == '\'') { /* string constant */
	    int n = strlen(c);
	    *ref = NULL;
	    *member = c;
	    if(c[n-1] == '\'') c[n-1] = '\0';
	    return 0;	/* can't have multiple string constants */
	}
	else if((p = strstr(c, ".")) != NULL) {
	    *p = '\0';
	    *ref = c;
	    *member = p+1;
	    return check_more(p+1);
	}
	else {
	    *ref = NULL;
	    *member = c;
	    return check_more(c);
	}
}

static int
check_more(char *p)
{
	if(p[(int)strlen(p)-1] == ',') {
	    p[(int)strlen(p)-1] = '\0';
	    return 1;
	}
	return 0;
}
