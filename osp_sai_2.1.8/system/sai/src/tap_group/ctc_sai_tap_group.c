#include <sal.h>
#include "ctc_api.h"
#include "sal_error.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_tap_group.h>
#include <ctc_sai_port.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_l2mc.h>
#include <ctc_sai_mirror.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_notifier_call.h>
#include <ctclib_opf.h>


#define 	CTC_SAI_TAP_GROUP_RES				4096
#define 	CTC_SAI_TAP_GROUP_BLOCK_SIZE		32

#define 	CTC_SAI_TAP_EGRESS_PORT_RES				512
#define 	CTC_SAI_TAP_EGRESS_PORT_BLOCK_SIZE		32

#define     CTC_SAI_TAP_TRUNCTION_LENGTH_MIN    64
#define     CTC_SAI_TAP_TRUNCTION_LENGTH_MAX    144

struct edit_group_travers_info
{
    ctc_sai_tap_group_edit_group    *edit_group;
    ctc_sai_tap_info_t              *ptap_info_entry;
    int32_t                         add;
};

/* l2-pdu action index */
enum glb_l2pdu_action_index_e
{
    /* statically assigned */
    GLB_L2PDU_ACTION_INDEX_EFM                  = 0,    /* EFM action index, GB Chip reserve action index for EFM */
    GLB_L2PDU_ACTION_INDEX_BPDU                 = 1,    /* BPDU action index*/
    GLB_L2PDU_ACTION_INDEX_SLOW_PROTO           = 2,    /* slow protocol action index */
    GLB_L2PDU_ACTION_INDEX_EAPOL                = 3,    /* EAPOL action index */
    GLB_L2PDU_ACTION_INDEX_LLDP                 = 4,    /* LLDP action index*/
    /* added by liuyang 2012-11-13*/
    GLB_L2PDU_ACTION_INDEX_ERPS                 = 5,    /* ERPS a ction index */
    GLB_L2PDU_ACTION_INDEX_SMART_LINK           = 6,    /* SMART LINK action index */
    /*support mask for l2protocol by weij 2013-12-18, cr id 26103*/
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MASK_MAC0    = 7,    /* L2Protocol MASK MAC0 */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC0         = 8,    /* L2Protocol MAC0 */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC1         = 9,    /* L2Protocol MAC1 */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC2         = 10,    /* L2Protocol MAC2 */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC3         = 11,   /* L2Protocol MAC3 */
    /*support mask for l2protocol by weij 2013-12-18, cr id 26103*/
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC4         = 12,   /* L2Protocol MAC4 */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_MAC_DFT      = 13,    /* L2Protocol MAC Default, MAC for 0180.c200.0000~0180.c200.001F */
    GLB_L2PDU_ACTION_INDEX_L2PROTO_TUNNEL_DMAC  = 14,   /* L2Protocol Tunnel DMAC */
    GLB_L2PDU_ACTION_INDEX_UDLD                 = 15,   /* UDLD action index */
    GLB_L2PDU_ACTION_INDEX_ISIS                 = 16,    /* ISIS action index*/
    /*add by zhangjx for bug 20488 , 20120905*/
    GLB_L2PDU_ACTION_INDEX_L2PROTO_FULL_MAC0    = 17,    /* L2Protocol FULL-MAC0 */
    /* modified by liyf for g8032 control vlan, 2011-11-24 */
    GLB_L2PDU_ACTION_INDEX_MAC_COPY_CPU         = 18,
    GLB_L2PDU_ACTION_INDEX_CFM                  = 19,    /* CFM action index in GB system*/

    /* modified by sunh for bug 28676, 2014-05-19 */
    GLB_L2PDU_ACTION_INDEX_LBDT                 = 20,   /* Loopback detection action index. */   
    /* The default L2-PDU action index used to send PDU to CPU for potential process in Peth */
    GLB_L2PDU_ACTION_INDEX_DEFAULT              = 31,
    GLB_L2PDU_ACTION_INDEX_INVALID              = 32
};

sai_status_t
__ctc_sai_tap_group_add_trunction_res(ctc_sai_tap_group_edit_group  *pedit_group);
sai_status_t
__ctc_sai_tap_group_del_trunction_res(ctc_sai_tap_group_edit_group  *pedit_group);

static ctc_sai_tap_group_master_t  g_sai_tap_group_master;
uint32 port_id[4] = {0};

static sai_status_t
__tap_create(ctc_sai_tap_info_t **pptap_info_entry)
{
    stbl_sai_glb_t          *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_info_t    	*ptap_info_entry 	= NULL;
    uint32_t                opf_index;

    /* 1: check res */
    if (g_sai_tap_group_master.tap_cnt >= g_sai_tap_group_master.max_count)
    {
        ret = SAI_STATUS_TABLE_FULL;
        goto out;
    }

    /* 2: alloc index */
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(
        ctclib_opb_alloc_offset(&p_sai_glb->tap_group_opb, &opf_index),ret,out);

    /* 3: malloc mem */
    ptap_info_entry = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_tap_info_t));
    if (NULL == ptap_info_entry)
    {
        ret = SAI_STATUS_NO_MEMORY;
        ctclib_opb_free_offset(&p_sai_glb->tap_group_opb, opf_index);
        goto out;
    }
    
    sal_memset(ptap_info_entry, 0, sizeof(ctc_sai_tap_info_t));

    /* 4: set OID */
    *pptap_info_entry = ptap_info_entry;
    ptap_info_entry->tap_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_TAP_GROUP,opf_index);

    /* 5: save to vector */
    ctc_vector_add(g_sai_tap_group_master.ptap_group_vector, opf_index, ptap_info_entry);

    g_sai_tap_group_master.tap_cnt++;

    CTC_SAI_DEBUG("__tap_create:tap_oid = 0x%llx\n",ptap_info_entry->tap_oid);

out:
    return ret;
}

void
__tap_remove(
    _In_  ctc_sai_tap_info_t* ptap_info_entry)
{
    stbl_sai_glb_t          *p_sai_glb = stbl_sai_glb_get_sai_glb();
    if(NULL == ptap_info_entry)
    {
        return ;
    }

    CTC_SAI_DEBUG("__tap_remove:tap_oid = 0x%llx\n",ptap_info_entry->tap_oid);

    /* 1: remove vector */
    ctc_vector_del( g_sai_tap_group_master.ptap_group_vector, 
                    CTC_SAI_OBJECT_INDEX_GET(ptap_info_entry->tap_oid));

    /* 2: free index */
    ctclib_opb_free_offset(&p_sai_glb->tap_group_opb, CTC_SAI_OBJECT_INDEX_GET(ptap_info_entry->tap_oid));

    /* 3: free mem */
    XFREE(MEM_FEA_TEMP_TODO, ptap_info_entry);

    g_sai_tap_group_master.tap_cnt--;
}

ctc_sai_tap_info_t*
ctc_sai_tap_get_by_oid(sai_object_id_t  tap_oid)
{
    return ctc_vector_get( g_sai_tap_group_master.ptap_group_vector, 
                    CTC_SAI_OBJECT_INDEX_GET(tap_oid));
}

static sai_status_t
ctc_sai_tap_group_create(ctc_sai_tap_info_t **pptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    ret = __tap_create(pptap_info_entry);

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    (*pptap_info_entry)->tap_info_type = CTC_SAI_TAP_INFO_TYPE_GROUP;

    (*pptap_info_entry)->u.tap_group.edit_header = ctc_slist_new();
    
    g_sai_tap_group_master.tap_group_cnt++;

out:
    return ret;
}

static sai_status_t
ctc_sai_tap_group_remove(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    ctc_slist_free(ptap_info_entry->u.tap_group.edit_header);

    __tap_remove(ptap_info_entry);

    g_sai_tap_group_master.tap_group_cnt--;

    return ret;
}

static sai_status_t
ctc_sai_tap_group_ingress_create(ctc_sai_tap_info_t **pptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    ret = __tap_create(pptap_info_entry);

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    (*pptap_info_entry)->tap_info_type = CTC_SAI_TAP_INFO_TYPE_MEMBER;
    (*pptap_info_entry)->u.tap_group_member.tap_member_stage = 
                                        CTC_SAI_TAP_GROUP_MEMBER_TYPE_INGRESS;
    
    g_sai_tap_group_master.tap_member_ing_cnt++;

out:
    return ret;
}

static sai_status_t
ctc_sai_tap_group_ingress_remove(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    __tap_remove(ptap_info_entry);

    g_sai_tap_group_master.tap_member_ing_cnt--;

    return ret;
}

static sai_status_t
ctc_sai_tap_group_egress_create(ctc_sai_tap_info_t **pptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    ret = __tap_create(pptap_info_entry);

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    (*pptap_info_entry)->tap_info_type = CTC_SAI_TAP_INFO_TYPE_MEMBER;
    (*pptap_info_entry)->u.tap_group_member.tap_member_stage = 
                                        CTC_SAI_TAP_GROUP_MEMBER_TYPE_EGRESS;
    
    g_sai_tap_group_master.tap_member_egr_cnt++;

out:
    return ret;
}

static sai_status_t
ctc_sai_tap_group_egress_remove(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;

    __tap_remove(ptap_info_entry);

    g_sai_tap_group_master.tap_member_egr_cnt--;

    return ret;
}

#define __EGRESS_MISC_FUNC
static uint32_t
__egress_misc_hash_make(
    _In_  void* data)
{
    ctc_sai_tap_group_egress_misc* pegress_misc_hash = (ctc_sai_tap_group_egress_misc*)data;

    return ctc_hash_caculate(sizeof(ctc_sai_tap_group_egress_misc_key), &pegress_misc_hash->key);
}

static bool
__egress_misc_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_tap_group_egress_misc* pegress_misc_hash = data;
    ctc_sai_tap_group_egress_misc* pegress_misc_hash1 = data1;

    if (sal_memcmp(&pegress_misc_hash->key, &pegress_misc_hash1->key, sizeof(ctc_sai_tap_group_egress_misc_key)))
    {
        return TRUE;
    }
    return FALSE;
}

static ctc_sai_tap_group_egress_misc*
__egress_misc_alloc (ctc_sai_tap_group_egress_misc_key *key)
{
    ctc_sai_tap_group_egress_misc   egress_misc; 
    ctc_sai_tap_group_egress_misc   *pegress_misc = NULL;
    ctc_misc_nh_param_t     nh_param;
    ctc_ip_nh_param_t       ipuc_nh_param;
    ctc_ip_tunnel_nh_param_t tunnel_nh_param;
    uint32_t                nexthopid    = 0;
    sai_status_t            ret 		 = SAI_STATUS_SUCCESS;
    uint32 tmp_fid = 0;
    
    sal_memset(&nh_param, 0, sizeof(ctc_misc_nh_param_t));
    sal_memset(&ipuc_nh_param, 0, sizeof(ctc_ip_nh_param_t));
    sal_memset(&tunnel_nh_param, 0, sizeof(tunnel_nh_param));
    sal_memset(&egress_misc, 0, sizeof(ctc_sai_tap_group_egress_misc));

    sal_memcpy(&egress_misc.key,key, sizeof(ctc_sai_tap_group_egress_misc_key));

    pegress_misc = ctc_hash_lookup(g_sai_tap_group_master.egress_misc_hash, &egress_misc);

    if(NULL == pegress_misc){
        pegress_misc = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_tap_group_egress_misc));
        if(NULL == pegress_misc){
            return NULL;
        }

        sal_memset(pegress_misc, 0, sizeof(ctc_sai_tap_group_egress_misc));
        sal_memcpy(&pegress_misc->key,key, sizeof(ctc_sai_tap_group_egress_misc_key));
    }

    /* if exist then ref count */
    if(pegress_misc->count > 0){
        pegress_misc->count++;

        CTC_SAI_DEBUG("%s%d: get egress port res inc count:port_oid = 0x%llx, count = %d\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid, pegress_misc->count);
        
        return pegress_misc;
    }

    /* create res */
    if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        CTC_SAI_DEBUG("%s%d:alloc nexthop port_oid = 0x%llx,timestamp\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid);
        
        nh_param.type = CTC_MISC_NH_TYPE_OVER_L2_WITH_TS;
        sal_memcpy(nh_param.misc_param.over_l2edit.mac_da,
            ctc_sai_get_sys_info()->timestamp.dmac, sizeof(sai_mac_t));
        sal_memcpy(nh_param.misc_param.over_l2edit.mac_sa,
            ctc_sai_get_sys_info()->timestamp.smac, sizeof(sai_mac_t));
        nh_param.misc_param.over_l2edit.ether_type = ctc_sai_get_sys_info()->timestamp.type;
    }

    if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM)){
        CTC_SAI_DEBUG("%s%d:alloc nexthop port_oid = 0x%llx,custom\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid);
#ifndef GREATBELT        
        nh_param.type = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;
        nh_param.misc_param.flex_edit.dscp_select = CTC_NH_DSCP_SELECT_PACKET;
#else
        nh_param.type = CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR;
        nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_VLAN_EDIT;
        nh_param.misc_param.l2_l3edit.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
        nh_param.misc_param.l2_l3edit.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
#endif
        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_DMAC)){
#ifndef GREATBELT            
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACDA;
            sal_memcpy(nh_param.misc_param.flex_edit.mac_da,
                pegress_misc->key.edit_key.mac_dest, sizeof(sai_mac_t));
#else            
            nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_DA;
            sal_memcpy(nh_param.misc_param.l2_l3edit.mac_da,
                pegress_misc->key.edit_key.mac_dest, sizeof(sai_mac_t));
#endif            
        }

        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_SMAC)){
#ifndef GREATBELT            
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_MACSA;
            sal_memcpy(nh_param.misc_param.flex_edit.mac_sa,
                pegress_misc->key.edit_key.mac_src, sizeof(sai_mac_t));
#else           
            nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_REPLACE_MAC_SA;
            sal_memcpy(nh_param.misc_param.l2_l3edit.mac_sa,
                pegress_misc->key.edit_key.mac_src, sizeof(sai_mac_t));
#endif            
        }
        
        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_VLAN)){
#ifndef GREATBELT            
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_VLAN_TAG;
	        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_REP_OR_ADD))
	        {
                nh_param.misc_param.flex_edit.stag_op = CTC_VLAN_TAG_OP_REP_OR_ADD;
	        }else if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_ADD)){
                nh_param.misc_param.flex_edit.stag_op = CTC_VLAN_TAG_OP_ADD;
	        }else{
                nh_param.misc_param.flex_edit.stag_op = CTC_VLAN_TAG_OP_REP;
	        }
            nh_param.misc_param.flex_edit.svid_sl = CTC_VLAN_TAG_SL_NEW;
            nh_param.misc_param.flex_edit.new_svid = pegress_misc->key.edit_key.vlan;
#else
            nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L2_EDIT_VLAN_EDIT;
            SAI_SET_FLAG(nh_param.misc_param.l2_l3edit.vlan_edit_info.edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            nh_param.misc_param.l2_l3edit.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            nh_param.misc_param.l2_l3edit.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_NONE;
            
            nh_param.misc_param.l2_l3edit.vlan_edit_info.output_svid = 
                pegress_misc->key.edit_key.vlan;
#endif            
        }        

        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_SIP)){
#ifndef GREATBELT            
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR;
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPSA;
            if (pegress_misc->key.edit_key.ip_sip.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
            {
                nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_IPV4;
                nh_param.misc_param.flex_edit.ip_sa.ipv4 = pegress_misc->key.edit_key.ip_sip.addr.ip4;
            }
            else
            {
                sal_memcpy(&nh_param.misc_param.flex_edit.ip_sa.ipv6,
                &pegress_misc->key.edit_key.ip_sip.addr.ip6, sizeof(sai_ip6_t));
                nh_param.misc_param.flex_edit.ip_sa.ipv6[0] = sal_htonl(nh_param.misc_param.flex_edit.ip_sa.ipv6[0]);
                nh_param.misc_param.flex_edit.ip_sa.ipv6[1] = sal_htonl(nh_param.misc_param.flex_edit.ip_sa.ipv6[1]);
                nh_param.misc_param.flex_edit.ip_sa.ipv6[2] = sal_htonl(nh_param.misc_param.flex_edit.ip_sa.ipv6[2]);
                nh_param.misc_param.flex_edit.ip_sa.ipv6[3] = sal_htonl(nh_param.misc_param.flex_edit.ip_sa.ipv6[3]);
            }
#endif            
        }

        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_DIP)){
#ifndef GREATBELT             
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR;
            nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_REPLACE_IPDA;
            if (pegress_misc->key.edit_key.ip_dip.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
            {
                nh_param.misc_param.flex_edit.flag |= CTC_MISC_NH_FLEX_EDIT_IPV4;
                nh_param.misc_param.flex_edit.ip_da.ipv4 = pegress_misc->key.edit_key.ip_dip.addr.ip4;
            }
            else
            {
                sal_memcpy(&nh_param.misc_param.flex_edit.ip_da.ipv6,
                &pegress_misc->key.edit_key.ip_dip.addr.ip6, sizeof(sai_ip6_t));
                nh_param.misc_param.flex_edit.ip_da.ipv6[0] = sal_htonl(nh_param.misc_param.flex_edit.ip_da.ipv6[0]);
                nh_param.misc_param.flex_edit.ip_da.ipv6[1] = sal_htonl(nh_param.misc_param.flex_edit.ip_da.ipv6[1]);
                nh_param.misc_param.flex_edit.ip_da.ipv6[2] = sal_htonl(nh_param.misc_param.flex_edit.ip_da.ipv6[2]);
                nh_param.misc_param.flex_edit.ip_da.ipv6[3] = sal_htonl(nh_param.misc_param.flex_edit.ip_da.ipv6[3]);
            }
#else           
            nh_param.misc_param.l2_l3edit.flag |= CTC_MISC_NH_L3_EDIT_REPLACE_IPDA;
            nh_param.misc_param.l2_l3edit.ipda = pegress_misc->key.edit_key.ip_dip.addr.ip4; 
#endif
        }
    }

    if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR)){
        CTC_SAI_DEBUG("%s%d:alloc nexthop port_oid = 0x%llx,custom_gre\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid);
#ifndef GREATBELT         
        ipuc_nh_param.flag |= CTC_IP_NH_FLAG_USE_L2EDIT;
#ifdef USW
        GLB_SET_FLAG(ipuc_nh_param.flag, CTC_IP_NH_FLAG_REWRITE_ETHER_TYPE);
#endif
        ipuc_nh_param.oif.vid = 0;
        ipuc_nh_param.oif.is_l2_port = TRUE;
#endif
        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_DMAC)){
#ifndef GREATBELT             
            sal_memcpy(ipuc_nh_param.mac,
                pegress_misc->key.edit_key.mac_dest, sizeof(sai_mac_t));
#endif            
        }

        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_SMAC)){
#ifndef GREATBELT            
            sal_memcpy(ipuc_nh_param.mac_sa,
                pegress_misc->key.edit_key.mac_src, sizeof(sai_mac_t));
#endif            
        }

        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_VLAN)){
#ifndef GREATBELT            
            ipuc_nh_param.oif.vid = pegress_misc->key.edit_key.vlan;
#endif            
        }
    }

    /*added by yejl for bug 53475, 2019-09-10*/    
    if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR))
    {
        uint32 tun_id = 0;
        mac_addr_t router_mac;
        sal_memset(&router_mac, 0, sizeof(mac_addr_t));
        ctc_l3if_get_router_mac(router_mac);
        sal_memcpy(tunnel_nh_param.mac_sa, router_mac, sizeof(mac_addr_t));
        sal_memcpy(tunnel_nh_param.mac, pegress_misc->key.edit_key.l2gre_mac_da, sizeof(mac_addr_t));
        
        tunnel_nh_param.tunnel_info.ip_sa.ipv4 = pegress_misc->key.edit_key.l2gre_ip_sip.addr.ip4;
        tunnel_nh_param.tunnel_info.ip_da.ipv4 = pegress_misc->key.edit_key.l2gre_ip_dip.addr.ip4;

        tunnel_nh_param.tunnel_info.inner_packet_type = PKT_TYPE_ETH;
        tunnel_nh_param.tunnel_info.dscp_select= CTC_NH_DSCP_SELECT_ASSIGN;
        tunnel_nh_param.tunnel_info.dscp_or_tos = 0;
        tunnel_nh_param.tunnel_info.ttl = 64;
        tunnel_nh_param.oif.vid = 0;
        tunnel_nh_param.oif.is_l2_port = 1;

        if (pegress_misc->key.edit_key.l2gre_key_len == GLB_L2GRE_KEY_LEN_16)
        {
            tun_id = pegress_misc->key.edit_key.l2gre_key_num << 16 ;
        }
        else if (pegress_misc->key.edit_key.l2gre_key_len == GLB_L2GRE_KEY_LEN_20)
        {
            tun_id = pegress_misc->key.edit_key.l2gre_key_num << 12 ;
        }
        else if (pegress_misc->key.edit_key.l2gre_key_len == GLB_L2GRE_KEY_LEN_24)
        {
            tun_id = pegress_misc->key.edit_key.l2gre_key_num << 8 ;
        }
        else
        {
            tun_id = pegress_misc->key.edit_key.l2gre_key_num;
        }
        tunnel_nh_param.tunnel_info.gre_info.protocol_type = 0x6558;
        CTC_SET_FLAG(tunnel_nh_param.tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY);
        tunnel_nh_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN4;
        tunnel_nh_param.tunnel_info.gre_info.gre_key = tun_id;
    }
    else if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR))
    {
        mac_addr_t router_mac;
        sal_memset(&router_mac, 0, sizeof(mac_addr_t));
        ctc_l3if_get_router_mac(router_mac);
        sal_memcpy(tunnel_nh_param.mac_sa, router_mac, sizeof(mac_addr_t));
        sal_memcpy(tunnel_nh_param.mac, pegress_misc->key.edit_key.l3gre_mac_da, sizeof(mac_addr_t));
        
        tunnel_nh_param.tunnel_info.ip_sa.ipv4 = pegress_misc->key.edit_key.l3gre_ip_sip.addr.ip4;
        tunnel_nh_param.tunnel_info.ip_da.ipv4 = pegress_misc->key.edit_key.l3gre_ip_dip.addr.ip4;

        tunnel_nh_param.tunnel_info.inner_packet_type = PKT_TYPE_ETH;
        tunnel_nh_param.tunnel_info.dscp_select= CTC_NH_DSCP_SELECT_ASSIGN;
        tunnel_nh_param.tunnel_info.dscp_or_tos = 0;
        tunnel_nh_param.tunnel_info.ttl = 64;
        tunnel_nh_param.oif.vid = 0;
        tunnel_nh_param.oif.is_l2_port = 1;
        tunnel_nh_param.tunnel_info.gre_info.protocol_type = 0x0800;
        tunnel_nh_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN4;
        tunnel_nh_param.tunnel_info.gre_info.gre_key = 0;
    }
    /*added by yejl for bug 53475, 2019-09-10*/

     /*added by guoxd for vxlan*/   
    else if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR))
    {
        mac_addr_t router_mac;
        sal_memset(&router_mac, 0, sizeof(mac_addr_t));
        ctc_l3if_get_router_mac(router_mac);
        
        tunnel_nh_param.oif.vid = 0;
        tunnel_nh_param.oif.is_l2_port = 1;

        sal_memcpy(tunnel_nh_param.mac, pegress_misc->key.edit_key.vxlan_mac_da, sizeof(mac_addr_t));
        sal_memcpy(tunnel_nh_param.mac_sa, router_mac, sizeof(mac_addr_t));
        
        tunnel_nh_param.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_VXLAN_IN4;
        tunnel_nh_param.tunnel_info.ttl = 64;

        if(0 != pegress_misc->key.edit_key.vxlan_port_src)
        {
            CTC_SET_FLAG(tunnel_nh_param.tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_VXLAN_L4_SRC_PORT);
            tunnel_nh_param.tunnel_info.l4_src_port = pegress_misc->key.edit_key.vxlan_port_src;
        }
        CTC_SAI_ERROR_GOTO(ctc_global_ctl_set(CTC_GLOBAL_VXLAN_UDP_DEST_PORT, &pegress_misc->key.edit_key.vxlan_port_dst),ret,err1);
        
        tunnel_nh_param.tunnel_info.ip_sa.ipv4 = pegress_misc->key.edit_key.vxlan_ip_sa.addr.ip4;
        tunnel_nh_param.tunnel_info.ip_da.ipv4 = pegress_misc->key.edit_key.vxlan_ip_da.addr.ip4;
        
        tunnel_nh_param.tunnel_info.gre_info.protocol_type = 0x6558;

        CTC_SET_FLAG(tunnel_nh_param.tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_GRE_WITH_KEY);
        tunnel_nh_param.tunnel_info.gre_info.gre_key = pegress_misc->key.edit_key.vxlan_vni_set;/*added by guoxd, if gre_key is 0, sdk check will return error*/

        tunnel_nh_param.tunnel_info.vn_id = pegress_misc->key.edit_key.vxlan_vni_set;
        
        tmp_fid = ctclib_opf_alloc_offset(g_sai_tap_group_master.tap_group_vxlan_opf, 0, 1);
        CTC_SAI_ERROR_GOTO(ctc_overlay_tunnel_set_fid(tunnel_nh_param.tunnel_info.vn_id, tmp_fid),ret,err1);
        
    }
    /*ended by guoxd*/
    
    CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthopid),ret,err);

    if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR))
    {
        
        if(sai_object_type_query(pegress_misc->key.poid) == SAI_OBJECT_TYPE_PORT){
            ipuc_nh_param.oif.gport = CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid);
        }else{
            ipuc_nh_param.oif.gport = (0x1f00 | CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid));
        }

        CTC_SAI_ERROR_GOTO(ctc_nh_add_ipuc(nexthopid, &ipuc_nh_param),ret,err1);
    }
    /*added by yejl for bug 53475, 2019-09-10*/
    else if (SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR)||
             SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR))
    {
        if(sai_object_type_query(pegress_misc->key.poid) == SAI_OBJECT_TYPE_PORT)
        {
            tunnel_nh_param.oif.gport = CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid);
        }
        else
        {
            tunnel_nh_param.oif.gport = (0x1f00 | CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid));
        }

        CTC_SAI_ERROR_GOTO(ctc_nh_add_ip_tunnel(nexthopid, &tunnel_nh_param),ret,err1);
    }
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    else if (SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR))
    {
        if(sai_object_type_query(pegress_misc->key.poid) == SAI_OBJECT_TYPE_PORT)
        {
            tunnel_nh_param.oif.gport = CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid);
        }
        else
        {
            tunnel_nh_param.oif.gport = (0x1f00 | CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid));
        }

        CTC_SAI_ERROR_GOTO(ctc_nh_add_ip_tunnel(nexthopid, &tunnel_nh_param),ret,err1);
    }
    /*ended by guoxd*/
    else
    {
        if(sai_object_type_query(pegress_misc->key.poid) == SAI_OBJECT_TYPE_PORT){
            nh_param.gport = CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid);
        }else{
            nh_param.gport = (0x1f00 | CTC_SAI_OBJECT_INDEX_GET(pegress_misc->key.poid));
        }

        CTC_SAI_ERROR_GOTO(ctc_nh_add_misc(nexthopid, &nh_param),ret,err1);    
    }

    pegress_misc->nexthop_id = nexthopid;

    CTC_SAI_DEBUG("%s%d:alloc nexthop port_oid = 0x%llx,nhid = %d\n",
        __FUNCTION__,__LINE__,pegress_misc->key.poid, pegress_misc->nexthop_id);

    pegress_misc->count++;

    ctc_hash_insert(g_sai_tap_group_master.egress_misc_hash, pegress_misc);

    return pegress_misc;
err1:
    ctc_sai_nexthop_free_offset(nexthopid);
    XFREE(MEM_FEA_TEMP_TODO, pegress_misc);
err:
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,pegress_misc->key.poid);
    return NULL;
}

void
__egress_misc_free (ctc_sai_tap_group_egress_misc *pegress_misc)
{
    sal_assert(pegress_misc != NULL);

    pegress_misc->count--;

    CTC_SAI_DEBUG("%s%d:dec count port_oid = 0x%llx,count = %d\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid, pegress_misc->count);
    
    if(pegress_misc->count <= 0 )
    {
        CTC_SAI_DEBUG("%s%d:free nexthop port_oid = 0x%llx,nexthopid = %d\n",
            __FUNCTION__,__LINE__,pegress_misc->key.poid, pegress_misc->nexthop_id);
        if(SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR))
        {
            ctc_nh_remove_ipuc(pegress_misc->nexthop_id);
        }
        /*added by yejl for bug 53475, 2019-09-10*/
        else if (SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR) ||
                 SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR))
        {
            ctc_nh_remove_ip_tunnel(pegress_misc->nexthop_id);
        }
        /*ended by yejl*/

        /*added by guoxd for vxlan*/
        else if (SAI_IS_BIT_SET(pegress_misc->key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR))
        {
            ctc_nh_remove_ip_tunnel(pegress_misc->nexthop_id);
        }
        /*ended by guoxd*/
        
        else
        {
            ctc_nh_remove_misc(pegress_misc->nexthop_id);
        }

        ctc_sai_nexthop_free_offset(pegress_misc->nexthop_id);
    }
}



#define __EDIT_GROUP_FUNC

static uint32_t
__edit_group_port_res_hash_make(
    _In_  void* data)
{
    ctc_sai_tap_group_port_res* pegress_res_hash = (ctc_sai_tap_group_port_res*)data;

    return ctc_hash_caculate(sizeof(pegress_res_hash->poid), &pegress_res_hash->poid);
}

static bool
__edit_group_port_res_hash_cmp(
    _In_ void *data,
    _In_ void *data1)
{
    ctc_sai_tap_group_port_res* pegress_res_hash = data;
    ctc_sai_tap_group_port_res* pegress_res_hash1 = data1;

    if (pegress_res_hash->poid == pegress_res_hash1->poid)
    {
        return TRUE;
    }
    return FALSE;
}

static ctc_sai_tap_group_edit_group*
__edit_group_alloc()
{
    ctc_sai_tap_group_edit_group    *edit_group = NULL;
        
    edit_group = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_tap_group_edit_group));
    if (NULL == edit_group)
    {
        return NULL;
    }
    
    sal_memset(edit_group, 0, sizeof(ctc_sai_tap_group_edit_group));
    
    return edit_group;
}

static void
__edit_group_free(ctc_sai_tap_group_edit_group *edit_group)
{
    XFREE(MEM_FEA_TEMP_TODO, edit_group);
}

sai_status_t
__edit_group_egress_member_add_with_edit_group(ctc_sai_tap_group_edit_group *edit_group, 
                                            ctc_sai_tap_group_member_t   *ptap_member)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    uint32                      sdk_gport               = 0;
    ctc_mcast_nh_param_group_t  sdk_nh_mcast_group;
    ctc_sai_tap_group_port_res          *port_res           = NULL;
    ctc_sai_tap_group_egress_misc_key   egress_misc_key     = {};
    ctc_sai_tap_group_egress_misc       *pegress_misc       = NULL;

    ptap_group_member_info = ptap_member;

    ctc_sai_port_objectid_to_gport(ptap_group_member_info->port_oid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:edit_group->key.edit_flag = 0x%lx\n",__FUNCTION__,__LINE__,edit_group->key.edit_flag);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    CTC_SAI_DEBUG("%s%d:timestamp = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.egress.timestamp);
    CTC_SAI_DEBUG("%s%d:nexthop_id = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
    CTC_SAI_DEBUG("%s%d:nexthop_group_id = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_group_id);


    sal_memset(&sdk_nh_mcast_group, 0, sizeof(sdk_nh_mcast_group));
    sdk_nh_mcast_group.opcode               = CTC_NH_PARAM_MCAST_ADD_MEMBER;
    sdk_nh_mcast_group.mem_info.is_source_check_dis = TRUE;
    
    
    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        CTC_SAI_DEBUG("%s%d:add trunction mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
        
        sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
        sdk_nh_mcast_group.is_mirror            = TRUE;
        sdk_nh_mcast_group.mem_info.destid      = sdk_gport;
    }

    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL)){
        sdk_nh_mcast_group.mc_grp_id            = edit_group->nexthop_group_id;
        
        if (ptap_group_member_info->u.egress.timestamp)
        {
            CTC_SAI_DEBUG("%s%d:add timestamp mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
            /* 3: check timestamp function */
            sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;

            sal_memset(&egress_misc_key, 0, sizeof(ctc_sai_tap_group_egress_misc_key));
            egress_misc_key.poid = ptap_group_member_info->port_oid;
            SAI_SET_BIT(egress_misc_key.edit_key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION);
            
            pegress_misc = __egress_misc_alloc(&egress_misc_key);
            sdk_nh_mcast_group.mem_info.ref_nhid = pegress_misc->nexthop_id;

            port_res = XCALLOC(MEM_FEA_TEMP_TODO, sizeof *port_res);
            port_res->poid = ptap_group_member_info->port_oid;
            port_res->egress_misc = pegress_misc;
            ctc_hash_insert(edit_group->port_res_hash, port_res);           
        }else{
            CTC_SAI_DEBUG("%s%d:add normal mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
            sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
            sdk_nh_mcast_group.mem_info.destid = sdk_gport;
        }
    }

    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM) ||
       SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR) ){
        CTC_SAI_DEBUG("%s%d:add custom mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
        
        sal_memset(&egress_misc_key, 0, sizeof(ctc_sai_tap_group_egress_misc_key));
        egress_misc_key.poid = ptap_group_member_info->port_oid;
        sal_memcpy(&egress_misc_key.edit_key,&edit_group->key,sizeof(ctc_sai_tap_group_edit_group_key));
        
        pegress_misc = __egress_misc_alloc(&egress_misc_key);
        
        sdk_nh_mcast_group.mc_grp_id            = edit_group->nexthop_group_id;
        sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        sdk_nh_mcast_group.mem_info.ref_nhid    = pegress_misc->nexthop_id;

        port_res = XCALLOC(MEM_FEA_TEMP_TODO, sizeof *port_res);
        port_res->poid = ptap_group_member_info->port_oid;
        port_res->egress_misc = pegress_misc;

        ctc_hash_insert(edit_group->port_res_hash, port_res);
    }

    ctc_nh_update_mcast(edit_group->nexthop_id, &sdk_nh_mcast_group);
    
    return ret;
}

sai_status_t
__edit_group_egress_member_del_with_edit_group(ctc_sai_tap_group_edit_group *edit_group, 
                                            ctc_sai_tap_group_member_t   *ptap_member)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    uint32                      sdk_gport               = 0;
    ctc_mcast_nh_param_group_t  sdk_nh_mcast_group;
    ctc_sai_tap_group_port_res          port_res            = {};
    ctc_sai_tap_group_port_res          *pport_res          = NULL;

    ptap_group_member_info = ptap_member;

    ctc_sai_port_objectid_to_gport(ptap_group_member_info->port_oid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:edit_group->key.edit_flag = 0x%lx\n",__FUNCTION__,__LINE__,edit_group->key.edit_flag);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    CTC_SAI_DEBUG("%s%d:timestamp = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.egress.timestamp);
    CTC_SAI_DEBUG("%s%d:nexthop_id = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
    CTC_SAI_DEBUG("%s%d:nexthop_group_id = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_group_id);

    sal_memset(&sdk_nh_mcast_group, 0, sizeof(sdk_nh_mcast_group));
    sdk_nh_mcast_group.opcode               = CTC_NH_PARAM_MCAST_DEL_MEMBER;
    sdk_nh_mcast_group.mem_info.is_source_check_dis = TRUE;
    
    
    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        CTC_SAI_DEBUG("%s%d:del trunction mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
        
        sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
        sdk_nh_mcast_group.is_mirror            = TRUE;
        sdk_nh_mcast_group.mem_info.destid      = sdk_gport;
        sdk_nh_mcast_group.mc_grp_id            = edit_group->nexthop_group_id;
        ctc_nh_update_mcast(edit_group->nexthop_id, &sdk_nh_mcast_group);
    }

    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL)){
        sdk_nh_mcast_group.mc_grp_id            = edit_group->nexthop_group_id;
        
        if (ptap_group_member_info->u.egress.timestamp)
        {
            CTC_SAI_DEBUG("%s%d:del timestamp mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
            /* 3: check timestamp function */
            sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;

            sal_memset(&port_res, 0, sizeof(port_res));
            port_res.poid = ptap_group_member_info->port_oid;
            pport_res = ctc_hash_lookup(edit_group->port_res_hash, &port_res);
            
            sdk_nh_mcast_group.mem_info.ref_nhid = pport_res->egress_misc->nexthop_id;
            ctc_nh_update_mcast(edit_group->nexthop_id, &sdk_nh_mcast_group);

            __egress_misc_free(pport_res->egress_misc);
            ctc_hash_remove(edit_group->port_res_hash, pport_res);
            XFREE(MEM_FEA_TEMP_TODO, pport_res);
                       
        }else{
            CTC_SAI_DEBUG("%s%d:del normal mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
            sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
            sdk_nh_mcast_group.mem_info.destid = sdk_gport;
            ctc_nh_update_mcast(edit_group->nexthop_id, &sdk_nh_mcast_group);
        }
    }

    if(SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM) ||
       SAI_IS_BIT_SET(edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR)){
        CTC_SAI_DEBUG("%s%d:del custom mcast = %d\n",__FUNCTION__,__LINE__,edit_group->nexthop_id);
        
        sal_memset(&port_res, 0, sizeof(port_res));
        port_res.poid = ptap_group_member_info->port_oid;
        pport_res = ctc_hash_lookup(edit_group->port_res_hash, &port_res);
        
        sdk_nh_mcast_group.mc_grp_id            = edit_group->nexthop_group_id;
        sdk_nh_mcast_group.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        sdk_nh_mcast_group.mem_info.ref_nhid    = pport_res->egress_misc->nexthop_id;

        ctc_nh_update_mcast(edit_group->nexthop_id, &sdk_nh_mcast_group);

        __egress_misc_free(pport_res->egress_misc);
        ctc_hash_remove(edit_group->port_res_hash, pport_res);
        XFREE(MEM_FEA_TEMP_TODO, pport_res);
    }
    
    return ret;
}



static int32
__edit_group_traversal_egress_update_tap_member(void* array_data, void* user_data)
{
    struct edit_group_travers_info      *pinfo          = user_data;
    ctc_sai_tap_info_t                  *ptap_info      = array_data;
    ctc_sai_tap_group_member_t          *ptap_member    = NULL;

    sal_assert(pinfo != NULL);

    if(NULL == ptap_info)
        return 0;

    if(CTC_SAI_TAP_INFO_TYPE_GROUP == ptap_info->tap_info_type)
        return 0;

    ptap_member = &ptap_info->u.tap_group_member;

    if(CTC_SAI_TAP_GROUP_MEMBER_TYPE_INGRESS == ptap_member->tap_member_stage)
        return 0;

    if(ptap_member->tap_group_oid != pinfo->ptap_info_entry->tap_oid)
        return 0;

    if(pinfo->add){
        __edit_group_egress_member_add_with_edit_group(pinfo->edit_group, ptap_member);
    } else {
        __edit_group_egress_member_del_with_edit_group(pinfo->edit_group, ptap_member);
    }
    
    return 0;
}


static void
__edit_group_update_tap_group_member(ctc_sai_tap_group_edit_group *edit_group, 
                                                 ctc_sai_tap_info_t *ptap_info_entry,
                                                 int32_t   is_add)
{
    struct edit_group_travers_info  info;

    info.edit_group         = edit_group;
    info.ptap_info_entry    = ptap_info_entry;
    info.add                = is_add;
        
    ctc_vector_traverse(g_sai_tap_group_master.ptap_group_vector, 
                            (vector_traversal_fn)__edit_group_traversal_egress_update_tap_member, &info);
}


ctc_sai_tap_group_edit_group *
ctc_sai_tap_group_get_edit_group(ctc_sai_tap_group_edit_group_key    key, ctc_sai_tap_info_t *ptap_info_entry)
{
    struct ctc_slistnode_s          *node       = NULL;
    ctc_sai_tap_group_t			    *tap_group  = NULL;
    ctc_sai_tap_group_edit_group    *edit_group = NULL;
    ctc_sai_tap_group_edit_group    *temp_edit_group = NULL;
    uint32_t                        nexthop_id  = 0;
    uint32_t                        group_id    = 0;
    ctc_mcast_nh_param_group_t      nh_mcast_group;
    sai_status_t                    ret 		= SAI_STATUS_SUCCESS;
        
    sal_assert(ptap_info_entry != NULL);

    tap_group = &ptap_info_entry->u.tap_group;

    /* foreach .get entry by key */
    CTC_SLIST_LOOP(tap_group->edit_header, node){
        temp_edit_group = (ctc_sai_tap_group_edit_group*)node;

        if(sal_memcmp(&temp_edit_group->key, &key, sizeof(key)) == 0){
            edit_group = temp_edit_group;
            break;
        }
    }

    /* if get anyone ,then return */
    if(edit_group){
        edit_group->count++;
        CTC_SAI_DEBUG("%s%d:get edit group: groupid = %d, nexthopid = %d count = %d\n",__FUNCTION__,__LINE__,
            edit_group->nexthop_group_id,edit_group->nexthop_id,edit_group->count);
        return edit_group;
    }

    /* create new */
    temp_edit_group = __edit_group_alloc();
    if(!temp_edit_group){
        return NULL;
    }

    temp_edit_group->trunction_len = ptap_info_entry->u.tap_group.trunction_len;

    sal_memcpy(&temp_edit_group->key, &key, sizeof(key));
    temp_edit_group->count++;

    temp_edit_group->port_res_hash = 
        ctc_hash_create(CTC_SAI_TAP_EGRESS_PORT_RES, 
                        CTC_SAI_TAP_EGRESS_PORT_BLOCK_SIZE, 
                        __edit_group_port_res_hash_make, 
                        __edit_group_port_res_hash_cmp);

    CTC_SAI_ERROR_GOTO(ctc_sai_l2mc_alloc_offset(&group_id),ret,out);
    temp_edit_group->nexthop_group_id = group_id;
    
    CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&nexthop_id),ret,out);
    temp_edit_group->nexthop_id = nexthop_id;

    /* create nexthop of mcast */
    sal_memset(&nh_mcast_group, 0, sizeof(nh_mcast_group));
    nh_mcast_group.mc_grp_id = temp_edit_group->nexthop_group_id;
    if(SAI_IS_BIT_SET(temp_edit_group->key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        nh_mcast_group.is_mirror = TRUE;            /* use to CC iloop->mirror for trunction */
    }
    nh_mcast_group.mem_info.is_source_check_dis = TRUE;
    ctc_nh_add_mcast(temp_edit_group->nexthop_id, &nh_mcast_group);

    CTC_SAI_DEBUG("%s%d:new edit group: groupid = %d, nexthopid = %d count = %d\n",__FUNCTION__,__LINE__,
        group_id,nexthop_id,temp_edit_group->count);

    __edit_group_update_tap_group_member(temp_edit_group ,ptap_info_entry, TRUE);

    ctc_slist_add_tail(tap_group->edit_header, &temp_edit_group->list_node);

    return temp_edit_group;

out:
    if(0 != temp_edit_group->nexthop_id){
        ctc_sai_nexthop_free_offset(temp_edit_group->nexthop_id);
    }

    if(0 != temp_edit_group->nexthop_group_id){
        ctc_sai_l2mc_free_offset(temp_edit_group->nexthop_group_id);
    }

    if(temp_edit_group){
        __edit_group_free(temp_edit_group);
        temp_edit_group = NULL;
    }

    return temp_edit_group;
}

static ctc_sai_tap_group_edit_group *
ctc_sai_tap_group_get_edit_group_by_type(uint32_t type, ctc_sai_tap_info_t *ptap_info_entry)
{
    struct ctc_slistnode_s          *node       = NULL;
    ctc_sai_tap_group_edit_group    *temp_edit_group = NULL;
    ctc_sai_tap_group_t			    *tap_group  = NULL;

    sal_assert(ptap_info_entry != NULL);

    tap_group = &ptap_info_entry->u.tap_group;
    
    /* foreach .get entry by key */
    CTC_SLIST_LOOP(tap_group->edit_header, node){
        temp_edit_group = (ctc_sai_tap_group_edit_group*)node;

        if(SAI_IS_BIT_SET(temp_edit_group->key.edit_flag, type)){
            CTC_SAI_DEBUG("%s%d:get type edit group:type = 0x%x, groupid = %d, nexthopid = %d count = %d\n",__FUNCTION__,__LINE__,
                type,temp_edit_group->nexthop_group_id,temp_edit_group->nexthop_id,temp_edit_group->count);
            return temp_edit_group;
        }
    }

    return NULL;
}

void
ctc_sai_tap_group_release_edit_group(ctc_sai_tap_group_edit_group *edit_group, ctc_sai_tap_info_t *ptap_info_entry)
{
    sal_assert(edit_group != NULL);

    edit_group->count--;

    CTC_SAI_DEBUG("%s%d:release edit group: groupid = %d, nexthopid = %d count = %d\n",__FUNCTION__,__LINE__,
                edit_group->nexthop_group_id,edit_group->nexthop_id,edit_group->count);

    if(edit_group->count <=0){
        CTC_SAI_DEBUG("%s%d:release edit group ing\n",__FUNCTION__,__LINE__);
        /* free tap group member */
        __edit_group_update_tap_group_member(edit_group, ptap_info_entry, FALSE);

        ctc_hash_free(edit_group->port_res_hash);/*added by yejl for bug 54490, 2019-11-21*/
            
        /* free group */
        ctc_nh_remove_mcast(edit_group->nexthop_id);

        /* remove trunction res */
        if(edit_group->trunction_count){
            __ctc_sai_tap_group_del_trunction_res(edit_group);
        }

        /* free index res */
        ctc_sai_l2mc_free_offset(edit_group->nexthop_group_id);
        ctc_sai_nexthop_free_offset(edit_group->nexthop_id);

        ctc_slist_delete_node(ptap_info_entry->u.tap_group.edit_header, &edit_group->list_node);
        __edit_group_free(edit_group);
    }

}

#define __TRUNCTION_FUNC
#ifdef USW
static int32
__edit_group_traversal_update_trunction_len(void* array_data, void* user_data)
{
    uint32    *trunction_len  =   user_data;
    ctc_sai_tap_info_t           *ptap_info  = array_data;
    ctc_sai_tap_group_edit_group *edit_group = NULL;
    ctc_mirror_dest_t             dest_mirror;

    if(NULL == ptap_info)
        return 0;

    if(CTC_SAI_TAP_INFO_TYPE_MEMBER == ptap_info->tap_info_type)
        return 0;

    edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
                                    ptap_info);
    if (NULL == edit_group)
        return 0;
        
    if (edit_group->trunction_len == *trunction_len)
    {
        return 0;
    }
    else
    {
        edit_group->trunction_len = *trunction_len;
    }

    if(edit_group->trunction_count > 0){
        /* update mirror trunction res */
        sal_memset(&dest_mirror, 0, sizeof(dest_mirror));
        dest_mirror.session_id = edit_group->session_id;
        dest_mirror.rspan.nh_id = edit_group->illop_nexthop_id;
        dest_mirror.dir = CTC_BOTH_DIRECTION;
        dest_mirror.is_rspan = TRUE;
        dest_mirror.vlan_valid = FALSE;
        dest_mirror.type = CTC_MIRROR_L2SPAN_SESSION;
#ifdef USW 
        dest_mirror.truncated_len = edit_group->trunction_len;
#endif
        ctc_mirror_add_session(&dest_mirror);

        dest_mirror.session_id = edit_group->session_id;;
        dest_mirror.rspan.nh_id = edit_group->illop_nexthop_id;
        dest_mirror.dir = CTC_BOTH_DIRECTION;
        dest_mirror.is_rspan = TRUE;
        dest_mirror.vlan_valid = FALSE;
        dest_mirror.type = CTC_MIRROR_ACLLOG_SESSION;
#ifdef USW 
        dest_mirror.truncated_len = edit_group->trunction_len;
#endif
        ctc_mirror_add_session(&dest_mirror);
    }
    
    return 0;
}
#endif

sai_status_t
ctc_sai_tap_group_set_trunction_len(
    _In_  const sai_attribute_t *attr)
{
    int32_t             sdk_ret = CTC_E_NONE;
    uint32_t            length  = 0;

    if (0 == attr->value.s32)
    {
        length = CTC_SAI_TAP_TRUNCTION_LENGTH_MAX;
    }
    else
    {
        length = attr->value.s32;
        if(length < CTC_SAI_TAP_TRUNCTION_LENGTH_MIN) {
            length = CTC_SAI_TAP_TRUNCTION_LENGTH_MIN;
        }

        if(length > CTC_SAI_TAP_TRUNCTION_LENGTH_MAX) {
            length = CTC_SAI_TAP_TRUNCTION_LENGTH_MAX;
        }
    }

    /* update mirror trunction length for duet2 */
#ifdef USW
    ctc_vector_traverse(g_sai_tap_group_master.ptap_group_vector, 
                       (vector_traversal_fn)__edit_group_traversal_update_trunction_len, &length);
#endif

    return sdk_ret;
}


sai_status_t
__ctc_sai_tap_group_add_trunction_res(ctc_sai_tap_group_edit_group  *pedit_group)
{
    ctc_internal_port_assign_para_t port_assign_iloop;
    sai_status_t                    ret 		            = SAI_STATUS_SUCCESS;
    uint32_t                        iloop_nexthop           = 0;
    uint32_t                        mirror_session          = 0;
    ctc_loopback_nexthop_param_t    nh_param;
    ctc_mirror_dest_t               dest_mirror;

    /* 1: alloc loopback nexthop_id offset */
    CTC_SAI_ERROR_GOTO(ctc_sai_nexthop_alloc_offset(&iloop_nexthop),ret,err);
    
    /* 2: alloc mirror offset */
    CTC_SAI_ERROR_GOTO(ctc_sai_mirror_alloc_offset(&mirror_session),ret,err);

    CTC_SAI_DEBUG("%s%d:iloop_nexthop = %d\n",__FUNCTION__,__LINE__,iloop_nexthop);
    CTC_SAI_DEBUG("%s%d:mirror_session = %d\n",__FUNCTION__,__LINE__,mirror_session);

    /* 3: alloc sdk iloop offset */
    sal_memset(&port_assign_iloop, 0x0, sizeof(ctc_internal_port_assign_para_t));
    port_assign_iloop.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    port_assign_iloop.gchip = 0;
    CTC_SAI_ERROR_GOTO(ctc_alloc_internal_port(&port_assign_iloop),ret,err);

    /* 4: bind illop2mcast */
    sal_memset(&nh_param, 0x0, sizeof(ctc_loopback_nexthop_param_t));
    nh_param.lpbk_lport = port_assign_iloop.inter_port;
    port_id[mirror_session] = nh_param.lpbk_lport;
    CTC_SAI_ERROR_GOTO(ctc_nh_add_iloop(iloop_nexthop, &nh_param),ret,err1);

    /* 5: bind iloop to mcast */
    CTC_SAI_ERROR_GOTO(ctc_port_set_cross_connect(port_assign_iloop.inter_port, 
        pedit_group->nexthop_id),ret,err2);
    
    /* 5: bind iloop to mirror */
    sal_memset(&dest_mirror, 0, sizeof(dest_mirror));
    dest_mirror.session_id = mirror_session;
    dest_mirror.rspan.nh_id = iloop_nexthop;
    dest_mirror.dir = CTC_BOTH_DIRECTION;
    dest_mirror.is_rspan = TRUE;
    dest_mirror.vlan_valid = FALSE;
    dest_mirror.type = CTC_MIRROR_L2SPAN_SESSION;
#ifdef USW 
    dest_mirror.truncated_len = pedit_group->trunction_len;
#endif
    ctc_mirror_add_session(&dest_mirror);

    dest_mirror.session_id = mirror_session;
    dest_mirror.rspan.nh_id = iloop_nexthop;
    dest_mirror.dir = CTC_BOTH_DIRECTION;
    dest_mirror.is_rspan = TRUE;
    dest_mirror.vlan_valid = FALSE;
    dest_mirror.type = CTC_MIRROR_ACLLOG_SESSION;
#ifdef USW 
    dest_mirror.truncated_len = pedit_group->trunction_len;
#endif
    ctc_mirror_add_session(&dest_mirror);

    pedit_group->illop_nexthop_id = iloop_nexthop;
    pedit_group->session_id       = mirror_session;
    
    return ret;

err2:
    ctc_nh_remove_iloop(iloop_nexthop);
err1:
    ctc_free_internal_port(&port_assign_iloop);
err:
    if(mirror_session)  ctc_sai_mirror_free_offset(mirror_session);
    if(iloop_nexthop)  ctc_sai_nexthop_free_offset(iloop_nexthop);
    return ret;
}

sai_status_t
__ctc_sai_tap_group_del_trunction_res(ctc_sai_tap_group_edit_group  *pedit_group)
{
    ctc_internal_port_assign_para_t     port_assign_iloop;

    CTC_SAI_DEBUG("%s%d:iloop_nexthop = %d\n",__FUNCTION__,__LINE__,pedit_group->illop_nexthop_id);
    CTC_SAI_DEBUG("%s%d:mirror_session = %d\n",__FUNCTION__,__LINE__,pedit_group->session_id);
    
    sal_memset(&port_assign_iloop, 0x0, sizeof(ctc_internal_port_assign_para_t));

    ctc_nh_remove_iloop(pedit_group->illop_nexthop_id);

    port_assign_iloop.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    port_assign_iloop.gchip = 0;
    port_assign_iloop.inter_port = port_id[pedit_group->session_id];
    ctc_free_internal_port(&port_assign_iloop);
    port_id[pedit_group->session_id] = 0;

    ctc_sai_nexthop_free_offset(pedit_group->illop_nexthop_id);
    ctc_sai_mirror_free_offset(pedit_group->session_id);

    pedit_group->illop_nexthop_id = 0;
    pedit_group->session_id       = 0;
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_tap_group_add_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_edit_group_key    key;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;
    
    sal_memset(&key, 0, sizeof(key));
    SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
    
    edit_group = ctc_sai_tap_group_get_edit_group(key, ptap_info_entry);
    if(NULL == edit_group){
        goto out;
    }

    sal_memset(&key, 0, sizeof(key));
    SAI_SET_BIT(key.edit_flag, CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION);
    
    edit_group = ctc_sai_tap_group_get_edit_group(key, ptap_info_entry);
    if(NULL == edit_group){
        goto out;
    }
    
out:
    return ret;
}

sai_status_t
ctc_sai_tap_group_del_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		     = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_t         *ptap_group_info = NULL;
    ctc_sai_tap_group_edit_group    *temp_edit_group = NULL;
    struct ctc_slistnode_s          *node       = NULL;
    struct ctc_slistnode_s          *n_node       = NULL;

    ptap_group_info = &ptap_info_entry->u.tap_group;

    /* foreach .get entry by key */
    CTC_SLIST_LOOP_DEL(ptap_group_info->edit_header, node, n_node){
        temp_edit_group = (ctc_sai_tap_group_edit_group*)node;

        ctc_sai_tap_group_release_edit_group(temp_edit_group, ptap_info_entry);
    }

    return ret;
}

sai_status_t
ctc_sai_tap_group_get_session_id(sai_object_id_t tap_group_id, uint32_t  *psession_id)
{
    ctc_sai_tap_info_t          *ptap_info             = NULL;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("%s%d: tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);

    /* 1: check tap_info_entry */
    ptap_info = ctc_sai_tap_get_by_oid(tap_group_id);
    if(NULL == ptap_info)
    {
        CTC_SAI_DEBUG("%s%d:item not found tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
 
    edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
                                    ptap_info);

    /* 2: check trunction & if need then alloc trunction res */
    if(edit_group->trunction_count == 0){
        /* realloc trunction res */
        __ctc_sai_tap_group_add_trunction_res(edit_group);
    }

    edit_group->trunction_count++;

    *psession_id = edit_group->session_id;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_tap_group_put_session_id(sai_object_id_t tap_group_id)
{
    ctc_sai_tap_info_t          *ptap_info             = NULL;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("%s%d: tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);
    
    /* 1: check tap_info_entry */
    ptap_info = ctc_sai_tap_get_by_oid(tap_group_id);
    if(NULL == ptap_info)
    {
        CTC_SAI_DEBUG("%s%d:item not found tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
                                    ptap_info);
    
    /* 2: check trunction & if need then alloc trunction res */
    edit_group->trunction_count--;
    if(edit_group->trunction_count == 0){
        /* realloc trunction res */
        __ctc_sai_tap_group_del_trunction_res(edit_group);
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
ctc_sai_tap_group_get_nexthop_id(sai_object_id_t tap_group_id, uint32_t  *pnexthop_id)
{
    ctc_sai_tap_info_t          *ptap_info             = NULL;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_DEBUG("%s%d: tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);

    /* 1: check tap_info_entry */
    ptap_info = ctc_sai_tap_get_by_oid(tap_group_id);
    if(NULL == ptap_info)
    {
        CTC_SAI_DEBUG("%s%d:item not found tap_group_oid = 0x%llx\n",
            __FUNCTION__,__LINE__,tap_group_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL,
                                    ptap_info);
    
    *pnexthop_id = edit_group->nexthop_id;

    return SAI_STATUS_SUCCESS;
}

/*
 * 
 */
static void 
__ingress_add_entry(ctc_sai_tap_info_t *ptap_info_entry,
                        sai_object_id_t portid)
{
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    ctc_sai_tap_info_t          *ptap_info1             = NULL;
    uint32                      sdk_gport        = 0;
    ctc_scl_default_action_t    sdk_scl_def_action;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;
    ctc_sai_tap_group_edit_group_key    edit_key    = {};

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;
    ptap_info1 = ctc_sai_tap_get_by_oid(ptap_group_member_info->tap_group_oid);

    ctc_sai_port_objectid_to_gport(portid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:update member port = 0x%llx\n",__FUNCTION__,__LINE__,portid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    CTC_SAI_DEBUG("%s%d:ingress edit_key = 0x%lx\n",__FUNCTION__,__LINE__,
        ptap_group_member_info->u.ingress.edit_key.edit_flag);

    ctc_l2pdu_set_port_action(sdk_gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_FWD);
    
    /* check trunction and set mirror or scl */
    if(SAI_IS_BIT_SET(ptap_group_member_info->u.ingress.edit_key.edit_flag,
        CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
                                    ptap_info1);

        /* 1: check trunction & if need then alloc trunction res */
        if(edit_group->trunction_count == 0){
            /* realloc trunction res */
            __ctc_sai_tap_group_add_trunction_res(edit_group);
        }
        
        /* set mirror */
        ctc_mirror_set_port_en(sdk_gport, CTC_INGRESS, TRUE, edit_group->session_id);
        edit_group->trunction_count++;
    }else{
        if(SAI_IS_BIT_SET(ptap_group_member_info->u.ingress.edit_key.edit_flag,
            CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL)){
            edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                        CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL,
                                        ptap_info1);
        }else{
            sal_memcpy(&edit_key, &ptap_group_member_info->u.ingress.edit_key, sizeof(edit_key));
            edit_group = ctc_sai_tap_group_get_edit_group(edit_key, ptap_info1);   
            if(NULL == edit_group){
                return ;
            }
            ptap_group_member_info->u.ingress.pcustom_edit_group = edit_group;
        }
    
        /* set scl */
        sal_memset(&sdk_scl_def_action, 0, sizeof(sdk_scl_def_action));
        sdk_scl_def_action.gport = sdk_gport;
        /*added by yejl for tsingma tap, 2019-08-06*/
        #ifdef TSINGMA
        sdk_scl_def_action.scl_id = 1;
        #endif
        /*ended*/
        sdk_scl_def_action.action.u.igs_action.flag = CTC_SCL_IGS_ACTION_FLAG_REDIRECT;
        sdk_scl_def_action.action.u.igs_action.nh_id = edit_group->nexthop_id;

        if (ptap_group_member_info->u.ingress.mark_source)
        {
            sdk_scl_def_action.action.u.igs_action.flag |= CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT;
            sdk_scl_def_action.action.u.igs_action.vlan_edit.stag_op  = CTC_VLAN_TAG_OP_ADD;
            sdk_scl_def_action.action.u.igs_action.vlan_edit.svid_sl  = CTC_VLAN_TAG_SL_NEW;
            sdk_scl_def_action.action.u.igs_action.vlan_edit.svid_new = 
                                                        (uint16_t)ptap_group_member_info->u.ingress.mark_source;
        }else if (ptap_group_member_info->u.ingress.untag){
            sdk_scl_def_action.action.u.igs_action.flag |= CTC_SCL_IGS_ACTION_FLAG_VLAN_EDIT;
            if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_DOUBLE_VLAN == ptap_group_member_info->u.ingress.untag)
            {
                sdk_scl_def_action.action.u.igs_action.vlan_edit.stag_op = CTC_VLAN_TAG_OP_DEL;
                sdk_scl_def_action.action.u.igs_action.vlan_edit.ctag_op = CTC_VLAN_TAG_OP_DEL;
            }
            else if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_SVLAN == ptap_group_member_info->u.ingress.untag)
            {
                sdk_scl_def_action.action.u.igs_action.vlan_edit.stag_op = CTC_VLAN_TAG_OP_DEL;
                sdk_scl_def_action.action.u.igs_action.vlan_edit.ctag_op = CTC_VLAN_TAG_OP_NONE;
            }
            else if(SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_CVLAN == ptap_group_member_info->u.ingress.untag)
            {
                sdk_scl_def_action.action.u.igs_action.vlan_edit.stag_op = CTC_VLAN_TAG_OP_NONE;
                sdk_scl_def_action.action.u.igs_action.vlan_edit.ctag_op = CTC_VLAN_TAG_OP_DEL;
            }
        }

        ctc_scl_set_default_action(&sdk_scl_def_action);
    }
}

static void
__ingress_del_entry(ctc_sai_tap_info_t *ptap_info_entry,
                        sai_object_id_t portid)
{
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    ctc_sai_tap_info_t          *ptap_info1             = NULL;
    uint32                      sdk_gport        = 0;
    ctc_scl_default_action_t    sdk_scl_def_action;
    ctc_sai_tap_group_edit_group        *edit_group = NULL;

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;
    ptap_info1 = ctc_sai_tap_get_by_oid(ptap_group_member_info->tap_group_oid);

    ctc_sai_port_objectid_to_gport(portid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:update member port = 0x%llx\n",__FUNCTION__,__LINE__,portid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    
    ctc_l2pdu_set_port_action(sdk_gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU);

    /* 1: check trunction and set mirror or scl */
    if(SAI_IS_BIT_SET(ptap_group_member_info->u.ingress.edit_key.edit_flag,
        CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION)){
        edit_group = ctc_sai_tap_group_get_edit_group_by_type(
                                    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
                                    ptap_info1);
        
        /* set mirror */
        ctc_mirror_set_port_en(sdk_gport, CTC_INGRESS, FALSE, edit_group->session_id);
        edit_group->trunction_count--;

        /* 2: check trunction & if need then free trunction res */
        if(edit_group->trunction_count == 0){
            /* realloc trunction res */
            __ctc_sai_tap_group_del_trunction_res(edit_group);
        }
    }else{
        if(ptap_group_member_info->u.ingress.pcustom_edit_group){
            ctc_sai_tap_group_release_edit_group(ptap_group_member_info->u.ingress.pcustom_edit_group, ptap_info1);
            ptap_group_member_info->u.ingress.pcustom_edit_group = NULL;
        }
    
        /* set scl */
        sal_memset(&sdk_scl_def_action, 0, sizeof(sdk_scl_def_action));
        /*added by yejl for tsingma tap, 2019-08-06*/
        #ifdef TSINGMA
        sdk_scl_def_action.scl_id = 1;
        #endif
        /*ended*/
        sdk_scl_def_action.gport = sdk_gport;
#if 0		
        sdk_scl_def_action.action.u.igs_action.flag = CTC_SCL_IGS_ACTION_FLAG_REDIRECT;
        sdk_scl_def_action.action.u.igs_action.nh_id = CTC_NH_RESERVED_NHID_FOR_DROP;
#else
		//sdk_scl_def_action.action.u.igs_action.flag = CTC_SCL_IGS_ACTION_FLAG_DENY_BRIDGE;
    #ifdef GREATBELT
        sdk_scl_def_action.action.u.igs_action.flag     = CTC_SCL_IGS_ACTION_FLAG_REDIRECT;
        sdk_scl_def_action.action.u.igs_action.nh_id    = CTC_NH_RESERVED_NHID_FOR_DROP;
    #else
	    sdk_scl_def_action.action.u.igs_action.flag     = CTC_SCL_IGS_ACTION_FLAG_DENY_BRIDGE;
    #endif
#endif		
        ctc_scl_set_default_action(&sdk_scl_def_action);
    }
}

/*
 * 
 */
static int 
__egress_add_entry(ctc_sai_tap_group_member_t *ptap_egr_member_entry,
                        ctc_sai_tap_group_t        *ptap_group_entry)
{
    struct ctc_slistnode_s          *node               = NULL;
    ctc_sai_tap_group_edit_group    *temp_edit_group    = NULL;
    
    sal_assert(ptap_egr_member_entry != NULL);
    sal_assert(ptap_group_entry != NULL);

    CTC_SAI_DEBUG("%s%d:foreach edit group",__FUNCTION__,__LINE__);
    
    /* foreach .get entry by key */
    CTC_SLIST_LOOP(ptap_group_entry->edit_header, node){
        temp_edit_group = (ctc_sai_tap_group_edit_group*)node;

        CTC_SAI_DEBUG("%s%d:foreach one edit group: groupid = %d, nexthopid = %d count = %d\n",
            __FUNCTION__,__LINE__,
            temp_edit_group->nexthop_group_id,temp_edit_group->nexthop_id,temp_edit_group->count);

        __edit_group_egress_member_add_with_edit_group(temp_edit_group, ptap_egr_member_entry);
    }

    return 0;
}

static int 
__egress_del_entry(ctc_sai_tap_group_member_t *ptap_egr_member_entry,
                        ctc_sai_tap_group_t        *ptap_group_entry)
{
    struct ctc_slistnode_s          *node               = NULL;
    struct ctc_slistnode_s          *n_node               = NULL;
    ctc_sai_tap_group_edit_group    *temp_edit_group    = NULL;
    
    sal_assert(ptap_egr_member_entry != NULL);
    sal_assert(ptap_group_entry != NULL);

    CTC_SAI_DEBUG("%s%d:foreach edit group",__FUNCTION__,__LINE__);
    
    /* foreach .get entry by key */
    CTC_SLIST_LOOP_DEL(ptap_group_entry->edit_header, node,n_node){
        temp_edit_group = (ctc_sai_tap_group_edit_group*)node;

        CTC_SAI_DEBUG("%s%d:foreach one edit group: groupid = %d, nexthopid = %d count = %d\n",
            __FUNCTION__,__LINE__,
            temp_edit_group->nexthop_group_id,temp_edit_group->nexthop_id,temp_edit_group->count);

        __edit_group_egress_member_del_with_edit_group(temp_edit_group, ptap_egr_member_entry);
    }

    return 0;
}


static int32
__traversal_ingress_update_lag_member(void* array_data, void* user_data)
{
    ctc_sai_lag_notifier_t  *plag_nf    = user_data;
    ctc_sai_tap_info_t      *ptap_info  = array_data;
    ctc_sai_tap_group_member_t  *ptap_member = NULL;

    sal_assert(plag_nf != NULL);

    if(NULL == ptap_info)
        return 0;

    if(CTC_SAI_TAP_INFO_TYPE_GROUP == ptap_info->tap_info_type)
        return 0;

    ptap_member = &ptap_info->u.tap_group_member;

    if(CTC_SAI_TAP_GROUP_MEMBER_TYPE_EGRESS == ptap_member->tap_member_stage)
        return 0;

    if(ptap_member->port_oid != plag_nf->lag_id)
        return 0;

    if(CTC_SAI_LAG_NF_NEW_LAG_MEMBER == plag_nf->op){
        __ingress_add_entry(ptap_info, plag_nf->lag_member_port_id);
    }else{
        __ingress_del_entry(ptap_info, plag_nf->lag_member_port_id);
    }
    
    return 0;
}

int32_t
ctc_tap_group_lag_notifier_callback(
    _In_  struct ctc_sai_notifier_block *nb,
    _In_  uint32_t action,
    _In_  void *data)
{
    switch(action){
    case CTC_SAI_LAG_NF_NEW_LAG_MEMBER:
    case CTC_SAI_LAG_NF_DEL_LAG_MEMBER:    
        ctc_vector_traverse(g_sai_tap_group_master.ptap_group_vector, 
                            (vector_traversal_fn)__traversal_ingress_update_lag_member, data);
        break;
    }

    return 0;
}

sai_status_t
ctc_sai_tap_group_ingress_add_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    uint32_t                    port_idx            = 0;
    uint32_t                    port_max            = CTC_MAX_LPORT;
    sai_object_id_t             *parr_port_id       = NULL;
    sai_attribute_t             lag_ports;

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);  
    CTC_SAI_DEBUG("%s%d:trunction = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.ingress.trunction);
//    CTC_SAI_DEBUG("%s%d:group_trunction_count = %d\n",__FUNCTION__,__LINE__,ptap_group_info->trunction_count);
    CTC_SAI_DEBUG("%s%d:mark_source = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.ingress.mark_source);
    CTC_SAI_DEBUG("%s%d:untag = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.ingress.untag);
    
    

    /* 2: foreach linkagg member port */
    if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(ptap_group_member_info->port_oid)){
        port_max = CTC_MAX_LPORT;
        parr_port_id = XCALLOC(MEM_FEA_TAP, sizeof(sai_object_id_t) * port_max);
        if(!parr_port_id){
            ret = SAI_STATUS_NO_MEMORY;
            CTC_SAI_DEBUG("%s%d:no member\n",__FUNCTION__,__LINE__);
            goto out;
        }

        lag_ports.value.objlist.list = parr_port_id;
        lag_ports.value.objlist.count= port_max;
        CTC_SAI_ERROR_GOTO(ctc_sai_lag_get_ports(ptap_group_member_info->port_oid,&lag_ports),ret,out);

        for(port_idx = 0; port_idx < lag_ports.value.objlist.count;port_idx++)
        {
            __ingress_add_entry(ptap_info_entry, lag_ports.value.objlist.list[port_idx]);
        }
        XFREE(MEM_FEA_TAP, parr_port_id);
    }else{
        __ingress_add_entry(ptap_info_entry, ptap_group_member_info->port_oid);
    }
    
out:
    return ret;
}

sai_status_t
ctc_sai_tap_group_ingress_del_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    uint32_t                    port_idx            = 0;
    uint32_t                    port_max            = CTC_MAX_LPORT;
    sai_object_id_t             *parr_port_id       = NULL;
    sai_attribute_t             lag_ports;

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    CTC_SAI_DEBUG("%s%d:trunction = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.ingress.trunction);
//    CTC_SAI_DEBUG("%s%d:group_trunction_count = %d\n",__FUNCTION__,__LINE__,ptap_group_info->trunction_count);

    /* 1: foreach linkagg member port */
    if(SAI_OBJECT_TYPE_LAG == CTC_SAI_OBJECT_TYPE_GET(ptap_group_member_info->port_oid)){
        port_max = CTC_MAX_LPORT;
        parr_port_id = XCALLOC(MEM_FEA_TAP, sizeof(sai_object_id_t) * port_max);
        if(!parr_port_id){
            ret = SAI_STATUS_NO_MEMORY;
            CTC_SAI_DEBUG("%s%d:no member\n",__FUNCTION__,__LINE__);
            goto out;
        }

        lag_ports.value.objlist.list = parr_port_id;
        lag_ports.value.objlist.count= port_max;
        CTC_SAI_ERROR_GOTO(ctc_sai_lag_get_ports(ptap_group_member_info->port_oid,&lag_ports),ret,out);

        for(port_idx = 0; port_idx < lag_ports.value.objlist.count;port_idx++)
        {
            __ingress_del_entry(ptap_info_entry, lag_ports.value.objlist.list[port_idx]);
        }
        XFREE(MEM_FEA_TAP, parr_port_id);
    }else{
        __ingress_del_entry(ptap_info_entry, ptap_group_member_info->port_oid);
    }
#if 0
    /* 2: check trunction & if need then free trunction res */
    if(ptap_group_member_info->u.ingress.trunction && (ptap_group_info->trunction_count == 0)){
        /* realloc trunction res */
        __ctc_sai_tap_group_del_trunction_res(ptap_group_info);
    }
#endif    
out:
    return ret;   
}

sai_status_t
ctc_sai_tap_group_egress_add_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    ctc_sai_tap_group_t         *ptap_group_info        = NULL;
    ctc_sai_tap_info_t          *ptap_info1             = NULL;
    uint32                      sdk_gport               = 0;
//    uint32                      timestamp_nhid          = 0;
//    ctc_mcast_nh_param_group_t  sdk_nh_mcast_group;

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;
    ptap_info1 = ctc_sai_tap_get_by_oid(ptap_group_member_info->tap_group_oid);
    ptap_group_info        = &ptap_info1->u.tap_group;

    ctc_sai_port_objectid_to_gport(ptap_group_member_info->port_oid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    CTC_SAI_DEBUG("%s%d:timestamp = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.egress.timestamp);
//    CTC_SAI_DEBUG("%s%d:nexthop_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->nexthop_id);
//    CTC_SAI_DEBUG("%s%d:nexthop_group_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->nexthop_group_id);
//    CTC_SAI_DEBUG("%s%d:trunction_nexthop_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->trunction_nexthop_id);
//    CTC_SAI_DEBUG("%s%d:trunction_nexthop_group_id = %d\n",__FUNCTION__,__LINE__,
//                                            ptap_group_info->trunction_nexthop_group_id);


    __egress_add_entry(ptap_group_member_info, ptap_group_info);

    return ret;

//err:
//    CTC_SAI_DEBUG("%s%d:alloc_timestamp_faild = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    return ret;   
}

sai_status_t
ctc_sai_tap_group_egress_del_entry(ctc_sai_tap_info_t *ptap_info_entry)
{
    sai_status_t                ret 		            = SAI_STATUS_SUCCESS;
    ctc_sai_tap_group_member_t  *ptap_group_member_info = NULL;
    ctc_sai_tap_group_t         *ptap_group_info        = NULL;
    ctc_sai_tap_info_t          *ptap_info1             = NULL;
    uint32                      sdk_gport        = 0;
//    ctc_mcast_nh_param_group_t  sdk_nh_mcast_group;

    ptap_group_member_info = &ptap_info_entry->u.tap_group_member;
    ptap_info1 = ctc_sai_tap_get_by_oid(ptap_group_member_info->tap_group_oid);
    ptap_group_info        = &ptap_info1->u.tap_group;

    ctc_sai_port_objectid_to_gport(ptap_group_member_info->port_oid, &sdk_gport);

    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->tap_group_oid);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,ptap_group_member_info->port_oid);
    CTC_SAI_DEBUG("%s%d:gport = 0x%lx\n",__FUNCTION__,__LINE__,sdk_gport);
    CTC_SAI_DEBUG("%s%d:timestamp = %d\n",__FUNCTION__,__LINE__,ptap_group_member_info->u.egress.timestamp);
//    CTC_SAI_DEBUG("%s%d:nexthop_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->nexthop_id);
//    CTC_SAI_DEBUG("%s%d:nexthop_group_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->nexthop_group_id);
//    CTC_SAI_DEBUG("%s%d:trunction_nexthop_id = %d\n",__FUNCTION__,__LINE__,ptap_group_info->trunction_nexthop_id);
//    CTC_SAI_DEBUG("%s%d:trunction_nexthop_group_id = %d\n",__FUNCTION__,__LINE__,
//                                            ptap_group_info->trunction_nexthop_group_id);

    __egress_del_entry(ptap_group_member_info, ptap_group_info);
    
    return ret;    
}



sai_status_t 
ctc_sai_create_tap_group(
	_Out_ sai_object_id_t		*tap_group_id,
    _In_ const sai_attribute_t  *attr,
	_In_ uint32_t 				attr_count)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_info_t     *ptap_info_entry     = NULL;
    const sai_attribute_t  *attr_name           = NULL;
    const sai_attribute_t  *attr_trun           = NULL;

    CTC_SAI_DEBUG_FUNC();

    attr_name = 
        ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_ATTR_NAME,
                                            attr,
                                            attr_count,
                                            NULL);
    assert(attr_name != NULL);

    CTC_SAI_DEBUG("ctc_sai_create_tap_group:name = %s\n",attr_name->value.chardata);

    attr_trun = 
        ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_ATTR_TRUNCTION_LEN,
                                            attr,
                                            attr_count,
                                            NULL);
    assert(attr_trun != NULL);

    /* 1: create tap entry */
    CTC_SAI_ERROR_GOTO(ctc_sai_tap_group_create(&ptap_info_entry),ret,out);

    /* 2: copy tap-group name */
    sal_strcpy(ptap_info_entry->u.tap_group.name, attr_name->value.chardata);
    ptap_info_entry->u.tap_group.trunction_len = attr_trun->value.u32;

    /* 3: add tap-group entry */
    if(SAI_STATUS_SUCCESS != ctc_sai_tap_group_add_entry(ptap_info_entry))
    {
        ctc_sai_tap_group_remove(ptap_info_entry);
    }

    *tap_group_id = ptap_info_entry->tap_oid;

out:
    
    return ret;
}

sai_status_t 
ctc_sai_remove_tap_group(
    _In_ sai_object_id_t 		tap_group_id)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_info_t      *ptap_info_entry    = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_DEBUG("ctc_sai_remove_tap_group:tap_group_id = 0x%llx\n",tap_group_id);

    ptap_info_entry = ctc_sai_tap_get_by_oid(tap_group_id);
    if(NULL == ptap_info_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 1: remove tap-group entry */
    ctc_sai_tap_group_del_entry(ptap_info_entry);

    /* 3: free tap entry */
    ctc_sai_tap_group_remove(ptap_info_entry);

    return ret;
}

sai_status_t 
ctc_sai_set_tap_group_attribute(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ const sai_attribute_t  *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_get_tap_group_attribute(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ uint32_t 				attr_count,
    _Inout_ sai_attribute_t 	*attr_list)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_create_tap_group_member(
	_Out_ sai_object_id_t		*tap_group_id,
    _In_ const sai_attribute_t  *attr_list,
	_In_ uint32_t 				attr_count)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_info_t      *ptap_info_entry    = NULL;
    ctc_sai_tap_group_member_t *ptap_group_member     = NULL;
    const sai_attribute_t         *attr               = NULL;
    const sai_attribute_t         *attr_stage         = NULL;
    const sai_attribute_t         *attr_tapgroup_id   = NULL;
    const sai_attribute_t         *attr_port_id       = NULL;

    CTC_SAI_DEBUG_FUNC();

    attr_stage = 
        ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_STAGE,
                                            attr_list,
                                            attr_count,
                                            NULL);

    attr_tapgroup_id = 
        ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ENTRY_ATTR_TAP_GROUP_ID,
                                            attr_list,
                                            attr_count,
                                            NULL);

    attr_port_id = 
        ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ENTRY_ATTR_PORT_ID,
                                            attr_list,
                                            attr_count,
                                            NULL);
    assert(attr_stage != NULL);
    assert(attr_tapgroup_id != NULL);
    assert(attr_port_id != NULL);

    CTC_SAI_DEBUG("%s%d:stage = %s\n",__FUNCTION__,__LINE__,attr_stage->value.s32 == 1 ? "egress" : "ingress");
    CTC_SAI_DEBUG("%s%d:tap_group_oid = 0x%llx\n",__FUNCTION__,__LINE__,attr_tapgroup_id->value.oid);
    CTC_SAI_DEBUG("%s%d:port_oid = 0x%llx\n",__FUNCTION__,__LINE__,attr_port_id->value.oid);

    if(attr_stage->value.s32 == SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) {
        /* 1: create tap group ingress entry */
        CTC_SAI_ERROR_GOTO(ctc_sai_tap_group_ingress_create(&ptap_info_entry),ret,out);
    }else{
        /* 1: create tap group egress entry */
        CTC_SAI_ERROR_GOTO(ctc_sai_tap_group_egress_create(&ptap_info_entry),ret,out);
    }
    ptap_group_member = &ptap_info_entry->u.tap_group_member;
    
    /* 2: check tap_group exist? */
    if(NULL == ctc_sai_tap_get_by_oid(attr_tapgroup_id->value.oid)){
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 3: copy attr2db */
    /* 3.1 base info */
    ptap_group_member->tap_group_oid = attr_tapgroup_id->value.oid;
    ptap_group_member->port_oid = attr_port_id->value.oid;

    if(attr_stage->value.s32 == SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) {
        /* 3.3 trunction */
        attr = 
            ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_TRUNCATION,
                                                attr_list,
                                                attr_count,
                                                NULL);
        if(NULL != attr && attr->value.booldata){
            ptap_group_member->u.ingress.trunction = attr->value.booldata;
            SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION);
        }else{
            SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
            /* 3.2 mark-source */
            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_MARK_SOURCE,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr && attr->value.booldata){
                attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_MARK_SOURCE_VID,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);

                ptap_group_member->u.ingress.mark_source = attr->value.u16;
            }

            /* 3.3 untag */
            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_UNTAG,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr && attr->value.u32){
                ptap_group_member->u.ingress.untag = attr->value.u32;
            }

            /* 3.3 edit-dmac */
            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_MACDA,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr){
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                SAI_CLEAR_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_DMAC);
                sal_memcpy(ptap_group_member->u.ingress.edit_key.mac_dest, 
                    attr->value.mac, sizeof(sai_mac_t));
            }
            
            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_MACSA,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr){
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                SAI_CLEAR_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_SMAC);
                sal_memcpy(ptap_group_member->u.ingress.edit_key.mac_src, 
                    attr->value.mac, sizeof(sai_mac_t));
            }

            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_IPDA,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr){
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                SAI_CLEAR_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_DIP);
                ptap_group_member->u.ingress.edit_key.ip_dip.addr_family = attr->value.ipaddr.addr_family;
                if (SAI_IP_ADDR_FAMILY_IPV4 == attr->value.ipaddr.addr_family)
                {
                    ptap_group_member->u.ingress.edit_key.ip_dip.addr.ip4 = 
                                attr->value.ipaddr.addr.ip4;
                }
                else
                {
                    sal_memcpy(&ptap_group_member->u.ingress.edit_key.ip_dip.addr.ip6,
                        &attr->value.ipaddr.addr.ip6, sizeof(sai_ip6_t));
                }
            }

            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_IPSA,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr){
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                SAI_CLEAR_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_SIP);
                ptap_group_member->u.ingress.edit_key.ip_sip.addr_family = attr->value.ipaddr.addr_family;
                if (SAI_IP_ADDR_FAMILY_IPV4 == attr->value.ipaddr.addr_family)
                {
                    ptap_group_member->u.ingress.edit_key.ip_sip.addr.ip4 = 
                                attr->value.ipaddr.addr.ip4;
                }
                else
                {
                    sal_memcpy(&ptap_group_member->u.ingress.edit_key.ip_sip.addr.ip6,
                        &attr->value.ipaddr.addr.ip6, sizeof(sai_ip6_t));
                }
               
            }

            attr = 
                ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_VLAN,
                                                    attr_list,
                                                    attr_count,
                                                    NULL);
            if(NULL != attr){
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM);
                SAI_CLEAR_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL);
                SAI_SET_BIT(ptap_group_member->u.ingress.edit_key.edit_flag,
                    CTC_SAI_TAP_GROUP_EDIT_VLAN);
               ptap_group_member->u.ingress.edit_key.vlan = 
                    attr->value.u16;
            }
        }
    }else{
        attr = 
            ctc_sai_attr_get_attr_by_id(SAI_TAP_GROUP_MEMBER_ATTR_TIMESTAMP,
                                                attr_list,
                                                attr_count,
                                                NULL);
        if(NULL != attr && attr->value.booldata){
            ptap_group_member->u.egress.timestamp = attr->value.booldata;
        }
    }

    /* 4: add tap-group entry */
    if(attr_stage->value.s32 == SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) {
        if(SAI_STATUS_SUCCESS != ctc_sai_tap_group_ingress_add_entry(ptap_info_entry))
        {
            ctc_sai_tap_group_ingress_remove(ptap_info_entry);
        }
    }else{
        if(SAI_STATUS_SUCCESS != ctc_sai_tap_group_egress_add_entry(ptap_info_entry))
        {
            ctc_sai_tap_group_egress_remove(ptap_info_entry);
        }
    }

    *tap_group_id = ptap_info_entry->tap_oid;
    
out:
    
    return ret;
}

sai_status_t 
ctc_sai_remove_tap_group_member(
    _In_ sai_object_id_t 		tap_group_id)
{
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_tap_info_t      *ptap_info_entry    = NULL;
    ctc_sai_tap_group_member_t *ptap_group_member = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_DEBUG("%s%d:tap_group_member_oid = 0x%llx\n",__FUNCTION__,__LINE__,tap_group_id);

    ptap_info_entry = ctc_sai_tap_get_by_oid(tap_group_id);
    if(NULL == ptap_info_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* 1: check ingress/egress entry */
    if(ptap_info_entry->tap_info_type != CTC_SAI_TAP_INFO_TYPE_MEMBER){
        return SAI_STATUS_INVALID_PARAMETER;
    }

    ptap_group_member = &ptap_info_entry->u.tap_group_member;
    if(ptap_group_member->tap_member_stage == CTC_SAI_TAP_GROUP_MEMBER_TYPE_INGRESS){
        /* 2: remove tap-group-ingress entry */
        ctc_sai_tap_group_ingress_del_entry(ptap_info_entry);
    }else{
        /* 2: remove tap-group-egress entry */
        ctc_sai_tap_group_egress_del_entry(ptap_info_entry);
    }

    /* 3: free tap entry */
    if(ptap_group_member->tap_member_stage == CTC_SAI_TAP_GROUP_MEMBER_TYPE_INGRESS){
        ctc_sai_tap_group_ingress_remove(ptap_info_entry);
    }else{
        ctc_sai_tap_group_egress_remove(ptap_info_entry);
    }

    return ret;
}

sai_status_t 
ctc_sai_set_tap_group_member_attribute(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ const sai_attribute_t  *attr_list,
    _In_ uint32_t 				attr_count)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t 
ctc_sai_get_tap_group_member_attribute(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ uint32_t 				attr_count,
    _Inout_ sai_attribute_t 	*attr_list)
{
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_TAP_GROUP_INNER_FUNC
sai_status_t
ctc_sai_tap_group_db_init()
{
    stbl_sai_glb_t          *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sal_memset(&g_sai_tap_group_master, 0, sizeof(g_sai_tap_group_master));
    g_sai_tap_group_master.max_count = CTC_SAI_TAP_GROUP_RES;

    if (0 != ctclib_opb_create(&p_sai_glb->tap_group_opb, 1, g_sai_tap_group_master.max_count, "tapgroup"))
    {
        return SAI_STATUS_NO_MEMORY;
    }
    
    g_sai_tap_group_master.ptap_group_vector = 
        ctc_vector_init(CTC_VEC_BLOCK_NUM(CTC_SAI_TAP_GROUP_RES, CTC_SAI_TAP_GROUP_BLOCK_SIZE),
                        CTC_SAI_TAP_GROUP_BLOCK_SIZE);

    g_sai_tap_group_master.egress_misc_hash = 
        ctc_hash_create(CTC_SAI_TAP_GROUP_RES, 
                        CTC_SAI_TAP_GROUP_BLOCK_SIZE, 
                        __egress_misc_hash_make, 
                        __egress_misc_hash_cmp);
    /*added by guoxd for vxlan*/   
    g_sai_tap_group_master.tap_group_vxlan_opf = ctclib_opf_init(1);
    ctclib_opf_init_offset (g_sai_tap_group_master.tap_group_vxlan_opf, 0, 1, 0xffff);
    /*ended by guoxd*/
    return SAI_STATUS_SUCCESS;
}

static struct ctc_sai_notifier_block nb_lag_notifier_cb = {
    .notifier_call = ctc_tap_group_lag_notifier_callback,
};

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    ret = ctc_sai_tap_group_db_init();
    
    preg->init_status = INITIALIZED;

    ctc_sai_lag_evnet_notifier_register(&nb_lag_notifier_cb);

out:
    return ret;

    goto out;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_tap_group_api_t g_sai_api_func = {
		.create_tap_group				= ctc_sai_create_tap_group,
		.remove_tap_group				= ctc_sai_remove_tap_group,
		.set_tap_group_attribute		= ctc_sai_set_tap_group_attribute,
		.get_tap_group_attribute		= ctc_sai_get_tap_group_attribute,
		.create_tap_group_member		= ctc_sai_create_tap_group_member,
		.remove_tap_group_member		= ctc_sai_remove_tap_group_member,
		.set_tap_group_member_attribute	= ctc_sai_set_tap_group_member_attribute,
		.get_tap_group_member_attribute	= ctc_sai_get_tap_group_member_attribute,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_TAP_GROUP,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

#define ________SAI_TAP_GROUP_OUTER_FUNC
sai_status_t
ctc_sai_tap_group_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

