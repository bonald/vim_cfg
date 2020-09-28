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
