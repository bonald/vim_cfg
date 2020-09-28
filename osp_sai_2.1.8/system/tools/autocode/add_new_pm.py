#!/usr/bin/python
import os
import sys
import string
import filecmp
import shutil
import CONST
import code_util
import pm_info

import re
from stat import *

def one_digital_add_one(oldLine):
	oldcount = re.sub("\D", "", oldLine)
	if (oldcount):
		newcount = str(int(oldcount)+1)
		newLine = oldLine.replace(oldcount, newcount)
		return newLine
	else:
		return oldLine

def update_ctc_msg_h(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if ('g_ipc_dest_desc[(_dest_)' in eachLine):
			newLine = one_digital_add_one(eachLine)
			file_out.write(newLine)
		else:
			file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_ctc_msg_c(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	insert_line_no = -1
	for line_no, eachLine in enumerate(file_in):
		if ('g_ipc_dest_desc[]' in eachLine):
			insert_line_no = line_no+pm_info.NEW_PM_INFO[1]+1

		if (insert_line_no == line_no):
			file_out.write(eachLine)
			new_pm_str = pm_info.NEW_PM_INFO[0][6:].lower()
			newLine = '    "' + new_pm_str + '",\n'
			file_out.write(newLine)
		else:
			file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_proto_module_h(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	insert_line_no = -1
	need_modify = 0
	for line_no, eachLine in enumerate(file_in):
		if (str(pm_info.NEW_PM_INFO[1]) in eachLine):
			print >> file_out, '    %-14s      = %d,' % (pm_info.NEW_PM_INFO[0], pm_info.NEW_PM_INFO[1])
			newLine = one_digital_add_one(eachLine)
			file_out.write(newLine)
			need_modify = 1
		else:
			if (need_modify):
				newLine = one_digital_add_one(eachLine)
				file_out.write(newLine)
			else:
				file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_proto_module_c(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	case_no = 0
	for line_no, eachLine in enumerate(file_in):
		if ('case PM_ID_' in eachLine):
			case_no += 1
		if (pm_info.NEW_PM_INFO[1] == case_no+1):
			case_no += 1000
			print >> file_out, '    case %s:' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '        return "%s";' % (pm_info.NEW_PM_INFO[0][6:])

		file_out.write(eachLine)

	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_cds_c(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if ('/* Add new PM_ID_XXX subscribe TBLs below */' in eachLine):
			print >> file_out, '    /* %s */' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_INTERFACE,           %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_SYS_GLOBAL,          %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_SYS_SPEC,            %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_LOG_GLOBAL,          %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_CHASSIS,             %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, '    _cds_register_subscribe_pm(TBL_CHIP,                %s);' % (pm_info.NEW_PM_INFO[0])
			print >> file_out, ''
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_gencdb_const_py(filepath):
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	in_PM_ARRAY = 0
	in_DEST_PM_ARRAY = 0
	for line_no, eachLine in enumerate(file_in):
		if ("PM_ARRAY = ['all'," in eachLine):
			in_PM_ARRAY = 1
		if ('DEST_PM_ARRAY' in eachLine):
			in_DEST_PM_ARRAY = 1
		if (in_PM_ARRAY):
			if ("]" in eachLine):
				in_PM_ARRAY = 0
				print >> file_out, "            '%s'," % (pm_info.NEW_PM_INFO[0][6:].lower())

		if (in_DEST_PM_ARRAY):
			if ("PM_ID_MAX" in eachLine):
				in_DEST_PM_ARRAY = 0
				print >> file_out, "        ['%s', '%s']," % (pm_info.NEW_PM_INFO[0][6:].lower(), pm_info.NEW_PM_INFO[0])

		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def create_pm_files():
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	cmd_line = "mkdir -p %sproto/%s/include/" % (CONST.PATH_CNOS_SYSTEM, new_pm_lower)
	os.system(cmd_line)
	cmd_line = "mkdir -p %sproto/%s/src/" % (CONST.PATH_CNOS_SYSTEM, new_pm_lower)
	os.system(cmd_line)

	filepath_h = "%sproto/%s/include/%s.h" % (CONST.PATH_CNOS_SYSTEM, new_pm_lower, new_pm_lower)
	filepath_main_c = "%sproto/%s/src/%s_main.c" % (CONST.PATH_CNOS_SYSTEM, new_pm_lower, new_pm_lower)
	filepath_makefile = "%sproto/%s/Makefile" % (CONST.PATH_CNOS_SYSTEM, new_pm_lower)

	# write xxx.h
	file_h = open(filepath_h, 'w+')
	new_pm = pm_info.NEW_PM_INFO[0][6:]
	print >> file_h, ""
	print >> file_h, "#ifndef __%s_H__" % (new_pm)
	print >> file_h, "#define __%s_H__" % (new_pm)
	print >> file_h, ""
	print >> file_h, "#endif /* !__%s_H__ */" % (new_pm)
	print >> file_h, ""
	file_h.close()

	# write xxx_main.c
	file_in = open(CONST.FILE_XXX_MAIN_C, 'r')
	file_out = open(filepath_main_c, 'w+')
	for line_no, eachLine in enumerate(file_in):
		tmpLine = eachLine.replace('XXX', new_pm_lower)
		newLine = tmpLine.replace('PM_ID_', pm_info.NEW_PM_INFO[0])
		file_out.write(newLine)
	file_in.close()
	file_out.close()

	# write Makefile
	file_in = open(CONST.FILE_XXX_MAKEFILE, 'r')
	file_out = open(filepath_makefile, 'w+')
	for line_no, eachLine in enumerate(file_in):
		newLine = eachLine.replace('XXX', new_pm_lower)
		file_out.write(newLine)
	file_in.close()
	file_out.close()

def update_proto_makefile(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (new_pm_lower in eachLine):
			need_update = 0
		if ("all_targets += ptp" in eachLine):
			print >> file_out, "all_targets += %s" % (new_pm_lower)
		if ("ctl: $(pm_depends) FORCE" in eachLine):
			print >> file_out, "%s: $(pm_depends) FORCE" % (new_pm_lower)
			print >> file_out, "	$(OSP_MAKE) -C %s -f Makefile" % (new_pm_lower)
			print >> file_out, ""
			print >> file_out, "clean_%s: $(pm_depends) FORCE" % (new_pm_lower)
			print >> file_out, "	$(OSP_MAKE) -C clean_%s -f Makefile" % (new_pm_lower)
			print >> file_out, ""
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

def update_startup_script(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (new_pm_lower in eachLine):
			need_update = 0
		if ("OSPDIR/sbin/dhcrelay" in eachLine):
			print >> file_out, "$OSPDIR/sbin/%s &" % (new_pm_lower)
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

def update_build_ppc_script(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if ("pm_list=" in eachLine):
			newLine = eachLine.replace('chvrf', new_pm_lower + ' chvrf')
			file_out.write(newLine)
		else:
			file_out.write(eachLine)
	file_in.close()
	file_out.close()
	cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
	os.system(cmd_line)
	cmd_line = "chmod 755 %s" % (filepath)
	os.system(cmd_line)

def update_rcinit_script(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (new_pm_lower in eachLine):
			need_update = 0
		if ("authd &" in eachLine):
			print >> file_out, "%s &" % (new_pm_lower)
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

def update_mibd_cdb_const_py(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	need_add_one = 0
	case_no = 0
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (need_add_one):
			newLine = one_digital_add_one(eachLine)
			file_out.write(newLine)
		else:
			file_out.write(eachLine)
		if (pm_info.NEW_PM_INFO[0] in eachLine):
			need_update = 0
		if ("PM_ID_" in eachLine):
			case_no += 1
		if (case_no == pm_info.NEW_PM_INFO[1]):
			print >> file_out, "%-19s = 16" % (pm_info.NEW_PM_INFO[0])
			need_add_one = 1
		if (need_add_one):
			if ("PM_ID_MAX" in eachLine):
				need_add_one = 0
	file_in.close()
	file_out.close()
	
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

def update_sys_cfg_xml(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	match_show_common = 0
	match_show_memory = 0
	write_show_common = 0
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (new_pm_lower in eachLine):
			need_update = 0
		if ("cdbctl show/cdb/sys/show_common/" in eachLine):
			match_show_common = 1
		if (match_show_common == 1):
			if ("</ACTION>" in eachLine):
				if (write_show_common == 0):
					match_show_common = 0
					write_show_common = 1
					print >> file_out, "            cdbctl show/cdb/sys/show_common/%s/memory" % (new_pm_lower)

		if ('<COMMAND name="show memory"' in eachLine):
			match_show_memory = 1
		if (match_show_memory == 1):
			if ("Summary memory information" in eachLine):
				match_show_memory = 0
				print >> file_out, '            <PARAM name="%s" help="%s" ptype="SUBCOMMAND" mode="subcommand" />' % (new_pm_lower, pm_info.NEW_PM_INFO[2])
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

def update_int_debug_xml(filepath):
	new_pm_lower = pm_info.NEW_PM_INFO[0][6:].lower()
	need_update = 1
	match_show_common = 0
	match_show_socket = 0
	write_show_common = 0
	if (False == os.path.exists(filepath)):
		return
	
	file_in = open(filepath, 'r')
	file_out = open(filepath + '.tmp', 'w+')
	code_util.debug_out(1, "Update %s " % (filepath))
	for line_no, eachLine in enumerate(file_in):
		if (new_pm_lower in eachLine):
			need_update = 0
		if ("cdbctl show/cdb/sys/show_common/" in eachLine):
			match_show_common = 1
		if (match_show_common == 1):
			if ("</ACTION>" in eachLine):
				if (write_show_common == 0):
					match_show_common = 0
					write_show_common = 1
					print >> file_out, "            cdbctl show/cdb/sys/show_common/%s/socket/${detail}" % (new_pm_lower)

		if ('<COMMAND name="show socket"' in eachLine):
			match_show_socket = 1
		if (match_show_socket == 1):
			if ("</PARAM>" in eachLine):
				match_show_socket = 0
				print >> file_out, '            <PARAM name="%s" help="%s" ptype="SUBCOMMAND" mode="subcommand" />' % (new_pm_lower, pm_info.NEW_PM_INFO[2])
		file_out.write(eachLine)
	file_in.close()
	file_out.close()
	
	if (need_update):
		cmd_line = "mv %s %s" % (filepath + '.tmp', filepath)
		os.system(cmd_line)
		cmd_line = "chmod 755 %s" % (filepath)
		os.system(cmd_line)

print "1. Add Macro %s" % (pm_info.NEW_PM_INFO[0])
update_ctc_msg_h(CONST.FILE_CTC_MSG_H)
update_ctc_msg_c(CONST.FILE_CTC_MSG_C)
update_proto_module_h(CONST.FILE_PROTO_MODULE_H)
update_proto_module_c(CONST.FILE_PROTO_MODULE_C)

print "2. Add PM files %s" % (pm_info.NEW_PM_INFO[0])
create_pm_files()
update_proto_makefile(CONST.FILE_PROTO_MAKEFILE)

print "3. Add startup script %s" % (pm_info.NEW_PM_INFO[0])
update_startup_script(CONST.FILE_UML_START)
update_startup_script(CONST.FILE_UML_NEW_START)
update_build_ppc_script(CONST.FILE_BUILD_PPC)
update_rcinit_script(CONST.FILE_RCINIT_E580)
update_rcinit_script(CONST.FILE_RCINIT_E350)

print "4. Add startup script %s" % (pm_info.NEW_PM_INFO[0])
update_sys_cfg_xml(CONST.FILE_SYS_CFG_XML)
update_int_debug_xml(CONST.FILE_INT_DEBUG_XML)

print "5. cds.c add %s" % (pm_info.NEW_PM_INFO[0])
update_cds_c(CONST.FILE_CDS_C)

print "6. gencdb CONST.py add %s" % (pm_info.NEW_PM_INFO[0])
update_gencdb_const_py(CONST.FILE_CONST_PY)

print "7. mibd cdb_const.py add %s" % (pm_info.NEW_PM_INFO[0])
update_mibd_cdb_const_py(CONST.FILE_CDB_CONST_PY)
