import os
import sys
import string
import CONST
import cdb_util

def gen_ds_func_add_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, cmp_func_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	funcstr = "%s_add_%s(%s %s, %s%s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_add_%s_sync(%s, %s%s, TRUE);" % (ds_name, accusative, tbl_value_name, parent_ds_param_value_str, ds_value_name)
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_del_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_dbvalue = 'p_db_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (len(key_array)):
		haskey_get_ds_param = ', ' + ds_value_name
	else:
		haskey_get_ds_param = ''
	if (len(key_array)):
		funcstr = "%s_del_%s(%s %s, %s%s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' +  ds_value_name)
	else:
		funcstr = "%s_del_%s(%s %s%s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if (len(key_array)):
		print >> fobj, "    return %s_del_%s_sync(%s, %s%s, TRUE);" % (ds_name, accusative, tbl_value_name, parent_ds_param_value_str, ds_value_name)
	else:
		print >> fobj, "    return %s_del_%s_sync(%s, %sTRUE);" % (ds_name, accusative, tbl_value_name, parent_ds_param_value_str)
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_set_field_implement(fobj, ds, key_array, field_array, parent_table, parent_ds, ds_field_name):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	dbvalue = 'p_db_' + ds[1]
	ds_field_type = ds_name + '_field_id_t'
	accusative = ds_name[3:]
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (len(key_array)):
		haskey_get_ds_param = ', ' + ds_value_name
	else:
		haskey_get_ds_param = ''
	funcstr = "%s_set_%s_field(%s %s, %s%s %s, %s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name, ds_field_type, 'field_id')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_set_%s_field_sync(%s, %s%s, %s, TRUE);" % (ds_name, accusative, tbl_value_name, parent_ds_param_value_str, ds_value_name, 'field_id')
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_add_sync_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, cmp_func_array, is_parent_global):
	tbl_name_upper = parent_table[0]
	ds_name_upper = ds[0]
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_dbvalue = 'p_db_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_dbvalue = 'p_db_' + ds[1]
	accusative = ds_name[3:]
	tbl_accusative = tbl_name[4:]
	parent_db_value = tbl_dbvalue
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (None != parent_ds):
		parent_ds_name_upper = parent_ds[0][0]
		parent_ds_name = string.lower(parent_ds[0][0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value = 'p_' + parent_ds[0][1]
		parent_ds_db_value = 'p_db_' + parent_ds[0][1]
		parent_ds_accusative = parent_ds_name[3:]
		parent_db_value = parent_ds_db_value
	if (len(key_array)):
		haskey_get_ds_param = ', ' + parent_ds_param_value_str + ds_value_name
	else:
		haskey_get_ds_param = ''

	funcstr = "%s_add_%s_sync(%s %s, %s%s %s, uint32 sync)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, tbl_dbvalue)
	if (None != parent_ds):
		print >> fobj, "    %s *%s = NULL;" % (parent_ds_type, parent_ds_db_value)
	print >> fobj, "    %s *%s = NULL;" % (ds_type, ds_dbvalue)
	referlist_array = cdb_util.has_cdb_refer_list(data_array)
	if (len(referlist_array) or len(key_array)):
		print >> fobj, "    int32 rc = PM_E_NONE;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	if (is_parent_global):
		print >> fobj, "    %s = %s_get_%s();" % (tbl_dbvalue, tbl_name, tbl_accusative)
	else:
		print >> fobj, "    %s = %s_get_%s(&%s->key);" % (tbl_dbvalue, tbl_name, tbl_accusative, tbl_value_name)
	print >> fobj, "    if (NULL == %s)" % (tbl_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"

	if (None != parent_ds):
		print >> fobj, "    %s = %s_get_%s(%s, %s);" % (parent_ds_db_value, parent_ds_name, parent_ds_accusative, tbl_value_name, parent_ds_value)
		print >> fobj, "    if (NULL == %s)" % (parent_ds_db_value)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NOT_EXIST;"
		print >> fobj, "    }"

	if (len(key_array)):
		print >> fobj, "    %s = %s_get_%s(%s%s);" % (ds_dbvalue, ds_name, accusative, tbl_value_name, haskey_get_ds_param)
	else:
		print >> fobj, "    %s = %s->%s;" % (ds_dbvalue, tbl_dbvalue, ds_field_name)
	print >> fobj, "    if (%s)" % (ds_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. alloc a new db entry */"
	print >> fobj, "    %s = XCALLOC(%s, sizeof(%s));" % (ds_dbvalue, ds_mem_type, ds_type)
	print >> fobj, "    if (NULL == %s)" % (ds_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NO_MEMORY;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 3. evaluate db entry */"
	print >> fobj, "    sal_memcpy(%s, %s, sizeof(%s));" % (ds_dbvalue, ds_value_name, ds_type)
	for i, referlist_field in enumerate(referlist_array):
		object_info = cdb_util.get_refer_object_info(referlist_field)
		cmp_func = string.lower(object_info[2]) + '_cmp'
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_init(&%s->%s, %s, %s, %s));" % (ds_dbvalue, object_info[0], object_info[1], object_info[2], cmp_func)
		cmp_func_array.append(cmp_func)
	print >> fobj, ""
	print >> fobj, "    /* 4. add to db */"
	if (len(key_array)):
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_add_object(&%s->%s, %s));" % (parent_db_value, ds_field_name, ds_dbvalue)
	else:
		print >> fobj, "    %s->%s = %s;" % (tbl_dbvalue, ds_field_name, ds_dbvalue)

	print >> fobj, ""
	print >> fobj, "    /* 5. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	if (None != parent_ds):
		print >> fobj, "        cdb_sync_ds2_add(%s, %s, %s, %s, %s, %s);" % (tbl_name_upper, parent_ds_name_upper, ds_name_upper, tbl_dbvalue, parent_ds_db_value, ds_dbvalue)
	else:
		print >> fobj, "        cdb_sync_ds_add(%s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, tbl_dbvalue, ds_dbvalue)
		print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
		print >> fobj, "        cdb_sync_publish_ds_add(%s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, tbl_dbvalue, ds_dbvalue)
		print >> fobj, "        #endif "
	print >> fobj, "    }"

	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_del_sync_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, is_parent_global):
	tbl_name_upper = parent_table[0]
	ds_name_upper = ds[0]
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_dbvalue = 'p_db_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_dbvalue = 'p_db_' + ds[1]
	accusative = ds_name[3:]
	tbl_accusative = tbl_name[4:]
	has_referlist = 0
	parent_db_value = tbl_dbvalue
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (None != parent_ds):
		parent_ds_name_upper = parent_ds[0][0]
		parent_ds_name = string.lower(parent_ds[0][0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value = 'p_' + parent_ds[0][1]
		parent_ds_db_value = 'p_db_' + parent_ds[0][1]
		parent_ds_accusative = parent_ds_name[3:]
		parent_db_value = parent_ds_db_value
	if (len(key_array)):
		haskey_get_ds_param = ', ' + parent_ds_param_value_str + ds_value_name
	else:
		haskey_get_ds_param = ''
	if (len(key_array)):
		funcstr = "%s_del_%s_sync(%s %s, %s%s %s, uint32 sync)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' +  ds_value_name)
	else:
		funcstr = "%s_del_%s_sync(%s %s, %suint32 sync)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, tbl_dbvalue)
	if (None != parent_ds):
		print >> fobj, "    %s *%s = NULL;" % (parent_ds_type, parent_ds_db_value)
	print >> fobj, "    %s *%s = NULL;" % (ds_type, ds_dbvalue)
	referlist_array = cdb_util.has_cdb_refer_list(data_array)
	if (len(referlist_array) or len(key_array)):
		print >> fobj, "    int32 rc = PM_E_NONE;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	if (is_parent_global):
		print >> fobj, "    %s = %s_get_%s();" % (tbl_dbvalue, tbl_name, tbl_accusative)
	else:
		print >> fobj, "    %s = %s_get_%s(&%s->key);" % (tbl_dbvalue, tbl_name, tbl_accusative, tbl_value_name)
	print >> fobj, "    if (NULL == %s)" % (tbl_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"

	if (None != parent_ds):
		print >> fobj, "    %s = %s_get_%s(%s, %s);" % (parent_ds_db_value, parent_ds_name, parent_ds_accusative, tbl_value_name, parent_ds_value)
		print >> fobj, "    if (NULL == %s)" % (parent_ds_db_value)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NOT_EXIST;"
		print >> fobj, "    }"

	if (len(key_array)):
		print >> fobj, "    %s = %s_get_%s(%s%s);" % (ds_dbvalue, ds_name, accusative, tbl_value_name, haskey_get_ds_param)
	else:
		print >> fobj, "    %s = %s->%s;" % (ds_dbvalue, tbl_dbvalue, ds_field_name)
	print >> fobj, "    if (NULL == %s)" % (ds_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	if (None != parent_ds):
		print >> fobj, "        cdb_sync_ds2_del(%s, %s, %s, %s, %s, %s);" % (tbl_name_upper, parent_ds_name_upper, ds_name_upper, tbl_dbvalue, parent_ds_db_value, ds_dbvalue)
	else:
		print >> fobj, "        cdb_sync_ds_del(%s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, tbl_dbvalue, ds_dbvalue)
		print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
		print >> fobj, "        cdb_sync_publish_ds_del(%s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, tbl_dbvalue, ds_dbvalue)
		print >> fobj, "        #endif "
	print >> fobj, "    }"
	print >> fobj, ""
	for i, referlist_field in enumerate(referlist_array):
		field_name = cdb_util.format_field_name(referlist_field[1])
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_deinit(&%s->%s));" % (ds_dbvalue, field_name)
		has_referlist = 1
	if (has_referlist):
		print >> fobj, ""
	print >> fobj, "    /* 3. delete from db */"
	if (len(key_array)):
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_del_object(&%s->%s, %s));" % (parent_db_value, ds_field_name, ds_dbvalue)
	else:
		print >> fobj, "    %s->%s = NULL;" % (tbl_dbvalue, ds_field_name)
	print >> fobj, ""
	print >> fobj, "    /* 4. free db entry */"
	print >> fobj, "    XFREE(%s, %s);" % (ds_mem_type, ds_dbvalue)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_set_implement(fobj, ds, key_array, parent_table, ds_field_name):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_dbvalue = 'p_db_' + ds[1]
	accusative = ds_name[3:]
	if (len(key_array)):
		haskey_get_ds_param = ', ' + ds_value_name
	else:
		haskey_get_ds_param = ''
	funcstr = "%s_set_%s(%s %s, %s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = NULL;" % (ds_type, ds_dbvalue)
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	print >> fobj, "    %s = %s_get_%s(%s%s);" % (ds_dbvalue, ds_name, accusative, tbl_value_name, haskey_get_ds_param)
	print >> fobj, "    if (NULL == %s)" % (ds_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. update db entry */"
	print >> fobj, "    sal_memcpy(%s, %s, sizeof(%s));" % (ds_dbvalue, ds_value_name, ds_type)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_set_field_evaluate(fobj, ds_name, dbvalue, ds_value_name, field, data_array):
	field_len = len(field)
	dt_type = field[field_len - 1]
	paramvalue = ds_value_name
	# sub DS_ need not set field
	dt_info = dt_type[2]
	if ('DS_' in dt_type[1]):
		return
	if ('cdb_reference_list_t' == dt_type[1]):
		return
	if ('key' == field[1]):
		return

	ds_name_upper = string.upper(ds_name)
	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])	
	field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)
	db_field_value = dbvalue + '->' + field_name
	param_field_value = ds_value_name + '->' + field_name
	
	print >> fobj, "    case %s:" % (field_id_enum)
	if ('UINT' == dt_type[1] or 'UINT64' == dt_type[1] or 'INT' == dt_type[1] or 'INT64' == dt_type[1]):
		print >> fobj, "        %s->%s = %s->%s;" % (dbvalue, field_name, paramvalue, field_name)
	elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
		print >> fobj, "        %s->%s = %s->%s;" % (dbvalue, field_name, paramvalue, field_name)
	elif ('STRING' == dt_type[1]):
		print >> fobj, "        sal_memcpy(%s, %s, sizeof(%s));" % (db_field_value, param_field_value, param_field_value)
	elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		data_value = field[1]
		field_value = field[3]
		flag_value = field[4]
		print >> fobj, "        CDB_FLAG_CHECK_SET(%s, %s, %s, %s)" % (dbvalue, paramvalue, data_value, flag_value)
	elif ('VOID' == dt_type[1]):
		print >> fobj, "        %s->%s = %s->%s;" % (dbvalue, field_name, paramvalue, field_name)
	elif ('TASK' == dt_type[1]):
		print >> fobj, "        %s->%s = %s->%s;" % (dbvalue, field_name, paramvalue, field_name)
	elif ('DOUBLE' == dt_type[1]):
		print >> fobj, "        %s->%s = %s->%s;" % (dbvalue, field_name, paramvalue, field_name)
	else:
		if ('memcpy' == dt_info[1]):
			if (6 == len(dt_info) and '&' == dt_info[5]):
				print >> fobj, "        sal_memcpy(&%s, &%s, sizeof(%s));" % (db_field_value, param_field_value, param_field_value)
			else:
				print >> fobj, "        sal_memcpy(%s, %s, sizeof(%s));" % (db_field_value, param_field_value, param_field_value)
		else:
			cdb_util.error_return("unhandled field in DS set_field: %s, dt_type[1]: %s", field, dt_type[1])
	print >> fobj, "        break;"
	print >> fobj, ""

def gen_ds_func_set_field_sync_implement(fobj, ds, key_array, field_array, parent_table, parent_ds, ds_field_name, is_parent_global):
	tbl_name_upper = parent_table[0]
	ds_name_upper = ds[0]
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_dbvalue = 'p_db_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	dbvalue = 'p_db_' + ds[1]
	ds_field_type = ds_name + '_field_id_t'
	tbl_accusative = tbl_name[4:]
	accusative = ds_name[3:]
	parent_db_value = tbl_dbvalue
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (None != parent_ds):
		parent_ds_name_upper = parent_ds[0][0]
		parent_ds_name = string.lower(parent_ds[0][0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value = 'p_' + parent_ds[0][1]
		parent_ds_db_value = 'p_db_' + parent_ds[0][1]
		parent_ds_accusative = parent_ds_name[3:]
		parent_db_value = parent_ds_db_value
	if (len(key_array)):
		haskey_get_ds_param = ', ' + parent_ds_param_value_str + ds_value_name
	else:
		haskey_get_ds_param = ''
	funcstr = "%s_set_%s_field_sync(%s %s, %s%s %s, %s %s, uint32 sync)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name, ds_field_type, 'field_id')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, tbl_dbvalue)
	if (None != parent_ds):
		print >> fobj, "    %s *%s = NULL;" % (parent_ds_type, parent_ds_db_value)
	print >> fobj, "    %s *%s = NULL;" % (ds_type, dbvalue)
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	if (is_parent_global):
		print >> fobj, "    %s = %s_get_%s();" % (tbl_dbvalue, tbl_name, tbl_accusative)
	else:
		print >> fobj, "    %s = %s_get_%s(&%s->key);" % (tbl_dbvalue, tbl_name, tbl_accusative, tbl_value_name)
	print >> fobj, "    if (NULL == %s)" % (tbl_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"
	if (None != parent_ds):
		print >> fobj, "    %s = %s_get_%s(%s, %s);" % (parent_ds_db_value, parent_ds_name, parent_ds_accusative, tbl_value_name, parent_ds_value)
		print >> fobj, "    if (NULL == %s)" % (parent_ds_db_value)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NOT_EXIST;"
		print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    %s = %s_get_%s(%s%s);" % (dbvalue, ds_name, accusative, tbl_value_name, haskey_get_ds_param)
	print >> fobj, "    if (NULL == %s)" % (dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. update db entry field */"
	print >> fobj, "    switch (field_id)"
	print >> fobj, "    {"
	for i, field in enumerate(field_array):
		gen_tbl_func_set_field_evaluate(fobj, ds_name, dbvalue, ds_value_name, field, field_array)
	print >> fobj, "    case %s:" % (ds_name_upper + '_FLD_MAX')
	print >> fobj, "        sal_memcpy(%s, %s, sizeof(%s));" % (dbvalue, ds_value_name, ds_type)
	print >> fobj, "        break;"
	print >> fobj, ""
	print >> fobj, "    default:"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 3. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	if (None != parent_ds):
		print >> fobj, "        cdb_sync_ds2_set(%s, %s, %s, %s, %s, %s, %s);" % (tbl_name_upper, parent_ds_name_upper, ds_name_upper, 'field_id', tbl_dbvalue, parent_ds_db_value, dbvalue)
	else:
		print >> fobj, "        cdb_sync_ds_set(%s, %s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, 'field_id', tbl_dbvalue, dbvalue)
		print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
		print >> fobj, "        cdb_sync_publish_ds_set(%s, %s, %s, %s, %s);" % (tbl_name_upper, ds_name_upper, 'field_id', tbl_dbvalue, dbvalue)
		print >> fobj, "        #endif "
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_get_implement(fobj, ds, key_array, parent_tbl, parent_ds, ds_field_name, is_array_ds, is_parent_global):
	parent_table = parent_tbl[0]
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_dbvalue = 'p_db_' + parent_table[1]
	tbl_accusative = tbl_name[4:]

	if (None == parent_ds):
		data_name = tbl_name
		data_type = tbl_type
		data_dbvalue = tbl_dbvalue
		data_accusative = tbl_accusative
	else:
		parent_ds_name = string.lower(parent_ds[0][0])
		ds_accusative = parent_ds_name[3:]
		data_name = parent_ds_name
		data_type = parent_ds_name + '_t'
		data_dbvalue = 'p_db_' + parent_ds[0][1]
		data_accusative = ds_accusative
		parent_ds_value_name = 'p_' + parent_ds[0][1]

	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_dbvalue = 'p_db_' + ds[1]
	accusative = ds_name[3:]
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)

	if (len(key_array)):
		funcstr = "%s_get_%s(%s %s, %s%s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' +  ds_value_name)
	else:
		funcstr = "%s_get_%s(%s %s%s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str)
	print >> fobj, "%s*" % (ds_type)
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = NULL;" % (data_type, data_dbvalue)
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	if (None == parent_ds):
		if (is_parent_global):
			print >> fobj, "    %s = %s_get_%s();" % (data_dbvalue, data_name, data_accusative)
		else:
			print >> fobj, "    %s = %s_get_%s(&%s->key);" % (data_dbvalue, data_name, data_accusative, tbl_value_name)
	else:
		print >> fobj, "    %s = %s_get_%s(%s, %s);" % (data_dbvalue, data_name, data_accusative, tbl_value_name, parent_ds_value_name)
	print >> fobj, "    if (NULL == %s)" % (data_dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""

	if (is_array_ds):
		array_ds_array = cdb_util.has_cdb_array_ds(parent_tbl[2])
		get_parent_info = 0
		check_array_ds = 0
		for i, array_ds in enumerate(array_ds_array):
			if (string.lower(array_ds[0][6:]) == ds_name):
				params = array_ds[1][:-1]
				params_array = params.split('[')
				get_parent_info = 1
		if (get_parent_info == 1):
			if (1 == len(key_array)):
				key_item = key_array[0]
				if (key_item[1] == 'index'):
					check_array_ds = 1
			if (0 == check_array_ds):
				cdb_util.error_return("Array DS %s should has one key index", ds[0])
			print >> fobj, "    if (%s->key.index >= %s)" % (ds_value_name, params_array[1])
			print >> fobj, "    {"
			print >> fobj, "        return NULL;"
			print >> fobj, "    }"
			print >> fobj, "    return &(%s->%s[%s->key.index]);" % (data_dbvalue, ds_field_name, ds_value_name)
		else:
			cdb_util.error_return("Array DS %s Not found array field in parent %s", ds[0], parent_table)
	elif (len(key_array)):
		print >> fobj, "    return cdb_reference_list_lookup_object(&%s->%s, %s);" % (data_dbvalue, ds_field_name, ds_value_name)
	else:
		print >> fobj, "    return %s->%s;" % (tbl_dbvalue, ds_field_name)
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_dump_one_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array, val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	paramtype = ds_type
	node_value = 'p_node'
	field_value = 'p_field'
	bak_field_id = 'bak_field_id'
	listnode = 'listnode'
	accusative = ds_name[3:]
	pargs_value = 'pargs'
	fp_value = 'fp'
	tbl_info_value = 'p_tbl_info'
	need_listnode = 0
	refer_object_array = []
	funcstr = "%s_dump_one(%s %s, %s %s)" % (ds_name, ds_type, '*' +  ds_value_name, 'tbl_iter_args_t', '*' + pargs_value)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    cdb_node_t *%s = cdb_get_ds(%s);" % (node_value, ds_name_upper)
	print >> fobj, "    field_parse_t *%s = %s->argv[0];" % (field_value, pargs_value)
	print >> fobj, "    FILE *%s = %s->argv[1];" % (fp_value, pargs_value)
	print >> fobj, "    cdb_tbl_info_t *%s = %s->tbl_info;" % (tbl_info_value, node_value)
	print >> fobj, ""

	has_buf1 = cdb_util.check_dump_one_use_buf(ds, key_array, field_array, dt_array, 1)
	has_buf2 = 0

	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	for i, referlist_field in enumerate(referlist_array):
		refer_object_info = cdb_util.get_refer_object_info(referlist_field)
		need_bak_field_id = 0
		if ('CDB_NODE_DS' == refer_object_info[1]):
			refer_object_array.append(string.lower(refer_object_info[2]))
		elif ('CDB_NODE_TBL' == refer_object_info[1]):
			has_buf2 = 1

	refer_object_array = cdb_util.array_remove_duplicate_entry(refer_object_array)
	for i, refer_object in enumerate(refer_object_array):
		refer_object_type = refer_object + '_t'
		refer_object_param = 'p_' + 'db_' + refer_object[3:]
		print >> fobj, "    %s *%s = NULL;" % (refer_object_type, refer_object_param)
		need_listnode = 1

	if (need_listnode):
		print >> fobj, "    ctclib_slistnode_t *%s = NULL;" % listnode
		print >> fobj, ""

	if (has_buf1 or has_buf2):
		print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
		print >> fobj, ""

	for i, field in enumerate(field_array):
		field_len = len(field)
		dt_type = field[field_len-1]
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)
		print >> fobj, "    if (FLD_MATCH(%s, %s->field_id[1]))" % (field_id_enum, field_value)
		print >> fobj, "    {"

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = ds_value_name + '->' + key_str + '.' + data_name
			else:
				param_str = ds_value_name + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = ds_value_name + '->' + data_name
		
		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		field_name_string = """/%s"""
		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			if ('cdb_bitmap_val2str' == func_val2str or 'cdb_int32_array_val2str' == func_val2str or 'cdb_uint32_array_val2str' == func_val2str or 'cdb_int8_array_val2str' == func_val2str or 'cdb_uint8_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)/4));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
 			elif ('cdb_int8_array_val2str' == func_val2str or 'cdb_uint8_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_double_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)/sizeof(double)));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_string_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, (const char*)%s," % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
				print >> fobj, "            sizeof(%s[0]), sizeof(%s)/sizeof(%s[0])));" % (param_str, param_str, param_str)
			else:
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
		elif ('UINT' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%u\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('UINT64' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%\"PRIu64,"
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('INT' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%d\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('INT64' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%\"PRId64,"
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            cdb_enum_val2str(%s, %s, %s));" % (enum_type_str, enum_type_max, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%u\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            GLB_FLAG_ISSET(%s, %s));" % (param_str, field[4])
		elif ('cdb_reference_list_t' in dt_type[1]):
			if ('REFERLIST_DS_' in field[0]):
				referlist_field = cdb_util.get_cdb_referlist_field(referlist_array, field[0])
				if (None == referlist_field):
					cdb_util.error_return("Not found %s in refer list array %s", field[0], referlist_array)

				refer_object_info = cdb_util.get_refer_object_info(referlist_field)
				refer_object_type = string.lower(refer_object_info[2]) + '_t'
				name_upper = refer_object_info[2]
				name_lower = string.lower(name_upper)
				sub_field_id_enum = 'FLD_ID_ALL'
				object_list = param_str + '.obj_list'
				refer_object_param = 'p_' + 'db_' + string.lower(refer_object_info[2])[3:]
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name);" % (field_name_string, '{', tbl_info_value, field_id_enum)
				print >> fobj, "        if (%s)" % (object_list)
				print >> fobj, "        {"
				print >> fobj, "            p_field->field_id[1] = %s;" % (sub_field_id_enum)
				print >> fobj, "            CTCLIB_SLIST_LOOP(%s, %s, %s)" % (object_list, refer_object_param, listnode)
				print >> fobj, "            {"
				print >> fobj, "                %s_dump_one(%s, %s);" % (name_lower, refer_object_param, pargs_value)
				print >> fobj, "                sal_fprintf(fp, \"/,\");"
				print >> fobj, "            }"
				print >> fobj, "        }"
				print >> fobj, "        sal_fprintf(fp, \"%s\");" % ('}')

			elif ('REFERLIST_TBL_' in field[0]):
				referlist_field = cdb_util.get_cdb_referlist_field(referlist_array, field[0])
				if (None == referlist_field):
					cdb_util.error_return("Not found %s in refer list array %s", field[0], referlist_array)
				refer_object_info = cdb_util.get_refer_object_info(referlist_field)
				name_upper = refer_object_info[2]
				referlist_tbl_array.append(name_upper)
				param_str = '&' + param_str
				func_val2str = 'cdb_reference_list_val2str'
				val2str_func_array.append(func_val2str)
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
			else:
				cdb_util.error_return("Invalid field type %s for dt_type %s", field[0], dt_type[1])
		elif ('VOID' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"\","
			print >> fobj, "            %s->field[%s].name);" % (tbl_info_value, field_id_enum)
		elif ('TASK' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            ctc_task_val2str(buf, MAX_CMD_STR_LEN, %s));" % (param_str)
		elif ('DOUBLE' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%f\","
			print >> fobj, "            %s->field[%s].name," % (tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		else:
			cdb_util.error_return("unhandled field in DS dump_one: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"

	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"


def gen_ds_func_key_str2val_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	paramtype = ds_type
	node_value = 'p_node'
	field_value = 'p_field'
	bak_field_id = 'bak_field_id'
	accusative = ds_name[3:]
	tbl_info_value = 'p_tbl_info'
	need_listnode = 0
	refer_object_array = []
	have_key = 0
	funcstr = "%s_key_str2val(char* tbl_key_value, char* str, %s %s, %s %s)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""
	print >> fobj, "    int32 ret = 0;"


	for i, field in enumerate(field_array):
		if ('key' == field[1]):
			have_key = 1
			break

	print >> fobj, ""
	print >> fobj, "    ret = %s_key_str2val(tbl_key_value, %s);" % (tbl_name, tbl_value_name)

	if not have_key:
		print >> fobj, "    return ret;"
		print >> fobj, "}"
		return

	for i, field in enumerate(field_array):
		field_len = len(field)
		dt_type = field[field_len - 1]
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = ds_value_name + '->' + key_str + '.' + data_name
			else:
				param_str = ds_value_name + '->' + key_str
		else:
			break

		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			func_str2val = string.strip(func_val2str, "val2str")
			func_str2val = func_str2val + "str2val"
			val2str_func_array.append(func_val2str)
			print >> fobj, ""
			if ('cdb_bitmap_val2str' == func_val2str):
				print >> fobj, "    char err[MAX_CMD_STR_LEN];"
				print >> fobj, "    ret = %s(str, 0, GLB_UINT16_BMP_WORD_MAX, %s, sizeof(%s)/4, err);" % (
				func_str2val, param_str, param_str)
			elif ('cdb_addr_val2str' == func_val2str):
				print >> fobj, "    ret = %s(%s, str, 0);" % (func_str2val, param_str)
			elif ('cdb_addr_ipv4_val2str' == func_val2str):
				print >> fobj, "    ret = %s(%s, str, 0);" % (func_str2val, param_str)
			elif ('cdb_prefix_val2str' == func_val2str):
				print >> fobj, "    ret = cdb_prefix_str2val_wo_masklen(str, %s);" % (param_str)
			elif ('cdb_prefix_ipv4_val2str' == func_val2str):
				print >> fobj, "    ret = cdb_prefix_ipv4_str2val_wo_masklen(str, %s);" % (param_str)
			elif ('cdb_fdb_key_val2str' == func_val2str):
				print >> fobj, "    char err[MAX_CMD_STR_LEN];"
				print >> fobj, "    ret = %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_mac_addr_val2str' == func_val2str):
				print >> fobj, "    ret = %s(%s, str);" % (func_str2val, param_str)
			elif ('cdb_auth_server_key_val2str' == func_val2str or
						  'cdb_ns_port_forwarding_key_val2str' == func_val2str or
						  'cdb_user_val2str' == func_val2str or
						  'cdb_binding_key_str2val' == func_str2val or
						  'cdb_auth_session_key_str2val' == func_str2val):
				print >> fobj, "    char err[MAX_CMD_STR_LEN];"
				print >> fobj, "    ret = %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_snmp_info_val2str' == func_val2str):
				print >> fobj, "    char err[MAX_CMD_STR_LEN];"
				print >> fobj, "    ret = %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_mstp_digest_val2str' == func_val2str):
				print >> fobj, "    ret = %s(str, %s);" % (func_str2val, param_str)
			elif ('cdb_string_array_str2val' == func_str2val):
				print >> fobj, "    ret = %s(str, &%s);" % (func_str2val, param_str)
			else:
				print func_val2str
				print >> fobj, "    ret = %s(str, %s);" % (func_str2val, param_str)

		elif ('UINT' == dt_type[1]):
			print >> fobj, ""
			print >> fobj, "    %s = cdb_uint_str2val(str, &ret);" % (param_str)
		elif ('UINT64' == dt_type[1]):
			print >> fobj, ""
			print >> fobj, "    %s = cdb_uint64_str2val(str, &ret);" % (param_str)
		elif ('INT' == dt_type[1]):
			print >> fobj, ""
			print >> fobj, "    %s = cdb_int_str2val(str, &ret);" % (param_str)
		elif ('INT64' == dt_type[1]):
			print >> fobj, ""
			print >> fobj, "    %s = cdb_int64_str2val(str, &ret);" % (param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, ""
			print >> fobj, "    sal_strcpy(%s, str);" % (param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "    ret =  cdb_enum_str2val(%s, %s, str);" % (enum_type_str, enum_type_max)
		else:
			cdb_util.error_return("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])

	print >> fobj, ""
	print >> fobj, "    return ret;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_key_value_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_name_upper = parent_table[0]
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	have_key = 0

	funcstr = "%s_key_value_dump(%s %s, %s %s, char* str)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""

	for i, field in enumerate(field_array):
		if ('key' == field[1]):
			have_key = 1
			break
	if not have_key:
		print >> fobj, "    sal_sprintf(str, \"%s\","
		print >> fobj, "        %s_key_value_dump(%s, str));" % (tbl_name, tbl_value_name)
		print >> fobj, ""
		print >> fobj, "    return str;"
		print >> fobj, "}"
		print >> fobj, ""
		return

	for i, field in enumerate(field_array):
		field_len = len(field)
		dt_type = field[field_len - 1]
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)

			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = ds_value_name + '->' + key_str + '.' + data_name
			else:
				param_str = ds_value_name + '->' + key_str
		else:
			break


		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = tbl_name_upper + '_FLD_' + string.upper(field_name)


		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			print tbl_name_upper
			print >> fobj, ""
			print >> fobj, "    %s(str, %s, %s);" % (func_val2str,  CONST.BUF_SIZE, param_str)
		elif ('UINT' == dt_type[1]):
			field_name_string = """%u"""
			print >> fobj, "    sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('UINT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_sprintf(str, \"%s\"PRIu64, %s);" % (field_name_string, param_str)
		elif ('INT' == dt_type[1]):
			field_name_string = """%d"""
			print >> fobj, "    sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('INT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_sprintf(str, \"%s\"PRId64, %s);" % (field_name_string, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "    sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "    sal_sprintf(str, \"%s\", cdb_enum_val2str(%s, %s, %s));" % (
			field_name_string, enum_type_str, enum_type_max, param_str)
		else:
			cdb_util.error_return("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_ds_func_key_name_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_name_upper = parent_table[0]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	paramtype = ds_type
	node_value = 'p_node'
	field_value = 'p_field'
	bak_field_id = 'bak_field_id'
	accusative = ds_name[3:]
	tbl_info_value = 'p_tbl_info'
	need_listnode = 0
	refer_object_array = []
	have_key = 0
	funcstr = "%s_key_name_dump(%s %s, %s %s, char* str)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""
	print >> fobj, "    cdb_node_t *p_node_tbl = cdb_get_tbl(%s);" % (tbl_name_upper)
	print >> fobj, "    cdb_node_t *p_node_ds = cdb_get_ds(%s);" % (ds_name_upper)
	print >> fobj, "    char tbl_str[MAX_CMD_STR_LEN];"
	print >> fobj, "    char ds_str[MAX_CMD_STR_LEN];"
	print >> fobj, ""
	print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR\"%s\"KEY_CONNECT_STR\"%s\"KEY_CONNECT_STR\"%s\","
	print >> fobj, "        p_node_tbl->name, %s_key_value_dump(%s, tbl_str), p_node_ds->name, %s_key_value_dump(%s, %s, ds_str));"\
				   %(tbl_name, tbl_value_name, ds_name, tbl_value_name, ds_value_name)
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_field_name_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	tbl_name_upper = parent_table[0]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	paramtype = ds_type
	node_value = 'p_node'
	field_value = 'p_field'
	bak_field_id = 'bak_field_id'
	accusative = ds_name[3:]
	tbl_info_value = 'p_tbl_info'
	need_listnode = 0
	refer_object_array = []
	have_key = 0
	funcstr = "%s_field_name_dump(%s %s, %s %s, int32 field_id, char* str)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""
	print >> fobj, "    cdb_node_t *p_node = cdb_get_ds(%s);" % (ds_name_upper)
	print >> fobj, "    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;"
	print >> fobj, "    if (field_id >= %s_FLD_MAX)" % (ds_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    sal_sprintf(str, \"%s\", p_tbl_info->field[field_id].name);"
	print >> fobj, ""
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_field_value_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_name_upper = parent_table[0]
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	have_key = 0

	funcstr = "%s_field_value_dump(%s %s, %s %s, int32 field_id, char* str)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""


	for i, field in enumerate(field_array):
		field_len = len(field)
		dt_type = field[field_len - 1]
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = ds_value_name + '->' + key_str + '.' + data_name
			else:
				param_str = ds_value_name + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = ds_value_name + '->' + data_name

		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		print >> fobj, "    if (FLD_MATCH(%s, field_id))" % (field_id_enum)
		print >> fobj, "    {"

		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			if ('cdb_bitmap_val2str' == func_val2str or 'cdb_int32_array_val2str' == func_val2str or 'cdb_uint32_array_val2str' == func_val2str or 'cdb_uint64_array_val2str' == func_val2str):
				print >> fobj, "        %s(str, %s, %s, sizeof(%s)/4);" % (
				func_val2str, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_int8_array_val2str' == func_val2str or 'cdb_uint8_array_val2str' == func_val2str):
				print >> fobj, "        %s(str, %s, %s, sizeof(%s));" % (
				func_val2str, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_double_array_val2str' == func_val2str):
				print >> fobj, "        %s(str, %s, %s, sizeof(%s)/sizeof(double));" % (
				func_val2str, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_string_array_val2str' == func_val2str):
				print >> fobj, "        %s(str, %s, (const char*)%s," % (
				func_val2str, CONST.BUF_SIZE, param_str)
				print >> fobj, "        sizeof(%s[0]), sizeof(%s)/sizeof(%s[0]));" % (
				param_str, param_str, param_str)
			else:
				print >> fobj, "        %s(str, %s, %s);" % (func_val2str, CONST.BUF_SIZE, param_str)
		elif ('UINT' == dt_type[1]):
			field_name_string = """%u"""
			print >> fobj, "        sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('UINT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "        sal_sprintf(str, \"%s\"PRIu64, %s);" % (field_name_string, param_str)
		elif ('INT' == dt_type[1]):
			field_name_string = """%d"""
			print >> fobj, "        sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('INT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "        sal_sprintf(str, \"%s\"PRId64, %s);" % (field_name_string, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "        sal_sprintf(str, \"%s\", %s);" % (field_name_string, param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "        sal_sprintf(str, \"%s\", cdb_enum_val2str(%s, %s, %s));" % (field_name_string, enum_type_str, enum_type_max, param_str)
		elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			print >> fobj, "        sal_sprintf(str, \"%s\"," % (field_value_string)
			print >> fobj, "            GLB_FLAG_ISSET(%s, %s));" % (param_str, field[4])
		elif ('DOUBLE' == dt_type[1]):
			print >> fobj, "        sal_sprintf(str, \"%s\"," % (field_value_string)
			print >> fobj, "            %s);" % (param_str)
		else:
			print("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"

	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_table_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_name_upper = parent_table[0]
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	have_key = 0

	funcstr = "%s_table_dump(%s %s, %s %s, char** str)" % (
	ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char**"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
	print >> fobj, "    int i = 0, j= 0;"
	print >> fobj, ""
	print >> fobj, "    sal_sprintf(str[j++], \"%s\","
	print >> fobj, "         %s_key_name_dump(%s, %s, buf));" % (ds_name, tbl_value_name, ds_value_name)
	print >> fobj, "    for(i=1; i<%s_FLD_MAX; i++)" % (ds_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        sal_sprintf(str[j++], \"%s\","
	print >> fobj, "            %s_field_name_dump(%s, %s, i, buf));" % (ds_name, tbl_value_name, ds_value_name)
	print >> fobj, "        sal_sprintf(str[j++], \"%s\","
	print >> fobj, "            %s_field_value_dump(%s, %s, i, buf));" % (ds_name, tbl_value_name, ds_value_name)
	print >> fobj, "    }"
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_field_value_parser_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_name_upper = parent_table[0]
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	have_key = 0

	funcstr = "%s_field_value_parser(char *str, int32 field_id, %s %s, %s %s)" % (
	    ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    int32 ret=0;"

	for i, field in enumerate(field_array):
		field_len = len(field)
		dt_type = field[field_len - 1]
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = ds_name_upper + '_FLD_' + string.upper(field_name)

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = ds_value_name + '->' + key_str + '.' + data_name
			else:
				param_str = ds_value_name + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = ds_value_name + '->' + data_name

		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		print >> fobj, "    if (FLD_MATCH(%s, field_id))" % (field_id_enum)
		print >> fobj, "    {"

		if (has_val2str):
			func_val2str = dt_type[2][4]
			func_str2val = string.strip(func_val2str, "val2str")
			func_str2val = func_str2val + "str2val"
			val2str_func_array.append(func_val2str)
			if ('cdb_bitmap_val2str' == func_val2str):
				print >> fobj, "        char err[MAX_CMD_STR_LEN];"
				print >> fobj, "        %s(str, 0, GLB_UINT16_BMP_WORD_MAX, %s, sizeof(%s)/4, err);" % (
				func_str2val, param_str, param_str)
			elif ('cdb_addr_val2str' == func_val2str):
				print >> fobj, "        %s(%s, str, 0);" % (func_str2val, param_str)
			elif ('cdb_addr_ipv4_val2str' == func_val2str):
				print >> fobj, "        %s(%s, str, 0);" % (func_str2val, param_str)
			elif ('cdb_prefix_val2str' == func_val2str):
				print >> fobj, "        cdb_prefix_str2val_wo_masklen(str, %s);" % (param_str)
			elif ('cdb_prefix_ipv4_val2str' == func_val2str):
				print >> fobj, "        cdb_prefix_ipv4_str2val_wo_masklen(str, %s);" % (param_str)
			elif ('cdb_fdb_key_val2str' == func_val2str):
				print >> fobj, "        char err[MAX_CMD_STR_LEN];"
				print >> fobj, "        %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_mac_addr_val2str' == func_val2str):
				print >> fobj, "        %s(%s, str);" % (func_str2val, param_str)
			elif ('cdb_auth_server_key_val2str' == func_val2str or
						  'cdb_ns_port_forwarding_key_val2str' == func_val2str or
						  'cdb_user_val2str' == func_val2str or
						  'cdb_binding_key_str2val' == func_str2val or
						  'cdb_auth_session_key_str2val' == func_str2val):
				print >> fobj, "        char err[MAX_CMD_STR_LEN];"
				print >> fobj, "        %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_snmp_info_val2str' == func_val2str):
				print >> fobj, "        char err[MAX_CMD_STR_LEN];"
				print >> fobj, "        %s(%s, str, err);" % (func_str2val, param_str)
			elif ('cdb_string_array_str2val' == func_str2val):
				print >> fobj, "        %s(str, (char**)(&%s));" % (func_str2val, param_str)
			elif ('cdb_mstp_digest_str2val' == func_str2val):
				print >> fobj, "        %s(%s, str);" % (func_str2val, param_str)
			else:
				print >> fobj, "        %s(str, %s);" % (func_str2val, param_str)
		elif ('ARRAY_DS' in field[0]):
				name_upper = field[0][6:]
				name_lower = string.lower(name_upper)
				sub_field_id_enum = 'FLD_ID_ALL'
				params = field[1][:-1]
				params_array = params.split('[')
				refer_object_param = 'p_' + 'db_' + params_array[0]
				print >> fobj, "        p_field->field_id[1] = %s;" % (sub_field_id_enum)
				print >> fobj, "        for (i = 0; i < %s; i++)" % (params_array[1])
				print >> fobj, "        {"
				print >> fobj, "            %s = &(%s->%s[i]);" % (refer_object_param, paramvalue, params_array[0])
				print >> fobj, "            %s_dump_one(%s, %s);" % (name_lower, refer_object_param, pargs_value)
				print >> fobj, "            sal_sprintf(str, \"/,\");"
				print >> fobj, "        }"
				print >> fobj, "        sal_sprintf(str, \"%s\");" % ('}')
				print >> fobj, ""
				print >> fobj, "    %s = cdb_uint_str2val(str, &ret);" % (param_str)
		elif ('UINT64' == dt_type[1]):
			print >> fobj, "        %s = cdb_uint64_str2val(str, &ret);" % (param_str)
		elif ('UINT' == dt_type[1]):
			print >> fobj, "        %s = cdb_uint_str2val(str, &ret);" % (param_str)
		elif ('INT64' == dt_type[1]):
			print >> fobj, "        %s = cdb_int64_str2val(str, &ret);" % (param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "        sal_strcpy(%s, str);" % (param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "        cdb_enum_str2val(%s, %s, str);" % (enum_type_str, enum_type_max)
		elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			print >> fobj, "        GLB_SET_FLAG(%s, %s);" % (param_str, field[4])
		elif ('DOUBLE' == dt_type[1]):
			print >> fobj, "        %s = cdb_double_str2val(str, &ret);" % (param_str)
		else:
			print("unhandled field in TBL field value parser: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"

	print >> fobj, "    return ret;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_func_table_parser_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array):
	tbl_name = string.lower(parent_table[0])
	tbl_name_upper = parent_table[0]
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name_upper = ds[0]
	ds_value_name = 'p_' + ds[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	have_key = 0

	funcstr = "%s_table_parser(char** array, char* tbl_key_value, char* ds_key_value,%s %s, %s %s)"  % (
	    ds_name, tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    int32 rc;"
	print >> fobj, "    int32 i = 0, j= 0;"
	print >> fobj, "    cdb_node_t *p_node = cdb_get_ds(%s);" % (ds_name_upper)
	print >> fobj, "    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;"
	print >> fobj, ""
	print >> fobj, "    PM_E_RETURN(%s_key_str2val(tbl_key_value, ds_key_value, %s, %s));" % (ds_name, tbl_value_name, ds_value_name)
	print >> fobj, ""
	print >> fobj, "    for(i=1; i<%s_FLD_MAX; i++)" % (ds_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))"
	print >> fobj, "        {"
	print >> fobj, "             return PM_E_INVALID_PARAM;"
	print >> fobj, "        }"
	print >> fobj, "        PM_E_RETURN(%s_field_value_parser( array[j++], i, %s, %s));" % (ds_name, tbl_value_name, ds_value_name)
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_ds_func_set_refer_list_func(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, referlist_field):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_mem_type = 'MEM_' + ds[0]
	ds_value_name = 'p_' + ds[1]
	dbvalue = 'p_db_' + ds[1]
	ds_field_type = ds_name + '_field_id_t'
	accusative = ds_name[3:]
	object_info = cdb_util.get_refer_object_info(referlist_field)
	if ('CDB_NODE_DS' == object_info[1]):
		return
	field_name = object_info[0]
	full_field_name = ds[0] + '_FLD_' + string.upper(field_name)
	is_add = 'add'
	p_object = 'p_object'
	parent_ds_param_str = cdb_util.gen_parent_ds_param_str(parent_ds)
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)
	if (len(key_array)):
		haskey_get_ds_param = ', ' + ds_value_name
	else:
		haskey_get_ds_param = ''

	refer_tbl_name = referlist_field[0][10:]
	funcstr = "%s_set_%s_sync(%s %s, %s %s, %s %s, %s %s, uint32 sync)" % (ds_name, field_name, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    cdb_sync_refer_hdr_t refer;"
	print >> fobj, "    %s *%s = NULL;" % (ds_type, dbvalue)
	print >> fobj, "    int32 rc = PM_E_NONE;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	print >> fobj, "    %s = %s_get_%s(%s%s);" % (dbvalue, ds_name, accusative, tbl_value_name, haskey_get_ds_param)
	print >> fobj, "    if (NULL == %s)" % (dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. update db entry field */"
	print >> fobj, "    if (%s)" % (is_add)
	print >> fobj, "    {"
	print >> fobj, "        PM_E_RETURN(cdb_reference_list_add_object(&%s->%s, %s));" % (dbvalue, field_name, p_object)
	print >> fobj, "    }"
	print >> fobj, "    else"
	print >> fobj, "    {"
	print >> fobj, "        PM_E_RETURN(cdb_reference_list_del_object(&%s->%s, %s));" % (dbvalue, field_name, p_object)
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 3. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	print >> fobj, "        cdb_build_refer_tbl(%s, add, &refer);" % (refer_tbl_name)
	print >> fobj, "        cdb_sync_ds_set_refer(%s, %s, %s, %s, %s, &refer, %s);" % (parent_table[0], ds[0], full_field_name, tbl_value_name, ds_value_name, p_object)
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"

	funcstr = "%s_set_%s(%s %s, %s %s, %s %s, %s %s)" % (ds_name, field_name, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_set_%s_sync(%s, %s, %s, %s, TRUE);" % (ds_name, field_name, tbl_value_name, ds_value_name, is_add, p_object)
	print >> fobj, "}"
	print >> fobj, ""

def gen_ds_c_one(ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array):
	ds_name = ds[0]
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	tbl_db_type = ds[2]
	parent_table = parent_tbl[0]
	tbl_name = string.lower(parent_table[0])
	fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
	fname_h = 'gen/' + string.lower(tbl_name) + '.h'
	fname = CONST.CDB_C_PATH + string.lower(ds_name) + '.c'
	fobj = open(fname, 'w')
	is_parent_global = 0
	is_sub_ds = 0
	parent_ds = None

	if ('GLOBAL' == parent_table[2]):
		is_parent_global = 1

	if (4 == len(ds)):
		is_sub_ds = 1

	if (is_sub_ds):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)

	print >> fobj, ""
	print >> fobj, "#include \"%s\"" % (CONST.PROTO_H_FILE)
	print >> fobj, "#include \"%s\"" % (fname_define_h)
	print >> fobj, "#include \"%s\"" % (fname_h)
	referlist_array = cdb_util.has_cdb_refer_list(data_array)
	cdb_util.include_refer_tbl_ds_h_file(fobj, referlist_array, parent_table[0])
	print >> fobj, "#include \"%s\"" % (CONST.CDB_CMP_H_FILE)
	print >> fobj, ""

	cdb_util.insert_tbl_user_define_begin(fobj, ds_name, '.c')

	if (is_sub_ds):
		ds_field_name = cdb_util.ds_get_parent_ds_field(parent_ds, ds)
	else:
		ds_field_name = cdb_util.ds_get_parent_tbl_field(parent_tbl, ds)

	if (0 == is_array_ds):
		gen_ds_func_add_sync_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, cmp_func_array, is_parent_global)
		gen_ds_func_del_sync_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, is_parent_global)
	gen_ds_func_set_field_sync_implement(fobj, ds, key_array, field_array, parent_table, parent_ds, ds_field_name, is_parent_global)
	if (0 == is_array_ds):
		gen_ds_func_add_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, cmp_func_array)
		gen_ds_func_del_implement(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name)
	gen_ds_func_set_field_implement(fobj, ds, key_array, field_array, parent_table, parent_ds, ds_field_name)
	gen_ds_func_get_implement(fobj, ds, key_array, parent_tbl, parent_ds, ds_field_name, is_array_ds, is_parent_global)
	gen_ds_func_key_str2val_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
								   val2str_func_array, referlist_tbl_array)
	gen_ds_func_key_value_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
										val2str_func_array, referlist_tbl_array)
	gen_ds_func_key_name_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
									  val2str_func_array, referlist_tbl_array)
	gen_ds_func_field_name_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
										val2str_func_array, referlist_tbl_array)
	gen_ds_func_field_value_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
										  val2str_func_array, referlist_tbl_array)
	gen_ds_func_table_dump_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
										   val2str_func_array, referlist_tbl_array)
	gen_ds_func_field_value_parser_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
									 val2str_func_array, referlist_tbl_array)
	gen_ds_func_table_parser_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array,
									 val2str_func_array, referlist_tbl_array)
	gen_ds_func_dump_one_implement(fobj, ds, key_array, field_array, parent_table, ds_field_name, dt_array, val2str_func_array, referlist_tbl_array)

	for i, referlist_field in enumerate(referlist_array):
		gen_ds_func_set_refer_list_func(fobj, ds, key_array, data_array, parent_table, parent_ds, ds_field_name, referlist_field)

	cdb_util.insert_tbl_user_define_end(fobj, ds_name, '.c')
	fobj.close()
	print "generate file %s" % (fname)

def gen_ds_c_files(ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array):
	'generate ds api c files'
	for i, ds_one in enumerate(ds_array):
		gen_ds_c_one(ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array)