#ifndef LIST_H
#define LIST_H

#include <string>
using namespace std;
#include "motif++/Component.h"

/** A class for the XmList widget.
 *  @ingroup libmotif
 */
class List : public Component
{
    public:

	List(const string &name, Component *parent,
			ActionListener *listener=NULL);
	List(const string &name, Component *parent, Arg *args, int n,
			ActionListener *listener=NULL);
	~List(void);

	int getSelectedPos(int **pos);
	/** Deselect all items in the list */
	void deselectAll(void) { XmListDeselectAllItems(base_widget); }
	/** Delete all items from the list */
	void deleteAll(void) {
	    XmListDeselectAllItems(base_widget); // for bug in XmList
	    XmListDeleteAllItems(base_widget);
	}
	void addItems(const char **s, int num, int pos=1) throw(int);
	void addItems(char **s, int num, int pos=1) throw(int)
	{
	    addItems((const char **)s, num, pos);
	}
	/** Add (append) an item to the list. */
	void addItem(const string &s, int pos=0);
	/** Select an item by position.
	 *  @param[in] position the position of the item starting at 1.
	 *  @param[in] do_callbacks if true, do the select callbacks.
	 */
	void selectPos(int position, bool do_callbacks=false) {
	    XmListSelectPos(base_widget, position, (Boolean)do_callbacks);
	}
	void selectItem(const string &item) {
	    Arg args[1];
	    unsigned char policy;
	    XtSetArg(args[0], XmNselectionPolicy, &policy);
	    XtGetValues(base_widget, args, 1);
	    if(policy == XmEXTENDED_SELECT) {
		XtSetArg(args[0], XmNselectionPolicy, XmMULTIPLE_SELECT);
		XtSetValues(base_widget, args, 1);
	    }
	    XmString xm = createXmString(item);
	    XmListSelectItem(base_widget, xm, True);
	    XmStringFree(xm);
	    if(policy == XmEXTENDED_SELECT) {
		XtSetArg(args[0], XmNselectionPolicy, XmEXTENDED_SELECT);
		XtSetValues(base_widget, args, 1);
	    }
	}
	int getSelectedItems(vector<string> &items);
	int getSelectedItems(char ***items) throw(int);
	int getSelectedItems(int **items_q) throw(int);
	int getItems(char ***items) throw(int);
	int getItems(vector<string> &items);
	void select(const string &item, bool notify=false);
	void select(int pos, bool notify=false);
	void select(int *pos, int num, bool notify=false);
	void selectOnly(int *pos, int num, bool notify=false) {
	    vector<int> p;
	    for(int i = 0; i < num; i++) p.push_back(pos[i]);
	    selectOnly(p, notify);
	}
	void selectOnly(vector<int> &pos, bool notify=false);
	void selectAll(void);
	int numItems(void) {
	    int num = 0; Arg args[1];
	    XtSetArg(args[0], XmNitemCount, &num);
	    getValues(args, 1);
	    return num;
	}
	bool itemSelected(void) {
	    int num, *pos=NULL;
	    XmListGetSelectedPos(base_widget, &pos, &num);
	    Free(pos);
	    return (num > 0) ? true : false;
	}
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	void init(Component *, Arg *args, int n);

    private:
	static void defaultActionCallback(Widget, XtPointer, XtPointer);
	static void browseSelectionCallback(Widget, XtPointer, XtPointer);
	static void extendedSelectionCallback(Widget, XtPointer, XtPointer);
	static void multipleSelectionCallback(Widget, XtPointer, XtPointer);
	static void singleSelectionCallback(Widget, XtPointer, XtPointer);
};

#endif
