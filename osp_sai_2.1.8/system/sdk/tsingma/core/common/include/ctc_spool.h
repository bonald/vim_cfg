/****************************************************************************
 *file ctc_spool.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2012-04-23

 *version v2.0

 The file define  Share pool lib
 ****************************************************************************/

#ifndef _ctc_spool_H_
#define _ctc_spool_H_

#ifdef __cplusplus
extern "C" {
#endif



/*
ret >- -- add/remove success
ret =0 -- other success
ret <0 -- failed
*/
enum ctc_spool_error_code_e
{
    CTC_SPOOL_E_UPDATE_NONE = 3,
    CTC_SPOOL_E_OPERATE_REFCNT = 2,
    CTC_SPOOL_E_OPERATE_MEMORY = 1,
    CTC_SPOOL_E_NONE = 0,
    CTC_SPOOL_E_NOT_INIT = -100,
    CTC_SPOOL_E_NO_MEMORY,
    CTC_SPOOL_E_FULL,
    CTC_SPOOL_E_ENTRY_NOT_EXIST
};
typedef enum ctc_spool_error_code_e ctc_spool_error_code_t;

struct ctc_spool_node_s
{
    struct ctc_spool_node_s* next;
    void*  data;
    uint32 ref_cnt;

};
typedef struct ctc_spool_node_s ctc_spool_node_t;

/*
spool_cmp : return TRUE means equal. vice versa.
*/
struct ctc_spool_s
{
    ctc_spool_node_t*** index;
    uint16 user_data_size;
    uint8 lchip;    /*[GB,GG]*/
    uint8 rsv0;
    uint16 block_num;
    uint16 block_size;
    uint32 count;
    uint32 max_count;
    uint32 (* spool_key) (void* data);
    bool   (* spool_cmp) (void* node_data, void* data);
    int32 (*spool_alloc) (void* node_data, void* user_data);
    int32 (*spool_free) (void* node_data, void* user_data);
};
typedef struct ctc_spool_s ctc_spool_t;

typedef int32 (* spool_traversal_fn)(void* node, void* user_data);

typedef int32 (* spool_alloc_fn) (void* node_data, void* user_data);
typedef int32 (* spool_free_fn) (void* node_data, void* user_data);

extern bool
ctc_spool_is_static(ctc_spool_t* spool, void* data);

extern ctc_spool_t*
ctc_spool_create(ctc_spool_t* spool);

extern int32
ctc_spool_static_add(ctc_spool_t* spool, void* data);

extern int32
ctc_spool_add(ctc_spool_t* spool, void* new_data, void* old_data, void* node_out);

extern int32
ctc_spool_traverse(ctc_spool_t* spool, spool_traversal_fn fn, void* data);

extern void*
ctc_spool_lookup(ctc_spool_t* spool,  void* p_db);

extern int32
ctc_spool_remove(ctc_spool_t* spool, void* data, void* data_out);

extern int32
ctc_spool_get_refcnt(ctc_spool_t* spool, void* data);

extern void
ctc_spool_free(ctc_spool_t* spool);

#ifdef __cplusplus
}
#endif

#endif

