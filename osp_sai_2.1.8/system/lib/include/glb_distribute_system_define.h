#ifndef __GLB_DISTRIBUTE_SYSTEM_H__
#define __GLB_DISTRIBUTE_SYSTEM_H__

#define GLB_SLOT_NO_FILE SWITCH_PREFIX"/etc/slot_no"
#define GLB_STACK_ENABLE_FILE "/tmp/stack_enable"
#define GLB_STACK_MASTER_SLOT_FILE "/tmp/stack_master_slot"
enum glb_ha_type_e
{
    GLB_HA_TYPE_RPR,
    GLB_HA_TYPE_SSO,
    GLB_HA_TYPE_MAX
};

#endif /* !__GLB_DISTRIBUTE_SYSTEM_H__*/
