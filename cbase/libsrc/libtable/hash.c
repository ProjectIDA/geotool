/*
 * --BEGIN--
 * Copyright 1992 Science Applications International Corporation
 *		  All Rights Reserved
 *
 * Author: Pete Ware, SAIC, Geophysics Division / San Diego CA / (619) 458-2520
 * --END--
 */
/*
 * Dynamic hashing, after CACM April 1988 pp 446-457, by Per-Ake Larson.
 * Coded into C, with minor code improvements, and with hsearch(3) interface,
 * by ejp@ausmelb.oz, Jul 26, 1988: 13:16;
 * also, hcreate/hdestroy routines added to simulate hsearch(3).
 *
 * These routines simulate hsearch(3) and family, with the important
 * difference that the hash table is dynamic - can grow indefinitely
 * beyond its original size (as supplied to hcreate()).
 *
 * Performance appears to be comparable to that of hsearch(3).
 * The 'source-code' options referred to in hsearch(3)'s 'man' page
 * are not implemented; otherwise functionality is identical.
 *
 * Compilation controls:
 * DEBUG controls some informative traces, mainly for debugging.
 * HASH_STATISTICS causes HashAccesses and HashCollisions to be maintained;
 * when combined with DEBUG, these are displayed by hdestroy().
 *
 * Problems & fixes to ejp@ausmelb.oz. WARNING: relies on pre-processor
 * concatenation property, in probably unnecessary code 'optimisation'.
 *
 *
 *	table_hash_create() -- Create a table with a hash structure.
 *
 *	TABLE	*
 *	table_hash_create(compar, release, destroy, hash, count)
 *	int		(*compar)();	(i) A pointer to a comparison function.
 *	void		(*release)();	(i) A pointer to a release function.
 *	void		(*destroy)();	(i) A pointer to a table destroy 
 *					    function.
 *	uintmax_t	(*hash)();	(i) A pointer to a hash function.
 *	unsigned int	count;		(i) Number of segments to preallocate.
 *
 *
 *	table_hash_stats () --  Prints out the number of table entries, 
 *			        accesses and collisions.
 *
 *	void
 *	table_hash_stats (table)
 *	TABLE	*table;
 *
 * NOTES:
 *		The add, append, prepend and delete functions have been 
 *		modified to return values consistent with the other table 
 *		routines (those for lists, etc.).  TABLE_OK is returned if 
 *		data was successfully added (note that if the data already 
 *		exists then TABLE_ERR is returned).
 *
 */

#include        <inttypes.h>
#include	<stdio.h>
#include	<assert.h>
#include	<string.h>
#include	"table.h"
#include	"hash.h"
#include	"aesir.h"


/*
 * Fast arithmetic, relying on powers of 2,
 */

#define MUL(x,y)		((x) << (y))
#define DIV(x,y)		((x) >> (y))
#define MOD(x,y)		((x) & ((y)-1))

#define HashAccess(table)	(++(table)->HashAccesses)
#define HashCollision(table)	(++(table)->HashCollisions)


TABLE	*
table_hash_create (compar, release, destroy, hash, count)
TABLE_COMPARE    compar;
TABLE_RELEASE    release;
TABLE_DESTROY    destroy;
TABLE_HASH       hash;
unsigned int     count;
{
	TABLE			*table;

	if (!compar)
		compar = (TABLE_COMPARE) default_compar;

	if ((table = UALLOC(TABLE, 1))
		!= (TABLE*) NULL)
	{
		table->add = hash_add;
		table->delet = hash_delete;
		table->find = hash_find;
		table->first = hash_first;
		table->next = hash_next;
		table->compar = compar;
		table->release = release;
		table->destroy = destroy;	/* User destroy function. */
		table->prepend = hash_prepend; 
		table->append = hash_append;
		table->privat = (char *)hash_create ( compar, 
						      hash,
						      count);
		/*
		 *  A function to destroy the internal hash structure.
		 *
		 *  It is also used to check to see if this TABLE is a 
		 *  "hash" TABLE.
		 */
		table->private_destroy = hash_destroy;

		table->count = hash_count;
	}
	return (table);
}


	/*
	 *  Print the internal hash table statistics.
	 */
void
table_hash_stats (table)
TABLE	*table;
{
	HashTable	*htable;

	/*
	 *  Check to make sure we are dealing with a hash TABLE.  We 
	 *  can check the private destroy function and confirm it matches 
	 *  what was set up in the table_hash_create() call.
	 */
	if (table && 
	    table->private_destroy == hash_destroy)
	{
		htable = (HashTable *) table->privat;
		(void) fprintf (stderr, 
				"Entries: %ld, Accesses: %ld, Collisions: %ld\n",
				(long int)htable->KeyCount,
				(long int)htable->HashAccesses,
				(long int)htable->HashCollisions);
	}
}

/*
 * Internal routines 
 */

static 
HashTable *
hash_create (compare, hash, Count)
int		(*compare)();
uintmax_t       (*hash)();
unsigned int	Count;
{
	int             i;
	HashTable	*table;

	/*
	 * Adjust Count to be nearest higher power of 2, * minimum
	 * SegmentSize, then convert into segments.
	 */
	i = SegmentSize;
	while (i < Count)
		i <<= 1;
	Count = DIV (i, SegmentSizeShift);

	table = (HashTable *) calloc (1, (sizeof (HashTable)));
	if (table == NULL)
		return (table);
	/*
	 * Resets are redundant - done by calloc(3)
	 * The initialization performed by calloc() is effectively 
	 *
	 *    Table->SegmentCount = table->p = table->KeyCount = 0;
	 */

	if (compare)
		table->compare = compare;
	else
		table->compare = DefaultCompare;
	if (hash)
		table->hash = hash;
	else
		table->hash = DefaultHash;

	/*
	 * * Allocate initial 'i' segments of buckets
	 */
	for (i = 0; i < Count; i++)
	{
		table->Directory[i] = (Segment *) calloc (SegmentSize, 
							  sizeof (Segment));
		if (table->Directory[i] == NULL)
		{
			hash_destroy ((char *) table);
			return ((HashTable *) NULL);
		}
		table->SegmentCount++;
	}
	table->maxp = MUL (Count, SegmentSizeShift);
	table->MinLoadFactor = 1;
	table->MaxLoadFactor = DefaultMaxLoadFactor;
	table->HashAccesses = 0;
	table->HashCollisions = 0;
	return (table);
}

	/*
	 *  Return the number of elements in the hash table.
	 */
static 
int
hash_count (htable)
char	*htable;
{
	return ((int)((HashTable *) htable)->KeyCount);
}


/*ARGSUSED*/
static
int
hash_add (datum, table, compar)
char		*datum;
char		*table;
int		(*compar)();
{
	if (hash_search (datum, (HashTable *) table, 1))
		return TABLE_OK;
	else
		return TABLE_ERR;
}

/*ARGSUSED*/
static 
int
hash_append (datum, table, compar)
char		*datum;
char		*table;
int		(*compar)();
{
	/* Append is the same as add for a hash table - SKT */
	if (hash_search (datum, (HashTable *) table, 1))
		return TABLE_OK;
	else
		return TABLE_ERR;
}

/*ARGSUSED*/
static 
int
hash_prepend (datum, table, compar)
char		*datum;
char		*table;
int		(*compar)();
{
	/* Prepend is the same as add for a hash table - SKT */
	if (hash_search (datum, (HashTable *) table, 1))
		return TABLE_OK;
	else
		return TABLE_ERR;
}

/*ARGSUSED*/
static 
char *
hash_find (datum, table, compar)
char		*datum;
char		*table;
int		(*compar)();
{
	return (hash_search (datum, (HashTable *)table, 0));
}

/*ARGSUSED*/
static 
int
hash_delete (item, t, compar, release)
char		*item;
char		*t;
int		(*compar)();
void		(*release)();
{
	uintmax_t       h;
	Segment        *CurrentSegment;
	int             SegmentIndex;
	int             SegmentDir;
	Segment        *prev;
	Segment		q;
	HashTable	*table = (HashTable *) t;

	if (!table)
		return TABLE_ERR;

	HashAccess (table);
	h = Hash (table, item);
	SegmentDir = DIV (h, SegmentSizeShift);
	SegmentIndex = MOD (h, SegmentSize);
	CurrentSegment = table->Directory[SegmentDir];
	prev = &CurrentSegment[SegmentIndex];
	q = *prev;

	/*
	 * Follow collision chain
	 */

	while (q != NULL)
	{
		HashCollision (table);
		if (table->compare (q->item, item) == 0)
		{
			*prev = q->Next;
			--table->KeyCount;
			if (release)
				(*release)(q->item);

			/*
			 *  Remove this element (above release function 
			 *  should know nothing about this internal 
			 *  structure so we need to free it here).
			 */
			UFREE (q);

			return TABLE_OK;
		}
		else
		{
			prev = &q->Next;
		}
		q = *prev;
	}
	return TABLE_ERR;	/* Error only because not found. */
}

static 
void
hash_first (t)
char	*t;
{
	HashTable	*table = (HashTable *) t;
	table->curSegDir = table->curSegment = 0;
	table->curElement = NULL;
}

static 
char *
hash_next (t)
char		*t;
{
	Segment        *cursegment;
	HashTable	*table = (HashTable *) t;
	Element		*ret;

	ret = NULL;
	while (!ret && table->curSegDir < table->SegmentCount)
	{
		if ((cursegment = table->Directory[table->curSegDir]) == NULL)
			break;
		/*
		 *  We want to make sure we do not exceed SegmentSize but 
		 *  that we traverse the entire chain.  Previously we 
		 *  erroniously checked only the segment size so we only 
		 *  found the first item in the chain and never traveresed 
		 *  the others.  This occurred only when table->curSegment 
		 *  equalled SegmentSize.
		 */
		while (!ret && 
		       (table->curSegment < SegmentSize || 
			(table->curElement && table->curElement->Next)))
		{
			if (table->curElement)
			{
				ret = table->curElement->Next;
			}
			else
			{
				ret = cursegment[table->curSegment++];
			}
			table->curElement = ret;
		}
		if (!ret)
		{
			table->curSegment = 0;
			++table->curSegDir;
		}
	}
	if (ret)
		return (ret->item);
	else
		return ((char *)NULL);
}


	/*
	 *  This routine deallocates the internal data structures 
	 *  used; this is irrelevant of any data being stored in 
	 *  the table by the user (i.e., the user doesn't need 
	 *  to know about this structure).
	 */
static 
void
hash_destroy (t)
char		*t;
{
	int             i, j;
	Segment        *s;
	Element        *p, *q;
	HashTable	*table = (HashTable *) t;

	if (table != NULL)
	{
		for (i = 0; i < table->SegmentCount; i++)
		{
			/* test probably unnecessary	 */
			if ((s = table->Directory[i]) != NULL)
			{
				for (j = 0; j < SegmentSize; j++)
				{
					p = s[j];
					while (p != NULL)
					{
						q = p->Next;
						UFREE (p);
						p = q;
					}
				}
				UFREE (table->Directory[i]);
			}
		}
		UFREE (table);
	}
}

static 
char *
hash_search (item, table, add)
char		*item;
HashTable	*table;
int		add;
{
	uintmax_t       h;
	Segment        *CurrentSegment;
	int             SegmentIndex;
	int             SegmentDir;
	Segment        *p, q;

	if (!table)
		return ((char *) NULL);
	HashAccess (table);
	h = Hash (table, item);
	SegmentDir = DIV (h, SegmentSizeShift);
	SegmentIndex = MOD (h, SegmentSize);

	/*
	 * valid segment ensured by Hash()
	 */
	CurrentSegment = table->Directory[SegmentDir];
	if (!CurrentSegment)
		return ((char *) NULL);

	p = &CurrentSegment[SegmentIndex];
	q = *p;

	/*
	 * Follow collision chain
	 */
	while (q != NULL && table->compare (q->item, item))
	{
		p = &q->Next;
		q = *p;
		HashCollision (table);
	}

	if (q == NULL)
	{
		if (!add)
			return ((char *) NULL);
	}
	else
	{
		if (!add)
			return (q->item);
		else
		{
			return ((char *) NULL);
		}
	}

	/* not found, no room	 */
	if ((q = (Element *) calloc (1, sizeof (Element))) == NULL)
	{
		return ((char *) NULL);
	}

	*p = q;			/* link into chain	 */

	/*
	 * Initialize new element
	 */
	q->item = item;

	/*
	 * No need to set "Next", it's cleared by calloc(3) so q->Next = NULL;
	 */

	/*
	 *  Is the table over-full? If so expand it.
	 */
	if (++table->KeyCount / MUL (table->SegmentCount, SegmentSizeShift) 
		> table->MaxLoadFactor)
	{
		ExpandTable (table);	/* doesn't affect q	 */
	}

	return (q->item);
}

	/*
	 *  Hashes the argument to a unique address.  Note this assumes 
	 *  the argument is a null terminated string not the address of 
	 *  a structure.
	 */
static uintmax_t
DefaultHash (k)
char		*k;
{
	uintmax_t	h;

	h = 0;
	/*
	 * * Convert string to integer
	 */
	while (*k)
		h = h * Prime1 ^ (*k++ - ' ');
	h %= Prime2;
	return h;
}

	/*
	 *  DefaultCompare assumes the arguments are null terminated 
	 *  strings not structures.
	 */
static int
DefaultCompare (k1, k2)
char		*k1;
char		*k2;
{
	return strcmp (k1, k2);
}


static uintmax_t
Hash (table, item)
HashTable	*table;
char		*item;
{
	uintmax_t       h;
	uintmax_t	address;

	h = table->hash (item);
	address = MOD (h, table->maxp);
	if (address < table->p)
	{
		/* h % (2*table->maxp)	 */
		address = MOD (h, (table->maxp << 1));
	}
	return (address);
}

static void
ExpandTable (table)
HashTable	*table;
{
	uintmax_t	NewAddress;
	int		OldSegmentIndex, NewSegmentIndex;
	int		OldSegmentDir, NewSegmentDir;
	Segment		*OldSegment, *NewSegment;
	Element		*Current, **Previous, **LastOfNew;

	if (table->maxp + table->p < MUL (DirectorySize, SegmentSizeShift))
	{
		/*
		 * Locate the bucket to be split
		 */
		OldSegmentDir = DIV (table->p, SegmentSizeShift);
		OldSegment = table->Directory[OldSegmentDir];
		OldSegmentIndex = MOD (table->p, SegmentSize);

		/*
		 * Expand address space; if necessary create a new segment
		 */
		NewAddress = table->maxp + table->p;
		NewSegmentDir = DIV (NewAddress, SegmentSizeShift);
		NewSegmentIndex = MOD (NewAddress, SegmentSize);
		if (NewSegmentIndex == 0)
		{
			table->Directory[NewSegmentDir] = 
			      (Segment *) calloc (SegmentSize, sizeof(Segment));
		}
		NewSegment = table->Directory[NewSegmentDir];

		/*
		 * * Adjust state variables
		 */
		table->p++;
		if (table->p == table->maxp)
		{
			table->maxp <<= 1;	/* table->maxp *= 2	 */
			table->p = 0;
		}
		table->SegmentCount++;

		/*
		 * * Relocate records to the new bucket
		 */
		Previous = &OldSegment[OldSegmentIndex];
		Current = *Previous;
		LastOfNew = &NewSegment[NewSegmentIndex];
		*LastOfNew = NULL;
		while (Current != NULL)
		{
			if (Hash (table, Current->item) == NewAddress)
			{
				/*
				 * * Attach it to the end of the new chain
				 */
				*LastOfNew = Current;

				/*
				 * * Remove it from old chain
				 */
				*Previous = Current->Next;
				LastOfNew = &Current->Next;
				Current = Current->Next;
				*LastOfNew = NULL;
			}
			else
			{
				/*
				 * * leave it on the old chain
				 */
				Previous = &Current->Next;
				Current = Current->Next;
			}
		}
	}
}
