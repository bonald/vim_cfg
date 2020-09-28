#!/usr/bin/python
import os
import sys
import string
import CONST
import cdb_util

trace_on = 0
if (len(sys.argv) > 1):
	if ('-trace' == sys.argv[1]):
		trace_on = 1

GEN_STORE_TBL		= 1
GEN_STORE_DS		= 1
GEN_FORMAT_TBL		= 1
GEN_FORMAT_DS		= 1

filename_input_tbl  = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds   = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_dt   = CONST.CDB_INPUT_PATH + 'datatype.txt'
filename_output_tbl = CONST.CDB_C_PATH + 'cdb_store_tbl.c'
filename_output_ds  = CONST.CDB_C_PATH + 'cdb_store_ds.c'
filename_output_format_tbl = CONST.CDB_C_PATH + 'cdb_format_tbl.c'
filename_output_format_ds  = CONST.CDB_C_PATH + 'cdb_format_ds.c'

tbl_array = []
ds_array = []
dt_oper_array = []
dt_array = []
cmp_func_array = []
val2str_func_array = []
referlist_tbl_array = []

#########################################################
# create output dirs
#########################################################
cdb_util.create_output_dirs()

#########################################################
# 1. parse dt/tbl/ds.txt
#########################################################
cdb_util.parse_input_dt_file(dt_oper_array, dt_array, filename_input_dt, trace_on)
cdb_util.parse_input_tbl_file(tbl_array, dt_array, filename_input_tbl, trace_on)
cdb_util.parse_input_ds_file(tbl_array, ds_array, dt_array, filename_input_ds, trace_on)
cdb_util.check_referlist(tbl_array, ds_array, trace_on)

def gen_store_tbl_func_cdb_store_tbl(file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_store_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)"
	print >> file_output, "{"
	print >> file_output, "    switch (oper)"
	print >> file_output, "    {"
	print >> file_output, "    case CDB_OPER_ADD:"
	print >> file_output, "        SAL_ASSERT(p_tbl_node->tbl_info->data_size == tbl_len);"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    case CDB_OPER_DEL:"
	print >> file_output, "        SAL_ASSERT(p_tbl_node->tbl_info->key_size == tbl_len);"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    case CDB_OPER_SET:"
	print >> file_output, "        SAL_ASSERT(p_tbl_node->tbl_info->data_size == tbl_len);"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    default:"
	print >> file_output, "        return PM_E_INVALID_PARAM;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    if (NULL == g_cdb_store_tbl_func[p_tbl_node->id])"
	print >> file_output, "    {"
	print >> file_output, "        return PM_E_NOT_SUPPORT;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return g_cdb_store_tbl_func[p_tbl_node->id](oper, fields, p_tbl_node, p_tbl, tbl_len);"
	print >> file_output, "}"
	print >> file_output, ""

def gen_store_tbl_func_cdb_store_tbl_refer_one(tbl_one, dt_array, file_output):
	table = tbl_one[0]
	table_name = table[0]
	table_name_low = string.lower(table[0])
	field_array = tbl_one[5]
	has_refer_tbl = 0
	for j, field_one in enumerate(field_array):
		if ('REFERLIST_TBL_' == field_one[0][:14]):
			has_refer_tbl = 1
	if (0 == has_refer_tbl):
		return
	p_refer_obj = 'p_refer_obj'
	print >> file_output, "        case %s:" % (table_name)
	print >> file_output, "            switch (fields[0])"
	print >> file_output, "            {"
	for j, field_one in enumerate(field_array):
		if ('REFERLIST_TBL_' == field_one[0][:14]):
			refer_tbl_name = string.lower(field_one[0][10:])
			refer_tbl_param = refer_tbl_name[4:]
			field_name = field_one[1]
			field_name_full = table_name + '_FLD_' + string.upper(field_name)
			print >> file_output, "            case %s:" % (field_name_full)
			print >> file_output, "                if (%s_get_master())" % (refer_tbl_name)
			print >> file_output, "                {"
			print >> file_output, "                    %s = %s_get_%s(%s);" % (p_refer_obj, refer_tbl_name, refer_tbl_param, p_refer_obj)
			print >> file_output, "                    PM_E_RETURN(%s_set_%s_sync(p_tbl, add, %s, FALSE));" % (table_name_low, field_name, p_refer_obj)
			print >> file_output, "                }"
			print >> file_output, "                break;"
	print >> file_output, "            default:"
	print >> file_output, "                break;"
	print >> file_output, "            }"
	print >> file_output, "            break;"

def gen_store_tbl_func_cdb_store_tbl_refer(tbl_array, dt_array, file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_store_tbl_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, "
	print >> file_output, "                    void *p_data, uint32 data_len)"
	print >> file_output, "{"
	print >> file_output, "    void *p_tbl = NULL;"
	print >> file_output, "    void *p_left = NULL;"
	print >> file_output, "    cdb_sync_refer_t *p_refer = NULL;"
	print >> file_output, "    void *p_refer_obj = NULL;"
	print >> file_output, "    uint32 add = FALSE;"
	print >> file_output, "    int32 rc = PM_E_NONE;"
	print >> file_output, ""
	print >> file_output, "    p_tbl = p_data;"
	print >> file_output, "    p_left = p_data + p_tbl_node->tbl_info->key_size;"
	print >> file_output, "    p_refer = p_left;"
	print >> file_output, "    p_refer_obj = &(p_refer->object);"
	print >> file_output, "    SAL_ASSERT(p_tbl_node->tbl_info->key_size + sizeof(cdb_sync_refer_hdr_t) + p_refer->hdr.size == data_len);"
	print >> file_output, "    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;"
	print >> file_output, ""
	print >> file_output, "    if (CDB_REFER_TBL == p_refer->hdr.type)"
	print >> file_output, "    {"
	print >> file_output, "        switch (p_tbl_node->id)"
	print >> file_output, "        {"
	for i, tbl_one in enumerate(tbl_array):
		gen_store_tbl_func_cdb_store_tbl_refer_one(tbl_one, dt_array, file_output)
	print >> file_output, "        default:"
	print >> file_output, "            return PM_E_INVALID_PARAM;"
	print >> file_output, "        }"
	print >> file_output, "    }"
	print >> file_output, "    else if (CDB_REFER_TIMER == p_refer->hdr.type)"
	print >> file_output, "    {"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return PM_E_NONE;"
	print >> file_output, "}"
	print >> file_output, ""

def gen_store_tbl_func_cdb_store_ds_refer_one(tbl_array, ds_one, dt_array, file_output):
	ds = ds_one[0]
	ds_name = ds[0]
	ds_name_low = string.lower( ds[0])
	field_array = ds_one[4]
	has_refer_tbl = 0
	for j, field_one in enumerate(field_array):
		if ('REFERLIST_TBL_' == field_one[0][:14]):
			has_refer_tbl = 1
	if (0 == has_refer_tbl):
		return
	p_refer_obj = 'p_refer_obj'
	print >> file_output, "        case %s:" % (ds_name)
	print >> file_output, "            switch (fields[0])"
	print >> file_output, "            {"
	for j, field_one in enumerate(field_array):
		if ('REFERLIST_TBL_' == field_one[0][:14]):
			refer_tbl_name = string.lower(field_one[0][10:])
			refer_tbl_param = refer_tbl_name[4:]
			field_name = field_one[1]
			field_name_full = ds_name + '_FLD_' + string.upper(field_name)
			print >> file_output, "            case %s:" % (field_name_full)
			print >> file_output, "                if (%s_get_master())" % (refer_tbl_name)
			print >> file_output, "                {"
			print >> file_output, "                    %s = %s_get_%s(%s);" % (p_refer_obj, refer_tbl_name, refer_tbl_param, p_refer_obj)
			print >> file_output, "                    PM_E_RETURN(%s_set_%s_sync(p_tbl, p_ds, add, %s, FALSE));" % (ds_name_low, field_name, p_refer_obj)
			print >> file_output, "                }"
			print >> file_output, "                break;"
	print >> file_output, "            default:"
	print >> file_output, "                break;"
	print >> file_output, "            }"
	print >> file_output, "            break;"


def gen_store_tbl_func_cdb_store_ds_refer(tbl_array, ds_array, dt_array, file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_store_ds_refer(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,"
	print >> file_output, "                void *p_data, uint32 data_len)"
	print >> file_output, "{"
	print >> file_output, "    void *p_tbl = NULL;"
	print >> file_output, "    void *p_ds = NULL;"
	print >> file_output, "    void *p_left = NULL;"
	print >> file_output, "    cdb_sync_refer_t *p_refer = NULL;"
	print >> file_output, "    void *p_refer_obj = NULL;"
	print >> file_output, "    uint32 ds2_data_size = 0;"
	print >> file_output, "    uint32 add = FALSE;"
	print >> file_output, "    int32 rc = PM_E_NONE;"
	print >> file_output, ""
	print >> file_output, "    if (p_ds2_node)"
	print >> file_output, "    {"
	print >> file_output, "        ds2_data_size = p_ds2_node->tbl_info->key_size;"
	print >> file_output, "    }"
	print >> file_output, "    p_tbl = p_data;"
	print >> file_output, "    p_ds = p_data + p_tbl_node->tbl_info->key_size;"
	print >> file_output, "    p_left = p_data + p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->key_size + ds2_data_size;"
	print >> file_output, "    p_refer = p_left;"
	print >> file_output, "    p_refer_obj = &(p_refer->object);"
	print >> file_output, "    SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->key_size + ds2_data_size \\"
	print >> file_output, "        + sizeof(cdb_sync_refer_hdr_t) + p_refer->hdr.size == data_len);"
	print >> file_output, "    add = (CDB_REFER_OPER_ADD == p_refer->hdr.oper) ? TRUE : FALSE;"
	print >> file_output, ""
	print >> file_output, "    if (CDB_REFER_TBL == p_refer->hdr.type)"
	print >> file_output, "    {"
	print >> file_output, "        switch (p_ds_node->id)"
	print >> file_output, "        {"
	for i, ds_one in enumerate(ds_array):
		gen_store_tbl_func_cdb_store_ds_refer_one(tbl_array, ds_one, dt_array, file_output)
	print >> file_output, "        default:"
	print >> file_output, "            return PM_E_INVALID_PARAM;"
	print >> file_output, "        }"
	print >> file_output, "    }"
	print >> file_output, "    else if (CDB_REFER_TIMER == p_refer->hdr.type)"
	print >> file_output, "    {"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return PM_E_NONE;"
	print >> file_output, "}"
	print >> file_output, ""


def gen_store_ds_func_cdb_store_ds(file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_store_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,"
	print >> file_output, "                void *p_data, uint32 data_len)"
	print >> file_output, "{"
	print >> file_output, "    void *p_tbl = NULL;"
	print >> file_output, "    void *p_ds = NULL;"
	print >> file_output, "    uint32 ds2_data_size = 0;"
	print >> file_output, ""
	print >> file_output, "    if (p_ds2_node)"
	print >> file_output, "    {"
	print >> file_output, "        ds2_data_size = p_ds2_node->tbl_info->data_size;"
	print >> file_output, "    }"
	print >> file_output, "    SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size + ds2_data_size == data_len);"
	print >> file_output, "    p_tbl = p_data;"
	print >> file_output, "    p_ds = p_data + p_tbl_node->tbl_info->key_size;"
	print >> file_output, ""
	print >> file_output, "    switch (oper)"
	print >> file_output, "    {"
	print >> file_output, "    case CDB_OPER_ADD:"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    case CDB_OPER_DEL:"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    case CDB_OPER_SET:"
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    default:"
	print >> file_output, "        return PM_E_INVALID_PARAM;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    if (NULL == g_cdb_store_ds_func[p_ds_node->id])"
	print >> file_output, "    {"
	print >> file_output, "        return PM_E_NOT_SUPPORT;"
	print >> file_output, ""
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    if (p_ds2_node)"
	print >> file_output, "    {"
	print >> file_output, "        return g_cdb_store_ds_func[p_ds2_node->id](oper, fields, p_tbl_node, p_ds_node,"
	print >> file_output, "                p_tbl, p_tbl_node->tbl_info->key_size, p_ds, p_ds_node->tbl_info->data_size);"
	print >> file_output, "    }"
	print >> file_output, "    else"
	print >> file_output, "    {"
	print >> file_output, "        return g_cdb_store_ds_func[p_ds_node->id](oper, fields, p_tbl_node, p_ds_node,"
	print >> file_output, "                p_tbl, p_tbl_node->tbl_info->key_size, p_ds, p_ds_node->tbl_info->data_size);"
	print >> file_output, "    }"
	print >> file_output, "}"
	print >> file_output, ""

def gen_store_tbl_func_variable(tbl_array, ds_array, dt_array, file_output):
	'generate func variable'
	print >> file_output, "CDB_STORE_TBL_FUNC g_cdb_store_tbl_func[TBL_MAX] = "
	print >> file_output, "{"
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		func_name = '_cdb_store_' + string.lower(tbl_name)
		print >> file_output, "    %s," % (func_name)
	print >> file_output, "};"
	print >> file_output, ""

def gen_store_ds_func_variable(ds_array, dt_array, file_output):
	'generate func variable'
	print >> file_output, "CDB_STORE_DS_FUNC g_cdb_store_ds_func[DS_MAX] = "
	print >> file_output, "{"
	for i, ds_one in enumerate(ds_array):
		ds_name = ds_one[0][0]
		func_name = '_cdb_store_' + string.lower(ds_name)
		print >> file_output, "    %s," % (func_name)
	print >> file_output, "};"
	print >> file_output, ""

def gen_store_tbl_include(tbl_array, ds_array, dt_array, file_output):
	'generate store tbl c include'
	print >> file_output, ""
	print >> file_output, "#include \"%s\"" % (CONST.PROTO_H_FILE)
	print >> file_output, "#include \"%s\"" % ('cdb_store.h')
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
		fname_h = 'gen/' + string.lower(tbl_name) + '.h'
		print >> file_output, "#include \"%s\"" % (fname_define_h)
		print >> file_output, "#include \"%s\"" % (fname_h)
	print >> file_output, ""

def gen_store_ds_include(ds_array, dt_array, file_output):
	'generate store ds c include'
	tbl_name_array = []
	print >> file_output, ""
	print >> file_output, "#include \"%s\"" % ('proto.h')
	print >> file_output, "#include \"%s\"" % ('cdb_store.h')
	for i, ds_one in enumerate(ds_array):
		ds_name = ds_one[0][0]
		parent_tbl = ds_one[3]
		tbl_name = string.lower(parent_tbl[0][0])
		tbl_name_array.append(tbl_name)

	for i, ds_one in enumerate(ds_array):
		field_array = ds_one[4]
		for j, field_one in enumerate(field_array):
			if ('REFERLIST_TBL_' == field_one[0][:14]):
				tbl_name = string.lower(field_one[0][10:])
				tbl_name_array.append(tbl_name)

	tbl_name_array = cdb_util.array_remove_duplicate_entry(tbl_name_array)
	for i, tbl_name in enumerate(tbl_name_array):
		fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
		fname_h = 'gen/' + string.lower(tbl_name) + '.h'
		print >> file_output, "#include \"%s\"" % (fname_define_h)
		print >> file_output, "#include \"%s\"" % (fname_h)
	print >> file_output, ""

def gen_store_tbl_c_one(table, key_array, data_array, master_array, field_array, ds_array, dt_array, file_output):
	tbl_name = table[0]
	tbl_value_name = table[1]
	tbl_name_low = string.lower(tbl_name)
	tbl_db_type = table[2]
	accusative = tbl_name_low[4:]
	key_type = tbl_name_low + '_key_t'
	tbl_type = tbl_name_low + '_t'
	key_param = 'p_' + tbl_value_name + '_key'
	tbl_param = 'p_' + tbl_value_name
	is_global = 0

	if ('GLOBAL' == tbl_db_type):
		is_global = 1

	print >> file_output, "static int32"
	print >> file_output, "_cdb_store_%s(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)" % (tbl_name_low)
	print >> file_output, "{"
	if (0 == is_global):
		print >> file_output, "    %s *%s = NULL;" % (key_type, key_param)
	print >> file_output, "    %s *%s = NULL;" % (tbl_type, tbl_param)
	print >> file_output, "    int32 rc = PM_E_NONE;"
	print >> file_output, ""
	print >> file_output, "    switch (oper)"
	print >> file_output, "    {"
	if (0 == is_global):
		print >> file_output, "    case CDB_OPER_ADD:"
		print >> file_output, "        %s = (%s*)p_tbl;" % (tbl_param, tbl_type)
		print >> file_output, "        rc = %s_add_%s_sync(%s, FALSE);" % (tbl_name_low, accusative, tbl_param)
		print >> file_output, "        break;"
		print >> file_output, ""
		print >> file_output, "    case CDB_OPER_DEL:"
		print >> file_output, "        %s = (%s*)p_tbl;" % (key_param, key_type)
		print >> file_output, "        rc = %s_del_%s_sync(%s, FALSE);" % (tbl_name_low, accusative, key_param)
		print >> file_output, "        break;"
		print >> file_output, ""
	print >> file_output, "    case CDB_OPER_SET:"
	print >> file_output, "        %s = (%s*)p_tbl;" % (tbl_param, tbl_type)
	print >> file_output, "        rc = %s_set_%s_field_sync(%s, fields[0], FALSE);" % (tbl_name_low, accusative, tbl_param)
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    default:"
	print >> file_output, "        return PM_E_INVALID_PARAM;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return rc;"
	print >> file_output, "}"
	print >> file_output, ""

def gen_store_ds_c_one(ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array, file_output):
	parent_table = parent_tbl[0]
	tbl_name = parent_table[0]
	ds_name = ds[0]
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	ds_value_name = ds[1]
	ds_name_low = string.lower(ds_name)
	accusative = ds_name_low[3:]
	ds_type = ds_name_low + '_t'
	ds_param = 'p_' + ds_value_name
	tbl_type = string.lower(tbl_name) + '_t'
	tbl_param = 'p_' + parent_table[1]
	parent_ds = None
	if (4 == len(ds)):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		parent_ds_name_upper = parent_ds[0][0]
		parent_ds_name = string.lower(parent_ds[0][0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value = 'p_' + parent_ds[0][1]
		parent_ds_db_value = 'p_db_' + parent_ds[0][1]
		parent_ds_accusative = parent_ds_name[3:]
		parent_db_value = parent_ds_db_value
	parent_ds_param_value_str = cdb_util.gen_parent_ds_param_value_str(parent_ds)

	print >> file_output, "static int32"
	print >> file_output, "_cdb_store_%s(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node," % (ds_name_low)
	print >> file_output, "                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)"
	print >> file_output, "{"
	print >> file_output, "    %s *%s = NULL;" % (tbl_type, tbl_param)
	if (None != parent_ds):
		print >> file_output, "    %s *%s = NULL;" % (parent_ds_type, parent_ds_value)
	print >> file_output, "    %s *%s = NULL;" % (ds_type, ds_param)
	print >> file_output, "    int32 rc = PM_E_NONE;"
	print >> file_output, ""

	print >> file_output, "    %s = (%s*)p_tbl;" % (tbl_param, tbl_type)
	if (None != parent_ds):
		print >> file_output, "    %s = (%s*)p_ds;" % (parent_ds_value, parent_ds_type)
		print >> file_output, "    %s = (%s*)((uint8*)p_ds + sizeof(%s));" % (ds_param, ds_type, parent_ds_type)
	else:
		print >> file_output, "    %s = (%s*)p_ds;" % (ds_param, ds_type)
	print >> file_output, ""
	print >> file_output, "    switch (oper)"
	print >> file_output, "    {"
	if (0 == is_array_ds):
		print >> file_output, "    case CDB_OPER_ADD:"
		print >> file_output, "        rc = %s_add_%s_sync(%s, %s%s, FALSE);" % (ds_name_low, accusative, tbl_param, parent_ds_param_value_str, ds_param)
		print >> file_output, "        break;"
		print >> file_output, ""
		print >> file_output, "    case CDB_OPER_DEL:"	
		if (len(key_array)):
			print >> file_output, "        rc = %s_del_%s_sync(%s, %s%s, FALSE);" % (ds_name_low, accusative, tbl_param, parent_ds_param_value_str, ds_param)
		else:
			print >> file_output, "        rc = %s_del_%s_sync(%s, FALSE);" % (ds_name_low, accusative, tbl_param)
		print >> file_output, "        break;"
		print >> file_output, ""
	print >> file_output, "    case CDB_OPER_SET:"
	print >> file_output, "        rc = %s_set_%s_field_sync(%s, %s%s, fields[0], FALSE);" % (ds_name_low, accusative, tbl_param, parent_ds_param_value_str, ds_param)
	print >> file_output, "        break;"
	print >> file_output, ""
	print >> file_output, "    default:"
	print >> file_output, "        return PM_E_INVALID_PARAM;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return rc;"
	print >> file_output, "}"
	print >> file_output, ""


def gen_store_tbl_c_files(tbl_array, ds_array, dt_array, filename_output):
	'generate store tbl c files'
	file_output = open(filename_output, 'w')
	print filename_output
	gen_store_tbl_include(tbl_array, ds_array, dt_array, file_output)
	for i, tbl_one in enumerate(tbl_array):
		gen_store_tbl_c_one(tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], ds_array, dt_array, file_output)
	gen_store_tbl_func_variable(tbl_array, ds_array, dt_array, file_output)
	gen_store_tbl_func_cdb_store_tbl(file_output)
	gen_store_tbl_func_cdb_store_tbl_refer(tbl_array, dt_array, file_output)
	file_output.close()
	print "generate file %s" % (filename_output)

def gen_store_ds_c_files(tbl_array, ds_array, dt_array, filename_output):
	'generate store ds c files'
	file_output = open(filename_output, 'w')
	gen_store_ds_include(ds_array, dt_array, file_output)
	for i, ds_one in enumerate(ds_array):
		gen_store_ds_c_one(ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array, file_output)
	gen_store_ds_func_variable(ds_array, dt_array, file_output)
	gen_store_ds_func_cdb_store_ds(file_output)
	gen_store_tbl_func_cdb_store_ds_refer(tbl_array, ds_array, dt_array, file_output)
	file_output.close()
	print "generate file %s" % (filename_output)


def gen_format_tbl_func_variable(tbl_array, ds_array, dt_array, file_output):
	'generate func variable'
	print >> file_output, "CDB_STORE_TBL_FUNC g_cdb_format_tbl_func[TBL_MAX] = "
	print >> file_output, "{"
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		func_name = '_cdb_format_' + string.lower(tbl_name)
		print >> file_output, "    %s," % (func_name)
	print >> file_output, "};"
	print >> file_output, ""

def gen_format_tbl_func_cdb_store_tbl(file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_format_tbl(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)"
	print >> file_output, "{"
	print >> file_output, "    if (NULL == g_cdb_format_tbl_func[p_tbl_node->id])"
	print >> file_output, "    {"
	print >> file_output, "        return PM_E_NOT_SUPPORT;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return g_cdb_format_tbl_func[p_tbl_node->id](oper, fields, p_tbl_node, p_tbl, tbl_len);"
	print >> file_output, "}"
	print >> file_output, ""

def gen_format_tbl_c_one_field(file_output, tbl_param, field):
	field_type = field[0]
	field_str = field[1]
	if (field_str.find('*') == 0):
		format_field_str = cdb_util.format_field_name(field_str)
		print >> file_output, "    %s->%s = NULL;" % (tbl_param, format_field_str)
	if (field_type.find('REFERLIST_') == 0):
		format_field_str = cdb_util.format_field_name(field_str)
		print >> file_output, "    %s->%s.obj_list = NULL;" % (tbl_param, format_field_str)

def gen_format_check_need_format(field_array):
	for i, field in enumerate(field_array):
		if (field[1].find('*') == 0):
			return 1
	for i, field in enumerate(field_array):
		if (field[0].find('REFERLIST_') == 0):
			return 1
	return 0

def gen_format_tbl_c_one(table, key_array, data_array, master_array, field_array, ds_array, dt_array, file_output):
	tbl_name = table[0]
	tbl_value_name = table[1]
	tbl_name_low = string.lower(tbl_name)
	tbl_db_type = table[2]
	accusative = tbl_name_low[4:]
	key_type = tbl_name_low + '_key_t'
	tbl_type = tbl_name_low + '_t'
	key_param = 'p_' + tbl_value_name + '_key'
	tbl_param = 'p_' + tbl_value_name
	is_global = 0

	if ('GLOBAL' == tbl_db_type):
		is_global = 1

	print >> file_output, "static int32"
	print >> file_output, "_cdb_format_%s(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)" % (tbl_name_low)
	print >> file_output, "{"
	need_format = gen_format_check_need_format(field_array)
	if (need_format == 1):
		print >> file_output, "    %s *%s = NULL;" % (tbl_type, tbl_param)
		print >> file_output, ""
		print >> file_output, "    if (CDB_OPER_DEL == oper)"
		print >> file_output, "    {"
		print >> file_output, "        return PM_E_NONE;"
		print >> file_output, "    }"
		print >> file_output, "    %s = (%s*)p_tbl;" % (tbl_param, tbl_type)
		print >> file_output, ""
		for i, field in enumerate(field_array):
			gen_format_tbl_c_one_field(file_output, tbl_param, field)
		print >> file_output, ""

	print >> file_output, "    return PM_E_NONE;"
	print >> file_output, "}"
	print >> file_output, ""

def gen_format_tbl_c_files(tbl_array, ds_array, dt_array, filename_output):
	'generate fromat tbl c files'
	file_output = open(filename_output, 'w')
	print filename_output
	gen_store_tbl_include(tbl_array, ds_array, dt_array, file_output)
	for i, tbl_one in enumerate(tbl_array):
		gen_format_tbl_c_one(tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], ds_array, dt_array, file_output)
	gen_format_tbl_func_variable(tbl_array, ds_array, dt_array, file_output)
	gen_format_tbl_func_cdb_store_tbl(file_output)
	file_output.close()
	print "generate file %s" % (filename_output)


def gen_format_ds_func_variable(ds_array, dt_array, file_output):
	'generate func variable'
	print >> file_output, "CDB_STORE_DS_FUNC g_cdb_format_ds_func[DS_MAX] = "
	print >> file_output, "{"
	for i, ds_one in enumerate(ds_array):
		ds_name = ds_one[0][0]
		func_name = '_cdb_format_' + string.lower(ds_name)
		print >> file_output, "    %s," % (func_name)
	print >> file_output, "};"
	print >> file_output, ""

def gen_format_ds_func_cdb_store_ds(file_output):
	print >> file_output, "int32"
	print >> file_output, "cdb_format_ds(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, cdb_node_t* p_ds2_node,"
	print >> file_output, "                void *p_data, uint32 data_len)"
	print >> file_output, "{"
	print >> file_output, "    void *p_tbl = NULL;"
	print >> file_output, "    void *p_ds = NULL;"
	print >> file_output, "    uint32 ds2_data_size = 0;"
	print >> file_output, ""
	print >> file_output, "    if (p_ds2_node)"
	print >> file_output, "    {"
	print >> file_output, "        ds2_data_size = p_ds2_node->tbl_info->data_size;"
	print >> file_output, "    }"
	print >> file_output, "    SAL_ASSERT(p_tbl_node->tbl_info->key_size + p_ds_node->tbl_info->data_size + ds2_data_size == data_len);"
	print >> file_output, "    p_tbl = p_data;"
	print >> file_output, "    p_ds = p_data + p_tbl_node->tbl_info->key_size;"
	print >> file_output, ""
	print >> file_output, "    if (NULL == g_cdb_format_ds_func[p_ds_node->id])"
	print >> file_output, "    {"
	print >> file_output, "        return PM_E_NOT_SUPPORT;"
	print >> file_output, "    }"
	print >> file_output, ""
	print >> file_output, "    return g_cdb_format_ds_func[p_ds_node->id](oper, fields, p_tbl_node, p_ds_node,"
	print >> file_output, "            p_tbl, p_tbl_node->tbl_info->key_size, p_ds, data_len);"
	print >> file_output, "}"
	print >> file_output, ""


def gen_format_tbl_c_one(table, key_array, data_array, master_array, field_array, ds_array, dt_array, file_output):
	tbl_name = table[0]
	tbl_value_name = table[1]
	tbl_name_low = string.lower(tbl_name)
	tbl_db_type = table[2]
	accusative = tbl_name_low[4:]
	key_type = tbl_name_low + '_key_t'
	tbl_type = tbl_name_low + '_t'
	key_param = 'p_' + tbl_value_name + '_key'
	tbl_param = 'p_' + tbl_value_name
	is_global = 0

	if ('GLOBAL' == tbl_db_type):
		is_global = 1

	print >> file_output, "static int32"
	print >> file_output, "_cdb_format_%s(uint32 oper, uint32 *fields, cdb_node_t* p_tbl_node, void *p_tbl, uint32 tbl_len)" % (tbl_name_low)
	print >> file_output, "{"
	need_format = gen_format_check_need_format(field_array)
	if (need_format == 1):
		print >> file_output, "    %s *%s = NULL;" % (tbl_type, tbl_param)
		print >> file_output, ""
		print >> file_output, "    if (CDB_OPER_DEL == oper)"
		print >> file_output, "    {"
		print >> file_output, "        return PM_E_NONE;"
		print >> file_output, "    }"
		print >> file_output, "    %s = (%s*)p_tbl;" % (tbl_param, tbl_type)
		print >> file_output, ""
		for i, field in enumerate(field_array):
			gen_format_tbl_c_one_field(file_output, tbl_param, field)
		print >> file_output, ""

	print >> file_output, "    return PM_E_NONE;"
	print >> file_output, "}"
	print >> file_output, ""

def gen_format_ds_c_one_field(file_output, ds_param, field):
	field_type = field[0]
	field_str = field[1]
	if (field_str.find('*') == 0):
		format_field_str = cdb_util.format_field_name(field_str)
		print >> file_output, "    %s->%s = NULL;" % (ds_param, format_field_str)
	if (field_type.find('REFERLIST_') == 0):
		format_field_str = cdb_util.format_field_name(field_str)
		print >> file_output, "    %s->%s.obj_list = NULL;" % (ds_param, format_field_str)

def gen_format_ds_c_one(ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array, file_output):
	parent_table = parent_tbl[0]
	tbl_name = parent_table[0]
	ds_name = ds[0]
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	ds_value_name = ds[1]
	ds_name_low = string.lower(ds_name)
	accusative = ds_name_low[3:]
	ds_type = ds_name_low + '_t'
	ds_param = 'p_' + ds_value_name
	tbl_type = string.lower(tbl_name) + '_t'
	tbl_param = 'p_' + parent_table[1]
	parent_ds = None

	print >> file_output, "static int32"
	print >> file_output, "_cdb_format_%s(uint8 oper, uint32 *fields, cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node," % (ds_name_low)
	print >> file_output, "                void *p_tbl, uint32 tbl_len, void *p_ds, uint32 ds_len)"
	print >> file_output, "{"

	need_format = gen_format_check_need_format(field_array)
	if (need_format == 1):
		print >> file_output, "    %s *%s = NULL;" % (ds_type, ds_param)
		print >> file_output, ""
		print >> file_output, "    %s = (%s*)p_ds;" % (ds_param, ds_type)
		print >> file_output, ""
		for i, field in enumerate(field_array):
			gen_format_ds_c_one_field(file_output, ds_param, field)

	print >> file_output, "    return PM_E_NONE;"
	print >> file_output, "}"
	print >> file_output, ""

def gen_format_ds_c_files(ds_array, dt_array, filename_output):
	'generate fromat ds c files'
	file_output = open(filename_output, 'w')
	gen_store_ds_include(ds_array, dt_array, file_output)
	for i, ds_one in enumerate(ds_array):
		gen_format_ds_c_one(ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array, file_output)
	gen_format_ds_func_variable(ds_array, dt_array, file_output)
	gen_format_ds_func_cdb_store_ds(file_output)
	file_output.close()
	print "generate file %s" % (filename_output)

############################################################
# gen cdb_store_tbl.c
if (GEN_STORE_TBL):
	gen_store_tbl_c_files(tbl_array, ds_array, dt_array, filename_output_tbl)

############################################################
# gen cdb_store_ds.c
if (GEN_STORE_DS):
	gen_store_ds_c_files(tbl_array, ds_array, dt_array, filename_output_ds)

############################################################
# gen cdb_format_tbl.c
if (GEN_FORMAT_TBL):
	gen_format_tbl_c_files(tbl_array, ds_array, dt_array, filename_output_format_tbl)

############################################################
# gen cdb_format_ds.c
if (GEN_FORMAT_DS):
	gen_format_ds_c_files(ds_array, dt_array, filename_output_format_ds)
