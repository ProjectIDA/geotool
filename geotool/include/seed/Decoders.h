/** \file Decoders.h
 *  \brief Declares the Decoders class methods.
 *  \author Ivan Henson
 */
#ifndef _DECODERS_H_
#define _DECODERS_H_

#include "seed/Seed.h"
#include "seed/SeedTime.h"

/** 
 * SEED Decompression methods.
 *
 *@see Blockette1000
 */
class Decoders
{
    public:
	static int decode(char format, const char *bytes, int nbytes, int *wo,
			int *so, int nsamples, int *data);
	static int decode(char format, const char *bytes, int nbytes, int *wo,
			int *so, int nsamples, float *data);
	static int steim1(const char *bytes, int nbytes, int *wo, int *so,
			int nsamples, float *data);
	static int steim2(const char *bytes, int nbytes, int *wo, int *so,
			int nsamples, float *data);
	static int data16(const char *bytes, int nbytes, int *so, int nsamples,
			int *data);
	static int data24(const char *bytes, int nbytes, int *wo, int nsamples,
			int *data);
	static int data32(const char *bytes, int nbytes, int *wo, int nsamples,
			int *data);
	static int fdata32(const char *bytes, int nbytes, int *wo, int nsamples,
			float *data);
};

#endif
