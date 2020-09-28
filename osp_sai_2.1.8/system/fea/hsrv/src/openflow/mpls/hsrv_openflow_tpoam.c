#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "hsrv_openflow_tpoam.h"
#include "hsrv_openflow_tpoam_priv.h"

#include "glb_g8131_define.h"
#include "fei_api_openflow.h"
#include "glb_tp_oam.h"
#include "hsrv_debug.h"

#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"

hsrv_openflow_tpoam_t *g_pst_openflow_tpoam = NULL;

extern int32 hsrv_openflow_nexthop_offset_release(hsrv_of_nh_offset_t* p_nh_offset);

static uint32
_hsrv_tpoam_add_meg(uint8* p_megid)
{
    uint8                                              megid[GLB_TPOAM_MEGID_LEN];
    Hsrv2AgtMsg_t                                 hagt_msg;
    glb_tpoam_megid_t*                        pst_megid = NULL;
    glb_tpoam_megid_t                          megid_key;
    int32                                              rc = HSRV_E_NONE;
    void*                                              p_hash_ret = NULL;
    
    sal_memset(megid, 0, GLB_TPOAM_MEGID_LEN);
    sal_memset(&megid_key, 0, sizeof(glb_tpoam_megid_t));
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_megid);

    /* make megid */
    megid[0] = 0x01;
    megid[1] = 0x20;
    megid[2] = 0x0d;
    sal_memcpy(&megid[3], p_megid, GLB_TPOAM_MEGID_STR_LEN);

    /* find megid */
    sal_memcpy(&megid_key.id, megid, GLB_TPOAM_MEGID_LEN);
    pst_megid = ctclib_hash_lookup(g_pst_openflow_tpoam->pst_megid_hash,  (void*)&megid_key);
    if (pst_megid)
    {
        pst_megid->count++;
        return rc;
    }

    /* reserve megid to db */
    pst_megid = HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW, sizeof(glb_tpoam_megid_t));
    HSRV_OPENFLOW_PTR_CHECK(pst_megid);
    
    sal_memcpy(pst_megid->id, megid, GLB_TPOAM_MEGID_LEN);
    pst_megid->count++;
    
    p_hash_ret = ctclib_hash_get(g_pst_openflow_tpoam->pst_megid_hash, (void*)pst_megid, ctclib_hash_alloc_intern);
    if (!p_hash_ret)
    {
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_megid);
        return HSRV_E_NO_MEMORY;
    }

    /* add meg to hsrv */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamAddMegid;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamAddMegid.id.buf = (uint8_t*)pst_megid->id;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamAddMegid.id.size = GLB_TPOAM_MEGID_LEN;
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Add MEGID %d,%d,%d,%s,%s to hagt failure ret is %d", 
                        pst_megid->id[0], pst_megid->id[1], pst_megid->id[2],
                        &pst_megid->id[3], &pst_megid->id[9], rc); 
        
        ctclib_hash_release(g_pst_openflow_tpoam->pst_megid_hash,  (void*)pst_megid);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_megid);
        
        return rc;
    }

    return rc;
}

static uint32
_hsrv_tpoam_del_meg(uint8* p_megid)
{
    Hsrv2AgtMsg_t                                 hagt_msg;
    glb_tpoam_megid_t*                        pst_megid = NULL;
    glb_tpoam_megid_t                          megid_key;
    int32                                              rc = HSRV_E_NONE;
    uint8                                              megid[GLB_TPOAM_MEGID_LEN];
    
    sal_memset(&megid_key, 0, sizeof(glb_tpoam_megid_t));
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(megid, 0, GLB_TPOAM_MEGID_LEN);

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_megid);

    megid[0] = 0x01;
    megid[1] = 0x20;
    megid[2] = 0x0d;
    sal_memcpy(&megid[3], p_megid, GLB_TPOAM_MEGID_STR_LEN);

    /* find megid */
    sal_memcpy(&megid_key.id, megid, GLB_TPOAM_MEGID_LEN);
    pst_megid = ctclib_hash_lookup(g_pst_openflow_tpoam->pst_megid_hash,  (void*)&megid_key);
    if (!pst_megid)
    {
        HSRV_OPENFLOW_LOG_ERR("Megid %s no exist\n", p_megid[3]);
        return rc;
    }

    pst_megid->count--;
    if (0 == pst_megid->count)
    {
        /* del meg to hsrv */
        hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamDelMegid;
        hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamDelMegid.id.buf = (uint8_t*)pst_megid->id;
        hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamDelMegid.id.size = GLB_TPOAM_MEGID_LEN;
        HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
        
        rc = HSRV2AGT_MSG_SEND(&hagt_msg);
        if (rc)
        {
            HSRV_OPENFLOW_LOG_ERR("Del MEGID %d,%d,%d,%s,%s to hagt failure ret is %d", 
                            pst_megid->id[0], pst_megid->id[1], pst_megid->id[2],
                            &pst_megid->id[3], &pst_megid->id[9], rc); 
        }
        
        ctclib_hash_release(g_pst_openflow_tpoam->pst_megid_hash,  (void*)pst_megid);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_megid);    
    }

    return rc;
}

static uint32
_hsrv_tpoam_add_mep(fei_openflow_add_tpoam_req_t*   p_msg_req, uint16 gport, uint32 nh_id, uint32 offset)
{
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;
    HalMsgTpoamAddOamReq_t*             pst_msg = NULL;
    uint32                                             index = 0;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    void*                                              p_hash_ret = NULL;
    uint8                                              megid[GLB_TPOAM_MEGID_LEN];
    ctclib_hash_t*                                 p_lmep_hash = NULL;
    hsrv_openflow_group_t*                    p_group = NULL;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(megid, 0, GLB_TPOAM_MEGID_LEN);

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    if (p_msg_req->flow.aps_group_id)
    {
        hsrv_openflow_groupdb_lookup_group(p_msg_req->flow.aps_group_id, &p_group);
        if (!p_group)
        {
            HSRV_OPENFLOW_LOG_ERR("Aps group id %u does not exist", p_msg_req->flow.aps_group_id);
            return HSRV_E_INVALID_PARAM;
        }
    }
     
    /* reserve mep & rmep */
    /* alloc index */
    index = ctclib_opf_alloc_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2);
    if( CTCLIB_OPF_INVALID_OFFSET == index)
    {
        HSRV_OPENFLOW_LOG_WARN("Not enough MEP offset resource.");
        return HSRV_E_RESOURCE_FULL;
    }

    /* allocate node */
    pst_lmep = HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW, sizeof(glb_openflow_tpoam_t));
    if (!pst_lmep)
    {
        ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        HSRV_OPENFLOW_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); 
        return HSRV_E_NO_MEMORY;
    }

    sal_memcpy(pst_lmep, &p_msg_req->flow, sizeof(glb_openflow_tpoam_t));
    pst_lmep->mepIndex = index;
    pst_lmep->rmepIndex = index + 1;
    pst_lmep->nh_id = nh_id;
    pst_lmep->offset = offset;
    pst_lmep->gport = gport;

    megid[0] = 0x01;
    megid[1] = 0x20;
    megid[2] = 0x0d;
    sal_memcpy(&megid[3], p_msg_req->flow.megid, GLB_TPOAM_MEGID_STR_LEN);
    sal_memcpy(pst_lmep->megid, megid, GLB_TPOAM_MEGID_LEN);

    if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        if (0 == p_msg_req->flow.lsp_outlabel_exp_is_set)
        {
            pst_lmep->lsp_exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
    }
    else if (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type)
    {
         if (0 == p_msg_req->flow.pw_outlabel_exp_is_set)
        {
            pst_lmep->pw_exp = GLB_OPENFLOW_MPLS_DEFAULT_EXP;
        }
    }
    
    pst_lmep->level = p_msg_req->flow.level;
    if (p_group)
    {
        pst_lmep->aps_group_offset = p_group->aps_group.aps_group_offset;

        if (p_msg_req->flow.is_working_path)
        {
            p_group->aps_group.working_mepIndex = index;
        }
        else
        {
            p_group->aps_group.protection_mepIndex = index;    
        }
    }
    
    /* add lmep&rmep node to db */
    p_hash_ret = ctclib_hash_get(g_pst_openflow_tpoam->pst_lmep_hash, 
        (void*)pst_lmep, ctclib_hash_alloc_intern);
    if (!p_hash_ret)
    {
        ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);
        return HSRV_E_NO_MEMORY;
    }
    
    if (GLB_OAM_TYPE_MPLS_SECTION == p_msg_req->flow.oam_type)
    {
        p_lmep_hash = g_pst_openflow_tpoam->pst_section_lmep_hash;
        
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        p_lmep_hash = g_pst_openflow_tpoam->pst_lsp_lmep_hash;
    }
    else
    {
        p_lmep_hash = g_pst_openflow_tpoam->pst_pw_lmep_hash;
    }
    
    p_hash_ret = ctclib_hash_get(p_lmep_hash, (void*)pst_lmep, ctclib_hash_alloc_intern);
    if (!p_hash_ret)
    {
        ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        ctclib_hash_release(g_pst_openflow_tpoam->pst_lmep_hash,  (void*)pst_lmep);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);
        return HSRV_E_NO_MEMORY;
    }

    /* add mep & rmep to hagt */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamAddOam;
    pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamAddOam;
    
    /* key */
    pst_msg->key.oamType = p_msg_req->flow.oam_type;
    if ((GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
        || (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type))
    {
        pst_msg->key.id = p_msg_req->flow.inlabel;
    }
    else
    {
        pst_msg->key.id = gport;    
    }

    pst_msg->key.megid.buf = megid;
    pst_msg->key.megid.size = GLB_TPOAM_MEGID_LEN;
    pst_msg->key.mepid = p_msg_req->flow.mepid;

    /* data */
    pst_msg->nhid = nh_id;

    if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        pst_msg->ttl = pst_lmep->lsp_ttl;
    }
    else  if (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type)
    {
        pst_msg->ttl = pst_lmep->pw_ttl;
    }
    else
    {
        pst_msg->ttl = GLB_OPENFLOW_MPLS_DEFAULT_TTL;      
    }
    pst_msg->exp = pst_lmep->exp;
    
    pst_msg->level = pst_lmep->level;
    pst_msg->ccEnable = p_msg_req->flow.cc_enable;
    pst_msg->dmEnable = p_msg_req->flow.dm_enable;
    pst_msg->lmEnable = p_msg_req->flow.lm_enable;
    pst_msg->active = TRUE;
    pst_msg->interval = p_msg_req->flow.cc_interval;
    pst_msg->mepIndex = index;    /* allocated in hsrvd */
    pst_msg->rmepIndex = index + 1;
    pst_msg->rmepid = p_msg_req->flow.rmepid;

    pst_msg->lmType = p_msg_req->flow.lm_type;
    pst_msg->lmCosType = p_msg_req->flow.lm_cos_type;
    pst_msg->lmCosValue = p_msg_req->flow.lm_cos_value;
    pst_msg->lmStatsInterval = p_msg_req->flow.lm_stats_interval;

    if (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type)
    {
        if (g_pst_openflow_tpoam->pw_not_standard_mode)
        {
            pst_msg->withoutGal = FALSE;
        }
        else
        {
            pst_msg->withoutGal = TRUE;
        }
    }

    if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        if (p_group && (!p_msg_req->flow.is_working_path))
        {
            pst_msg->isProtection = TRUE;
        }
    }

    if (1 == g_pst_openflow_tpoam->pst_lmep_hash->count)
    {
        pst_msg->firstMep = TRUE;
    }

    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Add LMEP key %d fail %d.", pst_msg->key.id, rc);

        ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        ctclib_hash_release(p_lmep_hash,  (void*)pst_lmep);
        ctclib_hash_release(g_pst_openflow_tpoam->pst_lmep_hash,  (void*)pst_lmep);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);
        return rc;    
    }
 
    return rc;
}

static uint32
_hsrv_tpoam_add_mip(fei_openflow_add_tpoam_req_t*   p_msg_req, uint16 gport, uint32 nh_id, uint32 offset)
{
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;
    //HalMsgTpoamAddOamReq_t*             pst_msg = NULL;
    HalMsgEthOamCfmMipReq_t*             pst_msg = NULL;
    //uint32                                             index = 0;
    glb_openflow_tpoam_t*                    pst_mip = NULL;
    //void*                                              p_hash_ret = NULL;
    uint8                                              megid[GLB_TPOAM_MEGID_LEN];

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(megid, 0, GLB_TPOAM_MEGID_LEN);

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    /* allocate node */
    #if 0
    pst_mip = HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW, sizeof(glb_openflow_tpoam_t));
    if (!pst_mip)
    {
        //ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        HSRV_OPENFLOW_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); 
        return HSRV_E_NO_MEMORY;
    }
    #endif

    pst_mip = XCALLOC(MEM_PM_TPOAM_HSRV_MIP_ENTRY, sizeof(glb_openflow_tpoam_t));
    if (!pst_mip)
    {
        //ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        HSRV_OPENFLOW_LOG_ALERT("malloc/calloc failed, %s()@%d", __FUNCTION__, __LINE__); 
        return HSRV_E_NO_MEMORY;
    }

    sal_memcpy(pst_mip, &p_msg_req->flow, sizeof(glb_openflow_tpoam_t));
    pst_mip->nh_id = nh_id;
    pst_mip->offset = offset;
    pst_mip->gport = gport;

    megid[0] = 0x01;
    megid[1] = 0x20;
    megid[2] = 0x0d;
    sal_memcpy(&megid[3], p_msg_req->flow.megid, GLB_TPOAM_MEGID_STR_LEN);
    sal_memcpy(pst_mip->megid, megid, GLB_TPOAM_MEGID_LEN);

    //pst_mip->level = p_msg_req->flow.level;
    
    /* add lmep&rmep node to db */
    #if 0
    p_hash_ret = ctclib_hash_get(g_pst_openflow_tpoam->pst_lmep_hash, 
        (void*)pst_mip, ctclib_hash_alloc_intern);
    if (!p_hash_ret)
    {
        //ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, index);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_mip);
        return HSRV_E_NO_MEMORY;
    }
    #endif

    #if 0
    p_hash_ret = ctclib_hash_get(g_pst_openflow_tpoam->pst_mip_hash, (void*)pst_mip, ctclib_hash_alloc_intern);
    if (!p_hash_ret)
    {
        return HSRV_E_NO_MEMORY;
    }
    #endif
    ctclib_slistnode_insert(g_pst_openflow_tpoam->pst_mip_list, pst_mip);
    
    /* add mip to hagt */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgEthOamCfmAddMip;
    pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgEthOamCfmAddMip;
    
    pst_msg->vlanid = p_msg_req->flow.inlabel;//inlabel east
    pst_msg->glPort = p_msg_req->flow.inlabel_mip;//inlabel west

    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Add mip inlabel %u %u fail %d.", pst_msg->vlanid, pst_msg->glPort,  rc);

        //ctclib_hash_release(p_lmep_hash,  (void*)pst_lmep);
        //ctclib_hash_release(g_pst_openflow_tpoam->pst_lmep_hash,  (void*)pst_lmep);
        //HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);
        return rc;    
    }
 
    return rc;
}

int32
fei_api_openflow_add_tpoam(fei_openflow_add_tpoam_req_t*   p_msg_req)
{
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;
    hsrv_of_nh_offset_t                          nh_offset;
    uint32                                             nh_id = 0;
    uint16                                             gport = GLB_INVALID_GPORTID;
    uint32                                             offset = 0;
     
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    sal_memset(&nh_offset, 0, sizeof(hsrv_of_nh_offset_t));
  
    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    if (GLB_INVALID_IFINDEX != p_msg_req->flow.out_ifindex)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_msg_req->flow.out_ifindex, &gport));
    }

    /* get or allocate nexthop */
    HSRV_IF_ERROR_RETURN(hsrv_openflow_nexthop_alloc_tpoam_nh(gport, &p_msg_req->flow, &nh_id, &offset));

    /* add meg */
    rc = _hsrv_tpoam_add_meg(p_msg_req->flow.megid);
    if (rc)
    {
        hsrv_openflow_nexthop_release_tpoam(p_msg_req->flow.oam_type, nh_id, offset);
        return rc;
    }

    if (p_msg_req->flow.is_mip)
    {
        /* add mip */
        rc = _hsrv_tpoam_add_mip(p_msg_req, gport, nh_id, offset);
        if (rc)
        {
            _hsrv_tpoam_del_meg(p_msg_req->flow.megid);
            //hsrv_openflow_nexthop_release_tpoam(p_msg_req->flow.oam_type, nh_id, offset);
            return rc;
        }
    }
    else
    {
        /* add mep/rmep */
        rc = _hsrv_tpoam_add_mep(p_msg_req, gport, nh_id, offset);
        if (rc)
        {
            _hsrv_tpoam_del_meg(p_msg_req->flow.megid);
            hsrv_openflow_nexthop_release_tpoam(p_msg_req->flow.oam_type, nh_id, offset);
            return rc;
        }
    }
    
    return rc;
}

static uint32
_hsrv_tpoam_del_mip(fei_openflow_del_tpoam_req_t*   p_msg_req)
{
    uint32 find_list_node = FALSE;
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;
    HalMsgEthOamCfmMipReq_t*             pst_msg = NULL;

    glb_openflow_tpoam_t*                   pst_mip = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;

    uint8                                              megid[GLB_TPOAM_MEGID_LEN];
    sal_memset(&megid, 0, GLB_TPOAM_MEGID_LEN);

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    /* del mip to hagt */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgEthOamCfmDelMip;
    pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgEthOamCfmDelMip;

    pst_msg->vlanid = p_msg_req->flow.inlabel;//inlabel east
    pst_msg->glPort = p_msg_req->flow.inlabel_mip;//inlabel west

    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Del mip inlabel %u %u fail %d.", pst_msg->vlanid, pst_msg->glPort,  rc);

        //ctclib_hash_release(p_lmep_hash,  (void*)pst_lmep);
        //ctclib_hash_release(g_pst_openflow_tpoam->pst_lmep_hash,  (void*)pst_lmep);
        //HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);
        //return rc;
    }

    /*MIP*/
    #if 0
    sal_memset(&mip, 0, sizeof(glb_openflow_tpoam_t));
    mip.inlabel = key;
    mip.inlabel_mip = key;
    pst_mip = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_mip_hash, (void*)&mip);
    if(pst_mip)
    {
        ctclib_hash_release(g_pst_openflow_tpoam->pst_mip_hash,  (void*)pst_mip);
        HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_mip);
    }
    else
    {
        HSRV_OPENFLOW_LOG_ERR("Del mip inlabel %u %u fail %d.", pst_msg->vlanid, pst_msg->glPort,  rc);
    }
    #endif

    CTCLIB_SLIST_LOOP_DEL(g_pst_openflow_tpoam->pst_mip_list, pst_mip, listnode, next)
    {
        if (pst_mip->inlabel == p_msg_req->flow.inlabel && pst_mip->inlabel_mip == p_msg_req->flow.inlabel_mip)
        {
            find_list_node = TRUE;
            break;
        }
    }

    if (FALSE == find_list_node)
    {
        HSRV_OPENFLOW_LOG_ERR("Del mip inlabel %u %u fail when delete list node.", p_msg_req->flow.inlabel, p_msg_req->flow.inlabel);
    }
    else
    {
        sal_memcpy(megid, pst_mip->megid, GLB_TPOAM_MEGID_LEN);
        
        /* release nexthop */
        hsrv_openflow_nexthop_release_tpoam(p_msg_req->flow.oam_type, pst_mip->nh_id, pst_mip->offset);

        /* delete megid */
        //if (!sal_strcmp(p_msg_req->flow.megid, &megid[3]))
        {
            _hsrv_tpoam_del_meg(&megid[3]);
        }
        //else
        {
            //HSRV_OPENFLOW_LOG_ERR("Del mip MEGID not same %s %s.\n", p_msg_req->flow.megid, &megid[3]);
        }

        /*delete list node*/
        ctclib_slistnode_delete(g_pst_openflow_tpoam->pst_mip_list, pst_mip);
        XFREE(MEM_PM_TPOAM_HSRV_MIP_ENTRY, pst_mip);
    }
    
    return rc;
}

int32
fei_api_openflow_del_tpoam(fei_openflow_del_tpoam_req_t*   p_msg_req)
{
    uint16                                            gport = 0;
    Hsrv2AgtMsg_t                                hagt_msg;
    HalMsgTpoamDelOamReq_t*             pst_msg = NULL;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    glb_openflow_tpoam_t                      lmep;
    int32                                              rc = HSRV_E_NONE;
    bool                                               opf_ret = 0;
    uint8                                              megid[GLB_TPOAM_MEGID_LEN];
    hsrv_openflow_group_t*                    p_group = NULL;
    
    sal_memset(&lmep, 0, sizeof(glb_openflow_tpoam_t));
    sal_memset(&megid, 0, GLB_TPOAM_MEGID_LEN);
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    if (p_msg_req->flow.is_mip)
    {
        /* del mip */
        rc = _hsrv_tpoam_del_mip(p_msg_req);
        if (rc)
        {

            return rc;
        }
        return rc;
    }

    /* find mep&rmep */
    if (GLB_OAM_TYPE_MPLS_SECTION == p_msg_req->flow.oam_type)
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_msg_req->flow.out_ifindex, &gport));
    }
    
    lmep.inlabel = p_msg_req->flow.inlabel;
    lmep.gport = gport;
    if (GLB_OAM_TYPE_MPLS_SECTION == p_msg_req->flow.oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_section_lmep_hash, (void*)&lmep);
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lsp_lmep_hash, (void*)&lmep);
    }
    else
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_pw_lmep_hash, (void*)&lmep);
    }
    if (!pst_lmep)
    {
        HSRV_LOG_WARN("Lmep&rmep do not exist for inlabel %u\n", lmep.inlabel);
        return HSRV_E_NONE;
    }

    if (pst_lmep->aps_group_id)
    {
        hsrv_openflow_groupdb_lookup_group(pst_lmep->aps_group_id, &p_group);
        if (p_group)
        {
            if (pst_lmep->is_working_path)
            {
                p_group->aps_group.working_mepIndex = 0;
            }
            else
            {
                p_group->aps_group.protection_mepIndex = 0;    
            }
        }
    }
    
    /* del mep/rmep */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamDelOam;
    pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamDelOam;

    /* key */
    pst_msg->key.oamType = p_msg_req->flow.oam_type;
    if ((GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
        || (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type))
    {
        pst_msg->key.id = p_msg_req->flow.inlabel;
    }
    else
    {
        pst_msg->key.id = gport;    
    }

    sal_memcpy(megid, pst_lmep->megid, GLB_TPOAM_MEGID_LEN);
    pst_msg->key.megid.buf = pst_lmep->megid;
    pst_msg->key.megid.size = GLB_TPOAM_MEGID_LEN;
    pst_msg->key.mepid = pst_lmep->mepid;
    pst_msg->mepIndex = pst_lmep->mepIndex;
    pst_msg->rmepIndex = pst_lmep->rmepIndex;
    pst_msg->rmepid = pst_lmep->rmepid;

    if (1 == g_pst_openflow_tpoam->pst_lmep_hash->count)
    {
        pst_msg->lastMep = TRUE;
    }

    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Delete mep %d rmep %d failure rc %d", pst_lmep->mepid, pst_msg->rmepid, rc);    
    }

    /* release nexthop */
    hsrv_openflow_nexthop_release_tpoam(p_msg_req->flow.oam_type, pst_lmep->nh_id, pst_lmep->offset);

    /* free mep&rmep index */
    opf_ret = ctclib_opf_free_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, 2, (uint32)pst_lmep->mepIndex);
    if (!opf_ret)
    {
        HSRV_OPENFLOW_LOG_ERR("Release MEP index %d fail.", pst_lmep->mepIndex);
    }

    /* free lmep */
    ctclib_hash_release(g_pst_openflow_tpoam->pst_lmep_hash,  (void*)pst_lmep);
    if (GLB_OAM_TYPE_MPLS_SECTION == p_msg_req->flow.oam_type)
    {
        ctclib_hash_release(g_pst_openflow_tpoam->pst_section_lmep_hash,  (void*)pst_lmep);
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        ctclib_hash_release(g_pst_openflow_tpoam->pst_lsp_lmep_hash,  (void*)pst_lmep);
    }
    else
    {
        ctclib_hash_release(g_pst_openflow_tpoam->pst_pw_lmep_hash,  (void*)pst_lmep);
    }
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW, pst_lmep);

    /* del megid */
    _hsrv_tpoam_del_meg(&megid[3]);
    
    return rc;
}

int32
fei_api_openflow_update_tpoam(fei_openflow_update_tpoam_req_t*   p_msg_req)
{
    uint16                                            gport = 0;
    Hsrv2AgtMsg_t                                hagt_msg;
    HalMsgTpoamAddOamReq_t*             pst_msg = NULL;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    glb_openflow_tpoam_t                      lmep;
    int32                                              rc = HSRV_E_NONE;
    
    sal_memset(&lmep, 0, sizeof(glb_openflow_tpoam_t));
    
    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    /* add oam */
    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamUpdateOam;
    pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamUpdateOam;

    /* update flag & enable */
    pst_msg->flag = p_msg_req->flow.flag;
    
    /* key */
    pst_msg->key.oamType = p_msg_req->flow.oam_type;
    if ((GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
        || (GLB_OAM_TYPE_MPLS_PW == p_msg_req->flow.oam_type))
    {
        pst_msg->key.id = p_msg_req->flow.inlabel;
    }
    else
    {
        HSRV_IF_ERROR_RETURN(hsrv_ifdb_get_portid(p_msg_req->flow.out_ifindex, &gport));
        pst_msg->key.id = gport;    
    }

    lmep.inlabel = p_msg_req->flow.inlabel;
    lmep.gport = gport;
    if (GLB_OAM_TYPE_MPLS_SECTION == p_msg_req->flow.oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_section_lmep_hash, (void*)&lmep);
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == p_msg_req->flow.oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lsp_lmep_hash, (void*)&lmep);
    }
    else
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_pw_lmep_hash, (void*)&lmep);
    }
    
    if (!pst_lmep)
    {
        HSRV_LOG_WARN("Lmep&rmep do not exist for inlabel %u\n", lmep.inlabel);
        return HSRV_E_NONE;
    }

    /*cmd enable cc*/
    if (FLAG_ISSET(pst_msg->flag, GLB_TPOAM_UPDATE_CC_EN))
    {
        if (pst_lmep->cc_enable != p_msg_req->flow.cc_enable)
        {
            pst_lmep->cc_enable = p_msg_req->flow.cc_enable;
            /*commit*/
            pst_msg->ccEnable = p_msg_req->flow.cc_enable;
        }
        else
        {
            /*duplicate, not set*/
            GLB_UNSET_FLAG(pst_msg->flag, GLB_TPOAM_UPDATE_CC_EN);
        }
    }

    /*event set rdi*/
    if (FLAG_ISSET(pst_msg->flag, GLB_TPOAM_UPDATE_RDI))
    {
        /*commit*/
        pst_msg->rdiEnable = p_msg_req->flow.rdi_enable;
    }

    /*event set lm enable*/
    if (FLAG_ISSET(pst_msg->flag, GLB_TPOAM_UPDATE_LM_EN))
    {
        pst_lmep->lm_enable = p_msg_req->flow.lm_enable;
        pst_lmep->lm_type = p_msg_req->flow.lm_type;
        pst_lmep->lm_cos_type = p_msg_req->flow.lm_cos_type;
        pst_lmep->lm_cos_value = p_msg_req->flow.lm_cos_value;
        pst_lmep->lm_stats_interval = p_msg_req->flow.lm_stats_interval;
        
        pst_msg->lmEnable = p_msg_req->flow.lm_enable;
        pst_msg->lmType = p_msg_req->flow.lm_type;
        pst_msg->lmCosType = p_msg_req->flow.lm_cos_type;
        pst_msg->lmCosValue = p_msg_req->flow.lm_cos_value;
        pst_msg->lmStatsInterval = p_msg_req->flow.lm_stats_interval;
    }

    pst_msg->key.megid.buf = pst_lmep->megid;
    pst_msg->key.megid.size = GLB_TPOAM_MEGID_LEN;
    pst_msg->key.mepid = pst_lmep->mepid;
    pst_msg->mepIndex = pst_lmep->mepIndex;
   
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));

    return rc;
}

int32
fei_api_openflow_g8131_fast_aps_en(fei_g8131_mpls_fast_aps_req_t*   p_msg_req)
{
    int32                                              rc = HSRV_E_NONE;
    hsrv_openflow_group_t*                    p_group = NULL;

    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    HSRV_IF_ERROR_RETURN(hsrv_openflow_group_lookup_group(p_msg_req->aps_group_id, &p_group));
    if (!p_group)
    {
        /* temp modified by liwh for bug 46591 to support mpls tpoam, 2018-05-11 */
        //HSRV_OPENFLOW_LOG_ERR("Aps group id %u does not exist", p_msg_req->aps_group_id); 
        //return HSRV_E_INVALID_PARAM;
        /* liwh end */
        return rc;
    }

    p_group->aps_group.fast_aps_en = p_msg_req->fast_aps_en;

    return rc;
}

int32
fei_api_openflow_g8131_protection_en(fei_g8131_mpls_protection_en_req_t*   p_msg_req)
{
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;
    hsrv_openflow_group_t*                    p_group = NULL;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    hsrv_openflow_groupdb_lookup_group(p_msg_req->aps_group_id, &p_group);
    if (!p_group)
    {
        HSRV_OPENFLOW_LOG_ERR("Aps group id %u does not exist", p_msg_req->aps_group_id);
        return HSRV_E_INVALID_PARAM;
    }

    /* add meg to hsrv */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgMplsApsProtectingEnReq;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgMplsApsProtectingEnReq.apsgroupid = p_group->aps_group.aps_group_offset;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgMplsApsProtectingEnReq.protectingen = p_msg_req->protection_en;
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);

    /*Modified by yejl for bug 47706, 2018-06-21*/
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR(" aps %d; enable %d rc %d\n", p_msg_req->aps_group_id, p_msg_req->protection_en, rc);   
        return rc;
    }

    if (GLB_OF_GROUP_TYPE_LSP_APS == p_group->group.group_type)
    {
        hsrv_tpoam_update_pw_mep_nh_by_groupid(p_msg_req->aps_group_id);    
    }

    return rc;
}

int32
fei_api_set_pw_mode(uint32*   p_msg_req)
{
    int32                                              rc = HSRV_E_NONE;
    Hsrv2AgtMsg_t                                 hagt_msg;

    sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_msg_req);

    /* set pw mode to hagt */
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetPwMode;
    hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamSetPwMode.enable= *p_msg_req;
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    
    rc = HSRV2AGT_MSG_SEND(&hagt_msg);
    if (rc)
    {
        HSRV_OPENFLOW_LOG_ERR("Set pw mode %d\n", *p_msg_req);   
        return rc;
    }

    g_pst_openflow_tpoam->pw_not_standard_mode = *p_msg_req;

    return rc;
}
static uint32
_hsrv_tpoam_label_hash_key(glb_openflow_tpoam_t* pst_lmep)
{
    uint32 key = 0;

    key = pst_lmep->inlabel;

    return key;
}

static bool
_hsrv_tpoam_label_hash_cmp(void* pva, void* pvb)
{
    glb_openflow_tpoam_t* pst_lmep_a = (glb_openflow_tpoam_t*)pva;
    glb_openflow_tpoam_t* pst_lmep_b = (glb_openflow_tpoam_t*)pvb;

    if (pst_lmep_a->inlabel == pst_lmep_b->inlabel)
    {
        return TRUE;
    }

    return FALSE;
}

static uint32
_hsrv_tpoam_port_hash_key(glb_openflow_tpoam_t* pst_lmep)
{
    uint32 key = 0;

    key = pst_lmep->gport;

    return key;
}

static bool
_hsrv_tpoam_port_hash_cmp(void* pva, void* pvb)
{
    glb_openflow_tpoam_t* pst_lmep_a = (glb_openflow_tpoam_t*)pva;
    glb_openflow_tpoam_t* pst_lmep_b = (glb_openflow_tpoam_t*)pvb;

    if (pst_lmep_a->gport == pst_lmep_b->gport)
    {
        return TRUE;
    }

    return FALSE;
}

static uint32
_hsrv_tpoam_mepindex_hash_key(glb_openflow_tpoam_t* pst_lmep)
{
    uint32 key = 0;

    key = pst_lmep->mepIndex;

    return key;
}

static bool
_hsrv_tpoam_mepindex_hash_cmp(void* pva, void* pvb)
{
    glb_openflow_tpoam_t* pst_lmep_a = (glb_openflow_tpoam_t*)pva;
    glb_openflow_tpoam_t* pst_lmep_b = (glb_openflow_tpoam_t*)pvb;

    if (pst_lmep_a->mepIndex == pst_lmep_b->mepIndex)
    {
        return TRUE;
    }

    return FALSE;
}

static uint32
_hsrv_tpoam_megid_hash_key(glb_tpoam_megid_t* pst_megid)
{
    uint32 index = 0;
    uint32 key = 0;

    for (index = 0; index < GLB_TPOAM_MEGID_LEN; index++)
    {
        key += (uint8)pst_megid->id[index];
    }

    return key;
}

static bool
_hsrv_tpoam_megid_hash_cmp(void* pva, void* pvb)
{
    glb_tpoam_megid_t* pst_megid_a = (glb_tpoam_megid_t*)pva;
    glb_tpoam_megid_t* pst_megid_b = (glb_tpoam_megid_t*)pvb;

    if (!sal_memcmp(pst_megid_a->id, pst_megid_b->id, GLB_TPOAM_MEGID_LEN))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

#ifndef hsrv_process_hagt_msg
/*event use high priority*/
int32
hsrv_tpoam_rx_hagt_event(void* pv_msg_data)
{
    uint32 count = 0;
    uint32 send_count = 0;
    glb_tpoam_event_entry_t* pst_events = NULL;
    Hagt2SrvMsg_t* pst_hagt2srv_msg = NULL;
    //hsrv_tpoam_lmep_t* pst_mep = NULL;
    glb_openflow_tpoam_t *pst_lmep = NULL;
    
    uint32 i = 0;
    hal_tpoam_event_t hal_event;

    pst_hagt2srv_msg = (Hagt2SrvMsg_t*)pv_msg_data;
    count = pst_hagt2srv_msg->opHagt2Srv.hagt2SrvMsgTpoamEvent.count;
    pst_events = (glb_tpoam_event_entry_t*)pst_hagt2srv_msg->opHagt2Srv.hagt2SrvMsgTpoamEvent.rawData.buf;

    HSRV_TPOAM_EVENT_DEBUG("Receive event from HAGT, count is %d.", count);
    for (i = 0; i < count; i++)
    {
        if ((GLB_TPOAM_MEP_INDEX_MIN > pst_events[i].lmep_index)
            || (GLB_TPOAM_MEP_INDEX_MAX < pst_events[i].lmep_index))
        {
            HSRV_LOG_ERR("Invalid Event index LMEP index %d.", pst_events[i].lmep_index);
            continue;
        }

        glb_openflow_tpoam_t                      lmep;
        lmep.mepIndex = pst_events[i].lmep_index;
        
        HSRV_TPOAM_EVENT_DEBUG("before rcv event LMEP index %d, RMEP index %d event status 0x%04X, %s.", 
            pst_events[i].lmep_index, pst_events[i].rmep_index, pst_events[i].event_bmp, tpoam_api_event_ext_desc(pst_events[i].event_bmp));
            
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lmep_hash, (void*)&lmep);
        if (!pst_lmep)
        {
            HSRV_LOG_ERR("Not found LMEP by LMEP index %d.", pst_events[i].lmep_index);
            continue;
        }
        HSRV_TPOAM_EVENT_DEBUG("after rcv event session id %u MEGID %s LMEP index %d, RMEP index %d event status 0x%04X, %s.", 
            pst_lmep->oam_session_id, &pst_lmep->megid, pst_events[i].lmep_index, pst_events[i].rmep_index, 
            pst_events[i].event_bmp, tpoam_api_event_ext_desc(pst_events[i].event_bmp));

        // TODO: fast aps
        #if 0
        HSRV_MPLS_DB_LOCK;
        if (g_pst_hsrv_tpoam->fast_aps_en)
        {
            /* only if fast_aps_en and sf_reason has GLB_TPOAM_EVT_LOC and/or GLB_TPOAM_EVT_RDI_RX, could to do fast aps switchover */
            if (!((~(GLB_TPOAM_EVT_LOC | GLB_TPOAM_EVT_RDI_RX)) & g_pst_hsrv_tpoam->sf_reason))
            {
                _hsrv_tpoam_fast_aps(pst_mep, pst_events[i].event_bmp);
            }
        }
        HSRV_MPLS_DB_UNLOCK;
        #endif
        
        pst_lmep->event_bmp = pst_events[i].event_bmp;
        hal_event.events[i].oam_session_id = pst_lmep->oam_session_id;
        sal_memcpy(hal_event.events[i].megid, &pst_lmep->megid[3], GLB_TPOAM_MEGID_STR_LEN);
        hal_event.events[i].event_bmp = pst_events[i].event_bmp;

        send_count++;
    }
    //hal_event.count = count;
    hal_event.count = send_count;
    if (send_count)
    {
        return HSRV2OPM_MSG_SEND(TPOAM_HAL_MSG_TYPE_TPOAM_EVENT, &hal_event, sizeof(hal_event));
    }
    else
    {
        HSRV_LOG_ERR("Not send event msg to hsrv for not have event.");
        return HSRV_E_NONE;
    }
}

int32
hsrv_tpoam_rx_hagt_lm_dual_counters(void* pv_msg_data)
{
    uint32 count = 0;
    hal_tpoam_lm_dual_t hal_lm;
    glb_tpoam_lm_dual_counter_t* pst_lm_entry = NULL;
    Hagt2SrvMsg_t  *pst_hagt2srvmsg = NULL;
    //hsrv_tpoam_lmep_t* pst_lmep = NULL;
    glb_openflow_tpoam_t *pst_lmep = NULL;
    uint32 i = 0;
    
    /* dump information */
    //HSRV_TPOAM_DBG_FUNC();
    
    sal_memset(&hal_lm, 0, sizeof(hal_tpoam_lm_dual_t));
    pst_hagt2srvmsg = (Hagt2SrvMsg_t*)pv_msg_data;
    //count = pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgTpoamLmDualReq.count;
    count = pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgEthOamCfmLmDualReq.count;
    //pst_lm_entry = (glb_tpoam_lm_dual_counter_t*)pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgTpoamLmDualReq.rawData.buf;
    pst_lm_entry = (glb_tpoam_lm_dual_counter_t*)pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgEthOamCfmLmDualReq.rawData.buf;

    if (count > GLB_TPOAM_MAX_LM_DUAL_COUNTER_CACHE_NUM)
    {
        HSRV_LOG_ERR("The number of lm counter cache is %d, more than default value", count);
        return HSRV_E_INVALID_PARAM;    
    }

    HSRV_TPOAM_EVENT_DEBUG("Receive lm entries from HAGT, count is %d.", count);
    for (i = 0; i < count; i++)
    {
        if ((GLB_TPOAM_MEP_INDEX_MIN > pst_lm_entry[i].mep_index)
            || (GLB_TPOAM_MEP_INDEX_MAX < pst_lm_entry[i].mep_index))
        {
            HSRV_LOG_ERR("Invalid Event index LMEP index %d.", pst_lm_entry[i].mep_index);
            continue;
        }
        
        glb_openflow_tpoam_t                      lmep;
        lmep.mepIndex = pst_lm_entry[i].mep_index;
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lmep_hash, (void*)&lmep);
        if (!pst_lmep)
        {
            HSRV_LOG_ERR("Not found LMEP by LMEP index %d.", pst_lm_entry[i].mep_index);
            continue;
        }
        HSRV_TPOAM_EVENT_DEBUG("Session id %u MEGID %s LMEP index %d cos %d TxFCf %u RxFCb %u TxFCb %u RxFCl %u.", 
            pst_lmep->oam_session_id, &pst_lmep->megid, pst_lm_entry[i].mep_index, pst_lm_entry[i].ccm_cos_value,
            pst_lm_entry[i].TxFCf, pst_lm_entry[i].RxFCb, pst_lm_entry[i].TxFCb, pst_lm_entry[i].RxFCl);

        hal_lm.lm_data[i].TxFCf = pst_lm_entry[i].TxFCf;
        hal_lm.lm_data[i].RxFCb = pst_lm_entry[i].RxFCb;
        hal_lm.lm_data[i].TxFCb = pst_lm_entry[i].TxFCb;
        hal_lm.lm_data[i].RxFCl = pst_lm_entry[i].RxFCl;
        hal_lm.lm_data[i].ccm_cos_value = pst_lm_entry[i].ccm_cos_value;       
        hal_lm.lm_data[i].oam_session_id = pst_lmep->oam_session_id;
        sal_memcpy(hal_lm.lm_data[i].megid, &pst_lmep->megid, GLB_TPOAM_MEGID_STR_LEN);
    }
    hal_lm.count = count;
    
    HSRV_IF_ERROR_RETURN(HSRV2OPM_MSG_SEND(TPOAM_HAL_MSG_TYPE_LM_DUAL_COUNTERS, &hal_lm, sizeof(hal_lm)));
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_rx_hagt_lm_sd(void* pv_msg_data)
{
    glb_tpoam_lm_sd_cache_t req;
    Hagt2SrvMsg_t  *pst_hagt2srvmsg = NULL;
    
    sal_memset(&req, 0, sizeof(glb_tpoam_lm_sd_cache_t));

    pst_hagt2srvmsg = (Hagt2SrvMsg_t*)pv_msg_data;
    req.num = pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgEthOamCfmLmSDReq.count;

    if (req.num > GLB_TPOAM_MAX_LM_SD_CACHE_NUM)
    {
        HSRV_LOG_ERR("The number of lm sd cache is %d", req.num);
        return HSRV_E_INVALID_PARAM;    
    }
    
    sal_memcpy(req.entry, pst_hagt2srvmsg->opHagt2Srv.hagt2SrvMsgEthOamCfmLmSDReq.rawData.buf, 
                  req.num * sizeof(glb_tpoam_lm_sd_t));  
    
    HSRV_IF_ERROR_RETURN(HSRV2OPM_MSG_SEND(TPOAM_HAL_MSG_TYPE_LM_SD, &req, sizeof(req)));
    return HSRV_E_NONE;
}


#endif

#ifndef hsrv_sync_tpoam_global
int32
_hsrv_process_sync_tpoam_global_set_fast_aps_en(uint32 fast_aps_en)
{
    Hsrv2AgtMsg_t hagt_msg;
    HalMsgTpoamHagtAPSReq_t* pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamSetHagtAPS;
   
    sal_memset(&hagt_msg, 0, sizeof(hagt_msg));
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetHagtAPS;

    pst_msg->enable = fast_aps_en;

    /* send to hagt */
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));
    
    g_pst_openflow_tpoam->fast_aps_en = fast_aps_en;

    return HSRV_E_NONE;
}

static int32
_hsrv_tpoam_hagt_set_rdi_reason(uint8 rdi_reason)
{
    Hsrv2AgtMsg_t hagt_msg;
    HalMsgTpoamRdiReasonReq_t* pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamRdiReason;
   
    sal_memset(&hagt_msg, 0, sizeof(hagt_msg));
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamRdiReason;

    /* 1. Build key - no key */

    /* 2. Build data */
    pst_msg->rdiReason = rdi_reason;

    /* 3. Send to HAGT */
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));
    return HSRV_E_NONE;
}

int32
_hsrv_process_sync_tpoam_global_set_rdi_reason(uint32 rdi_reason)
{
    int32 ret = HSRV_E_NONE;

    HSRV_TPOAM_COMMON_DEBUG("Set RDI reason 0x%02X.", rdi_reason);
    
    g_pst_openflow_tpoam->rdi_reason = rdi_reason;

    /*send to hagt*/
    ret = _hsrv_tpoam_hagt_set_rdi_reason(rdi_reason);
    if (ret < 0)
    {
        HSRV_LOG_ERR("Set RDI reason failed %d.", ret);
        return ret;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_tpoam_hagt_set_sf_reason(uint32 sf_reason)
{
    Hsrv2AgtMsg_t hagt_msg;
    HalMsgTpoamSFReasonReq_t* pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamSetSFReason;
   
    sal_memset(&hagt_msg, 0, sizeof(hagt_msg));
    hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamSetSFReason;

    /* 1. Build key - no key */

    /* 2. Build data */
    pst_msg->sfReason = sf_reason;

    /* 3. Send to HAGT */
    HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
    HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));
    return HSRV_E_NONE;
}

int32
_hsrv_process_sync_tpoam_global_set_sf_reason(uint32 sf_reason)
{
    int32 ret = HSRV_E_NONE;

    HSRV_TPOAM_COMMON_DEBUG("Set SF reason 0x%08X.", sf_reason);
    
    g_pst_openflow_tpoam->sf_reason = sf_reason;

    /*send to hagt*/
    ret = _hsrv_tpoam_hagt_set_sf_reason(sf_reason);
    if (ret < 0)
    {
        HSRV_LOG_ERR("Set SF reason failed %d.", ret);
        return ret;
    }
    
    return HSRV_E_NONE;
}

static int32
_hsrv_process_sync_tpoam_global_after_cb(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, void *p_tbl, void *p_ds)
{
    tbl_tpoam_global_t *p_tpoam_global = NULL;
    int32 rc = PM_E_NONE;
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        break;

    case CDB_OPER_SET:
        if (p_ds_node)
        {
            /* set sub DS */
            break;
        }
        
        p_tpoam_global = (tbl_tpoam_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_TPOAM_GLOBAL_FLD_FAST_APS_EN:
            _hsrv_process_sync_tpoam_global_set_fast_aps_en(p_tpoam_global->fast_aps_en);
            break;
        case TBL_TPOAM_GLOBAL_FLD_RDI_REASON:
            _hsrv_process_sync_tpoam_global_set_rdi_reason(p_tpoam_global->rdi_reason);
            break;
        case TBL_TPOAM_GLOBAL_FLD_SF_REASON_MASK:
            _hsrv_process_sync_tpoam_global_set_rdi_reason(p_tpoam_global->rdi_reason);
            break;
        default:
            return PM_E_INVALID_PARAM;
        }
        break;

    case CDB_OPER_DEL:
        break;

    default:
        return PM_E_INVALID_PARAM;
    }
    return rc;
}
#endif

#ifndef for_hsrv_hostif
uint32
hsrv_openflow_tpoam_api_get_pw_mode(void)
{
    uint32  pw_is_standard = 0;

    if (0 == g_pst_openflow_tpoam->pw_not_standard_mode)
    {
        pw_is_standard = TRUE;
    }
    
    return pw_is_standard;
}

int32
hsrv_openflow_tpoam_api_get_oam_session_id(uint32 oam_type, uint32 key, uint32 *p_oam_session_id)
{
    uint16                                            gport = 0;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    glb_openflow_tpoam_t                      lmep;

    glb_openflow_tpoam_t*                   pst_mip = NULL;
    glb_openflow_tpoam_t                     mip = {0};
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    
    sal_memset(&lmep, 0, sizeof(glb_openflow_tpoam_t));
    
    hsrv_ifdb_get_portid(key, &gport);
    lmep.gport = gport;
    lmep.inlabel = key;
    if (GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP == oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_section_lmep_hash, (void*)&lmep);
    }
    else if (GLB_TPOAM_TX_KEY_TYPE_LSP == oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lsp_lmep_hash, (void*)&lmep);
        /*MIP*/
        if (!pst_lmep)
        {
            sal_memset(&mip, 0, sizeof(glb_openflow_tpoam_t));
            mip.inlabel = key;
            mip.inlabel_mip = key;
            pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_mip_hash, (void*)&mip);
        }

        if(!pst_lmep)
        {
            CTCLIB_SLIST_LOOP_DEL(g_pst_openflow_tpoam->pst_mip_list, pst_mip, listnode, next)
            {
                if (pst_mip->inlabel == key || pst_mip->inlabel_mip == key)
                {
                    *p_oam_session_id = pst_mip->oam_session_id;
                    return HSRV_E_NONE;
                }
            }
        }
    }
    else
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_pw_lmep_hash, (void*)&lmep);
    }
    
    if (!pst_lmep)
    {
        HSRV_TPOAM_PKT_RX_DEBUG("Lmep&rmep do not exist for inlabel %u gport %u when get session id\n", lmep.inlabel, gport);
        return HSRV_E_FAIL;
    }

    *p_oam_session_id = pst_lmep->oam_session_id;
    
    return HSRV_E_NONE;
}


int32
hsrv_openflow_tpoam_api_get_nhid_offset(uint32 oam_type, uint32 key, uint32 *p_nh_id, uint32 *p_nh_offset, uint32* p_lsp_aps, uint32* working)
{
    uint16                                            gport = 0;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    glb_openflow_tpoam_t                      lmep;
    
    sal_memset(&lmep, 0, sizeof(glb_openflow_tpoam_t));
    
    hsrv_ifdb_get_portid(key, &gport);
    lmep.gport = gport;
    lmep.inlabel = key;
    if (GLB_TPOAM_TX_KEY_TYPE_SECTION_NONIP == oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_section_lmep_hash, (void*)&lmep);
    }
    else if (GLB_TPOAM_TX_KEY_TYPE_LSP == oam_type)
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lsp_lmep_hash, (void*)&lmep);
    }
    else
    {
        pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_pw_lmep_hash, (void*)&lmep);
    }
    
    if (!pst_lmep)
    {
        HSRV_TPOAM_PKT_TX_DEBUG("Lmep&rmep do not exist for inlabel %u gport %u when get nhid\n", lmep.inlabel, gport);
        return HSRV_E_FAIL;
    }

    *p_nh_offset = pst_lmep->offset;
    *p_nh_id = pst_lmep->nh_id;

    if ((GLB_TPOAM_TX_KEY_TYPE_LSP == oam_type) && pst_lmep->aps_group_id)
    {
        *p_lsp_aps = TRUE;
        *working = pst_lmep->is_working_path;
    }
    /* modified by liwh for bug 47601, 2018-06-12 */
    else
    {
        *p_lsp_aps = FALSE;    
    }
    /* liw end */
    
    return HSRV_E_NONE;
}

int32
hsrv_openflow_tpoam_api_get_dest_gport(uint32 ifindex, uint16 *p_dest_gport, uint16 *p_src_gport)
{
    uint16 src_gport;
    uint16  dest_gport = 0;
    //uint16 destid;
    //uint8 dest_slotid;
    //uint8 dest_chipid;

    //uint8 tid = 0;
    
    tbl_interface_t *p_db_if = NULL;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_FAIL;
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        // TODO:
        #if 0
        //ret = hsrv_ifdb_get_tid(p_encap_info->peth_hdr.ifindex, &tid);
        dest_gport = hsrv_ifdb_get_agg_gport_by_tid(tid);
        dest_gport = GLB_TRANS_TID_TO_AGGPORT(tid);
        #endif
        
        hsrv_ifdb_get_portid(ifindex, &dest_gport);
        //tid = GLB_TRANS_AGGPORT_TO_TID(dest_gport);
        src_gport = GLB_CPU_LOCAL_PHYPORT;
        //destid = tid;
        //dest_chipid = GLB_LINKAGG_CHIPID;
        //ret = hsrv_ifdb_get_slotno_by_gportid(gport, &(p_encap_info->dest_slotid));
    }
    else if (GLB_IF_TYPE_PORT_IF== p_db_if->hw_type)
    {
        hsrv_ifdb_get_portid(ifindex, &dest_gport);
        src_gport = GLB_CPU_LOCAL_PHYPORT;
        //destid = GLB_GET_LOCAL_PORTID(dest_gport);
        //dest_chipid = GLB_GET_GLOBAL_CHIPID(dest_gport);
        //ret = hsrv_ifdb_get_slotno_by_gportid(gport, &(p_encap_info->dest_slotid));
    }
    else if (GLB_IF_TYPE_VLAN_IF == p_db_if->hw_type)
    {
    }
    else
    {
    }

    *p_dest_gport = dest_gport;
    *p_src_gport = src_gport;
    
    return HSRV_E_NONE;
}
#endif
/*****************************************************************************************************
 * Name    : _hsrv_aps_group_offset_alloc
 * Purpose : alloc aps offset by opf
 * Input   : N/A
 * Output  : p_aps_offset: aps  group offset to alloc
 * Return  : HSRV_E_***  - hal server return type
 * Note    : N/A
******************************************************************************************************/
int32
hsrv_aps_group_offset_alloc(uint32* p_aps_offset)
{
    uint32 offset = CTCLIB_OPF_INVALID_OFFSET;

    HSRV_TPOAM_DBG_FUNC();
    
    HSRV_OPENFLOW_PTR_CHECK(p_aps_offset);
    
    offset = ctclib_opf_alloc_offset (g_pst_openflow_tpoam->pst_aps_offset_opf, 0, 1);
    if( CTCLIB_OPF_INVALID_OFFSET == offset )
    {
        HSRV_LOG_WARN("APS has no enough offset resource.");
        return HSRV_E_RESOURCE_FULL;
    }
    *p_aps_offset = offset;

    HSRV_TPOAM_COMMON_DEBUG(" aps group alloc offset = %d.", *p_aps_offset);

    return HSRV_E_NONE;
}
/*****************************************************************************************************
 * Name    : hsrv_aps_group_offset_release
 * Purpose : release aps group offsetby opf
 * Input   : offset: aps group offset to release
 * Output  : N/A
 * Return  : HSRV_E_***  - hal server return type
 * Note    : N/A
******************************************************************************************************/
int32
hsrv_aps_group_offset_release(uint32 offset)
{
    bool ret;

    HSRV_TPOAM_DBG_FUNC();

    ret = ctclib_opf_free_offset (g_pst_openflow_tpoam->pst_aps_offset_opf, 0, 1, offset);
    if (FALSE == ret)
    {
        HSRV_OPENFLOW_LOG_ERR("Offset resource release failed, offset = %d\n", offset);
        return HSRV_E_INVALID_PARAM;
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_get_aps_group_fast_aps_en(uint32 mep_index, uint32 event_bmp, uint32* p_fast_aps_en, uint32* p_aps_group, uint32* p_aps_group_offset, uint32* p_protection_en)
{
    glb_openflow_tpoam_t                      lmep;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    hsrv_openflow_group_t*                    p_group = NULL;
    glb_openflow_tpoam_t*                    pst_protection_lmep = NULL;
    
    sal_memset(&lmep, 0, sizeof(glb_openflow_tpoam_t));

    HSRV_TPOAM_DBG_FUNC();

    HSRV_OPENFLOW_PTR_CHECK(p_protection_en);
    HSRV_OPENFLOW_PTR_CHECK(p_fast_aps_en);
    HSRV_OPENFLOW_PTR_CHECK(p_aps_group_offset);

    /* find mep&rmep */
    lmep.mepIndex = mep_index;
    pst_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lmep_hash, (void*)&lmep);
    if (!pst_lmep)
    {
        HSRV_OPENFLOW_LOG_ERR("Mep for mep_index %u does not exist", mep_index);
        return HSRV_E_INVALID_PARAM;
    }

    /* only wokring path status change will lead to aps switch */
    if (!pst_lmep->is_working_path)
    {
        pst_lmep->event_bmp = event_bmp;
        return HSRV_E_NONE;
    }

    /*Added by yejl for fast aps log print, 2018-05-15*/
    if (FLAG_ISSET(pst_lmep->event_bmp, GLB_TPOAM_EVT_LOC) || FLAG_ISSET(pst_lmep->event_bmp, GLB_TPOAM_EVT_RDI_RX))
    {
        pst_lmep->event_bmp = event_bmp;
        return HSRV_E_NONE;
    }

    if ((FLAG_ISSET(event_bmp, GLB_TPOAM_EVT_LOC) && !FLAG_ISSET(pst_lmep->event_bmp, GLB_TPOAM_EVT_LOC))
        || (FLAG_ISSET(event_bmp, GLB_TPOAM_EVT_RDI_RX) && !FLAG_ISSET(pst_lmep->event_bmp, GLB_TPOAM_EVT_RDI_RX)))
    {
        pst_lmep->event_bmp = event_bmp;
        *p_protection_en = TRUE;
    } 
    else
    {
        pst_lmep->event_bmp = event_bmp;
        return HSRV_E_NONE;    
    }

    if (0== pst_lmep->aps_group_id)
    {
        return HSRV_E_NONE;
    }
    
    HSRV_IF_ERROR_RETURN(hsrv_openflow_groupdb_lookup_group(pst_lmep->aps_group_id, &p_group));
    if (!p_group)
    {
        HSRV_OPENFLOW_LOG_ERR("Aps group id %u does not exist", pst_lmep->aps_group_id);    
        return HSRV_E_INVALID_PARAM;
    }

    /* if no protection mep, return */
    if (0 == p_group->aps_group.protection_mepIndex)
    {
        return HSRV_E_NONE;
    }
    
    /* find protection mep&rmep */
    lmep.mepIndex = p_group->aps_group.protection_mepIndex;
    pst_protection_lmep = ctclib_hash_lookup (g_pst_openflow_tpoam->pst_lmep_hash, (void*)&lmep);
    if (!pst_protection_lmep)
    {
        return HSRV_E_NONE;
    }

    /* if protection mep is down, shouldn't switch to protection */
    if (FLAG_ISSET(pst_protection_lmep->event_bmp, GLB_TPOAM_EVT_LOC) ||FLAG_ISSET(pst_protection_lmep->event_bmp, GLB_TPOAM_EVT_RDI_RX))
    {
        return HSRV_E_NONE;    
    }

    *p_fast_aps_en = p_group->aps_group.fast_aps_en;
    *p_aps_group_offset = p_group->aps_group.aps_group_offset;
    *p_aps_group = pst_lmep->aps_group_id;
    
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_brief(FILE* fp)
{
    hsrv_openflow_tpoam_t* p_master = g_pst_openflow_tpoam;
   
    sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM BRIEF------------------\n");
    sal_fprintf(fp, "Meg count                : %d\n", p_master->pst_megid_hash->count);
    sal_fprintf(fp, "Mep count                : %d\n", p_master->pst_lmep_hash->count);
    sal_fprintf(fp, "Mip count                : %d\n", p_master->pst_mip_list->count);
    sal_fprintf(fp, "Fast aps enable          : %d\n", p_master->fast_aps_en);
    sal_fprintf(fp, "Pw not standard          : %d\n", p_master->pw_not_standard_mode);
    sal_fprintf(fp, "----------------------------------------------------\n");
    
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_meg(FILE* fp)
{
    hsrv_openflow_tpoam_t*                  p_master = g_pst_openflow_tpoam;
    ctclib_hash_backet_t*                      hb;
    ctclib_hash_backet_t*                      next;
    glb_tpoam_megid_t*                        pst_megid = NULL;
    uint32                                             i = 0;
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MEG------------------\n");

    if (0== p_master->pst_megid_hash->count)
    {
        sal_fprintf(fp, "N/A\n");    
    }

    for (i = 0; i < p_master->pst_megid_hash->size; i++)
    {
        for (hb = p_master->pst_megid_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            pst_megid = (glb_tpoam_megid_t *)hb->data;

            sal_fprintf(fp, "Meg id                : %s\n", &pst_megid->id[3]);
            sal_fprintf(fp, "Meg ref count         : %d\n", pst_megid->count);   
        }
    }

    sal_fprintf(fp, "----------------------------------------------------\n");
    
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_mep(FILE* fp, glb_openflow_tpoam_t*  pst_lmep)
{
    sal_fprintf(fp, "Oam session ID          : %d\n", pst_lmep->oam_session_id);

    if (GLB_OAM_TYPE_MPLS_LSP == pst_lmep->oam_type)
    {
        sal_fprintf(fp, "Oam type                : lsp\n");
    }
    else if (GLB_OAM_TYPE_MPLS_PW == pst_lmep->oam_type)
    {
        sal_fprintf(fp, "Oam type                : pw\n");
    }
    else if (GLB_OAM_TYPE_MPLS_SECTION == pst_lmep->oam_type)
    {
        sal_fprintf(fp, "Oam type                : section\n");
    }

    sal_fprintf(fp, "Inlable                 : %d\n", pst_lmep->inlabel);
    sal_fprintf(fp, "Outlable  for lsp       : %d\n", pst_lmep->outlabel_lsp);
    sal_fprintf(fp, "Outlable  for pw        : %d\n", pst_lmep->outlabel_pw);
    sal_fprintf(fp, "Out ifindex             : %d\n", pst_lmep->out_ifindex);

    sal_fprintf(fp, "Meg id                  : %s\n", &pst_lmep->megid[3]);
    sal_fprintf(fp, "Mep id                  : %d\n", pst_lmep->mepid);
    sal_fprintf(fp, "Remote mep id           : %d\n", pst_lmep->rmepid);

    sal_fprintf(fp, "CC enable               : %d\n", pst_lmep->cc_enable);
    sal_fprintf(fp, "CC interval             : %d\n", pst_lmep->cc_interval);
    sal_fprintf(fp, "LM enable               : %d\n", pst_lmep->lm_enable);
    sal_fprintf(fp, "LM type                 : %d\n", pst_lmep->lm_type);
    sal_fprintf(fp, "LM cos type             : %d\n", pst_lmep->lm_cos_type);
    sal_fprintf(fp, "LM cos value            : %d\n", pst_lmep->lm_cos_value);
    sal_fprintf(fp, "LM stats interval       : %d\n", pst_lmep->lm_stats_interval);
    sal_fprintf(fp, "DM interval             : %d\n", pst_lmep->dm_enable);

    sal_fprintf(fp, "Mep index               : %d\n", pst_lmep->mepIndex);
    sal_fprintf(fp, "Remote mep index        : %d\n", pst_lmep->rmepIndex);

    sal_fprintf(fp, "Nexthop id              : %d\n", pst_lmep->nh_id);

    sal_fprintf(fp, "LSP outlabel exp is set : %d\n", pst_lmep->lsp_outlabel_exp_is_set);
    sal_fprintf(fp, "LSP outlabel exp        : %d\n", pst_lmep->lsp_exp);
    sal_fprintf(fp, "LSP TTL                 : %d\n", pst_lmep->lsp_ttl);

    sal_fprintf(fp, "PW outlabel exp is set  : %d\n", pst_lmep->pw_outlabel_exp_is_set);
    sal_fprintf(fp, "PW outlabel exp         : %d\n", pst_lmep->pw_exp);
    sal_fprintf(fp, "PW TTL                  : %d\n", pst_lmep->pw_ttl);
    
    sal_fprintf(fp, "Level                   : %d\n", pst_lmep->level);
    sal_fprintf(fp, "Event bmp               : %d\n", pst_lmep->event_bmp);

    if (pst_lmep->aps_group_id)
    {
        if (GLB_OAM_TYPE_MPLS_LSP == pst_lmep->oam_type)
        {
            sal_fprintf(fp, "Lsp aps group id        : %d\n", pst_lmep->aps_group_id);
        }
        else if (GLB_OAM_TYPE_MPLS_PW == pst_lmep->oam_type)
        { 
            sal_fprintf(fp, "Pw aps group id         : %d\n", pst_lmep->aps_group_id);
        }
    }

    if (pst_lmep->lsp_aps_group_id)
    {
        sal_fprintf(fp, "Lsp aps group id        : %d\n", pst_lmep->lsp_aps_group_id);    
    }
    
    sal_fprintf(fp, "RDI enable              : %d\n", pst_lmep->rdi_enable);
    sal_fprintf(fp, "IS working path         : %d\n", pst_lmep->is_working_path);
    
    sal_fprintf(fp, "Nexthop mac             : %2x:%2x:%2x:%2x:%2x:%2x\n", pst_lmep->nexthop_mac[0], pst_lmep->nexthop_mac[1], pst_lmep->nexthop_mac[2],
        pst_lmep->nexthop_mac[3], pst_lmep->nexthop_mac[4], pst_lmep->nexthop_mac[5]);
        
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_mip(FILE* fp, glb_openflow_tpoam_t*  pst_lmep)
{
    sal_fprintf(fp, "Oam session ID          : %d\n", pst_lmep->oam_session_id);

    if (GLB_OAM_TYPE_MPLS_LSP == pst_lmep->oam_type)
    {
        sal_fprintf(fp, "Oam type                : lsp\n");
    }

    if (TPOAM_MP_TYPE_MIP == pst_lmep->is_mip)
    {
        sal_fprintf(fp, "Mp type                 : MIP\n");
    }

    sal_fprintf(fp, "Inlable east            : %d\n", pst_lmep->inlabel);
    sal_fprintf(fp, "Inlable west            : %d\n", pst_lmep->inlabel_mip);
    #if 0
    sal_fprintf(fp, "Outlable  for lsp       : %d\n", pst_lmep->outlabel_lsp);
    sal_fprintf(fp, "Outlable  for pw        : %d\n", pst_lmep->outlabel_pw);
    sal_fprintf(fp, "Out ifindex             : %d\n", pst_lmep->out_ifindex);
    #endif
    sal_fprintf(fp, "Meg id                  : %s\n", &pst_lmep->megid[3]);
    #if 0
    sal_fprintf(fp, "Mep id                  : %d\n", pst_lmep->mepid);
    sal_fprintf(fp, "Remote mep id           : %d\n", pst_lmep->rmepid);

    sal_fprintf(fp, "CC enable               : %d\n", pst_lmep->cc_enable);
    sal_fprintf(fp, "CC interval             : %d\n", pst_lmep->cc_interval);
    sal_fprintf(fp, "LM enable               : %d\n", pst_lmep->lm_enable);
    sal_fprintf(fp, "LM type                 : %d\n", pst_lmep->lm_type);
    sal_fprintf(fp, "LM cos type             : %d\n", pst_lmep->lm_cos_type);
    sal_fprintf(fp, "LM cos value            : %d\n", pst_lmep->lm_cos_value);
    sal_fprintf(fp, "LM stats interval       : %d\n", pst_lmep->lm_stats_interval);
    sal_fprintf(fp, "DM interval             : %d\n", pst_lmep->dm_enable);

    sal_fprintf(fp, "Mep index               : %d\n", pst_lmep->mepIndex);
    sal_fprintf(fp, "Remote mep index        : %d\n", pst_lmep->rmepIndex);

    sal_fprintf(fp, "Nexthop id              : %d\n", pst_lmep->nh_id);

    sal_fprintf(fp, "LSP outlabel exp is set : %d\n", pst_lmep->lsp_outlabel_exp_is_set);
    sal_fprintf(fp, "LSP outlabel exp        : %d\n", pst_lmep->lsp_exp);
    sal_fprintf(fp, "LSP TTL                 : %d\n", pst_lmep->lsp_ttl);

    sal_fprintf(fp, "PW outlabel exp is set  : %d\n", pst_lmep->pw_outlabel_exp_is_set);
    sal_fprintf(fp, "PW outlabel exp         : %d\n", pst_lmep->pw_exp);
    sal_fprintf(fp, "PW TTL                  : %d\n", pst_lmep->pw_ttl);
    
    sal_fprintf(fp, "Level                   : %d\n", pst_lmep->level);
    sal_fprintf(fp, "Event bmp               : %d\n", pst_lmep->event_bmp);

    if (pst_lmep->aps_group_id)
    {
        if (GLB_OAM_TYPE_MPLS_LSP == pst_lmep->oam_type)
        {
            sal_fprintf(fp, "Lsp aps group id        : %d\n", pst_lmep->aps_group_id);
        }
        else if (GLB_OAM_TYPE_MPLS_PW == pst_lmep->oam_type)
        {
            if (pst_lmep->lsp_aps_group_id)
            {
                sal_fprintf(fp, "Lsp aps group id        : %d\n", pst_lmep->lsp_aps_group_id);    
            }
            
            sal_fprintf(fp, "Pw aps group id         : %d\n", pst_lmep->aps_group_id);
        }
    }
    
    sal_fprintf(fp, "RDI enable              : %d\n", pst_lmep->rdi_enable);
    sal_fprintf(fp, "IS working path         : %d\n", pst_lmep->is_working_path);
    
    sal_fprintf(fp, "Nexthop mac             : %2x:%2x:%2x:%2x:%2x:%2x\n", pst_lmep->nexthop_mac[0], pst_lmep->nexthop_mac[1], pst_lmep->nexthop_mac[2],
        pst_lmep->nexthop_mac[3], pst_lmep->nexthop_mac[4], pst_lmep->nexthop_mac[5]);
    #endif
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_all_mep(FILE *fp, uint32 oam_type)
{
    hsrv_openflow_tpoam_t*                  p_master = g_pst_openflow_tpoam;
    ctclib_hash_t*                                 pst_mep_hash = NULL;
    ctclib_hash_backet_t*                      hb;
    ctclib_hash_backet_t*                      next;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    uint32                                             i = 0;
    bool                                               first_mep = TRUE;

    if (GLB_OAM_TYPE_MPLS_SECTION == oam_type)
    {
        pst_mep_hash =  p_master->pst_section_lmep_hash;   
    }
    else if (GLB_OAM_TYPE_MPLS_LSP == oam_type)
    {
        pst_mep_hash =  p_master->pst_lsp_lmep_hash;     
    }
    else if (GLB_OAM_TYPE_MPLS_PW == oam_type)
    {
        pst_mep_hash =  p_master->pst_pw_lmep_hash;     
    }
    else
    {
        pst_mep_hash =  p_master->pst_lmep_hash;    
    }
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MEP------------------\n");

    if (0== pst_mep_hash->count)
    {
        sal_fprintf(fp, "N/A\n");    
    }

    for (i = 0; i < pst_mep_hash->size; i++)
    {
        for (hb = pst_mep_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            pst_lmep = (glb_openflow_tpoam_t *)hb->data;

            hsrv_tpoam_show_mep(fp, pst_lmep);

            sal_fprintf(fp, "----------------------------------------------------\n");

            if (first_mep)
            {
                first_mep = FALSE;
            }
        }
    }        

    if (first_mep)
    {
        sal_fprintf(fp, "----------------------------------------------------\n");
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_show_all_mip(FILE *fp)
{
    hsrv_openflow_tpoam_t*                  p_master = g_pst_openflow_tpoam;
    bool                                               first_mep = TRUE;
    glb_openflow_tpoam_t*                   pst_mip = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next_list = NULL;
    
    sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MIP------------------\n");

    if (0 == p_master->pst_mip_list->count)
    {
        sal_fprintf(fp, "N/A\n");    
    }
    
    CTCLIB_SLIST_LOOP_DEL(g_pst_openflow_tpoam->pst_mip_list, pst_mip, listnode, next_list)
    {
        hsrv_tpoam_show_mip(fp, pst_mip);

        sal_fprintf(fp, "----------------------------------------------------\n");

        if (first_mep)
        {
            first_mep = FALSE;
        }
    }

    if (first_mep)
    {
        sal_fprintf(fp, "----------------------------------------------------\n");
    }
    
    return HSRV_E_NONE;
}


int32
hsrv_tpoam_show_single_mep(FILE *fp, char* p_session_id)
{
    hsrv_openflow_tpoam_t*                  p_master = g_pst_openflow_tpoam;
    ctclib_hash_t*                                 pst_mep_hash = NULL;
    ctclib_hash_backet_t*                      hb;
    ctclib_hash_backet_t*                      next;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    uint32                                             i = 0;
    bool                                               first_mep = TRUE;
    uint32                                            session_id = 0;

    sal_sscanf(p_session_id, "%d", &session_id);

    pst_mep_hash =  p_master->pst_lmep_hash;    
    
    //sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MEP------------------\n");

    if (0== pst_mep_hash->count)
    {
        //sal_fprintf(fp, "N/A\n");    
    }

    for (i = 0; i < pst_mep_hash->size; i++)
    {
        for (hb = pst_mep_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            pst_lmep = (glb_openflow_tpoam_t *)hb->data;

            if (pst_lmep->oam_session_id != session_id)
            {
                continue;    
            }
            sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MEP------------------\n");

            hsrv_tpoam_show_mep(fp, pst_lmep);

            sal_fprintf(fp, "----------------------------------------------------\n");

            if (first_mep)
            {
                first_mep = FALSE;
                break;
            }
        }

        if (!first_mep)
        {
            break;    
        }
    }        

    if (first_mep)
    {
        //sal_fprintf(fp, "----------------------------------------------------\n");
    }

    first_mep = TRUE;
    glb_openflow_tpoam_t*                   pst_mip = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next_list = NULL;
    CTCLIB_SLIST_LOOP_DEL(g_pst_openflow_tpoam->pst_mip_list, pst_mip, listnode, next_list)
    {
        if (pst_mip->oam_session_id != session_id)
        {
            continue;
        }

        sal_fprintf(fp, "-------------HSRV OPENFLOW TPOAM MIP------------------\n");

        hsrv_tpoam_show_mip(fp, pst_mip);
        sal_fprintf(fp, "----------------------------------------------------\n");

        if (first_mep)
        {
            first_mep = FALSE;
            break;
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_openflow_show_tpoam(FILE* fp, char** argv, int32 argc)
{
     if (!sal_strcmp(argv[0], "all"))
     {
         hsrv_tpoam_show_brief(fp);
         hsrv_tpoam_show_meg(fp);
         hsrv_tpoam_show_all_mep(fp, GLB_OAM_TYPE_MPLS_ALL);
         hsrv_tpoam_show_all_mip(fp);
     }
     else if (!sal_strcmp(argv[0], "brief"))
     {
         hsrv_tpoam_show_brief(fp);
     }
     else if (!sal_strcmp(argv[0], "meg"))
     {
         hsrv_tpoam_show_meg(fp);
     }
     else if (!sal_strcmp(argv[0], "mip"))
     {
         hsrv_tpoam_show_all_mip(fp);
     }
     else if (!sal_strcmp(argv[0], "mep"))
     {
         if (!sal_strcmp(argv[1], "all"))
         {
             hsrv_tpoam_show_all_mep(fp, GLB_OAM_TYPE_MPLS_ALL);
         }
         else if (!sal_strcmp(argv[1], "section"))
         {
             hsrv_tpoam_show_all_mep(fp, GLB_OAM_TYPE_MPLS_SECTION);
         }
         else if (!sal_strcmp(argv[1], "lsp"))
         {
             hsrv_tpoam_show_all_mep(fp, GLB_OAM_TYPE_MPLS_LSP);
         }
         else if (!sal_strcmp(argv[1], "pw"))
         {
             hsrv_tpoam_show_all_mep(fp, GLB_OAM_TYPE_MPLS_PW);
         }
         else
         {
              hsrv_tpoam_show_single_mep(fp, argv[1]);   
         }
     }
        
    return HSRV_E_NONE;
}

int32
hsrv_tpoam_update_pw_mep_nh_by_groupid(uint32   lsp_aps_group_id)
{
    hsrv_openflow_tpoam_t*                  p_master = g_pst_openflow_tpoam;
    ctclib_hash_t*                                 pst_mep_hash = NULL;
    ctclib_hash_backet_t*                      hb;
    ctclib_hash_backet_t*                      next;
    glb_openflow_tpoam_t*                    pst_lmep = NULL;
    uint32                                             i = 0;
    Hsrv2AgtMsg_t                                hagt_msg;
    HalMsgTpoamAddOamReq_t*             pst_msg = NULL;

    pst_mep_hash =  p_master->pst_pw_lmep_hash;     
    
    if (0== pst_mep_hash->count)
    {
        return HSRV_E_NONE;
    }

    for (i = 0; i < pst_mep_hash->size; i++)
    {
        for (hb = pst_mep_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            pst_lmep = (glb_openflow_tpoam_t *)hb->data;

            if (pst_lmep->lsp_aps_group_id == lsp_aps_group_id)
            {
                /* add oam */
                sal_memset(&hagt_msg, 0, sizeof(Hsrv2AgtMsg_t));
                hagt_msg.opHsrv2Agt.present = opHsrv2Agt_PR_hsrv2AgtMsgTpoamUpdateOam;
                pst_msg = &hagt_msg.opHsrv2Agt.hsrv2AgtMsgTpoamUpdateOam;
            
                /* update flag & enable */
                pst_msg->flag = GLB_TPOAM_UPDATE_NHOP;
                
                /* key */
                pst_msg->key.oamType = GLB_OAM_TYPE_MPLS_PW;
                pst_msg->key.id = pst_lmep->inlabel;
            
                pst_msg->key.megid.buf = pst_lmep->megid;
                pst_msg->key.megid.size = GLB_TPOAM_MEGID_LEN;
                pst_msg->key.mepid = pst_lmep->mepid;
                pst_msg->mepIndex = pst_lmep->mepIndex;           
                pst_msg->nhid = pst_lmep->nh_id;
            
                HSRV_AGENTID_SET_ALL(hagt_msg.agentId);
                HSRV_IF_ERROR_RETURN(HSRV2AGT_MSG_SEND(&hagt_msg));
            }
        }
    }        

    return HSRV_E_NONE;
}

int32
hsrv_openflow_tpoam_start(void)
{
    g_pst_openflow_tpoam = HSRV_CALLOC(MEM_FEA_HSRV_MODULE, sizeof(hsrv_openflow_tpoam_t));
    HSRV_OPENFLOW_MEM_CHECK(g_pst_openflow_tpoam);

    g_pst_openflow_tpoam->pst_mep_opf = ctclib_opf_init(1);
    ctclib_opf_init_offset(g_pst_openflow_tpoam->pst_mep_opf, 0, GLB_TPOAM_MEP_INDEX_MIN, 
        GLB_TPOAM_MEP_INDEX_MAX);

    /* init hash table */
    g_pst_openflow_tpoam->pst_megid_hash = ctclib_hash_create_size(HSRV_TPOAM_MEGID_HASH_SIZE,
        _hsrv_tpoam_megid_hash_key, _hsrv_tpoam_megid_hash_cmp);
    
    g_pst_openflow_tpoam->pst_section_lmep_hash = ctclib_hash_create_size(HSRV_TPOAM_LMEP_HASH_SIZE, 
        _hsrv_tpoam_port_hash_key, _hsrv_tpoam_port_hash_cmp);
    
    g_pst_openflow_tpoam->pst_lsp_lmep_hash = ctclib_hash_create_size(HSRV_TPOAM_LMEP_HASH_SIZE, 
        _hsrv_tpoam_label_hash_key, _hsrv_tpoam_label_hash_cmp);

    g_pst_openflow_tpoam->pst_pw_lmep_hash = ctclib_hash_create_size(HSRV_TPOAM_LMEP_HASH_SIZE, 
        _hsrv_tpoam_label_hash_key, _hsrv_tpoam_label_hash_cmp);

    g_pst_openflow_tpoam->pst_lmep_hash = ctclib_hash_create_size(HSRV_TPOAM_LMEP_HASH_SIZE, 
        _hsrv_tpoam_mepindex_hash_key, _hsrv_tpoam_mepindex_hash_cmp);

    /*MIP*/
    //g_pst_openflow_tpoam->pst_mip_hash = ctclib_hash_create_size(HSRV_TPOAM_LMEP_HASH_SIZE, 
        //_hsrv_tpoam_mip_hash_key, _hsrv_tpoam_mip_hash_cmp);

    g_pst_openflow_tpoam->pst_mip_list = ctclib_slist_create(NULL, NULL);

    /*3. init APS group offset opf*/
    g_pst_openflow_tpoam->pst_aps_offset_opf = ctclib_opf_init(1);
    ctclib_opf_init_offset (g_pst_openflow_tpoam->pst_aps_offset_opf, 0, HSRV_APS_OFFSET_MIN, HSRV_APS_OFFSET_MAX);

    cdb_pm_subscribe_tbl(TBL_TPOAM_GLOBAL, NULL, _hsrv_process_sync_tpoam_global_after_cb);

    //cdb_pm_subscribe_tbl(TBL_OAM_SESSION, NULL, _hsrv_process_sync_oam_session_after_cb);

    g_pst_openflow_tpoam->fast_aps_en = TRUE;
    
    return HSRV_E_NONE;
}
