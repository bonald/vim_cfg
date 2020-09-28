
#include "proto.h"
#include "glb_l2_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_version_define.h"
#include "sdb_defines.h"

/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s [options] [script_file] [script_file] ...\n", name);
        sal_printf("CLI utility. Command line shell."
            "The part of the klish project.\n");
        sal_printf("Options:\n");
        sal_printf("\t-v, --version\tPrint version.\n");
        sal_printf("\t-h, --help\tPrint this help.\n");
    }
}

#define CDB_ROOR_PATH   "/cdb"
#define VIEW_RREFIX     "# "

static char curr_path[MAX_CMD_STR_LEN];
static char view_prefix[MAX_CMD_STR_LEN];


typedef struct
{
    char       *argv[DB_MAX_DEPTH];
    uint32      argc;
} view_arg_info_t;

int32
view_strip_end(char *line)
{
    uint32 len = sal_strlen(line) - 1;
    uint32 i = 0;
    for (i = len; i >= 0; i--)
    {
        switch (line[i])
        {
        case '\r':
        case '\n':
            line[i] = '\0';
            break;
            
        default:
            return PM_E_NONE;
        }
    }

    return PM_E_NONE;
}

static int32
_view_cmd_get_parent_path(char *path)
{
    int i = 0;
    int len = 0;

    if (0 == sal_strcmp(path, CDB_ROOR_PATH))
    {
        return 0;
    }

    len = sal_strlen(path);
    for (i = len; i > 0; i--)
    {
        if ('/' == path[i])
        {
            path[i] = '\0';
            return 0;
        }
    }
    
    return PM_E_NONE;
}

int32
_view_dump_sub_path(cdb_node_t* p_node)
{
    cdb_tbl_info_t     *p_tbl_info = NULL;
    cdb_field_t        *p_field = NULL;
    uint32 i = 0;

    p_tbl_info = p_node->tbl_info;
    sal_printf("ID          : %d\n", p_node->id);
    sal_printf("Name        : %s\n", p_node->name);
    sal_printf("Full Name   : %s\n", p_node->full_name);
    sal_printf("Type        : %s\n", cdb_node_type_str(p_node->type));
    sal_printf("Key Size    : %u\n", p_tbl_info->key_size);
    sal_printf("Data Size   : %u\n", p_tbl_info->data_size);
    sal_printf("Field Num   : %u\n", p_tbl_info->field_num);
    sal_printf("Sub-path has:\n");
    sal_printf("%-10s %-20s %-12s %-16s %-12s\n", "Field ID", "Field Name", "DT Type", "DS Type", "Access");
    sal_printf("----------------------------------------------------------------------------\n");
    for (i = 0; i < p_tbl_info->field_num; i++)
    {
        p_field = &p_tbl_info->field[i];
        sal_printf("%-10u %-20s %-12s %-16s %-12s\n",
            i,
            p_field->name,
            cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type),
            cdb_ds_val2str(p_field->ds_type),
            cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));
    }
    return PM_E_NONE;
}

static int32
_view_dump_tbl_level_key_from_file_one(char *indent, char *line)
{
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);
    sal_printf("%s%s\n", indent, info.value[0]);
    return rc;
}

static int32
_view_dump_tbl_level_key_from_file(cdb_node_t *p_node, char *file_name)
{
    FILE* fp = NULL;
    char  line[FILE_READ_LINE_LEN];
    char *indent = "  ";

    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _view_dump_tbl_level_key_from_file_one(indent, line);
    }
    sal_fclose(fp);
    sal_unlink(file_name);

    return PM_E_NONE;
}

cdb_field_t*
get_field_by_name(cdb_tbl_info_t *p_tbl_info, const char* field_name)
{
    uint32 fld_id = 0;
    
    for (fld_id = 0; fld_id < p_tbl_info->field_num; fld_id++)
    {
        if (0 == sal_strcmp(p_tbl_info->field[fld_id].name, field_name))
        {
            return &(p_tbl_info->field[fld_id]);
        }
    }

    return NULL;
}

static int32
_view_dump_tbl_level_data_from_file_one_field(cdb_field_t *p_field, char *indent, char *argv, char *value)
{
    cfg_parse_get_info_t ds_info;
    uint32 str_len = 0;
    int32 i = 0;
    char *p_equal = NULL;
    
    if (CDB_DT_DS == p_field->dt_type)
    {
        sal_printf("%s%-25s:\n", indent, argv);

        // strip '}'
        str_len = sal_strlen(value);
        if (CMD_DS_RIGHT_BRA_CHAR == value[str_len - 1])
        {
            value[str_len - 1] = CMD_ZERO_CHAR;
        }
        
        cfg_get_parse(value, &ds_info, 1);
        for (i = 0; i < ds_info.argc; i++)
        {
            p_equal = sal_strstr(ds_info.argv[i], CMD_EQUAL_STR);
            if (p_equal)
            {
                p_equal[0] = CMD_ZERO_CHAR;
            }
            sal_printf("%s%s%-25s: %s\n", indent, indent, ds_info.argv[i], ds_info.value[i]);
        }
    }
    else
    {
        sal_printf("%s%-25s: %s\n", indent, argv, value);
    }

    return PM_E_NONE;
}

static int32
_view_dump_tbl_level_data_from_file_one(cdb_node_t *p_node, char *indent, char *line, uint32 detail)
{
    cfg_parse_get_info_t info;
    cdb_tbl_info_t      *p_tbl_info = NULL;
    cdb_field_t         *p_field = NULL;
    cdb_node_t *p_ds_node = NULL;
    char *p_equal = NULL;
    int32 i = 0;
    int32 rc = PM_E_NONE;
    
    cfg_get_parse(line, &info, 0);

    p_tbl_info = p_node->tbl_info;
    if (1 < info.argc)
    {
        sal_printf("ID          : %d\n", p_node->id);
        sal_printf("Name        : %s\n", p_node->name);
        sal_printf("Full Name   : %s\n", p_node->full_name);
        sal_printf("Type        : %s\n", cdb_node_type_str(p_node->type));
        sal_printf("\n");
        
        sal_printf("### table info ###\n");
        sal_printf("Key Size    : %u\n", p_tbl_info->key_size);
        sal_printf("Data Size   : %u\n", p_tbl_info->data_size);
        sal_printf("Field Num   : %u\n", p_tbl_info->field_num);
        sal_printf("%-10s %-20s %-12s %-16s %-12s\n", "Field ID", "Field Name", "DT Type", "DS Type", "Access");
        sal_printf("----------------------------------------------------------------------------\n");
        for (i = 0; i < p_tbl_info->field_num; i++)
        {
            p_field = &p_tbl_info->field[i];
            sal_printf("%-10u %-20s %-12s %-16s %-12s\n",
                i,
                p_field->name,
                cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type),
                cdb_ds_val2str(p_field->ds_type),
                cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));
        }
    }
    else if (1 == info.argc)
    {
        p_equal = sal_strstr(info.argv[0], CMD_EQUAL_STR);
        if (p_equal)
        {
            p_equal[0] = CMD_ZERO_CHAR;
        }
        p_field = get_field_by_name(p_tbl_info, info.argv[0]);
        if (NULL == p_field)
        {
            sal_printf(" Invalid field name: %s\n", info.argv[0]);
        }

        sal_printf("ID          : %d\n", p_node->id);
        sal_printf("Name        : %s\n", p_node->name);
        sal_printf("Full Name   : %s\n", p_node->full_name);
        sal_printf("Type        : %s\n", cdb_node_type_str(p_node->type));
        sal_printf("\n");
        
        sal_printf("### field info ###\n");

        if (CDB_DS_INVALID == p_field->ds_type)
        {        
            sal_printf("%-10s %-20s %-12s %-16s %-12s\n", "Field ID", "Field Name", "DT Type", "DS Type", "Access");
            sal_printf("----------------------------------------------------------------------------\n");
            sal_printf("%-10u %-20s %-12s %-16s %-12s\n",
                    i,
                    p_field->name,
                    cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type),
                    cdb_ds_val2str(p_field->ds_type),
                    cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));
        }
        else
        {
            p_ds_node = cdb_get_ds(p_field->ds_type);
            if (p_ds_node)
            {
                sal_printf("ID          : %d\n", p_node->id);
                sal_printf("Name        : %s\n", p_node->name);
                sal_printf("Type        : %s\n", cdb_node_type_str(p_node->type));
                sal_printf("\n");

                sal_printf("### ds info ###\n");
                sal_printf("Key Size    : %u\n", p_tbl_info->key_size);
                sal_printf("Data Size   : %u\n", p_tbl_info->data_size);
                sal_printf("Field Num   : %u\n", p_tbl_info->field_num);
                sal_printf("%-10s %-20s %-12s %-16s %-12s\n", "Field ID", "Field Name", "DT Type", "DS Type", "Access");
                sal_printf("----------------------------------------------------------------------------\n");
                
                p_tbl_info = p_ds_node->tbl_info;
                for (i = 0; i < p_tbl_info->field_num; i++)
                {
                    p_field = &p_tbl_info->field[i];
                    sal_printf("%-10u %-20s %-12s %-16s %-12s\n",
                        i,
                        p_field->name,
                        cdb_enum_val2str(cdb_dt_type_str_array, CDB_DT_MAX, p_field->dt_type),
                        cdb_ds_val2str(p_field->ds_type),
                        cdb_enum_val2str(cdb_access_str_array, CDB_ACCESS_MAX, p_field->access));
                }
            }
        }
    }
    else
    {
        sal_printf(" Invalid return line: %s\n", line);
        return PM_E_NONE;
    } 

    if (detail)
    {
        sal_printf("\n### detail value ###\n");
        p_tbl_info = p_node->tbl_info;
        
        if (1 < info.argc)
        {
            for (i = 0; i < info.argc; i++)
            {
                p_equal = sal_strstr(info.argv[i], CMD_EQUAL_STR);
                if (p_equal)
                {
                    p_equal[0] = CMD_ZERO_CHAR;
                }

                p_field = &p_tbl_info->field[i];
                _view_dump_tbl_level_data_from_file_one_field(p_field, indent, info.argv[i], info.value[i]);

            }
        }
        else if (1 == info.argc)
        {
            p_equal = sal_strstr(info.argv[0], CMD_EQUAL_STR);
            if (p_equal)
            {
                p_equal[0] = CMD_ZERO_CHAR;
            }
            p_field = get_field_by_name(p_tbl_info, info.argv[0]);
            if (NULL == p_field)
            {
                sal_printf(" Invalid field name: %s\n", info.argv[0]);
            }
            _view_dump_tbl_level_data_from_file_one_field(p_field, indent, info.argv[0], info.value[0]);
        }
        else
        {
            sal_printf(" Invalid return line: %s\n", line);
            return PM_E_NONE;
        }
    }
    
    return rc;
}

static int32
_view_dump_tbl_level_data_from_file(cdb_node_t *p_node, char *file_name, uint32 detail)
{
    FILE* fp = NULL;
    char  line[FILE_READ_LINE_LEN];
    char *indent = "  ";

    fp = sal_fopen(file_name, "r");
    if (!fp)
    {
        return PM_E_FAIL;
    }

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _view_dump_tbl_level_data_from_file_one(p_node, indent, line, detail);
    }
    sal_fclose(fp);
    sal_unlink(file_name);

    return PM_E_NONE;
}

int32
view_dump_tbl_value(cdb_node_t* p_node, char *p_data, uint32 data_len, char *path, uint32 detail)
{
    cdb_tbl_info_t *p_tbl_info = NULL;
    ctc_msg_t resp_msg;
    ctc_msg_t *p_resp_msg = &resp_msg;
    cfg_show_t *p_resp_show = NULL;
    cfg_result_t *p_resp_rslt = NULL;
    int32 rc = PM_E_NONE;

    p_tbl_info = p_node->tbl_info;
    rc = ipc_ctl_to_ccs_cfg(CDB_OPER_GET, p_node, p_data, data_len, p_resp_msg);
    if (rc < 0)
    {
        sal_printf("%% Command: %s failed, rc is %d\n", (char*)p_data, rc);
        return rc;
    }

    if (p_resp_msg->data_len == sizeof(cfg_show_t))
    {
        p_resp_show = (cfg_show_t *)p_resp_msg->data;
        /* global table should show data */
        if (0 == data_len && p_tbl_info->key_size)
        {
            _view_dump_tbl_level_key_from_file(p_node, p_resp_show->path);
        }
        else
        {
            _view_dump_tbl_level_data_from_file(p_node, p_resp_show->path, detail);
        }
    }
    else if (p_resp_msg->data_len == sizeof(cfg_result_t))
    {
        p_resp_rslt = (cfg_result_t *)p_resp_msg->data;
        rc = p_resp_rslt->rc;
        sal_printf("Path %s does not exist!\n", path);
        return PM_E_FAIL;
    }
    ctc_msg_free(p_resp_msg);

    return rc;
}

int32
view_dump_sub_path(char *path, uint32 detail)
{
    cdb_node_t* p_node = NULL;
    cdb_node_t *p_sub = NULL;
    ctclib_list_node_t *p_listnode = NULL;
    char *p_data = NULL;
    uint32 data_len = 0;
    int32 rc = PM_E_NONE;
 
    p_node = cdb_get_node_by_name(path);
    if (NULL == p_node)
    {
        sal_printf("Unexpected Error: Path %s does not exist!\n", path);
        SAL_ASSERT(0);
        return PM_E_FAIL;
    }

    if (CDB_NODE_DIR == p_node->type)
    {
        sal_printf("ID          : %d\n", p_node->id);
        sal_printf("Name        : %s\n", p_node->name);
        sal_printf("Full Name   : %s\n", p_node->full_name);
        sal_printf("Type        : %s\n", cdb_node_type_str(p_node->type));
        sal_printf("Sub-path has:\n");
        if (!ctclib_list_empty(&p_node->sub))
        {
            sal_printf("%-20s %-15s\n", "Name", "Type");
            sal_printf("----------------------------\n");
            ctclib_list_for_each(p_listnode, &p_node->sub)
            {
                p_sub = ctclib_container_of(p_listnode, cdb_node_t, node);
                rc = cdb_check_product(p_sub);
                if (rc < 0)
                {
                    continue;
                }
                sal_printf("%-20s %-15s\n", p_sub->name, cdb_node_type_str(p_sub->type));
            }
        }
    }
    else if (CDB_NODE_TBL == p_node->type)
    {
        p_data = path;
        p_data = sal_strstr(p_data, p_node->full_name);
        if (NULL == p_data)
        {
            SAL_ASSERT(0);
        }
        p_data += sal_strlen(p_node->full_name);
        if (CMD_DIVISION_CHAR == p_data[0])
        {
            p_data++;
        }

        if ((NULL == p_data) || (CMD_ZERO_CHAR == p_data[0]))
        {
            data_len = 0;
        }
        else
        {
            data_len = sal_strlen(p_data) + 1;
        }

        return view_dump_tbl_value(p_node, p_data, data_len, path, detail);
    }
    else
    {

    }
    
    return PM_E_NONE;
}   

int32
view_check_path(char *new_path, char *curr_path)
{
    cdb_node_t* p_node = NULL;
    int32 rc = PM_E_NONE;

    p_node = cdb_get_node_by_name(new_path);
    if (NULL == p_node)
    {
        sal_printf("Path %s does not exist!\n", new_path);
        return PM_E_FAIL;
    }
    
    rc = cdb_check_product(p_node);
    if (rc < 0)
    {
        sal_printf("Path %s is not supported in the product!\n", new_path);
        return PM_E_FAIL;
    }

    rc = view_dump_sub_path(new_path, FALSE);
    
    return rc;
}

int32
view_cmd_ls(view_arg_info_t *p_info)
{
    view_dump_sub_path(curr_path, TRUE);
    return PM_E_NONE;
}

int32
view_cmd_cd(view_arg_info_t *p_info)
{
    char new_path[MAX_CMD_STR_LEN];
    int32 rc = PM_E_NONE;
    
    if (2 != p_info->argc)
    {
        sal_printf("cd command should has 2 parameters\n");
        return PM_E_NONE;
    }

    sal_memset(new_path, 0, sizeof(new_path));
    
    if (0 == sal_strcmp(p_info->argv[1], "."))
    {
    }
    else if (0 == sal_strcmp(p_info->argv[1], ".."))
    {
        sal_strncpy(new_path, curr_path, MAX_CMD_STR_LEN);
        _view_cmd_get_parent_path(new_path);
    }
    else
    {
        sal_snprintf(new_path, 256, "%s/%s", curr_path, p_info->argv[1]);
    }

    rc = view_check_path(new_path, curr_path);
    if (rc < 0)
    {
        return rc;
    }
    sal_strncpy(curr_path, new_path, MAX_CMD_STR_LEN);
        
    return PM_E_NONE;
}

int32
view_get_arg_info(view_arg_info_t *p_info, char *line)
{
    uint32 valid_char = FALSE;
    uint32 len = 0;
    uint32 i = 0;
    
    len = sal_strlen(line);
    
    for (i = 0; i < len; i++)
    {
        if (' ' == line[i] || '\t' == line[i])
        {
            line[i] = '\0';
            valid_char = FALSE;
        }
        else
        {
            if (!valid_char)
            {
                p_info->argv[p_info->argc] = &line[i];
                p_info->argc++;
                valid_char = TRUE;
            }
        }
    }

    return PM_E_NONE;
}

int32
view_cmd_print_prompt()
{
    sal_printf("%s%s", curr_path, view_prefix);
    sal_fflush(stdout);
    
    return PM_E_NONE;
}

int32
view_cmd_dispatch(char *line)
{
    char buf[MAX_CMD_STR_LEN];
    view_arg_info_t info;

    sal_memset(&info, 0, sizeof(info));
    sal_memset(buf, 0, sizeof(buf));

    view_strip_end(line);
    sal_strncpy(buf, line, MAX_CMD_STR_LEN);
    view_get_arg_info(&info, buf);
    
    if (0 == info.argc)
    {
    }    
    else if (0 == sal_strcmp("ls", info.argv[0]))
    {
        view_cmd_ls(&info);
    }
    else if (0 == sal_strcmp("pwd", info.argv[0]))
    {
        sal_printf("%s\n", curr_path);
    }
    else if (0 == sal_strcmp("cd", info.argv[0]))
    {
        view_cmd_cd(&info);
    }
    else
    {
        sal_printf("invalid cmd: %s\n", line);
    }
    sal_fflush(stdout);
    
    view_cmd_print_prompt();
    
    return PM_E_NONE;
}

int32
view_wait_loop()
{
    char buf[MAX_CMD_STR_LEN];
    int32 rc = PM_E_NONE;

    sal_memset(curr_path, 0, sizeof(curr_path));
    sal_memset(view_prefix, 0, sizeof(view_prefix));
    
    sal_strncpy(curr_path, CDB_ROOR_PATH, MAX_CMD_STR_LEN);
    sal_strncpy(view_prefix, VIEW_RREFIX, MAX_CMD_STR_LEN);

    view_cmd_print_prompt();

    while(1) 
    {
        sal_memset(buf, 0, sizeof(buf));
        rc = read(fileno(stdin), buf, MAX_CMD_STR_LEN);
        if (0 == rc)
        {
            break;
        }
        else if (rc < 0) 
        {
            perror("Reading from standard input");
            exit(1);
        }
        if (0 == sal_strncmp(buf, "exit", 4))
        {
            break;
        }
        view_cmd_dispatch(buf);
    }

    return PM_E_NONE;
}

int32
main(int32 argc, char **argv)
{
    int32 rc = PM_E_NONE;
    char *p;
    char *progname;

    static const char *shortopts = "h";
    
/* Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
    { "help",        no_argument,       NULL, 'h'},
    { 0 }
};
#endif /* HAVE_GETOPT_H */

    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {
        int32 opt;
      
#ifdef HAVE_GETOPT_H
        opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
        opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
      
        if (EOF == opt)
        {
            break;
        }

        switch (opt)
        {
        case 'h':
            usage(FALSE, progname);
            break;
        default:
            usage(TRUE, progname);
            break;
        }
    }

    lib_init(PM_ID_CTL);
    sdb_init(PM_ID_CTL);
    ctc_task_init();

    rc = ipc_connect_ccs(5);
    view_wait_loop();
    exit(rc);
}

