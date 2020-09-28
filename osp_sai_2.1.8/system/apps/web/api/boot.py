#!/usr/bin/python
#-*- coding: utf-8 -*-

import os
import re
from vcl import vcmd
from werkzeug import secure_filename

import file_mgr
def boot_info():
    """
    API: GET
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
        boot: {
            mode: 'flash' or 'tftp',
            bootcmd: string,
            current_image: string,
            image: string,
            ipaddr: string,
            serverip: string,
        }
    }
    """ 
    _err_reason = [
        '',                 # err_code: 0
        'cmd excute error', # err_code: 1
    ]
    
    obj = {'error': False, 'err_code': 0, 'err_reason': '' } 
    
    res = file_mgr._get_boot_info()
    
    obj['boot'] = res
    
    return obj
    

def set_next_boot_img(filename):
    """
    API:
    
    PUT: /<filename>/
        filename: image name in /mnt/flash/boot folder,
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """ 
    _err_reason = [
        '',            # err_code: 0
        'no such file: %s set_next_boot_img' % str(filename), # err_code: 1
        'invalid image file', # err_code: 2
        '', # err_code: 3
    ]
    
    obj = {
        'error': False,
        'err_code': 0,
        'err_reason': ''
    } 
    
    status, lines = vcmd.get_status_lines('cdbctl -t 60 update/cdb/sys/bootimage/flash:/%s' % str(filename))
    
    if status != 0: # error
        obj['error'] = True
        if re.search(r'^% Not a file error:', lines[0]):
            obj['err_code'] = 1
            obj['err_reason'] = _err_reason[1]
        elif re.search(r'^% Invalid image file:', lines[0]):
            obj['err_code'] = 2
            obj['err_reason'] = _err_reason[2]
        else:
            obj['err_code'] = 3
            obj['err_reason'] = lines[0]
        return obj
    
    return obj

def reboot(is_save):
    """
    API
    
    GET: /boot/reboot/<is_save:'true' or 'false'> 
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    
    _err_reason = [
        '',            # err_code: 0
        'bad request', # err_code: 1
    ]
    
    obj = { 'error': False, 'err_code': 0, 'err_reason': '' } 
    
    if str(is_save) == 'true':
        vcmd.send('cdbctl update/cdb/sys/reboot/normal/save')
        pass
    elif str(is_save) == 'false':
        vcmd.send('cdbctl update/cdb/sys/reboot/normal')
        pass
    else:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
        
    
    return obj

def upload_image(fil):
    """
    API
    
    PUSH: 
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """     
    
    _err_reason = [
        '',            # err_code: 0
        'bad request', # err_code: 1
        '',            # err_code: 2, file already exists
        'IO Error',    # err_code: 3
    ]
    
    obj = {
        'error': False,
        'err_code': 0,
        'err_reason': ''
    } 
    
    if not fil:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    file_path = '/mnt/flash/boot/' + secure_filename(fil.filename)
    # check exists
#    if os.path.exists(file_path):
#        obj['error'] = True
#        obj['err_code'] = 2
#        obj['err_reason'] = 'file "%s" already exists, please rename the image and reupload' % file_path
#        return obj
    
    # save file
    try:
        #1. unlock protect for ../boot path
        status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -rw ubi1:boot /mnt/flash/boot/')
        fil.save(file_path)
        #2. restore protect for ../boot path
        status, lines = vcmd.get_status_lines('mount -t ubifs -o remount -r ubi1:boot /mnt/flash/boot/')
    except IOError:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj
        
    
    return obj


def set_next_boot_startup_conf(filename):
    """
    API:
    
    PUT: /<filename>/
        filename: next startup-config.conf file,
    
    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """ 
    _err_reason = [
        '',            # err_code: 0
        'no such file: %s set_next_boot_startup_conf' % str(filename), # err_code: 1
        'copy error', # err_code: 2
    ]
    
    obj = {
        'error': False,
        'err_code': 0,
        'err_reason': ''
    }  
    
    # check filename exists
    file_path = '/mnt/flash/' + filename
    if not os.path.exists(file_path):
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj
    
    startup_conf_path = '/mnt/flash/startup-config.conf'
    # conve start-config.conf
    status, lines = vcmd.get_status_lines('cp %s %s' % (file_path, startup_conf_path))
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 2
        obj['err_reason'] = '; '.join(lines)
        return obj


    return obj

def upload_flash_files(fil):
    """
    API

    PUSH:

    Retrun: {
        error: bool,
        err_code: int,
        err_reason: string
    }
    """
    _err_reason = [
        '',  # err_code: 0
        'bad request',  # err_code: 1
        '',  # err_code: 2, file already exists
        'IO Error',  # err_code: 3
    ]

    obj = {
        'error': False,
        'err_code': 0,
        'err_reason': ''
    }

    if not fil:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = _err_reason[1]
        return obj

    file_path = '/mnt/flash/' + secure_filename(fil.filename)

    try:
        fil.save(file_path)
    except IOError:
        obj['error'] = True
        obj['err_code'] = 3
        obj['err_reason'] = _err_reason[3]
        return obj

    return obj
