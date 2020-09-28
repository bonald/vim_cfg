/****************************************************************************
 *file ctc_debug.c

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2009-11-26

 *version v2.0

  This file contains  debug header file.
 ****************************************************************************/

#include "sal.h"
#include "ctc_debug.h"
#include "ctc_macro.h"
#include "ctc_error.h"
#include "ctc_linklist.h"

#define CTC_DEBUG_MODULE_REG(mod, sub)     \
    ctc_listnode_add(ctc_debug_list, &ctc_##mod##_##sub##_debug_handle);

static struct ctc_linklist* ctc_debug_list;
#ifdef SDK_IN_DEBUG_VER
uint8    g_ctc_debug_on = 1;
#else
uint8    g_ctc_debug_on = 0;
#endif
int32 ctc_debug_module_reg();

ctc_debug_stub_t* p_ctc_debug_stub_info = NULL;
ctc_debug_stub_new_t* p_ctc_debug_stub_new_info = NULL;
sal_task_t* ctc_log_to_file = NULL;
void* p_log_file = NULL;
ctc_debug_out_func g_ctc_debug_log_print_cb = NULL;
ctc_debug_out_func g_ctc_debug_show_print_cb = NULL;
/**************************************************************************
 *
 * Defines and Macros
 *
 **************************************************************************/
CTC_DEBUG_IMPLEMENT(qos, process);
CTC_DEBUG_IMPLEMENT(qos, queue);
CTC_DEBUG_IMPLEMENT(acl, acl);
CTC_DEBUG_IMPLEMENT(cpu, traffic);
CTC_DEBUG_IMPLEMENT(qos, policer);
CTC_DEBUG_IMPLEMENT(qos, class);

CTC_DEBUG_IMPLEMENT(l3if, l3if);
CTC_DEBUG_IMPLEMENT(ip, ipuc);
CTC_DEBUG_IMPLEMENT(ip, ipmc);
CTC_DEBUG_IMPLEMENT(ip, rpf);
CTC_DEBUG_IMPLEMENT(mcast, mcast);
CTC_DEBUG_IMPLEMENT(mpls, mpls);
CTC_DEBUG_IMPLEMENT(fcoe, fcoe);

CTC_DEBUG_IMPLEMENT(l2, fdb);
CTC_DEBUG_IMPLEMENT(l2, learning_aging);
CTC_DEBUG_IMPLEMENT(l2, stp);
CTC_DEBUG_IMPLEMENT(vlan, vlan_class);
CTC_DEBUG_IMPLEMENT(vlan, vlan_mapping);
CTC_DEBUG_IMPLEMENT(vlan, vlan_switching);
CTC_DEBUG_IMPLEMENT(vlan, protocol_vlan);
CTC_DEBUG_IMPLEMENT(vlan, vlan);
CTC_DEBUG_IMPLEMENT(mirror, mirror);
CTC_DEBUG_IMPLEMENT(dot1x, dot1x);
CTC_DEBUG_IMPLEMENT(security, security);
CTC_DEBUG_IMPLEMENT(oam, oam);
CTC_DEBUG_IMPLEMENT(oam, efm);
CTC_DEBUG_IMPLEMENT(oam, pbx);
CTC_DEBUG_IMPLEMENT(ptp, ptp);
CTC_DEBUG_IMPLEMENT(sync_ether, sync_ether);

CTC_DEBUG_IMPLEMENT(interrupt, interrupt);
CTC_DEBUG_IMPLEMENT(packet, packet);

CTC_DEBUG_IMPLEMENT(chip, chip);
CTC_DEBUG_IMPLEMENT(chip, peri);
CTC_DEBUG_IMPLEMENT(port, port);
CTC_DEBUG_IMPLEMENT(port, mac);
CTC_DEBUG_IMPLEMENT(port, cl73);
CTC_DEBUG_IMPLEMENT(linkagg, linkagg);
CTC_DEBUG_IMPLEMENT(nexthop, nexthop);
CTC_DEBUG_IMPLEMENT(scl, scl);
CTC_DEBUG_IMPLEMENT(opf, opf);
CTC_DEBUG_IMPLEMENT(fpa, fpa);
CTC_DEBUG_IMPLEMENT(stats, stats);
CTC_DEBUG_IMPLEMENT(alloc, alloc);
CTC_DEBUG_IMPLEMENT(memmngr, memmngr);
CTC_DEBUG_IMPLEMENT(dma, dma);

CTC_DEBUG_IMPLEMENT(parser, parser);
CTC_DEBUG_IMPLEMENT(pdu, pdu);
CTC_DEBUG_IMPLEMENT(chipagent, chipagent);
CTC_DEBUG_IMPLEMENT(aps, aps);

/*Stacking*/
CTC_DEBUG_IMPLEMENT(stacking, stacking);

CTC_DEBUG_IMPLEMENT(app, app);
CTC_DEBUG_IMPLEMENT(app, api);
CTC_DEBUG_IMPLEMENT(ftm, ftm);
CTC_DEBUG_IMPLEMENT(bpe, bpe);
CTC_DEBUG_IMPLEMENT(overlay_tunnel, overlay_tunnel);
CTC_DEBUG_IMPLEMENT(trill, trill);
CTC_DEBUG_IMPLEMENT(ipfix, ipfix);
CTC_DEBUG_IMPLEMENT(monitor, monitor);
CTC_DEBUG_IMPLEMENT(efd, efd);
CTC_DEBUG_IMPLEMENT(init, init);
CTC_DEBUG_IMPLEMENT(datapath, datapath);

CTC_DEBUG_IMPLEMENT(wlan, wlan);
CTC_DEBUG_IMPLEMENT(npm, npm);
CTC_DEBUG_IMPLEMENT(wb, wb);
CTC_DEBUG_IMPLEMENT(dot1ae, dot1ae);

CTC_DEBUG_IMPLEMENT(register, register);

CTC_DEBUG_IMPLEMENT(diag, diag);

int32 g_error_on = 0;

/****************************************************************************
 *
* SDK module add it's debug info to list
*
* NOTE:
* 1, pattern: CTC_DEBUG_MODULE_REG(module, submodule);
* 2, module and submodule name length should less than 20 characters
*
****************************************************************************/
int32
ctc_debug_module_reg()
{

    CTC_DEBUG_MODULE_REG(qos, process);
    CTC_DEBUG_MODULE_REG(qos, queue);
    CTC_DEBUG_MODULE_REG(acl, acl);
    CTC_DEBUG_MODULE_REG(cpu, traffic);
    CTC_DEBUG_MODULE_REG(qos, policer);
    CTC_DEBUG_MODULE_REG(qos, class);

    CTC_DEBUG_MODULE_REG(ip, ipuc);
    CTC_DEBUG_MODULE_REG(ip, ipmc);
    CTC_DEBUG_MODULE_REG(ip, rpf);
    CTC_DEBUG_MODULE_REG(mcast, mcast);
    CTC_DEBUG_MODULE_REG(mpls, mpls);
    CTC_DEBUG_MODULE_REG(l3if, l3if);
    CTC_DEBUG_MODULE_REG(fcoe, fcoe);

    CTC_DEBUG_MODULE_REG(l2, fdb);
    CTC_DEBUG_MODULE_REG(l2, learning_aging);
    CTC_DEBUG_MODULE_REG(l2, stp);
    CTC_DEBUG_MODULE_REG(vlan, vlan_class);
    CTC_DEBUG_MODULE_REG(vlan, vlan_mapping);
    CTC_DEBUG_MODULE_REG(vlan, vlan_switching);
    CTC_DEBUG_MODULE_REG(vlan, protocol_vlan);
    CTC_DEBUG_MODULE_REG(vlan, vlan);
    CTC_DEBUG_MODULE_REG(mirror, mirror);
    CTC_DEBUG_MODULE_REG(dot1x, dot1x);
    CTC_DEBUG_MODULE_REG(security, security);
    CTC_DEBUG_MODULE_REG(dot1ae, dot1ae);
    CTC_DEBUG_MODULE_REG(oam, oam);
    CTC_DEBUG_MODULE_REG(oam, efm);
    CTC_DEBUG_MODULE_REG(ptp, ptp);
    CTC_DEBUG_MODULE_REG(sync_ether, sync_ether);

    CTC_DEBUG_MODULE_REG(interrupt, interrupt);
    CTC_DEBUG_MODULE_REG(packet, packet);

    CTC_DEBUG_MODULE_REG(chip, chip);
    CTC_DEBUG_MODULE_REG(chip, peri);
    CTC_DEBUG_MODULE_REG(port, port);
    CTC_DEBUG_MODULE_REG(port, mac);
    CTC_DEBUG_MODULE_REG(port, cl73);
    CTC_DEBUG_MODULE_REG(linkagg, linkagg);
    CTC_DEBUG_MODULE_REG(nexthop, nexthop);
    CTC_DEBUG_MODULE_REG(scl, scl);
    CTC_DEBUG_MODULE_REG(opf, opf);
    CTC_DEBUG_MODULE_REG(fpa, fpa);
    CTC_DEBUG_MODULE_REG(stats, stats);
    CTC_DEBUG_MODULE_REG(alloc, alloc);
    CTC_DEBUG_MODULE_REG(memmngr, memmngr);
    CTC_DEBUG_MODULE_REG(dma, dma);

    CTC_DEBUG_MODULE_REG(parser, parser);
    CTC_DEBUG_MODULE_REG(pdu, pdu);
    CTC_DEBUG_MODULE_REG(chipagent, chipagent);
    CTC_DEBUG_MODULE_REG(aps, aps);
    CTC_DEBUG_MODULE_REG(stacking, stacking);
    CTC_DEBUG_MODULE_REG(bpe, bpe);
    CTC_DEBUG_MODULE_REG(overlay_tunnel, overlay_tunnel);
    CTC_DEBUG_MODULE_REG(trill, trill);
    CTC_DEBUG_MODULE_REG(ipfix, ipfix);
    CTC_DEBUG_MODULE_REG(monitor, monitor);
    CTC_DEBUG_MODULE_REG(efd, efd);
    CTC_DEBUG_MODULE_REG(init, init);
    CTC_DEBUG_MODULE_REG(datapath, datapath);
    CTC_DEBUG_MODULE_REG(app, app);
    CTC_DEBUG_MODULE_REG(app, api);
    CTC_DEBUG_MODULE_REG(ftm, ftm);

    CTC_DEBUG_MODULE_REG(wlan, wlan);
    CTC_DEBUG_MODULE_REG(dot1ae, dot1ae);
    CTC_DEBUG_MODULE_REG(npm, npm);
    CTC_DEBUG_MODULE_REG(wb, wb);

    CTC_DEBUG_MODULE_REG(register, register);

    CTC_DEBUG_MODULE_REG(diag, diag);

    return CTC_E_NONE;
}

int32
ctc_debug_init()
{

    ctc_debug_list = ctc_list_new();
    CTC_ERROR_RETURN(ctc_debug_module_reg());
    CTC_ERROR_RETURN(ctc_debug_enable(TRUE));

    g_ctc_debug_log_print_cb = sal_printf;
    g_ctc_debug_show_print_cb= sal_printf;

    return CTC_E_NONE;
}

int32
ctc_debug_cmd_handle(ctc_debug_list_t* info)
{
    ctc_debug_list_t* debug_info;
    struct ctc_listnode* node;

    CTC_LIST_LOOP(ctc_debug_list, debug_info, node)
    {
        if (0 == sal_strcmp(debug_info->module, info->module) &&
            0 == sal_strcmp(debug_info->submodule, info->submodule))
        {
            debug_info->flags = info->flags;
            break;
        }
    }
    return CTC_E_NONE;
}

int32
ctc_debug_cmd_clearall()
{

    struct ctc_listnode* node;
    ctc_debug_list_t* debug_info;

    CTC_LIST_LOOP(ctc_debug_list, debug_info, node)
    {
        debug_info->flags = 0;
    }

    return CTC_E_NONE;
}

int32
ctc_debug_enable(bool enable)
{
    g_ctc_debug_on = (enable == TRUE) ? 1 : 0;

    return CTC_E_NONE;
}

bool
ctc_get_debug_enable()
{
    return g_ctc_debug_on ? TRUE : FALSE;
}

void
ctc_debug_register_log_cb(ctc_debug_out_func func)
{
    g_ctc_debug_log_print_cb = func;
}

void
ctc_debug_register_cb(ctc_debug_out_func func)
{
    g_ctc_debug_show_print_cb = func;
}

int32
ctc_debug_set_flag(char* module, char* submodule, uint32 typeenum, uint8 debug_level, bool debug_on)
{

    ctc_debug_list_t* debug_info;
    struct ctc_listnode* node;

    CTC_LIST_LOOP(ctc_debug_list, debug_info, node)
    {
        if (0 == sal_strcmp(debug_info->module, module) &&
            0 == sal_strcmp(debug_info->submodule, submodule))
        {
            if (debug_on == TRUE)
            {
                debug_info->flags |= (1 << (typeenum));
                if (0 == typeenum)
                {
                    debug_info->level = debug_level;
                }
                else if (1 == typeenum)
                {
                    debug_info->level1 = debug_level;
                }
            }
            else
            {
                if(CTC_FLAG_ISSET(debug_info->level1, CTC_DEBUG_LEVEL_LOGFILE))
                {
                    ctc_debug_set_log_file(module, submodule, NULL, 0);
                }

                debug_info->flags &= ~(1 << (typeenum));
                if (0 == typeenum)
                {
                    debug_info->level = 0;
                }
                else if (1 == typeenum)
                {
                    debug_info->level1 = 0;
                }
            }

            break;
        }
    }
    return CTC_E_NONE;

}

bool
ctc_debug_get_flag(char* module, char* submodule, uint32 typeenum, uint8* level)
{

    ctc_debug_list_t* debug_info;
    struct ctc_listnode* node;

    CTC_LIST_LOOP(ctc_debug_list, debug_info, node)
    {
        if (0 == sal_strcmp(debug_info->module, module) &&
            0 == sal_strcmp(debug_info->submodule, submodule))
        {
            if (0 == typeenum)
            {
                *level = debug_info->level;
            }
            else if (1 == typeenum)
            {
                *level = debug_info->level1;
            }

            return (debug_info->flags & (1 << (typeenum))) ? TRUE : FALSE;
        }
    }
    return FALSE;

}

bool
ctc_debug_check_flag(ctc_debug_list_t flag_info, uint32 typeenum, uint8 level)
{
    if (0 == g_ctc_debug_on)
    {
        return FALSE;
    }

    if (level == CTC_DEBUG_LEVEL_DUMP)
    {
        return TRUE;
    }

    if (level == CTC_DEBUG_LEVEL_LOGFILE)
    {
        return TRUE;
    }

    if (typeenum < (sizeof(uint32) * 8))
    {
        if (flag_info.flags & (1 << (typeenum)))
        {
            if (((0 == typeenum) && (flag_info.level & level))
                || ((1 == typeenum) && (flag_info.level1 & level)))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void
ctc_debug_reg_log_func(log_func func, void* p_para)
{
    ctc_debug_write_info_para_t write_entry_para;

    if (func == NULL)
    {
        return;
    }

    sal_memcpy(&write_entry_para, (ctc_debug_write_info_para_t*)p_para, sizeof(ctc_debug_write_info_para_t));

    p_log_file = sal_fopen((char*)write_entry_para.file, "w+");
    if (NULL == p_log_file)
    {
        return;
    }

    /* log in file */
    if (0 != sal_task_create(&ctc_log_to_file,
                             "ctcLogFl",
                             SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, func, (void*)p_para))
    {
        sal_task_destroy(ctc_log_to_file);
        return;
    }

    return;
}

int32
ctc_debug_set_log_file(char* module, char* submodule, void* p_file_name, uint8 enable)
{
    ctc_debug_list_t* debug_info;
    struct ctc_listnode* node;
    sal_file_t fp = NULL;

    CTC_LIST_LOOP(ctc_debug_list, debug_info, node)
    {
        if (0 == sal_strcmp(debug_info->module, module) &&
            0 == sal_strcmp(debug_info->submodule, submodule))
        {
            if (enable)
            {
                fp = sal_fopen((char*)p_file_name, "w+");
                if (NULL == fp)
                {
                    return CTC_E_INVALID_PTR;
                }

                debug_info->p_file = fp;
            }
            else
            {
                fp = debug_info->p_file;
                if (fp)
                {
                    sal_fclose(fp);
                }
            }

            break;
        }
    }
    return CTC_E_NONE;

}


