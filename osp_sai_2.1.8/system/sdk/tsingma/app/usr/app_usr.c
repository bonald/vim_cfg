/**
 @file app_usr.c

 @date 2010-06-09

 @version v2.0

 User define code
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "api/include/ctc_api.h"
#include "api/include/ctcs_api.h"
#include "app_usr.h"

struct app_usr_traverse_data_s
{
    uint32 nhid;                 /*if not 0, traverse mcast member match nhid*/
    uint16 tunnel_id;            /*if not 0, traverse mcast member match tunnel id, used for mpls nexthop*/
    uint8  lchip;
    uint8  rsv;
};
typedef struct app_usr_traverse_data_s app_usr_traverse_data_t;

struct app_usr_member_node_s
{
    uint16 group_id;
    uint32 nhid;
};
typedef struct app_usr_member_node_s app_usr_member_node_t;

extern uint8 g_ctcs_api_en;
extern uint8 g_api_lchip;

/**************************************************/
/*    Following pseudocode is used for system adapter reference   */
/**************************************************/
/*
void
ctc_app_link_monitor_thread(void* para)
{
    uint16 lport = 0;
    uint16 gport = 0;
    uint8 gchip = 0;
    uint32 mac_en = 0;
    bool is_up = 0;
    uint32 is_detect = 0;
    uint8 lchip = 0;
    uint8 lchip_num = 0;
    int32 ret = 0;

    ctc_get_local_chip_num(&lchip_num);

    while(1)
    {
        for (lchip = 0; lchip < lchip_num; lchip++)
        {
            ctc_get_gchip_id(lchip, &gchip);

            for (lport = 0; lport < CTC_MAX_PHY_PORT; lport++)
            {
                mac_en = 0;
                is_up = 0;

                gport = CTC_MAP_LPORT_TO_GPORT(gchip, lport);

                //Step1: get signal detect
                ret = ctc_port_get_property(gport, CTC_PORT_PROP_SIGNAL_DETECT, &is_detect);
                if (ret < 0)
                {
                    continue;
                }

                //Step2: If signal detect, need enable mac
                if (is_detect)
                {
                    ret = ctc_port_set_mac_en(gport, TRUE);
                    if (ret < 0)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }

                //Step3: Get mac linkup
                //If using ext phy, should get phy link status, else f using Mac Pcs directly get link from mac pcs
                ret = ctc_port_get_mac_link_up(gport, &is_up);
                if (ret < 0)
                {
                    continue;
                }

                Step4: If using link down interrupt, do interrupt enable
                ctc_port_set_property(gport,CTC_PORT_PROP_LINK_INTRRUPT_EN,TRUE);

                Step5: If mac link up, do port enable
                if (is_detect)
                {
                    ret = ctc_port_set_property(gport,CTC_PORT_PROP_PORT_EN,TRUE);
                    if (ret < 0)
                    {
                        continue;
                    }

                    //Do other thing
                }
            }
        }

        sal_task_sleep(100);
    }

    return;
}
*/

ctc_linklist_t* p_member_list = NULL;

int32
_ctc_app_update_mcast_member_cb(uint16 mc_grp_id, ctc_mcast_nh_param_member_t* p_member, void* user_data)
{
    app_usr_traverse_data_t* p_usr_data = NULL;
    uint8 is_match= 0;
    app_usr_member_node_t* p_member_node;

    if (!p_member)
    {
        return CTC_E_NONE;
    }

    p_usr_data = (app_usr_traverse_data_t*)user_data;
    /*match nexthop id or tunnel id*/
    if (p_usr_data)
    {
        ctc_nh_info_t nh_info;
        if ((p_member->member_type == CTC_NH_PARAM_MEM_LOCAL_WITH_NH) && (p_usr_data->nhid == p_member->ref_nhid) && p_usr_data->nhid)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Match NHID member Mcast Grp:%u, member nh:%u\n", mc_grp_id, p_member->ref_nhid);
            is_match = 1;
        }

        if ((p_member->member_type == CTC_NH_PARAM_MEM_LOCAL_WITH_NH) && p_usr_data->tunnel_id)
        {
            sal_memset(&nh_info, 0, sizeof(ctc_nh_info_t));
            ctc_nh_get_nh_info(p_member->ref_nhid, &nh_info);
            if (nh_info.tunnel_id == p_usr_data->tunnel_id)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Match tunnel member Mcast Grp:%u, tunnel_id:%u, member nh:%u\n",
                    mc_grp_id, nh_info.tunnel_id, p_member->ref_nhid);
                is_match = 1;
            }
        }

        if (is_match)
        {
            p_member_node = sal_malloc(sizeof(app_usr_member_node_t));
            if (p_member_node == NULL)
            {
                return CTC_E_NO_MEMORY;
            }

            p_member_node->group_id = mc_grp_id;
            p_member_node->nhid = p_member->ref_nhid;
            ctc_listnode_add_tail(p_member_list, p_member_node);

        }
    }
    return CTC_E_NONE;
}

int32
ctc_app_update_mcast_member(uint8 lchip, uint32 nhid, uint16 tunnel_id)
{
    ctc_nh_mcast_traverse_t mcast_travers;
    ctc_nh_mcast_traverse_fn fn;
    app_usr_traverse_data_t usr_data;
    int32 ret = 0;
    ctc_listnode_t * listnode  = NULL;
    app_usr_member_node_t* p_member;
    ctc_mcast_nh_param_group_t mcast_param;
    uint8 gchip = 0;
    uint32 mcast_nh = 0;

    if (nhid && tunnel_id)
    {
        /*only support match nhid or tunnel id*/
        return CTC_E_INVALID_PARAM;
    }

    p_member_list = ctc_list_new();
    if (!p_member_list)
    {
        return CTC_E_INVALID_CONFIG;
    }

    sal_memset(&usr_data, 0, sizeof(app_usr_traverse_data_t));
    sal_memset(&mcast_travers, 0, sizeof(ctc_nh_mcast_traverse_t));
    usr_data.lchip = lchip;
    usr_data.nhid = nhid;
    usr_data.tunnel_id = tunnel_id;
    mcast_travers.lchip_id = lchip;
    mcast_travers.user_data = &usr_data;
    fn = _ctc_app_update_mcast_member_cb;

    if(g_ctcs_api_en)
    {
        ret = ctcs_nh_traverse_mcast(g_api_lchip, fn, &mcast_travers);
    }
    else
    {
        ret = ctc_nh_traverse_mcast(fn, &mcast_travers);
    }
    if (ret)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Mcast traverse fail, ret:%d\n", ret);
        goto error;
    }

    listnode = CTC_LISTHEAD(p_member_list);
    while(listnode)
    {
        p_member = listnode->data;
        sal_memset(&mcast_param, 0, sizeof(ctc_mcast_nh_param_group_t));
        mcast_param.mc_grp_id = p_member->group_id;
        mcast_param.opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;
        mcast_param.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        mcast_param.mem_info.ref_nhid = p_member->nhid;

        if(g_ctcs_api_en)
        {
            ctcs_get_gchip_id(lchip, &gchip);
            mcast_param.mem_info.gchip_id = gchip;
            ctcs_nh_get_mcast_nh(g_api_lchip, p_member->group_id, &mcast_nh);
            ctcs_nh_update_mcast(g_api_lchip, mcast_nh, &mcast_param);
        }
        else
        {
            ctcs_get_gchip_id(lchip, &gchip);
            mcast_param.mem_info.gchip_id = gchip;
            ctc_nh_get_mcast_nh(p_member->group_id, &mcast_nh);
            ctc_nh_update_mcast(mcast_nh, &mcast_param);
        }

        mcast_param.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;
        if(g_ctcs_api_en)
        {
            ctcs_nh_update_mcast(g_api_lchip, mcast_nh, &mcast_param);
        }
        else
        {
            ctc_nh_update_mcast(mcast_nh, &mcast_param);
        }

        sal_free(listnode->data);
        CTC_NEXTNODE(listnode);
    }
error:
    ctc_list_delete(p_member_list);

    return CTC_E_NONE;
}

extern int32 ctc_register_cli_exec_cb(void* cb);
extern int cli_com_source_file(ctc_cmd_element_t *, ctc_vti_t *, int, char **);
extern ctc_cmd_element_t cli_com_source_file_cmd;
extern ctc_vti_t *g_ctc_vti;

int32 ctc_cli_exec()
{
    char *argv[1];
    sal_file_t fp = NULL;
    argv[0] = (char*)START_UP_CONFIG;

    if (0 == SDK_WORK_PLATFORM)
    {

        fp = sal_fopen((char*)START_UP_CONFIG, "r");
        if (NULL == fp)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Use default FFE config\n");
        }
        else
        {
            cli_com_source_file(&cli_com_source_file_cmd, g_ctc_vti, 1, argv);
            sal_fclose(fp);
        }
    }

    return CTC_E_NONE;
}



int32
ctc_app_usr_init(void)
{

    ctc_register_cli_exec_cb(ctc_cli_exec);



    return CTC_E_NONE;
}


