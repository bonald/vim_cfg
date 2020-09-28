
#include "proto.h"
#include "param_check.h"
#include "lib_fs.h"

static int32
_cdbctl_local_act_copy_file(cdb_node_t* p_node, cfg_parse_info_t *p_info)
{
    char *srcname = NULL;
    char *dstname = NULL;
    char dst_file[M_FULLPATH_MAX_LEN];
    int32 rc = PM_E_NONE;
    uint32 i = 0;
    char *vrf = NULL;
    char errmsg[M_FULLPATH_MAX_LEN];

    errmsg[0] = CMD_ZERO_CHAR;
    rc = cfg_cmd_strip_quotation(p_info);
    if (rc < 0)
    {
        return rc;
    }

    if (0 == sal_strcmp(p_info->argv[0], "mgmt-if"))
    {
        vrf = "MANG_VRF";
        i++;
    }
    
    srcname = p_info->argv[i];
    dstname = p_info->argv[i+1];
    
    if (gen_check_and_get_filename(dstname, dst_file, M_FULLPATH_MAX_LEN) != 0)
    {
        sal_printf("%% Invalid destination file name: %s\n", dstname);
        return PM_E_FAIL;
    }

    if (syscmd_check_actual_directory_exist(dst_file) != 0)
    {
        sal_printf("%% Invalid destination directory: %s\n", dstname);
        return PM_E_FAIL;
    }
    
/* delete by weij for bug 4662
       if (syscmd_is_directory(dst_file))
       {
       cli_out(cli, "%% Invalid file name: Is directory\n");
       return CLI_ERROR;
    }

    if (imish_confirm_overwrite_check(cli, "overwrite", dst_file, argv[1]) != 0)
    {
        return CLI_SUCCESS;
    }
*/

    rc = gen_recv_from_url(errmsg, M_FULLPATH_MAX_LEN, dst_file, srcname, vrf);
    if (rc < 0)
    {
        sal_printf("%s", errmsg);
    }
    return rc;
}

static int32
_cdbctl_local_dispatch(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    int32 rc = PM_E_NONE;
    
    if (CDB_NODE_ACT == p_node->type)
    {
        switch (p_node->id)
        {
        case ACT_COPY_FILE:
            rc = _cdbctl_local_act_copy_file(p_node, p_info);
            break;
            
        default:
            sal_printf("Invalid local dispatch action %s", p_node->full_name);
        }
    }
    else
    {
        sal_printf("Invalid local dispatch %s", p_node->full_name);
    }

    return rc;
}

int32
cdbclt_process_cmd_local(cdb_node_t* p_node, void *p_data, uint32 data_len, cfg_parse_info_t *p_info)
{
    char           *para_str = NULL;
    cfg_parse_info_t parse_info;
    int32 rc = PM_E_NONE;

    para_str = p_data;
    rc = cfg_cmd_parse_para(para_str, &parse_info);
    if (rc < 0)
    {
        sal_printf("Parse parameter fail, return code %d", rc);
    }

    rc = _cdbctl_local_dispatch(p_node, p_data, data_len, &parse_info);
    return rc;
}

