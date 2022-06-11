/** \file List.cpp
 *  \brief Defines class List.
 *  \author Ivan Henson
 */
#include "config.h"
#include "motif++/List.h"
#include "motif++/Parse.h"
using namespace Parse;

extern "C" {
#include "libstring.h"
}

/** Constructor with listener.
 *  @param[in] name the name given to this List instance.
 *  @param[in] parent the Component parent.
 *  @param[in] listener for XmNextendedSelectionCallback,
 *	XmNmultipleSelectionCallback, and XmNsingleSelectionCallback actions.
 */
List::List(const string &name, Component *parent, ActionListener *listener) :
		Component(name, parent)
{
    init(parent, NULL, 0);
    if(listener) {
	addActionListener(listener, XmNextendedSelectionCallback);
	addActionListener(listener, XmNmultipleSelectionCallback);
	addActionListener(listener, XmNsingleSelectionCallback);
    }
}

/** Constructor with X-resources and listener.
 *  @param[in] name the name given to this List instance.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 *  @param[in] listener for XmNextendedSelectionCallback,
 *	XmNmultipleSelectionCallback, and XmNsingleSelectionCallback actions.
 */
List::List(const string &name, Component *parent, Arg *args, int n,
		ActionListener *listener) : Component(name, parent)
{
    init(parent, args, n);
    if(listener) {
	addActionListener(listener, XmNextendedSelectionCallback);
	addActionListener(listener, XmNmultipleSelectionCallback);
	addActionListener(listener, XmNsingleSelectionCallback);
    }
}

/** Initialize.
 *  @param[in] parent the Component parent.
 *  @param[in] args X-resource structures.
 *  @param[in] n the number of X-resource structures.
 */
void List::init(Component *parent, Arg *args, int n)
{
    if(!args) {
	base_widget = XtVaCreateManagedWidget(getName(),
			xmListWidgetClass, parent->baseWidget(), NULL);
    }
    else {
	base_widget = XtCreateManagedWidget(getName(),
			xmListWidgetClass, parent->baseWidget(),args,n);
    }
    installDestroyHandler();

    XtAddCallback(base_widget, XmNdefaultActionCallback,
		List::defaultActionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNbrowseSelectionCallback,
		List::browseSelectionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNextendedSelectionCallback,
		List::extendedSelectionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNmultipleSelectionCallback,
		List::multipleSelectionCallback, (XtPointer)this);
    XtAddCallback(base_widget, XmNsingleSelectionCallback,
		List::singleSelectionCallback, (XtPointer)this);

    enableCallbackType(XmNdefaultActionCallback);
    enableCallbackType(XmNbrowseSelectionCallback);
    enableCallbackType(XmNextendedSelectionCallback);
    enableCallbackType(XmNmultipleSelectionCallback);
    enableCallbackType(XmNsingleSelectionCallback);
}

/** Destructor
 */
List::~List(void)
{
    if(base_widget) {
	XtRemoveAllCallbacks(base_widget, XmNdefaultActionCallback);
	XtRemoveAllCallbacks(base_widget, XmNbrowseSelectionCallback);
	XtRemoveAllCallbacks(base_widget, XmNextendedSelectionCallback);
	XtRemoveAllCallbacks(base_widget, XmNmultipleSelectionCallback);
	XtRemoveAllCallbacks(base_widget, XmNsingleSelectionCallback);
    }
}

void List::defaultActionCallback(Widget w, XtPointer client, XtPointer data)
{
    List *list = (List *)client;
    list->doCallbacks(w, data, XmNdefaultActionCallback);
}

void List::browseSelectionCallback(Widget w, XtPointer client, XtPointer data)
{
    List *list = (List *)client;
    list->doCallbacks(w, data, XmNbrowseSelectionCallback);
}

void List::extendedSelectionCallback(Widget w, XtPointer client, XtPointer data)
{
    List *list = (List *)client;
    list->doCallbacks(w, data, XmNextendedSelectionCallback);
}

void List::multipleSelectionCallback(Widget w, XtPointer client, XtPointer data)
{
    List *list = (List *)client;
    list->doCallbacks(w, data, XmNmultipleSelectionCallback);
}

void List::singleSelectionCallback(Widget w, XtPointer client, XtPointer data)
{
    List *list = (List *)client;
    list->doCallbacks(w, data, XmNsingleSelectionCallback);
}

/** Get the positions of the selected items.
 *  @param[out] pos the positions (the first item is at position 1)
 *  @returns the number of selected items.
 */
int List::getSelectedPos(int **pos)
{
    int n = 0;
    XmListGetSelectedPos(base_widget, pos, &n);
    return n;
}

/** Add items to the list.
 *  @param[in] s an array of string items to add to the list.
 *  @param[in] num the number of strings in s[].
 *  @param[in] pos add the items at this position. A value of 1 makes the
 *	first new item the first item in the list.
 *  @throw GERROR_MALLOC_ERROR
 */
void List::addItems(const char **s, int num, int pos) throw(int)
{
    if(num <= 0) return;
    XmString *xm = (XmString *)malloc(num*sizeof(XmString));
    if( !xm ) {
	GError::setMessage("List.addItems: malloc failed.");
	throw GERROR_MALLOC_ERROR;
    }
    for(int i = 0; i < num; i++) {
	xm[i] = createXmString(s[i]);
    }
    XmListAddItems(base_widget, xm, num, pos);
    for(int i = 0; i < num; i++) XmStringFree(xm[i]);
    free(xm);
}

/** Add one item to the list.
 *  @param[in] s the string item to add to the list.
 *  @param[in] pos add the items at this position. A value of 1 makes the
 *	first new item the first item in the list.
 */
void List::addItem(const string &s, int pos)
{
    XmString xm = createXmString(s);
    XmListAddItem(base_widget, xm, pos);
    XmStringFree(xm);
}

/** Get all the items in the list.
 *  @param[out] items an array of string items. Free each string and the array
 * 	pointer when no longer needed.
 *  @returns the number of items in items[].
 *  @throw GERROR_MALLOC_ERROR
 */
int List::getItems(char ***items) throw(int)
{
    XmString *xms;
    Arg args[2];
    int n = 0, num;

    XtSetArg(args[n], XmNitems, &xms); n++;
    XtSetArg(args[n], XmNitemCount, &num); n++;
    getValues(args, n);

    if(num > 0) {
	*items = (char **)malloc(num*sizeof(const char *));
	if( !items ) {
	    GError::setMessage("List.getItems: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	for(int i = 0; i < num; i++) {
	    (*items)[i] = getXmString(xms[i]);
	}
    }
    else {
	*items = NULL;
    }
    return num;
}

/** Get all the items in the list.
 *  @param[out] items an vector of string items.
 *  @returns the number of items in items[].
 *  @throw GERROR_MALLOC_ERROR
 */
int List::getItems(vector<string> &items)
{
    char *s;
    XmString *xms;
    Arg args[2];
    int n = 0, num;

    items.clear();

    XtSetArg(args[n], XmNitems, &xms); n++;
    XtSetArg(args[n], XmNitemCount, &num); n++;
    getValues(args, n);

    for(int i = 0; i < num; i++) {
	s = getXmString(xms[i]);
	items.push_back(string(s));
	free(s);
    }
    return (int)items.size();
}

/** Get all the selected items in the list.
 *  @param[out] items a vector of string items.
 *  @returns the size of items.
 */
int List::getSelectedItems(vector<string> &items)
{
    char *s;
    XmString *selectedItems;
    Arg args[2];
    int n = 0, num_selected;

    items.clear();

    XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
    XtSetArg(args[n], XmNselectedItemCount, &num_selected); n++;
    getValues(args, n);

    for(int i = 0; i < num_selected; i++) {
	s = getXmString(selectedItems[i]);
	items.push_back(string(s));
	free(s);
    }
    return (int)items.size();
}

/** Get all the selected items in the list.
 *  @param[out] items an array of string items. Free each string and the array
 * 	pointer when no longer needed.
 *  @returns the number of items in items[].
 *  @throw GERROR_MALLOC_ERROR
 */
int List::getSelectedItems(char ***items) throw(int)
{
    XmString *selectedItems;
    Arg args[2];
    int n = 0, num_selected;

    XtSetArg(args[n], XmNselectedItems, &selectedItems); n++;
    XtSetArg(args[n], XmNselectedItemCount, &num_selected); n++;
    getValues(args, n);

    if(num_selected > 0) {
	*items = (char **)malloc(num_selected*sizeof(const char *));
	if( !items ) {
	    GError::setMessage("List.getSelectedItems: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	for(int i = 0; i < num_selected; i++) {
	    (*items)[i] = getXmString(selectedItems[i]);
	}
    }
    else {
	*items = NULL;
    }
    return num_selected;
}

/** Get all the selected items in the list returned as quarks.
 *  @param[out] items an array of string quarks.
 *  @returns the number of items in items_q[]. Free it when no longer needed.
 *  @throw GERROR_MALLOC_ERROR
 */
int List::getSelectedItems(int **items_q) throw(int)
{
    char **items=NULL;
    int n = getSelectedItems(&items);
    *items_q = NULL;
    if(n > 0) {
	*items_q = (int *)malloc(n*sizeof(int));
	if( !items_q ) {
	    GError::setMessage("List.getSelectedItems: malloc failed.");
	    throw GERROR_MALLOC_ERROR;
	}
	for(int i = 0; i < n; i++) {
	    (*items_q)[i] = stringUpperToQuark(items[i]);
	    free(items[i]);
	}
    }
    Free(items);
    return n;
}

/** Select an item.
 *  @param[in] item the string item to select.
 *  @param[in] notify if true, do the select item callbacks.
 */
void List::select(const string &item, bool notify)
{
    XmString xm = createXmString(item);
    XmListSelectItem(base_widget, xm, (Boolean)notify);
    XmStringFree(xm);
}

/** Select an item by position.
 *  @param pos the position of the item.
 *  @param[in] notify if true, do the select item callbacks.
 */
void List::select(int pos, bool notify)
{
    XmListSelectPos(base_widget, pos, (Boolean)notify);
} 

/** Select one or more items by position.
 *  @param[in] pos an array of item positions.
 *  @param[in] num the number of positions.
 *  @param[in] notify if true, do the select item callbacks.
 */
void List::select(int *pos, int num, bool notify)
{
    Arg args[1];
    unsigned char selection_policy;
    int j, *selected=NULL, nselected;

    XtSetArg(args[0], XmNselectionPolicy, &selection_policy);
    getValues(args, 1);
    XtSetArg(args[0], XmNselectionPolicy, XmMULTIPLE_SELECT);
    setValues(args, 1);

    XmListGetSelectedPos(base_widget, &selected, &nselected);

    for(int i = 0; i < num; i++) {
	for(j = 0; j < nselected && pos[i] != selected[j]; j++);
	if(j == nselected) {
	    XmListSelectPos(base_widget, pos[i], (Boolean)notify);
	}
    }
    Free(selected);

    XtSetArg(args[0], XmNselectionPolicy, selection_policy);
    setValues(args, 1);
} 

/** Select one or more items by position. Deselect all other items.
 *  @param[in] pos an array of item positions.
 *  @param[in] num the number of positions.
 *  @param[in] notify if true, do the select item callbacks.
 */
void List::selectOnly(vector<int> &pos, bool notify)
{
    Arg args[1];
    unsigned char selection_policy;
    int j, *selected=NULL, nselected;

    XtSetArg(args[0], XmNselectionPolicy, &selection_policy);
    getValues(args, 1);
    XtSetArg(args[0], XmNselectionPolicy, XmMULTIPLE_SELECT);
    setValues(args, 1);

    XmListGetSelectedPos(base_widget, &selected, &nselected);

    for(int i = 0; i < nselected; i++) {
	for(j = 0; j < (int)pos.size() && selected[i] != pos[j]; j++);
	if(j == (int)pos.size()) {
	    // this deselects the selected item
	    XmListSelectPos(base_widget, selected[i], (Boolean)notify);
	}
    }

    for(int i = 0; i < (int)pos.size(); i++) {
	for(j = 0; j < nselected && pos[i] != selected[j]; j++);
	if(j == nselected) {
	    XmListSelectPos(base_widget, pos[i], (Boolean)notify);
	}
    }
    Free(selected);

    XtSetArg(args[0], XmNselectionPolicy, selection_policy);
    setValues(args, 1);
} 

/** Select all items in the list.
 */
void List::selectAll(void)
{
    int j, num, *selected=NULL, nselected;
    unsigned char policy;
    Arg args[2];

    num = 0;
    XtSetArg(args[0], XmNitemCount, &num);
    XtSetArg(args[1], XmNselectionPolicy, &policy);
    getValues(args, 2);

    XtSetArg(args[0], XmNselectionPolicy, XmMULTIPLE_SELECT);
    setValues(args, 1);

    XmListGetSelectedPos(base_widget, &selected, &nselected);

    for(int i = 0; i < num; i++)
    {
	for(j = 0; j < nselected && i+1 != selected[j]; j++);
	if(j == nselected) {
	    XmListSelectPos(base_widget, i+1, False);
	}
    }
    XtSetArg(args[0], XmNselectionPolicy, policy);
    setValues(args, 1);
}

ParseCmd List::parseCmd(const string &cmd, string &msg)
{
    vector<string> items;
    string s;

    if(parseArg(cmd, "select", s)) {
	int i, num = getItems(items);

	for(i = 0; i < num && !parseCompare(items[i], s); i++);
	if(i < num) {
	    select(i+1, true);
	    return COMMAND_PARSED;
	}
	return ARGUMENT_ERROR;
    }
    return Component::parseCmd(cmd, msg);
}

ParseVar List::parseVar(const string &name, string &value)
{
    vector<string> items;

    if(parseCompare(name, "num_selected")) {
	ostringstream os;
	getSelectedItems(items);
	os << (int)items.size();
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "get_selected")) {
	getSelectedItems(items);
	value.clear();
	for(int i = 0; i < (int)items.size(); i++) {
	    value.append(items[i]);
	    if(i < (int)items.size()-1) value.append(",");
	}
        return STRING_RETURNED;
    }
    
    return Component::parseVar(name, value);
}

void List::parseHelp(const char *prefix)
{
    char **items=NULL;

    int num = getItems(&items);
    for(int i = 0; i < num; i++) {
	char *s = strdup(items[i]);
	for(int j = 0; s[j] != '\0'; j++) {
	    if(isspace((int)s[j])) s[j] = '_';
	    else s[j] = tolower((int)s[j]);
	}
	printf("%s%s\n", prefix, s);
	Free(s);
    }
    Free(items);
}
