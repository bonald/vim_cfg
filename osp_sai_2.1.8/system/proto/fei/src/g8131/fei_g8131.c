#include "sal.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "proto.h"
#include "glb_g8131_define.h"
#include "lib.h"
#include "fei.h"
#include "fei_g8131.h"
#include "hsrv_msg.h"


int32
fei_g8131_mpls_fast_aps_en(uint32 aps_type, u_int32_t aps_group_id, uint32 fast_aps_en)
{
    fei_g8131_mpls_fast_aps_req_t req;
    uint32 req_len;
    int32  rc;

    sal_memset(&req, 0, sizeof(fei_g8131_mpls_fast_aps_req_t));
    req_len = sizeof(fei_g8131_mpls_fast_aps_req_t);
    req.aps_type = aps_type;
    req.aps_group_id = aps_group_id;
    req.fast_aps_en = fast_aps_en;
    rc = fei_talk(FEI_G8131_MPLS_FAST_APS_EN, &req, req_len);
    return rc;
}

int32
fei_g8131_mpls_protection_en(uint32 aps_type, uint32_t aps_group_id, uint32 protection_en)
{
    fei_g8131_mpls_protection_en_req_t req;
    uint32 req_len;
    int32  rc;

    sal_memset(&req, 0, sizeof(fei_g8131_mpls_protection_en_req_t));
    req_len = sizeof(fei_g8131_mpls_protection_en_req_t);
    req.aps_type = aps_type;
    req.aps_group_id = aps_group_id;
    req.protection_en = protection_en;
    rc = fei_talk(FEI_G8131_MPLS_PROTECTION_EN, &req, req_len);
    return rc;
}