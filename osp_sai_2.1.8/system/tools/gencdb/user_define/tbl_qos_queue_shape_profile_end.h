tbl_qos_queue_shape_profile_t*
tbl_qos_queue_shape_profile_get_profile_by_name(char *name);

int32
tbl_qos_queue_shape_profile_get_field(char *name, tbl_qos_queue_shape_profile_field_id_t field_id, void* field_value);

int32
tbl_qos_queue_shape_profile_alloc_profile_id(uint32 *p_id);
