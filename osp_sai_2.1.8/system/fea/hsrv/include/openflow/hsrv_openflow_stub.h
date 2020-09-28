
#ifndef __HSRV_OPENFLOW_STUB_H__
#define __HSRV_OPENFLOW_STUB_H__

struct hsrv_openflow_stub_data_s
{
    uint32 ifindex;    /* stub ifindex */
    bool enable;
};
typedef struct hsrv_openflow_stub_data_s hsrv_openflow_stub_data_t;

int32
hsrv_openflow_stub_lookup_info(uint32 ifindex, hsrv_openflow_stub_data_t **pp_info);

bool
hsrv_openflow_port_is_stub(uint32 stub_ifindex);

bool
hsrv_openflow_stub_get_link_status(uint32 stub_ifindex);

int32
hsrv_openflow_stub_start(void);

int32 
fei_api_openflow_stub_create(fei_openflow_stub_create_t *pst_req);

int32 
fei_api_openflow_stub_enable(fei_openflow_stub_enable_t *pst_req);

#endif /* !__HSRV_OPENFLOW_PORT_H__ */

