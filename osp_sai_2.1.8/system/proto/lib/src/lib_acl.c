#include "proto.h"
#include "glb_tap_define.h"
#include "glb_acl_define.h"

char *
acl_generate_acl_config_key_name_by_type(char *acl_name, int type, char *outkey_name)
{
    char _acl_name[ACL_NAME_FULL_NAME_SIZE] = "";

    sal_assert(outkey_name != NULL);

    switch(type)
    {
        default:
            sal_strcpy(_acl_name, acl_name);
            break;

        case GLB_ACL_CONFIG_TYPE_FLOW:
            sal_strcpy(_acl_name, TAP_ACL_CONFIG_PREFIX);
            sal_strcat(_acl_name, acl_name);
            break;
        case GLB_ACL_CONFIG_TYPE_FLOW_MATCH:
            sal_strcpy(_acl_name, "#FW_MAT#");
            sal_strcat(_acl_name, acl_name);
            break;
    }

    sal_strncpy(outkey_name, _acl_name,sal_strlen(_acl_name));
    return outkey_name;
}

int32
acl_get_acl_config_type_by_name(char *type_name)
{
    int idx = 0;
    struct {
        char *  type_name;
        int32   type;
    } type_name_key[] = {
        { "ipv4",       GLB_ACL_CONFIG_TYPE_NORMAL },
        { "flow",       GLB_ACL_CONFIG_TYPE_FLOW },
        { "flow_match", GLB_ACL_CONFIG_TYPE_FLOW_MATCH },
        { "ipv6", GLB_ACL_CONFIG_TYPE_IPV6 },
        { "mac", GLB_ACL_CONFIG_TYPE_MAC },
        { "mpls", GLB_ACL_CONFIG_TYPE_MPLS },
        { NULL,         GLB_ACL_CONFIG_TYPE_MAX },
    };

    while (type_name_key[idx].type_name != NULL)
    {
        if (sal_strcmp(type_name, type_name_key[idx].type_name) == 0)
        {
            return type_name_key[idx].type;
        }

        idx ++;
    }

    return GLB_ACL_CONFIG_TYPE_NORMAL;
}

char *
acl_get_acl_config_key_name(char *acl_name, int type)
{
    char *p_acl_name = NULL;

    sal_assert(acl_name != NULL);

    switch(type)
    {
        default:
            p_acl_name = acl_name; 
            break;

        case GLB_ACL_CONFIG_TYPE_FLOW:
            p_acl_name = acl_name + TAP_ACL_CONFIG_PREFIX_LEN;
            break;
        case GLB_ACL_CONFIG_TYPE_FLOW_MATCH:
            p_acl_name = acl_name +sal_strlen("#FW_MAT#");
            break;
    }

    return p_acl_name;
}

