#ifndef _GCDF_H_
#define _GCDF_H_

#include <netcdf.h> 

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */


typedef struct cdfInfo
{
	int	ncid;

	int	Xid;
	int	nXdim;
	float	*Xvals;
	float	minX;
	float	maxX;
	float	intervalX;

	int	Yid;
	int	nYdim;
	float	*Yvals;
	float	minY;
	float	maxY;
	float	intervalY;

	int	dataId;
} cdfInfo_t;

/**
 *  the structure containing the boundary of the requested area, and the output data
 *
 *  @member
 *  @member leftLon left longitude of requested area. -180 to 180 
 *  @member rightLon right longitude of requested area. -180 to 180
 *  @member bottomLat bottom latitude of requested area. -90 to 90
 *  @member topLat top latitude of requested area. -90 to 90 

 *  @member xReqPix requested number of pixels in X direction
 *          this will be used to determine the best dataset to read if > 10
 *  @member yReqPix requested number of pixels in Y direction
 *          this will be used to determine the best dataset to read if > 10
 *  @member reqType requested output data type. Can be NC_DOUBLE, NC_FLOAT, NC_INT, NC_SHORT
 *          reqType should be know when reading data (which is void *)

 *  @member nXdim number of output data values in X direction
 *  @member *Xvals array of X (longitude) values in X direction
 *  @member minX leftmost longitude value in output data 
 *  @member maxX rightmost longitude value in output data
 *  @member intervalX spacing of values in X direction 

 *  @member nYdim number of output data values in Y direction
 *  @member *Yvals array of  (latitude) values in Y direction
 *  @member minY bottommost latitude values in output data 
 *  @member maxY topmost latitude values in output data 
 *  @member intervalY spacing of values in Y direction 

 *  @member *data output data. first value in upper left corner, followed
				   by first row of data
 */

/*
from etopo40.nc
        float ROSE(ETOPO40Y, ETOPO40X) ;
                ROSE:missing_value = -1.e+34f ;
                ROSE:_FillValue = -1.e+34f ;
                ROSE:long_name = "RELIEF OF THE SURFACE OF THE EARTH" ;


*/

typedef struct cdfData
{
	float	leftLon;	
	float	rightLon;
	float 	bottomLat;
	float	topLat;	

	float	adjustedLeftLon;	
	float	adjustedRightLon;
	float 	adjustedBottomLat;
	float	adjustedTopLat;	

	int	xReqPix;
	int	yReqPix;
	int	reqType;
	int	reqDataSize;

	int	nXdim;
	float	*Xvals;
	float	minX;	
	float	maxX;
	float	intervalX;

	int	nYdim;	
	float	*Yvals;
	float	minY;
	float	maxY;
	float	intervalY;

	void	*data;	
} cdfData_t;

#define ETOPO2 2
#define GLOBE  3
#define SRTM1  4
#define SRTM30  5

int getCdfData(char * filename, cdfData_t * cdfData, int verbose);
int freeCdfData(cdfData_t * cdfData);

int getRawGriddedData(char *root, int type, cdfData_t * cdfData, int verbose);


#endif /* _GCDF_H_ */
