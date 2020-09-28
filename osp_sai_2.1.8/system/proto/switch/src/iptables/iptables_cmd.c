
#include "proto.h"
#include "lib_tblinc.h"
#include "switch.h"
#include "iptables.h"


static int32
_ipt_prevent_cmd_set_icmp(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_icmp(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_icmp_rate_limit(tbl_iptables_prevent_t *p_ipt_pre, int32 value)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_icmp_rate_limit(p_ipt_pre, value));
    return rc;
}

static int32
_ipt_prevent_cmd_set_tcp(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_tcp(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_tcp_rate_limit(tbl_iptables_prevent_t *p_ipt_pre, int32 value)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_tcp_rate_limit(p_ipt_pre, value));
    return rc;
}

static int32
_ipt_prevent_cmd_set_udp(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_udp(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_udp_rate_limit(tbl_iptables_prevent_t *p_ipt_pre, int32 value)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_udp_rate_limit(p_ipt_pre, value));
    return rc;
}

static int32
_ipt_prevent_cmd_set_small_pkt(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_small_pkt(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_small_pkt_len(tbl_iptables_prevent_t *p_ipt_pre, int32 value)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_small_pkt_len(p_ipt_pre, value));
    return rc;
}

static int32
_ipt_prevent_cmd_set_smurf(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_smurf(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_fraggle(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_fraggle(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_ipeq(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_ipeq(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_maceq(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_maceq(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_pkt_statistic_update(tbl_iptables_prevent_t *p_ipt_pre, int32 enable)
{
    int32 rc = PM_E_NONE;

    PM_E_RETURN(ipt_prevent_set_pkt_statistic_update(p_ipt_pre, enable?TRUE:FALSE));
    return rc;
}

static int32
_ipt_prevent_cmd_set_global_field(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    int32 i = 0;
    int32 field_id = 0;
    int32 value = 0;
    cdb_field_t *p_field = NULL;
    cdb_node_t* p_node = NULL;
    tbl_iptables_prevent_t ipt_prevent_glb;
    tbl_iptables_prevent_t* p_db_ipt_prevent_glb;
    
    if (argc < 2)
    {
        CFG_INVALID_PARAM_RET("Parmameter is not specified");
    }
                
    p_db_ipt_prevent_glb = tbl_iptables_prevent_get_iptables_prevent();
    sal_memset(&ipt_prevent_glb, 0, sizeof(ipt_prevent_glb));

    /* 4. parse field */
    p_node = cdb_get_tbl(TBL_IPTABLES_PREVENT);
    i = 0;
    field_id = cdb_get_field_id(argv[i], p_node, &p_field);        
    
    if (field_id < 0)
    {
        CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
    }
    if (CDB_ACCESS_RD == p_field->access)
    {    
        CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
    }
    
    i++;
    /* 5. set field */
    switch (field_id)
    {
    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_ICMP:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_ICMP))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_ICMP)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_icmp(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_ICMP_RATE_LIMIT:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT_RANGE(value, GLB_IPTABLES_MIN_MAXCOUNT_NUM, GLB_IPTABLES_MAX_MAXCOUNT_NUM, 
                                argv[i], para->p_rs);
        rc = _ipt_prevent_cmd_set_icmp_rate_limit(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_TCP:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_TCP))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_TCP)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_tcp(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_TCP_RATE_LIMIT:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT_RANGE(value, GLB_IPTABLES_MIN_MAXCOUNT_NUM, GLB_IPTABLES_MAX_MAXCOUNT_NUM, 
                                argv[i], para->p_rs);
        rc = _ipt_prevent_cmd_set_tcp_rate_limit(&ipt_prevent_glb, value);
        break;
        
    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_UDP:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_UDP))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_UDP)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_udp(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_UDP_RATE_LIMIT:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT_RANGE(value, GLB_IPTABLES_MIN_MAXCOUNT_NUM, GLB_IPTABLES_MAX_MAXCOUNT_NUM, 
                                argv[i], para->p_rs);
        rc = _ipt_prevent_cmd_set_udp_rate_limit(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMALL_PKT:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_SMALL_PKT))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_SMALL_PKT)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_small_pkt(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_SMALL_PKT_LENGTH:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT_RANGE(value, GLB_IPTABLES_DEFAULT_LENGTH, GLB_IPTABLES_MAX_LENGTH, 
                                argv[i], para->p_rs);
        rc = _ipt_prevent_cmd_set_small_pkt_len(&ipt_prevent_glb, value);
        break;
        
    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_SMURF:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_SMURF))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_SMURF)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_smurf(&ipt_prevent_glb, value);
        break;
        
    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_FRAGGLE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_FRAGGLE))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_FRAGGLE)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_fraggle(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_IPEQ:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_IPEQ))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_IPEQ)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_ipeq(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_IPT_CFG_MACEQ:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        if ((value && GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_MACEQ))
            || (!value && !GLB_FLAG_ISSET(p_db_ipt_prevent_glb->flags , GLB_IPTABLES_CFG_FLAG_MACEQ)))
        {
            return PM_E_NONE;
        }
        rc = _ipt_prevent_cmd_set_maceq(&ipt_prevent_glb, value);
        break;

    case TBL_IPTABLES_PREVENT_FLD_PKT_STATISTIC_UPDATE:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_BOOL(value, argv[i], para->p_rs);
        rc = _ipt_prevent_cmd_set_pkt_statistic_update(&ipt_prevent_glb, value);
        break;

    default:
        CFG_FUNC_ERR_RET("Unknown support changed field %s", argv[i-1]);
        break; 
    }

   return rc;
}

static int32
_ipt_prevent_cmd_get_ipt_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    tbl_iter_args_t dump_args;
    field_parse_t field; 
    FILE *fp = NULL;
    tbl_iptables_prevent_t* p_db_global; 

    sal_memset(&dump_args, 0, sizeof(tbl_iter_args_t));
    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");        
    dump_args.argv[0] = &field;
    dump_args.argv[1] = fp;

    p_db_global = tbl_iptables_prevent_get_iptables_prevent();
    
    PM_E_RETURN(tbl_iptables_prevent_dump_one(p_db_global, &dump_args));
    sal_fclose(fp);
    return rc;
}


int32
ipt_prevent_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_E_RETURN(_ipt_prevent_cmd_set_global_field(argv, argc, para));
        break;
        
    case CDB_OPER_GET:
        PM_E_RETURN(_ipt_prevent_cmd_get_ipt_global(argv, argc, para)); 
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

