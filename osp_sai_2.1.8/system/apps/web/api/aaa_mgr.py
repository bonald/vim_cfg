#!/usr/bin/python
#xx
#-*- coding: utf-8 -*-

from vcl import vcmd
import re

def aaa_status_get():
    """
    TBD
    """
    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/sys_global/aaa_new_model/')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        obj['aaa_new_model']= arr[0].get('aaa_new_model')

    return obj


def aaa_status_set(enable):
    """
    TBD
    """

    _err_reason = [
            '',                 # err_code: 0
            'bad request',      # err_code: 1
            'cmd excute error', # err_code: 2
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    # excute
    status, lines = vcmd.get_status_lines('cdbctl update/cdb/sys/sys_global/aaa_new_model/%d' % int(enable))

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    if enable == '1':
        for i in range(0, 8):
            status, lines = vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/auth_method' % int(i))
            if lines == "/auth_method=":
                vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/auth_method/default' % int(i))

            status, lines = vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/author_method' % int(i))
            if lines == "/author_method=":
                vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/author_method/default' % int(i))

            status, lines = vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/account_method' % int(i))
            if lines == "/account_method=":
                vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/account_method/default' % int(i))

            status, lines = vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/accountcmd_method' % int(i))
            if lines == "/accountcmd_method=":
                vcmd.get_status_lines('cdbctl read/cdb/app/vty/%d/accountcmd_method/default' % int(i))

    return obj


def aaa_method_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    type = str(req_data.get('type'))
    name = str(req_data.get('name'))
    method1 = str(req_data.get('method1'))
    method2 = str(req_data.get('method2'))
    method3 = str(req_data.get('method3'))
    method4 = str(req_data.get('method4'))
    method5 = str(req_data.get('method5'))
    method6 = str(req_data.get('method6'))
    method7 = str(req_data.get('method7'))
    method8 = str(req_data.get('method8'))
    method5before = str(req_data.get('method5before'))
    method_list = method1 + ',' + method2
    author_method_list = method3 + ',' + method4
    account_method_list  = method5 + ',' + method6
    accountcmd_method_list  = method7 + ',' + method8
    account_method_list_before = method5before
    if account_method_list_before == 'start-stop':
        account_method_before = 'start_stop/1/stop_only/0'
    if account_method_list_before == 'stop-only':
        account_method_before = 'stop_only/1/start_stop/0'
    if account_method_list_before == 'none':
        account_method_before ='stop_only/0/start_stop/0'
        account_method_list='none'
    # 1> authen
    if type == 'AUTHEN_LOGIN':

        cmd = 'cdbctl update/cdb/app/authen/%s/methods/%s'%(name ,method_list)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:

            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj

    # 2> author
    if type == 'AAA_ML_AUTHOR_SHELL':

        cmd = 'cdbctl update/cdb/app/author/%s/methods/%s' %(name , author_method_list)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:

            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj    
    # 3> account
    if type == 'AAA_ML_ACCT_SHELL':

        cmd = 'cdbctl update/cdb/app/account/%s/%s/methods/%s' % (name, account_method_before, account_method_list)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj
    # 4> accountcmd
    if type == 'AAA_ML_ACCT_COMMAND':

        cmd = 'cdbctl update/cdb/app/accountcmd/%s/methods/%s' % (name, accountcmd_method_list)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj

    return obj


def aaa_method_get():
    """
    TBD
    """

    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'method_list': [], 'author_method_list': [], 'account_method_list': [], 'accountcmd_method_list': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    status, arr = vcmd.get_status_arr('cdbctl read/cdb/sys/sys_global/aaa_new_model/')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj
    else:
        aaa_enable = arr[0].get('aaa_new_model')
        if aaa_enable == "0":
            return obj

    # 1> authen
    status, arr = vcmd.get_status_lines('cdbctl show/cdb/app/authen')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        subarr = arr[1:]

        for str in subarr:
            temp = {}
            temp['type'] = 'AUTHEN_LOGIN'
            temp['method'] = re.findall(r":\s(.+?)$", str)[0]
            temp['name'] = re.findall(r"^Name\s=\s(.+?)\s", str)[0]
            temp['state'] = re.findall(r"state\s=\s(.+?):", str)[0].lower()
            obj['method_list'].append(temp)


    # 2> author
    status, arr = vcmd.get_status_lines('cdbctl show/cdb/app/author')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        subarr = arr[1:]

        for str in subarr:
            temp = {}
            temp['type'] = 'AAA_ML_AUTHOR_SHELL'
            temp['method'] = re.findall(r":\s(.+?)$", str)[0]
            temp['name'] = re.findall(r"^Name\s=\s(.+?)\s", str)[0]
            temp['state'] = re.findall(r"state\s=\s(.+?):", str)[0].lower()
            obj['author_method_list'].append(temp)

    # not support now, realize it later
    # 3> account
    status, arr = vcmd.get_status_lines('cdbctl show/cdb/app/account')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        subarr = arr[1:]

        for str in subarr:
            temp = {}
            temp['type'] = 'AAA_ML_ACCT_SHELL'
            temp['method'] = re.findall(r":\s(.+?)$", str)[0]
            temp['name'] = re.findall(r"^Name\s=\s(.+?)\s", str)[0]
            temp['state'] = re.findall(r"state\s=\s(.+?):", str)[0].lower()
            obj['account_method_list'].append(temp)
    # 4> accountcmd
    status, arr = vcmd.get_status_lines('cdbctl show/cdb/app/accountcmd')

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(arr)
        return obj

    else:
        subarr = arr[1:]

        for str in subarr:
            temp = {}
            temp['type'] = 'AAA_ML_ACCT_COMMAND'
            temp['method'] = re.findall(r":\s(.+?)$", str)[0]
            temp['name'] = re.findall(r"^Name\s=\s(.+?)\s", str)[0]
            temp['state'] = re.findall(r"state\s=\s(.+?):", str)[0].lower()
            obj['accountcmd_method_list'].append(temp)

    return obj


def aaa_method_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',             # err_code: 0
            'log is null',  # err_code: 1
            ]

    obj = {'method_list': [], 'author_method_list': [], 'account_method_list': [], 'accountcmd_method_list': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    type = str(req_data.get('type'))

    name = str(req_data.get('name'))

    # 1> authen
    if type == 'AUTHEN_LOGIN':

        if name == 'default':
            cmd = 'cdbctl update/cdb/app/authen/default/methods/local'
        else:
            cmd = 'cdbctl delete/cdb/app/authen/%s' % str(name)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:

            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj

    # 2> author
    if type == 'AAA_ML_AUTHOR_SHELL':

        if name == 'default':
            cmd = 'cdbctl update/cdb/app/author/default/methods/local'
        else:
            cmd = 'cdbctl delete/cdb/app/author/%s' % str(name)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:

            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj    
    # 3> account
    if type == 'AAA_ML_ACCT_SHELL':

        if name == 'default':
            cmd = 'cdbctl update/cdb/app/account/default/stop_only/0/start_stop/0/methods/none'
        else:
            cmd = 'cdbctl delete/cdb/app/account/%s' % str(name)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj
    # 4> accountcmd
    if type == 'AAA_ML_ACCT_COMMAND':

        if name == 'default':
            cmd = 'cdbctl update/cdb/app/accountcmd/default/methods/none'
        else:
            cmd = 'cdbctl delete/cdb/app/accountcmd/%s' % str(name)

        status, arr = vcmd.get_status_lines(cmd)

        if status != 0:
            obj['error'] = True
            obj['err_code'] = 2
            obj['err_reason'] = '; '.join(arr)
            return obj

    return obj


if __name__ == '__main__':
    pass
