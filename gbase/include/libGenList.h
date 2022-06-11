/******************************************************************
 *                                                                *
 *  libGenList - provides a generic linked list                   *
 *             - provides a generic queue                         *
 *                                                                *
 *         International Data Centre, CTBTO, Vienna Austria       *
 *         http://www.ctbto.org                                   *
 *                                                                *
 *  Designed and Implemented by Dr.techn. Dipl.-Ing. Idriz Smaili *
 *                                                                *
 *       Address: Thomas-Morusgasse 10/4                          *
 *                1130 Vienna, Austria                            *
 *         Phone: ++ 43 1 92 45 170                               *
 *    Mob. Phone: ++ 43 650 92 45 170                             *
 *         Email: smaili@swengLab.com                             *
 *           Web: http://www.swengLab.com                         *
 *                                                                *
 ******************************************************************/
/**
 * @file  libGenList.h
 *
 * @brief  Provides a generic linked list.
 *
 * Provides a generic linked list, i.e., it offers the same interface
 * for both single and double linked lists. The only difference of the
 * interface are the 'constructors' (initSList & initDList). After one
 * has initialized the list (i.e., single or double) then the rest of
 * the interfaces are the same.
 *
 * The usage of the linked lists is explained by means of an example:
 * genList listObj;
 * int     elem = 3;
 *
 * initSList (&listObj, terminateHandler);
 *
 * The 'terminateHandler' is a callback function called in case the fatal
 * error is detected.
 *
 * Adding an element to the list is done as follows:
 * appendList (&listObj, &elem, sizeof (elem));
 *
 * If you want to add an element at a particular location in the list then
 * you have to do as follows:
 * addAtList (&listObj, position, &elem, sizeof (elem));
 *
 * To remove an element from the list you can use:
 * removeList (&listObj, position);
 *
 * To check if there exists an element in the list you have to use:
 * existsList (&listObj, &value, cmpFunc);
 * 'value' is the element you are looking for, while 'cmpFunc' is the compare
 * function used to compare the provided value and elements of the list.
 *
 * For iteration over the list elements you can use the list iterators:
 * lIterator  iter;
 * int       *elem;
 *
 * for (initLi(&iter, &listObj); isValidLi (&iter); getNextLi (&iter))
 *   {
 *      elem = GETVALUE (&iter, int);
 *      do something with the elem
 *   }
 *
 * To remove elements within the iteration loop one has to call removeListIt
 * instead of removeList. This function provides also the needed changes to
 * the iterator object, which guarantees the correct behavior of the
 * iterator. In the following an example is provided how to remove elements
 * from the list within the iterator loop:
 * lIterator  iter;
 * fooBar_t  *elem;
 *
 * for (initLi(&iter, &listObj); isValidLi (&iter); getNextLi (&iter))
 *   {
 *      elem = GETVALUE (&iter, fooBar_t);
 *      if (check(elem))
 *        {
 *          the element must be removed
 *          removeListIt (&iter);
 *        }
 *   }
 *
 * The elements of the root list could be also lists. In that case you
 * have to append these lists to the root list by using of listAppendList
 * member (please take a look on .../src/genList/testGenList.c)
 *
 * @author (IS) Dr.techn. Dipl.-Ing. Idriz Smaili (smaili@swengLab.com)
 * @date   $Date: Mon Mar  7 09:13:59 WEST 2005 $
 *
 * 25-Nov-2005 (IS) replace 'list' with 'listObj' in all function
 *                  declarations and data structures.
 * 25-Nov-2005 (IS) NextMb is redefined - it takes only the iterator as
 *                  parameter.
 * 25-Nov-2005 (IS) The multitasking synchronization added.
 * 28-Nov-2005 (IS) All interface functions return status.
 * 16-Mar-2006 (IS) Queue object added.
 * 25-Oct-2006 (IS) Iterator functionality added to Queue.
 *
 */

#ifndef _libGenList_h
#define _libGenList_h

#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>

#ifdef NEED_SYNCHRONIZATION
#include <semaphore.h>
#endif /* NEED_SYNCHRONIZATION */

#define E_MEMORY  1
#define E_PRECOND 2
#define E_EMPTY   19

/* synchronization object - not supported yet => void */
#ifdef NEED_SYNCHRONIZATION
typedef sem_t SyncObj;
#else
typedef void * SyncObj;
#endif /* NEED_SYNCHRONIZATION */

/* predeclaration of the list object */
struct _genList;

/* pointer to compare function */
typedef
int (* cmpFuncGS) (const void *l, const void *r);

/* pointer to member functions */
typedef
void (* FreeMb)    (      struct _genList *const listObj);

typedef
int  (* AddAtMb)   (      struct _genList *const listObj,
			         int             index,
		    const        void     *const elm,
			         size_t          size);
typedef
int  (* AppendMb)  (      struct _genList *const listObj,
		    const        void     *const elm,
			         size_t          size,
                                 FreeMb          free);
typedef
void (* RemoveMb)  (      struct _genList *const listObj,
			         size_t          index);
typedef
void * (* NextMb)  (const        void     *const node);

typedef
void * (* ValueMb) (const        void     *const node);

typedef
int    (* ValLenMb)(const        void     *const node);

typedef
void (* TermHndl)  (const        uint16_t        event,
 	            const        char     *const functionName,
	            const        char     *const extraMsg);

/*
 * 'sNode' describes a node of the single linked list
 */
typedef struct _sNode
{
         void   *value;
         size_t  size;
         FreeMb  free;
  struct _sNode *next;
}
sNode_t;

/*
 * 'dNode' describes a node of the double linked list
 */
typedef struct _dNode
{
         sNode_t  *slNode;
  struct _dNode   *prev;
}
dNode_t;

/*
 * 'genList' describes a generic linked list that could be single or double
 * linked
 */
typedef struct _genList
{
  /* attributes */
  size_t     nElm;
  void      *root;
  void      *last;

  /* objet identification attribute */
  uint32_t   oida;

  /* synchronization attributes */
  SyncObj    sync;

  /* members */
  FreeMb     free;
  AppendMb   append;
  AddAtMb    addAt;
  RemoveMb   remove;
  NextMb     getNext;
  ValueMb    getValue;
  ValLenMb   getValLen;
  TermHndl   termHndl;
}
genList_t;

/*
 * 'lIterator' models an iterator for the generic linked list
 */
typedef struct _lIterator
{
        size_t   index;   /* current index      */
        void    *cNode;   /* current node       */
  const genList_t *listObj; /* list               */
}
lIterator_t;

void *
getNodeLi (lIterator_t *const iterator);

/* get value */
#define GETVALUE(iterator, type)\
 (type *) (* ((lIterator_t *)iterator)->listObj->getValue) (\
              ((lIterator_t *)iterator)->cNode);

/* get value */
#define GETVALLEN(iterator)\
  (* ((lIterator_t *)iterator)->listObj->getValLen) (\
              ((lIterator_t *)iterator)->cNode);

/* get node */
#define GETNODE(iterator, nodeType)\
 (nodeType *) ((lIterator_t *)iterator)->cNode;

/*****************************************************************
 * general linked list interfaces
 ****************************************************************/
extern int
initSList (genList_t *const listObj, TermHndl termHndl);

extern int
initDList (genList_t *const listObj, TermHndl termHndl);

extern int
isvalidList (const genList_t *const listObj);

extern void
freeList (genList_t *const listObj);

extern int
appendList (genList_t *const listObj, const void *const elm, size_t size);

extern int
appendListElm (genList_t *const listObj, const genList_t *const elm);

extern int
addAtList  (      struct _genList *const listObj,
	       	         size_t          index,
            const        void     *const elm,
       		         size_t          size);
extern int
removeList (      struct _genList *const listObj,
		         size_t          index);
extern int
removeListIt(     struct _lIterator *const iter);

extern void *
existsList (const genList_t   *const listObj,
	    const void      *const value,
	          cmpFuncGS        cmpFunc);
extern int
cloneList (genList_t *const dest, const genList_t *const src);

extern int
listAppendList (genList_t *const dest, const genList_t *const src);

extern int
truncateList (genList_t *const dest);

int
sortList (genList_t *const listObj,cmpFuncGS cmpFunc);

int
listCmpList (const genList_t *const l, const genList_t *const r, cmpFuncGS cmpFunc);

/*****************************************************************
* Iterator interface functions
*****************************************************************/
extern void
initLi (/*@out@*/lIterator_t *const iterator, const genList_t *const listObj);

extern int
isValidLi (const lIterator_t *const iterator);

extern void
getNextLi (lIterator_t *const iterator);

extern int
isLastLi  (const lIterator_t *const iterator);

/*****************************************************************
* compare function for different data types
*****************************************************************/
extern int
cmpString (const void *ls, const void *rs);

extern int
cmpInt (const void *ls, const void *rs);

extern int
cmpFloat (const void *ls, const void *rs);

extern int
cmpDouble (const void *ls, const void *rs);

/*****************************************************************
 * last error interface
 ****************************************************************/
extern char *
listErrorMsg (void);

/*****************************************************************
 * Queue (FIFO order) data structure
 ****************************************************************/
/*
 * 'Queue' models an iterator for the generic linked list
 */
typedef struct _genQueue
{
  /* synchronization attributes */
  SyncObj sync;

  genList_t listObj; /* list                                */
}
genQueue_t;

/*
 * 'qIterator' models an iterator for the generic queue
 */
typedef struct _qIterator
{
  const    size_t *index;     /* current index      */
        lIterator_t  lIterator; /* lIterator */
}
qIterator_t;

/* get value */
#define QUEUE_GETVALUE(queueIt, type)\
 	GETVALUE(&((qIterator *)queueIt)->lIterator, type);

extern int
initQueue (genQueue_t *const queueObj, TermHndl termHndl);

extern int
isvalidQueue (const genQueue_t *const queueObj);

extern void
freeQueue (genQueue_t *const queueObj);

extern int
nElmQueue (const genQueue_t *const queueObj);

extern int
pushQueue (     genQueue_t *const queueObj,
          const void       *const elm,
          const size_t            size);

extern int
peekQueue (genQueue_t *const queueObj, void **elm);

extern int
popQueue (genQueue_t *const queueObj, void **elm);

/*****************************************************************
 * last error interface
 ****************************************************************/
extern char *
queueErrorMsg (void);

/*****************************************************************
* Queue Iterator interface functions
*****************************************************************/
extern void
initQu (/*@out@*/qIterator_t *const iterator, const genQueue_t *const queueObj);

extern int
isValidQu (const qIterator_t *const iterator);

extern void
getNextQu (qIterator_t *const iterator);

extern int
isLastQu  (const qIterator_t *const iterator);

#endif /* _libGenList_h */

/* EOF */

