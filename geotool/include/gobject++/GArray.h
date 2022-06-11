#ifndef _GARRAY_H
#define _GARRAY_H

#include "gobject++/Gobject.h"

/** A class for holding 2-D arrays. This class holds a two-dimensional
 *  array of data values that represent multiple time series that have the
 *  same beginning time, sample interval and duration. The time series data
 *  are store in array data[size][data_length], where size is the number of
 *  time series and data_length is the length of each series.
 *  @ingroup libgobject
 */
class GArray : public Gobject
{
    public:
	GArray(int length, double tbeg, double tdel);
	GArray(GArray &g) : data(NULL), data_length(0), num(0), beg(0.),
		end(0.), del(0.) { copy(g); }
        GArray & operator=(const GArray &rhs) {
	    if(this != &rhs) {
		for(int i = 0; i < num; i++) free(data[i]);
		free(data);
		copy(rhs);
	    }
	    return *this;
	}

	~GArray(void);
	Gobject *clone(void);

	void add(float *f);
	/** Get the beginning time.
	 *  @returns the beginning epochal time.
	 */
	double tbeg(void) { return beg; }
	/** Get the end time.
	 *  @returns the end epochal time.
	 */
	double tend(void) { return end; }
	/** Get the sample time interval.
	 *  @returns the end epochal time.
	 */
	double tdel(void) { return del; }
	/** Get the number of samples.
	 *  @returns the number of samples.
	 */
	int length(void) { return data_length; }
	/** Get the number of time series.
	 *  @returns the number of time series.
	 */
	int size(void) { return num; }

	float **data; //<! A two dimensional array data[size][data_length].

    protected:
	int	data_length; //!< The length of each time series.
	int	num;  //!< The number of time series.
	double	beg;  //!< The beginning time of the data.
	double	end;  //!< The ending time of the data.
	double	del;  //!< The sample time interval of the data.

    private:
	void copy(const GArray &g)
	{
	    data = NULL;
	    num = g.num;
	    data_length = g.data_length;
	    beg = g.beg;
	    end = g.end;
	    del = g.del;
	    if(num > 0) {
		data = (float **)malloc(sizeof(float *));
		for(int i = 0; i < num; i++) {
		    data[i] = (float *)malloc(data_length*sizeof(float));
		    memcpy(data[i], g.data[i], data_length*sizeof(float));
		}
	    }
	}
};

#endif

