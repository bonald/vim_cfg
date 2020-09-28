#ifndef __HSRV_OPENFLOW_PORT_H__
#define __HSRV_OPENFLOW_PORT_H__

#define HSRV_OF_PHY_PORT_LABEL(gport)   \
    (g_pst_openflow_port_master->phy_port_label_base + gport)

/* hsrv openflow memory port related macros */
#define HSRV_OPENFLOW_PORT_MALLOC(size) HSRV_MALLOC(MEM_FEA_HSRV_OPENFLOW_PORT, size)
#define HSRV_OPENFLOW_PORT_CALLOC(size) HSRV_CALLOC(MEM_FEA_HSRV_OPENFLOW_PORT, size)
#define HSRV_OPENFLOW_PORT_FREE(ptr) \
do { \
    HSRV_FREE(MEM_FEA_HSRV_OPENFLOW_PORT, ptr);  \
    ptr = NULL; \
}while(0)

/* Added by weizj for tunnel */
#define HSRV_OPENFLOW_PORT_L3IFID_BASE     1
#define HSRV_OPENFLOW_BOND_L3IFID_BASE     100
#define HSRV_OPENFLOW_PORT_INVALID_VLANID  0xFFFF

/* Added by weizj for bug 36414 */
#define HSRV_OPENFLOW_PORT_LABEL_MAX       256

enum hsrv_openflow_port_config {
    HSRV_OPENFLOW_PORT_DOWN    = 1 << 0,  /* Port is administratively down. */
    HSRV_OPENFLOW_NO_RECV      = 1 << 2,  /* Drop all packets received by port. */
    HSRV_OPENFLOW_NO_FWD       = 1 << 5,  /* Drop packets forwarded to port. */
    HSRV_OPENFLOW_NO_PACKET_IN = 1 << 6   /* Do not send packet-in msgs for port. */
};

struct hsrv_openflow_port_data_s
{
    bool enable;
    bool ignore_vlan_filter;
    bool link;
    uint32 port_config;
    uint16 l3ifid;
    uint16 rsv0;
    
    ctclib_list_t list;           /**< hsrv_group_watch_port_info_t */

    hsrv_of_nh_info_t nh_info;
    uint32 bond_ifindex;          /* bond ifindex, if this physical port is bond's slave */
    uint32 port_label;            /* Added by weizj for bug 36414 */
    uint32 flag;                  /* hsrv_port_of_flag_t */
};
typedef struct hsrv_openflow_port_data_s hsrv_openflow_port_data_t;

/* Added by weizj for udf */
struct hsrv_openflow_port_udf_s
{
    uint32 ifindex;
    uint16 gport;
};
typedef struct hsrv_openflow_port_udf_s hsrv_openflow_port_udf_t;
/* End by weizj for udf */

struct hsrv_openflow_port_master_s
{
    /* Added by weizj for udf */
    ctclib_hash_t* openflow_port_udf_hash;
    /* Added by weizj for bug 36414 */
    ctclib_opf_t*  port_lable_opf;           /* port label pool */

    uint32 phy_port_label_base;
};
typedef struct hsrv_openflow_port_master_s hsrv_openflow_port_master_t;

/* Added by weizj for udf */
#define HSRV_OPENFLOW_PORT_UDF_HASH          g_pst_openflow_port_master->openflow_port_udf_hash
/* Added by weizj for bug 36414 */
#define HSRV_OPENFLOW_PORT_LABLE_OPF         g_pst_openflow_port_master->port_lable_opf

int32
hsrv_openflow_port_alloc_port_label(hsrv_openflow_port_data_t* p_port_data);

int32
hsrv_openflow_port_free_port_label(uint32 port_label);

int32
hsrv_openflow_msg_port_set_label(uint32 ifindex, uint32 label);

bool
hsrv_openflow_port_is_enabled(uint32 ifindex);

bool
hsrv_openflow_port_is_add_to_bridge(uint32 ifindex);

bool
hsrv_openflow_port_get_link_status(uint32 ifindex);

/* Added by weizj for udf */
int32
hsrv_openflow_msg_port_enable(uint32 ifindex, bool enable, uint32 label, uint16 l3ifd);

int32
hsrv_openflow_port_set_udf_enable(void);

int32
hsrv_openflow_port_set_udf_disable(void);
/* End by weizj for udf */

int32
fei_api_openflow_port_operation_agg(fei_openflow_port_operation_agg_t *pst_req);

int32
hsrv_openflow_show_port(FILE *fp);

int32
hsrv_openflow_port_start(void);

int32
hsrv_openflow_port_init(hsrv_system_spec_info_t* p_system_info);

#endif /* !__HSRV_OPENFLOW_PORT_H__ */
