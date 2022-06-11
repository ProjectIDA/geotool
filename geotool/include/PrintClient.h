#ifndef _PRINT_CLIENT_H
#define _PRINT_CLIENT_H

#include <stdio.h>
#include "PrintParam.h"

/** An interface for classes that need to print AxesClass objects.
 *  @ingroup libwgets
 */
class PrintClient
{
    public:
	virtual ~PrintClient(void) {}

	virtual void print(FILE *fp, PrintParam *p) {}

    protected:
	// can only be constructed by a subclass
	PrintClient(void){}

    private:

};

#endif
