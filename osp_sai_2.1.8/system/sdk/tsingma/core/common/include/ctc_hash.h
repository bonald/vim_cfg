/****************************************************************************
 *file ctc_hash.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2009-11-26

 *version v2.0

 The file define  HASH arithmetic lib
 ****************************************************************************/

#ifndef _CTC_HASH_H_
#define _CTC_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

struct ctc_hash_bucket_s
{
    struct ctc_hash_bucket_s* next;

    void* data;
};
typedef struct ctc_hash_bucket_s ctc_hash_bucket_t;

struct ctc_hash_s
{
    ctc_hash_bucket_t*** index;
    uint16 block_num;
    uint16 block_size;
    uint32 count;
    uint32 (* hash_key) (void* data);
    bool (* hash_cmp) (void* bucket_data, void* data);

};
typedef struct ctc_hash_s ctc_hash_t;

typedef int32 (* hash_traversal_fn)(void* bucket_data, void* user_data);
typedef int32 (* hash_traversal_3_fn)(void* bucket_data, uint32 node_id, uint32 sub_list_id, void* user_data);
typedef uint32 (* hash_key_fn) (void* data);
typedef bool (* hash_cmp_fn) (void* data1, void* data2);

extern ctc_hash_t*
ctc_hash_create(uint16 block_num, uint16 block_size, hash_key_fn hash_key, hash_cmp_fn hash_cmp);

extern void*
ctc_hash_lookup(ctc_hash_t* hash, void* data);

extern void*
ctc_hash_lookup2(ctc_hash_t* hash, void* data, uint32* hash_key);

extern void*
ctc_hash_lookup3(ctc_hash_t* hash,  uint32 *hash_index, uint32 *node_cnt);

extern int32
ctc_hash_traverse(ctc_hash_t* hash, hash_traversal_fn fn, void* data);

extern int32
ctc_hash_traverse2(ctc_hash_t* hash, hash_traversal_fn fn, void* data);

extern int32
ctc_hash_traverse3(ctc_hash_t* hash, hash_traversal_3_fn fn, void* data, uint32 start_hash_node, uint32 start_list_node);

extern int32
ctc_hash_traverse_through(ctc_hash_t* hash, hash_traversal_fn fn, void* data);

extern void*
ctc_hash_insert(ctc_hash_t* hash, void* data);

extern void*
ctc_hash_remove(ctc_hash_t* hash, void* data);

extern void
ctc_hash_traverse_remove(ctc_hash_t* hash, hash_traversal_fn fn, void* data);

extern void
ctc_hash_traverse2_remove(ctc_hash_t* hash, hash_traversal_fn fn, void* data);

extern void
ctc_hash_free(ctc_hash_t* hash);

extern uint32
ctc_hash_caculate(uint32 size, void* k);

#ifdef __cplusplus
}
#endif

#endif

