import os
import sys
import string
import CONST
import cdb_util

bypass_tbl_files = ['TBL_ROUTE_NODE', 'TBL_LOG']

def ds_find_ds(ds_array, ds_name):
	for i, ds_entry in enumerate(ds_array):
		ds = ds_entry[0]
		if (ds_name == ds[0]):
			return ds_entry

def get_array_size(array_value):
	start_index = array_value.find('[')
	end_index = array_value.find(']')
	if (start_index <= 0 or end_index <= 0):
		cdb_util.error_return("invalid array value %s\n", array_value)
	return array_value[start_index+1:end_index]

def gen_tbl_master_value(fobj, table, is_global=0):
	tbl_name = string.lower(table[0])
	if (is_global):
		tbl_mst_type = tbl_name + '_t'
		tbl_g_mst_name = '_g_p_' + tbl_name
	else:
		tbl_mst_type = tbl_name + '_master_t'
		tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	print >> fobj, "%s *%s = NULL;" % (tbl_mst_type, tbl_g_mst_name)
	print >> fobj, ""

def gen_tbl_master_field(fobj, tbl_g_mst_name, tbl_name, master_field, cmp_func_array):
	tbl_db_type = master_field[0]
	tbl_field = cdb_util.format_field_name(master_field[1])
	if (tbl_db_type.find('hash') > 0):
		tbl_hash_func_make = '_' + tbl_name + '_hash_make'
		tbl_hash_func_cmp = '_' + tbl_name + '_hash_cmp'
		print >> fobj, "    %s->%s = ctclib_hash_create(%s, %s);" % (tbl_g_mst_name, tbl_field, tbl_hash_func_make, tbl_hash_func_cmp)
	elif (tbl_db_type.find('list') > 0):
		cmp_func = tbl_name + '_cmp'
		print >> fobj, "    %s->%s = ctclib_slist_create(%s, NULL);" % (tbl_g_mst_name, tbl_field, cmp_func)
		cmp_func_array.append(cmp_func)
	else:
		# array of pointer
		pass

def gen_tbl_func_add_sync_implement(fobj, table, key_array, data_array, field_array, master_array, cmp_func_array, array_ds_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	accusative = tbl_name[4:]
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	tbl_mst_type = tbl_name + '_master_t'
	tbl_mst_name = 'p_master'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	mst_name = 'p_master'
	hash_ret = 'p_ret'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	dbvalue = 'p_db_' + tbl_value_name
	master_field = master_array[0]
	tbl_field = cdb_util.format_field_name(master_field[1])

	funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'add', accusative, paramtype, '*' + paramvalue, '_sync')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = %s;" % (tbl_mst_type, mst_name, tbl_g_mst_name)
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
	if ('HASH' == tbl_db_type or 'HASH+LIST' == tbl_db_type):
		print >> fobj, "    %s *%s = NULL;" % ('void', hash_ret)
	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	if (len(referlist_array)):
		print >> fobj, "    int32 rc = PM_E_NONE;"
	if (len(array_ds_array)):
		print >> fobj, "    uint32 i = 0;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	print >> fobj, "    if (%s_get_%s(&%s->key))" % (tbl_name, accusative, paramvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_EXIST;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. alloc a new db entry */"
	print >> fobj, "    %s = XCALLOC(%s, sizeof(%s));" % (dbvalue, mem_type, tbl_type)
	print >> fobj, "    if (NULL == %s)" % (dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NO_MEMORY;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 3. evaluate db entry */"
	print >> fobj, "    sal_memcpy(%s, %s, sizeof(%s));" % (dbvalue, paramvalue, tbl_type)
	for i, referlist_field in enumerate(referlist_array):
		object_info = cdb_util.get_refer_object_info(referlist_field)
		cmp_func = string.lower(object_info[2]) + '_cmp'
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_init(&%s->%s, %s, %s, %s));" % (dbvalue, object_info[0], object_info[1], object_info[2], cmp_func)
		cmp_func_array.append(cmp_func)

	tailq_head_array = cdb_util.has_cdb_tailq_head_param_list(data_array)
	for i, tailq_head in enumerate(tailq_head_array):
		print >> fobj, "    CTCLIB_TAILQ_INIT(&%s->%s);" % (dbvalue, tailq_head)

	for i, array_ds in enumerate(array_ds_array):		
		params = array_ds[1][:-1]
		params_array = params.split('[')
		print >> fobj, "    for (i = 0; i < %s; i++)" % (params_array[1])
		print >> fobj, "    {"
		print >> fobj, "        %s->%s[i].key.index = i;" % (dbvalue, params_array[0])
		print >> fobj, "    }"

	print >> fobj, ""
	print >> fobj, "    /* 4. add to db */"
	if ('HASH' == tbl_db_type or 'HASH+LIST' == tbl_db_type):
		print >> fobj, "    %s = ctclib_hash_get(%s->%s, (void*)%s, ctclib_hash_alloc_intern);" % (hash_ret, mst_name, tbl_field, dbvalue)
		print >> fobj, "    if (NULL == %s)" % (hash_ret)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NO_MEMORY;"
		print >> fobj, "    }"
		if ('HASH+LIST' == tbl_db_type):
			master_field_list = master_array[1]
			tbl_field_list = cdb_util.format_field_name(master_field_list[1])
			print >> fobj, "    ctclib_slistnode_insert_sort(%s->%s, %s);" % (mst_name, tbl_field_list, dbvalue)
	elif ('LIST' == tbl_db_type):
		master_field_list = master_array[0]
		tbl_field_list = cdb_util.format_field_name(master_field_list[1])
		insert_func_name = "ctclib_slistnode_insert_sort"
		if (len(master_field_list) > 2):
			if (master_field_list[2].find("no sort") >= 0):
				insert_func_name = "ctclib_slistnode_insert"
		print >> fobj, "    %s(%s->%s, %s);" % (insert_func_name, mst_name, tbl_field_list, dbvalue)
	elif ('ARRAY' == tbl_db_type):
		keyvalue = key_array[0][1]
		print >> fobj, "    %s->%s[%s->key.%s] = %s;" % (mst_name, tbl_field, paramvalue, keyvalue, dbvalue)
	elif ('USERDEFINE' == tbl_db_type):
		pass
	else:
		cdb_util.error_return("invalid table database type: %s", table[2])

	# for tbl_interface if_list
	if (2 == len(master_array)):
		master_field_1 = master_array[1]
		tbl_field_1 = cdb_util.format_field_name(master_field_1[1])

	print >> fobj, ""
	print >> fobj, "    /* 5. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	print >> fobj, "        cdb_sync_tbl_add(%s, %s);" % (tbl_name_upper, dbvalue)
	print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
	print >> fobj, "        cdb_sync_publish_tbl_add(%s, %s);" % (tbl_name_upper, dbvalue)
	print >> fobj, "        #endif "
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_del_sync_implement(fobj, table, key_array, field_array, master_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	tbl_type = tbl_name + '_t'
	tbl_key_type = tbl_name + '_key_t'
	paramtype = tbl_key_type
	paramvalue = 'p_' + tbl_value_name + '_key'
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name

	funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'del', accusative, paramtype, '*' + paramvalue, '_sync')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s *%s = %s;" % (tbl_mst_type, mst_name, tbl_g_mst_name)
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	if (len(referlist_array)):
		print >> fobj, "    int32 rc = PM_E_NONE;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	print >> fobj, "    %s = %s_get_%s(%s);" % (dbvalue, tbl_name, accusative, paramvalue)
	print >> fobj, "    if (NULL == %s)" % (dbvalue)
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NONE;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	print >> fobj, "        cdb_sync_tbl_del(%s, %s);" % (tbl_name_upper, dbvalue)
	print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
	print >> fobj, "        cdb_sync_publish_tbl_del(%s, %s);" % (tbl_name_upper, dbvalue)
	print >> fobj, "        #endif "
	print >> fobj, "    }"

	print >> fobj, ""
	print >> fobj, "    /* 3. delete from db */"

	if ('HASH' == tbl_db_type or 'HASH+LIST' == tbl_db_type):
		master_field_hash = master_array[0]
		tbl_hash_name = cdb_util.format_field_name(master_field_hash[1])
		print >> fobj, "    ctclib_hash_release(%s->%s, (void*)%s);" % (mst_name, tbl_hash_name, dbvalue)
		if ('HASH+LIST' == tbl_db_type):
			master_field_list = master_array[1]
			tbl_field_list = cdb_util.format_field_name(master_field_list[1])
			print >> fobj, "    ctclib_slistnode_delete(%s->%s, %s);" % (mst_name, tbl_field_list, dbvalue)
	elif ('LIST' == tbl_db_type):
		master_field_list = master_array[0]
		tbl_field_list = cdb_util.format_field_name(master_field_list[1])
		print >> fobj, "    ctclib_slistnode_delete(%s->%s, %s);" % (mst_name, tbl_field_list, dbvalue)
	elif ('ARRAY' == tbl_db_type):
		keyvalue = key_array[0][1]
		master_field_array = master_array[0]
		tbl_array_name = cdb_util.format_field_name(master_field_array[1])
		print >> fobj, "    %s->%s[%s->%s] = NULL;" % (mst_name, tbl_array_name, paramvalue, keyvalue)
	elif ('USERDEFINE' == tbl_db_type):
		pass
	for i, referlist_field in enumerate(referlist_array):
		field_name = cdb_util.format_field_name(referlist_field[1])
		print >> fobj, "    PM_E_RETURN(cdb_reference_list_deinit(&%s->%s));" % (dbvalue, field_name)
	print >> fobj, ""
	print >> fobj, "    /* 4. free db entry */"
	print >> fobj, "    XFREE(%s, %s);" % (mem_type, dbvalue)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_set_implement(fobj, table, key_array, master_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name

	funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'set', accusative, paramtype, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if (is_global):
		tbl_g_mst_name = '_g_p_' + tbl_name
		print >> fobj, "    %s *%s = %s;" % (tbl_type, dbvalue, tbl_g_mst_name)
	else:
		print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
		print >> fobj, ""
		print >> fobj, "    /* 1. lookup entry exist */"
		print >> fobj, "    %s = %s_get_%s(&%s->key);" % (dbvalue, tbl_name, accusative, paramvalue)
		print >> fobj, "    if (NULL == %s)" % (dbvalue)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NOT_EXIST;"
		print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. update db entry */"
	print >> fobj, "    sal_memcpy(%s, %s, sizeof(%s));" % (dbvalue, paramvalue, tbl_type)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_set_field_evaluate(fobj, field, tbl_name_upper, dbvalue, paramvalue):
	if (len(field) > 4):
		dt_type = field[6]
	else:
		dt_type = field[3]
	dt_info = dt_type[2]
	# sub DS_ need not set field
	if ('DS_' in dt_type[1]):
		return
	if ('cdb_reference_list_t' == dt_type[1]):
		return
	if ('key' == field[1]):
		return

	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])	
	field_id_enum = tbl_name_upper + '_FLD_' + string.upper(field_name)
	db_field_value = dbvalue + '->' + field_name
	param_field_value = paramvalue + '->' + field_name
	
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
			cdb_util.error_return("unhandled field in TBL set_field: %s, dt_type[1]: %s", field, dt_type[1])
	print >> fobj, "        break;"
	print >> fobj, ""

def gen_tbl_func_set_field_sync_implement(fobj, table, key_array, field_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	suffix_str = ', uint32 sync'

	funcstr = "%s_set_%s_field_sync(%s %s, %s %s%s)" % (tbl_name, accusative, tbl_type, '*' + paramvalue, field_id_type, 'field_id', suffix_str)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if (is_global):
		tbl_g_mst_name = '_g_p_' + tbl_name
		print >> fobj, "    %s *%s = %s;" % (tbl_type, dbvalue, tbl_g_mst_name)
	else:
		print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
		print >> fobj, ""
		print >> fobj, "    /* 1. lookup entry exist */"
		print >> fobj, "    %s = %s_get_%s(&%s->key);" % (dbvalue, tbl_name, accusative, paramvalue)
		print >> fobj, "    if (NULL == %s)" % (dbvalue)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NOT_EXIST;"
		print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 2. update db entry field */"
	print >> fobj, "    switch (field_id)"
	print >> fobj, "    {"
	for i, field in enumerate(field_array):
		gen_tbl_func_set_field_evaluate(fobj, field, tbl_name_upper, dbvalue, paramvalue)
	print >> fobj, "    case %s:" % (tbl_name_upper + '_FLD_MAX')
	print >> fobj, "        sal_memcpy(%s, %s, sizeof(%s));" % (dbvalue, paramvalue, tbl_type)
	print >> fobj, "        break;"
	print >> fobj, "    default:"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    /* 3. sync to CDB */"
	print >> fobj, "    if (sync)"
	print >> fobj, "    {"
	print >> fobj, "        cdb_sync_tbl_set(%s, %s, %s);" % (tbl_name_upper, 'field_id', dbvalue)
	print >> fobj, "        #ifdef LDB_SYNC_TO_RDB "
	print >> fobj, "        cdb_sync_publish_tbl_set(%s, %s, %s);" % (tbl_name_upper, 'field_id', dbvalue)
	print >> fobj, "        #endif "
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_set_field_implement(fobj, table, key_array, field_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	funcstr = "%s_set_%s_field(%s %s, %s %s)" % (tbl_name, accusative, tbl_type, '*' + paramvalue, field_id_type, 'field_id')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_set_%s_field_sync(%s, %s, TRUE);" % (tbl_name, accusative, paramvalue, 'field_id')
	print >> fobj, "}"

def gen_tbl_func_add_implement(fobj, table, key_array, field_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	funcstr = "%s_add_%s(%s %s)" % (tbl_name, accusative, tbl_type, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_add_%s_sync(%s, TRUE);" % (tbl_name, accusative, paramvalue)
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_del_implement(fobj, table, key_array, field_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_key_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name + '_key'
	accusative = tbl_name[4:]
	funcstr = "%s_del_%s(%s %s)" % (tbl_name, accusative, tbl_type, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_del_%s_sync(%s, TRUE);" % (tbl_name, accusative, paramvalue)
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_set_field_implement(fobj, table, key_array, field_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	funcstr = "%s_set_%s_field(%s %s, %s %s)" % (tbl_name, accusative, tbl_type, '*' + paramvalue, field_id_type, 'field_id')
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_set_%s_field_sync(%s, %s, TRUE);" % (tbl_name, accusative, paramvalue, 'field_id')
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_get_implement(fobj, table, key_array, master_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	tbl_type = tbl_name + '_t'
	tbl_key_type = tbl_name + '_key_t'
	paramtype = tbl_key_type
	paramvalue = 'p_' + tbl_value_name + '_key'
	accusative = tbl_name[4:]
	mst_name = 'p_master'

	if (is_global):
		funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'get', accusative, '', '')
	else:
		funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'get', accusative, paramtype, '*' + paramvalue)
	print >> fobj, "%s*" % (tbl_type)
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"

	if (is_global):
		tbl_g_mst_name = '_g_p_' + tbl_name
		print >> fobj, "    return %s;" % (tbl_g_mst_name)
	elif ('USERDEFINE' == tbl_db_type):
		pass
	else:
		print >> fobj, "    %s *%s = %s;" % (tbl_mst_type, mst_name, tbl_g_mst_name)
		if ('HASH' == tbl_db_type or 'HASH+LIST' == tbl_db_type):
			master_field_hash = master_array[0]
			tbl_hash_name = cdb_util.format_field_name(master_field_hash[1])
			print >> fobj, "    %s lkp;" % (tbl_type)
			print >> fobj, ""
			print >> fobj, "    sal_memcpy(&lkp.key, %s, sizeof(%s));" % (paramvalue, tbl_key_type)
			print >> fobj, "    return ctclib_hash_lookup(%s->%s, &lkp);" % (mst_name, tbl_hash_name)
		if ('LIST' == tbl_db_type):
			master_field_list = master_array[0]
			tbl_list_name = cdb_util.format_field_name(master_field_list[1])
			print >> fobj, "    %s lkp;" % (tbl_type)
			print >> fobj, ""
			print >> fobj, "    sal_memcpy(&lkp.key, %s, sizeof(%s));" % (paramvalue, tbl_key_type)
			print >> fobj, "    return ctclib_slistdata_lookup(%s->%s, &lkp);" % (mst_name, tbl_list_name)
		elif ('ARRAY' == tbl_db_type):
			keyvalue = key_array[0][1]
			array_size = get_array_size(master_array[0][1])
			master_field_array = master_array[0]
			tbl_array_name = cdb_util.format_field_name(master_field_array[1])
			print >> fobj, ""
			print >> fobj, "    if (%s <= %s->%s)" % (array_size, paramvalue, keyvalue)
			print >> fobj, "    {"
			print >> fobj, "        return NULL;"
			print >> fobj, "    }"
			print >> fobj, "    return (%s->%s[%s->%s]);" % (mst_name, tbl_array_name, paramvalue, keyvalue)
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_iterate_implement(fobj, table, key_array, data_array, master_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name = string.lower(table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	tbl_hash_name = tbl_value_name + '_hash'
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name

	param1_type = 'TBL_ITER_CB_FUNC'
	param1_value = 'fn'
	param2_type = 'tbl_iter_args_t'
	param2_value = 'pargs'

	funcstr = "%s_iterate(%s %s, %s %s)" % (tbl_name, param1_type, param1_value, param2_type, '*' + param2_value)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if (is_global):
		tbl_g_mst_name = '_g_p_' + tbl_name
		print >> fobj, "    %s *%s = %s;" % (tbl_type, dbvalue, tbl_g_mst_name)
		print >> fobj, "    int32 rc = PM_E_NONE;"
		print >> fobj, ""
		print >> fobj, "    if (NULL == %s)" % (dbvalue)
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NONE;"
		print >> fobj, "    }"
		print >> fobj, ""
		print >> fobj, "    rc = %s(%s, %s);" % (param1_value, dbvalue, param2_value)
		print >> fobj, "    return (rc) ? PM_E_FAIL : PM_E_NONE;"
	else:
		print >> fobj, "    %s *%s = %s;" % (tbl_mst_type, mst_name, tbl_g_mst_name)
		if ('USERDEFINE' == tbl_db_type):
			pass
		elif ('HASH' == tbl_db_type):
			print >> fobj, "    if (NULL == %s)" % (mst_name)
			print >> fobj, "    {"
			print >> fobj, "        return PM_E_NONE;"
			print >> fobj, "    }"
			print >> fobj, ""
			print >> fobj, "    ctclib_hash_iterate2(p_master->%s, %s, %s, %s);" % (tbl_hash_name, 'tbl_hash_iter_adpt_fn', param1_value, param2_value)
			print >> fobj, "    return PM_E_NONE;"
		elif ('HASH+LIST' == tbl_db_type or 'LIST' == tbl_db_type):
			if ('HASH+LIST' == tbl_db_type):
				master_field_list = master_array[1]
			else:
				master_field_list = master_array[0]
			tbl_field_list = cdb_util.format_field_name(master_field_list[1])
			node_type = 'ctclib_slistnode_t'
			node_value = 'listnode'
			next_node_value = 'next'
			print >> fobj, "    %s *%s = NULL;" % (node_type, node_value)
			print >> fobj, "    %s *%s = NULL;" % (node_type, next_node_value)
			print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
			print >> fobj, "    int32 rc = PM_E_NONE;"
			print >> fobj, ""
			print >> fobj, "    if (NULL == %s)" % (mst_name)
			print >> fobj, "    {"
			print >> fobj, "        return PM_E_NONE;"
			print >> fobj, "    }"
			print >> fobj, ""
			print >> fobj, "    CTCLIB_SLIST_LOOP_DEL(%s->%s, %s, %s, %s)" % (mst_name, tbl_field_list, dbvalue, node_value, next_node_value)
			print >> fobj, "    {"
			print >> fobj, "        rc |= %s(%s, %s);" % (param1_value, dbvalue, param2_value)
			print >> fobj, "    }"
			print >> fobj, ""
			print >> fobj, "    return (rc) ? PM_E_FAIL : PM_E_NONE;"
		elif ('ARRAY' == tbl_db_type):
			master_field = master_array[0]
			tbl_field = cdb_util.format_field_name(master_field[1])
			keyvalue = key_array[0][1]
			array_size = get_array_size(master_array[0][1])
			print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
			print >> fobj, "    uint32 %s = 0;" % (keyvalue)
			print >> fobj, "    int32 rc = PM_E_NONE;"
			print >> fobj, ""
			print >> fobj, "    if (NULL == %s)" % (mst_name)
			print >> fobj, "    {"
			print >> fobj, "        return PM_E_NONE;"
			print >> fobj, "    }"
			print >> fobj, ""
			print >> fobj, "    for (%s = 0; %s < %s; %s++)" % (keyvalue, keyvalue, array_size, keyvalue)
			print >> fobj, "    {"
			print >> fobj, "        %s = %s->%s[%s];" % (dbvalue, mst_name, tbl_field, keyvalue)
			print >> fobj, "        if (NULL == %s)" % (dbvalue)
			print >> fobj, "        {"
			print >> fobj, "            continue;"
			print >> fobj, "        }"
			print >> fobj, "        rc |= %s(%s, %s);" % (param1_value, dbvalue, param2_value)
			print >> fobj, "    }"
			print >> fobj, ""
			print >> fobj, "    return (rc) ? PM_E_FAIL : PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_get_master_implement(fobj, table, key_array, data_array, master_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	funcstr = "%s_get_master()" % (tbl_name)
	print >> fobj, "%s*" % (tbl_mst_type)
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s;" % (tbl_g_mst_name)
	print >> fobj, "}"
	print >> fobj, ""


def gen_tbl_func_key_val2str_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array, referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0
	need_listnode = 0
	pargs_value = 'pargs'
	node_value = 'p_node'
	field_value = 'p_field'
	fp_value = 'fp'
	tbl_info_value = 'p_tbl_info'
	ds_value = 'p_ds'
	bak_field_id = 'bak_field_id'
	listnode = 'listnode'
	funcstr = "%s_key_val2str(%s %s, char *str, uint32 str_len)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"

	print >> fobj, "    sal_memset(str, 0, str_len);"
	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]
		
		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
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
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			print tbl_name_upper
			print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
			print >> fobj, ""
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\", " % (field_name_string)
			print >> fobj, "            %s(%s, %s, %s));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
		elif ('UINT' == dt_type[1]):
			field_name_string = """%u"""
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\", %s);" % (field_name_string, param_str)
		elif ('UINT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\"PRIu64, %s);" % (field_name_string, param_str)
		elif ('INT' == dt_type[1]):
			field_name_string = """%d"""
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\", %s);" % (field_name_string, param_str)
		elif ('INT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\"PRId64, %s);" % (field_name_string, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\", %s);" % (field_name_string, param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "    sal_snprintf(str, str_len, \"%s\", cdb_enum_val2str(%s, %s, %s));" % (field_name_string, enum_type_str, enum_type_max, param_str)
		else:
			cdb_util.error_return("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_tbl_func_key_str2val_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0
	need_listnode = 0
	pargs_value = 'pargs'
	node_value = 'p_node'
	field_value = 'p_field'
	fp_value = 'fp'
	tbl_info_value = 'p_tbl_info'
	ds_value = 'p_ds'
	bak_field_id = 'bak_field_id'
	listnode = 'listnode'
	funcstr = "%s_key_str2val(char *str, %s %s)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if is_global:
		print >> fobj, "    return PM_E_NONE;"
		print >> fobj, "}"
		print >> fobj, ""
		return


	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
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
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			func_str2val = string.strip(func_val2str, "val2str")
			func_str2val = func_str2val + "str2val"
			val2str_func_array.append(func_val2str)
			print >> fobj, "    int32 ret = 0;"
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
			print >> fobj, "    return ret;"

		elif ('UINT' == dt_type[1]):
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, ""
			print >> fobj, "    %s = cdb_uint_str2val(str, &ret);" % (param_str)
			print >> fobj, "    return ret;"
		elif ('UINT64' == dt_type[1]):
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, ""
			print >> fobj, "    %s = cdb_uint64_str2val(str, &ret);" % (param_str)
			print >> fobj, "    return ret;"
		elif ('INT' == dt_type[1]):
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, ""
			print >> fobj, "    %s = cdb_int_str2val(str, &ret);" % (param_str)
			print >> fobj, "    return ret;"
		elif ('INT64' == dt_type[1]):
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, ""
			print >> fobj, "    %s = cdb_int64_str2val(str, &ret);" % (param_str)
			print >> fobj, "    return ret;"
		elif ('STRING' == dt_type[1]):
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, ""
			print >> fobj, "    sal_strcpy(%s, str);" % (param_str)
			print >> fobj, "    return ret;"
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "    int32 ret = 0;"
			print >> fobj, "    ret =  cdb_enum_str2val(%s, %s, str);" % (enum_type_str, enum_type_max)
			print >> fobj, "    return ret;"
		else:
			cdb_util.error_return("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_dump_one_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array, referlist_tbl_array, array_ds_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0
	need_listnode = 0
	pargs_value = 'pargs'
	node_value = 'p_node'
	field_value = 'p_field'
	fp_value = 'fp'
	tbl_info_value = 'p_tbl_info'
	ds_value = 'p_ds'
	bak_field_id = 'bak_field_id'
	listnode = 'listnode'
	funcstr = "%s_dump_one(%s %s, %s %s)" % (tbl_name, paramtype, '*' + paramvalue, 'tbl_iter_args_t', '*' + pargs_value)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"

	print >> fobj, "    cdb_node_t *%s = cdb_get_tbl(%s);" % (node_value, tbl_name_upper)
	print >> fobj, "    field_parse_t *%s = %s->argv[0];" % (field_value, pargs_value)
	print >> fobj, "    FILE *%s = %s->argv[1];" % (fp_value, pargs_value)
	print >> fobj, "    cdb_tbl_info_t *%s = %s->tbl_info;" % (tbl_info_value, node_value)
	print >> fobj, ""

	has_buf1 = cdb_util.check_dump_one_use_buf(table, key_array, field_array, dt_array, 0)
	has_buf2 = 0
	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	refer_object_array = []
	for i, referlist_field in enumerate(referlist_array):
		refer_object_info = cdb_util.get_refer_object_info(referlist_field)
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

	if (len(array_ds_array)):
		for i, array_ds in enumerate(array_ds_array):
			refer_object_type = string.lower(array_ds[0][6:]) + '_t'
			params = array_ds[1][:-1]
			params_array = params.split('[')
			refer_object_param = 'p_' + 'db_' + params_array[0]
			print >> fobj, "    %s *%s = NULL;" % (refer_object_type, refer_object_param)

	if (len(array_ds_array)):
		need_array_ds = 1
	else:
		need_array_ds = 0

	if (need_listnode):
		print >> fobj, "    ctclib_slistnode_t *%s = NULL;" % listnode
	if (need_array_ds):
		print >> fobj, "    uint32 i = 0;"
	if (need_listnode or need_array_ds):
		print >> fobj, ""

	if (has_buf1 or has_buf2):
		print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
		print >> fobj, ""

	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = paramvalue + '->' + data_name
		
		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])	
		field_id_enum = tbl_name_upper + '_FLD_' + string.upper(field_name)
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

		print >> fobj, "    if (FLD_MATCH(%s, %s->field_id[0]))" % (field_id_enum, field_value)
		print >> fobj, "    {"
		field_name_string = """/%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			if ('cdb_bitmap_val2str' == func_val2str or 'cdb_int32_array_val2str' == func_val2str or 'cdb_uint32_array_val2str' == func_val2str or 'cdb_uint64_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)/4));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
 			elif ('cdb_int8_array_val2str' == func_val2str or 'cdb_uint8_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_double_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s, sizeof(%s)/sizeof(double)));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str, param_str)
			elif ('cdb_string_array_val2str' == func_val2str):
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, (const char*)%s," % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
				print >> fobj, "            sizeof(%s[0]), sizeof(%s)/sizeof(%s[0])));" % (param_str, param_str, param_str)
			else:
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
				print >> fobj, "            %s(%s, %s, %s));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
		elif ('ARRAY_DS' in field[0]):
				name_upper = field[0][6:]
				name_lower = string.lower(name_upper)
				sub_field_id_enum = 'FLD_ID_ALL'
				params = field[1][:-1]
				params_array = params.split('[')
				refer_object_param = 'p_' + 'db_' + params_array[0]
				print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name);" % (field_name_string, '{', tbl_info_value, field_id_enum)
				print >> fobj, "        p_field->field_id[1] = %s;" % (sub_field_id_enum)
				print >> fobj, "        for (i = 0; i < %s; i++)" % (params_array[1])
				print >> fobj, "        {"
				print >> fobj, "            %s = &(%s->%s[i]);" % (refer_object_param, paramvalue, params_array[0])
				print >> fobj, "            %s_dump_one(%s, %s);" % (name_lower, refer_object_param, pargs_value)
				print >> fobj, "            sal_fprintf(fp, \"/,\");"
				print >> fobj, "        }"
				print >> fobj, "        sal_fprintf(fp, \"%s\");" % ('}')

		elif ('UINT' == dt_type[1] or 'INT' == dt_type[1] ):
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name," % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('UINT64' == dt_type[1] or 'INT64' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%%\"%s, %s->field[%s].name," % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name, " % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            cdb_enum_val2str(%s, %s, %s));" % (enum_type_str, enum_type_max, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name," % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name," % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            GLB_FLAG_ISSET(%s, %s));" % (param_str, field[4])
		elif ('DS_' in dt_type[1]):
			ds_name_upper = string.upper(field[0])
			ds_name_upper = ds_name_upper.rstrip('_T')
			ds_entry = ds_find_ds(ds_array, ds_name_upper)
			ds_name_lower = string.lower(ds_name_upper)
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name);" % (field_name_string, '{', tbl_info_value, field_id_enum)
			print >> fobj, "        if (%s)" % (param_str)
			print >> fobj, "        {"
			print >> fobj, "            %s_dump_one(%s, %s);" % (ds_name_lower, param_str, pargs_value)
			print >> fobj, "        }"
			print >> fobj, "        sal_fprintf(fp, \"%s\");" % ('}')
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
				name_upper = refer_object_info[2]
				referlist_tbl_array.append(name_upper)
				param_str = '&' + param_str
				func_val2str = 'cdb_reference_list_val2str'
				val2str_func_array.append(func_val2str)
				print >> fobj, "        sal_fprintf(fp, \"/%s\"CMD_EQUAL_STR\"%s\","
				print >> fobj, "            %s->tbl_info->field[%s].name," % (node_value, field_id_enum)
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
			print >> fobj, "        sal_fprintf(fp, \"%s\"CMD_EQUAL_STR\"%s\", %s->field[%s].name," % (field_name_string, field_value_string, tbl_info_value, field_id_enum)
			print >> fobj, "            %s);" % (param_str)
		else:
			cdb_util.error_return("unhandled field in TBL dump_one: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"
	print >> fobj, ""

	print >> fobj, "    sal_fprintf(%s, \"\\n\");" % (fp_value)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_tbl_func_key_name_dump_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name

	funcstr = "%s_key_name_dump(%s %s, char *str)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"

	print >> fobj, "    cdb_node_t *p_node = cdb_get_tbl(%s);" % (tbl_name_upper)
	if (is_global):
		print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR\"0\","
		print >> fobj, "        p_node->name);"
		print >> fobj, "    return str;"
		print >> fobj, "}"
		print >> fobj, ""
		return

	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
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
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

		field_name_string = """%s"""
		if (None != dt_type[2]):
			field_value_string = dt_type[2][2]

		if (has_val2str):
			func_val2str = dt_type[2][4]
			val2str_func_array.append(func_val2str)
			print tbl_name_upper
			print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
			print >> fobj, ""
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR\"%s\", p_node->name, "
			print >> fobj, "            %s(%s, %s, %s));" % (func_val2str, CONST.BUF, CONST.BUF_SIZE, param_str)
		elif ('UINT' == dt_type[1]):
			field_name_string = """%u"""
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR"
			print >> fobj, "          \"%s\", p_node->name, %s);" % (field_name_string, param_str)
		elif ('UINT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR"
			print >> fobj, "        \"%s\"PRIu64, p_node->name, %s);" % (field_name_string, param_str)
		elif ('INT' == dt_type[1]):
			field_name_string = """%d"""
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR"
			print >> fobj, "        \"%s\", p_node->name, %s);" % (field_name_string, param_str)
		elif ('INT64' == dt_type[1]):
			field_name_string = """%"""
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR"
			print >> fobj, "     \"%s\"PRId64, %s);" % (field_name_string, param_str)
		elif ('STRING' == dt_type[1]):
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR\"%s\", p_node->name, "
			print >> fobj, "        %s);" % (param_str)
		elif ('ENUM8' == dt_type[1] or 'ENUM32' == dt_type[1]):
			enum_type = dt_type[0].rstrip('t')
			enum_type = enum_type.rstrip('_')
			enum_type_str = enum_type + '_strs'
			enum_type_max = string.upper(enum_type) + '_MAX'
			print >> fobj, "    sal_sprintf(str, \"%s\"KEY_CONNECT_STR"
			print >> fobj, "        \"%s\", p_node->name, cdb_enum_val2str(%s, %s, %s));" % (
			field_name_string, enum_type_str, enum_type_max, param_str)
		else:
			cdb_util.error_return("unhandled field in TBL key_val2str: %s, dt_type: %s", field, dt_type[1])
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_key_value_dump_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array, is_global=0):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0

	funcstr = "%s_key_value_dump(%s %s, char *str)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	if (is_global):
		print >> fobj, ""
		print >> fobj, "    sal_sprintf(str, \"%s\", \"0\");"
		print >> fobj, "    return str;"
		print >> fobj, "}"
		print >> fobj, ""
		return

	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
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
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

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

def gen_tbl_func_field_name_dump_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	funcstr = "%s_field_name_dump(%s %s, int32 field_id, char *str)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, ""
	print >> fobj, "    cdb_node_t *p_node = cdb_get_tbl(%s);" % (tbl_name_upper)
	print >> fobj, "    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;"
	print >> fobj, "    if (field_id >= %s_FLD_MAX)" % (tbl_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    sal_sprintf(str, \"%s\", p_tbl_info->field[field_id].name);"
	print >> fobj, ""
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_field_value_dump_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0
	need_listnode = 0
	pargs_value = 'pargs'

	funcstr = "%s_field_value_dump(%s %s, int32 field_id, char* str)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"

	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = paramvalue + '->' + data_name

		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = tbl_name_upper + '_FLD_' + string.upper(field_name)
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

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
			print >> fobj, "        sal_sprintf(str, \"%s\", cdb_enum_val2str(%s, %s, %s));" % (
			field_name_string, enum_type_str, enum_type_max, param_str)
		elif ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			print >> fobj, "        sal_sprintf(str, \"%s\"," % (field_value_string)
			print >> fobj, "            GLB_FLAG_ISSET(%s, %s));" % (param_str, field[4])
		elif ('DOUBLE' == dt_type[1]):
			print >> fobj, "        sal_sprintf(str, \"%s\"," % (field_value_string)
			print >> fobj, "            %s);" % (param_str)
		else:
			print("unhandled field in TBL field value dump: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"

	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_table_dump_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]

	funcstr = "%s_table_dump(%s %s, char **str)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "char**"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    char buf[MAX_CMD_STR_LEN];"
	print >> fobj, "    int i = 0, j= 0;"
	print >> fobj, ""
	print >> fobj, "    sal_sprintf(str[j++], \"%s\","
	print >> fobj, "         %s_key_name_dump(%s, buf));" % (tbl_name, paramvalue)
	print >> fobj, "    for(i=1; i<%s_FLD_MAX; i++)" % (tbl_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        sal_sprintf(str[j++], \"%s\","
	print >> fobj, "            %s_field_name_dump(%s, i, buf));" % (tbl_name, paramvalue)
	print >> fobj, "        sal_sprintf(str[j++], \"%s\","
	print >> fobj, "            %s_field_value_dump(%s, i, buf));" % (tbl_name, paramvalue)
	print >> fobj, "    }"
	print >> fobj, "    return str;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_field_value_parser_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name
	use_buf = 0
	need_listnode = 0
	pargs_value = 'pargs'


	funcstr = "%s_field_value_parser(char *str, int32 field_id, %s %s)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"


	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('key' == field[1]):
			key = key_array[0]
			key_str = 'key'
			dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
			if ('' != key[1]):
				data_name = cdb_util.format_field_name(key[1])
				param_str = paramvalue + '->' + key_str + '.' + data_name
			else:
				param_str = paramvalue + '->' + key_str
		else:
			data_name = cdb_util.format_field_name(field[1])
			param_str = paramvalue + '->' + data_name

		has_val2str = cdb_util.dt_type_has_val2str(dt_type)
		if (has_val2str):
			if (6 == len(dt_type[2])):
				param_str = '&' + param_str

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_id_enum = tbl_name_upper + '_FLD_' + string.upper(field_name)
		db_field_value = dbvalue + '->' + field_name
		param_field_value = paramvalue + '->' + field_name

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
				print >> fobj, "        int32 ret;"
				print >> fobj, "        p_field->field_id[1] = %s;" % (sub_field_id_enum)
				print >> fobj, "        for (i = 0; i < %s; i++)" % (params_array[1])
				print >> fobj, "        {"
				print >> fobj, "            %s = &(%s->%s[i]);" % (refer_object_param, paramvalue, params_array[0])
				print >> fobj, "            %s_dump_one(%s, %s);" % (name_lower, refer_object_param, pargs_value)
				print >> fobj, "            sal_sprintf(str, \"/,\");"
				print >> fobj, "        }"
				print >> fobj, "        sal_sprintf(str, \"%s\");" % ('}')
				print >> fobj, ""
				print >> fobj, "        %s = cdb_uint_str2val(str, &ret);" % (param_str)
		elif ('UINT64' == dt_type[1]):
			print >> fobj, "        int32 ret;"
			print >> fobj, "        %s = cdb_uint64_str2val(str, &ret);" % (param_str)
		elif ('UINT' == dt_type[1]):
			print >> fobj, "        int32 ret;"
			print >> fobj, "        %s = cdb_uint_str2val(str, &ret);" % (param_str)
		elif ('INT64' == dt_type[1]):
			print >> fobj, "        int32 ret;"
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
			print >> fobj, "        int32 ret;"
			print >> fobj, "        %s = cdb_double_str2val(str, &ret);" % (param_str)
		else:
			print("unhandled field in TBL field value parser: %s, dt_type: %s", field, dt_type[1])
		print >> fobj, "    }"

	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_table_parser_implement(fobj, table, key_array, field_array, ds_array, dt_array, val2str_func_array,
									   referlist_tbl_array):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name


	funcstr = "%s_table_parser(char** array, char* key_value,%s %s)" % (tbl_name, paramtype, '*' + paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    int32 rc;"
	print >> fobj, "    int i = 0, j= 0;"
	print >> fobj, "    cdb_node_t *p_node = cdb_get_tbl(%s);" % (tbl_name_upper)
	print >> fobj, "    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;"
	print >> fobj, ""
	print >> fobj, "    PM_E_RETURN(%s_key_str2val(key_value, %s));" % (tbl_name, paramvalue)
	print >> fobj, ""
	print >> fobj, "    for(i=1; i<%s_FLD_MAX; i++)" % (tbl_name_upper)
	print >> fobj, "    {"
	print >> fobj, "        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))"
	print >> fobj, "        {"
	print >> fobj, "             return PM_E_INVALID_PARAM;"
	print >> fobj, "        }"
	print >> fobj, "        PM_E_RETURN(%s_field_value_parser( array[j++], i, %s));" % (tbl_name, paramvalue)
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_init_implement(fobj, table, master_array, cmp_func_array, is_global=0):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	tbl_hash_name = tbl_value_name + '_hash'
	mem_type = 'MEM_TBL_MASTER'
	if (is_global):
		tbl_mst_type = tbl_name + '_t'
		tbl_g_mst_name = '_g_p_' + tbl_name
		mem_type = 'MEM_' + table[0]
	accusative = tbl_name[4:]
	funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, 'init', accusative, '', '')
	print >> fobj, "%s*" % (tbl_mst_type)
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    %s = XCALLOC(%s, sizeof(%s));" % (tbl_g_mst_name, mem_type, tbl_mst_type)
	for i, master_field in enumerate(master_array):
		gen_tbl_master_field(fobj, tbl_g_mst_name, tbl_name, master_field, cmp_func_array)
	print >> fobj, "    return %s;" % (tbl_g_mst_name)
	print >> fobj, "}"
	print >> fobj, ""


def gen_tbl_func_set_refer_list_func(fobj, table, key_array, field_array, referlist_field):
	tbl_db_type = table[2]
	tbl_name_upper = table[0]
	tbl_name = string.lower(table[0])
	mem_type = 'MEM_' + table[0]
	tbl_value_name = table[1]
	tbl_mst_type = tbl_name + '_master_t'
	tbl_g_mst_name = '_g_p_' + tbl_name + '_master'
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	paramtype = tbl_type
	paramvalue = 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	mst_name = 'p_master'
	dbvalue = 'p_db_' + tbl_value_name

	object_info = cdb_util.get_refer_object_info(referlist_field)
	if ('CDB_NODE_DS' == object_info[1]):
		return
	field_name = object_info[0]
	refer_tbl_name = referlist_field[0][10:]
	full_field_name = tbl_name_upper + '_FLD_' + string.upper(field_name)

	is_add = 'add'
	p_object = 'p_object'
	funcstr = "%s_set_%s_sync(%s %s, %s %s, %s %s, uint32 sync)" % (tbl_name, field_name, tbl_type, '*' + paramvalue, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    cdb_sync_refer_hdr_t refer;"
	print >> fobj, "    %s *%s = NULL;" % (tbl_type, dbvalue)
	print >> fobj, "    int32 rc = PM_E_NONE;"
	print >> fobj, ""
	print >> fobj, "    /* 1. lookup entry exist */"
	print >> fobj, "    %s = %s_get_%s(&%s->key);" % (dbvalue, tbl_name, accusative, paramvalue)
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
	print >> fobj, "        cdb_sync_tbl_set_refer(%s, %s, %s, &refer, %s);" % (table[0], full_field_name, paramvalue, p_object)
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

	funcstr = "%s_set_%s(%s %s, %s %s, %s %s)" % (tbl_name, field_name, tbl_type, '*' + paramvalue, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s" % (funcstr)
	print >> fobj, "{"
	print >> fobj, "    return %s_set_%s_sync(%s, %s, %s, TRUE);" % (tbl_name, field_name, paramvalue, is_add, p_object)
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_static_hash_make(fobj, table, key, master_array):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	tbl_hash_name = tbl_value_name + '_hash'
	paramvalue = 'p_' + tbl_value_name
	tbl_hash_func_make = '_' + tbl_name + '_hash_make'

	print >> fobj, "static uint32"
	print >> fobj, "%s(%s *%s)" % (tbl_hash_func_make, tbl_type, paramvalue)
	print >> fobj, "{"
	print >> fobj, "    uint32 index = 0;"
	print >> fobj, "    uint32 key = 0;"
	print >> fobj, "    uint8 *pval = NULL;"
	print >> fobj, ""
	print >> fobj, "    pval = (uint8*)&%s->key;" % (paramvalue)
	print >> fobj, "    for (index = 0; index < sizeof(%s->key); index++)" % (paramvalue)
	print >> fobj, "    {"
	print >> fobj, "        key += pval[index];"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return key;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_func_static_hash_cmp(fobj, table, key, master_array):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_type = tbl_name + '_t'
	tbl_key_type = tbl_name + '_key_t'
	tbl_hash_name = tbl_value_name + '_hash'
	paramvalue = 'p_' + tbl_value_name
	tbl_hash_func_cmp = '_' + tbl_name + '_hash_cmp'
	print >> fobj, "static bool"
	print >> fobj, "%s(void *p_arg1, void *p_arg2)" % (tbl_hash_func_cmp)
	print >> fobj, "{"
	print >> fobj, "    %s *%s1 = (%s*)p_arg1;" % (tbl_type, paramvalue, tbl_type)
	print >> fobj, "    %s *%s2 = (%s*)p_arg2;" % (tbl_type, paramvalue, tbl_type)
	print >> fobj, ""
	print >> fobj, "    if (0 == sal_memcmp(&%s1->key, &%s2->key, sizeof(%s)))" % (paramvalue, paramvalue, tbl_key_type)
	print >> fobj, "    {"
	print >> fobj, "        return TRUE;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return FALSE;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_tbl_c_one(table, key_array, data_array, master_array, field_array, ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array):
	'''
	print "######11111111111"
	print table
	print "######22222222222"
	print key_array
	print "########333333333"
	print data_array
	print "#######444444444"
	print master_array
	print "#######55555555"
	print field_array
	print "#######666666"
	print ds_array
	print "#########777777"
	print dt_array
	print "########8888888888"
	print cmp_func_array
	print "#########999999"
	print val2str_func_array
	print "########1010101010"
	print referlist_tbl_array
	'''
	tbl_db_type = table[2]
	tbl_name = table[0]
	fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
	fname_h = 'gen/' + string.lower(tbl_name) + '.h'
	fname = CONST.CDB_C_PATH + string.lower(tbl_name) + '.c'
	file_tbl = open(fname, 'w')
	is_global = 0

	print >> file_tbl, ""
	print >> file_tbl, "#include \"%s\"" % (CONST.PROTO_H_FILE)
	cdb_util.include_tailq_head_h_file(file_tbl, data_array, tbl_name) # add for tailq_head field
	print >> file_tbl, "#include \"%s\"" % (fname_define_h)
	print >> file_tbl, "#include \"%s\"" % (fname_h)
	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	cdb_util.include_refer_tbl_ds_h_file(file_tbl, referlist_array, tbl_name)
	array_ds_array = cdb_util.has_cdb_array_ds(field_array)
	cdb_util.include_array_tbl_ds_h_file(file_tbl, array_ds_array, tbl_name)
	print >> file_tbl, "#include \"%s\"" % (CONST.CDB_CMP_H_FILE)
	print >> file_tbl, ""

	if ('GLOBAL' == tbl_db_type):
		is_global = 1
	gen_tbl_master_value(file_tbl, table, is_global)

	cdb_util.insert_tbl_user_define_begin(file_tbl, tbl_name, '.c')

	if ('HASH' == tbl_db_type or 'HASH+LIST' == tbl_db_type):
		key = key_array[0]
		gen_tbl_func_static_hash_make(file_tbl, table, key, master_array)
		gen_tbl_func_static_hash_cmp(file_tbl, table, key, master_array)

	if (is_global):
		gen_tbl_func_set_field_sync_implement(file_tbl, table, key_array, field_array, is_global)
		gen_tbl_func_set_field_implement(file_tbl, table, key_array, field_array, is_global)
		gen_tbl_func_get_implement(file_tbl, table, key_array, master_array, is_global)
		gen_tbl_func_key_str2val_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
										   val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_key_name_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_key_value_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_field_name_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array)
		gen_tbl_func_field_value_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											   val2str_func_array, referlist_tbl_array)
		gen_tbl_func_table_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											   val2str_func_array, referlist_tbl_array)
		gen_tbl_func_field_value_parser_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
										  val2str_func_array, referlist_tbl_array)
		gen_tbl_func_table_parser_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											val2str_func_array, referlist_tbl_array)
		gen_tbl_func_dump_one_implement(file_tbl, table, key_array, field_array, ds_array, dt_array, val2str_func_array, referlist_tbl_array, array_ds_array)
		gen_tbl_func_iterate_implement(file_tbl, table, key_array, data_array, master_array, is_global)
		gen_tbl_func_init_implement(file_tbl, table, master_array, cmp_func_array, is_global)
	else:
		gen_tbl_func_add_sync_implement(file_tbl, table, key_array, data_array, field_array, master_array, cmp_func_array, array_ds_array)
		gen_tbl_func_del_sync_implement(file_tbl, table, key_array, field_array, master_array)
		gen_tbl_func_set_field_sync_implement(file_tbl, table, key_array, field_array)
		gen_tbl_func_add_implement(file_tbl, table, key_array, field_array, is_global)
		gen_tbl_func_del_implement(file_tbl, table, key_array, field_array, is_global)
		gen_tbl_func_set_field_implement(file_tbl, table, key_array, field_array, is_global)
		gen_tbl_func_get_implement(file_tbl, table, key_array, master_array)
		gen_tbl_func_key_val2str_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
										   val2str_func_array, referlist_tbl_array)
		gen_tbl_func_key_str2val_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
										   val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_key_name_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_key_value_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array, is_global)
		gen_tbl_func_field_name_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											 val2str_func_array, referlist_tbl_array)
		gen_tbl_func_field_value_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											   val2str_func_array, referlist_tbl_array)
		gen_tbl_func_table_dump_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											   val2str_func_array, referlist_tbl_array)
		gen_tbl_func_field_value_parser_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
										  val2str_func_array, referlist_tbl_array)
		gen_tbl_func_table_parser_implement(file_tbl, table, key_array, field_array, ds_array, dt_array,
											val2str_func_array, referlist_tbl_array)
		gen_tbl_func_dump_one_implement(file_tbl, table, key_array, field_array, ds_array, dt_array, val2str_func_array, referlist_tbl_array, array_ds_array)
		gen_tbl_func_iterate_implement(file_tbl, table, key_array, data_array, master_array)
		gen_tbl_func_get_master_implement(file_tbl, table, key_array, data_array, master_array)
		gen_tbl_func_init_implement(file_tbl, table, master_array, cmp_func_array)

	for i, referlist_field in enumerate(referlist_array):
		gen_tbl_func_set_refer_list_func(file_tbl, table, key_array, field_array, referlist_field)

	cdb_util.insert_tbl_user_define_end(file_tbl, tbl_name, '.c')
	file_tbl.close()
	print "generate file %s" % (fname)

def gen_tbl_c_files(tbl_array, ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array):
	'generate table api c files'
	for i, tbl_one in enumerate(tbl_array):
		if (tbl_one[0][0] in bypass_tbl_files):
			print "Bypass table %s" % (tbl_one[0][0])
			continue
		gen_tbl_c_one(tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array)
