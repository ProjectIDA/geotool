#ifndef _TABLE_LISTENER_H
#define _TABLE_LISTENER_H

#include "motif++/Component.h"
#include "gobject++/CssTableClass.h"

#define CssTableChange "CssTableChange"

typedef struct
{
    CssTableClass *table;
    int num_members;
    int members[100];
} TableListenerCallback;

/**
 *  @ingroup libgio
 */
class TableListener
{
    public:
	static void addListener(CssTableClass *table, ActionListener *listener);
	static void removeListener(CssTableClass *table, ActionListener *listener);
	static void doCallbacks(CssTableClass *orig_table, CssTableClass *new_table,
			Component *source);
	static void doCallbacks(CssTableClass *table, Component *source, int member);
	static void doCallbacks(CssTableClass *table, Component *source,
			int num, int *changed);
	static void doCallbacks(CssTableClass *table, Component *source,
			const char *command);
};

#endif

