/**
 @file ctc_pdu_cli.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-1-11

 @version v2.0

This file contains pdu module cli command
*/

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_pdu_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_pdu.h"
#include "ctc_error.h"
#include "ctc_debug.h"

#define CTC_CLI_MAC_FORMAT_PDU_MAC "MAC address in HHHH.HHHH.HHHH format,0180.C2XX.XXXX"
#define CTC_CLI_MAC_FORMAT_PDU_MASK "MAC address in HHHH.HHHH.HHHH format,FFFF.FFXX.XXXX"

CTC_CLI(ctc_cli_pdu_debug_on,
        ctc_cli_pdu_debug_on_cmd,
        "debug pdu (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PDU_STR,
        "Ctc layer",
        "Sys layer",
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
        typeenum = PDU_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PDU_SYS;
    }

    ctc_debug_set_flag("pdu", "pdu", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_debug_off,
        ctc_cli_pdu_debug_off_cmd,
        "no debug pdu (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PDU_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PDU_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PDU_SYS;
    }

    ctc_debug_set_flag("pdu", "pdu", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_show_debug,
        ctc_cli_pdu_show_debug_cmd,
        "show debug pdu (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_PDU_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = PDU_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = PDU_SYS;
    }

    en = ctc_debug_get_flag("pdu", "pdu", typeenum, &level);
    ctc_cli_out("pdu:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l2_classify_l2pdu_by_macda,
        ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd,
        "pdu l2pdu classify macda-48bit index INDEX macda MACDA (mask MASK|) ",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Base on 48bit macda",
        "Entry index",
        CTC_CLI_L2PDU_MACDA_48BIT_ENTRY_INDEX,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        "Macda mask",
        CTC_CLI_MAC_FORMAT)
{
    ctc_pdu_l2pdu_key_t entry;
    uint8 index = 0;
    uint8 prefix_index = 0;
    int32 ret = 0;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));
    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA;

    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    CTC_CLI_GET_MAC_ADDRESS("dest mac address", entry.l2pdu_by_mac.mac, argv[1]);

    prefix_index = CTC_CLI_GET_ARGC_INDEX("mask");

    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("dest mac address mask", entry.l2pdu_by_mac.mac_mask, argv[prefix_index + 1]);

    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_classify_l2pdu(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_classify_l2pdu(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_show_l2_classify_l2pdu_by_macda,
        ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd,
        "show pdu l2pdu classify macda-48bit index INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Base on 48bit macda",
        "Entry index",
        CTC_CLI_L2PDU_MACDA_48BIT_ENTRY_INDEX)
{
    ctc_pdu_l2pdu_type_t l2pdu_type;
    ctc_pdu_l2pdu_key_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_get_classified_key(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_get_classified_key(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("mac:%.2x%.2x.%.2x%.2x.%.2x%.2x\n", entry.l2pdu_by_mac.mac[0], entry.l2pdu_by_mac.mac[1],
                entry.l2pdu_by_mac.mac[2], entry.l2pdu_by_mac.mac[3], entry.l2pdu_by_mac.mac[4], entry.l2pdu_by_mac.mac[5]);

    ctc_cli_out("mac_mask:%.2x%.2x.%.2x%.2x.%.2x%.2x\n", entry.l2pdu_by_mac.mac_mask[0], entry.l2pdu_by_mac.mac_mask[1],
                entry.l2pdu_by_mac.mac_mask[2], entry.l2pdu_by_mac.mac_mask[3], entry.l2pdu_by_mac.mac_mask[4], entry.l2pdu_by_mac.mac_mask[5]);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24,
        ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd,
        "pdu l2pdu classify macda-24bit index INDEX macda MACDA (mask MASK|)",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Base on 24bit macda",
        "Entry index",
        CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT_PDU_MAC,
        CTC_CLI_MACDA_24BIT_MASK_DESC,
        CTC_CLI_MAC_FORMAT_PDU_MASK)
{
    ctc_pdu_l2pdu_key_t entry;
    uint8 index = 0;
    uint8 prefix_index = 0;
    int32 ret = 0;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));
    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA_LOW24;

    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    CTC_CLI_GET_MAC_ADDRESS("dest mac address", entry.l2pdu_by_mac.mac, argv[1]);

    prefix_index = CTC_CLI_GET_ARGC_INDEX("mask");

    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_MAC_ADDRESS("dest mac address mask", entry.l2pdu_by_mac.mac_mask, argv[prefix_index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_classify_l2pdu(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_classify_l2pdu(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24,
        ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd,
        "show pdu l2pdu classify macda-24bit index INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Base on 24bit macda",
        "Entry index",
        CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX)
{
    ctc_pdu_l2pdu_type_t l2pdu_type;
    ctc_pdu_l2pdu_key_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));

    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA_LOW24;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_get_classified_key(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_get_classified_key(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("mac:%.2x%.2x.%.2x%.2x.%.2x%.2x\n", 0x01, 0x80,
                0xc2, entry.l2pdu_by_mac.mac[3], entry.l2pdu_by_mac.mac[4], entry.l2pdu_by_mac.mac[5]);

    ctc_cli_out("mac_mask:%.2x%.2x.%.2x%.2x.%.2x%.2x\n", 0xff, 0xff,
                0xff, entry.l2pdu_by_mac.mac_mask[3], entry.l2pdu_by_mac.mac_mask[4], entry.l2pdu_by_mac.mac_mask[5]);
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto,
        ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd,
        "pdu l2pdu classify l2hdr-proto index  INDEX  l2hdr-proto L2_HDR_PROTO",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Base on layer2 header protocol",
        "Entry index",
        "<0-15>",
        "Layer2 header protocol",
        "<0x0 - 0xFFFF>")
{
    ctc_pdu_l2pdu_key_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    uint8 index = 0;
    int32 ret = 0;
    uint16 l2hdr_proto = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));
    l2pdu_type = CTC_PDU_L2PDU_TYPE_L2HDR_PROTO;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("l2 hdr ptl", l2hdr_proto, argv[1], 0, CTC_MAX_UINT16_VALUE);

    entry.l2hdr_proto = l2hdr_proto;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_classify_l2pdu(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_classify_l2pdu(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto,
        ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd,
        "show pdu l2pdu classify l2hdr-proto index INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Classify pdu based on type",
        "Layer2 header protocol",
        "Entry index",
        "<0-15>")
{
    ctc_pdu_l2pdu_type_t l2pdu_type;
    ctc_pdu_l2pdu_key_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l2pdu_key_t));
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    l2pdu_type = CTC_PDU_L2PDU_TYPE_L2HDR_PROTO;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_get_classified_key(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_get_classified_key(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Layer2 header protocol:0x%x\n", entry.l2hdr_proto);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l2_set_global_action_by_macda,
        ctc_cli_pdu_l2_set_global_action_by_macda_cmd,
        "pdu l2pdu global-action macda-48bit index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Base on 48bit Macda",
        "Entry index",
        CTC_CLI_L2PDU_MACDA_48BIT_ENTRY_INDEX,
        "Entry valid",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Bypass all",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 prefix_index = 0;
    ctc_pdu_global_l2pdu_action_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("entry valid", entry.entry_valid, argv[1], 0, CTC_MAX_UINT8_VALUE);

    prefix_index = CTC_CLI_GET_ARGC_INDEX("bypass-all");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("action-index", entry.action_index, argv[prefix_index + 2], 0, CTC_MAX_UINT8_VALUE);
    }
    else
    {
        CTC_CLI_GET_UINT8_RANGE("action-index", entry.action_index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_set_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l2_set_global_action_by_macda_low24,
        ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd,
        "pdu l2pdu global-action macda-24bit index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Base on 24bit Macda",
        "Entry index",
        CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX,
        "Entry valid",
        "<0-0xFF>,when great than 1,as 1 to process",
        "By pass all",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 prefix_index = 0;
    ctc_pdu_global_l2pdu_action_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA_LOW24;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("entry valid", entry.entry_valid, argv[1], 0, CTC_MAX_UINT8_VALUE);

    prefix_index = CTC_CLI_GET_ARGC_INDEX("bypass-all");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[prefix_index + 2], 0, CTC_MAX_UINT8_VALUE);
    }
    else
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_set_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto,
        ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd,
        "pdu l2pdu global-action l2hdr-proto index INDEX entry-valid ENTRY_VALID (bypass-all BYPASS_ALL|) action-index ACTION_INDEX",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Base on layer2 header protocol",
        "Entry index",
        "<0-15>",
        "Entry valid",
        "<0-0xFF>,when great than 1,as 1 to process",
        "By pass all",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 prefix_index = 0;
    ctc_pdu_global_l2pdu_action_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    l2pdu_type = CTC_PDU_L2PDU_TYPE_L2HDR_PROTO;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("entry valid", entry.entry_valid, argv[1], 0, CTC_MAX_UINT8_VALUE);
    prefix_index = CTC_CLI_GET_ARGC_INDEX("bypass-all");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[prefix_index + 2], 0, CTC_MAX_UINT8_VALUE);
    }
    else
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_set_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l2_set_global_action_by_layer3_type,
        ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd,
        "pdu l2pdu global-action l3type index INDEX (entry-valid ENTRY_VALID bypass-all BYPASS_ALL|) action-index ACTION_INDEX copy-to-cpu COPY_TO_CPU",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Based on layer3 type",
        "Layer3 type index",
        "<0-15>",
        "Entry valid",
        "<0-0xFF>,when great than 1,as 1 to process",
        "By pass all",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Per-port action index",
        CTC_CLI_L2PDU_L3TYPE_ACTION_INDEX,
        "Copy to cpu",
        "<0-0xFF>,when great than 1,as 1 to process")
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 prefix_index = 0;
    ctc_pdu_global_l2pdu_action_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    l2pdu_type = CTC_PDU_L2PDU_TYPE_L3TYPE;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    prefix_index = CTC_CLI_GET_ARGC_INDEX("entry-valid");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("entry valid", entry.entry_valid, argv[prefix_index + 1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[prefix_index + 4], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("copy to cpu", entry.copy_to_cpu, argv[prefix_index + 5], 0, CTC_MAX_UINT8_VALUE);
    }
    else
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[1], 0, CTC_MAX_UINT8_VALUE);
        CTC_CLI_GET_UINT8_RANGE("copy to cpu", entry.entry_valid, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_set_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l2_set_global_action_by_bpdu,
        ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd,
        "pdu l2pdu global-action (bpdu|slow-proto|efm-oam|eapol|lldp|fip|isis) (entry-valid ENTRY_VALID) (copy-to-cpu COPY_TO_CPU bypass-all BYPASS_ALL|)  (action-index ACTION_INDEX)",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Bpdu",
        "Slow protocol",
        "Efm oam",
        "Eapol",
        "Lldp",
        "Fip",
        "ISIS",
        "Entry valid",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Copy to cpu",
        "<0-0xFF>,when great than 1,as 1 to process",
        "By pass all",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8 prefix_index = 0, index1 = 0;
    ctc_pdu_global_l2pdu_action_t entry;
    ctc_pdu_l2pdu_type_t l2pdu_type = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    prefix_index = CTC_CLI_GET_ARGC_INDEX("bpdu");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_BPDU;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("slow-proto");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_SLOW_PROTO;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("efm-oam");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_EFM_OAM;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("eapol");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_EAPOL;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("lldp");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_LLDP;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("fip");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_FIP;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("isis");
    if (prefix_index != 0xFF)
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_ISIS;
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("entry-valid");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("entry-valid", entry.entry_valid, argv[prefix_index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    prefix_index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
    if (prefix_index != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("copy to cpu", entry.copy_to_cpu, argv[prefix_index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index1 = CTC_CLI_GET_ARGC_INDEX("action-index");
    if (index1 != 0xFF)
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[index1 + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_set_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_show_l2_global_action,
        ctc_cli_pdu_show_l2_global_action_cmd,
        "show pdu l2pdu global-action \
        (macda-48bit index INDEX|macda-24bit index INDEX|l2hdr-proto index INDEX|bpdu|slow-proto|efm-oam|eapol|lldp|fip|isis|l3type index INDEX)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Global action",
        "Base on 48bit Macda",
        "Entry index",
        "<0-3>",
        "Base on 24bit Macda",
        "Entry index",
        CTC_CLI_L2PDU_24BIT_MACDA_ENTRY_INDEX,
        "Base on layer2 header protocol",
        "Entry index",
        "<0-15>",
        "Bpdu",
        "Slow protocol",
        "Efm oam",
        "Eapol",
        "Lldp",
        "Fip",
        "ISIS",
        "Layer3 type",
        "Entry index",
        "<0-15>")
{
    ctc_pdu_l2pdu_type_t l2pdu_type = MAX_CTC_PDU_L2PDU_TYPE;
    ctc_pdu_global_l2pdu_action_t entry;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l2pdu_action_t));

    if (0 == sal_memcmp("macda-48bit", argv[0], sal_strlen("macda-48bit")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA;
        CTC_CLI_GET_UINT8_RANGE("index", index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if (0 == sal_memcmp("macda-24bit", argv[0], sal_strlen("macda-24bit")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_MACDA_LOW24;
        CTC_CLI_GET_UINT8_RANGE("index", index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if (0 == sal_memcmp("l2hdr-proto", argv[0], sal_strlen("l2hdr-proto")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_L2HDR_PROTO;
        CTC_CLI_GET_UINT8_RANGE("index", index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if (0 == sal_memcmp("l3type", argv[0], sal_strlen("l3type")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_L3TYPE;
        CTC_CLI_GET_UINT8_RANGE("index", index, argv[2], 0, CTC_MAX_UINT8_VALUE);
    }

    if (0 == sal_memcmp("bpdu", argv[0], sal_strlen("bpdu")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_BPDU;
    }

    if (0 == sal_memcmp("slow-proto", argv[0], sal_strlen("slow-proto")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_SLOW_PROTO;
    }

    if (0 == sal_memcmp("efm-oam", argv[0], sal_strlen("efm-oam")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_EFM_OAM;
    }

    if (0 == sal_memcmp("eapol", argv[0], sal_strlen("eapol")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_EAPOL;
    }

    if (0 == sal_memcmp("lldp", argv[0], sal_strlen("lldp")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_LLDP;
    }

    if (0 == sal_memcmp("fip", argv[0], sal_strlen("fip")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_FIP;
    }

    if (0 == sal_memcmp("isis", argv[0], sal_strlen("isis")))
    {
        l2pdu_type = CTC_PDU_L2PDU_TYPE_ISIS;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_get_global_action(g_api_lchip, l2pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l2pdu_get_global_action(l2pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("entry_valid:0x%x\n", entry.entry_valid);
    ctc_cli_out("copy_to_cpu:0x%x\n", entry.copy_to_cpu);
    ctc_cli_out("action_index:0x%x\n", entry.action_index);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l2_set_port_action,
        ctc_cli_pdu_l2_set_port_action_cmd,
        "pdu l2pdu port-action port PORT action-index ACTION_INDEX action-type (forward|copy-to-cpu|redirect-to-cpu|discard)",
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Per-port l2pdu action",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX,
        "Action type",
        "Forward packet",
        "Forward and send to cpu",
        "Direct to cpu",
        "Discard packet")
{
    ctc_pdu_port_l2pdu_action_t action = MAX_CTC_PDU_L2PDU_ACTION_TYPE;
    uint8 action_index = 0;
    int32 ret = 0;

    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("action index", action_index, argv[1], 0, CTC_MAX_UINT8_VALUE);
    if (0 == sal_memcmp("forward", argv[2], 7))
    {
        action = CTC_PDU_L2PDU_ACTION_TYPE_FWD;
    }

    if (0 == sal_memcmp("copy", argv[2], 4))
    {
        action = CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU;
    }

    if (0 == sal_memcmp("redirect", argv[2], 8))
    {
        action = CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
    }

    if (0 == sal_memcmp("discard", argv[2], 7))
    {
        action = CTC_PDU_L2PDU_ACTION_TYPE_DISCARD;
    }


    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_set_port_action(g_api_lchip, gport, action_index, action);
    }
    else
    {
        ret = ctc_l2pdu_set_port_action(gport, action_index, action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_show_l2_port_action,
        ctc_cli_pdu_show_l2_port_action_cmd,
        "show pdu l2pdu port-action port GPORT_ID action-index ACTION_INDEX action-type",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L2_PDU_STR,
        "Per-port l2pdu action",
        "Port",
        CTC_CLI_GPHYPORT_ID_DESC,
        "Per-port action index",
        CTC_CLI_L2PDU_PER_PORT_ACTION_INDEX,
        "Action type")
{
    ctc_pdu_port_l2pdu_action_t action;
    uint8 action_index = 0;
    int32 ret = 0;

    uint16 gport = 0;

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("action index", action_index, argv[1], 0, CTC_MAX_UINT8_VALUE);


    if(g_ctcs_api_en)
    {
        ret = ctcs_l2pdu_get_port_action(g_api_lchip, gport, action_index, &action);
    }
    else
    {
        ret = ctc_l2pdu_get_port_action(gport, action_index, &action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (action == CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU)
    {
        ctc_cli_out(" gport:0x%04x action index:%d action:%s  \n", gport, action_index, "redirect to cpu");
    }
    else if (action == CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU)
    {
        ctc_cli_out("gport:0x%04x action index:%d action:%s  \n", gport, action_index, "copy to cpu");
    }
    else if (action == CTC_PDU_L2PDU_ACTION_TYPE_FWD)
    {
        ctc_cli_out(" gport:0x%04x action index:%d action:%s  \n", gport, action_index, "normal forwarding");
    }
    else if (action == CTC_PDU_L2PDU_ACTION_TYPE_DISCARD)
    {
        ctc_cli_out(" gport:0x%04x action index:%d action:%s  \n", gport, action_index, "discard ");
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l3_classify_l3pdu_by_l3proto,
        ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd,
        "pdu l3pdu classify layer3-proto index INDEX layer3-proto LAYER3_PROTO",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Classify pdu base on layer3 protocol",
        "Base on layer3 protocol",
        "Entry index",
        CTC_CLI_L3PDU_ENTRY_INDEX,
        "Layer3 header protocol",
        "<0-255>")
{
    ctc_pdu_l3pdu_key_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type;

    uint8 index = 0;
    int32 ret = 0;
    uint8 l3hdr_proto = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l3pdu_key_t));
    l3pdu_type = CTC_PDU_L3PDU_TYPE_L3HDR_PROTO;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);

    CTC_CLI_GET_UINT8_RANGE("l3 hdr ptl", l3hdr_proto, argv[1], 0, CTC_MAX_UINT8_VALUE);
    entry.l3hdr_proto = l3hdr_proto;

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_classify_l3pdu(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_classify_l3pdu(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l3_classify_l3pdu_by_l4port,
        ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd,
        "pdu l3pdu classify layer4-port index INDEX is-udp UDP_EN_VAL is-tcp TCP_EN_VAL destport DESTPORT",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Classify pdu based on layer3 protocol",
        "Layer4 port",
        "Entry index",
        CTC_CLI_L3PDU_ENTRY_INDEX,
        "Udp packet",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Tcp packet",
        "<0-0xFF>,when great than 1,as 1 to process",
        "Layer4 dest port",
        "<0-0xFFFF>")
{
    ctc_pdu_l3pdu_key_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type;

    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l3pdu_key_t));
    l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_PORT;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is udp", entry.l3pdu_by_port.is_udp, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is tcp", entry.l3pdu_by_port.is_tcp, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("l4-dest-port", entry.l3pdu_by_port.dest_port, argv[3], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_classify_l3pdu(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_classify_l3pdu(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;

}


CTC_CLI(ctc_cli_pdu_l3_classify_l3pdu_by_cxp,
        ctc_cli_pdu_l3_classify_l3pdu_by_cxp_cmd,
        "pdu l3pdu classify cxp destport DESTPORT",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Classify CXP (CID Exchange protocol)",
        "Layer4 dest port",
        "<0-0xFFFF>")
{

    ctc_pdu_l3pdu_key_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type;

    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l3pdu_key_t));
    l3pdu_type = CTC_PDU_L3PDU_TYPE_CXP;
    CTC_CLI_GET_UINT16_RANGE("l4-dest-port", entry.l3pdu_by_port.dest_port, argv[0], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_classify_l3pdu(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_classify_l3pdu(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }


    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l3_classify_l3pdu_by_ipda,
        ctc_cli_pdu_l3_classify_l3pdu_by_ipda_cmd,
        "pdu l3pdu classify layer3-ipda index INDEX is-ipv4 IS_IPV4_VAL is-ipv6 IS_IPV6_VAL ipda IP_ADDR",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Classify pdu base on layer3 ipda",
        "Base on layer3 ipda",
        "Entry index",
        CTC_CLI_L3PDU_IPDA_ENTRY_INDEX,
        "IPv4 packet",
        "<0-0xFF>,when great than 1,as 1 to process",
        "IPv6 packet",
        "<0-0xFF>, when great than 1,as 1 to process",
        "ip da address, applied for IPv4 ipda[11, 8]; IPv6 ipda[115, 112]",
        "<0-0xFFF>")
{
    ctc_pdu_l3pdu_key_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l3pdu_key_t));

    l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER3_IPDA;
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is-ipv4", entry.l3pdu_by_ipda.is_ipv4, argv[1], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT8_RANGE("is-ipv6", entry.l3pdu_by_ipda.is_ipv6, argv[2], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_UINT16_RANGE("ipda", entry.l3pdu_by_ipda.ipda, argv[3], 0, CTC_MAX_UINT16_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_classify_l3pdu(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_classify_l3pdu(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_pdu_show_l3_classify_l3pdu,
        ctc_cli_pdu_show_l3_classify_l3pdu_cmd,
        "show pdu l3pdu classify (layer3-proto|layer4-port|ipda|cxp) index INDEX",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Classify pdu based on layer3 protocol",
        "Layer3 protocol",
        "Layer4 port",
        "IP address",
        "CXP (CID Exchange Protocol)",
        "Entry index",
        CTC_CLI_L3PDU_ENTRY_INDEX)
{
    ctc_pdu_l3pdu_key_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type = MAX_CTC_PDU_L3PDU_TYPE;

    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_l3pdu_key_t));

    if (0 == sal_memcmp("layer3-proto", argv[0], sal_strlen("layer3-proto")))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_L3HDR_PROTO;
    }

    if (0 == sal_memcmp("layer4-port", argv[0], sal_strlen("layer4-port")))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_PORT;
    }

    if (0 == sal_memcmp("ipda", argv[0], sal_strlen("ipda")))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER3_IPDA;
    }
   if (0 == sal_memcmp("cxp", argv[0], sal_strlen("cxp")))
    {

        l3pdu_type = CTC_PDU_L3PDU_TYPE_CXP;

    }
    CTC_CLI_GET_UINT8_RANGE("index", index, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_get_classified_key(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_get_classified_key(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (0 == sal_memcmp("layer3-proto", argv[0], sal_strlen("layer3-proto")))
    {
        ctc_cli_out("l3hdr_proto:0x%x\n", entry.l3hdr_proto);
    }

    if (0 == sal_memcmp("layer4-port", argv[0], sal_strlen("layer4-port"))

		|| l3pdu_type == CTC_PDU_L3PDU_TYPE_CXP

		)
    {
        ctc_cli_out("is_tcp_val:0x%x\n", entry.l3pdu_by_port.is_tcp);
        ctc_cli_out("is_udp_val:0x%x\n", entry.l3pdu_by_port.is_udp);
        ctc_cli_out("dest_port:0x%x\n", entry.l3pdu_by_port.dest_port);
    }

    if (0 == sal_memcmp("ipda", argv[0], sal_strlen("ipda")))
    {
        ctc_cli_out("\n");
        ctc_cli_out(" Item         Value\n");
        ctc_cli_out(" ------------------\n", entry.l3pdu_by_ipda.is_ipv4);
        ctc_cli_out(" IsIPv4       %u\n", entry.l3pdu_by_ipda.is_ipv4);
        ctc_cli_out(" IsIPv6       %u\n", entry.l3pdu_by_ipda.is_ipv6);
        ctc_cli_out(" IPDA         %u\n", entry.l3pdu_by_ipda.ipda);
        ctc_cli_out(" ActionIndex  %u\n\n", index);
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_pdu_l3_set_global_action,
        ctc_cli_pdu_l3_set_global_action_cmd,
        "pdu l3pdu global-action (((ospf|pim|vrrp|bgp|rsvp|rip|msdp|ldp|ipv4-ospf|ipv6-ospf|ipv4-pim|ipv6-pim|\
        ipv4-vrrp|ipv6-vrrp|ipv4-bgp|ipv6-bgp|ipv6-icmp-rs|ipv6-icmp-ra|ipv6-icmp-ns|ipv6-icmp-na|ipv6-icmp-redirect|ripng|\
        isis|igmp-query|igmp-report-leave|igmp-unknown|mld-query|mld-report-done|mld-unknown|l3pdu-type L3PDU_TYPE|\
        (((layer3-proto|layer4-port|layer4-type) index INDEX) | (layer3-ipda index INDEX))) \
        entry-valid ENTRY_VALID action-index ACTION_INDEX) | (arp entry-valid ENTRY_VALID \
        action-index ACTION_INDEX) | (dhcp entry-valid ENTRY_VALID action-index ACTION_INDEX))",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Global action",
        "OSPF both IPv4 and IPv6",
        "PIM  both IPv4 and IPv6",
        "VRRP both IPv4 and IPv6",
        "BGP  both IPv4 and IPv6",
        "RSVP",
        "RIP",
        "MSDP",
        "LDP",
        "IPv4 OSPF",
        "IPv6 OSPF",
        "IPv4 PIM",
        "IPv6 PIM",
        "IPv4 VRRP",
        "IPv6 VRRP",
        "IPv4 BGP",
        "IPv6 BGP",
        "IPv6 ICMP RS",
        "IPv6 ICMP RA",
        "IPv6 ICMP NS",
        "IPv6 ICMP NA",
        "IPv6 ICMP REDIRECT",
        "RIPNG",
        "ISIS",
        "IGMP QUERY",
        "IGMP REPORT AND LEAVE",
        "IGMP UNKNOWN",
        "MLD QUERY",
        "MLD REPORT AND DONE",
        "MLD UNKNOWN",
        "l3 pdu type",
        "l3 pdu type,refer to ctc_pdu_l3pdu_type_t",
        "Base on layer3 protocol",
        "Base on layer4 port",
        "Base on layer4 type",
        "Entry index",
        "<0-7>",
        "Base on layer3 ipda",
        "Entry index",
        "<0-3>",
        "Entry valid",
        "<0-0xFF>, when great than 1,as 1 to process",
        "Action index",
        CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX,
        "ARP",
        "Entry valid",
        "<0-0xFF>, when great than 1,as 1 to process",
        "Action index",
        CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX,
        "DHCP",
        "Entry valid",
        "<0-0xFF>, when great than 1,as 1 to process",
        "Action index",
        CTC_CLI_L3PDU_GLOBAL_ACTION_INDEX)
{
    ctc_pdu_global_l3pdu_action_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type = MAX_CTC_PDU_L3PDU_TYPE;
    uint8 entry_index = 0;
    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l3pdu_action_t));

    index = CTC_CLI_GET_ARGC_INDEX("ospf");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_OSPF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pim");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_PIM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vrrp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_VRRP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rip");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_RIP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("rsvp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_RSVP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bgp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_BGP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("msdp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MSDP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ldp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LDP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ldp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LDP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-ospf");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV4OSPF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-ospf");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV6OSPF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-pim");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV4PIM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-pim");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV6PIM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-vrrp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV4VRRP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-vrrp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV6VRRP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-bgp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV4BGP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-bgp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IPV6BGP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-icmp-rs");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-icmp-ra");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-icmp-ns");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-icmp-na");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6-icmp-redirect");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ripng");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_RIPNG;
    }

    index = CTC_CLI_GET_ARGC_INDEX("isis");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ISIS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("igmp-query");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_QUERY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("igmp-report-leave");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("igmp-unknown");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mld-query");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_QUERY;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mld-report-done");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mld-unknown");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3pdu-type");
    if (0xFF != index)
    {
         CTC_CLI_GET_UINT8("l3pdu-type", l3pdu_type, argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("layer3-proto");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_L3HDR_PROTO;
    }

    index = CTC_CLI_GET_ARGC_INDEX("layer4-port");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("layer4-type");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_TYPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("layer3-ipda");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER3_IPDA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("arp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ARP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dhcp");
    if (0xFF != index)
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_DHCP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("entry index", entry_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-valid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("entry valid", entry.entry_valid, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("action-index");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("action index", entry.action_index, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_set_global_action(g_api_lchip, l3pdu_type, entry_index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_set_global_action(l3pdu_type, entry_index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_pdu_show_l3_global_action,
        ctc_cli_pdu_show_l3_global_action_cmd,
        "show pdu l3pdu global-action (ospf|pim|vrrp|rsvp|rip|bgp|msdp|ldp|\
        isis|igmp-query|igmp-report-leave|igmp-unknown|mld-query|mld-report-done|mld-unknown|arp|dhcp|\
        layer3-proto|layer4-port|layer4-type|l3pdu-type L3PDU_TYPE) (index INDEX)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Global action",
        "OSPF, not need entry index",
        "PIM,  not need entry index",
        "VRRP, not need entry index",
        "RSVP, not need entry index",
        "RIP,  not need entry index",
        "BGP,  not need entry index",
        "MSDP, not need entry index",
        "LDP,  not need entry index",
        "ISIS, not need entry index",
        "IGMP QUERY, not need entry index",
        "IGMP REPORT AND LEAVE, not need entry index",
        "IGMP UNKNOWN, not need entry index",
        "MLD QUERY, not need entry index",
        "MLD REPORT AND DONE, not need entry index",
        "MLD UNKNOWN, not need entry index",
        "ARP , not need entry index",
        "DHCP, not need entry index",
        "Base on layer3 protocol",
        "Base on layer4 port",
        "Base on layer4 type",
        "l3 pdu type",
        "l3 pdu type,refer to ctc_pdu_l3pdu_type_t",
        "Entry index",
        CTC_CLI_L3PDU_ENTRY_INDEX)
{
    ctc_pdu_global_l3pdu_action_t entry;
    ctc_pdu_l3pdu_type_t l3pdu_type = MAX_CTC_PDU_L3PDU_TYPE;

    uint8 index = 0;
    int32 ret = 0;

    sal_memset(&entry, 0, sizeof(ctc_pdu_global_l3pdu_action_t));

    if (CLI_CLI_STR_EQUAL("ospf", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_OSPF;
    }

    if (CLI_CLI_STR_EQUAL("pim", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_PIM;
    }

    if (CLI_CLI_STR_EQUAL("vrrp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_VRRP;
    }

    if (CLI_CLI_STR_EQUAL("rsvp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_RSVP;
    }

    if (CLI_CLI_STR_EQUAL("rip", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_RIP;
    }

    if (CLI_CLI_STR_EQUAL("bgp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_BGP;
    }

    if (CLI_CLI_STR_EQUAL("msdp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MSDP;
    }

    if (CLI_CLI_STR_EQUAL("ldp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LDP;
    }

    if (CLI_CLI_STR_EQUAL("isis", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ISIS;
    }

    if (CLI_CLI_STR_EQUAL("igmp-query", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_QUERY;
    }

    if (CLI_CLI_STR_EQUAL("igmp-report-leave", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE;
    }

    if (CLI_CLI_STR_EQUAL("igmp-unknown", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN;
    }

    if (CLI_CLI_STR_EQUAL("mld-query", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_QUERY;
    }

    if (CLI_CLI_STR_EQUAL("mld-report-done", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE;
    }

    if (CLI_CLI_STR_EQUAL("mld-unknown", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN;
    }

    if (CLI_CLI_STR_EQUAL("arp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_ARP;
    }

    if (CLI_CLI_STR_EQUAL("dhcp", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_DHCP;
    }

    if (CLI_CLI_STR_EQUAL("layer3", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_L3HDR_PROTO;
    }

    if (CLI_CLI_STR_EQUAL("layer4-port", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_PORT;
    }

    if (CLI_CLI_STR_EQUAL("layer4-type", 0))
    {
        l3pdu_type = CTC_PDU_L3PDU_TYPE_LAYER4_TYPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3pdu-type");
    if (0xFF != index)
    {
         CTC_CLI_GET_UINT8("l3pdu-type", l3pdu_type, argv[index+1]);
    }
	index = CTC_CLI_GET_ARGC_INDEX("index");
    if (0xFF != index)
    {
         CTC_CLI_GET_UINT8("index", index, argv[index+1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_get_global_action(g_api_lchip, l3pdu_type, index, &entry);
    }
    else
    {
        ret = ctc_l3pdu_get_global_action(l3pdu_type, index, &entry);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("action_index:0x%x\n", entry.action_index);
    ctc_cli_out("entry_valid:0x%x\n", entry.entry_valid);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_l3_set_l3if_action,
        ctc_cli_pdu_l3_set_l3if_action_cmd,
        "pdu l3pdu l3if-action l3if L3IFID action-index ACTION_INDEX action-type(forward|copy-to-cpu)",
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Per-l3if l3pdu action ",
        "Layer3 interface",
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Per-l3if l3pdu action index",
        CTC_CLI_L3PDU_L3IF_ACTION_INDEX,
        "Action type",
        "Forward",
        "Copy to cpu")
{
    ctc_pdu_l3if_l3pdu_action_t action = MAX_CTC_PDU_L3PDU_ACTION_TYPE;
    uint8 action_index = 0;
    int32 ret = 0;

    uint16 l3ifid = 0;

    CTC_CLI_GET_UINT16_RANGE("layer3 ifid", l3ifid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("action index", action_index, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if (0 == sal_memcmp("forward", argv[2], 7))
    {
        action = CTC_PDU_L3PDU_ACTION_TYPE_FWD;
    }

    if (0 == sal_memcmp("copy", argv[2], 4))
    {
        action = CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU;
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_set_l3if_action(g_api_lchip, l3ifid, action_index, action);
    }
    else
    {
        ret = ctc_l3pdu_set_l3if_action(l3ifid, action_index, action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_pdu_show_l3if_action_action,
        ctc_cli_pdu_show_l3if_action_action_cmd,
        "show pdu l3pdu l3if-action l3if L3IFID action-index ACTION_INDEX (action-type|)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PDU_STR,
        CTC_CLI_L3_PDU_STR,
        "Per-l3if l3pdu action ",
        "Layer3 interface",
        CTC_CLI_L3IF_ID_RANGE_DESC,
        "Per-l3if l3pdu action index",
        CTC_CLI_L3PDU_L3IF_ACTION_INDEX,
        "Action type")
{
    ctc_pdu_l3if_l3pdu_action_t action;
    uint8 action_index = 0;
    uint16 l3ifid = 0;
    int32 ret = 0;

    CTC_CLI_GET_UINT16_RANGE("l3ifid", l3ifid, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_UINT8_RANGE("action index", action_index, argv[1], 0, CTC_MAX_UINT8_VALUE);

    if(g_ctcs_api_en)
    {
        ret = ctcs_l3pdu_get_l3if_action(g_api_lchip, l3ifid, action_index, &action);
    }
    else
    {
        ret = ctc_l3pdu_get_l3if_action(l3ifid, action_index, &action);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    if (action == CTC_PDU_L3PDU_ACTION_TYPE_FWD)
    {
        ctc_cli_out("l3if:%d action index:%d action:%s  \n", l3ifid, action_index, "normal forwarding");
    }
    else if (action == CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU)
    {
        ctc_cli_out("l3if:%d action index:%d action:%s  \n", l3ifid, action_index, "copy to cpu");
    }

    return CLI_SUCCESS;
}

int32
ctc_pdu_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_debug_cmd);

    /*layer2 pdu cmd*/
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_classify_l2pdu_by_macda_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_classify_l2pdu_by_macda_low24_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l2_classify_l2pdu_by_macda_low24_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_classify_l2pdu_by_l2hdr_proto_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l2_classify_l2pdu_by_l2hdr_proto_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_global_action_by_macda_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_global_action_by_macda_low24_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_global_action_by_l2hdr_proto_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_global_action_by_layer3_type_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_global_action_by_bpdu_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l2_global_action_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l2_set_port_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l2_port_action_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_classify_l3pdu_by_l3proto_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_classify_l3pdu_by_l4port_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_classify_l3pdu_by_ipda_cmd);
	install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_classify_l3pdu_by_cxp_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l3_classify_l3pdu_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_set_global_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l3_global_action_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_pdu_l3_set_l3if_action_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_pdu_show_l3if_action_action_cmd);

    return CLI_SUCCESS;

}

