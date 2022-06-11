/** \file seedtocss.cpp
 *  \brief
 */
#include "config.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SeedToCss.h"

using namespace std;

int
main(int argc, const char **argv)
{
    extern char Versionstr[];
    const char *seedfile=NULL, *prefix=NULL, *respdir=NULL, *dir=NULL, *up=NULL, *getd=NULL;
    bool update = true, getdata = true;

    if(argc <= 1) {
	cerr << "Usage: seedtocss seed_file [dir=] [prefix=] [respdir=] [update=(1,0)] [getdata=(1,0)]" << endl;
	return 1;
    }
    if(argc == 2 && strstr(argv[1], "-version")) {
	printf("%s\n", Versionstr);
	return 0;
    }
	
    for(int i = 1; i < argc; i++) {
	if(!strstr(argv[i], "=")) {
	    seedfile = argv[i];
	}
	else if(!strncmp(argv[i], "dir=", 4)) {
	    dir = argv[i]+4;
	}
	else if(!strncmp(argv[i], "prefix=", 7)) {
	    prefix = argv[i]+7;
	}
	else if(!strncmp(argv[i], "respdir=", 8)) {
	    respdir = argv[i]+8;
	}
	else if(!strncmp(argv[i], "update=", 7)) {
	    up = argv[i]+7;
	}
	else if(!strncmp(argv[i], "getdata=", 8)) {
	    getd = argv[i]+8;
	}
    }
    if(seedfile == NULL) {
	cerr << "Usage: seedtocss seed_file [dir=] [prefix=] [respdir=] [update=(1,0)] [getdata=(1,0)]" << endl;
	return 1;
    }
    if(dir == NULL) dir = ".";
    if(prefix == NULL) prefix = "out";
    if(respdir == NULL) respdir = ".";
    if(up != NULL) update = (*up == '1' || *up == 't' || *up == 'T') ? true : false;
    if(getd != NULL) getdata = (*getd == '1' || *getd == 't' || *getd == 'T') ? true : false;

    SeedToCss::convertToCss(seedfile, dir, prefix, respdir, update, getdata);

    return 0;
}
