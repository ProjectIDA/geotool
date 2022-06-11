#ifndef _PROPERTIES_H_
#define	_PROPERTIES_H_

#include <stdio.h>

#include "gobject/Hashtable.h"

Hashtable programProperties(void);
void readProgramProperties(int argc, const char **argv, const char *filename);
Hashtable getProperties(FILE *fp);
bool writeProperties(Hashtable h);
bool writeProgramProperties(void);
bool putFileProperties(Hashtable h, const char *filename);
bool putProperties(Hashtable h, FILE *fp);
void putProperty(Hashtable properties, const char *name, const char *value);
void putProgramProperty(const char *name, const char *value);
void putProgramTmpProperty(const char *name, const char *value);
char *getProperty(Hashtable properties, const char *name);
char *getProgramProperty(const char *name);
void putProgramResource(const char *name, void *resource);
void *getProgramResource(const char *name);
int getIntProperty(const char *name, int default_value);
double getDoubleProperty(const char *name, double default_value);
bool getBoolProperty(const char *name, bool default_value);
void removeProgramProperty(const char *name);


#endif /* _PROPERTIES_H_ */
