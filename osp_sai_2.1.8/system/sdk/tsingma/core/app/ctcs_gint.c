
#if defined(TSINGMA)

#include "ctcs_gint_api.h"

#include "ctc_opf.h"
#include "ctc_app.h"
#include "common/include/ctc_const.h"
#include "common/include/ctc_error.h"
#include "common/include/ctc_hash.h"
#include "api/include/ctc_api.h"
#include "api/include/ctcs_api.h"

#define CTC_GINT_SID_MAX  0X3FF
#define CTC_GINT_IOOP_NHID  0xF00
#define CTC_GINT_PORT_LOGIC_PORT_NUM  (16*1024)
#define CTC_GINT_UNI_NH_UDF_PROFILE_ID 8
#define CTC_GINT_ENCODE_NH_ID(in)  (0xF00 + in)
#define CTC_GINT_DEFAULT_FID (4097) 
#define CTC_GINT_STORM_RESTRICT_TYPE_MAX 3
/*SCL*/
#define CTC_GINT_UNI_SCL_GROUP 0x10000000
#define CTC_GINT_DEFT_VLAN_SCL_GROUP 0x10000001
#define CTC_GINT_PORT_ISOLATION_SCL_GROUP 0x10000002
#define CTC_GINT_ENCODE_SCL_ENTRY_ID(logicport)  (0x10000000 + logicport) 
#define CTC_GINT_ENCODE_ISOLATION_ENTRY_ID(logicport)  (0x11000000 + logicport)
#define CTC_GINT_ENCODE_LIMIT_ENTRY_ID(logicport)  (0x12000000 + logicport) 
#define CTC_GINT_ILOOP_DEFT_VLAN_ENTRY_ID    (0x10000000)

#define CTC_GINT_SERVICE_SCL_ID    0
#define CTC_GINT_PORT_ISOLATION_SCL_ID    1
#define CTC_GINT_UNI_SCL_ID  2
#define CTC_GINT_DEFT_VLAN_SCL_ID  3

/*ACL*/
#define CTC_GINT_UNI_ACL_UDF_ENTRY_ID 1000
#define CTC_GINT_ACL_STROM_RESTRICT_PROI    0
#define CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST (0x10000000)
#define CTC_GINT_ACL_GRP_ID_STORM_MUTICAST  (0x10000001)
#define CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN   (0x10000002)
#define CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER (0x10000003)
#define CTC_GINT_ENCODE_ACL_ENTRY_ID(logicport, type)  (0x10000000 + 0x1000000*type +logicport) 

/*policer*/
#define CTC_GINT_POLICER_START 0x8000
#define CTC_GINT_POLICER_END 0xFFFE

#define CTC_GINT_INIT_CHECK(lchip) \
do { \
       if (p_gint_master[lchip] == NULL){ \
           return CTC_E_NOT_INIT;\
   } \
} while (0)

#define CTC_GINT_DBG_FUNC()          CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__)
#define CTC_GINT_DBG_INFO(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_INFO, FMT, ##__VA_ARGS__)
#define CTC_GINT_DBG_ERROR(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, FMT, ##__VA_ARGS__)
#define CTC_GINT_DBG_PARAM(FMT, ...) CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, FMT, ##__VA_ARGS__)
#define CTC_GINT_DBG_DUMP(FMT, ...)  CTC_APP_API_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, FMT, ##__VA_ARGS__)

#define CTC_GINT_PORT_VLAN_ID_CHECK(val) \
do { \
    if ((val) > CTC_MAX_VLAN_ID)\
    {\
        return CTC_E_BADID;\
    }\
}while(0)


extern int32 sys_usw_port_set_logic_port_type_en(uint8 lchip, uint32 gport, uint8 enable);
extern int32 sys_usw_global_set_gint_en(uint8 lchip, uint8 enable);

struct ctc_gint_storm_ctl_s
{
    uint32 acl_entry_id;
    uint32 policer_id;
    uint32 cir;
    uint32 cbs;
    uint8  enable;    
}; 
typedef struct ctc_gint_storm_ctl_s  ctc_gint_storm_ctl_t;

struct ctc_gint_dsl_node_s
{
    uint32 dsl_id;  /*key*/

    uint32 sid;
    uint16 gport; 

    uint8  flag; /*refer to ctc_gint_upd_action_t*/  
    uint16 igs_policer_id;   
    uint32 igs_stats_id;  
    uint16 egs_policer_id;  
    uint32 egs_stats_id; 

    ctc_gint_storm_ctl_t storm_restrict[CTC_GINT_STORM_RESTRICT_TYPE_MAX];   
    ctc_gint_mac_limit_t mac_limit;

    uint32 nh_id;
    uint32 dsl_entry_id;        
    uint32 isolation_entry_id;  
    uint32 limit_entry_id;      

    uint32 service_num;
    
    uint16 logic_port;
}; 
typedef struct ctc_gint_dsl_node_s  ctc_gint_dsl_node_t;

struct ctc_gint_dsl_node_find_s
{
    uint16 logic_port;  
    ctc_gint_dsl_node_t* p_dsl_node;
}; 
typedef struct ctc_gint_dsl_node_find_s  ctc_gint_dsl_node_find_t;


struct ctc_gint_service_node_s
{
    uint32 dsl_id;  /*key*/
    uint16 svlan;   /*key*/
    uint16 cvlan;   /*key*/

    ctc_gint_vlan_action_t igs_vlan_action;
    ctc_gint_vlan_action_t egs_vlan_action;

    uint32 nh_id;
    uint8  vlan_mapping_use_flex;
    
    uint16 logic_port;
}; 
typedef struct ctc_gint_service_node_s  ctc_gint_service_node_t;

struct ctc_gint_service_node_find_s
{
    uint16 logic_port;
    ctc_gint_service_node_t* p_service_node;
}; 
typedef struct ctc_gint_service_node_find_s  ctc_gint_service_node_find_t;

struct ctc_gint_service_mac_lim_s
{
    uint32 dsl_id;
    uint8  learn_disable;
    uint8  lchip;
}; 
typedef struct ctc_gint_service_mac_lim_s  ctc_gint_service_mac_lim_t;

struct ctc_gint_master_s
{
    void*  p_dsl_hash;
    void*  p_service_hash;

    uint32 uni_iloop_port;
    
    uint32 nni_gport;
    uint16 nni_logic_port;
    uint8  gint_deinit;
};

typedef struct ctc_gint_master_s ctc_gint_master_t;


ctc_gint_master_t* p_gint_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

STATIC uint32
_ctc_gint_dsl_hash_key(void* p_data)
{
    uint8 len = 4;

    return ctc_hash_caculate(len, p_data);
}

STATIC bool
_ctc_gint_dsl_hash_cmp(uint8* bucket_data , uint32* p_data)
{
    if (sal_memcmp(bucket_data, p_data, 4) == 0)
    {
        return TRUE;
    }

    return FALSE;
}



STATIC int32
_ctc_gint_dsl_hash_match(ctc_gint_dsl_node_t* bucket_data , ctc_gint_dsl_node_find_t* p_find_node)
{
    if (bucket_data->logic_port == p_find_node->logic_port)
    {
        p_find_node->p_dsl_node = bucket_data;
        return -1;
    }

    return 0;
}

STATIC int32
_ctc_gint_uni_gport_hash_match(ctc_gint_dsl_node_t* bucket_data , uint32* p_gport)
{
    if (bucket_data->gport == *p_gport)
    {
        return -1;
    }

    return 0;
}

STATIC int32
_ctc_gint_uni_sid_hash_match(ctc_gint_dsl_node_t* bucket_data , uint32* p_sid)
{
    if (bucket_data->sid == *p_sid)
    {
        return -1;
    }

    return 0;
}

STATIC uint32
_ctc_gint_service_hash_key(void* p_data)
{
    uint8 len = 8;

    return ctc_hash_caculate(len, p_data);
}

STATIC bool
_ctc_gint_service_hash_cmp(uint8* bucket_data , uint32* p_data)
{
    if (sal_memcmp(bucket_data, p_data, 8) == 0)
    {
        return TRUE;
    }

    return FALSE;
}


STATIC int32
_ctc_gint_service_hash_match(ctc_gint_service_node_t* bucket_data , ctc_gint_service_node_find_t* p_find_node)
{
    if (bucket_data->logic_port == p_find_node->logic_port)
    {
        p_find_node->p_service_node = bucket_data;
        return -1;
    }

    return 0;
}

STATIC int32
_ctc_gint_dsl_node_remove(ctc_gint_dsl_node_t* bucket_data , uint8* p_data)
{
    uint8 lchip = *p_data;
    ctc_gint_uni_t cfg;
    ctc_gint_storm_restrict_t  restrict_cfg;
    ctc_gint_mac_limit_t       limit_cfg;

    sal_memset(&limit_cfg, 0, sizeof(ctc_gint_mac_limit_t));
    sal_memset(&restrict_cfg, 0, sizeof(ctc_gint_storm_restrict_t));
    sal_memset(&cfg, 0, sizeof(ctc_gint_uni_t));
    if (bucket_data)
    {
        /*remove security*/
        ctcs_gint_set_port_isolation(lchip, bucket_data->dsl_id, 0);
        
        restrict_cfg.dsl_id = bucket_data->dsl_id;
        restrict_cfg.type = 0;
        restrict_cfg.enable = 0;
        ctcs_gint_set_storm_restrict(lchip, &restrict_cfg);
        restrict_cfg.type = 1;
        restrict_cfg.enable = 0;
        ctcs_gint_set_storm_restrict(lchip, &restrict_cfg);
        restrict_cfg.type = 2;
        restrict_cfg.enable = 0;
        ctcs_gint_set_storm_restrict(lchip, &restrict_cfg);

        limit_cfg.dsl_id = bucket_data->dsl_id;
        limit_cfg.enable = 0;
        ctcs_gint_set_mac_limit(lchip, &limit_cfg);
        

        cfg.dsl_id = bucket_data->dsl_id;
        ctcs_gint_destroy_uni(lchip, &cfg);
    }

    return 0;
}


STATIC int32
_ctc_gint_service_node_remove(ctc_gint_service_node_t* bucket_data , uint8* p_data)
{
    uint8 lchip = *p_data;
    ctc_gint_service_t cfg;

    sal_memset(&cfg, 0, sizeof(ctc_gint_service_t));
    if (bucket_data)
    {
        cfg.dsl_id = bucket_data->dsl_id;
        cfg.svlan  = bucket_data->svlan;
        cfg.cvlan  = bucket_data->cvlan;
        ctcs_gint_remove_service(lchip, &cfg);
    }

    return 0;
}

#if 0
STATIC int32
_ctc_gint_service_update_mac_limit(ctc_gint_service_node_t* bucket_data , ctc_gint_service_mac_lim_t* p_mac_lim)
{
    ctc_vlan_mapping_t vlan_mapping;
    ctc_gint_dsl_node_t*  p_dsl_hash_node = NULL;
    uint8 lchip = 0;
    if (bucket_data->dsl_id == p_mac_lim->dsl_id)/*hit*/
    {
        lchip = p_mac_lim->lchip;
        /*get dsl logic port*/
        p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_mac_lim);
        if (NULL == p_dsl_hash_node)
        {
            return CTC_E_NOT_EXIST;
        }
    
        /*get service vlan mapping entry*/
        sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
        CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
        vlan_mapping.old_svid = bucket_data->svlan;
        vlan_mapping.old_cvid = bucket_data->cvlan;
        vlan_mapping.scl_id = CTC_GINT_SERVICE_SCL_ID;
        vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
        if (bucket_data->vlan_mapping_use_flex == 1)
        {
            CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
        }
        CTC_ERROR_RETURN(ctcs_vlan_get_vlan_mapping(lchip, p_dsl_hash_node->logic_port, &vlan_mapping));

        
        if (p_mac_lim->learn_disable)
        {
            CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS);
            CTC_ERROR_RETURN(ctcs_vlan_update_vlan_mapping(lchip, p_dsl_hash_node->logic_port, &vlan_mapping));
        }
        else
        {   
            CTC_UNSET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_FLAG_VPLS_LRN_DIS);
            CTC_ERROR_RETURN(ctcs_vlan_update_vlan_mapping(lchip, p_dsl_hash_node->logic_port, &vlan_mapping));
        }

    }

    return 0;
}
#endif

/*
 @brief This function is to write chip to set the destination port
*/
STATIC int32
_ctc_gint_set_iloop_port(uint8 lchip, uint32* p_iloop_port ,bool is_alloc)
{
    uint8  gchip = 0;
    ctc_internal_port_assign_para_t port_assign;

    ctcs_get_gchip_id(lchip, &gchip);
    sal_memset(&port_assign, 0, sizeof(port_assign));
    port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    port_assign.gchip = gchip;
    if (is_alloc)
    {
        CTC_ERROR_RETURN(ctcs_alloc_internal_port(lchip, &port_assign));
        *p_iloop_port = port_assign.inter_port;
        *p_iloop_port = CTC_MAP_LPORT_TO_GPORT(gchip, *p_iloop_port);
    }
    else
    {
        port_assign.inter_port = CTC_MAP_GPORT_TO_LPORT(*p_iloop_port);
        CTC_ERROR_RETURN(ctcs_free_internal_port(lchip, &port_assign));
    }
    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_logic_port_init_opf(uint8 lchip)
{
    ctc_opf_t opf;

    CTC_ERROR_RETURN(ctc_opf_init(CTC_OPF_CUSTOM_ID_START, 1));

    sal_memset(&opf, 0, sizeof(ctc_opf_t));
    opf.pool_type = CTC_OPF_CUSTOM_ID_START;

    opf.pool_index = 0;
    CTC_ERROR_RETURN(ctc_opf_init_offset(&opf, 1, CTC_GINT_PORT_LOGIC_PORT_NUM-1));

    return CTC_E_NONE;
}
STATIC int32
_ctc_gint_policer_id_init_opf(uint8 lchip)
{
    ctc_opf_t opf;
    
    CTC_ERROR_RETURN(ctc_opf_init(CTC_OPF_CUSTOM_ID_START + 1, 1));

    sal_memset(&opf, 0, sizeof(ctc_opf_t));
    opf.pool_type = CTC_OPF_CUSTOM_ID_START + 1;

    opf.pool_index = 0;
    CTC_ERROR_RETURN(ctc_opf_init_offset(&opf, CTC_GINT_POLICER_START, CTC_GINT_POLICER_END - CTC_GINT_POLICER_START + 1));

    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_set_logic_port(uint8 lchip, uint16* p_logic_port, bool is_alloc)
{
    ctc_opf_t opf;
    uint32 logic_port_32 = 0;
    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_CUSTOM_ID_START;
    opf.pool_index = 0;
    
    if (is_alloc)
    {
        CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, &logic_port_32));
        *p_logic_port = logic_port_32;
    }
    else
    {
        logic_port_32 = *p_logic_port;
        CTC_ERROR_RETURN(ctc_opf_free_offset(&opf, 1, logic_port_32));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_set_policer_id(uint8 lchip, uint32* p_policer_id, bool is_alloc)
{
    ctc_opf_t opf;

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = CTC_OPF_CUSTOM_ID_START + 1;
    opf.pool_index = 0;
    
    if (is_alloc)
    {
        CTC_ERROR_RETURN(ctc_opf_alloc_offset(&opf, 1, p_policer_id));
    }
    else
    {
        CTC_ERROR_RETURN(ctc_opf_free_offset(&opf, 1, *p_policer_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_map_sid_to_user_data(uint32 sid, uint32* p_tci)
{
    uint8 buf[4] = {0};
    
    if (sid > CTC_GINT_SID_MAX) 
    {
        return CTC_E_INVALID_PARAM;
    }
    /*
    TCI:
    0 b[x][x][x][x] [x][x][1][0]
    1 b[9][8][7][6] [5][4][3][2]
    2 b[x][x][x][x] [x][x][x][x]
    3 b[x][x][x][x] [x][x][x][x]
    */  
 
    buf[1] = sid >> 2;
    buf[0] = sid & 0x03;
    sal_memcpy(p_tci, buf, 4);

    return CTC_E_NONE;
}


STATIC int32
_ctc_gint_uni_acl_portbitmap_enable(uint8 lchip, uint8 enable) 
{
    ctc_acl_classify_udf_t acl_udf_entry = {0}; 
    ctc_field_key_t  acl_udf_key_field1  = {0};
    ctc_field_key_t  acl_udf_key_field2  = {0};
    ctc_field_port_t port_info           = {0};
    uint32   acl_udf_id = CTC_GINT_UNI_ACL_UDF_ENTRY_ID;

    if (!enable)
    {
        /***acl udf entry port bitmap***/
        acl_udf_entry.udf_id = acl_udf_id;                                                        
        acl_udf_entry.offset_type = CTC_ACL_UDF_OFFSET_L2;                           
        acl_udf_entry.offset_num = 4;                            
        acl_udf_entry.offset[0] = 0;              
        CTC_ERROR_RETURN(ctcs_acl_remove_udf_entry(lchip, &acl_udf_entry));   
        return CTC_E_NONE;
    }
    
    /***acl udf entry port bitmap***/
    acl_udf_entry.udf_id = acl_udf_id;                                                        
    acl_udf_entry.offset_type = CTC_ACL_UDF_OFFSET_L2;                           
    acl_udf_entry.offset_num = 4;                            
    acl_udf_entry.offset[0] = 0;              
    CTC_ERROR_RETURN(ctcs_acl_add_udf_entry(lchip, &acl_udf_entry));

    /*get the bitmap value*/
    sal_memset(port_info.port_bitmap, 0, sizeof(ctc_port_bitmap_t)); 
    port_info.port_bitmap[0] = 0xffff;
    port_info.port_bitmap[1] = 0xffff;
    acl_udf_key_field1.type = CTC_FIELD_KEY_PORT; 
    port_info.type = CTC_FIELD_PORT_TYPE_PORT_BITMAP;
    acl_udf_key_field1.ext_data = &port_info;    
    acl_udf_key_field1.ext_mask = NULL;   
    CTC_ERROR_RETURN(ctcs_acl_add_udf_entry_key_field(lchip, acl_udf_id, &acl_udf_key_field1));

    acl_udf_key_field2.type = CTC_FIELD_KEY_UDF_ENTRY_VALID;
    acl_udf_key_field2.data = 1;        
    acl_udf_key_field2.mask = 1;        
    acl_udf_key_field2.ext_data = NULL;    
    acl_udf_key_field2.ext_mask = NULL; 
    CTC_ERROR_RETURN(ctcs_acl_add_udf_entry_key_field(lchip, acl_udf_id, &acl_udf_key_field2));

    return CTC_E_NONE;
}


STATIC int32
_ctc_gint_service_vlan_mapping(uint8 lchip, ctc_gint_service_t* p_vlan_port,
                               ctc_vlan_mapping_t* p_vlan_mapping)
{
    /*SVLAN op*/
    switch(p_vlan_port->igs_vlan_action.svid_action)
    {
        case CTC_GINT_VLAN_ACTION_ADD:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->igs_vlan_action.new_svid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_svid = p_vlan_port->igs_vlan_action.new_svid;
            }
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->igs_vlan_action.new_svid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_mapping->svid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_svid = p_vlan_port->igs_vlan_action.new_svid;
            }
            break;

        case CTC_GINT_VLAN_ACTION_DEL:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_DEL;
            }
            break;

         case CTC_GINT_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SVID);
                p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_VALID;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->igs_vlan_action.cvid_action)
    {
        case CTC_GINT_VLAN_ACTION_ADD:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->igs_vlan_action.new_cvid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_ADD;
                p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_cvid = p_vlan_port->igs_vlan_action.new_cvid;
            }
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            CTC_VLAN_RANGE_CHECK(p_vlan_port->igs_vlan_action.new_cvid);
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
                p_vlan_mapping->cvid_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_cvid = p_vlan_port->igs_vlan_action.new_cvid;
            }
            break;

        case CTC_GINT_VLAN_ACTION_DEL:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_DEL;
            }
            break;


         case CTC_GINT_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CVID);
                p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_VALID;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->igs_vlan_action.scos_action)
    {
        case CTC_GINT_VLAN_ACTION_ADD:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_scos = p_vlan_port->igs_vlan_action.new_scos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
            }
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_scos = p_vlan_port->igs_vlan_action.new_scos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_SCOS);
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->stag_op)
                {
                    p_vlan_mapping->stag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            break;

        case  CTC_GINT_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->scos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    switch(p_vlan_port->igs_vlan_action.ccos_action)
    {
        case CTC_GINT_VLAN_ACTION_ADD:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_ccos = p_vlan_port->igs_vlan_action.new_ccos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
            }
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_NEW;
                p_vlan_mapping->new_ccos = p_vlan_port->igs_vlan_action.new_ccos;
                CTC_SET_FLAG(p_vlan_mapping->action, CTC_VLAN_MAPPING_OUTPUT_CCOS);
                if(CTC_VLAN_TAG_OP_NONE == p_vlan_mapping->ctag_op)
                {
                    p_vlan_mapping->ctag_op = CTC_VLAN_TAG_OP_REP;
                }
            }
            break;

         case CTC_GINT_VLAN_ACTION_NONE:
            if (p_vlan_mapping)
            {
                p_vlan_mapping->ccos_sl = CTC_VLAN_TAG_SL_AS_PARSE;
            }
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}



STATIC int32
_ctc_gint_service_nh_xlate_mapping(uint8 lchip, ctc_gint_service_t* p_vlan_port,
                               ctc_vlan_egress_edit_info_t* p_xlate)
{
    /* SVLAN op */
    switch(p_vlan_port->egs_vlan_action.svid_action)
    {
        case CTC_GINT_VLAN_ACTION_DEL:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_svid = p_vlan_port->egs_vlan_action.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

         case CTC_GINT_VLAN_ACTION_NONE:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_GINT_VLAN_ACTION_ADD:
            p_xlate->svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_svid = p_vlan_port->egs_vlan_action.new_svid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CVLAN op */
    switch(p_vlan_port->egs_vlan_action.cvid_action)
    {
        case CTC_GINT_VLAN_ACTION_DEL:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_STRIP_VLAN;
            break;

        case CTC_GINT_VLAN_ACTION_REPLACE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_REPLACE_VLAN;
            p_xlate->output_cvid = p_vlan_port->egs_vlan_action.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;

        case CTC_GINT_VLAN_ACTION_NONE:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
            break;

        case CTC_GINT_VLAN_ACTION_ADD:
            p_xlate->cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
            p_xlate->output_cvid = p_vlan_port->egs_vlan_action.new_cvid;
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID);
            break;


        default:
            return CTC_E_INVALID_PARAM;
    }

    /* SCOS op */
    switch(p_vlan_port->egs_vlan_action.scos_action)
    {
        case CTC_GINT_VLAN_ACTION_NONE:
            break;

        case CTC_GINT_VLAN_ACTION_ADD:
        case CTC_GINT_VLAN_ACTION_REPLACE:
            CTC_SET_FLAG(p_xlate->edit_flag, CTC_VLAN_EGRESS_EDIT_REPLACE_SVLAN_COS);
            p_xlate->stag_cos = p_vlan_port->egs_vlan_action.new_scos;

         break;
        case CTC_GINT_VLAN_ACTION_DEL:
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    /* CCOS op */
    switch(p_vlan_port->egs_vlan_action.ccos_action)
    {
        case CTC_GINT_VLAN_ACTION_NONE:
            break;

        case CTC_GINT_VLAN_ACTION_ADD:
        case CTC_GINT_VLAN_ACTION_REPLACE:
            return CTC_E_INVALID_PARAM;

        case CTC_GINT_VLAN_ACTION_DEL:
            break;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_add_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_user_data, ctc_gint_dsl_node_t* p_node)
{
    uint8                  is_update = 0;
    uint32                 group_id = 0;
    ctc_qos_policer_t      policer_param;
    ctc_field_port_t       port_data;
    ctc_field_port_t       port_mask;
    ctc_acl_entry_t        acl_entry;
    ctc_field_key_t        acl_key_field;
    ctc_acl_field_action_t acl_action_field;
    uint32                 policer_id = 0;
    uint8                  acl_key_type   = 0;
    mac_addr_t  boardcast_data = {0xff,0xff,0xff, 0xff,0xff,0xff};
    mac_addr_t  boardcast_mask = {0xff,0xff,0xff, 0xff,0xff,0xff};
    mac_addr_t  muticast_data =  {0x01,0,0,0,0,0};
    mac_addr_t  muticast_mask =  {0x01,0,0,0,0,0};

    
    is_update = p_node->storm_restrict[p_user_data->type].acl_entry_id ? 1 : 0;
    switch (p_user_data->type)
    {
        case 0:
        {   
            group_id = CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST;
            acl_key_type = CTC_ACL_KEY_MAC;
            break;
        }
        case 1:
        {
            group_id = CTC_GINT_ACL_GRP_ID_STORM_MUTICAST;
            acl_key_type = CTC_ACL_KEY_MAC;
            break;
        }
        case 2:
        {
            group_id = CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN; 
            acl_key_type = CTC_ACL_KEY_FWD;
            break;
        }
        default:
            return CTC_E_INVALID_PARAM;
    }
    
    /*S1: alloc policer id*/
    if (!is_update)
    {
        _ctc_gint_set_policer_id(lchip, &policer_id, 1);
    }
    else
    {
        policer_id = p_node->storm_restrict[p_user_data->type].policer_id;
    }
    
    /*S2 :create/update qos policer*/
    if (is_update 
        && (p_node->storm_restrict[p_user_data->type].cir == p_user_data->cir)
        && (p_node->storm_restrict[p_user_data->type].cbs == p_user_data->cbs)
        && (p_node->dsl_id == p_user_data->dsl_id))
    {
        return CTC_E_EXIST;
    }
    /*default cfg*/
    sal_memset(&policer_param, 0, sizeof(ctc_qos_policer_t));
    policer_param.type   = CTC_QOS_POLICER_TYPE_FLOW;
    policer_param.dir = CTC_INGRESS;
    policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
    policer_param.policer.is_color_aware = 0;
    policer_param.policer.pbs = 0;
    policer_param.policer.drop_color = CTC_QOS_COLOR_RED;
    policer_param.policer.use_l3_length = 0;
    /*user cfg*/
    policer_param.id.policer_id = policer_id;
    policer_param.enable = 1;  
    policer_param.policer.cir = p_user_data->cir;
    policer_param.policer.cbs = p_user_data->cbs;
    CTC_ERROR_RETURN(ctcs_qos_set_policer(lchip, &policer_param));
    
    p_node->storm_restrict[p_user_data->type].policer_id = policer_id;
    p_node->storm_restrict[p_user_data->type].cir = p_user_data->cir;
    p_node->storm_restrict[p_user_data->type].cbs = p_user_data->cbs;
    p_node->storm_restrict[p_user_data->type].enable = 1;
        
    /*S3: create ACL*/
    if (!is_update)
    {
        /*add*/
        /*create entry*/
        sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
        acl_entry.key_type          = acl_key_type;            
        acl_entry.entry_id          = CTC_GINT_ENCODE_ACL_ENTRY_ID(p_node->logic_port, p_user_data->type);            
        acl_entry.mode              = 1; 
        CTC_ERROR_RETURN(ctcs_acl_add_entry(lchip, group_id, &acl_entry));
        p_node->storm_restrict[p_user_data->type].acl_entry_id = acl_entry.entry_id;

        /*key: logic port + macda*/
        sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
        sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));  
        sal_memset(&acl_key_field, 0, sizeof(ctc_field_key_t));
        port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        port_data.logic_port = p_node->logic_port;
        port_mask.logic_port = 0xFFFF;

        acl_key_field.type = CTC_FIELD_KEY_PORT;
        acl_key_field.ext_data = &port_data;
        acl_key_field.ext_mask = &port_mask;
        CTC_ERROR_RETURN(ctcs_acl_add_key_field(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id , &acl_key_field));

        if (CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN == group_id)
        {
            sal_memset(&acl_key_field, 0, sizeof(ctc_field_key_t));
            acl_key_field.type = CTC_FIELD_KEY_MACDA_HIT;
            acl_key_field.data = 0;
            acl_key_field.mask = 1;
            CTC_ERROR_RETURN(ctcs_acl_add_key_field(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id , &acl_key_field));
        }
        else
        {
            sal_memset(&acl_key_field, 0, sizeof(ctc_field_key_t));
            acl_key_field.type = CTC_FIELD_KEY_MAC_DA;
            if (CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST == group_id)
            {
                acl_key_field.ext_data = boardcast_data;
                acl_key_field.ext_mask = boardcast_mask;
            }
            else/*CTC_GINT_ACL_GRP_ID_STORM_MUTICAST*/
            {
                acl_key_field.ext_data = muticast_data;
                acl_key_field.ext_mask = muticast_mask;        
            }
            CTC_ERROR_RETURN(ctcs_acl_add_key_field(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id , &acl_key_field));        
        }

        /*action: policer*/
        sal_memset(&acl_action_field, 0, sizeof(ctc_acl_field_action_t));
        acl_action_field.type =  CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
        acl_action_field.data0 = policer_id;
        CTC_ERROR_RETURN(ctcs_acl_add_action_field(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id, &acl_action_field));   
        CTC_ERROR_RETURN(ctcs_acl_install_entry(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id));

    }
    
    return CTC_E_NONE;
}

STATIC int32
_ctc_gint_remove_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_user_data, ctc_gint_dsl_node_t* p_node)
{
    ctc_qos_policer_t policer_param;

    if (p_node->storm_restrict[p_user_data->type].acl_entry_id == 0)
    {
        return CTC_E_NONE;  
    }
    
    /*S1: Remove acl*/
    CTC_ERROR_RETURN(ctcs_acl_uninstall_entry(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id));
    CTC_ERROR_RETURN(ctcs_acl_remove_entry(lchip, p_node->storm_restrict[p_user_data->type].acl_entry_id));
       
    /*S2: Remove policer*/
    sal_memset(&policer_param, 0, sizeof(ctc_qos_policer_t));
    policer_param.type   = CTC_QOS_POLICER_TYPE_FLOW;
    policer_param.dir = CTC_INGRESS;
    policer_param.policer.policer_mode = CTC_QOS_POLICER_MODE_RFC2697;
    policer_param.id.policer_id = p_node->storm_restrict[p_user_data->type].policer_id;
    policer_param.enable = 0;  
    CTC_ERROR_RETURN(ctcs_qos_set_policer(lchip, &policer_param));

    /*S3: Free opf*/
    _ctc_gint_set_policer_id(lchip, &p_node->storm_restrict[p_user_data->type].policer_id, 0);

    /*S4: clear soft-table*/
    sal_memset(&(p_node->storm_restrict[p_user_data->type]) , 0 , sizeof(ctc_gint_storm_ctl_t));  
    
    return CTC_E_NONE;    
}

STATIC int32
_ctc_gint_uni_sid_check(uint8 lchip, uint32 sid)
{
    int32 ret =0;

    ret = ctc_hash_traverse(p_gint_master[lchip]->p_dsl_hash ,
                      (hash_traversal_fn)_ctc_gint_uni_sid_hash_match, &sid);
   
    return ret;
}

STATIC int32
_ctc_gint_uni_gport_check(uint8 lchip, uint32 gport)
{
    int32 ret = 0;
    
    ret = ctc_hash_traverse(p_gint_master[lchip]->p_dsl_hash ,
                      (hash_traversal_fn)_ctc_gint_uni_gport_hash_match, &gport);

    return ret;
}

STATIC ctc_gint_dsl_node_t*
_ctc_gint_find_dsl_hash_node(uint8 lchip, uint32 logic_port)
{
    ctc_gint_dsl_node_find_t find_node;

    sal_memset(&find_node, 0, sizeof(ctc_gint_dsl_node_find_t));
    find_node.logic_port = logic_port;
    ctc_hash_traverse(p_gint_master[lchip]->p_dsl_hash ,
                      (hash_traversal_fn)_ctc_gint_dsl_hash_match, &find_node);

    return find_node.p_dsl_node;
}


STATIC ctc_gint_service_node_t*
_ctc_gint_find_service_hash_node(uint8 lchip, uint32 logic_port)
{
    ctc_gint_service_node_find_t find_node;

    sal_memset(&find_node, 0, sizeof(ctc_gint_service_node_find_t));
    find_node.logic_port = logic_port;
    ctc_hash_traverse(p_gint_master[lchip]->p_service_hash ,
                      (hash_traversal_fn)_ctc_gint_service_hash_match, &find_node);

    return find_node.p_service_node;
}

#if 0
STATIC int32
_ctc_gint_service_upd_mac_limit_all(uint8 lchip, uint32 dsl_id, uint8 learn_disable)
{
    ctc_gint_service_mac_lim_t ser_mac_lim;
    
    sal_memset(&ser_mac_lim, 0, sizeof(ctc_gint_service_mac_lim_t));

    ser_mac_lim.dsl_id = dsl_id;
    ser_mac_lim.learn_disable = learn_disable;
    ser_mac_lim.lchip = lchip;
    ctc_hash_traverse(p_gint_master[lchip]->p_service_hash ,
                      (hash_traversal_fn)_ctc_gint_service_update_mac_limit, &ser_mac_lim);

    return 0;
}
#endif

STATIC int32 
_ctc_gint_mac_limit(uint8 lchip, ctc_gint_dsl_node_t* p_dsl_node, ctc_gint_service_node_t* p_service_node, uint8 is_add)
{
    int ret = 0;
    ctc_gint_dsl_node_t* p_dsl_hash_node = NULL; 
    
    if (p_dsl_node)
    {
        if (is_add)
        {
            p_dsl_node->mac_limit.limit_cnt ++;
            if (p_dsl_node->mac_limit.enable && p_dsl_node->mac_limit.limit_num  &&  (p_dsl_node->mac_limit.limit_cnt >= p_dsl_node->mac_limit.limit_num))
            {
                ret =  -1;/*out of threshold*/
            }
        }
        else
        {   
            if (p_dsl_node->mac_limit.enable && p_dsl_node->mac_limit.limit_num  &&  (p_dsl_node->mac_limit.limit_cnt >= p_dsl_node->mac_limit.limit_num))
            {
                ret =  -1;/*out of threshold*/
            }
            p_dsl_node->mac_limit.limit_cnt --;
        }
    }
    else if(p_service_node)
    {
        /*get dsl logic port*/
        p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_service_node);
        if (NULL == p_dsl_hash_node)
        {
            return CTC_E_NOT_EXIST;
        }
        
        if (is_add)
        {
            p_dsl_hash_node->mac_limit.limit_cnt ++;
            if (p_dsl_hash_node->mac_limit.enable && p_dsl_hash_node->mac_limit.limit_num  &&  (p_dsl_hash_node->mac_limit.limit_cnt >= p_dsl_hash_node->mac_limit.limit_num))
            {
                ret = -1;/*out of threshold*/
            }  
        }
        else
        {   
            if (p_dsl_hash_node->mac_limit.enable && p_dsl_hash_node->mac_limit.limit_num  &&  (p_dsl_hash_node->mac_limit.limit_cnt >= p_dsl_hash_node->mac_limit.limit_num))
            {
                ret =  -1;/*out of threshold*/
            }  
            p_dsl_hash_node->mac_limit.limit_cnt --;
        }
    }

    return ret;
}

int32
_ctc_gint_update_logic_port_learning(uint8 lchip, ctc_gint_dsl_node_t* p_dsl_node, ctc_gint_service_node_t* p_service_node, uint8 learn_disable) 
{
    ctc_scl_field_action_t field_action;
    ctc_gint_dsl_node_t* p_dsl_hash_node = NULL; 

    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DENY_LEARNING;
    field_action.data0 = 1;
    field_action.data1 = 1;
    
    if (p_dsl_node)
    {
        p_dsl_hash_node = p_dsl_node;
    }
    else if(p_service_node)
    {
        /*get dsl logic port*/
        p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_service_node);
    }

    /*for flex add/remove dslid*/
    if (NULL == p_dsl_hash_node)
    {
        return CTC_E_NOT_EXIST;
    }
    
    /*travers all service node: update scl action deny learning*/
    /*CTC_ERROR_RETURN(_ctc_gint_service_upd_mac_limit_all(lchip, p_dsl_hash_node->dsl_id, learn_disable));*/       

    
    if (learn_disable)
    {
        CTC_ERROR_RETURN(ctcs_scl_add_action_field(lchip, p_dsl_hash_node->limit_entry_id, &field_action));
    }
    else
    {         
        CTC_ERROR_RETURN(ctcs_scl_remove_action_field(lchip, p_dsl_hash_node->limit_entry_id, &field_action));
    }
    return CTC_E_NONE;
}

int32
_ctc_gint_hw_learning_sync(uint8 gchip, void* p_data)
{
    uint8 lchip = 0;
    uint16 pkt_svlan = 0;
    uint16 pkt_cvlan = 0;
    uint32 index = 0;
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;
    ctc_learning_cache_t* p_cache = (ctc_learning_cache_t*)p_data;
    ctc_l2_addr_t l2_addr;
    ctc_gint_dsl_node_t*     p_dsl_node = NULL;
    ctc_gint_service_node_t* p_service_node = NULL;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    ctc_app_index_get_lchip_id(gchip, &lchip);
    for (index = 0; index < p_cache->entry_num; index++)
    {
        /* pizzabox */
        l2_addr.fid = p_cache->learning_entry[index].fid;
        l2_addr.flag = 0;

        /*Using Dma*/
        l2_addr.mac[0] = p_cache->learning_entry[index].mac[0];
        l2_addr.mac[1] = p_cache->learning_entry[index].mac[1];
        l2_addr.mac[2] = p_cache->learning_entry[index].mac[2];
        l2_addr.mac[3] = p_cache->learning_entry[index].mac[3];
        l2_addr.mac[4] = p_cache->learning_entry[index].mac[4];
        l2_addr.mac[5] = p_cache->learning_entry[index].mac[5];
        
        if (!p_cache->learning_entry[index].is_logic_port)
        {
            continue;
        }
  
        l2_addr.gport = p_cache->learning_entry[index].logic_port;
        pkt_svlan = p_cache->learning_entry[index].svlan_id;
        pkt_cvlan = p_cache->learning_entry[index].cvlan_id;

        if (l2_addr.gport == p_gint_master[lchip]->nni_logic_port)
        {
            nhid = CTC_GINT_ENCODE_NH_ID(p_gint_master[lchip]->nni_logic_port);
        }
        else
        {
            /*service*/
            p_service_node = _ctc_gint_find_service_hash_node(lchip, l2_addr.gport);
            if ((NULL != p_service_node) && ((pkt_svlan == p_service_node->svlan) && (pkt_cvlan == p_service_node->cvlan)))
            {

                 nhid =  p_service_node->nh_id;
            }
            else  
            {
                /*dsl_id*/
                p_dsl_node = _ctc_gint_find_dsl_hash_node(lchip, l2_addr.gport);
                if (p_dsl_node)
                {
                    nhid =  p_dsl_node->nh_id;
                }
                else
                {   
                    continue;
                }
            }

            if ((NULL == p_service_node) && (NULL == p_dsl_node))
            {
                continue;  
            }
        }
               
        if  (!p_cache->learning_entry[index].is_hw_sync)/*sw learning*/
        {   

            ret = ctcs_l2_add_fdb_with_nexthop(lchip, &l2_addr, nhid);
            CTC_GINT_DBG_INFO("Learning: logic_port->0x%x \n", l2_addr.gport);
        }
        else/*hw learning*/
        {
            if(p_cache->learning_entry[index].is_logic_port)
            {
                CTC_GINT_DBG_INFO("Learning Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d  LogicPort:%d\n",
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                                                    l2_addr.fid, l2_addr.gport);
            }
        }
        if (0 != _ctc_gint_mac_limit(lchip, p_dsl_node, p_service_node, 1))
        {
            /*logic port learning disable*/
            CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_node, p_service_node, 1));
        }
    }

    return ret;
}

int32
_ctc_gint_hw_aging_sync(uint8 gchip, void* p_data)
{
    ctc_aging_fifo_info_t* p_fifo = (ctc_aging_fifo_info_t*)p_data;
    uint32 i = 0;
    ctc_l2_addr_t l2_addr;
    ctc_l2_addr_t l2_addr_rst;
    uint8 lchip = 0;
    ctc_gint_dsl_node_t*     p_dsl_node = NULL;
    ctc_gint_service_node_t* p_service_node = NULL;

    sal_memset(&l2_addr_rst, 0, sizeof(ctc_l2_addr_t));

    ctc_app_index_get_lchip_id(gchip, &lchip);
    /*Using Dma*/
    for (i = 0; i < p_fifo->fifo_idx_num; i++)
    {
        ctc_aging_info_entry_t* p_entry = NULL;
        sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

        p_entry = &(p_fifo->aging_entry[i]);
        l2_addr.fid = p_entry->fid;
        CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_REMOVE_DYNAMIC);
        l2_addr.gport = p_entry->gport;
        if (p_entry->is_logic_port == 0)
        {
            continue;
        }
        
        p_service_node = _ctc_gint_find_service_hash_node(lchip, l2_addr.gport);
        if (NULL == p_service_node)
        {
            p_dsl_node = _ctc_gint_find_dsl_hash_node(lchip, l2_addr.gport);
        }

        
        if (!p_entry->is_hw_sync)
        {
            CTC_ERROR_RETURN(ctcs_l2_remove_fdb(lchip, &l2_addr));
        }
        else
        {
            CTC_GINT_DBG_INFO("Aging Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d\n",
                                                sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                                                sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                                                sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                                                l2_addr.fid);  
        }
        
        if (0 != _ctc_gint_mac_limit(lchip, p_dsl_node, p_service_node, 0))
        {
            /*logic port learning enable*/
            CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_node, p_service_node, 0));
        }
    }
    return CTC_E_NONE;
}

STATIC int32 
_ctc_gint_set_gport_default_fid(uint8 lchip, uint32 gport, uint32 scl_entry_id, uint8 is_add)
{
    ctc_port_scl_property_t scl_prop;
    ctc_scl_entry_t scl_entry;
    ctc_field_key_t scl_key_field;
    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;
    ctc_scl_field_action_t scl_action_field;
    
    if (is_add)
    {
        /*port enable scl lookup*/
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = CTC_GINT_DEFT_VLAN_SCL_ID;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
        CTC_ERROR_RETURN(ctcs_port_set_scl_property(lchip, gport, &scl_prop));

        /*SCL FLOW*/
        sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
        scl_entry.key_type = CTC_SCL_KEY_TCAM_MAC;
        scl_entry.action_type = CTC_SCL_ACTION_FLOW;
        scl_entry.entry_id = scl_entry_id;
        scl_entry.mode     = 1;
        CTC_ERROR_RETURN(ctcs_scl_add_entry(lchip, CTC_GINT_DEFT_VLAN_SCL_GROUP, &scl_entry));

        /*key: PORT + SVLAN0*/
        sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
        sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));  
        sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
        port_data.type = CTC_FIELD_PORT_TYPE_GPORT;
        port_data.gport = gport;
        port_mask.gport = 0xFFFF;

        scl_key_field.type = CTC_FIELD_KEY_PORT;
        scl_key_field.ext_data = &port_data;
        scl_key_field.ext_mask = &port_mask;
        CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, scl_entry_id, &scl_key_field));

        sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
        scl_key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        scl_key_field.data = 0;
        scl_key_field.mask = 0xffff;
        CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, scl_entry_id, &scl_key_field));

        /*action: fid*/
        sal_memset(&scl_action_field, 0, sizeof(ctc_scl_field_action_t));
        scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
        scl_action_field.data0 = CTC_GINT_DEFAULT_FID;
        CTC_ERROR_RETURN(ctcs_scl_add_action_field(lchip, scl_entry_id, &scl_action_field));
        CTC_ERROR_RETURN(ctcs_scl_install_entry(lchip, scl_entry_id));
    }
    else
    {
        /*port enable scl lookup*/
        sal_memset(&scl_prop, 0, sizeof(scl_prop));
        scl_prop.scl_id = CTC_GINT_DEFT_VLAN_SCL_ID;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
        CTC_ERROR_RETURN(ctcs_port_set_scl_property(lchip, gport, &scl_prop));
        
        /*remove scl entry*/
        CTC_ERROR_RETURN(ctcs_scl_uninstall_entry(lchip, scl_entry_id));
        CTC_ERROR_RETURN(ctcs_scl_remove_entry(lchip, scl_entry_id));
    }

    return CTC_E_NONE;
}

STATIC int32 
_ctc_gint_set_logic_port_default_fid(uint8 lchip, uint32 nh_id, uint8 is_add)
{
    ctc_l2dflt_addr_t l2dflt_addr;

    sal_memset(&l2dflt_addr, 0 , sizeof(ctc_l2dflt_addr_t));
        
    l2dflt_addr.with_nh = 1;
    l2dflt_addr.member.nh_id = nh_id;
    l2dflt_addr.fid = CTC_GINT_DEFAULT_FID;
    if (is_add)
    {
        CTC_ERROR_RETURN(ctcs_l2_add_port_to_default_entry(lchip, &l2dflt_addr)); 
    }
    else
    {
        CTC_ERROR_RETURN(ctcs_l2_remove_port_from_default_entry(lchip, &l2dflt_addr));        
    }
    return CTC_E_NONE;
}

#define _GINT_SECURITY_  ""
int32 
ctcs_gint_set_port_isolation(uint8 lchip, uint32 dsl_id, uint8 enable)
{
    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;
    ctc_scl_entry_t  scl_entry;
    ctc_field_key_t  scl_key_field;
    ctc_scl_field_action_t scl_action_field;
    uint16 dsl_logic_port = 0;
    ctc_gint_dsl_node_t*     p_dsl_hash_node = NULL; 

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    

    /*get dsl logic port*/
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, &dsl_id);
    if ((NULL == p_dsl_hash_node) || (0 == p_dsl_hash_node->logic_port) || (0 == p_gint_master[lchip]->uni_iloop_port))
    {
        return CTC_E_INVALID_PARAM;
    }
    dsl_logic_port = p_dsl_hash_node->logic_port;

   
    /*P1:remove*/
    if (!enable)
    {
        if (p_dsl_hash_node->isolation_entry_id)
        {
            CTC_ERROR_RETURN(ctcs_scl_uninstall_entry(lchip, p_dsl_hash_node->isolation_entry_id));
            CTC_ERROR_RETURN(ctcs_scl_remove_entry(lchip, p_dsl_hash_node->isolation_entry_id));

            p_dsl_hash_node->isolation_entry_id = 0;
        }
        return CTC_E_NONE;
    }

    /*P2: update*/
    if (p_dsl_hash_node->isolation_entry_id)
    {
        return CTC_E_NONE;    
    }
        
    /*create entry*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.key_type = CTC_SCL_KEY_HASH_PORT;
    scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
    scl_entry.entry_id = CTC_GINT_ENCODE_ISOLATION_ENTRY_ID(dsl_logic_port);
    scl_entry.mode     = 1;
    CTC_ERROR_RETURN(ctcs_scl_add_entry(lchip, CTC_GINT_PORT_ISOLATION_SCL_GROUP, &scl_entry));
    p_dsl_hash_node->isolation_entry_id = scl_entry.entry_id;

    /*key: logic port*/
    sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));  
    sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
    port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
    port_data.logic_port = dsl_logic_port;
    port_mask.logic_port = 0xFFFF;

    scl_key_field.type = CTC_FIELD_KEY_PORT;
    scl_key_field.ext_data = &port_data;
    scl_key_field.ext_mask = &port_mask;
    CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, p_dsl_hash_node->isolation_entry_id, &scl_key_field));

    sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
    scl_key_field.type = CTC_FIELD_KEY_HASH_VALID;
    CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, p_dsl_hash_node->isolation_entry_id, &scl_key_field));


    /*action: enable logic port type*/
    sal_memset(&scl_action_field, 0, sizeof(ctc_scl_field_action_t));
    scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT_TYPE;
    CTC_ERROR_RETURN(ctcs_scl_add_action_field(lchip, scl_entry.entry_id, &scl_action_field));   

    CTC_ERROR_RETURN(ctcs_scl_install_entry(lchip, scl_entry.entry_id));

    return CTC_E_NONE;
}

int32 
ctcs_gint_get_port_isolation(uint8 lchip, uint32 dsl_id, uint8* enable)
{
    ctc_gint_dsl_node_t*  p_dsl_hash_node = NULL;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    
    /*get dsl logic port*/
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, &dsl_id);
    if ((NULL == p_dsl_hash_node) || (0 == p_dsl_hash_node->logic_port))
    {
        *enable = 0;
        return CTC_E_NONE;
    }

    *enable = p_dsl_hash_node->isolation_entry_id ? 1 : 0;
    
    
    return CTC_E_NONE;
}

int32
ctcs_gint_set_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_cfg)
{
    ctc_gint_dsl_node_t*   p_dsl_hash_node = NULL; 

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if (p_cfg->type >= CTC_GINT_STORM_RESTRICT_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    /*get dsl logic port*/
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if ((NULL == p_dsl_hash_node) || (0 == p_dsl_hash_node->logic_port))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (0 == p_cfg->enable)/*remove*/
    {
        CTC_ERROR_RETURN(_ctc_gint_remove_storm_restrict(lchip, p_cfg, p_dsl_hash_node));     
    }
    else/*add or update*/
    {
        CTC_ERROR_RETURN(_ctc_gint_add_storm_restrict(lchip, p_cfg, p_dsl_hash_node));  
    }
    
    return CTC_E_NONE;
}

int32
ctcs_gint_get_storm_restrict(uint8 lchip, ctc_gint_storm_restrict_t* p_cfg)
{
    ctc_gint_dsl_node_t*   p_dsl_hash_node = NULL;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if (p_cfg->type >= CTC_GINT_STORM_RESTRICT_TYPE_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if ((NULL == p_dsl_hash_node) || (0 == p_dsl_hash_node->logic_port))
    {
        return CTC_E_INVALID_PARAM;
    }
    p_cfg->cbs = p_dsl_hash_node->storm_restrict[p_cfg->type].cbs;
    p_cfg->cir = p_dsl_hash_node->storm_restrict[p_cfg->type].cir;
    p_cfg->enable = p_dsl_hash_node->storm_restrict[p_cfg->type].enable;
    
    return CTC_E_NONE;
}

int32
ctcs_gint_set_mac_limit(uint8 lchip, ctc_gint_mac_limit_t* p_cfg)
{
    ctc_gint_dsl_node_t*       p_dsl_node = NULL;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    
    p_dsl_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if (NULL == p_dsl_node)
    {
        return CTC_E_NOT_EXIST;
    }

    if (p_cfg->limit_num == 0)
    {
        p_cfg->enable = 0;
    }
    
    if ((p_cfg->enable == 0) && (p_dsl_node->mac_limit.enable == 0))
    {
        return CTC_E_NONE;
    }
    if (p_cfg->enable && (p_cfg->limit_num < p_dsl_node->mac_limit.limit_cnt))
    {
        return CTC_E_INVALID_PARAM;
    }
    
    p_dsl_node->mac_limit.limit_num = p_cfg->limit_num; 
    p_dsl_node->mac_limit.enable    = p_cfg->enable;

    /*add deny-learing*/
    if ((p_dsl_node->mac_limit.enable) 
        && (p_dsl_node->mac_limit.limit_cnt) 
        && (p_dsl_node->mac_limit.limit_cnt == p_dsl_node->mac_limit.limit_num))
    {
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_node, NULL, 1));
    }
   
    /*remove deny-learing*/
    if ((p_dsl_node->mac_limit.enable == 0) 
        && (p_dsl_node->mac_limit.limit_cnt) 
        && (p_dsl_node->mac_limit.limit_cnt >= p_dsl_node->mac_limit.limit_num))
    {
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_node, NULL, 0));
    }
    return CTC_E_NONE;
}

int32
ctcs_gint_get_mac_limit(uint8 lchip, ctc_gint_mac_limit_t* p_cfg)
{
    ctc_gint_dsl_node_t*       p_dsl_node = NULL;

    CTC_GINT_DBG_FUNC();    
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);
    
    p_dsl_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if (NULL == p_dsl_node)
    {
        return CTC_E_NOT_EXIST;
    }
    
    p_cfg->limit_cnt  = p_dsl_node->mac_limit.limit_cnt;
    p_cfg->limit_num  = p_dsl_node->mac_limit.limit_num ; 
    p_cfg->enable     = p_dsl_node->mac_limit.enable;
    
    return CTC_E_NONE;
    
}

#define _GINT_CFG_ ""

int32
ctcs_gint_create_uni(uint8 lchip, ctc_gint_uni_t* p_cfg)
{
    uint16 logic_port = 0;
    ctc_gint_dsl_node_t* p_hash_node = NULL; 
    ctc_port_scl_property_t scl_prop;
    ctc_scl_entry_t  scl_entry;
    ctc_field_key_t  scl_key_field;
    ctc_scl_field_action_t scl_action_field;
    ctc_acl_udf_t udf_data;
    ctc_acl_udf_t udf_mask;
    ctc_scl_logic_port_t  scl_logic_port;
    ctc_scl_force_decap_t scl_decap;
    ctc_ip_tunnel_nh_param_t  nh_param;
    ctc_acl_property_t acl_prop;
    mac_addr_t mac_sa = {0xab,0xcd,0x12,0x34,0x56};
    mac_addr_t mac_da = {0x12,0x34,0x56,0xab,0xcd};
    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);
    if ((CTC_MAP_GPORT_TO_LPORT(p_cfg->gport) >= 64)/*acl udf portbitmap0-63 valid*/
        || (p_gint_master[lchip]->nni_logic_port && (p_cfg->gport == p_gint_master[lchip]->nni_gport)))
    {
        return CTC_E_INVALID_PARAM;
    }
    
    if (NULL != ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg))
    {
        return CTC_E_EXIST;
    }

    if (0 != _ctc_gint_uni_sid_check(lchip, p_cfg->sid))
    {
        return CTC_E_IN_USE;
    }
    
    /*S11:alloc logic port*/
    CTC_ERROR_RETURN(_ctc_gint_set_logic_port(lchip, &logic_port, 1)); 
    p_cfg->logic_port = logic_port;

    /*S12:HASH process*/
    /*insert node*/
    p_hash_node = (ctc_gint_dsl_node_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_gint_dsl_node_t));
    if (NULL == p_hash_node)
    {
        _ctc_gint_set_logic_port(lchip, &logic_port, 0);
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_hash_node, 0, sizeof(ctc_gint_dsl_node_t));
    p_hash_node->dsl_id =  p_cfg->dsl_id;
    p_hash_node->sid    =  p_cfg->sid;
    p_hash_node->gport  =  p_cfg->gport;
    p_hash_node->logic_port  =  p_cfg->logic_port;

    if (NULL == ctc_hash_insert(p_gint_master[lchip]->p_dsl_hash, p_hash_node))
    {
        if (p_hash_node)
        {
            mem_free(p_hash_node);
        }
        return CTC_E_NO_MEMORY;
    }
    
    /*S13:port enable scl lookup*/
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    scl_prop.scl_id = CTC_GINT_UNI_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_UDF;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    CTC_ERROR_RETURN(ctcs_port_set_scl_property(lchip, p_cfg->gport, &scl_prop));

    /*S14:SCL FLOW*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.key_type = CTC_SCL_KEY_TCAM_UDF;
    scl_entry.action_type = CTC_SCL_ACTION_FLOW;
    scl_entry.entry_id = CTC_GINT_ENCODE_SCL_ENTRY_ID(logic_port);
    scl_entry.mode     = 1;
    CTC_ERROR_RETURN(ctcs_scl_add_entry(lchip, CTC_GINT_UNI_SCL_GROUP, &scl_entry));
    p_hash_node->dsl_entry_id = scl_entry.entry_id;

    /*SID -> TCI*/
    sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
    sal_memset(&udf_mask, 0, sizeof(ctc_acl_udf_t));
    _ctc_gint_map_sid_to_user_data(p_cfg->sid, (uint32*)udf_data.udf);
    _ctc_gint_map_sid_to_user_data(CTC_GINT_SID_MAX, (uint32*)udf_mask.udf);

    /*key: udf data*/
    sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
    udf_data.udf_id = CTC_GINT_UNI_ACL_UDF_ENTRY_ID;
    scl_key_field.type = CTC_FIELD_KEY_UDF;
    scl_key_field.ext_data = &udf_data;
    scl_key_field.ext_mask = &udf_mask;
    CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, p_hash_node->dsl_entry_id, &scl_key_field));

    /*action: strip gint-header*/
    sal_memset(&scl_decap, 0, sizeof(ctc_scl_force_decap_t));
    scl_decap.force_decap_en  =  0;
    scl_decap.offset_base_type = CTC_SCL_OFFSET_BASE_TYPE_L2;
    scl_decap.ext_offset = 2;
    sal_memset(&scl_action_field, 0, sizeof(ctc_scl_field_action_t));
    scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_FORCE_DECAP;
    scl_action_field.ext_data = (void*)&scl_decap;
    CTC_ERROR_RETURN (ctcs_scl_add_action_field(lchip, p_hash_node->dsl_entry_id, &scl_action_field));

    /*action: logic port*/
    sal_memset(&scl_action_field, 0, sizeof(ctc_scl_field_action_t));
    sal_memset(&scl_logic_port, 0, sizeof(ctc_scl_logic_port_t));  
    scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
    scl_logic_port.logic_port = p_cfg->logic_port;
    scl_action_field.ext_data = (void*)&scl_logic_port;
    CTC_ERROR_RETURN (ctcs_scl_add_action_field(lchip, p_hash_node->dsl_entry_id, &scl_action_field));

    /*action: iloop-nh*/
    sal_memset(&scl_action_field, 0, sizeof(ctc_scl_field_action_t));
    scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_REDIRECT;
    scl_action_field.data0 = CTC_GINT_IOOP_NHID;
    CTC_ERROR_RETURN (ctcs_scl_add_action_field(lchip,  p_hash_node->dsl_entry_id, &scl_action_field));

    CTC_ERROR_RETURN (ctcs_scl_install_entry(lchip, scl_entry.entry_id));

   
    /*S21:create nh,encap gint header +  dest to gport*/
    p_hash_node->nh_id = CTC_GINT_ENCODE_NH_ID(p_cfg->logic_port);
    sal_memset(&nh_param, 0 , sizeof(ctc_ip_tunnel_nh_param_t));
    nh_param.upd_type = CTC_NH_UPD_FWD_ATTR;
    nh_param.oif.gport = p_hash_node->gport;
    nh_param.oif.vid = 0;
    nh_param.oif.is_l2_port = 1;
    nh_param.tunnel_info.logic_port = p_cfg->logic_port;/*for check:source logicport == dest logicport*/
    CTC_SET_FLAG(nh_param.tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_HOVERLAY);
    sal_memcpy(nh_param.mac ,mac_da, 6);/*no meanings*/
    sal_memcpy(nh_param.mac ,mac_sa, 6);/*no meanings*/
    nh_param.tunnel_info.udf_profile_id = CTC_GINT_UNI_NH_UDF_PROFILE_ID;
    
    sal_memcpy(((uint8*)nh_param.tunnel_info.udf_replace_data) + 1, udf_data.udf, 1);/*gint header length*/
    sal_memcpy(nh_param.tunnel_info.udf_replace_data, ((uint8*)udf_data.udf)+1, 1);/*gint header length*/
    ((uint8*)nh_param.tunnel_info.udf_replace_data)[1] |= 0xE0;/*add SOF EOF etc*/
    CTC_ERROR_RETURN(ctcs_nh_add_ip_tunnel(lchip, p_hash_node->nh_id, &nh_param));
    
    /*bind logic port and nh*/
    CTC_ERROR_RETURN(ctcs_l2_set_nhid_by_logic_port(lchip, p_cfg->logic_port, p_hash_node->nh_id));

    /*enable logicporttype for dslid isolation*/
    CTC_ERROR_RETURN(sys_usw_port_set_logic_port_type_en(lchip, p_hash_node->gport, 1));

    /*add logic port to default fid */
    /*
    CTC_ERROR_RETURN(_ctc_gint_set_logic_port_default_fid(lchip, p_hash_node->nh_id, 1));
    */

    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_cfg->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

    /*mac limit:scl key logicport, action:none.  later can update action to denylearning*/
    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    scl_entry.key_type = CTC_SCL_KEY_TCAM_MAC;
    scl_entry.action_type = CTC_SCL_ACTION_FLOW;
    scl_entry.entry_id = CTC_GINT_ENCODE_LIMIT_ENTRY_ID(logic_port);
    scl_entry.mode     = 1;
    CTC_ERROR_RETURN(ctcs_scl_add_entry(lchip, CTC_GINT_UNI_SCL_GROUP, &scl_entry));
    p_hash_node->limit_entry_id = scl_entry.entry_id;

    /*key: logic port*/
    sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
    sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));  
    sal_memset(&scl_key_field, 0, sizeof(ctc_field_key_t));
    port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
    port_data.logic_port = p_cfg->logic_port;
    port_mask.logic_port = 0xFFFF;

    scl_key_field.type = CTC_FIELD_KEY_PORT;
    scl_key_field.ext_data = &port_data;
    scl_key_field.ext_mask = &port_mask;
    CTC_ERROR_RETURN(ctcs_scl_add_key_field(lchip, p_hash_node->limit_entry_id, &scl_key_field));
    CTC_ERROR_RETURN (ctcs_scl_install_entry(lchip,  p_hash_node->limit_entry_id));

    /*update mac-limit*/
    if ((p_hash_node->mac_limit.enable == 1) && (p_hash_node->mac_limit.limit_cnt >= p_hash_node->mac_limit.limit_num))
    {
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_hash_node, NULL, 0));
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_hash_node, NULL, 1));
    }
    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_cfg->gport, CTC_PORT_PROP_LEARNING_EN, FALSE));

    /*EGS-Policer:port enable acl*/
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));   
    acl_prop.flag = CTC_ACL_PROP_FLAG_USE_LOGIC_PORT;   
    acl_prop.acl_en = 1;            
    acl_prop.acl_priority = 0;      
    acl_prop.direction = CTC_EGRESS;         
    (ctcs_port_set_acl_property(lchip, p_cfg->gport, &acl_prop));/*no need error return*/

    /*Storm control:port enable acl*/
    sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
    acl_prop.flag = CTC_ACL_PROP_FLAG_USE_LOGIC_PORT;              
    acl_prop.acl_en = 1;            
    acl_prop.acl_priority =  CTC_GINT_ACL_STROM_RESTRICT_PROI;      
    acl_prop.direction = CTC_INGRESS;         
    acl_prop.tcam_lkup_type =  CTC_ACL_TCAM_LKUP_TYPE_L2;    
    acl_prop.class_id = 0;          
    acl_prop.hash_lkup_type = 0;    
    acl_prop.hash_field_sel_id = 0; 
    CTC_ERROR_RETURN(ctcs_port_set_acl_property(lchip,  p_cfg->gport, &acl_prop));  
    acl_prop.acl_priority = CTC_GINT_ACL_STROM_RESTRICT_PROI + 1;             
    acl_prop.tcam_lkup_type =   CTC_ACL_TCAM_LKUP_TYPE_FORWARD;    
    CTC_ERROR_RETURN(ctcs_port_set_acl_property(lchip,  p_cfg->gport, &acl_prop));
    
    return CTC_E_NONE ;    
}

int32
ctcs_gint_destroy_uni(uint8 lchip, ctc_gint_uni_t* p_cfg)
{
    int32 ret = 0;
    ctc_gint_dsl_node_t* p_hash_node = NULL; 
    uint32 gport_buf = 0;
    ctc_port_scl_property_t scl_prop;
    ctc_gint_upd_t upd_cfg;
    ctc_acl_property_t acl_prop;

    CTC_GINT_DBG_FUNC();        
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);
    
    /*hash lookup*/
    p_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if (NULL == p_hash_node)
    {
        return CTC_E_NOT_EXIST;
    }

    /*destroy dsl must remove all link service*/
    if (p_hash_node->service_num)
    {
        return CTC_E_INVALID_CONFIG;
    }

    /*remove all stats/policer relation*/
    sal_memset(&upd_cfg, 0, sizeof(ctc_gint_upd_t));
    upd_cfg.dsl_id = p_cfg->dsl_id;
    upd_cfg.flag |= CTC_GINT_UPD_ACTION_IGS_POLICER;
    upd_cfg.flag |= CTC_GINT_UPD_ACTION_IGS_STATS;
    upd_cfg.flag |= CTC_GINT_UPD_ACTION_EGS_POLICER;
    upd_cfg.flag |= CTC_GINT_UPD_ACTION_EGS_STATS;
    (ctcs_gint_update(lchip, &upd_cfg));

    gport_buf = p_hash_node->gport;
    /*remove logic port from default fid */
    (_ctc_gint_set_logic_port_default_fid(lchip, p_hash_node->nh_id, 0));
    
    /*unbind logic port and nh*/
    (ctcs_l2_set_nhid_by_logic_port(lchip, p_hash_node->logic_port, 0));
 
    /*remove encap nh*/
    (ctcs_nh_remove_ip_tunnel(lchip, p_hash_node->nh_id));

    /*remove scl entry*/
    (ctcs_scl_uninstall_entry(lchip, p_hash_node->dsl_entry_id));
    (ctcs_scl_remove_entry(lchip, p_hash_node->dsl_entry_id));

    /*remove mac limit entry*/
    (ctcs_scl_uninstall_entry(lchip, p_hash_node->limit_entry_id));
    (ctcs_scl_remove_entry(lchip, p_hash_node->limit_entry_id));

    /*free opf logicport*/
    (_ctc_gint_set_logic_port(lchip, &(p_hash_node->logic_port), 0));
    
    /*remove hash node*/
    if (0 == p_gint_master[lchip]->gint_deinit)
    {
        ctc_hash_remove(p_gint_master[lchip]->p_dsl_hash, p_cfg);
        if (p_hash_node)
        {
            mem_free(p_hash_node);
        }
    }
    
    /*disable gport property, 1 gport face N dsl_id*/
    if (0 == _ctc_gint_uni_gport_check(lchip, gport_buf))
    {
        /*Storm control:port disable acl*/
        sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
        acl_prop.flag = CTC_ACL_PROP_FLAG_USE_LOGIC_PORT;              
        acl_prop.acl_en = 0;            
        acl_prop.acl_priority =  CTC_GINT_ACL_STROM_RESTRICT_PROI;      
        acl_prop.direction = CTC_INGRESS;         
        acl_prop.tcam_lkup_type =  CTC_ACL_TCAM_LKUP_TYPE_L2;    
        acl_prop.class_id = 0;          
        acl_prop.hash_lkup_type = 0;    
        acl_prop.hash_field_sel_id = 0; 
        CTC_ERROR_RETURN(ctcs_port_set_acl_property(lchip, gport_buf, &acl_prop));           
        acl_prop.acl_priority = CTC_GINT_ACL_STROM_RESTRICT_PROI + 1;            
        acl_prop.tcam_lkup_type =   CTC_ACL_TCAM_LKUP_TYPE_FORWARD;    
        CTC_ERROR_RETURN(ctcs_port_set_acl_property(lchip, gport_buf, &acl_prop));
    
        /*EGS-Policer:port disable acl*/
        sal_memset(&acl_prop, 0, sizeof(ctc_acl_property_t));
        acl_prop.flag = CTC_ACL_PROP_FLAG_USE_LOGIC_PORT;   
        acl_prop.acl_en = 0;            
        acl_prop.acl_priority = 0;      
        acl_prop.direction = CTC_EGRESS;         
        (ctcs_port_set_acl_property(lchip, gport_buf, &acl_prop));
    
        sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
        scl_prop.scl_id = CTC_GINT_UNI_SCL_ID;
        scl_prop.direction = CTC_INGRESS;
        scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
        scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        (ctcs_port_set_scl_property(lchip, gport_buf, &scl_prop));

        (sys_usw_port_set_logic_port_type_en(lchip, gport_buf, 0));
    }
    
    (ctcs_port_set_property(lchip, gport_buf, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
    (ctcs_port_set_property(lchip, gport_buf, CTC_PORT_PROP_LEARNING_EN, TRUE));
    return ret;    
}

int32
ctcs_gint_create_nni(uint8 lchip, ctc_gint_nni_t* p_cfg)
{
    uint16 logic_port = 0;
    uint32 nh_id = 0;
    ctc_vlan_edit_nh_param_t nh_param;

    CTC_GINT_DBG_FUNC();    
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if (CTC_MAP_GPORT_TO_LPORT(p_cfg->gport) >= 64)
    {
        return CTC_E_INVALID_PARAM; 
    }
    if (p_gint_master[lchip]->nni_logic_port)
    {
        return CTC_E_EXIST;
    }
    if (0 != _ctc_gint_uni_gport_check(lchip, p_cfg->gport))
    {
        return CTC_E_IN_USE;
    }
    /*alloc logic port*/
    CTC_ERROR_RETURN(_ctc_gint_set_logic_port(lchip, &logic_port, 1)); 
    p_cfg->logic_port = logic_port;
    p_gint_master[lchip]->nni_logic_port = logic_port;
    p_gint_master[lchip]->nni_gport = p_cfg->gport;

    /*create xlate nexthop*/
    nh_id = CTC_GINT_ENCODE_NH_ID(logic_port);
    sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    nh_param.gport_or_aps_bridge_id = p_cfg->gport;
    nh_param.logic_port             = logic_port;
    nh_param.logic_port_check       = 1;
    CTC_ERROR_RETURN(ctcs_nh_add_xlate(lchip, nh_id, &nh_param));
    
    /*bind nh and logic port*/
    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_cfg->gport, CTC_PORT_PROP_LOGIC_PORT, p_cfg->logic_port));
    CTC_ERROR_RETURN(ctcs_l2_set_nhid_by_logic_port(lchip, p_cfg->logic_port, nh_id));

    /*nni set default fid*/
    /*
    CTC_ERROR_RETURN(_ctc_gint_set_gport_default_fid(lchip, p_cfg->gport, CTC_GINT_ENCODE_SCL_ENTRY_ID(logic_port), 1));
    */

    /*add logic port to default fid */
    /*
    CTC_ERROR_RETURN(_ctc_gint_set_logic_port_default_fid(lchip, nh_id, 1));
    */

    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_gint_master[lchip]->nni_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));
    return CTC_E_NONE;    
}

int32
ctcs_gint_destroy_nni(uint8 lchip, ctc_gint_nni_t* p_cfg)
{ 
    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if (p_gint_master[lchip]->nni_gport != p_cfg->gport)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (0 == p_gint_master[lchip]->nni_logic_port)
    {
        return CTC_E_NOT_EXIST;
    }

    /*remove logic port from default fid */
    (_ctc_gint_set_logic_port_default_fid(lchip, CTC_GINT_ENCODE_NH_ID(p_gint_master[lchip]->nni_logic_port), 0));
        
    /*nni set default fid*/
    (_ctc_gint_set_gport_default_fid(lchip, p_cfg->gport, CTC_GINT_ENCODE_SCL_ENTRY_ID(p_gint_master[lchip]->nni_logic_port), 0));
   
    /*unbind nh and logic port*/
    (ctcs_l2_set_nhid_by_logic_port(lchip, p_gint_master[lchip]->nni_logic_port, 0));
    (ctcs_port_set_property(lchip, p_cfg->gport, CTC_PORT_PROP_LOGIC_PORT, 0));

    /*destroy xlate nexthop*/
     (ctcs_nh_remove_xlate(lchip, CTC_GINT_ENCODE_NH_ID(p_gint_master[lchip]->nni_logic_port)));
    
    /*free opf logicport*/
    (_ctc_gint_set_logic_port(lchip, &(p_gint_master[lchip]->nni_logic_port), 0));

    (ctcs_port_set_property(lchip, p_gint_master[lchip]->nni_gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
    p_gint_master[lchip]->nni_gport = 0;
    p_gint_master[lchip]->nni_logic_port = 0;

    return CTC_E_NONE;    
}

int32
ctcs_gint_add_service(uint8 lchip, ctc_gint_service_t* p_cfg)
{
    uint16 service_logic_port = 0;
    uint16 dsl_logic_port = 0;
    uint32 dsl_nh = 0;
    ctc_gint_service_node_t* p_service_hash_node = NULL;
    ctc_gint_dsl_node_t*     p_dsl_hash_node = NULL; 
    ctc_vlan_mapping_t vlan_mapping;
    ctc_vlan_edit_nh_param_t nh_param;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if (NULL != ctc_hash_lookup(p_gint_master[lchip]->p_service_hash, p_cfg))
    {
        return CTC_E_EXIST;
    }

    CTC_GINT_PORT_VLAN_ID_CHECK(p_cfg->svlan);
    CTC_GINT_PORT_VLAN_ID_CHECK(p_cfg->cvlan);
    
    /*get dsl logic port*/
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if (NULL == p_dsl_hash_node)
    {
        return CTC_E_NOT_EXIST;
    }
    dsl_logic_port = p_dsl_hash_node->logic_port;
    dsl_nh = p_dsl_hash_node->nh_id;
    
    /*S11:alloc logic port*/
    CTC_ERROR_RETURN(_ctc_gint_set_logic_port(lchip, &service_logic_port, 1)); 
    p_cfg->logic_port = service_logic_port;

    /*S12:HASH process*/
    /*insert node*/
    p_service_hash_node = (ctc_gint_service_node_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_gint_service_node_t));
    if (NULL == p_service_hash_node)
    {
        _ctc_gint_set_logic_port(lchip, &service_logic_port, 0);
        return CTC_E_NO_MEMORY;
    }
    
    sal_memset(p_service_hash_node, 0, sizeof(ctc_gint_service_node_t));
    p_service_hash_node->dsl_id      =  p_cfg->dsl_id;
    p_service_hash_node->svlan       =  p_cfg->svlan;
    p_service_hash_node->cvlan       =  p_cfg->cvlan;
    p_service_hash_node->logic_port  =  p_cfg->logic_port;
    sal_memcpy(&(p_service_hash_node->igs_vlan_action), &(p_cfg->igs_vlan_action), sizeof(ctc_gint_vlan_action_t));
    sal_memcpy(&(p_service_hash_node->egs_vlan_action), &(p_cfg->egs_vlan_action), sizeof(ctc_gint_vlan_action_t));

    if (NULL == ctc_hash_insert(p_gint_master[lchip]->p_service_hash, p_service_hash_node))
    {
        if (p_service_hash_node)
        {
            mem_free(p_service_hash_node);
        }
        return CTC_E_NO_MEMORY;
    }
    
    /*scl key logicport+ dvid, action:logic port vlan edit*/
    /*key*/
    sal_memset(&vlan_mapping, 0, sizeof(vlan_mapping));
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.old_svid = p_service_hash_node->svlan;
    vlan_mapping.old_cvid = p_service_hash_node->cvlan;
    vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    vlan_mapping.scl_id = CTC_GINT_SERVICE_SCL_ID;

    /*action-logic_port*/    
    CTC_SET_FLAG(vlan_mapping.action, CTC_VLAN_MAPPING_OUTPUT_LOGIC_SRC_PORT);
    vlan_mapping.logic_src_port = p_service_hash_node->logic_port;
    /*action-vlan_edit*/
    CTC_ERROR_RETURN (_ctc_gint_service_vlan_mapping(lchip, p_cfg, &vlan_mapping));
    
    /*install*/
    if (CTC_E_HASH_CONFLICT == ctcs_vlan_add_vlan_mapping(lchip, dsl_logic_port, &vlan_mapping))
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
        CTC_ERROR_RETURN(ctcs_vlan_add_vlan_mapping(lchip, dsl_logic_port , &vlan_mapping));
        p_service_hash_node->vlan_mapping_use_flex = 1;
    }
    

    /*S21:create xlate-nh : action: vlan edit +loop-nh*/
    p_service_hash_node->nh_id = CTC_GINT_ENCODE_NH_ID(p_service_hash_node->logic_port);
    sal_memset(&nh_param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    CTC_ERROR_RETURN(_ctc_gint_service_nh_xlate_mapping(lchip, p_cfg, &nh_param.vlan_edit_info));
    nh_param.logic_port             = p_service_hash_node->logic_port;
    nh_param.logic_port_check       = 1;
    nh_param.gport_or_aps_bridge_id = p_dsl_hash_node->gport;
    nh_param.vlan_edit_info.loop_nhid = dsl_nh;
    CTC_ERROR_RETURN (ctcs_nh_add_xlate(lchip, p_service_hash_node->nh_id, &nh_param));


    /*bind logic port and nh*/
    CTC_ERROR_RETURN (ctcs_l2_set_nhid_by_logic_port(lchip, p_cfg->logic_port, p_service_hash_node->nh_id));

    
    /*remove dsl port from default fid */
    /*
    if (0 == p_dsl_hash_node->service_num)
    {
        CTC_ERROR_RETURN (_ctc_gint_set_logic_port_default_fid(lchip, p_dsl_hash_node->nh_id, 0));
    }
    */
    
    /*add service port to default fid */
    /*
    CTC_ERROR_RETURN (_ctc_gint_set_logic_port_default_fid(lchip, p_service_hash_node->nh_id, 1));
    */


    /*update mac-limit*/
    if ((p_dsl_hash_node->mac_limit.enable == 1) && (p_dsl_hash_node->mac_limit.limit_cnt >= p_dsl_hash_node->mac_limit.limit_num))
    {
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_hash_node, NULL, 0));
        CTC_ERROR_RETURN(_ctc_gint_update_logic_port_learning(lchip, p_dsl_hash_node, NULL, 1));
    }
    
    p_dsl_hash_node->service_num ++;
    
    return CTC_E_NONE;    
}

int32
ctcs_gint_remove_service(uint8 lchip, ctc_gint_service_t* p_cfg)
{
    ctc_gint_service_node_t* p_service_hash_node = NULL; 
    ctc_gint_dsl_node_t* p_dsl_hash_node = NULL;
    ctc_vlan_mapping_t vlan_mapping;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);
    
    /*hash lookup*/
    p_service_hash_node = (ctc_gint_service_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_service_hash, p_cfg);
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if ((NULL == p_service_hash_node) || (NULL == p_dsl_hash_node))
    {
        return CTC_E_NOT_EXIST;
    }

    /*remove service port from default fid */
    (_ctc_gint_set_logic_port_default_fid(lchip, p_service_hash_node->nh_id, 0));

    /*add dsl port to default fid */
    if (1 == p_dsl_hash_node->service_num)
    {
        (_ctc_gint_set_logic_port_default_fid(lchip, p_dsl_hash_node->nh_id, 1));
    }
 
    /*unbind logic port and nh*/
    (ctcs_l2_set_nhid_by_logic_port(lchip, p_service_hash_node->logic_port, 0));

    /*remove service nh*/
    (ctcs_nh_remove_xlate(lchip, p_service_hash_node->nh_id));
   
    /*remove scl entry*/
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_SVID);
    CTC_SET_FLAG(vlan_mapping.key, CTC_VLAN_MAPPING_KEY_CVID);
    vlan_mapping.flag = CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT;
    vlan_mapping.scl_id = CTC_GINT_SERVICE_SCL_ID;
    vlan_mapping.old_svid = p_cfg->svlan;
    vlan_mapping.old_cvid = p_cfg->cvlan;
    if (p_service_hash_node->vlan_mapping_use_flex)
    {
        CTC_SET_FLAG(vlan_mapping.flag, CTC_VLAN_MAPPING_FLAG_USE_FLEX);
    }
    (ctcs_vlan_remove_vlan_mapping(lchip, p_dsl_hash_node->logic_port, &vlan_mapping));

    /*don't disable port scl lookup, 1 gport face N dsl_id*/

    /*free opf logicport*/
    (_ctc_gint_set_logic_port(lchip, &(p_service_hash_node->logic_port), 0));

    /*remove hash node*/ 
    if (0 == p_gint_master[lchip]->gint_deinit)
    {
        ctc_hash_remove(p_gint_master[lchip]->p_service_hash, p_cfg);
        if (p_service_hash_node)
        {
            mem_free(p_service_hash_node);
        }
    }
    p_dsl_hash_node->service_num --;
    
    return CTC_E_NONE;   
}

int32
ctcs_gint_update(uint8 lchip, ctc_gint_upd_t* p_cfg)
{
    ctc_gint_dsl_node_t* p_dsl_hash_node = NULL; 
    ctc_scl_field_action_t field_action;
    ctc_ip_tunnel_nh_param_t nh_param;
    ctc_acl_udf_t udf_data;
    ctc_acl_entry_t acl_entry;
    ctc_field_key_t acl_key_field;
    ctc_acl_field_action_t acl_action_field;
    ctc_field_port_t port_data;
    ctc_field_port_t port_mask;
    mac_addr_t mac_sa = {0xab,0xcd,0x12,0x34,0x56};
    mac_addr_t mac_da = {0x12,0x34,0x56,0xab,0xcd};

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    if ((p_cfg->igs_policer_id > 0x7fff)
        || (p_cfg->egs_policer_id > 0x7fff))
    {
        return CTC_E_INVALID_PARAM;
    }
    /*hash lookup*/
    p_dsl_hash_node = (ctc_gint_dsl_node_t*)ctc_hash_lookup(p_gint_master[lchip]->p_dsl_hash, p_cfg);
    if (NULL == p_dsl_hash_node)
    {
        return CTC_E_NOT_EXIST;
    }

    /*update dsl scl entry*/
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    if ((p_cfg->flag & CTC_GINT_UPD_ACTION_IGS_POLICER) && ((p_cfg->igs_policer_id != p_dsl_hash_node->igs_policer_id) && (p_cfg->igs_policer_id  <= 0x7fff)))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
        field_action.data0 = p_cfg->igs_policer_id;
        field_action.data1 = 1;        
        if (p_cfg->igs_policer_id)
        {
            CTC_ERROR_RETURN(ctcs_scl_add_action_field(lchip, p_dsl_hash_node->dsl_entry_id, &field_action));
            p_dsl_hash_node->igs_policer_id =  p_cfg->igs_policer_id;
        }
        else
        {
            CTC_ERROR_RETURN(ctcs_scl_remove_action_field(lchip, p_dsl_hash_node->dsl_entry_id, &field_action));
            p_dsl_hash_node->igs_policer_id =  0;
        }
    }
        
    sal_memset(&field_action, 0, sizeof(ctc_scl_field_action_t));
    if ((p_cfg->flag & CTC_GINT_UPD_ACTION_IGS_STATS) && (p_cfg->igs_stats_id != p_dsl_hash_node->igs_stats_id))
    {
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
        field_action.data0 = p_cfg->igs_stats_id;      
        if (p_cfg->igs_stats_id)
        {
            CTC_ERROR_RETURN(ctcs_scl_add_action_field(lchip, p_dsl_hash_node->dsl_entry_id, &field_action));
            p_dsl_hash_node->igs_stats_id =  p_cfg->igs_stats_id;
        }
        else
        {
            CTC_ERROR_RETURN(ctcs_scl_remove_action_field(lchip, p_dsl_hash_node->dsl_entry_id, &field_action));
            p_dsl_hash_node->igs_stats_id =  0;
        }
    }
        

    /*update dsl nh*/
    sal_memset(&nh_param, 0 , sizeof(ctc_ip_tunnel_nh_param_t));
    if ((p_cfg->flag & CTC_GINT_UPD_ACTION_EGS_POLICER) && ((p_cfg->egs_policer_id != p_dsl_hash_node->egs_policer_id) && (p_cfg->egs_policer_id  <= 0x7fff)))
    {
        if (p_dsl_hash_node->egs_policer_id)
        {
            if (p_cfg->egs_policer_id)/*update*/
            {
                /*action: policer*/
                sal_memset(&acl_action_field, 0, sizeof(ctc_acl_field_action_t));
                acl_action_field.type =  CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
                acl_action_field.data0 = p_cfg->egs_policer_id;
                CTC_ERROR_RETURN(ctcs_acl_add_action_field(lchip, CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX), &acl_action_field)); 
            }
            else/*remove*/
            {
                CTC_ERROR_RETURN(ctcs_acl_uninstall_entry(lchip, CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX)));
                /*remove acl entry*/
                sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
                acl_entry.key_type          = CTC_ACL_KEY_MAC;            
                acl_entry.entry_id          = CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX);            
                acl_entry.mode              = 1; 
                CTC_ERROR_RETURN(ctcs_acl_remove_entry(lchip, CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX)));
                
            }
        }
        else/*create*/
        {
            /*create entry*/
            sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
            acl_entry.key_type          = CTC_ACL_KEY_MAC;            
            acl_entry.entry_id          = CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX);            
            acl_entry.mode              = 1; 
            CTC_ERROR_RETURN(ctcs_acl_add_entry(lchip, CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER, &acl_entry));       

            /*key: logic port*/
            sal_memset(&port_data, 0, sizeof(ctc_field_port_t));
            sal_memset(&port_mask, 0, sizeof(ctc_field_port_t));  
            sal_memset(&acl_key_field, 0, sizeof(ctc_field_key_t));
            port_data.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            port_data.logic_port = p_dsl_hash_node->logic_port;
            port_mask.logic_port = 0xFFFF;

            acl_key_field.type = CTC_FIELD_KEY_PORT;
            acl_key_field.ext_data = &port_data;
            acl_key_field.ext_mask = &port_mask;
            CTC_ERROR_RETURN(ctcs_acl_add_key_field(lchip,  CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX), &acl_key_field));

            /*action: policer*/
            sal_memset(&acl_action_field, 0, sizeof(ctc_acl_field_action_t));
            acl_action_field.type =  CTC_ACL_FIELD_ACTION_MICRO_FLOW_POLICER;
            acl_action_field.data0 = p_cfg->egs_policer_id;
            CTC_ERROR_RETURN(ctcs_acl_add_action_field(lchip, CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX), &acl_action_field));   
            CTC_ERROR_RETURN(ctcs_acl_install_entry(lchip, CTC_GINT_ENCODE_ACL_ENTRY_ID(p_dsl_hash_node->logic_port, CTC_GINT_STORM_RESTRICT_TYPE_MAX)));
        }

        p_dsl_hash_node->egs_policer_id = p_cfg->egs_policer_id;
    }

    sal_memset(&nh_param, 0 , sizeof(ctc_ip_tunnel_nh_param_t));
    if ((p_cfg->flag & CTC_GINT_UPD_ACTION_EGS_STATS) && (p_cfg->egs_stats_id != p_dsl_hash_node->egs_stats_id)) 
    {     
        /*SID -> TCI*/
        sal_memset(&udf_data, 0, sizeof(ctc_acl_udf_t));
        _ctc_gint_map_sid_to_user_data(p_dsl_hash_node->sid, (uint32*)udf_data.udf);

        /*update stats-id to nh*/
        sal_memset(&nh_param, 0 , sizeof(ctc_ip_tunnel_nh_param_t));
        nh_param.upd_type = CTC_NH_UPD_FWD_ATTR;
        nh_param.oif.gport = p_dsl_hash_node->gport;
        nh_param.oif.vid = 0;
        nh_param.oif.is_l2_port = 1;
        nh_param.tunnel_info.logic_port = p_dsl_hash_node->logic_port;/*for check:source logicport == dest logicport*/
        CTC_SET_FLAG(nh_param.tunnel_info.flag, CTC_IP_NH_TUNNEL_FLAG_OVERLAY_HOVERLAY);
        sal_memcpy(nh_param.mac ,mac_da, 6);/*no meanings*/
        sal_memcpy(nh_param.mac ,mac_sa, 6);/*no meanings*/
        nh_param.tunnel_info.stats_id = p_cfg->egs_stats_id;
        nh_param.tunnel_info.udf_profile_id = CTC_GINT_UNI_NH_UDF_PROFILE_ID;

        sal_memcpy(((uint8*)nh_param.tunnel_info.udf_replace_data) + 1, udf_data.udf, 1);/*gint header length*/
        sal_memcpy(nh_param.tunnel_info.udf_replace_data, ((uint8*)udf_data.udf)+1, 1);/*gint header length*/
        ((uint8*)nh_param.tunnel_info.udf_replace_data)[1] |= 0xE0;/*add SOF EOF etc*/

        CTC_ERROR_RETURN(ctcs_nh_update_ip_tunnel(lchip, p_dsl_hash_node->nh_id, &nh_param));

        p_dsl_hash_node->egs_stats_id =  p_cfg->egs_stats_id;
    }
    
    
    return CTC_E_NONE;    
}

int32
ctcs_gint_init(uint8 lchip)
{
    ctc_scl_group_info_t group_info;
    ctc_acl_group_info_t acl_group_info;
    uint32   iloop_port = 0;
    ctc_loopback_nexthop_param_t nh_param = {0};
    ctc_nh_udf_profile_param_t   udf_nh;
    uint8 buf[2]  = {0xff,0xff};
    /*
    ctc_l2dflt_addr_t l2dflt_addr;
    */
    ctc_port_scl_property_t scl_prop;
    int32 value = 0;

    CTC_GINT_DBG_FUNC();
    if (p_gint_master[lchip])
    {
        return CTC_E_EXIST;
    }
    
    p_gint_master[lchip] = (ctc_gint_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_gint_master_t));
    if (!p_gint_master[lchip])
    {
        return CTC_E_NO_MEMORY;    
    }
    sal_memset(p_gint_master[lchip], 0 , sizeof(ctc_gint_master_t));
    /*****hash create*****/
    p_gint_master[lchip]->p_dsl_hash = ctc_hash_create(30,
                                                        50,
                                                        (hash_key_fn)_ctc_gint_dsl_hash_key,
                                                        (hash_cmp_fn)_ctc_gint_dsl_hash_cmp);
    
    p_gint_master[lchip]->p_service_hash = ctc_hash_create(30,
                                                           100,
                                                           (hash_key_fn)_ctc_gint_service_hash_key,
                                                           (hash_cmp_fn)_ctc_gint_service_hash_cmp);
 
   

    /*opf init*/
    CTC_ERROR_RETURN(_ctc_gint_logic_port_init_opf(lchip));
    CTC_ERROR_RETURN(_ctc_gint_policer_id_init_opf(lchip));

    /*internal port alloc :for iloop*/
    CTC_ERROR_RETURN(_ctc_gint_set_iloop_port(lchip, &iloop_port, 1));
    p_gint_master[lchip]->uni_iloop_port = iloop_port;

    /*iloop nh create*/
    nh_param.lpbk_lport = iloop_port;
    CTC_ERROR_RETURN(ctcs_nh_add_iloop(lchip, CTC_GINT_IOOP_NHID, &nh_param)); 

    /*service: port enable scl lookup*/
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    scl_prop.scl_id = CTC_GINT_SERVICE_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN;
    scl_prop.use_logic_port_en = 1;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    CTC_ERROR_RETURN (ctcs_port_set_scl_property(lchip, p_gint_master[lchip]->uni_iloop_port, &scl_prop));

    /*mac limit:scl key logicport, action:none*/
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    scl_prop.scl_id = CTC_GINT_UNI_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    scl_prop.use_logic_port_en = 1;
    CTC_ERROR_RETURN(ctcs_port_set_scl_property(lchip,  p_gint_master[lchip]->uni_iloop_port, &scl_prop));

    /*scl create group for uni-ings scl*/
    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.type = CTC_SCL_GROUP_TYPE_NONE;
    group_info.priority = CTC_GINT_UNI_SCL_ID;
    CTC_ERROR_RETURN(ctcs_scl_create_group(lchip, CTC_GINT_UNI_SCL_GROUP, &group_info));
    /*
    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.type = CTC_SCL_GROUP_TYPE_NONE;
    group_info.priority = CTC_GINT_DEFT_VLAN_SCL_ID;
    CTC_ERROR_RETURN(ctcs_scl_create_group(lchip, CTC_GINT_DEFT_VLAN_SCL_GROUP, &group_info));
    */
    
    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    group_info.type = CTC_SCL_GROUP_TYPE_NONE;
    group_info.priority = CTC_GINT_PORT_ISOLATION_SCL_ID;
    CTC_ERROR_RETURN(ctcs_scl_create_group(lchip, CTC_GINT_PORT_ISOLATION_SCL_GROUP, &group_info));

    /*storm restrict*/
    sal_memset(&acl_group_info, 0, sizeof(ctc_acl_group_info_t));
    acl_group_info.type = CTC_ACL_GROUP_TYPE_NONE;
    acl_group_info.priority = CTC_GINT_ACL_STROM_RESTRICT_PROI;
    CTC_ERROR_RETURN(ctcs_acl_create_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST, &acl_group_info));

    sal_memset(&acl_group_info, 0, sizeof(ctc_acl_group_info_t));
    acl_group_info.type = CTC_ACL_GROUP_TYPE_NONE;
    acl_group_info.priority = CTC_GINT_ACL_STROM_RESTRICT_PROI;
    CTC_ERROR_RETURN(ctcs_acl_create_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_MUTICAST, &acl_group_info));

    sal_memset(&acl_group_info, 0, sizeof(ctc_acl_group_info_t));
    acl_group_info.type = CTC_ACL_GROUP_TYPE_NONE;
    acl_group_info.priority = CTC_GINT_ACL_STROM_RESTRICT_PROI + 1;
    CTC_ERROR_RETURN(ctcs_acl_create_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN, &acl_group_info));

    /*egress policer*/
    sal_memset(&acl_group_info, 0, sizeof(ctc_acl_group_info_t));
    acl_group_info.type = CTC_ACL_GROUP_TYPE_NONE;
    acl_group_info.priority = 0;
    acl_group_info.dir = CTC_EGRESS;
    CTC_ERROR_RETURN(ctcs_acl_create_group(lchip, CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER, &acl_group_info));
    
    /*create nh udf profile*/
    sal_memset(&udf_nh, 0 , sizeof(ctc_nh_udf_profile_param_t));
    udf_nh.profile_id = CTC_GINT_UNI_NH_UDF_PROFILE_ID;
    udf_nh.offset_type = CTC_NH_UDF_OFFSET_TYPE_RAW;
    udf_nh.edit_length = 2;/*gint header length*/
    udf_nh.edit[0].type = 1;
    udf_nh.edit[0].length = 16;
    sal_memcpy(udf_nh.edit_data, buf, 2);
    CTC_ERROR_RETURN(ctcs_nh_add_udf_profile(lchip, &udf_nh));

    /*enable uni acl udf entry portbitmap*/
    CTC_ERROR_RETURN(_ctc_gint_uni_acl_portbitmap_enable(lchip, 1)); 

    /*iloop port set default fid*/
    /*
    CTC_ERROR_RETURN(_ctc_gint_set_gport_default_fid(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_GINT_ILOOP_DEFT_VLAN_ENTRY_ID, 1));
    */

    /*create default fid*/
    /*
    sal_memset(&l2dflt_addr, 0 , sizeof(ctc_l2dflt_addr_t));
    l2dflt_addr.l2mc_grp_id = CTC_GINT_DEFAULT_FID;
    l2dflt_addr.fid         = CTC_GINT_DEFAULT_FID;
    l2dflt_addr.flag = CTC_L2_DFT_VLAN_FLAG_USE_LOGIC_PORT;
    CTC_ERROR_RETURN(ctcs_l2_add_default_entry(lchip, &l2dflt_addr));
    */

    
    /*register interrupt*/
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_L2_SW_LEARNING, _ctc_gint_hw_learning_sync);
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_L2_SW_AGING, _ctc_gint_hw_aging_sync);

    sys_usw_global_set_gint_en(lchip, 1);
    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 0));
    CTC_ERROR_RETURN(ctcs_port_set_property(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

    value = 0;
    CTC_ERROR_RETURN(ctcs_global_ctl_set(lchip, CTC_GLOBAL_DISCARD_MCAST_SA_PKT, &value));

    /*Dslid-Port_isolation:port enable scl*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = CTC_GINT_PORT_ISOLATION_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    (ctcs_port_set_scl_property(lchip, p_gint_master[lchip]->uni_iloop_port, &scl_prop));

    return CTC_E_NONE;   
}

int32
ctcs_gint_deinit(uint8 lchip)
{
    uint32 iloop_port = 0;
    /*
    ctc_l2dflt_addr_t l2dflt_addr;
    */
    ctc_port_scl_property_t scl_prop;
    ctc_gint_nni_t    nni_cfg;
    int32 value = 0;

    CTC_GINT_DBG_FUNC();
    CTC_GINT_INIT_CHECK(lchip);

    if (NULL == p_gint_master[lchip])
    {
        return CTC_E_NOT_EXIST;
    }
    p_gint_master[lchip]->gint_deinit = 1; 
    (ctcs_port_set_property(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP, 1));
    (ctcs_port_set_property(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));

    value = 1;
    (ctcs_global_ctl_set(lchip, CTC_GLOBAL_DISCARD_MCAST_SA_PKT, &value));

    /*iloop nh remove*/
    CTC_ERROR_RETURN(ctcs_nh_remove_iloop(lchip, CTC_GINT_IOOP_NHID)); 

    /*destroy nni*/ 
    if (p_gint_master[lchip]->nni_logic_port)
    {
        nni_cfg.gport = p_gint_master[lchip]->nni_gport;
        ctcs_gint_destroy_nni(lchip, &nni_cfg);
    }
    
    /*dsl/service remove*/
    ctc_hash_traverse(p_gint_master[lchip]->p_service_hash ,
                      (hash_traversal_fn)_ctc_gint_service_node_remove, &lchip);
    ctc_hash_traverse(p_gint_master[lchip]->p_dsl_hash ,
                      (hash_traversal_fn)_ctc_gint_dsl_node_remove, &lchip);

    /*Port-isolation: port disable scl*/
    sal_memset(&scl_prop, 0, sizeof(scl_prop));
    scl_prop.scl_id = CTC_GINT_PORT_ISOLATION_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    (ctcs_port_set_scl_property(lchip, p_gint_master[lchip]->uni_iloop_port, &scl_prop));

    /*service: port disable scl lookup*/
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    scl_prop.scl_id = CTC_GINT_SERVICE_SCL_ID;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
    scl_prop.use_logic_port_en = 1;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
     (ctcs_port_set_scl_property(lchip, p_gint_master[lchip]->uni_iloop_port, &scl_prop));

    /*mac limit:scl key logicport, action:none*/
    sal_memset(&scl_prop, 0, sizeof(ctc_port_scl_property_t));
    scl_prop.scl_id = CTC_GINT_UNI_SCL_ID;
    scl_prop.use_logic_port_en = 1;
    scl_prop.direction = CTC_INGRESS;
    scl_prop.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
    scl_prop.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    (ctcs_port_set_scl_property(lchip,  p_gint_master[lchip]->uni_iloop_port, &scl_prop));
    
    /*destroy default fid*/
    /*
    sal_memset(&l2dflt_addr, 0 , sizeof(ctc_l2dflt_addr_t));
    l2dflt_addr.l2mc_grp_id = CTC_GINT_DEFAULT_FID;
    l2dflt_addr.fid         = CTC_GINT_DEFAULT_FID;
    (ctcs_l2_remove_default_entry(lchip, &l2dflt_addr));
    */
   
    /*iloop port set default fid*/
    /*
    (_ctc_gint_set_gport_default_fid(lchip, p_gint_master[lchip]->uni_iloop_port, CTC_GINT_ILOOP_DEFT_VLAN_ENTRY_ID, 0));
    */

    /*disable uni acl udf entry portbitmap*/
    (_ctc_gint_uni_acl_portbitmap_enable(lchip, 0));
    
    (ctcs_nh_remove_udf_profile(lchip, CTC_GINT_UNI_NH_UDF_PROFILE_ID));

    (ctcs_acl_uninstall_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST));
    (ctcs_acl_uninstall_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_MUTICAST));
    (ctcs_acl_uninstall_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN));
    (ctcs_acl_uninstall_group(lchip, CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER));
    (ctcs_acl_remove_all_entry(lchip, CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER));

    (ctcs_scl_uninstall_group(lchip, CTC_GINT_PORT_ISOLATION_SCL_GROUP));
    (ctcs_scl_uninstall_group(lchip, CTC_GINT_UNI_SCL_GROUP));
    /*
    (ctcs_scl_uninstall_group(lchip, CTC_GINT_DEFT_VLAN_SCL_GROUP));
    */

    (ctcs_acl_destroy_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_BOARDCAST));
    (ctcs_acl_destroy_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_MUTICAST));
    (ctcs_acl_destroy_group(lchip, CTC_GINT_ACL_GRP_ID_STORM_UNKNOWN));
    (ctcs_acl_destroy_group(lchip, CTC_GINT_ACL_GRP_ID_DSL_EGS_POLICER));
    
    (ctcs_scl_destroy_group(lchip, CTC_GINT_PORT_ISOLATION_SCL_GROUP));
    (ctcs_scl_destroy_group(lchip, CTC_GINT_UNI_SCL_GROUP));
    /*
    (ctcs_scl_destroy_group(lchip, CTC_GINT_DEFT_VLAN_SCL_GROUP));
    */

    if (p_gint_master[lchip]->p_dsl_hash)
    {
        ctc_hash_free(p_gint_master[lchip]->p_dsl_hash);
    }
    
    if (p_gint_master[lchip]->p_service_hash)
    {
        ctc_hash_free(p_gint_master[lchip]->p_service_hash);
    }
    
    iloop_port = p_gint_master[lchip]->uni_iloop_port;
    (_ctc_gint_set_iloop_port(lchip, &iloop_port, 0));

    (ctc_opf_deinit(lchip, CTC_OPF_CUSTOM_ID_START));
    (ctc_opf_deinit(lchip, CTC_OPF_CUSTOM_ID_START+1));
    if (p_gint_master[lchip])
    {
        mem_free(p_gint_master[lchip]);
        p_gint_master[lchip] = NULL;
    }

    return CTC_E_NONE;   
}

#endif

