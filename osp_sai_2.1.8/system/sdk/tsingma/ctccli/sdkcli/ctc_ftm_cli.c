/**
 @file ctc_alloc_cli.c

 @date 2009-11-23

 @version v2.0

---file comments----
*/

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_ftm_cli.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_alloc.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_cli_common.h"

CTC_CLI(ctc_cli_ftm_debug_on,
        ctc_cli_ftm_debug_on_cmd,
        "debug ftm (ctc|sys) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "CTC layer",
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
        typeenum = FTM_CTC;
    }
    else
    {
        typeenum = FTM_SYS;
    }

    ctc_debug_set_flag("ftm", "ftm", typeenum, level, TRUE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_debug_off,
        ctc_cli_ftm_debug_off_cmd,
        "no debug ftm (ctc|sys)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "CTC layer",
        "Sys layer")
{
    uint32 typeenum = 0;
    uint8 level = CTC_DEBUG_LEVEL_INFO;

    if (0 == sal_memcmp(argv[0], "ctc", 3))
    {
        typeenum = FTM_CTC;
    }
    else
    {
        typeenum = FTM_SYS;
    }

    ctc_debug_set_flag("ftm", "ftm", typeenum, level, FALSE);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_opf_debug_on,
        ctc_cli_ftm_opf_debug_on_cmd,
        "debug (opf|fpa) (debug-level {func|param|info|error} |)",
        CTC_CLI_DEBUG_STR,
        "Offset pool freelist",
        "Fast priority arrangement",
        CTC_CLI_DEBUG_LEVEL_STR,
        CTC_CLI_DEBUG_LEVEL_FUNC,
        CTC_CLI_DEBUG_LEVEL_PARAM,
        CTC_CLI_DEBUG_LEVEL_INFO,
        CTC_CLI_DEBUG_LEVEL_ERROR)
{

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

    if(CLI_CLI_STR_EQUAL("opf", 0))
    {
        ctc_debug_set_flag("opf", "opf", OPF_SYS, level, TRUE);
    }
    else if(CLI_CLI_STR_EQUAL("fpa", 0))
    {
        ctc_debug_set_flag("fpa", "fpa", FPA, level, TRUE);
    }

    return CLI_SUCCESS;
}
CTC_CLI(ctc_cli_ftm_opf_debug_off,
        ctc_cli_ftm_opf_debug_off_cmd,
        "no debug (opf|fpa)",
        CTC_CLI_NO_STR,
        CTC_CLI_DEBUG_STR,
        "Offset pool freelist",
        "Fast priority arrangement")
{
    uint8 level = 0;

    if(CLI_CLI_STR_EQUAL("opf", 0))
    {
        ctc_debug_set_flag("opf", "opf", OPF_SYS, level, FALSE);
    }
    else if(CLI_CLI_STR_EQUAL("fpa", 0))
    {
        ctc_debug_set_flag("fpa", "fpa", FPA, level, FALSE);
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_opf_show_debug,
        ctc_cli_ftm_opf_show_debug_cmd,
        "show debug (opf|fpa)",
        CTC_CLI_SHOW_STR,
        CTC_CLI_DEBUG_STR,
        "Offset pool freelist",
        "Fast priority arrangement")
{
    uint8 level = 0;
    uint8 en = 0;

    if(CLI_CLI_STR_EQUAL("opf", 0))
    {
        en = ctc_debug_get_flag("opf", "opf", OPF_SYS, &level);
        ctc_cli_out(" opf  debug %s level:%s\n",
                    en ? "on" : "off", ctc_cli_get_debug_desc(level));
    }
    else if(CLI_CLI_STR_EQUAL("fpa", 0))
    {
        en = ctc_debug_get_flag("fpa", "fpa", FPA, &level);
        ctc_cli_out(" fpa  debug %s level:%s\n",
                    en ? "on" : "off", ctc_cli_get_debug_desc(level));
    }

    return CLI_SUCCESS;

}

CTC_CLI(ctc_cli_ftm_set_hash_poly,
        ctc_cli_ftm_set_hash_poly_cmd,
        "ftm hash-poly mem-id MEM_ID ploy POLY",
        CTC_CLI_MEM_ALLOCM_STR,
        "Hash poly",
        "Memory id",
        "Memory value",
        "Polynomial",
        "Type value")
{
    int32 ret = 0;
    ctc_ftm_hash_poly_t hash_poly;

    sal_memset(&hash_poly, 0, sizeof(hash_poly));

    CTC_CLI_GET_UINT32("mem_id", hash_poly.mem_id, argv[0]);
    CTC_CLI_GET_UINT32("poly", hash_poly.cur_poly_type, argv[1]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_ftm_set_hash_poly(g_api_lchip, &hash_poly);
    }
    else
    {
        ret = ctc_ftm_set_hash_poly(&hash_poly);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_get_hash_poly,
        ctc_cli_ftm_get_hash_poly_cmd,
        "show ftm hash-poly mem-id MEM_ID",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Hash poly",
        "Memory id",
        "Memory value")
{
    int32 ret = 0;
    ctc_ftm_hash_poly_t hash_poly;
    uint8 i = 0;

    sal_memset(&hash_poly, 0, sizeof(hash_poly));

    CTC_CLI_GET_UINT32("mem_id", hash_poly.mem_id, argv[0]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_ftm_get_hash_poly(g_api_lchip, &hash_poly);
    }
    else
    {
        ret = ctc_ftm_get_hash_poly(&hash_poly);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    ctc_cli_out("Memory              : %d\n", hash_poly.mem_id);
    ctc_cli_out("Current-type        : %d\n", hash_poly.cur_poly_type);
    ctc_cli_out("Hash-capability     :");
    for (i = 0; i  < hash_poly.poly_num; i ++)
    {
        ctc_cli_out(" %d", hash_poly.poly_type[i]);
    }
    ctc_cli_out("\n");

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_set_spec_profile,
        ctc_cli_ftm_set_spec_profile_cmd,
        "ftm spec SPEC_TYPE profile VALUE",
        CTC_CLI_MEM_ALLOCM_STR,
        "Spec type",
        "Type value",
        "Spec profile",
        "Profile value")
{
    int32 ret = CLI_SUCCESS;
    uint32 spec_type = 0;
    uint32 profile = 0;

    CTC_CLI_GET_UINT32("spec", spec_type, argv[0]);
    CTC_CLI_GET_UINT32("profile", profile, argv[1]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_ftm_set_profile_specs(g_api_lchip, spec_type, profile);
    }
    else
    {
        ret = ctc_ftm_set_profile_specs(spec_type, profile);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_ftm_get_spec_profile,
        ctc_cli_ftm_get_spec_profile_cmd,
        "show ftm spec SPEC_TYPE",
        CTC_CLI_SHOW_STR,
        CTC_CLI_MEM_ALLOCM_STR,
        "Spec type",
        "Type value")
{
    int32 ret = CLI_SUCCESS;
    uint32 spec_type = 0;
    uint32 profile = 0;

    CTC_CLI_GET_UINT32("spec", spec_type, argv[0]);

    if (g_ctcs_api_en)
    {
        ret = ctcs_ftm_get_profile_specs(g_api_lchip, spec_type, &profile);
    }
    else
    {
        ret = ctc_ftm_get_profile_specs(spec_type, &profile);
    }

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    ctc_cli_out("spec_type: %d  profile: %u\n\r", spec_type, profile);

    return CLI_SUCCESS;
}


int32
ctc_ftm_cli_init(void)
{
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_debug_off_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ftm_opf_debug_on_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_opf_debug_off_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_opf_show_debug_cmd);

    install_element(CTC_SDK_MODE, &ctc_cli_ftm_set_hash_poly_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_get_hash_poly_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_set_spec_profile_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_ftm_get_spec_profile_cmd);

    return CTC_E_NONE;
}

