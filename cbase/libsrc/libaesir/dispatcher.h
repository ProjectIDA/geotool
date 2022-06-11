/*
 * dispatcher.h:	Define structures used by the executive.
 *
 *	@(#)dispatcher.h	105.1	19 May 1995
 */

#ifndef _DISPATCHER_
#define _DISPATCHER_ 1


#include <inttypes.h>
#include <sys/types.h>
#include "aesir.h"
#include "mask.h"

#define	MAX_ERRORS	10	/* how many times to retry on error */
#define TIME_ERROR	12	/* how many seconds to wait before
				 retrying to establish the connection */
#define MAX_CONN	5	/* number of connection to queue up */
#define MAX_SOCKETWRITE	1000	/* THIS SHOULD BE VERIFIED.  It is the
				 maximum amount a socket can handle at
				 a time. */
#define HOST_LENGTH(len)	ntohs((u_short) (len))
#define NET_LENGTH(len)		htons((u_short) (len))
#define W_LENGTH(len)		((int) NET_LENGTH(len))
#define D_LSIZE			(sizeof (Mess_len))

#define SYNC1		('s'+128)
#define SYNCR		('r'+128)
#define SYNCW		('w'+128)

typedef int16_t         Mess_len;	/* number of bytes in the message */

typedef struct 
{
	Mess_len	m_len; /* length of message */
	Mess_len	m_done; /* amount of message sent/recieved */
	Bool		m_needlen; /* need to send/get the message length */
	char		m_buf[MAX_MESSSIZE]; /* the sending/recv buffer */
} Mbuf;

typedef struct			/* Information needed for connection */
{
	char	*d_name;	/* this processes class */
	char	*d_host;	/* the host machine */
	int	d_pid;		/* the process id */
	int	d_subpid;	/* used for more than one connection */
	char	*d_ppoid;	/* the process-object id */
	char	d_lpoid[LPOID_SIZE]; /* the logical poid (may not be valid) */
	char	d_recipient[LPOID_SIZE]; /* who received the last message */
	char	d_sender[LPOID_SIZE]; /* who sent the last message */
	int	d_socket;	/* the socket */
	int	d_start;	/* the time the connection was established */
	char	*d_version;	/* the dispatcher version  */
} Dispatch;

typedef struct			/* a message */
{
	unsigned int	md_tag;	/* unique identifier (monotonic) */
	unsigned int	md_time; /* when the message was first started */
	char	md_src[LPOID_SIZE]; /* the source logical poid */
	char	md_dst[LPOID_SIZE]; /* the destination logical poid */
	char	md_id[ID_SIZE];	/* the message id */
	Mess_len	md_len;	/* bytes in the message */
	char 	*md_mess;	/* dynanically allocated message data */
} MessData;

typedef struct mq		/* a list of messages */
{
	MessData	mq_mess; /* the message */
	int	mq_client;	/* the client name (valid if sending) */
	int	mq_src_client;	/* the source client */
	Bool	mq_deleted;	/* TRUE if it is deleted */
	Bool	mq_cancel;	/* TRUE if it is cancelled */
	Bool	mq_make;	/* TRUE if being composed */
	Bool	mq_new;		/* TRUE if this is a new message */
	Bool	mq_send;	/* TRUE if the message is being sent */
	Bool	mq_resolve;	/* TRUE if message dst is being resolved */
	Bool	mq_carbon;	/* TRUE if it has been carbon copied */
} MessQ;

/*
 * Main states that each client is in
 */

typedef enum
{
	FATAL_STATE,		/* client had fatal error, shut him down */
	NEW_STATE,		/* client is establishing a connection */
	READY_STATE,		/* client is idle (as far as dispatcher goes */
	WRITING_STATE,		/* sending message to the client */
	READING_STATE		/* reading a message from the client */
} State;

/*
 * Substates that a client that is in NEW_STATE can be in.
 */

typedef enum
{
	NEWFATAL_STATE,		/* connection failed, shut client down */
	NEWSTART_STATE,		/* detected the client wants a connection */
	NEWDONE_STATE,		/* connections is established */
	NEWGETVERSION_STATE,	/* verision of library client linked with */
	NEWSENDVERSION_STATE,	/* the version the dispatcher linked with */
	NEWGETNAME_STATE,	/* get the class of this client */
	NEWGETPOID_STATE,	/* find out the lpoid of this client */
	NEWSENDPOID_STATE	/* tell the client its lpoid */
} NewState;

/*
 * Substates that a client that is in READING_STATE
 * can be in (client sending a message means we are reading from the
 * client) 
 */

typedef enum
{
	READFATAL_STATE,	/* read failed, shut client down */
	READSTART_STATE,	/* detected the client is sending a message */
	READDONE_STATE,		/* finished reading message */
	READGETSYNC1_STATE,	/* waiting for client to send sync */
	READGETSYNC2_STATE,	/* waiting for client to send sync */
	READGOTSYNCW_STATE,	/* client is ready to start writing */
	READNOREAD_STATE,	/* go back to ready state (read some garbage) */
	READSENDSYNC_STATE,	/* tell client dispatcher is listening */
	READGETRECPOID_STATE,	/* reading the recipient lpoid */
	READGETID_STATE,	/* reading the message id */
	READGETMESS_STATE,	/* reading the message data */
	READNEEDPOID_STATE,	/* need to get back the poid from recipient */
	READGOTRECPOID_STATE,	/* message delivered, ready to tell client */
	READFAILED_STATE,	/* client timed out sending the message */
	READSENDRECPOID_STATE,	/* send the recpient poid to the client */
	READACK_STATE		/* acknowledge the end of the transmission */
} ReadState;

/*
 * Substates that a client that is reading a message (WRITING_STATE) can be in
 * (client reading a message means we are writing a message to the client
 */

typedef enum
{
	WRITEFATAL_STATE,		/* read failed, shut client down */
	WRITESTART_STATE,		/* start sending message to the client*/
	WRITEDONE_STATE,		/* finished reading message */
	WRITENOWRITE_STATE,		/* client started writing */
	WRITESENDSYNC_STATE,		/* send the sync to the client */
	WRITEGETSYNC1_STATE,		/* read the sync */
	WRITEGETSYNC2_STATE,		/* read the sync */
	WRITEGOTSYNCW_STATE,		/* client wants to write, not read */
	WRITESENDPOID_STATE,		/* write the sender poid */
	WRITESENDID_STATE,		/* send the message id to the client */
	WRITESENDMESS_STATE		/* send the message data to the client*/
} WriteState;
	
typedef union {
	NewState	st_new;
	ReadState	st_read;
	WriteState	st_write;
} SubState;

extern Dispatch	*dispatcher;	/* connection to the dispatcher */

/*
 * Debug flags
 */

#define PRINT_STATE	1		/* print state changes */

/*
 * From read.c
 */
extern int do_read (int socket, Mbuf *getbuf);
extern int do_var_read (int socket, Mbuf *getbuf);

/*
 * From write.c
 */
extern int do_var_write (int socket, Mbuf *sendbuf);
extern int do_write (int socket, Mbuf *sendbuf);

/*
 *  From select.c
 */
extern int ok_except (int fd);
extern int ok_read (int fd);
extern int ok_read_mask (Mask mask);
extern int ok_write (int fd, int size);


#endif /* _DISPATCHER_ */
