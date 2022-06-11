#include "config.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_NETCDF

#include "libgCDF.h"

typedef struct Tile
{
	    char  filename[16];
	    float	minLat;
	    float	maxLat;
	    float	minLon;
	    float	maxLon;
	    short	minElev;
	    short	maxElev;
	    int		nCol;
	    int		nRow;
	    int		type;	
	    int		endian;
} Tile_t;

/* type is one of NC_DOUBLE, NC_FLOAT, NC_INT, NC_SHORT */


static int
findInputTiles(cdfData_t * cdfData, int nInputTiles, Tile_t * tiles, int * nTiles, Tile_t * tileList);

static int
copyData(short * in, float *out, int npts, int endian);
static int
allocDataArray(cdfData_t * cdfData);
static void
adjustLat(float minX, float maxX, float inLon, float *outLon);
static void
adjustLon(float minX, float maxX, float inLon, float *outLon);

int
getGlobeData(char *root, Tile_t tile, cdfData_t *cdfData);
int
showCorners(cdfData_t cdfData);

#ifdef _MAIN
int
main()
{
	cdfData_t cdfData;
	char	root[256];
#ifdef HAVE_LIBZ
	Tile_t tiles [] = 
	{
	    {"etopo2.sun.gz", -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 0},
	    {"etopo2.pc.gz",  -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 1},
	    {"a10g.gz",  50.,  90., -180., -90.,   1, 6098, 10800, 4800, NC_SHORT, 1},
	    {"b10g.gz",  50.,  90.,  -90.,   0.,   1, 3940, 10800, 4800, NC_SHORT, 1},
	    {"c10g.gz",  50.,  90.,    0.,  90., -30, 4010, 10800, 4800, NC_SHORT, 1},
	    {"d10g.gz",  50.,  90.,   90., 180.,   1, 4588, 10800, 4800, NC_SHORT, 1},
	    {"e10g.gz",   0.,  50., -180., -90., -84, 5443, 10800, 6000, NC_SHORT, 1},
	    {"f10g.gz",   0.,  50.,  -90.,   0., -40, 6085, 10800, 6000, NC_SHORT, 1},
	    {"g10g.gz",   0.,  50.,    0.,  90.,-407, 8752, 10800, 6000, NC_SHORT, 1},
	    {"h10g.gz",   0.,  50.,   90., 180., -63, 7491, 10800, 6000, NC_SHORT, 1},
	    {"i10g.gz", -50.,   0., -180., -90.,   1, 2732, 10800, 6000, NC_SHORT, 1},
	    {"j10g.gz", -50.,   0.,  -90.,   0.,-127, 6798, 10800, 6000, NC_SHORT, 1},
	    {"k10g.gz", -50.,   0.,    0.,  90.,   1, 5825, 10800, 6000, NC_SHORT, 1},
	    {"l10g.gz", -50.,   0.,   90., 180.,   1, 5179, 10800, 6000, NC_SHORT, 1},
	    {"l10b.gz", -50.,   0.,   90., 180., -34, 5179, 10800, 6000, NC_SHORT, 1},
	    {"m10g.gz", -90., -50., -180., -90.,   1, 4009, 10800, 4800, NC_SHORT, 1},
	    {"n10g.gz", -90., -50.,  -90.,   0.,   1, 4743, 10800, 4800, NC_SHORT, 1},
	    {"o10g.gz", -90., -50.,    0.,  90.,   1, 4039, 10800, 4800, NC_SHORT, 1},
	    {"p10g.gz", -90., -50.,   90., 180.,   1, 4363, 10800, 4800, NC_SHORT, 1},
	};
#else
	Tile_t tiles [] = 
	{
	    {"etopo2.sun", -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 0},
	    {"etopo2.pc",  -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 1},
	    {"a10g",  50.,  90., -180., -90.,   1, 6098, 10800, 4800, NC_SHORT, 1},
	    {"b10g",  50.,  90.,  -90.,   0.,   1, 3940, 10800, 4800, NC_SHORT, 1},
	    {"c10g",  50.,  90.,    0.,  90., -30, 4010, 10800, 4800, NC_SHORT, 1},
	    {"d10g",  50.,  90.,   90., 180.,   1, 4588, 10800, 4800, NC_SHORT, 1},
	    {"e10g",   0.,  50., -180., -90., -84, 5443, 10800, 6000, NC_SHORT, 1},
	    {"f10g",   0.,  50.,  -90.,   0., -40, 6085, 10800, 6000, NC_SHORT, 1},
	    {"g10g",   0.,  50.,    0.,  90.,-407, 8752, 10800, 6000, NC_SHORT, 1},
	    {"h10g",   0.,  50.,   90., 180., -63, 7491, 10800, 6000, NC_SHORT, 1},
	    {"i10g", -50.,   0., -180., -90.,   1, 2732, 10800, 6000, NC_SHORT, 1},
	    {"j10g", -50.,   0.,  -90.,   0.,-127, 6798, 10800, 6000, NC_SHORT, 1},
	    {"k10g", -50.,   0.,    0.,  90.,   1, 5825, 10800, 6000, NC_SHORT, 1},
	    {"l10g", -50.,   0.,   90., 180.,   1, 5179, 10800, 6000, NC_SHORT, 1},
	    {"l10b", -50.,   0.,   90., 180., -34, 5179, 10800, 6000, NC_SHORT, 1},
	    {"m10g", -90., -50., -180., -90.,   1, 4009, 10800, 4800, NC_SHORT, 1},
	    {"n10g", -90., -50.,  -90.,   0.,   1, 4743, 10800, 4800, NC_SHORT, 1},
	    {"o10g", -90., -50.,    0.,  90.,   1, 4039, 10800, 4800, NC_SHORT, 1},
	    {"p10g", -90., -50.,   90., 180.,   1, 4363, 10800, 4800, NC_SHORT, 1},
	};
#endif /* HAVE_LIBZ */

        cdfData.leftLon = 30.0;
        cdfData.rightLon = 31.0;
        cdfData.topLat = 0.;
        cdfData.bottomLat = -40.0;

/* request type was set coming into this routine!
        cdfData.reqType = NC_SHORT;
*/
        cdfData.xReqPix = 0;
        cdfData.yReqPix = 0;
/*
getRawGriddedData("./", GLOBE, &cdfData, 1);
*/
	strcpy(root, "/media/card/download");
	getGlobeData(root, tiles[12], &cdfData);

	showCorners(cdfData);

	exit (0);
}
#endif

int 
getRawGriddedData(char *root, int type, cdfData_t * cdfData, int verbose)
{
	Tile_t tileList[20];
	int	nTiles;
	int	nInputTiles;

#ifdef HAVE_LIBZ
	Tile_t tiles [20] = 
	{
	    {"etopo2.sun.gz", -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 0},
	    {"etopo2.pc.gz",  -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 1},
	    {"a10g.gz",  50.,  90., -180., -90.,   1, 6098, 10800, 4800, NC_SHORT, 1},
	    {"b10g.gz",  50.,  90.,  -90.,   0.,   1, 3940, 10800, 4800, NC_SHORT, 1},
	    {"c10g.gz",  50.,  90.,    0.,  90., -30, 4010, 10800, 4800, NC_SHORT, 1},
	    {"d10g.gz",  50.,  90.,   90., 180.,   1, 4588, 10800, 4800, NC_SHORT, 1},
	    {"e10g.gz",   0.,  50., -180., -90., -84, 5443, 10800, 6000, NC_SHORT, 1},
	    {"f10g.gz",   0.,  50.,  -90.,   0., -40, 6085, 10800, 6000, NC_SHORT, 1},
	    {"g10g.gz",   0.,  50.,    0.,  90.,-407, 8752, 10800, 6000, NC_SHORT, 1},
	    {"h10g.gz",   0.,  50.,   90., 180., -63, 7491, 10800, 6000, NC_SHORT, 1},
	    {"i10g.gz", -50.,   0., -180., -90.,   1, 2732, 10800, 6000, NC_SHORT, 1},
	    {"j10g.gz", -50.,   0.,  -90.,   0.,-127, 6798, 10800, 6000, NC_SHORT, 1},
	    {"k10g.gz", -50.,   0.,    0.,  90.,   1, 5825, 10800, 6000, NC_SHORT, 1},
	    {"l10g.gz", -50.,   0.,   90., 180.,   1, 5179, 10800, 6000, NC_SHORT, 1},
	    {"l10b.gz", -50.,   0.,   90., 180., -34, 5179, 10800, 6000, NC_SHORT, 1},
	    {"m10g.gz", -90., -50., -180., -90.,   1, 4009, 10800, 4800, NC_SHORT, 1},
	    {"n10g.gz", -90., -50.,  -90.,   0.,   1, 4743, 10800, 4800, NC_SHORT, 1},
	    {"o10g.gz", -90., -50.,    0.,  90.,   1, 4039, 10800, 4800, NC_SHORT, 1},
	    {"p10g.gz", -90., -50.,   90., 180.,   1, 4363, 10800, 4800, NC_SHORT, 1},
	};
#else
	Tile_t tiles [20] = 
	{
	    {"etopo2.sun", -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 0},
	    {"etopo2.pc",  -90.,  90., -180., 180.,   1, 6098, 10800, 5400, NC_SHORT, 1},
	    {"a10g",  50.,  90., -180., -90.,   1, 6098, 10800, 4800, NC_SHORT, 1},
	    {"b10g",  50.,  90.,  -90.,   0.,   1, 3940, 10800, 4800, NC_SHORT, 1},
	    {"c10g",  50.,  90.,    0.,  90., -30, 4010, 10800, 4800, NC_SHORT, 1},
	    {"d10g",  50.,  90.,   90., 180.,   1, 4588, 10800, 4800, NC_SHORT, 1},
	    {"e10g",   0.,  50., -180., -90., -84, 5443, 10800, 6000, NC_SHORT, 1},
	    {"f10g",   0.,  50.,  -90.,   0., -40, 6085, 10800, 6000, NC_SHORT, 1},
	    {"g10g",   0.,  50.,    0.,  90.,-407, 8752, 10800, 6000, NC_SHORT, 1},
	    {"h10g",   0.,  50.,   90., 180., -63, 7491, 10800, 6000, NC_SHORT, 1},
	    {"i10g", -50.,   0., -180., -90.,   1, 2732, 10800, 6000, NC_SHORT, 1},
	    {"j10g", -50.,   0.,  -90.,   0.,-127, 6798, 10800, 6000, NC_SHORT, 1},
	    {"k10g", -50.,   0.,    0.,  90.,   1, 5825, 10800, 6000, NC_SHORT, 1},
	    {"l10g", -50.,   0.,   90., 180.,   1, 5179, 10800, 6000, NC_SHORT, 1},
	    {"l10b", -50.,   0.,   90., 180., -34, 5179, 10800, 6000, NC_SHORT, 1},
	    {"m10g", -90., -50., -180., -90.,   1, 4009, 10800, 4800, NC_SHORT, 1},
	    {"n10g", -90., -50.,  -90.,   0.,   1, 4743, 10800, 4800, NC_SHORT, 1},
	    {"o10g", -90., -50.,    0.,  90.,   1, 4039, 10800, 4800, NC_SHORT, 1},
	    {"p10g", -90., -50.,   90., 180.,   1, 4363, 10800, 4800, NC_SHORT, 1},
	    {"N35E023.hgt", 35., 36.,  23., 24.,   1, 4363, 1201, 1201, NC_SHORT, 0},
	};
#endif /* HAVE_LIBZ */
	nInputTiles = 20;

	if (type == ETOPO2)
	{
		/* see what type of machine (big endian/little endian) */
		tileList[0] = tiles[1];
		nTiles = 1;
	
		/* for the time being, just get one tile */
		getGlobeData(root, tileList[0], cdfData);
	}
	else if (type == SRTM1)
	{
		/* see what type of machine (big endian/little endian) */
		tileList[0] = tiles[19];
		nTiles = 1;
	
		/* for the time being, just get one tile */
		getGlobeData(root, tileList[0], cdfData);
	}
	else 
	{
		nTiles = 0;
		findInputTiles(cdfData, nInputTiles, tiles, &nTiles, tileList);
		getGlobeData(root, tileList[0], cdfData);
	}


	return 0;
}

/* 
	based on the requested data, find which tiles need to be read
 */
static int
findInputTiles(cdfData_t * cdfData, int nInputTiles, Tile_t * tiles, int * nTiles, Tile_t * tileList)
{
	int	i;
	int	n;

	n = *nTiles;

	/* start at 2, since first two files are ETOPO2 */
	for (i=2; i<nInputTiles; i++)
	{
	    if (((cdfData->leftLon >= tiles[i].minLon && cdfData->leftLon < tiles[i].maxLon) ||
	         (cdfData->rightLon > tiles[i].minLon && cdfData->rightLon <= tiles[i].maxLon)) &&
	        ((cdfData->bottomLat >= tiles[i].minLat && cdfData->bottomLat < tiles[i].maxLat) ||
	         (cdfData->topLat > tiles[i].minLat && cdfData->topLat <= tiles[i].maxLat)))
	    {
		tileList[n++] = tiles[i];
printf("found tile min/max Lon %.3f %.3f    min/max Lat %.3f %.3f \n",
	tiles[i].minLon, tiles[i].maxLon, tiles[i].minLat, tiles[i].maxLat);
	    }
	}

	*nTiles = n;
	return 0;
}

int
showCorners(cdfData_t cdfData)
{
	short *sarr = NULL;

        if (cdfData.reqType == NC_SHORT)
        {
            sarr = (short *) cdfData.data;

	    printf("top left:  lat=%f lon=%f z=%d\n",
		cdfData.adjustedTopLat,
		cdfData.adjustedLeftLon, 
		sarr[0]);

	    printf("top right:  lat=%f lon=%f z=%d\n",
		cdfData.adjustedTopLat,
		cdfData.adjustedRightLon, 
		sarr[cdfData.nXdim-1]);

	    printf("bottom left:  lat=%f lon=%f z=%d\n",
		cdfData.adjustedBottomLat,
		cdfData.adjustedLeftLon, 
		sarr[(cdfData.nXdim * cdfData.nYdim) - cdfData.nXdim ]);

	    printf("bottom right:  lat=%f lon=%f z=%d\n",
		cdfData.adjustedBottomLat,
		cdfData.adjustedRightLon, 
		sarr[(cdfData.nXdim * cdfData.nYdim) - 1 ]);
	}

	return 0;
}


int
getGlobeData(char *root, Tile_t tile, cdfData_t *cdfData)
{
	char	path[256];
	int	data_size;
	int	pts_per_row;
	int	i, k;
	float	intervalLon, intervalLat;
	int	startLatIndex, endLatIndex;
	int	startLonIndex, endLonIndex;
	off_t bytes_in_row, bytes_to_read, bytes_to_skip;
	size_t	offset;
	off_t 	bytes_to_next_read;
	short 	*dataRead = NULL;
	int	pos;
#ifdef HAVE_LIBZ
	gzFile zfd;
#else
	int	fd;
	off_t	off;
#endif /* HAVE_LIBZ */

        /* may need to adjust the coordinates for the data set */
        adjustLon(tile.minLon, tile.maxLon, cdfData->leftLon, &cdfData->adjustedLeftLon);
        adjustLon(tile.minLon, tile.maxLon, cdfData->rightLon, &cdfData->adjustedRightLon);
        adjustLat(tile.minLat, tile.maxLat, cdfData->bottomLat, &cdfData->adjustedBottomLat);
        adjustLat(tile.minLat, tile.maxLat, cdfData->topLat, &cdfData->adjustedTopLat);

	intervalLon = (tile.maxLon - tile.minLon)/tile.nCol;
	intervalLat = (tile.maxLat - tile.minLat)/tile.nRow;

	/* find the indexes that are needed to read the data */
	startLonIndex = (int)((cdfData->adjustedLeftLon - tile.minLon)/intervalLon);
	endLonIndex = (int)((cdfData->adjustedRightLon - tile.minLon)/intervalLon);

	startLatIndex = (int)((tile.maxLat - cdfData->adjustedTopLat)/intervalLat);
	endLatIndex = (int)((tile.maxLat - cdfData->adjustedBottomLat)/intervalLat);

	printf("startLatIndex=%d endLatIndex=%d\n",
		startLatIndex, endLatIndex);
	printf("startLonIndex=%d endLonIndex=%d\n",
		startLonIndex, endLonIndex);

	/* do the lat and lon arrays */
	cdfData->nXdim = endLonIndex - startLonIndex;
	cdfData->nYdim = endLatIndex - startLatIndex;
        cdfData->Xvals = (float *) malloc(cdfData->nXdim * sizeof(float));
        cdfData->Yvals = (float *) malloc(cdfData->nYdim * sizeof(float));
	allocDataArray(cdfData);

	for (i=0; i<cdfData->nXdim; i++)
	{
		cdfData->Xvals[i] = cdfData->adjustedLeftLon + (i*intervalLon);
	}
	for (i=0; i<cdfData->nYdim; i++)
	{
		/* cdfData->Yvals[i] = cdfData->adjustedTopLat - (i*intervalLat); */
		cdfData->Yvals[i] = cdfData->adjustedBottomLat + (i*intervalLat);
	}

/*
	sprintf(path, "%s/%s", root, tile.filename);
*/
	strncpy(path, root, sizeof(path));
#ifdef HAVE_LIBZ
	zfd = gzopen(path, "rb");
#else
	fd = open(path, O_RDONLY);
#endif /* HAVE_LIBZ */
        
	if (tile.type == NC_SHORT)
	{
		data_size = sizeof(short);
	}
	else
	{
		printf("code change needed, works only if tile.type is short\n");
		return 2;
	}

	pts_per_row = endLonIndex - startLonIndex;
	bytes_in_row = tile.nCol * data_size;
	bytes_to_read = pts_per_row * data_size;
	bytes_to_skip = startLonIndex * data_size;
	bytes_to_next_read = bytes_in_row - bytes_to_read;

	offset = 0;
	dataRead = (short *) malloc(bytes_to_read);

	/* read the first record outside the loop, since the reads in the loop
	   are preceeded with a relative seek
	 */
#ifdef HAVE_LIBZ
	gzseek(zfd, (bytes_in_row*startLatIndex)+bytes_to_skip, SEEK_SET);
	if((k = gzread(zfd, (char *) dataRead, bytes_to_read)) != bytes_to_read)
#else
	if ((off = lseek(fd, (bytes_in_row*startLatIndex)+bytes_to_skip, SEEK_SET)) < 0)
	{
		printf("problem seeking %d bytes\n",
		    (int)((bytes_in_row*startLatIndex)+bytes_to_skip));
		exit (1);
	}
printf("seek %d\n", (int) ((bytes_in_row*startLatIndex)+bytes_to_skip));
	if((k = read(fd, (char *) dataRead, bytes_to_read)) != bytes_to_read)
#endif /* HAVE_LIBZ */

	{
		printf("problem reading row %d: wanted %d, read %d\n",
		    i, pts_per_row, k);
		exit (1);
	}
	/* if we want to read data into the first row of the array, 
	   pos should begin at 0, and then add pts_per_row to pos
	   after each read
	pos = 0;
	*/
	/* if we want to read data into the LAST row of the array,
	   pos should begin near the end, and then subtract pts_per_row to pos
	   after each read. This is needed for the ETOPO2 data
	 */
	pos = (cdfData->nXdim * (cdfData->nYdim - 1));
	copyData(dataRead, (float *)cdfData->data+pos, pts_per_row, tile.endian);
	pos -= pts_per_row;
	offset += bytes_to_read;

	/* this assumes that cdfData->data[] is short */
	for (i=startLatIndex+1; i<endLatIndex; i++)
	{
#ifdef HAVE_LIBZ
	    if (bytes_to_next_read > 0) gzseek(zfd, bytes_to_next_read, SEEK_CUR);
	    if((k = gzread(zfd, (char *) dataRead, bytes_to_read)) != bytes_to_read)
#else
	    if (bytes_to_next_read > 0) lseek(fd, bytes_to_next_read, SEEK_CUR);
printf("seek %d to read %d\n", (int) bytes_to_next_read, (int) bytes_to_read);
	    if((k = read(fd, (char *) dataRead, bytes_to_read)) != bytes_to_read)
#endif /* HAVE_LIBZ */
	    {
		printf("problem reading row %d: wanted %d, read %d:  %s\n",
		    i, (int) bytes_to_read, k, strerror(errno));
		exit (1);
	    }
	    copyData(dataRead, (float *)cdfData->data+pos, pts_per_row, tile.endian);
	    pos -= pts_per_row;
	    offset += bytes_to_read;

	    if (i%100 == 0) printf("i=%d read %d bytes\n", i, k);

	}
#ifdef HAVE_LIBZ
	gzclose(zfd);
#else
	close(fd);
#endif /* HAVE_LIBZ */
	free(dataRead);

	return NC_NOERR;
}

static int
copyData(short * in, float *out, int npts, int endian)
{
	int i;
            union
            {
                char    a[2];
                short   s;
            } e1, e2;


	if (endian)
	{
	    for (i=0; i<npts; i++)
	    {
	        out[i] = (float ) in[i];
	    }
	}
	else
	{
	    for (i=0; i<npts; i++)
	    {
		e1.s = in[i];
                e2.a[0] = e1.a[1];
                e2.a[1] = e1.a[0];
                out[i] = (float)e2.s;
	    }
	}
	return i;
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

#endif /* HAVE_NETCDF */
