#!/usr/bin/python
import os
import sys
import string
import filecmp
import shutil
import CONST
import cdb_util

INC_SUB = 'include/gen/'
SRC_SUB = 'src/gen/'

GENCDB_LIB_BASE = './core/'
PROTO_LIB_BASE = '../../cdb/core/'

gencdb_h_dir = GENCDB_LIB_BASE + INC_SUB
gencdb_c_dir = GENCDB_LIB_BASE + SRC_SUB
proto_h_dir  = PROTO_LIB_BASE + INC_SUB
proto_c_dir  = PROTO_LIB_BASE + SRC_SUB

PB_INC_SUB = 'include/gen/'
PB_SRC_SUB = 'src/gen/'

PB_GENCDB_LIB_BASE = './protobuf/'
PB_PROTO_LIB_BASE = '../../cdb/protobuf/'

pb_gencdb_h_dir = PB_GENCDB_LIB_BASE + PB_INC_SUB
pb_gencdb_c_dir = PB_GENCDB_LIB_BASE + PB_SRC_SUB
pb_proto_h_dir  = PB_PROTO_LIB_BASE + PB_INC_SUB
pb_proto_c_dir  = PB_PROTO_LIB_BASE + PB_SRC_SUB

def gen_copy_get_files(dir):
	files = os.listdir(dir)
	file_array = []
	for name in files:
		fullname = os.path.join(dir, name)
		if (os.path.isdir(fullname)):
			continue
		file_array.append(name)
	return file_array

gencdb_h_file_array = gen_copy_get_files(gencdb_h_dir)
gencdb_c_file_array = gen_copy_get_files(gencdb_c_dir)
proto_h_file_array  = gen_copy_get_files(proto_h_dir)
proto_c_file_array  = gen_copy_get_files(proto_c_dir)

pb_gencdb_h_file_array = gen_copy_get_files(pb_gencdb_h_dir)
pb_gencdb_c_file_array = gen_copy_get_files(pb_gencdb_c_dir)
pb_proto_h_file_array  = gen_copy_get_files(pb_proto_h_dir)
pb_proto_c_file_array  = gen_copy_get_files(pb_proto_c_dir)

overwrite_array = []
overwrite_ignored_array = []
copy_array = []
print "src DIR %s has %u files" % (gencdb_h_dir, len(gencdb_h_file_array))
print "src DIR %s has %u files" % (gencdb_c_dir, len(gencdb_c_file_array))
print "dest DIR %s has %u files" % (proto_h_dir,  len(proto_h_file_array))
print "dest DIR %s has %u files" % (proto_c_dir,  len(proto_c_file_array))
print "PB src DIR %s has %u files" % (gencdb_h_dir, len(pb_gencdb_h_file_array))
print "PB src DIR %s has %u files" % (gencdb_c_dir, len(pb_gencdb_c_file_array))
print "PB dest DIR %s has %u files" % (proto_h_dir,  len(pb_proto_h_file_array))
print "PB dest DIR %s has %u files" % (proto_c_dir,  len(pb_proto_c_file_array))
print ""

def gen_overwrite_one_file(dest_dir, src_dir, name, overwrite_array, overwrite_ignored_array):
	dest_fullname_old = dest_dir+name+'.old'
	dest_fullname     = dest_dir+name
	src_fullname      = src_dir+name
	is_same = filecmp.cmp(dest_fullname, src_fullname)
	if (is_same):
		return

	if ('gencdb ignored' in open(dest_fullname).read()):
		overwrite_ignored_array.append(name)
	else:
		overwrite_array.append(name)
		if ('ds_' == name[:3] or 'tbl_' == name[:4]):
			print dest_fullname_old
			shutil.copy(dest_fullname, dest_fullname_old)
		shutil.copy(src_fullname, dest_fullname)

def gen_copy_one_file(dest_dir, src_dir, name, copy_array):
	dest_fullname = dest_dir+name
	src_fullname = src_dir+name
	copy_array.append(name)
	shutil.copy(src_fullname, dest_fullname)

def gen_copy_files(dest_dir, dest_files, src_dir, src_files, overwrite_array, overwrite_ignored_array, copy_array):
	for i, name in enumerate(src_files):
		if (name in dest_files):
			gen_overwrite_one_file(dest_dir, src_dir, name, overwrite_array, overwrite_ignored_array)
		else:
			gen_copy_one_file(dest_dir, src_dir, name, copy_array)

gen_copy_files(proto_h_dir, proto_h_file_array, gencdb_h_dir, gencdb_h_file_array, overwrite_array, overwrite_ignored_array, copy_array)
gen_copy_files(proto_c_dir, proto_c_file_array, gencdb_c_dir, gencdb_c_file_array, overwrite_array, overwrite_ignored_array, copy_array)
gen_copy_files(pb_proto_h_dir, pb_proto_h_file_array, pb_gencdb_h_dir, pb_gencdb_h_file_array, overwrite_array, overwrite_ignored_array, copy_array)
gen_copy_files(pb_proto_c_dir, pb_proto_c_file_array, pb_gencdb_c_dir, pb_gencdb_c_file_array, overwrite_array, overwrite_ignored_array, copy_array)

print "----- copy %s files -----"				% (cdb_util.inred(len(copy_array)))
for i, name in enumerate(copy_array):
	print "%s" % (cdb_util.inred(name))

print "----- overwrite %s files -----"			% (cdb_util.inred(len(overwrite_array)))
for i, name in enumerate(overwrite_array):
	print "%s" % (cdb_util.inred(name))

print "----- ignore %s files -----" % (cdb_util.ingreen(len(overwrite_ignored_array)))
for i, name in enumerate(overwrite_ignored_array):
	print "%s" % (cdb_util.ingreen(name))
