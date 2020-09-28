/**
 @file ctc_stress_test_cli.c

 @date 2017-03-21

 @version v1.0

 The file applies cli to test sdk module of SDK
*/

#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_stress_test_cli.h"

struct ctc_cli_stress_test_oam_s
{
    uint8  is_ipv6;
    uint8  is_p2mp;
    uint8  maid_type;
    uint32 entry_num;

    ctc_oam_lmep_t* p_lmep;
    ctc_oam_rmep_t* p_rmep;
    ctc_oam_rmep_t** p_rmep_p2mp;
    ctc_oam_mip_t* p_mip;
    ctc_oam_maid_t* p_maid;
};
typedef struct ctc_cli_stress_test_oam_s ctc_cli_stress_test_oam_t;

ctc_cli_stress_test_oam_t  *g_ctc_stress_oam;


int32
_ctc_cli_stress_oam_mep(ctc_oam_lmep_t* lmep, ctc_oam_rmep_t* rmep, ctc_cli_stress_test_oam_t* oam_info, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    int32 ret_rmep = CLI_SUCCESS;
    uint32 num_temp = 0;
    uint32 loop = 0,valid_count =0;
    uint32 loop_rmep = 0;
    uint32 rmep_num_per_lmep = 0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};

    ctc_mpls_ilm_t ilm_info;

    uint8 is_random = 0;
    uint8 is_random_all = 0;
    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    oam_info->entry_num = (oam_info->entry_num > 4096) ? 4096 : oam_info->entry_num;
    if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type)||(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep->key.mep_type))
    {
        sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
        ilm_info.nh_id = 2;
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV4;
        ilm_info.type = CTC_MPLS_LABEL_TYPE_NORMAL;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        if (!g_ctc_stress_oam->p_lmep)
        {
            g_ctc_stress_oam->p_lmep = (ctc_oam_lmep_t*)sal_malloc(sizeof(ctc_oam_lmep_t)*4096);
            if (!g_ctc_stress_oam->p_lmep)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
        }
        sal_memset(g_ctc_stress_oam->p_lmep, 0, sizeof(ctc_oam_lmep_t)*4096);


        if ((!g_ctc_stress_oam->p_rmep)&&(!oam_info->is_p2mp))
        {
            g_ctc_stress_oam->p_rmep = (ctc_oam_rmep_t*)sal_malloc(sizeof(ctc_oam_rmep_t)*4096);
            if (!g_ctc_stress_oam->p_rmep)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
            sal_memset(g_ctc_stress_oam->p_rmep, 0, sizeof(ctc_oam_rmep_t)*4096);
        }
        else if((!g_ctc_stress_oam->p_rmep)&&(oam_info->is_p2mp))
        {
            g_ctc_stress_oam->p_rmep_p2mp = (ctc_oam_rmep_t**)sal_malloc(sizeof(ctc_oam_rmep_t*)*4096);
            if (!g_ctc_stress_oam->p_rmep_p2mp)
            {
                ctc_cli_out("Memory alloc error\n");
                sal_free(g_ctc_stress_oam->p_lmep);
                return CLI_SUCCESS;
            }
            sal_memset(g_ctc_stress_oam->p_rmep_p2mp, 0, sizeof(ctc_oam_rmep_t*)*4096);
            for (loop = 0; loop < 4096; loop++)
            {
                g_ctc_stress_oam->p_rmep_p2mp[loop] = (ctc_oam_rmep_t*)sal_malloc(sizeof(ctc_oam_rmep_t)*4); /*P2MP per lmep max with 4 rmep*/
                if (!g_ctc_stress_oam->p_rmep_p2mp[loop])
                {
                    ctc_cli_out("Memory alloc error\n");
                    sal_free(g_ctc_stress_oam->p_lmep);
                    num_temp = loop;
                    for (loop = 0; loop < num_temp; loop++)
                    {
                        sal_free(g_ctc_stress_oam->p_rmep_p2mp[loop]);
                    }
                    sal_free(g_ctc_stress_oam->p_rmep_p2mp);
                    return CLI_SUCCESS;
                }
            }
        }

    }


    for (loop = 0; loop < oam_info->entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if ((CTC_OAM_MEP_TYPE_IP_BFD == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == lmep->key.mep_type))
            {
                lmep->key.u.bfd.discr = (is_random)? sal_rand():loop;
                rmep->key.u.bfd.discr = lmep->key.u.bfd.discr;
                if (oam_info->is_ipv6)
                {
                    lmep->u.bfd_lmep.ipv6_sa[0]= (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_sa[1] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_sa[2] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_sa[3] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_da[0] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_da[1] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_da[2] = (is_random)? sal_rand():loop;
                    lmep->u.bfd_lmep.ipv6_da[3] = (is_random)? sal_rand():loop;
                }

            }
            else if((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep->key.mep_type)||(CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type))
            {
                lmep->key.u.tp.label = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
                rmep->key.u.tp.label = lmep->key.u.tp.label;
                ilm_info.label = lmep->key.u.tp.label;
            }
            else if(CTC_OAM_MEP_TYPE_ETH_Y1731 == lmep->key.mep_type)
            {
                lmep->key.u.eth.gport = is_random ?(sal_rand() % 255): loop;
                lmep->key.u.eth.vlan_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
                lmep->key.u.eth.cvlan_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096 );
                lmep->key.u.eth.l2vpn_oam_id = (is_random)?((sal_rand() % 6119) + 1):(loop % 6120);
                lmep->key.flag = (is_random)?(sal_rand() % 32):(loop % 32);

                rmep->key.u.eth.gport = lmep->key.u.eth.gport;
                rmep->key.u.eth.vlan_id = lmep->key.u.eth.vlan_id;
                rmep->key.u.eth.cvlan_id = lmep->key.u.eth.cvlan_id;
                rmep->key.u.eth.l2vpn_oam_id = lmep->key.u.eth.l2vpn_oam_id;
            }
            if ((CTC_OAM_MEP_TYPE_ETH_Y1731 == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type))
            {
                lmep->u.y1731_lmep.mep_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
            }

            /*AD random*/
            if (is_random_all)
            {
                if ((CTC_OAM_MEP_TYPE_IP_BFD == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == lmep->key.mep_type)
                    || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep->key.mep_type) )
                {
                    lmep->u.bfd_lmep.bfd_src_port = (sal_rand() % 1000) + 49100;
                    lmep->u.bfd_lmep.desired_min_tx_interval = sal_rand() % 1023 + 1 ;
                    lmep->u.bfd_lmep.ip4_da = sal_rand();
                    lmep->u.bfd_lmep.ip4_sa = sal_rand();
                    lmep->u.bfd_lmep.lm_cos = sal_rand() % 8;
                    lmep->u.bfd_lmep.local_state = sal_rand() % 4;
                    lmep->u.bfd_lmep.local_detect_mult =
                    lmep->u.bfd_lmep.ttl = (sal_rand() % 255) + 1;
                    lmep->u.bfd_lmep.tx_cos_exp = sal_rand() % 8;
                    lmep->u.bfd_lmep.tx_csf_type = sal_rand() % 5;
                    lmep->u.bfd_lmep.lm_cos_type = sal_rand() % 4;
                    lmep->u.bfd_lmep.nhid = (sal_rand() % 16380) + 2;

                    rmep->u.bfd_rmep.remote_detect_mult = sal_rand() % 16;
                    rmep->u.bfd_rmep.required_min_rx_interval = sal_rand() % 1023 + 1 ;
                    rmep->u.bfd_rmep.remote_diag = sal_rand() % 32;
                    rmep->u.bfd_rmep.remote_discr = sal_rand();
                }
                if ((CTC_OAM_MEP_TYPE_ETH_Y1731 == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type))
                {
                    lmep->u.y1731_lmep.mpls_ttl = (sal_rand() % 255) + 1;
                    lmep->u.y1731_lmep.nhid = (sal_rand() % 16380) + 2;
                    lmep->u.y1731_lmep.ccm_interval = sal_rand() % 8;
                    lmep->u.y1731_lmep.lm_type = sal_rand() % 3;
                    lmep->u.y1731_lmep.lm_cos_type = sal_rand() % 4;
                    lmep->u.y1731_lmep.lm_cos = sal_rand() % 8;
                    lmep->u.y1731_lmep.tx_csf_type = sal_rand() % 5;
                    lmep->u.y1731_lmep.tx_cos_exp = sal_rand() % 8;
                    lmep->u.y1731_lmep.tpid_index = sal_rand() % 4;

                }


            }

            if (g_ctcs_api_en)
            {
                if ((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type))
                {
                    ret = ctcs_mpls_add_ilm(g_api_lchip, &ilm_info);
                }
                ret = ctcs_oam_add_lmep(g_api_lchip, lmep);
            }
            else
            {
                if ((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep->key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep->key.mep_type))
                {
                    ret = ctc_mpls_add_ilm(&ilm_info);
                }
                ret = ctc_oam_add_lmep(lmep);
            }
            sal_memcpy(g_ctc_stress_oam->p_lmep + loop, lmep, sizeof(ctc_oam_lmep_t));
            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Add lmep %d normal\n", loop + 1);

            }
            else
            {
                ctc_cli_out("Add lmep %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

            if (!oam_info->is_p2mp)  /*P2P Mode*/
            {
                rmep->u.y1731_rmep.rmep_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
                if (g_ctcs_api_en)
                {
                    ret_rmep = ctcs_oam_add_rmep(g_api_lchip, rmep);
                }
                else
                {
                    ret_rmep = ctc_oam_add_rmep(rmep);
                }
                sal_memcpy(g_ctc_stress_oam->p_rmep + loop, rmep, sizeof(ctc_oam_rmep_t));
                if (ret_rmep == CTC_E_NONE)
                {
                    ctc_cli_out("Add rmep %d normal\n", loop + 1);
                }
                else
                {
                    ctc_cli_out("Add rmep %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
                }
            }
            else  /*P2MP Mode*/
            {
                rmep_num_per_lmep = (sal_rand()%4 + 1);   /*rmep num per lmep is random, and max value is 4*/
                for (loop_rmep = 0; loop_rmep < rmep_num_per_lmep; loop_rmep++)
                {
                    rmep->u.y1731_rmep.rmep_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
                    if (g_ctcs_api_en)
                    {
                        ret_rmep = ctcs_oam_add_rmep(g_api_lchip, rmep);
                    }
                    else
                    {
                        ret_rmep = ctc_oam_add_rmep(rmep);
                    }

                    sal_memcpy(&g_ctc_stress_oam->p_rmep_p2mp[loop][loop_rmep], rmep, sizeof(ctc_oam_rmep_t));
                    if (ret_rmep == CTC_E_NONE)
                    {
                        ctc_cli_out("Add lmep %d rmep %d normal\n", loop + 1, loop_rmep + 1);
                    }
                    else
                    {
                        ctc_cli_out("Add lmep %d rmep %d error: %s\n", loop + 1, loop_rmep + 1, ctc_get_error_desc(ret_rmep));
                    }

                }
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            /*p2p remove rmep*/
            if (!oam_info->is_p2mp)
            {
                if (!g_ctc_stress_oam->p_rmep)
                {
                    ctc_cli_out("No rmep resource need to remove \n");
                    return CLI_SUCCESS;
                }
                if (NULL == g_ctc_stress_oam->p_rmep + loop)
                {
                    continue;
                }
                sal_memcpy(rmep, g_ctc_stress_oam->p_rmep + loop, sizeof(ctc_oam_rmep_t));

                if (g_ctcs_api_en)
                {
                    ret_rmep = ctcs_oam_remove_rmep(g_api_lchip, rmep);
                }
                else
                {
                    ret_rmep = ctc_oam_remove_rmep(rmep);
                }

                if (ret_rmep == CTC_E_NONE)
                {
                    ctc_cli_out("Remove rmep %d normal\n", loop + 1);
                }
                else
                {
                    ctc_cli_out("Remove rmep %d error: %s\n", loop + 1, ctc_get_error_desc(ret_rmep));
                }
            }
            else/*p2mp remove rmep*/
            {

                for (loop_rmep = 0; loop_rmep < 4; loop_rmep++)
                {
                    if (!g_ctc_stress_oam->p_rmep_p2mp[loop])
                    {
                        ctc_cli_out("lmep %d , No rmep resource need to remove \n", loop_rmep + 1);
                        return CLI_SUCCESS;
                    }
                    if (NULL == g_ctc_stress_oam->p_rmep_p2mp[loop] + loop_rmep)
                    {
                        continue;
                    }
                    sal_memcpy(rmep, g_ctc_stress_oam->p_rmep_p2mp[loop] + loop_rmep, sizeof(ctc_oam_rmep_t));

                    if (g_ctcs_api_en)
                    {
                        ret_rmep = ctcs_oam_remove_rmep(g_api_lchip, rmep);
                    }
                    else
                    {
                        ret_rmep = ctc_oam_remove_rmep(rmep);
                    }

                    if (ret_rmep == CTC_E_NONE)
                    {
                        ctc_cli_out("Remove lmep %d rmep %d normal\n", loop + 1 , loop_rmep + 1);
                    }
                    else
                    {
                        ctc_cli_out("Remove lmep %d rmep %d error: %s\n", loop + 1, loop_rmep + 1, ctc_get_error_desc(ret_rmep));
                    }
                }


            }

            if (!g_ctc_stress_oam->p_lmep)
            {
                ctc_cli_out("No lmep resource need to remove \n");
                return CLI_SUCCESS;
            }
            if (NULL == g_ctc_stress_oam->p_lmep + loop)
            {
                continue;
            }
            sal_memcpy(lmep, g_ctc_stress_oam->p_lmep + loop, sizeof(ctc_oam_lmep_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_oam_remove_lmep(g_api_lchip, lmep);
            }
            else
            {
                ret = ctc_oam_remove_lmep(lmep);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Remove lmep %d normal\n", loop+1);
            }
            else
            {
                ctc_cli_out("Remove lmep %d error: %s\n", loop+1, ctc_get_error_desc(ret));
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", oam_info->entry_num);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        if (g_ctc_stress_oam->p_lmep)
        {
            sal_free(g_ctc_stress_oam->p_lmep);
            g_ctc_stress_oam->p_lmep = NULL;
        }
        if (g_ctc_stress_oam->p_rmep)
        {
            sal_free(g_ctc_stress_oam->p_rmep);
            g_ctc_stress_oam->p_rmep = NULL;
        }
        if (g_ctc_stress_oam->p_rmep_p2mp)
        {
            for (loop = 0; loop < 4096; loop++)
            {
                if (g_ctc_stress_oam->p_rmep_p2mp[loop])
                {
                    sal_free(g_ctc_stress_oam->p_rmep_p2mp[loop]);
                }
            }
            g_ctc_stress_oam->p_rmep_p2mp = NULL;
        }
    }

    return ret;

}



int32
_ctc_cli_stress_y1731_mip(ctc_oam_mip_t* mip, ctc_cli_stress_test_oam_t* oam_info, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0,valid_count =0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint8 is_random = 0;
    uint8 is_random_all = 0;


    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    oam_info->entry_num = (oam_info->entry_num > 4096) ? 4096 : oam_info->entry_num;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        if (!g_ctc_stress_oam->p_mip)
        {
            g_ctc_stress_oam->p_mip = (ctc_oam_mip_t*)sal_malloc(sizeof(ctc_oam_mip_t)*5*1024);
            if (!g_ctc_stress_oam->p_mip)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
        }
        sal_memset(g_ctc_stress_oam->p_mip, 0, sizeof(ctc_oam_mip_t)*5*1024);
    }


    for (loop = 0; loop < oam_info->entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            mip->key.u.eth.vlan_id = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
            mip->key.u.eth.gport = (is_random)?(sal_rand() % 255):(loop % 255);

            mip->master_gchip = is_random_all ? sal_rand() % 255:0;

            if (g_ctcs_api_en)
            {
                ret = ctcs_oam_add_mip(g_api_lchip, mip);
            }
            else
            {
                ret = ctc_oam_add_mip(mip);
            }
            sal_memcpy(g_ctc_stress_oam->p_mip + loop, mip, sizeof(ctc_oam_mip_t));
            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Add mip %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Add mip %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_oam->p_mip)
            {
                ctc_cli_out("No mip resource need to remove \n");
                return CLI_SUCCESS;
            }
            if (NULL == g_ctc_stress_oam->p_mip + loop)
            {
                continue;
            }
            sal_memcpy(mip, g_ctc_stress_oam->p_mip + loop, sizeof(ctc_oam_mip_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_oam_remove_mip(g_api_lchip, mip);
            }
            else
            {
                ret = ctc_oam_remove_mip(mip);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Remove mip %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Remove mip %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", oam_info->entry_num);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        if (g_ctc_stress_oam->p_mip)
        {
            sal_free(g_ctc_stress_oam->p_mip);
            g_ctc_stress_oam->p_mip = NULL;
        }
    }

    return ret;

}


int32
_ctc_cli_stress_maid(ctc_oam_maid_t* maid, ctc_cli_stress_test_oam_t* oam_info, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0,valid_count =0;
    uint16 temp;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};

    uint8 is_random = 0;
    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    oam_info->entry_num = (oam_info->entry_num > 4096) ? 4096 : oam_info->entry_num;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        if (!g_ctc_stress_oam->p_maid)
        {
            g_ctc_stress_oam->p_maid = (ctc_oam_maid_t*)sal_malloc(sizeof(ctc_oam_maid_t)*5*1024);
            if (!g_ctc_stress_oam->p_maid)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
        }
        sal_memset(g_ctc_stress_oam->p_maid, 0, sizeof(ctc_oam_maid_t)*5*1024);
    }

    for (loop = 0; loop < oam_info->entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            temp = (is_random)?((sal_rand() % 4095) + 1):(loop % 4096);
            maid->maid[1] = temp&0xFF;
            maid->maid[0] = (temp >> 8)&0xFF; /* just for stress test*/

            if (g_ctcs_api_en)
            {
                ret = ctcs_oam_add_maid(g_api_lchip, maid);
            }
            else
            {
                ret = ctc_oam_add_maid(maid);
            }
            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Add maid %d normal\n", loop + 1);
                sal_memcpy(g_ctc_stress_oam->p_maid + loop, maid, sizeof(ctc_oam_maid_t));
            }
            else
            {
                ctc_cli_out("Add maid %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_oam->p_maid)
            {
                ctc_cli_out("No maid resource need to remove \n");
                return CLI_SUCCESS;
            }
            if (NULL == g_ctc_stress_oam->p_maid + loop)
            {
                continue;
            }
            sal_memcpy(maid, g_ctc_stress_oam->p_maid + loop, sizeof(ctc_oam_maid_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_oam_remove_maid(g_api_lchip, maid);
            }
            else
            {
                ret = ctc_oam_remove_maid(maid);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Remove maid %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Remove maid %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", oam_info->entry_num);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        if (g_ctc_stress_oam->p_maid)
        {
            sal_free(g_ctc_stress_oam->p_maid);
            g_ctc_stress_oam->p_maid = NULL;
        }
    }

    return ret;

}

#define MEP_STRESS ""
CTC_CLI(ctc_cli_stress_oam_mep_add,
        ctc_cli_stress_oam_mep_add_cmd,
        "oam mep add (ip-bfd(ipv6|)|mpls-bfd|tp-bfd|tp-y1731|ethoam (is-p2mp|)) (nhid NHID|) (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "OAM module",
        "Mep",
        "Add",
        "IP BFD",
        "IPV6",
        "MPLS BFD",
        "TP BFD",
        "TP Y1731",
        "EthOAM",
        "P2mp mode",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    int32 ret_maid = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint32 nhid = 2;
    ctc_cli_stress_t stress;
    char* meg_id = "aabb";
    ctc_oam_maid_t maid;
    ctc_oam_lmep_t lmep;
    ctc_oam_rmep_t rmep;
    ctc_oam_y1731_lmep_t* p_y1731_lmep  = NULL;
    ctc_oam_bfd_lmep_t* p_bfd_lmep  = NULL;
    ctc_oam_y1731_rmep_t* p_y1731_rmep  = NULL;
    ctc_oam_bfd_rmep_t* p_bfd_rmep  = NULL;
    sal_memset(g_ctc_stress_oam, 0, sizeof(ctc_cli_stress_test_oam_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&maid, 0, sizeof(maid));
    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));

    p_y1731_lmep = &lmep.u.y1731_lmep;
    p_bfd_lmep = &lmep.u.bfd_lmep;

    sal_memset(g_ctc_stress_oam, 0, sizeof(ctc_cli_stress_test_oam_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&maid, 0, sizeof(maid));
    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));
    p_y1731_lmep = &lmep.u.y1731_lmep;
    p_bfd_lmep = &lmep.u.bfd_lmep;
    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));
    p_y1731_rmep = &rmep.u.y1731_rmep;
    p_bfd_rmep = &rmep.u.bfd_rmep;

    maid.maid[0] = 0x1; /* MD name format 1 */
    maid.maid[1] = 32;  /* ICC based format */
    maid.maid[2] = 13; /* megid len */
    maid.maid_len = 7;
    sal_memcpy(&maid.maid[3], meg_id, 4); /* copy megid string */


    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("nhid", nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IP_SINGLE_HOP);
        p_bfd_lmep->bfd_src_port = 49153;

        index = CTC_CLI_GET_ARGC_INDEX("ipv6");
        if (0xFF != index)
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP);
            g_ctc_stress_oam->is_ipv6 = 1;
        }
        else
        {
            CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
            p_bfd_lmep->ip4_sa = 0x01010101;
            p_bfd_lmep->ip4_da = 0x02020202;
        }

        rmep.key.mep_type = CTC_OAM_MEP_TYPE_IP_BFD;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;
        p_bfd_lmep->bfd_src_port = 49153;
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP);
        p_bfd_lmep->ip4_sa = 0x01010101;
        p_bfd_lmep->ip4_da = 0x10000001;

        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_BFD;

    }
    index = CTC_CLI_GET_ARGC_INDEX("tp-bfd");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;
        lmep.key.u.tp.mpls_spaceid = 0;

        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_BFD;
    }
    if ((CTC_OAM_MEP_TYPE_IP_BFD == lmep.key.mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == lmep.key.mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == lmep.key.mep_type))
    {
        CTC_SET_FLAG(p_bfd_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN);
        p_bfd_lmep->desired_min_tx_interval = 3;
        p_bfd_lmep->local_state = 2;
        p_bfd_lmep->local_detect_mult = 3;
        p_bfd_lmep->nhid = nhid;
        p_bfd_lmep->ttl = 255;

        CTC_SET_FLAG(p_bfd_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN);
        p_bfd_rmep->required_min_rx_interval = 3;
        p_bfd_rmep->remote_state = 2;
        p_bfd_rmep->remote_detect_mult = 3;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tp-y1731");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;
        p_y1731_lmep->mpls_ttl = 10;
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

        maid.mep_type = CTC_OAM_MEP_TYPE_MPLS_TP_Y1731;

    }
    index = CTC_CLI_GET_ARGC_INDEX("ethoam");
    if (0xFF != index)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        lmep.key.u.eth.md_level = 3;
        p_y1731_lmep->tpid_index = CTC_PARSER_L2_TPID_SVLAN_TPID_0;
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_MEP_EN;
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE;

        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_MEP_EN;
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        rmep.key.u.eth.md_level = 3;

        maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    g_ctc_stress_oam->maid_type = maid.mep_type;
    if (g_ctcs_api_en)
    {
        ret_maid = ctcs_oam_add_maid(g_api_lchip, &maid);
    }
    else
    {
        ret_maid = ctc_oam_add_maid(&maid);
    }
    if (ret_maid < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret_maid));
    }


    if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == lmep.key.mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == lmep.key.mep_type))
    {
        lmep.maid = maid;
        p_y1731_lmep->ccm_interval = 1;
        p_y1731_lmep->nhid = nhid;
        p_y1731_lmep->flag |= CTC_OAM_Y1731_LMEP_FLAG_MEP_EN;

        p_y1731_rmep->flag |= CTC_OAM_Y1731_RMEP_FLAG_MEP_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("entry-num", g_ctc_stress_oam->entry_num, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("is-p2mp");
    if (index != 0xFF)
    {
        p_y1731_lmep->flag &= (~CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE) ;
        g_ctc_stress_oam->is_p2mp = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_oam_mep(&lmep, &rmep, g_ctc_stress_oam, &stress);

    return ret;
}

CTC_CLI(ctc_cli_stress_oam_mep_remove,
        ctc_cli_stress_oam_mep_remove_cmd,
        "oam mep remove ",
        "OAM module",
        "Mep",
        "Remove")
{
    int32 ret = CLI_SUCCESS;
    int32 ret_maid = CLI_SUCCESS;

    ctc_cli_stress_t stress;
    char* meg_id = "aabb";
    ctc_oam_maid_t maid;
    ctc_oam_lmep_t lmep;
    ctc_oam_rmep_t rmep;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&maid, 0, sizeof(maid));
    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));
    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));

    maid.maid[0] = 0x1; /* MD name format 1 */
    maid.maid[1] = 32;  /* ICC based format */
    maid.maid[2] = 13; /* megid len */
    maid.maid_len = 7;
    sal_memcpy(&maid.maid[3], meg_id, 4); /* copy megid string */
    maid.mep_type = g_ctc_stress_oam->maid_type;

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    ret = _ctc_cli_stress_oam_mep(&lmep, &rmep, g_ctc_stress_oam, &stress);

    if (g_ctcs_api_en)
    {
        ret_maid = ctcs_oam_remove_maid(g_api_lchip, &maid);
    }
    else
    {
        ret_maid = ctc_oam_remove_maid(&maid);
    }
    if (ret_maid < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret_maid));
    }

    return ret;
}


#define MIP_STRESS ""

CTC_CLI(ctc_cli_stress_oam_mip_add,
        ctc_cli_stress_oam_mip_add_cmd,
        "oam mip add (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "OAM module",
        "Mip",
        "Add",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;

    ctc_oam_mip_t mip;
    ctc_cli_stress_t stress;

    sal_memset(&mip, 0, sizeof(ctc_oam_mip_t));
    sal_memset(g_ctc_stress_oam, 0, sizeof(ctc_cli_stress_test_oam_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    mip.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    mip.key.u.eth.md_level = 3;


    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("entry-num", g_ctc_stress_oam->entry_num, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }


    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }


    ret = _ctc_cli_stress_y1731_mip(&mip, g_ctc_stress_oam, &stress);

    return ret;
}



CTC_CLI(ctc_cli_stress_oam_mip_remove,
        ctc_cli_stress_oam_mip_remove_cmd,
        "oam mip remove",
        "OAM module",
        "Mip",
        "Remove")
{
    int32 ret = CLI_SUCCESS;

    ctc_oam_mip_t mip;
    ctc_cli_stress_t stress;

    sal_memset(&mip, 0, sizeof(ctc_oam_mip_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    ret = _ctc_cli_stress_y1731_mip(&mip, g_ctc_stress_oam, &stress);

    return ret;
}


#define MAID_STRESS ""

CTC_CLI(ctc_cli_stress_oam_maid_add,
        ctc_cli_stress_oam_maid_add_cmd,
        "oam maid add (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "OAM module",
        "Maid",
        "Add",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    char* meg_id = "aabb";
    ctc_cli_stress_t stress;
    ctc_oam_maid_t maid;

    sal_memset(g_ctc_stress_oam, 0, sizeof(ctc_cli_stress_test_oam_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&maid, 0, sizeof(maid));

    maid.maid[2] = 13; /* megid len */
    maid.maid_len = 7;
    sal_memcpy(&maid.maid[3], meg_id, 4); /* copy megid string */
    maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;


    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("entry-num", g_ctc_stress_oam->entry_num, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }


    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }


    ret = _ctc_cli_stress_maid(&maid, g_ctc_stress_oam, &stress);

    return ret;
}






CTC_CLI(ctc_cli_stress_oam_maid_remove,
        ctc_cli_stress_oam_maid_remove_cmd,
        "oam maid remove",
        "OAM module",
        "Maid",
        "Remove")
{
    int32 ret = CLI_SUCCESS;
    char* meg_id = "aabb";
    ctc_cli_stress_t stress;
    ctc_oam_maid_t maid;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&maid, 0, sizeof(maid));

    maid.maid[2] = 13; /* megid len */
    maid.maid_len = 7;
    sal_memcpy(&maid.maid[3], meg_id, 4); /* copy megid string */
    maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    ret = _ctc_cli_stress_maid(&maid, g_ctc_stress_oam, &stress);

    return ret;
}




int32
ctc_stress_test_oam_cli_init(void)
{
    g_ctc_stress_oam = (ctc_cli_stress_test_oam_t*)sal_malloc(sizeof(ctc_cli_stress_test_oam_t));
    if(!g_ctc_stress_oam)
    {
        return CLI_ERROR;
    }
    sal_memset(g_ctc_stress_oam, 0, sizeof(ctc_cli_stress_test_oam_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_mep_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_mip_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_maid_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_mep_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_mip_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_oam_maid_remove_cmd);

    return CLI_SUCCESS;
}


