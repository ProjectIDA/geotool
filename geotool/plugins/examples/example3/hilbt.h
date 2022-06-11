 #ifndef __HILBT_H_
 #define __HILBT_H_
 
 #include "DataMethod.h"
 
 class GTimeSeries;
 class GSegment;
 
 class hilbert : public DataMethod
 {
     public:
 	hilbert(void);
 	~hilbert(void);
 
 	Gobject *clone();
 	const char *toString(void);
 	bool applyMethod(int num_waveforms, GTimeSeries **ts);
 
     protected:
 	static void applySegment(GSegment *s);
 };
 
 #endif
