/** \file GObject.cpp
 *  \brief Defines class Gobject.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include "gobject++/Gobject.h"
extern "C" {
#include "logErrorMsg.h"
}

using namespace std;

/** Destructor */
Gobject::~Gobject(void)
{
    if((int)owners.size() > 0) {
	debug();
    }
    owners.clear();
}

void Gobject::debug(void)
{
    cerr << "warning: deleting a Gobject that has owners.\n Use deleteObject() instead." << endl;
}

/** Delete a Gobject. Delete the object if is has no owners.  If there are no
 *  Gobjects registered as owners of this object, i.e. hasOwners() returns
 *  false, then delete the object.
 */
void Gobject::deleteObject(void)
{
    if( ! hasOwners() ) {
	delete this;
    }
}

/** Get the registered state. Returns true if there object owners
 *  of this object. Returns false if there are no owners.
 *  @returns true if this object has one or more owners.
 */
bool Gobject::hasOwners(void)
{
    return ((int)owners.size() > 0) ? true : false;
}

/** Add an owner. This prevents the object from being deleted by calls to
 *  deleteObject(). The object can not be deleted until removeOwner has been
 *  called to remove all owners of this object. The owner can be itself. Calling
 *  addOwner more than once with the same input owner has no effect. An owner is
 *  only registered once and needs to be removed only once.
 *  @param[in] owner a Gobject that will be an owner of this Gobject.
 */
void Gobject::addOwner(Gobject *owner)
{
    if( !owner ) return;

    int i;
    /* don't register an owner twice */
    for(i = 0; i < (int)owners.size(); i++) {
	if(owners[i] == owner) break;
    }
    if(i == (int)owners.size()) {
	owners.push_back(owner);
    }
}

/** Remove an owner with optional delete. Remove an object as an owner of this
 *  object. If there are no remaining owners, delete this object only if
 *  do_delete is true.
 *  @param[in] owner a Gobject to be removed as an owner of this object.
 *  @param[in] do_delete delete the object only if do_delete is true.
 *  @returns true if there are no remaining owners and the object was deleted.
 *		returns false if there are still owners of this object and/or
 *		the input owner object was not actually an owner.
 */
bool Gobject::removeOwner(Gobject *owner, bool do_delete)
{
    bool is_owner = false;
    bool found = true;

    /* remove owner from this->owners
     */
    while(found) {
	found = false;
	for(int i = 0; i < (int)owners.size(); i++) {
	    if(owners[i] == owner) {
		owners.erase(owners.begin()+i);
		found = true;
		is_owner = true;
		break;
	    }
	}
    }

    if((int)owners.size() > 0) {
	return false; // another owner
    }

    if(is_owner && do_delete) {
	deleteObject();
	return true;
    }
    return false;
}

static char *err_msg = NULL;
static bool new_msg = false;
static bool print_stderr = true;

extern "C" {

void
setPrintError(bool print)
{
    print_stderr = print;
}

void
logErrorMsg(int priority, const char *msg)
{
    if(print_stderr) fprintf(stderr, "%s\n", msg);

    if(msg) {
	int len = (int)strlen(msg);
	Free(err_msg);
	err_msg = (char *)malloc(len+1);
	strcpy(err_msg, msg);
	new_msg = true;
    }
}

}

/** Return the last error message. Returns the last error message set
 *  by a call to logErrorMsg(). Returns NULL, if no error message has been
 *  set since the last call to errorMsg().
 *  @returns an error message set by logErrorMsg.
 */
char * Gobject::errorMsg(void)
{
    if( !new_msg ) return NULL;
    new_msg = false;
    return err_msg;
}

int sortGVectorElements(const void *A, const void *B)
{
    Gobject *a = *(Gobject **)A;
    Gobject *b = *(Gobject **)B;
    if(a == b) return 0;
    return (a < b) ? -1 : 1;
}
