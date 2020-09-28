/*
 * snmpd.h
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _CENTEC_ /* Modified by Alexander 2014-12-10 08:26 */
typedef enum
{
    E_AGENT_VERSION_ALL = 0,
    E_AGENT_VERSION_V1,
    E_AGENT_VERSION_V2C,
    E_AGENT_VERSION_V3
} agent_version_e;

int check_agent_version(long version);
#endif /* !_CENTEC_ */

#define MASTER_AGENT 0
#define SUB_AGENT    1
extern int      agent_role;

extern int      snmp_dump_packet;
extern int      verbose;
extern int      (*sd_handlers[]) (int);
extern int      smux_listen_sd;

extern int      snmp_read_packet(int);

/*
 * config file parsing routines 
 */
void            agentBoots_conf(char *, char *);

#ifdef __cplusplus
}
#endif
