#include "proto.h"
#include "version.h"
#include "cdbclient.h"
#include "gen/tbl_iptables_prevent_define.h"


static int32
_cdbctl_show_iptables_statistics(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;

    sal_printf("Current DDoS Prevent statistics: \n");
    sal_printf("--------------------------------------------------+---------------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {        
        rc = cfg_get_parse(line, &info, 0);

        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE]))
        {
            sal_printf("%-50s:  %s\n", "Resist Fraggle Attack packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_FRAGGLE_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP]))
        {
            sal_printf("%-50s:  %s\n", "Resist ICMP Flood packets number", 
                info.value[TBL_IPTABLES_PREVENT_FLD_ICMP_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT]))
        {
            sal_printf("%-50s:  %s\n", "Resist Small-packet Attack packets number", 
                info.value[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF]))
        {
            sal_printf("%-50s:  %s\n", "Resist Smurf Attack packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_SMURF_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP]))
        {
            sal_printf("%-50s:  %s\n", "Resist SYN Flood packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_TCP_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP]))
        {
            sal_printf("%-50s:  %s\n", "Resist UDP Flood packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_UDP_COUNT]);
        }

        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist Fraggle Attack packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_FRAGGLE_MGMT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist ICMP Flood packets number", 
                info.value[TBL_IPTABLES_PREVENT_FLD_ICMP_MGMT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist Small-packet Attack packets number", 
                info.value[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_MGMT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist Smurf Attack packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_SMURF_MGMT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist SYN Flood packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_TCP_MGMT_COUNT]);
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP]))
        {
            sal_printf("%-50s:  %s\n", "mgmt-if Resist UDP Flood packets number",
                info.value[TBL_IPTABLES_PREVENT_FLD_UDP_MGMT_COUNT]);
        }
    }
    return rc;
}


static int32
_cdbctl_show_iptables_config(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;

    sal_printf("Current DDoS Prevent configuration: \n");
    sal_printf("--------------------------------+--------+--------+---------\n");

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {        
        rc = cfg_get_parse(line, &info, 0);

        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE]))
        {
            sal_printf("%-32s :Enable\n", "Fraggle Attack Intercept");
        }
        else
        {
            sal_printf("%-32s :Disable\n", "Fraggle Attack Intercept");
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP]))
        {
            sal_printf("%-32s :Enable  Maxcount:%s\n", "ICMP Flood Intercept", 
                info.value[TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT]);
        }
        else
        {
            sal_printf("%-32s :Disable\n", "ICMP Flood Intercept");
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ]))
        {
            sal_printf("%-32s :Enable\n", "IP Equal Intercept");
        }
        else
        {
            sal_printf("%-32s :Disable\n", "IP Equal Intercept");
        }

        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ]))
        {
            sal_printf("%-32s :Enable\n", "MAC Equal Intercept");
        }
        else
        {
            sal_printf("%-32s :Disable\n", "MAC Equal Intercept");
        }  
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT]))
        {
            sal_printf("%-32s :Enable  Packet Length:%s\n", "Small-packet Attack Intercept", 
                info.value[TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH]);
        }
        else
        {
            sal_printf("%-32s :Disable\n", "Small-packet Attack Intercept");
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF]))
        {
            sal_printf("%-32s :Enable\n", "Smurf Attack Intercept");
        }
        else
        {
            sal_printf("%-32s :Disable\n", "Smurf Attack Intercept");
        }
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP]))
        {
            sal_printf("%-32s :Enable  Maxcount:%s\n", "SYN Flood Intercept",
                info.value[TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT]);
        }
        else
        {
            sal_printf("%-32s :Disable\n", "SYN Flood Intercept");
        } 
        
        if (0 != sal_strcmp("0", info.value[TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP]))
        {
            sal_printf("%-32s :Enable  Maxcount:%s\n", "UDP Flood Intercept",
                info.value[TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT]);
        }
        else
        {
            sal_printf("%-32s :Disable\n", "UDP Flood Intercept");
        }

    }

    return rc;
}


int32
cdbctl_show_iptables_prevent(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info) 
{
    char  *para = p_data;
    
    if (data_len)
    {
        if (!sal_strcmp(para, "statistics"))
        {
            _cdbctl_show_iptables_statistics(fp, p_info);
        }
        if (!sal_strcmp(para, "config"))
        {
            _cdbctl_show_iptables_config(fp, p_info);
        }        

    }

    return PM_E_NONE;
}

