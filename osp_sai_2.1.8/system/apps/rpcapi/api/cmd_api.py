#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import local_cli
from vcl import config
import re, string, copy, os

"""
"""

unsupport_rpc_string = {'service rpc-api':'% do not support RPC-API CLI by RPC',
                        'ssh':'% do not support ssh CLI by RPC',
                        'telnet':'% do not support telnet CLI by RPC',
                        'source':'% do not support source CLI by RPC',
                        'ovs-ofctl snoop':'%do not support ovs-ofctl snoop CLI by RPC',
                        'start sh':'% do not support shell CLI by RPC',
                        'reboot':'% do not support reboot CLI by RPC',
                        'reload':'% do not support reload CLI by RPC',
                        'format':'% do not support format CLI by RPC'}

unsupport_rpc_cmd_combination = {'copy flash.* running-config':'% do not support the CLI by RPC'}

unsupport_rpc_char = ['?']
unsupport_rpc_char_info = {'?':'% do not support ? in CLI by RPC'}

special_rpc_cli = ['hostname']
users_list = []
auth_mode = 0
debug_request = 0
debug_command = 0
counter_request_execute = 0
counter_request_deny = 0
debug_command = 0

try:
    cli = local_cli.Clilib()
except:
    pass
    
obj = {'cmd':'', 'sequence': 0, 'error': False, 'err_code': 0, 'err_reason': '', 'sourceDetails': ''}
res_list = []
for i in range(1000):
    res_list.append(copy.deepcopy(obj))

COUNT_TYPE_EXECUTE = 0
COUNT_TYPE_DENY    = 1

def counter_request(type):
    global counter_request_execute
    global counter_request_deny
    # try to read counter before inc counters
    try:              
        fobj = open("/tmp/rpcapi_counter", "r")
        countstr = fobj.read()
        [execute, deny] = countstr.split(' ')
        counter_request_execute = int(execute)
        counter_request_deny = int(deny)                           
        fobj.close()                                                  
    except IOError:                                                   
        debugstr = '0'
    # inc counters
    if (COUNT_TYPE_EXECUTE == type):
        counter_request_execute += 1
    else:
        counter_request_deny += 1
    # write
    try:                  
        fobj = open("/tmp/rpcapi_counter", "w+")
        print >> fobj, "%u %u" % (counter_request_execute, counter_request_deny)
        fobj.close()
    except IOError:           
        debugstr = '0'   
    # print "exec %u, deny %u" % (counter_request_execute, counter_request_deny)

def execute_cmd(cmdline):
    return os.popen(cmdline).read().strip("\r\n")
                                              
def GetUsers(int_auth_mode):
    global users_list
    global auth_mode
    auth_mode = int_auth_mode
    szmsg = execute_cmd('cdbctl read/cdb/app/get_auth')
    users_str = szmsg.split('\n')                  
    if 0 == len(szmsg):
        return
    for i, user_str in enumerate(users_str):
        useritem = user_str.split("/")
        user = [useritem[0], useritem[1], useritem[2]]
        users_list.append(user)                                         

def StoreUserAndIP(username="", addr="RPC"):
	str = 'rpcapi_user ' + addr + '/' + username
	cli.cmd(str)

def CheckUsers(auth):
    global users_list
    global auth_mode
    if auth_mode == 0:
        StoreUserAndIP()
        return config.RPC_ERROR_SUCCESS
    if auth == None:                                                                                       
        return config.RPC_ERROR_CLI_AUTH_FAIL    
    __username = auth['username']
    __password = auth['password']
    for i, user_str in enumerate(users_list):
        if __username == user_str[0]:
            if __password == user_str[1]:
                if '4' == user_str[2]:
                    StoreUserAndIP(__username)
                    return config.RPC_ERROR_SUCCESS
                else:
                    return config.RPC_ERROR_CLI_AUTH_LOW
    return config.RPC_ERROR_CLI_AUTH_FAIL

#check rpc format and version
def rpc_msg_un_match(format, version):
    error = False
    err_code = 0
    res = ''
    if format != 'json' or version != 1:
        error = True
        err_code = config.RPC_ERROR_MESSAGE_NOT_SUPPORT
        res = '% not supported RPC format or version'
    return error,res,err_code

#limit some cli from RPC
def rpc_cmd_un_support(cmd):
    error = False
    err_code = 0
    res = ''

    if cmd == 'no' or cmd == 'n':
        mode = cli.get_mode()
        if mode == 0:
            res = ' ' + cmd + '\r\n' + 'Limited CLI string in EXEC mode'
            err_code = config.RPC_ERROR_CLI_NOT_SUPPORT
            error = True
    if not error:
        for char in unsupport_rpc_char:
            if cmd.find(char) >= 0:
                res = ' ' + cmd + '\r\n' + unsupport_rpc_char_info[char]
                err_code = config.RPC_ERROR_CHAR_NOT_SUPPORT
                error = True
                break
    if not error:
        for str1 in unsupport_rpc_string:
            if cmd.find(str1) == 0:
                res = ' ' + cmd + '\r\n' + unsupport_rpc_string[str1]
                err_code = config.RPC_ERROR_STRING_NOT_SUPPORT
                error = True
                break
    if not error:
        for str1 in unsupport_rpc_cmd_combination:
            if re.match(str1,cmd):
                res = ' ' + cmd + '\r\n' + unsupport_rpc_cmd_combination[str1]
                err_code = config.RPC_ERROR_STRING_NOT_SUPPORT
                error = True
                break

    return error,res,err_code

def rpc_cmd_is_special(cmd):
    for str1 in special_rpc_cli:
        if cmd.find(str1) == 0:
            return True
        else:
            return False

#check rpc cli response error
def rpc_error_found(res):
    error = False
    err_code = 0
    resp = res
    if res.find('\n% ') >= 0 and res.find('% Aborted') < 0:
        err_code = config.RPC_ERROR_CLI_FAIL
        resp = res
        error = True
    if res.find('% Timeout for cli response') >= 0:
        err_code = config.RPC_ERROR_CLI_TIMEOUT
        resp = res
        error = True
    return error,resp,err_code

def rpc_response_process(resp):
    res = string.replace(resp, chr(1), ' ')
    res = string.replace(res, chr(7), ' ')
    return res

def get_rpc_debug():                                                  
    global debug_request                                              
    global debug_command
    debug_request = 0                                                                            
    debug_command = 0
    try:                  
        fobj = open("/tmp/rpcapi_debug")                          
        debugstr = fobj.read()           
        fobj.close()          
    except IOError:           
        debugstr = '0'
    if ('3' == debugstr):                   
        debug_request = 1                   
        debug_command = 1   
    elif ('2' == debugstr):
        debug_command = 1  
    elif ('1' == debugstr):
        debug_request = 1                                                                            
    return

def rpc_debug(debugstr):
    global debug_request
    if (debug_request):
        cmd = 'logger -t RPC -p 7 -s %s' % (debugstr)                                                
        os.system(cmd)

def rpc_debug_cmd(debugstr, retcode):
    global debug_command
    if (debug_command):
        if (0 == retcode):
            cmd = 'logger -t RPC -p 7 -s "%s return success"' % (debugstr)
        else:
            cmd = 'logger -t RPC -p 7 -s "%s return error"' % (debugstr)
        os.system(cmd)

#RCP API commands
def command(req_data, auth, request):
    global debug_request
    global debug_command
    global res_list
    obj0 = res_list[0]
    obj0['error'] = False
    obj0['err_code'] = 0
    obj0['err_reason'] = ''
    obj0['sourceDetails'] = ''
    item = {'0':obj0}
    result = {'result':[item]}

    get_rpc_debug()
    if (debug_request or debug_command):
        rip = request.remote_addr                                                                        
        rport = request.environ.get('REMOTE_PORT')
        getrip_cmd = "getrip %s %s" % (rip, rport)
        cmd = os.popen(getrip_cmd)         
        rip = cmd.read()
        debugstr = 'RPC connect from %s:%s' % (rip, rport)
        rpc_debug(debugstr)

    check_ret = CheckUsers(auth)
    if (config.RPC_ERROR_SUCCESS != check_ret):
        obj0['sourceDetails'] = ''
        obj0['error'] = True
        item['0'] = obj0
        if (config.RPC_ERROR_CLI_AUTH_FAIL == check_ret):
            obj0['err_code'] = config.RPC_ERROR_CLI_AUTH_FAIL
            obj0['err_reason'] = '% user authentication failed'
        else:
            obj0['err_code'] = config.RPC_ERROR_CLI_AUTH_LOW
            obj0['err_reason'] = '% user privilege is low'
        result['result'] = [item]
		
        if (debug_request):
            debugstr = 'RPC from %s:%s is rejected, because of user authentication failed' % (rip, rport)
            rpc_debug(debugstr)
        counter_request(COUNT_TYPE_DENY)
        return result

    # get param
    data = req_data.get('params')
    # bug52785 for wrong json format parse failed not return Internal Server Error
    if (None == data):
        obj0['error'] = True
        obj0['err_code'] = config.RPC_ERROR_MESSAGE_NOT_SUPPORT
        obj0['err_reason'] = '% not supported RPC format or version'
        item['0'] = obj0
        result['result'] = [item]
        if (debug_request):
            debugstr = 'RPC from %s:%s is rejected for format or version' % (rip, rport)
            rpc_debug(debugstr)
        counter_request(COUNT_TYPE_DENY)
        return result
    # support data is a list
    if (type(data) is list):                                                                             
        data = data[0]   
    format = data.get('format')
    version = data.get('version')

    error, res, err_code = rpc_msg_un_match(format, version)
    if error:
        obj0['error'] = True
        obj0['err_code'] = err_code
        obj0['err_reason'] = res
        item['0'] = obj0
        result['result'] = [item]
        if (debug_request):
            debugstr = 'RPC from %s:%s is rejected for format or version' % (rip, rport)
            rpc_debug(debugstr)
        counter_request(COUNT_TYPE_DENY)
        return result

    cmds = data.get('cmds')
    if (None == cmds):
        if (debug_request):
            debugstr = 'RPC has no cmds information'
            rpc_debug(debugstr)
    if (type(cmds) is not list):
        obj0['error'] = True
        obj0['err_code'] = config.RPC_ERROR_MESSAGE_NOT_SUPPORT
        obj0['err_reason'] = '% not supported RPC format or version'
        item['0'] = obj0
        result['result'] = [item]
        if (debug_request):
            debugstr = 'RPC from %s:%s is rejected for format or version' % (rip, rport)
            rpc_debug(debugstr)
        counter_request(COUNT_TYPE_DENY)
        return result
    i = 0
    for cmd in cmds:
        error = False
        obj = res_list[i]
        obj['error'] = False
        obj['err_code'] = 0
        obj['err_reason'] = ''

        cmd = cmd.strip()
        if not cli.is_wait_input():
            error, res, err_code = rpc_cmd_un_support(cmd)
            if error:
                obj['error'] = True
                obj['err_code'] = err_code
                obj['err_reason'] = res

        if not error:
            if rpc_cmd_is_special(cmd):
                resp = cli.easy_cmd(cmd)
            else:
                resp = cli.cmd(cmd)
            res = rpc_response_process(resp)
            error, res, err_code = rpc_error_found(res)
            if (debug_command):
                debugstr = 'IP %s, send command : %s,' % (rip, cmd)
                rpc_debug_cmd(debugstr, err_code)
            if error:
                obj['error'] = True
                obj['err_code'] = err_code
                obj['err_reason'] = res

        obj['sequence'] = i
        obj['cmd'] = cmd
        obj['sourceDetails'] = res

        item[str(i)] = obj
        i = i + 1

    result['result'] = [item]

    if (debug_request):
        debugstr = 'RPC disconnect from %s:%s' % (rip, rport)
        rpc_debug(debugstr)
    counter_request(COUNT_TYPE_EXECUTE)
    return result

