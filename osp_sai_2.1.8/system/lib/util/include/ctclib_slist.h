#ifndef __CTCLIB_SLIST_H__
#define __CTCLIB_SLIST_H__
#include "sal.h"

typedef int32 (*CTCLIB_SLIST_CMP_FN) (void *data1, void *data2);
typedef void  (*CTCLIB_SLIST_DEL_FN) (void *data);

typedef struct slistnode
{
    struct slistnode    *next;
    struct slistnode    *prev;
    void                *data;
} ctclib_slistnode_t;

typedef struct
{
    ctclib_slistnode_t  *head;
    ctclib_slistnode_t  *tail;
    uint32              count;
    CTCLIB_SLIST_CMP_FN cmp;
    CTCLIB_SLIST_DEL_FN del;
} ctclib_slist_t;

#define CTCLIB_SLISTHEAD(X)     ((X)->head)
#define CTCLIB_SLISTTAIL(X)     ((X)->tail)
#define CTCLIB_SLISTCOUNT(X)    (((X) != NULL) ? ((X)->count) : 0)
#define CTCLIB_SLIST_ISEMPTY(X) (((X) == NULL) || ((X)->head == NULL && (X)->tail == NULL))
#define GETDATA(X)              (((X) != NULL) ? (X)->data : NULL)

/* List iteration macro. */
#define CTCLIB_SLIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->head; (N); (N) = (N)->next) \
      if (((V) = (N)->data) != NULL)

/* List iteration macro. 
 * It allows to delete N and V in the middle of the loop 
 */
#define CTCLIB_SLIST_LOOP_DEL(L,V,N,NN) \
  if (L) \
    for ((N) = (L)->head, NN = ((N)!=NULL) ? (N)->next : NULL;  \
         (N);                                                   \
         (N) = (NN), NN = ((N)!=NULL) ? (N)->next : NULL)       \
      if (((V) = (N)->data) != NULL)

/* List iteration macro. */
#define CTCLIB_SLIST_LOOP_REV(L,V,N) \
  if (L) \
    for ((N) = (L)->tail; (N); (N) = (N)->prev) \
      if (((V) = (N)->data) != NULL)

/* add and delete */
ctclib_slist_t*
ctclib_slist_create(CTCLIB_SLIST_CMP_FN cmp, CTCLIB_SLIST_DEL_FN del);

int32
ctclib_slist_delete(ctclib_slist_t *p_list);

/* add and delete node */
ctclib_slistnode_t*
ctclib_slistnode_insert(ctclib_slist_t *p_list, void *data);

ctclib_slistnode_t*
ctclib_slistnode_insert_sort(ctclib_slist_t *p_list, void *data);

int32
ctclib_slistnode_delete(ctclib_slist_t *p_list, void *data);

/* lookup */
ctclib_slistnode_t*
ctclib_slistnode_lookup(ctclib_slist_t *p_list, void *data);

void*
ctclib_slistdata_lookup(ctclib_slist_t *p_list, void *data);
int32
ctclib_slist_count(ctclib_slist_t *p_list);

#endif /* !__CTCLIB_SLIST_H__ */
