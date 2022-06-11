/** \file DataMethod.cpp
 *  \brief Defines class DatMethod.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sstream>
#include <math.h>

#include "DataMethod.h"
#include "gobject++/DataSource.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "motif++/Component.h"

using namespace std;

/** Apply this method to one waveform. Apply the method to the input
 *  GTimeSeries object.
 *  @param[in] ts a GTimeSeries object.
 *  @returns a GTimeSeries instance with the method applied to the data. This
 *	pointer can be different from the input pointer and it should be used
 *	in subsequent references to the waveform data. Returns NULL if the
 *	method could not be applied to the waveform.
 */
GTimeSeries * DataMethod::apply(GTimeSeries *ts)
{
    GTimeSeries *ts_array[1];
    ts_array[0] = ts;
    if(apply(1, ts_array)) {
	return ts_array[0];
    }
    return NULL;
}

/** Apply this method to an array of GTimeSeries objects.
 *  @param[in] num_waveforms the number of GTimeSeries objects in ts[].
 *  @param[in] ts an array of GTimeSeries objects.
 *  @returns true if the method was successfully applied. Returns false if the
 *  method could not be applied to all waveforms.
 */
bool DataMethod::apply(int num_waveforms, GTimeSeries **ts)
{
    bool ret = applyMethod(num_waveforms, ts);
    if(ret) {
	// this method was successfully applied. Save it in ts.
	for(int i = 0; i < num_waveforms; i++) {
	    ts[i]->addDataMethod(this);
//	    Component::printLog("%x %s/%s: %s\n", ts[i],
//		ts[i]->sta(), ts[i]->chan(), toString());
	}
    }
    return ret;
}

/** Apply this method to one waveforms. Apply the method to the GTimeSeries
 *  member of the Waveform object.
 *  @param[in] w a Waveform with a non-NULL GTimeSeries member ts.
 *  @returns true if the method was successfully applied. Returns false if the
 *  method could not be applied to the waveform.
 */
bool DataMethod::apply(Waveform *w)
{
    return apply(w->ts) ? true : false;
}

/** Apply this method to an array of Waveform objects.
 *  @param[in] wvec an array of Waveform objects with non-NULL
 *	GTimeSeries ts members.
 *  @returns true if the method was successfully applied. Returns false if the
 *  method could not be applied to all waveforms.
 *  @throws GERROR_MALLOC_ERROR
 */
bool DataMethod::apply(gvector<Waveform *> &wvec)
{
    GTimeSeries **ts = (GTimeSeries **)malloc(
				wvec.size()*sizeof(GTimeSeries *));
    if( !ts ) {
	GError::setMessage("DataMethod.apply: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }
    for(int i = 0; i < wvec.size(); i++) {
	ts[i] = wvec[i]->ts;
    }
    bool ret = applyMethod(wvec.size(), ts);
    if(ret) {
	// this method was successfully applied. Save it in ts.
	for(int i = 0; i < wvec.size(); i++) {
	    ts[i]->addDataMethod(this);
	    wvec[i]->ts = ts[i];
//	    Component::printLog("%x %s/%s: %s\n", ts[i],
//		ts[i]->sta(), ts[i]->chan(), toString());
	}
    }
    Free(ts);
    return ret;
}

// static
/** Apply one or more methods to an array of Waveform objects.
 *  @param[in] dm a sequence of DataMethod objects to apply.
 *  @param[in] wvec an array of Waveform objects with non-NULL
 *	GTimeSeries ts members.
 *  @returns true if the method was successfully applied. Returns false if the
 *  method could not be applied to all waveforms.
 *  @throws GERROR_MALLOC_ERROR
 */
bool DataMethod::applyMethods(int num_methods, DataMethod **dm,
			gvector<Waveform *> &wvec)
{
    int i, j;
    GTimeSeries **ts = NULL;

    ts = (GTimeSeries **)malloc(wvec.size()*sizeof(GTimeSeries *));
    if( !ts ) {
	GError::setMessage("DataMethod.applyMethods: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }
    for(i = 0; i < wvec.size(); i++) ts[i] = wvec[i]->ts;

    for(j = 0; j < num_methods; j++)
    {
	if(!dm[j]->apply(wvec.size(), ts)) {
	    Free(ts);
	    return false;
	}
    }
    for(i = 0; i < wvec.size(); i++) {
	wvec[i]->ts = ts[i];
    }
    Free(ts);
    return true;
}

// static
/** Change a DataMethod that has been applied to the input waveforms. The last
 *  occurrence of a DataMethod of the same class as the input dm is replaced for
 *  all input GTimeSeries members of the Waveform objects. All DataMethods
 *  are then reapplied to each waveform.
 *  @param[in] dm the new DataMethod that will replace the last DataMethod
 *	instance of the same class found in the hashtable of each GTimeSeries
 *	object.
 *  @param[in] num the number of Waveform objects in wvec[].
 *  @param[in] wvec an array of Waveform objects with non-NULL
 * 	GTimeSeries member objects.
 *  @returns true if the replacement was successful.
 */
bool DataMethod::changeMethod(DataMethod *dm, gvector<Waveform *> &wvec)
{
    return changeMethods(1, &dm, wvec);
}

// static
/** Change a sequence of DataMethod objects that has been applied to the input
 *  waveforms. The most recent (last) occurrence of a DataMethod sequence of
 *  objects is replaced for all input GTimeSeries members of the Waveform
 *  objects. All DataMethods are then reapplied to each waveform.
 *  @param[in] num_methods the number of DataMethod objects in dm[].
 *  @param[in] dm a sequence of new DataMethod objects that will replace the
 *  most recent occurence of the sequence of objects of the same classes found
 *	in the hashtable of each GTimeSeries object.
 *  @param[in] wvec an array of Waveform objects with non-NULL
 * 	GTimeSeries member objects.
 *  @returns true if the replacement was successful.
 *  @throws GERROR_MALLOC_ERROR
 */
bool DataMethod::changeMethods(int num_methods, DataMethod **dm,
			gvector<Waveform *> &wvec)
{
    GTimeSeries **ts = NULL;

    if(num_methods <= 0) return true;

    ts = (GTimeSeries **)malloc(wvec.size()*sizeof(GTimeSeries *));
    if( !ts ) {
	GError::setMessage("DataMethod.changeMethods: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }
    for(int i = 0; i < wvec.size(); i++) {
	ts[i] = wvec[i]->ts;
    }

    bool ret = changeMethods(num_methods, dm, wvec.size(), ts);

    for(int i = 0; i < wvec.size(); i++) {
	wvec[i]->ts = ts[i];
    }
    Free(ts);
    return ret;
}

// static
/** Change a DataMethod that has been applied to the input waveforms. The last
 *  occurrence of a DataMethod of the same class as the input dm is replaced for
 *  all input GTimeSeries objects. All DataMethods are then reapplied to each
 *  waveform.
 *  @param[in] dm the new DataMethod that will replace the last DataMethod
 *	instance of the same class found in the hashtable of each GTimeSeries
 *	object.
 *  @param[in] ts an array of GTimeSeries objects
 *  @returns true if the replacement was successful.
 */
bool DataMethod::changeMethod(DataMethod *dm, int num_waveforms,
			GTimeSeries **ts)
{
    return changeMethods(1, &dm, num_waveforms, ts);
}

// static
/** Change a sequence of DataMethod objects that has been applied to the input
 *  waveforms. The most recent (last) occurrence of a DataMethod sequence of
 *  objects is replaced for all input GTimeSeries objects. All DataMethods are
 *  then reapplied to each waveform.
 *  @param[in] num_methods the number of DataMethod objects in dm[].
 *  @param[in] dm a sequence of new DataMethod objects that will replace the
 *  most recent occurence of the sequence of objects of the same classes found
 *	in the hashtable of each GTimeSeries object.
 *  @param[in] num_waveforms the number of GTimeSeries objects in ts[].
 *  @param[in] ts an array of GTimeSeries objects
 *  @returns true if the replacement was successful.
 */
bool DataMethod::changeMethods(int num_methods, DataMethod **dm,
			int num_waveforms, GTimeSeries **ts)
{
    int i, j, k, num_same;
    gvector<DataMethod *> *methods = NULL;

    if(num_waveforms <= 0) return true;

    for(i = num_same = 0; i < num_waveforms; i++)
    {
	methods = ts[i]->dataMethods();
	/* look for the most recent sequential occurrence of num_methods
	 */
	for(j = methods->size() - num_methods; j >= 0; j--)
	{
	    for(k=0; k < num_methods && dm[k]->sameType(methods->at(j+k)); k++);

	    if(k == num_methods)    /* found all num_methods */
	    {
		for(k = 0; k < num_methods; k++)
		{
		    if( dm[k] == methods->at(j+k) )
		    {
			num_same++;  /* already applied */
		    }
		    methods->set(dm[k], j+k);
		}
		break;
	    }
	}
	if(j < 0) { /* methods not found. add them */
	    for(k = 0; k < num_methods; k++) {
		methods->add(dm[k]);
	    }
	}
	ts[i]->setDataMethods(methods);
	delete methods;
    }
    if(num_same < num_methods*num_waveforms)
    {
	for(i = 0; i < num_waveforms; i++) {
	    if( !ts[i]->reread() ) return false;
	}
	methods = ts[0]->dataMethods();
	// assumes that the methods vector is the same for all ts.
	if(!doMethods(methods, num_waveforms, ts)) {
	    delete methods;
	    return false;
	}
	delete methods;
    }
    return true;
}

// static
/** Remove a method from the sequence of operations on the input waveforms.
 *  The most recent (last) occurrence of a DataMethod of the class indicated
 *  is removed from each input waveform's sequence of DataMethod objects.
 *  @param[in] method_name the DataMethod name.
 *  @param[in] wvec an array of Waveform objects with non-NULL
 *	GTimeSeries ts members.
 *  @returns true if the method was successfully removed. Returns false if the
 *  method could not be removed from all waveforms.
 */
bool DataMethod::remove(const char *method_name,
			gvector<Waveform *> &wvec)
{
    return remove(1, &method_name, wvec);
}

// static
/** Remove a sequence of methods from the method lists of the input waveforms.
 *  The most recent (last) occurrence of a sequence of DataMethod types of is
 *  removed from each input waveform's list of DataMethod objects.
 *  @param[in] num_methods the number of DataMethod class names in
 *	method_names[].
 *  @param[in] method_names an array of DataMethod class names.
 *  @param[in] wvec an array of Waveform objects with non-NULL
 *	GTimeSeries ts members.
 *  @returns true if the methods were successfully removed. Returns false if the
 *  methods could not be removed from all waveforms.
 *  @throws GERROR_MALLOC_ERROR
 */
bool DataMethod::remove(int num_methods, const char **method_names,
			gvector<Waveform *> &wvec)
{
    int	i, j, k;
    bool found_method;
    GTimeSeries **ts = NULL;
    gvector<DataMethod *> *methods = NULL;

    if(wvec.size() <= 0 || num_methods <= 0) return true;

    ts = (GTimeSeries **)malloc(wvec.size()*sizeof(GTimeSeries *));
    if( !ts ) {
	GError::setMessage("DataMethod.remove: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }
    for(i = 0; i < wvec.size(); i++) ts[i] = wvec[i]->ts;

    found_method = false;
    for(i = 0; i < wvec.size(); i++)
    {
	methods = wvec[i]->dataMethods();

	k = num_methods - 1;
	for(j = (int)methods->size()-1; j >= 0; j--) {
	    if(!strcmp(methods->at(j)->methodName(), method_names[k])) {
		methods->removeAt(j);
		found_method = true;
		break;
	    }
	}
	while(--k >= 0 && --j >= 0 && 
		!strcmp(methods->at(j)->methodName(), method_names[k]))
	{
	    methods->removeAt(j);
	}
	wvec[i]->setDataMethods(methods);
	delete methods;
    }
    if(found_method) {
	for(i = 0; i < wvec.size(); i++) {
	    if( !ts[i]->reread() ) {
		Free(ts);
		return false;
	    }
	}
	methods = ts[0]->dataMethods();
	// assumes that the methods vector is the same for all ts.
	if(!doMethods(methods, wvec.size(), ts)) {
	    Free(ts);
	    delete methods;
	    return false;
	}
	delete methods;
	for(i = 0; i < wvec.size(); i++) {
	    wvec[i]->ts = ts[i];
	}
    }
    Free(ts);
    return found_method;
}

// static
/** Reapply all DataMethod instances. Reapply all of the DataMethod instances
 *  from the method list of the GTimeSeries member of the input Waveform
 *  object. The raw data is re-read prior to reappling the methods.
 *  @param[in] w a Waveform object with a non-NULL GTimeSeries member.
 *  @returns true if the methods were successfully reapplied.
 */
bool DataMethod::update(Waveform *w)
{
    if( !w->reread() ) return false;

    gvector<DataMethod *> *methods = w->dataMethods();
    bool ret = doMethods(methods, 1, &w->ts);
    delete methods;
    return ret;
}

// static
/** Apply methods to an array of GTimeSeries.
 *  @param[in] methods a vector of DataMethod objects.
 *  @param[in] num the number of GTimeSeries objects in ts[].
 *  @param[in] ts an array of GTimeSeries objects.
 */
bool DataMethod::doMethods(gvector<DataMethod *> *methods, int num,
		GTimeSeries **ts)
{
    for(int i = 0; i < (int)methods->size(); i++) {
	if(!methods->at(i)->applyMethod(num, ts))
	{
	    return false;
	}
	for(int j = 0; j < num; j++) {
//	    Component::printLog("%x %s/%s: %s\n", ts[j], ts[j]->sta(),
//			ts[j]->chan(), methods->at(i)->toString());
	}
    }
    return true;
}

// static
bool DataMethod::applyMethods(int num_methods, DataMethod **dm, Waveform *w)
{
    gvector<Waveform *> v;
    v.push_back(w);
    return applyMethods(num_methods, dm, v);
}

// static
bool DataMethod::changeMethod(DataMethod *dm, Waveform *w)
{
    gvector<Waveform *> v;
    v.push_back(w);
    return changeMethod(dm, v);
}

// static
bool DataMethod::changeMethods(int num_methods, DataMethod **dm, Waveform *w)
{
    gvector<Waveform *> v;
    v.push_back(w);
    return changeMethods(num_methods, dm, v);
}

// static
bool DataMethod::remove(int num_methods, const char **method_name, Waveform *w)
{
    gvector<Waveform *> wvec;
    wvec.push_back(w);
    return remove(num_methods, method_name, wvec);
}
