/**
 @file sys_usw_nexthop_ecmp.c

 @date 2009-12-23

 @version v2.0

 The file contains all ecmp related function
*/

/****************************************************************************
*
* Header Files
*
****************************************************************************/
#include "ctc_const.h"
#include "ctc_error.h"
#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_nexthop_hw.h"
#include "sys_usw_l3if.h"
#include "sys_usw_wb_nh.h"
#include "drv_api.h"


#define SYS_ECMP_ALLOC_MEM_STEP  4

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/
extern int32
sys_usw_flow_stats_add_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id);
extern int32
sys_usw_flow_stats_remove_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id);

int32
sys_usw_nh_update_ecmp_group(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, uint8 is_create);
int32
sys_usw_nh_delete_ecmp_member(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, sys_nh_info_com_t* p_nh_info);
int32
sys_usw_nh_add_ecmp_member(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, sys_nh_info_com_t* p_nh_info);



#define __fragment_move__
/*
typedef int32 (* vector_traversal_fn)(void* array_data, void* user_data);
typedef void (* ctc_list_del_cb_t) (void* val);
typedef int32 (* ctc_list_cmp_cb_t) (void* val1, void* val2);
*/
STATIC int32 _sys_usw_nh_ecmp_linklist_cmp(sys_nh_info_ecmp_t* val1, sys_nh_info_ecmp_t* val2)
{
    if (val1->ecmp_member_base < val2->ecmp_member_base)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

STATIC int32 _sys_usw_nh_ecmp_fragment_traverse(sys_nh_info_ecmp_t* val, ctc_linklist_t* p_member_list)
{
    if (val->hdr.nh_entry_type == SYS_NH_TYPE_ECMP)
    {
        ctc_listnode_add_sort(p_member_list, val);
    }
    return CTC_E_NONE;
}

int32 sys_usw_nh_ecmp_fragment(uint8 lchip)
{
    int32  ret = CTC_E_NONE;
    int32  free_length;
    uint8  find;
    uint16 alloc_member_num = 0;
    uint16 first_alloc_member_num = 0;
    uint16 max_member_num = 0;
    sys_usw_opf_t opf;

    sys_nh_info_ecmp_t* first_group = NULL;
    sys_nh_info_ecmp_t* next_group = NULL;
    sys_nh_info_ecmp_t* find_group = NULL;

    ctc_listnode_t* f_node;
    ctc_listnode_t* node;

    ctc_linklist_t* p_member_list = ctc_list_create((ctc_list_cmp_cb_t)_sys_usw_nh_ecmp_linklist_cmp, NULL);


    if(!p_member_list)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(&opf, 0, sizeof(opf));

     /* vector traverse;*/
    CTC_ERROR_GOTO(_sys_usw_nh_get_max_ecmp(lchip, &max_member_num), ret, error);
    CTC_ERROR_GOTO(ctc_hash_traverse(g_usw_nh_master[lchip]->nhid_hash, \
                                (hash_traversal_fn)_sys_usw_nh_ecmp_fragment_traverse, p_member_list), ret, error);

    for(f_node = p_member_list->head; f_node && f_node->next; f_node = f_node->next)
    {
        next_group = (sys_nh_info_ecmp_t*)f_node->next->data;
        first_group = f_node->data;

        if (0 == max_member_num)
        {
            first_alloc_member_num = (first_group->mem_num+SYS_ECMP_ALLOC_MEM_STEP-1)/SYS_ECMP_ALLOC_MEM_STEP*SYS_ECMP_ALLOC_MEM_STEP;
        }
        else
        {
            first_alloc_member_num = max_member_num;
        }


        free_length = next_group->ecmp_member_base - first_group->ecmp_member_base - first_alloc_member_num;
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "first group info:\n");
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","tid", first_group->ecmp_group_id);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","max member num", first_group->mem_num);
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","member base", first_group->ecmp_member_base);
        if (free_length == 0)
        {
            continue;
        }
        else if(free_length < 0)
        {
            return CTC_E_UNEXPECT;
        }

        while (1)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n","------free_length  ", free_length);
             /*double check for while*/
            if (free_length <= 0)
            {
                break;
            }

            find = 0;
            for (node = p_member_list->tail; node; node = node->prev)
            {
                if (node == f_node)
                {
                    break;
                }
                find_group = (sys_nh_info_ecmp_t*)node->data;
                if (0 == max_member_num)
                {
                    alloc_member_num = (find_group->mem_num + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP*SYS_ECMP_ALLOC_MEM_STEP;
                }
                else
                {
                    alloc_member_num = max_member_num;
                }

                if ((find_group->type != CTC_NH_ECMP_TYPE_DLB) && (find_group->type != CTC_NH_ECMP_TYPE_XERSPAN) &&
                    (alloc_member_num == free_length) && (find_group->ecmp_member_base > first_group->ecmp_member_base))
                {
                    find = 1;
                    break;
                }
            }

            if (find)
            {
                 /* first update  hardware*/
                uint16 mem_entry_idx = 0;
                uint16 mem_entry_num = 0;
                DsEcmpMember_m member;
                DsEcmpGroup_m  group;

                uint16 new_member_base = first_group->ecmp_member_base + first_alloc_member_num;
                uint32 cmdr = DRV_IOR(DsEcmpMember_t, DRV_ENTRY_FLAG);
                uint32 cmdw = DRV_IOW(DsEcmpMember_t, DRV_ENTRY_FLAG);

                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "The find group info:\n");
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n", "tid", find_group->ecmp_group_id);
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n", "max member num", find_group->mem_num);
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "      %10s: %-10d\n", "member base", find_group->ecmp_member_base);

                mem_entry_num = (find_group->mem_num + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;
                for (mem_entry_idx = 0; mem_entry_idx < mem_entry_num; mem_entry_idx++)
                {
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->ecmp_member_base / 4 + mem_entry_idx, cmdr, &member), ret, error);
                    CTC_ERROR_GOTO(DRV_IOCTL(lchip, new_member_base / 4 + mem_entry_idx, cmdw, &member), ret, error);
                }


                cmdr = DRV_IOR(DsEcmpGroup_t, DRV_ENTRY_FLAG);
                cmdw = DRV_IOW(DsEcmpGroup_t, DRV_ENTRY_FLAG);

                CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->ecmp_group_id, cmdr, &group), ret, error);
                SetDsEcmpGroup(V, memberPtrBase_f, &group, new_member_base / 4);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, find_group->ecmp_group_id, cmdw, &group), ret, error);

                opf.pool_type  = g_usw_nh_master[lchip]->ecmp_member_opf_type;
                opf.pool_index = 0;
                opf.reverse    = 0;

                CTC_ERROR_GOTO(sys_usw_opf_free_offset(lchip, &opf, alloc_member_num, (uint32)find_group->ecmp_member_base), ret, error);
                CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, alloc_member_num, new_member_base), ret, error);

                find_group->ecmp_member_base = new_member_base;
                ctc_listnode_delete_node(p_member_list, node);
                ctc_listnode_add_sort(p_member_list, find_group);
                break;
            }
            else
            {
                free_length -= SYS_ECMP_ALLOC_MEM_STEP;
            }
        }
    }

 error:
    ctc_list_delete(p_member_list);
    return ret;
}


int32
_sys_usw_nh_ecmp_add_dlb_entry(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb)
{
    uint8 lower_bound = 0;
    uint8 upper_bound = 0;
    uint8 threshold = 0;
    uint8 next_group_index = 0;
    uint8 index = 0;
    uint8 member_index = 0;
    uint8 entry_index = 0;
    uint8 mem_group_index = 0;
    uint8 next_entry_index = 0;
    uint8 new_member_id = 0;
    uint16 num_entries = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    num_entries = p_nhdb->mem_num;

    lower_bound = num_entries / (p_nhdb->valid_cnt);
    upper_bound = (num_entries % (p_nhdb->valid_cnt)) ? (lower_bound + 1) : lower_bound;
    threshold = upper_bound;

    new_member_id = p_nhdb->valid_cnt - 1;
    while (p_nhdb->entry_count_array[new_member_id] < lower_bound)
    {
        /* Pick a random member group */
        mem_group_index = sal_rand() % (num_entries/4);
        entry_index = mem_group_index * 4 + sal_rand() % 4;

        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mem_group_index = %d, entry_index:%d  \n", mem_group_index, entry_index);

        index = 0;
        while (index < 4)
        {
            entry_index = mem_group_index * 4 + (entry_index + index) % 4;
            member_index = p_nhdb->member_id_array[entry_index];

            if ((member_index != new_member_id) && (p_nhdb->entry_count_array[member_index] > threshold))
            {
                p_nhdb->member_id_array[entry_index] = new_member_id;
                p_nhdb->entry_count_array[member_index]--;
                p_nhdb->entry_count_array[new_member_id]++;
                break;
            }

            index++;
        }


        if (index == 4)
        {
            /* Either the member of the randomly chosen entry is
             * the same as the new member, or the member is an existing
             * member and its entry count has decreased to threshold.
             * In both cases, find the next entry that contains a
             * member that's not the new member and whose entry count
             * has not decreased to threshold.
             */

            next_group_index = (mem_group_index + 1) % (num_entries/4);
            next_entry_index = next_group_index * 4;

            while (next_group_index != mem_group_index)
            {
                member_index = p_nhdb->member_id_array[next_entry_index];

                if ((member_index != new_member_id) && (p_nhdb->entry_count_array[member_index] > threshold))
                {
                    p_nhdb->member_id_array[next_entry_index] = new_member_id;
                    p_nhdb->entry_count_array[member_index]--;
                    p_nhdb->entry_count_array[new_member_id]++;
                    break;
                }
                else
                {
                    next_entry_index = (next_entry_index + 1) % num_entries;
                    next_group_index = next_entry_index / 4;
                }
            }

            if (next_group_index == mem_group_index)
            {
                /* The entry count of all existing members has decreased
                 * to threshold. The entry count of the new member has
                 * not yet increased to lower_bound. Lower the threshold.
                 */
                threshold--;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_nh_ecmp_dlb_member_chosen(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, uint16 max_entry_num, uint8* chosen_index)
{
    uint8 max_entry_count = 0;
    uint8 member_index = 0;
    uint8 next_index = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!p_nhdb->valid_cnt)
    {
        return CTC_E_NONE;
    }

    max_entry_count = max_entry_num / (p_nhdb->valid_cnt);
    member_index = sal_rand() % (p_nhdb->valid_cnt);

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "member_index = %d \n", member_index);

    if (p_nhdb->entry_count_array[member_index] < max_entry_count)
    {
        p_nhdb->entry_count_array[member_index]++;
        *chosen_index = member_index;
    }
    else
    {
        /* The randomly chosen member has reached the maximum
         * flow set entry count. Choose the next member that
         * has not reached the maximum entry count.
         */
        next_index = (member_index + 1) % (p_nhdb->valid_cnt);
        while (next_index != member_index)
        {
            if (p_nhdb->entry_count_array[next_index] < max_entry_count)
            {
                p_nhdb->entry_count_array[next_index]++;
                *chosen_index = next_index;
                break;
            }
            else
            {
                next_index = (next_index + 1) % (p_nhdb->valid_cnt);
            }
        }

        if (next_index == member_index)
        {
            /* All members have reached the maximum flow set entry count */
            max_entry_count++;
            if (p_nhdb->entry_count_array[member_index] < max_entry_count)
            {
                p_nhdb->entry_count_array[member_index]++;
                *chosen_index = member_index;
            }
            else
            {
                next_index = (member_index + 1) % (p_nhdb->valid_cnt);
                while (next_index != member_index)
                {
                    if (p_nhdb->entry_count_array[next_index] < max_entry_count)
                    {
                        p_nhdb->entry_count_array[next_index]++;
                        *chosen_index = next_index;
                        break;
                    }
                    else
                    {
                        next_index = (next_index + 1) % (p_nhdb->valid_cnt);
                    }
                }

                if (next_index == member_index)
                {
                    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
                    return CTC_E_NOT_EXIST;
                }
            }
        }
    }

    return CTC_E_NONE;
}
int32
sys_usw_nh_update_xerspan_group(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb)
{
    uint8 index = 0;
    uint32 nhid = 0;
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_nh_info_ip_tunnel_t* p_tunnel_nhdb = NULL;
    ds_t ds;
    sys_nexthop_t dsnh_8w;
    uint8 i = 0;
    uint16 hash_num = 0;
    uint16 mem_cnt = 0;
    uint32 dsnh_offset = 0;

    for (i = 7; i >= 1; i--)
    {
        if ((p_nhdb->valid_cnt <= (1 << i)) && (p_nhdb->valid_cnt > (1 << (i - 1))))
        {
            break;
        }
    }

    mem_cnt =  (p_nhdb->valid_cnt > 2)?((1 << i)):p_nhdb->valid_cnt ;
    hash_num = (p_nhdb->valid_cnt > 2)?i: ((p_nhdb->valid_cnt == 2)?1:0);


    for (index = 0 ; index < mem_cnt; index++)
    {
        i  = (index < p_nhdb->valid_cnt )?index:(index - p_nhdb->valid_cnt);
        nhid =  p_nhdb->nh_array[i];
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Update nhid:%d\n", nhid);

        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_info));

        p_tunnel_nhdb = (sys_nh_info_ip_tunnel_t *)p_nh_info;
        if ((CTC_FLAG_ISSET(p_tunnel_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))&&(p_tunnel_nhdb->p_dsl2edit_4w))
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W, (p_tunnel_nhdb->outer_l2_edit_offset), ds));

            if(((p_tunnel_nhdb->outer_l2_edit_offset) % 2) == 1)
            {
                sal_memcpy(ds, (uint8*)ds + sizeof(DsL2Edit3WOuter_m), sizeof(DsL2Edit3WOuter_m));
            }

            /*TBD*/
            CTC_ERROR_RETURN(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W, (p_nhdb->l2edit_offset_base + index*4) , ds));
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l2edit_offset = read:  0x%x, write: 0x%x \n", p_tunnel_nhdb->outer_l2_edit_offset, p_nhdb->l2edit_offset_base + index*4);
        }

        if (CTC_FLAG_ISSET(p_tunnel_nhdb->flag, SYS_NH_IP_TUNNEL_FLAG_IN_V6))
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, p_tunnel_nhdb->dsl3edit_offset, ds));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V4, p_tunnel_nhdb->dsl3edit_offset, ds));
        }
        CTC_ERROR_RETURN(sys_usw_nh_set_asic_table(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, p_nhdb->l3edit_offset_base + index*4, ds));
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3edit_offset = read:  0x%x, write: 0x%x \n",  p_tunnel_nhdb->dsl3edit_offset, p_nhdb->l3edit_offset_base + index*4);
    }

    dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
    dsnh_8w.offset = dsnh_offset;
    dsnh_8w.hash_num =hash_num;
    dsnh_8w.update_type = 1;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dsnh_offset = %d, hash_cnt:%d Valid edit_num = %d\n", dsnh_offset, hash_num, p_nhdb->valid_cnt);
    CTC_ERROR_RETURN(sys_usw_nh_update_asic_table(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, dsnh_offset, &dsnh_8w));

    return CTC_E_NONE;

}

int32
_sys_usw_nh_update_ecmp_member(uint8 lchip,sys_nh_info_ecmp_t* p_nhdb,uint8 change_type,
                                  uint16 mem_idx,
                                  uint16 member_num,
                                 uint16 max_ecmp,
                                 uint32 nhid)
{

    uint16 loop, loop2 = 0;

    uint16 move_num = 0;
    uint32 *move_mem_array;

    move_mem_array = mem_malloc(MEM_NEXTHOP_MODULE, max_ecmp * sizeof(uint32));
    if(!move_mem_array)
    {
      SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "update_ecmp_member nhid:%d failed\n", nhid);
      return CTC_E_NO_MEMORY;
    }

    switch (change_type)
    {
        case SYS_NH_CHANGE_TYPE_FWD_TO_UNROV:
            if (mem_idx < p_nhdb->valid_cnt)
            {
                for (loop = 0,loop2 =0; loop < p_nhdb->valid_cnt; loop++)
                {/*find valid member*/
                    if (p_nhdb->nh_array[loop] != nhid)
                    {
                        move_mem_array[loop2++] = p_nhdb->nh_array[loop];
                    }
                }
                move_num = p_nhdb->valid_cnt - member_num;

                for (loop = 0,loop2 =0; loop < p_nhdb->valid_cnt; loop++)
                {
                    if (loop < move_num )
                        p_nhdb->nh_array[loop] = move_mem_array[loop2++];
                    else
                        p_nhdb->nh_array[loop]  =  nhid;

                }
                p_nhdb->valid_cnt -= member_num;
            }

            break;

        case SYS_NH_CHANGE_TYPE_UNROV_TO_FWD:
        case SYS_NH_CHANGE_TYPE_ADD_DYNTBL:
            if (mem_idx >= p_nhdb->valid_cnt)
            {
                for (loop = p_nhdb->valid_cnt,loop2 =0; loop < p_nhdb->ecmp_cnt; loop++)
                {
                    if (p_nhdb->nh_array[loop]  != nhid)
                    {/*find invalid member*/
                        move_mem_array[loop2++] = p_nhdb->nh_array[loop];
                    }
                }
                for (loop = p_nhdb->valid_cnt, loop2 = 0; loop < p_nhdb->ecmp_cnt; loop++)
                {
                    if (loop < (p_nhdb->valid_cnt + member_num) )
                        p_nhdb->nh_array[loop] =  nhid;
                    else
                        p_nhdb->nh_array[loop] = move_mem_array[loop2++];

                }
                p_nhdb->valid_cnt += member_num;
            }


            break;

        case SYS_NH_CHANGE_TYPE_NH_DELETE:
            for (loop = 0,loop2= 0; loop < p_nhdb->ecmp_cnt; loop++)
            {
                if (p_nhdb->nh_array[loop] != nhid)
                {
                    move_mem_array[loop2++] = p_nhdb->nh_array[loop];
                }
            }
            for (loop = 0,loop2 =0; loop < (p_nhdb->ecmp_cnt - member_num); loop++)
            {
                p_nhdb->nh_array[loop] = move_mem_array[loop2++];
            }
            if (mem_idx < p_nhdb->valid_cnt)
            {
                p_nhdb->valid_cnt -= member_num ;
            }
            p_nhdb->ecmp_cnt -= member_num;

            break;

        case SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR:
            break;
    }

    mem_free(move_mem_array);
    if (p_nhdb->type != CTC_NH_ECMP_TYPE_XERSPAN)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_ecmp_group(lchip, p_nhdb, 0));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_xerspan_group(lchip, p_nhdb));
    }
    return CTC_E_NONE;
}
int32
_sys_usw_nh_update_ecmp_dlb_member(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, uint8 change_type,
                                   uint16 ecmp_mem_idx,
                                   uint16 max_ecmp,
                                   uint32 nhid)
{
    uint8 loop2 = 0;
    uint8  chosen_index = 0;

    if (p_nhdb->type != CTC_NH_ECMP_TYPE_DLB)
    {
       return CTC_E_NONE;
    }
    if ((change_type  == SYS_NH_CHANGE_TYPE_FWD_TO_UNROV
           || change_type == SYS_NH_CHANGE_TYPE_NH_DELETE)
        && (ecmp_mem_idx <= p_nhdb->valid_cnt))
    {
        p_nhdb->entry_count_array[ecmp_mem_idx] = p_nhdb->entry_count_array[p_nhdb->valid_cnt];
        p_nhdb->entry_count_array[p_nhdb->valid_cnt] = 0;

        for (loop2 = 0; loop2 < max_ecmp; loop2++)
        {
            if (p_nhdb->member_id_array[loop2] == ecmp_mem_idx)
            {
                CTC_ERROR_RETURN(_sys_usw_nh_ecmp_dlb_member_chosen(lchip, p_nhdb, max_ecmp, &chosen_index));
                p_nhdb->member_id_array[loop2] = chosen_index;
            }
            else if (p_nhdb->member_id_array[loop2] == p_nhdb->valid_cnt)
            {
                p_nhdb->member_id_array[loop2] = ecmp_mem_idx;
            }
        }

    }
    else if(change_type  == SYS_NH_CHANGE_TYPE_ADD_DYNTBL
           || change_type == SYS_NH_CHANGE_TYPE_UNROV_TO_FWD)
    {
        if (p_nhdb->valid_cnt == 1)
        {
            for (loop2 = 0; loop2 < max_ecmp; loop2++)
            {
                p_nhdb->member_id_array[loop2] = 0;
            }
            p_nhdb->entry_count_array[0] = max_ecmp;
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_nh_ecmp_add_dlb_entry(lchip, p_nhdb));
        }
    }
    return CTC_E_NONE;
}
int32
sys_usw_nh_update_ecmp_member(uint8 lchip, uint32 nhid, uint8 change_type)
{
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_nh_info_ecmp_t* p_nhdb = NULL;
    uint8 ecmp_mem_idx = 0, member_num = 0;
    sys_nh_ref_list_node_t* p_curr = NULL;
    sys_nh_info_dsnh_t nh_info;
    uint8 loop = 0;
    uint16 max_ecmp = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    CTC_ERROR_RETURN(_sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info));
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_info));

    p_curr = p_nh_info->hdr.p_ref_nh_list;


    while (p_curr)
    {
        member_num = 0;
        ecmp_mem_idx = 0;
        p_nhdb = (sys_nh_info_ecmp_t*)p_curr->p_ref_nhinfo;
        if (!p_nhdb || !p_nhdb->nh_array)
        {
            p_curr = p_curr->p_next;
            continue;
        }
        for (loop = 0; loop < p_nhdb->ecmp_cnt; loop++)
        {
            if (p_nhdb->nh_array[loop] == nhid)
            {
                member_num ++;
                ecmp_mem_idx = loop;
            }
        }

        max_ecmp = p_nhdb->mem_num;

        if (member_num == 0)
        {
            p_curr = p_curr->p_next;
            continue;
        }

        if ((SYS_MAP_CTC_GPORT_TO_GCHIP(nh_info.gport) == 0x1f) && (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB ||
        p_nhdb->failover_en))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop]Ecmp Dlb member cannot support linkagg \n");
            return CTC_E_INVALID_CONFIG;
        }

        _sys_usw_nh_update_ecmp_member( lchip, p_nhdb, change_type,
                                              ecmp_mem_idx, member_num, max_ecmp, nhid);


        if(p_nhdb->type == CTC_NH_ECMP_TYPE_DLB && (member_num ==1))
         {
           _sys_usw_nh_update_ecmp_dlb_member(lchip, p_nhdb, change_type,
                                         ecmp_mem_idx,  max_ecmp, nhid);
         }

        if (p_nhdb->type != CTC_NH_ECMP_TYPE_XERSPAN)
        {
            CTC_ERROR_RETURN(sys_usw_nh_update_ecmp_group(lchip, p_nhdb, 0));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_update_xerspan_group(lchip, p_nhdb));
        }

        p_curr = p_curr->p_next;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_add_ecmp_member(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, sys_nh_info_com_t* p_nh_info)
{
    sys_nh_ref_list_node_t* p_curr = NULL;
    sys_nh_ref_list_node_t* p_nh_ref_list_node = NULL;
    uint8 find_flag = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_nhdb);
    CTC_PTR_VALID_CHECK(p_nh_info);

    p_curr = p_nh_info->hdr.p_ref_nh_list;

    while (p_curr)
    {
        if ((sys_nh_info_ecmp_t*)p_curr->p_ref_nhinfo == p_nhdb)
        {
            find_flag = 1;
            break;
        }

        p_curr = p_curr->p_next;
    }

    if (find_flag == 0)
    {
        p_nh_ref_list_node = (sys_nh_ref_list_node_t*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(sys_nh_ref_list_node_t));
        if (p_nh_ref_list_node == NULL)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }
        p_nh_ref_list_node->p_ref_nhinfo = (sys_nh_info_com_t*)p_nhdb;
        p_nh_ref_list_node->p_next = p_nh_info->hdr.p_ref_nh_list;

        p_nh_info->hdr.p_ref_nh_list = p_nh_ref_list_node;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_delete_ecmp_member(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, sys_nh_info_com_t* p_nh_info)
{
    sys_nh_ref_list_node_t* p_curr = NULL;
    sys_nh_ref_list_node_t* p_prev = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_nhdb);
    CTC_PTR_VALID_CHECK(p_nh_info);

    p_curr = p_nh_info->hdr.p_ref_nh_list;

    while (p_curr)
    {
        if ((sys_nh_info_ecmp_t*)p_curr->p_ref_nhinfo == p_nhdb)
        {
            if (NULL == p_prev)
            /*Remove first node*/
            {
                p_nh_info->hdr.p_ref_nh_list = p_curr->p_next;
            }
            else
            {
                p_prev->p_next = p_curr->p_next;
            }

            mem_free(p_curr);
            return CTC_E_NONE;
        }

        p_prev = p_curr;
        p_curr = p_curr->p_next;
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_ecmp_group(uint8 lchip, sys_nh_info_ecmp_t* p_nhdb, uint8 is_create)
{
    sys_nh_info_com_t* p_nh_info = NULL;
    uint8 ecmp_mem_idx = 0;
    uint8 dest_channel = 0;
    uint16 max_ecmp = 0;
    uint32 ecmp_mem_id = 0;
    sys_fwd_t dsfwd;
    sys_dsecmp_member_t sys_ecmp_member;
    sys_dsecmp_group_t sys_ecmp_group;
    sys_dsecmp_rr_t sys_ecmp_rr;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_nhdb);

    if (p_nhdb->failover_en && (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
       return CTC_E_NOT_SUPPORT;

    }

    sal_memset(&sys_ecmp_member, 0, sizeof(sys_ecmp_member));
    sal_memset(&sys_ecmp_group, 0, sizeof(sys_ecmp_group));
    sal_memset(&sys_ecmp_rr, 0, sizeof(sys_ecmp_rr));
    sal_memset(&dsfwd, 0, sizeof(dsfwd));

    CTC_ERROR_RETURN(_sys_usw_nh_get_max_ecmp(lchip, &max_ecmp));

    for (ecmp_mem_idx = 0; ecmp_mem_idx < p_nhdb->mem_num; ecmp_mem_idx++)
    {
        if (p_nhdb->valid_cnt == 0)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, CTC_NH_RESERVED_NHID_FOR_DROP, (sys_nh_info_com_t**)&p_nh_info));
            if (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB)
            {
               p_nhdb->member_id_array[ecmp_mem_idx] = 0xFF;
            }
        }
        else
        {

            if (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB)
            {
               if (!is_create )
               {
                   ecmp_mem_id = p_nhdb->member_id_array[ecmp_mem_idx];
               }
               else
               {
                   ecmp_mem_id = ecmp_mem_idx % (p_nhdb->valid_cnt);
                   p_nhdb->member_id_array[ecmp_mem_idx] = ecmp_mem_id;
                   p_nhdb->entry_count_array[ecmp_mem_id]++;
               }
               SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " INFO: ecmp_mem_idx:%d(), mem_id=%d, nh_id=%d, entry_count=%d\n",
                        ecmp_mem_idx, p_nhdb->member_id_array[ecmp_mem_idx],  p_nhdb->nh_array[ecmp_mem_id], p_nhdb->entry_count_array[ecmp_mem_id]);
            }
            else
            {
              ecmp_mem_id = ecmp_mem_idx % (p_nhdb->valid_cnt);
            }
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nhdb->nh_array[ecmp_mem_id], (sys_nh_info_com_t**)&p_nh_info));
        }
        sys_usw_nh_get_entry_dsfwd(lchip, p_nh_info->hdr.dsfwd_info.dsfwd_offset, (void*)&dsfwd);
        /* for mux-demux port, need get by func, do it later ? */
        dest_channel = ((dsfwd.dest_map>>8 & 0x1) << 6) | (dsfwd.dest_map & 0x3F);
        if (ecmp_mem_idx%4 == 0)
        {
            sal_memset(&sys_ecmp_member, 0, sizeof(sys_ecmp_member));
            sys_ecmp_member.valid0 = 1;
            sys_ecmp_member.dsfwdptr0 = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
            sys_ecmp_member.dest_channel0 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 1)
        {
            sys_ecmp_member.valid1 = 1;
            sys_ecmp_member.dsfwdptr1 = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
            sys_ecmp_member.dest_channel1 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 2)
        {
            sys_ecmp_member.valid2 = 1;
            sys_ecmp_member.dsfwdptr2 = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
            sys_ecmp_member.dest_channel2 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 3)
        {
            sys_ecmp_member.valid3 = 1;
            sys_ecmp_member.dsfwdptr3 = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
            sys_ecmp_member.dest_channel3 = dest_channel;
        }

        if (((ecmp_mem_idx+1) == p_nhdb->mem_num) ||(ecmp_mem_idx%4 == 3))
        {
            /* write ecmp member */
            sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_ECMP_MEMBER,
                (p_nhdb->ecmp_member_base + ecmp_mem_idx)/4, &sys_ecmp_member);
        }
    }

    sys_ecmp_group.dlb_en = (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB) ? 1 : 0;
    sys_ecmp_group.failover_en = p_nhdb->failover_en ? 1 : 0;
    sys_ecmp_group.rr_en = ((p_nhdb->type == CTC_NH_ECMP_TYPE_RR) || (p_nhdb->type == CTC_NH_ECMP_TYPE_RANDOM_RR)) ? 1 : 0;
    sys_ecmp_group.member_base = p_nhdb->ecmp_member_base / 4;
    sys_ecmp_group.member_num = (p_nhdb->valid_cnt > 0) ? ((p_nhdb->type == CTC_NH_ECMP_TYPE_DLB) ? (max_ecmp-1) : (p_nhdb->valid_cnt-1)) : 0;
    sys_ecmp_group.stats_valid = p_nhdb->valid_cnt ? p_nhdb->stats_valid : 0;

    if (sys_ecmp_group.rr_en)
    {
        sys_ecmp_rr.random_rr_en = p_nhdb->random_rr_en;
        sys_ecmp_rr.member_num = p_nhdb->valid_cnt-1;
        CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_ECMP_RR_COUNT,
                p_nhdb->ecmp_group_id%MCHIP_CAP(SYS_CAP_NH_ECMP_RR_GROUP_NUM), &sys_ecmp_rr));
    }

    /* write ecmp group */
    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_ECMP_GROUP,
                p_nhdb->ecmp_group_id, &sys_ecmp_group));

    return CTC_E_NONE;
}

int32
sys_usw_nh_update_ecmp_interface_group(uint8 lchip, sys_l3if_ecmp_if_t* p_group)
{
    uint8 valid_nh_idx = 0;
    uint8 ecmp_mem_idx = 0;
    uint8 dest_channel = 0;
    uint16 max_ecmp = 0;
    uint32 dsfwd_offset = 0;
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_fwd_t dsfwd;
    sys_dsecmp_member_t sys_ecmp_member;
    sys_dsecmp_group_t sys_ecmp_group;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_group);

    if (p_group->failover_en && (p_group->ecmp_group_type == CTC_NH_ECMP_TYPE_DLB))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
		return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&sys_ecmp_member, 0, sizeof(sys_ecmp_member));
    sal_memset(&sys_ecmp_group, 0, sizeof(sys_ecmp_group));
    sal_memset(&dsfwd, 0, sizeof(dsfwd));

    max_ecmp = SYS_USW_MAX_DEFAULT_ECMP_NUM;

    for (ecmp_mem_idx = 0; ecmp_mem_idx < max_ecmp; ecmp_mem_idx++)
    {
        if (p_group->intf_count == 0)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, CTC_NH_RESERVED_NHID_FOR_DROP, (sys_nh_info_com_t**)&p_nh_info));
            dsfwd_offset = p_nh_info->hdr.dsfwd_info.dsfwd_offset;
        }
        else
        {
            dsfwd_offset = p_group->dsfwd_offset[valid_nh_idx];
            if ((++valid_nh_idx) == p_group->intf_count)
            {
                valid_nh_idx = 0;
            }
        }
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " INFO: ecmp_mem_idx:%d()\n", ecmp_mem_idx);

        sys_usw_nh_get_entry_dsfwd(lchip, dsfwd_offset, (void*)&dsfwd);
        /* for mux-demux port, need get by func, do it later ? */
        dest_channel = ((dsfwd.dest_map>>8 & 0x1) << 6) | (dsfwd.dest_map & 0x3F);

        if (ecmp_mem_idx%4 == 0)
        {
            sal_memset(&sys_ecmp_member, 0, sizeof(sys_ecmp_member));
            sys_ecmp_member.valid0 = 1;
            sys_ecmp_member.dsfwdptr0 = dsfwd_offset;
            sys_ecmp_member.dest_channel0 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 1)
        {
            sys_ecmp_member.valid1 = 1;
            sys_ecmp_member.dsfwdptr1 = dsfwd_offset;
            sys_ecmp_member.dest_channel1 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 2)
        {
            sys_ecmp_member.valid2 = 1;
            sys_ecmp_member.dsfwdptr2 = dsfwd_offset;
            sys_ecmp_member.dest_channel2 = dest_channel;
        }
        else if (ecmp_mem_idx%4 == 3)
        {
            sys_ecmp_member.valid3 = 1;
            sys_ecmp_member.dsfwdptr3 = dsfwd_offset;
            sys_ecmp_member.dest_channel3 = dest_channel;

            /* write ecmp member */
            sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_ECMP_MEMBER,
                (p_group->ecmp_member_base + ecmp_mem_idx)/4, &sys_ecmp_member);
        }
    }

    sys_ecmp_group.dlb_en = (p_group->ecmp_group_type == CTC_NH_ECMP_TYPE_DLB) ? 1 : 0;
    sys_ecmp_group.failover_en = p_group->failover_en ? 1 : 0;
    sys_ecmp_group.member_base = p_group->ecmp_member_base / 4;
    sys_ecmp_group.member_num = (p_group->intf_count > 0) ? (p_group->intf_count-1) : 0;
    sys_ecmp_group.stats_valid = (p_group->stats_id > 0) ? 1 : 0;

    CTC_ERROR_RETURN(sys_usw_nh_write_asic_table(lchip, SYS_NH_ENTRY_TYPE_ECMP_GROUP,
                p_group->hw_group_id, &sys_ecmp_group));

    return CTC_E_NONE;
}


int32
_sys_usw_nh_ecmp_update_member_nh(uint8 lchip, uint32 nhid, sys_nh_info_com_t * p_com_db)
{
    sys_nh_param_com_t nh_param;
    sal_memset(&nh_param, 0, sizeof(sys_nh_param_com_t));
    nh_param.hdr.nhid = nhid;
    nh_param.hdr.nh_param_type = p_com_db->hdr.nh_entry_type;
    nh_param.hdr.change_type  = SYS_NH_CHANGE_TYPE_ADD_DYNTBL;


    if (p_com_db->hdr.nh_entry_type == SYS_NH_TYPE_IPUC)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_ipuc_cb(lchip, (sys_nh_info_com_t*)p_com_db, (sys_nh_param_com_t*)&nh_param));

    }
    else if(p_com_db->hdr.nh_entry_type == SYS_NH_TYPE_MPLS)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_mpls_cb(lchip, (sys_nh_info_com_t*)p_com_db, (sys_nh_param_com_t*)&nh_param));


    }
    else if(p_com_db->hdr.nh_entry_type == SYS_NH_TYPE_IP_TUNNEL)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_ip_tunnel_cb(lchip, (sys_nh_info_com_t*)p_com_db, (sys_nh_param_com_t*)&nh_param));
    }
    else if(p_com_db->hdr.nh_entry_type == SYS_NH_TYPE_MISC)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_misc_cb(lchip, (sys_nh_info_com_t*)p_com_db, (sys_nh_param_com_t*)&nh_param));
    }
    else if(p_com_db->hdr.nh_entry_type == SYS_NH_TYPE_TRILL)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_trill_cb(lchip, (sys_nh_info_com_t*)p_com_db, (sys_nh_param_com_t*)&nh_param));
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_create_normal_ecmp(uint8 lchip, sys_nh_param_com_t* p_com_nh_param, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_ecmp_t* p_nh_param = NULL;
    ctc_nh_ecmp_nh_param_t* p_nh_ecmp_param = NULL;
    sys_nh_info_ecmp_t* p_nhdb = NULL;
    sys_usw_opf_t opf;
    uint16 max_ecmp = 0;
    uint16 member_num = 0;
    uint8  nh_array_size = 0;
    uint8  loop = 0, loop2 = 0;
    uint8  valid_cnt = 0, unrov_nh_cnt = 0;
    uint32* p_nh_array = NULL;
    uint32* p_unrov_nh_array = NULL;
    uint32* p_valid_nh_array = NULL;

    uint32 offset = 0;
    int32  ret = CTC_E_NONE;

    sys_nh_info_com_t* p_nh_info = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_com_nh_param);
    CTC_PTR_VALID_CHECK(p_com_db);

    sal_memset(&opf, 0, sizeof(opf));

    p_nh_param = (sys_nh_param_ecmp_t*)(p_com_nh_param);
    p_nhdb = (sys_nh_info_ecmp_t*)(p_com_db);
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_ECMP;
    p_nh_ecmp_param = p_nh_param->p_ecmp_param;


    if (p_nh_ecmp_param->failover_en && (p_nh_ecmp_param->type == CTC_NH_ECMP_TYPE_DLB))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ECMP not supported Failover and Dlb!\n");
        return CTC_E_NOT_SUPPORT;
    }

    CTC_ERROR_RETURN(_sys_usw_nh_get_max_ecmp(lchip, &max_ecmp));

    p_nh_array = (uint32*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(uint32)*SYS_USW_MAX_ECPN);
    p_unrov_nh_array = (uint32*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(uint32)*SYS_USW_MAX_ECPN);
    p_valid_nh_array = (uint32*)mem_malloc(MEM_NEXTHOP_MODULE, sizeof(uint32)*SYS_USW_MAX_ECPN);
    if ((NULL == p_nh_array) || (NULL == p_unrov_nh_array) || (NULL == p_valid_nh_array))
    {
        ret = CTC_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_nh_array, 0, sizeof(uint32)*SYS_USW_MAX_ECPN);
    sal_memset(p_unrov_nh_array, 0, sizeof(uint32)*SYS_USW_MAX_ECPN);
    sal_memset(p_valid_nh_array, 0, sizeof(uint32)*SYS_USW_MAX_ECPN);

    /*1.only static ecmp support repeated Member */
    for (loop = 0; loop < p_nh_ecmp_param->nh_num; loop++)
    {
        for (loop2 = 0; loop2 < valid_cnt; loop2++)
        {
            if (p_nh_ecmp_param->nhid[loop] == p_nh_array[loop2]
                && (p_nh_ecmp_param->type !=CTC_NH_ECMP_TYPE_STATIC) )
            {
                ret = CTC_E_INVALID_CONFIG;
                goto out;
            }
        }
       p_nh_array[valid_cnt] = p_nh_ecmp_param->nhid[loop];
       valid_cnt++;

    }

    /* group member num is flex */
    if (max_ecmp == 0)
    {
        if (p_nh_ecmp_param->type == CTC_NH_ECMP_TYPE_DLB) /* dlb not support group member num is flex */
        {
            ret = CTC_E_INVALID_CONFIG;
            goto out;
        }

        member_num = p_nh_ecmp_param->member_num;
        if ((member_num < 2) || (member_num > SYS_USW_MAX_ECPN))
        {
            ret = CTC_E_INVALID_PARAM;
            goto out;
        }

        if (p_nh_ecmp_param->nh_num > member_num)
        {
            ret = CTC_E_NH_EXCEED_MAX_ECMP_NUM;
            goto out;
        }

        p_nhdb->mem_num = member_num;
        member_num = (member_num+SYS_ECMP_ALLOC_MEM_STEP-1)/SYS_ECMP_ALLOC_MEM_STEP*SYS_ECMP_ALLOC_MEM_STEP;
    }
    else  /* group member num is fixed */
    {
        member_num = max_ecmp;
        if (p_nh_ecmp_param->nh_num > member_num)
        {
            ret = CTC_E_NH_EXCEED_MAX_ECMP_NUM;
            goto out;
        }

        p_nhdb->mem_num = member_num;
    }

    /*2.Judge  Member is FWD or unrsv */
    valid_cnt = 0;
    unrov_nh_cnt = 0;

    for (loop = 0; loop < p_nh_ecmp_param->nh_num; loop++)
    {
        sys_nh_info_dsnh_t nh_info;
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

        CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_ecmp_param->nhid[loop], (sys_nh_info_com_t**)&p_nh_info), ret, out);
        if (p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_IPUC
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_MPLS
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_IP_TUNNEL
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_MISC
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_DROP
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_TRILL)
        {
                SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
                ret = CTC_E_INVALID_CONFIG;
                goto out;
        }

        CTC_ERROR_GOTO(_sys_usw_nh_get_nhinfo(lchip, p_nh_ecmp_param->nhid[loop], &nh_info), ret, out);
        if ((SYS_MAP_CTC_GPORT_TO_GCHIP(nh_info.gport) == 0x1f) && (p_nh_ecmp_param->type == CTC_NH_ECMP_TYPE_DLB ||
            p_nh_ecmp_param->failover_en))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop]Ecmp Dlb member cannot support linkagg \n");
            ret = CTC_E_INVALID_CONFIG;
            goto out;
        }

        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV)
            && !CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
        {
            _sys_usw_nh_ecmp_update_member_nh(lchip, p_nh_ecmp_param->nhid[loop],(sys_nh_info_com_t*)p_nh_info);
        }

        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            p_valid_nh_array[valid_cnt++] = p_nh_ecmp_param->nhid[loop];

        }
        else
        {
            p_unrov_nh_array[unrov_nh_cnt++] = p_nh_ecmp_param->nhid[loop];
        }
    }


    if ((g_usw_nh_master[lchip]->cur_ecmp_cnt + 1) > g_usw_nh_master[lchip]->max_ecmp_group_num)
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ECMP Group have No resource in ASIC \n");
        ret = CTC_E_NO_RESOURCE;
        goto out;
    }

    /*3.Save DB */
    p_nhdb->valid_cnt = valid_cnt;
    p_nhdb->ecmp_cnt = valid_cnt + unrov_nh_cnt;
    p_nhdb->ecmp_nh_id = p_nh_param->hdr.nhid;
    p_nhdb->type = p_nh_ecmp_param->type;
    p_nhdb->failover_en = p_nh_ecmp_param->failover_en;
    if ((p_nhdb->type == CTC_NH_ECMP_TYPE_RR) || (p_nhdb->type == CTC_NH_ECMP_TYPE_RANDOM_RR))
    {
        /* for rr, only support groupId 1-15 */

        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_group_opf_type;
        opf.pool_index = 0;
        opf.reverse    = 0;

        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset), ret, out);
        if (offset >= MCHIP_CAP(SYS_CAP_NH_ECMP_RR_GROUP_NUM))
        {
            sys_usw_opf_free_offset(lchip, &opf, 1, offset);
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " ECMP RR Group num exceed max number,ecmp_group_id:%d \n", offset);
            ret = CTC_E_NO_RESOURCE;
            goto out;
        }

        p_nhdb->ecmp_group_id = offset;

        if (p_nhdb->type == CTC_NH_ECMP_TYPE_RANDOM_RR)
        {
            p_nhdb->random_rr_en = 1;
        }

    }
    else
    {
        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_group_opf_type;
        opf.pool_index = 0;
        opf.reverse    = 1;

        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset), ret, out);
        p_nhdb->ecmp_group_id = offset;
    }
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " INFO: alloc ecmp_group_id:%d\n", p_nhdb->ecmp_group_id);

    opf.pool_type  = g_usw_nh_master[lchip]->ecmp_member_opf_type;
    opf.pool_index = 0;
    opf.reverse    = 0;

    ret = sys_usw_opf_alloc_offset(lchip, &opf, member_num, &offset);
    if ((max_ecmp == 0) && (ret == CTC_E_NO_RESOURCE))   /* only flex mode support fragment process */
    {
        CTC_ERROR_GOTO(sys_usw_nh_ecmp_fragment(lchip), ret, error0);
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, member_num, &offset), ret, error0);
    }
    else if(ret < 0)
    {
        goto error0;
    }

    p_nhdb->ecmp_member_base = offset;

    /* process stats */
    if (p_nh_ecmp_param->stats_id)
    {
        p_nhdb->stats_valid = 1;
        p_nhdb->stats_id = p_nh_ecmp_param->stats_id;
        ret = sys_usw_flow_stats_add_ecmp_stats(lchip, p_nh_ecmp_param->stats_id, p_nhdb->ecmp_group_id);
        if (ret < 0)
        {
            goto error1;
        }
    }

    if (0 != p_nhdb->ecmp_cnt)
    {
        nh_array_size = (p_nhdb->ecmp_cnt + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;
        nh_array_size = nh_array_size * SYS_ECMP_ALLOC_MEM_STEP;

        p_nhdb->nh_array = mem_malloc(MEM_NEXTHOP_MODULE, nh_array_size * sizeof(uint32));

        if (!p_nhdb->nh_array)
        {
            ret = CTC_E_NO_MEMORY;
            goto error2;
        }

        sal_memcpy(&p_nhdb->nh_array[0], &p_valid_nh_array[0], valid_cnt * sizeof(uint32));
        if (unrov_nh_cnt)
        {
            sal_memcpy(&p_nhdb->nh_array[valid_cnt], &p_unrov_nh_array[0], unrov_nh_cnt * sizeof(uint32));
        }
    }

    if (p_nhdb->type == CTC_NH_ECMP_TYPE_DLB)
    {
      p_nhdb->member_id_array = mem_malloc(MEM_NEXTHOP_MODULE, max_ecmp * sizeof(uint8));
      if (!p_nhdb->member_id_array)
      {
          ret = CTC_E_NO_MEMORY;
          goto error3;
      }
      sal_memset(p_nhdb->member_id_array, 0xff, max_ecmp * sizeof(uint8));

      p_nhdb->entry_count_array = mem_malloc(MEM_NEXTHOP_MODULE, max_ecmp * sizeof(uint8));
      if (!p_nhdb->entry_count_array)
      {
          ret = CTC_E_NO_MEMORY;
          goto error4;
      }
      sal_memset(p_nhdb->entry_count_array, 0, max_ecmp * sizeof(uint8));
    }

    /*4.Create ECMP group */
    CTC_ERROR_GOTO(sys_usw_nh_update_ecmp_group(lchip, p_nhdb, 1), ret, error5);

    /*5.Add ECMP Nhid to member nexthop's linklist*/
    for (loop = 0; loop < p_nhdb->ecmp_cnt; loop++)
    {
        ret = sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nhdb->nh_array[loop], (sys_nh_info_com_t**)&p_nh_info);
        if (ret < 0)
        {
            goto error5;
        }
        ret = sys_usw_nh_add_ecmp_member(lchip, p_nhdb, p_nh_info);
        if (ret < 0)
        {
            for (loop2 = 0; loop2 < loop; loop2++)
            {
                /*rollback already add nhid member*/
                sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_param->p_ecmp_param->nhid[loop2], (sys_nh_info_com_t**)&p_nh_info);
                sys_usw_nh_delete_ecmp_member(lchip, p_nhdb, p_nh_info);
            }
            goto error5;
        }
    }

    ret = CTC_E_NONE;
    goto out;

error5:
     mem_free(p_nhdb->entry_count_array);
error4:
    mem_free(p_nhdb->member_id_array);
error3:
    mem_free(p_nhdb->nh_array);
error2:
    if (p_nh_ecmp_param->stats_id)
    {
        sys_usw_flow_stats_remove_ecmp_stats(lchip, p_nh_ecmp_param->stats_id, p_nhdb->ecmp_group_id);
    }
error1:
    /* free ecmp groupId ? */
    {
        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_member_opf_type;
        opf.pool_index = 0;
        opf.reverse    = 0;
        sys_usw_opf_free_offset(lchip, &opf, member_num, (uint32)p_nhdb->ecmp_member_base);
    }
error0:
    opf.pool_type = g_usw_nh_master[lchip]->ecmp_group_opf_type;
    opf.pool_index = 0;
    sys_usw_opf_free_offset(lchip, &opf, 1, (uint32)(p_nhdb->ecmp_group_id));

out:
    if (p_nh_array)
    {
        mem_free(p_nh_array);
    }
    if (p_unrov_nh_array)
    {
        mem_free(p_unrov_nh_array);
    }
    if (p_valid_nh_array)
    {
        mem_free(p_valid_nh_array);
    }

    return ret;
}

int32
sys_usw_nh_update_ecmp_cb(uint8 lchip, sys_nh_info_com_t* p_com_db, sys_nh_param_com_t* p_com_nh_param)
{
    sys_nh_param_ecmp_t* p_nh_param = NULL;
    ctc_nh_ecmp_nh_param_t* p_nh_ecmp_param = NULL;
    sys_nh_info_ecmp_t* p_nhdb = NULL;
    uint8 nh_array_size = 0, nh_array_size2 = 0;
    uint8 loop = 0, loop2 = 0;
    uint8 valid_cnt = 0;
    uint8 exist = 0;
    uint8  unrov_nh_cnt = 0;
    uint32 unrov_nh_array[SYS_USW_MAX_ECPN] = {0};
    uint32 valid_nh_array[SYS_USW_MAX_ECPN] = {0};
    sys_nh_info_com_t* p_nh_info = NULL;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_com_db);
    CTC_PTR_VALID_CHECK(p_com_nh_param);

    p_nh_param = (sys_nh_param_ecmp_t*)(p_com_nh_param);
    p_nhdb = (sys_nh_info_ecmp_t*)(p_com_db);
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_ECMP;
    p_nh_ecmp_param = p_nh_param->p_ecmp_param;

    if (p_nh_ecmp_param->nh_num == 0 || (p_nh_ecmp_param->nh_num > 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memcpy(&valid_nh_array[0], &p_nhdb->nh_array[0], p_nhdb->valid_cnt * sizeof(uint32));
    sal_memcpy(&unrov_nh_array[0], &p_nhdb->nh_array[p_nhdb->valid_cnt], (p_nhdb->ecmp_cnt - p_nhdb->valid_cnt) * sizeof(uint32));
    unrov_nh_cnt = (p_nhdb->ecmp_cnt - p_nhdb->valid_cnt);
    valid_cnt = p_nhdb->valid_cnt;


    /*only support update one member*/
    if (p_nh_ecmp_param->upd_type == CTC_NH_ECMP_ADD_MEMBER)
    {
        sys_nh_info_dsnh_t nh_info;
        sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

        if ((p_nhdb->ecmp_cnt + p_nh_ecmp_param->nh_num) > p_nhdb->mem_num)
        {
            return CTC_E_INVALID_PARAM;
        }

        for (loop2 = 0; loop2 < p_nhdb->ecmp_cnt; loop2++)
        {
            if (p_nh_ecmp_param->nhid[0] == p_nhdb->nh_array[loop2]
                && (p_nhdb->type != CTC_NH_ECMP_TYPE_STATIC))
            {
                /*only staic ecmp support repeated Member*/
                return CTC_E_INVALID_CONFIG;
            }
        }

        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_ecmp_param->nhid[loop], (sys_nh_info_com_t**)&p_nh_info));

        if (p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_IPUC
            && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_MPLS
        && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_IP_TUNNEL
        && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_MISC
        && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_DROP
        && p_nh_info->hdr.nh_entry_type != SYS_NH_TYPE_TRILL)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
            return CTC_E_INVALID_CONFIG;
        }


        CTC_ERROR_RETURN(_sys_usw_nh_get_nhinfo(lchip, p_nh_ecmp_param->nhid[loop], &nh_info));
        if ((SYS_MAP_CTC_GPORT_TO_GCHIP(nh_info.gport) == 0x1f) && (p_nh_ecmp_param->type == CTC_NH_ECMP_TYPE_DLB ||
            p_nhdb->failover_en))
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop]Ecmp Dlb member cannot support linkagg \n");
            return CTC_E_INVALID_CONFIG;
        }
           if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV)
            && !CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_DSFWD))
        {
            CTC_ERROR_RETURN(_sys_usw_nh_ecmp_update_member_nh(lchip, p_nh_ecmp_param->nhid[loop], (sys_nh_info_com_t*)p_nh_info));
        }

        if (!CTC_FLAG_ISSET(p_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
        {
            valid_nh_array[valid_cnt++] = p_nh_ecmp_param->nhid[loop];
            p_nhdb->valid_cnt = valid_cnt;
            CTC_ERROR_RETURN(_sys_usw_nh_update_ecmp_dlb_member( lchip, p_nhdb,SYS_NH_CHANGE_TYPE_UNROV_TO_FWD,
                               0, p_nhdb->mem_num,p_nh_ecmp_param->nhid[0]));

        }
        else
        {
            unrov_nh_array[unrov_nh_cnt++] = p_nh_ecmp_param->nhid[loop];
        }

    }
    else if (p_nh_ecmp_param->upd_type == CTC_NH_ECMP_REMOVE_MEMBER)
    {
        for (loop2 = 0; loop2 < p_nhdb->ecmp_cnt; loop2++)
        {
            if (p_nh_ecmp_param->nhid[0] == p_nhdb->nh_array[loop2])
            {
                exist = 1;
                break;
            }
        }
        if (exist == 0)
        {
           SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] ECMP member not exist! \n");
           return CTC_E_NOT_EXIST;
        }
        p_nhdb->nh_array[loop2]  = CTC_MAX_UINT32_VALUE;

        for (loop = 0; loop < p_nhdb->ecmp_cnt; loop++)
        {
            if (p_nhdb->nh_array[loop] != CTC_MAX_UINT32_VALUE)
            {
                continue;
            }

            if (loop < p_nhdb->valid_cnt)
            {
                valid_nh_array[loop] = valid_nh_array[valid_cnt - 1];
                valid_cnt--;
                p_nhdb->valid_cnt = valid_cnt;
                CTC_ERROR_RETURN(_sys_usw_nh_update_ecmp_dlb_member( lchip, p_nhdb,SYS_NH_CHANGE_TYPE_NH_DELETE,
                                    loop, p_nhdb->mem_num,p_nh_ecmp_param->nhid[0]));
            }
            else
            {
                unrov_nh_array[loop - valid_cnt] = unrov_nh_array[unrov_nh_cnt - 1];
                unrov_nh_cnt--;
            }
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    /*old nh_array_size*/
    nh_array_size = (p_nhdb->ecmp_cnt + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;

    /*new nh_array_size*/
    nh_array_size2 = (valid_cnt + unrov_nh_cnt + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;
    nh_array_size2  = (nh_array_size2 == 0) ? 1 : nh_array_size2;

    if (nh_array_size != nh_array_size2)
    {
        mem_free(p_nhdb->nh_array);

        nh_array_size2 = nh_array_size2 * SYS_ECMP_ALLOC_MEM_STEP;
        p_nhdb->nh_array = mem_malloc(MEM_NEXTHOP_MODULE, nh_array_size2 * sizeof(uint32));

        if (!p_nhdb->nh_array)
        {
            SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
    }

    p_nhdb->valid_cnt = valid_cnt;
    p_nhdb->ecmp_cnt = valid_cnt + unrov_nh_cnt;
    sal_memcpy(&p_nhdb->nh_array[0], &valid_nh_array[0], valid_cnt * sizeof(uint32));
    if (unrov_nh_cnt)
    {
        sal_memcpy(&p_nhdb->nh_array[valid_cnt], &unrov_nh_array[0], unrov_nh_cnt * sizeof(uint32));
    }

    if (p_nhdb->type != CTC_NH_ECMP_TYPE_XERSPAN)
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_ecmp_group(lchip, p_nhdb, 0));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_update_xerspan_group(lchip, p_nhdb));
    }

    if (p_nh_ecmp_param->upd_type == CTC_NH_ECMP_ADD_MEMBER)
    {
        for (loop = 0; loop < p_nh_ecmp_param->nh_num; loop++)
        {
            if (CTC_MAX_UINT32_VALUE != p_nh_ecmp_param->nhid[loop])
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_ecmp_param->nhid[loop], (sys_nh_info_com_t**)&p_nh_info));
                CTC_ERROR_RETURN(sys_usw_nh_add_ecmp_member(lchip, p_nhdb, p_nh_info));
            }
        }
    }
    else if (p_nh_ecmp_param->upd_type == CTC_NH_ECMP_REMOVE_MEMBER)
    {
        for (loop = 0; loop < p_nh_ecmp_param->nh_num; loop++)
        {
            if (CTC_MAX_UINT32_VALUE != p_nh_ecmp_param->nhid[loop])
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_ecmp_param->nhid[loop], (sys_nh_info_com_t**)&p_nh_info));
                CTC_ERROR_RETURN(sys_usw_nh_delete_ecmp_member(lchip, p_nhdb, p_nh_info));
            }
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_nh_delete_ecmp_cb(uint8 lchip, sys_nh_info_com_t* p_com_db, uint32* p_nhid)
{
    sys_nh_info_ecmp_t* p_nhdb = NULL;
    uint8 loop = 0;
    uint16 alloc_member_num = 0;
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_usw_opf_t opf;
    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_com_db);
    CTC_PTR_VALID_CHECK(p_nhid);

    sal_memset(&opf, 0, sizeof(opf));

    p_nhdb = (sys_nh_info_ecmp_t*)(p_com_db);


    if (p_nhdb->type != CTC_NH_ECMP_TYPE_XERSPAN )
    {
        CTC_ERROR_RETURN(_sys_usw_nh_get_max_ecmp(lchip, &alloc_member_num));
        if (0 == alloc_member_num)
        {
            alloc_member_num = (p_nhdb->mem_num+SYS_ECMP_ALLOC_MEM_STEP-1)/SYS_ECMP_ALLOC_MEM_STEP*SYS_ECMP_ALLOC_MEM_STEP;
        }

        if (p_nhdb->stats_valid)
        {
            sys_usw_flow_stats_remove_ecmp_stats(lchip, p_nhdb->stats_id, p_nhdb->ecmp_group_id);
        }

        p_nhdb->valid_cnt = 0;
        p_nhdb->type = CTC_NH_ECMP_TYPE_STATIC;
        sys_usw_nh_update_ecmp_group(lchip, p_nhdb, 0);

        /* free ecmp groupId ? */
        opf.pool_type = g_usw_nh_master[lchip]->ecmp_group_opf_type;
        opf.pool_index = 0;

        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, (uint32)(p_nhdb->ecmp_group_id)));
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " INFO: free ecmp_group_id:%d\n", p_nhdb->ecmp_group_id);

        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_member_opf_type;
        opf.pool_index = 0;
        opf.reverse    = 0;

        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, alloc_member_num, (uint32)p_nhdb->ecmp_member_base));
    }
    else
    {
        uint8 hash_num = 16;
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, 1, p_nhdb->hdr.dsfwd_info.dsnh_offset));

        /*1. Free new dsl3edit  entry*/
        CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip,
                                                       SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, hash_num,
                                                       p_nhdb->l3edit_offset_base));

        if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {

            /*1. Free new dsl2edit  entry*/
            CTC_ERROR_RETURN(sys_usw_nh_offset_free(lchip,
                                                           SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, hash_num*4,
                                                           p_nhdb->l2edit_offset_base));
        }

    }

    for (loop = 0; loop < p_nhdb->ecmp_cnt; loop++)
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nhdb->nh_array[loop], (sys_nh_info_com_t**)&p_nh_info));
        sys_usw_nh_delete_ecmp_member(lchip, p_nhdb, p_nh_info);
    }

    mem_free(p_nhdb->nh_array);

    if(p_nhdb->member_id_array)
    {
        mem_free(p_nhdb->member_id_array);
    }

    if(p_nhdb->entry_count_array)
    {
        mem_free(p_nhdb->entry_count_array);
    }
    return CTC_E_NONE;
}


int32
sys_usw_nh_create_rspan_ecmp(uint8 lchip, sys_nh_param_com_t* p_com_nh_para,
                                  sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_ecmp_t* p_nh_param = NULL;
    sys_nh_info_ecmp_t* p_nhdb = NULL;
    uint8 hash_num = 0;
    uint32 nhid = 0;
    sys_nh_info_ip_tunnel_t* p_tunnel_nhdb = NULL;
    sys_nh_info_com_t* p_nh_info = NULL;
    sys_nh_param_dsnh_t dsnh_param;
    ctc_nh_ecmp_nh_param_t ecmp_nh_param;
    sys_nh_param_ecmp_t sys_ecmp;
    uint8 index = 0;
    int32 ret = 0;
    uint8 sub_index = 0;
    uint8  nh_array_size = 0;

    SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /* 1. sanity check & init */
    CTC_PTR_VALID_CHECK(p_com_nh_para);
    CTC_PTR_VALID_CHECK(p_com_db);
    p_nh_param = (sys_nh_param_ecmp_t*)p_com_nh_para;
    p_nhdb = (sys_nh_info_ecmp_t*)p_com_db;
    CTC_PTR_VALID_CHECK(p_nh_param->p_ecmp_param);
    sal_memset(&dsnh_param, 0, sizeof(sys_nh_param_dsnh_t));

    p_nhdb->type = CTC_NH_ECMP_TYPE_XERSPAN;

    hash_num = 16;
    CTC_ERROR_RETURN(sys_usw_nh_offset_alloc(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, 1, &p_nhdb->hdr.dsfwd_info.dsnh_offset));

    dsnh_param.dsnh_type = SYS_NH_PARAM_DSNH_TYPE_XERSPAN;
    dsnh_param.dsnh_offset = p_nhdb->hdr.dsfwd_info.dsnh_offset;
    p_nhdb->hdr.nh_entry_type = SYS_NH_TYPE_ECMP;

    nhid =  p_nh_param->p_ecmp_param->nhid[0];
    CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo_by_nhid(lchip, nhid, (sys_nh_info_com_t**)&p_nh_info), ret, error1);
    p_tunnel_nhdb = (sys_nh_info_ip_tunnel_t *)p_nh_info;

    if (CTC_FLAG_ISSET(p_tunnel_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_IS_UNROV))
    {
        SYS_NH_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Current nexthop is unresolved nexthop \n");
		ret = CTC_E_INVALID_CONFIG;
        goto error1;
    }

    /*1. Allocate new dsl3edit mpls entry*/
    CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip,
                                                    SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, hash_num,
                                                    &dsnh_param.l3edit_ptr), ret, error1);
    p_nhdb->l3edit_offset_base = dsnh_param.l3edit_ptr;


    if (CTC_FLAG_ISSET(p_tunnel_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
    {

        /*1. Allocate new dsl2edit mpls entry*/
        CTC_ERROR_GOTO(sys_usw_nh_offset_alloc(lchip,
                                                        SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W, hash_num*2,
                                                        &dsnh_param.l2edit_ptr), ret, error2);
        p_nhdb->l2edit_offset_base = dsnh_param.l2edit_ptr;
        CTC_SET_FLAG(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT);
    }


    dsnh_param.hash_num = 0;
    dsnh_param.dest_vlan_ptr = p_tunnel_nhdb->dest_vlan_ptr;
    dsnh_param.span_id = p_tunnel_nhdb->span_id;
    p_nhdb->gport = p_tunnel_nhdb->gport;
    p_nhdb->mem_num = hash_num;
    if (CTC_FLAG_ISSET(p_tunnel_nhdb->flag, SYS_NH_IP_TUNNEL_KEEP_IGS_TS))
    {
        CTC_SET_FLAG(dsnh_param.flag, SYS_NH_PARAM_FLAG_ERSPAN_KEEP_IGS_TS);
    }

    CTC_ERROR_GOTO(sys_usw_nh_write_entry_dsnh8w(lchip, &dsnh_param), ret, error3);

    nh_array_size = (p_nh_param->p_ecmp_param->nh_num + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;
    nh_array_size = nh_array_size * SYS_ECMP_ALLOC_MEM_STEP;
    p_nhdb->nh_array = mem_malloc(MEM_NEXTHOP_MODULE,  nh_array_size * sizeof(uint32));

    sal_memcpy(&ecmp_nh_param, p_nh_param->p_ecmp_param, sizeof(ctc_nh_ecmp_nh_param_t));
    sal_memcpy(&(sys_ecmp.hdr), &(p_nh_param->hdr), sizeof(sys_nh_param_hdr_t));
    for (index = 0; index < p_nh_param->p_ecmp_param->nh_num; index++)
    {
        ecmp_nh_param.nh_num = 1;
        ecmp_nh_param.nhid[0] = p_nh_param->p_ecmp_param->nhid[index];
        sys_ecmp.p_ecmp_param = &ecmp_nh_param;
        ret = sys_usw_nh_update_ecmp_cb(lchip, p_com_db, (sys_nh_param_com_t*)&sys_ecmp );
        if (ret < 0)
        {
            for (sub_index = 0; sub_index < index; sub_index++)
            {
                /*rollback already add nhid member*/
                sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_param->p_ecmp_param->nhid[sub_index], (sys_nh_info_com_t**)&p_nh_info);
                sys_usw_nh_delete_ecmp_member(lchip, p_nhdb, p_nh_info);
            }
            goto error4;
        }
    }
    return CTC_E_NONE;
error4:
    if (p_nhdb->nh_array)
    {
        mem_free(p_nhdb->nh_array);
    }
    if (p_nhdb->entry_count_array)
    {
        mem_free(p_nhdb->entry_count_array);
    }
error3:
    if (CTC_FLAG_ISSET(p_nhdb->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
    {
        sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_6W, 16*2, p_nhdb->l2edit_offset_base);
    }
error2:
    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, 16, p_nhdb->l3edit_offset_base);
error1:
    sys_usw_nh_offset_free(lchip, SYS_NH_ENTRY_TYPE_NEXTHOP_8W, 1, p_nhdb->hdr.dsfwd_info.dsnh_offset);
    return ret;
}



int32
sys_usw_nh_create_ecmp_cb(uint8 lchip, sys_nh_param_com_t* p_com_nh_param, sys_nh_info_com_t* p_com_db)
{
    sys_nh_param_ecmp_t* p_nh_param = NULL;
    ctc_nh_ecmp_nh_param_t* p_nh_ecmp_param = NULL;

    CTC_PTR_VALID_CHECK(p_com_nh_param);
    CTC_PTR_VALID_CHECK(p_com_db);

    p_nh_param = (sys_nh_param_ecmp_t*)(p_com_nh_param);

    p_nh_ecmp_param = p_nh_param->p_ecmp_param;

    if (p_nh_ecmp_param->type != CTC_NH_ECMP_TYPE_XERSPAN)
    {
        CTC_ERROR_RETURN(sys_usw_nh_create_normal_ecmp(lchip, p_com_nh_param, p_com_db));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_create_rspan_ecmp(lchip, p_com_nh_param, p_com_db));
    }

    return CTC_E_NONE;
}


int32
sys_usw_nh_wb_restore_ecmp_nhinfo(uint8 lchip,  sys_nh_info_com_t* p_nh_info_com, sys_wb_nh_info_com_t *p_wb_nh_info)

{
    sys_nh_info_ecmp_t  *p_nh_info = (sys_nh_info_ecmp_t  *)p_nh_info_com;
    uint8 nh_array_size = 0;
	uint8 loop =0;
    sys_nh_info_com_t* p_member_nh_info = NULL;

    p_nh_info->ecmp_cnt = p_wb_nh_info->info.ecmp.ecmp_cnt;
    p_nh_info->valid_cnt = p_wb_nh_info->info.ecmp.valid_cnt;
    p_nh_info->type = p_wb_nh_info->info.ecmp.type;
    p_nh_info->failover_en = p_wb_nh_info->info.ecmp.failover_en;
    p_nh_info->ecmp_nh_id = p_wb_nh_info->info.ecmp.ecmp_nh_id;
    p_nh_info->mem_num = p_wb_nh_info->info.ecmp.mem_num;

    p_nh_info->ecmp_group_id = p_wb_nh_info->info.ecmp.ecmp_group_id ;
    p_nh_info->random_rr_en = p_wb_nh_info->info.ecmp.random_rr_en ;
    p_nh_info->stats_valid =  p_wb_nh_info->info.ecmp.stats_valid;
    p_nh_info->gport = p_wb_nh_info->info.ecmp.gport ;
    p_nh_info->ecmp_member_base = p_wb_nh_info->info.ecmp.ecmp_member_base;
	p_nh_info->l3edit_offset_base = p_wb_nh_info->info.ecmp.l3edit_offset_base ;
    p_nh_info->l2edit_offset_base = p_wb_nh_info->info.ecmp.l2edit_offset_base;
    p_nh_info->stats_id = p_wb_nh_info->info.ecmp.stats_id;

    if (p_nh_info->ecmp_cnt)
    {
    	nh_array_size = (p_nh_info->ecmp_cnt + SYS_ECMP_ALLOC_MEM_STEP - 1) / SYS_ECMP_ALLOC_MEM_STEP;
        nh_array_size = nh_array_size * SYS_ECMP_ALLOC_MEM_STEP;
    	p_nh_info->nh_array = mem_malloc(MEM_NEXTHOP_MODULE, nh_array_size * sizeof(uint32));
        if (NULL == p_nh_info->nh_array)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memcpy(&p_nh_info->nh_array[0],&p_wb_nh_info->info.ecmp.nh_array[0], p_nh_info->ecmp_cnt*sizeof(uint32));
    }

    if (p_nh_info->type == CTC_NH_ECMP_TYPE_DLB)
    {
        p_nh_info->member_id_array = mem_malloc(MEM_NEXTHOP_MODULE, g_usw_nh_master[lchip]->max_ecmp * sizeof(uint8));
        sal_memcpy(&p_nh_info->member_id_array[0],&p_wb_nh_info->info.ecmp.member_id_array[0], g_usw_nh_master[lchip]->max_ecmp*sizeof(uint8));

        p_nh_info->entry_count_array = mem_malloc(MEM_NEXTHOP_MODULE, g_usw_nh_master[lchip]->max_ecmp * sizeof(uint8));
        sal_memcpy(&p_nh_info->entry_count_array[0],&p_wb_nh_info->info.ecmp.entry_count_array[0], g_usw_nh_master[lchip]->max_ecmp*sizeof(uint8));
    }

    if (p_nh_info->type == CTC_NH_ECMP_TYPE_XERSPAN )
    {
        uint8 hash_num = 16;

        sys_usw_nh_offset_alloc_from_position(lchip,
                                                     SYS_NH_ENTRY_TYPE_NEXTHOP_8W, 1,
                                                     p_nh_info->hdr.dsfwd_info.dsnh_offset);

        sys_usw_nh_offset_alloc_from_position(lchip,
                                                     SYS_NH_ENTRY_TYPE_L3EDIT_TUNNEL_V6, hash_num,
                                                     p_nh_info->l3edit_offset_base);

        if (CTC_FLAG_ISSET(p_wb_nh_info->hdr.nh_entry_flags, SYS_NH_INFO_FLAG_HAVE_L2EDIT))
        {


            sys_usw_nh_offset_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W,
                                                         hash_num * 4, p_nh_info->l2edit_offset_base);

        }
	    sys_usw_nh_update_xerspan_group(lchip, p_nh_info);
    }
    else
    {
        sys_usw_opf_t opf;
        uint16 max_ecmp = 0;

        CTC_ERROR_RETURN(_sys_usw_nh_get_max_ecmp(lchip, &max_ecmp));
        if (0 == max_ecmp)
        {
            max_ecmp = (p_nh_info->mem_num+SYS_ECMP_ALLOC_MEM_STEP-1)/SYS_ECMP_ALLOC_MEM_STEP*SYS_ECMP_ALLOC_MEM_STEP;
        }
         sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_group_opf_type;
        opf.pool_index = 0;
        opf.reverse = ((CTC_NH_ECMP_TYPE_RR == p_nh_info->type) || (CTC_NH_ECMP_TYPE_RANDOM_RR == p_nh_info->type)) ? 0 : 1;

        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_nh_info->ecmp_group_id));

        sal_memset(&opf, 0, sizeof(opf));
        opf.pool_type  = g_usw_nh_master[lchip]->ecmp_member_opf_type;
        opf.pool_index = 0;

        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, max_ecmp, p_nh_info->ecmp_member_base));

        sys_usw_nh_update_ecmp_group(lchip, p_nh_info, 0);
    }

     /*5.Add ECMP Nhid to member nexthop's linklist*/
     for (loop = 0; loop < p_nh_info->ecmp_cnt; loop++)
     {
         sys_usw_nh_get_nhinfo_by_nhid(lchip, p_nh_info->nh_array[loop], (sys_nh_info_com_t**)&p_member_nh_info);

         sys_usw_nh_add_ecmp_member(lchip, p_nh_info, p_member_nh_info);
     }

     return CTC_E_NONE;
}













