#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "gobject/GObject.h"

/**
 *  A GObject that holds other GObjects. There are several functions for
 *  adding and retrieving objects from a vector. Memory is dynamically
 *  allocated. Vector elements are unregistered (and possibly freed) when
 *  they are removed from the vector, or the vector is deleted.
 *
 *  @see new_Vector
 *  @see new_GObjectPart
 */

/** 
 *  A GObject that holds other GObjects.
 *
 *  @member capacityIncrement The incremental amount by which the storage is increased when necessary.
 *  @member elementData An array of GObjects.
 *  @member elementCount The number of GObjects in elementData.
 *  @member capacity The current space allocated to elementData.
 *  @member registerElements If True, the GObjects are registered with this Vector when they are added and unregistered when they are removed.
 */

typedef struct
{
        GObjectPart	core;

	int		capacityIncrement;
	GObject		*elementData; 
	int		elementCount;
	int		capacity;
	bool		registerElements;
} _Vector, *Vector;

/** 
 *  A GObject that represents the enumeration of a Vector object.
 *
 *  @member index The index of the enumeration.
 *  @member vector The Vector that is being enumerated.
 *  @see new_Enumeration
 */
typedef struct
{
        GObjectPart	core;

	int		index;
	Vector		vector;
} _Enumeration, *Enumeration;

Vector new_Vector(void);
Vector new_Vector2(int initialCapacity, int capacityIncrement);
Vector new_Vector_noRegister(void);
bool Vector_addElement(Vector v, GObject element);
bool Vector_removeElement(Vector v, GObject element);
bool Vector_insertElementAt(Vector v, GObject element, int index);
void Vector_removeElementAt(Vector v, int index);
void Vector_removeAllElements(Vector v);
void Vector_setElementAt(Vector v, GObject element, int index);
GObject Vector_elementAt(Vector v, int index);
void Vector_copyInto(Vector v, GObject *array);
GObject *Vector_arrayCopy(Vector v);
void Vector_trimToSize(Vector v);
void Vector_ensureCapacity(Vector v, int minCapacity);
void Vector_setSize(Vector v, int newSize);
int Vector_size(Vector v);
bool Vector_contains(Vector v, GObject element);
Enumeration Vector_elements(Vector v);
bool Enumeration_hasMoreElements(Enumeration enumeration);
GObject Enumeration_nextElement(Enumeration enumeration);
int Vector_index(Vector v, GObject element);
void Vector_reposition(Vector v, int old_index, int new_index);

#define Vector_last(v) v->elementData[v->elementCount-1]

#endif
