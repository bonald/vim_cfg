/****************************************************************************
 *file ctc_hash.c

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2009-11-26

 *version v2.0

 The file define  HASH arithmetic lib
 ****************************************************************************/
#include "sal.h"
#include "ctc_hash.h"

ctc_hash_t*
ctc_hash_create(uint16 block_num, uint16 block_size, hash_key_fn hash_key, hash_cmp_fn hash_cmp)
{
    ctc_hash_t* hash = NULL;

    hash = mem_malloc(MEM_HASH_MODULE, sizeof(ctc_hash_t));

    if (NULL == hash)
    {
        return NULL;
    }

    sal_memset(hash, 0, sizeof(ctc_hash_t));
    hash->index = mem_malloc(MEM_HASH_MODULE,  block_num * sizeof(void*));

    if (NULL == hash->index)
    {
        mem_free(hash);
        return NULL;
    }

    sal_memset(hash->index, 0, sizeof(void*) * block_num);
    hash->block_size = block_size;
    hash->block_num = block_num;
    hash->hash_key = hash_key;
    hash->hash_cmp = hash_cmp;
    return hash;
}

void*
ctc_hash_lookup(ctc_hash_t* hash, void* data)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;

    if (!hash)
    {
        return NULL;
    }

    hash_size = hash->block_size * hash->block_num;
    key = (*hash->hash_key)(data);
    index = key % hash_size;

    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;

    if (!hash->index[idx_1d])
    {
        return NULL;
    }

    for (bucket = hash->index[idx_1d][idx_2d]; bucket != NULL; bucket = bucket->next)
    {
        if ((*hash->hash_cmp)(bucket->data, data) == TRUE)
        {
            return bucket->data;
        }
    }

    return NULL;
}

/*lookup + get hash_index*/
void*
ctc_hash_lookup2(ctc_hash_t* hash, void* data, uint32* hash_index)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    if (!hash || !hash_index)
    {
        return NULL;
    }

    hash_size = hash->block_size * hash->block_num;
    key = (*hash->hash_key)(data);

    index = key % hash_size;
    *hash_index = index;
    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;

    if (!hash->index[idx_1d])
    {
        return NULL;
    }

    for (bucket = hash->index[idx_1d][idx_2d]; bucket != NULL; bucket = bucket_next)
    {
        bucket_next = bucket->next;
        if ((*hash->hash_cmp)(bucket->data, data) == TRUE)
        {
            return bucket->data;
        }
    }

    return NULL;
}

void*
ctc_hash_lookup3(ctc_hash_t* hash,  uint32 *hash_index, uint32 *node_cnt)
{
  
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;


   *node_cnt  = 0;
    idx_1d  = *hash_index / hash->block_size;
    idx_2d  = *hash_index % hash->block_size;


    for (; idx_1d < hash->block_num; idx_1d++)
    {
        if (!hash->index[idx_1d])
        {
            idx_2d = 0;
            continue;
        }   

        for (; idx_2d < hash->block_size; idx_2d++)
        {
            if (! hash->index[idx_1d][idx_2d])
            {
                continue;
            }

            for (bucket = hash->index[idx_1d][idx_2d]; bucket != NULL; bucket = bucket->next)
            {
                (*node_cnt) ++;
            }

            *hash_index = idx_1d * hash->block_size + idx_2d;
            return hash->index[idx_1d][idx_2d]->data;
        }
        idx_2d = 0;
    }

    return NULL;


}
/*traverse all node*/
int32
ctc_hash_traverse(ctc_hash_t* hash, hash_traversal_fn fn, void* data)
{
    uint32 count = 0;
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    int32 ret = 0;

    if (!hash)
    {
        return -1;
    }

    if (hash->count == 0)
    {
        return 0;
    }

    for (index1 = 0; index1 < hash->block_num; index1++)
    {
        if (!hash->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < hash->block_size; index2++)
        {
            for (bucket = hash->index[index1][index2]; bucket != NULL; bucket = bucket_next)
            {
                count++;
                bucket_next = bucket->next;
                if ((ret = (* fn)(bucket->data, data)) < 0)
                {
                    return ret;
                }

                if (count >= hash->count)
                {
                    return 0;
                }
            }
        }
    }

    return 0;

}

/* Traverse bucket. Note! struct of data must consist of bucket_data*/
int32
ctc_hash_traverse2(ctc_hash_t* hash, hash_traversal_fn fn, void* data)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    int32 ret = 0;

    if (!hash)
    {
        return -1;
    }

    hash_size = hash->block_size * hash->block_num;
    key = (*hash->hash_key)(data);
    index = key % hash_size;

    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;
    if (!hash->index[idx_1d])
    {
        return -1;
    }

    for (bucket = hash->index[idx_1d][idx_2d];
         bucket; bucket = bucket_next)
    {
        bucket_next = bucket->next;
        if ((ret = (* fn)(bucket->data, data)) < 0)
        {
            return ret;
        }
    }

    return 0;
}

/* traverse all through, without check count
 * fn's return value < 0,traverse stop
 */

/*traverse hash node from start_node_id and begin from start_list_node as current linklist node*/
int32
ctc_hash_traverse3(ctc_hash_t* hash, hash_traversal_3_fn fn, void* data, uint32 node_id, uint32 sub_list_id)
{
    uint32 count = 0;
    uint32 current_list_node_id = 0;
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;
    int32 ret = 0;

    if (!hash)
    {
        return -1;
    }
    if (hash->count == 0)
    {
        return 0;
    }

    for (index1 = node_id / hash->block_size; index1 < hash->block_num; index1++)
    {
        if (!hash->index[index1])
        {
            continue;
        }
        for (index2 = 0; index2 < hash->block_size; index2++)
        {
            if ( index1*hash->block_size + index2 < node_id)
            {
                continue;
            }
            current_list_node_id = 0;
            for (bucket = hash->index[index1][index2]; bucket != NULL; bucket = bucket_next)
            {
                count++;
                bucket_next = bucket->next;
                if ((current_list_node_id >= sub_list_id) && ((ret = (* fn)(bucket, index1*hash->block_size + index2,  current_list_node_id, data)) < 0)) 
                {
                    return ret;
                }

                if (count >= hash->count)
                {
                    return 0;
                }
                current_list_node_id ++;
            }
            sub_list_id = 0;            
        }
    }

    return 0;

}

int32
ctc_hash_traverse_through(ctc_hash_t* hash, hash_traversal_fn fn, void* data)
{
    uint32 count = 0;
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    int32 ret = 0;

    if (!hash)
    {
        return -1;
    }

    if (hash->count == 0)
    {
        return 0;
    }

    count = hash->count;

    for (index1 = 0; index1 < hash->block_num; index1++)
    {
        if (!hash->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < hash->block_size; index2++)
        {

            for (bucket = hash->index[index1][index2]; bucket != NULL; bucket = bucket_next)
            {
                count--;
                bucket_next = bucket->next;
                if ((ret = (* fn)(bucket->data, data)) < 0)
                {
                    return ret;
                }

                if (count == 0)
                {
                    return 0;
                }
            }
        }
    }

    return 0;

}

void*
ctc_hash_insert(ctc_hash_t* hash, void* data)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;

    if (!hash)
    {
        return NULL;
    }

    key = (*hash->hash_key)(data);
    hash_size = hash->block_size * hash->block_num;
    index = key % hash_size;
    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;

    if (!hash->index[idx_1d])
    {
        hash->index[idx_1d] = (void*)mem_malloc(MEM_HASH_MODULE,
                                                hash->block_size * sizeof(void*));

        if (!hash->index[idx_1d])
        {
            return NULL;
        }

        sal_memset(hash->index[idx_1d], 0,  hash->block_size * sizeof(void*));
    }

    bucket = mem_malloc(MEM_HASH_MODULE, sizeof(ctc_hash_bucket_t));
    if (!bucket)
    {
        return NULL;
    }

    bucket->data = data;
    bucket->next = hash->index[idx_1d][idx_2d];

    hash->index[idx_1d][idx_2d] = bucket;

    hash->count++;

    return bucket->data;
}

int32
ctc_hash_get_count(ctc_hash_t* hash, uint32* count)
{
    if (!hash)
    {
        return 0;
    }

    *count = hash->count;
    return 0;
}

void*
ctc_hash_remove(ctc_hash_t* hash, void* data)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_prev = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;
    void*   user_data;

    hash_size = hash->block_size * hash->block_num;
    key = (*hash->hash_key)(data);
    index = key % hash_size;
    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;

    if (!hash->index[idx_1d])
    {
        return NULL;
    }

    for (bucket = hash->index[idx_1d][idx_2d];
         bucket; bucket_prev = bucket ? bucket : bucket_prev, bucket = bucket_next)
    {
        bucket_next = bucket->next;
        if ((*hash->hash_cmp)(bucket->data, data) == TRUE)
        {
            if (bucket == hash->index[idx_1d][idx_2d])
            { /*head node*/
                hash->index[idx_1d][idx_2d] = bucket_next;
            }
            else
            {
                bucket_prev->next = bucket_next;
            }

            user_data = bucket->data;
            mem_free(bucket);
            hash->count--;
            return user_data;
        }
    }

    return NULL;
}

void
ctc_hash_traverse_remove(ctc_hash_t* hash, hash_traversal_fn fn, void* data)
{
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_prev = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;
    int32 ret = 0;

    /*  the mearning of return value of fn:
        0  -- skip current linklist node & continue remove
        1  -- delete current linklist node & continue remove
        -1 -- terminate hash remove   */
    if (!hash)
    {
        return;
    }

    for (index1 = 0; index1 < hash->block_num; index1++)
    {
        if (!hash->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < hash->block_size; index2++)
        {
            for (bucket = hash->index[index1][index2];
                 bucket; bucket_prev = bucket ? bucket : bucket_prev, bucket = bucket_next)
            {
                bucket_next = bucket->next;
                ret = (* fn) (bucket->data, data);

                switch (ret)
                {
                case 1:
                    if (bucket == hash->index[index1][index2])
                    {
                        hash->index[index1][index2] = bucket_next;
                    }
                    else
                    {
                        if(NULL != bucket_prev)
                        {
                            bucket_prev->next = bucket_next;
                        }
                    }

                    mem_free(bucket);
                    hash->count--;

                    if (hash->count == 0)
                    {
                        return;
                    }

                    break;

                case 0:
                    break;

                case -1:
                    return;
                }
            }
        }
    }
}

/* Traverse hash special array according to KEY */
void
ctc_hash_traverse2_remove(ctc_hash_t* hash, hash_traversal_fn fn, void* data)
{
    uint32 key = 0;
    uint32 index = 0;
    uint32 hash_size = 0;
    uint16 idx_1d = 0;
    uint16 idx_2d = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    if (!hash)
    {
        return;
    }

    hash_size = hash->block_size * hash->block_num;
    key = (*hash->hash_key)(data);
    index = key % hash_size;

    idx_1d  = index / hash->block_size;
    idx_2d  = index % hash->block_size;
    if (!hash->index[idx_1d])
    {
        return;
    }

    for (bucket = hash->index[idx_1d][idx_2d];
         bucket; bucket = bucket_next)
    {
        bucket_next = bucket->next;
        (* fn) (bucket->data, data);
    }
}

void
ctc_hash_free(ctc_hash_t* hash)
{
    uint16 index1 = 0;
    uint16 index2 = 0;
    ctc_hash_bucket_t* bucket = NULL;
    ctc_hash_bucket_t* bucket_next = NULL;

    if (!hash)
    {
        return;
    }
    
    for (index1 = 0; index1 < hash->block_num; index1++)
    {
        if (!hash->index[index1])
        {
            continue;
        }

        for (index2 = 0; index2 < hash->block_size; index2++)
        {
            for (bucket = hash->index[index1][index2]; bucket; bucket = bucket_next)
            {
                bucket_next = bucket->next;
                mem_free(bucket);
            }
        }

        mem_free(hash->index[index1]);
    }

    mem_free(hash->index);
    mem_free(hash);
}

#define HASH_MIX(a, b, c)        \
    {                                         \
        a -= b; a -= c; a ^= (c >> 13);           \
        b -= c; b -= a; b ^= (a << 8);            \
        c -= a; c -= b; c ^= (b >> 13);           \
        a -= b; a -= c; a ^= (c >> 12);           \
        b -= c; b -= a; b ^= (a << 16);           \
        c -= a; c -= b; c ^= (b >> 5);            \
        a -= b; a -= c; a ^= (c >> 3);            \
        b -= c; b -= a; b ^= (a << 10);           \
        c -= a; c -= b; c ^= (b >> 15);           \
    }                                         \



uint32
ctc_hash_caculate(uint32 size, void* kk)
{
    uint32 len = 0;
    uint32 a = 0x9e3779b9;
    uint32 b = 0x9e3779b9;
    uint32 c = 0;
    uint8* k = (uint8*) kk;

    len = size;

    while (len >= 12)
    {
        a += (k[0] + ((uint32)k[1] << 8) + ((uint32)k[2] << 16) + ((uint32)k[3] << 24));
        b += (k[4] + ((uint32)k[5] << 8) + ((uint32)k[6] << 16) + ((uint32)k[7] << 24));
        c += (k[8] + ((uint32)k[9] << 8) + ((uint32)k[10] << 16) + ((uint32)k[11] << 24));
        HASH_MIX(a, b, c);
        k += 12;
        len -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    c += size;

    switch (len)            /* all the case statements fall through */
    {
    case 11:
        c += ((uint32)k[10] << 24);

    case 10:
        c += ((uint32)k[9] << 16);

    case 9:
        c += ((uint32)k[8] << 8);

    /* the first byte of c is reserved for the length */
    case 8:
        b += ((uint32)k[7] << 24);

    case 7:
        b += ((uint32)k[6] << 16);

    case 6:
        b += ((uint32)k[5] << 8);

    case 5:
        b += k[4];

    case 4:
        a += ((uint32)k[3] << 24);

    case 3:
        a += ((uint32)k[2] << 16);

    case 2:
        a += ((uint32)k[1] << 8);

    case 1:
        a += k[0];
        /* case 0: nothing left to add */
    }

    HASH_MIX(a, b, c);
    /*-------------------------------------------- report the result */

    return c;
}

