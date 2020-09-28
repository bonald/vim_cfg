#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import url_for
import os
import base
from vcl import vcmd
import vcl.config
import re
import types

def _get_boot_info():
    """get boot image infomations
    """
    res = vcmd.get_arr('cdbctl read/cdb/sys/bootimage')[0]
    if res['mode'] == 'flash:':
        tmp = res['current_image']
        tmp = tmp.replace(res['mode'], '')
        res['current_image'] = os.path.basename(tmp)
    return res

def _get_files(param):
    """
    Return: [
       {filename: string, size: int, date: string, time: string, datetime: string, url: string,},
       ...
       ...
    ]
    """
    res = []
    static_flash_dir = r'%s/static/flash' % base.config.root_dir
    dirname = re.sub(r'^/mnt/flash/?', '', os.path.dirname(param))
    
    # check flash folder exist
    if(not os.path.exists(static_flash_dir)):
        vcmd.execute('ln -s /mnt/flash %s' % static_flash_dir)

    
    # get cmd lines
    cmd = 'ls -l --full-time %s | grep -r \'^-\' | awk \'{printf("%%s=%%s=%%s=%%s\\n", $5, $6, $7, $9)}\'' % param;
    status,lines = vcmd.get_status_lines(cmd)
    if not lines:
        return res

    # make dict
    tmp = []
    for line in lines:
        tmp = line.split('=')
        if not tmp or len(tmp) != 4:
            continue
        elif tmp[0] and tmp[1] and tmp[2] and tmp[3]:
            
            # append data
            size, date, time, filename = tmp[0], tmp[1], re.sub(r'\.\d+$', '', tmp[2]), re.sub(r'^/mnt/flash/', '', tmp[3])
            
            if dirname:
                url = url_for('static', filename='flash/%s/%s' % (dirname, filename))
            else:
                url = url_for('static', filename='flash/%s' % filename)
                
            res.append({
                'size': base.parse.parse_int(size, 0),
                'date': date,
                'time': time,
                'datetime': str(date) + '_' + str(time),
                'filename': filename,
                'url': url,
            });
    
    return res

def get(file_type):
    """
    API:
    
    GET: /<file_type>/
        file_type: 'image' or 'start_conf' or 'syslog',
    
    Retrun: {
        boot_files: [
            {filename: string, size: int, date: string, time: string, datetime: string, url: string},
            ...
        ],
        image: {
            current: string,
            next: string,
        },
        
        or start_conf_files: [
            same as boot_files
        ],
        start_conf: {
            current: string,
        },
        
        or log_files: [
            same as boot_files
        ],
        
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',            # err_code: 0
        'bad request', # err_code: 1
    ]
    
    obj = {
        'error': False,
        'err_code': 0,
        'err_reason': ''
    }
    
    # get files
    if file_type == 'image':
        boot_files = _get_files('/mnt/flash/boot/')
        obj['boot_files'] = boot_files
        # get boot infomations
        info = _get_boot_info()
        obj['image'] = {'current': info.get('current_image'), 'next': info.get('image')}
        
    elif file_type == 'start_conf':
        start_conf_files = _get_files('/mnt/flash/startup-config.conf*')
        obj['start_conf_files'] = start_conf_files
        # get startup config infomations
        obj['start_conf'] = {'current': 'startup-config.conf'}
        
    elif file_type == 'syslog':
        log_files = _get_files('/mnt/flash/syslogfile/')
        obj['log_files'] = log_files

    elif file_type == 'flash':
        flash_files = _get_files('/mnt/flash/')
        obj['flash_files'] = flash_files
        
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    return obj


def delete(req_data):
    """
    API:
    
    DELETE: {
       file_arr: [
            {
               'type': 'image' or 'start_conf' or 'syslog',
               'filename': string,
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
        'delete failed', # err_code: 2
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    # param check
    file_arr = req_data.get('file_arr')
    if not type(file_arr) is types.ListType: # bad request
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
        
    # make cmd
    paths = []
    for f in file_arr:
        if not f.get('filename'):
            continue
        if f.get('type') == 'image':
            paths.append('/mnt/flash/boot/' + str(f['filename']))
        elif f.get('type') == 'start_conf':
            paths.append('/mnt/flash/' + str(f['filename']))
        elif f.get('type') == 'syslog':
            paths.append('/mnt/flash/syslogfile/' + str(f['filename']))
        elif f.get('type') == 'flash':
            paths.append('/mnt/flash/' + str(f['filename']))
    if not paths: # bad request
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    
    # delete start
    ## 1. change path to 'wr', remove protect
    status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -rw ubi1:boot /mnt/flash/boot')
    res = 0
    if vcl.config.is_dev: # remote
        cmd = 'rm ' + ' '.join(paths)
        res = vcmd.execute(cmd)
    
    else:
        for path in paths: # local
            if os.path.exists(path):
                os.remove(path)
            else:
                res = 255

    ## 2. restore path to 'r', restore protect
    status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -r ubi1:boot /mnt/flash/boot')
    if res != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj
    
    return obj

def backup(file_type, filename, backup_filename):
    """
    API:
    
    PUT: /<file_type>/<filename>/<backup_filename>/
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    
    """
    
    _err_reason = [
        '',              # err_code: 0
        'no such file: %s' % str(filename),   # err_code: 1
        'file "%s" already exists' % str(backup_filename), # err_code: 2
        'copy failed',   # err_code: 3
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': ''}
    
    path_base, path_file, path_backup_file = '', '', ''
    if file_type == 'image':
        path_base = '/mnt/flash/boot/'
    elif file_type == 'start_conf':
        path_base = '/mnt/flash/'
    elif file_type == 'syslog':
        path_base = '/mnt/flash/syslogfile/'
    elif file_type == 'flash':
        path_base = '/mnt/flash/'
    path_file = path_base + str(filename)
    path_backup_file = path_base + str(backup_filename)

    # check filename
    if not os.path.exists(path_file):
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    # check backup filename
    if os.path.exists(path_backup_file):
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = _err_reason[2]
        return obj

    ## 1. change path to 'wr', remove protect
    status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -rw ubi1:boot /mnt/flash/boot')

    # copy file
    res = vcmd.execute('cp %s %s' % (path_file, path_backup_file))

    ## 2. restore path to 'r', restore protect
    status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -r ubi1:boot /mnt/flash/boot')

    if res != 0:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj
    
    return obj


