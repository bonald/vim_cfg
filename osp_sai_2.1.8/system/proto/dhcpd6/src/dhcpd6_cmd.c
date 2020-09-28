
#if 0
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "proto.h"
#include "dhcrelay.h"
#include "gen/tbl_dhcrelay_define.h"
#include "gen/tbl_dhcrelay.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "param_check.h"
#include "lib_fs.h"

extern int add_agent_options;
extern int g_dhcprelay_trustall;
extern int agent_relay_mode;
extern int drop_agent_mismatches;

extern void reset_dhcrelay_statistics();

/* --- dhcrelay --- */
static int
_check_dhcrelay_field(char *pszkey, char *pszValue, tbl_dhcrelay_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_DHCRELAY);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_dhcrelay_field_id_t)field_id) {
    case TBL_DHCRELAY_FLD_AGENT_PROC_OPTION:
        nValue = cdb_enum_str2val(agent_proc_option_strs, AGENT_PROC_OPTION_MAX, pszValue);
        if (-1 == nValue) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid agent_proc_option value"); 
            goto err_out;
        }
        pitem->agent_proc_option = nValue;
        break;
    case TBL_DHCRELAY_FLD_ENABLE:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid enable value"); 
            goto err_out;
        }
        pitem->enable = atoi(pszValue);
        break;
    case TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid drop_agent_mismatch value"); 
            goto err_out;
        }
        pitem->drop_agent_mismatch = atoi(pszValue);
        break;
    case TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid add_agent_options value"); 
            goto err_out;
        }
        pitem->add_agent_options = atoi(pszValue);
        break;
    case TBL_DHCRELAY_FLD_TRUST_ALL:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid trust_all value"); 
            goto err_out;
        }
        pitem->trust_all = atoi(pszValue);
        break;
    case TBL_DHCRELAY_FLD_MAX:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_dhcrelay_cmd_get(tbl_dhcrelay_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dhcrelay_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhcrelay_set(tbl_dhcrelay_field_id_t field_id, tbl_dhcrelay_t *pitem)
{
    int32 rc = PM_E_NONE;

    /* 1. process */
    switch (field_id) {
    case TBL_DHCRELAY_FLD_ENABLE:
    case TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH:
    case TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS:
    case TBL_DHCRELAY_FLD_AGENT_PROC_OPTION:
    case TBL_DHCRELAY_FLD_TRUST_ALL:
        break;
    case TBL_DHCRELAY_FLD_MAX:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_dhcrelay_set_dhcrelay_field(pitem, field_id));
    return rc;
}

static int32
_dhcrelay_cmd_set(tbl_dhcrelay_field_id_t field_id, tbl_dhcrelay_t *pitem, 
    tbl_dhcrelay_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_DHCRELAY_FLD_ENABLE:
        if (pitem->enable == p_db_item->enable) {
            return PM_E_NONE;
        }
        break;
    case TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH:
        if (pitem->drop_agent_mismatch == p_db_item->drop_agent_mismatch) {
            return PM_E_NONE;
        }
        drop_agent_mismatches = pitem->drop_agent_mismatch;
        break;
    case TBL_DHCRELAY_FLD_ADD_AGENT_OPTIONS:
        if (pitem->add_agent_options == p_db_item->add_agent_options) {
            return PM_E_NONE;
        }
        add_agent_options = pitem->add_agent_options;
        break;
    case TBL_DHCRELAY_FLD_AGENT_PROC_OPTION:
        if (pitem->agent_proc_option == p_db_item->agent_proc_option) {
            return PM_E_NONE;
        }
        agent_relay_mode = pitem->agent_proc_option;
        break;
    case TBL_DHCRELAY_FLD_TRUST_ALL:
        if (pitem->trust_all == p_db_item->trust_all) {
            return PM_E_NONE;
        }
        g_dhcprelay_trustall = pitem->trust_all;
        break;
    case TBL_DHCRELAY_FLD_MAX:
        break;
    }

    PM_E_RETURN(dhcrelay_set(field_id, pitem));
    return rc;
}

static int32
_dhcrelay_interface_cfg(tbl_route_if_t *p_intf, tbl_iter_args_t *pargs)
{
    FILE *fp = (FILE *)pargs;

    if (NULL == p_intf) {
        return -1;
    }

    if (p_intf->dhcp_server_group || p_intf->dhcp_relay_option_trust) {
        fprintf(fp, "/key=%s/dhcp_relay_option_trust=%u/dhcp_server_group=%u\r\n", 
            p_intf->key.name, p_intf->dhcp_relay_option_trust,
            p_intf->dhcp_server_group);
    }
    return 0;
}

static int32
_dhcrelay_get_config(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    /* output interface config */
    tbl_route_if_iterate((TBL_ITER_CB_FUNC)_dhcrelay_interface_cfg, (tbl_iter_args_t *)fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

extern void output_dhcrelay_statistics(FILE *fp);
static int32
_dhcrelay_get_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    output_dhcrelay_statistics(fp);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhcrelay_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_dhcrelay_field_id_t field_id = TBL_DHCRELAY_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_dhcrelay_t item;
    tbl_dhcrelay_t *p_db_item = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCRELAY);    
    p_db_item = tbl_dhcrelay_get_dhcrelay();
    switch (para->oper) {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        if (!sal_strcmp(argv[0], "statistics") && 1 == argc) {
            reset_dhcrelay_statistics();
        } 
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_dhcrelay_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access) {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _dhcrelay_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && (!strcasecmp(argv[0], "interfaces") 
                           || !strcasecmp(argv[0], "trusted"))) {
            rc = _dhcrelay_get_config(para);
            break;
        } else if (1 == argc && !strcasecmp(argv[0], "statistics")) {
            rc = _dhcrelay_get_statistics(para);
            break;
        }

        if (1 == argc && !strcasecmp(argv[0], "config")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else {    
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0) {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }

        rc = _dhcrelay_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* --- dhcsrvgrp --- */
static int dhcsrvgrp_addr_find(tbl_dhcsrvgrp_t *pCfg, const char *pSrvAddr)
{
    char szBuf[DHCP_SRV_ADDRS_BUF_SIZE];

    assert (NULL != pSrvAddr);

    if ('\0' == pCfg->addrs[0]) {
        return 0;
    }
    
    snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s;", pSrvAddr);
    if (NULL != strstr(pCfg->addrs, szBuf)) {
        return 1;
    }
    
    return 0;
}

static int dhcsrvgrp_addr_add(tbl_dhcsrvgrp_t *pCfg, const char *pSrvAddr)
{
    char szBuf[DHCP_SRV_ADDRS_BUF_SIZE];
    int len;

    assert (NULL != pSrvAddr);
    snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s;", pSrvAddr);
    if ('\0' != pCfg->addrs[0]) {    
        if (NULL != strstr(pCfg->addrs, szBuf)) {
            return -1;
        }
        len = strlen(pCfg->addrs);
    } else { 
        len = 0;
    }

    snprintf(pCfg->addrs + len, DHCP_SRV_ADDRS_BUF_SIZE - len, "%s", szBuf);
    return 0;
}

static int dhcsrvgrp_addr_del(tbl_dhcsrvgrp_t *pCfg, const char *pSrvAddr)
{
    char szBuf[DHCP_SRV_ADDRS_BUF_SIZE];
    char *p = NULL;
    int totallen;
    int len;
    
    assert (NULL != pSrvAddr);

    if ('\0' == pCfg->addrs[0]) {
        return -1;
    }
    len = snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s;", pSrvAddr);
    p = strstr(pCfg->addrs, szBuf);
    if (NULL == p) {
        return -1;
    }
    totallen = strlen(pCfg->addrs);
    if (len == totallen) {
        /* only the last record */
        pCfg->addrs[0] = '\0';
        return 0;
    } 

    if (p == pCfg->addrs) {
        /* at the header */
        snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s", p + len);
        snprintf(pCfg->addrs, DHCP_SRV_ADDRS_BUF_SIZE, "%s", szBuf);
    } else if ((p + len) == (pCfg->addrs + totallen)) {
        /* at the tailer */
        pCfg->addrs[p - pCfg->addrs] = '\0';
    } else {
        snprintf(szBuf, DHCP_SRV_ADDRS_BUF_SIZE, "%s", p + len);
        pCfg->addrs[p - pCfg->addrs] = '\0';
        snprintf(pCfg->addrs + (p - pCfg->addrs), 
            DHCP_SRV_ADDRS_BUF_SIZE - (p - pCfg->addrs),
            "%s", szBuf);
    }
    
    return 0;
}

static int
dhcsrvgrp_check_ip(const char *pszAddr)
{
    gensock_t su;
    struct in6_addr addr6;
    u_int32_t network = 0;

    /* is it ip address */
    if (0 != str2gensock(&su, pszAddr)) {
        return -1;
    } 

    if (AF_INET == su.sa.sa_family) {
        u_char *p =  (u_char *)&su.sin.sin_addr;
        
        /* filtrate ip with 255.255.255.255 */
        if (255 == p[0] && 255 && p[1] && 255 == p[2] && 255 == p[3]) {
            return -1;
        }
        
        network = ntohl (*(int*)&su.sin.sin_addr);
        if (network == 0
            || IN_BADCLASS(network)
            || IN_MULTICAST (network)
            || IN_EXPERIMENTAL(network)
            || IPV4_NET127(network)
            || IPV4_NET0(network)) {
            return -1;
        }

        return 0;
    }

    inet_pton (AF_INET6, pszAddr, &addr6); 
    if (IN6_IS_ADDR_MULTICAST (&addr6)
            || IN6_IS_ADDR_LOOPBACK (&addr6)
            || IN6_IS_ADDR_UNSPECIFIED (&addr6)) {
        return -1;
    }
    return 0;
}

/*******************************************************************************
 * Name:    dhcprelay_server_address_check
 * Purpose: check dhcp-server address (ipv4 only)
 * Input:  
 *   nToks:  token numbers
 *   ppTokArr: token array
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
#define IPV4_NET0(a)    ((((u_int32_t) (a)) & 0xff000000) == 0x00000000)
#define IPV4_NET127(a)  ((((u_int32_t) (a)) & 0xff000000) == 0x7f000000)
static int 
dhcsrvgrp_address_check(unsigned long nToks, char **ppTokArr, char *pszErrMsg, 
    size_t err_bufsz, tbl_dhcsrvgrp_t *pitem, int bDel)
{
    unsigned long nIdx = 0;
    char *pszAddr;
   
    for (nIdx = 0; nIdx < nToks; nIdx++) {
        pszAddr = ppTokArr[nIdx];
        if (NULL == pszAddr) {
            continue;
        }

        if (dhcsrvgrp_check_ip(pszAddr) != 0) {
            snprintf(pszErrMsg, err_bufsz, "Invalid address %s", pszAddr);
            return -1;
        }

        if (dhcsrvgrp_addr_find(pitem, pszAddr) == 1) {
            /* item exist */
            if (bDel) {
                dhcsrvgrp_addr_del(pitem, pszAddr);
                pitem->cnt--;
            }
            continue;
        }

        if (bDel) {
            /* item not exist */
            continue;
        }

        if (pitem->cnt >= DHCP_SRV_ADDRS_NUM) {
            snprintf(pszErrMsg, err_bufsz, "Too many address");
            return -1;
        }

        dhcsrvgrp_addr_add(pitem, pszAddr);
        pitem->cnt++;
    }
    
    return 0;
}


static int
dhcsrvgrp_addr_list_parse(char *pszAddrs, char *pszErrMsg, 
    size_t err_bufsz, tbl_dhcsrvgrp_t *pitem, int bDel)
{
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
    int nRet = -1;

    if (split_tokens(pszAddrs, strlen(pszAddrs), DHCP_SRV_ADDRS_NUM,
                     ";", &nToks, &ppTokArr) != 0) {
        snprintf(pszErrMsg, err_bufsz, "Invalid ip address list");
        goto error_out;
    }

    if (dhcsrvgrp_address_check(nToks, ppTokArr, pszErrMsg, 
            err_bufsz, pitem, bDel) != 0) {
        goto error_out;
    }

    nRet = 0;
    
error_out:    
    if (NULL != ppTokArr) {
        free_tokens(&ppTokArr);
    }
    return nRet;
}

static int
_check_dhcsrvgrp_field(char *pszkey, char *pszValue, tbl_dhcsrvgrp_t *pitem,
    cfg_cmd_para_t *para, tbl_dhcsrvgrp_t *pdbitem, int bDel)
{
    char szErrMsg[256];
    cdb_node_t* p_node = cdb_get_tbl(TBL_DHCSRVGRP);
    cdb_field_t *p_field = NULL;
    int32 field_id = TBL_DHCSRVGRP_FLD_MAX;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_dhcsrvgrp_field_id_t)field_id) {
    case TBL_DHCSRVGRP_FLD_ADDRS:
        /*added by yejl to fix bug43588 and bug43589, 2017-06-13*/
        if (!bDel)
        {
            if (dhcsrvgrp_addr_find(pdbitem, pszValue) != 1)
            {
                if (pdbitem->cnt >= DHCP_SRV_ADDRS_NUM) {
                snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Too many address"); 
                goto err_out;
                }
            }
        }
        /*ended by yejl*/
        
        if ('\0' != pdbitem->addrs[0]) {
            snprintf(pitem->addrs, DHCP_SRV_ADDRS_BUF_SIZE, "%s", pdbitem->addrs);
            pitem->cnt = pdbitem->cnt;
        }
        if (dhcsrvgrp_addr_list_parse(pszValue, szErrMsg, 256, pitem, bDel) != 0) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid server address list"); 
            goto err_out;
        }
        break;
    case TBL_DHCSRVGRP_FLD_CNT:
    case TBL_DHCSRVGRP_FLD_KEY:
    case TBL_DHCSRVGRP_FLD_MAX:
        break;
    }

    return field_id;
    
err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
tbl_dhcsrvgrp_dump_filter(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_iter_args_t *pargs)
{
    tbl_dhcsrvgrp_t *p_filter = (tbl_dhcsrvgrp_t *)pargs->argv[2];

    if (NULL == p_filter || p_dhcsrvgrp == p_filter) {
        return tbl_dhcsrvgrp_dump_one(p_dhcsrvgrp, pargs);
    }

    return PM_E_NONE;
}

static int32
_dhcsrvgrp_cmd_get(tbl_dhcsrvgrp_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    args.argv[2] = pitem;
    tbl_dhcsrvgrp_iterate((TBL_ITER_CB_FUNC)tbl_dhcsrvgrp_dump_filter, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_dhcsrvgrp_cmd_set(tbl_dhcsrvgrp_field_id_t field_id, tbl_dhcsrvgrp_t *pitem, 
    tbl_dhcsrvgrp_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_DHCSRVGRP_FLD_ADDRS:
        if (!strcasecmp(pitem->addrs, p_db_item->addrs)) {
            return PM_E_NONE;
        }
        rc = tbl_dhcsrvgrp_set_dhcsrvgrp_field(pitem, TBL_DHCSRVGRP_FLD_ADDRS);
        rc |= tbl_dhcsrvgrp_set_dhcsrvgrp_field(pitem, TBL_DHCSRVGRP_FLD_CNT);
        break;
    case TBL_DHCSRVGRP_FLD_CNT:
    case TBL_DHCSRVGRP_FLD_KEY:
    case TBL_DHCSRVGRP_FLD_MAX:
        return PM_E_NONE;
    }

    return rc;
}


int32
dhcsrvgrp_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_dhcsrvgrp_field_id_t field_id = TBL_DHCSRVGRP_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_dhcsrvgrp_t item;
    tbl_dhcsrvgrp_t *p_db_item = NULL;
    int nId;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCSRVGRP);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        nId = atoi(argv[0]);
        if (strspn(argv[0], "0123456789") != strlen(argv[0]) 
                || (nId < 0 || nId >= GLB_MAX_DHCSRVGRP_NUM)) {
            CFG_INVALID_PARAM_RET("Invalid dhcp server group index: %s", argv[0]);
        }
    
        sal_memset(&item, 0, sizeof(tbl_dhcsrvgrp_t));
        item.key.id = nId;
        p_db_item = tbl_dhcsrvgrp_get_dhcsrvgrp(&item.key);
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Internal error: can't found dhcp server group %s", argv[0]);
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    case CDB_OPER_DEL:
    case CDB_OPER_SET:
        if (2 == argc) {
            if (!strcasecmp(argv[1], "addrs")) { 
                field_id = TBL_DHCSRVGRP_FLD_ADDRS;
                memset(item.addrs, 0x00, DHCP_SRV_ADDRS_BUF_SIZE); 
                item.cnt = 0;
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _dhcsrvgrp_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_dhcsrvgrp_field(argv[i], argv[i + 1], 
                    &item, para, p_db_item, (CDB_OPER_DEL == para->oper)) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _dhcsrvgrp_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Dhcp server group %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _dhcsrvgrp_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

/* --- dhcdbg --- */
static int
_check_dhcdbg_field(char *pszkey, char *pszValue, tbl_dhcp_debug_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_DHCP_DEBUG);
    cdb_field_t *p_field = NULL;
    int32 field_id;
    int32 nValue;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "invalid value for %s", pszkey); 
        goto err_out;
    }
    nValue = atoi(pszValue);
    switch ((tbl_dhcp_debug_field_id_t)field_id) {
    case TBL_DHCP_DEBUG_FLD_RELAY_ERROR:
    case TBL_DHCP_DEBUG_FLD_RELAY_EVENTS:
    case TBL_DHCP_DEBUG_FLD_RELAY_PACKET:
    case TBL_DHCP_DEBUG_FLD_RELAY_DUMP:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_PACKET:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_DUMP:
    case TBL_DHCP_DEBUG_FLD_CLIENT_ERROR:
    case TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS:
    case TBL_DHCP_DEBUG_FLD_CLIENT_PACKET:
    case TBL_DHCP_DEBUG_FLD_CLIENT_DUMP:

        GLB_SET_FLAG_COND(nValue, pitem->flags, (1 << field_id));
        break;
    case TBL_DHCP_DEBUG_FLD_MAX:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

int32
dhcdbg_cmd_get(tbl_dhcp_debug_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dhcp_debug_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_dhcdbg_cmd_set(tbl_dhcp_debug_field_id_t field_id, tbl_dhcp_debug_t *pitem, 
    tbl_dhcp_debug_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (field_id >= TBL_DHCP_DEBUG_FLD_MAX) {
        return PM_E_FAIL;
    }

    
    switch ((tbl_dhcp_debug_field_id_t)field_id) {
    case TBL_DHCP_DEBUG_FLD_RELAY_ERROR:
    case TBL_DHCP_DEBUG_FLD_RELAY_EVENTS:
    case TBL_DHCP_DEBUG_FLD_RELAY_PACKET:
    case TBL_DHCP_DEBUG_FLD_RELAY_DUMP:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET:
    case TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_PACKET:
    case TBL_DHCP_DEBUG_FLD_SNOOPINGV6_DUMP:
    case TBL_DHCP_DEBUG_FLD_CLIENT_ERROR:
    case TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS:
    case TBL_DHCP_DEBUG_FLD_CLIENT_PACKET:
    case TBL_DHCP_DEBUG_FLD_CLIENT_DUMP:

        if ((pitem->flags & (1 << field_id)) 
                == (p_db_item->flags & (1 << field_id))) {
            return PM_E_NONE;
        }

        break;
    case TBL_DHCP_DEBUG_FLD_MAX:
        break;
    }
    

    PM_E_RETURN(tbl_dhcp_debug_set_dhcp_debug_field(pitem, field_id));
    return rc;
}

#define DBG_DHCP_TYPE_FIELD     0
#define DBG_DHCP_TYPE_ALL       1

int32
dhcdbg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_dhcp_debug_field_id_t field_id = TBL_DHCP_DEBUG_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    int32 rc = PM_E_NONE;
    tbl_dhcp_debug_t item;
    tbl_dhcp_debug_t *p_db_item = NULL;
    int32 value = 0;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCP_DEBUG);    
    p_db_item = tbl_dhcp_debug_get_dhcp_debug();
    switch (para->oper) {
    case CDB_OPER_SET:
        if (0 == sal_strcmp(argv[i], "all"))
        {
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);

            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_RELAY_ERROR);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_RELAY_EVENTS);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_RELAY_PACKET);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_RELAY_DUMP);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPING_ERROR);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPING_EVENTS);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPING_PACKET);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPING_DUMP);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPINGV6_ERROR);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPINGV6_EVENTS);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPINGV6_PACKET);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_SNOOPINGV6_DUMP);

            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_CLIENT_ERROR);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_CLIENT_EVENTS);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_CLIENT_PACKET);
            GLB_SET_FLAG_COND(value, item.flags, DHCDBG_FLAG_CLIENT_DUMP);

            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_RELAY_ERROR);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_RELAY_EVENTS);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_RELAY_PACKET);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_RELAY_DUMP);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPING_ERROR);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPING_EVENT);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPING_PACKET);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPING_DUMP);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_ERROR);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_EVENTS);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_PACKET);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_SNOOPINGV6_DUMP);

            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_CLIENT_ERROR);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_CLIENT_EVENTS);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_CLIENT_PACKET);
            rc = tbl_dhcp_debug_set_dhcp_debug_field(&item, TBL_DHCP_DEBUG_FLD_CLIENT_DUMP);

        }
        else
        {
            if ((argc % 2) != 0) {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }

            for (i = 0; i < argc; i += 2) {
                if (_check_dhcdbg_field(argv[i], argv[i + 1], &item, para) == -1) {
                    return PM_E_CFG_INVALID_PARAM;
                }

                cdb_get_field_id(argv[i], p_node, &p_field);
                if (CDB_ACCESS_RD == p_field->access) {
                    CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
                }
            }

            for (i = 0; i < argc; i += 2) {
                field_id = cdb_get_field_id(argv[i], p_node, &p_field);
                rc |= _dhcdbg_cmd_set(field_id, &item, p_db_item, para);
            }
        }
        break;

    case CDB_OPER_GET:
        /* 
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0) {
            CFG_INVALID_PARAM_RET("Invalid field");
        }
        */
        
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = dhcdbg_cmd_get(p_db_item, para, p_node, &field);    

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
dhcrelay_tbl_init()
{
    tbl_dhcrelay_t dhcrelay;
    tbl_dhcsrvgrp_t dhcsrvgrp;
    int i;
    
    /* -- dhcrelay -- */
    memset(&dhcrelay, 0x00, sizeof(tbl_dhcrelay_t));
    dhcrelay.drop_agent_mismatch = 1;
    dhcrelay.agent_proc_option = AGENT_PROC_OPTION_UNTOUCHED;
    
    tbl_dhcrelay_set_dhcrelay_field(&dhcrelay, TBL_DHCRELAY_FLD_DROP_AGENT_MISMATCH);
    tbl_dhcrelay_set_dhcrelay_field(&dhcrelay, TBL_DHCRELAY_FLD_AGENT_PROC_OPTION);

    /* -- dhcp server group for relay */
    memset(&dhcsrvgrp, 0x00, sizeof(tbl_dhcsrvgrp_t));
    for (i = 0; i < GLB_MAX_DHCSRVGRP_NUM; i++) {
        dhcsrvgrp.key.id = i;
        tbl_dhcsrvgrp_add_dhcsrvgrp(&dhcsrvgrp);
    }
    return PM_E_NONE;
}

int32
dhcrelay_proc_startup_done(void)
{
    return PM_E_NONE;
}

int32
dhcrelay_proc_init_done(void)
{
    /* Modified by kcao for bug 40062, should init CDB table after connection to CDS is ready */
    dhcrelay_tbl_init();
    
    return PM_E_NONE;
}

#endif
