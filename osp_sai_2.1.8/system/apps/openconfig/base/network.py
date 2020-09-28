#!/usr/bin/python
#-*- coding: utf-8 -*-

import socket

def get_lo_ip():
    """
    get localhost ip address
    """
    try:
        ip = socket.gethostbyname(socket.gethostname())
    except Exception:
        ip = '127.0.0.1'
        
    return ip
    

if __name__ == '__main__':
    print get_lo_ip()
    pass