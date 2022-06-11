/** \file beam.cpp
 *  \brief Defines class Beam.
 *  \author Ivan Henson
 */
#include "config.h"
#include <math.h>
#include "Beam.h"
#include "gobject++/GTimeSeries.h"
#include "Waveform.h"
#include "DataMethod.h"
#include "gobject++/DataSource.h"
#include "gobject++/GSourceInfo.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
}

/** Get the latitude and longitude of the beam location. This function uses
 *  the locations of the stations in the network and the BeamLocation method
 *  to compute the latitude and longitude of the beam location. In some cases,
 *  if the desired location method (<b>beam_location</b>) cannot be used, the
 *  ARRAY_CENTER location method will be used. The GError::getMessage function
 *  will return a non-NULL message in this case. The location method actually
 *  used is returned by the function.
 *
 *  <h3>beam_location = DNORTH_DEAST</h3>
 *
 *  If the desired BeamLocation method is DNORTH_DEAST, all of the waveforms
 *  must have valid dnorth and deast values. The dnorth and deast values are
 *  obtained from the GTimeSeries checked for valid magnitudes with the code:
 *  \code
    if((dnorth = wvec[i]->dnorth()) > 100000. ||
       (deast = wvec[i]->deast()) > 100000.)
    {
	GError::setMessage(...
    \endcode <p>
 *  <h4>DNORTH_DEAST errors:</h4>
 *  If dnorth and deast are not available for all stations in the network, an
 *  error message is set (use GError::getMessage), and the ARRAY_CENTER
 *  method is used to compute the beam location.
 *
 *  If there are no valid station locations in the input Waveform objects
 *  (wvec[i]->lat() == -999. || wvec[i]->lon() == -999 for all waveforms),
 *  then the an error message will be set and BEAM_LOCATION_ERROR will be
 *  returned. \code
	    GError::setMessage("BeamGetLocation: no station locations.");
	    return BEAM_LOCATION_ERROR; \endcode
 *  
 *  <h3>beam_location = REFERENCE_STATION</h3>
 *  If the desired BeamLocation method is REFERENCE_STATION, the reference
 *  station name is obtained from the site table for the station of the
 *  first waveform (wvec[0]->sta()). The reference station's latitude and
 *  longitude from the site table are returned as the beam location. For this
 *  method, the DataSource routine DataSource::getNetwork is called to get the
 *  network
 *  stations and the reference station.
 *  <h4>REFERENCE_STATION errors:</h4>
 *  An error message is set and BEAM_LOCATION_ERROR is returned, if any of the
 *  following occur:
 *  - If the first station wvec[0]->sta() does not have a reference station.
 *  - If the reference station is not in the network.
 *  - If the reference station does not have valid lat and lon values from the
 *	site table.
 *
 *  <h3>beam_location = ARRAY_CENTER</h3>
 *  The station that is closest to the geometrical center of the network is
 *  used as the beam location.
 *  <h4>ARRAY_CENTER errors:</h4>
 *  An error message is set and BEAM_LOCATION_ERROR is returned, if any of the
 *  following occur:
 *  - one of the input waveforms does not have a valid lat and lon
 *	(wvec[i]->lat() == -999 || wvec[i]->lon() == -999)
 *  - all of the input waveform locations are the same (wvec[i]->lat() is
 *      the same for all waveforms and wvec[i]->lon is the same for all
 *  	waveforms)
 *
 *  @param[in] ds a DataSource used to get the network stations that belong to
 *	the network of the first waveform (wvec[0]->net). The function
 *	DataSource::getNetwork is used. (ds is only used when beam_location is
 *	REFERENCE_STATION)
 *  @param[in] beam_location the method of computing the beam location:
 *	- DNORTH_DEAST use the dnorth and deast values from the site table.
 *	- REFERENCE_STATION use the location of the refsta from the site table.
 *      - ARRAY_CENTER use the station closest to the geometrical center of
 *	the array.
 *  @param[in] wvec the waveforms.
 *  @param[out] ref_lon the longitude of the beam.
 *  @param[out] ref_lat the latitude of the beam.
 *  @returns the actual beam location method used. Returns BEAM_LOCATION_ERROR
 *  and sets an error message, if an error was encountered.
 *  @throws GERROR_MALLOC_ERROR
 */
BeamLocation Beam::getLocation(DataSource *ds, BeamLocation beam_location,
	gvector<Waveform *> &wvec, double *ref_lon, double *ref_lat)
{
    int		i;
    double	rad;
    double	pi2 = M_PI/2., radius = 6371.;

    rad = M_PI/180.;

    if(wvec.size() <= 0) {
	GError::setMessage("Beam.getLocation: Number of waveforms = %d",
		wvec.size());
	return BEAM_LOCATION_ERROR;
    }

    if(beam_location == DNORTH_DEAST)
    {
	/* Use dnorth and deast, if we have them for all waveforms.
	 */
	bool first = true;
	bool all_zero = true; 

	for(i = 0; i < wvec.size(); i++)
	{
	    double dnorth, deast;

	    dnorth = wvec[i]->dnorth();
	    deast = wvec[i]->deast();

	    if(dnorth != 0. || deast != 0.) all_zero = false;

	    /* Use the first valid station location to compute the beam
	     * location.
	     */
	    if(first && wvec[i]->lat() > -900. && wvec[i]->lon()> -900.)
	    {
		double theta, phi, theta0, phi0, x, y, z;

		first = false;
		/* compute the reference location using the dnorth/deast
		 * values from the first station.
		 */

		/* get the theta,phi coordinates of the reference location
		 * in a coordinate system with the z-axis through the first
		 * station and the y-axis pointing north.
		 */
		y = -dnorth;
		x = -deast;
		z = radius;
		theta = atan2(sqrt(x*x+y*y), z);
		phi = atan2(y, x);

		theta0 = pi2 - rad*wvec[i]->lat();
		phi0 = rad*wvec[i]->lon();

		/* rotate theta, phi back to the regular coordinate system.
		 * Euler angles to this new system are -pi2, -theta0, -phi0
		 */
		euler(&theta, &phi, -pi2, -theta0, -phi0);
		*ref_lon = phi/rad;
		*ref_lat = (pi2 - theta)/rad;
	    }
	}
	if(first) {
	    GError::setMessage("Beam.getLocation: no station locations.");
	    return BEAM_LOCATION_ERROR;
	}
	if(i == wvec.size() && all_zero) {
	    /* the null value for dnorth and deast is actually 0., but the
	     * center element can have an true dnorth/deast of 0., so check
	     * if all elements are 0.
	     */
	    beam_location = ARRAY_CENTER;
	    GError::setMessage(
			"Beam.getLocation: Missing dnorth/deast values\n%s\n",
		       "Array geometric center will be used as the reference.");
	}
	else {
	    return DNORTH_DEAST;
	}
    }

    if(beam_location == REFERENCE_STATION)
    {
	/* use reference station as beam location
	 */
	if(wvec.size() > 0) {
	    int q_sta = stringUpperToQuark(wvec[0]->sta());
	    int q_refsta = 0;
	    int num_stations;
	    GStation **stations=NULL;

	    num_stations = ds->getNetwork(wvec[0]->net(), &stations);
	    for(i = 0; i < num_stations; i++) {
		if(stations[i]->sta == q_sta) {
		    q_refsta = stations[i]->refsta;
		    break;
		}
	    }
	    if(q_refsta == 0) {
		GError::setMessage(
		   "Beam.getLocation: No reference station has been selected.");
		Free(stations);
		return BEAM_LOCATION_ERROR;
	    }
	    for(i = 0; i < num_stations; i++) {
		if(q_refsta == stations[i]->sta) break;
	    }
	    if(i == num_stations) {
		GError::setMessage(
			"Beam.getLocation: Cannot find reference station.");
		Free(stations);
		return BEAM_LOCATION_ERROR;
	    }
	    if(stations[i]->lat < -900. || stations[i]->lon < -900) {
		GError::setMessage(
		    "Beam.getLocation: No lat/lon for reference station: %s",
			quarkToString(stations[i]->sta));
		Free(stations);
		return BEAM_LOCATION_ERROR;
	    }
	    *ref_lat = stations[i]->lat;
	    *ref_lon = stations[i]->lon;
	    Free(stations);
	}
    }
    else if(beam_location == ARRAY_CENTER)
    {
	int j, n, n_unique;
	double x0, y0, z0;
	double *lat = NULL, *lon = NULL;

	if(!(lat = (double *)malloc(wvec.size()*sizeof(double))) ||
	   !(lon = (double *)malloc(wvec.size()*sizeof(double))))
	{
	    Free(lat);
	    GError::setMessage("Beam.getLocation: malloc failed.");
	    throw(GERROR_MALLOC_ERROR);
	}
	    
	/* first check if we have lat, lon for all selected waveforms.
	 */
	for(i = n = n_unique = 0; i < wvec.size(); i++)
	{
	    if(wvec[i]->lat() < -900. || wvec[i]->lon() < -900.)
	    {
		GError::setMessage("No station location for %s",wvec[i]->sta());
		Free(lon); Free(lat);
		return BEAM_LOCATION_ERROR;
	    }
	    else
	    {
		n++;
		for(j = 0; j < n_unique; j++) {
		    if( lat[j] == wvec[i]->lat() &&
			lon[j] == wvec[i]->lon()) break;
		}
		if(j == n_unique) {
		    lat[n_unique] = wvec[i]->lat();
		    lon[n_unique] = wvec[i]->lon();
		    n_unique++;
		}
	    }
	}
	if(n == 0)
	{
	    Free(lon); Free(lat);
	    return BEAM_LOCATION_ERROR;
	}

	/* compute the geometrical center of the station locations.
	 */
	x0 = y0 = z0 = 0;
	for(i = 0; i < n_unique; i++)
	{
	    /* cartesian coordinates */
	    double theta = pi2 - rad*lat[i];
	    double phi = rad*lon[i];
	    double x = sin(theta)*cos(phi);
	    double y = sin(theta)*sin(phi);
	    double z = cos(theta);
	    x0 += x;
	    y0 += y;
	    z0 += z;
	}
	x0 /= n_unique;
	y0 /= n_unique;
	z0 /= n_unique;
	double lat0 = 90. - atan2(sqrt(x0*x0+y0*y0), z0)/rad;
	double lon0 = atan2(y0, x0)/rad;
	/*
	 * Use the station lat,lon closest to the center as the ref station
	 */
	int k;
	double d, dist;
	k = 0;
	dist = (lat[0]-lat0)*(lat[0]-lat0) +(lon[0]-lon0)*(lon[0]-lon0);
	for(i = 1; i < n_unique; i++)
	{
	    d =(lat[i]-lat0)*(lat[i]-lat0) +(lon[i]-lon0)*(lon[i]-lon0);
	    if(d < dist) {
		k = i;
		dist = d;
	    }
	}
	*ref_lon = lon[k];
	*ref_lat = lat[k];

	Free(lon); Free(lat);
    }
    else {
	GError::setMessage("Beam.getLocation: invalid beam_location = %d.",
		beam_location);
	return BEAM_LOCATION_ERROR;
    }
    return beam_location;
}

/** Returns station time lags for a signal with the input azimuth and slowness.
 *  The station time lags are with respect to the beam location, where the
 *  signal arrives at time zero. The time lags can be both greater and less
 *  that zero. The beam location is specified by the BeamLocation argument.
 *
 *  <h3>beam_location = DNORTH_DEAST</h3>
 *
 *  If the desired BeamLocation method is DNORTH_DEAST, all of the waveforms
 *  must have valid dnorth and deast values. The dnorth and deast values are
 *  obtained from the GTimeSeries checked for valid magnitudes with the code:
 *  \code
    if((dnorth = wvec[i]->dnorth()) > 100000. ||
       (deast = wvec[i]->deast()) > 100000.)
    {
	GError::setMessage(...
    \endcode <p>
 *
 *  If all waveforms in wvec[] have valid dnorth and deast values, then
 *  the time lag is computed as:
\code
    sx = slowness*sin(az*PI/180.);
    sy = slowness*cos(az*PI/180.);
    tlags[i] = sx*deast + sy*dnorth;
\endcode
 *
 *  <h4>DNORTH_DEAST errors:</h4>
 *  If dnorth and deast are not available for all stations in the network, an
 *  error message is set (use GError::getMessage), and the ARRAY_CENTER
 *  method is used to compute the beam location.
 *
 *  <h3>beam_location = REFERENCE_STATION</h3>
 *  If the desired BeamLocation method is REFERENCE_STATION, the reference
 *  station name is obtained from the site table for the station of the
 *  first waveform (wvec[0]->sta()). The time lags are computed with respect
 *  to the reference station's location. For this method, the DataSource
 *  routine DataSource::getNetwork is called to get the network stations and
 *  the reference station.
 *  <h4>REFERENCE_STATION errors:</h4>
 *  An error message is set and NULL is returned, if any of the following occur:
 *  - If the first station wvec[0]->sta() does not have a reference station.
 *  - If the reference station is not in the network.
 *  - If the reference station does not have valid lat and lon values from the
 *	site table.
 *
 *  <h3>beam_location = ARRAY_CENTER</h3>
 *  The station that is closest to the geometrical center of the network is
 *  used as the beam location. Time lags are computed with respect to this
 *  station location.
 *  <h4>ARRAY_CENTER errors:</h4>
 *  An error message is set and BEAM_LOCATION_ERROR is returned, if any of the
 *  following occur:
 *  - one of the input waveforms does not have a valid lat and lon
 *	(wvec[i]->lat == -999 || wvec[i]->lon == -999)
 *  - all of the input waveform locations are the same (wvec[i]->lat is the
 *	same for all waveforms and wvec[i]->lon is the same for all
 *  	waveforms)
 *
 *  @param[in] ds a DataSource used to get the network stations that belong to
 *	the network of the first waveform (wvec[0]->net()). The function
 *	DataSource::getNetwork is used. (ds is only used when beam_location is
 *	REFERENCE_STATION)
 *  @param[in] wvec the waveforms.
 *  @param[in] az the azimuth of the signal.
 *  @param[in] slowness the slowness of the signal (sec/km).
 *  @param[in] beam_location the method of computing the beam location:
 *	- DNORTH_DEAST use the dnorth and deast values from the site table.
 *	- REFERENCE_STATION use the location of the refsta from the site table.
 *      - ARRAY_CENTER use the geometrical center of the array.
 *  @param[out] beam_lat the latitude of the beam location. (ignored if NULL)
 *  @param[out] beam_lon the longitude of the beam location. (ignored if NULL)
 *  @returns a pointer to an array of time lags corresponding to the stations
 *  of the Waveform objects in wvec[]. Free the pointer when no longer
 *  needed. Returns NULL and sets an error message, if an error was encountered.
 *  @throws GERROR_MALLOC_ERROR
 */
bool Beam::getTimeLags(DataSource *ds, gvector<Waveform *> &wvec,
		double az, double slowness, BeamLocation beam_location,
		vector<double> &tlags, double *beam_lat, double *beam_lon)
{
    int		i;
    double	sx, sy, rad, dt0=0.;
    double	dnorth, deast;
    double	pi2 = M_PI/2., radius = 6371.;

    tlags.clear();

    // Handle the special case of one waveform
    if(wvec.size() == 1) {
	tlags.push_back(0.);
	return true;
    }

    rad = M_PI/180.;
    az *= rad;
    sx = slowness*sin(az);
    sy = slowness*cos(az);
    for(i = 0; i < wvec.size(); i++) tlags.push_back(0.);

    if(beam_location == DNORTH_DEAST)
    {
	/* first check if we have dnorth, deast for all selected waveforms.
	 */
	for(i = 0; i < wvec.size(); i++)
	{
	    dnorth = wvec[i]->dnorth();
	    deast = wvec[i]->deast();

	    if(i == 0) {
		dt0 = wvec[0]->segment(0)->tdel();
		if( dt0 == 0.) {
		    GError::setMessage("%s/%s: Invalid sample interval.",
				wvec[i]->sta(), wvec[0]->chan());
		    return false;
		}
	    }
	    else {
		double f = fabs((wvec[i]->segment(0)->tdel() - dt0)/dt0);
		if(f > .01) {
		    GError::setMessage("%s/%s: Nonuniform sample rate.",
				wvec[i]->sta(), wvec[i]->chan());
		    return false;
		}
	    }
	    tlags[i] = sx*deast + sy*dnorth;

	    if(i == 0 && (beam_lat || beam_lon))
	    {
		double theta, phi, theta0, phi0, x, y, z;
		y = -dnorth;
		x = -deast;
		z = radius;
		theta = atan2(sqrt(x*x+y*y), z);
		phi = atan2(y, x);

		theta0 = pi2 - rad*wvec[i]->lat();
		phi0 = rad*wvec[i]->lon();

		/* rotate theta, phi back to the regular coordinate system.
		 * Euler angles to this new system are -pi2, -theta0, -phi0
		 */
		euler(&theta, &phi, -pi2, -theta0, -phi0);
		if(beam_lon) *beam_lon = phi/rad;
		if(beam_lat) *beam_lat = (pi2 - theta)/rad;
	    }
	}
	if(i == wvec.size()) {
	    /* the null value for dnorth and deast is actually 0., but the
	     * center element can have an true dnorth/deast of 0., so check
	     * if all elements are 0.
	     */
	    for(i = 0; i < wvec.size(); i++) {
		if(tlags[i] != 0.) break;
	    }
	    if(i == wvec.size()) {
		beam_location = ARRAY_CENTER;
		GError::setMessage(
		    "Beam.getTimeLags: Missing dnorth/deast values\n%s\n",
		    "Array geometric center will be used as the reference.");
	    }
	    else {
		return true;
	    }
	}
    }

    if(beam_location != DNORTH_DEAST)
    {
	int j, n, n_unique;
	double x0, y0, z0, theta0=0., phi0=0.;
	double *lat = NULL, *lon = NULL;

	if( !(lat = (double *)malloc(wvec.size()*sizeof(double))) ||
	    !(lon = (double *)malloc(wvec.size()*sizeof(double))))
	{
	    Free(lon); Free(lat);
	    GError::setMessage("Beam.getTimeLags: malloc failed.");
	    throw(GERROR_MALLOC_ERROR);
	}
	    
	/* first check if we have lat, lon for all selected waveforms.
	 */
	for(i = n = n_unique = 0; i < wvec.size(); i++)
	{
	    if(wvec[i]->lat() < -900. || wvec[i]->lon() < -900.)
	    {
		GError::setMessage(
		    "Beam.getTimeLags:No station location for %s",
				wvec[i]->sta());
		Free(lon); Free(lat);
		return false;
	    }
	    else
	    {
		n++;
		for(j = 0; j < n_unique; j++) {
		    if( lat[j] == wvec[i]->lat() &&
			lon[j] == wvec[i]->lon()) break;
		}
		if(j == n_unique) {
		    lat[n_unique] = wvec[i]->lat();
		    lon[n_unique] = wvec[i]->lon();
		    n_unique++;
		}
	    }
	}
	if(n == 0)
	{
	    Free(lon); Free(lat);
	    return false;
	}

	if(beam_location == REFERENCE_STATION)
	{
	    /* use reference station as beam location
	     */
	    if(wvec.size() > 0) {
		int q_sta = stringUpperToQuark(wvec[0]->sta());
		int q_refsta = 0;
		int num_stations;
		GStation **stations=NULL;

		num_stations = ds->getNetwork(wvec[0]->net(), &stations);
		for(i = 0; i < num_stations; i++) {
		    if(stations[i]->sta == q_sta) {
			q_refsta = stations[i]->refsta;
			break;
		    }
		}
		if(q_refsta == 0) {
		    GError::setMessage(
		"Beam.getTimeLags: No reference station has been selected.");
		    Free(lon); Free(lat); Free(stations);
		    return false;
		}
		for(i = 0; i < num_stations; i++) {
		    if(q_refsta == stations[i]->sta) break;
		}
		if(i == num_stations) {
		    GError::setMessage(
			"Beam.getTimeLags: Cannot find reference station.");
		    Free(lon); Free(lat); Free(stations);
		    return false;
		}
		if(stations[i]->lat < -900. || stations[i]->lon < -900) {
		    GError::setMessage(
		       "Beam.getTimeLags: No lat/lon for reference station: %s",
			quarkToString(stations[i]->sta));
		    Free(lon); Free(lat); Free(stations);
		    return false;
		}
		theta0 = pi2 - rad*stations[i]->lat;
		phi0 = rad*stations[i]->lon;
		if(beam_lat) *beam_lat = stations[i]->lat;
		if(beam_lon) *beam_lon = stations[i]->lon;
		Free(stations);
	    }
	}
	else
	{
	    /* compute the geometrical center of the station locations.
	     */
	    x0 = y0 = z0 = 0;
	    for(i = 0; i < n_unique; i++)
	    {
		/* cartesian coordinates */
		double theta = pi2 - rad*lat[i];
		double phi = rad*lon[i];
		double x = sin(theta)*cos(phi);
		double y = sin(theta)*sin(phi);
		double z = cos(theta);
		x0 += x;
		y0 += y;
		z0 += z;
	    }
	    x0 /= n_unique;
	    y0 /= n_unique;
	    z0 /= n_unique;
	    double lat0 = 90. - atan2(sqrt(x0*x0+y0*y0), z0)/rad;
	    double lon0 = atan2(y0, x0)/rad;
	    /*
	     * Use the station lat,lon closest to the center as the ref station
	     */
	    int k;
	    double d, dist;
	    k = 0;
	    dist = (lat[0]-lat0)*(lat[0]-lat0) +(lon[0]-lon0)*(lon[0]-lon0);
	    for(i = 1; i < n_unique; i++)
	    {
		d =(lat[i]-lat0)*(lat[i]-lat0) +(lon[i]-lon0)*(lon[i]-lon0);
		if(d < dist) {
		    k = i;
		    dist = d;
		}
	    }
	    theta0 = rad*(90. - lat[k]);
	    phi0 = rad*lon[k];
	    if(beam_lat) *beam_lat = lat[k];
	    if(beam_lon) *beam_lon = lon[k];
	}

	/* find the x and y coordinates of each station in a coordinate
	 * system with the z-axis at theta0, phi0, and the y-axis north:
	 * Euler angles to this new system are phi0, theta0, pi/2.  Then
	 * compute the time lag as minus the dot product of the horizontal
	 * slowness vector with the local station coordinates(x,y)
	 */
	for(i = 0; i < wvec.size(); i++)
	{
	    double theta, phi, x, y;
	    theta = pi2 - rad*wvec[i]->lat();
	    phi = rad*wvec[i]->lon();
		
	    euler(&theta, &phi, phi0, theta0, pi2);
	    x = radius*sin(theta)*cos(phi);
	    y = radius*sin(theta)*sin(phi);
	    tlags[i] = sx*x + sy*y;
	}
	Free(lon); Free(lat);
    }

    return true;
}

/** Compute a beamed waveform. Beam the input waveforms using the input time
 *  lags and weights. The beamed waveform will be in counts.
 *  @param[in] wvec Waveform objects.
 *  @param[in] tlags the time lags for the waveforms.
 *  @param[in] weights the weights for the waveforms.
 *  @param[in] coherent if true, the absolute values of the waveforms are
 *  summed to make the beam instead of the signed values.
 *  @returns the beam as a GTimeSeries object. Returns NULL if num_waveforms
 *  <= 0 or the number of beam samples would be 0.
 */
GTimeSeries * Beam::BeamTimeSeries(gvector<Waveform *> &wvec,
		vector<double> &tlags, vector<double> &weights, bool coherent)
{
    int		i, j, k, npts, n, k0;
    int		*count = NULL;
    float	*b = NULL;
    double	*aver_calib = NULL;
    double	tdel, calper, start, tbeg, w, norm=0.;
    GTimeSeries	*t, *ts;
    bool	calib_applied;

    if(wvec.size() <= 0) return NULL;
    for(i = (int)tlags.size(); i < wvec.size(); i++) tlags.push_back(0.);
    for(i = (int)weights.size(); i < wvec.size(); i++) weights.push_back(1.);

    tbeg = wvec[0]->tbeg();
    start = tbeg + tlags[0];

    for(i = 1; i < wvec.size(); i++)
    {
	tbeg = wvec[i]->tbeg();
	if(start > tbeg + tlags[i]) {
	    start = tbeg + tlags[i];
	}
    }
    for(i = npts = 0; i < wvec.size(); i++)
    {
	t = wvec[i]->ts;
	for(j = 0; j < t->size(); j++) {
	    GSegment *s = t->segment(j);
	    k0 = (int)((s->tbeg() + tlags[i] - start)/s->tdel() + .5);
	    if(npts < k0 + s->length()) npts = k0 + s->length();
	}
    }
    if(!npts) return NULL;

    if( !(b = (float *)malloc(npts*sizeof(float))) ||
	!(count = (int *)malloc(npts*sizeof(int))) ||
	!(aver_calib = (double *)malloc(npts*sizeof(double))))
    {
	Free(aver_calib); Free(count); Free(b);
	GError::setMessage("Beam.BeamTimeSeries: malloc failed.");
	throw(GERROR_MALLOC_ERROR);
    }
    for(i = 0; i < npts; i++) {
	b[i] = 0.;
	count[i] = 0;
	aver_calib[i] = 0.;
    }

    for(i = 0; i < wvec.size(); i++)
    {
	t = wvec[i]->ts;

	norm += weights[i];

	/* Determine if the calib has been applied.
	 */
	calib_applied = t->getMethod("CalibData") ? true : false;

	/* Sum with each waveform. Remove calib if necessary.
	 */
	for(j = 0; j < t->size(); j++)
	{
	    double cal;
	    GSegment *s = t->segment(j);
	    k0 = (int)((s->tbeg() + tlags[i] - start)/s->tdel() + .5);

	    cal = (s->calib() != 0.) ? s->calib() : 1.;

	    w = weights[i];
	    if(calib_applied) w /= cal;

	    n = s->length();

	    if(coherent)
	    {
		for(k = 0; k < n; k++) {
		    b[k0+k] += w * s->data[k];
		    count[k0+k] += 1;
		    aver_calib[k0+k] += cal;
		}
	    }
	    else
	    {
		for(k = 0; k < n; k++) {
		    b[k0+k] += w * (float)fabs((double) s->data[k]);
		    count[k0+k] += 1;
		    aver_calib[k0+k] += cal;
		}
	    }
	}
    }
    if(norm) {
	norm = 1./norm;
	for(i = 0; i < npts; i++) b[i] *= norm;
    }

    ts = new GTimeSeries();

    ts->source_info.setSource(wvec[0]->ts->source_info);

    tdel = wvec[0]->segment(0)->tdel();

    calper = 0.;
    for(i = 0; i < wvec.size(); i++) {
	calper += wvec[i]->segment(0)->calper();
    }
    calper /= wvec.size();

    i = 0;
    while(i < npts)
    {
	GSegment *s;
	/* Only create Segments for the periods where all (num_waveforms)
	 * elements have data.
	 */
	while(i < npts && count[i] != wvec.size()) i++;

	if(i < npts) {
	    double calib = aver_calib[i];
	    /* Look for the next point where the number of contributing
	     * elements is less than num_waveforms, or the calib
	     * changes. If the calib changes, start a new Segment. Skip
	     * periods that have less than num_waveforms.
	     */
	    for(j = i+1; j < npts; j++) {
		if(count[j] != wvec.size() || (calib &&
		    fabs((aver_calib[j] - calib)/calib) > .01)) break;
	    }
	    calib /= wvec.size();
	    s = new GSegment(b+i, (j-i), start+i*tdel, tdel, calib, calper);
	    ts->addSegment(s);
	    i = j;
	}
    }
    Free(b);
    Free(count);
    Free(aver_calib);

    if(ts->length() <= 0) {
	ts->deleteObject();
	return NULL;
    }

    // Record the beam elements
    ts->beam_elements.clear();
    for(i = 0; i < wvec.size(); i++) {
	BeamSta beam_sta;
	strncpy(beam_sta.sta, wvec[i]->sta(), sizeof(beam_sta.sta));
	strncpy(beam_sta.chan, wvec[i]->chan(), sizeof(beam_sta.chan));
	beam_sta.wgt = weights[i];
	ts->beam_elements.push_back(beam_sta);
    }

    return ts;
}

/** Return a beamed waveform for a time window. Beam the input waveforms using
 *  the input time lags and weights, for the time window from tstart to tend
 *  only. The beamed waveform will be in counts.
 *  @param[in] num_waveforms the number of Waveform objects in wvec[].
 *  @param[in] wvec Waveform objects.
 *  @param[in] tlags the time lags for the waveforms.
 *  @param[in] weights the weights for the waveforms.
 *  @param[in] tstart the epochal start time of the time window to beam
 *  @param[in] tend the epochal end time of the time window to beam
 *  @param[in] coherent if true, the absolute values of the waveforms are
 *  summed to make the beam instead of the signed values.
 *  @returns the beam as a GTimeSeries object. Returns NULL if num_waveforms
 *  <= 0 or the number of beam samples would be 0.
 */
GTimeSeries * Beam::BeamSubSeries(gvector<Waveform *> &wvec,
		vector<double> &tlags, vector<double> &weights, double tstart,
		double tend, bool coherent)
{
    int		i, j, k, k0, k1, k2, npts;
    int		*count = NULL;
    float	*b = NULL;
    double	*aver_calib = NULL;
    double	tdel, cal, calper, end, start, w, norm=0.;
    GTimeSeries	*t, *ts;
    bool	calib_applied;

    if(wvec.size() <= 0) return NULL;
    for(i = (int)tlags.size(); i < wvec.size(); i++) tlags.push_back(0.);
    for(i = (int)weights.size(); i < wvec.size(); i++) weights.push_back(1.);

    start = wvec[0]->tbeg() + tlags[0];
    end = wvec[0]->tend() + tlags[0];

    for(i = 1; i < wvec.size(); i++)
    {
	if(start > wvec[i]->tbeg() + tlags[i]) {
	    start = wvec[i]->tbeg() + tlags[i];
	}
	if(end < wvec[i]->tend() + tlags[i]) {
	    end = wvec[i]->tend() + tlags[i];
	}
    }
    if(start < tstart) start = tstart;
    if(end > tend) end = tend;

    tdel = wvec[0]->segment(0)->tdel();
    npts = (int)((end - start)/tdel + .5) + 1;

    if( !(b = (float *)malloc(npts*sizeof(float))) ||
	!(count = (int *)malloc(npts*sizeof(int))) ||
	!(aver_calib = (double *)malloc(npts*sizeof(double))))
    {
	GError::setMessage("Beam.BeamSubSeries: malloc failed. Not enough information in the IDC tables to calculate fk-beam.");
	Free(aver_calib); Free(count); Free(b);
	throw(GERROR_MALLOC_ERROR);
    }
    for(i = 0; i < npts; i++) {
	b[i] = 0.;
	count[i] = 0;
	aver_calib[i] = 0.;
    }

    for(i = 0; i < wvec.size(); i++)
    {
	t = wvec[i]->ts;

	norm += weights[i];

	/* Determine if the calib has been applied.
	 */
	calib_applied = t->getMethod("CalibData") ? true : false;

	/* Sum with each waveform. Remove calib if necessary.
	 */
	for(j = 0; j < t->size(); j++)
	    if(t->segment(j)->tend() > start && t->segment(j)->tbeg() < end)
	{
	    GSegment *s = t->segment(j);
	    k0 = (int)((s->tbeg() + tlags[i] - start)/s->tdel() + .5);
	    k1 = 0;
	    if(k0 < 0) k1 = -k0;
	    k2 = s->length()-1;
	    if(k0+k2 > npts-1) k2 = npts-1-k0;

	    cal = (s->calib() != 0.) ? s->calib() : 1.;

	    w = weights[i];
	    if(calib_applied) w /= cal;

	    if(coherent)
	    {
		for(k = k1; k <= k2; k++) {
		    b[k0+k] += w*s->data[k];
		    count[k0+k] += 1;
		    aver_calib[k0+k] += cal;
		}
	    }
	    else
	    {
		for(k = k1; k <= k2; k++) {
		    b[k0+k] += w*(float)fabs((double)s->data[k]);
		    count[k0+k] += 1;
		    aver_calib[k0+k] += cal;
		}
	    }
	}
    }
    if(norm) {
	norm = 1./norm;
	for(i = 0; i < npts; i++) b[i] *= norm;
    }

    ts = new GTimeSeries();

    ts->source_info.setSource(wvec[0]->ts->source_info);

    calper = 0.;
    for(i = 0; i < wvec.size(); i++) {
	calper += wvec[i]->segment(0)->calper();
    }
    calper /= wvec.size();

    i = 0;
    while(i < npts)
    {
	GSegment *s;
	/* Only create GSegments for the periods where all (num_waveforms)
	 * elements have data.
	 */
	while(i < npts && count[i] != wvec.size()) i++;

	if(i < npts) {
	    double calib = aver_calib[i];
	    /* Look for the next point where the number of contributing
	     * elements is less than num_waveforms, or the calib
	     * changes. If the calib changes, start a new GSegment. Skip
	     * periods that have less than num_waveforms.
	     */
	    for(j = i+1; j < npts; j++) {
		if(count[j] != wvec.size() || (calib &&
			fabs((aver_calib[j] - calib)/calib) > .01)) break;
	    }
	    calib /= wvec.size();
	    s = new GSegment(b+i, (j-i), start+i*tdel, tdel, calib, calper);
	    ts->addSegment(s);
	    i = j;
	}
    }
    Free(b);
    Free(count);
    Free(aver_calib);

    // Record the beam elements
    ts->beam_elements.clear();
    for(i = 0; i < wvec.size(); i++) {
	BeamSta beam_sta;
	strncpy(beam_sta.sta, wvec[i]->sta(), sizeof(beam_sta.sta));
	strncpy(beam_sta.chan, wvec[i]->chan(), sizeof(beam_sta.chan));
	beam_sta.wgt = weights[i];
	ts->beam_elements.push_back(beam_sta);
    }

    return ts;
}
