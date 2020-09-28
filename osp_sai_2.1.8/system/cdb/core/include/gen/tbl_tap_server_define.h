
#ifndef __TBL_TAP_SERVER_DEFINE_H__
#define __TBL_TAP_SERVER_DEFINE_H__

/* TBL_TAP_SERVER field defines */
typedef enum
{
    TBL_TAP_SERVER_FLD_SERVER_SOCK          = 0 ,  /* READ */
    TBL_TAP_SERVER_FLD_CLIENT_SOCK          = 1 ,  /* READ */
    TBL_TAP_SERVER_FLD_PEER_ADDR            = 2 ,  /* READ */
    TBL_TAP_SERVER_FLD_LASTREAD             = 3 ,  /* READ */
    TBL_TAP_SERVER_FLD_DEBUG                = 4 ,  /* RW */
    TBL_TAP_SERVER_FLD_CLIENT_PEER_ADDR     = 5 ,  /* RW */
    TBL_TAP_SERVER_FLD_CLIENT_TEST          = 6 ,  /* RW */
    TBL_TAP_SERVER_FLD_MAX                  = 7 
} tbl_tap_server_field_id_t;

/* TBL_TAP_SERVER defines */
typedef struct
{
    int32                server_sock;
    int32                client_sock;
    addr_ipv4_t          peer_addr;
    sal_time_t           lastread;
    uint32               debug;
    addr_ipv4_t          client_peer_addr;
    uint32               client_test;
} tbl_tap_server_t;

#endif /* !__TBL_TAP_SERVER_DEFINE_H__ */

