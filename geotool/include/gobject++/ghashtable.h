#ifndef _GHASHTABLE_H_
#define _GHASHTABLE_H_

#include <utility>
#include <string>
#include <vector>
using namespace std;

#include "gobject++/Gobject.h"

template <typename Type> class ghashtable : public Gobject
{
    public:
	ghashtable(void) : elements() { }
	ghashtable(const ghashtable<Type> &g) : elements() { copy(g); }
	ghashtable<Type> & operator=(const ghashtable<Type> &rhs) {
	    if(this != &rhs) {
		clear();
		copy(rhs);
	    }
	    return *this;
	}
	~ghashtable(void) {
	    clear();
	}
	bool get(const string &key, Type *value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    *value = elements[i]->second;
		    return true;
		}
	    }
	    return false;
	}
	void put(const string &key, Type value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    elements[i]->second = value;
		    return;
		}
	    }
	    elements.push_back(new pair<string *,Type>(new string(key), value));
	}
	void remove(const string &key) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    delete elements[i]->first; // the key string
		    delete elements[i];
		    elements.erase(elements.begin() + i);
		    return;
		}
	    }
	}
	void copy(const ghashtable<Type> &h) {
	    for(int i = 0; i < (int)h.elements.size(); i++) {
		elements.push_back(new pair<string *,Type>(
			new string(*h.elements[i]->first),
			h.elements[i]->second));
	    }
	}
	void clear(void) {
	    for(int i = (int)elements.size()-1; i >= 0; i--) {
		remove(elements[i]->first->c_str());
	    }
	}
	vector< pair<string *, Type> * > elements;
};

template <>
class ghashtable<string *> : public Gobject
{
    public:
        ghashtable(void) : elements() { }
	ghashtable(const ghashtable &g) : elements() { copy(g); }
	ghashtable & operator=(const ghashtable &rhs) {
	    if(this != &rhs) {
		clear();
		copy(rhs);
	    }
	    return *this;
	}
	~ghashtable(void) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		delete elements[i]->first;
		delete elements[i]->second;
		delete elements[i];
	    }
	}
	bool get(const string &key, string **value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    *value = elements[i]->second;
		    return true;
		}
	    }
	    return false;
	}
	void put(const string &key, string *value) {
	    put(key, *value);
	}
	void put(const string &key, const string &value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    if(elements[i]->second->compare(value)) {
			delete elements[i]->second;
			elements[i]->second = new string(value);
		    }
		    return;
		}
	    }
	    elements.push_back(new pair<string *,string *>(
			new string(key), new string(value)));
	}
	void remove(const string &key) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    delete elements[i]->first; //  the key string
		    delete elements[i]->second;
		    delete elements[i];
		    elements.erase(elements.begin() + i);
		}
	    }
	}
	void copy(const ghashtable &h) {
	    for(int i = 0; i < (int)h.elements.size(); i++) {
		elements.push_back(new pair<string *, string *>(
			new string(*h.elements[i]->first),
			new string(*h.elements[i]->second)));
	    }
	}
	void clear(void) {
	    for(int i = (int)elements.size()-1; i >= 0; i--) {
		remove(elements[i]->first->c_str());
	    }
	}
        vector< pair<string *, string *> * > elements;
};

template <>
class ghashtable<Gobject *> : public Gobject
{
    public:
        ghashtable(void) : elements() { }
	ghashtable(const ghashtable &g) : elements() { copy(g); }
	ghashtable & operator=(const ghashtable &rhs) {
	    if(this != &rhs) {
		clear();
		copy(rhs);
	    }
	    return *this;
	}
	~ghashtable(void) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		delete elements[i]->first;
		elements[i]->second->removeOwner(this);
		delete elements[i];
	    }
	}
	void put(const string &key, Gobject* value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    if(value != elements[i]->second) {
			value->addOwner(this);
			elements[i]->second->removeOwner(this);
			elements[i]->second = value;
		    }
		    return;
		}
	    }
	    elements.push_back(
		new pair<string *, Gobject *>(new string(key), value));
	    value->addOwner(this);
	}
	bool get(const string &key, Gobject* *value) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    *value = elements[i]->second;
		    return true;
		}
	    }
	    return false;
	}
	void remove(const string &key) {
	    for(int i = 0; i < (int)elements.size(); i++) {
		if( !elements[i]->first->compare(key) ) {
		    delete elements[i]->first; //  the key string
		    elements[i]->second->removeOwner(this);
		    delete elements[i];
		    elements.erase(elements.begin() + i);
		}
	    }
	}
	void copy(const ghashtable &h) {
	    for(int i = 0; i < (int)h.elements.size(); i++) {
		Gobject *g = h.elements[i]->second->clone();
		g->addOwner(this);
		elements.push_back(new pair<string *,Gobject *>(
		new string(h.elements[i]->first->c_str()), g));
	    }
	}
	void clear(void) {
	    for(int i = (int)elements.size()-1; i >= 0; i--) {
		remove(elements[i]->first->c_str());
	    }
	}
        vector< pair<string *, Gobject *> * > elements;
};

#endif
