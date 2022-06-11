#ifndef _C_VECTOR_H_
#define _C_VECTOR_H_

#include "gobject++/CssTableClass.h"

// template class for CssTableClass objects: cvector<CssArrival> v, etc.

template <class Type> class cvector : public gvector<CssTableClass *>
{
    public:
    cvector(void) { }
    cvector(int initial_capacity) : gvector<CssTableClass *>(initial_capacity) { }
    cvector(bool element_owner) : gvector<CssTableClass *>(element_owner) { }
    cvector(const cvector<Type> &v) :
	gvector<CssTableClass *>((const gvector<CssTableClass *> &)v) { }
    cvector(Type element) : gvector<CssTableClass *>((CssTableClass *)element) { }

    int sortByMember(const string &member_name) {
	return CssTableClass::sort(*this, member_name);
    }
    Type * find(const string &member_name, long value) {
	return (Type *)CssTableClass::find(*this, member_name, value);
    }
    Type *front(void) {
	return goodIndex(0, "front") ? (Type *)element_data[0] : NULL; }
    Type *back(void) { return goodIndex(element_count-1, "back") ?
			(Type *)element_data[element_count-1] : NULL; }
    Type * at(int position) const { return (Type *)element_data[position]; }

    void copyInto(Type **array) {
	memcpy((void *)array,(void *)element_data,element_count*sizeof(Type *));
    }
    void load(cvector<Type> *v) {
	clear();
	ensureCapacity(v->size());
	for(int i = 0; i < v->size(); i++) add(v->at(i));
    }
    void load(cvector<Type> &v) {
	clear();
	ensureCapacity(v.size());
	for(int i = 0; i < v.size(); i++) add(v[i]);
    }

    Type * operator[](int i) { return at(i); }

    bool nameIs(const string &s) {
	Type o;
	return o.nameIs(s);
    }
};

#endif
