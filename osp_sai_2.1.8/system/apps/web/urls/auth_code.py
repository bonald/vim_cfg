#!/usr/bin/python
#-*- coding: utf-8 -*-

from flask import request, send_file
import base, api
import os
import random
from PIL import Image, ImageDraw, ImageFont,ImageFilter

imglen = 4
base_str = ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z')

def create_verficode():
    try:
        fontimg = Image.open("/var/web/12345.png")
        #fontimg = Image.open("./12345.png")
    except:
        code = ''
    else:
        codeimg = Image.new('RGBA', (19 * imglen, 25), (255, 255, 255))
        code = ''
        for x in range(imglen):
            ran = random.randint(0, 35)
            code = ''.join((code, base_str[ran]))
            img_s = fontimg.crop((ran * 19, 0, (ran + 1) * 19, 20))
            img_s = img_s.rotate(random.randint(-30, 30))
            codeimg.paste(img_s, (x * 19, 2))

        codeimg.save("/var/web/verficode.png")
    return code

@base.web_app.app.route("/static/switch/auth_code/auth_code.jpg", methods=['GET'])
def auth_code():
    str = create_verficode()
    resp = send_file('/var/web/verficode.png', mimetype="image/png")
    resp.set_cookie('verficode_id',str)
    return resp