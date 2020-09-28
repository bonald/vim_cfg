
#ifndef __SDB_H__
#define __SDB_H__

#include "sdb_include.h"
#include "sdb_defines.h"
#include "sdb_ds.h"
#include "gen/sdb_define.h"
#include "gen/stbl_port_define.h"

int32
sdb_get_field_id(char *field_name, sdb_node_t* p_node, sdb_field_t **pp_field);

int32
stbl_hash_iter_adpt_fn(ctclib_hash_backet_t* phb, void *parg1, void *p_arg2);

const char*
sdb_enum_val2str(const char **array, uint32 array_size, uint32 value);

#endif /* !__SDB_H__ */

