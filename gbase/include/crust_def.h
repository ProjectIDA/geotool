#ifndef _CRUST_DEF_H_
#define _CRUST_DEF_H_

#include "crust.h"

#define	NUM_DEFAULT_CRUSTS	28

static CrustModel default_crusts[NUM_DEFAULT_CRUSTS] =
{
    {"iasp", "iaspei91",
	{20.0, 15.0}, {5.8, 6.5, 8.0}, {3.36,3.75,4.47},
    },
    {"jb",   "jeffreys and bullen",
	{15.0, 18.0}, {5.57,6.5, 7.8}, {3.37,3.74,4.42},
    },
    {"ccus", "california coastal region",
	{15.0,  5.0}, {6.2, 7.0, 8.1}, {3.6, 4.0, 4.7},
    },
    {"snus", "sierra nevada",
	{25.0, 25.0}, {6.2, 7.0, 7.9}, {3.6, 4.0, 4.5},
    },
    {"pcus", "pacific northwest coastal region",
	{10.0, 25.0}, {6.2, 7.0, 7.7}, {3.6, 4.0, 4.4},
    },
    {"caus", "columbia plateaus",
	{10.0, 35.0}, {6.2, 7.0, 7.9}, {3.6, 4.0, 4.5},
    },
    {"brus", "basin and range",
	{20.0, 10.0}, {6.2, 7.0, 7.9}, {3.6, 4.0, 4.5},
    },
    {"cpus", "colorado plateaus",
	{25.0, 15.0}, {6.2, 7.0, 7.8}, {3.6, 4.0, 4.5},
    },
    {"rmus", "rocky mountains",
	{25.0, 15.0}, {6.2, 7.0, 8.0}, {3.6, 4.0, 4.6},
    },
    {"ipus", "interior plains and highlands",
	{20.0, 30.0}, {6.2, 7.0, 8.2}, {3.6, 4.0, 4.7},
    },
    {"clus", "coastal plain",
	{20.0, 15.0}, {6.2, 7.0, 8.1}, {3.6, 4.0, 4.7},
    },
    {"asus", "appalachian highlands and superior upland",
	{15.0, 25.0}, {6.2, 7.0, 8.1}, {3.6, 4.0, 4.7},
    },
    {"bsrp", "baltic shield",
	{13.0, 25.0}, {5.9, 6.8, 8.1}, {3.4, 3.9, 4.7},
    },
    {"usrp", "ukrainian shield",
	{16.0, 36.0}, {6.0, 6.8, 8.1}, {3.4, 3.9, 4.7},
    },
    {"vurp", "volga-ural anteclise",
	{18.0, 17.0}, {6.1, 6.8, 8.1}, {3.5, 3.9, 4.7},
    },
    {"pana", "pamir",
	{37.0, 28.0}, {5.5, 6.4, 8.1}, {3.2, 3.7, 4.7},
    },
    {"caaf", "caucasus anticlinorium",
	{20.0, 30.0}, {5.8, 7.2, 8.1}, {3.3, 4.1, 4.7},
    },
    {"feaf", "mesozoids of the far east",
	{22.0, 14.0}, {6.2, 6.7, 8.1}, {3.6, 3.9, 4.7},
    },
    {"scbs", "south caspian",
	{20.0, 23.0}, {3.6, 6.5, 8.0}, {2.1, 3.7, 4.6},
    },
    {"bsbs", "black sea",
	{10.0, 13.0}, {3.3, 6.6, 8.0}, {1.9, 3.8, 4.6},
    },
    {"dfsc", "denmark",
	{ 8.0, 21.0}, {6.1, 6.6, 8.1}, {3.5, 3.8, 4.7},
    },
    {"kabs", "kamchatka",
	{20.0, 15.0}, {6.2, 6.6, 8.0}, {3.6, 3.8, 4.6},
    },
    {"ausp", "australia",
	{20.0, 20.0}, {6.0, 6.6, 8.0}, {3.4, 3.8, 4.6},
    },
    {"kkal", "kodial-katmai",
	{12.0, 20.0}, {5.5, 6.5, 8.1}, {3.2, 3.7, 4.7},
    },
    {"inal", "interior alaska",
	{10.0, 42.0}, {5.8, 6.8, 8.1}, {3.3, 3.9, 4.7},
    },
    {"dcaa", "deep caucasus anticlinorium",
	{30.0, 30.0}, {5.8, 7.2, 8.1}, {3.3, 4.1, 4.7},
    },
    {"kule", "kuril islands",
	{ 0.1, 26.0}, {0.1, 6.5, 7.9}, {0.1, 3.7, 4.5},
    },
    {"kazz", "kazakh",
	{20.0, 30.0}, {6.37,7.0,8.34}, {3.6, 4.0, 4.9},
    },
};
#endif