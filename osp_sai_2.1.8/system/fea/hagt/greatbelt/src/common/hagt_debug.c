/****************************************************************************
 * hagt_debug.s      Hal Agent debug source file.
 *
 * Copyright     :(c)2010 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      :R0.01.
 * Author        :Xianting Huang
 * Date          :2010-07
 * Reason        :First Create.
 ****************************************************************************/
#include "hagt_inc.h"

hagt_debug_t g_hagt_debug;
uint16 linecard_num;

/*********************************************************************
 * Name    : _hagt_debugctrl_set
 * Purpose : This is function will set the debug control switches for hagt debug
 * Input   : Hsrv2AgtMsg_t* pst_hsrv_msg
 * Output  : N/A
 * Return  : HAGT_E_NONE
 * Note    : 2010.8 init version
*********************************************************************/
static int32
_hagt_debug_set(Hsrv2AgtMsg_t* pst_hsrv_msg)
{
    uint32 module = 0;
    uint32 submodule = 0;
    uint32 flags = 0;

    module = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgDebugSet.module;
    submodule = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgDebugSet.submodule;
    flags = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgDebugSet.flags;

    if (0 == module)    /* hagt */
    {
        switch (submodule)
        {
        case HAGTDBG_TYPE_INTERFACE:
            g_hagt_debug.interface = flags;
            break;
        case HAGTDBG_TYPE_CPU:
            g_hagt_debug.cpu = flags;
            break;
        case HAGTDBG_TYPE_NEXTHOP:
            g_hagt_debug.nexthop = flags;
            break;
        case HAGTDBG_TYPE_VLAN:
            g_hagt_debug.vlan = flags;
            break;
        case HAGTDBG_TYPE_FDB:
            g_hagt_debug.fdb = flags;
            break;
        case HAGTDBG_TYPE_L2MC:
            g_hagt_debug.l2mc = flags;
            break;
        case HAGTDBG_TYPE_AGG:
            g_hagt_debug.agg = flags;
            break;
        case HAGTDBG_TYPE_IPUC:
            g_hagt_debug.ipuc = flags;
            break;
        case HAGTDBG_TYPE_NEIGHBOR:
            g_hagt_debug.neighbor = flags;
            break;
        case HAGTDBG_TYPE_ECMP:
            g_hagt_debug.ecmp = flags;
            break;
        case HAGTDBG_TYPE_ACL:
            g_hagt_debug.acl = flags;
            break;
        case HAGTDBG_TYPE_STATS:
            g_hagt_debug.stats = flags;
            break;
        case HAGTDBG_TYPE_QOS:
            g_hagt_debug.agg = flags;
            break;
        default:
            break;
        }
    }
    else
    {
        /* other modules: e.g. lcm/sdk/drv */
    }

    return HAGT_E_NONE;
}

/*********************************************************************
 * Name    : _hagt_debugctrl_set
 * Purpose : This is function will set the debug control switches for hagt debug
 * Input   : Hsrv2AgtMsg_t* pst_hsrv_msg
 * Output  : N/A
 * Return  : HAGT_E_NONE
 * Note    : 2010.8 init version
*********************************************************************/
static int32
_hagt_debugctrl_set (Hsrv2AgtMsg_t* pst_hsrv_msg)
{
#ifdef _GLB_DISTRIBUTE_SYSTEM_
    uint32 lc_on;
    uint16 slot_num = 3;
    uint8 line_function_on;

    line_function_on = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgDebugCtrlSet.linefunon;
    ctclib_debug_line_function_on(line_function_on);

    lc_on = pst_hsrv_msg->opHsrv2Agt.hsrv2AgtMsgDebugCtrlSet.lcon;
    slot_num = hagt_get_logic_slotid();

    if (lc_on & (1 << slot_num))
    {
        lc_on = 1;
    }
    else
    {
        lc_on = 0;
    }
    ctclib_debug_linecard_on(lc_on);
#endif /*_GLB_DISTRIBUTE_SYSTEM_*/

    return HAGT_E_NONE;
}

int32
hagt_debug_msg_callback_register(void)
{
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgDebugSet, _hagt_debug_set);
    hagt_message_set_msg_callback(opHsrv2Agt_PR_hsrv2AgtMsgDebugCtrlSet, _hagt_debugctrl_set);

    return HAGT_E_NONE;
}

/*Added by xgu for supporting SDK debug out, 2013-2-18, start*/
int
hagt_debug_sdk_debug_out_syslog(const char *fmt, ...)
{
    va_list ap;
#define M_DEBUG_OUT_BUFSZ     1024
    char szBuf[M_DEBUG_OUT_BUFSZ];
    va_start(ap, fmt);
    vsnprintf(szBuf, M_DEBUG_OUT_BUFSZ, fmt, ap);
    va_end(ap);
    log_sys(M_MOD_SDK, E_DEBUG, szBuf);
    return HAGT_E_NONE;
}

int
hagt_debug_sdk_debug_out_console(const char *fmt, ...)
{
    FILE * fp_console = NULL;
    va_list ap;
#define M_DEBUG_OUT_BUFSZ     1024
    char szBuf[M_DEBUG_OUT_BUFSZ];

    va_start(ap, fmt);
    vsnprintf(szBuf, M_DEBUG_OUT_BUFSZ, fmt, ap);
    va_end(ap);

    fp_console = fopen("/dev/console", "w+");
    sal_fprintf(fp_console, szBuf);
    fclose(fp_console);

    return HAGT_E_NONE;
}

int
hagt_debug_sdk_debug_out_tmpfile(const char *fmt, ...)
{
    FILE * fp_console = NULL;
    va_list ap;
#define M_DEBUG_OUT_BUFSZ     1024
    char szBuf[M_DEBUG_OUT_BUFSZ];

    va_start(ap, fmt);
    vsnprintf(szBuf, M_DEBUG_OUT_BUFSZ, fmt, ap);
    va_end(ap);

    fp_console = fopen("/tmp/sdk_debug", "a+");
    sal_fprintf(fp_console, szBuf);
    fclose(fp_console);

    return HAGT_E_NONE;
}
/*Added by xgu for supporting SDK debug out, 2013-2-18, end*/

/*********************************************************************
 * Name    : hagt_debug_init
 * Purpose : This is a init function for hagt debug module
 * Input   : N/A
 * Output  : N/A
 * Return  : HAGT_E_NONE
 * Note    : 2010.7 init version
*********************************************************************/
int32
hagt_debug_init(void)
{
    /*callback functions register*/
    hagt_debug_msg_callback_register();

    extern int (*cli_print_func_ptr)(const char* str, ...);
    cli_print_func_ptr = hagt_debug_sdk_debug_out_tmpfile;
    
    sal_memset(&g_hagt_debug, 0, sizeof(g_hagt_debug));
    linecard_num = hagt_get_phy_slotid();
    return HAGT_E_NONE;
}
