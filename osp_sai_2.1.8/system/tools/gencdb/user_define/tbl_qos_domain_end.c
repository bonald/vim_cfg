int32
tbl_qos_domain_get_qos_domain_field(uint8 domain_id, tbl_qos_domain_field_id_t field_id, tbl_qos_domain_t* p_domain)
{
    tbl_qos_domain_key_t key;
    tbl_qos_domain_t *p_db_qos_domain = NULL;

    key.id = domain_id;

    p_db_qos_domain = tbl_qos_domain_get_qos_domain(&key);

    /* 1. lookup entry exist */
    if (!p_db_qos_domain)
    {
        return PM_E_NOT_EXIST;
    }

    switch (field_id)
    {
    case TBL_QOS_DOMAIN_FLD_CFI_ENABLE:
        p_domain->cfi_enable = p_db_qos_domain->cfi_enable;
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PRI:
        sal_memcpy(p_domain->cos2pri, p_db_qos_domain->cos2pri, sizeof(p_db_qos_domain->cos2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2COLOR:
        sal_memcpy(p_domain->cos2color, p_db_qos_domain->cos2color, sizeof(p_db_qos_domain->cos2color));
        break;

    case TBL_QOS_DOMAIN_FLD_COS2PHB:
        sal_memcpy(p_domain->cos2phb, p_db_qos_domain->cos2phb, sizeof(p_db_qos_domain->cos2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PRI:
        sal_memcpy(p_domain->dscp2pri, p_db_qos_domain->dscp2pri, sizeof(p_db_qos_domain->dscp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2COLOR:
        sal_memcpy(p_domain->dscp2color, p_db_qos_domain->dscp2color, sizeof(p_db_qos_domain->dscp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_DSCP2PHB:
        sal_memcpy(p_domain->dscp2phb, p_db_qos_domain->dscp2phb, sizeof(p_db_qos_domain->dscp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PRI:
        sal_memcpy(p_domain->prec2pri, p_db_qos_domain->prec2pri, sizeof(p_db_qos_domain->prec2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2COLOR:
        sal_memcpy(p_domain->prec2color, p_db_qos_domain->prec2color, sizeof(p_db_qos_domain->prec2color));
        break;

    case TBL_QOS_DOMAIN_FLD_PREC2PHB:
        sal_memcpy(p_domain->prec2phb, p_db_qos_domain->prec2phb, sizeof(p_db_qos_domain->prec2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PRI:
        sal_memcpy(p_domain->exp2pri, p_db_qos_domain->exp2pri, sizeof(p_db_qos_domain->exp2pri));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2COLOR:
        sal_memcpy(p_domain->exp2color, p_db_qos_domain->exp2color, sizeof(p_db_qos_domain->exp2color));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2PHB:
        sal_memcpy(p_domain->exp2phb, p_db_qos_domain->exp2phb, sizeof(p_db_qos_domain->exp2phb));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2COS:
        sal_memcpy(p_domain->pri_color2cos, p_db_qos_domain->pri_color2cos, sizeof(p_db_qos_domain->pri_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2DSCP:
        sal_memcpy(p_domain->pri_color2dscp, p_db_qos_domain->pri_color2dscp, sizeof(p_db_qos_domain->pri_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PRI_COLOR2EXP:
        sal_memcpy(p_domain->pri_color2exp, p_db_qos_domain->pri_color2exp, sizeof(p_db_qos_domain->pri_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2COS:
        sal_memcpy(p_domain->phb2cos, p_db_qos_domain->phb2cos, sizeof(p_db_qos_domain->phb2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2DSCP:
        sal_memcpy(p_domain->phb2dscp, p_db_qos_domain->phb2dscp, sizeof(p_db_qos_domain->phb2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_PHB2EXP:
        sal_memcpy(p_domain->phb2exp, p_db_qos_domain->phb2exp, sizeof(p_db_qos_domain->phb2exp));
        break;
 
    case TBL_QOS_DOMAIN_FLD_COS2TC:
        sal_memcpy(p_domain->cos2tc, p_db_qos_domain->cos2tc, sizeof(p_db_qos_domain->cos2tc));
        break;
        
    case TBL_QOS_DOMAIN_FLD_DSCP2TC:
        sal_memcpy(p_domain->dscp2tc, p_db_qos_domain->dscp2tc, sizeof(p_db_qos_domain->dscp2tc));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2COS:
        sal_memcpy(p_domain->tc_color2cos, p_db_qos_domain->tc_color2cos, sizeof(p_db_qos_domain->tc_color2cos));
        break;

    case TBL_QOS_DOMAIN_FLD_TC_COLOR2DSCP:
        sal_memcpy(p_domain->tc_color2dscp, p_db_qos_domain->tc_color2dscp, sizeof(p_db_qos_domain->tc_color2dscp));
        break;

    case TBL_QOS_DOMAIN_FLD_EXP2TC:
        sal_memcpy(p_domain->exp2tc, p_db_qos_domain->exp2tc, sizeof(p_db_qos_domain->exp2tc));
        break;
        
    case TBL_QOS_DOMAIN_FLD_TC_COLOR2EXP:
        sal_memcpy(p_domain->tc_color2exp, p_db_qos_domain->tc_color2exp, sizeof(p_db_qos_domain->tc_color2exp));
        break;

    case TBL_QOS_DOMAIN_FLD_MAX:
        sal_memcpy(p_domain, p_db_qos_domain, sizeof(tbl_qos_domain_t));
        break;

    default:
        return PM_E_NOT_SUPPORT;
    }

    return PM_E_NONE;
}

tbl_qos_domain_t*
tbl_qos_domain_get_qos_domain_by_id(uint8 domain_id)
{
    tbl_qos_domain_key_t key;
    key.id = domain_id;

    if (domain_id == GLB_QOS_DOMAIN_NUM)
    {
        return _g_p_tbl_qos_domain_master->default_domain;
    }
    else
    {
        return tbl_qos_domain_get_qos_domain(&key);
    }
}
