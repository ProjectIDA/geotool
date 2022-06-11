#ifndef _BEAM_STA_H
#define _BEAM_STA_H
#include <sys/param.h>

/** @defgroup libgbeam library libgbeam
 *  C++ classes for waveform beaming and  frequency-wavenumber (FK) analysis.
 *  This library contains classes for performing FK analysis for array data.
 *  The FKData class holds the FK results. The FKGram class has functions for
 *  analyzing the time varying FK properties. The Beam class has functions for
 *  reading beam recipes and computing waveform beams.
 */

/** This structure contains the beam recipe information. The members are the
 *  paramters in the recipe par files plus the network name, the path to the
 *  recipe par file, and a selected flag.
 *  @ingroup libgbeam
 */
class BeamRecipe
{
    public:
	string	net;	    //!< the network name.
	int	path;	    //!< the path to the recipe file.
	string	name;	    //!< the recipe name.
	string	beam_type;  //!< beam type, "coh" or "inc"
	string	rot;
	int	std;	   //!< associated standard deivation
	double	snr;	   //!< associated signal-to-noise ratio
	double	azi;	   //!< associated azimuth
	double	slow;	   //!< associated slowness
	string	phase;     //!< associated phase name
	double	flo;	   //!< filter low frequency cutoff.
	double	fhi;	   //!< filter high frequency cutoff.
	int	ford;	   //!< filter order
	int	zp;	   //!< filter zero phase flag
	string	ftype;	   //!< filter type (BP, etc)
	string	group;    //!< group name (vertical, horizontal, etc)
	bool	selected;

	BeamRecipe() {
	    path = 0;
	    std = -1;
	    snr = -1.;
	    azi = -999.;
	    slow = -999.;
	    flo = -1.;
	    fhi = -1.;
	    ford = 3;
	    zp = 0;
	    selected = false;
	}
};

/** The beam station member structure.
 *  @ingroup libgbeam
 */
typedef struct
{
	char	sta[10]; //!< the station name
	char	chan[10]; //!< the channel name
	double	wgt;	//!< the beam member weight
} BeamSta;

/** The beam station group structure. This structure contains the stations that
 *  contribute to the beam.
 *  @ingroup libgbeam
 */
typedef struct
{
	char	net[20];	//!< the network name.
	char	group[50];	//!< the beam group name.
        int	path;		//!< the path to the beam group par file.
	vector<BeamSta>	sta;	//!< the stations in the beam group.
} BeamGroup;

/** The method of specifying the location of the beamed signal.
 *  @ingroup libgbeam
 */
enum BeamLocation {
    DNORTH_DEAST,	//!< use the dnorth/deast values form the site table
    REFERENCE_STATION,  //!< use the reference station name from the site table
    ARRAY_CENTER,	//!< use the geometrical center of the network.
	/** returned if an error occurs when determining the beam location */
    BEAM_LOCATION_ERROR
};

#endif
