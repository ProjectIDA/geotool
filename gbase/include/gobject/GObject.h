#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <stdio.h>
#include <stdlib.h>

/**
 *  A GObject is any structure that has as its first member a GObjectPart.
 *  The GObjectPart has three function pointers, GObject_EqualsMethod,
 *  GObject_CloneMethod and GObject_FreeMethod. 
 */

/** 
 *  GObjectPart structure. This is an opaque structure pointer here to
 *  protect its contents, which should not be changed after creation by
 *  new_GObjectPart().
 *  @see new_GObjectPart
 *  @no-sort
 */
typedef struct GObject_struct    *GObjectPart;

/** 
 *  GObject structure. A GObject is any structure that has a GObjectPart as
 *  its first member.
 */
typedef struct
{
	GObjectPart         core;
} _GObject, *GObject;

#ifndef __cplusplus
typedef char bool;
#endif

#define True	1
#define False	0

/** 
 *  Equals method.
 *  @param o1 A GObject structure pointer.
 *  @param o2 A GObject structure pointer.
 *  @return True if the two objects are equal, otherwise False.
 */
typedef bool (*GObject_EqualsMethod)(GObject o1, GObject o2);

/** 
 *  Clone method.
 *  @param o A GObject to be cloned.
 *  @return The cloned GObject.
 */
typedef GObject (*GObject_CloneMethod)(GObject o);

/** 
 *  Free method.
 *  @param o A GObject to be freed.
 *  @return True if the free was successful, otherwise False.
 */
typedef bool (*GObject_FreeMethod)(GObject o);


/* A random number for object_flag used to confirm that a pointer is an GObject
 */
#define OBJECT_FLAG	428653291

typedef struct
{
	GObjectPart		core;

	/* StringObj members */
	char			*string;
/*
	int			quark;
*/
} _StringObj, *StringObj;

typedef struct
{
	GObjectPart		core;

	/* Quark members */
	int			value;
} _Quark, *Quark;

typedef struct
{
	GObjectPart		core;

	/* Integer members */
	int			value;
} _Integer, *Integer;

typedef struct
{
	GObjectPart		core;

	/* IntegerArray members */
	int			*value;
	int			length;
} _IntegerArray, *IntegerArray;

typedef struct
{
	GObjectPart		core;

	/* Long members */
	long			value;
} _Long, *Long;

typedef struct
{
	GObjectPart		core;

	/* LongArray members */
	long			*value;
	int			length;
} _LongArray, *LongArray;

typedef struct
{
	GObjectPart		core;

	/* Double members */
	double			value;
} _Double, *Double;

typedef struct
{
	GObjectPart		core;

	/* DoubleArray members */
	double			*value;
	int			length;
} _DoubleArray, *DoubleArray;

typedef struct
{
	GObjectPart		core;

	/* Float members */
	float			value;
} _Float, *Float;

typedef struct
{
	GObjectPart		core;

	/* FloatArray members */
	float			*value;
	int			length;
} _FloatArray, *FloatArray;

typedef struct
{
	GObjectPart		core;

	/* Character members */
	char			value;
} _Character, *Character;

typedef struct
{
	GObjectPart		core;

	/* Short members */
	short			value;
} _Short, *Short;

typedef struct
{
	GObjectPart		core;

	/* ShortArray members */
	short			*value;
	int			length;
} _ShortArray, *ShortArray;

typedef struct
{
	GObjectPart		core;

	/* Block members */
	void			*value;
	int			length;
} _Block, *Block;

typedef struct
{
	GObjectPart		core;

	/* Pointer members */
	void			*value;
} _Pointer, *Pointer;

/*
#define GObject_equals(a,b)	_GObject_equals((GObject)a,(GObject)b)
#define GObject_clone(a)	_GObject_clone((GObject)a)
#define GObject_free(a)		_GObject_free((GObject)a)
#define GObject_register(a,b)	_GObject_register((GObject)a, (GObject)b)
#define GObject_unregister(a,b)	_GObject_unregister((GObject)a, (GObject)b)
#define GObject_unregisterNoFree(a,b)	_GObject_unregisterNoFree((GObject)a, (GObject)b)
#define GObject_isRegistered(a)	_GObject_isRegistered((GObject)a)
*/

#define is_GObject(o) (o!=NULL && o->core!=NULL && *(int *)o->core==OBJECT_FLAG)

/* Prototypes
 */
GObjectPart new_GObjectPart(int object_type, GObject_EqualsMethod equals,
                GObject_CloneMethod clone, GObject_FreeMethod free);
GObjectPart *new_GObjectParts(int num, GObject base, int object_type,
	        GObject_EqualsMethod equals_method,
		GObject_CloneMethod clone_method,
	        GObject_FreeMethod free_method);
int GObject_getType(GObject o);
const char *GObject_getName(GObject o);
int GObject_registerType(const char *s);
int GObject_type(const char *s);
bool GObject_equals(GObject g1, GObject g2);
bool GObject_free(GObject o);
GObject GObject_clone(GObject o);
bool GObject_register(GObject key, GObject value);
bool GObject_unregister(GObject key, GObject value);
bool GObject_unregisterAll(GObject key, GObject value);
bool GObject_unregisterNoFree(GObject key, GObject value);
bool GObject_isRegistered(GObject value);
/*bool is_GObject(GObject value); replaced by define */
StringObj new_StringObj(const char *s);
StringObj null_StringObj(void);
Quark new_Quark(const char *s);
Integer new_Integer(int i);
IntegerArray new_IntegerArray(int *i, int length);
Long new_Long(long i);
LongArray new_LongArray(long *i, int length);
Double new_Double(double d);
DoubleArray new_DoubleArray(double *d, int length);
Float new_Float(double f);
FloatArray new_FloatArray(float *f, int length);
Character new_Character(int c);
Short new_Short(int s);
ShortArray new_ShortArray(short *s, int length);
Block new_Block(void *s, int length);
Pointer new_Pointer(void *s);
void GObject_terminate(void);

#endif
