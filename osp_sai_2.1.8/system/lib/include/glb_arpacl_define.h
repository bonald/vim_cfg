#ifndef __GLB_ARPACL_DEFINE_H__
#define __GLB_ARPACL_DEFINE_H__

/* arpinspection */
#define GLB_ARPINSP_ACL_CLEAR      0
#define GLB_ARPINSP_ACL_STATIC     1
#define GLB_ARPINSP_ACL_DYNAMIC    2

#define GLB_ARPINSP_LOG_ACL_MACTH         1
#define GLB_ARPINSP_LOG_DHCP_BINGDING     2

#define GLB_ARPINSP_FLAG_VERIFY_SRCMAC    0x02
#define GLB_ARPINSP_FLAG_VERIFY_DESTMAC   0x04
#define GLB_ARPINSP_FLAG_VERIFY_IP        0x08

#define GLB_ARPINSP_LOGBUF_DEFAULT_NUM     32
#define GLB_ARPINSP_LOGBUF_MIN_NUM         0
#define GLB_ARPINSP_LOGBUF_MAX_NUM         1024
#define GLB_ARPINSP_LOGLIST_DEFAULT_NUM     5
#define GLB_ARPINSP_LOGLIST_DEFAULT_INTERVAL     1

#define GLB_ACC_ARP_LIST_NUM_MAX        64
#define GLB_ACC_ARP_ACE_NUM_MAX         512

#define GLB_ARPACL_MAX_ACE_SEQUENCE_NUMBER   65535 
#define GLB_ARPACL_MIN_ACE_SEQUENCE_NUMBER   1

#define GLB_ARP_REQUEST   1
#define GLB_ARP_RESPONSE  2
#define GLB_ARP_TYPE_ALL  3

/* data width log filter type */
enum log_filtering_e{
    GLB_ACL_DHCP_DENY = 0,
    GLB_ACL_MATCH_MATCHLOG,
    GLB_ACL_MATCH_NONE,
    GLB_DHCP_BINDINGS_ALL,
    GLB_DHCP_BINDINGS_NONE,
    GLB_DHCP_BINDINGS_PERMIT,
    GLB_ACL_DHCP_MAX,
}log_filtering_t;

/* ARP packet check return type */
enum arp_check_result_e
{
    ARPINSP_SUCCESS = 0,
    ARPINSP_IFP_DISABLE,
    ARPINSP_DIFF_SRCMAR_ERROR, /* the error include validation check and binding search */
    ARPINSP_DIFF_DESTMAR_ERROR,
    ARPINSP_DIFF_VLAN_ERROR,
    ARPINSP_DIFF_PORT_ERROR,
    ARPINSP_ERR_IP_ERROR,
    ARPINSP_PACERROR,
    ARPINSP_ACL_DENY,
    ARPINSP_ERROR,
    ARPINSP_NO_CHECK, 
};

enum arp_acl_back_s
{
  ARP_ACL_PERMIT,
  ARP_ACL_DENY,
  ARP_ACL_NOT_EXIT,
  ARP_ACL_NOT_MATCH,  
  ARP_BACK_MAX
};

enum arp_acl_err_s
{
  ARP_ACL_NONE ,
  ARP_ACL_IP_NOT_MATCH = -100,
  ARP_ACL_MAC_NOT_MATCH,  
  ARP_ACL_TYPE_NOT_MATCH,    
  ARP_ACL_TYPE_INVALID,
  ARP_ACL_NAME_INVALID
};

#endif /* __GLB_ACL_DEFINE_H__ */

