
%union	{
		int	op;
                int     d;
		char	*s;
		double	t;
	}

%{

#include <string.h>
#include <math.h>

#include "libstdtimeP.h"

#define YYDEBUG 0

#define ADDOP	1
#define SUBOP	2
#define MULOP	3
#define DIVOP	4  	
#define MODOP	5
#define SNAPOP	6

#define ONE_MINUTE 	60.0
#define ONE_HOUR	3600.0
#define ONE_DAY 	86400.0
#define ONE_WEEK 	604800.0
#define ONE_YEAR 	31536000.0

/* Declare these functions here so that the lexer code can use them */
static double ldate2epoch(int ldate);
static double date2epoch(int year, int month, int day);
static void   kill_lexer(void);

static double	now;
static double	today;
static double	tomorrow;
static double	yesterday;


static double	epoch_time;

int		TimeDebug;

%}

%token	<t>	EPOCH_SYM
%token	<t>	NUM_SYM
%token	<t>	DATE_SYM
%token	<t>	TIME_SYM
%token	<t>	GMT_SYM
%token	<d>	YEAR_SYM
%token	<d>	DAY_SYM
%token	<d>	ADAY_SYM
%token	<d>	AMONTH_SYM
%token	<t>	JDATE_SYM
%token	<t>	LDATE_SYM
%token	<d>	TIMEZONE_SYM
%token		LBRACKET_SYM
%token		RBRACKET_SYM
%token		COMMA_SYM
%token	<s>	TIME_STR_SYM
%token	<s>	TIME_FMT_SYM
%token		LPARENSYM
%token		RPARENSYM
%token	<op>	TERMOPSYM
%token	<op>	FACTOROPSYM

%left		TERMOPSYM
%left		FACTOROPSYM

%type	<t>	statement
%type	<t>	expr
%type	<t>	term
%type	<t>	factor
%type   <t>     datetime

%%


statement : expr
    {
	epoch_time = $1;
    }
| error
    {
	kill_lexer();
	YYABORT;
    }
;

expr: term
    {
	$$ = $1;
    }
| expr TERMOPSYM expr
    {
	if ($2 == ADDOP)
	    $$ = $1 + $3;
	else if ($2 == SUBOP)
	    $$ = $1 - $3;
    }
| TERMOPSYM term
    {
	if ( $1 == SUBOP )
	     $$ = 0 - $2;
	else if ($1 == ADDOP)
	     $$ = $2;
    }
;

term: factor
    {
	$$ = $1;
    }
| term FACTOROPSYM factor
    {
	double a,b,c;

	if ($2 == MULOP)
	    $$ = $1 * $3;
	else if ($2 == DIVOP)
	    $$ = $1 / $3;
	else if ($2 == MODOP) {
	    a = floor($1 / $3);
	    b = a*$3;
	    c = $1 - b;
	    $$ = c;
	}
	else if ($2 == SNAPOP)
	    $$ = floor($1 / $3) * $3;
    }
;

factor: NUM_SYM
    {
	$$ = $1;
    }
| LBRACKET_SYM datetime RBRACKET_SYM
    {
	$$ = $2;
    }
| LPARENSYM expr RPARENSYM
    {
	$$ = $2;
    }
;

datetime: EPOCH_SYM
            {
		$$ = $1;
	    }
	| DATE_SYM
            {
		$$ = $1;
	    }
	| TIME_SYM
            {
		$$ = $1;
	    }
	| GMT_SYM
            {
		$$ = $1;
	    }
	| DAY_SYM AMONTH_SYM YEAR_SYM
            {
		$$ = date2epoch($3,$2,$1);
	    }
	| DATE_SYM TIME_SYM
            {
		$$ = $1 + $2;
	    }
	| DAY_SYM AMONTH_SYM YEAR_SYM TIME_SYM
            {
		$$ = date2epoch($3,$2,$1) + $4;
	    }
	| AMONTH_SYM DAY_SYM YEAR_SYM
            {
		$$ = date2epoch($3,$1,$2);
	    }
	| AMONTH_SYM DAY_SYM YEAR_SYM TIME_SYM
            {
		$$ = date2epoch($3,$1,$2) + $4;
	    }
	| AMONTH_SYM DAY_SYM TIME_SYM YEAR_SYM
            {
		$$ = date2epoch($4,$1,$2) + $3;
	    }
	| ADAY_SYM AMONTH_SYM DAY_SYM TIME_SYM YEAR_SYM
            {
		$$ = date2epoch($5,$2,$3) + $4;
	    }
	| AMONTH_SYM DAY_SYM TIME_SYM TIMEZONE_SYM YEAR_SYM
            {
		$$ = date2epoch($5,$1,$2) + $3 + $4;
	    }
	| ADAY_SYM AMONTH_SYM DAY_SYM TIME_SYM TIMEZONE_SYM YEAR_SYM
            {
		$$ = date2epoch($6,$2,$3) + $4 + $5;
	    }
	| TIME_STR_SYM TIME_FMT_SYM
            {
		$$ = stdtime_unformat($1, $2);
		free($1);
		free($2);
		if ($$ == STDTIME_DOUBLE_ERR &&
		    stdtime_errno != STDTIME_NOERR)
		{
		    kill_lexer();
		    YYABORT;
		}
	    }
;


%%
static char	*TimeExpr;

#define YYLMAX      1024

#define YY_INPUT(buf,result,max_size) \
		result = (buf[0] = *TimeExpr++) ? 1 : 0

#include "stdtime_expr_time_lex.c"

static void
kill_lexer(void)
{
    BEGIN(INITIAL);
    YY_FLUSH_BUFFER;
}

/*
 * Converts year, month and day into epoch.
 * Derived from libtime.a code.
 */
static double 
date2epoch(int year, int mon, int day)
{
        int i,dim, doy;

	stdtime_errno = STDTIME_NOERR;

	if (check_year_mon_day(year, mon, day) != STDTIME_NOERR)
	{
		stdtime_errno = STDTIME_INVALID;
		return STDTIME_DOUBLE_ERR;
	}
	
        doy = 0;
        for( i = 0 ; i < mon - 1 ; i++ ){
                dim = stdtime_dim[i];
                if( i == 1 && stdtime_isleap(year) ) dim++;
                doy += dim;
        }
        doy += day;
        return(stdtime_jtoe(1000 * year + doy));
}


/*
 * Converts a lddate type date (YYYYMMDD) into epoch.
 */
static double
ldate2epoch(int ldate)
{
    int	year, mon, day;

    day = ldate % 100; ldate -= day;
    mon = (ldate % 10000)/100; ldate -= mon;
    year = ldate / 10000;

    return(date2epoch(year, mon, day));
}

/*
 * Parses time strings and time math operators.
 * Returns -1 and sets stderrno to STDTIME_PARSE if the parser
 * encounters any problems.
 */
epoch_t
stdtime_expr_time(expr)

    char	*expr;

{
    int		err;
    epoch_t	ans;

#if YYDEBUG != 0
    yydebug = 1;
#endif

    stdtime_errno = STDTIME_NOERR;

    /*
     * Get the epoch for now and update the today yesterday
     * and tomorrow values.
     */
    now = stdtime_get_epoch ();
    today = floor(now / ONE_DAY) * ONE_DAY;
    yesterday = today - ONE_DAY;
    tomorrow = today + ONE_DAY;

    /* Set TimeExpr equal to expr */
    TimeExpr = expr;

    /* Run the parser */
    err = stdtime_expr_time_parse();

    /* Print some diagnostics if the TimeDebug flag is true */
    if (TimeDebug) {
	if (err)
	    printf("%s %lf", expr, epoch_time);
	else
	    printf("Parse failed\n");
    }

    /* Set stdtime_errno and return an error if the parser failed */
    if (err)
    {
	stdtime_errno = STDTIME_PARSE;
        ans = STDTIME_DOUBLE_ERR;
    }
    else if (check_epoch (epoch_time) != STDTIME_NOERR)
    {
	stdtime_errno = STDTIME_PARSE;
	ans = STDTIME_DOUBLE_ERR;
    }
    else
	ans =  epoch_time;

    return ans;
}


/*
 * Parses time strings and fills in a calendar structure.  
 * Returns NULL and sets stderrno to STDTIME_PARSE if the parser
 * encounters any problems.
 *
 * For now this function calls stdtime_expr_time() and converts the
 * resulting epoch into a calendar_t structure.  But this limits 
 * the function to the range of an epoch_t.  Later the parser should
 * be changed to fill in a calendar structure directly and the
 * function stdtime_expr_time() should be a shell function calling
 * stdtime_expr_time_c().
 * 
 */
calendar_t *
stdtime_expr_time_c(expr, calendar)
char	     *expr;
calendar_t   *calendar;
{
     epoch_t    epoch;
     
     stdtime_errno = STDTIME_NOERR;

     if (calendar == (calendar_t *) NULL)
     {
	  stdtime_errno = STDTIME_INVALID;
	  return ((calendar_t *) NULL);
     }

     epoch = stdtime_expr_time(expr);

     if (stdtime_errno != STDTIME_NOERR)
     {
	  return ((calendar_t *) NULL);
     }
     
     if (stdtime_etoc(epoch, calendar) == (calendar_t *) NULL)
     {
	  return ((calendar_t *) NULL);
     }
     
     return (calendar);
}

