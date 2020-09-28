/**
 @file sys_usw_ofb.c

 @date 2017-8-15

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "sys_usw_ofb.h"
#include "sys_usw_opf.h"
#include "sys_usw_wb_common.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_OFB_MAX_BLOCK_NUM 256
#define SYS_OFB_MAX_OFFSET 0xFFFFF
#define SYS_OFB_INVALID_BLOCK 0xFFFF

#define START_OFB_TYPE_NUM     1
#define MAX_OFB_TYPE_NUM     256

#define SYS_OFB_BLOCK_VEC_BLOCK_NUM  8
#define SYS_OFB_OFFSET_VEC_BLOCK_NUM  32

#define SYS_OFB_MAX_MULTIPLE     4
#define SYS_OFB_MID_MULTIPLE     2
#define SYS_OFB_MIN_MULTIPLE     1

#define SYS_OFB_DBG_OUT(level, FMT, ...)                          \
    do                                                     \
    {                                                      \
        CTC_DEBUG_OUT(opf, opf, OPF_SYS, level, FMT, ##__VA_ARGS__);  \
    } while (0)

#define SYS_OFB_DBG_BLOCK(ofb_type, block_id)  \
 do                                                     \
    {                                                      \
        sys_usw_ofb_block_t* block_temp = SYS_OFB_BLOCK_INFO(ofb_type, block_id);\
        SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block id = %d, all_left_b = %d, all_right_b = %d, all_of_num = %d, used_of_num = %d\n", \
        block_id, block_temp->all_left_b, block_temp->all_right_b, block_temp->all_of_num, block_temp->used_of_num);\
    } while (0)


#define SYS_OFB_LOCK \
    do                                       \
    {                                          \
        if(p_usw_ofb_master[lchip]->p_mutex != NULL)\
        {                                            \
            sal_mutex_lock(p_usw_ofb_master[lchip]->p_mutex);  \
        }                                                 \
    }while(0)

#define SYS_OFB_UNLOCK \
    do               \
    {                                         \
        if(p_usw_ofb_master[lchip]->p_mutex != NULL)\
        {                                          \
            sal_mutex_unlock(p_usw_ofb_master[lchip]->p_mutex); \
        }                                                   \
    }while(0)

#define SYS_OFB_TYPE_CHECK(ofb_type)\
    do               \
    {                                         \
        if ((NULL == p_usw_ofb_master[lchip])\
        ||(NULL == p_usw_ofb_master[lchip]->ofb_info[ofb_type]))\
        {\
            return CTC_E_NOT_INIT;\
        }\
    }while(0)

#define SYS_OFB_BLOCK_CHECK(ofb_type, block_id)\
    do               \
    {                                         \
        if (block_id >= p_usw_ofb_master[lchip]->ofb_info[ofb_type]->block_num)\
        {\
            return CTC_E_INVALID_PARAM;\
        }                                            \
    }while(0)

#define SYS_OFB_BLOCK_INFO(ofb_type, block_id)\
    p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info[block_id]

#define SYS_OFB_SET_GROW(block, size)\
    p_usw_ofb_master[lchip]->preprocess[block].grow_valid = 1;\
    p_usw_ofb_master[lchip]->preprocess[block].grow_size = size;\

#define SYS_OFB_SET_SHRINK(block, time, size)\
    p_usw_ofb_master[lchip]->preprocess[block].shrink_valid = 1;\
    p_usw_ofb_master[lchip]->preprocess[block].shrink_time = time;\
    p_usw_ofb_master[lchip]->preprocess[block].shrink_size = size;

enum sys_usw_ofb_block_dir_e
{
    SYS_OFB_BLOCK_UP = 0,
    SYS_OFB_BLOCK_DOWN = 1,
    SYS_OFB_BLOCK_MAX
};
typedef enum sys_usw_ofb_block_dir_e sys_usw_ofb_block_dir_t;

enum sys_usw_ofb_move_type_e
{
    SYS_OFB_MOVE_OVER = 0,
    SYS_OFB_MOVE_FROM,
    SYS_OFB_MOVE_TO,
    SYS_OFB_MOVE_INSIDE
};
typedef enum sys_usw_ofb_move_type_e sys_usw_ofb_move_type_t;

struct sys_usw_ofb_block_s
{
    uint32      all_left_b; /* left boundary */
    uint32      all_right_b;    /* right boundary */
    uint32      all_of_num;   /*the number of all offset */
    uint32      used_of_num; /*the number of all used offset */
    uint8       multiple;
    uint8       adj_dir; /*for empty_grow*/
    uint8       have_limit;
    int32       (* ofb_cb)(uint8 lchip, uint32 new_offset, uint32 old_offset, void* user_data);

};
typedef struct sys_usw_ofb_block_s sys_usw_ofb_block_t;

struct sys_usw_ofb_info_s
{
    sys_usw_ofb_block_t**  pp_block_info; /*array to store block info*/
    ctc_vector_t*  offset_info;  /*store user data pointer*/
    uint32         block_num;
    uint32         max_size;/*offset size when sys_usw_ofb_init*/
    uint32         real_size;/*offset size after sys_usw_ofb_init_offset*/
    uint8          opf_type ;
    uint16         boundary_block;
    uint32         limit_boundary;
    uint32         max_multiple;
};
typedef struct sys_usw_ofb_info_s sys_usw_ofb_info_t;


struct sys_usw_ofb_preprocess_s
{
    uint8          shrink_valid;
    uint8          shrink_time ;
    uint8          shrink_size;

    uint8          grow_valid;
    uint8          grow_size;
};
typedef struct sys_usw_ofb_preprocess_s sys_usw_ofb_preprocess_t;

struct sys_usw_ofb_master_s
{
    sys_usw_ofb_info_t* ofb_info[MAX_OFB_TYPE_NUM];
    sal_mutex_t* p_mutex;
    uint32  type_bmp[8];
    sys_usw_ofb_preprocess_t* preprocess;
};
typedef struct sys_usw_ofb_master_s sys_usw_ofb_master_t;

static sys_usw_ofb_master_t* p_usw_ofb_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_usw_ofb_key_update_boundary_block(uint8 lchip, uint8 ofb_type, uint16 block_id)
{

    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];

    if (block->all_of_num
        && (block->all_left_b < ofb_info->limit_boundary)
    && (block->all_right_b >= ofb_info->limit_boundary))
    {
        ofb_info->boundary_block = block_id;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_do_alloc_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 grow_dir, uint32* p_offset, void* user_data)
{
    sys_usw_opf_t opf;
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint32 alloc_size = block->multiple;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    opf.multiple = alloc_size;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, alloc_size, p_offset));

    if ((ofb_info->limit_boundary)
        && (block->multiple != ofb_info->max_multiple)/* not max multiplex*/
        && (block->all_left_b < ofb_info->limit_boundary)/* min boundary < limit boundary */
        && (block->multiple != SYS_OFB_MIN_MULTIPLE))/* not min multiple*/
    {
        if (*p_offset < ofb_info->limit_boundary)
        {
            CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, alloc_size, *p_offset));
            opf.multiple = ofb_info->max_multiple;
            alloc_size = ofb_info->max_multiple;
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, alloc_size, p_offset));
        }
    }

    /* adjust block info */
    CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    CTC_ERROR_RETURN(ctc_vector_add(ofb_info->offset_info, *p_offset, user_data));
    block->used_of_num += alloc_size;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_do_free_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32 offset)
{
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    sys_usw_opf_t opf;
    uint8 free_size = block->multiple;

    CTC_NOT_EQUAL_CHECK(0, block->used_of_num);

    if ((offset < block->all_left_b) || (offset > block->all_right_b))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* 1. sanity check & init */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;

    if ((ofb_info->limit_boundary)
        && (block->multiple != ofb_info->max_multiple)/* not max multiplex*/
        && (block->all_left_b < ofb_info->limit_boundary)/* min boundary < limit boundary */
        && (offset < ofb_info->limit_boundary)
        && (block->multiple != SYS_OFB_MIN_MULTIPLE))/* not min multiple*/
    {
        free_size = ofb_info->max_multiple;
    }

    opf.multiple = free_size;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, free_size, offset));
    /* adjust block info */
    block->used_of_num -= free_size;
    ctc_vector_del(ofb_info->offset_info, offset);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_block_has_avil_offset(uint8 lchip, sys_usw_ofb_block_t* block, bool* p_has)
{
    /* 1. sanity check & init */
    CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    /* 2. do it */
    if (block->all_of_num - block->used_of_num < block->multiple)
    {
        *p_has = FALSE;
    }
    else
    {
        *p_has = TRUE;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ofb_key_get_valid_block(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint16* valid_block_id)
{
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    int32 i = 0;
    if (dir == SYS_OFB_BLOCK_DOWN)
    {
        for (i = block_id + 1; i < ofb_info->block_num; i++)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if (block->all_of_num)
            {
                *valid_block_id = i;
                return CTC_E_NONE;
            }
        }
    }
    else
    {
        for (i = block_id - 1; i >= 0; i--)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if (block->all_of_num)
            {
                *valid_block_id = i;
                return CTC_E_NONE;
            }
        }
    }
    return CTC_E_NO_RESOURCE;
}

STATIC uint32
_sys_usw_ofb_key_get_limit_used(uint8 lchip, uint8 ofb_type)
{
    int32 i = 0;
    uint32 used = 0;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];

    for (i = 0; ofb_info->block_num; i++)
    {
        block = SYS_OFB_BLOCK_INFO(ofb_type, i);
        if (0 == block->have_limit)
        {
            break;
        }
        used += block->used_of_num;
    }
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "limit_used = %d\n", used);
    return used;
}

STATIC int32
_sys_usw_ofb_key_adj_block_id(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint16* adj_block_id, uint8* adj_size)
{
    int32 i = 0;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_block_t* cur_block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    sys_usw_ofb_block_t* boundary_block = SYS_OFB_BLOCK_INFO(ofb_type, ofb_info->boundary_block);
    uint16 valid_block_id = 0;/*near valid block*/
    sys_usw_ofb_block_t* valid_block = NULL;
    int32 ret = CTC_E_NONE;

    if (cur_block->have_limit &&
        (_sys_usw_ofb_key_get_limit_used(lchip, ofb_type) >= (ofb_info->limit_boundary + 1)))
    {
        goto NO_RESOURCE;
    }

    if (SYS_OFB_BLOCK_UP == dir)
    {
        for (i = block_id - 1; i >= 0; i--)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
            if (0 == block->all_of_num)
            {
                continue;
            }
            if (block->all_of_num &&
                 (((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                ||(i< ofb_info->boundary_block)))
            {
                *adj_size = ofb_info->max_multiple;
            }
            else if (block->multiple > *adj_size)
            {
                *adj_size = block->multiple;
            }
            if (block->all_of_num - block->used_of_num >= *adj_size)
            {
                *adj_block_id = i;
                break;
            }
        }
        if (i < 0)
        {
            goto NO_RESOURCE;
        }
    }
    else
    {
        for (i = block_id + 1; i < ofb_info->block_num; i++)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
            if (0 == block->all_of_num)
            {
                continue;
            }
            if (ofb_info->limit_boundary && cur_block->have_limit)
            {
                if (cur_block->all_right_b == ofb_info->limit_boundary)
                {
                    goto NO_RESOURCE;
                }

                if (ofb_info->boundary_block == i)
                {
                    if ((SYS_OFB_MAX_MULTIPLE == block->multiple)&&(block->all_of_num == block->used_of_num))
                    {
                        goto NO_RESOURCE;
                    }

                    ret = _sys_usw_ofb_key_get_valid_block(lchip, ofb_type, block_id, dir, &valid_block_id);
                    if (CTC_E_NONE == ret)
                    {
                        valid_block = SYS_OFB_BLOCK_INFO(ofb_type, valid_block_id);
                        if ((valid_block->all_right_b == ofb_info->limit_boundary)&&(valid_block_id != i))
                        {
                            goto NO_RESOURCE;
                        }
                    }
                }
                else
                {
                    if ((block->used_of_num == block->all_of_num) /*full block*/
                        && (block->multiple == cur_block->multiple) /* max multiple block */
                    && (ofb_info->limit_boundary == block->all_right_b))/*max boundary reach the limit_boundary*/
                    {
                        goto NO_RESOURCE;
                    }
                }

                if ((cur_block->multiple == ofb_info->max_multiple) /* max multiple block */
                    && ((ofb_info->limit_boundary) == block->all_left_b))/*min boundary reach the limit_boundary*/
                {
                    goto NO_RESOURCE;
                }
                if ((block->multiple != cur_block->multiple)
                    && ((ofb_info->limit_boundary == block->all_right_b) ||
                         ((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary)))
                    && (block->all_left_b + cur_block->multiple - 1 > ofb_info->limit_boundary))
                {
                    goto NO_RESOURCE;
                }
            }
            if(block->all_of_num &&
                (((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                || (i< ofb_info->boundary_block)))
            {
                *adj_size = ofb_info->max_multiple;
            }
            else if (block->multiple > *adj_size)
            {
                *adj_size = block->multiple;
            }

            if(ofb_info->limit_boundary && (i >= ofb_info->boundary_block)
                && (block->all_of_num - block->used_of_num >= boundary_block->multiple)
                &&(SYS_OFB_MID_MULTIPLE == boundary_block->multiple))
            {
                *adj_block_id = i;
                break;
            }
            else if (block->all_of_num - block->used_of_num >= *adj_size)
            {
                *adj_block_id = i;
                break;
            }

        }
        if (i == ofb_info->block_num)
        {
            goto NO_RESOURCE;
        }
    }
    return CTC_E_NONE;
NO_RESOURCE:
    *adj_block_id = SYS_OFB_INVALID_BLOCK;
    return CTC_E_NO_RESOURCE;
}

STATIC int32
_sys_usw_ofb_key_do_block_shrink(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint8 adj_size)
{
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint32 old_offset = 0, new_offset = 0;
    sys_usw_opf_t opf;
    uint8 i = 0;
    uint8 adj_time = 0;
    int32 ret = CTC_E_NONE;
    void* user_data = NULL;
    uint8 alloc_size = adj_size;
    uint8 adj_used = 0;
    sys_usw_ofb_preprocess_t* preprocess = p_usw_ofb_master[lchip]->preprocess;

    /* 1. sanity check & init */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    opf.multiple = block->multiple;

    /*Empty!!!*/
    if (block->used_of_num == 0 && block->all_of_num == 0)
    {
        return CTC_E_NONE;
    }

    /*ONLY ONE!!!*/
    if (preprocess[block_id].shrink_valid)
    {
        adj_time = preprocess[block_id].shrink_time;
        alloc_size = preprocess[block_id].shrink_size;
    }
    if (block->used_of_num == 0 && block->all_of_num == alloc_size)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, alloc_size, &new_offset));
        (block->all_of_num) -= alloc_size;
        block->adj_dir = dir;

        /*block multiple is not 1*/
        if (SYS_OFB_BLOCK_UP == dir)
        {
            (block->all_left_b) += alloc_size - 1;
        }
        else
        {
            (block->all_right_b) -= alloc_size - 1;
        }
        return CTC_E_NONE;
    }


    alloc_size = block->multiple;
    if (adj_size == block->multiple)
    {
        adj_time = 1;
    }
    else
    {
        adj_time = adj_size / block->multiple;
    }

    if (preprocess[block_id].shrink_valid)
    {
        adj_time = preprocess[block_id].shrink_time;
        alloc_size = preprocess[block_id].shrink_size;
    }

    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %4d shrink, adj_time = %d, adj_size = %d, alloc_size = %d\n", block_id, adj_time, adj_size, alloc_size);

    opf.multiple = alloc_size;
    /* 2. do it */
    if (SYS_OFB_BLOCK_UP == dir)
    {
        for (i = 0; i < adj_time; i++)
        {
            /* 1) alloc one offset, which is supposed to be down side */
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, alloc_size, &new_offset));
            if ((ofb_info->limit_boundary)
                && ((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                && (block->multiple != SYS_OFB_MIN_MULTIPLE))
            {
                if (new_offset < ofb_info->limit_boundary)
                {
                    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, alloc_size, new_offset));
                    opf.multiple = ofb_info->max_multiple;
                    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, ofb_info->max_multiple, &new_offset));
                    alloc_size = ofb_info->max_multiple;
                }
                else
                {
                    adj_used = 1;
                }
            }
            /* 2) move the top item to the "new offset", causing the HARD & DB entry move as well */
            old_offset = block->all_left_b;
            user_data = ctc_vector_get(ofb_info->offset_info, old_offset);
            if ((block->ofb_cb))
            {
                ret = (block->ofb_cb)(lchip, new_offset, old_offset, user_data);
                if (ret)
                {
                    sys_usw_opf_free_offset(lchip, &opf, block->multiple, new_offset);
                    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ofb shrink error, ret=%d\n", ret);
                    return ret;
                }
            }
            ctc_vector_add(ofb_info->offset_info, new_offset, user_data);
            ctc_vector_del(ofb_info->offset_info, old_offset);
            /* 3) adjust the border property */
            if ((ofb_info->limit_boundary)
                && ((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                && (block->multiple != SYS_OFB_MIN_MULTIPLE))
            {
                if (old_offset < ofb_info->limit_boundary)
                {
                    alloc_size = ofb_info->max_multiple;
                }
            }
            CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_left_b);
            (block->all_left_b) += alloc_size;
            (block->all_of_num) -= alloc_size;

        }
    }
    else
    {
        for (i = 0; i < adj_time; i++)
        {
            /* 1) alloc one offset, which is supposed to be up side */
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_last(lchip, &opf, alloc_size, &new_offset));
            if ((ofb_info->limit_boundary)
                && ((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                && (block->multiple != SYS_OFB_MIN_MULTIPLE))
             {
                if (new_offset < ofb_info->limit_boundary)
                {
                    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, alloc_size, new_offset));
                    opf.multiple = ofb_info->max_multiple;
                    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_last(lchip, &opf, ofb_info->max_multiple, &new_offset));
                    alloc_size = ofb_info->max_multiple;
                    adj_used = 1;
                }
            }
            /* 2) move the top item to the "new offset", causing the HARD & DB entry move as well */
            if ((block->all_right_b <= ofb_info->limit_boundary) && (block->multiple == SYS_OFB_MID_MULTIPLE))
            {
                old_offset = block->all_right_b - ofb_info->max_multiple +  1;
            }
            else
            {
                old_offset = block->all_right_b - block->multiple +  1;
            }
            user_data = ctc_vector_get(ofb_info->offset_info, old_offset);
            if ((block->ofb_cb))
            {
                ret = (block->ofb_cb)(lchip, new_offset, old_offset, user_data);
                if (ret)
                {
                    sys_usw_opf_free_offset(lchip, &opf, block->multiple, new_offset);
                    return ret;
                }
            }
            ctc_vector_add(ofb_info->offset_info, new_offset, user_data);
            ctc_vector_del(ofb_info->offset_info, old_offset);
            /* 3) adjust the border property */
            if ((ofb_info->limit_boundary)
                && ((block->all_left_b < ofb_info->limit_boundary) && (block->all_right_b > ofb_info->limit_boundary))
                && (block->multiple != SYS_OFB_MIN_MULTIPLE))
            {
                if (old_offset > ofb_info->limit_boundary)
                {
                    alloc_size = block->multiple;
                }
            }

            CTC_NOT_EQUAL_CHECK(0, block->all_right_b);
            (block->all_right_b) -= alloc_size;
            (block->all_of_num) -= alloc_size;

        }
    }

    /*4.adjust block info*/
    //CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    if((adj_used)&&(user_data))
    {
        if(SYS_OFB_BLOCK_UP == dir)
        {
            block->used_of_num -= SYS_OFB_MID_MULTIPLE;
        }
        else
        {
            block->used_of_num += SYS_OFB_MID_MULTIPLE;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_do_block_grow_with_dir(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint8 adj_size)
{
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint32 desired_offset = 0;
    uint8 real_adj_size = adj_size;
    sys_usw_opf_t opf;
    sys_usw_ofb_preprocess_t* preprocess = p_usw_ofb_master[lchip]->preprocess;

    if (preprocess[block_id].grow_valid)
    {
        real_adj_size = preprocess[block_id].grow_size;
    }

    /* 1. sanity check & init */
    /*Empty!!!!!!!*/
    if (block->used_of_num == 0 && block->all_of_num == 0)
    {
        if (SYS_OFB_BLOCK_UP == dir)
        {
            if (block->all_left_b < real_adj_size)
            {
                block->all_left_b = 0;
                block->all_right_b = block->all_left_b + real_adj_size - 1;
            }
            else
            {
                (block->all_left_b) -= real_adj_size;
                (block->all_right_b) -= real_adj_size;
            }
        }
        else
        {
            if (block->all_right_b + real_adj_size >= ofb_info->max_size)
            {
                block->all_left_b = ofb_info->max_size - real_adj_size;
                block->all_right_b = ofb_info->max_size - 1;
            }
            else
            {
                (block->all_left_b) += real_adj_size;
                (block->all_right_b) += real_adj_size;
            }
        }
        return CTC_E_NONE;
    }

    /* 2. do it */
    if (SYS_OFB_BLOCK_UP == dir)
    {
        CTC_NOT_EQUAL_CHECK(0, block->all_left_b);
        (block->all_left_b) -= real_adj_size;
        desired_offset = block->all_left_b;
    }
    else
    {
        CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_right_b);
        desired_offset = block->all_right_b + 1;
        (block->all_right_b) += real_adj_size;
    }
    CTC_MAX_VALUE_CHECK(block->all_left_b, block->all_right_b);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, real_adj_size, desired_offset));

    CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_of_num);
    (block->all_of_num) += real_adj_size;


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_do_block_empty_grow(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint16 adj_block_id, uint8 adj_size)
{
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint32 desired_offset = 0;
    sys_usw_opf_t opf;
    uint16 valid_block_id = 0;/*near valid block*/
    sys_usw_ofb_block_t* valid_block = NULL;
    uint8 real_adj_size = adj_size;
    uint8 adj_dir = dir;
    int32 ret = CTC_E_NONE;
    sys_usw_ofb_preprocess_t* preprocess = p_usw_ofb_master[lchip]->preprocess;

    ret = _sys_usw_ofb_key_get_valid_block(lchip, ofb_type, block_id, adj_dir, &valid_block_id);
    if (CTC_E_NONE != ret)
    {
        adj_dir = (SYS_OFB_BLOCK_UP == (dir) ? SYS_OFB_BLOCK_DOWN : SYS_OFB_BLOCK_UP);
        _sys_usw_ofb_key_get_valid_block(lchip, ofb_type, block_id, adj_dir, &valid_block_id);
    }
    valid_block = SYS_OFB_BLOCK_INFO(ofb_type, valid_block_id);

    if (SYS_OFB_BLOCK_UP == dir)
    {
        if ((ofb_info->limit_boundary) && (block_id < ofb_info->boundary_block))
        {
            real_adj_size = ofb_info->max_multiple;
        }
        /*else if (ofb_info->limit_boundary)
        {
            real_adj_size = valid_block->multiple;
        }*/
    }
    else
    {
        if ((ofb_info->limit_boundary) && (valid_block_id < ofb_info->boundary_block))
        {
            real_adj_size = ofb_info->max_multiple;
        }
        /*else if (ofb_info->limit_boundary)
        {
            real_adj_size = valid_block->multiple;
        }*/
    }
    if (preprocess[block_id].grow_valid)
    {
        real_adj_size = preprocess[block_id].grow_size;
    }

    if (SYS_OFB_BLOCK_UP == adj_dir)
    {
        block->all_left_b = valid_block->all_right_b + 1;
        block->all_right_b = block->all_left_b + real_adj_size - 1;
        desired_offset = block->all_left_b;
    }
    else
    {
        block->all_right_b = valid_block->all_left_b - 1;
        block->all_left_b = block->all_right_b - real_adj_size + 1;
        desired_offset = block->all_left_b;
    }
    CTC_MAX_VALUE_CHECK(block->all_left_b, block->all_right_b);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, real_adj_size, desired_offset));

    CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_of_num);
    (block->all_of_num) += real_adj_size;
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_ofb_key_block_grow_with_dir(uint8 lchip, uint8 ofb_type, uint16 block_id, uint8 dir, uint16 adj_block_id, uint8 adj_size)
{
    uint8 adj_dir;
    int32 i = 0;
    sys_usw_ofb_block_t* block = NULL;

    adj_dir = (SYS_OFB_BLOCK_UP == (dir) ? SYS_OFB_BLOCK_DOWN : SYS_OFB_BLOCK_UP);
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "adj_dir = %d, adj_block_id = %d, adj_size = %d\n", adj_dir, adj_block_id, adj_size);
    if (dir == SYS_OFB_BLOCK_DOWN)
    {
        for (i = adj_block_id; i > block_id; i--)
        {
            CTC_ERROR_RETURN(_sys_usw_ofb_key_do_block_shrink(lchip, ofb_type, i, adj_dir, adj_size));
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %4d shrink, ", i);
            SYS_OFB_DBG_BLOCK(ofb_type, i);
            CTC_ERROR_RETURN(_sys_usw_ofb_key_do_block_grow_with_dir(lchip, ofb_type, i - 1, dir, adj_size));
            _sys_usw_ofb_key_update_boundary_block(lchip, ofb_type, i - 1);
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %4d grow,   ", i - 1);
            SYS_OFB_DBG_BLOCK(ofb_type, i - 1);
        }
    }
    else
    {
        for (i = adj_block_id; i < block_id; i++)
        {
            CTC_ERROR_RETURN(_sys_usw_ofb_key_do_block_shrink(lchip, ofb_type, i,adj_dir, adj_size));
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %4d shrink, ", i);
            SYS_OFB_DBG_BLOCK(ofb_type, i);
            CTC_ERROR_RETURN(_sys_usw_ofb_key_do_block_grow_with_dir(lchip, ofb_type, i+1, dir, adj_size));
            _sys_usw_ofb_key_update_boundary_block(lchip, ofb_type, i + 1);
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %4d grow,   ", i + 1);
            SYS_OFB_DBG_BLOCK(ofb_type, i + 1);
        }
    }
    block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    if (block->all_of_num == 0)
    {
        CTC_ERROR_RETURN(_sys_usw_ofb_key_do_block_empty_grow(lchip, ofb_type, block_id,  dir, adj_block_id, adj_size));
        _sys_usw_ofb_key_update_boundary_block(lchip, ofb_type, adj_block_id);
        SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block %d empty grow, ", block_id);
        SYS_OFB_DBG_BLOCK(ofb_type, block_id);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_key_get_grow_with_dir(uint8 lchip, uint8 ofb_type, uint16 block_id, uint16* adj_block_id, uint8* dir, uint8* adj_size)
{
    uint16 adj_block_id_down = 0;
    uint16 adj_block_id_up = 0;
    uint8 adj_size_down = 0;
    uint8 adj_size_up = 0;
    uint8 adj_dir = 0;
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);

    adj_size_down = block->multiple;
    adj_size_up = block->multiple;
    _sys_usw_ofb_key_adj_block_id(lchip, ofb_type, block_id, SYS_OFB_BLOCK_DOWN, &adj_block_id_down, &adj_size_down);
    _sys_usw_ofb_key_adj_block_id(lchip, ofb_type, block_id, SYS_OFB_BLOCK_UP, &adj_block_id_up, &adj_size_up);

    if ((SYS_OFB_INVALID_BLOCK == adj_block_id_down) && (SYS_OFB_INVALID_BLOCK == adj_block_id_up))
    {
        return CTC_E_NO_RESOURCE;
    }
    else if (SYS_OFB_INVALID_BLOCK == adj_block_id_down)
    {
        adj_dir= SYS_OFB_BLOCK_UP;
    }
    else if (SYS_OFB_INVALID_BLOCK == adj_block_id_up)
    {
        adj_dir= SYS_OFB_BLOCK_DOWN;
    }
    else if ((adj_block_id_down - block_id) >= (block_id - adj_block_id_up))
    {
         adj_dir= SYS_OFB_BLOCK_UP;
    }
    else
    {
        adj_dir= SYS_OFB_BLOCK_DOWN;
    }
    if (dir)
    {
        *dir = adj_dir;
    }
    if (adj_block_id)
    {
        *adj_block_id = (SYS_OFB_BLOCK_DOWN == adj_dir)? adj_block_id_down : adj_block_id_up;
    }
    if (adj_size)
    {
        *adj_size = (SYS_OFB_BLOCK_DOWN == adj_dir)? adj_size_down : adj_size_up;
    }
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "grow dir = %d\n", *dir);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ofb_preprocess(uint8 lchip, uint8 ofb_type, uint16 block_id, uint16 adj_block_id, uint8 grow_dir)
{
    int32 i = 0;
    uint8 is_on_boundary = 0;
    uint8 move_type = 0;/*sys_usw_ofb_move_type_t*/
    uint16 first_mid_blk = 0;
    uint16 first_min_blk = 0;
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    sys_usw_ofb_block_t* boundary_block = NULL;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_block_t* cur_block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);

    if (0 == ofb_info->limit_boundary)
    {
        return CTC_E_NONE;
    }

    boundary_block = SYS_OFB_BLOCK_INFO(ofb_type, ofb_info->boundary_block);
    is_on_boundary = (boundary_block->all_right_b == ofb_info->limit_boundary)? 1 : 0;

    if (SYS_OFB_BLOCK_UP == grow_dir)
    {
        if ((adj_block_id < ofb_info->boundary_block) && (block_id > ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_OVER;
        }
        else if ((adj_block_id == ofb_info->boundary_block) && (block_id > ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_FROM;
        }
        else if ((adj_block_id < ofb_info->boundary_block) && (block_id == ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_TO;
        }
        else if((adj_block_id < ofb_info->boundary_block) && (block_id < ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_INSIDE;
        }
        else
        {
            return CTC_E_NONE;
        }
    }
    else
    {
        if ((adj_block_id > ofb_info->boundary_block) && (block_id < ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_OVER;
        }
        else if ((adj_block_id == ofb_info->boundary_block) && (block_id < ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_FROM;
        }
        else if ((adj_block_id > ofb_info->boundary_block) && (block_id == ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_TO;
        }
        else if((adj_block_id < ofb_info->boundary_block) && (block_id < ofb_info->boundary_block))
        {
            move_type = SYS_OFB_MOVE_INSIDE;
        }
        else
        {
            return CTC_E_NONE;
        }
    }
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "preprocess->\n");
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "move_type = %d, is_on_boundary = %d\n", move_type, is_on_boundary);
    if (SYS_OFB_BLOCK_UP == grow_dir)
    {
        for (i = ofb_info->boundary_block; i <= block_id; i++)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if ((block->all_of_num|| (i == block_id)) && (SYS_OFB_MID_MULTIPLE == block->multiple) && (0 == first_mid_blk)
                &&(i != ofb_info->boundary_block))
            {
                first_mid_blk = i;
            }
            if ((block->all_of_num|| (i == block_id)) && (SYS_OFB_MIN_MULTIPLE == block->multiple) && (0 == first_min_blk)
                &&(i != ofb_info->boundary_block))
            {
                first_min_blk = i;
            }
        }
        SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "first_mid_blk = %d, first_min_blk = %d\n", first_mid_blk, first_min_blk);
        for (i = adj_block_id; i <= block_id; i++)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if ((0 == block->all_of_num) && (i != block_id))
            {
                continue;
            }

            if (SYS_OFB_MAX_MULTIPLE == boundary_block->multiple)
            {
                if (i <= ofb_info->boundary_block)
                {
                    continue;
                }
                if (SYS_OFB_MOVE_INSIDE == move_type)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                }
                else if ((first_mid_blk == i)
                    || ((0 == first_mid_blk) && (first_min_blk == i)))
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                }
                else if (first_mid_blk && (first_min_blk == i))
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MID_MULTIPLE);
                }
                else
                {
                    SYS_OFB_SET_GROW(i, block->multiple);
                    SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                }
            }
            else if (SYS_OFB_MID_MULTIPLE == boundary_block->multiple)
            {
                if (SYS_OFB_MOVE_INSIDE == move_type)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                }
                else if (i < ofb_info->boundary_block)
                {
                    if (SYS_OFB_MID_MULTIPLE == block->multiple)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                    }
                }
                else if (i == ofb_info->boundary_block)
                {
                    if ((SYS_OFB_MOVE_OVER == move_type) || (SYS_OFB_MOVE_TO == move_type))
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    }
                    if (is_on_boundary)
                    {
                        SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                    }
                }
                else if (is_on_boundary &&
                    ((first_mid_blk == i) || ((0 == first_mid_blk) && (first_min_blk == i))))
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                }
                else if (first_min_blk == i)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MID_MULTIPLE);
                }
                else
                {
                    SYS_OFB_SET_GROW(i, block->multiple);
                    SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                }
            }
            else if(SYS_OFB_MIN_MULTIPLE == boundary_block->multiple)
            {
                if (SYS_OFB_MOVE_INSIDE == move_type)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 4, SYS_OFB_MIN_MULTIPLE);
                }
                else if (i < ofb_info->boundary_block)
                {
                    if (SYS_OFB_MIN_MULTIPLE == block->multiple)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        SYS_OFB_SET_SHRINK(i, 4, SYS_OFB_MIN_MULTIPLE);
                    }
                    else if (SYS_OFB_MID_MULTIPLE == block->multiple)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                    }
                }
                else if (i == ofb_info->boundary_block)
                {
                    if ((SYS_OFB_MOVE_OVER == move_type)
                        ||(SYS_OFB_MOVE_TO == move_type))
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    }
                    if ((is_on_boundary) && (SYS_OFB_MOVE_TO != move_type))
                    {
                        SYS_OFB_SET_SHRINK(i, 4, SYS_OFB_MIN_MULTIPLE);
                    }
                }
                else if (is_on_boundary && (first_min_blk == i))
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                }
                else
                {
                    SYS_OFB_SET_GROW(i, block->multiple);
                    SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                }
            }
            if (0 == p_usw_ofb_master[lchip]->preprocess[i].grow_valid)
            {
                SYS_OFB_SET_GROW(i, block->multiple);
            }
            if (0 == p_usw_ofb_master[lchip]->preprocess[i].shrink_valid)
            {
                SYS_OFB_SET_SHRINK(i, 1, block->multiple);
            }
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block id = %d, g_valid = %d, g_size = %d, s_valid = %d, s_size = %d, s_time = %d\n",
                                          i,
                                          p_usw_ofb_master[lchip]->preprocess[i].grow_valid,
                                          p_usw_ofb_master[lchip]->preprocess[i].grow_size,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_valid,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_size,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_time);
        }
    }
    else
    {
        for (i = ofb_info->boundary_block; i <= adj_block_id; i++)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if (block->all_of_num && (SYS_OFB_MIN_MULTIPLE == block->multiple) && (0 == first_min_blk)
                &&(i != ofb_info->boundary_block))
            {
                first_min_blk = i;
                break;
            }
        }
        for (i = adj_block_id; i >= block_id; i--)
        {
            block = SYS_OFB_BLOCK_INFO(ofb_type, i);
            if (((0 == block->all_of_num)||(SYS_OFB_MAX_MULTIPLE == boundary_block->multiple))
                &&(i != block_id))
            {
                continue;
            }
            else if (SYS_OFB_MID_MULTIPLE == boundary_block->multiple)
            {
                if (SYS_OFB_MOVE_INSIDE == move_type)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                }
                else if ((SYS_OFB_MOVE_OVER == move_type) || (SYS_OFB_MOVE_TO == move_type))
                {
                    if (i > ofb_info->boundary_block)
                    {
                        if (SYS_OFB_MIN_MULTIPLE == block->multiple)
                        {
                            SYS_OFB_SET_SHRINK(i, 2, block->multiple);
                            SYS_OFB_SET_GROW(i, SYS_OFB_MID_MULTIPLE);
                        }
                    }
                    else if (i == ofb_info->boundary_block)
                    {
                        SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                    }
                    else if (i < ofb_info->boundary_block)
                    {
                        if (SYS_OFB_MID_MULTIPLE == block->multiple)
                        {
                            SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                            SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                        }
                    }
                    else
                    {
                        SYS_OFB_SET_GROW(i, block->multiple);
                        SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                    }
                }
                else if (SYS_OFB_MOVE_FROM == move_type)
                {
                    if (block->multiple == SYS_OFB_MID_MULTIPLE)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        if (i != ofb_info->boundary_block)
                        {
                            SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                        }
                        else
                        {
                            SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                        }
                    }
                    else
                    {
                        SYS_OFB_SET_GROW(i, block->multiple);
                        SYS_OFB_SET_SHRINK(i, 1, block->multiple);
                    }
                }
            }
            else if(SYS_OFB_MIN_MULTIPLE == boundary_block->multiple)
            {
                if (SYS_OFB_MOVE_INSIDE == move_type)
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 4, SYS_OFB_MIN_MULTIPLE);
                }
                else if ((SYS_OFB_MAX_MULTIPLE == cur_block->multiple)
                    ||(SYS_OFB_MID_MULTIPLE == cur_block->multiple))
                {
                    if (SYS_OFB_MID_MULTIPLE == block->multiple)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        SYS_OFB_SET_SHRINK(i, 1, SYS_OFB_MAX_MULTIPLE);
                    }
                    else if(SYS_OFB_MIN_MULTIPLE == block->multiple)
                    {
                        SYS_OFB_SET_GROW(i, SYS_OFB_MAX_MULTIPLE);
                        SYS_OFB_SET_SHRINK(i, 4, SYS_OFB_MIN_MULTIPLE);
                    }
                }
                else
                {
                    SYS_OFB_SET_GROW(i, SYS_OFB_MID_MULTIPLE);
                    SYS_OFB_SET_SHRINK(i, 2, SYS_OFB_MIN_MULTIPLE);
                }
            }

            if (0 == p_usw_ofb_master[lchip]->preprocess[i].grow_valid)
            {
                SYS_OFB_SET_GROW(i, block->multiple);
            }
            if (0 == p_usw_ofb_master[lchip]->preprocess[i].shrink_valid)
            {
                SYS_OFB_SET_SHRINK(i, 1, block->multiple);
            }
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block id = %d, g_valid = %d, g_size = %d, s_valid = %d, s_size = %d, s_time = %d\n",
                                          i,
                                          p_usw_ofb_master[lchip]->preprocess[i].grow_valid,
                                          p_usw_ofb_master[lchip]->preprocess[i].grow_size,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_valid,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_size,
                                          p_usw_ofb_master[lchip]->preprocess[i].shrink_time);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_ofb_init(uint8 lchip, uint16 block_num, uint32 size, uint8* ofb_type)
{
    uint32 index = 0;
    char str[64]   = {0};
    int32 ret = CTC_E_NONE;
    uint16 block_id = 0;
    sys_usw_ofb_block_t* block = NULL;

    if (size > SYS_OFB_MAX_OFFSET)
    {
        return  CTC_E_INVALID_PARAM;
    }

    if (NULL == p_usw_ofb_master[lchip])
    {
        p_usw_ofb_master[lchip] = (sys_usw_ofb_master_t*)mem_malloc(MEM_OPF_MODULE, sizeof(sys_usw_ofb_master_t));
        if (NULL == p_usw_ofb_master[lchip])
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_usw_ofb_master[lchip], 0, sizeof(sys_usw_ofb_master_t));

        sal_mutex_create(&p_usw_ofb_master[lchip]->p_mutex);
        if (NULL == p_usw_ofb_master[lchip]->p_mutex)
        {
            mem_free(p_usw_ofb_master[lchip]);
            return CTC_E_FAIL_CREATE_MUTEX;
        }
    }

    if (0 == *ofb_type)
    {
        for (index = START_OFB_TYPE_NUM; index < MAX_OFB_TYPE_NUM; index++)
        {
            if (!CTC_BMP_ISSET(p_usw_ofb_master[lchip]->type_bmp, index))
            {
                CTC_BMP_SET(p_usw_ofb_master[lchip]->type_bmp, index);
                break;
            }
        }
        if (index >= MAX_OFB_TYPE_NUM)
        {
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "invalid type index : %d\n", index);
            return CTC_E_NO_RESOURCE;
        }
        *ofb_type = index;
    }
    else
    {
        CTC_BMP_SET(p_usw_ofb_master[lchip]->type_bmp, *ofb_type);
    }

    if (NULL == p_usw_ofb_master[lchip]->ofb_info[*ofb_type])
    {
        p_usw_ofb_master[lchip]->ofb_info[*ofb_type] = (sys_usw_ofb_info_t*)mem_malloc(MEM_OPF_MODULE, sizeof(sys_usw_ofb_info_t));
        if (NULL == p_usw_ofb_master[lchip]->ofb_info[*ofb_type])
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_usw_ofb_master[lchip]->ofb_info[*ofb_type], 0, sizeof(sys_usw_ofb_info_t));

        p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info = (sys_usw_ofb_block_t**)mem_malloc(MEM_OPF_MODULE, sizeof(sys_usw_ofb_block_t*) * block_num);
        if (NULL == p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info)
        {
            ret = CTC_E_NO_MEMORY;
            goto Error;
        }
        sal_memset(p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info, 0, sizeof(sys_usw_ofb_block_t*) * block_num);

        for (block_id = 0; block_id < block_num; block_id++)
        {
            block = (sys_usw_ofb_block_t*)mem_malloc(MEM_OPF_MODULE, sizeof(sys_usw_ofb_block_t));
            if (NULL == block)
            {
                ret = CTC_E_NO_MEMORY;
                goto Error;
            }
            sal_memset(block, 0, sizeof(sys_usw_ofb_block_t));
            p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info[block_id] = block;
        }

        p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->offset_info =
                          ctc_vector_init(SYS_OFB_OFFSET_VEC_BLOCK_NUM, (size + SYS_OFB_OFFSET_VEC_BLOCK_NUM - 1) / SYS_OFB_OFFSET_VEC_BLOCK_NUM);
        if (NULL == p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->offset_info)
        {
            ret = CTC_E_NO_MEMORY;
            goto Error;
        }
        p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->block_num = block_num;
        p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->max_size = size;

        sal_sprintf(str, "opf-ofb-type%d", *ofb_type);
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &(p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->opf_type),
                                            block_num, str));

    }
    return CTC_E_NONE;

Error:
    for (block_id = 0; block_id < block_num; block_id++)
    {
        if (p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info[block_id])
        {
            mem_free(p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info[block_id]);
        }
    }
    if (p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info)
    {
        mem_free(p_usw_ofb_master[lchip]->ofb_info[*ofb_type]->pp_block_info);
    }
    if (p_usw_ofb_master[lchip]->ofb_info[*ofb_type])
    {
        mem_free(p_usw_ofb_master[lchip]->ofb_info[*ofb_type]);
    }

    return ret;
}

int32
sys_usw_ofb_deinit(uint8 lchip, uint8 ofb_type)
{
    uint8  idx = 0;
    int16  block_id = 0;
    if ((NULL == p_usw_ofb_master[lchip])
        ||(NULL == p_usw_ofb_master[lchip]->ofb_info[ofb_type]))
    {
        return CTC_E_NONE;
    }
    SYS_OFB_LOCK;
    ctc_vector_release(p_usw_ofb_master[lchip]->ofb_info[ofb_type]->offset_info);
    for (block_id = 0; block_id < p_usw_ofb_master[lchip]->ofb_info[ofb_type]->block_num; block_id++)
    {
        if (p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info[block_id])
        {
            mem_free(p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info[block_id]);
        }
        sys_usw_opf_free(lchip, p_usw_ofb_master[lchip]->ofb_info[ofb_type]->opf_type, block_id);
    }
    sys_usw_opf_deinit(lchip, p_usw_ofb_master[lchip]->ofb_info[ofb_type]->opf_type);

    mem_free(p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info);
    mem_free(p_usw_ofb_master[lchip]->ofb_info[ofb_type]);
    CTC_BMP_UNSET(p_usw_ofb_master[lchip]->type_bmp, ofb_type);
    SYS_OFB_UNLOCK;

    for (idx = 0; idx < 8; idx++)
    {
        if (p_usw_ofb_master[lchip]->type_bmp[idx])
        {
            return CTC_E_NONE;
        }
    }

    sal_mutex_destroy(p_usw_ofb_master[lchip]->p_mutex);
    mem_free(p_usw_ofb_master[lchip]);

    return CTC_E_NONE;
}


int32
sys_usw_ofb_init_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, sys_usw_ofb_param_t* p_ofb_param )
{
    sys_usw_ofb_info_t*  ofb_info = NULL;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_block_t* prev_block_info = NULL;
    uint32 offset = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_ofb_param);

    if ((NULL == p_usw_ofb_master[lchip])
        ||(NULL == p_usw_ofb_master[lchip]->ofb_info[ofb_type]))
    {
        return CTC_E_NOT_INIT;
    }

    if (0 == p_ofb_param->multiple)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_ofb_param->size % p_ofb_param->multiple)
    {
        return CTC_E_INVALID_PARAM;
    }

    ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];

    if (ofb_info->real_size % p_ofb_param->multiple)
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((block_id >= ofb_info->block_num)
        ||(ofb_info->real_size + p_ofb_param->size > ofb_info->max_size))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_ofb_param->max_limit_offset >= ofb_info->max_size)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (ofb_info->limit_boundary
        && p_ofb_param->max_limit_offset
        && (ofb_info->limit_boundary != p_ofb_param->max_limit_offset))
    {
        return CTC_E_INVALID_PARAM;
    }

    block = p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info[block_id];
    if (block->multiple)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (0 == block_id)/*first block*/
    {
        if (0 == p_ofb_param->size)
        {
            return CTC_E_INVALID_PARAM;
        }
        block->all_left_b = 0;
        block->all_right_b = (p_ofb_param->size)?  (p_ofb_param->size - 1) : 0;
    }
    else
    {
        prev_block_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type]->pp_block_info[block_id - 1];
        if (p_ofb_param->size)
        {
            block->all_left_b = prev_block_info->all_right_b + 1;
            block->all_right_b = block->all_left_b + p_ofb_param->size - 1;
        }
        else
        {
            block->all_left_b = prev_block_info->all_right_b - p_ofb_param->multiple + 1;
            block->all_right_b = prev_block_info->all_right_b;
        }
    }

    block->have_limit = p_ofb_param->max_limit_offset? 1 : 0;
    block->multiple = p_ofb_param->multiple;
    block->all_of_num = p_ofb_param->size;
    block->used_of_num = 0;
    block->ofb_cb = p_ofb_param->ofb_cb;

    if ((0 == ofb_info->limit_boundary) && (p_ofb_param->max_limit_offset != 0))
    {
        ofb_info->limit_boundary = p_ofb_param->max_limit_offset;
        ofb_info->max_multiple = block->multiple;
    }
    _sys_usw_ofb_key_update_boundary_block(lchip, ofb_type, block_id);

    sal_memset(&opf, 0 , sizeof(opf));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, ofb_info->max_size));
    if (p_ofb_param->size)
    {
        if (block->all_left_b)
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, block->all_left_b, &offset));
        }
        if (block->all_right_b + 1 < ofb_info->max_size)
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf,
                                                                      ofb_info->max_size - block->all_right_b - 1, block->all_right_b + 1));
        }
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, ofb_info->max_size, &offset));
    }

    ofb_info->real_size += p_ofb_param->size;

    return CTC_E_NONE;
}

int32
sys_usw_ofb_alloc_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32* p_offset, void* user_data)
{
    bool has = 0;
    int32 ret = CTC_E_NONE;
    sys_usw_ofb_block_t* block = NULL;
    uint8 grow_dir = SYS_OFB_BLOCK_MAX;
    uint16 adj_block_id = 0;
    uint8 adj_size = 1;
    sys_usw_ofb_info_t*  ofb_info = NULL;

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_offset);
    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_BLOCK_CHECK(ofb_type, block_id);

    /* 2. do it */
    SYS_OFB_LOCK;
    ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    p_usw_ofb_master[lchip]->preprocess = (sys_usw_ofb_preprocess_t*)mem_malloc(MEM_OPF_MODULE,
                                                                                sizeof(sys_usw_ofb_preprocess_t) * ofb_info->block_num);
    if (NULL == p_usw_ofb_master[lchip]->preprocess)
    {
        ret =  CTC_E_NO_MEMORY;
        goto End;
    }
    sal_memset(p_usw_ofb_master[lchip]->preprocess, 0, sizeof(sys_usw_ofb_preprocess_t) * ofb_info->block_num);

    block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    CTC_ERROR_GOTO(_sys_usw_ofb_key_block_has_avil_offset(lchip, block, &has), ret, End);
    if (!has)
    {
        CTC_ERROR_GOTO(_sys_usw_ofb_key_get_grow_with_dir(lchip, ofb_type, block_id, &adj_block_id, &grow_dir, &adj_size), ret, End);
        CTC_ERROR_GOTO(_sys_usw_ofb_preprocess(lchip, ofb_type, block_id, adj_block_id, grow_dir), ret, End);
        CTC_ERROR_GOTO(_sys_usw_ofb_key_block_grow_with_dir(lchip, ofb_type, block_id, grow_dir, adj_block_id, adj_size), ret, End);
    }
    CTC_ERROR_GOTO(_sys_usw_ofb_key_do_alloc_offset(lchip, ofb_type, block_id, grow_dir, p_offset, user_data), ret, End);
End:
    mem_free(p_usw_ofb_master[lchip]->preprocess);
    SYS_OFB_UNLOCK;
    return ret;
}

int32
sys_usw_ofb_free_offset(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32 offset)
{
    int32 ret = CTC_E_NONE;
    void* user_data = NULL;
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];

    /* 1. sanity check & init */
    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_BLOCK_CHECK(ofb_type, block_id);
    SYS_OFB_LOCK;
    /* 2. do it */
    user_data = ctc_vector_get(ofb_info->offset_info, offset);
    CTC_ERROR_GOTO(_sys_usw_ofb_key_do_free_offset(lchip, ofb_type, block_id, offset), ret, End);
    if (user_data)
    {
        mem_free(user_data);
    }
End:
    SYS_OFB_UNLOCK;
    return ret;
}

int32
sys_usw_ofb_traverse(uint8 lchip, uint8 ofb_type, uint16 min_block_id, uint16 max_block_id, OFB_TYAVERSE_FUN_P fn, void* data)
{
    int32 ret = CTC_E_NONE;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint16 block_id = 0;
    void* user_data;
    uint32 i = 0;

    /* 1. sanity check & init */
    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_BLOCK_CHECK(ofb_type, min_block_id);
    SYS_OFB_BLOCK_CHECK(ofb_type, max_block_id);
    if (NULL == fn)
    {
        return CTC_E_INVALID_PTR;
    }

    SYS_OFB_LOCK;
    for (block_id = min_block_id; block_id <= max_block_id; block_id++)
    {
        block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
        if (0 == block->used_of_num)
        {
            continue;
        }
        for (i = block->all_left_b; i <= block->all_right_b; i++)
        {
            user_data = ctc_vector_get(ofb_info->offset_info, i);
            if (NULL == user_data)
            {
                continue;
            }
            ret = (*fn)(lchip, user_data, data);
            if (ret)
            {
                goto End;
            }
        }
    }
End:
    SYS_OFB_UNLOCK;
    return CTC_E_NONE;
}


int32
sys_usw_ofb_check_resource(uint8 lchip, uint8 ofb_type, uint16 block_id)
{
    bool has = 0;
    sys_usw_ofb_block_t* block = NULL;
    int32 ret = CTC_E_NONE;
    /* 1. sanity check & init */

    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_BLOCK_CHECK(ofb_type, block_id);

    /* 2. do it */
    SYS_OFB_LOCK;
    block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    CTC_ERROR_GOTO(_sys_usw_ofb_key_block_has_avil_offset(lchip, block, &has),ret, End);
    if (!has)
    {
        CTC_ERROR_GOTO(_sys_usw_ofb_key_get_grow_with_dir(lchip, ofb_type, block_id, NULL, NULL, NULL), ret, End);
    }
End:
    SYS_OFB_UNLOCK;
    return ret;
}

int32
sys_usw_ofb_show_status(uint8 lchip)
{
    uint16 ofb_type = 0;
    sys_usw_ofb_info_t*  ofb_info = NULL;
    if (NULL == p_usw_ofb_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }
    SYS_OFB_LOCK;
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s %-10s %-10s %-10s %-10s %-10s %-7s %-10s\n", "OFB_TYPE", "BLOCK_NUM", "MAX_SIZE", "REAL_SIZE", "LIMIT_BD", "LIMIT_BL","MAX_M", "OPF_TYPE");
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "==================================================================================\n");
    for (ofb_type = START_OFB_TYPE_NUM; ofb_type < MAX_OFB_TYPE_NUM; ofb_type++)
    {
        if (CTC_BMP_ISSET(p_usw_ofb_master[lchip]->type_bmp, ofb_type))
        {
            ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10d %-10d %-10d %-10d %-10d %-10d %-7d %-10d\n", ofb_type,
                ofb_info->block_num,
                ofb_info->max_size,
                ofb_info->real_size,
                ofb_info->limit_boundary,
                ofb_info->boundary_block,
                ofb_info->max_multiple,
                ofb_info->opf_type);
        }
    }

    SYS_OFB_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_ofb_show_offset(uint8 lchip, uint8 ofb_type)
{
    uint16 block_id = 0;
    uint32 all_of_num = 0;
    uint32 used_of_num = 0;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_info_t*  ofb_info = NULL;
    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_LOCK;
    ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n", "Block",  "L", "R", "Limit", "M", "All", "Used");
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================================================================\n");
    for (block_id = 0; block_id < ofb_info->block_num; block_id++)
    {
        block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
         if (block->all_of_num)
        {
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",
                            block_id,  block->all_left_b, block->all_right_b, block->have_limit, block->multiple, block->all_of_num, block->used_of_num);
        }
        else
        {
            SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d %-8s %-8s %-8d %-8d %-8d %-8d\n",
                            block_id,  "-", "-", block->have_limit, block->multiple, block->all_of_num, block->used_of_num);
        }
        all_of_num += block->all_of_num;
        used_of_num += block->used_of_num;
    }
    SYS_OFB_DBG_OUT( CTC_DEBUG_LEVEL_DUMP, "===================================================================================\n");
    SYS_OFB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,  "%-8s %-8s %-8s %-8s %-8s %-8d %-8d\n", "Total",  "-", "-", "-", "-", all_of_num, used_of_num);
    SYS_OFB_UNLOCK;
    return CTC_E_NONE;
}
int32
sys_usw_ofb_dump_db(uint8 lchip, uint16 ofb_type, sal_file_t p_f)
{
    uint16 block_id = 0;
    uint32 all_of_num = 0;
    uint32 used_of_num = 0;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_ofb_info_t*  ofb_info = NULL;
    SYS_OFB_TYPE_CHECK(ofb_type);
    SYS_OFB_LOCK;
    ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    SYS_DUMP_DB_LOG(p_f, "%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n", "Block",  "L", "R", "Limit", "M", "All", "Used");
    SYS_DUMP_DB_LOG(p_f, "===================================================================================\n");
    for (block_id = 0; block_id < ofb_info->block_num; block_id++)
    {
        block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
         if (block->all_of_num)
        {
            SYS_DUMP_DB_LOG(p_f, "%-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",
                            block_id,  block->all_left_b, block->all_right_b, block->have_limit, block->multiple, block->all_of_num, block->used_of_num);
        }
        else
        {
            SYS_DUMP_DB_LOG(p_f, "%-8d %-8s %-8s %-8d %-8d %-8d %-8d\n",
                            block_id,  "-", "-", block->have_limit, block->multiple, block->all_of_num, block->used_of_num);
        }
        all_of_num += block->all_of_num;
        used_of_num += block->used_of_num;
    }
    SYS_DUMP_DB_LOG(p_f, "===================================================================================\n");
    SYS_DUMP_DB_LOG(p_f,  "%-8s %-8s %-8s %-8s %-8s %-8d %-8d\n", "Total",  "-", "-", "-", "-", all_of_num, used_of_num);
    SYS_OFB_UNLOCK;
    return CTC_E_NONE;
}
#define _________WB________
int32
sys_usw_ofb_alloc_offset_from_position(uint8 lchip, uint8 ofb_type, uint16 block_id, uint32 p_offset, void* user_data)
{
    sys_usw_opf_t opf;
    sys_usw_ofb_block_t* block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
    sys_usw_ofb_info_t*  ofb_info = p_usw_ofb_master[lchip]->ofb_info[ofb_type];
    uint8 alloc_size = block->multiple;
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = ofb_info->opf_type;
    opf.pool_index = block_id;
    if ((ofb_info->limit_boundary) && (p_offset < ofb_info->limit_boundary) && block->multiple != SYS_OFB_MIN_MULTIPLE)
    {
        alloc_size = ofb_info->max_multiple;
    }
    opf.multiple = alloc_size;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, alloc_size, p_offset));
    block->used_of_num += alloc_size;
    CTC_ERROR_RETURN(ctc_vector_add(p_usw_ofb_master[lchip]->ofb_info[ofb_type]->offset_info, p_offset, user_data));
    return CTC_E_NONE;
}


int32
sys_usw_ofb_wb_sync(uint8 lchip, uint8 ofb_type, uint32  module_id, uint32 sub_id)
{
    uint16 block_id = 0;
    uint16 max_entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_ofb_t* p_wb_ofb = NULL;
    sys_usw_ofb_block_t* block = NULL;

    SYS_OFB_TYPE_CHECK(ofb_type);

    wb_data.buffer = mem_malloc(MEM_OPF_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_ofb_t, module_id, sub_id);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);
    for (block_id = 0; block_id < p_usw_ofb_master[lchip]->ofb_info[ofb_type]->block_num; block_id++)
    {
        block = SYS_OFB_BLOCK_INFO(ofb_type, block_id);
        p_wb_ofb = (sys_wb_ofb_t *)wb_data.buffer + wb_data.valid_cnt;
        p_wb_ofb->type = sub_id;
        p_wb_ofb->ofb_type = ofb_type;
        p_wb_ofb->block_id = block_id;
        p_wb_ofb->all_left_b = block->all_left_b;
        p_wb_ofb->all_right_b = block->all_right_b;
        p_wb_ofb->all_of_num = block->all_of_num;
        p_wb_ofb->adj_dir = block->adj_dir;
        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
            wb_data.valid_cnt = 0;
        }
    }

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_ofb_wb_restore(uint8 lchip, uint8 ofb_type, uint32  module_id, uint32 sub_id)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_ofb_t* p_wb_ofb = NULL;
    uint16 entry_cnt = 0;
    uint32 offset = 0;
    sys_usw_ofb_block_t* block = NULL;
    sys_usw_opf_t opf;

    SYS_OFB_TYPE_CHECK(ofb_type);

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    /*restore mirror dest*/
    wb_query.buffer = mem_malloc(MEM_OPF_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    sal_memset(&opf, 0 , sizeof(opf));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ofb_t, module_id, sub_id);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        p_wb_ofb = (sys_wb_ofb_t*)wb_query.buffer + entry_cnt++;/*after reading entry from sqlite, wb_query.buffer is still the first address of the space*/
        if (p_wb_ofb->ofb_type != ofb_type)
        {
            continue;
        }
        block = SYS_OFB_BLOCK_INFO(ofb_type, p_wb_ofb->block_id);
        block->all_left_b = p_wb_ofb->all_left_b;
        block->all_right_b = p_wb_ofb->all_right_b;
        block->all_of_num = p_wb_ofb->all_of_num;
        block->adj_dir = p_wb_ofb->adj_dir;

        sys_usw_opf_free(lchip, p_usw_ofb_master[lchip]->ofb_info[ofb_type]->opf_type, p_wb_ofb->block_id);
        opf.pool_type = p_usw_ofb_master[lchip]->ofb_info[ofb_type]->opf_type;
        opf.pool_index = p_wb_ofb->block_id;
        sys_usw_opf_init_offset(lchip, &opf, 0, p_usw_ofb_master[lchip]->ofb_info[ofb_type]->max_size);
        sys_usw_opf_alloc_offset(lchip, &opf, p_usw_ofb_master[lchip]->ofb_info[ofb_type]->max_size, &offset);
        if (block->all_of_num)
        {
            sys_usw_opf_free_offset(lchip, &opf, block->all_right_b - block->all_left_b + 1, block->all_left_b);
        }
        _sys_usw_ofb_key_update_boundary_block( lchip, ofb_type, p_wb_ofb->block_id);

    CTC_WB_QUERY_ENTRY_END((&wb_query));
done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }
    return ret;
}


