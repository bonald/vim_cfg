#ifndef __HSRV_OPENFLOW_TTP_PORT_H__
#define __HSRV_OPENFLOW_TTP_PORT_H__

#define HSRV_OPENFLOW_PORT_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_PORT, size)
#define HSRV_OPENFLOW_PORT_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_PORT, ptr);  \
    ptr = NULL; \
}while(0)

struct hsrv_openflow_ttp_port_info_s
{
    uint32 ifindex;
    uint32 gport;
    uint32 group_id;
    uint32 class_id;
};
typedef struct hsrv_openflow_ttp_port_info_s hsrv_openflow_ttp_port_info_t;


hsrv_openflow_ttp_port_info_t*
hsrv_openflow_port_get_port_info_by_ifindex(uint32 ifindex);

hsrv_openflow_ttp_port_info_t*
hsrv_openflow_port_get_port_info_by_portid(uint32 gport);

int32
hsrv_openflow_port_start(void);

#endif
