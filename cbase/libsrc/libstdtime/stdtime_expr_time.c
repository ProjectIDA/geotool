/* A Bison parser, made by GNU Bison 1.875c.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse stdtime_expr_time_parse
#define yylex   stdtime_expr_time_lex
#define yyerror stdtime_expr_time_error
#define yylval  stdtime_expr_time_lval
#define yychar  stdtime_expr_time_char
#define yydebug stdtime_expr_time_debug
#define yynerrs stdtime_expr_time_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     EPOCH_SYM = 258,
     NUM_SYM = 259,
     DATE_SYM = 260,
     TIME_SYM = 261,
     GMT_SYM = 262,
     YEAR_SYM = 263,
     DAY_SYM = 264,
     ADAY_SYM = 265,
     AMONTH_SYM = 266,
     JDATE_SYM = 267,
     LDATE_SYM = 268,
     TIMEZONE_SYM = 269,
     LBRACKET_SYM = 270,
     RBRACKET_SYM = 271,
     COMMA_SYM = 272,
     TIME_STR_SYM = 273,
     TIME_FMT_SYM = 274,
     LPARENSYM = 275,
     RPARENSYM = 276,
     TERMOPSYM = 277,
     FACTOROPSYM = 278
   };
#endif
#define EPOCH_SYM 258
#define NUM_SYM 259
#define DATE_SYM 260
#define TIME_SYM 261
#define GMT_SYM 262
#define YEAR_SYM 263
#define DAY_SYM 264
#define ADAY_SYM 265
#define AMONTH_SYM 266
#define JDATE_SYM 267
#define LDATE_SYM 268
#define TIMEZONE_SYM 269
#define LBRACKET_SYM 270
#define RBRACKET_SYM 271
#define COMMA_SYM 272
#define TIME_STR_SYM 273
#define TIME_FMT_SYM 274
#define LPARENSYM 275
#define RPARENSYM 276
#define TERMOPSYM 277
#define FACTOROPSYM 278




/* Copy the first part of user declarations.  */


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef union YYSTYPE {
		int	op;
                int     d;
		char	*s;
		double	t;
	} YYSTYPE;
/* Line 191 of yacc.c.  */
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


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



/* Line 214 of yacc.c.  */

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (int) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   48

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  24
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  6
/* YYNRULES -- Number of rules. */
#define YYNRULES  25
/* YYNRULES -- Number of states. */
#define YYNSTATES  46

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   278

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    13,    16,    18,    22,
      24,    28,    32,    34,    36,    38,    40,    44,    47,    52,
      56,    61,    66,    72,    78,    85
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      25,     0,    -1,    26,    -1,     1,    -1,    27,    -1,    26,
      22,    26,    -1,    22,    27,    -1,    28,    -1,    27,    23,
      28,    -1,     4,    -1,    15,    29,    16,    -1,    20,    26,
      21,    -1,     3,    -1,     5,    -1,     6,    -1,     7,    -1,
       9,    11,     8,    -1,     5,     6,    -1,     9,    11,     8,
       6,    -1,    11,     9,     8,    -1,    11,     9,     8,     6,
      -1,    11,     9,     6,     8,    -1,    10,    11,     9,     6,
       8,    -1,    11,     9,     6,    14,     8,    -1,    10,    11,
       9,     6,    14,     8,    -1,    18,    19,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    82,    82,    86,    93,    97,   104,   113,   117,   136,
     140,   144,   150,   154,   158,   162,   166,   170,   174,   178,
     182,   186,   190,   194,   198,   202
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EPOCH_SYM", "NUM_SYM", "DATE_SYM",
  "TIME_SYM", "GMT_SYM", "YEAR_SYM", "DAY_SYM", "ADAY_SYM", "AMONTH_SYM",
  "JDATE_SYM", "LDATE_SYM", "TIMEZONE_SYM", "LBRACKET_SYM", "RBRACKET_SYM",
  "COMMA_SYM", "TIME_STR_SYM", "TIME_FMT_SYM", "LPARENSYM", "RPARENSYM",
  "TERMOPSYM", "FACTOROPSYM", "$accept", "statement", "expr", "term",
  "factor", "datetime", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    24,    25,    25,    26,    26,    26,    27,    27,    28,
      28,    28,    29,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    29,    29,    29,    29
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     3,     2,     1,     3,     1,
       3,     3,     1,     1,     1,     1,     3,     2,     4,     3,
       4,     4,     5,     5,     6,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     3,     9,     0,     0,     0,     0,     2,     4,     7,
      12,    13,    14,    15,     0,     0,     0,     0,     0,     0,
       6,     1,     0,     0,    17,     0,     0,     0,    25,    10,
      11,     5,     8,    16,     0,     0,    19,    18,     0,    21,
       0,    20,    22,     0,    23,    24
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     6,     7,     8,     9,    18
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -17
static const yysigned_char yypact[] =
{
      -1,   -17,   -17,    20,    -2,    -3,     7,   -14,   -12,   -17,
     -17,    18,   -17,   -17,    17,    22,    27,    16,    21,   -16,
     -12,   -17,    -2,    -3,   -17,    31,    32,    26,   -17,   -17,
     -17,   -17,   -17,    34,    36,     1,    37,   -17,     2,   -17,
      38,   -17,   -17,    39,   -17,   -17
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -17,   -17,     0,    40,    25,   -17
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       1,     2,     2,     2,    19,    30,    22,    21,    22,    39,
      42,    23,     3,     3,     3,    40,    43,     4,     4,     4,
       5,     5,    31,    10,    24,    11,    12,    13,    25,    14,
      15,    16,    35,    26,    36,    28,    27,    29,    17,    33,
      37,    34,    38,    41,     0,    20,    44,    45,    32
};

static const yysigned_char yycheck[] =
{
       1,     4,     4,     4,     4,    21,    22,     0,    22,     8,
       8,    23,    15,    15,    15,    14,    14,    20,    20,    20,
      22,    22,    22,     3,     6,     5,     6,     7,    11,     9,
      10,    11,     6,    11,     8,    19,     9,    16,    18,     8,
       6,     9,     6,     6,    -1,     5,     8,     8,    23
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     1,     4,    15,    20,    22,    25,    26,    27,    28,
       3,     5,     6,     7,     9,    10,    11,    18,    29,    26,
      27,     0,    22,    23,     6,    11,    11,     9,    19,    16,
      21,    26,    28,     8,     9,     6,     8,     6,     6,     8,
      14,     6,     8,    14,     8,     8
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (int *bottom, int *top)
#else
static void
yy_stack_print (bottom, top)
    int *bottom;
    int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}

int yyparse (void);
int yylex(void);
int yyerror(char *s);

/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  int	yyssa[YYINITDEPTH];
  int *yyss = yyssa;
  register int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;


#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	int *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	int *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %u\n",
		  (unsigned int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
    {
	epoch_time = yyvsp[0].t;
    ;}
    break;

  case 3:
    {
	kill_lexer();
	YYABORT;
    ;}
    break;

  case 4:
    {
	yyval.t = yyvsp[0].t;
    ;}
    break;

  case 5:
    {
	if (yyvsp[-1].op == ADDOP)
	    yyval.t = yyvsp[-2].t + yyvsp[0].t;
	else if (yyvsp[-1].op == SUBOP)
	    yyval.t = yyvsp[-2].t - yyvsp[0].t;
    ;}
    break;

  case 6:
    {
	if ( yyvsp[-1].op == SUBOP )
	     yyval.t = 0 - yyvsp[0].t;
	else if (yyvsp[-1].op == ADDOP)
	     yyval.t = yyvsp[0].t;
    ;}
    break;

  case 7:
    {
	yyval.t = yyvsp[0].t;
    ;}
    break;

  case 8:
    {
	double a,b,c;

	if (yyvsp[-1].op == MULOP)
	    yyval.t = yyvsp[-2].t * yyvsp[0].t;
	else if (yyvsp[-1].op == DIVOP)
	    yyval.t = yyvsp[-2].t / yyvsp[0].t;
	else if (yyvsp[-1].op == MODOP) {
	    a = floor(yyvsp[-2].t / yyvsp[0].t);
	    b = a*yyvsp[0].t;
	    c = yyvsp[-2].t - b;
	    yyval.t = c;
	}
	else if (yyvsp[-1].op == SNAPOP)
	    yyval.t = floor(yyvsp[-2].t / yyvsp[0].t) * yyvsp[0].t;
    ;}
    break;

  case 9:
    {
	yyval.t = yyvsp[0].t;
    ;}
    break;

  case 10:
    {
	yyval.t = yyvsp[-1].t;
    ;}
    break;

  case 11:
    {
	yyval.t = yyvsp[-1].t;
    ;}
    break;

  case 12:
    {
		yyval.t = yyvsp[0].t;
	    ;}
    break;

  case 13:
    {
		yyval.t = yyvsp[0].t;
	    ;}
    break;

  case 14:
    {
		yyval.t = yyvsp[0].t;
	    ;}
    break;

  case 15:
    {
		yyval.t = yyvsp[0].t;
	    ;}
    break;

  case 16:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-1].d,yyvsp[-2].d);
	    ;}
    break;

  case 17:
    {
		yyval.t = yyvsp[-1].t + yyvsp[0].t;
	    ;}
    break;

  case 18:
    {
		yyval.t = date2epoch(yyvsp[-1].d,yyvsp[-2].d,yyvsp[-3].d) + yyvsp[0].t;
	    ;}
    break;

  case 19:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-2].d,yyvsp[-1].d);
	    ;}
    break;

  case 20:
    {
		yyval.t = date2epoch(yyvsp[-1].d,yyvsp[-3].d,yyvsp[-2].d) + yyvsp[0].t;
	    ;}
    break;

  case 21:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-3].d,yyvsp[-2].d) + yyvsp[-1].t;
	    ;}
    break;

  case 22:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-3].d,yyvsp[-2].d) + yyvsp[-1].t;
	    ;}
    break;

  case 23:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-4].d,yyvsp[-3].d) + yyvsp[-2].t + yyvsp[-1].d;
	    ;}
    break;

  case 24:
    {
		yyval.t = date2epoch(yyvsp[0].d,yyvsp[-4].d,yyvsp[-3].d) + yyvsp[-2].t + yyvsp[-1].d;
	    ;}
    break;

  case 25:
    {
		yyval.t = stdtime_unformat(yyvsp[-1].s, yyvsp[0].s);
		free(yyvsp[-1].s);
		free(yyvsp[0].s);
		if (yyval.t == STDTIME_DOUBLE_ERR &&
		    stdtime_errno != STDTIME_NOERR)
		{
		    kill_lexer();
		    YYABORT;
		}
	    ;}
    break;


    }

/* Line 1000 of yacc.c.  */
  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
	  yychar = YYEMPTY;

	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


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
	    printf("Parse failed\n");
	else
	    printf("%s %f", expr, epoch_time);
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



