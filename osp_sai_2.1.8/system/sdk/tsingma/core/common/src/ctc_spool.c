
/****************************************************************************
*file ctc_spool.h

*author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

*date 2012-04-23

*version v2.0

The file define  Share pool lib
****************************************************************************/
#include "sal.h"
#include "ctc_spool.h"
#include "ctc_error.h"

#define STATIC_REF_CNT 0xFFFFFFFF
#define NOT_STATIC(node) (STATIC_REF_CNT != node->ref_cnt)



/*lookup spool_node*/
ctc_spool_node_t*
_ctc_spool_lookup(ctc_spool_t* spool,  void* data)
{
    uint32 index = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_spool_node_t* node = NULL;

    if (!spool)
    {
        return NULL;
    }

    index = (*spool->spool_key)(data) % (spool->block_size * spool->block_num);

    idx_1d  = index / spool->block_size;
    idx_2d  = index % spool->block_size;

    if (!spool->index[idx_1d])
    {
        return NULL;
    }

    for (node = spool->index[idx_1d][idx_2d]; node != NULL; node = node->next)
    {
        if ((*spool->spool_cmp)(node->data, data) == TRUE)
        {
            return node;
        }
    }

    return NULL;

}

bool
ctc_spool_is_static(ctc_spool_t* spool, void* data)
{
    ctc_spool_node_t* node = NULL;

    node = _ctc_spool_lookup(spool, data);

    if (node && (STATIC_REF_CNT == node->ref_cnt))
    {
        return TRUE;
    }

    return FALSE;
}

void*
ctc_spool_lookup(ctc_spool_t* spool,  void* data)
{
    ctc_spool_node_t* node = NULL;

    node = _ctc_spool_lookup(spool, data);
    if (node)
    {
        return node->data;
    }
    else
    {
        return NULL;
    }
}

STATIC int32
_ctc_spool_add(ctc_spool_t* spool, void* data, void* out_data, uint8 is_static)
{

    uint32 index = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    int32 ret = 0;
    ctc_spool_node_t* node_new = NULL;
    ctc_spool_node_t* node_lkup = NULL;
    void** cast = (void**)out_data;

    if (!spool)
    {
        return CTC_SPOOL_E_NOT_INIT;
    }

    /*#1 lookup: if exists?*/
    if(!is_static)
    {
        node_lkup = _ctc_spool_lookup(spool, data);
    }

    if (node_lkup)
    {   /*#2 yes, use old*/
        if (NOT_STATIC(node_lkup))
        {
            node_lkup->ref_cnt++;
        }
        if (cast)
        {
            *cast = node_lkup->data;
        }
        /**is_new = FALSE;*/
        ret = CTC_SPOOL_E_OPERATE_REFCNT;
    }
    else
    {   /*#3 no, create new */
        if (spool->count >= spool->max_count)
        {
            return CTC_SPOOL_E_FULL;
        }

        index = (*spool->spool_key)(data) % (spool->block_size * spool->block_num);
        idx_1d  = index / spool->block_size;
        idx_2d  = index % spool->block_size;

        if (!spool->index[idx_1d])
        {
            spool->index[idx_1d] = (void*)mem_malloc(MEM_SPOOL_MODULE,
                                                     spool->block_size * sizeof(void*));

            if (!spool->index[idx_1d])
            {
                return CTC_SPOOL_E_NO_MEMORY;
            }

            sal_memset(spool->index[idx_1d], 0,  spool->block_size * sizeof(void*));
        }

        node_new = mem_malloc(MEM_SPOOL_MODULE, sizeof(ctc_spool_node_t));
        if(!node_new)
        {
            return CTC_SPOOL_E_NO_MEMORY;
        }
       /*#4 create success */
        sal_memset(node_new, 0, sizeof(ctc_spool_node_t));
        node_new->data = data;
        node_new->next = spool->index[idx_1d][idx_2d];

        if(!is_static)
        {
            node_new->ref_cnt = 1;
        }
        else
        {
            node_new->ref_cnt = STATIC_REF_CNT;
        }

        spool->index[idx_1d][idx_2d] = node_new;

        spool->count++;
        if (cast)
        {
            *cast = data;
        }
        /**is_new = TRUE;*/
        ret = CTC_SPOOL_E_OPERATE_MEMORY;
    }

    return ret;
}

/*
  add       :called when key non-exists
  update    :called when key exists
*/
/*
STATIC add used to intial add
*/
int32
ctc_spool_static_add(ctc_spool_t* spool, void* data)
{
    uint32 index = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    int32  ret = 0;
    void* data_tmp;
    ctc_spool_node_t* node_new = NULL;

    if (!spool)
    {
        return CTC_SPOOL_E_NOT_INIT;
    }

    if(spool->spool_alloc && spool->spool_free)
    {
        data_tmp = mem_malloc(MEM_SPOOL_MODULE, spool->user_data_size);
        if(NULL == data_tmp)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memcpy(data_tmp, data, spool->user_data_size);
        ret = _ctc_spool_add(spool, data_tmp, NULL, 1);
        if(ret < 0)
        {
            mem_free(data_tmp);
            ret = CTC_E_NO_MEMORY;
        }
        else
        {
            ret = CTC_E_NONE;
        }
    }
    else
    {

        index = (*spool->spool_key)(data) % (spool->block_size * spool->block_num);
        idx_1d  = index / spool->block_size;
        idx_2d  = index % spool->block_size;

        if (!spool->index[idx_1d])
        {
            spool->index[idx_1d] = (void*)mem_malloc(MEM_SPOOL_MODULE,
                                                     spool->block_size * sizeof(void*));

            if (!spool->index[idx_1d])
            {
                return CTC_SPOOL_E_NO_MEMORY;
            }

            sal_memset(spool->index[idx_1d], 0,  spool->block_size * sizeof(void*));
        }

        node_new = mem_malloc(MEM_SPOOL_MODULE, sizeof(ctc_spool_node_t));
        if (!node_new)
        {
            ret = CTC_SPOOL_E_NO_MEMORY;
        }
        else
        {
            sal_memset(node_new, 0, sizeof(ctc_spool_node_t));
            node_new->data = data;
            node_new->next = spool->index[idx_1d][idx_2d];
            node_new->ref_cnt = STATIC_REF_CNT;

            spool->index[idx_1d][idx_2d] = node_new;

            spool->count++;
            ret = CTC_SPOOL_E_OPERATE_MEMORY;
        }
    }
    return ret;

}

/* if data_out = NULL, won't return find data.
 * if data_out != NULL, will return find data. so no need call ctc_spool_lookup.
 */
STATIC int32
_ctc_spool_remove(ctc_spool_t* spool, void* data, void* data_out, void** data_out2)
{
    uint32 index = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    int32 ret = 0;
    ctc_spool_node_t* node = NULL;
    ctc_spool_node_t* node_prev = NULL;
    ctc_spool_node_t* node_next = NULL;
    void** cast = (void**)data_out;
    void*  out = NULL;

    index = (*spool->spool_key)(data) % (spool->block_size * spool->block_num);
    idx_1d  = index / spool->block_size;
    idx_2d  = index % spool->block_size;

    if (!spool->index[idx_1d])
    {
        return CTC_SPOOL_E_ENTRY_NOT_EXIST;
    }
    ret = CTC_SPOOL_E_ENTRY_NOT_EXIST;

    for (node = spool->index[idx_1d][idx_2d];
         node; node_prev = node, node = node_next)
    {
        node_next = node->next;
        if ((*spool->spool_cmp)(node->data, data) == TRUE)
        {
            if (!NOT_STATIC(node))
            {
                return CTC_SPOOL_E_NONE;
            }

            if (node->ref_cnt > 1)
            {
                node->ref_cnt--;
                out = node->data;

                ret = CTC_SPOOL_E_OPERATE_REFCNT;
            }
            else if (node->ref_cnt == 1)
            {
                if (node == spool->index[idx_1d][idx_2d])
                { /*head node*/
                    spool->index[idx_1d][idx_2d] = node_next;
                }
                else
                {
                    node_prev->next = node_next;
                }

                out = node->data;

                mem_free(node);
                spool->count--;
                ret = CTC_SPOOL_E_OPERATE_MEMORY;
            }
            else
            {
            }

            break;
        }
    }

    if (ret < 0)
    {
        out = NULL;
    }

    /* input ptr is valid */
    if (cast)
    {
        *cast = out;
    }
    if (data_out2)
    {
        *data_out2 = out;
    }

    return ret;
}

int32
ctc_spool_remove(ctc_spool_t* spool, void* data, void* data_out)
{
    int32 ret = 0;
    void* data_out2;

    ret = _ctc_spool_remove(spool, data, data_out, &data_out2);

    if(spool->spool_alloc && spool->spool_free)
    {
        if(ret < 0)
        {
            ret = CTC_E_NOT_EXIST;
        }
        else if(CTC_SPOOL_E_OPERATE_MEMORY == ret)
        {
            spool->spool_free(data_out2, &(spool->lchip));
            mem_free(data_out2);
            ret = CTC_E_NONE;
        }
        else if(CTC_SPOOL_E_OPERATE_REFCNT == ret)
        {
            ret = CTC_E_NONE;
        }
    }

    return ret;
}


/*
return value:
0  -- error: data not exist
1~ -- success.
*/
int32
ctc_spool_get_refcnt(ctc_spool_t* spool, void* data)
{
    ctc_spool_node_t*  node_lkup  = NULL;

    node_lkup = _ctc_spool_lookup(spool, data);
    if (node_lkup)
    {
        return node_lkup->ref_cnt;
    }
    else
    {
        return 0;
    }
}


int32
ctc_spool_add(ctc_spool_t* spool, void* new_data, void* old_data, void* out_data)
{
    int32 ret = 0;
    void* data_tmp = NULL;
    ctc_spool_node_t* old_node_lkup = NULL;
    ctc_spool_node_t* new_node_lkup = NULL;

    if(spool->spool_alloc && spool->spool_free)
    {

        if(old_data)
        {
            old_node_lkup = _ctc_spool_lookup(spool, old_data);
            new_node_lkup = _ctc_spool_lookup(spool, new_data);
        }

        if(old_node_lkup && (1 == old_node_lkup->ref_cnt) && (NULL == new_node_lkup))   /*replace*/
        {
            if (NOT_STATIC(old_node_lkup))
            {
                spool->spool_free(old_node_lkup->data, &(spool->lchip));
                data_tmp = old_node_lkup->data;
                _ctc_spool_remove(spool, old_data, NULL, NULL);
                sal_memcpy(data_tmp, new_data, spool->user_data_size);
                _ctc_spool_add(spool, data_tmp, out_data, 0);
                spool->spool_alloc(data_tmp, &(spool->lchip));
            }
            ret = CTC_E_NONE;
        }
        else
        {
            data_tmp = mem_malloc(MEM_SPOOL_MODULE, spool->user_data_size);
            if(NULL == data_tmp)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memcpy(data_tmp, new_data, spool->user_data_size);
            ret = _ctc_spool_add(spool, data_tmp, out_data, 0);
            if(ret < CTC_SPOOL_E_NONE)
            {
                if(CTC_SPOOL_E_NOT_INIT == ret)
                {
                    ret = CTC_E_NOT_INIT;
                }
                else if(CTC_SPOOL_E_NO_MEMORY == ret)
                {
                    ret = CTC_E_NO_MEMORY;
                }
                else if(CTC_SPOOL_E_FULL == ret)
                {
                    ret = CTC_E_NO_RESOURCE;
                }
                else if(CTC_SPOOL_E_ENTRY_NOT_EXIST == ret)
                {
                    ret = CTC_E_NOT_EXIST;
                }
                mem_free(data_tmp);
                return ret;
            }
            else
            {
                if(CTC_SPOOL_E_OPERATE_REFCNT == ret)
                {
                    mem_free(data_tmp);
                    ret = CTC_E_NONE;
                }
                else if(CTC_SPOOL_E_OPERATE_MEMORY == ret)
                {
                    ret = spool->spool_alloc(data_tmp, &(spool->lchip));
                    if(ret < 0)
                    {
                        _ctc_spool_remove(spool, data_tmp, NULL, NULL);
                        mem_free(data_tmp);
                        return CTC_E_NO_RESOURCE;
                    }
                }
            }

            if(old_node_lkup)    /*remove old*/
            {
                if(CTC_E_NONE == ret)
                {
                    ctc_spool_remove(spool, old_data, NULL);
                }
            }
        }

    }
    else
    {
        ret = _ctc_spool_add(spool, new_data, out_data, 0);

        if (old_data && (CTC_SPOOL_E_FULL == ret))
        {
            old_node_lkup = _ctc_spool_lookup(spool, old_data);
            if (old_node_lkup && 1 == old_node_lkup->ref_cnt)
            {
                ctc_spool_remove(spool, old_data, NULL);
                sal_memcpy(old_data, new_data, spool->user_data_size);
                _ctc_spool_add(spool, old_data, out_data, 0);
                ret = CTC_SPOOL_E_OPERATE_REFCNT;
            }
        }
    }

    return ret;

}

/*traverse all node*/
int32
ctc_spool_traverse(ctc_spool_t* spool, spool_traversal_fn fn, void* user_data)
{
    uint32 count = 0;
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_spool_node_t* node = NULL;
    ctc_spool_node_t* node_next = NULL;

    int32 ret = 0;

    if (!spool)
    {
        return -1;
    }

    if (spool->count == 0)
    {
        return 0;
    }

    for (index1 = 0; index1 < spool->block_num; index1++)
    {
        if (!spool->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < spool->block_size; index2++)
        {
            for (node = spool->index[index1][index2]; node != NULL; node = node_next)
            {
                count++;
                node_next = node->next;
                if ((ret = (* fn)(node, user_data)) < 0)
                {
                    return ret;
                }

                if (count >= spool->count)
                {
                    return 0;
                }
            }
        }
    }

    return 0;

}

void
ctc_spool_free(ctc_spool_t* spool)
{
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_spool_node_t* node = NULL;
    ctc_spool_node_t* node_next = NULL;

    for (index1 = 0; index1 < spool->block_num; index1++)
    {
        if (!spool->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < spool->block_size; index2++)
        {
            for (node = spool->index[index1][index2]; node; node = node_next)
            {
                node_next = node->next;
                mem_free(node->data);
                mem_free(node);
            }
        }

        mem_free(spool->index[index1]);
    }

    mem_free(spool->index);
    mem_free(spool);
}

ctc_spool_t*
ctc_spool_create(ctc_spool_t *spool)
{
    ctc_spool_t* spool_new = NULL;

    spool_new = mem_malloc(MEM_SPOOL_MODULE, sizeof(ctc_spool_t));

    if (NULL == spool_new)
    {
        return NULL;
    }

    sal_memset(spool_new, 0, sizeof(ctc_spool_t));
    spool_new->index = mem_malloc(MEM_SPOOL_MODULE, spool->block_num * sizeof(void*));

    if (NULL == spool_new->index)
    {
        mem_free(spool_new);
        return NULL;
    }

    sal_memset(spool_new->index, 0, sizeof(void*) * spool->block_num);

    spool_new->lchip = spool->lchip;
    spool_new->user_data_size = spool->user_data_size;
    spool_new->block_size = spool->block_size;
    spool_new->block_num = spool->block_num;
    spool_new->max_count = spool->max_count;

    spool_new->spool_key = spool->spool_key;
    spool_new->spool_cmp = spool->spool_cmp;
    spool_new->spool_alloc = spool->spool_alloc;
    spool_new->spool_free = spool->spool_free;

    return spool_new;
}

