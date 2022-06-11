/*
 *  Copyright 1990 Science Applications International Corporation.
 */

#include        <inttypes.h>

/*
 *  hash.h include file.
 */
 
/*
 * Constants
 */

#define SegmentSize		256
#define SegmentSizeShift	8	/* log2(SegmentSize) */
#define DirectorySize		256
#define DirectorySizeShift	8	/* log2(DirectorySize) */
#define Prime1			37
#define Prime2			1048583
#define DefaultMaxLoadFactor	5

/*
 * local data templates
 */

typedef struct element
{
	char           *item;
	struct element *Next;		/* secret from user	 */
} Element, *Segment;

typedef struct
{
	int		(*compare)();	/* comparison routine */
	uintmax_t	(*hash)();	/* return hash value */
        uintmax_t    	HashAccesses;
	uintmax_t	HashCollisions;
	int		curSegDir;	/* current segment index */
	int		curSegment;	/* current segment (first&next) */
	Element		*curElement;	/* current element */
	int             p;		/* Next bucket to be split */
	int             maxp;		/* upper bound on p during expansion */
        uintmax_t       KeyCount;	/* current # keys */
	int             SegmentCount;	/* current # segments */
	int             MinLoadFactor;
	int             MaxLoadFactor;
	Segment        *Directory[DirectorySize];
} HashTable;

/*
 * Entry points
 */

#ifdef __STDC__

static uintmax_t	DefaultHash (char *key);
static int		DefaultCompare (char *k1, char *k2);
static uintmax_t	Hash (HashTable *table, char *key);
static void		ExpandTable (HashTable *table);
static HashTable	*hash_create (int (*compare)(),
				      uintmax_t (*hash)(),
				      unsigned int count);
static void		hash_destroy (char *table);
static int		hash_count (char *table);
static int		hash_add (char *datum, char *table,
				  int (*compar)());
static int		hash_append (char *datum, char *table,
				     int (*compar)());
static int		hash_prepend (char *datum, char *table,
				      int (*compar)());
static char		*hash_find (char *datum, char *table,
				    int (*compar)());
static int		hash_delete (char *item ,char *table,
				     int (*compar)(), void (*release)());
static void		hash_first (char *table);
static char		*hash_next (char *table);
static char		*hash_search (char *item, HashTable *table, int add);

#else /* !__STDC__ */

static uintmax_t	DefaultHash ();
static int		DefaultCompare ();
static uintmax_t	Hash ();
static void		ExpandTable ();
static HashTable	*hash_create ();
static void		hash_destroy ();
static int		hash_count ();
static int		hash_add ();
static int		hash_append ();
static int		hash_prepend ();
static char		*hash_find ();
static int		hash_delete ();
static void		hash_first ();
static char		*hash_next ();
static char		*hash_search ();
#endif /* __STDC__ */

