import os
import sys
import string
import copy
import inspect
import CONST


def ingreen(s):
	if (type("") == type(s)):
		return "%s[32;2m%s%s[0m" % (chr(27), s, chr(27))
	elif (type(1) == type(s)):
		return "%s[32;2m%d%s[0m" % (chr(27), s, chr(27))
	else:
		return s

def inred(s):
	return "%s[31;2m%s%s[0m" % (chr(27), s, chr(27))

def print_cmp_func(cmp_func):
	print "%s: please make sure function %-20s in files %s" % (inred('Notice'), ingreen(cmp_func), CONST.CMP_FUNC_FILES)

def print_val2str_func(val2str_func):
	print "%s: please make sure function %-20s in files %s" % (inred('Notice'), ingreen(val2str_func), CONST.VAL2STR_FUNC_FILES)

def print_referlist_val2str(table_name):
	print "%s: please make sure add process of %-20s in function %s" % (inred('Notice'), ingreen(table_name), CONST.FUNC_REFERLIST_VAL2STR)

def tmpprint(obj):
	print "tmpprint %s" % obj

def debug_out(debug_on, format, *arg):
	if (debug_on):
		print format%arg

def error_return(format, *arg):
	arglist = []
	for i, argv in enumerate(arg):
		if (type("") == type(argv)):
			tmpargv = ingreen(argv)
		else:
			tmpargv = ingreen(argv)
		arglist.append(tmpargv)
	tmparg = tuple(arglist)
	print format%tmparg
	str = "%s:%d" % inspect.stack()[1][1:3]
	print "called from %s" % inred(str)
	sys.exit(1)

def create_output_dirs():
	if (not os.path.exists(CONST.CDB_H_PATH)):
		os.makedirs(CONST.CDB_H_PATH)
		print "create dir %s" % (CONST.CDB_H_PATH)
	if (not os.path.exists(CONST.CDB_C_PATH)):
		os.makedirs(CONST.CDB_C_PATH)
		print "create dir %s" % (CONST.CDB_C_PATH)
#	if (not os.path.exists(CONST.CDB_PY_PATH)):
#		os.makedirs(CONST.CDB_PY_PATH)
#		print "create dir %s" % (CONST.CDB_PY_PATH)
	if (not os.path.exists(CONST.PBC_H_PATH)):
		os.makedirs(CONST.PBC_H_PATH)
		print "create dir %s" % (CONST.PBC_H_PATH)
	if (not os.path.exists(CONST.PBC_C_PATH)):
		os.makedirs(CONST.PBC_C_PATH)
		print "create dir %s" % (CONST.PBC_C_PATH)
	if (not os.path.exists(CONST.PBC_PY_PATH)):
		os.makedirs(CONST.PBC_PY_PATH)
		print "create dir %s" % (CONST.PBC_PY_PATH)

def is_pointer(str):
	if (0 == len(str)):
		return 0
	if ('*' == str[0]):
		return 1
	else:
		return 0

def format_field_name(field):
	if (is_pointer(field)):
		field_name = field[1:]
	else:
		field_name = field
	array_index = field_name.find('[')
	if (array_index > 0):
		field_name = field_name[:array_index]
	return field_name

def get_array_number(field):
	if (is_pointer(field)):
		field_name = field[1:]
	else:
		field_name = field
	array_index = field_name.find('[')
	if (array_index > 0):
		array_number = field_name[array_index+1:-1]
	return array_number

def check_str_in_array(str_element, str_array):
	for i, item1 in enumerate(str_array):
		if (item1 == str_element):
			return 1
	return 0

def is_typedef_key(key_array):
	if (1 == len(key_array) and 1 < len(key_array[0]) and '' == key_array[0][1]):
		return 1
	else:
		return 0

def is_parent_tbl_array_ds(ds_name, parent_tbl):
	parent_tbl_data_array = parent_tbl[2]
	for i, data_item in enumerate(parent_tbl_data_array):
		if ('ARRAY_DS_' in data_item[0]):
			data_item_name = data_item[0][6:]
			if (ds_name == data_item_name):
				return 1
	return 0

def convert_tbl_master_pm(table):
	'Check tbl order'
	pm_array = CONST.PM_ARRAY
	if (4 != len(table)):
		error_return("Table %s should has exact 4 elements", table)
	table_name = table[0]
	master_pms = table[3]
	pms_array = master_pms.split(',')
	has_all = 0
	for idx, pm_item in enumerate(pms_array):
		if ('all' == pm_item):
			has_all = 1
		is_match = check_str_in_array(pm_item, pm_array)
		if (0 == is_match):
			error_return("Table %s's master PM %s is invalid", table_name, master_pms)
	if (has_all and 1 != len(pms_array)):
			error_return("Table %s's master PM should only has all", table_name)
	table[3] = pms_array
#
# Check node.txt and other files order
#
def check_tbl_order(node_tbl_array, tbl_array, trace_on):
	'Check tbl order'
	node_array_size = len(node_tbl_array)
	array_size = len(tbl_array)
	if (node_array_size != array_size):
		error_return("Table's number mismatch, %s in node.txt and %s in tbl.txt", node_array_size, array_size)
	for idx, item1 in enumerate(node_tbl_array):
		item2 = tbl_array[idx]
		name1 = item1[0]
		name2 = item2[0][0]
		if (name1 != name2):
			error_return("Table's name mismatch at index %s, %s in node.txt and %s in tbl.txt", idx, name1, name2)

def check_ds_order(node_ds_array, ds_array, trace_on):
	'Check ds order'
	node_array_size = len(node_ds_array)
	array_size = len(ds_array)
	if (node_array_size != array_size):
		error_return("DS's number mismatch, %s in node.txt and %s in ds.txt", node_array_size, array_size)
	for idx, item1 in enumerate(node_ds_array):
		item2 = ds_array[idx]
		name1 = item1[0]
		name2 = item2[0][0]
		if (name1 != name2):
			error_return("DS's name mismatch at index %s, %s in node.txt and %s in ds.txt", idx, name1, name2)

def check_act_order(node_act_array, act_array, trace_on):
	'Check ds order'
	node_array_size = len(node_act_array)
	array_size = len(act_array)
	if (node_array_size != array_size):
		error_return("ACT's number mismatch, %s in node.txt and %s in act.txt", node_array_size, array_size)
	for idx, item1 in enumerate(node_act_array):
		item2 = act_array[idx]
		name1 = item1[0]
		name2 = item2[0][0]
		if (name1 != name2):
			error_return("ACT's name mismatch at index %s, %s in node.txt and %s in act.txt", idx, name1, name2)

def get_parent_ds(ds, ds_array):
	if (4 == len(ds)):
		parent_ds_name = ds[3]
		for i, entity_tmp in enumerate(ds_array):
			if (parent_ds_name == entity_tmp[0][0]):
				return entity_tmp
		return None
	else:
		return None

def gen_parent_ds_param_str(parent_ds):
	if (None == parent_ds):
		return ''
	else:
		parent_ds = parent_ds[0]
		parent_ds_name = string.lower(parent_ds[0])
		parent_ds_type = parent_ds_name + '_t'
		parent_ds_value_name = 'p_' + parent_ds[1]
		parent_ds_param_str = parent_ds_type + ' *' + parent_ds_value_name + ', '
		return parent_ds_param_str

def gen_parent_ds_param_value_str(parent_ds):
	if (None == parent_ds):
		return ''
	else:
		parent_ds = parent_ds[0]
		parent_ds_value_name = 'p_' + parent_ds[1]
		parent_ds_param_value_str = parent_ds_value_name + ', '
		return parent_ds_param_value_str

def convert_dest_pm(entity):
	'Convert new entity'
	is_found = 0
	dest_pm = entity[2]
	for idx, pm_item in enumerate(CONST.DEST_PM_ARRAY):
		if (dest_pm == pm_item[0]):
			is_found = 1
			break
	if (0 == is_found):
		error_return("%s's Destination PM %s is invalid", entity[0], dest_pm)
	entity[2] = pm_item[1]
	return entity

def check_product(product_str):
	'Check product'
	if (product_str in CONST.PRODUCT_ARRAY):
		pass
	else:
		error_return("Product %s is not exist", product_str)

def parse_input_node_file(dir_array, tbl_array, ds_array, act_array, bat_array, input_file):
	'Parse input node.txt'
	filename_input = 'node.txt'
	file_in = open(input_file, 'r')
	line_no = 0
	for line_no, eachLine in enumerate(file_in):
		if (eachLine[0] == '/'):
			continue
		else:
			entity = eachLine.split()
			if (0 == len(entity)):
				continue
			if (eachLine[0] == '#'):
				continue

			if (CONST.NODE_PREFIX_DIR == entity[0][:4]):
				if (3 != len(entity)):
					error_return("FILE %s, LINE %s item num != 3, invalid line: %s", filename_input, line_no+1, eachLine)
				check_product(entity[2])
				dir_array.append(entity)
			elif (CONST.NODE_PREFIX_TBL == entity[0][:4]):
				if (4 != len(entity)):
					error_return("FILE %s, LINE %s item num != 4, invalid line: %s", filename_input, line_no+1, eachLine)
				entity = convert_dest_pm(entity)
				check_product(entity[3])
				tbl_array.append(entity)
			elif (CONST.NODE_PREFIX_DS == entity[0][:3]):
				if (3 != len(entity)):
					error_return("FILE %s, LINE %s item num != 3, invalid line: %s", filename_input, line_no+1, eachLine)
				check_product(entity[2])
				ds_array.append(entity)
			elif (CONST.NODE_PREFIX_ACT == entity[0][:4]):
				if (4 != len(entity)):
					error_return("FILE %s, LINE %s item num != 4, invalid line: %s", filename_input, line_no+1, eachLine)
				entity = convert_dest_pm(entity)
				check_product(entity[3])
				act_array.append(entity)
			elif (CONST.NODE_PREFIX_BAT == entity[0][:4]):
				if (3 != len(entity)):
					error_return("FILE %s, LINE %s item num != 3, invalid line: %s", filename_input, line_no+1, eachLine)
				check_product(entity[2])
				bat_array.append(entity)
			else:
				error_return("FILE %s, LINE %s invalid entity: %s", filename_input, line_no+1, entity[0])
	file_in.close()

def build_sub_ds_include(fobj, tbl_name, field_array, is_define_h):
	'build table sub ds include'
	has_sub_ds = 0
	postfix = ''
	if (is_define_h):
		postfix = '_define'
	ds_name_array = []
	for i, field in enumerate(field_array):
		if (len(field) > 4):
			field_name = string.upper(format_field_name(field[4]))
			access_type = field[5]
		else:
			field_name = string.upper(format_field_name(field[1]))
			access_type = field[2]

		if ('SUB' == access_type):
			has_sub_ds = 1
			ds_name = field[0].strip('_t')
			if ('REFERLIST_' in ds_name):
				object_info = get_refer_object_info(field)
				ds_name = string.lower(object_info[2])
			elif ('ARRAY_DS_' in field[0]):
				ds_name = string.lower(ds_name)[6:]
			ds_name_array.append(ds_name)

	ds_name_array = array_remove_duplicate_entry(ds_name_array)
	for i, ds_name in enumerate(ds_name_array):
		print >> fobj, "#include \"gen/%s%s.h\"" % (ds_name, postfix)
	if (has_sub_ds):
		print >> fobj, ""

def build_ds_sub_ds_include(fobj, ds_name, field_array, is_define_h):
	'build ds sub ds include'
	has_sub_ds = 0
	postfix = ''
	if (is_define_h):
		postfix = '_define'
	ds_name_array = []
	for i, field in enumerate(field_array):
		field_name = string.upper(format_field_name(field[1]))
		access_type = field[2]

		if ('SUB' == access_type):
			has_sub_ds = 1
			ds_name = field[0].strip('_t')
			if ('REFERLIST_' in ds_name):
				object_info = get_refer_object_info(field)
				ds_name = string.lower(object_info[2])
			ds_name_array.append(ds_name)

	ds_name_array = array_remove_duplicate_entry(ds_name_array)
	for i, ds_name in enumerate(ds_name_array):
		print >> fobj, "#include \"gen/%s%s.h\"" % (ds_name, postfix)
	if (has_sub_ds):
		print >> fobj, ""

def build_tbl_field_array(table, data_array, flags_array, trace_on):
	'build g table field info'
	field_array = []
	flags_match = 0
	for i, data_ in enumerate(data_array):
		field = []
		field_len = len(data_)
		dt_type = data_[field_len-1]
		if (data_[2] == 'INTERNAL'):
			continue
		elif (data_[2] == 'RW'):
			field.append(data_[0])
			field.append(data_[1])
			field.append(data_[2])
			field.append(dt_type)
		elif (data_[2] == 'READ'):
			field.append(data_[0])
			field.append(data_[1])
			field.append(data_[2])
			field.append(dt_type)
		elif (data_[2] == 'SUB'):
			field.append(data_[0])
			field.append(data_[1])
			field.append(data_[2])
			field.append(dt_type)
		elif (data_[2] == 'FLAGS'):
			flags_match = 0
			for j, flags in enumerate(flags_array):
				field = []
				if (flags[0] == data_[1]):
					field.append(data_[0])
					field.append(data_[1])
					field.append(data_[2])
					field.append(flags[1])
					field.append(flags[2])
					field.append(flags[3].strip())
					field.append(dt_type)
					field_array.append(field)
					flags_match = 1
			if (0 == flags_match):
				error_return("%s [FLAGS] not found flags field %s", table[0], data_[1])
			continue
		field_array.append(field)

	if (len(field_array) > CONST.MAX_FIELD_NUM):
		error_return("%s Field number %s is lager than max %s", table[0], len(field_array), CONST.MAX_FIELD_NUM)		

	if (trace_on):
		for i, field in enumerate(field_array):
			if (len(field) > 4):
				debug_out(trace_on, "%-2d %-20s %-20s %-10s %-20s %-20s %-20s %-10s", i, field[0], field[1], field[2], field[3], field[4], field[5], field[6])
			else:
				debug_out(trace_on, "%-2d %-20s %-20s %-10s %-10s", i, field[0], field[1], field[2], field[3])
	return field_array

def check_tbl_duplcate(table, key_array, data_array, flags_array=None, master_array=None):
	table_name = table[0]
	if (None != key_array):
		tmp_array = key_array
		for i, entry in enumerate(key_array):
			for j, tmp in enumerate(tmp_array):
				if (i == j):
					continue
				if (entry[1] == tmp[1]):
					error_return("%s [KEY] has duplicate %s", table_name, entry[1])

	if (None != data_array):
		tmp_array = data_array
		for i, entry in enumerate(data_array):
			for j, tmp in enumerate(tmp_array):
				if (i == j):
					continue
				if (entry[1] == tmp[1]):
					error_return("%s [DATA] has duplicate %s", table_name, entry[1])

	if (None != flags_array):
		tmp_array = flags_array
		for i, entry in enumerate(flags_array):
			for j, tmp in enumerate(tmp_array):
				if (i == j):
					continue
				if (entry[1] == tmp[1]):
					error_return("%s [FLAGS] has duplicate %s", table_name, entry[1])

	if (None != master_array):
		tmp_array = master_array
		for i, entry in enumerate(master_array):
			for j, tmp in enumerate(tmp_array):
				if (i == j):
					continue
				if (entry[1] == tmp[1]):
					error_return("%s [MASTER] has duplicate %s", table_name, entry[1])

def check_data_dt_type(table, data, dt_array):
	found = 0
	dt_type = data[0]

	if (len(data) > 1):
		if ('[' in data[1]):
			if ('char' == data[0]):
				dt_type += '[]'
			elif ('uint8' == data[0]):
				dt_type += '[]'
			elif ('int32' == data[0]):
				dt_type += '[]'
			elif ('uint32' == data[0]):
				dt_type += '[]'
			elif ('uint64' == data[0]):
				dt_type += '[]'
			elif ('double' == data[0]):
				dt_type += '[]'
			elif ('qos_name_t' == data[0]):
				dt_type += '[]'
			elif ('ifname_info_t' == data[0]):
				dt_type += '[]'
			elif ('ds_' in data[0]):
				pass
			else:
				if ('tbl_route_node_t' != data[0]):
					error_return("%s Only char/int32/uint32/double/ARRAY_DS allows [], but line is %s", table[0], data)

	for i, dt in enumerate(dt_array):
		if (dt_type == dt[0]):
			return dt
	

	return

def find_in_master_array(master_array, table_name, tbl_db_type, db_index):
	table_type = string.lower(table_name) + '_t'
	is_find = 0
	if ('GLOBAL' == tbl_db_type):
		is_find = 1
	elif ('USERDEFINE' == tbl_db_type):
		is_find = 1
	else:
		i = 0
		for i, master in enumerate(master_array):
			if ('HASH' == tbl_db_type):
				if ('ctclib_hash_t' == master[0]):
					is_find = 1
					if ('*' not in master[1]):
						error_return("%s [MASTER] ctclib_hash_t %s should have *", table_name, master[1])
					break
			elif ('LIST' == tbl_db_type):
				if ('ctclib_slist_t' == master[0]):
					is_find = 1
					if ('*' not in master[1]):
						error_return("%s [MASTER] ctclib_slist_t %s should have *", table_name, master[1])
					break
			elif ('ARRAY' == tbl_db_type):
				if (table_type == master[0] and '[' in master[1] and ']' in master[1]):
					is_find = 1
					if ('*' not in master[1]):
						error_return("%s [MASTER] array %s should have *", table_name, master[1])
					break

		if (0 == is_find):
			if ('HASH' == tbl_db_type):
				db_type = 'ctclib_hash_t'
			elif ('LIST' == tbl_db_type):
				db_type = 'ctclib_slist_t'
			elif ('ARRAY' == tbl_db_type):
				db_type = '%s *array[]' % (table_type)
			error_return("%s [MASTER] not found database type %s/%s", table_name, tbl_db_type, db_type)
		if (i != db_index):
			error_return("%s [MASTER] index mismatch (%s, %s) vs (%s, %s)", table_name, master[0], i, tbl_db_type, db_index)

def check_input_tbl(tbl_one):
	table = tbl_one[0]
	master_array = tbl_one[4]
	table_name = table[0]
	tbl_db_type = table[2]
	if ('HASH' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'HASH', 0)
	elif ('HASH+LIST' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'HASH', 0)
		find_in_master_array(master_array, table_name, 'LIST', 1)
	elif ('LIST' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'LIST', 0)
		pass
	elif ('ARRAY' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'ARRAY', 0)
	elif ('USERDEFINE' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'USERDEFINE', 0)
	elif ('GLOBAL' == tbl_db_type):
		find_in_master_array(master_array, table_name, 'GLOBAL', 0)
	else:
		error_return("%s [TBL] Invalid table type %s", table_name, tbl_db_type)

def get_tbl_key(eachLine):
	line = eachLine.split(None, 2)
	if (len(line) == 1):
		line.append('')
	elif (len(line) > 1 and '/*' in line[1]):
		tmpline = eachLine.split(None, 1)
		line = []
		line.append(tmpline[0])
		line.append('')
		line.append(tmpline[1])
	return line

def parse_input_tbl_file(tbl_array, dt_array, filename_input, trace_on):
	line_type = CONST.TBL_LINE_INVALID	
	key_array = []
	data_array = []
	flags_array = []
	master_array = []
	field_array = []
	file_in = open(filename_input, 'r')
	for line_no, eachLine in enumerate(file_in):
		is_empty = is_empty_line(eachLine)
		if (is_empty):
			#print "LINE[%d] empty line, len %d" % (line_no, len(eachLine))
			continue

		if (eachLine[:5] == CONST.TBL_PREFIX_TBL):
			line_type = CONST.TBL_LINE_TBL
			continue
		elif (eachLine[:5] == CONST.TBL_PREFIX_KEY):
			line_type = CONST.TBL_LINE_KEY
			continue
		elif (eachLine[:6] == CONST.TBL_PREFIX_DATA):
			line_type = CONST.TBL_LINE_DATA
			continue
		elif (eachLine[:7] == CONST.TBL_PREFIX_FLAGS):
			line_type = CONST.TBL_LINE_FLAGS
			continue
		elif (eachLine[:8] == CONST.TBL_PREFIX_MASTER):
			line_type = CONST.TBL_LINE_MASTER
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			if (CONST.TBL_LINE_INVALID == line_type):
				continue
			elif (CONST.TBL_LINE_MASTER == line_type):
				line_type = CONST.TBL_LINE_END
			else:
				error_return("FILE %s, LINE %s: must has [MASTER] field before %s", filename_input, line_no+1, eachLine)
		else:
			if (CONST.TBL_LINE_INVALID == line_type):
				error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.TBL_LINE_TBL == line_type):
			line = eachLine.split(None, 4)
			table = line
			convert_tbl_master_pm(table)
		elif (CONST.TBL_LINE_KEY == line_type):
			line = get_tbl_key(eachLine)
			key_array.append(line)
		elif (CONST.TBL_LINE_DATA == line_type):
			line = eachLine.split(None, 3)
			check_data(filename_input, line_no, line)
			tmpline = copy.deepcopy(line)
			if ('KEY' in line[0]):
				line[0] = string.lower(line[0]) + '_t'
				tmpline[0] = line[0]
			if ('REFER_' in line[0]):
				tmpline[0] = 'cdb_reference_t'
			if ('REFERLIST_' in line[0]):
				tmpline[0] = 'cdb_reference_list_t'
			if ('ARRAY_DS_' in line[0]):
				tmpline[0] = string.lower(line[0])[6:] + '_t'
			dt = check_data_dt_type(table, tmpline, dt_array)
			if (None == dt):
				if (line[2] == 'INTERNAL'):
					pass
				else:
					error_return("FILE %s, LINE %s: not found datatype %s in datatype.txt [DT_MAP]", filename_input, line_no+1, tmpline[0])
			else:
				if ('FLAGS8' == line[0]):
					line[0] = 'uint8'
				if ('FLAGS32' == line[0]):
					line[0] = 'uint32'
				if ('ENUM8' == dt[1] and 'key' != line[1]):
					line[0] = 'uint8'
				if ('ENUM32' == dt[1] and 'key' != line[1]):
					line[0] = 'uint32'
				line.append(dt)
			data_array.append(line)
		elif (CONST.TBL_LINE_FLAGS == line_type):
			line = eachLine.split(None, 3)
			flags_array.append(line)
		elif (CONST.TBL_LINE_MASTER == line_type):
			line = eachLine.split(None, 2)
			master_array.append(line)
		elif (CONST.TBL_LINE_END == line_type):
			tbl_one = []
			field_array = build_tbl_field_array(table, data_array, flags_array, trace_on)
			tbl_one.append(table)
			tbl_one.append(key_array)
			tbl_one.append(data_array)
			tbl_one.append(flags_array)
			tbl_one.append(master_array)
			tbl_one.append(field_array)
			check_tbl_duplcate(table, key_array, data_array, flags_array, master_array)
			check_input_tbl(tbl_one)
			tbl_array.append(tbl_one)
			key_array = []
			data_array = []
			flags_array = []
			master_array = []
			field_array = []
	file_in.close()

def ds_get_data_type(data_one):
	data_type = data_one[0]
	if ('REFERLIST_' in data_type):
		object_info = get_refer_object_info(data_one)
		data_type = string.lower(object_info[2]) + '_t'
	if ('ARRAY_DS_' in data_type):
		data_type = string.lower(data_type)[6:] + '_t'
	return data_type

def ds_get_parent_tbl_field(parent_tbl, ds):
	tbl_name = parent_tbl[0][0]
	ds_name = ds[0]
	ds_type = string.lower(ds[0]) + '_t'
	data_array = parent_tbl[2]
	tbl_found_ds = 0
	for j, data_one in enumerate(data_array):
		data_type = ds_get_data_type(data_one)
		if (data_type == ds_type):
			tbl_found_ds = 1
			break
	if (0 == tbl_found_ds):
		error_return("DS %s cannot found in parent TBL %s", ds_name, tbl_name)
	return format_field_name(data_one[1])

def ds_get_parent_ds_field(parent_ds, ds):
	parent_ds_name = parent_ds[0][0]
	ds_name = ds[0]
	ds_type = string.lower(ds[0]) + '_t'
	data_array = parent_ds[2]
	tbl_found_ds = 0
	for j, data_one in enumerate(data_array):
		data_type = ds_get_data_type(data_one)
		if (data_type == ds_type):
			tbl_found_ds = 1
			break
	if (0 == tbl_found_ds):
		error_return("DS %s cannot found in parent DS %s", ds_name, parent_ds_name)
	return format_field_name(data_one[1])


def check_referlist(tbl_array, ds_array, trace_on):
	referlist_array = []
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		data_array = tbl_one[2]
		for j, field in enumerate(data_array):
			if ('REFERLIST_' in field[0]):
				referlist_tbl_name = field[0][10:]
				referlist_array.append(referlist_tbl_name)

	for i, tbl_one in enumerate(ds_array):
		tbl_name = tbl_one[0][0]
		data_array = tbl_one[2]
		for j, field in enumerate(data_array):
			if ('REFERLIST_' in field[0]):
				referlist_tbl_name = field[0][10:]
				referlist_array.append(referlist_tbl_name)

	for i, referlist_tbl_name in enumerate(referlist_array):
		if ('TBL_' in referlist_tbl_name):
			is_found = 0
			for j, tbl in enumerate(tbl_array):
				tbl_name = tbl[0][0]
				if (referlist_tbl_name == tbl_name):
					is_found = 1
					break
			if (0 == is_found):
				error_return("Referlist %s does not exist in tbl.txt", referlist_tbl_name)

		if ('DS_' in referlist_tbl_name):
			is_found = 0
			for j, tbl in enumerate(ds_array):
				tbl_name = tbl[0][0]
				if (referlist_tbl_name == tbl_name):
					is_found = 1
					break
			if (0 == is_found):
				error_return("Referlist %s does not exist in ds.txt", referlist_tbl_name)

def check_ds_parent_tbl(tbl_array, ds_array, ds, trace_on):
	'check ds parent table'
	ds_name = ds[0]
	ds_type = string.lower(ds[0]) + '_t'
	parent_tbl_name = ds[2]
	found_tbl = 0
	found_ds = 0
	tbl_found_ds = 0
	ds_found_ds = 0

	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		if (tbl_name == parent_tbl_name):
			found_tbl = 1
			break
	if (0 == found_tbl):
		error_return("Not found DS %s parent table %s", ds_name, parent_tbl_name)

	if (3 == len(ds)):
		data_array = tbl_one[2]
		for j, data_one in enumerate(data_array):
			data_type = ds_get_data_type(data_one)
			if (data_type == ds_type):
				tbl_found_ds = 1
	elif (4 == len(ds)):
		parent_ds_name = ds[3]
		for i, ds_one in enumerate(ds_array):
			tmp_ds_name = ds_one[0][0]
			if (tmp_ds_name == parent_ds_name):
				found_ds = 1
				break
		if (0 == found_ds):
			error_return("Not found DS %s parent DS %s", ds_name, parent_ds_name)

		data_array = ds_one[2]
		for j, data_one in enumerate(data_array):
			data_type = ds_get_data_type(data_one)
			if (data_type == ds_type):
				ds_found_ds = 1
	else:
		pass

	if (3 == len(ds)):
		if (0 == tbl_found_ds):
			error_return("Not found DS type %s in parent table %s field", ds_type, parent_tbl_name)
	else:
		if (0 == ds_found_ds):
			error_return("Not found DS type %s in parent DS %s field", ds_type, parent_ds_name)
			
	debug_out(trace_on, "DS %s parent table %s check pass" % (ds_name, tbl_name))
	return tbl_one

def check_data(filename_input, line_no, line):
	if (len(line) < 3):
		error_return("FILE %s, LINE %s: item num should be >=3: %s", filename_input, line_no+1, line)

	if ('REFERLIST_DS_' in line[0]):
		if ('SUB' != line[2]):
			error_return("FILE %s, LINE %s: line[0] %s mismatch with line[2] %s, should be %s", filename_input, line_no+1, line[0], line[2], 'SUB')
	elif ('ds_' in line[0]):
		if ('SUB' != line[2]):
			error_return("FILE %s, LINE %s: line[0] %s mismatch with line[2] %s, should be %s", filename_input, line_no+1, line[0], line[2], 'SUB')

	if ('READ' == line[2]):
		pass
	elif ('RW' == line[2]):
		pass
	elif ('FLAGS' == line[2]):
		pass
	elif ('INTERNAL' == line[2]):
		pass
	elif ('SUB' == line[2]):
		match = 0
		if ('REFERLIST_DS_' in line[0]):
			match = 1
		if ('ARRAY_DS_' in line[0]):
			match = 1
		if ('ds_' in line[0]):
			match = 1
		if (0 == match):
			error_return("FILE %s, LINE %s: line[0] %s mismatch with line[2] %s", filename_input, line_no+1, line[0], line[2])
		pass
	else:
		error_return("FILE %s, LINE %s: line access type %s invalid", filename_input, line_no+1, line[2])

def is_empty_line(line):
	line_len = len(line)
	if (line_len == 0):
		return 1
	else:
		for idx, eachchar in enumerate(line):
			if (eachchar != ' ' and eachchar != '\r' and eachchar != '\n'):
				return 0
		return 1

def parse_input_ds_file(tbl_array, ds_array, dt_array, filename_input, trace_on):
	ds_line_type = CONST.DS_LINE_INVALID
	key_array = []
	data_array = []
	flags_array = []
	field_array = []
	file_in_ds = open(filename_input, 'r')
	for line_no, eachLine in enumerate(file_in_ds):
		is_empty = is_empty_line(eachLine)
		if (is_empty):
			#print "LINE[%d] empty line, len %d" % (line_no, len(eachLine))
			continue
		if (eachLine[:4] == CONST.TBL_PREFIX_DS):
			ds_line_type = CONST.DS_LINE_DS
			continue
		elif (eachLine[:5] == CONST.TBL_PREFIX_KEY):
			ds_line_type = CONST.DS_LINE_KEY
			continue
		elif (eachLine[:6] == CONST.TBL_PREFIX_DATA):
			ds_line_type = CONST.DS_LINE_DATA
			continue
		elif (eachLine[:7] == CONST.TBL_PREFIX_FLAGS):
			ds_line_type = CONST.DS_LINE_FLAGS
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			if (CONST.DS_LINE_INVALID == ds_line_type):
				continue
			elif (CONST.DS_LINE_DATA == ds_line_type):
				ds_line_type = CONST.DS_LINE_END
			elif (CONST.DS_LINE_FLAGS == ds_line_type):
				ds_line_type = CONST.DS_LINE_END
			else:
				error_return("FILE %s, LINE %s: must has [DATA] or [FLAGS] field before %s", filename_input, line_no+1, eachLine)
		else:
			if (CONST.DS_LINE_INVALID == ds_line_type):
				error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.DS_LINE_DS == ds_line_type):
			line = eachLine.split(None, 3)
			# strip last entity
			if (4 == len(line)):
				line[3] = line[3].strip()
			ds = line
		if (CONST.DS_LINE_KEY == ds_line_type):
			line = get_tbl_key(eachLine)
			key_array.append(line)
		elif (CONST.DS_LINE_DATA == ds_line_type):
			line = eachLine.split(None, 3)
			check_data(filename_input, line_no, line)
			tmpline = copy.deepcopy(line)
			if ('KEY' in line[0]):
				line[0] = string.lower(line[0]) + '_t'
				tmpline[0] = line[0]
			if ('REFER_' in line[0]):
				tmpline[0] = 'cdb_reference_t'
			if ('REFERLIST_' in line[0]):
				tmpline[0] = 'cdb_reference_list_t'
			dt = check_data_dt_type(ds, tmpline, dt_array)
			if (None == dt):
				error_return("FILE %s, LINE %s: not found datatype %s in datatype.txt [DT_MAP]", filename_input, line_no+1, tmpline[0])
			if ('FLAGS8' == line[0]):
				line[0] = 'uint8'
			if ('FLAGS32' == line[0]):
				line[0] = 'uint32'
			if ('ENUM8' == dt[1]):
				line[0] = 'uint8'
			if ('ENUM32' == dt[1]):
				line[0] = 'uint32'
			line.append(dt)
			data_array.append(line)
		elif (CONST.DS_LINE_FLAGS == ds_line_type):
			line = eachLine.split(None, 3)
			flags_array.append(line)
		elif (CONST.DS_LINE_END == ds_line_type):
			field_array = build_tbl_field_array(ds, data_array, flags_array, trace_on)
			parent_tbl = check_ds_parent_tbl(tbl_array, ds_array, ds, trace_on)
			ds_one = []
			ds_one.append(ds)
			ds_one.append(key_array)
			ds_one.append(data_array)
			ds_one.append(parent_tbl)
			ds_one.append(field_array)
			check_tbl_duplcate(ds, key_array, data_array, None, None)
			ds_array.append(ds_one)
			key_array = []
			data_array = []
			flags_array = []
			field_array = []
	file_in_ds.close()

def parse_input_act_file(act_array, filename_input, trace_on):
	act_line_type = CONST.ACT_LINE_INVALID
	data_array = []
	file_in_act = open(filename_input, 'r')
	for line_no, eachLine in enumerate(file_in_act):
		is_empty = is_empty_line(eachLine)
		if (is_empty):
			#print "LINE[%d] empty line, len %d" % (line_no, len(eachLine))
			continue

		if (eachLine[:5] == CONST.ACT_PREFIX_ACT):
			act_line_type = CONST.ACT_LINE_ACT
			continue
		elif (eachLine[:5] == CONST.ACT_PREFIX_SUB):
			act_line_type = CONST.ACT_LINE_SUB
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			if (CONST.ACT_LINE_INVALID == act_line_type):
				continue
			elif (CONST.ACT_LINE_SUB == act_line_type):
				act_line_type = CONST.ACT_LINE_END
			else:
				error_return("FILE %s, LINE %s: must has [DATA] field before %s", filename_input, line_no+1, eachLine)
		else:
			if (CONST.ACT_LINE_INVALID == act_line_type):
				error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.ACT_LINE_ACT == act_line_type):
			line = eachLine.split(None, 1)
			act = line
		elif (CONST.ACT_LINE_SUB == act_line_type):
			line = eachLine.split(None, 3)
			data_array.append(line)
		elif (CONST.ACT_LINE_END == act_line_type):
			act_one = []
			act_one.append(act)
			act_one.append(data_array)
			act_array.append(act_one)
			data_array = []
	file_in_act.close()

def check_dt_type(dt, dt_oper_array, trace_on):
	found = 0
	if ('DS_' == dt[1][:3]):
		return
	for i, dt_type in enumerate(dt_oper_array):
		if (dt[1] == dt_type[0]):
			found = 1
			break
	if (0 == found):
		error_return("Not found datatype oper %s in datatype.txt [DT_TYPE]", dt)
	return dt_type

def parse_input_dt_file(dt_oper_array, dt_array, filename_input, trace_on):
	dt_line_type = CONST.DT_LINE_INVALID
	file_in_dt = open(filename_input, 'r')
	for line_no, eachLine in enumerate(file_in_dt):
		if (eachLine[:9] == CONST.DT_PREFIX_DT_TYPE):
			dt_line_type = CONST.DT_LINE_DT_TYPE
			continue
		elif (eachLine[:8] == CONST.DT_PREFIX_DT):
			dt_line_type = CONST.DT_LINE_DT
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			continue
		else:
			if (CONST.DT_LINE_INVALID == dt_line_type):
				error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.DT_LINE_DT_TYPE == dt_line_type):
			line = eachLine.split(None, 5)
			if (0 == len(line)):
				continue
			line_len = len(line)-1
			line[line_len] = line[line_len].strip()
			dt_oper_array.append(line)
		elif (CONST.DT_LINE_DT == dt_line_type):
			line = eachLine.split(None, 2)
			if (0 == len(line)):
				continue
			if (1 == len(line)):
				line.append(line[0])
			dt_type = check_dt_type(line, dt_oper_array, trace_on)
			line.append(dt_type)
			dt_array.append(line)
		elif (CONST.DT_LINE_END == dt_line_type):
			pass
	file_in_dt.close()

def parse_input_pydt_file(py_OTHR_TYPES_array, py_UINT_TYPES_array, py_STRING_KEY_array, py_CHAR_ARRAY_array, filename_input, trace_on):
	line_type = CONST.PYDT_LINE_INVALID
	file_in_dt = open(filename_input, 'r')
	for line_no, eachLine in enumerate(file_in_dt):
		if (eachLine[:12] == CONST.PYDT_OTHR_TYPES):
			line_type = CONST.PYDT_LINE_OTHR_TYPES
			continue
		elif (eachLine[:12] == CONST.PYDT_UINT_TYPES):
			line_type = CONST.PYDT_LINE_UINT_TYPES
			continue
		elif (eachLine[:12] == CONST.PYDT_STRING_KEY):
			line_type = CONST.PYDT_LINE_STRING_KEY
			continue
		elif (eachLine[:12] == CONST.PYDT_CHAR_ARRAY):
			line_type = CONST.PYDT_LINE_CHAR_ARRAY
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			continue
		else:
			if (CONST.DT_LINE_INVALID == line_type):
				error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.PYDT_LINE_OTHR_TYPES == line_type):
			line = eachLine.split(None, 2)
			if (0 == len(line)):
				continue
			line_len = len(line)-1
			line[line_len] = line[line_len].strip()
			py_OTHR_TYPES_array.append(line)
		elif (CONST.PYDT_LINE_UINT_TYPES == line_type):
			line = eachLine.split(None, 2)
			if (0 == len(line)):
				continue
			line_len = len(line)-1
			line[line_len] = line[line_len].strip()
			py_UINT_TYPES_array.append(line)
		elif (CONST.PYDT_LINE_STRING_KEY == line_type):
			line = eachLine.split(None, 2)
			if (0 == len(line)):
				continue
			line_len = len(line)-1
			line[line_len] = line[line_len].strip()
			py_STRING_KEY_array.append(line)
		elif (CONST.PYDT_LINE_CHAR_ARRAY == line_type):
			line = eachLine.split(None, 2)
			if (0 == len(line)):
				continue
			line_len = len(line)-1
			line[line_len] = line[line_len].strip()
			py_CHAR_ARRAY_array.append(line)
		elif (CONST.PYDT_LINE_END == line_type):
			pass
	file_in_dt.close()

def insert_tbl_user_define_begin(file_tbl, tbl_name, file_postfix):
	fname = 'user_define/' + string.lower(tbl_name) + '_begin' + file_postfix
	if (os.path.exists(fname)):
		file_begin = open(fname, 'r')
		for eachLine in file_begin:
			file_tbl.write(eachLine)
		print >> file_tbl, ""
	else:
		pass

def insert_tbl_user_define_beginmaster(file_tbl, tbl_name, file_postfix):
	fname = 'user_define/' + string.lower(tbl_name) + '_beginmaster' + file_postfix
	if (os.path.exists(fname)):
		file_begin = open(fname, 'r')
		for eachLine in file_begin:
			file_tbl.write(eachLine)
		print >> file_tbl, ""
	else:
		pass

def insert_tbl_user_define_endfield(file_tbl, tbl_name, file_postfix):
	fname = 'user_define/' + string.lower(tbl_name) + '_endfield' + file_postfix
	if (os.path.exists(fname)):
		file_begin = open(fname, 'r')
		for eachLine in file_begin:
			file_tbl.write(eachLine)
	else:
		pass

def insert_tbl_user_define_end(file_tbl, tbl_name, file_postfix):
	fname = 'user_define/' + string.lower(tbl_name) + '_end' + file_postfix
	if (os.path.exists(fname)):
		file_end = open(fname, 'r')
		for eachLine in file_end:
			file_tbl.write(eachLine)
		print >> file_tbl, ""
	else:
		pass

def get_tbl_api_func_prototype(tbl_name, action, accusative, paramtype, paramvalue, suffix=''):
	suffix_str = ''
	if ('_sync' == suffix):
		suffix_str = ', uint32 sync'
	if (paramtype):
		funcstr = "%s_%s_%s%s(%s %s%s)" % (tbl_name, action, accusative, suffix, paramtype, paramvalue, suffix_str)
	else:
		funcstr = "%s_%s_%s%s(%s)" % (tbl_name, action, accusative, suffix, suffix_str)
	return funcstr

def dt_type_has_val2str(dt_type):
	if (None == dt_type[2]):	# for DS_ 
		return 0

	if (len(dt_type[2]) >= 5):
		if ('val2str' in dt_type[2][4]):
			return 1
		else:
			return 0
	else:
		return 0

def check_dump_one_use_buf(table, key_array, field_array, dt_array, is_ds):
	has_buf = 0
	key_has_val2str = 0
	data_has_val2str = 0
	if (len(key_array)):
		dt_type = check_data_dt_type(table, key_array[0], dt_array)	
		key_has_val2str = dt_type_has_val2str(dt_type)
	for i, field in enumerate(field_array):
		if (is_ds):
			field_len = len(field)
			dt_type = field[field_len-1]
		else:
			if (len(field) > 4):
				dt_type = field[6]
			else:
				dt_type = field[3]
		data_has_val2str = dt_type_has_val2str(dt_type)
		if (1 == data_has_val2str):
			break
	has_buf = (key_has_val2str or data_has_val2str)
	return has_buf

def has_cdb_tailq_head_list(data_array):
	tailq_head_array = []
	for i, field in enumerate(data_array):
		if ('_tailq_head_t' in field[0]):
			tbl_field = 'tbl_' + field[0][:-13]
			tailq_head_array.append(tbl_field)		
	return tailq_head_array

def has_cdb_tailq_head_param_list(data_array):
	tailq_head_array = []
	for i, field in enumerate(data_array):
		if ('_tailq_head_t' in field[0]):
			tbl_field = field[1]
			tailq_head_array.append(tbl_field)		
	return tailq_head_array

def include_tailq_head_h_file(fobj, data_array, table_name):
	tailq_head_array = has_cdb_tailq_head_list(data_array)
	tailq_head_array = array_remove_duplicate_entry(tailq_head_array)
	for i, refer_name in enumerate(tailq_head_array):
		print refer_name
		fname_define_h = 'gen/' + string.lower(refer_name) + '_define.h'
		fname_h = 'gen/' + string.lower(refer_name) + '.h'
		print >> fobj, "#include \"%s\"" % (fname_define_h)

def has_cdb_refer_list(field_array, referlist_type=CONST.REFERLIST_TYPE_ALL):
	referlist_array = []
	for i, field in enumerate(field_array):
		if ('REFERLIST_' in field[0]):
			if (CONST.REFERLIST_TYPE_ALL == referlist_type):
				referlist_array.append(field)
			elif (CONST.REFERLIST_TYPE_TBL == referlist_type):
				if ('REFERLIST_TBL' in field[0]):
					referlist_array.append(field)
			elif (CONST.REFERLIST_TYPE_DS == referlist_type):
				if ('REFERLIST_DS' in field[0]):
					referlist_array.append(field)		
	return referlist_array

def has_cdb_array_ds(field_array):
	array_ds_array = []
	for i, field in enumerate(field_array):
		if ('ARRAY_DS_' in field[0]):
			array_ds_array.append(field)
	return array_ds_array

def get_cdb_referlist_field(referlist_array, field_name):
	for i, referlist_field in enumerate(referlist_array):
		if (field_name == referlist_field[0]):
			return referlist_field

def has_cdb_refer(field_array, is_ds):
	for i, field in enumerate(field_array):
		if ('REFER_' == field[0]):
			return field
	return

def get_refer_object_info(refer_list_field):
	object_info = []
	field_name = format_field_name(refer_list_field[1])
	refer_name = refer_list_field[0][len('REFERLIST_'):]
	if (CONST.NODE_PREFIX_TBL in refer_name):
		refer_type = 'CDB_NODE_TBL'
	elif (CONST.NODE_PREFIX_DS in refer_name):
		refer_type = 'CDB_NODE_DS'
	else:
		error_return("Invalid reference list object %s", refer_list_field[0])
	object_info.append(field_name)
	object_info.append(refer_type)
	object_info.append(refer_name)
	return object_info

def include_refer_tbl_ds_h_file(fobj, referlist_array, table_name):
	refer_name_array = []
	for i, referlist_field in enumerate(referlist_array):
		object_info = get_refer_object_info(referlist_field)
		refer_name = object_info[2]
		if (table_name != refer_name):
			refer_name_array.append(refer_name)
	
	refer_name_array = array_remove_duplicate_entry(refer_name_array)
	for i, refer_name in enumerate(refer_name_array):
		fname_define_h = 'gen/' + string.lower(refer_name) + '_define.h'
		fname_h = 'gen/' + string.lower(refer_name) + '.h'
		print >> fobj, "#include \"%s\"" % (fname_define_h)
		print >> fobj, "#include \"%s\"" % (fname_h)

def include_array_tbl_ds_h_file(fobj, array_ds_array, table_name):
	refer_name_array = []
	for i, array_ds in enumerate(array_ds_array):
		refer_name = array_ds[0][6:]
		if (table_name != refer_name):
			refer_name_array.append(refer_name)
	
	refer_name_array = array_remove_duplicate_entry(refer_name_array)
	for i, refer_name in enumerate(refer_name_array):
		fname_define_h = 'gen/' + string.lower(refer_name) + '_define.h'
		fname_h = 'gen/' + string.lower(refer_name) + '.h'
		print >> fobj, "#include \"%s\"" % (fname_define_h)
		print >> fobj, "#include \"%s\"" % (fname_h)

def array_remove_duplicate_entry(input_array):
	output_array = []
	for i, entry in enumerate(input_array):
		if (entry in output_array):
			continue
		output_array.append(entry)
	return output_array

def pb_get_cdb_compose_reference(compose_type):
	if ('mac_addr_t' == compose_type):
		return ""
	else:
		return "&"

glb_create_pb_type_dict = 0
glb_pb_type_dict = {}

def pb_create_dict(dt_array):
	global glb_pb_type_dict
	for i, dt in enumerate(dt_array):
		dt_map = dt[2]
		if (None == dt_map):
			continue
		cdb_type = dt_map[0]
		pb_type  = dt_map[3]
		if ('repeat' == pb_type[:6]):
			pb_type = pb_type.split(',')
		glb_pb_type_dict[cdb_type] = pb_type

#   dump the pb_type dict
#	for i, pb_type_item in enumerate(glb_pb_type_dict):
#		print "BBBBB %s %s %s" % (i, pb_type_item, glb_pb_type_dict[pb_type_item])

def pb_get_pb_type(dt_type, dt_array):
	global glb_create_pb_type_dict
	global glb_pb_type_dict
	if (0 == glb_create_pb_type_dict):
		pb_create_dict(dt_array)
		glb_create_pb_type_dict = 1

	if (glb_pb_type_dict.has_key(dt_type[1])):
		return glb_pb_type_dict[dt_type[1]]
	else:
		return 'none'

#	if ('UINT' == dt_type[1]):
#		return 'uint32'
#	elif ('UINT64' == dt_type[1]):
#		return 'uint64'
#	elif ('INT' == dt_type[1]):
#		return 'int32'
#	elif ('INT64' == dt_type[1]):
#		return 'int64'
#	elif ('ENUM8' == dt_type[1]):
#		return 'uint32'
#	elif ('ENUM32' == dt_type[1]):
#		return 'uint32'
#	elif ('FLAGS8' == dt_type[1]):
#		return 'uint32'
#	elif ('FLAGS32' == dt_type[1]):
#		return 'uint32'
#	elif ('STRING' == dt_type[1]):
#		return 'string'
#	elif ('DOUBLE' == dt_type[1]):
#		return 'double'
#	elif ('TASK' == dt_type[1]):
#		return 'none'
#	elif ('VOID' == dt_type[1]):
#		return 'none'
#	elif ('cdb_reference_list_t' == dt_type[1]):
#		return 'none'
#	elif ('BITMAP' == dt_type[1]):
#		return 'bytes'
#	elif ('INT8ARRAY' == dt_type[1]):
#		return 'bytes'
#	elif ('UINT8ARRAY' == dt_type[1]):
#		return 'bytes'
#	elif ('INT32ARRAY' == dt_type[1]):
#		return ['repeat', 'int32']
#	elif ('UINT32ARRAY' == dt_type[1]):
#		return ['repeat', 'uint32']
#	elif ('DOUBLEARRAY' == dt_type[1]):
#		return ['repeat', 'double']
#	elif ('STRINGARRAY' == dt_type[1]):
#		return ['repeat', 'string']
#	elif ('vlan_memberport_t' == dt_type[1]):
#		return 'bytes'
#	elif ('fdb_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('mac_addr_t' == dt_type[1]):
#		return 'compose'
#	elif ('brg_allowed_vlan_t' == dt_type[1]):
#		return 'compose'
#	elif ('addr_ipv4_t' == dt_type[1]):
#		return 'compose'
#	elif ('prefix_ipv4_t' == dt_type[1]):
#		return 'compose'
#	elif ('prefix_t' == dt_type[1]):
#		return 'compose'
#	elif ('glb_board_type_t' == dt_type[1]):
#		return 'compose'
#	elif ('ds_connected_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('time_range_value_t' == dt_type[1]):
#		return 'compose'
#	elif ('time_range_timer_t' == dt_type[1]):
#		return 'compose'
#	elif ('glb_flow_l4_port_t' == dt_type[1]):
#		return 'compose'
#	elif ('glb_flow_tcp_flag_t' == dt_type[1]):
#		return 'compose'
#	elif ('glb_stats_t' == dt_type[1]):
#		return 'compose'
#	elif ('glb_if_stats_t' == dt_type[1]):
#		return 'compose'
#	elif ('sal_time_t' == dt_type[1]):
#		return 'compose'
#	elif ('timestamp_t' == dt_type[1]):
#		return 'compose'
#	elif ('route_node_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('nexthop_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('aclqos_if_queue_t' == dt_type[1]):
#		return 'compose'
#	elif ('addr_t' == dt_type[1]):
#		return 'compose'
#	elif ('snmp_trap_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('snmp_inform_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('mail_t' == dt_type[1]):
#		return 'compose'
#	elif ('rsa_keystr_t' == dt_type[1]):
#		return 'compose'
#	elif ('mstp_brg_id_t' == dt_type[1]):
#		return 'compose'
#	elif ('binding_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('ns_port_forwarding_key_t' == dt_type[1]):
#		return 'compose'
#	elif ('mstp_digest_t' == dt_type[1]):
#		return 'compose'
#	elif ('port_msti_key_t' == dt_type[1]):
#		return 'compose'
#	else:
#		return 'todo'

def pb_get_compse_name(cdb_name):
	tbl_name_low = string.lower(cdb_name)
	strip_tbl_name = tbl_name_low
	new_tbl_name = ""
	next_upper = 0
	for i, char in enumerate(strip_tbl_name):
		if ('_' == char):
			next_upper = 1
			continue
		if (next_upper):
			next_upper = 0
			new_tbl_name += string.upper(char)
		else:
			if (i == 0):
				new_tbl_name += string.upper(char)
			else:
				new_tbl_name += char
	return 'Cdb__Compose' + new_tbl_name

def pb_get_tbl_name(tbl_name):
	tbl_name_low = string.lower(tbl_name)
	strip_tbl_name = tbl_name_low[3:]
	new_tbl_name = ""
	next_upper = 0
	for i, char in enumerate(strip_tbl_name):
		if ('_' == char):
			next_upper = 1
			continue
		if (next_upper):
			next_upper = 0
			new_tbl_name += string.upper(char)
		else:
			new_tbl_name += char
	return 'Cdb__Tbl' + new_tbl_name

def pb_get_ds_name(ds_name):
	ds_name_low = string.lower(ds_name)
	strip_tbl_name = ds_name_low[2:]
	new_tbl_name = ""
	next_upper = 0
	for i, char in enumerate(strip_tbl_name):
		if ('_' == char):
			next_upper = 1
			continue
		if (next_upper):
			next_upper = 0
			new_tbl_name += string.upper(char)
		else:
			new_tbl_name += char
	return 'Cdb__Ds' + new_tbl_name

def pb_get_key_compose_not_typedef(table, key, dt_array):
	key_dt_type = check_data_dt_type(table, key, dt_array)
	key_pb_type = pb_get_pb_type(key_dt_type, dt_array)
	if ('compose' == key_pb_type and '' != key[1]):
		return key
	else:
		return None
