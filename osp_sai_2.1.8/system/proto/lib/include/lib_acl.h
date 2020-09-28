
#ifndef __LIB_ACL_H__
#define __LIB_ACL_H__

char *
acl_generate_acl_config_key_name_by_type(char *acl_name, int type, char *outkey_name);

char *
acl_get_acl_config_key_name(char *acl_name, int type);

int32
acl_get_acl_config_type_by_name(char *type_name);

#endif /* !__LIB_ACL_H__ */

