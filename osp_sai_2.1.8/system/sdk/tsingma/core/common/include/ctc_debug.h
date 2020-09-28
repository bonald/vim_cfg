/****************************************************************************
 *file ctc_debug.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2009-11-26

 *version v2.0

  This file contains  debug header file.
 ****************************************************************************/

#ifndef _CTC_DEBUG_H_
#define _CTC_DEBUG_H_

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "common/include/ctc_linklist.h"
#include "common/../../sal/include/sal_file.h"

#define CTC_DEBUG_NAME_LEN 31   /**< Name length of debug module */
typedef int (* ctc_debug_out_func) (const char* str, ...);
typedef void log_func (void*);

extern ctc_debug_out_func g_ctc_debug_log_print_cb;
extern ctc_debug_out_func g_ctc_debug_show_print_cb;
extern uint8 g_ctc_debug_on;
int ctc_cli_out(const char* fmt, ...);
/* debug struct */
struct ctc_debug_list_s
{
    unsigned char flags;                    /* debug flag */
    unsigned char level;                    /* debug level */
    unsigned char level1;                   /* debug level1 */
    unsigned char rsv;                      /* reserved */
    char module[CTC_DEBUG_NAME_LEN + 1];      /**< Debug module */
    char submodule[CTC_DEBUG_NAME_LEN + 1];   /**< Debug submodule */
    sal_file_t p_file;      /* debug log file name */
};
typedef struct ctc_debug_list_s ctc_debug_list_t;

struct ctc_debug_stub_s
{
    uint32 times;    /* stub usage return error while the func do the times*/
    char fun_name[256];
    int32 error_code;
    int32 (* stub_func)(void* val);
};
typedef struct ctc_debug_stub_s ctc_debug_stub_t;

struct ctc_debug_stub_new_s
{
    uint32 times;    /* stub usage return error while the func do the times*/
    char fun_name[256];
    int32 (* stub_func)(uint32 time, void* val);
};
typedef struct ctc_debug_stub_new_s ctc_debug_stub_new_t;

enum ctc_debug_level_e
{
    CTC_DEBUG_LEVEL_NONE     = 0x00,
    CTC_DEBUG_LEVEL_FUNC     = 0x01,
    CTC_DEBUG_LEVEL_PARAM    = 0x02,
    CTC_DEBUG_LEVEL_INFO     = 0x04,
    CTC_DEBUG_LEVEL_ERROR    = 0x08,
    CTC_DEBUG_LEVEL_DUMP     = 0x10,
    CTC_DEBUG_LEVEL_LOGFILE     = 0x20,
    CTC_DEBUG_LEVEL_EXPORT      = 0x40
};
typedef enum ctc_debug_level_e ctc_debug_level_t;

#define CTC_DEBUG_IMPLEMENT(mod, sub)     \
    ctc_debug_list_t ctc_##mod##_##sub##_debug_handle =         \
    {                                                   \
        0x00,                              \
        0x00,                              \
        0x00,                              \
        0x00,                              \
        # mod,        \
        # sub,                 \
        NULL  \
    }

#define CTC_DEBUG_HEX_FORMAT_U(S, F, V, W)                                             \
                            ((sal_sprintf(F, "0x%%0%dX", W))                         \
                             ? ((sal_sprintf(S, F, V)) ? S : NULL) : NULL)

#define CTC_DEBUG_HEX_FORMAT_L(S, F, V, W)                                             \
                            ((sal_sprintf(F, "0x%%0%dx", W))                         \
                             ? ((sal_sprintf(S, F, V)) ? S : NULL) : NULL)

#define CTC_DEBUG_HEX_FORMAT_(S, F, V, W)      CTC_DEBUG_HEX_FORMAT_U(S, F, V, W)

#define CTC_DEBUG_HEX_FORMAT(S, F, V, W, ...)  CTC_DEBUG_HEX_FORMAT_ ##__VA_ARGS__(S, F, V, W)

#define CTC_DEBUG_OUT_DUMP(fmt, args...)            \
    { \
        if(g_ctc_debug_show_print_cb) \
        { \
            g_ctc_debug_show_print_cb(fmt,##args); \
        } \
    }

#define CTC_DEBUG_OUT_INFO(mod, sub, typeenum, fmt, args...) \
    {                                                                               \
        if (ctc_debug_check_flag(ctc_##mod##_##sub##_debug_handle, typeenum, CTC_DEBUG_LEVEL_INFO)) \
        {                                                                           \
            if(g_ctc_debug_show_print_cb) \
            { \
                g_ctc_debug_show_print_cb(fmt,##args); \
            } \
        }                                                                           \
    }

#define CTC_DEBUG_OUT_FUNC(mod, sub, typeenum) \
    {                                                                               \
        if (ctc_debug_check_flag(ctc_##mod##_##sub##_debug_handle, typeenum, CTC_DEBUG_LEVEL_FUNC)) \
        {                                                                           \
            if(g_ctc_debug_show_print_cb) \
            { \
                g_ctc_debug_show_print_cb("\n%s()\n", __FUNCTION__); \
            }   \
        }                                                                           \
    }

#define CTC_DEBUG_OUT(mod, sub, typeenum, level, fmt, args...) \
    {                                                                               \
        if (level == CTC_DEBUG_LEVEL_DUMP) \
        { \
            if(g_ctc_debug_show_print_cb) \
            { \
                g_ctc_debug_show_print_cb(fmt,##args); \
            } \
        }\
        else if (g_ctc_debug_on)\
        {\
            sal_file_t p_log = NULL;                                                  \
            if (ctc_debug_check_flag(ctc_##mod##_##sub##_debug_handle, typeenum, level)) \
            {                                                                           \
                if (CTC_FLAG_ISSET(ctc_##mod##_##sub##_debug_handle.level1, CTC_DEBUG_LEVEL_LOGFILE))   \
                {     \
                    p_log = ctc_##mod##_##sub##_debug_handle.p_file;   \
                    if (p_log)   \
                    {             \
                        sal_fprintf(p_log, fmt, ##args); \
                    }             \
                }   \
                else if(level == CTC_DEBUG_LEVEL_FUNC) \
                {    \
                    g_ctc_debug_log_print_cb("[FUNC] "fmt, ##args); \
                }     \
                else if(level == CTC_DEBUG_LEVEL_INFO) \
                {    \
                    g_ctc_debug_log_print_cb("  INFO : "fmt, ##args); \
                }     \
                else if(level == CTC_DEBUG_LEVEL_EXPORT) \
                {    \
                    g_ctc_debug_log_print_cb("  EXPORT : "fmt, ##args); \
                }     \
                else if(level == CTC_DEBUG_LEVEL_PARAM) \
                {    \
                    g_ctc_debug_log_print_cb("  PARAM: "fmt, ##args); \
                }     \
                else if(level == CTC_DEBUG_LEVEL_ERROR) \
                {    \
                    g_ctc_debug_log_print_cb("  ERROR: "fmt, ##args); \
                }     \
                else  \
                {    \
                    if(g_ctc_debug_show_print_cb) \
                    { \
                        g_ctc_debug_show_print_cb(fmt,##args); \
                    } \
                }  \
            }      \
        }\
    }

struct ctc_debug_write_info_para_s
{
    char file[256];
    void* pdata;
};
typedef struct ctc_debug_write_info_para_s ctc_debug_write_info_para_t;

#define CTC_DEBUG_LOGFILE_BEGIN() \
    {  \
        if (p_log_file)  \
        {  \
            sal_printf("Please read log-file after seeing \" file have been writen!\"\n");  \
        }  \
    }

#define CTC_DEBUG_LOGFILE_END() \
    {  \
        if (p_log_file)  \
        {  \
            fclose(p_log_file);  \
            p_log_file = NULL;  \
            sal_printf("file have been writen!\n");  \
        }  \
    }

/**********************************************************************************
                        Define module define debug enum
 * NOTE:
* 1, pattern: CTC_DEBUG_ENUM(module, submodule, debug_enum0, debug_enum1...);
* 2, module and sub_module name length should less than 31 characters
* 3, different module can't define the same enum
 ***********************************************************************************/

/* define debug enum for each mode */
#define CTC_DEBUG_ENUM(mod, sub, typeenum, ...)       \
    extern ctc_debug_list_t ctc_##mod##_##sub##_debug_handle; \
    enum ctc_##mod##_##sub##_debug_enum                          \
    {                                                   \
        typeenum = 0,                                   \
        __VA_ARGS__                                     \
    }

/* FOR UNIT TEST STRAT */

extern ctc_debug_stub_t* p_ctc_debug_stub_info;
extern ctc_debug_stub_new_t* p_ctc_debug_stub_new_info;
extern void* p_log_file;
extern int g_error_on;

/* stub usage : add this MARCO in function if times reach 0 it will return error_code*/
#define CTC_DEBUG_ADD_STUB_CTC(val)                                    \
    do                                                                  \
    {                                                                   \
        if (p_ctc_debug_stub_info &&                                     \
            (0 == sal_strcmp(p_ctc_debug_stub_info->fun_name, __FUNCTION__)))                  \
        {                                                                \
            if (p_ctc_debug_stub_info->times == 0xFFFFFFFF)     \
            {                 \
                if (p_ctc_debug_stub_info->stub_func)   \
                {    \
                    p_ctc_debug_stub_info->stub_func(val);   \
                }  \
                return p_ctc_debug_stub_info->error_code;                \
            }                 \
            else             \
            {                 \
                if (p_ctc_debug_stub_info->times == 0)                        \
                {                                                            \
                    p_ctc_debug_stub_info->times = 0xFFFFFFFE;               \
                    if (p_ctc_debug_stub_info->stub_func)   \
                    {    \
                        p_ctc_debug_stub_info->stub_func(val);   \
                    }  \
                    return p_ctc_debug_stub_info->error_code;                \
                }                                                            \
                p_ctc_debug_stub_info->times--;                              \
            }                 \
        }                                                                \
    } while (0)

/* stub usage : add this MARCO in function if times reach 0 it will return NULL*/
#define CTC_DEBUG_ADD_STUB_NULL()                                   \
    do                                                                  \
    {                                                                   \
        if (p_ctc_debug_stub_info &&                                     \
            (0 == sal_strcmp(p_ctc_debug_stub_info->fun_name, __FUNCTION__)))                  \
        {                                                                \
            if (p_ctc_debug_stub_info->times == 0)                        \
            {                                                            \
                p_ctc_debug_stub_info->times = 0xFFFFFFFF;               \
                return NULL;                                             \
            }                                                            \
            p_ctc_debug_stub_info->times--;                              \
        }                                                                \
    } while (0)

/* stub usage : add this MARCO in UNITTEST code to init stub */
#define CTC_DEBUG_CREATE_STUB(_ERROR_CODE, _FUN_NAME, STUB_FUNC, _TIMES)                         \
    do                                                                                  \
    {                                                                                   \
        p_ctc_debug_stub_info = (ctc_debug_stub_t*)sal_malloc(sizeof(ctc_debug_stub_t)); \
        sal_memset(p_ctc_debug_stub_info, 0, sizeof(ctc_debug_stub_t));                   \
        p_ctc_debug_stub_info->error_code = _ERROR_CODE;                                \
        p_ctc_debug_stub_info->times = _TIMES;                                        \
        p_ctc_debug_stub_info->stub_func = STUB_FUNC;                         \
        sal_memcpy(p_ctc_debug_stub_info->fun_name, _FUN_NAME, sal_strlen(_FUN_NAME));    \
    } while (0)

/* stub usage : add this MARCO in UNITTEST code to destory stub */
#define CTC_DEBUG_DESTORY_STUB()      \
    do                                    \
    {                                     \
        sal_free(p_ctc_debug_stub_info);  \
        p_ctc_debug_stub_info = NULL;     \
    } while (0)

/* stub usage : add this MARCO in function return error_code in stub function */
#define CTC_DEBUG_ADD_NEW_STUB(val)                                    \
    do                                                                  \
    {                                                                   \
        if (p_ctc_debug_stub_new_info &&                                     \
            (0 == sal_strcmp(p_ctc_debug_stub_new_info->fun_name, __FUNCTION__)))                  \
        {                                                                \
            return p_ctc_debug_stub_new_info->stub_func(p_ctc_debug_stub_new_info->times, val);   \
        }                                                                \
    } while (0)

#define CTC_DEBUG_CREATE_NEW_STUB(_FUN_NAME, STUB_FUNC, _TIMES)                         \
    do                                                                                  \
    {                                                                                   \
        p_ctc_debug_stub_new_info = (ctc_debug_stub_new_t*)sal_malloc(sizeof(ctc_debug_stub_new_t)); \
        sal_memset(p_ctc_debug_stub_new_info, 0, sizeof(ctc_debug_stub_new_t));                   \
        p_ctc_debug_stub_new_info->times = _TIMES;                                        \
        p_ctc_debug_stub_new_info->stub_func = STUB_FUNC;                         \
        sal_memcpy(p_ctc_debug_stub_new_info->fun_name, _FUN_NAME, sal_strlen(_FUN_NAME));    \
    } while (0)

/* stub usage : add this MARCO in UNITTEST code to destory stub */
#define CTC_DEBUG_DESTORY_NEW_STUB()      \
    do                                    \
    {                                     \
        sal_free(p_ctc_debug_stub_new_info);  \
        p_ctc_debug_stub_new_info = NULL;     \
    } while (0)

#define CTC_DEBUG_ADD_NEW_STUB_NULL()                                   \
    do                                                                  \
    {                                                                   \
        if (p_ctc_debug_stub_new_info &&                                     \
            (0 == sal_strcmp(p_ctc_debug_stub_new_info->fun_name, __FUNCTION__)))                  \
        {                                                                \
            if (p_ctc_debug_stub_new_info->times == 0)                        \
            {                                                            \
                p_ctc_debug_stub_new_info->times = 0xFFFFFFFF;               \
                return NULL;                                             \
            }                                                            \
            p_ctc_debug_stub_new_info->times--;                              \
        }                                                                \
    } while (0)

/* FOR UNIT TEST END */

/*acl/qos module*/
CTC_DEBUG_ENUM(qos, process, QOS_PROC_CTC, QOS_PROC_SYS);
CTC_DEBUG_ENUM(qos, queue,  QOS_QUE_CTC, QOS_QUE_SYS);
CTC_DEBUG_ENUM(acl, acl,  ACL_CTC, ACL_SYS);
CTC_DEBUG_ENUM(cpu, traffic,  CPU_TRAFFIC_CTC, CPU_TRAFFIC_SYS);

CTC_DEBUG_ENUM(aclqos, label, ACLQOS_LABEL_CTC, ACLQOS_LABEL_SYS);
CTC_DEBUG_ENUM(aclqos, entry, ACLQOS_ENTRY_CTC, ACLQOS_ENTRY_SYS);
CTC_DEBUG_ENUM(qos, policer,  QOS_PLC_CTC, QOS_PLC_SYS);
CTC_DEBUG_ENUM(qos, class,  QOS_CLASS_CTC, QOS_CLASS_SYS);

/* L3  module*/
CTC_DEBUG_ENUM(l3if, l3if, L3IF_CTC,  L3IF_SYS);
CTC_DEBUG_ENUM(ip, ipuc,  IPUC_CTC, IPUC_SYS);
CTC_DEBUG_ENUM(ip, ipmc, IPMC_CTC, IPMC_SYS);
CTC_DEBUG_ENUM(ip, rpf, RPF_CTC, RPF_SYS);
CTC_DEBUG_ENUM(mcast, mcast, MCAST_CTC, MCAST_SYS);
CTC_DEBUG_ENUM(mpls, mpls, MPLS_CTC, MPLS_SYS);
CTC_DEBUG_ENUM(fcoe, fcoe,  FCOE_CTC, FCOE_SYS);

/* L2  module*/

CTC_DEBUG_ENUM(l2, fdb,  L2_FDB_CTC, L2_FDB_SYS);
CTC_DEBUG_ENUM(l2, learning_aging,  L2_LEARNING_AGING_CTC, L2_LEARNING_AGING_SYS);
CTC_DEBUG_ENUM(l2, stp,  L2_STP_CTC, L2_STP_SYS);
CTC_DEBUG_ENUM(vlan, vlan,  VLAN_CTC, VLAN_SYS);
CTC_DEBUG_ENUM(vlan, vlan_class, VLAN_CLASS_CTC, VLAN_CLASS_SYS);
CTC_DEBUG_ENUM(vlan, vlan_mapping,  VLAN_MAPPING_CTC, VLAN_MAPPING_SYS);
CTC_DEBUG_ENUM(vlan, vlan_switching, VLAN_SWITCHING_CTC, VLAN_SWITCHING_SYS);
CTC_DEBUG_ENUM(vlan, protocol_vlan, PRO_VLAN_CTC, PRO_VLAN_SYS);
CTC_DEBUG_ENUM(dot1x, dot1x,  DOT1X_CTC, DOT1X_SYS);
CTC_DEBUG_ENUM(mirror, mirror, MIRROR_CTC, MIRROR_SYS);
CTC_DEBUG_ENUM(security, security, SECURITY_CTC, SECURITY_SYS);
CTC_DEBUG_ENUM(dot1ae, dot1ae, DOT1AE_CTC, DOT1AE_SYS);
CTC_DEBUG_ENUM(oam, oam, OAM_CTC, OAM_SYS);
CTC_DEBUG_ENUM(oam, efm, OAM_EFM_CTC, OAM_EFM_SYS);
CTC_DEBUG_ENUM(oam, pbx, OAM_PBX_CTC, OAM_PBX_SYS);
CTC_DEBUG_ENUM(ptp, ptp, PTP_PTP_CTC, PTP_PTP_SYS);
CTC_DEBUG_ENUM(sync_ether, sync_ether, SYNC_ETHER_CTC, SYNC_ETHER_SYS);

/*interrupt module*/
CTC_DEBUG_ENUM(interrupt, interrupt, INTERRUPT_CTC, INTERRUPT_SYS);

/* packet module*/
CTC_DEBUG_ENUM(packet, packet, PACKET_CTC, PACKET_SYS);

/* resource module*/
CTC_DEBUG_ENUM(chip, chip, CHIP_CTC, CHIP_SYS);
CTC_DEBUG_ENUM(chip, peri, PERI_CTC, PERI_SYS);
CTC_DEBUG_ENUM(port, port, PORT_CTC, PORT_SYS);
CTC_DEBUG_ENUM(port, mac, MAC_CTC, MAC_SYS);
CTC_DEBUG_ENUM(port, cl73, CL73_CTC, CL73_SYS);
CTC_DEBUG_ENUM(linkagg, linkagg, LINKAGG_CTC, LINKAGG_SYS);
CTC_DEBUG_ENUM(nexthop, nexthop, NH_CTC, NH_SYS);
CTC_DEBUG_ENUM(scl, scl, SCL_CTC,  SCL_SYS);
CTC_DEBUG_ENUM(alloc, alloc, ALLOC_CTC, ALLOC_SYS);
CTC_DEBUG_ENUM(opf, opf, OPF_SYS, OPF_UNDEFINED);
CTC_DEBUG_ENUM(fpa, fpa, FPA, FPA1);
CTC_DEBUG_ENUM(memmngr, memmngr, MEMMNGR_CTC, MEMMNGR_UNDEFINED);

CTC_DEBUG_ENUM(stats, stats,  STATS_CTC, STATS_SYS);

/*Parser*/
CTC_DEBUG_ENUM(parser, parser, PARSER_CTC,  PARSER_SYS);

/*PDU*/
CTC_DEBUG_ENUM(pdu, pdu, PDU_CTC,  PDU_SYS);

/*LearningAging*/
CTC_DEBUG_ENUM(learning_aging, learning_aging, LEARNING_AGING_CTC,  LEARNING_AGING_SYS);

/* DMA */
CTC_DEBUG_ENUM(dma, dma,  DMA_CTC, DMA_SYS);

/*Aps*/
CTC_DEBUG_ENUM(aps, aps, APS_CTC, APS_SYS);

/*Stacking*/
CTC_DEBUG_ENUM(stacking, stacking, STK_CTC, STK_SYS);


/* Chip Agent for EADP */
CTC_DEBUG_ENUM(chipagent, chipagent, CHIP_AGT_SOCKET, CHIP_AGT_CODE);

/* app */
CTC_DEBUG_ENUM(app, app, APP_SMP, APP_MAX);

/* core app*/
CTC_DEBUG_ENUM(app, api, APP_API);

/* ftm */
CTC_DEBUG_ENUM(ftm, ftm, FTM_CTC, FTM_SYS);

/* BPE */
CTC_DEBUG_ENUM(bpe, bpe,  BPE_CTC, BPE_SYS);

/* overlay tunnel */
CTC_DEBUG_ENUM(overlay_tunnel, overlay_tunnel, OVERLAY_TUNNEL_CTC, OVERLAY_TUNNEL_SYS);

/* trill */
CTC_DEBUG_ENUM(trill, trill, TRILL_CTC, TRILL_SYS);

/* ipfix */
CTC_DEBUG_ENUM(ipfix, ipfix, IPFIX_CTC, IPFIX_SYS);

/* monitor */
CTC_DEBUG_ENUM(monitor, monitor, MONITOR_CTC, MONITOR_SYS);

/* efd */
CTC_DEBUG_ENUM(efd, efd, EFD_CTC, EFD_SYS);

/* init */
CTC_DEBUG_ENUM(init, init, INIT_CODE);

/* datapath */
CTC_DEBUG_ENUM(datapath, datapath, DATAPATH_CTC, DATAPATH_SYS);

/* wlan */
CTC_DEBUG_ENUM(wlan, wlan, WLAN_CTC, WLAN_SYS);

/* npm */
CTC_DEBUG_ENUM(npm, npm, NPM_CTC, NPM_SYS);

/* warmboot */
CTC_DEBUG_ENUM(wb, wb, WB_CTC, WB_SYS);

/* register */
CTC_DEBUG_ENUM(register, register, REGISTER_CTC, REGISTER_SYS);

/* diag */
CTC_DEBUG_ENUM(diag, diag, DIAG_CTC, DIAG_SYS);
/**********************************************************************************
                      Define API function interfaces
***********************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_debug_init(void);
extern int32
ctc_debug_enable(bool enable);
extern bool
ctc_get_debug_enable(void);
extern void
ctc_debug_register_log_cb(ctc_debug_out_func func);
extern void
ctc_debug_register_cb(ctc_debug_out_func func);

extern int32
ctc_debug_clear_all_flag(void);
extern int32
ctc_debug_set_flag(char* module, char* submodule, uint32 typeenum, uint8 level, bool debug_on);
extern bool
ctc_debug_get_flag(char* module, char* submodule, uint32 typeenum, uint8* level);

extern bool
ctc_debug_check_flag(ctc_debug_list_t flag_info, uint32 typeenum, uint8 level);
extern void
ctc_debug_reg_log_func(log_func func, void* p_para);
extern int32
ctc_debug_set_log_file(char* module, char* submodule, void* p_file_name, uint8 enable);

#ifdef __cplusplus
}
#endif

#endif /* _CTC_DEBUG_H_ */

