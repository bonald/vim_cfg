/**
 @file ctc_bpe_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-14

 @version v2.0

 The file apply clis of bpe module
*/

#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_debug.h"
#include "ctc_bpe_cli.h"

CTC_CLI(ctc_cli_bpe_interlaken_en,
        ctc_cli_bpe_interlaken_en_cmd,
        "bpe interlaken (enable|disable)",
        "bpe module",
        "interlaken interface",
        "enable",
        "disable")
{
    int32 ret = 0;
    bool enable = FALSE;
    if (0 == sal_memcmp(argv[0], "enable", 3))
    {
        enable = TRUE;
    }
    else
    {
        enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_bpe_set_intlk_en(g_api_lchip, enable);
    }
    else
    {
        ret = ctc_bpe_set_intlk_en(enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_bpe_get_interlaken_en,
        ctc_cli_bpe_get_interlaken_en_cmd,
        "show bpe interlaken",
        CTC_CLI_SHOW_STR,
        "bpe module",
        "interlaken interface")
{
    int32 ret = 0;
    bool enable = FALSE;

    if(g_ctcs_api_en)
    {
        ret = ctcs_bpe_get_intlk_en(g_api_lchip, &enable);
    }
    else
    {
        ret = ctc_bpe_get_intlk_en(&enable);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("interlaken -enable               :%d\n", enable);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_bpe_set_bpe,
        ctc_cli_bpe_set_bpe_cmd,
        "bpe (((mux-demux|m-vepa|cb-cascade) port GPHYPORT_ID vlan-base VLAN_ID port-base PORT_ID extender-num NUM) |\
             (upstream port GPHYPORT_ID ecid-base ECID_BASE port-base PORT_ID) |\
             (extended port GPHYPORT_ID ecid ECID) |\
             (pe-cascade port GPHYPORT_ID ecid-base ECID_BASE extender-num NUM) )\
             (enable|disable)",
        "bpe module",
        CTC_CLI_BPE_MUX_DEMUX,
        CTC_CLI_8021QBG_M_VEPA,
        CTC_CLI_8021QBR_CB_CASCADE,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        CTC_CLI_VLAN_BASE_DESC,
        CTC_CLI_VLAN_RANGE_DESC,
        "extension port start base",
        CTC_CLI_PORT_INTERNAL_PORT_START,
        "extender number",
        CTC_CLI_PORT_EXTENDER_NUM,
        CTC_CLI_8021QBR_UPSTREAM,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ECID base",
        CTC_CLI_VLAN_RANGE_DESC,
        "extension port start base",
        CTC_CLI_PORT_INTERNAL_PORT_START,
        CTC_CLI_8021QBR_EXTENDED,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "802.1BR ECID",
        CTC_CLI_VLAN_RANGE_DESC,
        CTC_CLI_8021QBR_PE_CASCADE,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPHYPORT_ID_DESC,
        "ECID base",
        CTC_CLI_VLAN_RANGE_DESC,
        "extender number",
        CTC_CLI_PORT_EXTENDER_NUM,
        "enable",
        "disable")
{
    int32  ret = 0;
    uint8  index = 0;
    uint16 gport = 0;
    ctc_bpe_extender_t bpe_ext;

    sal_memset(&bpe_ext, 0, sizeof(ctc_bpe_extender_t));

    index = CTC_CLI_GET_ARGC_INDEX("mux-demux");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_MUX_DEMUX_MODE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("m-vepa");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_8021QBG_M_VEPA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cb-cascade");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_8021BR_CB_CASCADE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pe-cascade");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_8021BR_PE_CASCADE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("extended");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_8021BR_EXTENDED;
    }

    index = CTC_CLI_GET_ARGC_INDEX("upstream");
    if (0xFF != index)
    {
        bpe_ext.mode = CTC_BPE_8021BR_UPSTREAM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("gport", gport, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("vlan base", bpe_ext.vlan_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("internal-base", bpe_ext.port_base, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("extender-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("extender num", bpe_ext.port_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("ecid", bpe_ext.vlan_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecid-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("ecid base", bpe_ext.vlan_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        bpe_ext.enable = TRUE;
    }
    else
    {
        bpe_ext.enable = FALSE;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_bpe_set_port_extender(g_api_lchip, gport, &bpe_ext);
    }
    else
    {
        ret = ctc_bpe_set_port_extender(gport, &bpe_ext);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}



#if (FEATURE_MODE == 0)
CTC_CLI(ctc_cli_bpe_add_forward_entry,
        ctc_cli_bpe_add_forward_entry_cmd,
        "bpe (add|remove) forward-entry  (name-space NAMESPACE ecid ECID) (port GPORT (is-extend-port|) | mc-grp-id GRPID |) (copy-to-cpu | redirect-to-cpu|) ",
        "bpe module",
        "Add",
        "Remove",
        "Forward entry",        
        "Name space",
        "Name space value",
        "Ecid",
        "Ecid value",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Is extend port",
        "Mcast group id",
        "Group id value",
        "Copy to cpu",
        "Redirect to cpu")
{
    int32  ret = 0;
    uint8  index = 0;
    uint8 is_del = 0;
    ctc_bpe_8021br_forward_t bpe_fwd;

    sal_memset(&bpe_fwd, 0, sizeof(ctc_bpe_8021br_forward_t));

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (0xFF != index)
    {
        is_del = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("gport", bpe_fwd.dest_port, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("name-space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("name-space", bpe_fwd.name_space, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("ecid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("Ecid", bpe_fwd.ecid, argv[index + 1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX("mc-grp-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("Mcast group id", bpe_fwd.mc_grp_id, argv[index + 1]);
        CTC_SET_FLAG(bpe_fwd.flag, CTC_BPE_FORWARD_MCAST);
   
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-extend-port");
    if (0xFF != index)
    {
        CTC_SET_FLAG(bpe_fwd.flag, CTC_BPE_FORWARD_EXTEND_PORT);
    }

    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
    if (0xFF != index)
    {
        CTC_SET_FLAG(bpe_fwd.flag, CTC_BPE_FORWARD_COPY_TO_CPU);
    }

    index = CTC_CLI_GET_ARGC_INDEX("redirect-to-cpu");
    if (0xFF != index)
    {
        CTC_SET_FLAG(bpe_fwd.flag, CTC_BPE_FORWARD_REDIRECT_TO_CPU);
    }
	
	

    if (!is_del)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_bpe_add_8021br_forward_entry(g_api_lchip, &bpe_fwd);
        }
        else
        {
            ret = ctc_bpe_add_8021br_forward_entry(&bpe_fwd);
        }  
    }
    else
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_bpe_remove_8021br_forward_entry(g_api_lchip, &bpe_fwd);
        }
        else
        {
            ret = ctc_bpe_remove_8021br_forward_entry(&bpe_fwd);
        }
    }
	
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;
}

#endif

CTC_CLI(ctc_cli_bpe_show_bpe,
        ctc_cli_bpe_show_bpe_cmd,
        "show bpe port GPHYPORT_ID",
        CTC_CLI_SHOW_STR,
        "bpe module",
        CTC_CLI_PORT_M_STR,
        CTC_CLI_GPHYPORT_ID_DESC)
{
    int32 ret = 0;
    uint16 gport = 0;
    bool enable = FALSE;

    CTC_CLI_GET_UINT16("gport", gport, argv[0]);

    if(g_ctcs_api_en)
    {
        ret = ctcs_bpe_get_port_extender(g_api_lchip, gport, &enable);
    }
    else
    {
        ret = ctc_bpe_get_port_extender(gport, &enable);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret = %d, %s \n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("bpe  -enable               :%d\n", enable);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_bpe_debug_on,
        ctc_cli_bpe_debug_on_cmd,
        "debug bpe (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "bpe module",
        "CTC Layer",
        "SYS Layer",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;
    uint8 index = 0;

    index = CTC_CLI_GET_ARGC_INDEX("debug-level");
    if (index != 0xFF)
    {
        level = CTC_DEBUG_LEVEL_NONE;
        index = CTC_CLI_GET_ARGC_INDEX("func");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_FUNC;
        }

        index = CTC_CLI_GET_ARGC_INDEX("param");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_PARAM;
        }

        index = CTC_CLI_GET_ARGC_INDEX("info");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_INFO;
        }

        index = CTC_CLI_GET_ARGC_INDEX("error");
        if (index != 0xFF)
        {
            level |= CTC_DEBUG_LEVEL_ERROR;
        }
    }

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = BPE_CTC;
    }
    else
    {
        typeenum = BPE_SYS;
    }

    ctc_debug_set_flag("bpe", "bpe", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_bpe_debug_off,
        ctc_cli_bpe_debug_off_cmd,
        "no debug bpe (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "bpe module",
        "CTC Layer",
        "SYS Layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = BPE_CTC;
    }
    else
    {
        typeenum = BPE_SYS;
    }

    ctc_debug_set_flag("bpe", "bpe", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

int32
ctc_bpe_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_bpe_interlaken_en_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_bpe_get_interlaken_en_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_bpe_set_bpe_cmd);

#if (FEATURE_MODE == 0)
    install_element(CTC_SDK_MODE, &ctc_cli_bpe_add_forward_entry_cmd);
#endif

    install_element(CTC_SDK_MODE, &ctc_cli_bpe_show_bpe_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_bpe_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_bpe_debug_off_cmd);

    return CLI_SUCCESS;
}

