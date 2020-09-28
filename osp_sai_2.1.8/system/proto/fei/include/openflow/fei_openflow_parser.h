#ifndef __FEI_OPENFLOW_PARSER_H__
#define __FEI_OPENFLOW_PARSER_H__

int32
fei_parser_set_ecmp_hash_key(uint32 hash_key);

int32
fei_parser_set_linkagg_hash_key(uint32 hash_use, uint32 hash_key);

int32
fei_parser_set_mpls_parser(uint8 enable);

#endif /* !__FEI_OPENFLOW_PARSER_H__ */
