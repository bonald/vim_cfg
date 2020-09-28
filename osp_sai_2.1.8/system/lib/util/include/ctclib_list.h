#ifndef __CTCLIB_LIST_H__
#define __CTCLIB_LIST_H__
#include "sal.h"

#ifdef CTCLIB_LIST_MACRO_DEBUG
#define _CTCLIB_L_INVALIDATE(a) (a) = ((void *)-1)
#else
#define _CTCLIB_L_INVALIDATE(a)
#endif

/*
 * Doubly-linked List
 */
struct ctclib_list_node_s
{
    struct ctclib_list_node_s* p_next;
    struct ctclib_list_node_s* p_prev;
};
typedef struct ctclib_list_node_s ctclib_list_node_t;

struct ctclib_list_s
{
    ctclib_list_node_t head;
};
typedef struct ctclib_list_s ctclib_list_t;

/* Define a list */
#define CTCLIB_LIST_DEF(list) ctclib_list_t list = {{NULL, &list.head}}

#define CTCLIB_LHEAD(p_list) (p_list->head.p_next)
#define CTCLIB_LTAIL(p_list) (p_list->head.p_prev)

static inline void
ctclib_list_init(ctclib_list_t* p_list)
{
    CTCLIB_LHEAD(p_list) = NULL;
    CTCLIB_LTAIL(p_list) = &p_list->head;
}

static inline void
ctclib_list_insert_head(ctclib_list_t* p_list, ctclib_list_node_t* p_node)
{
    if ((p_node->p_next = CTCLIB_LHEAD(p_list)) != NULL)
        p_node->p_next->p_prev = p_node;
    else
        CTCLIB_LTAIL(p_list) = p_node;
    CTCLIB_LHEAD(p_list) = p_node;
    p_node->p_prev = &p_list->head;
}

static inline void
ctclib_list_insert_tail(ctclib_list_t* p_list, ctclib_list_node_t* p_node)
{
    p_node->p_next = NULL;
    p_node->p_prev = CTCLIB_LTAIL(p_list);
    CTCLIB_LTAIL(p_list)->p_next = p_node;
    CTCLIB_LTAIL(p_list) = p_node;
}

static inline void
ctclib_list_insert_after(ctclib_list_t* p_list, ctclib_list_node_t* p_node1, ctclib_list_node_t* p_node2)
{
    if ((p_node2->p_next = p_node1->p_next) != NULL)
        p_node2->p_next->p_prev = p_node2;
    else
        CTCLIB_LTAIL(p_list) = p_node2;
    p_node1->p_next = p_node2;
    p_node2->p_prev = p_node1;
}

static inline void
ctclib_list_insert_before(ctclib_list_t* p_list, ctclib_list_node_t* p_node1, ctclib_list_node_t* p_node2)
{
    p_node2->p_next = p_node1;
    p_node2->p_prev = p_node1->p_prev;
    p_node1->p_prev->p_next = p_node2;
    p_node1->p_prev = p_node2;
}

static inline ctclib_list_node_t*
ctclib_list_delete_head(ctclib_list_t* p_list)
{
    ctclib_list_node_t* p_node = CTCLIB_LHEAD(p_list);

    if ((CTCLIB_LHEAD(p_list) = p_node->p_next) != NULL)
        CTCLIB_LHEAD(p_list)->p_prev = &p_list->head;
    else
        CTCLIB_LTAIL(p_list) = &p_list->head;

    return p_node;
}

static inline ctclib_list_node_t*
ctclib_list_delete_tail(ctclib_list_t* p_list)
{
    ctclib_list_node_t* p_node = CTCLIB_LTAIL(p_list);

    CTCLIB_LTAIL(p_list) = p_node->p_prev;
    CTCLIB_LTAIL(p_list)->p_next = NULL;

    return p_node;
}

static inline void
ctclib_list_delete(ctclib_list_t* p_list, ctclib_list_node_t* p_node)
{
    if (p_node->p_next != NULL)
        p_node->p_next->p_prev = p_node->p_prev;
    else
        CTCLIB_LTAIL(p_list) = p_node->p_prev;
    p_node->p_prev->p_next = p_node->p_next;
}

static inline int
ctclib_list_empty(ctclib_list_t* p_list)
{
    return CTCLIB_LHEAD(p_list) == NULL;
}

static inline ctclib_list_node_t*
ctclib_list_head(ctclib_list_t* p_list)
{
    return CTCLIB_LHEAD(p_list);
}

static inline ctclib_list_node_t*
ctclib_list_tail(ctclib_list_t* p_list)
{
    return CTCLIB_LTAIL(p_list)->p_prev->p_next;
}

static inline ctclib_list_node_t*
ctclib_list_next(ctclib_list_node_t* p_node)
{
    return p_node->p_next;
};

static inline ctclib_list_node_t*
ctclib_list_prev(ctclib_list_node_t* p_node)
{
    return p_node->p_prev->p_prev->p_next;
}

#define ctclib_list_for_each(p_node, p_list) \
for (p_node = ctclib_list_head(p_list); p_node; p_node = ctclib_list_next(p_node))

#define ctclib_list_for_each_safe(p_node, p_next, p_list) \
	for (p_node = ctclib_list_head(p_list), p_next = (p_node ? ctclib_list_next(p_node) : NULL); \
         p_node != NULL; \
		 p_node = p_next, p_next = (p_node ? ctclib_list_next(p_node) : NULL))
		
#define ctclib_list_for_each_r(p_node, p_list) \
for (p_node = ctclib_list_tail(p_list); p_node; p_node = ctclib_list_prev(p_node))

#define ctclib_container_of(p, type, member) \
(type *)((char *)p - (long)&((type *)0)->member)

static inline size_t
ctclib_list_size(ctclib_list_t* p_list)
{
    ctclib_list_node_t* p_node;
    size_t cnt = 0;
    
    ctclib_list_for_each(p_node, p_list)
        cnt ++;
    return cnt;
}

#endif /* !__CTCLIB_LIST_H__ */
