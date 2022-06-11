%{
/*      SCMS C Code SCCS Header      */
#ifndef lint
static char *sccsid1[] =
{
	"@(#)SCCSID: stdtime_expr_time.lex.l 82.1",
	"@(#)SCCSID: Version Created: 08/27/98 12:26:43"
};
#endif

%}

%x DATE TIME DAYMONTH BRACKET COMMA EPOCH

nl      [\n]
any     .*

time	([0-2])?[0-9]([ \t]*:[ \t]*([0-5])?[0-9]([ \t]*:[ \t]*([0-6])?[0-9](\.([0-9]+)?)?)?)?
year    [0-9][0-9][0-9][0-9]
gmt     [0-9][0-9][0-9][0-9][0-3][0-9][0-9][0-2][0-9][0-6][0-9][0-6][0-9]
day	([0-3])?[0-9]
mon	([0-1])?[0-9]
jdate   [0-9][0-9][0-9][0-9][0-3][0-9][0-9]
ldate   [0-9][0-9][0-9][0-9][0-1][0-9][0-3][0-9]
timezone [ABCEGHJMNPWabceghjmnpw][DEMSdems][Tt]

ws      [ \t\n]*
wsnn    [ \t]*

%%

 /* These are tokens that are treated by the parser as NUM_SYM tokens */

"now"	    yylval.t = now;        return NUM_SYM;
"today"	    yylval.t = today;      return NUM_SYM;
"yesterday" yylval.t = yesterday;  return NUM_SYM;
"tomorrow"  yylval.t = tomorrow;   return NUM_SYM;
"minute"    yylval.t = ONE_MINUTE; return NUM_SYM;
"hour"      yylval.t = ONE_HOUR;   return NUM_SYM;
"day"       yylval.t = ONE_DAY;    return NUM_SYM;
"week"	    yylval.t = ONE_WEEK;   return NUM_SYM;

 /* These are math operators that the parser knows about */

"+"	    yylval.op = ADDOP;  return TERMOPSYM;
"-"	    yylval.op = SUBOP;  return TERMOPSYM;
"*"|"_"	    yylval.op = MULOP;  return FACTOROPSYM;
"/"	    yylval.op = DIVOP;  return FACTOROPSYM;
"%"	    yylval.op = MODOP;  return FACTOROPSYM;
"@"	    yylval.op = SNAPOP; return FACTOROPSYM;
"("	                        return LPARENSYM;
")"	                        return RPARENSYM;

 /*
  * These are the days of the week. They are specified such that
  * the case is unimportant and they can be given as 3 letter or full
  * spelling.
  *
  * These rules are only active in the BRACKET state.
  */
<BRACKET>{
[Mm][Oo][Nn]([Dd][Aa][Yy])?		yylval.d = 0; return ADAY_SYM;
[Tt][Uu][Ee]([Ss][Dd][Aa][Yy])?		yylval.d = 1; return ADAY_SYM;
[Ww][Ee][Dd]([Nn][Ee][Ss][Dd][Aa][Yy])? yylval.d = 2; return ADAY_SYM;
[Tt][Hh][Uu]([Rr][Ss][Dd][Aa][Yy])?	yylval.d = 3; return ADAY_SYM;
[Ff][Rr][Ii]([Dd][Aa][Yy])?  		yylval.d = 4; return ADAY_SYM;
[Ss][Aa][Tt]([Uu][Rr][Dd][Aa][Yy])?	yylval.d = 5; return ADAY_SYM;
[Ss][Uu][Nn]([Dd][Aa][Yy])?		yylval.d = 6; return ADAY_SYM;
}

 /*
  * These are months of the year. They are specified such that
  * the case is unimportant and they can be given as 3 letter or full
  * spelling.
  *
  * These rules are only active in the BRACKET and DAYMONTH state.
  */
<BRACKET,DAYMONTH>{
[Jj][Aa][Nn]([Uu][Aa][Rr][Yy])?		yylval.d = 1; return AMONTH_SYM;
[Ff][Ee][Bb]([Rr][Uu][Aa][Rr][Yy])?	yylval.d = 2; return AMONTH_SYM;
[Mm][Aa][Rr]([Cc][Hh])?			yylval.d = 3; return AMONTH_SYM;
[Aa][Pp][Rr]([Ii][Ll])?			yylval.d = 4; return AMONTH_SYM;
[Mm][Aa][Yy]				yylval.d = 5; return AMONTH_SYM;
[Jj][Uu][Nn]([Ee])?			yylval.d = 6; return AMONTH_SYM;
[Jj][Uu][Ll]([Yy])?			yylval.d = 7; return AMONTH_SYM;
[Aa][Uu][Gg]([Uu][Ss][Tt])?		yylval.d = 8; return AMONTH_SYM;
[Ss][Ee][Pp]([Tt][Ee][Mm][Bb][Ee][Rr])? yylval.d = 9; return AMONTH_SYM;
[Oo][Cc][Tt]([Oo][Bb][Ee][Rr])?		yylval.d = 10; return AMONTH_SYM;
[Nn][Oo][Vv]([Ee][Mm][Bb][Ee][Rr])?	yylval.d = 11; return AMONTH_SYM;
[Dd][Ee][Cc]([Ee][Mm][Bb][Ee][Rr])?	yylval.d = 12; return AMONTH_SYM;
}

 /*
  * Matches the left bracket '['. This rule is active both in the INITIAL
  * (starting) state and in the BRACKET state. When this token is matched
  * the lexer is placed in the BRACKET state.
  */
<INITIAL,BRACKET>"[" {
    BEGIN(BRACKET);
    return LBRACKET_SYM;
 }

 /*
  * Matches the right bracket ']'. This rule is active in the BRACKET, COMMA,
  * DATE and TIME states. These states can only be reached while in the
  * BRACKET state. When this token is matched the lexer state is placed
  * back in the INITIAL state.
  */
<BRACKET,COMMA,DATE,TIME>"]" {
    BEGIN(INITIAL);
    return RBRACKET_SYM;
 }

 /*
  * Matches the comma ','. This rule is active only in the BRACKET state.
  * It is neccessary to have a COMMA state so that the time format rule
  * can be restricted to only be active when following a comma.
  * This is neccesary to ensure that the time format rule does not swallow
  * input that would otherwise match another rule.
  */
  /*
 <BRACKET>"," {
    BEGIN(COMMA);
    return COMMA_SYM;
 }
  */

 /*
  * Matches a time string followed by a comma ',' but only if
  * it is followed by a string beginning with a percent (%) format character.  
  * This rule is only active in the BRACKET state and places the lexer
  * in the COMMA state.
  * It is neccessary to have a COMMA state so that the time format rule
  * can be restricted to only be active when following a comma.
  * This is neccesary to ensure that the time format rule does not swallow
  * input that would otherwise match another rule.
  */
<BRACKET>[^,]+,/[ \t]*% {
    yylval.s = strdup(yytext);
    BEGIN(COMMA);
    return TIME_STR_SYM;
 }

 /*
  * Matches a time format string, but only if the lexer is in the COMMA state.
  */
<COMMA>[^]]+ {
    yylval.s = strdup(yytext);
    return TIME_FMT_SYM;
 }

 /*
  * Matches an epoch number. The number must have a decimal. No numbers
  * are required after the decimal but the decimal is required in order
  * to distinguish it from other numbers that would otherwise be considered
  * for date rules. Such as, YYYYJJJ and YYYYMMDD.
  * This rule is only active in the BRACKET state and places the lexer in
  * the EPOCH state.
  */
<BRACKET>[0-9]+\.[0-9]* {
    yylval.t = atof(yytext);

    BEGIN(EPOCH);

    return EPOCH_SYM;
 }

 /*
  * This is a special rule that matches all text following an epoch time
  * up to and including the right bracket ']'. This allows the parser to
  * accept time formats that have the epoch at the beginning and a time
  * format after it.
  */
<EPOCH>.*"]" {
    BEGIN(INITIAL);
    return RBRACKET_SYM;
 }

 /*
  * Matches any number. This rule is separate from the above rule so that
  * numbers without a decimal may be used outside brackets '[]'.
  */
[0-9]+(\.[0-9]*)? {
    yylval.t = atof(yytext);
    return NUM_SYM;
 }

 /*
  * Matches a two digit day of month but only if it precedes spaces then
  * a alpha token (such as a month name).
  * This rule is only active in the BRACKET state and will place the lexer in
  * the DAYMONTH state so that the month name rules can be activated.
  */
<BRACKET>{day}/[ \t]+[A-Za-z]+ {
    yylval.d = atol(yytext);

    BEGIN(DAYMONTH);

    return DAY_SYM;
 }

 /*
  * Matches a two digit day of month but only if it precedes a TIME_SYM token.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state.
  */
<BRACKET>{day}/[ \t]+{time} {
    yylval.d = atol(yytext);

    BEGIN(DATE);

    return DAY_SYM;
 }

 /*
  * Matches a two digit day of month and optional comma (,) but only if
  * it precedes a YEAR_SYM token.  This rule is only active in the BRACKET 
  * state. 
  */
<BRACKET>{day}[ \t]*(,)?[ \t]+/{year} {
    yylval.d = atol(yytext);

    return DAY_SYM;
 }


 /*
  * Matches a 4 digit year but only if the lexer is in the BRACKET, DAYMONTH
  * or TIME state.   This means that this rule will match a 4 digit string
  * if it is part of a "DD Month YYYY" or "Dayofweek Month DD HH:MM:SS YYYY"
  * formated time string.
  * If the lexer is in the DAYMONTH state then this rule places the lexer
  * in the DATE state.
  */
<BRACKET,DAYMONTH,TIME>{year} {
    yylval.d = atol(yytext);

    if (YYSTATE == DAYMONTH)
        BEGIN(DATE);

    return YEAR_SYM;
 }

 /*
  * Matches a GMT format time string (YYYYJJJHHMMSS).
  * This rule is only active in the BRACKET state.
  */
<BRACKET>{gmt} {
    char buf[14];
    int	jd, hour, min, sec;

    strncpy(buf, yytext, sizeof(buf));
    sec  = atol(&buf[11]); buf[11] = '\0';
    min  = atol(&buf[9]);  buf[9] = '\0';
    hour = atol(&buf[7]);  buf[7] = '\0';
    jd = atol(&buf[0]);

    yylval.t = stdtime_jtoe(jd);
    if ((stdtime_errno == STDTIME_INVALID) 
        || (hour > 23) || (sec > 61))   /* parser picks up min > 59 */
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    yylval.t = yylval.t + hour * 3600 + min * 60 + sec;

    return GMT_SYM;
 }

 /*
  * Matches a date string of the format YYYY-MM-DD.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state.
  */
<BRACKET>{year}{wsnn}"-"{wsnn}{mon}{wsnn}"-"{wsnn}{day} {
    char *ptr;
    int year, mon, day = 0;

    year = atol(yytext);

    if ((ptr = strchr(yytext, '-')) != NULL)
    {
	mon = atol(++ptr);
        if ((ptr = strchr(ptr, '-')) != NULL)
	    day = atol(++ptr);
    }

    yylval.t = date2epoch(year, mon, day);

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }


 /*
  * Matches a date string of the format YYYY/MM/DD.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state.
 */
<BRACKET>{year}{wsnn}"/"{wsnn}{mon}{wsnn}"/"{wsnn}{day} {
    char *ptr;
    int year, mon, day = 0;

    year = atol(yytext);

    if ((ptr = strchr(yytext, '/')) != NULL)
    {
	mon = atol(++ptr);
        if ((ptr = strchr(ptr, '/')) != NULL)
	    day = atol(++ptr);
    }

    yylval.t = date2epoch(year, mon, day);

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }


 /*
  * Matches a date string of the format MM/DD/YYYY.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state.
  */
<BRACKET>{mon}{wsnn}"/"{wsnn}{day}{wsnn}"/"{wsnn}{year} {
    char *ptr;
    int year, mon, day = 0;

    mon = atol(yytext);

    if ((ptr = strchr(yytext, '/')) != NULL)
    {
	day = atol(++ptr);
        if ((ptr = strchr(ptr, '/')) != NULL)
	    year = atol(++ptr);
    }

    yylval.t = date2epoch(year, mon, day);

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }


 /*
  * Matches a date string of the format DD.MM.YYYY.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state.
  */
<BRACKET>{day}{wsnn}"."{wsnn}{mon}{wsnn}"."{wsnn}{year} {
    char *ptr;
    int year, mon, day = 0;

    day = atol(yytext);

    if ((ptr = strchr(yytext, '.')) != NULL)
    {
	mon = atol(++ptr);
        if ((ptr = strchr(ptr, '.')) != NULL)
	    year = atol(++ptr);
    }

    yylval.t = date2epoch(year, mon, day);

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }


 /*
  * Matches a string of the format HH[:MM[:SS[.N+]]] where the minutes,
  * seconds, and fractions of seconds are optional. The fraction can have
  * any number of digits in it.
  * This rule is only active in the BRACKET and DATE states and places
  * the lexer in the TIME state.
  */
<BRACKET,DATE>{time} {
    char	*ptr;
    int		hour = 0;
    int		min = 0;
    double	sec = 0;

    hour = atol(yytext);

    if ((ptr = strchr(yytext, ':')) != NULL)
    {
	min = atol(++ptr);
        if ((ptr = strchr(ptr, ':')) != NULL)
	    sec = atof(++ptr);
    }

    if ((hour > 23) || (sec > 61))   /* parser picks up min > 59 */
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    yylval.t = hour * 3600 + min * 60 + sec;

    BEGIN(TIME);

    return TIME_SYM;
 }

 /*
  * Matches a date string of the format YYYYJJJ.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state
  */
<BRACKET>{jdate} {
    yylval.t = stdtime_jtoe(atol(yytext));

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }

 /*
  * Matches a date string of the format YYYYMMDD.
  * This rule is only active in the BRACKET state and places the lexer
  * in the DATE state
  */
<BRACKET>{ldate} {
    yylval.t = ldate2epoch(atol(yytext));

    if (stdtime_errno == STDTIME_INVALID) 
    {
	BEGIN(INITIAL);
	yyterminate();
    }

    BEGIN(DATE);

    return DATE_SYM;
 }

 /*
  * Matches a time zone string. This rule is only active in the
  * BRACKET and TIME states.
  *
  * NOTE: The parser currently does nothing with this and this rule
  * does not provide a proper time offset.
  */
<BRACKET,TIME>{timezone} {
    yylval.d = 0;
    return TIMEZONE_SYM;
 }

<INITIAL,BRACKET,DAYMONTH,COMMA,DATE,TIME>{ws}     /* Do nothing with white space */

<INITIAL,BRACKET,DAYMONTH,COMMA,DATE,TIME>. {
	/*
	 * If we get here then we ran into an unexpected character.
	 * If we do nothing the the lexer will swallow the unknown
	 * characters and will result in a possible partially correct
	 * answer even though the time string was improperly formatted.
	 * So, instead we return -1 which is an unknown token as far
	 * as the parser is concerned. This causes the parser to bail
	 * and the end result is a proper error code being returned
	 * instead of a partially correct answer
	 */
	BEGIN(INITIAL);
	yyterminate();
 }
%%

 /*
  * Causes the lexer to exit when it hits EOF.
  */
int
yywrap()

{
    return 1;
}

int
yyerror(s)
    char        *s;
{
    /* don't print anything since the output is basically useless */
    /* printf("%s\n", s); */
    return 0;
}
