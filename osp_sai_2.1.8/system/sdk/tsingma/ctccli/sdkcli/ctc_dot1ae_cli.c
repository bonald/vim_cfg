#if (FEATURE_MODE == 0)
/**
 @file ctc_dot1ae_cli.c

 @author  Copyright (C) 2017 Centec Networks Inc.  All rights reserved.

 @date 2017-08-22

 @version v2.0

 The file apply clis of bpe module
*/

#include "ctc_dot1ae_cli.h"
#include "ctc_dot1ae.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_debug.h"


CTC_CLI(ctc_cli_dot1ae_update_sec_chan,
        ctc_cli_dot1ae_update_sec_chan_cmd,
        "dot1ae update (sec-chan CHAN_ID) (sci SCI |an-bitmap AN_BITMAP |(next-an AN)  \
       |(replay-protect (enable|disable) | window-size SIZE) | (include-sci|without-sci)) (lchip LCHIP|)",
        "Dot1AE",
        "Update",
        "Dot1AE Security Channel",
        "CHANNEL ID",
        "Secure Channel Identifier",
        "Secure Channel Identifier,64bit",
        "active AN to SC",
        "AN Bitmap",
        "Next-AN to SC",
        "Association Number",
        "Replay-protect",
        "Enable protect",
        "Disable protect",
        "Window-size",
        "Window-size",
        "TX SecTAG with sci",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  index = 0;
    uint8  sci[8] = {0};
    uint8  sci_tmp[8] = {0};
    uint32 value = 0;
    ctc_dot1ae_sc_t sec_chan;
    uint8  i = 0;
    uint8  lchip = 0;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(&sec_chan, 0, sizeof(ctc_dot1ae_sc_t));

    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan", sec_chan.chan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("sci");
    if (0xFF != index)
    {
        sal_sscanf(argv[index+1], "%16"PRIx64,(uint64*)sci);
        for (i=0;i<sizeof(sci);i++)
        {
            sci_tmp[sizeof(sci)-1-i] = sci[i];
        }
        sec_chan.ext_data= &sci_tmp[0];
        sec_chan.property = CTC_DOT1AE_SC_PROP_SCI;
    }
    index = CTC_CLI_GET_ARGC_INDEX("next-an");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("AN", value, argv[index + 1]);
        sec_chan.data = value;
        sec_chan.property = CTC_DOT1AE_SC_PROP_NEXT_AN;

    }
    index = CTC_CLI_GET_ARGC_INDEX("an-bitmap");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("an-bitmap", value, argv[index + 1]);
        sec_chan.data = value ;
        sec_chan.property = CTC_DOT1AE_SC_PROP_AN_EN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("window-size");
    if (0xFF != index)
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_REPLAY_WINDOW_SIZE;
        CTC_CLI_GET_UINT64("window-size",sec_chan.data, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("replay-protect");
    if (0xFF != index)
    {
        uint8 enable = 0;
        sec_chan.property = CTC_DOT1AE_SC_PROP_REPLAY_PROTECT_EN;
        if (CLI_CLI_STR_EQUAL("enable", index+1))
        {
            enable = TRUE;
        }
        else
        {
            enable = FALSE;
        }
        sec_chan.data = enable;
    }
    index = CTC_CLI_GET_ARGC_INDEX("include-sci");
    if (0xFF != index)
    {
        sec_chan.data = 1;
        sec_chan.property = CTC_DOT1AE_SC_PROP_INCLUDE_SCI;
    }
    index = CTC_CLI_GET_ARGC_INDEX("without-sci");
    if (0xFF != index)
    {
        sec_chan.data = 0;
        sec_chan.property = CTC_DOT1AE_SC_PROP_INCLUDE_SCI;
    }

    if(g_ctcs_api_en)
    {
        ret =  ctcs_dot1ae_update_sec_chan(g_api_lchip, &sec_chan);
    }
    else
    {
        ret =  ctc_dot1ae_update_sec_chan(lchip, &sec_chan);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_update_sa_cfg,
        ctc_cli_dot1ae_update_sa_cfg_cmd,
        "dot1ae update (sec-chan CHAN_ID) sa-cfg (an AN) {next-pn NEXT_PN| (cipher-suite SUITE|) aes-key AES_KEY (salt SALT ssci SSCI|) \
        |(encryption (confid-offset OFFSET)|no-encryption)| (validateframes_mode(disable|check|strict)|icv-error-action (forward |copy-to-cpu|redirect-to-cpu|discard))} (lchip LCHIP|)",
        "Dot1AE",
        "Update",
        "Secure channel",
        "CHANNEL ID",
        "SA Config",
        "Association Number",
        "AN",
        "Pn of next encrypt pkt",
        "Packet Numberr",
        "Cipher Suite",
        "GCM_AES_128: 0 , GCM_AES_256: 1, GCM_AES_XPN_128: 2, GCM_AES_XPN_256: 3",
        "Aes key",
        "KEY VALUE",
        "Salt for GCM_AES_XPN",
        "SALT VALUE",
        "Short SCI for GCM_AES_XPN",
        "SSCI VALUE",
        "Packet will be encrypted",
        "Confidentiality offset",
        "(0-offset 0; 1-offset 30; 2-offset 50)",
        "Packet won't be encrypted",
        "Validaterames mode",
        "Disable",
        "Check",
        "Strict",
        "action when icv check failed",
        "normal forwarding",
        "forwarding and copy to cpu",
        "discard and send to cpu",
        "discard",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = 0;
    uint8 index = 0;
    uint8  lchip = 0;
    ctc_dot1ae_sc_t sc;
    ctc_dot1ae_sa_t sa;
    sal_memset(&sc, 0, sizeof(ctc_dot1ae_sc_t));
    sal_memset(&sa, 0, sizeof(ctc_dot1ae_sa_t));
    sc.ext_data = &sa;
    sc.property = CTC_DOT1AE_SC_PROP_SA_CFG;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan", sc.chan_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("an");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("an", sa.an, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("cipher-suite");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("cipher-suite", sa.cipher_suite, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("aes-key");
    if (0xFF != index)
    {
        CTC_SET_FLAG(sa.upd_flag, CTC_DOT1AE_SA_FLAG_KEY);
        if (sal_strlen(argv[index + 1]) <= CTC_DOT1AE_KEY_LEN)
        {
            sal_memcpy(sa.key, argv[index + 1],  sal_strlen(argv[index + 1]));
        }
        else
        {
            ctc_cli_out("%%aes-key %s \n\r", ctc_get_error_desc(CTC_E_INVALID_PARAM));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("salt");
    if (0xFF != index)
    {
        if (sal_strlen(argv[index + 1]) <= sizeof(sa.salt))
        {
            sal_memcpy(sa.salt, argv[index + 1],  sal_strlen(argv[index + 1]));
        }
        else
        {
            ctc_cli_out("salt %s \n\r", ctc_get_error_desc(CTC_E_INVALID_PARAM));
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("ssci");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("ssci", sa.ssci, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("next-pn");
    if (0xFF != index)
    {
        CTC_SET_FLAG(sa.upd_flag,CTC_DOT1AE_SA_FLAG_NEXT_PN);
        CTC_CLI_GET_UINT64("next-pn", sa.next_pn, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("encryption");
    if (0xFF != index)
    {
        CTC_SET_FLAG(sa.upd_flag,CTC_DOT1AE_SA_FLAG_ENCRYPTION);
        index = CTC_CLI_GET_ARGC_INDEX("confid-offset");
        if (0xFF != index)
        {
        CTC_CLI_GET_UINT8("confid-offset", sa.confid_offset, argv[index + 1]);
       }

    }
    index = CTC_CLI_GET_ARGC_INDEX("no-encryption");
    if (0xFF != index)
    {
        CTC_SET_FLAG(sa.upd_flag,CTC_DOT1AE_SA_FLAG_ENCRYPTION);
        sa.no_encryption = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("validateframes_mode");
    if (0xFF != index)
    {

        CTC_SET_FLAG(sa.upd_flag, CTC_DOT1AE_SA_FLAG_VALIDATEFRAMES);
        index = CTC_CLI_GET_ARGC_INDEX("disable");
        if (0xFF != index)
        {
            sa.validateframes = CTC_DOT1AE_VALIDFRAMES_DISABLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("check");
        if (0xFF != index)
        {
            sa.validateframes = CTC_DOT1AE_VALIDFRAMES_CHECK;
        }
        index = CTC_CLI_GET_ARGC_INDEX("strict");
        if (0xFF != index)
        {
            sa.validateframes = CTC_DOT1AE_VALIDFRAMES_STRICT;
        }

    }
    index = CTC_CLI_GET_ARGC_INDEX("icv-error-action");
    if (0xFF != index)
    {
        CTC_SET_FLAG(sa.upd_flag,CTC_DOT1AE_SA_FLAG_ICV_ERROR_ACTION);
        index = CTC_CLI_GET_ARGC_INDEX("forward");
        if (0xFF != index)
        {
            sa.icv_error_action = CTC_EXCP_NORMAL_FWD;
        }
        index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
        if (0xFF != index)
        {
            sa.icv_error_action = CTC_EXCP_FWD_AND_TO_CPU;
        }
        index = CTC_CLI_GET_ARGC_INDEX("redirect-to-cpu");
        if (0xFF != index)
        {
            sa.icv_error_action = CTC_EXCP_DISCARD_AND_TO_CPU;
        }
        index = CTC_CLI_GET_ARGC_INDEX("discard");
        if (0xFF != index)
        {
            sa.icv_error_action = CTC_EXCP_DISCARD;
        }

    }
    if(g_ctcs_api_en)
    {
        ret =  ctcs_dot1ae_update_sec_chan(g_api_lchip,  &sc);
    }
    else
    {
        ret =  ctc_dot1ae_update_sec_chan(lchip, &sc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}
CTC_CLI(ctc_cli_dot1ae_add_remove_sec_chan,
        ctc_cli_dot1ae_add_remove_sec_chan_cmd,
        "dot1ae (add|remove) (sec-chan CHAN_ID) (rx|tx|) (lchip LCHIP|)",
        "Dot1AE",
        "Add",
        "Remove",
        "Dot1AE Security Channel",
        "CHANNEL ID",
        "Rx",
        "Tx",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  index = 0;
    uint8  is_add =0;
    ctc_dot1ae_sc_t  sc;
    uint8  lchip = 0;

    sc.ext_data= NULL;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    sal_memset(&sc,0,sizeof(ctc_dot1ae_sc_t));
    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        is_add = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
       sc.dir =  CTC_DOT1AE_SC_DIR_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan",sc.chan_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = is_add?ctcs_dot1ae_add_sec_chan(g_api_lchip,&sc):ctcs_dot1ae_remove_sec_chan(g_api_lchip,&sc);
    }
    else
    {
        ret = is_add?ctc_dot1ae_add_sec_chan(lchip, &sc):ctc_dot1ae_remove_sec_chan(lchip, &sc);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_set_glb_cfg,
        ctc_cli_dot1ae_set_glb_cfg_cmd,
        "dot1ae set glb-cfg {encrypt-pn-thrd EN_THRD | decrypt-pn-thrd DE_THRD  | tx-pn-overflow (forward |redirect-to-cpu|discard) |port-mirror-mode MODE}",
        "Dot1AE",
        "Set",
        "Global cfg",
        "Encrypt pn threhold",
        "EN_THRD",
        "Decrypt pn threhold",
        "DE_THRD",
        "Tx pn overflow action",
        "normal forwarding",
        "discard and send to cpu",
        "discard",
        "Port mirror mode",
        "1-encrypted packets,0-plain packets")
{
    int32  ret = 0;
    uint8  index = 0;
    ctc_dot1ae_glb_cfg_t glb_cfg;

    sal_memset(&glb_cfg, 0, sizeof(ctc_dot1ae_glb_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_dot1ae_get_global_cfg(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_dot1ae_get_global_cfg(&glb_cfg);
    }

    index = CTC_CLI_GET_ARGC_INDEX("encrypt-pn-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT64("encrypt-pn-thrd", glb_cfg.tx_pn_thrd, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("decrypt-pn-thrd");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT64("decrypt-pn-thrd", glb_cfg.rx_pn_thrd, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("tx-pn-overflow");
    if (0xFF != index)
    {
        index = CTC_CLI_GET_ARGC_INDEX("forward");
        if (0xFF != index)
        {
            glb_cfg.tx_pn_overflow_action = CTC_EXCP_NORMAL_FWD;
        }
        index = CTC_CLI_GET_ARGC_INDEX("redirect-to-cpu");
        if (0xFF != index)
        {
            glb_cfg.tx_pn_overflow_action = CTC_EXCP_DISCARD_AND_TO_CPU;
        }
        index = CTC_CLI_GET_ARGC_INDEX("discard");
        if (0xFF != index)
        {
            glb_cfg.tx_pn_overflow_action = CTC_EXCP_DISCARD;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("port-mirror-mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("value", glb_cfg.dot1ae_port_mirror_mode, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_dot1ae_set_global_cfg(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_dot1ae_set_global_cfg(&glb_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_show_glb_cfg,
        ctc_cli_dot1ae_show_glb_cfg_cmd,
        "show dot1ae glb-cfg",
        CTC_CLI_SHOW_STR,
        "Dot1AE",
        "global cfg")
{
    int32  ret = 0;
    ctc_dot1ae_glb_cfg_t glb_cfg;
    char  overflow_action[20] = "\0";

    sal_memset(&glb_cfg, 0, sizeof(ctc_dot1ae_glb_cfg_t));
    if(g_ctcs_api_en)
    {
        ret = ctcs_dot1ae_get_global_cfg(g_api_lchip, &glb_cfg);
    }
    else
    {
        ret = ctc_dot1ae_get_global_cfg(&glb_cfg);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("Global Dot1AE:\n");
    ctc_cli_out("\tencrypt pn threhold: %llu\n", glb_cfg.tx_pn_thrd);
    ctc_cli_out("\tdecrypt pn threhold: %llu\n", glb_cfg.rx_pn_thrd);
    ctc_cli_out("\tdecrypt pn threhold: %llu\n", glb_cfg.tx_pn_overflow_action);
    switch(glb_cfg.tx_pn_overflow_action)
    {
        case CTC_EXCP_NORMAL_FWD :
            sal_strcpy(overflow_action, "forward");
            break;
        case CTC_EXCP_DISCARD_AND_TO_CPU :
            sal_strcpy(overflow_action, "redirect-to-cpu");
            break;
        case CTC_EXCP_DISCARD :
            sal_strcpy(overflow_action, "discard");
            break;
    }
    ctc_cli_out("\tx pn overflow action: %s\n", overflow_action);
    ctc_cli_out("\tmirror mode: %d\n", glb_cfg.dot1ae_port_mirror_mode);

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_show_stats,
        ctc_cli_dot1ae_show_stats_cmd,
        "show dot1ae stats (sec-chan CHAN_ID) (lchip LCHIP|)",
        CTC_CLI_SHOW_STR,
        "Dot1AE",
        "stats",
        "Secure channel",
        "CHANNEL ID",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  index = 0;
    uint8  lchip = 0;
    uint32 chan_id = 0xffff;
    uint8  i = 0;
    ctc_dot1ae_stats_t stats;

    sal_memset(&stats, 0, sizeof(stats));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan", chan_id, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_dot1ae_get_stats(g_api_lchip, chan_id, &stats);
    }
    else
    {
        ret = ctc_dot1ae_get_stats(lchip, chan_id, &stats);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("global stats:\n");
    ctc_cli_out("%-20s%u\n","inPktsNoSci:", stats.in_pkts_no_sci);
    ctc_cli_out("%-20s%u\n","inPktsUnknownSci", stats.in_pkts_unknown_sci);
    ctc_cli_out("channel %u stats:\n", chan_id);
    for(i = 0; i < 4; i++)
    {
        ctc_cli_out("an[%u]:\n", i);
        ctc_cli_out("%-20s%u\n","outPktsProtected:", stats.an_stats[i].out_pkts_protected);
        ctc_cli_out("%-20s%u\n","outPktsEncrypted:", stats.an_stats[i].out_pkts_encrypted);
        ctc_cli_out("%-20s%u\n","inPktsOk:", stats.an_stats[i].in_pkts_ok);
        ctc_cli_out("%-20s%u\n","inPktsUnchecked:", stats.an_stats[i].in_pkts_unchecked);
        ctc_cli_out("%-20s%u\n","inPktsDelayed:", stats.an_stats[i].in_pkts_delayed);
        ctc_cli_out("%-20s%u\n","inPktsInvalid:", stats.an_stats[i].in_pkts_invalid);
        ctc_cli_out("%-20s%u\n","inPktsNotValid:", stats.an_stats[i].in_pkts_not_valid);
        ctc_cli_out("%-20s%u\n","inPktsLate:", stats.an_stats[i].in_pkts_late);
        ctc_cli_out("%-20s%u\n","inPktsNotUsingSa:", stats.an_stats[i].in_pkts_not_using_sa);
        ctc_cli_out("%-20s%u\n","inPktsUnusedSa:", stats.an_stats[i].in_pkts_unused_sa);
    }

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_clear_stats,
        ctc_cli_dot1ae_clear_stats_cmd,
        "dot1ae stats clear (sec-chan CHAN_ID) (an AN|) (lchip LCHIP|)",
        "Dot1AE",
        "Stats",
        "Clear",
        "Secure channel",
        "CHANNEL ID",
        "Association number",
        "Association number Value",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  index = 0;
    uint8  lchip = 0;
    uint32 chan_id = 0xffff;
    uint8  an = 0xFF;

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan", chan_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("an");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("an", an, argv[index + 1]);
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_dot1ae_clear_stats(g_api_lchip, chan_id, an);
    }
    else
    {
        ret = ctc_dot1ae_clear_stats(lchip, chan_id, an);
    }
    if (ret < 0)
    {
        ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_dot1ae_get_sec_chan,
        ctc_cli_dot1ae_get_sec_chan_cmd,
        "show dot1ae (sec-chan CHAN_ID) {an-en|include-sci|sci|replay-protect|reply-window-size|sa-cfg an AN} (lchip LCHIP|)",
        "Show",
        "Dot1ae",
        "Secure channel",
        "CHANNEL ID",
        "Association number enabled",
        "Include SCI",
        "Secure Channel Identifier",
        "Replay protect",
        "Replay window size",
        "Secure association configuration",
        "Association number",
        "Association number ID",
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32  ret = 0;
    uint8  i = 0;
    uint8  index = 0;
    uint8  lchip = 0;
    uint8  an_en[4] = {0};
    uint8  an_en_tmp = 0;
    uint8  include_sci = 0;
    char  icv_action[20] = "\0";
    uint64 window_size = 0;
    uint8  sci_tmp[8] = {0};
    uint32 offset = 0;
    uint8 replay_protect = 0;

    ctc_dot1ae_sc_t sec_chan;
    ctc_dot1ae_sa_t sa;
    sal_memset(&sec_chan, 0, sizeof(ctc_dot1ae_sc_t));
    sal_memset(&sa, 0, sizeof(ctc_dot1ae_sa_t));

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en?g_api_lchip:lchip;
    index = CTC_CLI_GET_ARGC_INDEX("sec-chan");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("sec-chan", sec_chan.chan_id, argv[index + 1]);
    }
    ctc_cli_out("%-20s:%-3u","chan_id",sec_chan.chan_id);
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("an-en"))/**< [tx/rx] Enable AN to channel  (0~3) */
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_AN_EN;
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip, &sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        i = 0;
        while(sec_chan.data){
            an_en_tmp = sec_chan.data % 2;
            sec_chan.data = sec_chan.data / 2;
            an_en[i++] = an_en_tmp;
        }
        ctc_cli_out("\n%-20s:","an-en[TX/RX]");
        for (i=3;i>0;i--)
        {
            ctc_cli_out("%d",an_en[i]);
        }
        ctc_cli_out("%d",an_en[0]);
    }
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("include-sci"))/**< [tx] Transmit SCI in MacSec Tag*/
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_INCLUDE_SCI;
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip,&sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        include_sci = sec_chan.data;
        sec_chan.data = 0;
        ctc_cli_out("\n%-20s:%-3d", "include-sci[TX]",include_sci);
    }
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("sci"))/**< [tx/rx] Secure Channel Identifier,64 bit. uint32 value[2]*/
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_SCI;
        sec_chan.ext_data = sci_tmp;
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip,&sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        ctc_cli_out("\n%-20s:0x","SCI[TX/RX]");
        for (i=0;i<sizeof(sci_tmp);i++)
        {
            ctc_cli_out("%02x",sci_tmp[i]);
        }
    }
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("reply-protect"))/**< [rx] Replay protect windows size*/
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_REPLAY_PROTECT_EN;
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip,&sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        window_size = sec_chan.data;
        ctc_cli_out("\n%-20s:%s","replay protect [RX]",replay_protect?"enable":"disable");
        sec_chan.data = 0;
    }
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("reply-window-size"))/**< [rx] Replay protect windows size*/
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_REPLAY_WINDOW_SIZE;
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip,&sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        window_size = sec_chan.data;
        ctc_cli_out("\n%-20s:%-3llu","replay windiw size[RX]",window_size);
        sec_chan.data = 0;
    }
    if (0xFF != CTC_CLI_GET_ARGC_INDEX("sa-cfg"))/**< [tx/rx] configure SA Property, ctc_dot1ae_sa_t*/
    {
        sec_chan.property = CTC_DOT1AE_SC_PROP_SA_CFG;
        sec_chan.ext_data = &sa;
        index = CTC_CLI_GET_ARGC_INDEX("an");
        CTC_CLI_GET_UINT8("an",sa.an, argv[index + 1]);
        ret = g_ctcs_api_en? ctcs_dot1ae_get_sec_chan(lchip,&sec_chan):ctc_dot1ae_get_sec_chan(lchip,&sec_chan);
        if (ret < 0)
        {
            ctc_cli_out("%% ret =%d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        offset = sa.confid_offset==0?0:(sa.confid_offset==1?30:50);
        ctc_cli_out("\n%-20s:%-3u\n%-20s:%-3llu\n%-20s:%-3llu\n%-20s:%-3u\n%-20s:%-3u\n%-20s:%-3u\n%-20s:%-s\n", \
                            "an",sa.an,"next-PN[TX/RX]",sa.next_pn,"lowest-PN[RX]",sa.lowest_pn,\
                            "no-encryption[TX]",sa.no_encryption,"confid-offset[TX]",offset,\
                            "in-use[TX/RX]",sa.in_use,"key[TX/RX]",sa.key);
        ctc_cli_out("%-20s [HEX]:"," ");
        for (i=0;i<sizeof(sa.key);i++)
        {
            if(i && !(i%2))
            {
                ctc_cli_out(" ");
            }
            ctc_cli_out("%x",sa.key[i]);
        }
        ctc_cli_out("\n");
        ctc_cli_out("%-20s:","salt:[TX/RX]");
        for (i=0;i<sizeof(sa.salt);i++)
        {
            ctc_cli_out("%c",sa.salt[i]);
        }
        ctc_cli_out("\n");
        ctc_cli_out("%-20s:0x%x\n","ssci:[TX/RX]",sa.ssci);
        ctc_cli_out("%-20s:%u\n","cipher_suite:[TX/RX]",sa.cipher_suite);
        ctc_cli_out("%-20s:%u\n","icv validate_frames mode[RX]",sa.validateframes);

            switch(sa.icv_error_action)
            {
            case CTC_EXCP_NORMAL_FWD :
                sal_strcpy(icv_action, "forward");
                break;
            case CTC_EXCP_FWD_AND_TO_CPU :
                sal_strcpy(icv_action, "copy-to-cpu");
                break;
            case CTC_EXCP_DISCARD_AND_TO_CPU :
                sal_strcpy(icv_action, "redirect-to-cpu");
                break;
            case CTC_EXCP_DISCARD :
                sal_strcpy(icv_action, "discard");
                break;
            }
            ctc_cli_out("%-20s:%-3s","icv_error_action[RX]",icv_action);

    }
    ctc_cli_out("\n%-20s:%-3s\n", "dir",sec_chan.dir?"rx":"tx");
    return ret;
}

CTC_CLI(ctc_cli_dot1ae_debug_on,
        ctc_cli_dot1ae_debug_on_cmd,
        "debug dot1ae (ctc | sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_DOT1AE_STR,
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
        typeenum = DOT1AE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DOT1AE_SYS;
    }

    ctc_debug_set_flag("dot1ae", "dot1ae", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_dot1ae_debug_off,
        ctc_cli_dot1ae_debug_off_cmd,
        "no debug dot1ae (ctc | sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_DOT1AE_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DOT1AE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DOT1AE_SYS;
    }

    ctc_debug_set_flag("dot1ae", "dot1ae", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_dot1ae_show_debug,
        ctc_cli_dot1ae_show_debug_cmd,
        "show debug dot1ae (ctc | sys)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_DOT1AE_STR,
        "Ctc layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = 0;
    uint8 en = 0;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = DOT1AE_CTC;
    }
    else if (0 == sal_memcmp(argv[0], "sys", 3))
    {
        typeenum = DOT1AE_SYS;
    }

    en = ctc_debug_get_flag("dot1ae", "dot1ae", typeenum, &level);
    ctc_cli_out("dot1ae:%s debug %s level:%s\n", argv[0],
                en ? "on" : "off", ctc_cli_get_debug_desc(level));

    return CLI_SUCCESS;
}

int32
ctc_dot1ae_cli_init(void)
{

    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_update_sa_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_update_sec_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_get_sec_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_add_remove_sec_chan_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_set_glb_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_show_glb_cfg_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_show_stats_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_clear_stats_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_dot1ae_show_debug_cmd);

    return CLI_SUCCESS;
}

#endif

