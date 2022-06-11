#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#ifdef HAVE_NETCDF

#define Free(p) if(p != NULL) {free(p); p = NULL;}

#include "libgCDF.h"

static void handle_error(int status);
static void cdfLimits(cdfInfo_t * cdf);
static void reportLimits(cdfInfo_t cdf);

static int initCDFfile(char * filename, cdfInfo_t * cdf, int verbose);

static int
extractData(cdfInfo_t cdf, cdfData_t * cdfData, int verbose);
static int
adjustCoords(cdfInfo_t cdf, cdfData_t * cdfData);
static void
adjustLon(float minX, float maxX, float inLat, float *outLat);
static void
adjustLat(float minY, float maxY, float inLat, float *outLat);
static int
findLonIndex(cdfInfo_t cdf, float lon, int *index, int first);
static int
findLatIndex(cdfInfo_t cdf, float lat, int *index, int first);
static int
allocDataArray(cdfData_t * cdfData);

static void
adjustXYvals(cdfData_t * cdfData);

static int
mergeBoxes(cdfData_t cdfLeftBox, cdfData_t cdfRightBox, cdfData_t * cdfData, int verbose);

#ifdef _MAIN
int
main()
{
	char	filename[256];
	cdfData_t	cdfData;
	int		status;
	int		verbose;
int i, j, k;
	double	*darr = NULL;
	float	*farr = NULL;
	short	*sarr = NULL;
	int	*iarr = NULL;

	strcpy(filename, "etopo5.nc");
	cdfData.leftLon = -180.;
	cdfData.rightLon = 180.;
	cdfData.bottomLat = -90.0;
	cdfData.topLat = 90.0;

	cdfData.reqType = NC_SHORT;

	cdfData.xReqPix = 0;
	cdfData.yReqPix = 0;

	verbose = 0;
	status = getCdfData(filename, &cdfData, verbose);
	if (status != NC_NOERR) handle_error(status);  

if (verbose)
{
	printf("%d x values\n", cdfData.nXdim);
	for (i=0; i<cdfData.nXdim; i++)
	   printf("%.3f ",  cdfData.Xvals[i]);
	printf("\n");

	printf("%d y values\n", cdfData.nYdim);
	for (i=0; i<cdfData.nYdim; i++)
	   printf("%.3f ",  cdfData.Yvals[i]);
	printf("\n");

	k = 0;
	if (cdfData.reqType == NC_FLOAT)
	{
	    farr = (float *) cdfData.data;
	    for (j=0; j<cdfData.nYdim; j++)
	    {
	        for (i=0; i<cdfData.nXdim; i++)
	        {
		    printf("%.4f %.4f %.3f\n",
		        cdfData.Xvals[i], cdfData.Yvals[j],  farr[k++]);
	        }
	    }
	}
	else if (cdfData.reqType == NC_DOUBLE)
	{
	    darr = (double *) cdfData.data;
	    for (j=0; j<cdfData.nYdim; j++)
	    {
	        for (i=0; i<cdfData.nXdim; i++)
	        {
		    printf("%.4f %.4f %.3f\n",
		        cdfData.Xvals[i], cdfData.Yvals[j],  darr[k++]);
	        }
	    }
	}
	else if (cdfData.reqType == NC_SHORT)
	{
	    sarr = (short *) cdfData.data;

	    for (j=0; j<cdfData.nYdim; j++)
	    {
	        for (i=0; i<cdfData.nXdim; i++)
	        {
		    printf("%.4f %.4f %d\n",
		        cdfData.Xvals[i], cdfData.Yvals[j],  sarr[k++]);
	        }
	    }
	}
	else if (cdfData.reqType == NC_INT)
	{
	    iarr = (int *) cdfData.data;

	    for (j=0; j<cdfData.nYdim; j++)
	    {
	        for (i=0; i<cdfData.nXdim; i++)
	        {
		    printf("%.4f %.4f %d\n",
		        cdfData.Xvals[i], cdfData.Yvals[j],  iarr[k++]);
	        }
	    }
	}
}
	status = freeCdfData(&cdfData);

	exit (0);
}
#endif /* _MAIN */

int
freeCdfData(cdfData_t * cdfData)
{

	Free(cdfData->Xvals);
	Free(cdfData->Yvals);
	Free(cdfData->data);

	cdfData->nXdim = 0;
	cdfData->nYdim = 0;

	return (NC_NOERR);
}


int
getCdfData(char	* filename, cdfData_t * cdfData, int verbose)
{
	int status; /* error status */ 
	cdfInfo_t	cdf;
	cdfData_t	cdfLeftBox;
	cdfData_t	cdfRightBox;

	status = initCDFfile(filename, &cdf, verbose);
	if (status != NC_NOERR) return(status);  

	if (verbose) reportLimits(cdf);

	status = adjustCoords(cdf, cdfData);
	if (status != NC_NOERR) handle_error(status);  

	if (cdfData->adjustedRightLon > cdfData->adjustedLeftLon)
	{
	    status = extractData(cdf, cdfData, verbose);
	}
	/* data cross the lon boundary of this data set. Need to request two blocks of data,
	   and merge the result
	 */
	else
	{
	    cdfLeftBox.reqType = cdfData->reqType;
	    cdfLeftBox.adjustedLeftLon = cdf.minX;
	    cdfLeftBox.adjustedRightLon = cdfData->adjustedRightLon;
	    cdfLeftBox.adjustedTopLat = cdfData->adjustedTopLat;
	    cdfLeftBox.adjustedBottomLat = cdfData->adjustedBottomLat;
	    status = extractData(cdf, &cdfLeftBox, verbose);

	    cdfRightBox.reqType = cdfData->reqType;
	    cdfRightBox.adjustedLeftLon = cdfData->adjustedLeftLon;
	    cdfRightBox.adjustedRightLon = cdf.maxX;
	    cdfRightBox.adjustedTopLat = cdfData->adjustedTopLat;
	    cdfRightBox.adjustedBottomLat = cdfData->adjustedBottomLat;
	    status = extractData(cdf, &cdfRightBox, verbose);
printf("crosses the boundary\n");

	    mergeBoxes(cdfLeftBox, cdfRightBox, cdfData, verbose);
	
	    status = freeCdfData(&cdfLeftBox);
	    status = freeCdfData(&cdfRightBox);
	}
	if (status != NC_NOERR) handle_error(status);  

	Free(cdf.Xvals);
	Free(cdf.Yvals);
	cdf.nXdim = 0;
	cdf.nYdim = 0;

	adjustXYvals(cdfData);

	return (NC_NOERR);
}

/* since the CDF coordinate system can be shifted, we need to correct 
   the lat and lon values which may be outside the expected range of 
   lat (-90 to 90) and lon (-180 to 180)
*/

static void
adjustXYvals(cdfData_t * cdfData)
{
	int	i;

	/* if any X values are > 180, subtract 360 from all Xvals */
	for (i=0; i<cdfData->nXdim; i++)
	{
		if (cdfData->Xvals[i] >  180.0) break;
	}
	if (i<cdfData->nXdim)
	{
	    for (i=0; i<cdfData->nXdim; i++)
	    {
		cdfData->Xvals[i] -= 360.;
	    }
	}

	/* if any X values are < i-180, add 360 from all Xvals */
	for (i=0; i<cdfData->nXdim; i++)
	{
		if (cdfData->Xvals[i] < -180.0) break;
	}
	if (i<cdfData->nXdim)
	{
	    for (i=0; i<cdfData->nXdim; i++)
	    {
		cdfData->Xvals[i] += 360.;
	    }
	}

	/* if any Y values are > 90, subtract 180 from all Yvals */
	for (i=0; i<cdfData->nYdim; i++)
	{
		if (cdfData->Yvals[i] >  90.0) break;
	}
	if (i<cdfData->nYdim)
	{
	    for (i=0; i<cdfData->nYdim; i++)
	    {
		cdfData->Yvals[i] -= 180.;
	    }
	}

	/* if any Y values are < -90, add 180 to all Yvals */
	for (i=0; i<cdfData->nYdim; i++)
	{
		if (cdfData->Yvals[i] < -90.0) break;
	}
	if (i<cdfData->nYdim)
	{
	    for (i=0; i<cdfData->nYdim; i++)
	    {
		cdfData->Yvals[i] += 180.;
	    }
	}

	cdfData->minX = cdfData->Xvals[0];
	cdfData->maxX = cdfData->Xvals[cdfData->nXdim-1];
	cdfData->minY = cdfData->Yvals[0];
	cdfData->maxY = cdfData->Yvals[cdfData->nYdim-1];
}
static int
mergeBoxes(cdfData_t cdfLeftBox, cdfData_t cdfRightBox, cdfData_t * cdfData, int verbose)
{
	size_t	rightBoxNbytes;
	size_t	leftBoxNbytes;
	int	i, j, k;

	/* add 360 to all X values (longitiude) in the left box.
	   this will essentially translate the left box to the right side of the right box
	 */
	for (i=0; i<cdfLeftBox.nXdim; i++)
	{
		cdfLeftBox.Xvals[i] += 360.0;
	}
	cdfLeftBox.minX += 360.;
	cdfLeftBox.maxX += 360.;

	cdfData->nYdim = cdfLeftBox.nYdim;

	cdfData->nXdim = cdfLeftBox.nXdim + cdfRightBox.nXdim;
	/* if the longitude values are the same, skip the first column of numbers from the left box */
	 
	if (cdfLeftBox.Xvals[0] == cdfRightBox.Xvals[cdfRightBox.nXdim-1])
	{
	    cdfData->nXdim--;
	}

	cdfData->Xvals = (float *) malloc(cdfData->nXdim * sizeof(float));
	cdfData->Yvals = (float *) malloc(cdfData->nYdim * sizeof(float));

	/* data array can be of different types */
	allocDataArray(cdfData);

	for (i=0; i<cdfLeftBox.nYdim; i++)
	{
	    cdfData->Yvals[i] = cdfLeftBox.Yvals[i];
	}

	for (i=0, j=0; i<cdfRightBox.nXdim; i++)
	{
	    cdfData->Xvals[j++] = cdfRightBox.Xvals[i];
	}
	for (i=0; i<cdfLeftBox.nXdim; i++)
	{
	    cdfData->Xvals[j++] = cdfLeftBox.Xvals[i];
	}
for (i=0; i<cdfData->nXdim; i++)
{
    printf("%f ", cdfData->Xvals[i]);
    if (i%10 == 0) printf("\n");
}
printf("\n");

	rightBoxNbytes = cdfRightBox.nXdim * cdfLeftBox.reqDataSize;
	leftBoxNbytes  = cdfLeftBox.nXdim  * cdfLeftBox.reqDataSize;

	for (i=0, k=0; i<cdfLeftBox.nYdim; i++)
	{
	    j = i*cdfRightBox.nXdim;
	    memcpy((char *)cdfData->data+k*cdfLeftBox.reqDataSize, 
		   (char *)cdfRightBox.data+j*cdfLeftBox.reqDataSize, 
		   rightBoxNbytes);
	    k += cdfRightBox.nXdim;

	    j = i*cdfLeftBox.nXdim;
	    memcpy((char *)cdfData->data+k*cdfLeftBox.reqDataSize, 
		   (char *)cdfLeftBox.data+j*cdfLeftBox.reqDataSize, 
		   leftBoxNbytes);
	    k += cdfLeftBox.nXdim;
	}

	return (NC_NOERR);
}

static int 
initCDFfile(char * filename, cdfInfo_t * cdf, int verbose)
{
	int i, j;
	size_t dim[NC_MAX_VAR_DIMS];
	int ndims, nvars, ngatts, unlimdimid;
        char recname[NC_MAX_NAME];
	int	i_arr[200];

	nc_type my_type;
	int my_dims[NC_MAX_VAR_DIMS];
	int my_ndims;
	int my_natts;
	int status;

	status = nc_open(filename, NC_NOWRITE, &cdf->ncid);
	if (status != NC_NOERR) return(status);  

	status = nc_inq(cdf->ncid, &ndims, &nvars, &ngatts, &unlimdimid); 
	if (status != NC_NOERR) return(status);

/*
fprintf(stderr, "ndims=%d nvars=%d ngatts=%d unlimdimid=%d\n", ndims, nvars, ngatts, unlimdimid);
*/

	for (i=0; i<ndims; i++)
	{
		nc_inq_dim(cdf->ncid, i, recname, &dim[i]);
		if (verbose) printf("dim[%d] = %s  %d \n", i, recname, (int)dim[i]);
	}

	for (i=0; i<nvars; i++)
	{
		nc_inq_var(cdf->ncid, i, recname, &my_type, &my_ndims, my_dims,
			&my_natts);

		    if (verbose) printf("var[%d] = %s  ndims = %d  natts = %d   \n", 
			i, recname, my_ndims, my_natts);

	
		if (my_type == NC_DOUBLE || my_type == NC_FLOAT)
		{
		    /* this is an assumption here */
		    /* if (i == 0) */
		    if (i < 2 && (strstr(recname, "x") || strstr(recname, "X")))
		    {
			cdf->Xid = i;
			cdf->nXdim = dim[i];
			cdf->Xvals = (float *) malloc(sizeof(float) * dim[i]);
		    	nc_get_var_float(cdf->ncid, i, cdf->Xvals);

			if (verbose)
			{
		    	  for (j=0; j < (signed int)dim[i]; j++)
		    	  {
			    printf(" %f ", cdf->Xvals[j]);
			    if (j > 0 && j%10 == 0) printf("\n");
			  }
		    	  printf("\n");
		    	}
		    }
		    /* else if (i == 1) */
		    else if (i < 2 && (strstr(recname, "y") || strstr(recname, "Y")))
		    {
			cdf->Yid = i;
			cdf->nYdim = dim[i];
			cdf->Yvals = (float *) malloc(sizeof(float) * dim[i]);
		    	nc_get_var_float(cdf->ncid, i, cdf->Yvals);

			if (verbose)
		        {
		    	  for (j=0; j < (signed int)dim[i]; j++)
		    	  {
			    printf(" %f ", cdf->Yvals[j]);
			    if (j > 0 && j%10 == 0) printf("\n");
		    	  }
		    	  printf("\n");
			}
		    }
		}
		else if (my_type == NC_INT)
		{
		    nc_get_var_int(cdf->ncid, i, i_arr);
		    if (verbose) printf("var[%d] = %s  ndims = %d  natts = %d  \n", 
			i, recname, my_ndims, my_natts);
		}
		else
		{
		    if (verbose) printf("var[%d] = %s  ndims = %d  natts = %d   \n", 
			i, recname, my_ndims, my_natts);
		}
	    if (verbose)
	    {
	      for (j=0; j<my_natts; j++)
	      {
		nc_inq_attname(cdf->ncid, i, j, recname);
		printf("  atts[%d] = %s\n", j, recname);
	      }
	    }
	}

	/* assumption */
	cdf->dataId = 2;

	/* store the cdf limits */
	cdfLimits(cdf);

	return NC_NOERR;
}



static void
handle_error(int status)
{
	if (status != NC_NOERR)
	{
		fprintf(stderr, "%s\n", nc_strerror(status));
		exit (-1);
	}
}

static void
cdfLimits(cdfInfo_t * cdf)
{
	if (cdf->nXdim > 0)
	{
		cdf->minX = cdf->Xvals[0];
		cdf->maxX = cdf->Xvals[cdf->nXdim-1];
		cdf->intervalX = cdf->Xvals[1] - cdf->Xvals[0];
	}

	if (cdf->nYdim > 0)
	{
		cdf->minY = cdf->Yvals[0];
		cdf->maxY = cdf->Yvals[cdf->nYdim-1];
		cdf->intervalY = cdf->Yvals[1] - cdf->Yvals[0];
	}
}

static void 
reportLimits(cdfInfo_t cdf)
{
        if (cdf.nXdim > 0)
        {
		printf("minX = %f  maxX = %f  intervalX = %f\n",
			cdf.minX, cdf.maxX, cdf.intervalX);
        }

        if (cdf.nYdim > 0)
        {
		printf("minY = %f  maxY = %f  intervalY = %f\n",
			cdf.minY, cdf.maxY, cdf.intervalY);
	}
}

static int
adjustCoords(cdfInfo_t cdf, cdfData_t * cdfData)
{
	/* may need to adjust the coordinates for the data set */
	adjustLon(cdf.minX, cdf.maxX, cdfData->leftLon, &cdfData->adjustedLeftLon);
	adjustLon(cdf.minX, cdf.maxX, cdfData->rightLon, &cdfData->adjustedRightLon);
	adjustLat(cdf.minY, cdf.maxY, cdfData->bottomLat, &cdfData->adjustedBottomLat);
	adjustLat(cdf.minY, cdf.maxY, cdfData->topLat, &cdfData->adjustedTopLat);

	return NC_NOERR;
}

/**
 * @param leftLon 	left longitude of requested area. Must be between -180 and 180
 * @param rightLon 	right longitude of requested area. Must be between -180 and 180
 * @param bottomLat	bottom latitude of requested area. Must be between -90 and 90
 * @param topLat	top latitude of requested area. Must be between -90 and 90

minX = 20.166667  maxX = 380.166631  intervalX = 0.333333
minY = -89.833333  maxY = 89.833315  intervalY = 0.333333
*/
static int
extractData(cdfInfo_t cdf, cdfData_t * cdfData, int verbose)
{
	int	leftIndex, rightIndex;
	int	bottomIndex, topIndex;
	int	status;
	int	i;
	double	*darr;

	size_t start[] = {4000, 4000};
	size_t count[] = {10, 10};

	if (verbose)
	{
	    printf("extractData: adjuested left lon: %f     right lon:  %f\n", 
		cdfData->adjustedLeftLon, cdfData->adjustedRightLon);
	    printf("extractData: bottom lat:  %f   top lat:  %f\n", 
		cdfData->adjustedBottomLat, cdfData->adjustedTopLat);
	}

	/* find the index for each boundary */
	findLonIndex(cdf, cdfData->adjustedLeftLon, &leftIndex, 1);
	findLonIndex(cdf, cdfData->adjustedRightLon, &rightIndex, 0);

	findLatIndex(cdf, cdfData->adjustedBottomLat, &bottomIndex, 1);
	findLatIndex(cdf, cdfData->adjustedTopLat, &topIndex, 0);

	if (verbose)
	{
	    printf("extractData: lon indexes: %d %d\n", leftIndex, rightIndex);
	    printf("extractData: lat indexes: %d %d\n", bottomIndex, topIndex);
	}

	cdfData->intervalX = cdf.intervalX;
	cdfData->intervalY = cdf.intervalY;

	/* assumption */
	if (cdf.Xid == 0 && cdf.Yid == 1)
	{
	    start[0] = bottomIndex;
	    start[1] = leftIndex;

	    count[0] = topIndex - bottomIndex + 1;
	    if (count[0] < 1) count[0] = 1;
	    count[1] = rightIndex - leftIndex + 1;
	    if (count[1] < 1) count[1] = 1;
    
	    cdfData->nXdim = count[1];
	    cdfData->nYdim = count[0];

	    cdfData->Xvals = (float *) malloc(cdfData->nXdim * sizeof(float));
	    cdfData->Yvals = (float *) malloc(cdfData->nYdim * sizeof(float));
    
	    for (i=0; i<cdfData->nXdim; i++)
	    {
		cdfData->Xvals[i] = cdf.Xvals[start[1]+i];
	    }
	    for (i=0; i<cdfData->nYdim; i++)
	    {
		cdfData->Yvals[i] = cdf.Yvals[start[0]+i];
	    }
	}
	else
	{
	    start[0] = bottomIndex;
	    start[1] = leftIndex;

	    count[0] = topIndex - bottomIndex + 1;
	    if (count[0] < 1) count[0] = 1;
	    count[1] = rightIndex - leftIndex + 1;
	    if (count[1] < 1) count[1] = 1;
    
	    cdfData->nXdim = count[1];
	    cdfData->nYdim = count[0];

	    cdfData->Xvals = (float *) malloc(cdfData->nXdim * sizeof(float));
	    cdfData->Yvals = (float *) malloc(cdfData->nYdim * sizeof(float));
    
	    for (i=0; i<cdfData->nXdim; i++)
	    {
		cdfData->Xvals[i] = cdf.Xvals[start[1]+i];
	    }
	    for (i=0; i<cdfData->nYdim; i++)
	    {
		cdfData->Yvals[i] = cdf.Yvals[start[0]+i];
	    }
	}

	cdfData->minX = cdfData->Xvals[0];
	cdfData->maxX = cdfData->Xvals[cdfData->nXdim-1];
	cdfData->minY = cdfData->Yvals[0];
	cdfData->maxY = cdfData->Yvals[cdfData->nYdim-1];

	status = allocDataArray(cdfData);
	if (status != NC_NOERR) return(status);

	if (cdfData->reqType == NC_SHORT)
	{
	    status = nc_get_vara_short(cdf.ncid, cdf.dataId, start, count,
 (short int*)cdfData->data); 
	}
	else if (cdfData->reqType == NC_INT)
	{
	    status = nc_get_vara_int(cdf.ncid, cdf.dataId, start, count, 
(int*)cdfData->data); 
	}
	else if (cdfData->reqType == NC_DOUBLE)
	{
	    darr = (double *)cdfData->data;

	    status = nc_get_vara_double(cdf.ncid, cdf.dataId, start, count, darr); 
	}
	else if (cdfData->reqType == NC_FLOAT)
	{
	    status = nc_get_vara_float(cdf.ncid, cdf.dataId, start, count, 
(float*)cdfData->data); 
	}
	/* should really set status and return */
	else
	{
		printf("cdfData->reqType set to unknown value: %d\n",
			cdfData->reqType);
		exit(1);
	}

	if (status != NC_NOERR) return(status);

	if (verbose)
	{
printf("extractData: start[0]=%d  start[1]=%d  count[0]=%d  count[1]=%d\n",
  (int)start[0], (int)start[1], (int)count[0], (int)count[1]);

printf("extractData:  after nc_get_vara_double()\n");
printf("extractData:  start at %d %d\n", (int)start[0], (int)start[1]);
printf("extractData:  count    %d %d\n", (int)count[0], (int)count[1]);

/*
for (i=0; i<20; i++)
 {
	printf("%.2f ", cdfData->data[i]);
 }
printf("\n");
*/

	}
	return NC_NOERR;
}

static int
allocDataArray(cdfData_t * cdfData)
{
	if (cdfData->reqType == NC_SHORT)
	{
	    cdfData->data = (short *) malloc(cdfData->nXdim * cdfData->nYdim * sizeof(short));
	    cdfData->reqDataSize = sizeof(short);
	}
	else if (cdfData->reqType == NC_INT)
	{
	    cdfData->data = (int *) malloc(cdfData->nXdim * cdfData->nYdim * sizeof(int));
	    cdfData->reqDataSize = sizeof(int);
	}
	else if (cdfData->reqType == NC_DOUBLE)
	{
	    cdfData->data = (double *) malloc(cdfData->nXdim * cdfData->nYdim * sizeof(double));
	    cdfData->reqDataSize = sizeof(double);
	}
	else if (cdfData->reqType == NC_FLOAT)
	{
	    cdfData->data = (float *) malloc(cdfData->nXdim * cdfData->nYdim * sizeof(float));
	    cdfData->reqDataSize = sizeof(float);
	}
	return NC_NOERR;
}

static void
adjustLon(float minX, float maxX, float inLon, float *outLon)
{
	/* inLon is inside the box */
	if (inLon >= minX && inLon <= maxX)
	{
		*outLon = inLon;
	}
	
	else if (inLon < minX)
	{
	    if ((inLon + 360) <= maxX)
	    {
		*outLon = inLon + 360.;
	    }
	    else if ((inLon + 360) > maxX)
	    {
		*outLon = minX;
	    }
	}
	else if (inLon > maxX)
	{
	    if ((inLon - 360) >= minX)
	    {
		*outLon = inLon - 360.;
	    }
	    else if ((inLon - 360) < minX)
	    {
		*outLon = maxX;
	    }
	}
}

/*
  currently we assume that all lat values will be between -90 and 90, 
  so there is no attempt to translate lat coordinates, 
*/
static void
adjustLat(float minY, float maxY, float inLat, float *outLat)
{
	/* inLat is inside the box */
	if (inLat >= minY && inLat <= maxY)
	{
		*outLat = inLat;
	}
	else if (inLat < minY)
	{
		*outLat = minY;
	}
	else if (inLat > maxY)
	{
		*outLat = maxY;
	}
}

static int
findLonIndex(cdfInfo_t cdf, float lon, int *index, int first)
{
	int	i;

	*index = -1;

	for (i=0; i<cdf.nXdim-1; i++)
	{
		if (lon >= cdf.Xvals[i] && lon < cdf.Xvals[i+1])
		{
			if (first && lon > cdf.Xvals[i]) *index = i+1;
			else *index = i;
			break;
		}
	}

	/* index was not found */
	if (i == cdf.nXdim-1)
	{
		if (lon <= cdf.Xvals[i] + 0.01 &&
		    lon >  cdf.Xvals[i-1])
		{
			*index = i;
		}
	}

	return i;
}

static int
findLatIndex(cdfInfo_t cdf, float lat, int *index, int first)
{
	int	i;

	*index = -1;

	for (i=0; i<cdf.nYdim-1; i++)
	{
		if (lat >= cdf.Yvals[i] && lat < cdf.Yvals[i+1])
		{
			if (first && lat > cdf.Yvals[i]) *index = i+1;
			else *index = i;
			break;
		}
	}

	/* index was not found */
	if (i == cdf.nYdim-1)
	{
		if (lat <= cdf.Yvals[i] + 0.01 &&
		    lat >  cdf.Yvals[i-1])
		{
			*index = i;
		}
	}

	return i;
}

#endif /* HAVE_NETCDF */
