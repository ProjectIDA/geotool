/** \file DataRecord.cpp
 *  \brief Defines word byte order methods.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;
#include "seed/DataRecord.h"
#include "seed/ByteOrder.h"

/** Determine the swap order from the seed data to the native order. */
void DataRecord::swapOrder()
{
    LONG u;
    WORD s;
    int i, j, k;

    if((int)word_order.size() != 4) {
	cerr << "DataRecord: invalid word_order: " << word_order << endl;
	word_order.assign("3210");
    }
    if((int)short_order.size() != 2) {
	cerr << "DataRecord: invalid short_order: " << short_order << endl;
	short_order.assign("10");
    }
    // get native 4-byte word order
    ByteOrder::getNativeWordOrder(u);

    for(i = 0; i < 4; i++) {
	k = (int)(word_order[i] - '0');
	for(j = 0; j < 4; j++) {
	    if(u.a[j] == k) { wo[i] = j; break; }
	}
    }

    // get native short order
    s.a[0] = 0; s.a[1] = 1;
    if(s.s != 256) {
	s.a[0] = 1; s.a[1] = 0;
    }

    for(i = 0; i < 2; i++) {
	k = (int)(short_order[i] - '0');
	for(j = 0; j < 2; j++) {
	    if(s.a[j] == k) so[i] = j;
	}
    }
}

void ByteOrder::getNativeWordOrder(LONG &u)
{

    u.a[0] = 0; u.a[1] = 1; u.a[2] = 2; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 0; u.a[1] = 1; u.a[2] = 3; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 0; u.a[1] = 2; u.a[2] = 1; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 0; u.a[1] = 2; u.a[2] = 3; u.a[3] = 1; if(u.i == 50462976) return;
    u.a[0] = 0; u.a[1] = 3; u.a[2] = 1; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 0; u.a[1] = 3; u.a[2] = 2; u.a[3] = 1; if(u.i == 50462976) return;


    u.a[0] = 1; u.a[1] = 0; u.a[2] = 2; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 1; u.a[1] = 0; u.a[2] = 3; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 1; u.a[1] = 2; u.a[2] = 0; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 1; u.a[1] = 2; u.a[2] = 3; u.a[3] = 0; if(u.i == 50462976) return;
    u.a[0] = 1; u.a[1] = 3; u.a[2] = 0; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 1; u.a[1] = 3; u.a[2] = 2; u.a[3] = 0; if(u.i == 50462976) return;


    u.a[0] = 2; u.a[1] = 0; u.a[2] = 1; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 2; u.a[1] = 0; u.a[2] = 3; u.a[3] = 1; if(u.i == 50462976) return;
    u.a[0] = 2; u.a[1] = 1; u.a[2] = 0; u.a[3] = 3; if(u.i == 50462976) return;
    u.a[0] = 2; u.a[1] = 1; u.a[2] = 3; u.a[3] = 0; if(u.i == 50462976) return;
    u.a[0] = 2; u.a[1] = 3; u.a[2] = 0; u.a[3] = 1; if(u.i == 50462976) return;
    u.a[0] = 2; u.a[1] = 3; u.a[2] = 1; u.a[3] = 0; if(u.i == 50462976) return;


    u.a[0] = 3; u.a[1] = 0; u.a[2] = 1; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 3; u.a[1] = 0; u.a[2] = 2; u.a[3] = 1; if(u.i == 50462976) return;
    u.a[0] = 3; u.a[1] = 1; u.a[2] = 0; u.a[3] = 2; if(u.i == 50462976) return;
    u.a[0] = 3; u.a[1] = 1; u.a[2] = 2; u.a[3] = 0; if(u.i == 50462976) return;
    u.a[0] = 3; u.a[1] = 2; u.a[2] = 0; u.a[3] = 1; if(u.i == 50462976) return;
    u.a[0] = 3; u.a[1] = 2; u.a[2] = 1; u.a[3] = 0; if(u.i == 50462976) return;
}

/** DataRecord copy constructor.
 */
DataRecord::DataRecord(const DataRecord &dr) : Seed("DR")
{
    word_order = dr.word_order;
    short_order = dr.short_order;
    memcpy(wo, dr.wo, 4*sizeof(int));
    memcpy(so, dr.so, 2*sizeof(int));
    reclen = dr.reclen;
    header = dr.header;
    for(int i = 0; i < (int)dr.blockettes.size(); i++) {
	blockettes.push_back(createDataBlockette(*dr.blockettes[i]));
    }
    record_offset = dr.record_offset;
    data_file_offset = dr.data_file_offset;
    data_length = dr.data_length;
    data = dr.data;
    samprate = dr.samprate;
    format = dr.format;
    clock_drift = dr.clock_drift;
}

DataRecord & DataRecord::operator=(const DataRecord &dr)
{
    if(this == &dr) return *this;
    blockettes.clear();
    for(int i = 0; i < (int)dr.blockettes.size(); i++) {
	blockettes.push_back(createDataBlockette(*dr.blockettes[i]));
    }
    return *this;
}
