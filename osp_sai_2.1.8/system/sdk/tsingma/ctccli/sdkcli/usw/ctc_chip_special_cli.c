
#include "sal.h"

#include "ctc_macro.h"
#include "ctc_chip.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_scl_cli.h"
#include "ctc_usw_port_cli.h"
#include "ctc_usw_acl_cli.h"
#include "ctc_usw_nexthop_cli.h"
#include "ctc_usw_l2_cli.h"
#include "ctc_usw_vlan_cli.h"
#include "ctc_usw_scl_cli.h"
#include "ctc_usw_security_cli.h"
#include "ctc_usw_mirror_cli.h"
#include "ctc_usw_qos_cli.h"
#include "ctc_usw_pdu_cli.h"
#include "ctc_usw_interrupt_cli.h"
#include "ctc_usw_packet_cli.h"
#include "ctc_usw_linkagg_cli.h"
#include "ctc_usw_dma_cli.h"
#include "ctc_usw_aps_cli.h"
#include "ctc_usw_chip_cli.h"
#include "ctc_usw_stats_cli.h"
#include "ctc_usw_ftm_cli.h"
#include "ctc_usw_l3if_cli.h"
#include "ctc_usw_packet_test_cli.h"
#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
#include "ctc_usw_ipuc_cli.h"
#include "ctc_usw_ipmc_cli.h"
#endif
#if (FEATURE_MODE == 0)
#include "ctc_usw_mpls_cli.h"
#include "ctc_usw_oam_cli.h"
#include "ctc_usw_stacking_cli.h"
#include "ctc_usw_overlay_tunnel_cli.h"
#include "ctc_usw_bpe_cli.h"
#include "ctc_usw_ipfix_cli.h"
#include "ctc_usw_monitor_cli.h"
#include "ctc_usw_trill_cli.h"
#include "ctc_usw_fcoe_cli.h"
#include "ctc_usw_dot1ae_cli.h"
#include "ctc_usw_npm_cli.h"
#include "ctc_usw_wlan_cli.h"
#endif

extern int32
ctc_usw_temp_init(void);
extern ctc_chip_special_callback_fun_t g_chip_special_cli[MAX_CTC_CHIP_TYPE];

extern int32
ctc_usw_temp_deinit(void);

int32
ctc_usw_chip_special_cli_init(void)
{
    ctc_usw_nexthop_cli_init();
    ctc_usw_l2_cli_init();
    ctc_usw_vlan_cli_init();
    ctc_usw_scl_cli_init();
    ctc_usw_security_cli_init();
    ctc_usw_port_cli_init();
    ctc_usw_mirror_cli_init();
    ctc_usw_acl_cli_init();
    ctc_usw_pdu_cli_init();
    ctc_usw_qos_cli_init();
    ctc_usw_interrupt_cli_init();
    ctc_usw_packet_cli_init();
    ctc_usw_linkagg_cli_init();
    ctc_usw_dma_cli_init();
    ctc_usw_aps_cli_init();
    ctc_usw_chip_cli_init();
    ctc_usw_stats_cli_init();
    ctc_usw_ftm_cli_init();
    ctc_usw_l3if_cli_init();
//    ctc_usw_packet_test_cli_init();
#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
    ctc_usw_ipmc_cli_init();
    ctc_usw_ipuc_cli_init();
#endif
#if (FEATURE_MODE == 0)
    ctc_usw_mpls_cli_init();
    ctc_usw_oam_cli_init();
    ctc_usw_stacking_cli_init();
    ctc_usw_overlay_tunnel_cli_init();
    ctc_usw_ipfix_cli_init();
    ctc_usw_monitor_cli_init();
    ctc_usw_trill_cli_init();
    ctc_usw_fcoe_cli_init();
    ctc_usw_npm_cli_init();
    ctc_usw_wlan_cli_init();
    ctc_usw_dot1ae_cli_init();
    ctc_usw_bpe_cli_init();
#endif
    return 0;
}

int32
ctc_usw_chip_special_cli_deinit(void)
{
    ctc_usw_acl_cli_deinit();
    ctc_usw_mirror_cli_deinit();
    ctc_usw_port_cli_deinit();
    ctc_usw_security_cli_deinit();
    ctc_usw_l3if_cli_deinit();
    ctc_usw_ftm_cli_deinit();
    ctc_usw_stats_cli_deinit();
    ctc_usw_chip_cli_deinit();
    ctc_usw_aps_cli_deinit();
    ctc_usw_dma_cli_deinit();
    ctc_usw_linkagg_cli_deinit();
    ctc_usw_packet_cli_deinit();
    ctc_usw_interrupt_cli_deinit();
    ctc_usw_qos_cli_deinit();
    ctc_usw_pdu_cli_deinit();
    ctc_usw_scl_cli_deinit();
    ctc_usw_vlan_cli_deinit();
    ctc_usw_l2_cli_deinit();
    ctc_usw_nexthop_cli_deinit();
  //  ctc_usw_packet_test_cli_deinit();/*SYSTEM MODIFIED by yejl for delete the packet cli, 2019-03-05*/
#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
    ctc_usw_ipuc_cli_deinit();
    ctc_usw_ipmc_cli_deinit();
#endif
#if (FEATURE_MODE == 0)
    ctc_usw_mpls_cli_deinit();
    ctc_usw_bpe_cli_deinit();
    ctc_usw_monitor_cli_deinit();
    ctc_usw_ipfix_cli_deinit();
    ctc_usw_overlay_tunnel_cli_deinit();
    ctc_usw_dot1ae_cli_deinit();
    ctc_usw_wlan_cli_deinit();
    ctc_usw_npm_cli_deinit();
    ctc_usw_fcoe_cli_deinit();
    ctc_usw_trill_cli_deinit();
    ctc_usw_stacking_cli_deinit();
    ctc_usw_oam_cli_deinit();
#endif
    return 0;
}

int32
ctc_usw_chip_special_cli_callback_init(void)
{
    g_chip_special_cli[CTC_CHIP_DUET2].chip_special_cli_init = ctc_usw_chip_special_cli_init;
    g_chip_special_cli[CTC_CHIP_DUET2].chip_special_cli_deinit = ctc_usw_chip_special_cli_deinit;

    g_chip_special_cli[CTC_CHIP_TSINGMA].chip_special_cli_init = ctc_usw_chip_special_cli_init;
    g_chip_special_cli[CTC_CHIP_TSINGMA].chip_special_cli_deinit = ctc_usw_chip_special_cli_deinit;

    return 0;
}

