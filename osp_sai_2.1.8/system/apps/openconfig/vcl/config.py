#!/usr/bin/python
#-*- coding: utf-8 -*-

is_dev = False
remote = '10.10.39.106'
dut_regexp = r'DUT\d#'
dut_stdout_pwd_info = r'Password:'
shell_pwd = '!@#'
cmd_regexp = r'\[\w+@.+\]\$'

login_stdout_username_info = r'Username:'
login_stdout_pwd_info = r'Password:'
login_username = 'admin'
login_pwd = 'admin'

RPC_ERROR_CLI_TIMEOUT = -1000
RPC_ERROR_CLI_FAIL = -1001
RPC_ERROR_CLI_NOT_SUPPORT = -1002
RPC_ERROR_CHAR_NOT_SUPPORT = -1003
RPC_ERROR_STRING_NOT_SUPPORT = -1004
RPC_ERROR_MESSAGE_NOT_SUPPORT = -1005
