#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_openflow_define.h"
#include "lib.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_flow.h"

#include "gen/tbl_tpoam_global_define.h"
#include "gen/tbl_tpoam_global.h"
#include "gen/tbl_oam_session_define.h"
#include "gen/tbl_oam_session.h"

#include "gen/tbl_lsp_pe_define.h"
#include "gen/tbl_lsp_pe.h"
#include "gen/tbl_pw_tpe_define.h"
#include "gen/tbl_pw_tpe.h"
#include "gen/tbl_section_oam_define.h"
#include "gen/tbl_section_oam.h"
#include "gen/tbl_lsp_p_define.h"
#include "gen/tbl_lsp_p.h"

int32
fei_flow_add_tpoam_flow(glb_openflow_tpoam_t* p_flow)
{
    fei_openflow_add_tpoam_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_add_tpoam_req_t);
    p_req = malloc(req_len);
    sal_memcpy(&p_req->flow, p_flow, req_len);

    rc = fei_talk(FEI_OPENFLOW_FLOW_ADD_TPOAM, p_req, req_len);
    free(p_req);
    return rc;
}

int32
fei_flow_del_tpoam_flow(glb_openflow_tpoam_t* p_flow)
{
    fei_openflow_del_tpoam_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_del_tpoam_req_t);
    p_req = malloc(req_len);
    sal_memcpy(&p_req->flow, p_flow, req_len);

    rc = fei_talk(FEI_OPENFLOW_FLOW_DEL_TPOAM, p_req, req_len);
    free(p_req);
    return rc;
}

int32
fei_flow_update_tpoam_flow(glb_openflow_tpoam_t* p_flow)
{
    fei_openflow_update_tpoam_req_t* p_req;
    uint32 req_len;
    int32  rc;

    req_len = sizeof(fei_openflow_update_tpoam_req_t);
    p_req = malloc(req_len);
    sal_memcpy(&p_req->flow, p_flow, req_len);

    rc = fei_talk(FEI_OPENFLOW_FLOW_UPDATE_TPOAM, p_req, req_len);
    free(p_req);
    return rc;
}

int32
fei_set_pw_mode(uint32* p_value)
{
    uint32 req_len;
    int32  rc;

    req_len = sizeof(uint32);
    
    rc = fei_talk(FEI_OPENFLOW_SET_PW_MODE, p_value, req_len);

    return rc;
}

tbl_section_oam_t*
tbl_section_oam_get_section_oam_by_ifindex(uint32 ifindex)
{
    tbl_section_oam_key_t section_oam_key;

    section_oam_key.ifindex = ifindex;
    return tbl_section_oam_get_section_oam(&section_oam_key);
}

tbl_lsp_pe_t*
tbl_lsp_pe_get_lsp_pe_by_name(char * name)
{
    tbl_lsp_pe_key_t lsp_pe_key;

    sal_memset(&lsp_pe_key, 0x00, sizeof(lsp_pe_key));
    sal_strcpy(lsp_pe_key.name, name);
    return tbl_lsp_pe_get_lsp_pe(&lsp_pe_key);
}

tbl_lsp_p_t*
tbl_lsp_p_get_lsp_p_by_name(char * name)
{
    tbl_lsp_p_key_t lsp_p_key;

    sal_memset(&lsp_p_key, 0x00, sizeof(lsp_p_key));
    sal_strcpy(lsp_p_key.name, name);
    return tbl_lsp_p_get_lsp_p(&lsp_p_key);
}

tbl_pw_tpe_t*
tbl_pw_tpe_get_pw_tpe_by_name(char * name)
{
    tbl_pw_tpe_key_t pw_tpe_key;

    sal_memset(&pw_tpe_key, 0x00, sizeof(pw_tpe_key));
    sal_strcpy(pw_tpe_key.name, name);
    return tbl_pw_tpe_get_pw_tpe(&pw_tpe_key);
}

int32
tpoam_api_get_oam_flow(glb_openflow_tpoam_t  *oam_flow, tbl_oam_session_t* p_db_oam_session)
{
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_pw_tpe_t * pst_tpe = NULL;

    sal_memset(oam_flow, 0x0, sizeof(glb_openflow_tpoam_t));

    oam_flow->oam_type = p_db_oam_session->oam_type;

    switch (p_db_oam_session->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        /*section*/
        oam_flow->out_ifindex = p_db_oam_session->bind_oam_ifindex;//?
        oam_flow->out_ifindex = p_db_oam_session->service_ifindex;//?
        break;
    case GLB_OAM_TYPE_MPLS_LSP:
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(p_db_oam_session->service_name);
        if (NULL == pst_lsp_pe)
        {
            return PM_E_FAIL;
        }
        
        /*lsp*/
        oam_flow->inlabel = pst_lsp_pe->inlabel;
        oam_flow->outlabel_lsp = pst_lsp_pe->outlabel;
        oam_flow->out_ifindex = pst_lsp_pe->ifindex;

        /*
        oam_flow.inlabel = p_data->inlabel;
        oam_flow.outlabel_lsp = p_data->outlabel1;
        oam_flow.out_ifindex = ifindex;*/
        break;
    case GLB_OAM_TYPE_MPLS_PW:
        /*pw*/
        pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(p_db_oam_session->service_name);
        if (NULL == pst_tpe)
        {
            return PM_E_FAIL;
        }
        oam_flow->inlabel = pst_tpe->inlabel;
        //oam_flow.outlabel_lsp = pst_tpe->outlabel1;
        //oam_flow.outlabel_pw = pst_tpe->outlabel0;
        //oam_flow.out_ifindex = pst_tpe->;
        
        /*
        oam_flow.inlabel = p_data->inlabel;
        oam_flow.outlabel_lsp = p_data->outlabel1;
        oam_flow.outlabel_pw = p_data->outlabel0;
        oam_flow.out_ifindex = ifindex;*/
        break;
    }
    
    return PM_E_NONE;
}

int32
tpoam_api_get_fei_key(tbl_oam_session_t* p_oam_session, glb_openflow_tpoam_t  *p_oam_flow)
{
    tbl_section_oam_t *pst_section = NULL;
    tbl_lsp_pe_t *pst_lsp_pe = NULL;
    tbl_pw_tpe_t * pst_tpe = NULL;

    sal_memset(p_oam_flow, 0x0, sizeof(glb_openflow_tpoam_t));
    
    p_oam_flow->oam_type = p_oam_session->oam_type;
    switch (p_oam_session->oam_type)
    {
    case GLB_OAM_TYPE_MPLS_SECTION:
        pst_section = tbl_section_oam_get_section_oam_by_ifindex(p_oam_session->service_ifindex);
        if (NULL != pst_section)
        {
            p_oam_flow->out_ifindex = p_oam_session->bind_oam_ifindex;//?
            //oam_flow.out_ifindex = pst_section->key.ifindex;
        }
        break;
    case GLB_OAM_TYPE_MPLS_LSP:
        pst_lsp_pe = tbl_lsp_pe_get_lsp_pe_by_name(p_oam_session->service_name);
        if (NULL != pst_lsp_pe)
        {
            p_oam_flow->inlabel = pst_lsp_pe->inlabel;
            p_oam_flow->out_ifindex = pst_lsp_pe->ifindex;
        }
        break;
    case GLB_OAM_TYPE_MPLS_PW:
        pst_tpe = tbl_pw_tpe_get_pw_tpe_by_name(p_oam_session->service_name);
        if (NULL != pst_tpe)
        {
            p_oam_flow->inlabel = pst_tpe->inlabel;
        }
        break;
    default:
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}


