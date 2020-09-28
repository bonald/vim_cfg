#!/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals, division

import os
import re
import sys
import getopt
import commands
import time

USAGE = """
Usage: {fname} OPTION
Add or delete openflow controller.

  -o, --operate=OPER      'add' or 'del'
  -i, --inband            Specific inband to connect controller
  -a, --address=IPADDR    controller ip address
  -6, --ipv6=TRUE         controller ipv6 connection
  -p, --port=PORT         controller port number
  -t, --type=TYPE         'tcp' or 'ssl', protocol type
  -b, --bind=IFACE        bind interface
  -l, --localip=IPADDR    bind ip address
  -e, --binden=ENABLE     enable bind local address
  -s  --switch=all        delet all or someone
""".format(fname=os.path.basename(__file__))

def usage():
    print(USAGE)
    exit(1)

def parse_args(argv):
    config = {
        'is_add': False,
        'is_inband': False,
        'address': '',
        'is_ipv6': False,
        'port': 0,
        'protocol': '',
        'iface': '',
        'localip': '',
        'bind_en': False,
        'switch': False
    }

    if len(argv) <= 1:
        usage()

    try:
        opts, args = getopt.gnu_getopt(argv, 'o:ia:p:t:b:l:es:6', 
                ['operate=', 'inband', 'address=', 'port=', 'type=', 'bind=', 'localip=', 'binden', 'switch=', 'ipv6='])
    except Exception:
        usage()
    for opt, val in opts:
        if opt in ['-o', '--oper']:
            config['is_add'] = True if val == 'add' else False
        elif opt in ['-i', '--inband']:
            config['is_inband'] = True
        elif opt in ['-a', '--address']:
            config['address'] = val
        elif opt in ['-p', '--port']:
            config['port'] = val
        elif opt in ['-t', '--type']:
            config['protocol'] = val
        elif opt in ['-b', '--bind']:
            config['iface'] = val
        elif opt in ['-l', '--localip']:
            config['localip'] = val
        elif opt in ['-e', '--binden']:
            config['bind_en'] = True
        elif opt in ['-s', '--switch']:
            config['switch'] = True if val == 'all' else False
        elif opt in ['-6', '--ipv6']:
            config['is_ipv6'] = True
        else:
            usage()

    return config

def of_protocol_to_int(protocol):
    if protocol == 'tcp':
        return 0
    elif protocol == 'udp':
        return 1
    elif protocol == 'ssl':
        return 2
    else:
        raise ValueError('% Unknow protocol "{}"'.format(protocol))

def of_int_to_protocol(num):
    num = int(num)

    if num == 0:
        return 'tcp'
    elif num == 1:
        return 'udp'
    elif num == 2:
        return 'ssl'
    else:
        raise ValueError('% Unknow protocol type "{}"'.format(num))

def protocol_to_int(protocol):
    if protocol in ['tcp', 'ssl']:
        return 0
    else:
        return 1

def add_ns_route(ip, port, protocol, is_inband):
    cmd = ('cdbctl create/cdb/sys/ns_route/'
           '{protocol}#{port}#{addr}/1/{inband}/'
           .format(protocol=protocol, port=port, addr=ip,
                   inband='inband' if is_inband else 'outband'))

    return os.system(cmd) >> 8

def del_ns_route(ip, port, protocol):
    cmd = ('cdbctl delete/cdb/sys/ns_route/'
           '{protocol}#{port}#{addr}/inband'
           .format(protocol=protocol, port=port, addr=ip))

    return os.system(cmd) >> 8

def add_inband_snat(ip, port, protocol, bind_en, iface, localip):
    if not bind_en:
        return 0

    cmd = ('cdbctl create/cdb/sys/inband_snat/'
           '{protocol}#{port}#{addr}/'
           'type/1/ifname/{bind_if}/ip/{bind_addr}/'
           .format(protocol=protocol, port=port, addr=ip, bind_if=iface, bind_addr=localip))

    return os.system(cmd) >> 8

def del_inband_snat(ip, port, protocol):
    cmd = ('cdbctl delete/cdb/sys/inband_snat/'
           '{protocol}#{port}#{addr}/'
           .format(protocol=protocol, port=port, addr=ip))

    return os.system(cmd) >> 8

def refresh_controller():
    output = commands.getoutput('cdbctl read/cdb/sys/ns_route | grep flag=1')

    # delete all controller
    if not output.strip():
        cmd = 'ovs-vsctl del-controller-centec br0'
        return os.system(cmd)

    # refresh controller
    ctrl_arr = []
    for line in output.split('\n'):
        key = re.findall(r'/key=(.*?)/|$', line)[0]
        protocol, port, ip = key.split('#')
        protocol = of_int_to_protocol(protocol)
        ctrl_arr.append(':'.join([protocol, ip, port]))

    cmd = 'ovs-vsctl set-controller-centec br0 {}'.format(' '.join(ctrl_arr))

    return os.system(cmd) >> 8

def add_inband(ip, port, protocol, bind_en, iface, localip):
    res = add_inband_snat(ip, port, protocol_to_int(protocol), bind_en, iface, localip)
    if res:
        del_inband_snat(ip, port, protocol_to_int(protocol))
        return res

    res = add_ns_route(ip, port, of_protocol_to_int(protocol), True)
    return res

def add_outband(ip, port, protocol):
    res = add_ns_route(ip, port, of_protocol_to_int(protocol), False)
    return res
        
def del_controller(ip, port, protocol, is_inband):
    del_ns_route(ip, port, of_protocol_to_int(protocol))
    if is_inband:
        del_inband_snat(ip, port, protocol_to_int(protocol))

def limit_controller_count():
    output = int(commands.getoutput('cdbctl read/cdb/sys/ns_route | grep flag=1 | wc -l'))
    if output >= 7:
        print('% The number of controller should not exceed 7')
        sys.exit(1)

def main():
    config = parse_args(sys.argv[1:])

    if config['is_add']:
        limit_controller_count()
        if config['is_ipv6']:
            target = config['protocol'] + ":\[" + config['address'] + "\]:" + config['port']
            target = target.replace(":","\:")
            cmd_stringx = 'ovs-vsctl  --columns=_uuid find controller target=\"' + target + '\"' + '| awk \'{print $3}\''
        else:
            target = config['protocol'] + "\\:" + config['address'] + "\\:" + config['port']
            cmd_stringx = 'ovs-vsctl  --columns=_uuid find controller target=\"' + target + '\"' + '| awk \'{print $3}\''
        uuid = commands.getoutput(cmd_stringx)
        lens = len(uuid)
        if(0 != lens):
            print('% the controller is exist!')
            return 0
        if config['is_inband']:
            res = add_inband(config['address'], config['port'], config['protocol'],
                       config['bind_en'], config['iface'], config['localip'])
        else:
            res = add_outband(config['address'], config['port'], config['protocol'])
        if(0 != res):
            sys.exit(1)
        if config['is_ipv6']:
            cmd = "ovs-vsctl -- add bridge br0 controller @m -- --id=@m create controller target=" + '\"' + target + '\"'
        else:
            cmd = "ovs-vsctl -- add bridge br0 controller @m -- --id=@m create controller target=" + '\"' + target + '\"'
        return os.system(cmd) >> 8
    else:
        if config['switch']:
            #cmd = 'ovs-vsctl del-controller-centec br0'
            #return os.system(cmd)
            return 0
        else:
            if config['is_ipv6']:
                target = config['protocol'] + ":\[" + config['address'] + "\]:" + config['port']
                target = target.replace(":","\:")
                cmd_stringx = 'ovs-vsctl  --columns=_uuid find controller target=\"' + target + '\"' + '| awk \'{print $3}\''
            else:
                target = config['protocol'] + "\\:" + config['address'] + "\\:" + config['port']
                cmd_stringx = 'ovs-vsctl  --columns=_uuid find controller target=\"' + target + '\"' + '| awk \'{print $3}\''
            uuid = commands.getoutput(cmd_stringx)
            lens = len(uuid)
            if(0 == lens):
                print('% the controller is not exist!')
                return 0
            #delete the route and controller			
            cmd = "ovs-vsctl -- remove bridge br0 controller " + uuid
            os.system(cmd)
            time.sleep(3)
            del_controller(config['address'], config['port'], config['protocol'], config['is_inband'])
            return 0

if __name__ == '__main__':
    main()


