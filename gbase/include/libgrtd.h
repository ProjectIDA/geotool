#ifndef _LIB_RTD_H_
#define	_LIB_RTD_H_

/* #include "libGenList.h" */
#include "gobject/Segment.h"
#include "gobject/Vector.h"

#define RTD_ERR_UNKNOWN		0
#define RTD_BAD_PATH		1
#define RTD_NULL_PATH		2
#define RTD_BAD_NETWORK		3
#define RTD_BAD_STATION		4
#define RTD_BAD_CHANNEL		5
#define RTD_OPEN_DIR_ERR	6
#define RTD_STAT_FILE_ERR	7
#define RTD_OPENR_FILE_ERR	8
#define RTD_OPENW_FILE_ERR	9
#define RTD_RTDreadCD_ERR	10
#define RTD_RTDseek_ERR		11
#define RTD_REACQUIRE_ERR	12
#define RTD_PTHREAD_ERR		13
#define RTD_SEM_INIT_ERR	14
#define RTD_SEM_POST_ERR	15
#define RTD_JOIN_ERR		16
#define RTD_MALLOC_ERR		17

#define RTD_ERROR		20
#define RTD_READING_FRAME_TABLE	21
#define RTD_READING_FRAME	22
#define RTD_DONE		23


typedef struct RTData_struct *RTData;

typedef struct
{
	char	sta[10];
	char	chan[10];
} StaChan;


/* ****** rtdata.c ********/
RTData RTDataOpen(int numPath, const char **paths, const char *historyPath);

RTData RTDataInitPaths(const char *configFile, const char *appName, int numPath, const char **paths);
/*
int RTDataAddPaths(RTData rtd, int numPath, const char **paths, int *portNumber);
*/
void RTDataClearPaths(RTData rtd);
void RTDataClose(RTData rtd);
int RTDataSetPaths(RTData rtd, int numPath, const char **paths);
int RTDataAddChannel(RTData rtd, char *net, char *sta, char *chan,
			double minDuration);
int RTDataStart(RTData rtd);
int RTDataRemoveChannel(RTData rtd, char *sta, char *chan);
int RTDataGetData(RTData rtd, char *sta, char *chan, Segment lastseg,
			Vector segments);
int RTDataGetChannels(RTData rtd, StaChan **stachan);
int RTDataGetActiveChannelsOld(int numDir, char **dirs, StaChan **stachan);
int RTDataGetAllActive(RTData rtd, StaChan **stachan);
int RTDataGetStatus(RTData rtd, char *binFile, int *status, int *file_size,
			int *file_offset);


/* ******* RTDError.c ********/
char *RTDErrMsg(void);
int RTDErrno(void);

/* ******* history.c, index.c  ********/
/* **
int  initHistoryList   (RTData rtd);
void updateHistoryList (RTData rtd);
int  getBinFiles       (RTData rtd, genList *binList, struct tm *utcTime);
int  getBinList        (RTData rtd);
int  getLastBinFiles   (RTData rtd, genList *idxList);
** */
#endif /* _LIB_RTD_H_ */
