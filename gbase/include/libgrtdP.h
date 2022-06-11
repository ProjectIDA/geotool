#ifndef _LIB_RTDP_H_
#define	_LIB_RTDP_H_

#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/param.h>

#include "libgrtd.h"
/* #include "libGenList.h" */
#include "gobject/Vector.h"
#include "gobject/DataLoop.h"
#include "cd_common.h"  /*  configParams_t; */
#include "fileAPI.h"    /* senderParams_t; */
#include "events.h"     /* senderParams_t; */

/** *
#include "frames.h"
#include "history.h"
#include "index.h"
** */

/**
 * @private
 */
typedef struct
{
        char            sta[10];
	char		chan[10];
	int		stachan_q;
	double		time;
} Active;

typedef struct
{
	bool	first_frame_read;
	char	*file;
} ReadFile;

typedef struct
{
	char	binFile[MAXPATHLEN+1];
	int	status;
	int	file_size;
	int	file_offset;
} FrameStatus;

/**
 * @private
 */
typedef struct RTData_struct
{
	int		numIdx;
	char		**directories;
	int             *portNumber;
        configParams_t  *pConfigData;
        senderParams_t  *pSenderParams; /* this is derived from pConfigData */
        uint32_t        strictMode;     /* set internally */
        time_t          *startDate;      /* set internally */
        /*
	char		*historyPath;
	historyCd2w_t	*history;
	genList		historyList;
	genList		binList;
	int		numBin;
        */
	pthread_t	thread;
	sem_t		sem;
	int		read;
	int		num_active;
	Active		*active;
	int		numDataLoops;
	DataLoop	*dataLoops;
	int		processed_frame;
	int		buffer_size;
	float		*buffer;
        /*
	int		numFiles;
	ReadFile	*f;
        */
	FrameStatus	s;
} RTDataPart;

/**
typedef struct
{
	uint32_t frameType;
	uint32_t frameLen;
	uint32_t dffLen;
	long     foff;
	long     dffOffset;
} FRAMETABLE;

FRAMETABLE *
getFrameTable (FILE                *const fp,
               FILE                *const fpClf,
               const historyCd2w_t *const history,
               int                 *const numFrames,
	       int		   read_only_one, FrameStatus *s);

uint32_t readFrame(FILE *const fp, const FRAMETABLE *const frameTable,
		historyCd2w_t *const history, char **dataFrame,
		CDdata_t *frame, const char *const fileName,
		FrameStatus *s);

**/
#ifdef HAVE_STDARG_H
void RTDSetErrorMsg(int err, const char *format, ...);
#else
void RTDSetErrorMsg();
#endif


#endif /* _LIB_RTDP_H_ */
