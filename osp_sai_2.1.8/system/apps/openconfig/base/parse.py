#!/usr/bin/python
#-*- coding: utf-8 -*-

import json
import re
import base64

def parse_json(string, error={}):
    res = error
    try:
        res = json.loads(string)
    except TypeError:
        pass
    except ValueError:
        pass
    
    return res

def parse_int(string, error=None):
    res = error
    try:
        res = int(string)
    except TypeError:
        pass
    except ValueError:
        tmp = re.findall(r'\d+', string)
        if tmp:
            res = int(tmp[0])
    
    return res

def parse_string(arg):
    return str(arg)

def parse_base64(string, error=None):
    res = error
    try:
        res = base64.decodestring(str(string))
    except Exception:
        pass
    
    return res
    

if __name__ == '__main__':
    
    pass