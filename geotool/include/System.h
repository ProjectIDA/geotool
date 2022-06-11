#ifndef _SYSTEM_H
#define _SYSTEM_H
#include <sys/types.h>

/**
 *  @ingroup libgx
 */
class System
{
    public:

	System(void);
	~System(void);

	static u_long getSwapSpace(void);

    protected:


    private:
};

#endif
