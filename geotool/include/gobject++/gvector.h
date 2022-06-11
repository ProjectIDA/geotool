#ifndef _gvECTOR_H_
#define _gvECTOR_H_

#include <iostream>
using namespace std;
#include "gobject++/Gobject.h"

/* File level C language type definition of the qsort compar function pointer.
 * This is required by the gvector class sort method to allow passage of a C
 * function pointer.
 */
extern "C" {
  typedef int (*qsort_compar_ptr_t)(const void *, const void *);
}

/**
 *  A class for holding Gobjects. The gvector provides automatic memory
 *  allocation as elements are added.
 *  @ingroup libgobject
 */
template <class Type> class gvector : public Gobject
{
    public:
    // constructors
    gvector(void) : capacity_increment(1), element_data(NULL),
	element_count(0), capacity(1), own_elements(true) { init(1, 1, true); }

    gvector(int initial_capacity) : capacity_increment(1), element_data(NULL),
	element_count(0), capacity(initial_capacity), own_elements(true)
    {
	init(initial_capacity, 1, true);
    }

    gvector(bool element_owner) : capacity_increment(1), element_data(NULL),
	element_count(0), capacity(1), own_elements(element_owner)
    {
	init(1, 1, element_owner);
    }

    gvector(const gvector<Type> &v) : capacity_increment(v.capacity_increment),
	element_data(NULL), element_count(0), capacity(v.capacity),
	own_elements(v.own_elements)
    {
	int num = (v.element_count > 0) ? v.element_count : 1;
	init(num, 1, v.own_elements);
	for(int i = 0; i < v.element_count; i++) add(v.element_data[i]);
    }
    gvector(Type element) : capacity_increment(1), element_data(NULL),
	element_count(0), capacity(1), own_elements(true)
    {
	init(1, 1, true);
	add(element);
    }

    gvector & operator=(const gvector &rhs) {
	if(this != &rhs) {
	    removeAll();
	    own_elements = rhs.own_elements;
	    capacity_increment = rhs.capacity_increment;
	    for(int i = 0; i < rhs.element_count; i++) add(rhs.element_data[i]);
	}
	return *this;
    }

    ~gvector(void) { removeAll(); free(element_data); }

    void push_back(Type element) { add(element); }
    Type front(void) { return goodIndex(0, "front") ? element_data[0] : NULL; }
    Type back(void) { return goodIndex(element_count-1, "back") ?
			element_data[element_count-1] : NULL; }
    void clear(void) { removeAll(); }

    void add(Type element)
    {
	if(!element) return;
	if(element_count == capacity) {
	    capacity += capacity_increment;
	    element_data = (Type *)realloc(element_data, capacity*sizeof(Type));
	}
	element_data[element_count++] = element;
	if(own_elements) element->addOwner(this);
    }

    bool remove(Type element)
    {
	int i;

	for(i = 0; i < element_count; i++)
	{
	    if(element_data[i] == element)
	    {
		int j;
		for(j = i; j < element_count-1; j++) {
		    element_data[j] = element_data[j+1];
		}
		element_count--;
		if(own_elements) {
		    /* if this is the only occurrence of this object, remove
		     * ownership of it.
		     */
		    for(j = i; j < element_count; j++) {
			if(element_data[j] == element) break;
		    }
		    if(j == element_count) element->removeOwner(this);
		}
		return true;
	    }
	}
	return false;
    }
    void insert(Type element, int position)
    {
	if(!element) return;
	if(position >= element_count) {
	    add(element);
	    return;
	}
	if(position < 0) position = 0;

	int i;

	if(element_count == capacity) {
	    capacity += capacity_increment;
	    element_data = (Type *)realloc(element_data, capacity*sizeof(Type));
	}
	for(i = element_count; i > position; i--) {
	    element_data[i] = element_data[i-1];
	}
	element_data[position] = element;
	element_count++;
	if(own_elements) element->addOwner(this);
    }

    bool removeAt(int position)
    {
	if(!goodIndex(position, "removeAt")) {
	    return false;
	}
	else {
	    int i;
	    Type element = element_data[position];
	    for(i = position; i < element_count-1; i++) {
		element_data[i] = element_data[i+1];
	    }
	    element_count--;
	    if(own_elements) {
		// if this is the only occurrence of this object, unregister it.
		for(i = position; i < element_count; i++) {
		    if(element_data[i] == element) break;
		}
		if(i == element_count) element->removeOwner(this);
	    }
	    return true;
	}
    }

    void removeAll(void)
    {
	if(own_elements && element_count > 0)
	{
	    qsort(element_data,element_count,sizeof(Type),sortGVectorElements);
	    element_data[0]->removeOwner(this);
	    for(int i = 1; i < element_count; i++) {
		// unregister duplicates only once.
		if(element_data[i] != element_data[i-1]) {
		    element_data[i]->removeOwner(this);
		}
	    }
	}
	element_count = 0;
	capacity = capacity_increment;
	element_data = (Type *)realloc(element_data, capacity*sizeof(Type));
    }

    void set(Type element, int position)
    {
	if(!element || !goodIndex(position, "set")) return;

	if(element != element_data[position]) {
	    Type o = element_data[position];
	    element_data[position] = element;
	    if(own_elements) {
		int i;
		for(i = 0; i < element_count; i++) if(i != position) {
		    if(element_data[position] == element_data[i]) break;
		}
		// if this is the only occurrence, unregister it.
		if(i == element_count) o->removeOwner(this);
		element->addOwner(this);
	    }
	}
    }
    void exchange(int i, int j)
    {
	if(!goodIndex(i, "exchange") ||
		!goodIndex(j, "exchange")) return;
	Type o = element_data[i];
	element_data[i] = element_data[j];
	element_data[j] = o;
    }

    /** Move the indicated element to the first position.
     *  @param[in] element the element to move to the first position.
     */
    void promote(Type element)
    {
	int i;
	for(i = 0; i < element_count && element != element_data[i]; i++);
	if(i == element_count) {
	    cerr << "gvector.promote: element not found\n";
	    return;
	}
	Type o = element_data[i];

	for(int j = i; j > 0; j--) {
	    element_data[j] = element_data[j-1];
	}
	element_data[0] = o;
    }

    void load(gvector<Type> *v) {
	clear();
	ensureCapacity(v->size());
	for(int i = 0; i < v->size(); i++) add(v->at(i));
    }
    void load(gvector<Type> &v) {
	clear();
	ensureCapacity(v.size());
	for(int i = 0; i < v.size(); i++) add(v[i]);
    }

    /** Returns true if the element are owned by the gvector.
     *  @returns true if the gvector is an owner of the elements.
     */
    bool ownElements(void) { return own_elements; }

    /** Get the element at the position.
     *  @param[in] position the position of the element in the gvector.
     *  @returns the element.
     */
    Type at(int position) const { return element_data[position]; }

    /** Copy all elements into an array.
     *  @param[in] array an array of length length().
     */
    void copyInto(Type *array)
    {
	memcpy(array, element_data, element_count*sizeof(Type));
    }

    /** Get the size.
     *  @returns the number of elements in the gvector.
     */
    int size(void) const { return element_count; }

    /** 
     *  @returns true if the vector contains no elements.
     */
    bool empty(void) const { return (element_count <= 0); }

    /** Increase the capacity to the indicated min_capacity.
     *  @param[in] min_capacity increase capacity to min_capacity if it is less.
     */
    void ensureCapacity(int min_capacity)
    {
	if(min_capacity > capacity) {
	    capacity = min_capacity;
	    element_data = (Type *)realloc(element_data, capacity*sizeof(Type));
	}
    }

    bool contains(Type element) {
	int i;
	for(i = 0; i < element_count; i++) {
	    if(element_data[i] == element) return true;
	}
	return false;
    }
    int indexOf(Type element) {
	int i;
	for(i = 0; i < element_count; i++) {
	    if(element_data[i] == element) return i;
	}
	return -1;
    }
    void truncate(int n) {
	for(int i = element_count-1; i >= n && i >= 0; i--) removeAt(i);
    }

    /* The C language qsort function (which is used by the class sort
     * method) takes a C function pointer, so that is what needs to be
     * passed to sort method as well.
     */
    void sort(qsort_compar_ptr_t compar) {
	qsort(element_data, element_count, sizeof(Type), compar);
    }
    void sort(qsort_compar_ptr_t compar, int start, int num) {
	if(start < 0 || start >= element_count) {
	    cerr << "gvector.sort: invalid start.\n";
	}
	else if(num > element_count-start) {
	    cerr << "gvector.sort: invalid num.\n";
	}
	else {
	    qsort(element_data+start, num, sizeof(Type), compar);
	}
    }

    Type operator[](int i) { return at(i); }

    virtual bool nameIs(const string &s) {
	cerr << "Waring: gvector.nameIs() not redefined." << endl;
	 return "gvector"; }

    protected:
    //!<The incremental amount by which the storage is increased when necessary.
    int	capacity_increment;
    Type *element_data; //!< An array of Gobjects.
    int	element_count;  //!< The number of Gobjects in element_data.
    int	capacity;      //!< The current space allocated to element_data.
    bool own_elements;  //!< if true, the gvector owns the elements

    private:
    void init(int initial_capacity, int capacity_incre, bool element_owner)
    {
	if(initial_capacity <= 0) initial_capacity = 1;
	if(capacity_incre <= 0) capacity_incre = 1;
	element_data = (Type *)malloc(initial_capacity*sizeof(Type));
	capacity = initial_capacity;
	capacity_increment = capacity_incre;
	element_count = 0;
	own_elements = element_owner;
    }
    bool goodIndex(int position, const string &func)
    {
	if(position < 0) {
	    cerr << "gvector." << func << " warning: position < 0\n";
	    return false;
	}
	else if(position >= element_count) {
	    cerr << "gvector." << func << " warning: position=" << position
		<< " > element_count=" << element_count << "\n";
	    return false;
	}
	return true;
    }
};

/** An enumeration class for a gvector object.
 *  @ingroup libgobject
 */
template <class Type> class genumeration : public Gobject
{
    public:
    /** Constructor.
     *  @param[in] v the vector to enumerate.
     */
    genumeration(gvector<Type> *v) : ndex(0), vec(v) {}
    genumeration(const genumeration &g) : ndex(g.ndex), vec(NULL) {
	vec = new gvector<Type>(*g.vec);
    }
    genumeration & operator=(const genumeration &rhs) {
	if(this != &rhs) {
	    ndex = rhs.ndex;
	    vec = rhs.vec;
	}
	return *this;
    }
    ~genumeration(void) {}

    /** Check for more elements.
     *  @returns true if there are more Gobject elements in the enumeration.
     */
    bool hasMoreElements(void) {
	return (ndex < vec->size()) ? true : false;
    }

    /** Get the next element in the enumeration.
     *  @returns the next Gobject.
     */
    Type nextElement(void) {
	if(ndex < vec->size()) {
	    return vec->at(ndex++);
	}
	else {
	    cerr << "GEnumeration:nextElement warning: ndex=" << ndex
		<< " >=elementCount=" << vec->size() << endl;
	    return NULL;
	}
    }

    /** Get the index.
     *  @returns the index of the enumeration.
     */
    int index(void) { return ndex; }

    protected:
    int ndex; //!< The index of the enumeration.
    gvector<Type> *vec; //!< The gvector that is being enumerated.
};

#endif
