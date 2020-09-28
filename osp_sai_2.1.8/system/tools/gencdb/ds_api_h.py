import os
import sys
import string
import CONST
import cdb_util

def gen_ds_add_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	suffix_str = ''
	if ('_sync' == suffix):
		suffix_str = ', uint32 sync'
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	funcstr = "%s_add_%s%s(%s %s, %s%s %s%s)" % (ds_name, accusative, suffix, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name, suffix_str)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_del_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	suffix_str = ''
	if ('_sync' == suffix):
		suffix_str = ', uint32 sync'
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	if (0 == len(key_array)):
		funcstr = "%s_del_%s%s(%s %s%s)" % (ds_name, accusative, suffix, tbl_type, '*' + tbl_value_name, suffix_str)
	else:
		funcstr = "%s_del_%s%s(%s %s, %s%s %s%s)" % (ds_name, accusative, suffix, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name, suffix_str)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_set_func_prototype(fobj, ds, parent_table, key_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	funcstr = "%s_set_%s(%s %s, %s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_set_field_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	suffix_str = ''
	if ('_sync' == suffix):
		suffix_str = ', uint32 sync'
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_set_%s_field%s(%s %s, %s%s %s, %s %s%s)" % (ds_name, accusative, suffix, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name, ds_field_type, 'field_id', suffix_str)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_key_str2val_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_key_str2val(char* tbl_key_value, char* ds_key_value, %s %s, %s %s)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_key_name_dump_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_key_name_dump(%s %s, %s %s, char* str)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_key_value_dump_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_key_value_dump(%s %s, %s %s, char* str)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_field_name_dump_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_field_name_dump(%s %s, %s %s, int32 field_id, char* str)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_field_value_dump_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_field_value_dump(%s %s, %s %s, int32 field_id, char* str)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char*"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_table_dump_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_table_dump(%s %s, %s %s, char** str)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "char**"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_field_value_parser_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_field_value_parser(char* str, int32 field_id, %s %s, %s %s)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_table_parser_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array, suffix=''):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	ds_field_type = ds_name + '_field_id_t'
	funcstr = "%s_table_parser(char** array, char* tbl_key_value, char* ds_key_value, %s %s, %s %s)" % \
			  (ds_name,  tbl_type, '*' + tbl_value_name,  ds_type, '*' + ds_value_name)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_get_func_prototype(fobj, ds, parent_table, parent_ds_param_str, key_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	accusative = ds_name[3:]
	if (0 == len(key_array)):
		funcstr = "%s_get_%s(%s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name)
	else:
		funcstr = "%s_get_%s(%s %s, %s%s %s)" % (ds_name, accusative, tbl_type, '*' + tbl_value_name, parent_ds_param_str, ds_type, '*' + ds_value_name)
	print >> fobj, "%s*" % (ds_type)
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_tbl_dump_one_func_prototype(fobj, ds, parent_table, key_array):
	tbl_name = string.lower(parent_table[0])
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	pargs_value = 'pargs'
	funcstr = "%s_dump_one(%s %s, %s %s)" % (ds_name, ds_type, '*' + ds_value_name, 'tbl_iter_args_t', '*' + pargs_value)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

def gen_ds_set_refer_list_func_prototype(fobj, ds, parent_table, key_array, refer_list_field):
	""" Only table need set refer_list"""
	tbl_name_upper = parent_table[0]
	tbl_name = string.lower(tbl_name_upper)
	tbl_type = tbl_name + '_t'
	tbl_value_name = 'p_' + parent_table[1]
	ds_name = string.lower(ds[0])
	ds_type = ds_name + '_t'
	ds_value_name = 'p_' + ds[1]
	dbvalue = 'p_db_' + ds[1]
	ds_field_type = ds_name + '_field_id_t'
	accusative = ds_name[3:]
	object_info = cdb_util.get_refer_object_info(refer_list_field)
	if ('CDB_NODE_DS' == object_info[1]):
		return
	field_name = object_info[0]
	is_add = 'add'
	p_object = 'p_object'

	funcstr = "%s_set_%s_sync(%s %s, %s %s, %s %s, %s %s, uint32 sync)" % (ds_name, field_name, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""

	funcstr = "%s_set_%s(%s %s, %s %s, %s %s, %s %s)" % (ds_name, field_name, tbl_type, '*' + tbl_value_name, ds_type, '*' + ds_value_name, 'uint32', is_add, 'void', '*' + p_object)
	print >> fobj, "int32"
	print >> fobj, "%s;" % (funcstr)
	print >> fobj, ""


def gen_ds_h_one(ds, key_array, data_array, parent_tbl, ds_array):
	ds_name = ds[0]
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	tbl_db_type = ds[2]
	parent_table = parent_tbl[0]
	parent_ds = None
	parent_ds_param_str = ''
	fname = CONST.CDB_H_PATH + string.lower(ds_name) + '.h'
	file_tbl = open(fname, 'w')
	_macro_h_ = "__" + ds_name + "_H__"

	print >> file_tbl, ""
	print >> file_tbl, "#ifndef %s" % (_macro_h_)
	print >> file_tbl, "#define %s" % (_macro_h_)
	print >> file_tbl, ""

	cdb_util.build_ds_sub_ds_include(file_tbl, ds_name, data_array, 0)

	if (4 == len(ds)):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		parent_ds = parent_ds[0]
		parent_ds_name = string.lower(parent_ds[0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value_name = 'p_' + parent_ds[1]
		parent_ds_param_str = parent_ds_type + ' *' + parent_ds_value_name + ', '

	if (0 == is_array_ds):
		gen_ds_add_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array, "_sync")
		gen_ds_del_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array, "_sync")
	gen_ds_set_field_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array, "_sync")
	if (0 == is_array_ds):
		gen_ds_add_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
		gen_ds_del_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)

	gen_ds_set_field_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_get_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_key_str2val_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_key_name_dump_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_key_value_dump_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_field_name_dump_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_field_value_dump_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_table_dump_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_field_value_parser_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_ds_table_parser_func_prototype(file_tbl, ds, parent_table, parent_ds_param_str, key_array)
	gen_tbl_dump_one_func_prototype(file_tbl, ds, parent_table, key_array)
	referlist_array = cdb_util.has_cdb_refer_list(data_array)
	for i, referlist_field in enumerate(referlist_array):
		gen_ds_set_refer_list_func_prototype(file_tbl, ds, parent_table, key_array, referlist_field)

	cdb_util.insert_tbl_user_define_end(file_tbl, ds_name, '.h')
	print >> file_tbl, "#endif /* !%s */" % (_macro_h_)
	print >> file_tbl, ""
	file_tbl.close()
	print "generate file %s" % (fname)

def gen_ds_h_files(ds_array):
	'generate ds structure header files'
	for i, ds_one in enumerate(ds_array):
		gen_ds_h_one(ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_array)

