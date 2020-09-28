import os
import sys
import string
import CONST
import cdb_util

def gen_tbl_set_field_func_prototype(fobj, table, suffix=''):
	suffix_str = ''
	if ('_sync' == suffix):
		suffix_str = ', uint32 sync'
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	field_id_type = tbl_name + '_field_id_t'
	tbl_type = tbl_name + '_t'
	accusative = tbl_name[4:]
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_set_%s_field%s(%s %s, %s %s%s)" % (tbl_name, accusative, suffix, tbl_type, paramvalue, field_id_type, 'field_id', suffix_str)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_key_val2str_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_key_val2str(%s %s, char *str, uint32 str_len)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_key_str2val_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_key_str2val(char *str, %s %s)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_dump_one_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_dump_one(%s %s, %s %s)" % (tbl_name, paramtype, paramvalue, 'tbl_iter_args_t', '*pargs')
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_key_name_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_key_name_dump(%s %s, char* str)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_key_value_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_key_value_dump(%s %s, char* str)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_field_name_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_field_name_dump(%s %s, int32 field_id, char* str)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_field_value_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_field_value_dump(%s %s, int32 field_id, char* str)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_table_dump_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_table_dump(%s %s, char** str)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "char**"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_table_parser_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	paramtype = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	funcstr = "%s_table_parser(char** array, char* key_value, %s %s)" % (tbl_name, paramtype, paramvalue)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_iterate_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	funcstr = "%s_iterate(%s %s, %s %s)" % (tbl_name, 'TBL_ITER_CB_FUNC', 'fn', 'tbl_iter_args_t', '*pargs')
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_get_master_func_prototype(fobj, table):
	tbl_name = string.lower(table[0])
	tbl_mst_type = tbl_name + '_master_t'
	funcstr = "%s_get_master()" % (tbl_name)
	print >> fobj, "%s*" % (tbl_mst_type)
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_func_prototype(fobj, tbl_name, action, accusative, paramtype, paramvalue, tbl_db_type, suffix):
	if ('init' == action):
		if ('GLOBAL' == tbl_db_type):
			tbl_mst_type = tbl_name + '_t'
		else:
			tbl_mst_type = tbl_name + '_master_t'
		print >> fobj, "%s*" % (tbl_mst_type)
	elif ('get' == action):
		tbl_type = tbl_name + '_t'
		print >> fobj, "%s*" % (tbl_type)
	else:
		print >> fobj, "int32"
	funcstr = cdb_util.get_tbl_api_func_prototype(tbl_name, action, accusative, paramtype, paramvalue, suffix)
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_basic_func_prototype(fobj, table, action, param, suffix=''):
	tbl_name = string.lower(table[0])
	tbl_value_name = table[1]
	tbl_db_type = table[2]
	tbl_type = tbl_name + '_t'
	tbl_key_value_name = table[1] + '_key'
	tbl_key_type = tbl_name + '_key' + '_t'
	accusative = tbl_name[4:]
	if (CONST.PARAM_TYPE_TBL == param):
		paramtype = tbl_key_type
		paramvalue = '*' + 'p_' + tbl_key_value_name
	elif (CONST.PARAM_TYPE_TBL_KEY == param):
		paramtype = tbl_type
		paramvalue = '*' + 'p_' + tbl_value_name
	else:
		paramtype = ''
		paramvalue = ''
	gen_tbl_func_prototype(fobj, tbl_name, action, accusative, paramtype, paramvalue, tbl_db_type, suffix)


def gen_tbl_set_refer_list_func_prototype(fobj, table, refer_list_field):
	""" Only table need set refer_list"""
	tbl_name_upper = table[0]
	tbl_name = string.lower(tbl_name_upper)
	tbl_value_name = table[1]
	tbl_db_type = table[2]
	tbl_type = tbl_name + '_t'
	paramvalue = '*' + 'p_' + tbl_value_name
	accusative = tbl_name[4:]
	object_info = cdb_util.get_refer_object_info(refer_list_field)
	if ('CDB_NODE_DS' == object_info[1]):
		return
	field_name = object_info[0]
	is_add = 'add'
	p_object = 'p_object'

	funcstr = "%s_set_%s_sync(%s %s, %s %s, %s %s, uint32 sync)" % (tbl_name, field_name, tbl_type, paramvalue, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

	funcstr = "%s_set_%s(%s %s, %s %s, %s %s)" % (tbl_name, field_name, tbl_type, paramvalue, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_h_one(table, data_array, field_array):

	tbl_name = table[0]
	tbl_db_type = table[2]
	fname = CONST.CDB_H_PATH + string.lower(tbl_name) + '.h'
	file_tbl = open(fname, 'w')
	_macro_h_ = "__" + tbl_name + "_H__"

	print >> file_tbl, ""
	print >> file_tbl, "#ifndef %s" % (_macro_h_)
	print >> file_tbl, "#define %s" % (_macro_h_)
	print >> file_tbl, ""

	cdb_util.build_sub_ds_include(file_tbl, tbl_name, field_array, 0)

	cdb_util.insert_tbl_user_define_begin(file_tbl, tbl_name, '.h')

	if ('GLOBAL' == tbl_db_type):
		gen_tbl_set_field_func_prototype(file_tbl, table, "_sync")
		gen_tbl_set_field_func_prototype(file_tbl, table)
		gen_tbl_basic_func_prototype(file_tbl, table, 'get', CONST.PARAM_TYPE_NONE)
		gen_tbl_key_val2str_func_prototype(file_tbl, table)
		gen_tbl_key_str2val_func_prototype(file_tbl, table)
		gen_tbl_dump_one_func_prototype(file_tbl, table)
		gen_tbl_key_name_func_prototype(file_tbl, table)
		gen_tbl_key_value_func_prototype(file_tbl, table)
		gen_tbl_field_name_func_prototype(file_tbl, table)
		gen_tbl_field_value_func_prototype(file_tbl, table)
		gen_tbl_table_dump_func_prototype(file_tbl, table)
		gen_tbl_table_parser_func_prototype(file_tbl, table)
		gen_tbl_iterate_func_prototype(file_tbl, table)
		gen_tbl_iterate_func_prototype(file_tbl, table)
		gen_tbl_basic_func_prototype(file_tbl, table, 'init', CONST.PARAM_TYPE_NONE)
	else:
		gen_tbl_basic_func_prototype(file_tbl, table, 'add', CONST.PARAM_TYPE_TBL_KEY, "_sync")
		gen_tbl_basic_func_prototype(file_tbl, table, 'del', CONST.PARAM_TYPE_TBL, "_sync")
		gen_tbl_set_field_func_prototype(file_tbl, table, "_sync")
		gen_tbl_basic_func_prototype(file_tbl, table, 'add', CONST.PARAM_TYPE_TBL_KEY)
		gen_tbl_basic_func_prototype(file_tbl, table, 'del', CONST.PARAM_TYPE_TBL)
		gen_tbl_set_field_func_prototype(file_tbl, table)
		gen_tbl_basic_func_prototype(file_tbl, table, 'get', CONST.PARAM_TYPE_TBL)
		gen_tbl_key_val2str_func_prototype(file_tbl, table)
		gen_tbl_key_str2val_func_prototype(file_tbl, table)
		gen_tbl_dump_one_func_prototype(file_tbl, table)
		gen_tbl_key_name_func_prototype(file_tbl, table)
		gen_tbl_key_value_func_prototype(file_tbl, table)
		gen_tbl_field_name_func_prototype(file_tbl, table)
		gen_tbl_field_value_func_prototype(file_tbl, table)
		gen_tbl_table_dump_func_prototype(file_tbl, table)
		gen_tbl_table_parser_func_prototype(file_tbl, table)
		gen_tbl_iterate_func_prototype(file_tbl, table)
		gen_tbl_get_master_func_prototype(file_tbl, table)
		gen_tbl_basic_func_prototype(file_tbl, table, 'init', CONST.PARAM_TYPE_NONE, "")

	referlist_array = cdb_util.has_cdb_refer_list(field_array)
	for i, referlist_field in enumerate(referlist_array):
		gen_tbl_set_refer_list_func_prototype(file_tbl, table, referlist_field)

	cdb_util.insert_tbl_user_define_end(file_tbl, tbl_name, '.h')
	print >> file_tbl, "#endif /* !%s */" % (_macro_h_)
	print >> file_tbl, ""
	file_tbl.close()
	print "generate file %s" % (fname)

def gen_tbl_h_files(tbl_array):
	'generate table structure header files'
	for i, tbl_one in enumerate(tbl_array):
		gen_tbl_h_one(tbl_one[0], tbl_one[2], tbl_one[5])

