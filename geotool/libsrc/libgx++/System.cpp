/** \file System.cpp
 *  \brief Defines class System.
 */
#include "config.h"
#include <iostream>
using namespace std;
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#ifdef HAVE_STATVFS
#include <sys/statvfs.h>
#else /* HAVE_STATVFS */
#ifdef HAVE_STATFS
#include <sys/vfs.h>
#endif  /* HAVE_STATFS */
#endif /* HAVE_STATVFS */

#include "System.h"

System::System()
{
}

System::~System()
{
}


/* 
	getSwapSpace returns the number of bytes left in swap.
	It's estimate is a little higher than that given by df,
	but is close enough
*/

u_long System::getSwapSpace(void)
{
#ifdef HAVE_STATVFS
    struct statvfs	buf;
    FILE		*fp;
    char		a[64];
    static  char	swap_mount[128] = "";
#else /* HAVE_STATVFS */
#ifdef HAVE_STATFS
    struct statfs	buf;
    FILE		*fp;
    char		a[64];
    static  char	swap_mount[128] = "";
#endif /* HAVE_STATFS */
#endif /* HAVE_STATVFS */
    static int		found_swap = 0;

    /* don't keep trying if fopen or statvfs fails the first time
     */
    if(found_swap < 0) return(1);

#ifdef HAVE_STATVFS
    if(!found_swap)
    {
	found_swap = -1;
	if((fp = fopen("/etc/mnttab", "r")) == NULL)
	{
	    fprintf(stderr, "Cannot open /etc/mnttab\n");
	    if(errno > 0) {
		fprintf(stderr, "%s", strerror(errno));
	    }
	    return(1);
	}
	while(fscanf(fp, "%s %s %*s %*s %*d", a, swap_mount) != EOF)
	{
	    if(!strcmp(a, "swap")) {
		found_swap = 1;
		break;
	    }
	}
	fclose(fp);
	if(found_swap == -1) return(1);
    }

    if(statvfs(swap_mount, &buf) == 0)
    {
	return(buf.f_bavail*buf.f_bsize);
    }
    else
    {
	fprintf(stderr, "statvfs failed for %s\n", swap_mount);
	if(errno > 0) {
	    fprintf(stderr, "%s", strerror(errno));
	}
	found_swap = -1;
    }
#else /* HAVE_STATVFS */
#ifdef HAVE_STATFS
    if(!found_swap)
    {
	found_swap = -1;
	if((fp = fopen("/etc/mtab", "r")) == NULL)
	{
	    fprintf(stderr, "Cannot open /etc/mnttab\n");
	    if(errno > 0) {
		fprintf(stderr, "%s", strerror(errno));
	    }
	    return(1);
	}
	while(fscanf(fp, "%s %s %*s %*s %*d %*d",a,swap_mount) != EOF)
	{
	    if(!strcmp(a, "swap")) {
		found_swap = 1;
		break;
	    }
	}
	fclose(fp);
	if(found_swap == -1) return(1);
    }

    if(statfs(swap_mount, &buf) == 0)
    {
	return(buf.f_bavail*buf.f_bsize);
    }
    else
    {
	fprintf(stderr, "statfs failed for %s\n", swap_mount);
	if(errno > 0) {
	    fprintf(stderr, "%s", strerror(errno));
	}
	found_swap = -1;
    }
#endif /* HAVE_STATFS */
#endif /* HAVE_STATVFS */
    return(1);
}
