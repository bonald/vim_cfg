#ifndef __HSRV_SYNC_IPSG_H__
#define __HSRV_SYNC_IPSG_H__

#include "hsrv_debug.h"

/*For log*/
#define HSRV_IP_BINDING_LOG_EMERG HSRV_LOG_EMERG
#define HSRV_IP_BINDING_LOG_ALERT HSRV_LOG_ALERT 
#define HSRV_IP_BINDING_LOG_CRIT HSRV_LOG_CRIT
#define HSRV_IP_BINDING_LOG_ERR HSRV_LOG_ERR
#define HSRV_IP_BINDING_LOG_WARN HSRV_LOG_WARN
#define HSRV_IP_BINDINGLOG_NOTICE HSRV_LOG_NOTICE
#define HSRV_IP_BINDING_LOG_INFO HSRV_LOG_INFO

/*For debug*/
#define HSRV_IP_BINDING_LOG_DEBUG(fmt, args...)\
    HSRV_LOG_DEBUG(ipsour, IPSOUR_IPSOUR, fmt, ##args)

/*For debug in special case*/
#define HSRV_IP_BINDING_MEM_LOG()\
    HSRV_IP_BINDING_LOG_CRIT("malloc failed, %s:%d", __FUNCTION__, __LINE__)

#define HSRV_IP_BINDING_FUNC_LOG()\
    HSRV_IP_BINDING_LOG_DEBUG("-----------------------------------------------------------")\
    HSRV_IP_BINDING_LOG_DEBUG("Enter the function <%s>. Line <%d>", __FUNCTION__, __LINE__)

int32
hsrv_ip_source_guard_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);

#endif /* !__HSRV_SYNC_VLAN_H__ */
