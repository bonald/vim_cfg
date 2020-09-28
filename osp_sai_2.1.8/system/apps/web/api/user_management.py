#-*- coding: utf-8 -*-

from flask import url_for
import os
import base
from vcl import vcmd
import types
import cmd

def get(file_type):
    """
    API:
    
    GET:
    
    Retrun: {
        user: [
            {user: string, privilege: number, password: string,},
            ...
        ],
    }
    """

    _err_reason = [
        '',              # err_code: 0
        'cmd excute error',   # err_code: 1
    ]
    
    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/app/user'
    arr = vcmd.get_arr(cmd)
    if arr:
        for entry in arr:
            # key = entry['key']
            # key = key[:(len(key) - 1)]

            # entry.pop('key')
            entry['username'] = entry['key']
            entry['privilege'] = entry['privilege']
            entry['password'] = entry['passwd']

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj


def delete(username):
    """
    API:
    
    DELETE: {
       file_arr: [
            {
               'id': string,
            },
            ...
            ...
       ],
    }
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',              # err_code: 0
        'bad request',   # err_code: 1
        'can not delete factory default user', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    cmd = 'cdbctl delete/cdb/app/user/%s' % username
    res = vcmd.execute(cmd)
    
    if res != 0 :
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
	
    return obj

def set(users):


    username = ''
    password = ''
    privilege = ''
    cmd = ''

    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        'can not update factory default user',  # err_code: 2
    ]

    obj = {'error': False, 'err_code': 0, 'err_reason': ''}

    if "username" in users.keys():
        username = str(users['username'])

    if "password" in users.keys():
        password = str(users['password'])

    if "privilege" in users.keys():
        privilege = str(users['privilege'])

    if username and password and privilege:
        cmd = 'cdbctl update/cdb/app/user/%s/privilege/%s/passwd/"%s"' % (username, privilege, password)

    elif username and password:
        cmd = 'cdbctl update/cdb/app/user/%s/passwd/"%s"' % (username, password)

    elif username and privilege:
        cmd = 'cdbctl update/cdb/app/user/%s/privilege/%s' % (username, privilege)

    elif username:
        cmd = 'cdbctl update/cdb/app/user/%s' % (username)

    status, lines = vcmd.get_status_lines(cmd)

    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj

    return obj

def add(users):
    """
    API:
    
    POST: {
        username: user_management_username,
        password: user_management_password,
        privilege:user_management_privilege
    }
    
    Retrun: {
        user: [
            {user: string, password: string, privilege: number },
            ...
        ],
    }
    """

    username=''
    password=''
    privilege=''
    cmd = ''
    
    _err_reason = [
        '',                     # err_code: 0
        'bad request',          # err_code: 1
        'can not create user'   # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}


    if "username" in users.keys():
        username = str(users['username'])

    if "password" in users.keys():
        password = str(users['password'])

    if "privilege" in users.keys():
        privilege = str(users['privilege'])






    if username and password and  privilege:
        cmd = 'cdbctl create/cdb/app/user/%s/privilege/%s/passwd/"%s"' % (username, privilege, password)

    elif  username and  password:
        cmd = 'cdbctl create/cdb/app/user/%s/passwd/"%s"' % (username, password)

    elif  username and  privilege:
        cmd = 'cdbctl create/cdb/app/user/%s/privilege/%s' % (username, privilege)
      
    elif  username:
        cmd = 'cdbctl create/cdb/app/user/%s' % (username)

    status, lines = vcmd.get_status_lines(cmd)

    # error
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj
        
    return obj


    
    
    