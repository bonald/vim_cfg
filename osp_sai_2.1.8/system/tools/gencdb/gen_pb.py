#!/usr/bin/python
import os
import sys
import string
import CONST
import cdb_util
import tbl_pb
import ds_pb
import commands

trace_on = 0
if (len(sys.argv) > 1):
	if ('-trace' == sys.argv[1]):
		trace_on = 1

GEN_PB_PROTO_TBL	= 1
GEN_PB_H_TBL		= 1
GEN_PB_C_TBL		= 1
GEN_PB_PROTO_DS		= 1
GEN_PB_H_DS			= 1
GEN_PB_C_DS			= 1
PROTO_GEN_CODE		= 1

filename_input_tbl = CONST.CDB_INPUT_PATH + 'tbl.txt'
filename_input_ds  = CONST.CDB_INPUT_PATH + 'ds.txt'
filename_input_dt = CONST.CDB_INPUT_PATH + 'datatype.txt'
filename_pb_inc_h  = CONST.PBC_H_PATH + 'pb_cdb_inc.h'

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

def gen_pb_cdb_include_file(tbl_array):
	'generate pb cdb include'
	file_output = open(filename_pb_inc_h, 'w')
	print >> file_output, ""
	print >> file_output, "#ifndef __PB_CDB_INC_H__"
	print >> file_output, "#define __PB_CDB_INC_H__"
	print >> file_output, ""
	for i, tbl_one in enumerate(tbl_array):
		tbl_name = tbl_one[0][0]
		fname_define_h = 'gen/' + string.lower(tbl_name) + '_define.h'
		fname_h = 'gen/' + string.lower(tbl_name) + '.h'
		print >> file_output, "#include \"%s\"" % (fname_define_h)
		print >> file_output, "#include \"%s\"" % (fname_h)
	print >> file_output, ""
	print >> file_output, "#endif /* !__PB_CDB_INC_H__*/ "
	print >> file_output, ""
	file_output.close()
	print "generate file %s" % (filename_pb_inc_h)

def proto_gen_code():
	ret = commands.getstatusoutput('protoc-c --c_out=. common.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for common.proto:\n%s", ret[1])
	ret = commands.getstatusoutput('protoc-c --c_out=. tbl.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for tbl.proto:\n%s", ret[1])
	ret = commands.getstatusoutput('protoc-c --c_out=. ds.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for ds.proto:\n%s", ret[1])
	ret = commands.getstatusoutput('sed -i "s/<protobuf-c\/protobuf-c.h>/\\"protobuf-c.h\\"/g" *.pb-c.h')
	ret = commands.getstatusoutput('mv *.pb-c.h protobuf/include/gen/')
	if (0 != ret[0]):
		cdb_util.error_return("Error when move h files\n%s", ret[1])
	ret = commands.getstatusoutput('mv *.pb-c.c protobuf/src/gen/')
	if (0 != ret[0]):
		cdb_util.error_return("Error when move c files\n%s", ret[1])
	print "generate proto c API files"

	ret = commands.getstatusoutput('protoc --python_out=./protobuf/py/ common.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for common.proto:\n%s", ret[1])
	ret = commands.getstatusoutput('protoc --python_out=./protobuf/py/ tbl.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for tbl.proto:\n%s", ret[1])
	ret = commands.getstatusoutput('protoc --python_out=./protobuf/py/ ds.proto')
	if (0 != ret[0]):
		cdb_util.error_return("Error when create c codes for ds.proto:\n%s", ret[1])
	print "generate proto python API  files"

gen_pb_cdb_include_file(tbl_array)

############################################################
# gen tbl.proto
if (GEN_PB_PROTO_TBL):
	tbl_pb.gen_proto_file(tbl_array, ds_array, dt_array)

############################################################
# gen pb_tbl.h
if (GEN_PB_H_TBL):
	tbl_pb.gen_pb_h_file(tbl_array, ds_array, dt_array)

############################################################
# gen pb_tbl.c
if (GEN_PB_C_TBL):
	tbl_pb.gen_pb_c_file(tbl_array, ds_array, dt_array)

############################################################
# gen ds.proto
if (GEN_PB_PROTO_DS):
	ds_pb.gen_proto_file(tbl_array, ds_array, dt_array)

############################################################
# gen pb_ds.h
if (GEN_PB_H_DS):
	ds_pb.gen_pb_h_file(tbl_array, ds_array, dt_array)

############################################################
# gen pb_ds.c
if (GEN_PB_C_DS):
	ds_pb.gen_pb_c_file(tbl_array, ds_array, dt_array)

if (PROTO_GEN_CODE):
	proto_gen_code()

