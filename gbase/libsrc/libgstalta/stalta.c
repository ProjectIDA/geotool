#include "config.h"
#include "libstalta.h"
#include "logErrorMsg.h"
#include <errno.h>
#include <math.h>

#define Free(a) if(a){free(a); a = NULL;}

int
allocStaltaSpace(StaLtaDef * s, int len)
{
        s->ontime = (int *) malloc(len * sizeof(int));
        s->offtime = (int *) malloc(len * sizeof(int));
        s->maxratio = (double *) malloc(len * sizeof(double));
        s->ratio = (double *) malloc(len * sizeof(double));
        s->vsta = (float *) malloc(len * sizeof(float));
        s->vlta = (float *) malloc(len * sizeof(float));

        if (!s->ontime || !s->offtime || !s->maxratio 
         || !s->ratio  || !s->vsta    || !s->vlta)
        {
            logErrorMsg(LOG_ERR, "allocStaltaSpace: malloc failed.");
            return -1;
        }

	return 0;
}

void
freeStaltaSpace(StaLtaDef * s)
{
	Free(s->ontime);
	Free(s->offtime);
	Free(s->maxratio);
	Free(s->ratio);
	Free(s->vsta);
	Free(s->vlta);

	if (s->nfilters > 0) Free(s->f);
}

/* apply the sample rate to the parameters that are a function of sample rate */
void
applyStaLtaSamprate(StaLtaDef * s, double tdel)
{
	s->sta = (int)(s->staSec / tdel);
	s->lta = (int)(s->ltaSec / tdel);
	s->buf = (int)(s->bufSec / tdel);
	if (s->buf < 0) s->buf = 0;
	s->wtrig = (int)(s->wtrigSec / tdel);
	s->trgsep = (int)(s->trgsepSec / tdel);
}



/* stalta(int sta, int lta, int *indata[],int *outdata[]) */
int 
stalta(StaLtaDef * s, int n, float *indata)
{ 
  int i;
  double tsta;
  double tlta;
  double tmaxratio;
  int tontime;
  bool trig; /* is there a trigger */
  int tnum;


  tsta=0.0;
  tlta=0.0;
  tnum=0;
  tontime = 0;

  /* if the segment is too short, then just return */
  if (n <= (s->lta + s->sta + s->buf))
  {
     return tnum;
  }

  for(i=0; i<n; i++) 
    {
      s->ratio[i]=0.0;
      s->vsta[i]=0.0;
      s->vlta[i]=0.0;
    }
  /*  for(i=0; i<nlta; i++)  tlta += fabs(indata[i])/s->lta;
      for(i=nlta+1; i<nlta+nsta; i++)  tsta += fabs(indata[i])/s->sta;
      for(i=0;i<=n; i++)  tenrgy += fabs(indata[i]);
      vlta[nlta]= (float) tlta;
      vsta[nlta]= (float) tsta;
      printf("tlta= %g tenrgy= %g data= %g\n", tlta, tenrgy/n, indata[1]);
      ratio[nlta]= tsta/tlta; 
  */
/* not sure about these two lines. aren't lta and sta given in seconds? 
but in these lines it looks like these should be in number of samples
*/

  /* fill the LTA with the first LTA number of points */
  for(i=0;i<=s->lta;i++) tlta+=fabs(indata[i])/s->lta;

  /* fill the STA with the first sta number of points */
  for(i=s->lta+s->buf;i<=(s->lta+s->sta+s->buf);i++) tsta+=fabs(indata[i])/s->sta;

  s->vlta[0]= (float) tlta;
  s->vsta[0]= (float) tsta;
  s->ratio[0]=s->vsta[0]/s->vlta[0];
  tmaxratio = 0.0;
  trig = False; 
  if(s->method == 1 )
    { 
      /*      for(i = nlta+1; i < n-nsta ; i++)*/
      for(i = (s->lta+s->buf+1); i < n - s->sta ; i++)
        {
       	  if(!trig)
       	    {
		/* won't tlta and tsta just continue growing? */
	      /*
	      tlta += (fabs(indata[i-1])-tlta)/s->lta;
	      tsta += (fabs(indata[i+s->sta])-tsta)/s->sta;
	      */
              tlta += ((fabs(indata[i-1])/s->lta) - (fabs(indata[i-s->lta-1])/s->lta));
              tsta += ((fabs(indata[i+s->sta])/s->sta) - (fabs(indata[i]/s->sta)));
#ifdef _DEBUG
printf(" indata[%d] = %.5f   - indata[%d] = %.5f\n", i-1, indata[i-1], i-s->lta, indata[i-s->lta]);
#endif /* _DEBUG */
#ifdef _DEBUG
printf(" i=%d    tlta = %.3f    tsta = %.3f ratio = %.3f\n", i, tlta, tsta, tsta/tlta);
#endif /* _DEBUG */


	      s->ratio[i] = tsta/tlta;
	      /* tmaxratio=ratio[i]; */
	      s->vlta[i]= (float) tlta;
	      s->vsta[i]= (float) tsta;
       	      if(s->ratio[i]>=s->htrig) 
		{
#ifdef _DEBUG
printf("new trigger ratio[%d] %.2f\n", i, s->ratio[i]);
#endif /* _DEBUG */
		  trig = True;
		  if(tnum > 0 && ((i- s->offtime[(tnum-1)]) <= s->trgsep))
		    {
#ifdef _DEBUG
printf("i = %d  offtime[] = %d, trgsep = %d\n", i, s->offtime[(tnum-1)], s->trgsep);
#endif /* _DEBUG */
	              if(s->ratio[i] >= tmaxratio) tmaxratio=s->ratio[i];
		      tnum=tnum-1;
		      tontime=s->ontime[tnum];
#ifdef _DEBUG
  printf("tmaxratio = %.2f\n", tmaxratio);
#endif /* _DEBUG */
/* wrong
	              if(s->maxratio[tnum]>= tmaxratio) tmaxratio=s->ratio[i];
*/
		    }
		  else
		    {
		      tontime=i;
			tmaxratio=s->ratio[i];
		    }
		}
	    }
       	  else
	    {
	      /*
	      tlta += (fabs(indata[i-1])-tlta)/s->lta;
	      tsta += (fabs(indata[i+s->sta])-tsta)/s->sta;
	       */
              tlta += (fabs(indata[i-1])/s->lta) - (fabs(indata[i-s->lta])/s->lta);
              tsta += (fabs(indata[i+s->sta])/s->sta) - (fabs(indata[i]/s->sta));
#ifdef _DEBUG
printf(" i=%d    tlta = %.3f    tsta = %.3f ratio = %.3f\n", i, tlta, tsta, tsta/tlta);
#endif /* _DEBUG */
              s->ratio[i] = tsta/tlta;
#ifdef _DEBUG
printf("i = %d tnum = %d  ratio[%d] = %.1f\n", i, tnum, i, s->ratio[i]);
  printf("tmaxratio = %.2f\n", tmaxratio);
#endif /* _DEBUG */
	      if(s->ratio[i] >= tmaxratio) tmaxratio=s->ratio[i];
	      s->vlta[i]= (float) tlta;
	      s->vsta[i]= (float) tsta;
       	      if(s->ratio[i]<=s->ltrig) 
		{
		  trig = False;
		  if((i-tontime)>=s->wtrig)
		    {
		      s->ontime[tnum]=tontime;
		      s->offtime[tnum]=i;
		      s->maxratio[tnum]=tmaxratio;
/*
		      tmaxratio = 0.0;
*/
/*
printf("i = %d maxratio[%d] = %.2f\n", i, tnum, s->maxratio[tnum]);
  printf("tmaxratio = %.2f\n", tmaxratio);
*/
		      tnum++;
		    }
		}
	    }
	}
    } 
         else
           {
	     /*             for(i = nlta+1; i < n-nsta ; i++)*/
             /* for(i = 1; i < n-s->sta ; i++) */
      	for(i = (s->lta+s->buf+1); i < n - s->sta ; i++)
       	{
       	  if(!trig)
       	    {
       	      tlta += (fabs(indata[i-1])-tlta)/s->lta;
       	      tsta += (fabs(indata[i+s->sta])-tsta)/s->sta;
       	      s->ratio[i] = tsta/tlta;
	      /* tmaxratio=ratio[i]; */
       	      s->vlta[i]= (float) tlta;
       	      s->vsta[i]= (float) tsta;
       	      if(s->ratio[i]>=s->htrig) 
		{
		  trig = True;
		  if(tnum > 0 && ((i-s->offtime[(tnum-1)]) <= s->trgsep))
		    {
	              if(s->ratio[i] >= tmaxratio) tmaxratio=s->ratio[i];
		      tnum=tnum-1;
		      tontime=s->ontime[tnum];
/* wrong
	              if(maxratio[tnum]>= tmaxratio) tmaxratio=s->ratio[i];
*/
		    }
		  else
		    {
		      tontime=i;
			tmaxratio=s->ratio[i];
		    }
		}
       	    }
       	  else
       	    {
       	      tsta += (fabs(indata[i+s->sta])-tsta)/s->sta;
       	      s->ratio[i] = tsta/tlta;
 	      if(s->ratio[i] >= tmaxratio) tmaxratio=s->ratio[i];
      	      s->vlta[i]= (float) tlta;
       	      s->vsta[i]= (float) tsta;
       	      if(s->ratio[i]<=s->ltrig) 
		{
		  trig = False;
		  if((i-tontime)>=s->wtrig)
		    {
		      s->ontime[tnum]=tontime;
		      s->offtime[tnum]=i;
		      s->maxratio[tnum]=tmaxratio;
		      tnum++;
		    }
		}
       	    }
       	}
           }
  /*testfile = fopen("run.log","w");*/
/*
  for (i=0;i<n-1;i++)  fprintf(logfile,"%g %g %g %g\n",indata[i], vsta[i],vlta[i], ratio[i]);
*/
  /*fclose(testfile);*/

  return tnum;
}

#include "libstring.h"
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/param.h>

#define file_warn(path) \
    if(errno > 0) \
    { \
        sprintf(msg, "Cannot open: %s\n%s", path, strerror(errno)); \
    } \
    else \
    { \
        sprintf(msg, "Cannot open: %s", path); \
    }

static int
ReadDetectRecipe(FILE *fp, int *nbands, StaLtaDef **sP);
static void
parseLine(char *line, int *pos, StaLtaDef *r);
static bool
findParTable(FILE *fp, const char *name);
static bool
getVariablePositions(FILE *fp, int num, const char **variables, int *pos);
static int
get_non_blank(FILE *fp, char *line, int n);
static bool
blankLine(const char *line);


int
loadStaLtaRecipe(char *station, char *recipe_dir, StaLtaParam *p, const char **err_msg)
{
        char            dir[MAXPATHLEN+1];
        char            path[MAXPATHLEN+1];
        DIR             *dirp = NULL;
        struct dirent   *dp = NULL;
	char		msg[128];
	int		len;
	FILE		*fp = NULL;

        if(recipe_dir == NULL)
        {
            logErrorMsg(LOG_ERR, "beam recipes directory = NULL.");
            return -1;
        }

	/* $GEOTOOL_HOME/tables/recipes/beam/3c/$STA.par */
        sprintf(dir, "%s/beam/3c", recipe_dir);

        if((dirp = opendir(dir)) == NULL)
        {
            if(errno > 0) {
                sprintf(msg,"Cannot open: %s\n%s", dir,strerror(errno));
                logErrorMsg(LOG_WARNING, msg);
            }
            else {
                sprintf(msg, "Cannot open: %s", dir);
                logErrorMsg(LOG_WARNING, msg);
            }
            return -1;
        }

	len = strlen(station);
	p->nbands = 0;

	/* find all of the files in this directory */
        for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
        {
	    printf("found file '%s'\n", dp->d_name);
	    if (!strncmp(dp->d_name, station, len))
	    {
		printf("found for this station '%s'\n", station);
        
		sprintf(path, "%s/%s", dir, dp->d_name);

	        if((fp = fopen(path, "r")) == NULL)
                {
                    file_warn(path);
                    *err_msg = msg;
                    if(fp != NULL) fclose(fp);
                    return -1;
                }
    
	        if (ReadDetectRecipe(fp, &p->nbands, &p->s) < 0)
	        {
		    sprintf(msg, "No recipes found in %s", path);
            	    *err_msg = msg;
	        }
	    
		fclose(fp);
	    }

        }
        closedir(dirp);

	return 1;
}

static int
ReadDetectRecipe(FILE *fp, int *nbands, StaLtaDef **sP)
{
	StaLtaDef *s = NULL;
        int pos[16];
        char line[501];
        const char *variables[] = {
            "name", "type", "group", "staSec", "ltaSec", "htrig", "ltrig", "snrThreshold", 
	    "wtrigSec", "trgsepSec", "method", 
            "flo", "fhi", "ford", "zp", "ftype"
        };

	*nbands = 0;
        if( ! findParTable(fp, "detect-recipe") ) return -1;

        if( ! getVariablePositions(fp, 16, variables, pos)
                || pos[0] == -1 || pos[13] == -1)
        {
            return -1;
        }

        s = (StaLtaDef *)malloc(sizeof(StaLtaDef));
        if (!s)
        {
            logErrorMsg(LOG_ERR, "ReadDetectRecipe: malloc failed.");
            return -1;
        }

        while(get_non_blank(fp, line, 500) != EOF &&
                strstr(line, "EndTable") == NULL)
        {
            s = (StaLtaDef *)realloc(s, (*nbands+1)*sizeof(StaLtaDef));
            if (!s)
            {
                logErrorMsg(LOG_ERR, "ReadDetectRecipe: realloc failed.");
                return -1;
            }
	    parseLine(line, pos, &s[*nbands]);
	    *nbands = *nbands + 1;
	}


	*sP = s;
	return 1;
}

static void
parseLine(char *line, int *pos, StaLtaDef *s)
{
	int j;
	char *c = NULL, *tok = NULL;

	/* hard coded for one filter right now */
	s->f = (FilterDef *) malloc(sizeof(FilterDef));

	tok = line;
	j = 0;
	while((c = strtok(tok, "| \t")) != NULL)
	{
	    tok = NULL;

	    if(j == pos[0]) {
		stringcpy(s->name, c, sizeof(s->name));
	    }
	    else if(j == pos[1]) {
		stringcpy(s->beam_type, c, sizeof(s->beam_type));
	    }
	    else if(j == pos[2]) {
		stringcpy(s->group, c, sizeof(s->group));
	    }
	    else if(j == pos[3]) {
		sscanf(c, "%lf", &s->staSec);
	    }
	    else if(j == pos[4]) {
		sscanf(c, "%lf", &s->ltaSec);
	    }
	    else if(j == pos[5]) {
		sscanf(c, "%lf", &s->htrig);
	    }
	    else if(j == pos[6]) {
		sscanf(c, "%lf", &s->ltrig);
	    }
	    else if(j == pos[7]) {
		sscanf(c, "%lf", &s->snrThreshold);
	    }
	    else if(j == pos[8]) {
		sscanf(c, "%lf", &s->wtrigSec);
	    }
	    else if(j == pos[9]) {
		sscanf(c, "%lf", &s->trgsepSec);
	    }
	    else if(j == pos[10]) {
		sscanf(c, "%d", &s->method);
	    }
	    else if(j == pos[11]) {
		sscanf(c, "%f", &s->f[0].lofreq);
	    }
	    else if(j == pos[12]) {
		sscanf(c, "%f", &s->f[0].hifreq);
	    }
	    else if(j == pos[13]) {
		sscanf(c, "%d", &s->f[0].forder);
	    }
	    else if(j == pos[14]) {
		stringcpy(s->f[0].constraint, c, sizeof(s->f[0].constraint));
	    }
	    else if(j == pos[15]) {
		stringcpy(s->f[0].ftype, c, sizeof(s->f[0].ftype));
	    }

	    j++;
	}
	s->nfilters = 1;
}

static bool
findParTable(FILE *fp, const char *name)
{
	char line[501], *c = NULL, buf[100];

	while(get_non_blank(fp, line, 500) != EOF)
	{
	    if((c = strstr(line, "BeginTable")) != NULL &&
		sscanf(c+10, "%s", buf) == 1 && !strcmp(buf, name))
	    {
		return True;
	    }
	}
	return False;
}

static bool
getVariablePositions(FILE *fp, int num, const char **variables, int *pos)
{
	char line[501], *c = NULL, *tok = NULL;
	int i, j;

	line[0] = '\0';
	if(get_non_blank(fp, line, 500) == EOF) return False;

	for(i = 0; i < num; i++)  pos[i] = -1;

	tok = line;
	j = 0;
	while((c = strtok(tok, "| \t")) != NULL)
	{
	    tok = NULL;
	    for(i = 0; i < num; i++) {
		if(!strcasecmp(variables[i], c)) {
		    pos[i] = j;
		    break;
		}
	    }
	    j++;
	}

	return True;
}

static int
get_non_blank(FILE *fp, char *line, int n)
{
	int err;

	while(!(err = stringGetLine(fp, line, n)) && blankLine(line));
	return err;
}

static bool
blankLine(const char *line)
{
	if(line[0] == '#' && line[1] != '!') return True;

	while(*line != '\0' && isspace((int)(*line))) line++;

	return (*line == '\0') ? True : False;
}

