#!/usr/bin/python
import os
import sys
import string
import shutil
import CONST
import cdb_util

trace_on = 0
if (len(sys.argv) > 1):
	if ('-trace' == sys.argv[1]):
		trace_on = 1

filename_input_log	= CONST.CDB_INPUT_PATH + 'log.txt'
filename_id 		= 'genlog_id.h'
filename_id_priv	= 'genlog_id_priv.h'

SEVERITY_ARRAY = ['E_EMERGENCY', 'E_ALERT', 'E_CRITICAL', 'E_ERROR', 'E_WARNING', 'E_NOTICE', 'E_INFORMATIONAL', 'E_DEBUG']

module_array = []
item_array = []

def is_module_match(module_array, module_name):
	for i, module_item in enumerate(module_array):
		if (module_name == module_item):
			return 1
	return 0

def get_params_info(params, log_param_array):
	for i, params_item in enumerate(log_param_array):
		if (params == params_item[0]):
			return params_item
	return None

def convert_params_array(params_code_array, params_array, log_param_array, log_sentence):
	for i, params in enumerate(params_array):
		params_info = get_params_info(params, log_param_array)
		if (None == params_info):
			cdb_util.error_return("cannot found params %s for sentence %s", params, log_sentence)
		params_code_array.append(params_info[1])

def gen_log_sentence_code(log_sentence, log_param_array):
	tmp_sentence = log_sentence.split('[')
	params_count = len(tmp_sentence) - 1
	params_count2 = len(log_param_array)
	if (params_count != params_count2):
		cdb_util.error_return("log sentence params number mismatch: %s != %s, log_sentence %s", params_count, params_count2, log_sentence)
	if (0 == params_count):
		return log_sentence
	sentence_array = []
	params_array = []
	params_code_array = []
	for i, entity in enumerate(tmp_sentence):
		idx = entity.find(']')
		if (idx >= 0):
			tmp_entity = entity.split(']')
			if (2 != len(tmp_entity)):
				cdb_util.error_return("entity should has 2 items: %s", tmp_entity)
			params_array.append(tmp_entity[0])
			sentence_array.append(tmp_entity[1])
		else:
			sentence_array.append(entity)
	convert_params_array(params_code_array, params_array, log_param_array, log_sentence)
	
	len_sentence_array = len(sentence_array)
	len_params_code_array = len(params_code_array)
	#print "params: %d %d" % (len_sentence_array, len_params_code_array)
	if (len_sentence_array == len_params_code_array + 1):
		log_sentence_code = sentence_array[0]
		for i, params in enumerate(params_code_array):
			log_sentence_code += params_code_array[i]
			log_sentence_code += sentence_array[i+1]
	elif (len_sentence_array == len_params_code_array):
		for i, params in enumerate(params_code_array):
			log_sentence_code += params_code_array[i]
			log_sentence_code += sentence_array[i]
	else:
		cdb_util.error_return("log sentence invalid for params %s", log_sentence)
	return log_sentence_code

def parse_input_log_file(module_array, item_array, input_file):
	'Parse input log.txt'
	filename_input = 'log.txt'
	file_in = open(input_file, 'r')
	log_item = []
	has_log_item_id = 0
	line_no = 0
	module_not_sorted = 0
	log_id = ""
	log_sentence = ""
	log_sentence_code = ""
	log_explanation = ""
	log_action = ""
	log_param_array = []
	line_type = CONST.LOG_LINE_INVALID
	for line_no, eachLine in enumerate(file_in):
		if (eachLine[:12] == "[LOG-MODULE]"):
			line_type = CONST.LOG_LINE_MODULE
			continue
		elif (eachLine[:10] == "[LOG-ITEM]"):
			line_type = CONST.LOG_ITEM_ID
			if (0 == has_log_item_id):
				has_log_item_id = 1
				module_array_sorted = module_array[:]
				module_array_sorted.sort()
				for i, module_item in enumerate(module_array_sorted):
					if (module_array[i] != module_array_sorted[i]):
						print "Warning: Module[%u] %s is not sorted" % (i, module_array[i])
						module_not_sorted = 1
				if (module_not_sorted):
					print "Sorted Module List is:"
					for i, module_item in enumerate(module_array_sorted):
						print "%s" % (module_item)						
			continue
		elif (eachLine[:14] == "[LOG-SENTENCE]"):
			line_type = CONST.LOG_ITEM_SENTENCE
			continue
		elif (eachLine[:11] == "[LOG-PARAM]"):
			line_type = CONST.LOG_ITEM_PARAM
			continue
		elif (eachLine[:17] == "[LOG-EXPLANATION]"):
			line_type = CONST.LOG_ITEM_EXPLANATION
			continue
		elif (eachLine[:24] == "[LOG-RECOMMENDED_ACTION]"):
			line_type = CONST.LOG_ITEM_ACTION
			continue
		elif (eachLine[:14] == "[LOG-PARAM-CN]"):
			line_type = CONST.LOG_ITEM_PARAM_CN
			continue
		elif (eachLine[:20] == "[LOG-EXPLANATION-CN]"):
			line_type = CONST.LOG_ITEM_EXPLANATION_CN
			continue
		elif (eachLine[:27] == "[LOG-RECOMMENDED_ACTION-CN]"):
			line_type = CONST.LOG_ITEM_ACTION_CN
			continue
		elif (eachLine[0] == '#'):
			continue
		elif (eachLine[0] == '-'):
			if (CONST.LOG_LINE_INVALID == line_type):
				continue
			elif (CONST.LOG_LINE_MODULE == line_type):
				line_type = CONST.LOG_ITEM_END
			elif (CONST.LOG_ITEM_ACTION_CN == line_type):
				line_type = CONST.LOG_ITEM_END
			else:
				cdb_util.error_return("FILE %s, LINE %s: must has MODULE or RECOMMENDED_ACTION_CN field before %s", filename_input, line_no+1, eachLine)
		else:
			if (CONST.LOG_LINE_INVALID == line_type):
				cdb_util.error_return("FILE %s, LINE %s: has invalid type: %s", filename_input, line_no+1, eachLine)

		if (CONST.LOG_LINE_MODULE == line_type):
			entity = eachLine.split()
			if (0 == len(entity)):
				continue
			if (1 != len(entity)):
				cdb_util.error_return("FILE %s, LINE %s item num != 1, invalid line: %s", filename_input, line_no+1, eachLine)
			module_array.append(entity[0])
			#print "module_array %s" % (entity[0])
		elif (CONST.LOG_ITEM_ID == line_type):
			entity = eachLine.split()
			if (0 == len(entity)):
				continue
			if (1 != len(entity)):
				cdb_util.error_return("FILE %s, LINE %s item num != 1, invalid line: %s", filename_input, line_no+1, eachLine)
			log_id = entity[0].split('-')
			# check '-' number
			if (3 != len(log_id)):
				cdb_util.error_return("FILE %s, LINE %s log_id should has 3 items, invalid line: %s", filename_input, line_no+1, eachLine)
			# check module
			module_match = is_module_match(module_array, log_id[0])
			if (0 == module_match):
				cdb_util.error_return("FILE %s, LINE %s log_id not found matched log module, invalid line: %s", filename_input, line_no+1, eachLine)
			# check brief length < 48
			if (48 <= len(log_id[2])):
				cdb_util.error_return("FILE %s, LINE %s log brief should less than 48 characters, invalid line: %s", filename_input, line_no+1, eachLine)
			# gen log_id_enum
			log_id_enum = 'LOG_' + log_id[0] + '_' + log_id[1] + '_' + log_id[2]
			# convert severity to int
			severity = int(log_id[1])
			if (severity >= 0 and severity <= 7):
				pass
			else:
				cdb_util.error_return("FILE %s, LINE %s severity should be [0-7], invalid line: %s", filename_input, line_no+1, eachLine)
			log_id[1] = severity
			log_id.append(log_id_enum)
			#print "log_id %s" % (log_id)
		elif (CONST.LOG_ITEM_SENTENCE == line_type):
			entity = eachLine.split()
			if (0 == len(entity)):
				cdb_util.error_return("FILE %s, LINE %s should not be empty", filename_input, line_no+1)
			log_sentence = eachLine.strip()
			#print "log_sentence %s" % (eachLine)
		elif (CONST.LOG_ITEM_PARAM == line_type):
			entity = eachLine.split(None, 2)
			if (0 == len(entity)):
				continue
			if (3 != len(entity)):
				cdb_util.error_return("FILE %s, LINE %s item num != 3, invalid line: %s", filename_input, line_no+1, eachLine)
			entity[2] = entity[2].strip()
			log_param_array.append(entity)
			#print "log_param_array %s" % (entity)
		elif (CONST.LOG_ITEM_EXPLANATION == line_type):
			entity = eachLine.split()
			if (0 == len(entity)):
				cdb_util.error_return("FILE %s, LINE %s should not be empty", filename_input, line_no+1)
			log_explanation = eachLine
			#print "log_explanation %s" % (log_explanation)
		elif (CONST.LOG_ITEM_ACTION == line_type):
			entity = eachLine.split()
			if (0 == len(entity)):
				cdb_util.error_return("FILE %s, LINE %s should not be empty", filename_input, line_no+1)
			log_action = eachLine
			#print "log_action %s" % (log_action)
		elif (CONST.LOG_ITEM_END == line_type):
			if (0 == len(log_id)):
				continue
			log_sentence_code = gen_log_sentence_code(log_sentence, log_param_array)
			log_item.append(log_id)
			log_item.append(log_sentence)
			log_item.append(log_sentence_code)
			log_item.append(log_param_array)
			log_item.append(log_explanation)
			log_item.append(log_action)
			item_array.append(log_item)
			log_param_array = []
			log_item = []
	file_in.close()

#########################################################
# 1. parse log.txt
#########################################################
parse_input_log_file(module_array, item_array, filename_input_log)

#########################################################
# 2. build full name
#########################################################

#########################################################
# build genlog_id.h
#########################################################
file_out = open(filename_id, 'w')

_macro_h_ = "__GENLOG_ID_H__"
print >> file_out, ""
print >> file_out, "#ifndef %s" % (_macro_h_)
print >> file_out, "#define %s" % (_macro_h_)
print >> file_out, ""
print >> file_out, "typedef enum"
print >> file_out, "{"
for i, entity in enumerate(item_array):
	log_id = entity[0]
	print >> file_out, "    %-40s = %u," % (log_id[3], i)
print >> file_out, "    LOG_ID_MAX"
print >> file_out, "} E_LOGID;"
print >> file_out, ""
print >> file_out, "#endif /* !%s */" % (_macro_h_)
print >> file_out, ""

file_out.close()
print "generate file %s" % (filename_id)

#########################################################
# build genlog_id_priv.h
#########################################################
file_out = open(filename_id_priv, 'w')

_macro_h_ = "__GENLOG_ID_PRIV_H__"
print >> file_out, ""
print >> file_out, "#ifndef %s" % (_macro_h_)
print >> file_out, "#define %s" % (_macro_h_)
print >> file_out, ""

def build_entity_log_priv(file_out, entity, i):
	log_id = entity[0]
	log_id_enum = log_id[3]
	log_id_str = log_id[0] + '-' + log_id[2]
	log_severity = log_id[1]
	log_sentence_code = entity[2]
	print >> file_out, "    {"
	print >> file_out, "        %s," % (log_id_enum)
	print >> file_out, "        %s," % (SEVERITY_ARRAY[log_severity])
	print >> file_out, "        \"%s\"," % (log_id_str)
	print >> file_out, "        \"%s\"," % (log_sentence_code)
	print >> file_out, "    },"

print >> file_out, "const static osp_log_info_t gLogInfoArr[LOG_ID_MAX] ="
print >> file_out, "{"
for i, entity in enumerate(item_array):
	build_entity_log_priv(file_out, entity, i)
print >> file_out, "};"
print >> file_out, ""
print >> file_out, "#endif /* !%s */" % (_macro_h_)
print >> file_out, ""

file_out.close()
print "generate file %s" % (filename_id_priv)

LOCAL_BASE = './'
GENLOG_LIB_BASE = '../../lib/genlog/'

def gen_copy_one_file(dest_dir, src_dir, name):
	dest_fullname = dest_dir+name
	src_fullname = src_dir+name
	shutil.move(src_fullname, dest_fullname)

gen_copy_one_file(GENLOG_LIB_BASE, LOCAL_BASE, filename_id)
gen_copy_one_file(GENLOG_LIB_BASE, LOCAL_BASE, filename_id_priv)
