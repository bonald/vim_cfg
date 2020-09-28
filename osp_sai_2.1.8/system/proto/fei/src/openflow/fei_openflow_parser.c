#include "proto.h"
#include "glb_macro.h"
#include "glb_const.h"
#include "glb_openflow_define.h"

#include "fei.h"

#include "hsrv_msg.h"
#include "hsrv_msg_openflow.h"

#include "fei_openflow_parser.h"

int32
fei_parser_set_ecmp_hash_key(uint32 hash_key)
{
    fei_openflow_parser_set_ecmp_hash_key_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_parser_set_ecmp_hash_key_req_t));
    req.hash_key = hash_key;
    return fei_talk(FEI_OPENFLOW_PARSER_SET_ECMP_HASH_KEY, &req, sizeof(req));
}

int32
fei_parser_set_linkagg_hash_key(uint32 hash_use, uint32 hash_key)
{
    fei_openflow_parser_set_linkagg_hash_key_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_parser_set_linkagg_hash_key_req_t));
    req.hash_use = hash_use;
    req.hash_key = hash_key;
    return fei_talk(FEI_OPENFLOW_PARSER_SET_LINKAGG_HASH_KEY, &req, sizeof(req));
}

int32
fei_parser_set_mpls_parser(uint8 enable)
{
    fei_openflow_parser_set_mpls_parser_req_t req;

    sal_memset(&req, 0, sizeof(fei_openflow_parser_set_mpls_parser_req_t));
    req.enable = enable;
    return fei_talk(FEI_OPENFLOW_PARSER_SET_MPLS_PARSER, &req, sizeof(req));
}


