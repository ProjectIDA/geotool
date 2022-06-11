/***********************************************************
Copyright 1987, 1988, 1990 by Digital Equipment Corporation, Maynard,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*

Copyright 1987, 1988, 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "libstring.h"

#define True	1
#define False	0

static int nextQuark = 1;	/* next available quark number */
static unsigned long quarkMask = 0;
static unsigned long zero = 0;
static unsigned long *quarkTable = &zero; /* crock */
static unsigned long quarkRehash;
static char ***stringTable = NULL;

#define NULLQUARK	0
#define QUANTUMSHIFT	8
#define QUANTUMMASK	((1 << QUANTUMSHIFT) - 1)
#define CHUNKPER	8
#define CHUNKMASK	((CHUNKPER << QUANTUMSHIFT) - 1)

#define LARGEQUARK	((unsigned long)0x80000000L)
#define QUARKSHIFT	18
#define QUARKMASK	((LARGEQUARK - 1) >> QUARKSHIFT)
#define XSIGMASK	((1L << QUARKSHIFT) - 1)

#define STRQUANTSIZE	(sizeof(char *) * (QUANTUMMASK + 1))
#define QUANTSIZE	STRQUANTSIZE

#define HASH(sig) ((sig) & quarkMask)
#define REHASHVAL(sig) ((((sig) % quarkRehash) + 2) | 1)
#define REHASH(idx,rehash) ((idx + rehash) & quarkMask)
#define NAME(q) stringTable[(q) >> QUANTUMSHIFT][(q) & QUANTUMMASK]

/* Permanent memory allocation */

#define WALIGN sizeof(unsigned long)
#define DALIGN sizeof(double)

#define NEVERFREETABLESIZE ((8192-12) & ~(DALIGN-1))
static char *neverFreeTable = NULL;
static int  neverFreeTableSize = 0;

static char *permalloc(unsigned int length);
static char *Xpermalloc(unsigned int length);
static int internalStringToQuark(const char *name, int len, unsigned long sig);

static char *
permalloc(unsigned int length)
{
    char *ret;

    if (neverFreeTableSize < (signed int)length) {
	if (length >= NEVERFREETABLESIZE)
	    return (char*)malloc(length);
	if (! (ret = (char*)malloc(NEVERFREETABLESIZE)))
	    return (char *) NULL;
	neverFreeTableSize = NEVERFREETABLESIZE;
	neverFreeTable = ret;
    }
    ret = neverFreeTable;
    neverFreeTable += length;
    neverFreeTableSize -= length;
    return(ret);
}

#ifndef WORD64
typedef struct {char a; double b;} TestType1;
typedef struct {char a; unsigned long b;} TestType2;
#endif

static char *
Xpermalloc(unsigned int length)
{
    int i;

    if (neverFreeTableSize && length < NEVERFREETABLESIZE) {
#ifndef WORD64
	if ((sizeof(TestType1) !=
	     (sizeof(TestType2) - sizeof(unsigned long) + sizeof(double))) &&
	    !(length & (DALIGN-1)) &&
	    ((i = (NEVERFREETABLESIZE - neverFreeTableSize) & (DALIGN-1)))) {
	    neverFreeTableSize -= DALIGN - i;
	    neverFreeTable += DALIGN - i;
	} else
#endif
	    if ((i = (NEVERFREETABLESIZE - neverFreeTableSize) & (WALIGN-1))) {
		neverFreeTableSize -= WALIGN - i;
		neverFreeTable += WALIGN - i;
	    }
    }
    return permalloc(length);
}

static int
ExpandQuarkTable(void)
{
    unsigned long oldmask, newmask;
    char c, *s;
    unsigned long *oldentries, *entries;
    unsigned long entry;
    int oldidx, newidx, rehash;
    unsigned long sig;
    int q;

    oldentries = quarkTable;
    if ((oldmask = quarkMask))
	newmask = (oldmask << 1) + 1;
    else {
	if (!stringTable) {
	    stringTable = (char ***)malloc(sizeof(char **) * CHUNKPER);
	    if (!stringTable)
		return False;
	    stringTable[0] = (char **)NULL;
	}
	stringTable[0] = (char **)Xpermalloc(QUANTSIZE);
	if (!stringTable[0])
	    return False;
	newmask = 0x1ff;
    }
    /* replace malloc with calloc */
    entries = (unsigned long *)calloc((newmask + 1), sizeof(unsigned long));
    if (!entries)
	return False;
    /* no longer needed, since malloc was recplaced by calloc 
       bzero((char *)entries, sizeof(unsigned long) * (newmask + 1));
     */
    quarkTable = entries;
    quarkMask = newmask;
    quarkRehash = quarkMask - 2;
    for (oldidx = 0; oldidx <= (signed int)oldmask; oldidx++) {
	if ((entry = oldentries[oldidx])) {
	    if (entry & LARGEQUARK)
		q = entry & (LARGEQUARK-1);
	    else
		q = (entry >> QUARKSHIFT) & QUARKMASK;
	    for (sig = 0, s = NAME(q); (c = *s++); )
		sig = (sig << 1) + c;
	    newidx = HASH(sig);
	    if (entries[newidx]) {
		rehash = REHASHVAL(sig);
		do {
		    newidx = REHASH(newidx, rehash);
		} while (entries[newidx]);
	    }
	    entries[newidx] = entry;
	}
    }
    if (oldmask)
	free(oldentries);
    return True;
}

static int
internalStringToQuark(const char *name, int len, unsigned long sig)
{
    int q;
    unsigned long entry;
    int idx, rehash;
    int i;
    const char *s1, *s2;
    char *new_index, *nam, *n1;

    rehash = 0;
    idx = HASH(sig);
    while ((entry = quarkTable[idx])) {
	if (entry & LARGEQUARK)
	    q = entry & (LARGEQUARK-1);
	else {
	    if ((entry - sig) & XSIGMASK)
		goto nomatch;
	    q = (entry >> QUARKSHIFT) & QUARKMASK;
	}
	for (i = len, s1 = (const char *)name, s2 = NAME(q); --i >= 0; ) {
	    if (*s1++ != *s2++)
		goto nomatch;
	}
	if (*s2) {
nomatch:    if (!rehash)
		rehash = REHASHVAL(sig);
	    idx = REHASH(idx, rehash);
	    continue;
	}
	return q;
    }
    if ((nextQuark + (nextQuark >> 2)) > (signed int)quarkMask) {
	if (!ExpandQuarkTable())
	    goto fail;
	return internalStringToQuark(name, len, sig);
    }
    q = nextQuark;
    if (!(q & QUANTUMMASK)) {
	if (!(q & CHUNKMASK)) {
	    if (!(new_index = (char*)realloc(stringTable, sizeof(char **) *
				 ((q >> QUANTUMSHIFT) + CHUNKPER))))
		goto fail;
	    stringTable = (char ***)new_index;
	}
	new_index = (char*)Xpermalloc(QUANTSIZE);
	if (!new_index)
	    goto fail;
	stringTable[q >> QUANTUMSHIFT] = (char **)new_index;
    }
    s2 = (const char *)name;
    nam = (char*)permalloc((size_t)(len+1));
    if (!nam) goto fail;
    for (i = len, n1 = nam; --i >= 0; ) *n1++ = *s2++;
    *n1++ = '\0';
    NAME(q) = nam;
    if (q <= (signed int)QUARKMASK)
	entry = (q << QUARKSHIFT) | (sig & XSIGMASK);
    else
	entry = q | LARGEQUARK;
    quarkTable[idx] = entry;
    nextQuark++;
    return q;
 fail:
    return NULLQUARK;
}

/**
 * Return a quark (unique int) for a string.
 */
int
stringToQuark(const char *name)
{
    char c;
    const char *tname;
    unsigned long sig = 0;

    if (!name)
	return (NULLQUARK);
    
    for (tname = name; (c = *tname++); )
	sig = (sig << 1) + c;

    return internalStringToQuark(name, tname-(const char *)name-1, sig);
}

/**
 * Return a quark (unique int) for a string.
 */
int
stringNToQuark(const char *name, int len)
{
    int i;
    unsigned long sig = 0;

    if (!name) return (NULLQUARK);
    
    for(i = 0; i < len && name[i] != '\0'; i++) {
	sig = (sig << 1) + name[i];
    }

    return internalStringToQuark(name, i, sig);
}

/**
 * Return a the string representation of a quark.
 */
const char *
quarkToString(int quark)
{
    char *s;

    if (quark <= 0 || quark >= nextQuark)
    	s = (char *)0;
    else {
	s = NAME(quark);
    }
    return s;
}
