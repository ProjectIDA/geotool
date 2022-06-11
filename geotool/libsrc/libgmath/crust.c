#include "config.h"
#include <stdlib.h>
#include <string.h>

#include "libgmath.h"
#include "libstring.h"
#include "crust_def.h"
#include "logErrorMsg.h"

static int get_line(FILE *fp, char *line, int n);

/**
 * Routines for reading and finding crust models. The models are two layers
 * over a half-space. The structure CrustModel is used to hold the layer
 * depths and velocities.
 * <pre>
 *
 * typedef struct
 * {
 *     char    name[5];
 *     char    *full_name;
 *     float   h[2];
 *     float   vp[3];
 *     float   vs[3];
 * } CrustModel;
 * </pre>
 * Crust models are stored in free-formatted ascii files. The format is
 * <pre>
 *
 *     "%s %f %f %f %f %f %f %f %f",
 *            cr.name, &cr.h[0], &cr.h[1], &cr.vp[0], &cr.vp[1], &cr.vp[2],
 *            &cr.vs[0], &cr.vs[1], &cr.vs[2]
 * </pre>
 * An example crust model file is:
 * <pre>
 *
 * iasp   20.0  15.0  5.8  6.5  8.0  3.36 3.75 4.47 iasp = iaspei91
 * jb     15.0  18.0  5.57 6.5  7.8  3.37 3.74 4.42 jb   = jeffreys and bullen
 * ccus   15.0   5.0  6.2  7.0  8.1  3.6  4.0  4.7  ccus = california coastal region
 * snus   25.0  25.0  6.2  7.0  7.9  3.6  4.0  4.5  snus = sierra nevada
 * pcus   10.0  25.0  6.2  7.0  7.7  3.6  4.0  4.4  pcus = pacific northwest coastal region
 * caus   10.0  35.0  6.2  7.0  7.9  3.6  4.0  4.5  caus = columbia plateaus
 * brus   20.0  10.0  6.2  7.0  7.9  3.6  4.0  4.5  brus = basin and range
 * cpus   25.0  15.0  6.2  7.0  7.8  3.6  4.0  4.5  cpus = colorado plateaus
 * rmus   25.0  15.0  6.2  7.0  8.0  3.6  4.0  4.6  rmus = rocky mountains
 * ipus   20.0  30.0  6.2  7.0  8.2  3.6  4.0  4.7  ipus = interior plains and highlands
 * clus   20.0  15.0  6.2  7.0  8.1  3.6  4.0  4.7  clus = coastal plain
 * asus   15.0  25.0  6.2  7.0  8.1  3.6  4.0  4.7  asus = appalachian highlands and superior upland
 * bsrp   13.0  25.0  5.9  6.8  8.1  3.4  3.9  4.7  bsrp = baltic shield
 * usrp   16.0  36.0  6.0  6.8  8.1  3.4  3.9  4.7  usrp = ukrainian shield
 * vurp   18.0  17.0  6.1  6.8  8.1  3.5  3.9  4.7  vurp = volga-ural anteclise
 * pana   37.0  28.0  5.5  6.4  8.1  3.2  3.7  4.7  pana = pamir
 * caaf   20.0  30.0  5.8  7.2  8.1  3.3  4.1  4.7  caaf = caucasus anticlinorium
 * feaf   22.0  14.0  6.2  6.7  8.1  3.6  3.9  4.7  feaf = mesozoids of the far east
 * scbs   20.0  23.0  3.6  6.5  8.0  2.1  3.7  4.6  scbs = south caspian
 * bsbs   10.0  13.0  3.3  6.6  8.0  1.9  3.8  4.6  bsbs = black sea
 * dfsc    8.0  21.0  6.1  6.6  8.1  3.5  3.8  4.7  dfsc = denmark
 * kabs   20.0  15.0  6.2  6.6  8.0  3.6  3.8  4.6  kabs = kamchatka
 * ausp   20.0  20.0  6.0  6.6  8.0  3.4  3.8  4.6  ausp = australia
 * kkal   12.0  20.0  5.5  6.5  8.1  3.2  3.7  4.7  kkal = kodial-katmai
 * inal   10.0  42.0  5.8  6.8  8.1  3.3  3.9  4.7  inal = interior alaska
 * dcaa   30.0  30.0  5.8  7.2  8.1  3.3  4.1  4.7  dcaa = deep caucasus anticlinorium
 * kule    0.1  26.0  0.1  6.5  7.9  0.1  3.7  4.5  kule = kuril islands
 * kazz   20.0  30.0  6.37  7.0  8.34  3.6  4.0  4.9kazz = kazakh
 * </pre>
 */

/**
 * Read a file of crust models. The format is "%s %f %f %f %f %f %f %f %f",
 * cr.name, &cr.h[0], &cr.h[1], &cr.vp[0], &cr.vp[1], &cr.vp[2], &cr.vs[0],
 * &cr.vs[1], &cr.vs[2] for CrustModel cr.
 * @param fp A FILE pointer to an open crust-model file.
 * @cmp Returned array of CrustModel structures.
 * @n_models Returned number of CrustModels structures read.
 * @return 0 for success, -1 for a malloc error. logErrorMsg is called if \
 *	malloc fails.
 */
int
readCrustModels(FILE *fp, CrustModel **cmp, int *n_models)
{
	char	*c, line[128];
	int	i, n, num_crusts = 0;
	CrustModel cr, *cm = NULL;

	*cmp = NULL;
	*n_models = 0;

	n = NUM_DEFAULT_CRUSTS*sizeof(CrustModel);
	if((cm = (CrustModel *)malloc(n)) == NULL)
	{
	    logErrorMsg(LOG_ERR, "readCrustModels: malloc failed.");
	    return(-1); /* malloc failed */
	}
	memcpy(cm, default_crusts, n);
	num_crusts = NUM_DEFAULT_CRUSTS;
	*cmp = cm;
	*n_models = num_crusts;

	for(n = 0; get_line(fp, line, 127) != EOF; n++);

	fseek(fp, 0, 0);
	n = 0;
	while(get_line(fp, line, 127) != EOF)
	{
	    if(sscanf(line, "%4s %f %f %f %f %f %f %f %f",
		cr.name, &cr.h[0], &cr.h[1], &cr.vp[0], &cr.vp[1], &cr.vp[2],
		&cr.vs[0], &cr.vs[1], &cr.vs[2]) != 9 ||
		(c = (char *)strchr(line, '=')) == NULL)
	    {
			continue;
	    }
	    c++;
	    while(*c != '\0' && (*c == ' ' || *c == '\t')) c++;

	    if(*c != '\0')
	    {
		snprintf(cr.full_name, sizeof(cr.full_name), "%s", c);

		for(i = 0; i < num_crusts; i++)
		{
		    if(!strcasecmp(cm[i].name, cr.name)) break;
		}
		if(i < num_crusts)
		{
		    cm[i] = cr;
		}
		else
		{
		    if((cm = (CrustModel *)realloc(cm,(num_crusts+1)
				*sizeof(CrustModel))) == NULL)
		    {
			logErrorMsg(LOG_ERR, "readCrustModels: malloc failed.");
			return(-1); /* malloc failed */
		    }
		    cm[num_crusts] = cr;
		    num_crusts++;
		}
	    }
	}

	*cmp = cm;
	*n_models = num_crusts;

	return(0);
}

/**
 * Find a CrustModel. Search the input array of CrustModel structures for
 * the specified crust_name. Both the short name and the full_name are tested.
 * @param num_crusts The input number of models.
 * @param cm A array of num_crust CrustModels strutures.
 * @param crust_name The input crust_name.
 * @param crust The returned CrustModel corresponding to crust_name.
 * @return 0 if the CrustModel is found, -1 if it is not found.
 */
int
crust_model(int num_crusts, CrustModel *cm, const char *crust_name,
		CrustModel *crust)
{
	int i;

	for(i = 0; i < num_crusts; i++)
	{
	    if(!strcasecmp(cm[i].name, crust_name))
	    {
		memcpy(crust, &cm[i], sizeof(CrustModel));
		break;
	    }
	}
	if(i == num_crusts)
	{
	    /* check full_name
	     */
	    for(i = 0; i < num_crusts; i++)
	    {
		if(!strcasecmp(cm[i].full_name, crust_name))
		{
		    memcpy(crust, &cm[i], sizeof(CrustModel));
		    break;
		}
	    }
	}
	if(i == num_crusts)
	{
	    return(-1);
	}
	return(0);
}

static int
get_line(FILE *fp, char *line, int n)
{
	int c = '\0';

	while(n > 0 && (c = getc(fp)) != EOF && c != '\n')
	{
	    n--;
	    *line++ = c;
	}
	*line = '\0';

	if(c == '\n') {
	    return(0);
	}
	else if(c == EOF) {
	    return(EOF);
	}

	while((c=getc(fp)) != '\n' && c != EOF);
	return(0);
}
