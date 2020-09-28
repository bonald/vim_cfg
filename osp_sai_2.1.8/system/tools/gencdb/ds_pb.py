import os
import sys
import string
import CONST
import cdb_util

filename_out_proto = CONST.PBC_PROTO_DS
filename_out_pb_h  = CONST.PBC_H_PATH + 'pb_ds.h'
filename_out_pb_c  = CONST.PBC_C_PATH + 'pb_ds.c'

glb_field_index = 0


def build_tbl_key_message(fobj, ds, key_array, field, dt_array):
	'build table key message'
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	key = key_array[0]
	global glb_field_index

	dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
	if (1 != len(key_array)):
		print "%s key num %d" % (ds_name, len(key_array))

	for i, key in enumerate(key_array):
		bypass = 0
		field_name = cdb_util.format_field_name(key[1])
		pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
		if (2 == len(pb_type)):
			bypass = 1
		else:
			if ('compose' == pb_type):
				compose_type = dt_type[1]
				compose_type = 'compose_' + compose_type
			elif ('none' == pb_type):
				bypass = 1
			elif ('todo' == pb_type):
				bypass = 1

		if (bypass):
			return

		if ('compose' == pb_type):
			print >> fobj, "    required %s %s = %d;" % (compose_type, field_name, glb_field_index)
		else:
			print >> fobj, "    required %s %s = %d;" % (pb_type, field_name, glb_field_index)
		glb_field_index += 1


def get_compose_field_array(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	compose_array = []
	for i, field in enumerate(field_array):
		ds_name = ds[0]
		ds_name_low = string.lower(ds_name[0])
		ds_name_low_key = ds_name_low + '_key'
		ds_value_name = ds[1]
		cdb_paramvalue = 'p_' + ds_value_name
		pb_paramvalue = 'p_pb_' + ds_value_name
		bypass = 0
		key_field_name = ""

		if ('key' == field[1]):
			continue

		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
			field_name = cdb_util.format_field_name(field[3])
		else:
			field_name = cdb_util.format_field_name(field[1])

		if ('static' == field_name):
			field_name += '_'
		field_name_low = string.lower(field_name)

		pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
		if ('compose' == pb_type):
			compose_type = dt_type[1]
			compose_item = []
			compose_item.append(compose_type)
			compose_item.append(field_name_low)
			compose_array.append(compose_item)
	return compose_array

def build_tbl_message(fobj, parent_tbl, ds, key_array, field, dt_array):
	'build table message'
	ds_name = ds[0]
	ds_name_low = string.lower(ds[0])
	ds_name_low_key = ds_name_low + '_key'
	global glb_field_index
	required_str = "required"
	is_key = 0
	bypass = 0
	is_repeated = 0

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
		required_str = "required"
		if (1 == cdb_util.is_typedef_key(key_array)):
			key_pb_type = 'compose_' + key_array[0][0]
		else:
			key_pb_type = ds_name_low_key
	else:
		is_key = 0
		required_str = "required"
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if (2 == len(pb_type)):
		is_repeated = 1
		pb_type = pb_type[1]
		required_str = "repeated"
	else:
		if ('compose' == pb_type):
			compose_type = dt_type[1]
			pb_type = 'compose_' + compose_type
		elif ('none' == pb_type):
			bypass = 1
		elif ('todo' == pb_type):
			bypass = 1

	if (bypass and 0 == is_key):
		return

	if (is_key):
		print >> fobj, "    %s %s %s = %d;" % (required_str, key_pb_type, field_name, glb_field_index)
	else:
		print >> fobj, "    %s %s %s = %d;" % (required_str, pb_type, field_name, glb_field_index)
	glb_field_index += 1

def gen_proto_file_one(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_key = ds[0] + "_KEY"
	ds_name_low_key = string.lower(ds_name_key)
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	tbl_db_type = ds[2]
	parent_table = parent_tbl[0]
	parent_table_name = parent_table[0]
	global glb_field_index
	is_sub_ds = 0

	print >> fobj, "// %s %s" % (parent_table_name, ds_name)

	if (len(key_array)):
		# typedef key need not create a pb message
		if (1 == cdb_util.is_typedef_key(key_array)):
			pass
		else:
			glb_field_index = 1
			print >> fobj, "message %s" % (ds_name_low_key)
			print >> fobj, "{"
			build_tbl_key_message(fobj, ds, key_array, field_array[0], dt_array)
			print >> fobj, "}"
			print >> fobj, ""

	glb_field_index = 1
	print >> fobj, "message %s" % (ds_name_low)
	print >> fobj, "{"

	# parent tbl key
	parent_tbl_key_array = parent_tbl[0]
	if ('GLOBAL' != parent_tbl_key_array[2]):
		required_str = "required"
		key_pb_type = string.lower(parent_tbl[0][0]) + '_key'
		field_name = 'parent_' + parent_tbl[0][1]
		if (key_pb_type == 'tbl_route_node_key'):
			parent_tbl_key_array = parent_tbl[1]
			if (1 == cdb_util.is_typedef_key(parent_tbl_key_array)):
				key_pb_type = 'compose_' + parent_tbl_key_array[0][0]
		print >> fobj, "    %s %s %s = %d;" % (required_str, key_pb_type, field_name, glb_field_index)
		glb_field_index += 1

	if (4 == len(ds)):
		is_sub_ds = 1

	if (is_sub_ds):
		required_str = "required"
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		key_pb_type = string.lower(parent_ds[0][0]) + '_key'
		field_name = 'parent_' + parent_ds[0][1]
		print >> fobj, "    %s %s %s = %d;" % (required_str, key_pb_type, field_name, glb_field_index)
		glb_field_index += 1

	for i, field in enumerate(field_array):
		build_tbl_message(fobj, parent_tbl, ds, key_array, field, dt_array)
	print >> fobj, "}"
	print >> fobj, ""

def gen_pb_c_typedef_key_declare_one(fobj, ds, key_array):
	ds_name = ds[0]
	tbl_name_key = ds_name + '_KEY'
	ds_name_low = string.lower(ds_name)
	tbl_name_key_low = ds_name_low + 'Key'

	if (1 == cdb_util.is_typedef_key(key_array)):
		key_pb_type_lower = key_array[0][0]
		key_pb_type = string.upper(key_pb_type_lower)
		pb_ds_name = cdb_util.pb_get_ds_name(ds_name + '_key')
		compose_tbl_name = cdb_util.pb_get_compse_name(key_pb_type_lower)

		cdb_key_str = pb_ds_name
		cdb_key_init_str = 'CDB__' + tbl_name_key + '__INIT'
		compose_str = compose_tbl_name
		compose_init_str = 'CDB__COMPOSE_' + key_pb_type + '__INIT'
		print >> fobj, '#define %-30s %s' % (cdb_key_str, compose_str)
		print >> fobj, '#define %-30s %s' % (cdb_key_init_str, compose_init_str)

def	gen_pb_c_typedef_key_declare(fobj, ds_array):
	for i, ds_one in enumerate(ds_array):
		ds = ds_one[0]
		ds_name = ds[0]
		gen_pb_c_typedef_key_declare_one(fobj, ds_one[0], ds_one[1])
	print >> fobj, '\n'

def gen_pb_h_file_one(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_key = ds[0] + "_KEY"
	ds_name_low_key = string.lower(ds_name_key)
	is_array_ds = cdb_util.is_parent_tbl_array_ds(ds_name, parent_tbl)
	tbl_db_type = ds[2]
	parent_table = parent_tbl[0]
	parent_table_name = parent_table[0]
	parent_table_name_key = string.lower(parent_table_name) + "_key_t"
	func_name_pb = 'pb_' + ds_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	func_name_pb_pack = 'pb_' + ds_name_low + '_pack'
	func_name_pb_dump = 'pb_' + ds_name_low + '_dump'
	cdb_ds_name = ds_name_low + '_t'
	pb_ds_name = cdb_util.pb_get_ds_name(ds_name)
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	tbl_value_name = parent_table[1]
	parent_table_paramvalue = 'p_' + tbl_value_name + '_key'
	global glb_field_index
	is_sub_ds = 0
	parent_ds_string = ''

	parent_tbl_key_array = parent_tbl[0]

	if (4 == len(ds)):
		is_sub_ds = 1

	if (is_sub_ds):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		parent_ds_key = string.lower(parent_ds[0][0]) + '_key_t'
		field_name = 'p_' + parent_ds[0][1] + '_key'
		parent_ds_string = parent_ds_key + ' *'  + field_name + ', '

	if (is_sub_ds):
		print >> fobj, "/* pb functions for %s %s %s */" % (parent_table_name, parent_ds[0][0], ds_name)
	else:
		print >> fobj, "/* pb functions for %s %s */" % (parent_table_name, ds_name)
	print >> fobj, "int32"
	if ('GLOBAL' != parent_tbl_key_array[2]):
		print >> fobj, "%s(%s *%s, %s%s *%s, %s *%s);" % (func_name_pb, parent_table_name_key, parent_table_paramvalue, parent_ds_string, cdb_ds_name, cdb_paramvalue, pb_ds_name, pb_paramvalue)
	else:
		print >> fobj, "%s(%s%s *%s, %s *%s);" % (func_name_pb, parent_ds_string, cdb_ds_name, cdb_paramvalue, pb_ds_name, pb_paramvalue)

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s);" % (func_name_pb_free, pb_ds_name, pb_paramvalue)

	print >> fobj, "int32"
	print >> fobj, "%s(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);" % (func_name_pb_pack)

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s, char *out);" % (func_name_pb_dump, pb_ds_name, pb_paramvalue)
	print >> fobj, ""

def gen_pb_h_file(tbl_array, ds_array, dt_array):
	'generate pb_tbl.h file'
	fname = filename_out_pb_h
	fobj = open(fname, 'w')
	
	print >> fobj, ""
	print >> fobj, "#ifndef __PB_DS_H__"
	print >> fobj, "#define __PB_DS_H__"
	print >> fobj, ""
	print >> fobj, "typedef int32 (*PB_DS_PACK_FUNC)(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);"
	print >> fobj, "typedef void* (*PB_DS_UNPACK_FUNC)(ProtobufCAllocator *allocator, size_t len, const uint8_t *data);"
	print >> fobj, "typedef int32 (*PB_DS_DUMP_FUNC)(void *object, char *out);"
	print >> fobj, "typedef void  (*PB_DS_FREE_UNPACKED_FUNC)(void *object, ProtobufCAllocator *allocator);"
	print >> fobj, ""

	gen_pb_c_typedef_key_declare(fobj, ds_array)

	for i, ds_one in enumerate(ds_array):
		gen_pb_h_file_one(fobj, ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array)

	print >> fobj, "int32"
	print >> fobj, "pb_ds_pack(uint32 ds_id, void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len);"
	print >> fobj, "void*"
	print >> fobj, "pb_ds_unpack(uint32 ds_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data);"
	print >> fobj, "int32"
	print >> fobj, "pb_ds_dump(uint32 ds_id, void *object, char *out);"
	print >> fobj, "int32"
	print >> fobj, "pb_ds_free_unpacked(uint32 ds_id, void *object, ProtobufCAllocator *allocator);"
	print >> fobj, ""
	print >> fobj, "#endif  /* __PB_DS_H__ */"
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def gen_pb_c_pack_array(fobj, tbl_array):
	print >> fobj, "PB_DS_PACK_FUNC g_pb_pack_ds_func[DS_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		ds_name = table[0]
		ds_name_low = string.lower(ds_name)
		func_name_pb_pack = 'pb_' + ds_name_low + '_pack'
		print >> fobj, "    %s," % (func_name_pb_pack)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_unpack_array(fobj, tbl_array):
	print >> fobj, "PB_DS_UNPACK_FUNC g_pb_unpack_ds_func[DS_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		ds_name = table[0]
		ds_name_low = string.lower(ds_name)
		func_name_pb_unpack = 'cdb__' + ds_name_low + '__unpack'
		print >> fobj, "    (PB_DS_UNPACK_FUNC)%s," % (func_name_pb_unpack)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_dump_array(fobj, tbl_array):
	print >> fobj, "PB_DS_DUMP_FUNC g_pb_dump_ds_func[DS_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		ds_name = table[0]
		ds_name_low = string.lower(ds_name)
		func_name_pb_dump = 'pb_' + ds_name_low + '_dump'
		print >> fobj, "    (PB_DS_DUMP_FUNC)%s," % (func_name_pb_dump)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_free_unpacked_array(fobj, tbl_array):
	print >> fobj, "PB_DS_FREE_UNPACKED_FUNC g_pb_free_unpacked_ds_func[DS_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		ds_name = table[0]
		ds_name_low = string.lower(ds_name)
		func_name_pb_free_unpacked = 'cdb__' + ds_name_low + '__free_unpacked'
		print >> fobj, "    (PB_DS_FREE_UNPACKED_FUNC)%s," % (func_name_pb_free_unpacked)
	print >> fobj, "};"
	print >> fobj, ""

def gen_func_pb_c_pb_key(fobj, ds, key_array, field, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""

	key = key_array[0]
	dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
	key_field_name = cdb_util.format_field_name(key[1])
	field_name = key_field_name
	field_name_low = string.lower(field_name)
	
	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if ('string' == pb_type):
		print >> fobj, '    %s->key->%s = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(%s->key.%s)+1);' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		print >> fobj, '    sal_strcpy(%s->key->%s, %s->key.%s);' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
	elif ('uint32' == pb_type):
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):			
			data_value = field[1]
			field_value = field[3]
			flag_value = field[4]
			print >> fobj, '    %s->key->%s = GLB_FLAG_ISSET(%s->key.%s, %s) ? TRUE : FALSE;' % (pb_paramvalue, field_name_low, cdb_paramvalue, data_value, flag_value)
		else:
			print >> fobj, '    %s->key->%s = %s->key.%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
	elif ('uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
		print >> fobj, '    %s->key->%s = %s->key.%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
	elif ('bytes' == pb_type):
		pass
	elif ('compose' == pb_type):
		if (1 == cdb_util.is_typedef_key(key_array)):
			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb'
			print >> fobj, '    %s(&%s->key, %s->key);' % (compose_func, cdb_paramvalue, pb_paramvalue)
		else:
			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb'
			compose_refer = cdb_util.pb_get_cdb_compose_reference(compose_type)
			print >> fobj, '    %s(%s%s->key.%s, %s->key->%s);' % (compose_func, compose_refer, cdb_paramvalue, key_field_name, pb_paramvalue, key_field_name)

	elif ('none' == pb_type):
		pass
	else:
		pass
	print >> fobj, ''

def gen_func_pb_c_pb_field(fobj, ds, key_array, field, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""
	is_repeated = 0

	if ('key' == field[1]):
		return

	if (len(field) > 4):
		dt_type = field[6]
	else:
		dt_type = field[3]

	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])

	field_name_low = string.lower(field_name)
	if ('static' == field_name_low):
		field_name_low += '_'

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)

	if (2 == len(pb_type)):
		pb_type = pb_type[1]
		is_repeated = 1
		repeat_number = cdb_util.get_array_number(field[1])

		if ('string' == pb_type):
			print >> fobj, "    %s->%s = XCALLOC(MEM_LIB_PROTOBUF, sizeof(char*)*%s);" % (pb_paramvalue, field_name_low, repeat_number)
			print >> fobj, '    %s->n_%s = %s;' % (pb_paramvalue, field_name_low, repeat_number)
			print >> fobj, '    for (i = 0; i < %s; i++)' % (repeat_number)
			print >> fobj, '    {'
			print >> fobj, '        %s->%s[i] = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(%s->%s[i])+1);'  % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '        sal_strcpy(%s->%s[i], %s->%s[i]);'  % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '    }'
		elif ('uint32' == pb_type or 'uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
			if ('uint32' == pb_type):
				data_type = "uint32_t"
			elif ('int32' == pb_type):
				data_type = "int32_t"
			elif ('uint64' == pb_type):
				data_type = "uint64_t"
			elif ('int64' == pb_type):
				data_type = "int64_t"
			elif ('double' == pb_type):
				data_type = "double"
			print >> fobj, "    %s->%s = XCALLOC(MEM_LIB_PROTOBUF, sizeof(%s)*%s);" % (pb_paramvalue, field_name_low, data_type, repeat_number)
			print >> fobj, '    %s->n_%s = %s;' % (pb_paramvalue, field_name_low, repeat_number)
			print >> fobj, '    for (i = 0; i < %s; i++)' % (repeat_number)
			print >> fobj, '    {'
			print >> fobj, '        %s->%s[i] = %s->%s[i];' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '    }'
	else:
		if ('string' == pb_type):
			print >> fobj, '    %s->%s = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(%s->%s)+1);' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '    sal_strcpy(%s->%s, %s->%s);' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		elif ('uint32' == pb_type):
			if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):			
				data_value = field[1]
				field_value = field[3]
				flag_value = field[4]
				print >> fobj, '    %s->%s = GLB_FLAG_ISSET(%s->%s, %s) ? TRUE : FALSE;' % (pb_paramvalue, field_name_low, cdb_paramvalue, data_value, flag_value)
			else:
				print >> fobj, '    %s->%s = %s->%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		elif ('uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
			print >> fobj, '    %s->%s = %s->%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		elif ('bytes' == pb_type):
			print >> fobj, '    %s->%s.len = sizeof(%s->%s);' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '    %s->%s.data = XCALLOC(MEM_LIB_PROTOBUF, sizeof(%s->%s));' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
			print >> fobj, '    sal_memcpy(%s->%s.data, %s->%s, sizeof(%s->%s));' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name, cdb_paramvalue, field_name)
		elif ('compose' == pb_type):
			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb'
			compose_refer = cdb_util.pb_get_cdb_compose_reference(compose_type)
			print >> fobj, '    %s(%s%s->%s, %s->%s);' % (compose_func, compose_refer, cdb_paramvalue, field_name, pb_paramvalue, field_name_low)
		elif ('none' == pb_type):
			pass
		else:
			pass

def gen_func_pb_c_pb_check_array(fobj, ds, key_array, data_array, field_array, dt_array):
	for i, field in enumerate(field_array):
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

		pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
		if (2 == len(pb_type)):
			is_repeated = 1
			if ('string' == pb_type[1]):
				return 2
			else:
				return 1
	return 0

def gen_func_pb_c_ds_parent_tbl_key(fobj, table, key_array, field, dt_array, pb_paramvalue):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	tbl_value_name = table[1]
	tbl_value_key = 'p_' + tbl_value_name + '_key'
	bypass = 0
	key_field_name = ""

	if (0 == len(key_array)):
		return

	key = key_array[0]
	dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
	key_field_name = cdb_util.format_field_name(key[1])
	field_name = key_field_name
	field_name_low = string.lower(field_name)
	
	pb_value_str = "%s->parent_%s->%s" % (pb_paramvalue, tbl_value_name, field_name_low)
	cdb_value_str = "%s->%s" % (tbl_value_key, field_name_low)

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if ('string' == pb_type):
		print >> fobj, '    %s = XCALLOC(MEM_LIB_PROTOBUF, sal_strlen(%s)+1);' % (pb_value_str, cdb_value_str)
		print >> fobj, '    sal_strcpy(%s, %s);' % (pb_value_str, cdb_value_str)
	elif ('uint32' == pb_type):
		if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):			
			data_value = field[1]
			field_value = field[3]
			flag_value = field[4]
			print >> fobj, '    %s = GLB_FLAG_ISSET(%s, %s) ? TRUE : FALSE;' % (pb_value_str, cdb_value_str, flag_value)
		else:
			print >> fobj, '    %s = %s;' % (pb_value_str, cdb_value_str)
	elif ('uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
		print >> fobj, '    %s = %s;' % (pb_value_str, cdb_value_str)
	elif ('bytes' == pb_type):
		pass
	elif ('compose' == pb_type):
		pb_value_str = "%s->parent_%s" % (pb_paramvalue, tbl_value_name)
		cdb_value_str = "%s" % (tbl_value_key)
		if (1 == cdb_util.is_typedef_key(key_array)):
			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb'
			print >> fobj, '    %s(%s, %s);' % (compose_func, cdb_value_str, pb_value_str)
		else:
			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb'
			compose_refer = cdb_util.pb_get_cdb_compose_reference(compose_type)
			print >> fobj, '    %s(%s%s->%s, %s->%s);' % (compose_func, compose_refer, cdb_value_str, key_field_name, pb_value_str, key_field_name)

	elif ('none' == pb_type):
		pass
	else:
		pass
	print >> fobj, ''

def gen_func_pb_c_pb_parent_ds_key(fobj, ds, parent_ds, dt_array, parent_ds_field_name):
	ds_name = ds[0]
	ds_paramvalue = 'p_pb_' + string.lower(ds[1])
	parent_ds_key = parent_ds[1][0]
	parent_ds_key_value = string.lower(parent_ds_key[1])
	parent_ds_name = parent_ds[0][0]
	parent_ds_value = parent_ds[0][1]
	dt_type = cdb_util.check_data_dt_type(ds, parent_ds_key, dt_array)
	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	pb_parent_paramvalue = 'parent_' + parent_ds_value
	if ('uint32' == pb_type):
		print >> fobj, '    %s->%s->%s = %s->%s;' % (ds_paramvalue, pb_parent_paramvalue, parent_ds_key_value, parent_ds_field_name, parent_ds_key_value)
	else:
		cdb_util.error_return("Only ds_nexthop has parent ds_rib key is uint32 %s", ds_name)


def gen_func_pb_c_pb(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	parent_table = parent_tbl[0]
	parent_table_name = parent_table[0]
	parent_table_name_key = string.lower(parent_table[0]) + '_key_t'
	parent_table_paramvalue_key = 'p_' + parent_table[1] + '_key'
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	func_name_pb = 'pb_' + ds_name_low + '_to_pb'
	cdb_ds_name = ds_name_low + '_t'
	pb_ds_name = cdb_util.pb_get_ds_name(ds_name)
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	ds_has_key = 0
	has_repeated = 0
	is_sub_ds = 0
	parent_ds_string = ''
	if (4 == len(ds)):
		is_sub_ds = 1

	if (is_sub_ds):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		parent_ds_key = string.lower(parent_ds[0][0]) + '_key_t'
		parent_ds_field_name = 'p_' + parent_ds[0][1] + '_key'
		parent_ds_string = parent_ds_key + ' *'  + parent_ds_field_name + ', '

	if (len(key_array)):
		ds_has_key = 1
	
	has_repeated = gen_func_pb_c_pb_check_array(fobj, ds, key_array, data_array, field_array, dt_array)

	parent_tbl_key_array = parent_tbl[0]

	print >> fobj, "int32"
	if ('GLOBAL' != parent_tbl_key_array[2]):
		print >> fobj, "%s(%s *%s, %s%s *%s, %s *%s)" % (func_name_pb, parent_table_name_key, parent_table_paramvalue_key, parent_ds_string, cdb_ds_name, cdb_paramvalue, pb_ds_name, pb_paramvalue)
	else:
		print >> fobj, "%s(%s%s *%s, %s *%s)" % (func_name_pb, parent_ds_string, cdb_ds_name, cdb_paramvalue, pb_ds_name, pb_paramvalue)
	print >> fobj, "{"

	if (has_repeated):
		print >> fobj, "    uint32 i = 0;"
		print >> fobj, ""

	# parent tbl key
	parent_field_array = parent_tbl[5]
	gen_func_pb_c_ds_parent_tbl_key(fobj, parent_tbl[0], parent_tbl[1], parent_field_array[0], dt_array, pb_paramvalue)

	if (is_sub_ds):
		gen_func_pb_c_pb_parent_ds_key(fobj, ds, parent_ds, dt_array, parent_ds_field_name)

	if (ds_has_key):
		gen_func_pb_c_pb_key(fobj, ds, key_array, field_array[0], dt_array)

	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_field(fobj, ds, key_array, field, dt_array)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_func_pb_c_pb_free_field(fobj, ds, key_array, field, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
		key_field_name = cdb_util.format_field_name(key[1])
	else:
		is_key = 0
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])

	if (is_key):
		field_name = field_name + '->' + key_field_name
	field_name_low = string.lower(field_name)

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if (2 == len(pb_type)):
		pb_type = pb_type[1]
		if ('string' == pb_type):
			print >> fobj, "    if (%s->%s)" % (pb_paramvalue, field_name_low)
			print >> fobj, "    {"
			print >> fobj, "        for (i = 0; i < %s->n_%s; i++)" % (pb_paramvalue, field_name_low)
			print >> fobj, "        {"
			print >> fobj, "            if (%s->%s[i])" % (pb_paramvalue, field_name_low)
			print >> fobj, "            {"
			print >> fobj, "                XFREE(MEM_LIB_PROTOBUF, %s->%s[i]);" % (pb_paramvalue, field_name_low)
			print >> fobj, "                %s->%s[i] = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "            }"
			print >> fobj, "        }"
			print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s);" % (pb_paramvalue, field_name_low)
			print >> fobj, "        %s->%s = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "    }"
			print >> fobj, ""
		elif ('uint32' == pb_type or 'uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
			print >> fobj, "    if (%s->%s)" % (pb_paramvalue, field_name_low)
			print >> fobj, "    {"
			print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s);" % (pb_paramvalue, field_name_low)
			print >> fobj, "        %s->%s = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "    }"
			print >> fobj, ""
	else:
		if ('string' == pb_type):
			print >> fobj, "    if (%s->%s)" % (pb_paramvalue, field_name)
			print >> fobj, "    {"
			print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s);" % (pb_paramvalue, field_name_low)
			print >> fobj, "        %s->%s = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "    }"
			print >> fobj, ""
		elif ('compose' == pb_type):
			if (is_key):
				if (1 == cdb_util.is_typedef_key(key_array)):
					field_name = 'key'
				else:
					field_name = 'key->' + key_field_name
				field_name_low = string.lower(field_name)

			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_to_pb_free_packed'

			print >> fobj, '    %s(%s->%s);' % (compose_func, pb_paramvalue, field_name_low)
		elif ('bytes' == pb_type):
			print >> fobj, "    if (%s->%s.data)" % (pb_paramvalue, field_name_low)
			print >> fobj, "    {"
			print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s.data);" % (pb_paramvalue, field_name_low)
			print >> fobj, "        %s->%s.data = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "    }"
			print >> fobj, ""

def gen_func_pb_c_pb_parent_tbl_free(fobj, ds, parent_tbl_key, parent_tbl, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""
	parent_table = parent_tbl[0]
	parent_tbl_paramvalue = 'parent_' + parent_table[1]

	dt_type = cdb_util.check_data_dt_type(ds, parent_tbl_key, dt_array)
	key_field_name = cdb_util.format_field_name(parent_tbl_key[1])

	field_name = parent_tbl_paramvalue + '->' + key_field_name
	field_name_low = string.lower(field_name)

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if ('string' == pb_type):
		print >> fobj, "    if (%s->%s)" % (pb_paramvalue, field_name_low)
		print >> fobj, "    {"
		print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s);" % (pb_paramvalue, field_name_low)
		print >> fobj, "        %s->%s = NULL;" % (pb_paramvalue, field_name_low)
		print >> fobj, "    }"
		print >> fobj, ""
	elif ('compose' == pb_type):
		compose_type = dt_type[1]
		compose_func = 'pb_compose_' + compose_type + '_to_pb_free_packed'
		field_name = parent_tbl_paramvalue
		field_name_low = string.lower(field_name)
		print >> fobj, '    %s(%s->%s->%s);' % (compose_func, pb_paramvalue, field_name_low, key_field_name)
	elif ('bytes' == pb_type):
		print >> fobj, "    if (%s->%s.data)" % (pb_paramvalue, field_name)
		print >> fobj, "    {"
		print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s.data);" % (pb_paramvalue, field_name_low)
		print >> fobj, "        %s->%s.data = NULL;" % (pb_paramvalue, field_name_low)
		print >> fobj, "    }"
		print >> fobj, ""

def gen_func_pb_c_pb_free(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	func_name_pb = 'pb_' + ds_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	cdb_ds_name = ds_name_low + '_t'
	pb_ds_name = cdb_util.pb_get_ds_name(ds_name)
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name

	parent_table_key_array = parent_tbl[1]
	parent_tbl_has_key = 0
	if (len(parent_table_key_array)):
		parent_tbl_key = parent_table_key_array[0]
		parent_tbl_has_key = 1

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s)" % (func_name_pb_free, pb_ds_name, pb_paramvalue)
	print >> fobj, "{"
	has_repeated = gen_func_pb_c_pb_check_array(fobj, ds, key_array, data_array, field_array, dt_array)
	if (2 == has_repeated):
		print >> fobj, "    uint32 i = 0;"
		print >> fobj, ""
	if (parent_tbl_has_key):
		gen_func_pb_c_pb_parent_tbl_free(fobj, ds, parent_tbl_key, parent_tbl, dt_array)
	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_free_field(fobj, ds, key_array, field, dt_array)
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def pb_declare_compose(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	compose_array = get_compose_field_array(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	for i, compose_item in enumerate(compose_array):
		compose_type = compose_item[0]
		field_name_low = compose_item[1]
		compose_tbl_name = cdb_util.pb_get_compse_name(compose_type)
		compose_str = compose_tbl_name
		compose_init_str = 'CDB__COMPOSE_' + string.upper(compose_type) + '__INIT'
		print >> fobj, '    %s %s = %s;' % (compose_str, field_name_low, compose_init_str)


def pb_set_compose(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array, pb_paramvalue):
	compose_array = get_compose_field_array(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	for i, compose_item in enumerate(compose_array):
		compose_type = compose_item[0]
		field_name_low = compose_item[1]
		print >> fobj, "    %s.%s = &%s;" % (pb_paramvalue, field_name_low, field_name_low)


def gen_func_pb_c_pb_pack(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	func_name_pb = 'pb_' + ds_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	func_name_pb_pack = 'pb_' + ds_name_low + '_pack'
	cdb_func_name_pb_pack = 'cdb__' + ds_name_low + '__pack'
	cdb_ds_name = ds_name_low + '_t'
	pb_ds_name = cdb_util.pb_get_ds_name(ds_name)
	pb_tbl_name_key = pb_ds_name + 'Key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'pb_' + ds_value_name
	pb_paramvalue_key = pb_paramvalue + '_key'
	ds_key_init = 'CDB__' + ds_name + '_KEY' + '__INIT'
	ds_init = 'CDB__' + ds_name + '__INIT'
	parent_table = parent_tbl[0]
	parent_table_name = parent_table[0]
	parent_table_value = parent_table[1]
	pb_parent_table_name_key = cdb_util.pb_get_tbl_name(parent_table_name) + 'Key'
	parent_table_value_key = 'pb_' + parent_table[1] + '_key'
	parent_table_key_array = parent_tbl[1]
	parent_tbl_key_init = 'CDB__' + parent_table_name + '_KEY' + '__INIT'
	ds_has_key = 0
	parent_tbl_has_key = 0
	is_sub_ds = 0
	parent_ds_string = ''

	if (4 == len(ds)):
		is_sub_ds = 1

	if (is_sub_ds):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		parent_ds_name = parent_ds[0][0]
		parent_ds_key = string.lower(parent_ds[0][0]) + '_key_t'
		parent_ds_field_name = 'p_' + parent_ds[0][1] + '_key'
		parent_ds_string = parent_ds_key + ' *'  + parent_ds_field_name + ', '

	if (len(key_array)):
		ds_has_key = 1

	print >> fobj, "int32"
	print >> fobj, "%s(void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)" % (func_name_pb_pack)
	print >> fobj, "{"

	if (len(parent_table_key_array)):
		parent_tbl_has_key = 1
		print >> fobj, "    %s %s = %s;" % (pb_parent_table_name_key, parent_table_value_key, parent_tbl_key_init)
		
	if (is_sub_ds):
		pb_parent_ds_name_key = cdb_util.pb_get_ds_name(parent_ds_name) + 'Key'
		pb_parent_ds_name_key_value = 'pb_' + parent_ds[0][1] + '_key'
		parent_ds_key_init	= 'CDB__' + parent_ds_name + '_KEY' + '__INIT'
		print >> fobj, "    %s %s = %s;" % (pb_parent_ds_name_key, pb_parent_ds_name_key_value, parent_ds_key_init)

	if (ds_has_key):
		key_compose = cdb_util.pb_get_key_compose_not_typedef(ds, key_array[0], dt_array)
		if (key_compose):
			key_compose_name = cdb_util.pb_get_compse_name(key_compose[0])
			key_compose_param = key_compose[1]
			key_compose_init = 'CDB__COMPOSE_' + string.upper(key_compose[0]) + '__INIT'
			print >> fobj, "    %s %s = %s;" % (key_compose_name, key_compose_param, key_compose_init)
		print >> fobj, "    %s %s = %s;" % (pb_tbl_name_key, pb_paramvalue_key, ds_key_init)
	print >> fobj, "    %s %s = %s;" % (pb_ds_name, pb_paramvalue, ds_init)
	if (parent_tbl_has_key):
		parent_tbl_key_type = string.lower(parent_table_name) + '_key_t'
		parent_tbl_key_value = 'p_' + parent_table_value + '_key'
		print >> fobj, "    %s *%s = (%s*)p_tbl;" % (parent_tbl_key_type, parent_tbl_key_value, parent_tbl_key_type)

	if (is_sub_ds):
		print >> fobj, "    %s *%s = (%s*)p_ds;" % (parent_ds_key, parent_ds_field_name, parent_ds_key)
		print >> fobj, "    %s *%s = (%s*)p_ds2;" % (cdb_ds_name, cdb_paramvalue, cdb_ds_name)
	else:
		print >> fobj, "    %s *%s = (%s*)p_ds;" % (cdb_ds_name, cdb_paramvalue, cdb_ds_name)
	print >> fobj, "    int32 len = 0;"
	pb_declare_compose(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	print >> fobj, ""
	if (ds_has_key):
		if (key_compose):
			print >> fobj, "    %s.%s = &%s;" % (pb_paramvalue_key, key_compose_param, key_compose_param)
		print >> fobj, "    %s.key = &%s;" % (pb_paramvalue, pb_paramvalue_key)
	pb_set_compose(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array, pb_paramvalue)

	if (parent_tbl_has_key):
		print >> fobj, "    %s.parent_%s = &%s;" % (pb_paramvalue, parent_table_value, parent_table_value_key)

	if (is_sub_ds):
		print >> fobj, "    %s.parent_%s = &%s;" % (pb_paramvalue, parent_ds[0][1], pb_parent_ds_name_key_value)

	if (parent_tbl_has_key):
		if (is_sub_ds):
			print >> fobj, "    %s(%s, %s, %s, &%s);" % (func_name_pb, parent_tbl_key_value, parent_ds_field_name, cdb_paramvalue, pb_paramvalue)
		else:
			print >> fobj, "    %s(%s, %s, &%s);" % (func_name_pb, parent_tbl_key_value, cdb_paramvalue, pb_paramvalue)
	else:
		print >> fobj, "    %s(%s, &%s);" % (func_name_pb, cdb_paramvalue, pb_paramvalue)
	print >> fobj, "    len = %s(&%s, buf);" % (cdb_func_name_pb_pack, pb_paramvalue)
	print >> fobj, "    %s(&%s);" % (func_name_pb_free, pb_paramvalue)
	print >> fobj, ""
	print >> fobj, "    return len;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_func_pb_c_pb_dump_parent_ds_key(fobj, ds, parent_ds, dt_array):
	ds_name = ds[0]
	ds_paramvalue = 'p_pb_' + string.lower(ds[1])
	parent_ds_key = parent_ds[1][0]
	parent_ds_key_value = string.lower(parent_ds_key[1])
	parent_ds_name = parent_ds[0][0]
	parent_ds_value = parent_ds[0][1]
	dt_type = cdb_util.check_data_dt_type(ds, parent_ds_key, dt_array)
	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	pb_parent_paramvalue = 'parent_' + parent_ds_value
	if ('uint32' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s->%s);' % (pb_parent_paramvalue, "%u", ds_paramvalue, pb_parent_paramvalue, parent_ds_key_value)
	else:
		cdb_util.error_return("Only ds_nexthop has parent ds_rib key is uint32 %s", ds_name)

def gen_func_pb_c_pb_dump_parent_tbl_key(fobj, ds, parent_tbl_key, parent_tbl, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""
	parent_table = parent_tbl[0]
	parent_tbl_paramvalue = 'parent_' + parent_table[1]

	dt_type = cdb_util.check_data_dt_type(ds, parent_tbl_key, dt_array)
	key_field_name = cdb_util.format_field_name(parent_tbl_key[1])

	field_name = parent_tbl_paramvalue + '->' + key_field_name
	field_name_low = string.lower(field_name)

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if ('string' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%s", pb_paramvalue, field_name_low)
	elif ('uint32' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%u", pb_paramvalue, field_name_low)
	elif ('uint64' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%%"%s, %s->%s);' % (field_name, "PRIu64", pb_paramvalue, field_name_low)
	elif ('int32' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%d", pb_paramvalue, field_name_low)
	elif ('int64' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%%"%s, %s->%s);' % (field_name, "PRI64", pb_paramvalue, field_name_low)
	elif ('double' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%f", pb_paramvalue, field_name_low)
	elif ('bytes' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=bytes");' % (field_name)
	elif ('compose' == pb_type):
		field_name = parent_tbl_paramvalue
		field_name_low = string.lower(field_name)
		compose_type = dt_type[1]
		compose_func = 'pb_compose_' + compose_type + '_dump'
		print >> fobj, '    offset += %s(%s->%s->%s, (out + offset));' % (compose_func, pb_paramvalue, field_name_low, key_field_name)
	elif ('none' == pb_type):
		pass
#		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=none");' % (field_name)
	elif ('todo' == pb_type):
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=todo");' % (field_name)
	else:
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=unknown");' % (field_name)

def gen_func_pb_c_pb_dump_field(fobj, ds, key_array, field, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	ds_name_low_key = ds_name_low + '_key'
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	bypass = 0
	key_field_name = ""

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(ds, key, dt_array)
		key_field_name = cdb_util.format_field_name(key[1])
	else:
		is_key = 0
		if (len(field) > 4):
			dt_type = field[6]
		else:
			dt_type = field[3]

	if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):
		field_name = cdb_util.format_field_name(field[3])
	else:
		field_name = cdb_util.format_field_name(field[1])

	if (is_key):
		field_name = field_name + '->' + key_field_name

	if ('static' == field_name):
		field_name += '_'
	field_name_low = string.lower(field_name)

	pb_type = cdb_util.pb_get_pb_type(dt_type, dt_array)
	if (2 == len(pb_type)):
		pb_type = pb_type[1]
		is_repeated = 1
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=");' % (field_name)
		if ('string' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, %s->n_%s, (out + offset));' % ('pb_string_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('int8' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, sizeof(%s->%s), (out + offset));' % ('pb_int8_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('uint8' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, sizeof(%s->%s), (out + offset));' % ('pb_uint8_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('int32' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, sizeof(%s->%s), (out + offset));' % ('pb_int32_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('uint32' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, sizeof(%s->%s), (out + offset));' % ('pb_uint32_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('double' == pb_type):
			print >> fobj, '    offset += %s(%s->%s, sizeof(%s->%s), (out + offset));' % ('pb_double_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
	else:
		if ('string' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%s", pb_paramvalue, field_name_low)
		elif ('uint32' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%u", pb_paramvalue, field_name_low)
		elif ('uint64' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%%"%s, %s->%s);' % (field_name, "PRIu64", pb_paramvalue, field_name_low)
		elif ('int32' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%d", pb_paramvalue, field_name_low)
		elif ('int64' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%%"%s, %s->%s);' % (field_name, "PRId64", pb_paramvalue, field_name_low)
		elif ('double' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=%s", %s->%s);' % (field_name, "%f", pb_paramvalue, field_name_low)
		elif ('bytes' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=");' % (field_name)
			if ('INT8ARRAY' == dt_type[1]):
				print >> fobj, '    offset += %s(%s->%s.data, %s->%s.len, (out + offset));' % ('pb_int8_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
			elif ('UINT8ARRAY' == dt_type[1]):
				print >> fobj, '    offset += %s(%s->%s.data, %s->%s.len, (out + offset));' % ('pb_uint8_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
			else:
				print >> fobj, '    offset += %s(%s->%s.data, %s->%s.len, (out + offset));' % ('pb_bitmap_array_dump', pb_paramvalue, field_name_low, pb_paramvalue, field_name_low)
		elif ('compose' == pb_type):
			if (is_key):
				if (1 == cdb_util.is_typedef_key(key_array)):
					field_name_low = 'key'
				else:
					field_name_low = 'key->' + key_field_name

			compose_type = dt_type[1]
			compose_func = 'pb_compose_' + compose_type + '_dump'
			print >> fobj, '    offset += %s(%s->%s, (out + offset));' % (compose_func, pb_paramvalue, field_name_low)
		elif ('none' == pb_type):
			pass
		elif ('todo' == pb_type):
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=todo");' % (field_name)
		else:
			print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=unknown");' % (field_name)

def gen_func_pb_c_pb_dump(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	ds_name = ds[0]
	ds_name_low = string.lower(ds_name)
	func_name_pb = 'pb_' + ds_name_low + '_to_pb'
	func_name_pb_dump = 'pb_' + ds_name_low + '_dump'
	cdb_ds_name = ds_name_low + '_t'
	pb_ds_name = cdb_util.pb_get_ds_name(ds_name)
	ds_value_name = ds[1]
	cdb_paramvalue = 'p_' + ds_value_name
	pb_paramvalue = 'p_pb_' + ds_value_name
	is_sub_ds = 0
	parent_ds_string = ''

	if (4 == len(ds)):
		is_sub_ds = 1

	print >> fobj, 'int32'
	print >> fobj, '%s(%s *%s, char *out)' % (func_name_pb_dump, pb_ds_name, pb_paramvalue)
	print >> fobj, '{'
	print >> fobj, '    int32 offset = 0;'
	print >> fobj, ''
	print >> fobj, '    out[0] = CMD_ZERO_CHAR;'

	parent_table_key_array = parent_tbl[1]
	parent_tbl_has_key = 0
	if (len(parent_table_key_array)):
		parent_tbl_key = parent_table_key_array[0]
		parent_tbl_has_key = 1

	if (parent_tbl_has_key):
		gen_func_pb_c_pb_dump_parent_tbl_key(fobj, ds, parent_tbl_key, parent_tbl, dt_array)

	if (is_sub_ds):
		parent_ds = cdb_util.get_parent_ds(ds, ds_array)
		gen_func_pb_c_pb_dump_parent_ds_key(fobj, ds, parent_ds, dt_array)

	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_dump_field(fobj, ds, key_array, field, dt_array)
	print >> fobj, '    offset += sal_sprintf(out + offset, "\\n");'
	print >> fobj, ''
	print >> fobj, '    return PM_E_NONE;'
	print >> fobj, '}'
	print >> fobj, ''

def gen_proto_file(tbl_array, ds_array, dt_array):
	'generate table proto file'
	fname = filename_out_proto
	fobj = open(fname, 'w')
	
	print >> fobj, "package cdb;"
	print >> fobj, ""
	print >> fobj, 'import "common.proto";'
	print >> fobj, 'import "tbl.proto";'
	print >> fobj, ''

	for i, ds_one in enumerate(ds_array):
		gen_proto_file_one(fobj, ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array)
	print "generate file %s" % (fname)

def gen_pb_c_file_one(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array):
	parent_table = parent_tbl[0]
	parent_table_name = parent_table[0]
	ds_name = ds[0]
	print >> fobj, "/* pb functions for %s %s */" % (parent_table_name, ds_name)
	gen_func_pb_c_pb(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	gen_func_pb_c_pb_free(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	gen_func_pb_c_pb_pack(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)
	gen_func_pb_c_pb_dump(fobj, ds, key_array, data_array, parent_tbl, field_array, ds_array, dt_array)

def gen_tbl_include(fobj, tbl_array, ds_array, dt_array):
	'generate store tbl c include'
	for i, tbl_one in enumerate(tbl_array):
		ds_name = tbl_one[0][0]
		fname_define_h = 'gen/' + string.lower(ds_name) + '_define.h'
		fname_h = 'gen/' + string.lower(ds_name) + '.h'
		print >> fobj, "#include \"%s\"" % (fname_define_h)
		print >> fobj, "#include \"%s\"" % (fname_h)


def gen_pb_c_file(tbl_array, ds_array, dt_array):
	'generate pb_tbl.c file'
	fname = filename_out_pb_c
	fobj = open(fname, 'w')

	print >> fobj, ''
	print >> fobj, '#include "proto.h"'
	print >> fobj, '#include "pb_cdb_inc.h"'
	print >> fobj, '#include "tbl.pb-c.h"'
	print >> fobj, '#include "ds.pb-c.h"'
	print >> fobj, '#include "pb_tbl.h"'
	print >> fobj, '#include "pb_ds.h"'
	print >> fobj, '#include "pb_common.h"'
	print >> fobj, ''

	for i, ds_one in enumerate(ds_array):
		gen_pb_c_file_one(fobj, ds_one[0], ds_one[1], ds_one[2], ds_one[3], ds_one[4], ds_array, dt_array)

	gen_pb_c_pack_array(fobj, ds_array)
	gen_pb_c_unpack_array(fobj, ds_array)
	gen_pb_c_dump_array(fobj, ds_array)
	gen_pb_c_free_unpacked_array(fobj, ds_array)

	print >> fobj, "int32"
	print >> fobj, "pb_ds_pack(uint32 ds_id, void *p_tbl, void *p_ds, void *p_ds2, uint8 *buf, uint32 buf_len)"
	print >> fobj, "{"
	print >> fobj, "    if (ds_id >= DS_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_pack_ds_func[ds_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_pack_ds_func[ds_id](p_tbl, p_ds, p_ds2, buf, buf_len);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "void*"
	print >> fobj, "pb_ds_unpack(uint32 ds_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data)"
	print >> fobj, "{"
	print >> fobj, "    if (ds_id >= DS_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_unpack_ds_func[ds_id])"
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_unpack_ds_func[ds_id](allocator, len, data);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "pb_ds_dump(uint32 ds_id, void *object, char *out)"
	print >> fobj, "{"
	print >> fobj, "    if (ds_id >= DS_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_dump_ds_func[ds_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_dump_ds_func[ds_id](object, out);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "pb_ds_free_unpacked(uint32 ds_id, void *object, ProtobufCAllocator *allocator)"
	print >> fobj, "{"
	print >> fobj, "    if (ds_id >= DS_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_free_unpacked_ds_func[ds_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    g_pb_free_unpacked_ds_func[ds_id](object, allocator);"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)