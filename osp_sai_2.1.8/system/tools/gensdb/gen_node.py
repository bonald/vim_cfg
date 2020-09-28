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

filename_input_node	= CONST.CDB_INPUT_PATH + 'node.txt'
filename_def		= CONST.CDB_H_PATH + 'sdb_define.h'
filename_dir		= CONST.CDB_H_PATH + 'sdb_dir_priv.h'
filename_tbl		= CONST.CDB_H_PATH + 'sdb_tbl_priv.h'
filename_ds	        = CONST.CDB_H_PATH + 'sdb_ds_priv.h'
filename_act		= CONST.CDB_H_PATH + 'sdb_act_priv.h'
filename_bat		= CONST.CDB_H_PATH + 'sdb_bat_priv.h'
filename_inc		= CONST.CDB_H_PATH + 'sdb_inc_priv.h'
filename_dest_pm	= CONST.CDB_H_PATH + 'sdb_dest_pm_priv.h'

dir_array = []
tbl_array = []
ds_array  = []
act_array = []
bat_array = []
dir_fullname_array = []
tbl_fullname_array = []
ds_fullname_array  = []
act_fullname_array = []
bat_fullname_array = []

enum_define = 0

def build_enum(fobj, entity, i):
	'build enum sdb_dir_t'
	print >> fobj, "    %-20s = %d, /* %s */" % (entity[0], i, entity[1])

def build_entity_dir(fobj, fullname_array, entity, i):
	'build sdb_node_t g_sdb_dir_node_info'
	print >> fobj, "    {"
	print >> fobj, "        %s," % (entity[0])
	print >> fobj, "        %s," % ('SDB_NODE_DIR')
	print >> fobj, "        %s," % (CONST.PRODUCT_PREFIX + entity[2])
	print >> fobj, "        \"%s\"," % (string.lower(entity[0][5:]))
	print >> fobj, "        \"%s\"," % (fullname_array[i])
	if ('NULL' == entity[1]):
		print >> fobj, "        %s," % ('NULL')
	else:
		print >> fobj, "        &g_sdb_dir_node_info[%s]," % (entity[1])
	print >> fobj, "    },"

def build_entity_tbl(fobj, fullname_array, entity, i):
	'build sdb_node_t g_tbl_node_info'
	print >> fobj, "    {"
	print >> fobj, "        %s," % (entity[0])
	print >> fobj, "        %s," % ('SDB_NODE_TBL')
	print >> fobj, "        %s," % (CONST.PRODUCT_PREFIX + entity[3])
	print >> fobj, "        \"%s\"," % (string.lower(entity[0][5:]))
	print >> fobj, "        \"%s\"," % (fullname_array[i])
	print >> fobj, "        &g_sdb_dir_node_info[%s]," % (entity[1])
	print >> fobj, "        &g_sdb_tbl_info[%s]," % (entity[0])
	print >> fobj, "    },"

def build_call_tbl_init(fobj, fullname_array, entity, i):
	tbl_name_upper = entity[0]
	tbl_name = string.lower(tbl_name_upper)
	accusative = tbl_name[4:]
	print >> fobj, "        if (%s == i)" % (tbl_name_upper)
	print >> fobj, "        {"
	print >> fobj, "            %s_init_%s();" % (tbl_name, accusative)
	print >> fobj, "        }"

def build_entity_ds(fobj, fullname_array, entity, i):
	'build sdb_node_t g_ds_node_info'
	print >> fobj, "    {"
	print >> fobj, "        %s," % (entity[0])
	print >> fobj, "        %s," % ('SDB_NODE_DS')
	print >> fobj, "        %s," % (CONST.PRODUCT_PREFIX + entity[2])
	print >> fobj, "        \"%s\"," % (string.lower(entity[0][3:]))
	print >> fobj, "        \"%s\"," % (fullname_array[i])
	print >> fobj, "        &g_tbl_node_info[%s]," % (entity[1])
	print >> fobj, "        &g_sdb_ds_info[%s]," % (entity[0])
	print >> fobj, "    },"

def build_entity_act(fobj, fullname_array, entity, i):
	'build sdb_node_t g_act_node_info'
	print >> fobj, "    {"
	print >> fobj, "        %s," % (entity[0])
	print >> fobj, "        %s," % ('SDB_NODE_ACT')
	print >> fobj, "        %s," % (CONST.PRODUCT_PREFIX + entity[3])
	print >> fobj, "        \"%s\"," % (string.lower(entity[0][4:]))
	print >> fobj, "        \"%s\"," % (fullname_array[i])
	print >> fobj, "        &g_sdb_dir_node_info[%s]," % (entity[1])
	print >> fobj, "        &g_sdb_act_info[%s]," % (entity[0])
	print >> fobj, "    },"

def build_entity_dest_pm(fobj, entity, i):
	'build sdb_node_t g_bat_node_info'
	item_name = entity[0]
	dest_pm = entity[2] + ','
	print >> fobj, "    %-20s /* %s */" % (dest_pm, item_name)

#########################################################
# write 
#########################################################
def build_fullname(fullname_array, dir_fullname_array, dir_array, entity, i, array_type, parent_ds=None):
	'build fullanme'
	global trace_on
	find_parent = 0
	full_name = ''
	entity_name = '/'

	if (parent_ds):
		for j, parent in enumerate(dir_array):
			if (parent_ds[1] == parent[0]):
				find_parent = 1		# find a parent
				break
	elif ('NULL' == entity[1]):
		find_parent = 2				# root, no parent
	else:
		for j, parent in enumerate(dir_array):
			if (entity[1] == parent[0]):
				find_parent = 1		# find a parent
				break

	if ('DS_' == entity[0][:3]):
		if (parent_ds):
			entity_name += string.lower(parent_ds[0][3:]) + '/' + string.lower(entity[0][3:])
		else:
			entity_name += string.lower(entity[0][3:])
	else:
		entity_name += string.lower(entity[0][5:])

	if (1 == find_parent):
		full_name = dir_fullname_array[j] + entity_name
		fullname_array.append(full_name)
	elif (2 == find_parent):
		full_name = entity_name
		fullname_array.append(full_name)
	else:
		cdb_util.error_return("parent entity %s not found", entity[1])
	cdb_util.debug_out(trace_on, "%-4d %-4s %-s", i, array_type, fullname_array[i])

#########################################################
# create output dirs
#########################################################
cdb_util.create_output_dirs()

#########################################################
# 1. parse node.txt
#########################################################
cdb_util.parse_input_node_file(dir_array, tbl_array, ds_array, act_array, bat_array, filename_input_node)

#########################################################
# 2. build full name
#########################################################
cdb_util.debug_out(trace_on, "\n##### %s #####", 'Fullpath Trace Begin')
cdb_util.debug_out(trace_on, "%-4s %-4s %-s", 'Id', 'Type', 'Fullpath')

def get_parent_ds(entity, ds_array):
	for i, entity_tmp in enumerate(ds_array):
		if (entity[1] == entity_tmp[0]):
			return entity_tmp
	return None

for i, entity in enumerate(dir_array):
	build_fullname(dir_fullname_array, dir_fullname_array, dir_array, entity, i, 'dir')

for i, entity in enumerate(tbl_array):
	build_fullname(tbl_fullname_array, dir_fullname_array, dir_array, entity, i, 'tbl')

for i, entity in enumerate(ds_array):
	parent_ds = get_parent_ds(entity, ds_array)
	build_fullname(ds_fullname_array, tbl_fullname_array, tbl_array, entity, i, 'ds', parent_ds)

for i, entity in enumerate(act_array):
	build_fullname(act_fullname_array, dir_fullname_array, dir_array, entity, i, 'act')

for i, entity in enumerate(bat_array):
	build_fullname(bat_fullname_array, dir_fullname_array, dir_array, entity, i, 'bat')
cdb_util.debug_out(trace_on, "##### %s #####\n", 'Fullpath Trace Begin')

#########################################################
# 3. build cdb_define.h
#########################################################
file_def = open(filename_def, 'w')

print >> file_def, ""
print >> file_def, "#ifndef __SDB_DEFINE_H__"
print >> file_def, "#define __SDB_DEFINE_H__"
print >> file_def, ""
print >> file_def, "typedef enum"
print >> file_def, "{"
for i, entity in enumerate(dir_array):
	build_enum(file_def, entity, i)
print >> file_def, "    %sMAX" % (CONST.NODE_PREFIX_DIR)
print >> file_def, "} sdb_dir_t;"
print >> file_def, ""

print >> file_def, "typedef enum"
print >> file_def, "{"
for i, entity in enumerate(tbl_array):
	build_enum(file_def, entity, i)
print >> file_def, "    %sMAX" % (CONST.NODE_PREFIX_TBL)
print >> file_def, "} sdb_tbl_t;"
print >> file_def, ""

#print >> file_def, "typedef enum"
#print >> file_def, "{"
#for i, entity in enumerate(ds_array):
#	build_enum(file_def, entity, i)
#print >> file_def, "    %sMAX" % (CONST.NODE_PREFIX_DS)
#print >> file_def, "} sdb_ds_t;"
#print >> file_def, ""

#print >> file_def, "typedef enum"
#print >> file_def, "{"
#for i, entity in enumerate(act_array):
#	build_enum(file_def, entity, i)
#print >> file_def, "    %sMAX" % (CONST.NODE_PREFIX_ACT)
#print >> file_def, "} sdb_act_t;"
#print >> file_def, ""

print >> file_def, "#endif /* !__SDB_DEFINE_H__ */"
print >> file_def, ""

file_def.close()
print "generate file %s" % (filename_def)

#########################################################
# build cdb_dir_priv.h
#########################################################
file_dir = open(filename_dir, 'w')

_macro_h_ = "__SDB_DIR_PRIV_H__"
print >> file_dir, ""
print >> file_dir, "#ifndef %s" % (_macro_h_)
print >> file_dir, "#define %s" % (_macro_h_)
print >> file_dir, ""
print >> file_dir, "sdb_node_t g_sdb_dir_node_info[SDIR_MAX] ="
print >> file_dir, "{"
for i, entity in enumerate(dir_array):
	build_entity_dir(file_dir, dir_fullname_array, entity, i)
print >> file_dir, "};"
print >> file_dir, ""
print >> file_dir, "#endif /* !%s */" % (_macro_h_)
print >> file_dir, ""

file_dir.close()
print "generate file %s" % (filename_dir)

#########################################################
# build cdb_tbl_priv.h
#########################################################
file_tbl = open(filename_tbl, 'w')

_macro_h_ = "__SDB_TBL_PRIV_H__"
print >> file_tbl, ""
print >> file_tbl, "#ifndef %s" % (_macro_h_)
print >> file_tbl, "#define %s" % (_macro_h_)
print >> file_tbl, ""
print >> file_tbl, "sdb_node_t g_sdb_tbl_node_info[STBL_MAX] ="
print >> file_tbl, "{"
for i, entity in enumerate(tbl_array):
	build_entity_tbl(file_tbl, tbl_fullname_array, entity, i)
print >> file_tbl, "};"
print >> file_tbl, ""
print >> file_tbl, "#endif /* !%s */" % (_macro_h_)
print >> file_tbl, ""

file_tbl.close()
print "generate file %s" % (filename_tbl)

##########################################################
## build cdb_ds_priv.h
##########################################################
#file_ds = open(filename_ds, 'w')
#_macro_h_ = "__SDB_DS_PRIV_H__"
#print >> file_ds, ""
#print >> file_ds, "#ifndef %s" % (_macro_h_)
#print >> file_ds, "#define %s" % (_macro_h_)
#print >> file_ds, ""
#print >> file_ds, "sdb_node_t g_sdb_ds_node_info[SDS_MAX] ="
#print >> file_ds, "{"
#for i, entity in enumerate(ds_array):
#	build_entity_ds(file_ds, ds_fullname_array, entity, i)
#print >> file_ds, "};"
#print >> file_ds, ""
#print >> file_ds, "#endif /* !%s */" % (_macro_h_)
#print >> file_ds, ""
#
#file_ds.close()
#print "generate file %s" % (filename_ds)
#
##########################################################
## build cdb_act_priv.h
##########################################################
#file_act = open(filename_act, 'w')
#_macro_h_ = "__SDB_ACT_PRIV_H__"
#print >> file_act, ""
#print >> file_act, "#ifndef %s" % (_macro_h_)
#print >> file_act, "#define %s" % (_macro_h_)
#print >> file_act, ""
#print >> file_act, "sdb_node_t g_sdb_act_node_info[SACT_MAX] ="
#print >> file_act, "{"
#for i, entity in enumerate(act_array):
#	build_entity_act(file_act, act_fullname_array, entity, i)
#print >> file_act, "};"
#print >> file_act, ""
#print >> file_act, "#endif /* !%s */" % (_macro_h_)
#print >> file_act, ""
#
#file_act.close()
#print "generate file %s" % (filename_act)

#########################################################
# build cdb_inc.h
#########################################################
file_inc = open(filename_inc, 'w')

print >> file_inc, ""
for i, entity in enumerate(tbl_array):
	tbl_type = string.lower(entity[0])
	print >> file_inc, "#include \"%s%s\"" % (tbl_type, '_define.h')
	print >> file_inc, "#include \"%s%s\"" % (tbl_type, '.h')
print >> file_inc, ""

file_inc.close()
print "generate file %s" % (filename_inc)

##########################################################
## build cdb_dest_pm_priv.h
##########################################################
#file_pm = open(filename_dest_pm, 'w')
#
#_macro_h_ = "__CDB_DEST_PM_PRIV_H__"
#print >> file_pm, ""
#print >> file_pm, "#ifndef %s" % (_macro_h_)
#print >> file_pm, "#define %s" % (_macro_h_)
#print >> file_pm, ""
#print >> file_pm, "const static uint32 g_sdb_tbl_dest_pm[STBL_MAX] ="
#print >> file_pm, "{"
#for i, entity in enumerate(tbl_array):
#	build_entity_dest_pm(file_pm, entity, i)
#print >> file_pm, "};"
#print >> file_pm, ""
#print >> file_pm, "const static uint32 g_sdb_act_dest_pm[SACT_MAX] ="
#print >> file_pm, "{"
#for i, entity in enumerate(act_array):
#	build_entity_dest_pm(file_pm, entity, i)
#print >> file_pm, "};"
#print >> file_pm, ""
#print >> file_pm, "#endif /* !%s */" % (_macro_h_)
#print >> file_pm, ""
#
#file_pm.close()
#print "generate file %s" % (filename_dest_pm)

############################################################
# print notice
print "%s: please make sure all TBL/DS has added corresponding %s/%s in %s" % (cdb_util.inred('Notice'), cdb_util.ingreen('MEM_TBL_XXX'), cdb_util.ingreen('MEM_DS_XXX'), CONST.MEMORY_FILES)
