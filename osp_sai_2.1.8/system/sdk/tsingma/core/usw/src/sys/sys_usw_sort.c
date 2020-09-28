/**
 @file sys_usw_sort.c

 @date 2009-12-19

 @version v2.0

 The file contains all sort related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "sys_usw_chip.h"
#include "sys_usw_sort.h"


/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_usw_sort_key_do_alloc_offset(uint8 lchip, sys_sort_key_info_t* key_info, uint32* p_offset)
{
    sys_usw_opf_t opf;
    sys_sort_block_t* block = NULL;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    opf.multiple = block->multiple;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, block->multiple, p_offset));

    /* adjust block info */
    CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    block->used_of_num += block->multiple;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_do_alloc_offset_from_position(uint8 lchip, sys_sort_key_info_t* key_info, uint32 offset)
{
    sys_usw_opf_t opf;
    sys_sort_block_t* block = NULL;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    opf.multiple = block->multiple;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, block->multiple, offset));

    /* adjust block info */
    CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    block->used_of_num += block->multiple;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_do_free_offset(uint8 lchip, sys_sort_key_info_t* key_info, uint32 offset)
{
    sys_sort_block_t* block = NULL;
    sys_usw_opf_t opf;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    opf.multiple = block->multiple;
    /* 2. do it */
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, block->multiple, offset));
    /* adjust block info */
    CTC_NOT_EQUAL_CHECK(0, block->used_of_num);
    block->used_of_num -= block->multiple;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_block_has_avil_offset(uint8 lchip, sys_sort_key_info_t* key_info, bool* p_has)
{
    sys_sort_block_t* block = NULL;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
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
_sys_usw_sort_key_adj_block_id(uint8 lchip, sys_sort_key_info_t* key_info, sys_sort_block_dir_t dir, uint32* adj_block_id, uint8* adj_size)
{
    int32 i = 0;
    sys_sort_block_t* block = NULL;

    block = key_info->block;

    if (SYS_SORT_BLOCK_DIR_UP == dir)
    {
        for (i = key_info->block_id - 1; i >= 0; i--)
        {
            if (FALSE == block[i].is_block_can_shrink)
            {
                return CTC_E_NO_RESOURCE;
            }

            CTC_MAX_VALUE_CHECK(block[i].used_of_num, block[i].all_of_num);
            if (block[i].multiple > *adj_size)
            {
                *adj_size = block[i].multiple;
            }
#if 0
            if ((block[i].all_of_num - block[i].used_of_num == 1) && (block[i].used_of_num == 0))
            {
                continue;
            }
#endif
            if (block[i].all_of_num - block[i].used_of_num >= *adj_size)
            {
                *adj_block_id = i;
                break;
            }
        }

        if (i < 0)
        {
            return CTC_E_NO_RESOURCE;
        }
    }
    else
    {
        for (i = key_info->block_id + 1; i < key_info->max_block_num; i++)
        {
            if (FALSE == block[i].is_block_can_shrink)
            {
                return CTC_E_NO_RESOURCE;
            }

            CTC_MAX_VALUE_CHECK(block[i].used_of_num, block[i].all_of_num);

            if (block[i].multiple > *adj_size)
            {
                *adj_size = block[i].multiple;
            }
#if 0
            if ((block[i].all_of_num - block[i].used_of_num == 1) && (block[i].used_of_num == 0))
            {
                continue;
            }
#endif
            if (block[i].all_of_num - block[i].used_of_num >= *adj_size)
            {
                *adj_block_id = i;
                break;
            }
        }

        if (i == key_info->max_block_num)
        {
            return CTC_E_NO_RESOURCE;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_do_block_shrink(uint8 lchip, sys_sort_key_info_t* key_info, sys_sort_block_dir_t dir)
{
    sys_sort_block_t* block = NULL;
    uint32 old_offset = 0, new_offset = 0;
    sys_usw_opf_t opf;
    uint8 i = 0;
    uint8 adj_time = 0;
    int32 ret = CTC_E_NONE;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    opf.multiple = block->multiple;

    /*Empty!!!*/
    if (block->used_of_num == 0 && block->all_of_num == 0)
    {
        return CTC_E_NONE;
    }

    /*ONLY ONE!!!*/
    if (block->used_of_num == 0 && block->all_of_num == key_info->adj_size)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, key_info->adj_size, &new_offset));
        (block->all_of_num) -= key_info->adj_size;
        block->adj_dir = dir;

        return CTC_E_NONE;
    }

    if (key_info->adj_size == block->multiple)
    {
        adj_time = 1;
    }
    else
    {
        adj_time = key_info->adj_size / block->multiple;
    }

    /* 2. do it */
    if (SYS_SORT_BLOCK_DIR_UP == dir)
    {
        for (i = 0; i < adj_time; i++)
        {
            /* 1) alloc one offset, which is supposed to be down side */
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, block->multiple, &new_offset));

            /* 2) move the top item to the "new offset", causing the HARD & DB entry move as well */
            old_offset = block->all_left_b;

            ret = (key_info->sync_fn)(lchip, new_offset, old_offset, key_info->pdata);
            if (ret)
            {
                sys_usw_opf_free_offset(lchip, &opf, block->multiple, new_offset);
                return ret;
            }

            /* 3) adjust the border property */
            CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_left_b);
            (block->all_left_b) += block->multiple;
        }
    }
    else
    {
        for (i = 0; i < adj_time; i++)
        {
            /* 1) alloc one offset, which is supposed to be up side */
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_last(lchip, &opf, block->multiple, &new_offset));

            /* 2) move the top item to the "new offset", causing the HARD & DB entry move as well */
            old_offset = block->all_right_b - block->multiple +  1;
            ret = (key_info->sync_fn)(lchip, new_offset, old_offset, key_info->pdata);
            if (ret)
            {
                sys_usw_opf_free_offset(lchip, &opf, block->multiple, new_offset);
                return ret;
            }

            /* 3) adjust the border property */
            CTC_NOT_EQUAL_CHECK(0, block->all_right_b);
            (block->all_right_b) -= block->multiple;
        }
    }

    /*4.adjust block info*/
    CTC_MAX_VALUE_CHECK(block->used_of_num, block->all_of_num);
    (block->all_of_num) -= key_info->adj_size;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_do_block_grow_with_dir(uint8 lchip, sys_sort_key_info_t* key_info, sys_sort_block_dir_t dir)
{
    sys_sort_block_t* block = NULL;
    uint32 desired_offset = 0;
    sys_usw_opf_t opf;

    /* take ONE offset in the direction "dir", put it into the offset pool, and that's it */

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);

    /*Empty!!!!!!!*/
    if (block->used_of_num == 0 && block->all_of_num == 0)
    {
        if (SYS_SORT_BLOCK_DIR_UP == dir)
        {
            (block->all_left_b) -= key_info->adj_size;
            (block->all_right_b) -= key_info->adj_size;
        }
        else
        {
            (block->all_left_b) += key_info->adj_size;
            (block->all_right_b) += key_info->adj_size;
        }

        return CTC_E_NONE;
    }

    /* 2. do it */
    /* 1) get offset */
    if (SYS_SORT_BLOCK_DIR_UP == dir)
    {
        CTC_NOT_EQUAL_CHECK(0, block->all_left_b);
        (block->all_left_b) -= key_info->adj_size;
        desired_offset = block->all_left_b;
    }
    else
    {
        CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_right_b);
        desired_offset = block->all_right_b + 1;
        (block->all_right_b) += key_info->adj_size;
    }

    CTC_MAX_VALUE_CHECK(block->all_left_b, block->all_right_b);

    /* 2) put the offset into the offset DB */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, key_info->adj_size, desired_offset));

    /* 4) adjust block info */
    CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_of_num);
    (block->all_of_num) += key_info->adj_size;

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_sort_key_do_block_empty_grow(uint8 lchip, sys_sort_key_info_t* key_info, sys_sort_block_dir_t dir)
{
    sys_sort_block_t* block = NULL;
    uint32 desired_offset = 0;
    sys_usw_opf_t opf;

    /* take ONE offset in the direction "dir", put it into the offset pool, and that's it */

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);

    if (SYS_SORT_BLOCK_DIR_UP == dir)
    {
        if (block->adj_dir == SYS_SORT_BLOCK_DIR_DOWN)
        {
            /*use current bound*/
        }
        else
        {
            block->all_left_b += key_info->adj_size;
            block->all_right_b += key_info->adj_size;
        }

        desired_offset = block->all_left_b;

    }
    else
    {

        if (block->adj_dir == SYS_SORT_BLOCK_DIR_UP)
        {
            /*use current bound*/
        }
        else
        {
            block->all_left_b -= key_info->adj_size;
            block->all_right_b -= key_info->adj_size;
        }


        desired_offset = block->all_left_b;
    }

    CTC_MAX_VALUE_CHECK(block->all_left_b, block->all_right_b);

    /* 2) put the offset into the offset DB */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = key_info->opf_type;
    opf.pool_index = key_info->block_id;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, key_info->adj_size, desired_offset));

    /* 4) adjust block info */
    CTC_NOT_EQUAL_CHECK(CTC_MAX_UINT32_VALUE, block->all_of_num);
    (block->all_of_num) += key_info->adj_size;

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_sort_key_block_grow_with_dir(uint8 lchip, sys_sort_key_info_t* key_info, sys_sort_block_dir_t dir)
{
    sys_sort_block_dir_t adj_dir;
    uint32 adj_block_id = 0;
    sys_sort_key_info_t adj_key_info;
    uint32 i = 0;
    uint8 adj_size = 1;
    sys_sort_block_t* block = NULL;

    /* 1. sanity check & init */
    SYS_SORT_CHECK_KEY_BLOCK_EXIST(key_info, dir);
    adj_size = key_info->block[key_info->block_id].multiple;
    CTC_ERROR_RETURN(_sys_usw_sort_key_adj_block_id(lchip, key_info, dir, &adj_block_id, &adj_size));
    adj_dir = SYS_SORT_OPPO_DIR(dir);
    sal_memcpy(&adj_key_info, key_info, sizeof(sys_sort_key_info_t));
    adj_key_info.block_id = adj_block_id;
    adj_key_info.adj_size = adj_size;

    if (dir == SYS_SORT_BLOCK_DIR_DOWN)
    {
        for (i = adj_block_id; i > key_info->block_id; i--)
        {
            adj_key_info.block_id = i;
            CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_shrink(lchip, &adj_key_info, adj_dir));
            adj_key_info.block_id = i - 1;
            CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_grow_with_dir(lchip, &adj_key_info, dir));
        }
    }
    else
    {
        for (i = adj_block_id; i < key_info->block_id; i++)
        {
            adj_key_info.block_id = i;
            CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_shrink(lchip, &adj_key_info, adj_dir));
            adj_key_info.block_id = i + 1;
            CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_grow_with_dir(lchip, &adj_key_info, dir));
        }
    }

    block = &(key_info->block[key_info->block_id]);
    if (block->all_of_num == 0)
    {
        key_info->adj_size = adj_size;
        CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_empty_grow(lchip, key_info, dir));
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_sort_key_block_grow(uint8 lchip, sys_sort_key_info_t* key_info)
{
    sys_sort_block_dir_t first_dir, second_dir;
    int32 ret = CTC_E_NONE;
    sys_sort_block_t* block = NULL;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    first_dir = block->preferred_dir;
    second_dir = SYS_SORT_OPPO_DIR(first_dir);

    /* 2. do it */
    ret = _sys_usw_sort_key_block_grow_with_dir(lchip, key_info, first_dir);
    if (CTC_E_NONE != ret)
    {
        CTC_ERROR_RETURN(_sys_usw_sort_key_block_grow_with_dir(lchip, key_info, second_dir));
    }

    return CTC_E_NONE;
}

/**
 @brief function of alloc offset

 @param[in] key_info, key of sort data
 @param[out] p_offset, alloced offset
 @return CTC_E_XXX
 */
int32
sys_usw_sort_key_alloc_offset(uint8 lchip, sys_sort_key_info_t* key_info, uint32* p_offset)
{
    bool has = 0;

    /* 1. sanity check & init */
    SYS_SORT_CHECK_KEY_INFO(key_info);
    CTC_PTR_VALID_CHECK(p_offset);

    /* 2. do it */
    CTC_ERROR_RETURN(_sys_usw_sort_key_block_has_avil_offset(lchip, key_info, &has));
    if (!has)
    {
        CTC_ERROR_RETURN(_sys_usw_sort_key_block_grow(lchip, key_info));
    }

    CTC_ERROR_RETURN(_sys_usw_sort_key_do_alloc_offset(lchip, key_info, p_offset));

    return CTC_E_NONE;
}

/**
 @brief function of alloc offset from position

 @param[in] key_info, key of sort data
 @param[in] offset, alloced offset
 @return CTC_E_XXX
 */
int32
sys_usw_sort_key_alloc_offset_from_position(uint8 lchip, sys_sort_key_info_t* key_info, uint32 offset)
{
    /* 1. sanity check & init */
    SYS_SORT_CHECK_KEY_INFO(key_info);

    CTC_ERROR_RETURN(_sys_usw_sort_key_do_alloc_offset_from_position(lchip, key_info, offset));

    return CTC_E_NONE;
}

/**
 @brief function of free offset

 @param[in] key_info, key of sort data
 @param[in] p_offset, offset should be free
 @return CTC_E_XXX
 */
int32
sys_usw_sort_key_free_offset(uint8 lchip, sys_sort_key_info_t* key_info, uint32 offset)
{
    sys_sort_block_t* block = NULL;
    sys_sort_key_info_t grow_key_info;
    uint8       adj_size = 0;
    uint32 left_num =0;

    /* 1. sanity check & init */
    block = &(key_info->block[key_info->block_id]);
    sal_memcpy(&grow_key_info, key_info, sizeof(sys_sort_key_info_t));
    adj_size = block->multiple;
    /* 2. do it */
    CTC_ERROR_RETURN(_sys_usw_sort_key_do_free_offset(lchip, key_info, offset));
    if (block->all_of_num == block->multiple)
    {
        return CTC_E_NONE;
    }

    left_num = block->all_of_num - block->used_of_num;
    if (offset < block->init_left_b)
    {
        grow_key_info.block_id -= 1;
        block = &(grow_key_info.block[grow_key_info.block_id]);
        if (left_num < block->multiple)
        {
            return CTC_E_NONE;
        }
        if (block->multiple > adj_size)
        {
            adj_size = block->multiple;
        }
        key_info->adj_size = adj_size;
        grow_key_info.adj_size = adj_size;
        if (block->all_of_num == 0)
        {
            return CTC_E_NONE;
        }
        CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_shrink(lchip, key_info, SYS_SORT_BLOCK_DIR_UP));
        CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_grow_with_dir(lchip, &grow_key_info, SYS_SORT_BLOCK_DIR_DOWN));
    }
    else if (offset > block->init_right_b)
    {
        grow_key_info.block_id += 1;

        block = &(grow_key_info.block[grow_key_info.block_id]);
        if (left_num < block->multiple)
        {
            return CTC_E_NONE;
        }
        if (block->multiple > adj_size)
        {
            adj_size = block->multiple;
        }
        key_info->adj_size = adj_size;
        grow_key_info.adj_size = adj_size;
        if (block->all_of_num == 0)
        {
            return CTC_E_NONE;
        }
        CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_shrink(lchip, key_info, SYS_SORT_BLOCK_DIR_DOWN));
        CTC_ERROR_RETURN(_sys_usw_sort_key_do_block_grow_with_dir(lchip, &grow_key_info, SYS_SORT_BLOCK_DIR_UP));
    }

    return CTC_E_NONE;
}

int32
sys_usw_sort_key_init_block(uint8 lchip, sys_sort_block_init_info_t* init_info)
{
    uint32 offset = 0;

    /* 1. check */
    CTC_PTR_VALID_CHECK(init_info->block);
    CTC_MAX_VALUE_CHECK(init_info->boundary_l, init_info->boundary_r);

    /*2.do it*/

    /* 3). init the first of-node */
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, init_info->opf, 0, init_info->max_offset_num));
    if (init_info->is_empty)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, init_info->opf, init_info->max_offset_num, &offset));
    }
    else
    {
        if (init_info->boundary_l > 0)
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, init_info->opf, init_info->boundary_l, &offset));
        }

        if (init_info->boundary_r + 1 < init_info->max_offset_num)
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, init_info->opf,
                                                                          init_info->max_offset_num - init_info->boundary_r - 1, init_info->boundary_r + 1));
        }
    }

    init_info->block->all_left_b = init_info->boundary_l;
    init_info->block->all_right_b = init_info->boundary_r;
    init_info->block->all_of_num = init_info->is_empty ? 0 : (init_info->boundary_r - init_info->boundary_l + 1);
    init_info->block->used_of_num = 0;
    init_info->block->preferred_dir = init_info->dir;
    init_info->block->init_left_b = init_info->boundary_l;
    init_info->block->init_right_b = init_info->boundary_r;
    init_info->block->is_block_can_shrink = init_info->is_block_can_shrink;

    if (0 == init_info->multiple)
    {
        init_info->block->multiple = 1;
    }
    else
    {
        init_info->block->multiple = init_info->multiple;
    }

    if (init_info->block->all_of_num % init_info->block->multiple)
    {
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

int32
sys_usw_sort_key_init_offset_array(uint8 lchip, skinfo_2a* pp_offset_array, uint32 max_offset_num)
{
    uint32 num = 0, left = 0;
    uint32 alloc_num = 0;
    int32 i = 0;
    skinfo_a* p_offset_array = NULL;

    if (max_offset_num > SORT_KEY_OFFSET_ARRAY_UNIT * SORT_KEY_OFFSET_ARRAY_MAX_NUM)
    {
        return CTC_E_NO_RESOURCE;
    }

    num = max_offset_num / SORT_KEY_OFFSET_ARRAY_UNIT;
    left = max_offset_num % SORT_KEY_OFFSET_ARRAY_UNIT;

    alloc_num = left ? (num + 1) : num;

    if (0 == left)
    {
        p_offset_array = mem_malloc(MEM_SORT_KEY_MODULE, num * sizeof(skinfo_a));
        if (NULL == p_offset_array)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_offset_array, 0, num * sizeof(skinfo_a));
    }
    else
    {
        p_offset_array = mem_malloc(MEM_SORT_KEY_MODULE, (num + 1) * sizeof(skinfo_a));
        if (NULL == p_offset_array)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_offset_array, 0, (num + 1) * sizeof(skinfo_a));
    }

    for (i = 0; i < num; i++)
    {
        p_offset_array[i] = mem_malloc(MEM_SORT_KEY_MODULE,
                                       SORT_KEY_OFFSET_ARRAY_UNIT * sizeof(skinfo_t));
        if (NULL == p_offset_array[i])
        {
            goto error_return;
        }
        sal_memset(p_offset_array[i], 0, SORT_KEY_OFFSET_ARRAY_UNIT * sizeof(skinfo_t));
    }

    if (left != 0)
    {
        p_offset_array[i] = mem_malloc(MEM_SORT_KEY_MODULE, left * sizeof(skinfo_t));
        if (NULL == p_offset_array[i])
        {
            goto error_return;
        }
        sal_memset(p_offset_array[i], 0, left * sizeof(skinfo_t));
    }

    *pp_offset_array = p_offset_array;

    return CTC_E_NONE;

error_return:
    for (i = 0; i < alloc_num; i++)
    {
        if (p_offset_array[i])
        {
           mem_free(p_offset_array[i]);
        }
    }
    mem_free(p_offset_array);
    return CTC_E_NO_MEMORY;
}

