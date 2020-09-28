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

GEN_TBL_DEF_H	= 1
GEN_DS_DEF_H	= 1
GEN_TBL_INFO	= 1
GEN_TBL_FIELD	= 1
GEN_DS_INFO		= 1
GEN_DS_FIELD	= 1
GEN_ACT_INFO	= 1
GEN_ACT_FIELD	= 1

filename_input_node = CONST.CDB_INPUT_PATH + 'node.txt'
filename_input_tbl = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds  = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_act = CONST.CDB_INPUT_PATH + 'act.txt'
filename_input_dt = CONST.CDB_INPUT_PATH + 'datatype.txt'

node_dir_array = []
node_tbl_array = []
node_ds_array  = []
node_act_array = []
node_bat_array = []

tbl_array = []
ds_array = []
act_array = []
dt_oper_array = []
dt_array = []

#########################################################
# create output dirs
#########################################################
cdb_util.create_output_dirs()

#########################################################
# 1. parse input files
#########################################################
cdb_util.parse_input_node_file(node_dir_array, node_tbl_array, node_ds_array, node_act_array, node_bat_array, filename_input_node)
cdb_util.parse_input_dt_file(dt_oper_array, dt_array, filename_input_dt, trace_on)
cdb_util.parse_input_tbl_file(tbl_array, dt_array, filename_input_tbl, trace_on)
#cdb_util.parse_input_ds_file(tbl_array, ds_array, dt_array, filename_input_ds, trace_on)
#cdb_util.parse_input_act_file(act_array, filename_input_act, trace_on)

#########################################################
# 2. check node.txt and other files order
#########################################################
cdb_util.check_tbl_order(node_tbl_array, tbl_array, trace_on)
#cdb_util.check_ds_order(node_ds_array, ds_array, trace_on)
#cdb_util.check_act_order(node_act_array, act_array, trace_on)

############################################################
# gen cdb_tbl_info_priv.h

def build_one_tbl_info(fobj, table, data_array):
        'build g table info structure'
        tbl_name = table[0]
        tbl_name_low = string.lower(tbl_name)
        with_key = 0
        tbl_name_field = tbl_name_low + '_fields'
        for i, field in enumerate(data_array):
            if ('key_t' in field[0]):
                key_type = string.lower(field[0])
                with_key = 1
        print >> fobj, "    {"
        if (with_key):
            print >> fobj, "        sizeof(%s)," % (key_type)
        else:
            print >> fobj, "        0,"
        print >> fobj, "        sizeof(%s)," % (tbl_name_low + '_t')
        print >> fobj, "        sizeof(%s)/sizeof(sdb_field_t)," % (tbl_name_field)
        print >> fobj, "        %s," % (tbl_name_field)
        print >> fobj, "        {"
        print >> fobj, "            %s_iterate," % (tbl_name_low)
        print >> fobj, "            (STBL_DUMP_FUNC)%s_dump_one," % (tbl_name_low)
        if (with_key):
            print >> fobj, "            (STBL_KEY_V2S_FUNC)%s_key_val2str," % (tbl_name_low)
        else:
            print >> fobj, "            NULL,"
        print >> fobj, "            NULL"
        print >> fobj, "        },"
        print >> fobj, "    },"

def gen_tbl_init_func(fobj, tbl_array):
	print >> fobj, "int32"
	print >> fobj, "sdb_tbl_pm_init(uint32 pm_id)"
	print >> fobj, "{"
	print >> fobj, "    sdb_node_t *p_tbl = NULL;"
	print >> fobj, "    uint32 i = 0;"
	print >> fobj, ""
	print >> fobj, "    for (i = 0; i < STBL_MAX; i++)"
	print >> fobj, "    {"
	print >> fobj, "        p_tbl = sdb_get_tbl(i);"
	print >> fobj, ""
	print >> fobj, "        if (0)"
	print >> fobj, "        {"
	print >> fobj, "        }"

	for i, entity in enumerate(tbl_array):
		tbl_name_upper = entity[0][0]
		tbl_name = string.lower(tbl_name_upper)
		accusative = tbl_name[5:]
		pms_array = entity[0][3]
		print >> fobj, "        else if (%s == i)" % (tbl_name_upper)
		print >> fobj, "        {"
		if (len(pms_array) == 1 and 'all' == pms_array[0]):
			pm_id = 'PM_ID_' + string.upper(pms_array[0])
			print >> fobj, "            %s_init_%s();" % (tbl_name, accusative)
			print >> fobj, "            p_tbl->inited = TRUE;"
		else:
			tmp_str = ''
			for j, pm_item in enumerate(pms_array):
				pm_id = 'PM_ID_' + string.upper(pm_item)
				tmp_str += ' || (' + pm_id + ' == pm_id)'
			print >> fobj, "            {"
			print >> fobj, "                %s_init_%s();" % (tbl_name, accusative)
			print >> fobj, "                p_tbl->inited = TRUE;"
			print >> fobj, "            }"
		print >> fobj, "        }"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return SDB_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_tbl_info_file(fname, tbl_array):
	'generate table info file'

	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_TBL_INFO_PRIV_H__"

	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	print >> fobj, "static sdb_tbl_info_t g_sdb_tbl_info[STBL_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		build_one_tbl_info(fobj, tbl_one[0], tbl_one[2])
	print >> fobj, "};"
	print >> fobj, ""

	gen_tbl_init_func(fobj, tbl_array)

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def build_one_ds_info(fobj, ds):
    'build g ds info structure'
    ds_name = ds[0]
    ds_name_low = string.lower(ds_name)
    ds_name_field = ds_name_low + '_fields'
    print >> fobj, "    {"
    print >> fobj, "        0,"
    print >> fobj, "        sizeof(%s)," % (ds_name_low + '_t')
    print >> fobj, "        sizeof(%s)/sizeof(sdb_field_t)," % (ds_name_field)
    print >> fobj, "        %s," % (ds_name_field)
    print >> fobj, "        {"
    print >> fobj, "            NULL,"
    print >> fobj, "            (STBL_DUMP_FUNC)%s_dump_one," % (ds_name_low)
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL"
    print >> fobj, "        },"
    print >> fobj, "        {"
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL,"
    print >> fobj, "            NULL"
    print >> fobj, "        },"
    print >> fobj, "        {"
    print >> fobj, "            (DS_KEY_NAME_DUMP_FUNC)%s_key_name_dump," % (ds_name_low)
    print >> fobj, "            (DS_KEY_VALUE_DUMP_FUNC)%s_key_value_dump," % (ds_name_low)
    print >> fobj, "            (DS_FIELD_NAME_DUMP_FUNC)%s_field_name_dump," % (ds_name_low)
    print >> fobj, "            (DS_FIELD_VALUE_DUMP_FUNC)%s_field_value_dump," % (ds_name_low)
    print >> fobj, "            (DS_DUMP_FUNC)%s_table_dump," % (ds_name_low)
    print >> fobj, "            (DS_PARSER_FUNC)%s_table_parser," % (ds_name_low)
    print >> fobj, "        }"
    print >> fobj, "    },"

def gen_ds_info_file(fname, ds_array):
	'generate table info file'
	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_DS_INFO_PRIV_H__"

	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	print >> fobj, "static sdb_tbl_info_t g_sdb_ds_info[SDS_MAX] ="
	print >> fobj, "{"
	for i, ds_one in enumerate(ds_array):
		build_one_ds_info(fobj, ds_one[0])
	print >> fobj, "};"

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def build_one_act_info(fobj, act):
	'build g act info structure'
	act_name = act[0]
	act_name_low = string.lower(act_name)
	act_name_field = act_name_low + '_fields'
	print >> fobj, "    {"
	print >> fobj, "        0,"
	print >> fobj, "        0,"
	print >> fobj, "        sizeof(%s)/sizeof(sdb_field_t)," % (act_name_field)
	print >> fobj, "        %s," % (act_name_field)
	print >> fobj, "        {"
	print >> fobj, "            NULL,"
	print >> fobj, "            NULL"
	print >> fobj, "        }"
	print >> fobj, "    },"

def gen_act_info_file(fname, act_array):
	'generate act info file'
	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_ACT_INFO_PRIV_H__"

	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	print >> fobj, "static sdb_tbl_info_t g_Sdb_act_info[SACT_MAX] ="
	print >> fobj, "{"
	for i, act_one in enumerate(act_array):
		build_one_act_info(fobj, act_one[0])
	print >> fobj, "};"

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def get_dt_type_type(field):
	ds_name = '-1'
	field_len = len(field)
	dt_type = field[field_len - 1]
	dt_type_type = dt_type[1]
	if ('sdb_reference_list_t' == dt_type_type):
		dt_type_type = 'REFERLIST'
		if ('REFERLIST_DS' in field[0]):
			ds_name = field[0][10:]
	elif (None != dt_type[2]):
		if (len(dt_type[2]) >= 5):
			dt_type_type = 'COMPOSE'
	elif ('DS_' in dt_type_type):
		ds_name = dt_type_type
		dt_type_type = 'DS'
	dt_type_type = 'SDB_DT_' + dt_type_type

	return [dt_type_type, ds_name]

def build_one_act_field(fobj, act, field_array):
	'generate act field structure'
	act_name = act[0]
	act_name_low = string.lower(act_name)
	act_name_field = act_name_low + '_fields'

	print >> fobj, "static sdb_field_t %s[] = " % (act_name_field)
	print >> fobj, "{"
	for i, field in enumerate(field_array):
		field_name = cdb_util.format_field_name(field[1])
		field_access_enum = get_field_access_enum(field)
		[dt_type_type, ds_name] = ['SDB_DT_NONE', '-1']
		print >> fobj, "    { %-15s %-18s %-20s \"%s\" }," % (field_access_enum + ',', dt_type_type + ',', ds_name + ',', field_name)
	print >> fobj, "};"
	print >> fobj, ""

def gen_act_field_file(fname, act_array):
	'generate act field file'
	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_ACT_FIELD_PRIV_H__"
	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	for i, act_one in enumerate(act_array):
		build_one_act_field(fobj, act_one[0], act_one[1])

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)


############################################################
# gen cdb_tbl_field_priv.h

def get_field_access_enum(field):
	access_type = field[2]
	if ('FLAGS' == access_type):
		access_type = field[5]
	if (access_type == 'READ'):
		return 'SDB_ACCESS_RD'
	elif (access_type == 'RW'):
		return 'SDB_ACCESS_RW'
	elif (access_type == 'SUB'):
		return 'SDB_ACCESS_SUB'
	elif (access_type == 'SUB'):
		return 'SDB_ACCESS_SUB'
	else:
		cdb_util.error_return("Invalid field access type %s", field)

def build_one_tbl_field(fobj, table, field_array):
	'generate table field structure'
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	tbl_name_field = tbl_name_low + '_fields'

	print >> fobj, "static sdb_field_t %s[] = " % (tbl_name_field)
	print >> fobj, "{"
	for i, field in enumerate(field_array):
		if (len(field) > 4):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_access_enum = get_field_access_enum(field)
		[dt_type_type, ds_name] = get_dt_type_type(field)
		print >> fobj, "    { %-15s %-18s %-20s \"%s\" }," % (field_access_enum + ',', dt_type_type + ',', ds_name + ',', field_name)
	print >> fobj, "};"
	print >> fobj, ""

def gen_tbl_field_file(fname, tbl_array):
	'generate table field file'
	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_TBL_FIELD_PRIV_H__"
	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	for i, tbl_one in enumerate(tbl_array):
		build_one_tbl_field(fobj, tbl_one[0], tbl_one[5])

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def build_one_ds_field(fobj, ds, key_array, field_array):
	'generate ds field structure'
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_field = ds_name_low + '_fields'

	print >> fobj, "static sdb_field_t %s[] = " % (ds_name_field)
	print >> fobj, "{"
	for i, field in enumerate(field_array):
		if (len(field) > 4):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])
		field_access_enum = get_field_access_enum(field)
		[dt_type_type, ds_name] = get_dt_type_type(field)
		print >> fobj, "    { %-15s %-18s %-20s \"%s\" }," % (field_access_enum + ',', dt_type_type + ',', ds_name + ',', field_name)
	print >> fobj, "};"
	print >> fobj, ""

def gen_ds_field_file(fname, ds_array):
	'generate ds field file'
	fobj = open(fname, 'w')
	_macro_h_ = "__CDB_DS_FIELD_PRIV_H__"
	print >> fobj, ""
	print >> fobj, "#ifndef %s" % (_macro_h_)
	print >> fobj, "#define %s" % (_macro_h_)
	print >> fobj, ""

	for i, ds_one in enumerate(ds_array):
		build_one_ds_field(fobj, ds_one[0], ds_one[1], ds_one[4])

	print >> fobj, "#endif /* !%s */" % (_macro_h_)
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def build_tbl_key(fobj, tbl_name, key_array):
	'build table key structure'
	if (1 == len(key_array) and 1 < len(key_array[0]) and '' == key_array[0][1]):
		print >> fobj, "typedef %s %s_key_t;" % (key_array[0][0], string.lower(tbl_name))
	else:
		print >> fobj, "typedef struct"
		print >> fobj, "{"
		for i, key in enumerate(key_array):
			print >> fobj, "    %-20s %s" % (key[0], key[1]+";")
		print >> fobj, "} %s_key_t;" % (string.lower(tbl_name))
	print >> fobj, ""

def build_tbl_master(fobj, tbl_name, master_array):
	'build table master structure'
	if (0 == len(master_array)):
		return

	print >> fobj, "typedef struct"
	print >> fobj, "{"
	for i, field in enumerate(master_array):
		field_type = field[0]
		field_name = field[1]+";"

		if (2 == len(field)):
			print >> fobj, "    %-20s %s" % (field_type, field_name)
		else:
			note = field[2].strip()
			print >> fobj, "    %-20s %-20s %s" % (field_type, field_name, note)
	print >> fobj, "} %s_master_t;" % (string.lower(tbl_name))
	print >> fobj, ""

def build_tbl_field_id(fobj, tbl_name, field_array):
	'build table field id'
	print >> fobj, "/* %s field defines */" % (tbl_name)
	print >> fobj, "typedef enum"
	print >> fobj, "{"
	for i, field in enumerate(field_array):
		if (len(field) > 5):
			field_name = string.upper(cdb_util.format_field_name(field[3]))
			access_type = field[5]
		else:
			field_name = string.upper(cdb_util.format_field_name(field[1]))
			access_type = field[2]
		print >> fobj, "    %s_FLD_%-20s = %-2d,  /* %s */" % (tbl_name, field_name, i, access_type)
	print >> fobj, "    %s_FLD_%-20s = %-2d" % (tbl_name, 'MAX', i+1)
	print >> fobj, "} %s_field_id_t;" % (string.lower(tbl_name))
	print >> fobj, ""

def build_tbl_data(fobj, tbl_name, data_array):
	'build table data structure'
	self_refer = 0
	data_name_s = string.lower(tbl_name) + '_s'
	data_name = string.lower(tbl_name) + '_t'

	for i, field in enumerate(data_array):
		if (data_name == field[0]):
			self_refer = 1
			break
		if ('CTCLIB_TAILQ_ENTRY' == field[0]):
			self_refer = 1
			break

	if (self_refer):
		print >> fobj, "typedef struct %s" % (data_name_s)
	else:
		print >> fobj, "typedef struct"
	print >> fobj, "{"
	for i, field in enumerate(data_array):
		if ('KEY' in field[0]):
			field_type = string.lower(field[0]) + '_t'
		elif ('REFER_' in field[0]):
			field_type = 'cdb_reference_t'
		elif ('REFERLIST_' in field[0]):
			field_type = 'sdb_reference_list_t'
		elif ('ARRAY_DS_' in field[0]):
			field_type = string.lower(field[0])[6:] + '_t'
		elif ('CTCLIB_TAILQ_ENTRY' in field[0]):
			field_type = field[0] + '(' + data_name_s + ')'
		elif (data_name == field[0]):
			field_type = 'struct ' + data_name_s
		else:
			field_type = field[0]
		field_name = field[1]+";"

		if (len(field) > 4):
			note = field[3].strip()
			print >> fobj, "    %-20s %-20s %s" % (field_type, field_name, note)
		else:
			print >> fobj, "    %-20s %s" % (field_type, field_name)
	cdb_util.insert_tbl_user_define_endfield(fobj, tbl_name+'_define', '.h')
	print >> fobj, "} %s;" % (data_name)
	print >> fobj, ""

def build_tbl_data_include_tbl(fobj, tbl_name, data_array):
	'build table include tbl'
	self_refer = 0
	data_name_s = string.lower(tbl_name) + '_s'
	data_name = string.lower(tbl_name) + '_t'
	fname_define_h_array = []

	for i, field in enumerate(data_array):
		field_type = field[0]
		if (field_type[:4] == "tbl_" and field_type[-5:] != "key_t" and field_type != "tbl_route_node_t"):
			fname_define_h = 'gen/' + string.lower(field_type[:-2]) + '_define.h'
			fname_define_h_array.append(fname_define_h)

	fname_define_h_array = cdb_util.array_remove_duplicate_entry(fname_define_h_array)
	for i, fname_define_h in enumerate(fname_define_h_array):
		print >> fobj, "#include \"%s\"" % (fname_define_h)

def gen_tbl_define_h_one(table, key_array, data_array, master_array, field_array):
	tbl_name = table[0]
	fname = CONST.CDB_H_PATH + string.lower(tbl_name) + '_define.h'
	file_tbl = open(fname, 'w')
	_macro_h_ = "__" + tbl_name + "_DEFINE_H__"

	print >> file_tbl, ""
	print >> file_tbl, "#ifndef %s" % (_macro_h_)
	print >> file_tbl, "#define %s" % (_macro_h_)
	print >> file_tbl, ""

	build_tbl_data_include_tbl(file_tbl, tbl_name, data_array)
	cdb_util.build_sub_ds_include(file_tbl, tbl_name, field_array, 1)

	cdb_util.insert_tbl_user_define_begin(file_tbl, tbl_name+'_define', '.h')

	build_tbl_field_id(file_tbl, tbl_name, field_array)

	print >> file_tbl, "/* %s defines */" % (tbl_name)
	if (len(key_array)):
		build_tbl_key(file_tbl, tbl_name, key_array)

	build_tbl_data(file_tbl, tbl_name, data_array)

	cdb_util.insert_tbl_user_define_beginmaster(file_tbl, tbl_name+'_define', '.h')

	build_tbl_master(file_tbl, tbl_name, master_array)

	cdb_util.insert_tbl_user_define_end(file_tbl, tbl_name+'_define', '.h')

	print >> file_tbl, "#endif /* !%s */" % (_macro_h_)
	print >> file_tbl, ""
	file_tbl.close()
	print "generate file %-50s field_num: %d" % (fname, len(field_array))

def gen_tbl_define_h_files(tbl_array):
	'generate table structure header files'
	for i, tbl_one in enumerate(tbl_array):
		gen_tbl_define_h_one(tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5])

def gen_ds_define_h_one(table, key_array, data_array, field_array):
	tbl_name = table[0]
	fname = CONST.CDB_H_PATH + string.lower(tbl_name) + '_define.h'
	file_tbl = open(fname, 'w')
	_macro_h_ = "__" + tbl_name + "_DEFINE_H__"

	print >> file_tbl, ""
	print >> file_tbl, "#ifndef %s" % (_macro_h_)
	print >> file_tbl, "#define %s" % (_macro_h_)
	print >> file_tbl, ""

	cdb_util.build_ds_sub_ds_include(file_tbl, tbl_name, data_array, 1)

	build_tbl_field_id(file_tbl, tbl_name, field_array)

	print >> file_tbl, "/* %s defines */" % (tbl_name)
	if (len(key_array)):
		build_tbl_key(file_tbl, tbl_name, key_array)

	build_tbl_data(file_tbl, tbl_name, data_array)

	print >> file_tbl, "#endif /* !%s */" % (_macro_h_)
	print >> file_tbl, ""
	file_tbl.close()
	print "generate file %-50s field_num: %d" % (fname, len(field_array))

def gen_ds_define_h_files(ds_array):
	'generate ds structure header files'
	for i, ds_one in enumerate(ds_array):
		gen_ds_define_h_one(ds_one[0], ds_one[1], ds_one[2], ds_one[4])

############################################################
# gen tbl_xxx_define.h
if (GEN_TBL_DEF_H):
	gen_tbl_define_h_files(tbl_array)

############################################################
# gen ds_xxx_define.h
if (GEN_DS_DEF_H):
	gen_ds_define_h_files(ds_array)

############################################################
# gen cdb_tbl_info_priv.h
if (GEN_TBL_INFO):
	gen_tbl_info_file(CONST.CDB_H_PATH + 'sdb_tbl_info_priv.h', tbl_array)

############################################################
# gen cdb_tbl_field_priv.h
if (GEN_TBL_FIELD):
	gen_tbl_field_file(CONST.CDB_H_PATH + 'sdb_tbl_field_priv.h', tbl_array)

#############################################################
## gen cdb_ds_info_priv.h
#if (GEN_DS_INFO):
#	gen_ds_info_file(CONST.CDB_H_PATH + 'sdb_ds_info_priv.h', ds_array)
#
#############################################################
## gen cdb_ds_field_priv.h
#if (GEN_DS_FIELD):
#	gen_ds_field_file(CONST.CDB_H_PATH + 'sdb_ds_field_priv.h', ds_array)
#
#############################################################
## gen cdb_act_info_priv.h
#if (GEN_ACT_INFO):
#	gen_act_info_file(CONST.CDB_H_PATH + 'sdb_act_info_priv.h', act_array)
#
#############################################################
## gen cdb_act_field_priv.h
#if (GEN_ACT_FIELD):
#	gen_act_field_file(CONST.CDB_H_PATH + 'sdb_act_field_priv.h', act_array)
#
