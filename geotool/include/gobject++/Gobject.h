#ifndef _GoBJECT_H
#define _GoBJECT_H

#include <sstream>
#include <vector>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "gobject++/CommandParser.h"
#include "gobject++/GError.h"
extern "C" {
#include "logErrorMsg.h"
void setPrintError(bool print);
int sortGVectorElements(const void *a, const void *b);
}

#define Free(a) {if(a) free((void *)a); a = NULL; }

/** @defgroup libgobject library libgobject++
 *  C++ classes for data objects including time series data.
 *  This library contains the Gobject class, which is the base class for most
 *  of the classes in Geotool. It also contains classes for primitive data
 *  objects (GDouble, GInteger, etc.), object container classes
 *  (gvector, ghashtable, etc.), and classes for time series data (GDataPoint,
 *  GSegment, GTimeSeries, etc.).
 */

class Gobject;
class GDouble;

/** Base class.
 *  @ingroup libgobject
 */
class Gobject : public CommandParser
{
    public:
	Gobject(void) : owners() {}	// constructor
	virtual ~Gobject(void);	// destructor
	/** Clone or copy a Gobject.
	 *  @returns a copy of this Gobject.
	 */
        Gobject & operator=(const Gobject &o) {
	    // do not copy owners
	    if(this == &o) return *this;
	    owners.clear();
	    return *this;
	}
	Gobject(const Gobject &o) {
	    // do not copy owners
	}
	virtual Gobject *clone(void) { return new Gobject(); }
	void deleteObject(void);
	bool hasOwners(void);
	void addOwner(Gobject *owner);
	bool removeOwner(Gobject *owner, bool do_delete=true);
	char *errorMsg(void);

    private:
	void debug(void);
	vector<Gobject *> owners; // list of owner Gobjects
};

/** A class to hold a void pointer.
 */
class GPointer : public Gobject
{
    public:
	/** Constructor.
	 *  @param[in] p the void pointer.
	 */
	GPointer(void *p) : value(p) {}
	GPointer(GPointer &gp) : value(gp.value) {}
	GPointer & operator=(const GPointer &p) {value = p.value; return *this;}
	/** Destructor. */
	~GPointer(void) {}
	void *value;
	Gobject *clone(void) { return new GPointer(value); }
};

#endif
