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

GEN_PY_TBL	= 1
GEN_PY_DS	= 1

filename_input_node = CONST.CDB_INPUT_PATH + 'node.txt'
filename_input_tbl  = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds   = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_act  = CONST.CDB_INPUT_PATH + 'act.txt'
filename_input_dt   = CONST.CDB_INPUT_PATH + 'datatype.txt'
filename_input_dtpy = CONST.CDB_INPUT_PATH + 'pydatatype.txt'

node_dir_array = []
node_tbl_array = []
node_ds_array  = []
node_act_array = []
node_bat_array = []

# py datatype
py_OTHR_TYPES_array = []
py_UINT_TYPES_array = []
py_STRING_KEY_array = []
py_CHAR_ARRAY_array = []

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
cdb_util.parse_input_ds_file(tbl_array, ds_array, dt_array, filename_input_ds, trace_on)
cdb_util.parse_input_act_file(act_array, filename_input_act, trace_on)
cdb_util.parse_input_pydt_file(py_OTHR_TYPES_array, py_UINT_TYPES_array, py_STRING_KEY_array, py_CHAR_ARRAY_array, filename_input_dtpy, trace_on)

#########################################################
# 2. check node.txt and other files order
#########################################################
cdb_util.check_tbl_order(node_tbl_array, tbl_array, trace_on)
cdb_util.check_ds_order(node_ds_array, ds_array, trace_on)
cdb_util.check_act_order(node_act_array, act_array, trace_on)

def get_othr_type(dt_map_type):
	global py_OTHR_TYPES_array

	for i, othr_type in enumerate(py_OTHR_TYPES_array):
		if (dt_map_type == othr_type[0]):
			return othr_type
	return None

def get_string_key_type(dt_type):
	global py_STRING_KEY_array

	for i, string_key in enumerate(py_STRING_KEY_array):
		if (dt_type == string_key[0]):
			return string_key
	return None

def get_uint_types_type(dt_type):
	global py_UINT_TYPES_array
	for i, uint_types in enumerate(py_UINT_TYPES_array):
		if (dt_type == uint_types[0]):
			return uint_types
	return None


def get_commmon_array_type(raw_field_name, dt_map_type):
	global py_CHAR_ARRAY_array
	
	if (dt_map_type == 'UINT32ARRAY'):
		item_type = 'I'
	elif (dt_map_type == 'DOUBLEARRAY'):
		item_type = 'd'
	elif (dt_map_type == 'STRING'):
		item_type = 's'
	else:
		return 'X'

	is_found = 0
	for i, char_array in enumerate(py_CHAR_ARRAY_array):
		array_index = raw_field_name.find(char_array[0])
		if (array_index > 0):
			is_found = 1
			break

	if (is_found):
		num_str = char_array[1]
		tmp_index = raw_field_name.find('+1')
		if (tmp_index > 0):
			num = int(num_str) + 1
			num_str = str(num)
		return num_str + item_type
	else:
		return 'X'

def get_str_array_type(dt_type, raw_field_name):
	global py_CHAR_ARRAY_array
	dt_type1 = dt_type.strip('[]')
	string_key = get_string_key_type(dt_type1)
	if (string_key):
		str_type = string_key[1]
	else:
		return 'X'

	is_found = 0
	for i, char_array in enumerate(py_CHAR_ARRAY_array):
		array_index = raw_field_name.find(char_array[0])
		if (array_index > 0):
			is_found = 1
			break

	if (is_found):
		num_str = char_array[1]
		tmp_index = raw_field_name.find('+1')
		if (tmp_index > 0):
			num = int(num_str) + 1
			num_str = str(num)
		return '\'%s\'*%s' % (str_type, num_str)
	else:
		return 'X'

def get_ds_array_type(dt_type, raw_field_name):
	global py_CHAR_ARRAY_array
	dt_type1 = dt_type.strip('[]')
	othr_type = get_othr_type(dt_type1)
	if (othr_type):
		str_type = othr_type[1]
	else:
		return 'X'

	is_found = 0
	for i, char_array in enumerate(py_CHAR_ARRAY_array):
		array_index = raw_field_name.find(char_array[0])
		if (array_index > 0):
			is_found = 1
			break

	if (is_found):
		num_str = char_array[1]
		tmp_index = raw_field_name.find('+1')
		if (tmp_index > 0):
			num = int(num_str) + 1
			num_str = str(num)
		return '\'%s\'*%s' % (str_type, num_str)
	else:
		return 'X'

def get_uint8_array_type(raw_field_name):
	if (raw_field_name == 'wavelength[2]'):
		return '2s'
	elif (raw_field_name == 'length[5]'):
		return '5s'
	else:
		return 'X'

def build_py_tbl_one(fobj, table, data_array, field_array, is_table):
	'build a table info structure'
	if (is_table == 1):
		tbl_name = table[0]
		prefix = "Table"
	else:
		tbl_name = table[0]
		prefix = "Ds"
	tbl_name_low = string.lower(tbl_name) + '_t'
	with_key = 0
	tbl_name_field = tbl_name_low + '_fields'

	print >> fobj, "class %s(object):" % (tbl_name_low)
	print >> fobj, "    \"\"\""
	print >> fobj, "    %s %s struct" % (prefix, tbl_name)
	print >> fobj, "    \"\"\""

### Format C Type               Python type         Standard size     Notes 
### x      pad                  byte no value     
### c      char                 string of length 1  1   
### b      signed char          integer             1                (3) 
### B      unsigned char        integer             1                (3) 
### ?      _Bool                bool                1                (1) 
### h      short                integer             2                (3) 
### H      unsigned short       integer             2                (3) 
### i      int                  integer             4                (3) 
### I      unsigned int         integer             4                (3) 
### l      long                 integer             4                (3) 
### L      unsigned long        integer             4                (3) 
### q      long long            integer             8                (2), (3) 
### Q      unsigned long long   integer             8                (2), (3) 
### f      float                float               4                (4) 
### d      double               float               8                (4) 
### s      char[]               string     
### p      char[]               string     
### P      void *               integer                              (5), (3) 

	fmt_array = []
	for i, field in enumerate(data_array):
		if (len(field) < 3):
			error_return("Table %s field %s unknown", tbl_name, field)
		
		if (field[2] == 'INTERNAL'):
			dt_map_type = field[2]
			dt_type = field[0]
		else:
			if (len(field) > 4):
				dt_type_struct = field[4]
			else:
				dt_type_struct = field[3]
			dt_type = dt_type_struct[0]
			dt_map_type = dt_type_struct[1]
		raw_field_name = field[1]
		field_name = cdb_util.format_field_name(field[1])

		if (field[2] == 'INTERNAL'):
			if (dt_type == 'uint8'):
				fmt_value = 'B'
			elif (dt_type == 'uint16'):
				fmt_value = 'H'
			elif (dt_type == 'uint32'):
				fmt_value = 'I'
			else:
				if (raw_field_name[0] == '*'):
					if ('*link[2]' == raw_field_name):
						fmt_value = CONST.PYDT_PTR_STR*2
					else:
						fmt_value = CONST.PYDT_PTR_STR
				else:
					fmt_value = 'X'

		elif (dt_map_type == 'UINT' or dt_map_type == 'INT' or dt_map_type == 'UINT64' or dt_map_type == 'INT64'):
			uint_types = get_uint_types_type(dt_type)
			if (uint_types):
				fmt_value = uint_types[1]
			else:
				fmt_value = 'X'
		elif (dt_map_type == 'FLAGS8'):
			fmt_value = 'B'
		elif (dt_map_type == 'FLAGS32'):
			fmt_value = 'I'
		elif (dt_map_type == 'ENUM8'):
			fmt_value = 'B'
		elif (dt_map_type == 'ENUM32'):
			fmt_value = 'I'
		elif (dt_map_type == 'BITMAP'):
			if (dt_type == 'bmp_32_t'):
				fmt_value = 'I'
			elif (dt_type == 'bmp_64_t'):
				fmt_value = '2I'
			elif (dt_type == 'port_bmp_t'):
				fmt_value = '66I'
			elif (dt_type == 'logid_bmp_t'):
				fmt_value = 'I'
		elif (dt_map_type == 'DOUBLE'):
			fmt_value = 'd'
		elif (dt_map_type == 'STRING'):
			string_key = get_string_key_type(dt_type)
			if (string_key):
				fmt_value = string_key[1]
			elif (dt_type == 'char[]'):
				char_str = get_commmon_array_type(raw_field_name, dt_map_type)
				fmt_value = char_str
			else:
				fmt_value = 'X'
		elif (dt_map_type == 'UINT8ARRAY'):
			uint8_str = get_uint8_array_type(raw_field_name)
			fmt_value = uint8_str

		elif (dt_map_type == 'DOUBLEARRAY'):
			double_str = get_commmon_array_type(raw_field_name, dt_map_type)
			fmt_value = double_str

		elif (dt_map_type == 'UINT32ARRAY'):
			uint32_str = get_commmon_array_type(raw_field_name, dt_map_type)
			fmt_value = uint32_str

		elif (dt_map_type == 'STRINGARRAY'):
			str_str = get_str_array_type(dt_type, raw_field_name)
			fmt_value = str_str
		else:
			othr_type = get_othr_type(dt_map_type)
			if (othr_type):
				fmt_value = othr_type[1]
			elif (raw_field_name[0] == '*'):
				fmt_value = CONST.PYDT_PTR_STR
			elif (dt_map_type[:3] == 'DS_'):
				ds_str = get_ds_array_type(dt_type, raw_field_name)
				fmt_value = ds_str
			else:
				fmt_value = 'X'

#		print "###DEBUG### %s %s %s %s" % (tbl_name, raw_field_name, dt_type, dt_map_type)
		if (fmt_value == 'X'):
			print "###WARNING### unknown field size %s %s %s %s" % (tbl_name, raw_field_name, dt_type, dt_map_type)

		if (fmt_value == 'x'):
			value_str = '0'
		elif (fmt_value == 'c' or fmt_value == 'b' or fmt_value == 'B' or fmt_value == '?' or fmt_value == 'h' or fmt_value == 'H'):
			value_str = '0'
		elif (fmt_value == 'l' or fmt_value == 'I' or fmt_value == 'q' or fmt_value == 'Q' or fmt_value == 'f' or fmt_value == 'd'):
			value_str = '0'
		elif (fmt_value[-1:] == 's' or fmt_value == 'p'):
			value_str = '\'\''
		else:
			print "###WARNING### unknown value_str %s %s %s %s %s" % (tbl_name, raw_field_name, dt_type, dt_map_type, fmt_value)			
			value_str = 'XXX'

		print >> fobj, "    %-30s = %s" % (field_name, value_str)
		fmt_array.append(fmt_value)

	fmt_list = ''
	
	fmt_array_len = len(fmt_array)
	for i, fmt_value in enumerate(fmt_array):
		tmp_index = fmt_value.find('*')
		if (tmp_index > 0):
			fmt_list += fmt_value	# has 'XXX'*XXX
		else:
			fmt_list += '\'' + fmt_value + '\''
		if (i != (fmt_array_len-1)):
			fmt_list += '+'

	print >> fobj, "    fmt = %s" % (fmt_list)
	print >> fobj, ""

#########################################################
# 3. check node.txt and other files order
#########################################################
def gen_py_tbl_file(fname, tbl_array):
	'generate python object table file'

	fobj = open(fname, 'w')

	for i, tbl_one in enumerate(tbl_array):
		build_py_tbl_one(fobj, tbl_one[0], tbl_one[2], tbl_one[5], 1)

	fobj.close()
	print "generate file %s" % (fname)

def gen_py_ds_file(fname, tbl_array):
	'generate python object table file'

	fobj = open(fname, 'w')

	for i, ds_one in enumerate(tbl_array):
		build_py_tbl_one(fobj, ds_one[0], ds_one[2], ds_one[4], 0)

	fobj.close()
	print "generate file %s" % (fname)

############################################################
# gen cdb_object_tbl.py
if (GEN_PY_TBL):
	gen_py_tbl_file(CONST.CDB_PY_PATH + 'cdb_object_tbl.py', tbl_array)

############################################################
# gen cdb_object_ds.py
if (GEN_PY_DS):
	gen_py_ds_file(CONST.CDB_PY_PATH + 'cdb_object_ds.py', ds_array)
