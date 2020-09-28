
#ifndef __SNMPAGT_H__
#define __SNMPAGT_H__

extern int32
cdb_int_str2val(const char *str, int32 *ret);

struct counter64
snmpagt_u64_to_c64(uint64 u64);

/* update */
int32
mib2_system_update_syscontact(char *str);
int32
mib2_system_update_syslocation(char *str);
int32
mib2_system_update_sysname(char *str);

int32
priv_system_update_mem(uint32 total, uint32 used, uint32 free);
int32
priv_mgmt_update_mgmtip(char *str);
int32
priv_mgmt_update_mgmtgateway(addr_ipv4_t gw);

/* Nodes init */
int32
priv_operate_init();
int32
priv_system_init();
int32
priv_mgmt_init();
int32
priv_device_init();

int32
rmon_table_init();

int32
mib2_system_init();

/* TABLEs init */
int32
if_table_init();

int32
ifx_table_init();

int32
interface_table_init();

int32
mib2_entPhysicalTable_init();

int32
snmpagt_get_ifg_stats_en(uint32 ifindex);

int32
snmpagt_set_ifg_stats_en(tbl_interface_t *p_if);

oid
snmpagt_get_en_oid();

int32
snmpagt_str2oid(char *oid_str, oid *oids, size_t *oid_len);

int32
snmpagt_oid2str(oid *oids, size_t oid_len, char *oid_str, size_t str_len);

int32
snmpagt_agent_init();

int32
snmpagt_sub_init(uint32 sync);

int32
snmpagt_sub_resync();

int32
snmpagt_init(uint32 debug);

#endif /* !__SNMPAGT_H__ */

