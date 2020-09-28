
#include "sal.h"
#include "dal.h"
#include "ctc_dkit.h"
#if defined(GOLDENGATE)
#include "goldengate/include/drv_ftm.h"
#elif defined(GREATBELT)
#include "greatbelt/include/drv_ftm.h"
#else
#include "usw/include/drv_ftm.h"
#endif
#include "ctc_dkit_cli.h"
#include "ctc_cli.h"

extern int32 dal_set_device_access_type(dal_access_type_t device_type);
extern int32 dal_op_init(dal_op_t* p_dal_op);
#if defined(GOLDENGATE)
extern int32 drv_goldengate_init(uint8 lchip, uint8 base);
#elif defined(GREATBELT)
extern int32 drv_greatbelt_init(uint8 lchip, uint8 base);
#else
extern int32 drv_init(uint8 lchip, uint8 base);
#endif

extern dal_op_t g_dal_op;

uint8 cli_end = 0;
uint8 g_ctcs_api_en = 0;
uint8 g_api_lchip = 0;
uint8 g_lchip_num = 1;
int32 g_error_on = 0;
uint8
ctc_get_chip_type()
{
    uint8 chip_type = 0;

#ifdef GREATBELT
        chip_type = CTC_DKIT_CHIP_GREATBELT;
#elif defined GOLDENGATE
        chip_type = CTC_DKIT_CHIP_GOLDENGATE;
#elif defined DUET2
        chip_type = CTC_DKIT_CHIP_DUET2;
#elif defined TSINGMA
        chip_type = CTC_DKIT_CHIP_TSINGMA;
#endif
    return chip_type;
}

uint8
ctcs_get_chip_type(uint8 lchip)
{
    uint8 chip_type = 0;
    if (lchip >= g_lchip_num)
    {
        return 0;
    }
#ifdef GREATBELT
        chip_type = CTC_DKIT_CHIP_GREATBELT;
#elif defined GOLDENGATE
        chip_type = CTC_DKIT_CHIP_GOLDENGATE;
#elif defined DUET2
        chip_type = CTC_DKIT_CHIP_DUET2;
#elif defined TSINGMA
        chip_type = CTC_DKIT_CHIP_TSINGMA;
#endif
    return chip_type;
}

void
ctc_cli_mode_exit(void)
{
    switch (g_ctc_vti->node)
    {
    case EXEC_MODE:
        g_ctc_vti->quit(g_ctc_vti);
        break;

    case CTC_DKITS_MODE:
        g_ctc_vti->node = EXEC_MODE;
        break;

    default:
        g_ctc_vti->node = EXEC_MODE;
        break;

        break;
    }
}

CTC_CLI(exit_config,
        exit_cmd,
        "exit",
        "End current mode and down to previous mode")
{
    ctc_cli_mode_exit();
    return CLI_SUCCESS;
}

CTC_CLI(quit_config,
        quit_cmd,
        "quit",
        "Exit current mode and down to previous mode")
{
    ctc_cli_mode_exit();
    return CLI_SUCCESS;
}

ctc_cmd_node_t dkit_node =
{
    CTC_DKITS_MODE,
    "\rCTC_CLI(ctc-dkits)# ",
};

ctc_cmd_node_t sdk_node =
{
    CTC_SDK_MODE,
    "\rCTC_CLI(ctc-dkits)# ",
};

ctc_cmd_node_t exec_node =
{
    EXEC_MODE,
    "\rCTC_CLI# ",
};

int ctc_master_printf(struct ctc_vti_struct_s* vti, const char *szPtr, const int szPtr_len)
{
    sal_write(0,(void*)szPtr,szPtr_len);
    return 0;
}

int ctc_master_quit(struct ctc_vti_struct_s* vti)
{
    cli_end = 1;

    return 0;
}


int
ctc_dkit_cli_master(void)
{
    uint32  nbytes = 0;
    int8*   pread_buf = NULL;

    ctc_cmd_init(0);

    ctc_install_node(&dkit_node, NULL);
    ctc_install_node(&sdk_node, NULL);
    ctc_install_node(&exec_node, NULL);
    ctc_vti_init(CTC_DKITS_MODE);

    install_element(CTC_DKITS_MODE, &exit_cmd);
    install_element(CTC_DKITS_MODE, &quit_cmd);
    install_element(EXEC_MODE, &exit_cmd);
    install_element(EXEC_MODE, &quit_cmd);

    ctc_dkit_cli_init(CTC_DKITS_MODE);
    ctc_sort_node();
    pread_buf = sal_malloc(CTC_VTI_BUFSIZ);

    if(NULL == pread_buf)
    {
        return -1;
    }
    set_terminal_raw_mode(CTC_VTI_SHELL_MODE_DEFAULT);

    g_ctc_vti->printf = ctc_master_printf;
    g_ctc_vti->quit   = ctc_master_quit;

    while (cli_end == 0)
    {
        nbytes = ctc_vti_read(pread_buf,CTC_VTI_BUFSIZ,CTC_VTI_SHELL_MODE_DEFAULT);
        ctc_vti_read_cmd(g_ctc_vti,pread_buf,nbytes);
    }

    restore_terminal_mode(CTC_VTI_SHELL_MODE_DEFAULT);

    sal_free(pread_buf);
    pread_buf = NULL;
    return 0;
}

int
main(const int argc, char* argv[])
{
    uint8 loop = 0;
    int ret = 0;
    drv_ftm_profile_info_t profile;
    dal_op_t dal_cfg;
    int32 num = 0;
    uint8 lchip = 0;

    while(++loop < argc)
    {
        CTC_CLI_GET_UINT8("lchip num", num, argv[1]);
        g_lchip_num = num;
    }

    mem_mgr_init();

    if (0 == SDK_WORK_PLATFORM)
    {
        /* dal module init */
        ret = dal_set_device_access_type(DAL_PCIE_MM);
        if (ret < 0)
        {
            ctc_cli_out("Failed to register dal callback\r\n");
            return ret;
        }

        for (lchip = 0; lchip < g_lchip_num; lchip ++ )
        {
            sal_memcpy(&dal_cfg, &g_dal_op, sizeof(dal_op_t));
            dal_cfg.lchip = lchip;
            ret = dal_op_init(&dal_cfg);
            if (ret != 0)
            {
                ctc_cli_out("Failed to register dal callback\r\n");
                return ret;
            }
        }
    }

    /* drv profile init */
    sal_memset(&profile, 0, sizeof(profile));
    for (lchip = 0; lchip < g_lchip_num; lchip ++ )
    {
        /* drv module init */
#if defined(GOLDENGATE)
        drv_goldengate_init(g_lchip_num, 0);
        drv_goldengate_ftm_mem_alloc(&profile);
#elif defined(GREATBELT)
        drv_greatbelt_init(g_lchip_num, 0);
        drv_greatbelt_ftm_mem_alloc(&profile);
#else
        drv_init(lchip, 0);
        drv_usw_ftm_mem_alloc(lchip, &profile);
#endif
    }
    /* install dbg tools cli */
    ctc_dkit_cli_master();

    return 0;
}

