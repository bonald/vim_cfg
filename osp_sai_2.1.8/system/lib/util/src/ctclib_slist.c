
#include "sal.h"
#include "ctclib_memory.h"
#include "ctclib_slist.h"

static void
_ctclib_list_delete_all_node(ctclib_slist_t *p_list)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *next = NULL;

    for (node = p_list->head; node; node = next)
    {
        next = node->next;
        if (p_list->del)
        {
            p_list->del(node->data);
        }
        XFREE(MEM_LIB_SLISTNODE, node);
    }
    
    p_list->head = NULL;
    p_list->tail = NULL;
    p_list->count = 0;
}

ctclib_slist_t*
ctclib_slist_create(CTCLIB_SLIST_CMP_FN cmp, CTCLIB_SLIST_DEL_FN del)
{
    ctclib_slist_t *p_list = NULL;

    p_list = XCALLOC(MEM_LIB_SLIST, sizeof(ctclib_slist_t));
    if (NULL == p_list)
    {
        return NULL;
    }
    p_list->cmp = cmp;
    p_list->del = del;
    
    return p_list;
}

int32
ctclib_slist_delete(ctclib_slist_t *p_list)
{
    _ctclib_list_delete_all_node(p_list);
    XFREE(MEM_LIB_SLIST, p_list);

    return 0;
}

ctclib_slistnode_t*
ctclib_slistnode_insert(ctclib_slist_t *p_list, void *data)
{
    ctclib_slistnode_t *new_node = NULL;

    new_node = XCALLOC(MEM_LIB_SLISTNODE, sizeof(ctclib_slistnode_t));
    if (NULL == new_node)
    {
        return NULL;
    }
    
    new_node->data = data;
    new_node->prev = p_list->tail;
    if (NULL == p_list->head)
    {
        p_list->head = new_node;
    }
    else
    {
        p_list->tail->next = new_node;
    }
    p_list->tail = new_node;
    p_list->count++;
    
    return new_node;
}

ctclib_slistnode_t*
ctclib_slistnode_insert_sort(ctclib_slist_t *p_list, void *data)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *new_node = NULL;

    new_node = XCALLOC(MEM_LIB_SLISTNODE, sizeof(ctclib_slistnode_t));
    if (NULL == new_node)
    {
        return NULL;
    }

    new_node->data = data;

    if (p_list->cmp)
    {
        for (node = p_list->head; node; node = node->next)
        {
            if ((p_list->cmp(data, node->data)) < 0)
            {       
                new_node->next = node;
                new_node->prev = node->prev;

                if (node->prev)
                {
                    node->prev->next = new_node;
                }
                else
                {
                    p_list->head = new_node;
                }
                
                node->prev = new_node;
                p_list->count++;
                return new_node;
            }
        }
    }

    new_node->prev = p_list->tail;
    if (p_list->tail)
    {
        p_list->tail->next = new_node;
    }
    else
    {
        p_list->head = new_node;
    }
    p_list->tail = new_node;
    p_list->count++;
  
    return new_node;
}

int32
ctclib_slistnode_delete(ctclib_slist_t *p_list, void *data)
{
    ctclib_slistnode_t *node = NULL;

    for (node = p_list->head; node; node = node->next)
    {
        if (node->data == data)
        {
            if (node->prev)
            {
                node->prev->next = node->next;
            }
            else
            {
                p_list->head = node->next;
            }

            if (node->next)
            {
                node->next->prev = node->prev;
            }
            else
            {
                p_list->tail = node->prev;
            }
            p_list->count--;

            XFREE(MEM_LIB_SLISTNODE, node);
            return 0;
        }
    }

    return -1;
}

ctclib_slistnode_t*
ctclib_slistnode_lookup(ctclib_slist_t *p_list, void *data)
{
    ctclib_slistnode_t *node = NULL;

    for (node = p_list->head; node; node = node->next)
    {
        if (data == ((node) ? node->data : NULL))
        {
            return node;
        }
    }
    
    return NULL;
}

void*
ctclib_slistdata_lookup(ctclib_slist_t *p_list, void *data)
{
    ctclib_slistnode_t *node = NULL;
    int32 rc = 0;

    if (!p_list || !data || !p_list->cmp)
    {
        return NULL;
    }

    for (node = p_list->head; node; node = node->next)
    {
        if (NULL == node->data)
        {
            continue;
        }

        rc = p_list->cmp(data, node->data);
        if (0 == rc)
        {
            return node->data;
        }
    }
    
    return NULL;
}


int32
ctclib_slist_count(ctclib_slist_t *p_list)
{
    if (!p_list)
    {
        return -1;
    }

    return p_list->count;
}


