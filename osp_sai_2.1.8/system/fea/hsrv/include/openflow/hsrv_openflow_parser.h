#ifndef __HSRV_OPENFLOW_PARSER_H__
#define __HSRV_OPENFLOW_PARSER_H__

int32
fei_api_openflow_set_ecmp_hash_key(fei_openflow_parser_set_ecmp_hash_key_req_t* pst_req);

int32
fei_api_openflow_set_linkagg_hash_key(fei_openflow_parser_set_linkagg_hash_key_req_t* pst_req);

int32
fei_api_openflow_set_mpls_parser(fei_openflow_parser_set_mpls_parser_req_t* pst_req);

#endif /* !__HSRV_OPENFLOW_PARSER_H__ */
