import os
import sys
import string
import CONST
import cdb_util

filename_out_proto = CONST.PBC_PROTO_TBL
filename_out_pb_h  = CONST.PBC_H_PATH + 'pb_tbl.h'
filename_out_pb_c  = CONST.PBC_C_PATH + 'pb_tbl.c'

glb_field_index = 0

def build_tbl_key_message(fobj, table, key_array, field, dt_array):
	'build table key message'
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	key = key_array[0]
	global glb_field_index

	dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
	if (1 != len(key_array)):
		print "%s key num %d" % (tbl_name, len(key_array))

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


def get_compose_field_array(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	compose_array = []
	for i, field in enumerate(field_array):
		tbl_name = table[0]
		tbl_name_low = string.lower(table[0])
		tbl_name_low_key = tbl_name_low + '_key'
		tbl_value_name = table[1]
		cdb_paramvalue = 'p_' + tbl_value_name
		pb_paramvalue = 'p_pb_' + tbl_value_name
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

def build_tbl_message(fobj, table, key_array, field, dt_array):
	'build table message'
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	global glb_field_index
	required_str = "required"
	is_key = 0
	bypass = 0
	is_repeated = 0

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
		required_str = "required"
		if (1 == cdb_util.is_typedef_key(key_array)):
			key_pb_type = 'compose_' + key_array[0][0]
		else:
			key_pb_type = tbl_name_low_key
	else:
		is_key = 0
		required_str = "optional"
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

def gen_proto_file_one(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	global glb_field_index

	print >> fobj, "// %s" % (tbl_name)

	if (len(key_array)):
		# typedef key need not create a pb message
		if (1 == cdb_util.is_typedef_key(key_array)):
			pass
		else:
			glb_field_index = 1
			print >> fobj, "message %s" % (tbl_name_low_key)
			print >> fobj, "{"
			build_tbl_key_message(fobj, table, key_array, field_array[0], dt_array)
			print >> fobj, "}"
			print >> fobj, ""

	glb_field_index = 1
	print >> fobj, "message %s" % (tbl_name_low)
	print >> fobj, "{"
	for i, field in enumerate(field_array):
		build_tbl_message(fobj, table, key_array, field, dt_array)
	print >> fobj, "}"
	print >> fobj, ""
#	print "generate .proto of TBL %s" % (tbl_name)

def gen_pb_c_typedef_key_declare_one(fobj, table, key_array):
	tbl_name = table[0]
	tbl_name_key = tbl_name + '_KEY'
	tbl_name_low = string.lower(tbl_name)
	tbl_name_key_low = tbl_name_low + 'Key'

	if (1 == cdb_util.is_typedef_key(key_array)):
		key_pb_type_lower = key_array[0][0]
		key_pb_type = string.upper(key_pb_type_lower)
		pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name + '_key')
		compose_tbl_name = cdb_util.pb_get_compse_name(key_pb_type_lower)

		cdb_key_str = pb_tbl_name
		cdb_key_init_str = 'CDB__' + tbl_name_key + '__INIT'
		compose_str = compose_tbl_name
		compose_init_str = 'CDB__COMPOSE_' + key_pb_type + '__INIT'
		print >> fobj, '#define %-30s %s' % (cdb_key_str, compose_str)
		print >> fobj, '#define %-30s %s' % (cdb_key_init_str, compose_init_str)

def	gen_pb_c_typedef_key_declare(fobj, tbl_array):
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		tbl_name = table[0]
		gen_pb_c_typedef_key_declare_one(fobj, tbl_one[0], tbl_one[1])
	print >> fobj, '\n'

def gen_pb_h_file_one(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	func_name_pb = 'pb_' + tbl_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	func_name_pb_pack = 'pb_' + tbl_name_low + '_pack'
	func_name_pb_dump = 'pb_' + tbl_name_low + '_dump'
	cdb_tbl_name = tbl_name_low + '_t'
	pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name)
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name

	print >> fobj, "/* pb functions for %s */" % (tbl_name)
	print >> fobj, "int32"
	print >> fobj, "%s(uint32 only_key, %s *%s, %s *%s);" % (func_name_pb, cdb_tbl_name, cdb_paramvalue, pb_tbl_name, pb_paramvalue)

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s);" % (func_name_pb_free, pb_tbl_name, pb_paramvalue)

	print >> fobj, "int32"
	print >> fobj, "%s(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);" % (func_name_pb_pack)

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s, uint32 only_key, char *out);" % (func_name_pb_dump, pb_tbl_name, pb_paramvalue)
	print >> fobj, ""

def gen_pb_h_file(tbl_array, ds_array, dt_array):
	'generate pb_tbl.h file'
	fname = filename_out_pb_h
	fobj = open(fname, 'w')
	
	print >> fobj, ""
	print >> fobj, "#ifndef __PB_TBL_H__"
	print >> fobj, "#define __PB_TBL_H__"
	print >> fobj, ""
	print >> fobj, "typedef int32 (*PB_TBL_PACK_FUNC)(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);"
	print >> fobj, "typedef void* (*PB_TBL_UNPACK_FUNC)(ProtobufCAllocator *allocator, size_t len, const uint8_t *data);"
	print >> fobj, "typedef int32 (*PB_TBL_DUMP_FUNC)(void *object, uint32 only_key, char *out);"
	print >> fobj, "typedef void  (*PB_TBL_FREE_UNPACKED_FUNC)(void *object, ProtobufCAllocator *allocator);"
	print >> fobj, ""

	gen_pb_c_typedef_key_declare(fobj, tbl_array)

	for i, tbl_one in enumerate(tbl_array):
		gen_pb_h_file_one(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)

	print >> fobj, "int32"
	print >> fobj, "pb_tbl_pack(uint32 tbl_id, void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len);"
	print >> fobj, "void*"
	print >> fobj, "pb_tbl_unpack(uint32 tbl_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data);"
	print >> fobj, "int32"
	print >> fobj, "pb_tbl_dump(uint32 tbl_id, void *object, uint32 only_key, char *out);"
	print >> fobj, "int32"
	print >> fobj, "pb_tbl_free_unpacked(uint32 tbl_id, void *object, ProtobufCAllocator *allocator);"
	print >> fobj, ""
	print >> fobj, "#endif  /* __PB_TBL_H__ */"
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)

def gen_pb_c_pack_array(fobj, tbl_array):
	print >> fobj, "PB_TBL_PACK_FUNC g_pb_pack_tbl_func[TBL_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		tbl_name = table[0]
		tbl_name_low = string.lower(tbl_name)
		func_name_pb_pack = 'pb_' + tbl_name_low + '_pack'
		print >> fobj, "    %s," % (func_name_pb_pack)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_unpack_array(fobj, tbl_array):
	print >> fobj, "PB_TBL_UNPACK_FUNC g_pb_unpack_tbl_func[TBL_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		tbl_name = table[0]
		tbl_name_low = string.lower(tbl_name)
		func_name_pb_unpack = 'cdb__' + tbl_name_low + '__unpack'
		print >> fobj, "    (PB_TBL_UNPACK_FUNC)%s," % (func_name_pb_unpack)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_dump_array(fobj, tbl_array):
	print >> fobj, "PB_TBL_DUMP_FUNC g_pb_dump_tbl_func[TBL_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		tbl_name = table[0]
		tbl_name_low = string.lower(tbl_name)
		func_name_pb_dump = 'pb_' + tbl_name_low + '_dump'
		print >> fobj, "    (PB_TBL_DUMP_FUNC)%s," % (func_name_pb_dump)
	print >> fobj, "};"
	print >> fobj, ""

def gen_pb_c_free_unpacked_array(fobj, tbl_array):
	print >> fobj, "PB_TBL_FREE_UNPACKED_FUNC g_pb_free_unpacked_tbl_func[TBL_MAX] ="
	print >> fobj, "{"
	for i, tbl_one in enumerate(tbl_array):
		table = tbl_one[0]
		tbl_name = table[0]
		tbl_name_low = string.lower(tbl_name)
		func_name_pb_free_unpacked = 'cdb__' + tbl_name_low + '__free_unpacked'
		print >> fobj, "    (PB_TBL_FREE_UNPACKED_FUNC)%s," % (func_name_pb_free_unpacked)
	print >> fobj, "};"
	print >> fobj, ""

def gen_func_pb_c_pb_key(fobj, table, key_array, field, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name
	bypass = 0
	key_field_name = ""

	key = key_array[0]
	dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
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

def gen_func_pb_c_pb_field(fobj, table, key_array, field, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name
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
			print >> fobj, '    %s->has_%s = TRUE;' % (pb_paramvalue, field_name_low)
			if ('FLAGS8' == dt_type[1] or 'FLAGS32' == dt_type[1]):			
				data_value = field[1]
				field_value = field[3]
				flag_value = field[4]
				print >> fobj, '    %s->%s = GLB_FLAG_ISSET(%s->%s, %s) ? TRUE : FALSE;' % (pb_paramvalue, field_name_low, cdb_paramvalue, data_value, flag_value)
			else:
				print >> fobj, '    %s->%s = %s->%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		elif ('uint64' == pb_type or 'int32' == pb_type or 'int64' == pb_type or 'double' == pb_type):
			print >> fobj, '    %s->has_%s = TRUE;' % (pb_paramvalue, field_name_low)
			print >> fobj, '    %s->%s = %s->%s;' % (pb_paramvalue, field_name_low, cdb_paramvalue, field_name)
		elif ('bytes' == pb_type):
			print >> fobj, '    %s->has_%s = TRUE;' % (pb_paramvalue, field_name_low)
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

def gen_func_pb_c_pb_check_array(fobj, table, key_array, data_array, master_array, field_array, dt_array):
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

def gen_func_pb_c_pb(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	func_name_pb = 'pb_' + tbl_name_low + '_to_pb'
	cdb_tbl_name = tbl_name_low + '_t'
	pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name)
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name
	tbl_has_key = 0
	has_repeated = 0

	if (len(key_array)):
		tbl_has_key = 1
	
	has_repeated = gen_func_pb_c_pb_check_array(fobj, table, key_array, data_array, master_array, field_array, dt_array)

	print >> fobj, "int32"
	print >> fobj, "%s(uint32 only_key, %s *%s, %s *%s)" % (func_name_pb, cdb_tbl_name, cdb_paramvalue, pb_tbl_name, pb_paramvalue)
	print >> fobj, "{"

	if (has_repeated):
		print >> fobj, "    uint32 i = 0;"
		print >> fobj, ""

	if (tbl_has_key):
		gen_func_pb_c_pb_key(fobj, table, key_array, field_array[0], dt_array)
		print >> fobj, "    if (only_key)"
		print >> fobj, "    {"
		print >> fobj, "        return PM_E_NONE;"
		print >> fobj, "    }"
		print >> fobj, ""
	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_field(fobj, table, key_array, field, dt_array)
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def gen_func_pb_c_pb_free_field(fobj, table, key_array, field, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name
	bypass = 0
	key_field_name = ""

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
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
			print >> fobj, "    if (%s->%s)" % (pb_paramvalue, field_name_low)
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
			print >> fobj, "    if (%s->%s.data)" % (pb_paramvalue, field_name)
			print >> fobj, "    {"
			print >> fobj, "        XFREE(MEM_LIB_PROTOBUF, %s->%s.data);" % (pb_paramvalue, field_name_low)
			print >> fobj, "        %s->%s.data = NULL;" % (pb_paramvalue, field_name_low)
			print >> fobj, "    }"
			print >> fobj, ""

def gen_func_pb_c_pb_free(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	func_name_pb = 'pb_' + tbl_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	cdb_tbl_name = tbl_name_low + '_t'
	pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name)
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name

	print >> fobj, "int32"
	print >> fobj, "%s(%s *%s)" % (func_name_pb_free, pb_tbl_name, pb_paramvalue)
	print >> fobj, "{"
	
	has_repeated = gen_func_pb_c_pb_check_array(fobj, table, key_array, data_array, master_array, field_array, dt_array)
	if (2 == has_repeated):
		print >> fobj, "    uint32 i = 0;"
		print >> fobj, ""

	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_free_field(fobj, table, key_array, field, dt_array)
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""

def pb_declare_compose(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	compose_array = get_compose_field_array(fobj, table, key_array, data_array, master_array, field_array, dt_array)
	for i, compose_item in enumerate(compose_array):
		compose_type = compose_item[0]
		field_name_low = compose_item[1]
		compose_tbl_name = cdb_util.pb_get_compse_name(compose_type)
		compose_str = compose_tbl_name
		compose_init_str = 'CDB__COMPOSE_' + string.upper(compose_type) + '__INIT'
		print >> fobj, '    %s %s = %s;' % (compose_str, field_name_low, compose_init_str)


def pb_set_compose(fobj, table, key_array, data_array, master_array, field_array, dt_array, pb_paramvalue):
	compose_array = get_compose_field_array(fobj, table, key_array, data_array, master_array, field_array, dt_array)
	for i, compose_item in enumerate(compose_array):
		compose_type = compose_item[0]
		field_name_low = compose_item[1]
		print >> fobj, "    %s.%s = &%s;" % (pb_paramvalue, field_name_low, field_name_low)


def gen_func_pb_c_pb_pack(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	func_name_pb = 'pb_' + tbl_name_low + '_to_pb'
	func_name_pb_free = func_name_pb + '_free_packed'
	func_name_pb_pack = 'pb_' + tbl_name_low + '_pack'
	cdb_func_name_pb_pack = 'cdb__' + tbl_name_low + '__pack'
	cdb_tbl_name = tbl_name_low + '_t'
	pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name)
	pb_tbl_name_key = pb_tbl_name + 'Key'
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'pb_' + tbl_value_name
	pb_paramvalue_key = pb_paramvalue + '_key'
	tbl_key_init = 'CDB__' + tbl_name + '_KEY' + '__INIT'
	tbl_init = 'CDB__' + tbl_name + '__INIT'
	tbl_has_key = 0

	if (len(key_array)):
		tbl_has_key = 1

	print >> fobj, "int32"
	print >> fobj, "%s(void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len)" % (func_name_pb_pack)
	print >> fobj, "{"

	if (tbl_has_key):
		key_compose = cdb_util.pb_get_key_compose_not_typedef(table, key_array[0], dt_array)
		if (key_compose):
			key_compose_name = cdb_util.pb_get_compse_name(key_compose[0])
			key_compose_param = key_compose[1]
			key_compose_init = 'CDB__COMPOSE_' + string.upper(key_compose[0]) + '__INIT'
#			print "AAAAAAAA %s" % (key_compose)
			print >> fobj, "    %s %s = %s;" % (key_compose_name, key_compose_param, key_compose_init)
		print >> fobj, "    %s %s = %s;" % (pb_tbl_name_key, pb_paramvalue_key, tbl_key_init)
	print >> fobj, "    %s %s = %s;" % (pb_tbl_name, pb_paramvalue, tbl_init)
	print >> fobj, "    %s *%s = (%s*)p_tbl;" % (cdb_tbl_name, cdb_paramvalue, cdb_tbl_name)
	print >> fobj, "    int32 len = 0;"
	pb_declare_compose(fobj, table, key_array, data_array, master_array, field_array, dt_array)
	print >> fobj, ""
	if (tbl_has_key):
		if (key_compose):
			print >> fobj, "    %s.%s = &%s;" % (pb_paramvalue_key, key_compose_param, key_compose_param)
		print >> fobj, "    %s.key = &%s;" % (pb_paramvalue, pb_paramvalue_key)
	pb_set_compose(fobj, table, key_array, data_array, master_array, field_array, dt_array, pb_paramvalue)
	print >> fobj, "    %s(only_key, %s, &%s);" % (func_name_pb, cdb_paramvalue, pb_paramvalue)
	print >> fobj, "    len = %s(&%s, buf);" % (cdb_func_name_pb_pack, pb_paramvalue)
	print >> fobj, "    %s(&%s);" % (func_name_pb_free, pb_paramvalue)
	print >> fobj, ""
	print >> fobj, "    return len;"
	print >> fobj, "}"
	print >> fobj, ""


def gen_func_pb_c_pb_dump_field(fobj, table, key_array, field, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(table[0])
	tbl_name_low_key = tbl_name_low + '_key'
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name
	bypass = 0
	key_field_name = ""

	if ('key' == field[1]):
		is_key = 1
		key = key_array[0]
		dt_type = cdb_util.check_data_dt_type(table, key, dt_array)
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
		print >> fobj, '    offset += sal_sprintf(out + offset, "/%s=");' % (field_name)
		pb_type = pb_type[1]
		is_repeated = 1
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

	if (is_key):
		print >> fobj, '    if (only_key)'
		print >> fobj, '    {'
		print >> fobj, '        offset += sal_sprintf(out + offset, "\\n");'
		print >> fobj, '        return PM_E_NONE;'
		print >> fobj, '    }'
		print >> fobj, '    '

def gen_func_pb_c_pb_dump(fobj, table, key_array, data_array, master_array, field_array, dt_array):
	tbl_name = table[0]
	tbl_name_low = string.lower(tbl_name)
	func_name_pb = 'pb_' + tbl_name_low + '_to_pb'
	func_name_pb_dump = 'pb_' + tbl_name_low + '_dump'
	cdb_tbl_name = tbl_name_low + '_t'
	pb_tbl_name = cdb_util.pb_get_tbl_name(tbl_name)
	tbl_value_name = table[1]
	cdb_paramvalue = 'p_' + tbl_value_name
	pb_paramvalue = 'p_pb_' + tbl_value_name

	print >> fobj, 'int32'
	print >> fobj, '%s(%s *%s, uint32 only_key, char *out)' % (func_name_pb_dump, pb_tbl_name, pb_paramvalue)
	print >> fobj, '{'
	print >> fobj, '    int32 offset = 0;'
	print >> fobj, ''
	print >> fobj, '    out[0] = CMD_ZERO_CHAR;'
	for i, field in enumerate(field_array):
		gen_func_pb_c_pb_dump_field(fobj, table, key_array, field, dt_array)
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
	print >> fobj, ''

	for i, tbl_one in enumerate(tbl_array):
		gen_proto_file_one(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)
	print "generate file %s" % (fname)

def gen_pb_c_file_one(fobj, tbl_one, ds_array, dt_array):
	table = tbl_one[0]
	tbl_name = table[0]
	print >> fobj, "/* pb functions for %s */" % (tbl_name)
	gen_func_pb_c_pb(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)
	gen_func_pb_c_pb_free(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)
	gen_func_pb_c_pb_pack(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)
	gen_func_pb_c_pb_dump(fobj, tbl_one[0], tbl_one[1], tbl_one[2], tbl_one[4], tbl_one[5], dt_array)

def gen_tbl_include(fobj, tbl_array, ds_array, dt_array):
	'generate store tbl c include'
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
		fname_h = 'gen/' + string.lower(tbl_name) + '.h'
		print >> fobj, "#include \"%s\"" % (fname_define_h)
		print >> fobj, "#include \"%s\"" % (fname_h)

def gen_pb_c_file(tbl_array, ds_array, dt_array):
	'generate pb_tbl.c file'
	fname = filename_out_pb_c
	fobj = open(fname, 'w')

	print >> fobj, ""
	print >> fobj, '#include "proto.h"'
	print >> fobj, '#include "pb_cdb_inc.h"'
	print >> fobj, '#include "tbl.pb-c.h"'
	print >> fobj, '#include "pb_tbl.h"'
	print >> fobj, '#include "pb_common.h"'
	print >> fobj, ""

	for i, tbl_one in enumerate(tbl_array):
		gen_pb_c_file_one(fobj, tbl_one, ds_array, dt_array)

	gen_pb_c_pack_array(fobj, tbl_array)
	gen_pb_c_unpack_array(fobj, tbl_array)
	gen_pb_c_dump_array(fobj, tbl_array)
	gen_pb_c_free_unpacked_array(fobj, tbl_array)

	print >> fobj, "int32"
	print >> fobj, "pb_tbl_pack(uint32 tbl_id, void *p_tbl, uint32 only_key, uint8 *buf, uint32 buf_len)"
	print >> fobj, "{"
	print >> fobj, "    if (tbl_id >= TBL_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_pack_tbl_func[tbl_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_pack_tbl_func[tbl_id](p_tbl, only_key, buf, buf_len);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "void*"
	print >> fobj, "pb_tbl_unpack(uint32 tbl_id, ProtobufCAllocator *allocator, uint32 len, uint8 *data)"
	print >> fobj, "{"
	print >> fobj, "    if (tbl_id >= TBL_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_unpack_tbl_func[tbl_id])"
	print >> fobj, "    {"
	print >> fobj, "        return NULL;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_unpack_tbl_func[tbl_id](allocator, len, data);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "pb_tbl_dump(uint32 tbl_id, void *object, uint32 only_key, char *out)"
	print >> fobj, "{"
	print >> fobj, "    if (tbl_id >= TBL_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_dump_tbl_func[tbl_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    return g_pb_dump_tbl_func[tbl_id](object, only_key, out);"
	print >> fobj, "}"
	print >> fobj, ""
	print >> fobj, "int32"
	print >> fobj, "pb_tbl_free_unpacked(uint32 tbl_id, void *object, ProtobufCAllocator *allocator)"
	print >> fobj, "{"
	print >> fobj, "    if (tbl_id >= TBL_MAX)"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_INVALID_PARAM;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    if (NULL == g_pb_free_unpacked_tbl_func[tbl_id])"
	print >> fobj, "    {"
	print >> fobj, "        return PM_E_NOT_SUPPORT;"
	print >> fobj, "    }"
	print >> fobj, ""
	print >> fobj, "    g_pb_free_unpacked_tbl_func[tbl_id](object, allocator);"
	print >> fobj, ""
	print >> fobj, "    return PM_E_NONE;"
	print >> fobj, "}"
	print >> fobj, ""
	fobj.close()
	print "generate file %s" % (fname)