#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "gobject/GObject.h"
#include "gobject/Vector.h"

/** 
 *  A GObject for holding a hashtable. This is a simple hashtable
 *  consisting of a Vector of keys and a Vector of elements.
 *
 *  @see new_Hashtable
 *  @see Vector
 *  @see new_GObjectPart
 */

/** 
 *  The Hashtable structure definition.
 *
 *  @member keys A Vector containing the GObject keys.
 *  @member elements A Vector containing the GObject elements.
 */
typedef struct
{
	GObjectPart		core;

	/* Hashtable members */
	Vector			keys;
	Vector			elements;
} _Hashtable, *Hashtable;

Hashtable new_Hashtable(void);
Hashtable new_Hashtable_noRegister(void);
void Hashtable_clear(Hashtable h);
int Hashtable_size(Hashtable h);
bool Hashtable_isEmpty(Hashtable h);
Enumeration Hashtable_keys(Hashtable h);
Enumeration Hashtable_elements(Hashtable h);
bool Hashtable_contains(Hashtable h, GObject value);
bool Hashtable_containsKey(Hashtable h, GObject key);
GObject Hashtable_get(Hashtable h, const char *key);
GObject Hashtable_getObj(Hashtable h, GObject key);
void Hashtable_put(Hashtable h, const char *key, GObject value);
void Hashtable_putObj(Hashtable h, GObject key, GObject value);
void Hashtable_remove(Hashtable h, const char *key);
void Hashtable_removeObj(Hashtable h, GObject key);

#endif
