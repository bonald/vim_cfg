/**
 @file ctc_opf.c

 @date 2009-10-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_opf.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

struct ctc_offset_node_s
{
    ctc_slistnode_t head;
    uint32 start;
    uint32 end;
};
typedef struct ctc_offset_node_s ctc_offset_node_t;

#define CTC_OPF_TYPE_VALID_CHECK(ptr)                                       \
    {                                                                       \
        CTC_PTR_VALID_CHECK(ptr);                                           \
        if (NULL == opf_master){return CTC_E_INIT_FAIL; } \
        if (NULL == opf_master->start_offset_a[ptr->pool_type]){return CTC_E_NO_RESOURCE; } \
    }

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/

ctc_opf_master_t* opf_master = NULL;

/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
ctc_opf_init(uint8 opf_type, uint8 pool_num)
{
    uint8 type_index = 0;

    if (NULL == opf_master)
    {
        opf_master = (ctc_opf_master_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_master_t));
        CTC_PTR_VALID_CHECK(opf_master);
        sal_memset(opf_master, 0, sizeof(ctc_opf_master_t));

        opf_master->ppp_opf_forward = (ctc_opf_entry_t***)mem_malloc(MEM_OPF_MODULE, CTC_MAX_OPF_TBL_NUM * sizeof(void*));
        CTC_PTR_VALID_CHECK(opf_master->ppp_opf_forward);
        sal_memset(opf_master->ppp_opf_forward, 0, CTC_MAX_OPF_TBL_NUM * sizeof(void*));

        opf_master->ppp_opf_reverse = (ctc_opf_entry_t***)mem_malloc(MEM_OPF_MODULE, CTC_MAX_OPF_TBL_NUM * sizeof(void*));
        CTC_PTR_VALID_CHECK(opf_master->ppp_opf_reverse);
        sal_memset(opf_master->ppp_opf_reverse, 0, CTC_MAX_OPF_TBL_NUM * sizeof(void*));

    }

    type_index = opf_type;
    opf_master->count++;
    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {

        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (NULL != opf_master->ppp_opf_forward[type_index] || NULL != opf_master->ppp_opf_reverse[type_index])
    {
        return CTC_E_NONE;
    }

    opf_master->ppp_opf_forward[type_index] = (ctc_opf_entry_t**)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(void*));
    CTC_PTR_VALID_CHECK(opf_master->ppp_opf_forward[type_index]);
    sal_memset(opf_master->ppp_opf_forward[type_index], 0, pool_num * sizeof(void*));

    opf_master->ppp_opf_reverse[type_index] = (ctc_opf_entry_t**)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(void*));
    CTC_PTR_VALID_CHECK(opf_master->ppp_opf_reverse[type_index]);
    sal_memset(opf_master->ppp_opf_reverse[type_index], 0, pool_num * sizeof(void*));

    opf_master->start_offset_a[type_index] = (uint32*)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(uint32));
    CTC_PTR_VALID_CHECK(opf_master->start_offset_a[type_index]);

    opf_master->max_size_a[type_index] = (uint32*)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(uint32));
    CTC_PTR_VALID_CHECK(opf_master->max_size_a[type_index]);

    opf_master->forward_bound[type_index] = (uint32*)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(uint32));
    CTC_PTR_VALID_CHECK(opf_master->forward_bound[type_index]);

    opf_master->reverse_bound[type_index] = (uint32*)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(uint32));
    CTC_PTR_VALID_CHECK(opf_master->reverse_bound[type_index]);

    opf_master->is_reserve[type_index] = (uint8*)mem_malloc(MEM_OPF_MODULE, pool_num * sizeof(uint8));
    CTC_PTR_VALID_CHECK(opf_master->is_reserve[type_index]);

    opf_master->pool_num[type_index] = pool_num;

    return CTC_E_NONE;
}

int32
ctc_opf_deinit(uint8 lchip, uint8 type)
{
    ctc_opf_entry_t* p_rev_list_first_entry = NULL;
    ctc_opf_entry_t* p_forward_list_first_entry = NULL;
    ctc_opf_entry_t* p_entry = NULL;
    ctc_opf_entry_t* p_next = NULL;
    uint8 pool_index = 0;
    uint8 pool_num = 0;

    if (type >= CTC_MAX_OPF_TBL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    pool_num = opf_master->pool_num[type];


    for (pool_index=0; pool_index < pool_num; pool_index++)
    {
        p_rev_list_first_entry = opf_master->ppp_opf_reverse[type][pool_index];
        if (NULL != p_rev_list_first_entry)
        {
            if (NULL != p_rev_list_first_entry->next)
            {
                p_entry = p_rev_list_first_entry->next;

                /* walk through the entry list */
                while (p_entry && (p_entry != p_rev_list_first_entry))
                {
                    p_next = p_entry->next;
                    mem_free(p_entry);
                    p_entry = p_next;
                }
            }
            mem_free(p_rev_list_first_entry);
        }
        p_forward_list_first_entry = opf_master->ppp_opf_forward[type][pool_index];

        if (NULL != p_forward_list_first_entry)
        {
            if (NULL != p_forward_list_first_entry->next)
            {
                p_entry = p_forward_list_first_entry->next;

                /* walk through the entry list */
                while (p_entry && (p_entry != p_forward_list_first_entry))
                {
                    p_next = p_entry->next;
                    mem_free(p_entry);
                    p_entry = p_next;
                }
            }
            mem_free(p_forward_list_first_entry);
        }
    }

    mem_free(opf_master->start_offset_a[type]);
    mem_free(opf_master->max_size_a[type]);
    mem_free(opf_master->forward_bound[type]);
    mem_free(opf_master->reverse_bound[type]);
    mem_free(opf_master->is_reserve[type]);
    mem_free(opf_master->ppp_opf_forward[type]);
    mem_free(opf_master->ppp_opf_reverse[type]);


    if (0 == opf_master->count)
    {
        mem_free(opf_master->ppp_opf_reverse);
        mem_free(opf_master->ppp_opf_forward);
        mem_free(opf_master);
    }

    return CTC_E_NONE;
}


int32
ctc_opf_free(uint8 type, uint8 pool_index)
{
    ctc_opf_entry_t* p_rev_list_first_entry = NULL;
    ctc_opf_entry_t* p_forward_list_first_entry = NULL;
    ctc_opf_entry_t* p_entry = NULL;
    ctc_opf_entry_t* p_next = NULL;
    uint32 start_offset = 0;
    uint32 max_size = 0;

    if (pool_index >= CTC_MAX_OPF_TBL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type])
    {
        return CTC_E_INVALID_PARAM;
    }

    p_rev_list_first_entry = opf_master->ppp_opf_reverse[type][pool_index];
    if (!p_rev_list_first_entry)
    {
        return CTC_E_NOT_EXIST;
    }

    if ((NULL != p_rev_list_first_entry->next) && (NULL != p_rev_list_first_entry->prev))
    {
        p_entry = p_rev_list_first_entry->next;

        /* walk through the entry list */
        while (p_entry != p_rev_list_first_entry)
        {
            p_next = p_entry->next;
            mem_free(p_entry);
            p_entry = p_next;
        }
    }

    p_forward_list_first_entry = opf_master->ppp_opf_forward[type][pool_index];

    if ((NULL != p_forward_list_first_entry->next) && (NULL != p_forward_list_first_entry->prev))
    {
        p_entry = p_forward_list_first_entry->next;

        /* walk through the entry list */
        while (p_entry != p_forward_list_first_entry)
        {
            p_next = p_entry->next;
            mem_free(p_entry);
            p_entry = p_next;
        }
    }

    start_offset = opf_master->start_offset_a[type][pool_index];
    max_size = opf_master->max_size_a[type][pool_index];
    opf_master->forward_bound[type][pool_index] = start_offset;
    opf_master->reverse_bound[type][pool_index] = start_offset + max_size;
    opf_master->ppp_opf_forward[type][pool_index]->next = NULL;
    opf_master->ppp_opf_forward[type][pool_index]->prev = NULL;
    opf_master->ppp_opf_forward[type][pool_index]->offset = start_offset;
    opf_master->ppp_opf_forward[type][pool_index]->size = max_size;
    opf_master->ppp_opf_reverse[type][pool_index]->next = NULL;
    opf_master->ppp_opf_reverse[type][pool_index]->prev = NULL;
    opf_master->ppp_opf_reverse[type][pool_index]->offset = start_offset;
    opf_master->ppp_opf_reverse[type][pool_index]->size = max_size;

    return CTC_E_NONE;
}

int32
ctc_opf_init_offset(ctc_opf_t* opf, uint32 start_offset, uint32 max_size)
{
    uint8 type_index = 0;
    uint8 pool_index = 0;

    CTC_PTR_VALID_CHECK(opf);
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[opf->pool_type])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);

        return CTC_E_INVALID_PARAM;
    }

    if (NULL != opf_master->ppp_opf_forward[type_index][pool_index])
    {
        return CTC_E_EXIST;
    }

    opf_master->ppp_opf_forward[type_index][pool_index] = (ctc_opf_entry_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_entry_t));
    CTC_PTR_VALID_CHECK(opf_master->ppp_opf_forward[type_index][pool_index]);

    opf_master->ppp_opf_reverse[type_index][pool_index] = (ctc_opf_entry_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_entry_t));
    CTC_PTR_VALID_CHECK(opf_master->ppp_opf_reverse[type_index][pool_index]);

    opf_master->start_offset_a[type_index][pool_index] = start_offset;
    opf_master->max_size_a[type_index][pool_index] = max_size;

    opf_master->forward_bound[type_index][pool_index] = start_offset;

    opf_master->reverse_bound[type_index][pool_index] = start_offset + max_size; /* should not minus 1 */
    opf_master->is_reserve[type_index][pool_index] = FALSE;

    opf_master->ppp_opf_forward[type_index][pool_index]->next = NULL;
    opf_master->ppp_opf_forward[type_index][pool_index]->prev = NULL;
    opf_master->ppp_opf_forward[type_index][pool_index]->offset = start_offset;
    opf_master->ppp_opf_forward[type_index][pool_index]->size = max_size;

    opf_master->ppp_opf_reverse[type_index][pool_index]->next = NULL;
    opf_master->ppp_opf_reverse[type_index][pool_index]->prev = NULL;
    opf_master->ppp_opf_reverse[type_index][pool_index]->offset = start_offset;
    opf_master->ppp_opf_reverse[type_index][pool_index]->size = max_size;

    return CTC_E_NONE;
}

int32
ctc_opf_init_reverse_size(ctc_opf_t* opf, uint32 block_size)
{
    uint8 type_index = 0;
    uint8 pool_index = 0;
    uint32 start_offset = 0;
    uint32 max_size = 0;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);

        return CTC_E_INVALID_PARAM;
    }

    if (block_size > opf_master->max_size_a[type_index][pool_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid block_size:%d\n", block_size);

        return CTC_E_INVALID_PARAM;
    }

    if (block_size != 0)
    {
        start_offset = opf_master->start_offset_a[type_index][pool_index];
        max_size = opf_master->max_size_a[type_index][pool_index];

        opf_master->is_reserve[type_index][pool_index] = TRUE;
        opf_master->forward_bound[type_index][pool_index] = start_offset + max_size - block_size;
        opf_master->reverse_bound[type_index][pool_index] = start_offset + max_size - block_size;
    }

    return CTC_E_NONE;
}

#if 0
STATIC int32
_ctc_opf_set_bound(uint8 type_index, uint8 pool_index, uint8 reverse)
{

    uint32 start_offset = 0;
    uint32 max_size     = 0;
    uint32 end_offset   = 0;

    ctc_opf_entry_t* curr = NULL;
    ctc_opf_entry_t* prev = NULL;

    start_offset = opf_master->start_offset_a[type_index][pool_index];
    max_size     = opf_master->max_size_a[type_index][pool_index];
    end_offset   = start_offset + max_size;

    if (0 == reverse)
    {
        curr = opf_master->ppp_opf_forward[type_index][pool_index];
        prev = NULL;

        while (curr)
        {
            prev = curr;
            curr = curr->next;
        }

        if ((!prev) || ((prev->offset + prev->size) != max_size))
        { /* head is null, full free entry*/
            opf_master->forward_bound[type_index][pool_index] = max_size;
        }
        else
        {
            opf_master->forward_bound[type_index][pool_index] = prev->offset;
        }
    }
    else
    {
        curr = opf_master->ppp_opf_reverse[type_index][pool_index];
        prev = NULL;

        while (curr)
        {
            prev = curr;
            curr = curr->next;
        }

        if ((!prev) || ((prev->offset + prev->size) != max_size))
        { /* head is null, full free entry*/
            opf_master->reverse_bound[type_index][pool_index] = start_offset;
        }
        else
        {
            opf_master->reverse_bound[type_index][pool_index] =
                (end_offset + start_offset) - (prev->offset);
        }
    }

    return CTC_E_NONE;
}

#endif

int32
ctc_opf_get_bound(ctc_opf_t* opf, uint32* forward_bound, uint32* reverse_bound)
{
    uint8  type_index = 0;
    uint8  pool_index = 0;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);

        return CTC_E_INVALID_PARAM;
    }

    *forward_bound = opf_master->forward_bound[type_index][pool_index];
    *reverse_bound = opf_master->reverse_bound[type_index][pool_index];

    return CTC_E_NONE;
}

/*block_size must be inside of one single free_block, cannot cross them */
int32
ctc_opf_alloc_offset(ctc_opf_t* opf, uint32 block_size, uint32* offset)
{
    uint8  type_index = 0;
    uint8  pool_index = 0;
    uint8  free_flag = 0;
    uint32 start_offset = 0;
    uint32 end_offset = 0;
    uint32 revise_offset = 0;
    uint32 max_size = 0;
    uint32 skip_value = 0;
    uint8  multiple = 0;
    uint8  reverse = 0;

    ctc_opf_entry_t* entry, * next;
    ctc_opf_entry_t* node;
    ctc_opf_entry_t** head;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    *offset = CTC_MAX_UINT32_VALUE;
    multiple   = opf->multiple;
    reverse  = opf->reverse;
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    start_offset = opf_master->start_offset_a[type_index][pool_index];
    max_size     = opf_master->max_size_a[type_index][pool_index];
    end_offset   = start_offset + max_size;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);

        return CTC_E_INVALID_PARAM;
    }

    if (block_size == 0)
    {
        block_size = 1;
    }

    if (multiple == 0)
    {
        multiple = 1;
    }

    if (reverse)
    {
        head = &(opf_master->ppp_opf_reverse[type_index][pool_index]);
    }
    else
    {
        head = &(opf_master->ppp_opf_forward[type_index][pool_index]);
    }

    if (block_size > opf_master->max_size_a[type_index][pool_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid block_size:%d\n", block_size);

        return CTC_E_INVALID_PARAM;
    }

    if (reverse == 1)
    {
        entry = *head;

        while (entry)
        {
            revise_offset = (end_offset + start_offset) - (entry->offset + block_size);

            skip_value = revise_offset % multiple;

            if (entry->size >= (skip_value + block_size)
                && ((revise_offset - skip_value) >= opf_master->forward_bound[type_index][pool_index]))
            {
                break;
            }

            entry = entry->next;
        }
    }
    else
    {
        entry = *head;

        while (entry)
        {
            revise_offset = entry->offset;

            skip_value = ((revise_offset + multiple - 1) / multiple * multiple - revise_offset);

            if (entry->size >= (skip_value + block_size)
                && ((revise_offset + block_size - 1 + skip_value) < opf_master->reverse_bound[type_index][pool_index]))
            {
                break;
            }

            entry = entry->next;
        }
    }

    if (!entry)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,
                        "type_index=%d pool_index=%d This pool don't have enough memory!\n", type_index, pool_index);

        return CTC_E_NO_RESOURCE;
    }

    if (skip_value)
    {
        /*insert a node, allocate offset from first multiple offset*/
        node = (ctc_opf_entry_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_entry_t));
        if (node == NULL)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(node, 0, sizeof(ctc_opf_entry_t));
        /*malloc skip*/
        node->size   = entry->size - skip_value;
        node->offset = entry->offset + skip_value;
        node->prev   = entry;
        node->next   = entry->next;

        if(entry->next)
        {
            entry->next->prev = node;
        }
        entry->size = skip_value; /* curr entry become skip node*/
        entry->next = node;       /* skip_node --> new_node */
        entry = node;
        /* new entry, revise_offset is changed */
        if (reverse == 1)
        {
            revise_offset = (end_offset + start_offset) - (entry->offset + block_size);
        }
        else
        {
            revise_offset = entry->offset;
        }
    }

    *offset = revise_offset;

    /* shrink current node */
    entry->size -= block_size;
    entry->offset += block_size;

    if (entry->size == 0) /* free_block becomes empty, delete it */
    {
        next = entry->next;
        if (entry->prev)
        {
            entry->prev->next = next;
            free_flag = 1;
        }
        else
        {
            if (next)
            {
                *head = next;
                free_flag = 1;
            }
        }

        if (next)
        {
            next->prev = entry->prev;
        }

        if (free_flag)
        {
            mem_free(entry);
        }
    }

    if (reverse == 1)
    {
        if (*offset < opf_master->reverse_bound[type_index][pool_index])
        {
            opf_master->reverse_bound[type_index][pool_index] = *offset;
        }
    }
    else
    {
        if ((*offset + block_size) > opf_master->forward_bound[type_index][pool_index])
        {
            opf_master->forward_bound[type_index][pool_index] = *offset + block_size;
        }
    }

    return CTC_E_NONE;
}

int32
ctc_opf_free_offset(ctc_opf_t* opf, uint32 block_size, uint32 offset)
{
    uint8  type_index = 0;
    uint8  pool_index = 0;
    uint8  right_joint = 0;
    uint8  left_joint = 0;

    uint32 start_offset;
    uint32 revise_offset = 0;
    uint32 max_size = 0;
    uint32 end_offset = 0;
    uint8  reverse  = 0;
    uint32* forward_bound = NULL;
    uint32* reverse_bound = NULL;
    ctc_opf_entry_t** head = NULL;
    ctc_opf_entry_t* curr = NULL;
    ctc_opf_entry_t* left_entry  = NULL;
    ctc_opf_entry_t* right_entry = NULL;

    CTC_OPF_TYPE_VALID_CHECK(opf);

    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    start_offset = opf_master->start_offset_a[type_index][pool_index];
    max_size     = opf_master->max_size_a[type_index][pool_index];
    end_offset   = start_offset + max_size;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);

        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n",  pool_index);

        return CTC_E_INVALID_PARAM;
    }

    reverse = (offset >= opf_master->forward_bound[type_index][pool_index]); /* offset is in reverse_entry */

    forward_bound   = &(opf_master->forward_bound[type_index][pool_index]);
    reverse_bound   = &(opf_master->reverse_bound[type_index][pool_index]);
    if (reverse)
    {
        head = &(opf_master->ppp_opf_reverse[type_index][pool_index]);
    }
    else
    {
        head = &(opf_master->ppp_opf_forward[type_index][pool_index]);
    }

    if (block_size == 0)
    {
        block_size = 1;
    }

    if (block_size > max_size)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid block_size:%d\n", block_size);

        return CTC_E_INVALID_PARAM;
    }

    if (reverse)
    {
        /* revise_offset is start offset of forward_entry list */
        revise_offset = (end_offset + start_offset) - (offset + block_size);
    }
    else
    {
        revise_offset = offset;
    }

    if ((revise_offset < start_offset) || ((revise_offset + block_size) > end_offset))
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid offset: %d\n", revise_offset);

        return CTC_E_INVALID_PARAM;
    }

    {
        curr = *head;

        while (curr != NULL)
        {
            /* return error if user range inside current free block */
            if (((revise_offset >= curr->offset) &&
                 (revise_offset <= curr->offset + curr->size - 1))
                || ((revise_offset + block_size - 1 >= curr->offset) &&
                    (revise_offset + block_size <= curr->offset + curr->size)))
            {
                CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "User range inside of free block\n");
                return CTC_E_INVALID_PARAM;
            }

            /* current free entry is right aside of user range, break.  |   .....@@@@@
               |  start_offset
               .  user range
               @  free entry
             */
            if (curr->offset >= revise_offset + block_size)
            {
                if (curr->offset == revise_offset + block_size)
                {
                    right_joint = 1;
                }

                right_entry = curr;
                break;
            }

            /* current free block is left aside of user range, continue. @@@@....   |  */
            if (revise_offset >= curr->offset + curr->size)
            {
                if (revise_offset == curr->offset + curr->size)
                {
                    left_joint = 1;
                }

                left_entry = curr;
            }

            curr = curr->next;
        }

        /*set bound before malloc free. */
        if (0 == reverse) /* forward */
        {
            if ((*forward_bound == (revise_offset + block_size))
                && (!opf_master->is_reserve[type_index][pool_index]))
            {
                if ((left_entry) && (revise_offset == (left_entry->offset + left_entry->size)))
                {
                    *forward_bound = left_entry->offset;
                }
                else
                {
                    *forward_bound = revise_offset;
                }
            }
        }
        else
        {
            if ((*reverse_bound == offset)
                && (!opf_master->is_reserve[type_index][pool_index]))
            {
                if ((left_entry) && (revise_offset == (left_entry->offset + left_entry->size)))
                {
                    *reverse_bound = left_entry->offset;
                }
                else
                {
                    *reverse_bound = revise_offset;
                }

                *reverse_bound = (end_offset + start_offset) - (*reverse_bound);

            }
        }

        if (0 == (right_joint + left_joint))  /* no joint, malloc new curr. |  ****  |  */
        {
            ctc_opf_entry_t* p_new = NULL;
            p_new = (ctc_opf_entry_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_entry_t));
            CTC_PTR_VALID_CHECK(p_new);

            p_new->offset = revise_offset;
            p_new->size = block_size;
            p_new->prev = left_entry;
            p_new->next = right_entry;

            if (left_entry)
            {
                left_entry->next = p_new;
            }
            else
            {
                *head = p_new;
            }

            if (right_entry)
            {
                right_entry->prev = p_new;
            }
        }
        else if (2 == (right_joint + left_joint))  /* 2 joint, merge. @@@@....@@@@ */
        {

            left_entry->size += (block_size + right_entry->size);
            left_entry->next = right_entry->next;

            if (right_entry->next)
            {
                right_entry->next->prev = left_entry;
            }

            mem_free(right_entry);
        }
        else if (left_joint) /* left joint, expand left entry.  @@@@@....   |    */
        {
            left_entry->size += block_size;
        }
        else /* right joint, expand right entry. |   ....@@@@    */
        {
            right_entry->size += block_size;
            right_entry->offset = revise_offset;
        }
    }

    return CTC_E_NONE;
}

int32
ctc_opf_alloc_offset_last(ctc_opf_t* opf, uint32 block_size, uint32* offset)
{
    uint8  type_index = 0;
    uint8  pool_index = 0;
    uint8  free_flag = 0;
    uint32 start_offset = 0;
    uint32 max_size = 0;
    uint32 end_offset = 0;
    uint32 revise_offset = 0;

    ctc_opf_entry_t* entry, * next, * select = NULL;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    *offset = CTC_MAX_UINT32_VALUE;
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    start_offset = opf_master->start_offset_a[type_index][pool_index];
    max_size     = opf_master->max_size_a[type_index][pool_index];
    end_offset   = start_offset + max_size;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);
        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);
        return CTC_E_INVALID_PARAM;
    }

    if (block_size == 0)
    {
        block_size = 1;
    }

    if (block_size > opf_master->max_size_a[type_index][pool_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid block_size:%d\n", block_size);
        return CTC_E_INVALID_PARAM;
    }

    entry = opf_master->ppp_opf_forward[type_index][pool_index];

    while (entry)
    {
        revise_offset = (end_offset - entry->offset - block_size) + start_offset;

        if (entry->size >= block_size &&
            revise_offset < opf_master->reverse_bound[type_index][pool_index])
        {
            select = entry;
        }

        entry = entry->next;
    }

    if (!select)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "type_index=%d pool_index=%d This pool don't have enough memory!\n", type_index, pool_index);
        return CTC_E_NO_RESOURCE;
    }

    *offset = select->offset + select->size - block_size;

    select->size -= block_size;
    if (select->size == 0)
    {
        next = select->next;
        if (select->prev)
        {
            select->prev->next = next;
            free_flag = 1;
        }
        else
        {
            if (next)
            {
                opf_master->ppp_opf_forward[type_index][pool_index] = next;
                free_flag = 1;
            }
        }

        if (next)
        {
            next->prev = select->prev;
        }

        if (free_flag)
        {
            mem_free(select);
        }
    }

    if ((*offset + block_size) > opf_master->forward_bound[type_index][pool_index])
    {
        opf_master->forward_bound[type_index][pool_index] = *offset + block_size;
    }

    return CTC_E_NONE;
}

int32
ctc_opf_alloc_offset_from_position(ctc_opf_t* opf, uint32 block_size, uint32 begin)
{
    uint8  type_index = 0;
    uint8  pool_index = 0;
    uint8  free_flag = 0;
    int32 revise_offset = 0;
    uint32 end;

    ctc_opf_entry_t* entry, * next, * node;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", type_index);
        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index : %d\n", pool_index);
        return CTC_E_INVALID_PARAM;
    }

    if (block_size == 0)
    {
        block_size = 1;
    }

    if (block_size > opf_master->max_size_a[type_index][pool_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid block_size:%d\n", block_size);
        return CTC_E_INVALID_PARAM;
    }

    end = begin + block_size - 1;

    revise_offset = begin + block_size;
    if ((revise_offset > opf_master->reverse_bound[type_index][pool_index]))
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "This offset is allocated!\n");
        return CTC_E_NO_RESOURCE;
    }

    entry = opf_master->ppp_opf_forward[type_index][pool_index];

    while (entry)
    {
        if (begin >= entry->offset + entry->size)
        {
            entry = entry->next;
        }
        else
        {
            if (begin < entry->offset || entry->offset + entry->size < begin + block_size)
            {
                entry = NULL;
            }

            break;
        }
    }

    if (!entry)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "type_index=%d pool_index=%d This pool don't have enough memory!\n", type_index, pool_index);
        return CTC_E_NO_RESOURCE;
    }

    if (entry->offset == begin || entry->offset + entry->size - 1 == end)
    {
        entry->size -= block_size;
        if (entry->size == 0)
        {
            next = entry->next;
            if (entry->prev)
            {
                entry->prev->next = next;
                free_flag = 1;
            }
            else
            {
                if (next)
                {
                    opf_master->ppp_opf_forward[type_index][pool_index] = next;
                    free_flag = 1;
                }
            }

            if (next)
            {
                next->prev = entry->prev;
            }

            if (free_flag)
            {
                mem_free(entry);
            }
        }
        else if (entry->offset == begin)
        {
            entry->offset = end + 1;
        }
    }
    else
    {
        /* insert a new free node */
        node = (ctc_opf_entry_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_opf_entry_t));
        CTC_PTR_VALID_CHECK(node);

        node->offset = end + 1;
        node->size = entry->offset + entry->size - end - 1;
        node->prev = entry;
        node->next = entry->next;

        if(entry->next)
        {
            entry->next->prev = node;
        }
        entry->size = begin - entry->offset;
        entry->next = node;
    }

    if (end + 1 > opf_master->forward_bound[type_index][pool_index])
    {
        opf_master->forward_bound[type_index][pool_index] = end + 1;
    }

    return CTC_E_NONE;
}

STATIC int32
ctc_opf_print_used_offset_list(ctc_slist_t* opf_used_offset_list, ctc_opf_t* opf)
{
    uint8 all_offset_len = 0;
    uint8 row_length = 32;
    char out_offset[11];
    char all_offset_one_row[32];    /*equal to row_length*/
    ctc_offset_node_t* offset_node = NULL;
    ctc_slistnode_t* node = NULL, * next_node = NULL;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    CTC_PTR_VALID_CHECK(opf_used_offset_list);

    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "used offset:\n");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");

    sal_memset(all_offset_one_row, 0, sizeof(all_offset_one_row));
    /* print and free all nodes */
    if (0 == opf_used_offset_list->count)
    {
        if (NULL == opf_master->ppp_opf_forward[opf->pool_type][opf->pool_index])
        {
            CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "all offset used\n");
        }
        else
        {
            CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "no offset used\n");
        }
    }
    else
    {
        CTC_SLIST_LOOP_DEL(opf_used_offset_list, node, next_node)
        {
            offset_node = _ctc_container_of(node, ctc_offset_node_t, head);
            sal_memset(out_offset, 0, sizeof(out_offset));
            if (offset_node->start != offset_node->end)
            {
                sal_sprintf(out_offset, "%d-%d,", offset_node->start, offset_node->end);
            }
            else
            {
                sal_sprintf(out_offset, "%d,", offset_node->start);
            }

            if ((all_offset_len + sal_strlen(out_offset)) >= row_length)
            {
                all_offset_one_row[sal_strlen(all_offset_one_row) - 1] = '\0';
                CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s\n", all_offset_one_row);
                sal_memset(all_offset_one_row, 0, sizeof(all_offset_one_row));
                sal_strcat(all_offset_one_row, out_offset);
            }
            else
            {
                sal_strcat(all_offset_one_row, out_offset);
            }

            all_offset_len = sal_strlen(all_offset_one_row);

        }
    }

    all_offset_one_row[sal_strlen(all_offset_one_row) - 1] = '\0';
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s\n", all_offset_one_row);
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");

    return CTC_E_NONE;
}

STATIC int32
ctc_opf_free_used_offset_list(ctc_slist_t* opf_used_offset_list)
{
    ctc_slistnode_t* node = NULL, * next_node = NULL;

    CTC_PTR_VALID_CHECK(opf_used_offset_list);

    CTC_SLIST_LOOP_DEL(opf_used_offset_list, node, next_node)
    {
        ctc_slist_delete_node(opf_used_offset_list, node);
        mem_free(node);
    }

    ctc_slist_delete(opf_used_offset_list);
    opf_used_offset_list = NULL;
    node = NULL;
    next_node = NULL;

    return CTC_E_NONE;
}

int32
ctc_opf_print_alloc_info(ctc_opf_t* opf)
{
    uint8 type_index = 0;
    uint8 pool_index = 0;
    uint32 offset_index = 0;
    uint32 min_offset = 0;
    uint32 max_offset = 0;
    uint32 pre_start = 0;
    uint32 pre_end = 0;
    uint32 start = 0;
    uint32 end = 0;
    int32 ret = CTC_E_NONE;
    uint32 start_offset = 0;

    ctc_opf_entry_t* entry;
    ctc_offset_node_t* offset_node = NULL;
    ctc_slist_t* offset_slist_pr = NULL;
    ctc_slist_t* offset_slist_bf = NULL;
    ctc_slistnode_t* tmp_head_node = NULL;

    CTC_OPF_TYPE_VALID_CHECK(opf);

    offset_slist_pr = ctc_slist_new();
    CTC_PTR_VALID_CHECK(offset_slist_pr);

    offset_slist_bf = ctc_slist_new();
    if (NULL == offset_slist_bf)
    {
        ctc_slist_delete(offset_slist_pr);
        return CTC_E_INVALID_PTR;
    }


    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index: %d \n", type_index);
        ret = CTC_E_INVALID_PARAM;
        goto end;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid pool index:%d\n", pool_index);
        ret = CTC_E_INVALID_PARAM;
        goto end;
    }

    max_offset = opf_master->start_offset_a[type_index][pool_index] + opf_master->max_size_a[type_index][pool_index] - 1;
    min_offset = opf_master->start_offset_a[type_index][pool_index];
    start_offset = opf_master->start_offset_a[type_index][pool_index];
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "min offset:%u \n", min_offset);
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "max offset:%u \n", max_offset);
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "current allocated max offset for previous allocation scheme:%u \n", opf_master->forward_bound[type_index][pool_index]);
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "current allocated min offset for reverse allocation scheme:%u \n", opf_master->reverse_bound[type_index][pool_index]);

    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "previous allocation scheme\n");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%6s    %7s    %4s\n", "index", "offset", "size");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");

    pre_end = max_offset;
    pre_start = start_offset;
    entry = opf_master->ppp_opf_forward[type_index][pool_index];

    /* walk through the entry list */
    while (entry)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%6u    %7u    %4u \n", offset_index, entry->offset, entry->size);

        if (min_offset == entry->offset)
        {
            pre_start = entry->offset + entry->size;
        }
        else
        {
            start = pre_start;
            end = entry->offset - 1;

            /* offset_slist_pr add node */
            offset_node = (ctc_offset_node_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_offset_node_t));
            if (NULL == offset_node)
            {
                ret = CTC_E_NO_MEMORY;
                goto end;
            }

            offset_node->start = start;
            offset_node->end = end;
            offset_node->head.next = NULL;

            /* different from offset_slist_bf */
            ctc_slist_add_tail(offset_slist_pr, &offset_node->head);
            pre_start = entry->offset + entry->size;
        }

        entry = entry->next;
        offset_index++;
    }

    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nreverse allocation scheme\n");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%6s    %7s    %4s\n", "index", "offset", "size");
    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");
    offset_index = 0;
    entry = opf_master->ppp_opf_reverse[type_index][pool_index];

    /* walk through the entry list */
    while (entry)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%6u    %7u    %4u \n", offset_index, entry->offset, entry->size);

        if (min_offset == entry->offset)
        {
            pre_end = max_offset - entry->size;
        }
        else
        {
            start = max_offset - (entry->offset-start_offset) + 1;
            end = pre_end;

            /* offset_slist_bf add node */
            offset_node = (ctc_offset_node_t*)mem_malloc(MEM_OPF_MODULE, sizeof(ctc_offset_node_t));
            if (NULL == offset_node)
            {
                ret = CTC_E_NO_MEMORY;
                goto end;
            }

            offset_node->start = start;
            offset_node->end = end;
            offset_node->head.next = NULL;
            ctc_slist_add_head(offset_slist_bf, &offset_node->head);

            pre_end = max_offset - (entry->offset + entry->size - start_offset);
        }

        entry = entry->next;
        offset_index++;
    }

    CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------\n");

    /* merge offset_slist_pr and offset_slist_bf */
    tmp_head_node = CTC_SLISTHEAD(offset_slist_bf);
    ctc_slist_add_tail(offset_slist_pr, tmp_head_node);

    CTC_ERROR_GOTO(ctc_opf_print_used_offset_list(offset_slist_pr, opf), ret, end);
    CTC_ERROR_GOTO(ctc_opf_free_used_offset_list(offset_slist_pr), ret, end);
    ctc_slist_delete(offset_slist_bf);
    tmp_head_node = NULL;

    return CTC_E_NONE;
end:
    ctc_slist_delete(offset_slist_pr);
    ctc_slist_delete(offset_slist_bf);
    return ret;
}

int32
ctc_opf_print_sample_info(ctc_opf_t* opf)
{
    uint8 type_index = 0;
    uint8 pool_index = 0;
    ctc_opf_entry_t* entry;

    CTC_OPF_TYPE_VALID_CHECK(opf);
    type_index = opf->pool_type;
    pool_index = opf->pool_index;

    if (type_index >= CTC_MAX_OPF_TBL_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (pool_index >= opf_master->pool_num[type_index])
    {
        return CTC_E_INVALID_PARAM;
    }

    entry = opf_master->ppp_opf_forward[type_index][pool_index];

    /* walk through the entry list */
    while (entry)
    {
        CTC_OPF_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "offset %u    size %u\n", entry->offset, entry->size);
        entry = entry->next;
    }

    return CTC_E_NONE;
}

