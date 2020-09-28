#!/usr/bin/python
#-*- coding: utf-8 -*-

import base, urls, api
import logging
import commands

from vcl import vcmd

yaml_en = True

try:
    import url
except Exception:
    yaml_en = False

def main():
    if yaml_en:
        url.register('url.yaml')
    if not base.config.log_en:
        log = logging.getLogger('werkzeug')
        log.setLevel(logging.CRITICAL)
    try:
        ssl_en = commands.getoutput(r"cat /var/rpcapi/http.conf | grep ssl_en | sed 's/ssl_en=\([^\/]\+\).*/\1/'")
        auth_mode = commands.getoutput(r"cat /var/rpcapi/http.conf | grep auth_mode | sed 's/auth_mode=\([^\/]\+\).*/\1/'")
        int_auth_mode = int(auth_mode)
        port = commands.getoutput(r"cat /var/rpcapi/http.conf | grep port | sed 's/port=\([^\/]\+\).*/\1/'")
        port = int(port)
        disable = commands.getoutput(r"cat /var/rpcapi/http.conf | grep disable | sed 's/disable=\([^\/]\+\).*/\1/'")
        disable = int(disable)
    except Exception:
        port = 80
        disable = 0
        ssl_en = 0
        int_auth_mode = 0
    if disable == 0:
        api.cmd_api.GetUsers(int_auth_mode)
        int_ssl_en = int(ssl_en)
        if (1 == int_ssl_en):
            base.web_app.app.run(debug=False, host="::1", port=port, threaded=False)
        else:
            base.web_app.app.run(debug=False, host="::", port=port, threaded=False)
		
if __name__ == "__main__":
    main()
