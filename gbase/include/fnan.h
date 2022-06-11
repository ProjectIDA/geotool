/*
 * like nan.h for floats (IEEE format only)
 */
#ifndef _f_nan_
#define _f_nan_

#define set_fnan(x) {unsigned int bad_f=0xffffffff; memcpy(&(x), &bad_f, sizeof(int));}
#define fNaN(x) (x == 0xffffffff)

#endif
