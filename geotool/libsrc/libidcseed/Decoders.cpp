/** \file Decoders.cpp
 *  \brief Defines data decoding methods.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <seed/Decoders.h>
#include <seed/Seed.h>
#include "seed/ByteOrder.h"
using namespace std;

/**
 * SEED Decompression methods.
 * 
 * @see SeedInputStream
 */

int Decoders::decode(const char format, const char *bytes, int nbytes,
			int *wo, int *so, int nsamples, int *data)
{
    int num;
    float *fdata = (float *)data;

    switch(format) {
	case 1:
	return data16(bytes, nbytes, so, nsamples, data);

	case 2:
	return data24(bytes, nbytes, wo, nsamples, data);

	case 3:
	return data32(bytes, nbytes, wo, nsamples, data);

	case 4:
	num = fdata32(bytes, nbytes, wo, nsamples, fdata);
	for(int i = 0; i < num; i++) data[i] = (int)fdata[i];
	return num;

	case 10:
	num = steim1(bytes, nbytes, wo, so, nsamples, fdata);
	for(int i = 0; i < num; i++) data[i] = (int)fdata[i];
	return num;
	break;

	case 11:
	num =steim2(bytes, nbytes, wo, so, nsamples, fdata);
	for(int i = 0; i < num; i++) data[i] = (int)fdata[i];
	return num;
	break;

	default:
	cerr << "Cannot decompress format: " << (int)format << endl;
    }
    return 0;
}

int Decoders::decode(const char format, const char *bytes, int nbytes,
			int *wo, int *so, int nsamples, float *data)
{
    int num = 0;
    int *idata = (int *)data;

    switch(format) {
	case 1:
	case 2:
	case 3:
	case 10:
	case 11:
	num = decode(format, bytes, nbytes, wo, so, nsamples, idata);
	for(int i = 0; i < num; i++) data[i] = (float)idata[i];
	return num;
	break;

	case 4:
	return fdata32(bytes, nbytes, wo, nsamples, data);

	default:
	cerr << "Connot decompress format: " << (int)format << endl;
    }
    return 0;
}

/** Decompress a steim1 byte array into a float array.
 * Steim, J. M. (1986).  The Very-Broad-Band Seismograph.  Doctoral
 * thesis, Department of Geological Sciences, Harvard 
 * University, Cambridge, Massachusetts.  184 pp.
 * Translated from Dennis O'Neill's C routine.
 *
 *@param bytes input compressed byte array.
 *@param nsamples the expected number of data samples.
 *@param data output float array containing nsamples data values.
 */
int Decoders::steim1(const char *bytes, int nbytes, int *wo, int *so,
			int nsamples, float *data)
{
    int i, j, k, l, b, initial_value, temp;
    int compression_int, counter, num_frames;
    int compression_flag[16];
    LONG u;
    WORD w;
    double sample;		
    double last_value; // last value from previous block

    last_value = 0;
    // recover this block's initial and final values from 1st frame
    for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[4+b];
    initial_value = u.i;
//    for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[8+b];
//    final_value = u.i;

    // num_frames = total record length - header length / # bytes per frame
    // num_frames = (lrecl - bod) / (elements/frame * size_of_element)
    // elements/frame = 16
    num_frames = nbytes/(16 * 4); 

    counter = 0;
    for(i = l = 0; i < num_frames; i++) { // process each frame
	// get decompression flags
	for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[l+b];
	compression_int = u.i;

	for(j = 15; j >= 0; j--) {
	    // mask for bottom 2 bits
	    compression_flag[j] = compression_int & 0x00000003;
	    compression_int >>= 2; // 2 bits per decompress flag
	}
	l += 4;

	for(j = 1; j < 16; j++) { // decompress each frame

	    switch(compression_flag[j]) {

		case 1: // type 1, 4 differences

		for(k = 0; k < 4; k++, l++) {
		    temp = (int)bytes[l];
		    if(temp > 0x7f) temp -= 2 * (0x7f + 1);

		    if((i == 0) && (j == 3) && (k == 0)) {
			last_value = initial_value - temp;
		    }
		    sample = (double) temp + last_value;
		    if(counter < nsamples) data[counter] = (float)sample;
		    last_value = sample;
		    counter++;
		}
		break;

		case 2:	// type 2, 2 differences

		for(k = 0; k < 2; k++, l += 2) {
		    w.a[so[0]] = bytes[l];
		    w.a[so[1]] = bytes[l+1];
		    temp = (int)w.s;
		    if(temp > 0x7fff) temp -= 2 * (0x7fff + 1);

		    if((i == 0) && (j == 3) && (k == 0)) {
			last_value = initial_value - temp;
		    }
		    sample = (double) temp + last_value;
		    if(counter < nsamples) data[counter] = (float)sample;
		    last_value = sample;
		    counter++;
		}
		break;

		case 3: // type 3, 1 differences

		for(k = 0; k < 1; k++, l += 4) {
		    for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[l+b];
		    temp = u.i;
		    if(temp > 0x7fffffff) {
			temp -= 2 * ((unsigned int)0x7fffffff + 1);
		    }

		    if((i == 0) && (j == 3) && (k == 0)) {
			last_value = initial_value - temp;
		    }
		    sample = (double)temp + last_value;
		    if(counter < nsamples) data[counter] = (float)sample;
		    last_value = sample;
		    counter++;
		}
		break;

		case 0:	 // type 0, not data
		default:
		    l += 4;
		    break;
	    } // switch loop
	} // single frame for loop
    }   // all frames for loop

    for(i = counter; i < nsamples; i++) data[i] = 0.;

    if(nsamples > counter) {
	cerr << "Steim Decompress Sample Count Error." << endl;
    }
    return counter;
}

/** Decompress a steim2 byte array into a float array.
 * Steim, J. M. (1986).  The Very-Broad-Band Seismograph.  Doctoral
 * thesis, Department of Geological Sciences, Harvard 
 * University, Cambridge, Massachusetts.  184 pp.
 * Translated from Dennis O'Neill's C routine.
 *
 *@param bytes input compressed byte array.
 *@param nsamples the expected number of data samples.
 *@param data output float array containing nsamples data values.
 */
int Decoders::steim2(const char *bytes, int nbytes, int *wo, int *so,
			int nsamples, float *data)
{
    int i, j, k, l, b, initial_value;
    int temp, temp4;
    int compression_int, counter, num_frames;
    int compression_flag[16];
    LONG u;
    double sample;
    double last_value;  // last value from previous block

    last_value = 0;
    // recover this block's initial and final values from 1st frame
    for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[4+b];
    initial_value = u.i;
//    for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[8+b];
//    final_value = u.i;
   num_frames = nbytes/(16 * 4); 

    counter = 0;
    for(i = l = 0; i < num_frames; i++) { // process each frame
	// get decompression flags
	for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[l+b];
	compression_int = u.i;

	for(j = 15; j >= 0; j--) {
	    // mask for bottom 2 bits
	    compression_flag[j] = compression_int & 0x00000003;
	    compression_int >>= 2; // 2 bits per decompress flag
	}
	l += 4;

	for(j = 1; j < 16; j++) { // decompress each frame 

	    switch (compression_flag[j]) {

		case 1:		// type 1, 4 differences

		for(k = 0; k < 4; k++, l++) {
		    temp = (int)bytes[l];
		    if (temp > 0x7f) temp -= 2 * (0x7f + 1);

		    if((i == 0) && (j == 3) && (k == 0)) {
			last_value = initial_value - temp;
		    }
		    sample = (double) temp + last_value;
		    if(counter < nsamples) data[counter] = (float)sample;
		    last_value = sample;
		    counter++;
		}
		break;

		case 2:	// type 2,10,15,30 bit differences

		for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[l+b];
		temp4 = u.i;
		temp = (temp4>>30) & 3;

		switch(temp) {
		    case 3:
		    compression_int = temp4<<2;
		    for(k = 0; k < 3; k++) {
			temp = compression_int>>22;
			if ((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}
			sample = (double) temp + last_value;
			if(counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<10;
		    }
		    break;

		    case 2:
		    compression_int = temp4<<2;
		    for(k = 0; k < 2; k++) {
			temp = compression_int>>17;
			if((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}
			sample = (double) temp + last_value;
			if(counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<15;
		    }
		    break;

		    case 1:
		    compression_int = temp4<<2;
		    for(k = 0; k < 1; k++) {
			temp = compression_int>>2;
			if((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}
			sample = (double) temp + last_value;
			if(counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<30;
		    }
		    break;
		}
		l += 4;
		break;

		case 3:	// type 3,4,5,6 bit differences
		for(b = 0; b < 4; b++) u.a[wo[b]] = bytes[l+b];
		temp4 = u.i;
		temp = (temp4>>30) & 3;
		switch(temp) {
		    case 2:
		    compression_int = temp4<<4;
		    for(k = 0; k < 7; k++) {
			temp = compression_int>>28;
			if((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}
			sample = (double)temp + last_value;
			if (counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<4;
		    }
		    break;

		    case 1:
		    compression_int = temp4<<2;
		    for(k = 0; k < 6; k++) {
			temp = compression_int>>27;
			if((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}

			sample = (double)temp + last_value;
			if(counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<5;
		    }
		    break;

		    case 0:
		    compression_int = temp4<<2;
		    for(k = 0; k < 5; k++) {
			temp = compression_int>>26;
			if((i == 0) && (j == 3) && (k == 0)) {
			    last_value = initial_value - temp;
			}

			sample = (double)temp + last_value;
			if(counter < nsamples) data[counter] = (float)sample;
			last_value = sample;
			counter++;
			compression_int = compression_int<<6;
		    }
		    break;
		}
		l += 4;
		break;

		case 0:		// type 0, not data
		default:
		l += 4;
		break;
	    } // switch loop
	} // single frame for loop
    } // all frames for loop
    for(i = counter; i < nsamples; i++) data[i] = 0;

    if(nsamples > counter) {
	cerr << "Steim2 Decompress Sample Count Error." << endl;
    }
    return counter;
}

int Decoders::data16(const char *bytes, int nbytes, int *so, int nsamples,
			int *data)
{
    WORD u;
    if(nsamples > nbytes/2) nsamples = nbytes/2;

    for(int i = 0; i < nsamples; i++) {
	u.a[so[0]] = *bytes++;
	u.a[so[1]] = *bytes++;
	data[i] = u.s;
    }
    return nsamples;
}

int Decoders::data24(const char *bytes, int nbytes, int *wo, int nsamples,
			int *data)
{
    LONG u, n;
    int k;

    if(nsamples > nbytes/3) nsamples = nbytes/3;

    // find the highest byte
    ByteOrder::getNativeWordOrder(n);
    for(k = 0; k < 4 && n.a[k] != 3; k++);
    if(k == 4) {
	cerr << "Decoders::data24: invalid arguments." << endl;
	return 0;
    }

    // find conversion from data 3 byte to native 3 byte
    for(int i = 0; i < nsamples; i++) {
	for(int j = 0; j < 4; j++) {
	    // set the high byte to 0
	    u.a[wo[j]] = (wo[j] != k) ? *bytes++ : 0;
	}
	data[i] = u.i;
    }
    return nsamples;
}

int Decoders::data32(const char *bytes, int nbytes, int *wo, int nsamples,
			int *data)
{
    LONG u;

    if(nsamples > nbytes/4) nsamples = nbytes/4;

    for(int i = 0; i < nsamples; i++) {
	u.a[wo[0]] = *bytes++;
	u.a[wo[1]] = *bytes++;
	u.a[wo[2]] = *bytes++;
	u.a[wo[3]] = *bytes++;
	data[i] = u.i;
    }
    return nsamples;
}

int Decoders::fdata32(const char *bytes, int nbytes, int *wo, int nsamples,
			float *data)
{
    FLOAT u;

    if(nsamples > nbytes/4) nsamples = nbytes/4;

    for(int i = 0; i < nsamples; i++) {
	u.a[wo[0]] = *bytes++;
	u.a[wo[1]] = *bytes++;
	u.a[wo[2]] = *bytes++;
	u.a[wo[3]] = *bytes++;
	data[i] = u.f;
    }
    return nsamples;
}
