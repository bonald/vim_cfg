/**
 @file sys_usw_fpa.c

   @date 2016-08-01

   @version v3.0

 */

/****************************************************************************
*
* Header Files
*

****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
/*#include <time.h>*/


#include "sys_usw_fpa.h"

/* pe is ctc_fpa_entry_t */
#define FPA_OUTER_ENTRY(pe)          ((uint8 *) pe - (fpa->offset))
/*#define FPA_INNER_ENTRY(i) ( (ctc_fpa_entry_t*) ( (pb->entries[i]) + (fpa->offset) ))*/
#define FPA_INNER_ENTRY(i)           (pb->entries[i])
#define PREV_IDX(idx, step)          ((idx) - (step))
#define NEXT_IDX(idx, step)          ((idx) + (step))
#define TRIGGER_REORDER      25
#define TRIGGER_PRESS        10
#define DO_REORDER(move_num, all)    ((move_num) > ((all) / TRIGGER_REORDER))
#define DO_PRESS(move_num, all)      ((move_num) == TRIGGER_PRESS)
#define FPA_INVALID_INDEX    (-1)
#define FPA_FREE_COUNT(fpa_size)     (((pb->sub_free_count[fpa_size])>(pb->sub_rsv_count[fpa_size]))?((pb->sub_free_count[fpa_size])-(pb->sub_rsv_count[fpa_size])):0)

#define SYS_FPA_DBG_OUT(level, FMT, ...) \
    CTC_DEBUG_OUT(fpa, fpa, FPA, level, FMT, ## __VA_ARGS__)

#define SYS_FPA_DBG_FUNC() \
    SYS_FPA_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__)

#define SYS_FPA_DBG_INFO(FMT, ...) \
    SYS_FPA_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ## __VA_ARGS__)

#define SYS_FPA_DBG_PARAM(FMT, ...) \
    SYS_FPA_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ## __VA_ARGS__)

#define SYS_FPA_DBG_ERROR(FMT, ...) \
    SYS_FPA_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ## __VA_ARGS__)

#define SYS_FPA_DBG_DUMP(FMT, ...) \
    SYS_FPA_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ## __VA_ARGS__)



STATIC uint8
_fpa_usw_get_step(uint8 key_size)
{
    uint8 step = 0;

    switch(key_size)
    {
        case CTC_FPA_KEY_SIZE_80:
            step = 1;
            break;

        case CTC_FPA_KEY_SIZE_160:
            step = 2;
            break;

        case CTC_FPA_KEY_SIZE_320:
            step = 4;
            break;

        case CTC_FPA_KEY_SIZE_640:
            step = 8;
            break;

        default:
            break;
    }

    return step;
}

/*
 * move entry to new position in specific chip.
 */
STATIC int32
_fpa_usw_entry_move(ctc_fpa_t* fpa, ctc_fpa_entry_t* pe, int32 amount)
{
    int32          tcam_idx_old = 0; /* Original entry tcam index.    */
    int32          tcam_idx_new = 0; /* Next tcam index for the entry.*/
    ctc_fpa_block_t* pb = NULL;             /* Field slice control.          */

    int32          ret = 0;

    if (!pe)
    {
        return  CTC_E_NONE;/* skip */
    }
    fpa->get_info_by_pe(pe, &pb);
    CTC_PTR_VALID_CHECK(pb);

    if (amount == 0) /* move no where */
    {
        return CTC_E_NONE;
    }

    tcam_idx_old = pe->offset_a;
    tcam_idx_new = tcam_idx_old + amount;

    /* Move the hardware entry, even if entry is not installed, because of scl need update key index in pe entry .*/
    {
        ret = fpa->entry_move_hw(pe, tcam_idx_new);
        CTC_ERROR_RETURN(ret);
    }

    /* Move the software entry.*/

    pb->entries[tcam_idx_old] = NULL;
    pb->entries[tcam_idx_new] = pe;
    pe->offset_a       = tcam_idx_new;

    return(CTC_E_NONE);
}

/*
 * shift up entries from target entry to prev null entry
 */
STATIC int32
_fpa_usw_entry_shift_up(ctc_fpa_t* fpa,
                    ctc_fpa_block_t* pb,
                    int32 target_index,
                    int32 prev_null_index,
                    uint8 step,
                    uint32* move_num)
{
    int32 temp;

    /* Input parameter check. */
    CTC_PTR_VALID_CHECK(pb);

    temp = prev_null_index;

    /* start from prev-1:
     *  prev + 1 -- > prev
     *  prev + 2 -- > prev + 1
     *  ...
     *  target -- > target - 1
     */
    while ((temp < target_index))
    {
        /* Move the entry at the next index to the prev. empty index. */

        /* Perform entry move. */
        CTC_ERROR_RETURN(_fpa_usw_entry_move(fpa, pb->entries[temp + step], -step));

        temp = temp + step;
    }

    temp = (target_index - prev_null_index)/step;
    *move_num = (temp < 0) ? 0 : temp;
    return(CTC_E_NONE);
}

/*
 * shift down entries from target entry to next null entry
 */
STATIC int32
_fpa_usw_entry_shift_down(ctc_fpa_t* fpa,
                      ctc_fpa_block_t* pb,
                      int32 target_index,
                      int32 next_null_index,
                      uint8 step,
                      uint32* move_num)
{
    int32 temp;

    /* Input parameter check. */
    CTC_PTR_VALID_CHECK(pb);

    temp = next_null_index;

    /*
     * Move entries one step down
     *     starting from the last entry
     * start from prev-1:
     *  next - 1 -- > next
     *  next - 2 -- > next -1
     *  ...
     *  target -- > target + 1
     */
    while (temp > target_index)
    {
        /* Perform entry move. */
        CTC_ERROR_RETURN(_fpa_usw_entry_move(fpa, pb->entries[temp - step], step));
        temp = temp - step;
    }

    *move_num = (next_null_index - target_index)/step;

    return(CTC_E_NONE);
}

/*
 * get how many step need to shift entries from target entry to null entry.
 * return value:
 *   TRUE  -- shift is ok
 *   FALSE -- shift is not ok
 *   shift_count --  how many entries need shift
 */
STATIC bool
_fpa_usw_get_shift_count(int32 null, int32 target, int32 dir, int32* count)
{
    if (-1 == dir)
    {
        target--; /* shift up, targe is one up. */
    }
    *count = dir * (null - target);

    if (null == FPA_INVALID_INDEX)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*
 * get shift direction, up or down
 * return value
 *   TRUE  -- shift is feasible
 *   FALSE -- shift is not feasible
 *   dir   -- shift to which direction
 */
STATIC int32
_fpa_usw_get_shift_direction(int32 prev_null_index,
                         int32 target_index,
                         int32 next_null_index,
                         int32* dir)
{
    bool  shift_up          = FALSE;
    bool  shift_down        = FALSE;
    int32 shift_up_amount   = 0;
    int32 shift_down_amount = 0;

    /* Input parameter check. */

    CTC_PTR_VALID_CHECK(dir);


    shift_up = _fpa_usw_get_shift_count(prev_null_index, target_index, -1, &shift_up_amount);

    shift_down = _fpa_usw_get_shift_count(next_null_index, target_index, 1, &shift_down_amount);

    SYS_FPA_DBG_INFO("  %%INFO: shift up   %s count %d \n", shift_up ? "T" : "F", shift_up_amount);
    SYS_FPA_DBG_INFO("  %%INFO: shift down %s count %d \n", shift_down ? "T" : "F", shift_down_amount);

    if (shift_up == TRUE)
    {
        if (shift_down == TRUE)
        {
            if (shift_up_amount < shift_down_amount)
            {
                *dir = -1;
            }
            else
            {
                *dir = 1;
            }
        }
        else
        {
            *dir = -1;
        }
    }
    else
    {
        if (shift_down == TRUE)
        {
            *dir = 1;
        }
        else
        {
            return CTC_E_NO_RESOURCE;
        }
    }

    return CTC_E_NONE;
}

typedef struct
{
    uint16 t_idx; /* target index */
    uint16 o_idx; /* old index */
}_fpa_target_t;

#define REORDE_TYPE_SCATTER     0
#define REORDE_TYPE_DECREASE    1
#define REORDE_TYPE_INCREASE    2

STATIC int32
_fpa_usw_reorder(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size, uint8 type, int16 start_move_num, int16 sub_count_vary)
{
    int32         idx;
    int32         revise;
    int32         t_idx;
    int32         o_idx;
    _fpa_target_t * target_a = NULL;
    int32         ret        = 0;
    uint8         move_ok;
    static uint32 cnt = 0;

    uint8         step   = 0;
/*    static double time;*/
/*    clock_t begin,end;*/
    uint32 start_offset = 0;
    uint32 old_start_offset = 0;
    uint32 full_num;
    uint32 old_full_num;
    uint32 free_num;
    uint32 real_num;             /* actual entry number */
    uint32 left_num;

    cnt++;
/*    begin = clock();*/
    CTC_PTR_VALID_CHECK(pb);

    step = _fpa_usw_get_step(key_size);

    /* malloc a new array based on new exist entry*/
    old_start_offset = pb->start_offset[key_size];
    start_offset = pb->start_offset[key_size] + start_move_num;

    old_full_num = pb->sub_entry_count[key_size];
    full_num = pb->sub_entry_count[key_size] + sub_count_vary;
    free_num = pb->sub_free_count[key_size] + sub_count_vary;
    real_num = full_num - free_num; /* used entry count */

    if(0 == old_full_num || real_num == 0)
    {
        return CTC_E_NONE;
    }

    MALLOC_ZERO(MEM_FPA_MODULE, target_a, real_num * sizeof(_fpa_target_t))
    if(!target_a)
    {
       return CTC_E_NO_MEMORY;
    }

    /* save target idx to array */
    if (REORDE_TYPE_SCATTER == type)
    {
        for (t_idx = 0; t_idx < real_num; t_idx++)
        {
            target_a[t_idx].t_idx  = (full_num * t_idx / real_num) * step + start_offset;
        }
    }
    else if (REORDE_TYPE_DECREASE == type) /* move up,make room for low prio entry*/
    {
        for (t_idx = 0; t_idx < real_num; t_idx++)
        {
            target_a[t_idx].t_idx  = t_idx * step + start_offset;
        }
    }
    else /* move down,make room for high prio entry*/
    {
        for (t_idx = 0; t_idx < real_num; t_idx++)
        {
            target_a[t_idx].t_idx  = (free_num + t_idx) * step + start_offset;
        }
    }

    /* save old idx to array */
    o_idx = 0;
    for (idx = old_start_offset; idx < old_start_offset + old_full_num * step; idx = idx + step)  /* through all entry */
    {
        if (pb->entries[idx])
        {
            if (o_idx >= real_num )
            {
                SYS_FPA_DBG_DUMP("  unexpect real_num %d! \n", real_num);
            }
            target_a[o_idx].o_idx = idx;
            o_idx++;
        }
    }

    left_num = real_num;
    while (left_num) /* move_num */
    {
        SYS_FPA_DBG_INFO("left_num %d, real_num %d\n", left_num, real_num);

        for (idx = 0; idx < left_num; idx++)
        {
            revise  = (REORDE_TYPE_INCREASE == type) ? (left_num - 1 - idx) : idx;
            move_ok = 0;

            if (target_a[revise].o_idx == target_a[revise].t_idx) /* stay */
            {
                SYS_FPA_DBG_INFO("stay !\n");
                move_ok = 1;
            }
            else
            {
                if (target_a[revise].o_idx < target_a[revise].t_idx) /* move down */
                {
                    if ((revise == left_num - 1) || (target_a[revise + 1].o_idx > target_a[revise].t_idx))
                    {
                        move_ok = 1;
                    }
                }
                else /* move up */
                {
                    if ((revise == 0) || (target_a[revise - 1].o_idx > target_a[revise].t_idx))
                    {
                        move_ok = 1;
                    }
                }

                if (move_ok)
                {
                    SYS_FPA_DBG_INFO(" move from %d to %d!\n", target_a[revise].o_idx, target_a[revise].t_idx);
                    /* move revise to temp */
                    CTC_ERROR_GOTO(_fpa_usw_entry_move(fpa, pb->entries[target_a[revise].o_idx],
                                                   (target_a[revise].t_idx - target_a[revise].o_idx)), ret, cleanup);
                }
            }

            if (move_ok)
            {
                if (REORDE_TYPE_INCREASE != type)
                {
                    sal_memmove(&target_a[revise], &target_a[revise + 1], (left_num - revise - 1) * sizeof(_fpa_target_t));
                }
                left_num--;
                idx--;
            }
        }
    }

    mem_free(target_a);

/*    end = clock();*/
/*    time = time + ((double)(end - begin) / CLOCKS_PER_SEC);*/
/*    SYS_FPA_DBG_INFO(" t:%lf", time);*/

    return CTC_E_NONE;

 cleanup:
    mem_free(target_a);
    return ret;
}


/* output:
 * targ, if invalid: 1. all null. 2. prio is the smallest.
 * prev(first null before target), if invalid: 1. before targ no null.
 * next(first null after target), if invalid: 1. targ is invalid 2. targ found, but after targ no null.
 */


/*
 first_valid != first_normal. <==> highest entry exist.
 last_valid  != last_normal. <==> lowest entry exist.
 targ != targ_normal.   <==> targ_normal = -1, targ found but entry is not normal and has highest prio.
 prev === prev_n. next === next_n.
*/
STATIC int32
_fpa_usw_get_indexes(uint32 prio, ctc_fpa_entry_t* pe, ctc_fpa_block_t* pb, uint8 key_size,
                 int32* targ, int32* prev, int32* next,
                 int* first_null  , int* last_null,
                 int* first_valid, int* last_valid)
{
    uint8 step = 0;
    int32 idx;

    step = _fpa_usw_get_step(key_size);

    for (idx = pb->start_offset[key_size]; idx < pb->start_offset[key_size] + pb->sub_entry_count[key_size]*step; idx = idx + step)
    {
        /* Skip the pe itself */
        if (pe && (pe == pb->entries[idx]))
        {
            continue;
        }

        if (pb->entries[idx] == NULL)
        {
            if (FPA_INVALID_INDEX == *targ)
            {
                *prev = idx;
            }
            else if (FPA_INVALID_INDEX == *next)
            {
                *next = idx;
            }
            if (FPA_INVALID_INDEX == *first_null)
            {
                *first_null = idx;
            }
            *last_null = idx;
        }
        else /* valid entry */
        {
            *last_valid = idx;

            if ((prio > pb->entries[idx]->priority)
                && (FPA_INVALID_INDEX == *targ)) /* found target */
            {
                *targ = idx;
            }

            if (FPA_INVALID_INDEX == *first_valid)  /* found first_valid */
            {
                *first_valid = idx;
            }
        }
    }
    if (prio > pb->last_priority[key_size])
    {
        pb->decrease_trend[key_size] = 0;
        pb->increase_trend[key_size]++;
    }
    else
    {
        pb->decrease_trend[key_size]++;
        pb->increase_trend[key_size] = 0;
    }

    pb->trend_active[key_size] = ((pb->decrease_trend[key_size] + pb->increase_trend[key_size]) >= TRIGGER_PRESS);
    pb->last_priority[key_size] = prio;

    return CTC_E_NONE;
}
#define SUB_BLOCK_END(step)   (pb->start_offset[key_size] + (pb->sub_entry_count[key_size] - 1) * (step))

#define SUB_BLOCK_START       (pb->start_offset[key_size])

STATIC int32
_fpa_usw_get_offset(ctc_fpa_t* fpa, ctc_fpa_entry_t* pe, ctc_fpa_block_t* pb, uint8 key_size,
                uint32 prio, int32* target, uint32* move_num_out)
{
    int32  target_out;
    int32  prev        = FPA_INVALID_INDEX;
    int32  next        = FPA_INVALID_INDEX;
    int32  first_valid = FPA_INVALID_INDEX;
    int32  last_valid  = FPA_INVALID_INDEX;
    int32  first_null  = FPA_INVALID_INDEX;
    int32  last_null   = FPA_INVALID_INDEX;
    int32  targ        = FPA_INVALID_INDEX;
    int32  dir = 0;
    uint8  step = 0;
    uint32 move_num = 0;

    /* block is empty. */
   /*  if (pb->entry_count == pb->free_count)*/
   /*  {*/
  /*       *target = pb->entry_count >> 1;*/
   /*      return CTC_E_NONE;*/
   /*  }*/

    step = _fpa_usw_get_step(key_size);

    _fpa_usw_get_indexes(prio, pe, pb, key_size, &targ, &prev, &next,
                     &first_null, &last_null, &first_valid, &last_valid);

    if (FPA_INVALID_INDEX == targ)
    {
        if(FPA_INVALID_INDEX == first_valid)
        {
            target_out = pb->start_offset[key_size] + step * (pb->sub_entry_count[key_size] >> 1);   /*when set entry priority, only one entry exist(itself)*/
        }
        else if (SUB_BLOCK_END(step) == last_valid)    /*last valid entry in the end of block*/
        {
            target_out = SUB_BLOCK_END(step);
            CTC_ERROR_RETURN(_fpa_usw_entry_shift_up(fpa, pb, target_out, prev, step, &move_num));
        }
        else
        {
            target_out = NEXT_IDX(last_valid, step); /* must be null */
        }
    }
    else if(first_valid == targ)
    {
        if(SUB_BLOCK_START == targ)   /*in the start of the block*/
        {
            target_out = SUB_BLOCK_START;
            CTC_ERROR_RETURN(_fpa_usw_entry_shift_down(fpa, pb, target_out, next, step, &move_num));
        }
        else
        {
            target_out = PREV_IDX(targ, step);
        }
    }
    else
    {
        _fpa_usw_get_shift_direction(prev, targ, next, &dir);
        if (dir == 1)
        {
            target_out = targ;
            /*
             * Move the entry at the targ index to target_index+1. This may
             * mean shifting more entries down to make room. In other words,
             * shift the targ index and any that follow it down 1 as far as the
             * next empty index.
             */
            SYS_FPA_DBG_INFO("  %%INFO: from %d to %d do shift down \n", target_out, next - step);
            CTC_ERROR_RETURN(_fpa_usw_entry_shift_down(fpa, pb, target_out, next, step, &move_num));
        }
        else
        {
            /*
             * for Shifting UP , the targ is one up.
             */
            target_out = targ - step;

            SYS_FPA_DBG_INFO("  %%INFO: from %d to %d do shift up \n", target_out, prev + step);
            CTC_ERROR_RETURN(_fpa_usw_entry_shift_up(fpa, pb, target_out, prev, step, &move_num));
        }
    }


    *target = target_out;
    *move_num_out = move_num;
    return CTC_E_NONE;
}


STATIC int32
_fpa_usw_adjust_resource(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size)
{
    int8   idx1 = 0;
    int8   idx2 = 0;
    uint8  step = 0;
    uint8  factor = 0;
    uint8  adjust_ok = 0;
    uint8  require_step = 0;
    uint16 up_free_count = 0;
    uint16 down_free_count = 0;
    uint16 update_entry_count = 0;

    for(idx1 = 0; idx1 < CTC_FPA_KEY_SIZE_NUM; idx1++)
    {
        step = _fpa_usw_get_step(idx1);
        if(idx1 < key_size)
        {
            up_free_count += (FPA_FREE_COUNT(idx1) * step);
        }
        else
        {
            down_free_count += (FPA_FREE_COUNT(idx1) * step);
        }
    }
    require_step = _fpa_usw_get_step(key_size);

    /*one condition not care: up_free_count < step and down_free_count < step, but up+down >= step*/
    if((up_free_count >= down_free_count) && (up_free_count >= require_step))
    {
         /*key size must greater than CTC_FPA_KEY_SIZE_160*/
        for(idx1 = key_size - 1; idx1 >= 0; idx1--)
        {
            step = _fpa_usw_get_step(idx1);
            if(FPA_FREE_COUNT(idx1) * step >= require_step)
            {
                adjust_ok = 1;
                if(FPA_FREE_COUNT(idx1)*step > (pb->entry_count/16))
                {
                    factor = 2;     /*take half of free*/
                }
                else
                {
                    factor = 1;     /*take all*/
                }
                update_entry_count = FPA_FREE_COUNT(idx1) * step / require_step;
                update_entry_count = update_entry_count / factor * require_step;
                update_entry_count = update_entry_count ? update_entry_count : require_step;
                _fpa_usw_reorder(fpa, pb, idx1, REORDE_TYPE_SCATTER, 0, -(update_entry_count / step));
                pb->sub_entry_count[idx1] -= (update_entry_count / step);
                pb->sub_free_count[idx1] -= (update_entry_count / step);
                for(idx2 = idx1 + 1; idx2 < key_size; idx2++)
                {
                    _fpa_usw_reorder(fpa, pb, idx2, REORDE_TYPE_DECREASE, -update_entry_count, 0);    /*move up*/
                    pb->start_offset[idx2] -= update_entry_count;
                }
                pb->sub_entry_count[key_size] += (update_entry_count / require_step);
                pb->sub_free_count[key_size] += (update_entry_count / require_step);
                pb->start_offset[key_size] -= update_entry_count;   /*no need reorder*/
                break;
            }
        }
    }
    else if(down_free_count >= require_step)
    {
         /*key size must less than CTC_FPA_KEY_SIZE_640*/
        for(idx1 = key_size + 1; idx1 < CTC_FPA_KEY_SIZE_NUM; idx1++)
        {
            step = _fpa_usw_get_step(idx1);
            if(FPA_FREE_COUNT(idx1) > 0)
            {
                adjust_ok = 1;
                if(FPA_FREE_COUNT(idx1)*step > (pb->entry_count/16))
                {
                    factor = 2;
                }
                else
                {
                    factor = 1;
                }
                update_entry_count = FPA_FREE_COUNT(idx1) / factor * step;
                update_entry_count = update_entry_count ? update_entry_count : step;
                _fpa_usw_reorder(fpa, pb, idx1, REORDE_TYPE_SCATTER, update_entry_count, -(update_entry_count / step));
                pb->sub_entry_count[idx1] -= (update_entry_count / step);
                pb->sub_free_count[idx1] -= (update_entry_count / step);
                pb->start_offset[idx1] += update_entry_count;
                for(idx2 = idx1 - 1; idx2 > key_size; idx2--)
                {
                    _fpa_usw_reorder(fpa, pb, idx2, REORDE_TYPE_INCREASE, update_entry_count, 0);    /*move down*/
                    pb->start_offset[idx2] += update_entry_count;
                }
                pb->sub_entry_count[key_size] += (update_entry_count / require_step);
                pb->sub_free_count[key_size] += (update_entry_count / require_step);
                break;
            }
        }
    }
    if(!adjust_ok)
    {
        return CTC_E_NO_RESOURCE;
    }

    return CTC_E_NONE;
}


#define  ___FPA_SORT_OUTER__


/*
 * set priority of entry specific by pe and pb. pe must be in pb
 */
int32
fpa_usw_set_entry_prio(ctc_fpa_t* fpa, ctc_fpa_entry_t* pe, ctc_fpa_block_t* pb, uint8 key_size, uint32 prio)
{
    int32  target = 0;
    int32  temp;
    int32  temp1;
    uint32 move_num = 0;
    uint32 old_prio = 0;

    /* If the priority isn't changing, just return.*/
    old_prio = pe->priority;
    if (old_prio == prio)
    {
        return(CTC_E_NONE);
    }

    if (0 == pb->sub_free_count[key_size])   /*need adjust resource*/
    {
        CTC_ERROR_RETURN(_fpa_usw_adjust_resource(fpa, pb, key_size));
    }

    if ((0 == pb->free_count) || (0 == pb->sub_free_count[key_size]))
    {
        return CTC_E_NO_RESOURCE;
    }

    CTC_ERROR_RETURN(_fpa_usw_get_offset(fpa, pe, pb, key_size, prio, &target, &move_num));
    temp  = target;
    temp1 = pe->offset_a;

    if ((temp - temp1) != 0)
    {

        CTC_ERROR_RETURN(_fpa_usw_entry_move(fpa, pe, (temp - temp1)));
    }

    /* Assign the requested priority to the entry. */
    pe->priority = prio;

    /* do reorde after update count. this process is independant. */
    if (DO_REORDER(move_num, pb->sub_entry_count[key_size]))
    {
        CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_SCATTER, 0, 0));
    }

    return(CTC_E_NONE);
}

/*
 * worst is best
 */
int32
fpa_usw_alloc_offset(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size, uint32 prio, uint32* block_index)
{
    int32 target = 0;
    uint8 step = 0;
    CTC_PTR_VALID_CHECK(pb);
    CTC_PTR_VALID_CHECK(block_index);

    step = _fpa_usw_get_step(key_size);

    if (pb->free_count < step)
    {
        return CTC_E_NO_RESOURCE;
    }

    if (0 == pb->sub_free_count[key_size])   /*need adjust resource*/
    {
        CTC_ERROR_RETURN(_fpa_usw_adjust_resource(fpa, pb, key_size));
    }

    if (DO_PRESS(pb->decrease_trend[key_size], pb->sub_entry_count[key_size]))
    {
        CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_DECREASE, 0, 0));
    }
    else if (DO_PRESS(pb->increase_trend[key_size], pb->sub_entry_count[key_size]))
    {
        CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_INCREASE, 0, 0));
    }

    CTC_ERROR_RETURN(_fpa_usw_get_offset(fpa, NULL, pb, key_size, prio, &target, &pb->move_num));

    *block_index = target;

    (pb->free_count) -= step;
    (pb->sub_free_count[key_size])--;

    return CTC_E_NONE;
}


/*
 * worst is best
 */
int32
fpa_usw_free_offset(ctc_fpa_block_t* pb, uint32 block_index)

{
    uint8 step = 0;
    int8 key_size = 0;  /*must use int8, or it will always be true in for loop*/

    for(key_size = CTC_FPA_KEY_SIZE_640; key_size >= CTC_FPA_KEY_SIZE_80; key_size--)
    {
        if((block_index >= pb->start_offset[key_size]) && (pb->sub_entry_count[key_size] > 0))
        {
            step = _fpa_usw_get_step(key_size);
            pb->free_count += step;
            (pb->sub_free_count[key_size])++;
            pb->entries[block_index] = NULL;

            pb->decrease_trend[key_size]  = 0;
            pb->increase_trend[key_size]  = 0;
            pb->trend_active[key_size]    = 0;

            break;
        }
    }



    return CTC_E_NONE;
}

int32
fpa_usw_reorder(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size)
{
    if (DO_REORDER(pb->move_num, pb->sub_entry_count[key_size]) && !pb->trend_active[key_size])
    {
        CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_SCATTER, 0, 0));
    }

    pb->move_num = 0;
    return CTC_E_NONE;
}

int32
fpa_usw_scatter(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size, int32 shift_num, int16 vary)
{
    CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_SCATTER, shift_num, vary));

    pb->move_num = 0;
    return CTC_E_NONE;
}

int32
fpa_usw_increase(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size, int32 shift_num, int16 vary)
{
    CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_INCREASE, shift_num, vary));

    pb->move_num = 0;
    return CTC_E_NONE;
}

int32
fpa_usw_decrease(ctc_fpa_t* fpa, ctc_fpa_block_t* pb, uint8 key_size, int32 shift_num, int16 vary)
{
    CTC_ERROR_RETURN(_fpa_usw_reorder(fpa, pb, key_size, REORDE_TYPE_DECREASE, shift_num, vary));

    pb->move_num = 0;
    return CTC_E_NONE;
}

/*
 * set priority of entry specific by entry id
 */
ctc_fpa_t*
fpa_usw_create(fpa_get_info_by_pe_fn fn1,
           fpa_entry_move_hw_fn fn2,
           uint8 offset)
{
    ctc_fpa_t* fpa = NULL;

    MALLOC_ZERO(MEM_FPA_MODULE, fpa, sizeof(ctc_fpa_t));

    if (NULL == fpa)
    {
        return NULL;
    }

    fpa->get_info_by_pe = fn1;
    fpa->entry_move_hw  = fn2;
    fpa->offset         = offset;

    return fpa;
}

void
fpa_usw_free(ctc_fpa_t* fpa)
{
    mem_free(fpa);
}


