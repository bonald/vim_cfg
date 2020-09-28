
#include "sdb.h"
#include "gen/stbl_sai_glb_define.h"
#include "gen/stbl_sai_glb.h"
#include "sdb_data_cmp.h"

stbl_sai_glb_t *_g_p_stbl_sai_glb = NULL;

int32
stbl_sai_glb_set_sai_glb_field_sync(stbl_sai_glb_t *p_sai_glb, stbl_sai_glb_field_id_t field_id, uint32 sync)
{
    stbl_sai_glb_t *p_db_sai_glb = _g_p_stbl_sai_glb;

    /* 2. update db entry field */
    switch (field_id)
    {
    case STBL_SAI_GLB_FLD_NH_MAX_COUNT:
        p_db_sai_glb->nh_max_count = p_sai_glb->nh_max_count;
        break;

    case STBL_SAI_GLB_FLD_NH_GROUP_MAX_COUNT:
        p_db_sai_glb->nh_group_max_count = p_sai_glb->nh_group_max_count;
        break;

    case STBL_SAI_GLB_FLD_VRF_MAX_COUNT:
        p_db_sai_glb->vrf_max_count = p_sai_glb->vrf_max_count;
        break;

    case STBL_SAI_GLB_FLD_RIF_MAX_COUNT:
        p_db_sai_glb->rif_max_count = p_sai_glb->rif_max_count;
        break;

    case STBL_SAI_GLB_FLD_ROUTE_MAX_COUNT:
        p_db_sai_glb->route_max_count = p_sai_glb->route_max_count;
        break;

    case STBL_SAI_GLB_FLD_STP_ID_MAX_COUNT:
        p_db_sai_glb->stp_id_max_count = p_sai_glb->stp_id_max_count;
        break;

    case STBL_SAI_GLB_FLD_NH_GROUP_CURR_COUNT:
        p_db_sai_glb->nh_group_curr_count = p_sai_glb->nh_group_curr_count;
        break;

    case STBL_SAI_GLB_FLD_HOSTIF_MAX_COUNT:
        p_db_sai_glb->hostif_max_count = p_sai_glb->hostif_max_count;
        break;

    case STBL_SAI_GLB_FLD_HOSTIF_GROUP_MAX_COUNT:
        p_db_sai_glb->hostif_group_max_count = p_sai_glb->hostif_group_max_count;
        break;

    case STBL_SAI_GLB_FLD_VRRP_MAX_COUNT:
        p_db_sai_glb->vrrp_max_count = p_sai_glb->vrrp_max_count;
        break;

    case STBL_SAI_GLB_FLD_VLAN_CLASS_MAX_COUNT:
        p_db_sai_glb->vlan_class_max_count = p_sai_glb->vlan_class_max_count;
        break;

    case STBL_SAI_GLB_FLD_HASH_MAX_COUNT:
        p_db_sai_glb->hash_max_count = p_sai_glb->hash_max_count;
        break;

    case STBL_SAI_GLB_FLD_LAG_MAX_COUNT:
        p_db_sai_glb->lag_max_count = p_sai_glb->lag_max_count;
        break;

    case STBL_SAI_GLB_FLD_QOS_WRED_MAX_COUNT:
        p_db_sai_glb->qos_wred_max_count = p_sai_glb->qos_wred_max_count;
        break;

    case STBL_SAI_GLB_FLD_QOS_MAP_MAX_COUNT:
        p_db_sai_glb->qos_map_max_count = p_sai_glb->qos_map_max_count;
        break;

    case STBL_SAI_GLB_FLD_QOS_SCHEDULER_MAX_COUNT:
        p_db_sai_glb->qos_scheduler_max_count = p_sai_glb->qos_scheduler_max_count;
        break;

    case STBL_SAI_GLB_FLD_QOS_POLICER_MAX_COUNT:
        p_db_sai_glb->qos_policer_max_count = p_sai_glb->qos_policer_max_count;
        break;

    case STBL_SAI_GLB_FLD_DEFAULT_STP_ID:
        p_db_sai_glb->default_stp_id = p_sai_glb->default_stp_id;
        break;

    case STBL_SAI_GLB_FLD_HOSTIF_FD_HASH:
        p_db_sai_glb->hostif_fd_hash = p_sai_glb->hostif_fd_hash;
        break;

    case STBL_SAI_GLB_FLD_HOSTIF_PORTIF_HASH:
        p_db_sai_glb->hostif_portif_hash = p_sai_glb->hostif_portif_hash;
        break;

    case STBL_SAI_GLB_FLD_HOSTIF_VLANIF_HASH:
        p_db_sai_glb->hostif_vlanif_hash = p_sai_glb->hostif_vlanif_hash;
        break;

    case STBL_SAI_GLB_FLD_L2MC_GROUP_OPB:
        sal_memcpy(&p_db_sai_glb->l2mc_group_opb, &p_sai_glb->l2mc_group_opb, sizeof(p_sai_glb->l2mc_group_opb));
        break;

    case STBL_SAI_GLB_FLD_ISOLATION_ID_OPB:
        sal_memcpy(&p_db_sai_glb->isolation_id_opb, &p_sai_glb->isolation_id_opb, sizeof(p_sai_glb->isolation_id_opb));
        break;

    case STBL_SAI_GLB_FLD_MIRROR_ID_OPB:
        sal_memcpy(&p_db_sai_glb->mirror_id_opb, &p_sai_glb->mirror_id_opb, sizeof(p_sai_glb->mirror_id_opb));
        break;

    case STBL_SAI_GLB_FLD_NEXTHOP_ID_OPB:
        sal_memcpy(&p_db_sai_glb->nexthop_id_opb, &p_sai_glb->nexthop_id_opb, sizeof(p_sai_glb->nexthop_id_opb));
        break;

    case STBL_SAI_GLB_FLD_NEXTHOP_GROUP_ID_OPB:
        sal_memcpy(&p_db_sai_glb->nexthop_group_id_opb, &p_sai_glb->nexthop_group_id_opb, sizeof(p_sai_glb->nexthop_group_id_opb));
        break;

    case STBL_SAI_GLB_FLD_STPID_OPB:
        sal_memcpy(&p_db_sai_glb->stpid_opb, &p_sai_glb->stpid_opb, sizeof(p_sai_glb->stpid_opb));
        break;

    case STBL_SAI_GLB_FLD_TAP_GROUP_OPB:
        sal_memcpy(&p_db_sai_glb->tap_group_opb, &p_sai_glb->tap_group_opb, sizeof(p_sai_glb->tap_group_opb));
        break;

    case STBL_SAI_GLB_FLD_VLAN_CLASS_OPB:
        sal_memcpy(&p_db_sai_glb->vlan_class_opb, &p_sai_glb->vlan_class_opb, sizeof(p_sai_glb->vlan_class_opb));
        break;

    case STBL_SAI_GLB_FLD_VRF_ID_OPB:
        sal_memcpy(&p_db_sai_glb->vrf_id_opb, &p_sai_glb->vrf_id_opb, sizeof(p_sai_glb->vrf_id_opb));
        break;

    case STBL_SAI_GLB_FLD_HASH_OPB:
        sal_memcpy(&p_db_sai_glb->hash_opb, &p_sai_glb->hash_opb, sizeof(p_sai_glb->hash_opb));
        break;

    case STBL_SAI_GLB_FLD_LAG_OPB:
        sal_memcpy(&p_db_sai_glb->lag_opb, &p_sai_glb->lag_opb, sizeof(p_sai_glb->lag_opb));
        break;

    case STBL_SAI_GLB_FLD_QOS_WRED_OPB:
        sal_memcpy(&p_db_sai_glb->qos_wred_opb, &p_sai_glb->qos_wred_opb, sizeof(p_sai_glb->qos_wred_opb));
        break;

    case STBL_SAI_GLB_FLD_QOS_MAP_OPB:
        sal_memcpy(&p_db_sai_glb->qos_map_opb, &p_sai_glb->qos_map_opb, sizeof(p_sai_glb->qos_map_opb));
        break;

    case STBL_SAI_GLB_FLD_QOS_SCHEDULER_OPB:
        sal_memcpy(&p_db_sai_glb->qos_scheduler_opb, &p_sai_glb->qos_scheduler_opb, sizeof(p_sai_glb->qos_scheduler_opb));
        break;

    case STBL_SAI_GLB_FLD_QOS_POLICER_OPB:
        sal_memcpy(&p_db_sai_glb->qos_policer_opb, &p_sai_glb->qos_policer_opb, sizeof(p_sai_glb->qos_policer_opb));
        break;

    case STBL_SAI_GLB_FLD_UDF_OPB:
        sal_memcpy(&p_db_sai_glb->udf_opb, &p_sai_glb->udf_opb, sizeof(p_sai_glb->udf_opb));
        break;

    case STBL_SAI_GLB_FLD_COPP_ENTRY_OPB:
        sal_memcpy(&p_db_sai_glb->copp_entry_opb, &p_sai_glb->copp_entry_opb, sizeof(p_sai_glb->copp_entry_opb));
        break;

    case STBL_SAI_GLB_FLD_MAX:
        sal_memcpy(p_db_sai_glb, p_sai_glb, sizeof(stbl_sai_glb_t));
        break;
    default:
        return SDB_E_NOT_SUPPORT;
    }

    return SDB_E_NONE;
}

int32
stbl_sai_glb_set_sai_glb_field(stbl_sai_glb_t *p_sai_glb, stbl_sai_glb_field_id_t field_id)
{
    return stbl_sai_glb_set_sai_glb_field_sync(p_sai_glb, field_id, TRUE);
}

stbl_sai_glb_t*
stbl_sai_glb_get_sai_glb()
{
    return _g_p_stbl_sai_glb;
}

int32
stbl_sai_glb_key_str2val(char *str, stbl_sai_glb_t *p_sai_glb)
{
    return SDB_E_NONE;
}

int32
stbl_sai_glb_dump_one(stbl_sai_glb_t *p_sai_glb, stbl_iter_args_t *pargs)
{
    sdb_node_t *p_node = sdb_get_tbl(STBL_SAI_GLB);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    sdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[SDB_MAX_CMD_STR_LEN];

    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_NH_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_NH_MAX_COUNT].name,
            p_sai_glb->nh_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_NH_GROUP_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_NH_GROUP_MAX_COUNT].name,
            p_sai_glb->nh_group_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_VRF_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_VRF_MAX_COUNT].name,
            p_sai_glb->vrf_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_RIF_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_RIF_MAX_COUNT].name,
            p_sai_glb->rif_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_ROUTE_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_ROUTE_MAX_COUNT].name,
            p_sai_glb->route_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_STP_ID_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_STP_ID_MAX_COUNT].name,
            p_sai_glb->stp_id_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_NH_GROUP_CURR_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_NH_GROUP_CURR_COUNT].name,
            p_sai_glb->nh_group_curr_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HOSTIF_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_HOSTIF_MAX_COUNT].name,
            p_sai_glb->hostif_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HOSTIF_GROUP_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_HOSTIF_GROUP_MAX_COUNT].name,
            p_sai_glb->hostif_group_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_VRRP_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_VRRP_MAX_COUNT].name,
            p_sai_glb->vrrp_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_VLAN_CLASS_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_VLAN_CLASS_MAX_COUNT].name,
            p_sai_glb->vlan_class_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HASH_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_HASH_MAX_COUNT].name,
            p_sai_glb->hash_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_LAG_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_LAG_MAX_COUNT].name,
            p_sai_glb->lag_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_WRED_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_WRED_MAX_COUNT].name,
            p_sai_glb->qos_wred_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_MAP_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_MAP_MAX_COUNT].name,
            p_sai_glb->qos_map_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_SCHEDULER_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_SCHEDULER_MAX_COUNT].name,
            p_sai_glb->qos_scheduler_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_POLICER_MAX_COUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_POLICER_MAX_COUNT].name,
            p_sai_glb->qos_policer_max_count);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_DEFAULT_STP_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_DEFAULT_STP_ID].name, 
            sdb_object_id_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->default_stp_id));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HOSTIF_FD_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[STBL_SAI_GLB_FLD_HOSTIF_FD_HASH].name);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HOSTIF_PORTIF_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[STBL_SAI_GLB_FLD_HOSTIF_PORTIF_HASH].name);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HOSTIF_VLANIF_HASH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[STBL_SAI_GLB_FLD_HOSTIF_VLANIF_HASH].name);
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_L2MC_GROUP_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_L2MC_GROUP_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->l2mc_group_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_ISOLATION_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_ISOLATION_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->isolation_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_MIRROR_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_MIRROR_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->mirror_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_NEXTHOP_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_NEXTHOP_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->nexthop_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_NEXTHOP_GROUP_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_NEXTHOP_GROUP_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->nexthop_group_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_STPID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_STPID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->stpid_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_TAP_GROUP_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_TAP_GROUP_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->tap_group_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_VLAN_CLASS_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_VLAN_CLASS_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->vlan_class_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_VRF_ID_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_VRF_ID_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->vrf_id_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_HASH_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_HASH_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->hash_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_LAG_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_LAG_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->lag_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_WRED_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_WRED_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->qos_wred_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_MAP_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_MAP_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->qos_map_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_SCHEDULER_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_SCHEDULER_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->qos_scheduler_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_QOS_POLICER_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_QOS_POLICER_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->qos_policer_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_UDF_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_UDF_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->udf_opb));
    }
    if (SDB_FLD_MATCH(STBL_SAI_GLB_FLD_COPP_ENTRY_OPB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[STBL_SAI_GLB_FLD_COPP_ENTRY_OPB].name, 
            sdb_opb_val2str(buf, SDB_MAX_CMD_STR_LEN, &p_sai_glb->copp_entry_opb));
    }

    sal_fprintf(fp, "\n");

    return SDB_E_NONE;
}

int32
stbl_sai_glb_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs)
{
    stbl_sai_glb_t *p_db_sai_glb = _g_p_stbl_sai_glb;
    int32 rc = SDB_E_NONE;

    if (NULL == p_db_sai_glb)
    {
        return SDB_E_NONE;
    }

    rc = fn(p_db_sai_glb, pargs);
    return (rc) ? SDB_E_FAIL : SDB_E_NONE;
}

stbl_sai_glb_t*
stbl_sai_glb_init_sai_glb()
{
    _g_p_stbl_sai_glb = XCALLOC(MEM_STBL_SAI_GLB, sizeof(stbl_sai_glb_t));
    return _g_p_stbl_sai_glb;
}

