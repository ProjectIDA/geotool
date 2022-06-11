/** \file TableListener.cpp
 *  \brief Defines class TableListener.
 *  \author Ivan Henson
 */
#include "config.h"
#include "widget/TableListener.h"
#include "motif++/Application.h"
#include "gobject++/CssTables.h"
extern "C" {
#include "libtime.h"
}

class GV : public Gobject
{
    public:
    vector<ActionListener *> v;
};

// static
void TableListener::addListener(CssTableClass *table, ActionListener *listener)
{
    GV *g;

    if( !(g = (GV *)table->getValue("listeners")) ) {
	g = new GV();
	table->putValue("listeners", g);
    }
    int i;
    for(i = 0; i < (int)g->v.size() && g->v[i] != listener; i++);
    if(i == (int)g->v.size()) {
	g->v.push_back(listener);
    }
}

// static
void TableListener::removeListener(CssTableClass *table, ActionListener *listener)
{
    GV *g;
    if( (g = (GV *)table->getValue("listeners")) ) {
	int i;
	for(i = 0; i < (int)g->v.size() && g->v[i] != listener; i++);
	if(i < (int)g->v.size()) {
	    g->v.erase(g->v.begin()+i);
	}
    }
}

// static
void TableListener::doCallbacks(CssTableClass *orig_table, CssTableClass *new_table,
			Component *source)
{
    DateTime *t1, *t2;
    int num, members[100];

    if(orig_table == new_table) {
	cerr << "TableListener::doCallbacks orig_table == new_table" << endl;
	return;
    }
    else if(orig_table->getType() != new_table->getType()
	    || orig_table->getNumMembers() != new_table->getNumMembers()
	    || orig_table->getNumBytes() != new_table->getNumBytes()
	    || orig_table->description() != new_table->description())
    {
	cerr << "TableListener::doCallbacks table not the same type" << endl;
	return;
    }

    CssClassDescription *des1 = orig_table->description();
    CssClassDescription *des2 = new_table->description();

    num = 0;
    for(int i = 0; i < orig_table->getNumMembers() && i < 100; i++)
    {
	char *member1 = (char *)orig_table + des1[i].offset;
	char *member2 =  (char *)new_table + des2[i].offset;

	bool changed = false;
	switch(des1[i].type)
	{
	    case CSS_STRING:
		if(strcmp(member1, member2)) changed = true;
		break;
	    case CSS_DATE:
		t1 = (DateTime *)member1;
		t2 = (DateTime *)member2;
		if(t1->year != t2->year || t1->month != t2->month
			|| t1->day != t2->day) changed = true;
		break;
	    case CSS_LDDATE:
		t1 = (DateTime *)member1;
		t2 = (DateTime *)member2;
		if(t1->year != t2->year || t1->month != t2->month
			|| t1->day != t2->day) changed = true;
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		if(*(long *)member1 != *(long *)member2) changed = true;
		break;
	    case CSS_INT:
	    case CSS_QUARK:
		if(*(int *)member1 != *(int *)member2) changed = true;
		break;
	    case CSS_DOUBLE:
		if(*(double *)member1 != *(double *)member2) changed = true;
		break;
	    case CSS_FLOAT:
		if(*(float *)member1 != *(float *)member2) changed = true;
		break;
	    case CSS_TIME:
		if(*(double *)member1 != *(double *)member2) changed = true;
		break;
	    case CSS_BOOL:
		if(*(bool *)member1 != *(bool *)member2) changed = true;
		break;
	    default:
		logErrorMsg(LOG_WARNING,
		    "TableListener::doCallbacks: unknown member type.");
		break;
	}
	if(changed) members[num++] = i;
    }
    if(num > 0) {
	doCallbacks(new_table, source, num, members);
    }
}

// static
void TableListener::doCallbacks(CssTableClass *table, Component *source, int changed)
{
    doCallbacks(table, source, 1, &changed);
}

// static
void TableListener::doCallbacks(CssTableClass *table, Component *source, int num,
				int *changed)
{
    TableListenerCallback tc;
    GV *g;

    for(int i = 0; i < num && i < 100; i++) tc.members[i] = changed[i];
    tc.num_members = num;
    tc.table = table;

    if( !(g = (GV *)table->getValue("listeners")) ) {
	Application::getApplication()->tableModified(&tc, table->getName());
	return;
    }

    for(int i = 0; i < (int)g->v.size(); i++) {
	ActionListener *comp = g->v[i];
	if(comp != source) {
	    ActionEvent *a = new ActionEvent(source, source->baseWidget(),
			table->getName(), (void *)&tc, CssTableChange);

	    comp->actionPerformed(a);
	    a->deleteObject();
	}
    }
    Application::getApplication()->tableModified(&tc, table->getName());
}

// static
void TableListener::doCallbacks(CssTableClass *table, Component *source,
			const char *command)
{
    TableListenerCallback tc;
    GV *g;

    tc.num_members = 0;
    tc.table = table;

    if( !(g = (GV *)table->getValue("listeners")) ) {
	Application::getApplication()->tableModified(&tc, command);
	return;
    }

    for(int i = 0; i < (int)g->v.size(); i++) {
	ActionListener *comp = g->v[i];
//	if(comp != source) {
	    ActionEvent *a = new ActionEvent(source, source->baseWidget(),
					command, (void *)&tc, CssTableChange);
	    comp->actionPerformed(a);
	    a->deleteObject();
//	}
    }
    Application::getApplication()->tableModified(&tc, command);
}
