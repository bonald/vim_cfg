#!/usr/bin/python
#-*- coding: utf-8 -*-
'''
Created on Aug 20, 2016

@author: wangqj
'''

import yaml
import types

def yaml_to_dict(file_name):
    '''
    analyze yaml file
    '''
    obj = {}

    with open(file_name) as f:
        obj = yaml.load(f)

    return obj

def register_urls(obj):
    
    api_prefix = obj['api_prefix']
    version    = obj['version']
    ins_obj    = {}

    pkg = __import__(obj['class_pkg'])

    for i in obj['urls']:
        # get router info
        url       = i['url']
        methods   = i['method']
        ins_index = i['class']
        auth_en   = i['auth']
        file_name, class_name = tuple(i['class'].split('.'))

        # url shaping
        url = '/%s/%s%s' % (api_prefix, version, url)

        # check in ins_obj
        if ins_obj.get(ins_index) == None:
            # create instance
            class_tmp = getattr(getattr(pkg, file_name), class_name)
            ins = None
            if auth_en == True:
                ins = class_tmp(auth = True)
            else:
                ins = class_tmp(auth = False)

            # add to ins_obj
            ins_obj[ins_index] = ins

        # get ins
        ins = ins_obj[ins_index]

        # reigster url by method
        for method in methods:
            fn_str = methods[method]
            fn = getattr(ins, fn_str)
            ins.register_url(url, fn, method)


def register(file_name):
    '''
    register urls
    '''
    obj = {}

    obj = yaml_to_dict(file_name)
    register_urls(obj)

