#ifndef _OBJECT_P_H_
#define _OBJECT_P_H_

#include <stdio.h>
#include <stdlib.h>

#include "gobject/GObject.h"

typedef struct {
	GObject	*owners;
	int	num_owners;
} _Registry, *Registry;

typedef struct {
	int	num_objects;
	GObject base_object;
	char	*freed_mask;
} GObjectBlock;

/** 
 *  GObjectPart of a GObject structure. This is the private declaration of
 *  the GObject structure.
 *  @member object_flag A "magic" number used to confirm that this is a GObject.
 *  @member object_type The object type integer obtained from GObject_type().
 *  @member registry The registry for this object.
 *  @member equals The equals method for the type of GObject. Default is NULL.
 *  @member clone The equals method for the type of GObject. Default is NULL.
 *  @member free The equals method for the type of GObject. Default is NULL.
 *  @private
 */
typedef struct GObject_struct
{
	/* GObject members */
	int			object_flag;
	int			object_type;
	GObjectBlock		*gb;
	Registry		registry;
	GObject_EqualsMethod	equals;
	GObject_CloneMethod	clone;
	GObject_FreeMethod	free;
} GObject_Part;

#endif
