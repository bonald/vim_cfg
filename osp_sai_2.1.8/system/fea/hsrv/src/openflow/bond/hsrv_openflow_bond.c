
#include "hsrv_inc.h"
#include "hsrv_if.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_bond_priv.h"
#include "hsrv_openflow_bond.h"
#include "hsrv_openflow_port.h"

hsrv_openflow_bond_master_t* g_pst_openflow_bond_master = NULL;

#define _____MESSAGE_API_____

static int32
_hsrv_openflow_bond_create_send_msg(uint8 tid, uint16 gport, uint32 l3ifid)
{
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowCreateBondReq_t *p_req = NULL;

    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowCreateBond;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowCreateBond;
    p_req->tid = tid;
    p_req->gport = gport;
    p_req->l3ifid = l3ifid;    
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));

    return HSRV_E_NONE;
}

#define _____INTERNAL_API_____

static inline uint32
_hsrv_openflow_bond_info_hash_make(void *p_info_void)
{
    hsrv_openflow_bond_data_t *p_info = NULL;
    uint32 ifindex;

    p_info = (hsrv_openflow_bond_data_t *)p_info_void;
    ifindex = p_info->ifindex;

    p_info = NULL;

    return ifindex;
}

static inline bool
_hsrv_openflow_bond_info_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_bond_data_t *p_info_in_bucket = NULL;
    hsrv_openflow_bond_data_t *p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_bond_data_t *)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_bond_data_t *)p_info_to_lkp_void;

    if (p_info_in_bucket->ifindex != p_info_to_lkp->ifindex)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

static inline uint32
_hsrv_openflow_slave_hash_make(void *p_info_void)
{
    hsrv_openflow_slave_data_t *p_info = NULL;
    uint32 ifindex;

    p_info = (hsrv_openflow_slave_data_t *)p_info_void;
    ifindex = p_info->ifindex;

    p_info = NULL;

    return ifindex;
}

static inline bool
_hsrv_openflow_slave_hash_compare(void *p_info_in_bucket_void, void *p_info_to_lkp_void)
{
    hsrv_openflow_slave_data_t *p_info_in_bucket = NULL;
    hsrv_openflow_slave_data_t *p_info_to_lkp = NULL;

    p_info_in_bucket = (hsrv_openflow_slave_data_t *)p_info_in_bucket_void;
    p_info_to_lkp = (hsrv_openflow_slave_data_t *)p_info_to_lkp_void;

    if (p_info_in_bucket->ifindex != p_info_to_lkp->ifindex)
    {
        p_info_in_bucket = NULL;
        p_info_to_lkp = NULL;
        return FALSE;
    }

    p_info_in_bucket = NULL;
    p_info_to_lkp = NULL;

    return TRUE;
}

static int32
_hsrv_openflow_bond_get_slave(hsrv_openflow_bond_data_t *p_bond_info, uint32 ifindex, hsrv_openflow_slave_data_t **pp_info)
{
    hsrv_openflow_slave_data_t lkp_info;
    hsrv_openflow_slave_data_t *p_slave_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_slave_data_t));
    lkp_info.ifindex = ifindex;

    p_slave_info = ctclib_hash_lookup(p_bond_info->bond_slave_hash, &lkp_info);
    if (p_slave_info)
    {
        *pp_info = p_slave_info;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_bond_attach_slave_sync (hsrv_openflow_bond_data_t *p_bond_info, uint32 slave_ifindex)
{
    //hsrv_openflow_port_data_t *p_port_data = NULL;
    hsrv_openflow_slave_data_t *p_slave = NULL;

    HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_get_slave(p_bond_info, slave_ifindex, &p_slave));
    if (p_slave)
    {
        return HSRV_E_NONE;
    }

    /* alloc memery */
    p_slave = HSRV_OPENFLOW_CALLOC(sizeof(hsrv_openflow_slave_data_t));
    HSRV_OPENFLOW_MEM_CHECK(p_slave);
    sal_memset(p_slave, 0, sizeof(hsrv_openflow_slave_data_t));
    
    /* init slave node */
    p_slave->enable = FALSE;
    p_slave->ifindex = slave_ifindex;
    ctclib_hash_get(p_bond_info->bond_slave_hash, p_slave, ctclib_hash_alloc_intern);

    /* set port flag */
#ifdef V580_OPENFLOW
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(slave_ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    HSRV_OPENFLOW_PTR_CHECK(p_port_data);    
    GLB_SET_FLAG(p_port_data->flag, IF_PORT_OPENFLOW_FLAG_SLAVE);
    p_port_data->bond_ifindex = p_bond_info->ifindex;
#endif
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_bond_slave_enable(fei_openflow_bond_update_slave_t *pst_req)
{

    hsrv_openflow_cdb_bond_update_slave(pst_req->bond_ifindex, pst_req->slave_ifindex, pst_req->enable);

        /* update physical port property  */
    //HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_update_tunnel(p_bond_info, pst_req->slave_ifindex, TRUE));
    return HSRV_E_NONE;
}

static int32
_hsrv_openflow_bond_slave_disable(fei_openflow_bond_update_slave_t *pst_req)
{
    hsrv_openflow_cdb_bond_update_slave(pst_req->bond_ifindex, pst_req->slave_ifindex, pst_req->enable);
    return HSRV_E_NONE;
}

#define _____MODULE_APIs_____

static int32
hsrv_openflow_bond_create(fei_openflow_bond_create_t *pst_req)
{
    uint8 tid = 0;
    uint16 gport = 0;
    uint32 l3ifid = 0;
    hsrv_openflow_bond_data_t *p_bond_data = NULL;
    //hsrv_openflow_port_data_t *p_port_data = NULL;
    hsrv_openflow_label_t label;

    HSRV_OPENFLOW_PTR_CHECK(pst_req);
        
    tid = HSRV_OPENFLOW_GET_BOND_TID(pst_req->bond_ifindex);
    //gport = HSRV_OPENFLOW_TRANS_TID_TO_GPORT(tid);
    gport = hsrv_ifdb_get_agg_gport_by_tid(tid);

    /* alloc memery */
    p_bond_data = HSRV_OPENFLOW_CALLOC(sizeof(hsrv_openflow_bond_data_t));
    HSRV_OPENFLOW_MEM_CHECK(p_bond_data);
    sal_memset(p_bond_data, 0, sizeof(hsrv_openflow_bond_data_t));

    /* add acl group */
    label.type = GLB_OPENFLOW_LABEL_PORT;
    label.enable = pst_req->enable;
    HSRV_IF_ERROR_RETURN(hsrv_openflow_port_get_label(pst_req->bond_ifindex, &label.label));
    HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_op_label(&label));

    /* Allocated l3ifid through ifdb to reuse existing logic. */
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_new_l3ifid(GLB_IF_TYPE_LINKAGG_IF, &l3ifid));

    /* create bond */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_create_send_msg(tid, gport, l3ifid));

    /* sync bond db */
    p_bond_data->ifindex = pst_req->bond_ifindex;
    p_bond_data->label = label.label;
    p_bond_data->tid = tid;
    p_bond_data->gport = gport;
    p_bond_data->bond_slave_hash = 
        ctclib_hash_create(_hsrv_openflow_slave_hash_make, _hsrv_openflow_slave_hash_compare);

    ctclib_hash_get(HSRV_OPENFLOW_BOND_INFO_HASH, p_bond_data, ctclib_hash_alloc_intern);
#ifdef V580_OPENFLOW
    /* add interface in interface manager */
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_add_agg_if(pst_req->bond_ifindex, tid, NULL));
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_port_pm_data(pst_req->bond_ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data));
    if(NULL == p_port_data)
    {
        p_port_data = HSRV_OPENFLOW_PORT_MALLOC(sizeof(hsrv_openflow_port_data_t));
        HSRV_OPENFLOW_MEM_CHECK(p_port_data);
        sal_memset(p_port_data, 0, sizeof(hsrv_openflow_port_data_t));
        ctclib_list_init(&p_port_data->list);
        p_port_data->enable = TRUE;
        
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_set_port_pm_data(pst_req->bond_ifindex, HSRV_PMID_OPENFLOW, p_port_data));
    }

    p_port_data->l3ifid =l3ifid;
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_port_flex_nh(gport, &p_port_data->nh_info));
#endif
    return HSRV_E_NONE;
}

static int32
hsrv_openflow_bond_remove(fei_openflow_bond_create_t *pst_req)
{
    return hsrv_openflow_cdb_bond_remove(pst_req->bond_ifindex);
}

int32
hsrv_openflow_bond_lookup_info(uint32 ifindex, hsrv_openflow_bond_data_t **pp_info)
{
    hsrv_openflow_bond_data_t lkp_info;
    hsrv_openflow_bond_data_t *p_bond_info = NULL;

    HSRV_OPENFLOW_PTR_CHECK(pp_info);

    sal_memset(&lkp_info, 0, sizeof(hsrv_openflow_bond_data_t));
    lkp_info.ifindex = ifindex;

    p_bond_info = ctclib_hash_lookup(HSRV_OPENFLOW_BOND_INFO_HASH, &lkp_info);
    if (p_bond_info)
    {
        *pp_info = p_bond_info;
        p_bond_info = NULL;
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_bond_get_tid(uint32 bond_ifindex, uint8 *p_tid)
{
    hsrv_openflow_bond_data_t *p_bond_info = NULL;

    HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(bond_ifindex, &p_bond_info));
    if (!p_bond_info)
    {
        return HSRV_E_INVALID_PARAM;
    }

    *p_tid = p_bond_info->tid;

    return HSRV_E_NONE;
}

bool
hsrv_openflow_bond_get_link_status(uint32 bond_ifindex)
{
    bool link_up = FALSE;
    link_up = hsrv_openflow_agg_get_link_status(bond_ifindex);

    return link_up;
}

int32
hsrv_openflow_bond_get_gport_by_ifindex(uint32 ifindex, uint16 *gport)
{
    hsrv_ifdb_get_agg_portid(ifindex, gport);

    return 0;
}

bool
hsrv_openflow_port_is_bond(uint32 bond_ifindex)
{
    return hsrv_openflow_cdb_port_is_bond(bond_ifindex);
}

#define _____HAL_APIs_____

int32 
fei_api_openflow_bond_create(fei_openflow_bond_create_t *pst_req)
{
    if (pst_req->enable)
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_create(pst_req));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_remove(pst_req));
    }
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_bond_update_slave(fei_openflow_bond_update_slave_t *pst_req)
{
    if (pst_req->enable)
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_slave_enable(pst_req));
    }
    else
    {
        HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_slave_disable(pst_req));
    }
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_bond_attach_slave(fei_openflow_bond_attach_slave_t *pst_req)
{
    //uint32 label = 0;
    hsrv_openflow_bond_data_t *p_bond_info = NULL;

    return HSRV_E_NONE;

    HSRV_OPENFLOW_PTR_CHECK(pst_req);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(pst_req->bond_ifindex, &p_bond_info));
    if (!p_bond_info)
    {
        HSRV_OPENFLOW_LOG_ERR("Not find bond port:%d", pst_req->bond_ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /* sync label  */
    //HSRV_IF_ERROR_RETURN(hsrv_openflow_msg_port_set_label(pst_req->slave_ifindex, label));

    /* attach slave just sync db, not add slave to linkagg group  */
    HSRV_IF_ERROR_RETURN(_hsrv_openflow_bond_attach_slave_sync(p_bond_info, pst_req->slave_ifindex));
    
    return HSRV_E_NONE;
}

int32 
fei_api_openflow_bond_set_mac(fei_openflow_bond_set_mac_t *pst_req)
{
    uint16 gport = 0;
    hsrv_openflow_bond_data_t *p_bond_info = NULL;
    Hsrv2AgtMsg_t hsrv_msg;
    HalMsgOpenFlowBondSetMacReq_t *p_req = NULL;
    
    HSRV_OPENFLOW_PTR_CHECK(pst_req);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_bond_lookup_info(pst_req->bond_ifindex, &p_bond_info));
    if (!p_bond_info)
    {
        HSRV_OPENFLOW_LOG_ERR("Not find bond port:%d", pst_req->bond_ifindex);
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_agg_portid(pst_req->bond_ifindex, &gport));
    p_req = &hsrv_msg.opHsrv2Agt.hsrv2AgtMsgOpenFlowBondSetMac;

    sal_memset(&hsrv_msg, 0, sizeof(Hsrv2AgtMsg_t));
    HSRV_AGENTID_SET_ALL(hsrv_msg.agentId);

    hsrv_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgOpenFlowBondSetMac;
    p_req->mac.buf = pst_req->mac;
    p_req->mac.size = sizeof(mac_addr_t);
    p_req->gport = gport;
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hsrv_msg));
    
    return HSRV_E_NONE;
}

#define _____DEBUG________
static void 
_hsrv_openflow_show_slave_brief(ctclib_hash_backet_t *phb, void *user_data)
{
    hsrv_openflow_slave_debug_data_t *data = user_data;
    hsrv_openflow_slave_data_t *p_slave = (hsrv_openflow_slave_data_t *)phb->data;
    FILE *fp = data->fp;

    sal_fprintf(fp, " %7u %10u %6s\n", p_slave->ifindex, p_slave->port_config, p_slave->enable ? "true" : "false");
}

static void 
_hsrv_openflow_show_bond_brief(ctclib_hash_backet_t *phb, void *user_data)
{
    int32 ret = 0;
    hsrv_openflow_port_data_t *p_port_data = NULL;
    hsrv_openflow_slave_debug_data_t data;
    hsrv_openflow_bond_data_t *p_bond = (hsrv_openflow_bond_data_t *)phb->data;
    FILE *fp = user_data;
#ifdef V580_OPENFLOW
    ret = hsrv_ifdb_get_port_pm_data(p_bond->ifindex, HSRV_PMID_OPENFLOW, (void**)&p_port_data);
#endif
    if (HSRV_E_NONE != ret || NULL == p_port_data)
    {
        return;
    }

    sal_fprintf(fp, "---- bond%d ----\n", p_bond->tid);
    sal_fprintf(fp, "tid    : %d\n", p_bond->tid);
    sal_fprintf(fp, "config : %d\n", p_bond->port_config);
    sal_fprintf(fp, "gport  : 0x%x\n", p_bond->gport);
    sal_fprintf(fp, "lable  : %u\n", p_bond->label);
    sal_fprintf(fp, "l3ifid  : %u\n", p_port_data->l3ifid);
    sal_fprintf(fp, "nh_offset  : %u\n", p_bond->nh_info.nh_offset.nhid);
    sal_fprintf(fp, "slave_num  : %u\n", p_bond->bond_slave_hash->count);

    if (0 == p_bond->bond_slave_hash->count)
    {
        sal_fprintf(fp, "\n");
        return ;
    }

    sal_fprintf(fp, " --------------------------\n");
    sal_fprintf(fp, " %7s %10s %6s \n", 
                     "ifindex", "port-config", "enable");
    sal_fprintf(fp, " --------------------------\n");

    data.fp = fp;
    data.p_bond = p_bond;

    ctclib_hash_iterate(p_bond->bond_slave_hash, _hsrv_openflow_show_slave_brief, (void *)&data);

    sal_fprintf(fp, "\n");
}

int32
hsrv_openflow_show_bond_brief(FILE *fp)
{
    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_bond_master);
    
    sal_fprintf(fp, "----------------- HSRV OPENFLOW BOND -----------------\n");

    ctclib_hash_iterate(HSRV_OPENFLOW_BOND_INFO_HASH, _hsrv_openflow_show_bond_brief, fp);

    sal_fprintf(fp, "--------------------------------------------------------\n");

    return HSRV_E_NONE;
}

#define _____INIT_________
int32
hsrv_openflow_bond_start(void)
{
    g_pst_openflow_bond_master = HSRV_OPENFLOW_CALLOC(sizeof(hsrv_openflow_bond_master_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_bond_master);

    HSRV_OPENFLOW_BOND_INFO_HASH = ctclib_hash_create(_hsrv_openflow_bond_info_hash_make,
        _hsrv_openflow_bond_info_hash_compare);
    HSRV_OPENFLOW_MEM_CHECK(HSRV_OPENFLOW_BOND_INFO_HASH);

    return HSRV_E_NONE;
}

int32
hsrv_openflow_bond_init(hsrv_system_spec_info_t *p_system_info)
{
    HSRV_OPENFLOW_PTR_CHECK(g_pst_openflow_bond_master);

    g_pst_openflow_bond_master->bond_label_base = HSRV_OPENFLOW_AGG_PORT_LABEL_BASE;
    
    return HSRV_E_NONE;
}

#define _____END_LINE_____

