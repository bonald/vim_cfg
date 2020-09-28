
#ifndef __DHCSNOOPING_H__
#define __DHCSNOOPING_H__

#define M_TRIM_CR(szLine) \
do { \
    char *_p = szLine + strlen(szLine) - 1; \
    if (*_p == '\n') { \
        *_p = '\0'; \
        _p--; \
    } \
    if (*_p == '\r') { \
        *_p = '\0'; \
    } \
} while (0); \

    
#ifndef ITEM_CB_FUNC
typedef void (*ITEM_CB_FUNC) (void *, void *);
#endif

#define DHCBINDING_FILTER_INTERFACE     1
#define DHCBINDING_FILTER_VLAN          2
#define DHCBINDING_FILTER_IP            3
#define DHCBINDING_FILTER_MAC           4
#define DHCBINDING_FILTER_ALL           5

/*for bug 21259 modified by liuyang 2012-12-27*/
typedef struct stCmpArg
{
    int bIPv6;
    int cType;
    void *pArg;
    int cmpType;
    ITEM_CB_FUNC cb;
    void *pCbArg;
} S_CmpArg;

int32
dhcsnooping_tbl_init();

int32
dhcsnooping_proc_init_done();

int32
dhcsnooping_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

int32
dhcsnooping_ipsour_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds);

#endif /* !__DHCSNOOPING_H__ */

