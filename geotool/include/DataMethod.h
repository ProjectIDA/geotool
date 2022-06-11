#ifndef _DATA_METHOD_H
#define _DATA_METHOD_H

#include <iostream>
using namespace std;

#include "gobject++/Gobject.h"
#include "gobject++/gvector.h"

/** @defgroup libgmethod library libgmethod++
 *  C++ classes that operate on the data values in GTimeSeries objects.
 *  This library contains the DataMethod class, which is the virtual interface
 *  for subclasses that operate on GTimeSeries objects. The library also
 *  contains some subclasses of DataMethod for tapering, filtering,
 *  convolution, component rotation and other basic operations.
 */

class GTimeSeries;
class GSegment;

class AmpData;
class CalibData;
class ConvolveData;
class CopyData;
class CutData;
class Waveform;
class Demean;
class IIRFilter;
class OffsetData;
class RotateData;
class TaperData;

/** A virtual interface for subclasses that operate on GTimeSeries objects.
 *  A GTimeSeries object maintains an ordered list of all DataMethod instances
 *  that have been applied to its data. When required, the sequence of
 *  DataMethod operations can be reapplied to the raw waveform data. Individual
 *  DataMethod objects can be removed from the sequence of operations on a
 *  waveform. The parameters of a DataMethod object in the sequence can be
 *  changed. The DataMethod class handles the reapplication of the sequence of
 *  methods to the waveform.
 *  @ingroup libgmethod
 */
class DataMethod : public Gobject
{
    public:
	/** Destructor */
	virtual ~DataMethod(void) { }

	const char *methodName(void) { return method_name.c_str(); }
	GTimeSeries * apply(GTimeSeries *ts);
	bool apply(int num_waveforms, GTimeSeries **ts);
	bool apply(Waveform *w);
	bool apply(gvector<Waveform *> &wvec);

	bool sameType(DataMethod *dm) {
	    return !method_name.compare(dm->method_name); }

	/** Apply the DataMethod to the GTimeSeries. This function must be
	 *  implemented by the subclass.
	 *  @param[in] num_waveforms the number of GTimeSeries objects.
	 *  @param[in] ts an array of GTimeSeries objects.
	 */
	virtual bool applyMethod(int num_waveforms, GTimeSeries **ts) = 0;

	/** Apply the DataMethod to a GSegment. This function must be
	 *  implemented by the subclass.
	 *  @param[in] s a GSegment object.
	 */
	virtual void applyToSegment(GSegment *s) {
	    cerr << "DataMethod::applyToSegment(GSegment *s) not implemented\n";
	}

	/** Get a string representation of the DataMethod. The representation
 	 *  must be in the form "method_name: parameter1=value parameter2=value
	 *  ...", so the object can be created from the string. This function
	 * must be implemented by the subclass.
	 */
	virtual const char *toString(void) = 0;

	/** The clone function is required for all DataMethod subclasses.
	 */
	virtual Gobject *clone(void) = 0;

	/** Can the method be applied separately to individual segments.
	 */
	virtual bool canAppend(void) { return false; }

	/** Is the method commutative with component rotation
	 */
	virtual bool rotationCommutative(void) { return false; }

	/** Apply the method to a segment that will be appended without a gap.
	 */
	virtual void continueMethod(GSegment *s) { applyToSegment(s); }

	virtual AmpData *getAmpDataInstance(void) { return NULL; }
	virtual CalibData *getCalibDataInstance(void) { return NULL; }
	virtual CopyData *getCopyDataInstance(void) { return NULL; }
	virtual CutData *getCutDataInstance(void) { return NULL; }
	virtual Demean *getDemeanInstance(void) { return NULL; }
	virtual IIRFilter *getIIRFilterInstance(void) { return NULL; }
	virtual OffsetData *getOffsetDataInstance(void) { return NULL; }
	virtual RotateData *getRotateDataInstance(void) { return NULL; }
	virtual TaperData *getTaperDataInstance(void) { return NULL; }
	virtual ConvolveData *getConvolveDataInstance(void) { return NULL; }

        static bool applyMethods(int num_methods, DataMethod **dm,
			gvector<Waveform *> &wvec);
	/** Apply one or more methods to a Waveform object.
	 *  @param[in] num_methods the number of DataMethod objects in dm[].
	 *  @param[in] dm a sequence of DataMethod objects to apply.
	 *  @param[in] w a Waveform object with non-NULL GTimeSeries
 	 *  ts member.
	 *  @returns true if the method was successfully applied. Returns false
 	 *  if the method could not be applied.
	 */
        static bool applyMethods(int num_methods, DataMethod **dm,
			Waveform *w);
	static bool changeMethod(DataMethod *dm,
			gvector<Waveform *> &wvec);
	static bool changeMethod(DataMethod *dm, Waveform *w);
	static bool changeMethods(int num_methods, DataMethod **dm,
			gvector<Waveform *> &wvec);
	/** Change a sequence of DataMethod objects that has been applied to
	 *  the input waveform. The most recent (last) occurrence of a
 	 *  DataMethod sequence of objects is replaced for the input
	 *  GTimeSeries member of the Waveform object. All DataMethods
	 *  are then reapplied to the waveform.
	 *  @param[in] num_methods the number of DataMethod objects in dm[].
	 *  @param[in] dm a sequence of new DataMethod objects that will
	 *  replace the most recent occurence of the sequence of objects of
	 *  the same classes found in the hashtable of the GTimeSeries object.
	 *  @param[in] w a Waveform object with non-NULL GTimeSeries
	 *  member.
	 *  @returns true if the replacement was successful.
	 */
	static bool changeMethods(int num_methods, DataMethod **dm,
			Waveform *w);
	static bool changeMethod(DataMethod *dm, int num, GTimeSeries **ts);
	static bool changeMethods(int num_methods, DataMethod **dm,
                        int num_waveforms, GTimeSeries **ts);
	static bool remove(const char *method_name,
			gvector<Waveform *> &wvec);
	static bool remove(int num_methods, const char **method_name,
			gvector<Waveform *> &wvec);
	static bool remove(int num_methods, const char **method_name,
			Waveform *w);
	static bool removeMethod(const char *method_name, Waveform *w);
	static bool update(Waveform *w);

    protected:
	/** Constructor. The constructor is accessible only by a subclass.
	 */
	DataMethod(const string &name) : method_name(name), string_rep() { }
	DataMethod(const DataMethod &d): method_name(d.method_name),
		string_rep(d.string_rep)  { }
	DataMethod & operator=(const DataMethod &d) {
	    if(this != &d) {
		method_name = d.method_name;
		string_rep = d.string_rep;
	    }
	    return *this;
	}

	static bool doMethods(gvector<DataMethod *> *methods, int num,
			GTimeSeries **ts);

	string method_name;
	string string_rep; //!< The string representation of the method.

    private:

};

#endif
