#ifndef __FEI_OPENFLOW_GROUP_H__
#define __FEI_OPENFLOW_GROUP_H__

/**
 * @brief Adapter layer group add group
 */
int32
fei_group_add_group(glb_openflow_group_t* p_group);


/**
 * @brief Adapter layer group delete group
 */
int32
fei_group_del_group(uint32 group_id);

/**
 * @brief Adapter layer group clear stats
 */
int32
fei_group_clear_stats(uint32 group_id);

/**
 * @brief Adapter layer group set drop ingress, Added by weizj for bug 35135
 */
int32
fei_group_set_drop_ingress(uint8 is_group, uint8 is_drop);

#endif /* !__FEI_OPENFLOW_GROUP_H__ */
