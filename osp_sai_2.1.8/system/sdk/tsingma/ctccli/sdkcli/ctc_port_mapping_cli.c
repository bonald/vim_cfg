/**
 @date 2010-11-24

 @version v2.0

---file comments----
*/

/****************************************************************************
 *
 * Header files
 *
 *****************************************************************************/
#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_chip.h"
#include "ctc_port_mapping_cli.h"

/****************************************************************************
 *
 * Global and Declaration
 *
 *****************************************************************************/
#define CTC_MAX_PANNEL_PORT_NUM 96
#define PORT_MAPPING_IS_PANNEL_PORT(V) ((V) > 0 && (V) <= CTC_MAX_PANNEL_PORT_NUM)

#define CTC_PORT_MAP      "./port_mapping.cfg"
#define EMPTY_LINE(C)     ((C) == '\0' || (C) == '\r' || (C) == '\n')
#define WHITE_SPACE(C)    ((C) == '\t' || (C) == ' ')

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
int32
_ctc_cli_string_atrim(char* output, const char* input)
{
    char* p = NULL;

    if (!output)
    {
        return -1;
    }

    if (!input)
    {
        return -1;
    }

    /*trim left space*/
    while (*input != '\0')
    {
        if (WHITE_SPACE(*input))
        {
            ++input;
        }
        else
        {
            break;
        }
    }

    sal_strcpy(output, input);

    /*trim right space*/
    p = output + sal_strlen(output) - 1;

    while (p >= output)
    {
        /*skip empty line*/
        if (WHITE_SPACE(*p) || ('\r' == (*p)) || ('\n' == (*p)))
        {
            --p;
        }
        else
        {
            break;
        }
    }

    *(++p) = '\0';

    return 0;
}


int32
_ctc_cli_get_interage(const char* string, uint32* integer)
{
    char* ch = NULL;
    uint32 val = 0;

    ch = strstr((char*)string, "=");

    if (NULL == ch)
    {
        return -1;
    }
    else
    {
        ch++;
    }

    while (sal_isspace((int)*ch))
    {
        ch++;
    }
    if(ch[0] == '0' && ch[1] == 'x')
    {
        sscanf((char*)ch, "%x", &val);
    }
    else
    {
        sscanf((char*)ch, "%d", &val);
    }
    *integer = val;

    return 0;
}


int32
_ctc_cli_get_port_map(uint8* fname, uint32* api_port)
{
    char    filepath[64];
    char    string[64];
    char    line[64];
    sal_file_t fp = NULL;
    uint8 cur_entry_num = 0;
    uint8 find = 0;
    int32   ret = CTC_E_NONE;
    uint32 max_phy_port = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    CTC_PTR_VALID_CHECK(fname);

    sal_memset(filepath, 0, sizeof(filepath));
    sal_strcpy(filepath, (char*)fname);


    /*OPEN FILE*/
    fp = sal_fopen((char*)filepath, "r");

    if ((NULL == fp))
    {
        ret = -1;
        goto ERROR_RETURN;
    }

    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];

    /*parse profile*/
    while (sal_fgets((char*)line, 64, fp))
    {
        /*comment line*/
        if ('#' == line[0] && '{' != line[1] && '}' != line[1])
        {
            continue;
        }

        if (EMPTY_LINE(line[0]))
        {
            continue;
        }

        /*trim left and right space*/
        sal_memset(string, 0, sizeof(string));

        _ctc_cli_string_atrim(string, line);

        if (EMPTY_LINE(string[0]))
        {
            continue;
        }

        if(sal_strncmp(string,"[PORT_MAPPING_ITEM]",strlen("[PORT_MAPPING_ITEM]")) == 0)
        {
            find = 1;
        }

        if(sal_strncmp(line, "#}", strlen("#}")) == 0)
        {
            break;
        }

        if(sal_strncmp(string,"[API_PORT]",strlen("[API_PORT]")) == 0)
        {
            if(cur_entry_num < max_phy_port)
            {
                ret = _ctc_cli_get_interage(string, ((uint32*)api_port) + cur_entry_num);
                if (ret < 0)
                {
                    goto ERROR_RETURN;
                }
                cur_entry_num++;
            }
        }

    }



ERROR_RETURN:
    if (fp)
    {
        sal_fclose(fp);
    }

    if (find)
    {
        return ret;
    }
    else
    {
        return -1;
    }
}




uint32
_ctc_cli_get_port_mapping(uint32* port_mapping_table, uint8* port_mapping_en)
{
    int32 ret;

    /*read port mapping profile*/
    ret = _ctc_cli_get_port_map((uint8*)CTC_PORT_MAP, port_mapping_table);
    if (ret == CTC_E_NONE)
    {
        /* port_mapping.cfg exist and no error happen */
        *port_mapping_en = 1;
    }
    else
    {
        *port_mapping_en = 0;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_port_mapping_show_mapping_mode,
        ctc_cli_port_mapping_show_mapping_mode_cmd,
        "show port mapping ((pannel-lport-range START_PORT END_PORT) \
                           | (api-lport-range START_PORT END_PORT) | all)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_PORT_M_STR,
        "Port mapping",
        "Pannel lport range",
        "The minimum of the pannel port range",
        "The maximum of the pannel port range",
        "Api lport range",
        "The minimum of the api port range",
        "The maximum of the api port range",
        "All pannel and api port")
{
    uint8  idx = 0;
    uint16 i = 0;
    uint16 j = 0;
    uint32 pannel_port = CTC_MAX_UINT32_VALUE;
    uint32 api_port = CTC_MAX_UINT32_VALUE;
    uint32 end_pannel_port = CTC_MAX_UINT32_VALUE;
    uint32 end_api_port = CTC_MAX_UINT32_VALUE;
    uint8  pannel = 0, api = 0;
    uint32  tmp;
    uint32 min_port = CTC_MAX_UINT32_VALUE;
    uint32* p_tmp_table = NULL;
    uint32* p_port_mapping_table = NULL;
    uint8 port_mapping_en = 0;
    uint32 max_phy_port = 0;
    uint32 max_port_num_per_chip = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    int32 ret = CLI_SUCCESS;

    idx = CTC_CLI_GET_ARGC_INDEX("pannel-lport-range");
    if (idx != 0xFF && 1 == port_mapping_en)
    {
        CTC_CLI_GET_UINT16("val", pannel_port, argv[idx + 1]);
        CTC_CLI_GET_UINT16("val", end_pannel_port, argv[idx + 2]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("api-lport-range");
    if (idx != 0xFF && 1 == port_mapping_en)
    {
        CTC_CLI_GET_UINT16("val", api_port, argv[idx + 1]);
        CTC_CLI_GET_UINT16("val", end_api_port, argv[idx + 2]);
    }

    p_tmp_table = (uint32*)mem_malloc(MEM_CLI_MODULE, sizeof(uint32)*CTC_MAX_PHY_PORT);
    p_port_mapping_table = (uint32*)mem_malloc(MEM_CLI_MODULE, sizeof(uint32)*CTC_MAX_PHY_PORT);
    if ((NULL == p_tmp_table) || (NULL == p_port_mapping_table))
    {
        ret = CLI_ERROR;
        goto out;
    }
    sal_memset(p_tmp_table, 0, sizeof(uint32)*CTC_MAX_PHY_PORT);
    sal_memset(p_port_mapping_table, CTC_MAX_UINT8_VALUE, sizeof(uint32)*CTC_MAX_PHY_PORT);

    _ctc_cli_get_port_mapping(p_port_mapping_table, &port_mapping_en);
    sal_memcpy(p_tmp_table, p_port_mapping_table, sizeof(uint32)*CTC_MAX_PHY_PORT);

    ctc_cli_out("\n");

    if (0 == port_mapping_en)
    {
        ctc_cli_out(" port mapping NOT EXIST!\n");
        ctc_cli_out("\n");
        ret = CLI_ERROR;
        goto out;
    }

    if(g_ctcs_api_en)
    {
         ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];
    max_port_num_per_chip = capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];

    idx = CTC_CLI_GET_ARGC_INDEX("pannel-lport-range");
    if (idx != 0xFF && 1 == port_mapping_en)
    {
        if ((pannel_port > end_pannel_port) || (!PORT_MAPPING_IS_PANNEL_PORT(pannel_port)))
        {
            ctc_cli_out("Invalid pannel port range!\n\n");
            ret = CLI_ERROR;
            goto out;
        }
        ctc_cli_out(" %-14s%-14s\n", "Pannel-port", "API-port");
        ctc_cli_out(" ----------------------\n");
        for (i = pannel_port; (i <= end_pannel_port) && (i < max_phy_port); i++)
        {
            tmp = p_port_mapping_table[i-1];
            if(tmp != CTC_MAX_UINT32_VALUE)
            {
                ctc_cli_out(" %-14u0x%-14.4x\n", i, tmp);
            }
        }
        ctc_cli_out("\n");
        ret = CLI_SUCCESS;
        goto out;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("api-lport-range");
    if (idx != 0xFF && 1 == port_mapping_en)
    {
        if ((api_port > end_api_port) || (CTC_MAP_GPORT_TO_LPORT(api_port) > max_port_num_per_chip))
        {
            ctc_cli_out("Invalid api port range!\n\n");
            ret = CLI_ERROR;
            goto out;
        }
        ctc_cli_out(" %-14s%-14s\n", "API-port", "Pannel-port");
        ctc_cli_out(" ----------------------\n");
        for(i = 0; i < max_phy_port; i++)
        {
            for(j = 0; j < max_phy_port; j++)
            {
                if((min_port > p_tmp_table[j]) && (p_tmp_table[j] >= api_port))
                {
                    min_port = p_tmp_table[j];
                    pannel_port = j + 1;
                }
            }
            p_tmp_table[pannel_port - 1] = CTC_MAX_UINT32_VALUE;
            if(min_port > end_api_port)
            {
                break;
            }
            else
            {
                ctc_cli_out(" 0x%-14.4x%-14u\n", min_port, pannel_port);
                min_port = CTC_MAX_UINT32_VALUE;
            }

        }
        ctc_cli_out("\n");
        ret = CLI_SUCCESS;
        goto out;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("all");
    if (idx != 0xFF)
    {
        pannel = 1;
        api = 1;
    }

    if (1 == port_mapping_en)
    {
        if (pannel)
        {
            ctc_cli_out(" %-14s%-14s\n", "Pannel-port", "API-port");
            ctc_cli_out(" ----------------------\n");

            for (i = 0; i < max_phy_port; i++)
            {
                if (CTC_MAX_UINT32_VALUE == p_port_mapping_table[i])
                {
                    continue;
                }

                ctc_cli_out(" %-14u0x%-14.4x\n", i + 1, p_port_mapping_table[i]);
            }
            ctc_cli_out("\n");
        }

        ctc_cli_out("\n");

        if (api)
        {
            ctc_cli_out(" %-14s%-14s\n", "API-port", "Pannel-port");
            ctc_cli_out(" ----------------------\n");
            for(i = 0; i < max_phy_port; i++)
            {
                /*  aim: list the relation of api_port(order of small to large) and pannel_port
                    get the min gport from tmp_table(a copy of port_mapping_table) every time,
                    set it 0xFFFF to express that the port have been got
                */
                for(j = 0; j < max_phy_port; j++)
                {
                    if(min_port > p_tmp_table[j])
                    {
                        min_port = p_tmp_table[j];
                        pannel_port = j + 1;
                    }
                }
                p_tmp_table[pannel_port - 1] = CTC_MAX_UINT32_VALUE;
                if(min_port == CTC_MAX_UINT32_VALUE)
                {
                    break;
                }
                else
                {
                    ctc_cli_out(" 0x%-14.4x%-14u\n", min_port, pannel_port);
                    min_port = CTC_MAX_UINT32_VALUE;
                }

            }

            ctc_cli_out("\n");
        }
    }

out:
    if (p_tmp_table)
    {
        mem_free(p_tmp_table);
    }
    if (p_port_mapping_table)
    {
        mem_free(p_port_mapping_table);
    }

    return ret;
}

CTC_CLI(ctc_cli_port_mapping_show_port2mdio,
        ctc_cli_port_mapping_show_port2mdio_cmd,
        "show mdio mapping",
        CTC_CLI_SHOW_STR,
        "port to mdio",
        "mapping table")
{
    uint16 port = 0;
    int32 ret = 0;
    uint32 max_phy_port = 0;
    ctc_chip_phy_mapping_para_t* p_phy_mapping_rst = NULL;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    p_phy_mapping_rst = (ctc_chip_phy_mapping_para_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_chip_phy_mapping_para_t));
    if (NULL == p_phy_mapping_rst)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_phy_mapping_rst, 0, sizeof(ctc_chip_phy_mapping_para_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_get_phy_mapping(g_api_lchip, p_phy_mapping_rst);
    }
    else
    {
        ret = ctc_chip_get_phy_mapping(p_phy_mapping_rst);
    }
    if (CLI_SUCCESS != ret)
    {
        ctc_cli_out("Get phy mapping error\n");
        mem_free(p_phy_mapping_rst);
        return CLI_ERROR;
    }

    ctc_cli_out("device port --> mdio bus\n");

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];

    for (port = 0; port < max_phy_port; port++)
    {
        ctc_cli_out("port %02d-->%02d ", port, (int8)p_phy_mapping_rst->port_mdio_mapping_tbl[port]);
        if ((port + 1) % 4 == 0)
        {
            ctc_cli_out("\n");
        }
    }

    ctc_cli_out("\n");
    mem_free(p_phy_mapping_rst);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_mapping_show_port2phy,
        ctc_cli_port_mapping_show_port2phy_cmd,
        "show phy mapping",
        CTC_CLI_SHOW_STR,
        "port to phy",
        "mapping table")
{
    int32 ret = CLI_SUCCESS;
    uint16 port = 0;
    uint32 max_phy_port = 0;
    ctc_chip_phy_mapping_para_t* p_phy_mapping_rst = NULL;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    p_phy_mapping_rst = (ctc_chip_phy_mapping_para_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_chip_phy_mapping_para_t));
    if (NULL == p_phy_mapping_rst)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_phy_mapping_rst, 0, sizeof(ctc_chip_phy_mapping_para_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_get_phy_mapping(g_api_lchip, p_phy_mapping_rst);
    }
    else
    {
        ret = ctc_chip_get_phy_mapping(p_phy_mapping_rst);
    }
    if (CLI_SUCCESS != ret)
    {
        ctc_cli_out("Show phy mapping error\n");
        mem_free(p_phy_mapping_rst);
        return CLI_ERROR;
    }

    ctc_cli_out("device port --> phy address\n");

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];

    for (port = 0; port < max_phy_port; port++)
    {
        ctc_cli_out("port %02d-->%02d ", port, (int8)p_phy_mapping_rst->port_phy_mapping_tbl[port]);
        if ((port + 1) % 4 == 0)
        {
            ctc_cli_out("\n");
        }
    }

    ctc_cli_out("\n");
    mem_free(p_phy_mapping_rst);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_port_phy_mapping_set,
        ctc_cli_port_phy_mapping_set_cmd,
        "chip set phy mapping {(lport LPORT) (mdio-bus BUS) (phy-address ADDRESS)| end}",
        "chip module",
        "set port phy mapping",
        "phy mapping",
        "mapping table",
        "lport",
        "port value",
        "mdio bus",
        "bus value",
        "phy address",
        "address value",
        "Input over, and set software table")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    static uint16 mem_num = 0;
    static ctc_chip_phy_mapping_para_t phy_mapping_rst;
    uint32 max_phy_port = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};
    uint8 is_end = 0;
    int32 lport = 0;

    if (!mem_num)
    {
        sal_memset(&phy_mapping_rst, 0, sizeof(ctc_chip_phy_mapping_para_t));
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];

    index = CTC_CLI_GET_ARGC_INDEX("lport");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("lport", lport, argv[index+1]);
    }
    if (lport >= max_phy_port)
    {
        ctc_cli_out("invalid parameter\n");
        return CLI_ERROR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mdio-bus");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("bus", phy_mapping_rst.port_mdio_mapping_tbl[lport], argv[index+1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("phy-address");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("address", phy_mapping_rst.port_phy_mapping_tbl[lport], argv[index+1]);
        mem_num++;
    }

    index = CTC_CLI_GET_ARGC_INDEX("end");
    if (0xFF != index)
    {
        is_end = 1;
    }

    if (is_end)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_chip_set_phy_mapping(g_api_lchip, &phy_mapping_rst);
        }
        else
        {
            ret = ctc_chip_set_phy_mapping(&phy_mapping_rst);
        }

        sal_memset(&phy_mapping_rst, 0, sizeof(ctc_chip_phy_mapping_para_t));
        mem_num = 0;
        if (ret < 0)
        {
            ctc_cli_out("%% ret = %d, %s\n", ret, ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_mac_led_mapping_set,
        ctc_cli_mac_led_mapping_set_cmd,
        "chip set mac led mapping",
        "chip module",
        "set paramter",
        "mac led mapping",
        "mac led mapping",
        "mapping table")
{
    int32 ret = CLI_SUCCESS;
    int32 i = 0;
    uint16 port = 0;
    uint32 max_phy_port = 0;
    ctc_chip_mac_led_mapping_t led_mapping_rst[2];
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = {0};

    sal_memset(led_mapping_rst, 0, 2 * sizeof(ctc_chip_mac_led_mapping_t));

    if(g_ctcs_api_en)
    {
        ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);
    }
    max_phy_port = capability[CTC_GLOBAL_CAPABILITY_MAX_PHY_PORT_NUM];
    i = max_phy_port / 2;

    led_mapping_rst[0].p_mac_id = mem_malloc(MEM_CLI_MODULE, i * sizeof(uint8));
    sal_memset(led_mapping_rst[0].p_mac_id, 0, i * sizeof(uint8));

    led_mapping_rst[1].p_mac_id = mem_malloc(MEM_CLI_MODULE, (max_phy_port - i) * sizeof(uint8));
    sal_memset(led_mapping_rst[1].p_mac_id, 0, (max_phy_port - i) * sizeof(uint8));

    led_mapping_rst[0].lchip = 0;
    led_mapping_rst[0].ctl_id = 0;
    led_mapping_rst[0].mac_led_num = i;

    led_mapping_rst[1].lchip = 0;
    led_mapping_rst[1].ctl_id = 1;
    #ifdef TSINGMA
    led_mapping_rst[1].ctl_id = 0;
    #endif
    led_mapping_rst[1].mac_led_num = max_phy_port - i;

    for (port = 0; port < max_phy_port; port++)
    {
        if (port < i)
        {
            led_mapping_rst[0].p_mac_id[port] = port;
        }
        else
        {
            led_mapping_rst[1].p_mac_id[port - i] = port;
        }
    }

    if(g_ctcs_api_en)
    {
        ret = ctcs_chip_set_mac_led_mapping(g_api_lchip, &led_mapping_rst[0]);
        ret = ctcs_chip_set_mac_led_mapping(g_api_lchip, &led_mapping_rst[1]);
    }
    else
    {
        ret = ctc_chip_set_mac_led_mapping(&led_mapping_rst[0]);
        ret = ctc_chip_set_mac_led_mapping(&led_mapping_rst[1]);
    }
    if (CLI_SUCCESS != ret)
    {
        ctc_cli_out("set phy mapping error\n");
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_port_mapping_cli_init()
{

    install_element(CTC_SDK_MODE, &ctc_cli_port_mapping_show_mapping_mode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_mapping_show_port2mdio_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_mapping_show_port2phy_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_port_phy_mapping_set_cmd);
    install_element(CTC_INTERNAL_MODE, &ctc_cli_mac_led_mapping_set_cmd);

    return CLI_SUCCESS;
}

