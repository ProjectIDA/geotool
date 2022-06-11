/** \file ParamDialog.cpp
 *  \brief Defines class ParamDialog.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
using namespace std;

#include "motif++/ParamDialog.h"
#include "motif++/Toggle.h"
#include "motif++/TextField.h"
#include "motif++/MotifComp.h"


/** Constructor. Only accessible from a subclass.
 *  @param[in] name the name given to this ParamDialog instance.
 *  @param[in] parent the Component parent.
 */
ParamDialog::ParamDialog(const string &name, Component *parent) :
			FormDialog(name, parent, false, false)
{
}

/** Destructor */
ParamDialog::~ParamDialog(void)
{
}

/** Get a double value.
 *  @param[in] name the name of the TextField
 *  @param[out] d the double value from the TextField
 *  @returns true if the text is a valid double; false if the TextField is
 * 	empty or the text cannot be parsed as a double value.
 */
bool ParamDialog::getDouble(const string &name, double *d)
{
    TextField *t = findTextField(name);
    if(t) {
	return t->getDouble(d);
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return false;
}

/** Get an int value.
 *  @param[in] name the name of the TextField
 *  @param[out] n the int value from the TextField
 *  @returns true if the text is a valid int; false if the TextField is
 * 	empty or the text cannot be parsed as an int value.
 */
bool ParamDialog::getInt(const string &name, int *n)
{
    TextField *t = findTextField(name);
    if(t) {
	return t->getInt(n);
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return false;
}

/** Get a bool value.
 *  @param[in] name the name of the TextField
 *  @param[out] b the bool value from the TextField
 *  @returns true if the text is a valid bool; false if the TextField is
 * 	empty or the text cannot be parsed as a bool value.
 */
bool ParamDialog::getBool(const string &name, bool *b)
{
    Toggle *t = findToggle(name);
    if(t) {
	*b = t->state();
	return true;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a Toggle named " << name << endl;
    return false;
}

/** Get a string value.
 *  @param[in] name the name of the TextField
 *  @param[out] s the string value from the TextField. Free the pointer *s when
 *  	it is no longer needed.
 *  @returns true if the text contains any non-white characters.
 */
bool ParamDialog::getString(const string &name, char **s)
{
    TextField *t = findTextField(name);
    if(t) {
	*s = t->getString();
	return true;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return false;
}

/** Get a string value.
 *  @param[in] name the name of the TextField
 *  @param[out] s the string value from the TextField, up to len characters.
 *  @param[in] len the available length of s
 *  @returns true if the text contains any non-white characters.
 */
bool ParamDialog::getString(const string &name, char *s, int len)
{
    TextField *t = findTextField(name);
    if(t) {
	char *c = t->getString();
	snprintf(s, len, "%s", c);
	Free(c);
	return true;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return false;
}

/** Set a string value, do callback.
 *  @param[in] name the name of the TextField
 *  @param[in] s the string value.
 *  @param[in] do_callback if true, do XmNvalueChangedCallback callbacks.
 *  @returns true if name is a TextField child; returns false if a TextField
 * 	child with that name cannot be found.
 */
bool ParamDialog::setString(const string &name, const string &s, bool do_callback)
{
    TextField *t = findTextField(name);
    if(t) {
	t->setString(s, do_callback);
	return true;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return false;
}

/** Set a TextField child.
 *  @param[in] name the name of the TextField
 *  @returns the TextField child or NULL is none exists with the input name.
 */
TextField *ParamDialog::getTextField(const string &name)
{
    TextField *t = findTextField(name);
    if(t) {
	return t;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a TextField named " << name << endl;
    return NULL;
}

/** Set a Toggle child.
 *  @param[in] name the name of the Toggle
 *  @returns the Toggle child or NULL is none exists with the input name.
 */
Toggle *ParamDialog::getToggle(const string &name)
{
    Toggle *t = findToggle(name);
    if(t) {
	return t;
    }
    cerr << "Warning: ParamDialog " << component_name <<
                " does not contain a Toggle named " << name << endl;
    return NULL;
}

/** Set a Label string.
 *  @param[in] name the name of the Label
 *  @param[in] s the string value.
 *  @returns true if name is a Label child; returns false if no Label
 * 	child can be found by that name.
 */
bool ParamDialog::setLabel(const string &name, const string &s)
{
    Label *label = findLabel(name);
    if(label) {
	label->setLabel(s);
	return true;
    }
    return false;
}
