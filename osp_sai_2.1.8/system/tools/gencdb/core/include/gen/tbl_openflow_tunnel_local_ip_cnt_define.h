
#ifndef __TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_DEFINE_H__
#define __TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_DEFINE_H__

/* TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT field defines */
typedef enum
{
    TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_KEY                  = 0 ,  /* READ */
    TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_LOCAL_IP_REF         = 1 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_FLD_MAX                  = 2 
} tbl_openflow_tunnel_local_ip_cnt_field_id_t;

/* TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT defines */
typedef struct
{
    char                 local_ip[IPADDR_BUF_SIZE];
} tbl_openflow_tunnel_local_ip_cnt_key_t;

typedef struct
{
    tbl_openflow_tunnel_local_ip_cnt_key_t key;
    uint32               local_ip_ref;
} tbl_openflow_tunnel_local_ip_cnt_t;

typedef struct
{
    ctclib_hash_t        *openflow_tunnel_local_ip_hash;
} tbl_openflow_tunnel_local_ip_cnt_master_t;

#endif /* !__TBL_OPENFLOW_TUNNEL_LOCAL_IP_CNT_DEFINE_H__ */

