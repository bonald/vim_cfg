#!/usr/bin/python
import os
import sys
import string
import CONST
import cdb_util
import tbl_api_h
import tbl_api_c
import ds_api_h
import ds_api_c

trace_on = 0
if (len(sys.argv) > 1):
	if ('-trace' == sys.argv[1]):
		trace_on = 1

GEN_TBL_H		= 1
GEN_TBL_C		= 1
GEN_DS_H		= 1
GEN_DS_C		= 1

filename_input_tbl = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds  = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_dt = CONST.CDB_INPUT_PATH + 'datatype.txt'

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

############################################################
# gen tbl_xxx.h
if (GEN_TBL_H):
	tbl_api_h.gen_tbl_h_files(tbl_array)

############################################################
# gen tbl_xxx.c
if (GEN_TBL_C):
	tbl_api_c.gen_tbl_c_files(tbl_array, ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array)

############################################################
# gen ds_xxx.h
if (GEN_DS_H):
	ds_api_h.gen_ds_h_files(ds_array)

############################################################
# gen ds_xxx.c
if (GEN_DS_C):
	ds_api_c.gen_ds_c_files(ds_array, dt_array, cmp_func_array, val2str_func_array, referlist_tbl_array)

############################################################
# print notice
cmp_func_array = cdb_util.array_remove_duplicate_entry(cmp_func_array)
val2str_func_array = cdb_util.array_remove_duplicate_entry(val2str_func_array)
referlist_tbl_array = cdb_util.array_remove_duplicate_entry(referlist_tbl_array)

for i, cmp_func in enumerate(cmp_func_array):
	cdb_util.print_cmp_func(cmp_func)

for i, val2str_func in enumerate(val2str_func_array):
	cdb_util.print_val2str_func(val2str_func)

for i, table_name in enumerate(referlist_tbl_array):
	cdb_util.print_referlist_val2str(table_name)
