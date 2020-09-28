#if (FEATURE_MODE == 0)
#include "ctc_error.h"
#include "ctc_linklist.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_vlan.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_wlan.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "drv_api.h"
#include "sys_usw_dmps.h"

sys_wlan_master_t* p_usw_wlan_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_WLAN_INIT_CHECK(lchip)                  \
    do {                                                \
        SYS_LCHIP_CHECK_ACTIVE(lchip);                  \
        if (NULL == p_usw_wlan_master[lchip])           \
        {                                               \
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
        }                                               \
    } while (0)

#define CTC_WLAN_VLAN_ID_CHECK(val)                 \
    {                                                   \
        if ((val) > CTC_MAX_VLAN_ID)\
        {\
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid vlan id \n");\
			return CTC_E_BADID;\
		}\
    }

#define SYS_WLAN_LOGIC_PORT_CHECK(LOGIC_PORT)       \
{                                                       \
    if ( LOGIC_PORT > 0x3FFF ) \
    {\
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid logic port\n");\
        return CTC_E_INVALID_PORT;\
    }\
}

#define SYS_WLAN_TUNNEL_ID_CHECK(TUNNEL_ID)         \
{                                                       \
    if ( TUNNEL_ID > 0x3FF ) {                          \
        return CTC_E_INVALID_PARAM; }                   \
}

#define SYS_WLAN_CRYPT_ID_CHECK(ID)                  \
{                                                       \
    if (p_crypt_param->type == 0 && ID > 0x7F )                 \
    {                                                   \
        return CTC_E_INVALID_PARAM;                     \
    }                                                   \
    if (p_crypt_param->type != 0 && ID > 0x3F )                 \
    {                                                   \
        return CTC_E_INVALID_PARAM;                     \
    }                                                   \
}

#define SYS_WLAN_CRYPT_SEQ_CHECK(crypt)                  \
{                                                       \
    if(crypt->type == CTC_WLAN_CRYPT_TYPE_ENCRYPT  && crypt->key.valid && crypt->key.seq_num < 1)\
    {\
         return CTC_E_INVALID_PARAM;\
    }\
}

#define SYS_WLAN_DTLS_VERSION_CHECK(version)            \
{                                                       \
    if(version != 0xFEFF && version != 0xFEFD)          \
    {                                                   \
        return CTC_E_INVALID_PARAM;                     \
    }                                                   \
}

#define SYS_WLAN_CRYPT_TYPE_CHECK(TYPE)                  \
{                                                       \
    if ( TYPE > 1 ) {                                  \
        return CTC_E_INVALID_PARAM; }                   \
}
#define SYS_WLAN_PROPERTY_CHECK(EN)                 \
{                                                       \
    if (EN > 1) {                                       \
        return CTC_E_INVALID_PARAM; }                   \
}

#define SYS_WLAN_CREAT_LOCK(lchip)                   \
    do                                            \
    {                                             \
        sal_mutex_create(&p_usw_wlan_master[lchip]->mutex); \
        if (NULL == p_usw_wlan_master[lchip]->mutex)  \
        { \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); \
        } \
    } while (0)

#define SYS_WLAN_LOCK(lchip) \
    if(p_usw_wlan_master[lchip]->mutex) sal_mutex_lock(p_usw_wlan_master[lchip]->mutex)

#define SYS_WLAN_UNLOCK(lchip) \
    if(p_usw_wlan_master[lchip]->mutex) sal_mutex_unlock(p_usw_wlan_master[lchip]->mutex)

#define CTC_ERROR_RETURN_WLAN_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_wlan_master[lchip]->mutex); \
            return rv; \
        } \
    } while (0)

#define SYS_WLAN_REVERT_KEY(dest, src, len)      \
    {                                            \
        int32 i = 0;                             \
        for (i=0;i<(len);i++)                    \
        {                                        \
            (dest)[(len)-1-i] = (src)[i];          \
        }                                        \
    }

#define SYS_USW_WLAN_SET_HW_KEY(dest, src, len) SYS_WLAN_REVERT_KEY(dest, src, len)

extern int32
sys_usw_wlan_wb_sync(uint8 lchip);

extern int32
sys_usw_wlan_wb_restore(uint8 lchip);

STATIC int32
_sys_usw_default_client_init(uint8 lchip);

STATIC int32
_sys_usw_wlan_tunnel_build_network_key(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_tunnel_t* p_tunnel_param,uint8 is_add)
{
   ctc_field_key_t field_key;
   sys_scl_lkup_key_t lkup_key;
   int32 ret = 0;

   sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
   SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

   lkup_key.key_type = p_scl_entry->key_type;
   lkup_key.action_type = p_scl_entry->action_type;
   lkup_key.group_priority = 0;
   lkup_key.resolve_conflict = p_scl_entry->resolve_conflict;

    if(lkup_key.resolve_conflict)
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid;
    }
    else
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6))
    {
        field_key.type = CTC_FIELD_KEY_IPV6_DA;
        field_key.ext_data = p_tunnel_param->ipda.ipv6;

        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        field_key.type = CTC_FIELD_KEY_IPV6_SA;
        field_key.ext_data = p_tunnel_param->ipsa.ipv6;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

    }
    else
    {
     	field_key.type = CTC_FIELD_KEY_IP_DA;
        field_key.data = p_tunnel_param->ipda.ipv4;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
        field_key.type = CTC_FIELD_KEY_IP_SA;
        field_key.data = p_tunnel_param->ipsa.ipv4;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }
    field_key.type = CTC_FIELD_KEY_L4_DST_PORT;
    field_key.data = p_tunnel_param->l4_port ;
    ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
    CTC_ERROR_RETURN(ret);

    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }

    if (!is_add)
    {
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        field_key.data = 1;
        ret = sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
        p_scl_entry->entry_id = lkup_key.entry_id;
    }
	return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_tunnel_build_bssid_key(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_tunnel_t* p_tunnel_param,uint8 is_add)
{
    ctc_field_key_t field_key;
    sys_scl_lkup_key_t lkup_key;
    int32 ret = 0;


    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = p_scl_entry->key_type;
    lkup_key.action_type = p_scl_entry->action_type;
    lkup_key.group_priority = 1;  /*DsTunnel1*/
    lkup_key.resolve_conflict = p_scl_entry->resolve_conflict;

    if(lkup_key.resolve_conflict)
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid;
    }
    else
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid;
    }

    field_key.type = CTC_FIELD_KEY_WLAN_RADIO_MAC;
    field_key.ext_data = p_tunnel_param->bssid;
    ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
    CTC_ERROR_RETURN(ret);
    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }

    if (!is_add)
    {
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        field_key.data = 1;
        ret = sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
        p_scl_entry->entry_id = lkup_key.entry_id;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_tunnel_build_bssid_radio_key(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_tunnel_t* p_tunnel_param,uint8 is_add)
{

    ctc_field_key_t field_key;
    sys_scl_lkup_key_t lkup_key;
    int32 ret = 0;

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = p_scl_entry->key_type;
    lkup_key.action_type = p_scl_entry->action_type;
    lkup_key.group_priority = 1;  /*DsTunnel1*/
    lkup_key.resolve_conflict = p_scl_entry->resolve_conflict;

    if(lkup_key.resolve_conflict)
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid;
    }
    else
    {
        lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid;
    }

    field_key.type = CTC_FIELD_KEY_WLAN_RADIO_MAC;
    field_key.ext_data = p_tunnel_param->bssid;
    ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
    CTC_ERROR_RETURN(ret);


    field_key.type = CTC_FIELD_KEY_WLAN_RADIO_ID;
    field_key.data  = p_tunnel_param->radio_id;
    ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
    CTC_ERROR_RETURN(ret);

    if (!CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                    sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }

    if (!is_add)
    {
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        field_key.data = 1;
        ret = sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
        p_scl_entry->entry_id = lkup_key.entry_id;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_tunnel_build_network_action(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_tunnel_t* p_tunnel_param, uint32 tunnel_id, uint8 is_update)
{
    uint8 loop = 0;
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_wlan_decap_t wlan_decap;
    ctc_scl_qos_map_t   qos_map;
    ctc_scl_logic_port_t  scl_logic_port;
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tunnel_id:[%u]\n",tunnel_id);

    sal_memset(&qos_map, 0, sizeof(ctc_scl_qos_map_t));
    sal_memset(&wlan_decap, 0, sizeof(wlan_decap));
    /* build scl action */

    wlan_decap.wlan_tunnel_id = tunnel_id;
    wlan_decap.ds_fwd_ptr = p_usw_wlan_master[lchip]->decrypt_fwd_offset;
    wlan_decap.is_actoac_tunnel = (CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_tunnel_param->type);

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action));
    }
    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_DECRYPT_ENABLE))
    {
        wlan_decap.wlan_cipher_en = 1;
        wlan_decap.decrypt_key_index = (p_tunnel_param->decrypt_id)<<1;
    }

    if (CTC_WLAN_TUNNEL_MODE_NETWORK == p_tunnel_param->mode)
    {
        wlan_decap.is_tunnel = 1;
        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_REASSEMBLE_ENABLE))
        {
            wlan_decap.wlan_reassemble_en = 1;
        }

        if (p_tunnel_param->default_vlan_id > 0)
        {
            wlan_decap.user_default_vlan_tag_valid = 0;
            wlan_decap.user_default_vlan_valid = 1;
            wlan_decap.user_default_vid = p_tunnel_param->default_vlan_id;
        }

        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_ACL_EN) && p_tunnel_param->acl_lkup_num > 0)
        {
            for (loop = 0; loop < p_tunnel_param->acl_lkup_num; loop++)
            {
                SYS_ACL_PROPERTY_CHECK((&p_tunnel_param->acl_prop[loop]));
            }
            sal_memcpy(wlan_decap.acl_prop,p_tunnel_param->acl_prop,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
            wlan_decap.acl_lkup_num = p_tunnel_param->acl_lkup_num;
        }

        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_SPLIT_MAC))
        {
          wlan_decap.payload_pktType = 6;
        }
        wlan_decap.ttl_check_en = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_TTL_CHECK) ? 1 : 0;
        if(p_tunnel_param->logic_port > 0)
        {
            scl_logic_port.logic_port = p_tunnel_param->logic_port;
            scl_logic_port.logic_port_type = 0;
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
            field_action.ext_data = (void*)&scl_logic_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
        }

        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_SERVICE_ACL_EN))
        {
           field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
           field_action.data0 = 1;
           CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
        }

        if (p_tunnel_param->policer_id > 0)
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
            field_action.data0 = p_tunnel_param->policer_id;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
        }

        if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_QOS_MAP))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
            field_action.ext_data= &p_tunnel_param->qos_map;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
        }
    }

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_DECAP;
    field_action.ext_data= &wlan_decap;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
    return CTC_E_NONE;

Error0:

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, p_scl_entry->entry_id, &field_action);
    }
    return ret;
}

STATIC int32
_sys_usw_wlan_tunnel_build_bssid_action(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_tunnel_t* p_tunnel_param, uint32 tunnel_id, uint8 is_update)
{
    uint8 loop = 0;
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_wlan_decap_t wlan_decap;
     ctc_scl_logic_port_t scl_logic_port;
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&wlan_decap, 0, sizeof(sys_scl_wlan_decap_t));

    /* build scl action */
    wlan_decap.is_tunnel = 1;
    wlan_decap.wlan_tunnel_id = tunnel_id;
    wlan_decap.ds_fwd_ptr = p_usw_wlan_master[lchip]->decrypt_fwd_offset;

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        field_action.ext_data = (void*)&scl_logic_port;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action));
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_REASSEMBLE_ENABLE))
    {
        wlan_decap.wlan_reassemble_en = 1;
    }

    if (p_tunnel_param->default_vlan_id > 0)
    {
        wlan_decap.user_default_vlan_tag_valid = 0;
        wlan_decap.user_default_vlan_valid = 1;
        wlan_decap.user_default_vid = p_tunnel_param->default_vlan_id;
    }
    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_ACL_EN) && p_tunnel_param->acl_lkup_num > 0)
    {
        for (loop = 0; loop < p_tunnel_param->acl_lkup_num; loop++)
        {
            SYS_ACL_PROPERTY_CHECK((&p_tunnel_param->acl_prop[loop]));
        }
        sal_memcpy(wlan_decap.acl_prop,p_tunnel_param->acl_prop,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
        wlan_decap.acl_lkup_num = p_tunnel_param->acl_lkup_num;
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_SPLIT_MAC))
    {
      wlan_decap.payload_pktType = 6;
    }
    wlan_decap.ttl_check_en = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_TTL_CHECK) ? 1 : 0;

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_DECAP;
    field_action.ext_data= &wlan_decap;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);

    if (p_tunnel_param->logic_port > 0)
    {
       field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
	   scl_logic_port.logic_port = p_tunnel_param->logic_port;
	   scl_logic_port.logic_port_type = 0;
       field_action.ext_data= &scl_logic_port;
       CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_SERVICE_ACL_EN))
    {
     /* wlan_decap.service_acl_qos_en = 1;*/
       field_action.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;

       field_action.data0 = 1;
       CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if(CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_QOS_MAP))
    {
       field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
       field_action.ext_data= &p_tunnel_param->qos_map;
       CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (p_tunnel_param->policer_id > 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_tunnel_param->policer_id;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip,p_scl_entry->entry_id, &field_action), ret, Error0);
    }
    return CTC_E_NONE;

Error0:

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, p_scl_entry->entry_id, &field_action);
    }
    return ret;

}

STATIC int32
_sys_usw_wlan_lookup_tunnel_entry(uint8 lchip, void* p_param, sys_wlan_tunnel_sw_entry_t** pp_tunnel_info)
{
    sys_wlan_tunnel_sw_entry_t tunnel_info;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = NULL;
    ctc_wlan_tunnel_t* p_tunnel_param = NULL;

    sal_memset(&tunnel_info, 0, sizeof(sys_wlan_tunnel_sw_entry_t));

    p_tunnel_param = (ctc_wlan_tunnel_t*)p_param;
    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6))
    {
        tunnel_info.is_ipv6 = 1;
        sal_memcpy(&tunnel_info.ipsa.ipv6, &p_tunnel_param->ipsa.ipv6, sizeof(ipv6_addr_t));
        sal_memcpy(&tunnel_info.ipda.ipv6, &p_tunnel_param->ipda.ipv6, sizeof(ipv6_addr_t));
        tunnel_info.l4_port = p_tunnel_param->l4_port;
    }
    else
    {
        tunnel_info.is_ipv6 = 0;
        tunnel_info.ipsa.ipv4 = p_tunnel_param->ipsa.ipv4;
        tunnel_info.ipda.ipv4 = p_tunnel_param->ipda.ipv4;
        tunnel_info.l4_port = p_tunnel_param->l4_port;
    }

    p_tunnel_info = ctc_hash_lookup(p_usw_wlan_master[lchip]->tunnel_entry, &tunnel_info);

    *pp_tunnel_info = p_tunnel_info;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_alloc_tunnel_entry(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param, sys_wlan_tunnel_sw_entry_t** pp_tunnel_info)
{
    sys_usw_opf_t opf;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = NULL;
    uint32 tunnel_id = 0;
    int32 ret = CTC_E_NONE;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_index = 0;
    opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &tunnel_id));

    p_tunnel_info = mem_malloc(MEM_WLAN_MODULE, sizeof(sys_wlan_tunnel_sw_entry_t));
    if (!p_tunnel_info)
    {
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_tunnel_info, 0, sizeof(sys_wlan_tunnel_sw_entry_t));
    p_tunnel_info->tunnel_id = tunnel_id;
    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6))
    {
        p_tunnel_info->is_ipv6 = 1;
        sal_memcpy(&p_tunnel_info->ipsa.ipv6, &p_tunnel_param->ipsa.ipv6, sizeof(ipv6_addr_t));
        sal_memcpy(&p_tunnel_info->ipda.ipv6, &p_tunnel_param->ipda.ipv6, sizeof(ipv6_addr_t));
        p_tunnel_info->l4_port = p_tunnel_param->l4_port;
    }
    else
    {
        p_tunnel_info->is_ipv6 = 0;
        p_tunnel_info->ipsa.ipv4 = p_tunnel_param->ipsa.ipv4;
        p_tunnel_info->ipda.ipv4 = p_tunnel_param->ipda.ipv4;
        p_tunnel_info->l4_port = p_tunnel_param->l4_port;
    }
    if(NULL == ctc_hash_insert(p_usw_wlan_master[lchip]->tunnel_entry, p_tunnel_info))
    {
        ret = CTC_E_NO_MEMORY;
        mem_free(p_tunnel_info);
        goto END;
    }

    *pp_tunnel_info = p_tunnel_info;
    return CTC_E_NONE;
END:
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;

    sys_usw_opf_free_offset(lchip, &opf, 1, tunnel_id);
    *pp_tunnel_info = NULL;

    return ret;
}

STATIC int32
_sys_usw_wlan_free_tunnel_entry(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param, sys_wlan_tunnel_sw_entry_t** pp_tunnel_info)
{
    sys_usw_opf_t opf;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = *pp_tunnel_info;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if(NULL == ctc_hash_remove(p_usw_wlan_master[lchip]->tunnel_entry, p_tunnel_info))
    {
        return CTC_E_NOT_EXIST;
    }
    opf.pool_index = 0;
    opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;

    sys_usw_opf_free_offset(lchip, &opf, 1, p_tunnel_info->tunnel_id);
    mem_free(p_tunnel_info);
    *pp_tunnel_info = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_wlan_tunnel_input_parameters_check(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    int32 ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(p_tunnel_param);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->mode,CTC_WLAN_TUNNEL_MODE_BSSID_RADIO);
    CTC_WLAN_VLAN_ID_CHECK(p_tunnel_param->default_vlan_id);
    SYS_WLAN_LOGIC_PORT_CHECK(p_tunnel_param->logic_port);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->radio_id, 0x1F);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->acl_lkup_num,CTC_MAX_ACL_LKUP_NUM);
    CTC_MAX_VALUE_CHECK(p_tunnel_param->decrypt_id,0x3F);
    if((p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_BSSID_RADIO) && (p_tunnel_param->radio_id == 0))
    {
        return CTC_E_INVALID_PARAM;
    }
    if((p_tunnel_param->mode != CTC_WLAN_TUNNEL_MODE_NETWORK)
        && ((p_tunnel_param->bssid[0]== 0)&& (p_tunnel_param->bssid[1]== 0)&&(p_tunnel_param->bssid[2]== 0)
            &&(p_tunnel_param->bssid[3]== 0)&&(p_tunnel_param->bssid[4]== 0)&&(p_tunnel_param->bssid[5]== 0)))
    {
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}
int32
sys_usw_wlan_add_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    ctc_scl_group_info_t scl_group;
    ctc_scl_entry_t scl_entry;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = NULL;
    uint32 gid = 0;
    uint32 eid1 = 0,eid2 = 0,allocked = 0;
    uint32 tunnel_id  = 0;
    int32 ret = CTC_E_NONE;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip %u type %u l4_port 0x%x \n", lchip, p_tunnel_param->type, p_tunnel_param->l4_port);
    if(CTC_E_NONE != _sys_usw_wlan_tunnel_input_parameters_check(lchip,p_tunnel_param))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    SYS_WLAN_LOCK(lchip);
    CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_wlan_lookup_tunnel_entry(lchip, p_tunnel_param, &p_tunnel_info));
    SYS_WLAN_UNLOCK(lchip);
    if (NULL != p_tunnel_info && (CTC_WLAN_TUNNEL_MODE_NETWORK == p_tunnel_param->mode))
    {
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
        return CTC_E_EXIST;

    }
    /*init default client*/
    if(0 == p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid)
    {
        p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid= SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_LOCAL, 0, 0, 0);
        p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_REMOTE, 0, 0, 0);
        scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
        scl_group.priority = 0;
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid,&scl_group,1);
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid,&scl_group,1);
        CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_default_client_init(lchip));
    }

    scl_entry.entry_id = 0;
    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;
    scl_entry.mode = 1;
    scl_entry.priority_valid = 0;
    scl_entry.resolve_conflict = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT);

    if(!p_tunnel_info)
    {
        SYS_WLAN_LOCK(lchip);
        CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_wlan_alloc_tunnel_entry(lchip, p_tunnel_param, &p_tunnel_info));
        allocked = 1;
        tunnel_id = p_tunnel_info->tunnel_id;
        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
        {
            if(0 == p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid)
            {
                p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_TUNNEL, 0, 0, 0);
                scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
                scl_group.priority = 0;
                sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid,&scl_group,1);
            }
            gid = p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid;
        }
        else
        {
            if(0 == p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid)
            {
                p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH0_TUNNEL, 0, 0, 0);
                scl_group.type = CTC_SCL_GROUP_TYPE_HASH;
                scl_group.priority = 0;
                sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid,&scl_group,1);
            }
            gid = p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid;
        }
        SYS_WLAN_UNLOCK(lchip);
        scl_entry.key_type = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6)?SYS_SCL_KEY_HASH_WLAN_IPV6:SYS_SCL_KEY_HASH_WLAN_IPV4;
        /* write tunnel entry */
        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1), ret, END);
        eid1 = scl_entry.entry_id;
        /* build scl key */
        CTC_ERROR_GOTO(_sys_usw_wlan_tunnel_build_network_key(lchip, &scl_entry, p_tunnel_param,1), ret, END);

        /* build scl action */
        CTC_ERROR_GOTO(_sys_usw_wlan_tunnel_build_network_action(lchip, &scl_entry, p_tunnel_param, tunnel_id, 0), ret, END);

        /*install entry*/
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, END);

    }

    if(CTC_WLAN_TUNNEL_MODE_NETWORK != p_tunnel_param->mode)
    {
        tunnel_id = p_tunnel_info->tunnel_id;
        if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
        {
            if(0 == p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid)
            {
                p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM1_TUNNEL, 0, 0, 0);
                scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
                scl_group.priority = 1;
                sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid,&scl_group,1);
            }
            gid = p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid;
        }
        else
        {
            if (!DRV_IS_DUET2(lchip))
            {
                if (0 == p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid)
                {
                    p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH1_TUNNEL, 0, 0, 0);
                    scl_group.type = CTC_SCL_GROUP_TYPE_HASH;
                    scl_group.priority = 1;
                    sys_usw_scl_create_group(lchip, p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid, &scl_group, 1);
                }
                gid = p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid;
            }
            else
            {
                gid = p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid;
            }
        }

        if (CTC_WLAN_TUNNEL_MODE_BSSID == p_tunnel_param->mode)
        {
            scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC;
            /* write tunnel entry */
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1), ret, END);
            eid2 = scl_entry.entry_id;
            /* build scl key */
            CTC_ERROR_GOTO(_sys_usw_wlan_tunnel_build_bssid_key(lchip, &scl_entry, p_tunnel_param,1), ret, END);
        }
        else if (CTC_WLAN_TUNNEL_MODE_BSSID_RADIO == p_tunnel_param->mode)
        {
            scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC_RID;
            /* write tunnel entry */
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1), ret, END);
            eid2 = scl_entry.entry_id;
            /* build scl key */
            CTC_ERROR_GOTO(_sys_usw_wlan_tunnel_build_bssid_radio_key(lchip, &scl_entry, p_tunnel_param,1), ret, END);
        }

        /* build scl action */
        CTC_ERROR_GOTO(_sys_usw_wlan_tunnel_build_bssid_action(lchip, &scl_entry, p_tunnel_param, tunnel_id, 0), ret, END);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, END);

    }

    switch(p_tunnel_param->mode)
    {
    case CTC_WLAN_TUNNEL_MODE_NETWORK:
        p_tunnel_info->network_count++;
        break;
    case CTC_WLAN_TUNNEL_MODE_BSSID:
        p_tunnel_info->bssid_count++;
        break;
    case CTC_WLAN_TUNNEL_MODE_BSSID_RADIO:
        p_tunnel_info->bssid_rid_count++;
        break;
    default:
        break;
    }
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add wlan tunnel, group_id = %x entry_id = %x\n", gid, scl_entry.entry_id);

    return CTC_E_NONE;
END:
    if(allocked)
    {
        _sys_usw_wlan_free_tunnel_entry(lchip,  p_tunnel_param, &p_tunnel_info);
    }
    if(eid1)
    {
    	 sys_usw_scl_uninstall_entry(lchip, eid1, 1);
         sys_usw_scl_remove_entry(lchip, eid1, 1);
    }
    if(eid2)
    {
       sys_usw_scl_uninstall_entry(lchip, eid2, 1);
       sys_usw_scl_remove_entry(lchip, eid2, 1);
    }
    return ret;
}

int32
sys_usw_wlan_remove_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    ctc_scl_entry_t scl_entry;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = NULL;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if(CTC_E_NONE != _sys_usw_wlan_tunnel_input_parameters_check(lchip,p_tunnel_param))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip %u type %u l4_port 0x%x \n", lchip, p_tunnel_param->type, p_tunnel_param->l4_port);

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    SYS_WLAN_LOCK(lchip);
    CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_wlan_lookup_tunnel_entry(lchip, p_tunnel_param, &p_tunnel_info));
    if ((NULL == p_tunnel_info)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_NETWORK && p_tunnel_info->network_count == 0)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_BSSID && p_tunnel_info->bssid_count == 0)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_BSSID_RADIO && p_tunnel_info->bssid_rid_count == 0))
    {
        SYS_WLAN_UNLOCK(lchip);
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

    }

    SYS_WLAN_UNLOCK(lchip);

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
    {
        scl_entry.resolve_conflict = 1;
    }

    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;
    if (CTC_WLAN_TUNNEL_MODE_BSSID == p_tunnel_param->mode)
    {
        /* build scl key */
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC;
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_key(lchip, &scl_entry, p_tunnel_param,0));

        CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1));
        CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1));
        p_tunnel_info->bssid_count--;
    }
    else if (CTC_WLAN_TUNNEL_MODE_BSSID_RADIO == p_tunnel_param->mode)
    {
        /* build scl key */
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC_RID;
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_radio_key(lchip, &scl_entry, p_tunnel_param,0));

        CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1));
        CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1));
        p_tunnel_info->bssid_rid_count--;
    }
    else
    {
        scl_entry.key_type = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6)?SYS_SCL_KEY_HASH_WLAN_IPV6:SYS_SCL_KEY_HASH_WLAN_IPV4;
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_network_key(lchip, &scl_entry, p_tunnel_param,0));
        p_tunnel_info->network_count--;
    }

    if (0 == p_tunnel_info->network_count && 0 == p_tunnel_info->bssid_count && 0 == p_tunnel_info->bssid_rid_count)
    {
        /* build scl key */
        scl_entry.key_type = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6)?SYS_SCL_KEY_HASH_WLAN_IPV6:SYS_SCL_KEY_HASH_WLAN_IPV4;
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_network_key(lchip, &scl_entry, p_tunnel_param,0));
        CTC_ERROR_RETURN(sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1));
        CTC_ERROR_RETURN(sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1));

        CTC_ERROR_RETURN(_sys_usw_wlan_free_tunnel_entry(lchip,  p_tunnel_param, &p_tunnel_info));
    }

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove wlan tunnel, entry_id = %x\n",scl_entry.entry_id);

    return CTC_E_NONE;
}

int32
sys_usw_wlan_update_tunnel(uint8 lchip, ctc_wlan_tunnel_t* p_tunnel_param)
{
    ctc_scl_entry_t scl_entry;
    sys_wlan_tunnel_sw_entry_t* p_tunnel_info = NULL;
    uint32 tunnel_id  = 0;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip %u type %u l4_port 0x%x \n", lchip, p_tunnel_param->type, p_tunnel_param->l4_port);

    if(CTC_E_NONE != _sys_usw_wlan_tunnel_input_parameters_check(lchip,p_tunnel_param))
    {
        return CTC_E_INVALID_PARAM;
    }
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    SYS_WLAN_LOCK(lchip);
    CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_wlan_lookup_tunnel_entry(lchip, p_tunnel_param, &p_tunnel_info));
    if ((NULL == p_tunnel_info)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_NETWORK && p_tunnel_info->network_count == 0)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_BSSID && p_tunnel_info->bssid_count == 0)
        ||(p_tunnel_param->mode == CTC_WLAN_TUNNEL_MODE_BSSID_RADIO && p_tunnel_info->bssid_rid_count == 0))
    {
        SYS_WLAN_UNLOCK(lchip);
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        return CTC_E_NOT_EXIST;

    }
    tunnel_id = p_tunnel_info->tunnel_id;

    SYS_WLAN_UNLOCK(lchip);

    if (CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
    {
        scl_entry.resolve_conflict = 1;
    }

    scl_entry.action_type = SYS_SCL_ACTION_TUNNEL;

    scl_entry.key_type = CTC_FLAG_ISSET(p_tunnel_param->flag, CTC_WLAN_TUNNEL_FLAG_IPV6)?SYS_SCL_KEY_HASH_WLAN_IPV6:SYS_SCL_KEY_HASH_WLAN_IPV4;
    /* build scl key */
    CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_network_key(lchip, &scl_entry, p_tunnel_param, 0));

    /* build scl action */
    CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_network_action(lchip, &scl_entry, p_tunnel_param, tunnel_id, 1));

    CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));

    if (CTC_WLAN_TUNNEL_MODE_BSSID == p_tunnel_param->mode)
    {
        /* build scl key */
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC;
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_key(lchip, &scl_entry, p_tunnel_param,0));

        /* build scl action */
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_action(lchip, &scl_entry, p_tunnel_param, tunnel_id, 1));
        /*if acl vlan changed?*/
    }
    else if (CTC_WLAN_TUNNEL_MODE_BSSID_RADIO == p_tunnel_param->mode)
    {
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_RMAC_RID;
        /* build scl key */
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_radio_key(lchip, &scl_entry, p_tunnel_param,0));

        /* build scl action */
        CTC_ERROR_RETURN(_sys_usw_wlan_tunnel_build_bssid_action(lchip, &scl_entry, p_tunnel_param, tunnel_id, 1));
        /*if acl vlan changed?*/

    }
    CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_client_build_sta_status_key(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_client_t* p_client_param,uint8 is_add)
{
    ctc_field_key_t field_key;
    sys_scl_lkup_key_t lkup_key;
    int32 ret = 0;

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = p_scl_entry->key_type;
    lkup_key.action_type = p_scl_entry->action_type;
    lkup_key.group_priority = 0;  /*userid1*/
    lkup_key.resolve_conflict = p_scl_entry->resolve_conflict;
    if(lkup_key.resolve_conflict)
    {
        if(p_client_param->is_local)
        {
            lkup_key.group_id= p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid;
        }
        else
        {
            lkup_key.group_id = p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid;
        }

    }
    else
    {
        if(p_client_param->is_local)
        {
            lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash0_client_local_gid;
        }
        else
        {
            lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid;
        }
    }
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY))
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_client_param->sta;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
        field_key.type = CTC_FIELD_KEY_SVLAN_ID;
        field_key.data = p_client_param->src_vlan_id;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }
    else if (((CTC_WLAN_TOPO_POP_POA == p_usw_wlan_master[lchip]->roam_type) && !(p_client_param->is_local == 0 && CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type))
        || ((CTC_WLAN_TOPO_POP_POA < p_usw_wlan_master[lchip]->roam_type) && (CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type)))
    {

        field_key.type = CTC_FIELD_KEY_MAC_SA;
        field_key.ext_data = p_client_param->sta;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }
    else if((CTC_WLAN_TOPO_POP_POA == p_usw_wlan_master[lchip]->roam_type) && (p_client_param->is_local == 0 && CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type))
    {
        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_client_param->sta;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }

    if (!CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
         ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }

    if (!is_add)
    {
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  lkup_key.group_id:  0x%x \n",lkup_key.group_id);
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        field_key.data = 1;
        ret = sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
        p_scl_entry->entry_id = lkup_key.entry_id;
    }

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key type = %d\n",  p_scl_entry->key_type);

    return CTC_E_NONE;
}

#if 0
STATIC int32
_sys_usw_wlan_client_build_sta_forward_key(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_client_t* p_client_param,uint8 is_add)
{
    ctc_field_key_t field_key;
	sys_scl_lkup_key_t lkup_key;
    int32 ret = 0;

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&lkup_key,0,sizeof(sys_scl_lkup_key_t));
    lkup_key.key_type = p_scl_entry->key_type;
    lkup_key.action_type = p_scl_entry->action_type;
    lkup_key.group_priority = 1;  /*userid2*/
    lkup_key.resolve_conflict = p_scl_entry->resolve_conflict;
    if(lkup_key.resolve_conflict)
    {
        if(p_client_param->is_local)
        {
            lkup_key.group_id= p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid;
        }
        else
        {
            lkup_key.group_id = p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid;
        }

    }
    else
    {
        if(DRV_IS_DUET2(lchip))
        {
            if (p_client_param->is_local)
            {
                lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash0_client_local_gid;
            }
            else
            {
                lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid;
            }
        }
        else
        {
            if (p_client_param->is_local)
            {
                lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash1_client_local_gid;
            }
            else
            {
                lkup_key.group_id = p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid;
            }
        }
    }
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if (CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type)
    {
        if (p_client_param->src_vlan_id)
        {
            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
            field_key.data = p_client_param->src_vlan_id;
            ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                            sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
             CTC_ERROR_RETURN(ret);
        }
        else
        {
           return CTC_E_INVALID_PARAM;
        }
    }
    else if (CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type)
    {

        field_key.type = CTC_FIELD_KEY_MAC_DA;
        field_key.ext_data = p_client_param->sta;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);
    }
    if (!CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT))
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        ret = is_add ? sys_usw_scl_add_key_field(lchip, p_scl_entry->entry_id, &field_key):\
                        sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
	    CTC_ERROR_RETURN(ret);
    }

    if (!is_add)
    {
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "  lkup_key.group_id:  0x%x \n",lkup_key.group_id);
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        field_key.data = 1;
        ret = sys_usw_scl_construct_lkup_key(lchip,&field_key,&lkup_key);
        CTC_ERROR_RETURN(ret);

        CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
        p_scl_entry->entry_id = lkup_key.entry_id;
    }

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key type = %d\n",  p_scl_entry->key_type);

    return CTC_E_NONE;
}
#endif


STATIC int32
_sys_usw_wlan_client_build_sta_status_action(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_client_t* p_client_param, uint8 is_update)
{
    uint8 loop = 0;
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    sys_scl_wlan_client_t wlan_client;
    ctc_scl_logic_port_t scl_logic_port;

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        field_action.ext_data = (void*)&scl_logic_port;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action));
    }
    /* build scl action */
    p_scl_entry->action.type = CTC_SCL_ACTION_INGRESS;


    wlan_client.not_local_sta = !p_client_param->is_local;

    wlan_client.sta_roam_status = p_client_param->roam_status;

#if 0
    if (CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type && CTC_WLAN_TOPO_POP_POA < p_usw_wlan_master[lchip]->roam_type)
    {
        if (p_client_param->src_vlan_id)
        {
            wlan_client.forward_on_status = 1;
        }
    }
#endif

    wlan_client.vrfid = p_client_param->vrfid;
    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_ACL_EN) && p_client_param->acl_lkup_num > 0)
    {
        for (loop = 0; loop < p_client_param->acl_lkup_num; loop++)
        {
            SYS_ACL_PROPERTY_CHECK((&p_client_param->acl[loop]));
        }
       sal_memcpy(wlan_client.acl_prop,p_client_param->acl,sizeof(ctc_acl_property_t)* CTC_MAX_ACL_LKUP_NUM);
       wlan_client.acl_lkup_num = p_client_param->acl_lkup_num;
    }
    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_MAPPING_VLAN))
    {
        ctc_scl_vlan_edit_t vlan_edit;
        sal_memset(&vlan_edit, 0, sizeof(ctc_scl_vlan_edit_t));

        vlan_edit.vlan_domain= CTC_SCL_VLAN_DOMAIN_SVLAN;
        vlan_edit.svid_new = p_client_param->vlan_id;
        vlan_edit.svid_sl  = CTC_VLAN_TAG_SL_NEW;
        vlan_edit.stag_op  = CTC_VLAN_TAG_OP_REP_OR_ADD;
        vlan_edit.scos_sl  = CTC_VLAN_TAG_SL_ALTERNATIVE;
        wlan_client.is_vlan_edit = 1;
        sal_memcpy(&wlan_client.vlan_edit ,&vlan_edit,sizeof(ctc_scl_vlan_edit_t));
    }
    if(p_client_param->is_local)
    {
        wlan_client.sta_status_chk_en = 1;
    }
    field_action.ext_data = &wlan_client;
    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);

    if(CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_QOS_MAP))
    {
       field_action.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
       field_action.ext_data= &p_client_param->qos_map;
       CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_STATS_EN))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_client_param->stats_id;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (0 != p_client_param->policer_id)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_client_param->policer_id;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (p_client_param->cid > 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DST_CID;
        field_action.data0 = p_client_param->cid;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if(CTC_WLAN_TOPO_POP_POA == p_usw_wlan_master[lchip]->roam_type && p_client_param->is_local == 0)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
        field_action.data0 = p_client_param->nh_id;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    if (p_client_param->is_local && p_client_param->logic_port)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
        scl_logic_port.logic_port = p_client_param->logic_port;
        scl_logic_port.logic_port_type = 0;
        field_action.ext_data = (void*)&scl_logic_port;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);
    }

    return CTC_E_NONE;


Error0:

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, p_scl_entry->entry_id, &field_action);
    }
    return ret;

}

#if 0
STATIC int32
_sys_usw_wlan_client_build_sta_forward_action(uint8 lchip, ctc_scl_entry_t* p_scl_entry, ctc_wlan_client_t* p_client_param, uint8 is_update)
{
    int32 ret = 0;
    ctc_scl_field_action_t field_action;
    ctc_scl_logic_port_t  scl_logic_port;

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        field_action.ext_data = (void*)&scl_logic_port;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action));
    }
    /* build scl action */
    p_scl_entry->action.type = SYS_SCL_ACTION_INGRESS;

    /*nexthop*/
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    field_action.data0 = p_client_param->nh_id;
    CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, p_scl_entry->entry_id, &field_action), ret, Error0);

    return CTC_E_NONE;

Error0:

    if(is_update)
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        sys_usw_scl_remove_action_field(lchip, p_scl_entry->entry_id, &field_action);
    }
    return ret;

}
#endif

int32
_sys_usw_wlan_client_input_parameters_check(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    int32 ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(p_client_param);
    CTC_WLAN_VLAN_ID_CHECK(p_client_param->vlan_id);
    SYS_USW_CID_CHECK(lchip,p_client_param->cid);
    CTC_MAX_VALUE_CHECK(p_client_param->acl_lkup_num,CTC_MAX_ACL_LKUP_NUM);
    CTC_MAX_VALUE_CHECK(p_client_param->roam_status, 2);

    return ret;
}

int32
sys_usw_wlan_add_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    ctc_scl_group_info_t scl_group;
    ctc_scl_entry_t scl_entry;
    uint32 gid = 0;
    uint32 eid1 = 0,eid2 = 0;
    int32 ret = CTC_E_NONE;
#if 0
    uint8 need_fwd = 0;
#endif

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip: %u mac: %2x%.2x%.2x%.2x%.2x%.2x is_local: %u tunnel_type 0x%x \n",
                        lchip, p_client_param->sta[0],p_client_param->sta[1],p_client_param->sta[2],p_client_param->sta[3],p_client_param->sta[4],p_client_param->sta[5],p_client_param->is_local, p_client_param->tunnel_type);
    ret = _sys_usw_wlan_client_input_parameters_check(lchip ,p_client_param);
    if(ret < 0)
    {
        return ret;
    }

    if(p_client_param->is_local == 0 && p_client_param->nh_id == 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_WLAN_LOCK(lchip);

#if 0
    if (CTC_WLAN_TOPO_HUB_SPOK <= p_usw_wlan_master[lchip]->roam_type
         && ((0 == p_client_param->is_local && CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type)
              || CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type))
    {/*need userid output forward info*/
        need_fwd = 1;
    }
#endif
    /*init default client*/
    if(0 == p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid)
    {
        p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid= SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_LOCAL, 0, 0, 0);
        p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_REMOTE, 0, 0, 0);
        scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
        scl_group.priority = 0;
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid,&scl_group,1);
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid,&scl_group,1);
        CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_default_client_init(lchip));
    }

    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT))
    {
        if(p_client_param->is_local)
        {
            gid = p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid;
        }
        else
        {
            gid = p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid;
        }

    }
#if 0
    else if(need_fwd && !DRV_IS_DUET2(lchip))
    {
        if(0 == p_usw_wlan_master[lchip]->scl_hash1_client_local_gid)
        {
            p_usw_wlan_master[lchip]->scl_hash1_client_local_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH1_CLIENT_LOCAL, 0, 0, 0);
            p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH1_CLIENT_REMOTE, 0, 0, 0);
            scl_group.type = CTC_SCL_GROUP_TYPE_HASH;
            scl_group.priority = 1;
            sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_hash1_client_local_gid,&scl_group,1);
            sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid,&scl_group,1);
        }

        if(p_client_param->is_local)
        {
            gid = p_usw_wlan_master[lchip]->scl_hash1_client_local_gid;
        }
        else
        {
            gid = p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid;
        }
    }
#endif
    else
    {
        if(0 == p_usw_wlan_master[lchip]->scl_hash0_client_local_gid)
        {
            p_usw_wlan_master[lchip]->scl_hash0_client_local_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH0_CLIENT_LOCAL, 0, 0, 0);
            p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_HASH0_CLIENT_REMOTE, 0, 0, 0);
            scl_group.type = CTC_SCL_GROUP_TYPE_HASH;
            scl_group.priority = 0;
            sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_hash0_client_local_gid,&scl_group,1);
            sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid,&scl_group,1);
        }

        if(p_client_param->is_local)
        {
            gid = p_usw_wlan_master[lchip]->scl_hash0_client_local_gid;
        }
        else
        {
            gid = p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid;
        }
    }

    SYS_WLAN_UNLOCK(lchip);
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.entry_id = 0;
    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.mode = 1;
    scl_entry.priority_valid = 0;
    scl_entry.priority = 0;
    scl_entry.resolve_conflict = CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT);
    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY))
    {
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS;
    }
    else
    {
        scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_STA_STATUS;
    }
    CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1),ret, END);
    eid1 = scl_entry.entry_id;

    /* build scl key */
    CTC_ERROR_GOTO(_sys_usw_wlan_client_build_sta_status_key(lchip, &scl_entry, p_client_param,1), ret, END);

    CTC_ERROR_GOTO(_sys_usw_wlan_client_build_sta_status_action(lchip, &scl_entry, p_client_param, 0), ret, END);

    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, END);

#if 0
    if (need_fwd)
    {
        sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
        if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT))
        {
            if(0 == p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid)
            {
                p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM1_CLIENT_LOCAL, 0, 0, 0);
                p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM1_CLIENT_REMOTE, 0, 0, 0);
                scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
                scl_group.priority = 1;
                sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid,&scl_group,1);
                sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid,&scl_group,1);
            }
            if(p_client_param->is_local)
            {
                gid = p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid;
            }
            else
            {
                gid = p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid;
            }
        }

        if (CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type)
        {
            scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_MACDA_FORWARD;
        }
        else
        {
            scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_VLAN_FORWARD;
        }
        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &scl_entry, 1),ret, END);
        eid2 = scl_entry.entry_id;
        CTC_ERROR_GOTO(_sys_usw_wlan_client_build_sta_forward_key(lchip, &scl_entry, p_client_param,1), ret, END);
        CTC_ERROR_GOTO(_sys_usw_wlan_client_build_sta_forward_action(lchip, &scl_entry, p_client_param, 0), ret, END);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, END);
    }
#endif

    return CTC_E_NONE;
END:
    if(eid1)
    {
        sys_usw_scl_uninstall_entry(lchip, eid1, 1);
        sys_usw_scl_remove_entry(lchip, eid1, 1);
    }
    if(eid2)
    {
        sys_usw_scl_uninstall_entry(lchip, eid2, 1);
        sys_usw_scl_remove_entry(lchip, eid2, 1);
    }
    return ret;
}

int32
sys_usw_wlan_remove_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip: %u mac: %2x%.2x%.2x%.2x%.2x%.2x is_local: %u tunnel_type 0x%x \n",
                        lchip, p_client_param->sta[0],p_client_param->sta[1],p_client_param->sta[2],p_client_param->sta[3],p_client_param->sta[4],p_client_param->sta[5],p_client_param->is_local, p_client_param->tunnel_type);

    ret = _sys_usw_wlan_client_input_parameters_check(lchip ,p_client_param);
    if(ret < 0)
    {
        return ret;
    }
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY))
    {
       scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS;
    }
     else
    {
      scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_STA_STATUS;
    }

    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.resolve_conflict = CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT);

    CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_status_key(lchip, &scl_entry, p_client_param,0));

    sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1);
    sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);

#if 0
    if (2 == p_usw_wlan_master[lchip]->roam_type && ((0 == p_client_param->is_local && CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type)
                                                        || CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type))
    {
       sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

	   CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_forward_key(lchip, &scl_entry, p_client_param,0));
	   sys_usw_scl_uninstall_entry(lchip, scl_entry.entry_id, 1);

       sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);
    }
#endif

    return CTC_E_NONE;
}

int32
sys_usw_wlan_update_client(uint8 lchip, ctc_wlan_client_t* p_client_param)
{
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip: %u mac: %2x%.2x%.2x%.2x%.2x%.2x is_local: %u tunnel_type 0x%x \n",
                    lchip, p_client_param->sta[0],p_client_param->sta[1],p_client_param->sta[2],p_client_param->sta[3],p_client_param->sta[4],p_client_param->sta[5],p_client_param->is_local, p_client_param->tunnel_type);
    ret = _sys_usw_wlan_client_input_parameters_check(lchip ,p_client_param);
    if(ret < 0)
    {
        return ret;
    }

    if(p_client_param->is_local == 0 && p_client_param->nh_id == 0)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

    if (CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_IS_MC_ENTRY))
    {
       scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_MC_STA_STATUS;
    }
     else
    {
      scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_STA_STATUS;
    }

    scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
    scl_entry.resolve_conflict = CTC_FLAG_ISSET(p_client_param->flag, CTC_WLAN_CLIENT_FLAG_RESOLVE_HASH_CONFLICT);

    CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_status_key(lchip, &scl_entry, p_client_param,0));
    CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_status_action(lchip, &scl_entry, p_client_param, 1));
    CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));

#if 0
    if (2 == p_usw_wlan_master[lchip]->roam_type && ((0 == p_client_param->is_local && CTC_WLAN_TUNNEL_TYPE_WTP_2_AC == p_client_param->tunnel_type)
                                                        || CTC_WLAN_TUNNEL_TYPE_AC_2_AC == p_client_param->tunnel_type))
    {
       sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));

       CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_forward_key(lchip, &scl_entry, p_client_param,0));

       CTC_ERROR_RETURN(_sys_usw_wlan_client_build_sta_forward_action(lchip, &scl_entry, p_client_param, 1));
       CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));
    }
#endif

    return CTC_E_NONE;
}

int32
sys_usw_wlan_set_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint8 crypt_id = 0;
    ds_t ds_data;
    uint8 valid = 0;
    uint8 aes_key[WLAN_AES_KEY_LENGTH];
    uint8 sha_key[WLAN_SHA_KEY_LENGTH];

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_crypt_param);
    SYS_WLAN_CRYPT_ID_CHECK(p_crypt_param->crypt_id);
    SYS_WLAN_CRYPT_SEQ_CHECK(p_crypt_param);
    SYS_WLAN_CRYPT_TYPE_CHECK(p_crypt_param->type);
    CTC_MAX_VALUE_CHECK(p_crypt_param->key_id, 1);
    CTC_MAX_VALUE_CHECK(p_crypt_param->key.seq_num, 0xffffffffffffLL);

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip %u type %u crypt_id 0x%x key_id %u\n", lchip, p_crypt_param->type, p_crypt_param->crypt_id,p_crypt_param->key_id);

    sal_memset(aes_key, 0, WLAN_AES_KEY_LENGTH);
    sal_memset(sha_key, 0, WLAN_SHA_KEY_LENGTH);

    if (CTC_WLAN_CRYPT_TYPE_DECRYPT == p_crypt_param->type)
    {
        if(!DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOR(WlanEngineDecryptCtl_t, WlanEngineDecryptCtl_seqNumCheckZero_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
            if(field_value && p_crypt_param->type == CTC_WLAN_CRYPT_TYPE_DECRYPT && p_crypt_param->key.seq_num == 0)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
        crypt_id = p_crypt_param->crypt_id;
        /* write dtls epoch*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsDtlsEpochCheck_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        if(0 == p_crypt_param->key_id)
        {
            valid = GetDsDtlsEpochCheck(V, epoch0Valid_f, ds_data);
            if(valid && p_crypt_param->key.valid)
            {
                SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "epoch0 is in use(valid),can not be change!\n");
                return CTC_E_IN_USE;
            }
            cmd = DRV_IOW(DsDtlsEpochCheck_t, DsDtlsEpochCheck_epoch0Valid_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, &p_crypt_param->key.valid));

            cmd = DRV_IOW(DsDtlsEpochCheck_t, DsDtlsEpochCheck_epoch0_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, &p_crypt_param->key.epoch));

            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "epoch0: %u epoch0Valid: %u\n",p_crypt_param->key.epoch,p_crypt_param->key.valid);
            crypt_id = p_crypt_param->crypt_id<<1;
        }
        else if(1 == p_crypt_param->key_id)
        {
            valid = GetDsDtlsEpochCheck(V, epoch1Valid_f, ds_data);
            if(valid && p_crypt_param->key.valid)
            {
                SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "epoch1 is in use(valid),can not be change!\n");
                return CTC_E_IN_USE;
            }
            cmd = DRV_IOW(DsDtlsEpochCheck_t, DsDtlsEpochCheck_epoch1Valid_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, &p_crypt_param->key.valid));

            cmd = DRV_IOW(DsDtlsEpochCheck_t, DsDtlsEpochCheck_epoch1_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, &p_crypt_param->key.epoch));

            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "epoch1: %u epoch1Valid: %u\n",p_crypt_param->key.epoch,p_crypt_param->key.valid);
            crypt_id = (p_crypt_param->crypt_id<<1) + 1;
        }

        /* write dtls seq*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SetDsDtlsSeqCheck(A, lastSequenceNum_f, ds_data, &p_crypt_param->key.seq_num);
        cmd = DRV_IOW(DsDtlsSeqCheck_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));

        /* write aes key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SYS_USW_WLAN_SET_HW_KEY(aes_key, p_crypt_param->key.aes_key, WLAN_AES_KEY_LENGTH);
        SetDsCapwapAesKey(A, aesKey_f, ds_data, aes_key);
        cmd = DRV_IOW(DsCapwapAesKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0x80|crypt_id, cmd, ds_data));

        /* write sha key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SYS_USW_WLAN_SET_HW_KEY(sha_key, p_crypt_param->key.sha_key, WLAN_SHA_KEY_LENGTH);
        SetDsCapwapShaKey(A, shaKey_f, ds_data, sha_key);
        cmd = DRV_IOW(DsCapwapShaKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0x80|crypt_id, cmd, ds_data));
    }
    else if (CTC_WLAN_CRYPT_TYPE_ENCRYPT == p_crypt_param->type)
    {
        crypt_id = p_crypt_param->crypt_id;
        /* write egress dtls*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SetDsEgressDtls(V, epoch_f, ds_data, p_crypt_param->key.epoch);
        SetDsEgressDtls(A, sequenceNum_f, ds_data, &p_crypt_param->key.seq_num);
        cmd = DRV_IOW(DsEgressDtls_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));

        crypt_id = p_crypt_param->crypt_id;
        /* write aes key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SYS_USW_WLAN_SET_HW_KEY(aes_key, p_crypt_param->key.aes_key, WLAN_AES_KEY_LENGTH);
        SetDsCapwapAesKey(A, aesKey_f, ds_data, aes_key);
        cmd = DRV_IOW(DsCapwapAesKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));

        /* write sha key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        SYS_USW_WLAN_SET_HW_KEY(sha_key, p_crypt_param->key.sha_key, WLAN_SHA_KEY_LENGTH);
        SetDsCapwapShaKey(A, shaKey_f, ds_data, sha_key);
        cmd = DRV_IOW(DsCapwapShaKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_wlan_get_crypt(uint8 lchip, ctc_wlan_crypt_t* p_crypt_param)
{
    uint32 cmd = 0;
    uint8 crypt_id = 0;
    ds_t ds_data;
    uint8 aes_key[WLAN_AES_KEY_LENGTH];
    uint8 sha_key[WLAN_SHA_KEY_LENGTH];

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_crypt_param);
    SYS_WLAN_CRYPT_ID_CHECK(p_crypt_param->crypt_id);
    CTC_MAX_VALUE_CHECK(p_crypt_param->key_id, 1);

    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "lchip %u type %u crypt_id 0x%x \n", lchip, p_crypt_param->type, p_crypt_param->crypt_id);

    sal_memset(aes_key, 0, WLAN_AES_KEY_LENGTH);
    sal_memset(sha_key, 0, WLAN_SHA_KEY_LENGTH);

    if (CTC_WLAN_CRYPT_TYPE_DECRYPT == p_crypt_param->type)
    {
        crypt_id = p_crypt_param->crypt_id;
        /* read dtls epoch*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsDtlsEpochCheck_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        if(0 == p_crypt_param->key_id)
        {
            p_crypt_param->key.valid = GetDsDtlsEpochCheck(V, epoch0Valid_f, ds_data);
            p_crypt_param->key.epoch = GetDsDtlsEpochCheck(V, epoch0_f, ds_data);
            crypt_id = p_crypt_param->crypt_id<<1;
        }
        else
        {
            p_crypt_param->key.valid = GetDsDtlsEpochCheck(V, epoch1Valid_f, ds_data);
            p_crypt_param->key.epoch = GetDsDtlsEpochCheck(V, epoch1_f, ds_data);
            crypt_id = (p_crypt_param->crypt_id<<1) + 1;
        }

        /* read dtls seq*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsDtlsSeqCheck_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        GetDsDtlsSeqCheck(A, lastSequenceNum_f, ds_data, &p_crypt_param->key.seq_num);

        /* read aes key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsCapwapAesKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0x80|crypt_id, cmd, ds_data));
        GetDsCapwapAesKey(A, aesKey_f, ds_data, aes_key);
        SYS_USW_WLAN_SET_HW_KEY(p_crypt_param->key.aes_key, aes_key, WLAN_AES_KEY_LENGTH);

        /* read sha key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsCapwapShaKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0x80|crypt_id, cmd, ds_data));
        GetDsCapwapShaKey(A, shaKey_f, ds_data, sha_key);
        SYS_USW_WLAN_SET_HW_KEY(p_crypt_param->key.sha_key, sha_key, WLAN_SHA_KEY_LENGTH);

    }
    else if (CTC_WLAN_CRYPT_TYPE_ENCRYPT == p_crypt_param->type)
    {
        crypt_id = p_crypt_param->crypt_id;
        /* read egress dtls*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsEgressDtls_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        p_crypt_param->key.epoch = GetDsEgressDtls(V, epoch_f, ds_data);
        GetDsEgressDtls(A, sequenceNum_f, ds_data, &p_crypt_param->key.seq_num);

        /* read aes key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsCapwapAesKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        GetDsCapwapAesKey(A, aesKey_f, ds_data, aes_key);
        SYS_USW_WLAN_SET_HW_KEY(p_crypt_param->key.aes_key, aes_key, WLAN_AES_KEY_LENGTH);

        /* read sha key*/
        sal_memset(ds_data, 0, sizeof(ds_data));
        cmd = DRV_IOR(DsCapwapShaKey_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, crypt_id, cmd, ds_data));
        GetDsCapwapShaKey(A, shaKey_f, ds_data, sha_key);
        SYS_USW_WLAN_SET_HW_KEY(p_crypt_param->key.sha_key, sha_key, WLAN_SHA_KEY_LENGTH);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_wlan_set_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    ds_t ds_data;
    ctc_scl_field_action_t field_action;
    ctc_scl_vlan_edit_t  vlan_edit;
    sys_scl_wlan_client_t wlan_client;
    ctc_scl_group_info_t scl_group;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_glb_param);
    SYS_WLAN_PROPERTY_CHECK(p_glb_param->control_pkt_decrypt_en);
    SYS_WLAN_PROPERTY_CHECK(p_glb_param->fc_swap_enable);
    SYS_WLAN_DTLS_VERSION_CHECK(p_glb_param->dtls_version);

    sal_memset(&vlan_edit, 0, sizeof(vlan_edit));
    sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));

    wlan_client.not_local_sta = 1;
	wlan_client.sta_status_chk_en = 1;

    vlan_edit.vlan_domain= CTC_SCL_VLAN_DOMAIN_SVLAN;
    vlan_edit.svid_new = p_glb_param->default_client_vlan_id;
    vlan_edit.svid_sl  = CTC_VLAN_TAG_SL_NEW;
    vlan_edit.stag_op  = CTC_VLAN_TAG_OP_REP_OR_ADD;
    vlan_edit.scos_sl  = CTC_VLAN_TAG_SL_ALTERNATIVE;
    wlan_client.is_vlan_edit = 1;
    sal_memcpy(&wlan_client.vlan_edit ,&vlan_edit,sizeof(ctc_scl_vlan_edit_t));

    if ((p_glb_param->default_client_action >= CTC_WLAN_DEFAULT_CLIENT_ACTION_MAX)
        ||((p_glb_param->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN)
        && (p_glb_param->default_client_vlan_id < CTC_MIN_VLAN_ID || p_glb_param->default_client_vlan_id > CTC_MAX_VLAN_ID)))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* write ipee2iloop decrypt port enable*/
    field_val = p_glb_param->control_pkt_decrypt_en;
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapDecryptPortControlEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* write IpeLookupCtl decrypt  enable*/
    field_val = p_glb_param->control_pkt_decrypt_en;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_capwapControlPacketDecryptEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* write IpeUserIdCtl roam */
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    SetIpeUserIdCtl(V, capwapControlPacketDecryptEn_f, ds_data, p_glb_param->control_pkt_decrypt_en);
    cmd = DRV_IOW(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds_data));

    /* global config for dtls*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);
    SetEpePktProcCtl(V, dtlsProtocolVersion_f, ds_data, p_glb_param->dtls_version);
    SetEpePktProcCtl(V, fcSwapEn_f, ds_data , p_glb_param->fc_swap_enable); /*I flag = 1, P flag = 1, O flag = 0*/
    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds_data));

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    SetParserLayer4AppCtl(V, capwapControlUdpPort1_f, ds_data, p_glb_param->udp_dest_port1);
    SetParserLayer4AppCtl(V, capwapControlUdpPort0_f, ds_data, p_glb_param->udp_dest_port0);
    SetParserLayer4AppCtl(V, innerFcSwapEn_f, ds_data, p_glb_param->fc_swap_enable);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds_data));

    cmd = DRV_IOW(ParserEthernetCtl_t, ParserEthernetCtl_fcSwapEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_glb_param->fc_swap_enable));

    SYS_WLAN_LOCK(lchip);
    /*init default client*/
    if(0 == p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid)
    {
        p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid= SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_LOCAL, 0, 0, 0);
        p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_WLAN, SYS_WLAN_GROUP_SUB_TYPE_TCAM0_CLIENT_REMOTE, 0, 0, 0);
        scl_group.type = CTC_SCL_GROUP_TYPE_NONE;
        scl_group.priority = 0;
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid,&scl_group,1);
        sys_usw_scl_create_group(lchip,p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid,&scl_group,1);
        CTC_ERROR_RETURN_WLAN_UNLOCK(_sys_usw_default_client_init(lchip));
    }

    if ((p_glb_param->default_client_action != p_usw_wlan_master[lchip]->default_client_action)
        || (p_glb_param->default_client_vlan_id != p_usw_wlan_master[lchip]->default_client_vlan_id))
    {
        if ((p_usw_wlan_master[lchip]->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_DISCARD)
            && (p_glb_param->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN))
        {
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_remove_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));

            field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
            field_action.ext_data = &wlan_client;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_add_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));
        }
        else if ((p_usw_wlan_master[lchip]->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN)
            && (p_glb_param->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_DISCARD))
        {
            field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_remove_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));

            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_add_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));
        }
        else if ((p_usw_wlan_master[lchip]->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN)
            && (p_glb_param->default_client_action == CTC_WLAN_DEFAULT_CLIENT_ACTION_MAPPING_VLAN))
        {
            field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_remove_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));

            field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
            field_action.ext_data = &wlan_client;
            CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_add_action_field(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, &field_action));
        }
        CTC_ERROR_RETURN_WLAN_UNLOCK(sys_usw_scl_install_entry(lchip, p_usw_wlan_master[lchip]->default_clinet_entry_id, 1));
        p_usw_wlan_master[lchip]->default_client_action = p_glb_param->default_client_action;
        p_usw_wlan_master[lchip]->default_client_vlan_id = p_glb_param->default_client_vlan_id;
    }
    SYS_WLAN_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_wlan_get_global_cfg(uint8 lchip, ctc_wlan_global_cfg_t* p_glb_param)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    ds_t ds_data;

    SYS_WLAN_INIT_CHECK(lchip);
    SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_glb_param);

    /* write ipe e2 iloop*/
    cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapDecryptPortControlEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    p_glb_param->control_pkt_decrypt_en = field_val;

    /* write ipe userid ctl*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    p_glb_param->control_pkt_decrypt_en = GetIpeUserIdCtl(V, capwapControlPacketDecryptEn_f, ds_data);

    /* global config for UDP dest port and flag*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &ds_data);
    p_glb_param->dtls_version = GetEpePktProcCtl(V, dtlsProtocolVersion_f, ds_data);
    p_glb_param->fc_swap_enable = GetEpePktProcCtl(V, fcSwapEn_f, ds_data); /*I flag = 1, P flag = 1, O flag = 0*/

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    p_glb_param->udp_dest_port1 = GetParserLayer4AppCtl(V, capwapControlUdpPort1_f, ds_data);
    p_glb_param->udp_dest_port0 = GetParserLayer4AppCtl(V, capwapControlUdpPort0_f, ds_data);
    p_glb_param->fc_swap_enable = GetParserLayer4AppCtl(V, innerFcSwapEn_f, ds_data);

    p_glb_param->default_client_action = p_usw_wlan_master[lchip]->default_client_action;
    p_glb_param->default_client_vlan_id = p_usw_wlan_master[lchip]->default_client_vlan_id;

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_wlan_tunnel_entry_make_group(sys_wlan_tunnel_sw_entry_t* p_tunnel)
{
    uint8* k = NULL;

    if (p_tunnel->is_ipv6)
    {
        k = (uint8*)&(p_tunnel->ipsa.ipv6);
    }
    else
    {
        k = (uint8*)&(p_tunnel->ipsa.ipv4);
    }

    return ctc_hash_caculate(SYS_WLAN_TUNNEL_SW_ENTRY_INFO_LEN, k);
}

STATIC bool
_sys_usw_wlan_tunnel_entry_compare_group(sys_wlan_tunnel_sw_entry_t* stored_node, sys_wlan_tunnel_sw_entry_t* lkup_node)
{
    if (!stored_node || !lkup_node)
    {
        return TRUE;
    }

    if(lkup_node->is_ipv6)
    {
        if (!sal_memcmp((uint8*)&stored_node->ipsa.ipv6, (uint8*)&lkup_node->ipsa.ipv6, SYS_WLAN_TUNNEL_SW_ENTRY_INFO_LEN)
            && (stored_node->is_ipv6 == lkup_node->is_ipv6))
        {
            return TRUE;
        }
    }
    else
    {
        if (!sal_memcmp((uint8*)&stored_node->ipsa.ipv4, (uint8*)&lkup_node->ipsa.ipv4, SYS_WLAN_TUNNEL_SW_ENTRY_INFO_LEN)
            && (stored_node->is_ipv6 == lkup_node->is_ipv6))
        {
            return TRUE;
        }
    }

    return FALSE;
}

STATIC int32
_sys_usw_default_client_init(uint8 lchip)
{
    ctc_scl_entry_t scl_entry;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    sys_scl_wlan_client_t wlan_client;
    mac_addr_t mac;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&mac, 0, sizeof(mac_addr_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&wlan_client, 0, sizeof(sys_scl_wlan_client_t));

    scl_entry.priority_valid = 1;
    scl_entry.priority = FPA_PRIORITY_LOWEST;
    scl_entry.mode = 1;
    scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_STA_STATUS;
    scl_entry.resolve_conflict = 1;
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;


    CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid, &scl_entry, 1));
    field_key.type = CTC_FIELD_KEY_MAC_SA;
    field_key.ext_data = mac;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action));

    wlan_client.not_local_sta = 1;
	wlan_client.sta_status_chk_en = 1;
    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
    field_action.ext_data = &wlan_client;
    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action));

    CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));
    p_usw_wlan_master[lchip]->default_clinet_entry_id = scl_entry.entry_id;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.priority_valid = 1;
    scl_entry.priority = FPA_PRIORITY_LOWEST;
    scl_entry.mode = 1;
    scl_entry.key_type = SYS_SCL_KEY_HASH_WLAN_STA_STATUS;
    scl_entry.resolve_conflict = 1;
    scl_entry.action.type = CTC_SCL_ACTION_INGRESS;

    CTC_ERROR_RETURN(sys_usw_scl_add_entry(lchip, p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid, &scl_entry, 1));
    field_key.type = CTC_FIELD_KEY_MAC_DA;
    field_key.ext_data = mac;
    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_WLAN_CLIENT;
    wlan_client.not_local_sta = 1;
	wlan_client.sta_status_chk_en = 1;

    CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action));

    CTC_ERROR_RETURN(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_internal_port_encap_wlan_tunnel_init(uint8 lchip)
{
    uint8 gchip = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint16 interface_id = 0;
    uint32 field_val = 0;
    DsSrcPort_m src_port;
    DsPhyPortExt_m phy_port_ext;
    DsDestPort_m dest_port;

    sys_usw_get_gchip_id(lchip, &gchip);
    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_ENCAP, &lport));
    interface_id = MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID);

    sal_memset(&src_port, 0, sizeof(src_port));
    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));
    SetDsSrcPort(V, interfaceId_f, &src_port, interface_id);
    SetDsSrcPort(V, routedPort_f, &src_port, 1);
    SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 1);
    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));

    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
    SetDsPhyPortExt(V, defaultVlanId_f, &phy_port_ext, 0);
    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_E2ILOOP, &lport));
    sal_memset(&dest_port, 0, sizeof(DsDestPort_m));
    cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dest_port));
    SetDsDestPort(V, gAcl_0_aclLookupType_f, &dest_port, 1);
    SetDsDestPort(V, gAcl_0_aclUseGlobalPortType_f, &dest_port, 2);
    SetDsDestPort(V, gAcl_1_aclLookupType_f, &dest_port, 1);
    SetDsDestPort(V, gAcl_1_aclUseGlobalPortType_f, &dest_port, 2);
    SetDsDestPort(V, gAcl_2_aclLookupType_f, &dest_port, 1);
    SetDsDestPort(V, gAcl_2_aclUseGlobalPortType_f, &dest_port, 2);
    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dest_port));
    field_val = 2;
    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_wlan_rsv_nh_init(uint8 lchip)
{
    EpeNextHopCtl_m epe_nh_ctl;
    uint32 cmd = 0;

    sal_memset(&epe_nh_ctl, 0, sizeof(epe_nh_ctl));

    CTC_ERROR_RETURN(sys_usw_nh_wlan_create_reserve_nh(lchip, SYS_NH_ENTRY_TYPE_FWD,
        &p_usw_wlan_master[lchip]->decrypt_fwd_offset));

    CTC_ERROR_RETURN(sys_usw_nh_wlan_create_reserve_nh(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W,
        &p_usw_wlan_master[lchip]->l2_trans_edit_offset));

    cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapDecryptChannelFwdPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_usw_wlan_master[lchip]->decrypt_fwd_offset));

    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_capwapDecapNextHopPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_usw_wlan_master[lchip]->decap_nh_offset));

    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));
    SetEpeNextHopCtl(V, capwapL2TransEditPtr_f, &epe_nh_ctl, p_usw_wlan_master[lchip]->l2_trans_edit_offset);
    SetEpeNextHopCtl(V, capwapL2TransEditLocation_f, &epe_nh_ctl, 0);
    SetEpeNextHopCtl(V, capwapL2TransEditPtrType_f, &epe_nh_ctl, 0);
    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_nh_ctl));

    return CTC_E_NONE;
}

int32
sys_usw_wlan_init(uint8 lchip, ctc_wlan_global_cfg_t* p_global_t)
{
    uint32 cmd = 0;
    uint32 wlan_enable;
    uint32 field_val = 0;
    uint32 index = 0;
    ds_t ds_data;
    uint16 lport = 0;
    sys_usw_opf_t opf;
    int32 ret;
    uint32 capwapMaxFragNum = 16;
    sys_scl_default_action_t default_tunnel_action;
    ctc_scl_field_action_t field_action;
    IpeFwdCtl_m ipe_fwd_ctl;
    IpeAclQosCtl_m ipe_acl_qos_ctl;
    uint8  work_status = 0;
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, 0, SYS_DMPS_PORT_PROP_WLAN_ENABLE, (void *)&wlan_enable));
    if(0 == wlan_enable)
    {
        return CTC_E_NONE;
    }

    if(NULL != p_usw_wlan_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }

    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_global_t);
    SYS_WLAN_DTLS_VERSION_CHECK(p_global_t->dtls_version);

    sal_memset(&default_tunnel_action, 0, sizeof(default_tunnel_action));
    sal_memset(&ds_data, 0, sizeof(ds_t));
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));

    if (NULL == p_usw_wlan_master[lchip])
    {
        /* init master */
        p_usw_wlan_master[lchip] = mem_malloc(MEM_WLAN_MODULE, sizeof(sys_wlan_master_t));
        if (!p_usw_wlan_master[lchip])
        {
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

        }

        sal_memset(p_usw_wlan_master[lchip], 0, sizeof(sys_wlan_master_t));

        /*init opf for inner tunnel id */
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id, 1, "opf-type-wlan-tunnel-id"));

        opf.pool_index = 0;
        opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, MCHIP_CAP(SYS_CAP_WLAN_PER_TUNNEL_NUM)));
        p_usw_wlan_master[lchip]->tunnel_entry = ctc_hash_create(1,
                                        SYS_WLAN_TUNNEL_ENTRY_BLOCK_SIZE,
                                        (hash_key_fn) _sys_usw_wlan_tunnel_entry_make_group,
                                        (hash_cmp_fn) _sys_usw_wlan_tunnel_entry_compare_group);
    }
    /* config for wlan decapsulate key type*/
    /* write ipe chan*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeFwdChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    SetIpeFwdChanIdCfg(V, cfgCapwapDecrypt0ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetIpeFwdChanIdCfg(V, cfgCapwapDecrypt1ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1));
    SetIpeFwdChanIdCfg(V, cfgCapwapDecrypt2ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2));
    SetIpeFwdChanIdCfg(V, cfgCapwapDecrypt3ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3));
    SetIpeFwdChanIdCfg(V, cfgCapwapEncrypt0ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetIpeFwdChanIdCfg(V, cfgCapwapEncrypt1ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1));
    SetIpeFwdChanIdCfg(V, cfgCapwapEncrypt2ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2));
    SetIpeFwdChanIdCfg(V, cfgCapwapEncrypt3ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3));
    SetIpeFwdChanIdCfg(V, cfgCapwapReassembleChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    cmd = DRV_IOW(IpeFwdChanIdCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    /* write epe chan*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpeHdrEditChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, ds_data);
    SetEpeHdrEditChanIdCfg(V, cfgCapwapDecrypt0ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapDecrypt1ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT1));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapDecrypt2ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT2));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapDecrypt3ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT3));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapEncrypt0ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapEncrypt1ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT1));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapEncrypt2ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT2));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapEncrypt3ChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT3));
    SetEpeHdrEditChanIdCfg(V, cfgCapwapReassembleChanId_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    cmd = DRV_IOW(EpeHdrEditChanIdCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);


    /* write queue wlan chan*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(QWriteCapwapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetQWriteCapwapCtl(V, capwapEncryptChannel_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_ENCRYPT0));
    SetQWriteCapwapCtl(V, capwapDecryptChannel_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_DECRYPT0));
    SetQWriteCapwapCtl(V, capwapReassembleChannel_f, ds_data, MCHIP_CAP(SYS_CAP_CHANID_WLAN_REASSEMBLE));
    SetQWriteCapwapCtl(V, capwapDecryptChannelSelectByHash_f, ds_data, 1);
    SetQWriteCapwapCtl(V, capwapEncryptChannelSelectByHash_f, ds_data, 1);
    cmd = DRV_IOW(QWriteCapwapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapDecryptPortControlEn_f);
    field_val = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapDecapPortControlEn_f);
    field_val = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapEncapPortControlEn_f);
    field_val = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_capwapEncapedLoopbackPort_f);
    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_ENCAP, &lport));
    field_val = lport;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);

    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_capwapControlPacketDecryptEn_f);
    field_val = p_global_t->control_pkt_decrypt_en;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);

    /* write ipe userid ctl*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetIpeUserIdCtl(V, capwapControlPacketDecryptEn_f, ds_data, p_global_t->control_pkt_decrypt_en);
    SetIpeUserIdCtl(V, roamingDisable_f, ds_data, FALSE);
    SetIpeUserIdCtl(V, capwapRoamingType_f, ds_data, 0);
    SetIpeUserIdCtl(V, radioMacMaskOff_f, ds_data, 0);
    SetIpeUserIdCtl(V, capwapBssidBindingType_f, ds_data, USERIDHASHTYPE_MACPORT);
    cmd = DRV_IOW(IpeUserIdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    /* global config for UDP dest port and flag*/
    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetEpePktProcCtl(V, dtlsProtocolVersion_f, ds_data, p_global_t->dtls_version);
    SetEpePktProcCtl(V, dtlsContentType_f, ds_data, 0x17);
    SetEpePktProcCtl(V, capwapWbid1_f, ds_data, 1);
    SetEpePktProcCtl(V, capwapWbid0_f, ds_data , 0);
    SetEpePktProcCtl(V, capwapMcastEn_f, ds_data , 1);
    SetEpePktProcCtl(V, fcSwapEn_f, ds_data, p_global_t->fc_swap_enable);
    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetParserEthernetCtl(V, fcSwapEn_f, ds_data, p_global_t->fc_swap_enable);
    cmd = DRV_IOW(ParserEthernetCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret,  WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR( ParserPacketTypeMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetParserPacketTypeMap(V, dot11PacketTypeEn_f, ds_data, 1);
    cmd = DRV_IOW(ParserPacketTypeMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret,  WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetParserLayer4AppCtl(V, capwapControlPacketEn_f, ds_data, 1);
    SetParserLayer4AppCtl(V, capwapDataPacketEn_f, ds_data, 1);
    SetParserLayer4AppCtl(V, capwapControlUdpPort1_f, ds_data, p_global_t->udp_dest_port1);
    SetParserLayer4AppCtl(V, capwapControlUdpPort0_f, ds_data, p_global_t->udp_dest_port0);
    SetParserLayer4AppCtl(V, innerFcSwapEn_f, ds_data , p_global_t->fc_swap_enable);
    SetParserLayer4AppCtl(V, capwapSourcePortCheckEn_f, ds_data , 1);
    SetParserLayer4AppCtl(V, capwapDestPortCheckEn_f, ds_data, 1);
    SetParserLayer4AppCtl(V, unknownDot11Ctl_f, ds_data, 0);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret,  WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetEpeL2EditCtl(V, dot11WdsEn_f, ds_data, TRUE);
    cmd = DRV_IOW(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret,  WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(EpeL2TpidCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetEpeL2TpidCtl(V, capwapInnerTpidIndex_f, ds_data, 0);
    cmd = DRV_IOW(EpeL2TpidCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(WlanEngineDecryptCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetWlanEngineDecryptCtl(V, dtlsSeqNumCheckEn_f, ds_data, 1);
    SetWlanEngineDecryptCtl(V, dtlsEpochCheckEn_f, ds_data, 1);
    SetWlanEngineDecryptCtl(V, seqNumCheckZero_f, ds_data, 0);
    cmd = DRV_IOW(WlanEngineDecryptCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);



    sal_memset(&ipe_fwd_ctl, 0, sizeof(ipe_fwd_ctl));
    cmd = DRV_IOR(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl), ret, WLAN_INIT_ERROR);
    SetIpeFwdCtl(V, capwapTunnel_0_forwardStatus_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, capwapTunnel_1_forwardStatus_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, capwapTunnel_2_forwardStatus_f, &ipe_fwd_ctl, 1);
    SetIpeFwdCtl(V, capwapTunnel_3_forwardStatus_f, &ipe_fwd_ctl, 0);
    SetIpeFwdCtl(V, capwapDecapNextHopPtrDisable_f, &ipe_fwd_ctl, 1);
    cmd = DRV_IOW(IpeFwdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_fwd_ctl), ret, WLAN_INIT_ERROR);

    sal_memset(&ipe_acl_qos_ctl, 0, sizeof(ipe_acl_qos_ctl));
    cmd = DRV_IOR(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl), ret, WLAN_INIT_ERROR);
    SetIpeAclQosCtl(V, capwapTunnel_0_forwardStatus_f, &ipe_acl_qos_ctl, 0);
    SetIpeAclQosCtl(V, capwapTunnel_1_forwardStatus_f, &ipe_acl_qos_ctl, 0);
    SetIpeAclQosCtl(V, capwapTunnel_2_forwardStatus_f, &ipe_acl_qos_ctl, 1);
    SetIpeAclQosCtl(V, capwapTunnel_3_forwardStatus_f, &ipe_acl_qos_ctl, 0);
    cmd = DRV_IOW(IpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_acl_qos_ctl), ret, WLAN_INIT_ERROR);

    sal_memset(ds_data, 0, sizeof(ds_data));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);
    SetBufferStoreCtl(V, e2iErrorDiscardCtl_f, ds_data, 0x7FFF);
    /*except errorCode:
      E2ILOOPERROR_DOT1AE_DECRYPT_CIPHER_MODE_MISMATCH:0xa -1
      E2ILOOPERROR_DOT1AE_DECRYPT_ICV_CHECK_ERROR:0x9 -1    */
    SetBufferStoreCtl(V, e2iErrorExceptionCtl_f, ds_data, 0x7CFF);
    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, ds_data), ret, WLAN_INIT_ERROR);

    field_val = 1;
    cmd = DRV_IOW(WlanEngineEncryptCtl_t, WlanEngineEncryptCtl_dtlsEncryptCompatibleMode_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);
    cmd = DRV_IOW(WlanEngineDecryptCtl_t, WlanEngineDecryptCtl_dtlsCompatibleMode_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_val), ret, WLAN_INIT_ERROR);

    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_capwapMaxFragNum_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &capwapMaxFragNum));

    field_val = 1;
    cmd = DRV_IOW(ParserLayer3ProtocolCtl_t, ParserLayer3ProtocolCtl_udpLiteTypeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 73;
    cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_capwapControlPacketMinLength_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 73;
    cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_dtlsControlPacketMinLength_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 73;
    cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_dtlsControlPacketMinLength0_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 73;
    cmd = DRV_IOW(ParserLayer4AppCtl_t, ParserLayer4AppCtl_dtlsControlPacketMinLength1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*IpeTunnelDecapCtl*/
    field_val = 1;
    cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapCipherStatusMismatchDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_lookup2FragTunnelIdDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 0;
    cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_reassembleAlwaysEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*cfg wlan channel pkt len*/
    for(index = 0; index<9; index++)
    {
        if (DRV_IS_DUET2(lchip))
        {
            /*min pkt len*/
            field_val = CTC_FRAME_SIZE_7;
            cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLenSelId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

            /*mxa pkt len*/
            field_val = CTC_FRAME_SIZE_7;
            cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLenSelId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        }
        else
        {
            field_val = SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE;
            cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLen_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

            field_val = SYS_USW_MAX_FRAMESIZE_MAX_VALUE;
            cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLen_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        }
    }

    /* write check dtls seqNum 0 enable*/
    field_val = 0;
    cmd = DRV_IOW(WlanEngineDecryptCtl_t, WlanEngineDecryptCtl_seqNumCheckZero_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*set default tunnel*/
    default_tunnel_action.action_type = SYS_SCL_ACTION_TUNNEL;
    default_tunnel_action.hash_key_type = SYS_SCL_KEY_HASH_WLAN_IPV4;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    default_tunnel_action.field_action = &field_action;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_RPF_CHECK;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    default_tunnel_action.hash_key_type = SYS_SCL_KEY_HASH_WLAN_IPV6;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 1;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    field_action.type = SYS_SCL_FIELD_ACTION_TYPE_RPF_CHECK;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
    field_action.data0 = 0;
    CTC_ERROR_RETURN(sys_usw_scl_set_default_action(lchip,  &default_tunnel_action));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (work_status != CTC_FTM_MEM_CHANGE_RECOVER))
    {
        cmd = DRV_IOR(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_capwapDecryptChannelFwdPtr_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &p_usw_wlan_master[lchip]->decrypt_fwd_offset), ret, WLAN_INIT_ERROR);

        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_capwapDecapNextHopPtr_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &p_usw_wlan_master[lchip]->decap_nh_offset), ret, WLAN_INIT_ERROR);

        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_capwapL2TransEditPtr_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &p_usw_wlan_master[lchip]->l2_trans_edit_offset), ret, WLAN_INIT_ERROR);

        CTC_ERROR_GOTO(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD, 1,
            p_usw_wlan_master[lchip]->decrypt_fwd_offset), ret, WLAN_INIT_ERROR);
        CTC_ERROR_GOTO(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W, 1,
            p_usw_wlan_master[lchip]->l2_trans_edit_offset),ret, WLAN_INIT_ERROR);
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_wlan_rsv_nh_init(lchip), ret, WLAN_INIT_ERROR);
    }

    CTC_ERROR_GOTO(_sys_usw_internal_port_encap_wlan_tunnel_init(lchip), ret, WLAN_INIT_ERROR);
    SYS_WLAN_CREAT_LOCK(lchip);

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_WLAN, sys_usw_wlan_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_wlan_wb_restore(lchip));
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

WLAN_INIT_ERROR:
    if (p_usw_wlan_master[lchip])
    {
        mem_free(p_usw_wlan_master[lchip]);
        p_usw_wlan_master[lchip] = NULL;
    }
    return ret;
}

STATIC int32
_sys_usw_wlan_free_tunnel_entry_linklist_node(sys_wlan_tunnel_sw_entry_t* p_tunnel_info, void *user_data)
{
    sys_usw_opf_t opf;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    uint32 lchip = data->value1;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;

    sys_usw_opf_free_offset(lchip, &opf, 1, p_tunnel_info->tunnel_id);
    mem_free(p_tunnel_info);

    return 1;
}

int32
sys_usw_wlan_deinit(uint8 lchip)
{
    sys_traverse_t user_data;

    user_data.value1 = lchip;

    LCHIP_CHECK(lchip);
    if (NULL == p_usw_wlan_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(p_usw_wlan_master[lchip]->mutex);

    /*free sw tunnel entry*/
    ctc_hash_traverse_remove(p_usw_wlan_master[lchip]->tunnel_entry, (hash_traversal_fn) _sys_usw_wlan_free_tunnel_entry_linklist_node, (void *)&user_data);
    ctc_hash_free(p_usw_wlan_master[lchip]->tunnel_entry);

    /*free opf*/
    sys_usw_opf_free(lchip, p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id, 0);
    sys_usw_opf_deinit(lchip, p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id);

    /*deinit master*/
    mem_free(p_usw_wlan_master[lchip]);

    return CTC_E_NONE;
}

int32
_sys_usw_wlan_wb_mapping_sw_tunnel(sys_wb_wlan_tunnel_sw_entry_t *p_wb_wlan_tunnel_sw_entry, sys_wlan_tunnel_sw_entry_t *p_tunnel_info, uint8 sync)
{
    if (sync)
    {
        p_wb_wlan_tunnel_sw_entry->tunnel_id = p_tunnel_info->tunnel_id;
        p_wb_wlan_tunnel_sw_entry->l4_port = p_tunnel_info->l4_port;
        p_wb_wlan_tunnel_sw_entry->is_ipv6 = p_tunnel_info->is_ipv6;
        p_wb_wlan_tunnel_sw_entry->network_count = p_tunnel_info->network_count;
        p_wb_wlan_tunnel_sw_entry->bssid_count = p_tunnel_info->bssid_count;
        p_wb_wlan_tunnel_sw_entry->bssid_rid_count = p_tunnel_info->bssid_rid_count;
        sal_memcpy(&p_wb_wlan_tunnel_sw_entry->ipsa,&p_tunnel_info->ipsa, sizeof(ipv6_addr_t));
        sal_memcpy(&p_wb_wlan_tunnel_sw_entry->ipda,&p_tunnel_info->ipda, sizeof(ipv6_addr_t));
    }
    else
    {
        p_tunnel_info->tunnel_id = p_wb_wlan_tunnel_sw_entry->tunnel_id;
        p_tunnel_info->l4_port = p_wb_wlan_tunnel_sw_entry->l4_port;
        p_tunnel_info->is_ipv6 = p_wb_wlan_tunnel_sw_entry->is_ipv6;
        p_tunnel_info->network_count = p_wb_wlan_tunnel_sw_entry->network_count;
        p_tunnel_info->bssid_count = p_wb_wlan_tunnel_sw_entry->bssid_count;
        p_tunnel_info->bssid_rid_count = p_wb_wlan_tunnel_sw_entry->bssid_rid_count;
        sal_memcpy(&p_tunnel_info->ipsa,&p_wb_wlan_tunnel_sw_entry->ipsa, sizeof(ipv6_addr_t));
        sal_memcpy(&p_tunnel_info->ipda,&p_wb_wlan_tunnel_sw_entry->ipda, sizeof(ipv6_addr_t));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_wlan_wb_sync_sw_tunnel_func(sys_wlan_tunnel_sw_entry_t *p_tunnel_info, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_wlan_tunnel_sw_entry_t  *p_wb_wlan_tunnel_sw_entry;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_wlan_tunnel_sw_entry = (sys_wb_wlan_tunnel_sw_entry_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_wlan_tunnel_sw_entry, 0, sizeof(sys_wb_wlan_tunnel_sw_entry_t));
    CTC_ERROR_RETURN(_sys_usw_wlan_wb_mapping_sw_tunnel(p_wb_wlan_tunnel_sw_entry, p_tunnel_info, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_wlan_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_wlan_master_t  *p_wb_wlan_master;

    SYS_USW_FTM_CHECK_NEED_SYNC(lchip);
    /*syncup  wlan_matser*/
    wb_data.buffer = mem_malloc(MEM_WLAN_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_wlan_master_t, CTC_FEATURE_WLAN, SYS_WB_APPID_WLAN_SUBID_MASTER);

    p_wb_wlan_master = (sys_wb_wlan_master_t  *)wb_data.buffer;

    p_wb_wlan_master->version = SYS_WB_VERSION_WLAN;
    p_wb_wlan_master->default_clinet_entry_id = p_usw_wlan_master[lchip]->default_clinet_entry_id;
    p_wb_wlan_master->default_client_action = p_usw_wlan_master[lchip]->default_client_action;
    p_wb_wlan_master->default_client_vlan_id = p_usw_wlan_master[lchip]->default_client_vlan_id;
    p_wb_wlan_master->scl_hash0_tunnel_gid = p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid;
    p_wb_wlan_master->scl_hash1_tunnel_gid = p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid;
    p_wb_wlan_master->scl_tcam0_tunnel_gid = p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid;
    p_wb_wlan_master->scl_tcam1_tunnel_gid = p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid;
    p_wb_wlan_master->scl_hash0_client_local_gid = p_usw_wlan_master[lchip]->scl_hash0_client_local_gid;
    p_wb_wlan_master->scl_hash0_client_remote_gid = p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid;
    p_wb_wlan_master->scl_hash1_client_local_gid = p_usw_wlan_master[lchip]->scl_hash1_client_local_gid;
    p_wb_wlan_master->scl_hash1_client_remote_gid = p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid;
    p_wb_wlan_master->scl_tcam0_client_local_gid = p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid;
    p_wb_wlan_master->scl_tcam0_client_remote_gid = p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid;
    p_wb_wlan_master->scl_tcam1_client_local_gid =  p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid;
    p_wb_wlan_master->scl_tcam1_client_remote_gid =  p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid;


    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup  wlan tunnel sw entry*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_wlan_tunnel_sw_entry_t, CTC_FEATURE_WLAN, SYS_WB_APPID_WLAN_SUBID_TUNNEL_SW_ENTRY);
    user_data.data = &wb_data;

    wb_data.valid_cnt = 0;
    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_wlan_master[lchip]->tunnel_entry, (hash_traversal_fn) _sys_usw_wlan_wb_sync_sw_tunnel_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_wlan_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_wlan_master_t   wb_wlan_master ;
    sys_wlan_tunnel_sw_entry_t *p_tunnel_info;
    sys_wb_wlan_tunnel_sw_entry_t  wb_wlan_tunnel_sw_entry ;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_WLAN_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_wlan_master_t, CTC_FEATURE_WLAN, SYS_WB_APPID_WLAN_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query wlan master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }
    sal_memset( &wb_wlan_master, 0, sizeof(sys_wb_wlan_master_t));
    sal_memcpy( &wb_wlan_master, wb_query.buffer, wb_query.key_len + wb_query.data_len);
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_WLAN, wb_wlan_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    p_usw_wlan_master[lchip]->default_client_action = wb_wlan_master.default_client_action;
    p_usw_wlan_master[lchip]->default_client_vlan_id = wb_wlan_master.default_client_vlan_id;
    p_usw_wlan_master[lchip]->default_clinet_entry_id = wb_wlan_master.default_clinet_entry_id;
    p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid = wb_wlan_master.scl_hash0_tunnel_gid;
    p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid = wb_wlan_master.scl_hash1_tunnel_gid;
    p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid =  wb_wlan_master.scl_tcam0_tunnel_gid;
    p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid = wb_wlan_master.scl_tcam1_tunnel_gid;
    p_usw_wlan_master[lchip]->scl_hash0_client_local_gid =  wb_wlan_master.scl_hash0_client_local_gid;
    p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid =  wb_wlan_master.scl_hash0_client_remote_gid;
    p_usw_wlan_master[lchip]->scl_hash1_client_local_gid =  wb_wlan_master.scl_hash1_client_local_gid;
    p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid =  wb_wlan_master.scl_hash1_client_remote_gid;
    p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid = wb_wlan_master.scl_tcam0_client_local_gid;
    p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid = wb_wlan_master.scl_tcam0_client_remote_gid;
    p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid = wb_wlan_master.scl_tcam1_client_local_gid;
    p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid = wb_wlan_master.scl_tcam1_client_remote_gid;

    /*restore  wlan tunnel sw entry*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_wlan_tunnel_sw_entry_t, CTC_FEATURE_WLAN, SYS_WB_APPID_WLAN_SUBID_TUNNEL_SW_ENTRY);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
    sal_memcpy(&wb_wlan_tunnel_sw_entry, (sys_wb_wlan_tunnel_sw_entry_t *)wb_query.buffer + entry_cnt++, wb_query.key_len + wb_query.data_len);
    p_tunnel_info = mem_malloc(MEM_WLAN_MODULE,  sizeof(sys_wlan_tunnel_sw_entry_t));
    if (NULL == p_tunnel_info)
    {
        ret = CTC_E_NO_MEMORY;

        goto done;
    }
    sal_memset(p_tunnel_info, 0, sizeof(sys_wlan_tunnel_sw_entry_t));

    ret = _sys_usw_wlan_wb_mapping_sw_tunnel(&wb_wlan_tunnel_sw_entry, p_tunnel_info, 0);
    if (ret)
    {
        continue;
    }

    /*add to soft table*/
    if (NULL == ctc_hash_insert(p_usw_wlan_master[lchip]->tunnel_entry, p_tunnel_info))
    {
        ret = CTC_E_NO_MEMORY;
        mem_free(p_tunnel_info);
        goto done;
    }
    /*recover opf*/
    opf.pool_index = 0;
    opf.pool_type  = p_usw_wlan_master[lchip]->opf_type_wlan_tunnel_id;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, wb_wlan_tunnel_sw_entry.tunnel_id));

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

int32
_sys_usw_wlan_show_tunnels(sys_wlan_tunnel_sw_entry_t* p_tunnel_info, void* user_data)
{
    uint32 tempip = 0;
    char buf_v4[sizeof "255.255.255.255"] = "";
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    char buf_v6[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"] = "";
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    if(((data->value1 == 1) && (p_tunnel_info->network_count > 0))
    || ((data->value1 == 2) && (p_tunnel_info->bssid_count > 0))
    || ((data->value1 == 3) && (p_tunnel_info->bssid_rid_count > 0))
    || (data->value1 == 0))
    {

        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12d",p_tunnel_info->tunnel_id);
        if(p_tunnel_info->is_ipv6)
        {
            sal_memset(ipv6_address, 0, sizeof(ipv6_address));
            sal_memset(buf_v6, 0, sizeof(buf_v6));
            ipv6_address[0] = sal_htonl(p_tunnel_info->ipsa.ipv6[0]);
            ipv6_address[1] = sal_htonl(p_tunnel_info->ipsa.ipv6[1]);
            ipv6_address[2] = sal_htonl(p_tunnel_info->ipsa.ipv6[2]);
            ipv6_address[3] = sal_htonl(p_tunnel_info->ipsa.ipv6[3]);
            sal_inet_ntop(AF_INET6, ipv6_address, buf_v6, sizeof(buf_v6));
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s",buf_v6);

            sal_memset(ipv6_address, 0, sizeof(ipv6_address));
            sal_memset(buf_v6, 0, sizeof(buf_v6));
            ipv6_address[0] = sal_htonl(p_tunnel_info->ipda.ipv6[0]);
            ipv6_address[1] = sal_htonl(p_tunnel_info->ipda.ipv6[1]);
            ipv6_address[2] = sal_htonl(p_tunnel_info->ipda.ipv6[2]);
            ipv6_address[3] = sal_htonl(p_tunnel_info->ipda.ipv6[3]);
            sal_inet_ntop(AF_INET6, ipv6_address, buf_v6, sizeof(buf_v6));
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-30s",buf_v6);
        }
        else
        {
            sal_memset(buf_v4, 0, sizeof(buf_v4));
            tempip = sal_ntohl(p_tunnel_info->ipsa.ipv4);
            sal_inet_ntop(AF_INET, &tempip, buf_v4, sizeof(buf_v4));
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-19s",buf_v4);

            sal_memset(buf_v4, 0, sizeof(buf_v4));
            tempip = sal_ntohl(p_tunnel_info->ipda.ipv4);
            sal_inet_ntop(AF_INET, &tempip, buf_v4, sizeof(buf_v4));
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-19s",buf_v4);
        }

        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-13d%-15d%-15d%-15d\n",p_tunnel_info->l4_port,p_tunnel_info->network_count,p_tunnel_info->bssid_count,p_tunnel_info->bssid_rid_count);
    }
    return CTC_E_NONE;
}

extern int32
sys_usw_scl_show_entry(uint8 lchip, uint8 type, uint32 param, uint32 key_type, uint8 detail, ctc_field_key_t* p_grep, uint8 grep_cnt);
int32
sys_usw_wlan_show_status(uint8 lchip, uint8 type, uint8 param)
{
    int32 ret = CTC_E_NONE;
    sys_traverse_t user_data;

    SYS_WLAN_INIT_CHECK(lchip);

    user_data.value1 = type;

    if(param == 1)
    {
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------wlan tunnel----------------\n\r");
        SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s%-19s%-19s%-13s%-15s%-15s%-15s\n\r","tunnel-ID","ipsa","ipda","L4port","network_count", "bssid_count", "(bssid+Rid)_count");
        CTC_ERROR_RETURN(ctc_hash_traverse(p_usw_wlan_master[lchip]->tunnel_entry, (hash_traversal_fn) _sys_usw_wlan_show_tunnels, (void *)&user_data));
        if(p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid)
        {
            ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash0_tunnel_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
        }
        if(p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid)
        {
            ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash1_tunnel_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
        }
        if(p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid)
        {
            ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam0_tunnel_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
        }
        if(p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid)
        {
            ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam1_tunnel_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
        }
    }
    else
    {

        if(type)/*local*/
        {
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------local client----------------\n\r");
            if(p_usw_wlan_master[lchip]->scl_hash0_client_local_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash0_client_local_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_hash1_client_local_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash1_client_local_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam0_client_local_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam1_client_local_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
        }
        else/*remote*/
        {
            SYS_WLAN_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------remote client----------------\n\r");
            if(p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash0_client_remote_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_hash1_client_remote_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam0_client_remote_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
            if(p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid)
            {
                ret = sys_usw_scl_show_entry(lchip, 2, p_usw_wlan_master[lchip]->scl_tcam1_client_remote_gid, SYS_SCL_KEY_NUM, 0, NULL, 0);
            }
        }
    }

    return ret;
}

#endif

