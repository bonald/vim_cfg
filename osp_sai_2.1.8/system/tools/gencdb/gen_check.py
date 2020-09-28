#!/usr/bin/python
import os
import sys
import string
import CONST
import cdb_util

trace_on = 0

glb_pass = 1
filename_input_node	= CONST.CDB_INPUT_PATH + 'node.txt'
filename_input_tbl = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds  = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_dt = CONST.CDB_INPUT_PATH + 'datatype.txt'
filename_input_memory_h	= '../../lib/util/include/ctclib_memory.h'
filename_input_memory_c	= '../../lib/util/src/memory.c'
filename_input_cdb_ds_c	= '../../cdb/core/src/cdb_ds.c'
filename_input_cmp_c	= '../../cdb/core/src/cdb_data_cmp.c'

dir_array = []
tbl_array = []
ds_array  = []
act_array = []
bat_array = []
memory_h_array = []
memory_c_array = []

cmp_func_array = []
val2str_array = []
val2str_func_array = []
api_tbl_array = []
api_ds_array = []
api_dt_oper_array = []
api_dt_array = []

def parse_memory_h_file(memory_h_array, input_file):
	'Parse input ctclib_memory.h'
	file_in = open(input_file, 'r')
	line_no = 0
	for line_no, eachLine in enumerate(file_in):
		if ('MEM_TBL_' in eachLine or 'MEM_DS_' in eachLine):
			entity = eachLine.split()
			memory = entity[0].strip(',')
			memory_h_array.append(memory)
	file_in.close()

def parse_memory_c_file(memory_c_array, input_file):
	'Parse input memory.c'
	file_in = open(input_file, 'r')
	line_no = 0
	for line_no, eachLine in enumerate(file_in):
		if ('MEM_TBL_' in eachLine or 'MEM_DS_' in eachLine):
			entity = eachLine.split()
			memory = entity[0]
			memory = memory.strip(',')
			memory = memory.strip('{')
			memory = memory.strip('}')
			memory_c_array.append(memory)
	file_in.close()

def parse_cdb_ds_c_file_referlist(referlist_val2str_array, input_file):
	'Parse input cdb_ds.c'
	file_in = open(input_file, 'r')
	line_no = 0
	func_range = 0
	for line_no, eachLine in enumerate(file_in):
		if ('cdb_reference_list_val2str' in eachLine):
			func_range = 1
			if ('}' == eachLine[0]):
				func_range = 0		
		if (func_range):
			if ('case TBL_' in eachLine):
				referlist_val2str_array.append(eachLine)
	file_in.close()

def parse_cdb_ds_c_file_val2str(val2str_func_array, input_file):
	'Parse input cdb_ds.c'
	file_in = open(input_file, 'r')
	line_no = 0
	for line_no, eachLine in enumerate(file_in):
		line = eachLine.split()
		if (0 == len(line)):
			continue
		if ('_val2str' in line[0]):
			val2str_func_array.append(line[0])
	file_in.close()


def parse_cmp_file(cmp_func_array, input_file):
	'Parse input cdb_data_cmp.c'
	file_in = open(input_file, 'r')
	line_no = 0
	for line_no, eachLine in enumerate(file_in):
		line = eachLine.split()
		if (0 == len(line)):
			continue
		if ('_cmp' in line[0]):
			cmp_func_array.append(line[0])
	file_in.close()

def check_memory_name_in_array(memory_name, memory_h_array, memory_c_array):
	is_found = 0
	global glb_pass
	for i, memory in enumerate(memory_h_array):
		if (memory_name == memory):
			is_found = 1
			break
	if (0 == is_found):
		print "%s: not found %s in ctclib_memory.h" % (cdb_util.inred('FAIL'), cdb_util.ingreen(memory_name))
		glb_pass = 0

	is_found = 0
	for i, memory in enumerate(memory_c_array):
		if (memory_name == memory):
			is_found = 1
			break
	if (0 == is_found):
		print "%s: not found %s in memory.c" % (cdb_util.inred('FAIL'), cdb_util.ingreen(memory_name))
		glb_pass = 0

def find_tbl_in_val2str_array(tbl_name, val2str_array):
	for i, val2str in enumerate(val2str_array):
		if (tbl_name in val2str):
			return val2str

def find_cmp_in_cmp_array(cmp_func, cmp_func_array):
	for i, cmp_func_ in enumerate(cmp_func_array):
		if (cmp_func in cmp_func_):
			return cmp_func_

#########################################################
# 1. parse node.txt
#########################################################
cdb_util.parse_input_node_file(dir_array, tbl_array, ds_array, act_array, bat_array, filename_input_node)
parse_memory_h_file(memory_h_array, filename_input_memory_h)
parse_memory_c_file(memory_c_array, filename_input_memory_c)
parse_cdb_ds_c_file_referlist(val2str_array, filename_input_cdb_ds_c)
parse_cdb_ds_c_file_val2str(val2str_func_array, filename_input_cdb_ds_c)
parse_cmp_file(cmp_func_array, filename_input_cmp_c)

#########################################################
# 1. parse dt/tbl/ds.txt
#########################################################
cdb_util.parse_input_dt_file(api_dt_oper_array, api_dt_array, filename_input_dt, trace_on)
cdb_util.parse_input_tbl_file(api_tbl_array, api_dt_array, filename_input_tbl, trace_on)
cdb_util.parse_input_ds_file(api_tbl_array, api_ds_array, api_dt_array, filename_input_ds, trace_on)

#########################################################
# 2. check memory.h
#########################################################
for i, entity in enumerate(tbl_array):
	tbl_name = entity[0]
	memory_name = 'MEM_' + tbl_name
	check_memory_name_in_array(memory_name, memory_h_array, memory_c_array)

#########################################################
# 3. check memory.c
#########################################################
for i, entity in enumerate(ds_array):
	tbl_name = entity[0]
	memory_name = 'MEM_' + tbl_name
	check_memory_name_in_array(memory_name, memory_h_array, memory_c_array)

#########################################################
# 4. check referlist table in cdb_ds.c
#########################################################
for i, tbl_one in enumerate(api_tbl_array):
	tbl_name = tbl_one[0][0]
	data_array = tbl_one[2]
	for j, field in enumerate(data_array):
		if ('REFERLIST_TBL_' in field[0]):
			referlist_tbl_name = field[0][10:]
			val2str = find_tbl_in_val2str_array(referlist_tbl_name, val2str_array)
			if (None == val2str):
				print "%s: %s not found case %s in function cdb_reference_list_val2str()" % (cdb_util.inred('FAIL'), cdb_util.ingreen(tbl_name), cdb_util.ingreen(referlist_tbl_name))
				glb_pass = 0

#########################################################
# 5. check cmp function in cdb_data_cmp.c
#########################################################
need_cmp_array = []
for i, tbl_one in enumerate(api_tbl_array):
	tbl_name = tbl_one[0][0]
	data_array = tbl_one[2]
	for j, field in enumerate(data_array):
		if ('REFERLIST_' in field[0]):
			referlist_tbl_name = field[0][10:]
			need_cmp_array.append(referlist_tbl_name)

for i, tbl_one in enumerate(api_ds_array):
	tbl_name = tbl_one[0][0]
	data_array = tbl_one[2]
	for j, field in enumerate(data_array):
		if ('REFERLIST_' in field[0]):
			referlist_tbl_name = field[0][10:]
			need_cmp_array.append(referlist_tbl_name)

for i, tbl_one in enumerate(api_tbl_array):
	tbl_name = tbl_one[0][0]
	tbl_ds_type = tbl_one[0][2]
	if ('LIST' in tbl_ds_type):
		need_cmp_array.append(tbl_name)

need_cmp_array = cdb_util.array_remove_duplicate_entry(need_cmp_array)
for i, cmp_tbl in enumerate(need_cmp_array):
	dt_cmp_func = string.lower(cmp_tbl) + '_cmp'
	ret_data = find_cmp_in_cmp_array(dt_cmp_func, cmp_func_array)
	if (None == ret_data):
		print "%s: not found function %s in cdb_data_cmp.c" % (cdb_util.inred('FAIL'), cdb_util.ingreen(dt_cmp_func))
		glb_pass = 0

#########################################################
# 6. check val2str in cdb_ds.c
#########################################################
oper_val2str_array = []
for i, dt_oper in enumerate(api_dt_oper_array):
	if (len(dt_oper) >= 5):
		oper_val2str_array.append(dt_oper[4])

for i, val2str_func in enumerate(oper_val2str_array):
	ret_data = find_tbl_in_val2str_array(val2str_func, val2str_func_array)
	if (None == ret_data):
		if ('ctc_task_val2str' != val2str_func):
			print "%s: not found function %s in cdb_ds.c" % (cdb_util.inred('FAIL'), cdb_util.ingreen(val2str_func))
			glb_pass = 0

if (glb_pass):
	print "Pass!"
else:
	sys.exit(1)
