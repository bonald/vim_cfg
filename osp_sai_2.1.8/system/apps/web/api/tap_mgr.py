#!/usr/bin/python
#-*- coding: utf-8 -*-

from vcl import vcmd
import string
import base64
import portMapping

portMap = portMapping.portNameMapping()

def tap_group_table_add(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    group_name = str(req_data.get('group_name'))
    group_id   = str(req_data.get('group_id'))

    if len(group_name) == 0:
        return obj

    cmd = 'cdbctl create/cdb/tap/tap_group/%s' % (group_name)

    if group_id.isdigit():
        cmd = cmd + '/%s' % (group_id) 
    else:
        cmd = cmd + '/-1'

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj

def tap_group_table_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    group_name = str(req_data.get('key'))

    cmd = 'cdbctl delete/cdb/tap/tap_group/%s' % (group_name)
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj   

def tap_group_table_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/tap/tap_group'
    arr = vcmd.get_arr(cmd)
    if arr:
        obj['arr'] = arr
    else:
        obj['arr'] = []

    return obj       

def tap_truncation_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    enable   = str(req_data.get('truncationEn'))
    length     = str(req_data.get('length'))

    if enable=="True":
        cmd = 'cdbctl update/cdb/sys/sys_global/trunction_length/%s' % length
    else:
        cmd = 'cdbctl update/cdb/sys/sys_global/trunction_length/0'

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj    

def tap_truncation_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/sys/sys_global/trunction_length'
    arr = vcmd.get_arr(cmd)
    if arr:
        obj['trunction'] = arr[0]['trunction_length']
    else:
        obj['trunction'] = ''

    return obj        

def tap_truncation_del():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl update/cdb/sys/sys_global/trunction_length/0'
    arr = vcmd.get_arr(cmd)
    if arr:
        obj['arr'] = arr
    else:
        obj['arr'] = []

    return obj   

def tap_description_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    group_name = str(req_data.get('group_name'))
    description = str(req_data.get('description'))


    if len(description):
        cmd = 'cdbctl update/cdb/tap/tap_group/%s/desc/1/%s' % (group_name, description)
    else:
        cmd = 'cdbctl update/cdb/tap/tap_group/%s/desc/0' % (group_name)
 
    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj     

def tap_entry_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    group = str(req_data.get('group'))
    dir = str(req_data.get('direction'))
    port = str(req_data.get('port'))
    trunEn = str(req_data.get('trunEn'))
    tstamp = str(req_data.get('tstamp'))
    removeVlanEn = str(req_data.get('removeVlanEn'))

    markEn = str(req_data.get('markEn'))
    vlanId = str(req_data.get('vlanId'))

    flow = str(req_data.get('flow'))

    editMacDaValue = str(req_data.get('editMacDaValue'))
    editMacSaValue = str(req_data.get('editMacSaValue'))
    editIPDaEn  = str(req_data.get('editIPDaEn'))
    editdstIpType = str(req_data.get('editdstIpType'))
    editIPDaValue = str(req_data.get('editIPDaValue'))
    editIPv6DaValue = str(req_data.get('editIPv6DaValue'))
    editIPSaEn  = str(req_data.get('editIPSaEn'))
    editsrcIpType = str(req_data.get('editsrcIpType'))
    editIPSaValue = str(req_data.get('editIPSaValue'))
    editIPv6SaValue = str(req_data.get('editIPv6SaValue'))
    editVlanValue  = str(req_data.get('editVlanValue'))

    if dir == "egress":
        cmd = 'cdbctl create/cdb/tap/tap_group_egress/%s#%s/timestamp/false' % (group, port)

        if tstamp == 'True':
            cmd = 'cdbctl create/cdb/tap/tap_group_egress/%s#%s/timestamp/true' % (group, port)
    else:
        if flow != 'None':
            ## Flow conflict with truncation
            cmd = 'cdbctl create/cdb/tap/tap_group_ingress_flow/%s/%s/FLOW/%s' % (group, port, flow)

            if markEn == 'True':
                cmd += '/mark-source/%s//' % (vlanId)
            if removeVlanEn == 'double-vlan':
                cmd += '/mark-source//untag/true'
            elif removeVlanEn == 'outer-vlan':
                cmd += '/mark-source//untag/s-vlan'
            elif removeVlanEn == 'inner-vlan':
                cmd += '/mark-source//untag/c-vlan'
        else:
            ## Untag conflict with mark-source
            cmd = 'cdbctl create/cdb/tap/tap_group_ingress/%s#%s' % (group, port)

            if trunEn == 'True':
                cmd += '/mark-source/0/untag/false/truncation/true'
            elif removeVlanEn == 'double-vlan':
                cmd += '/mark-source/0/untag/true/truncation/false'
            elif removeVlanEn == 'outer-vlan':
                cmd += '/mark-source/0/untag/s-vlan/truncation/false'
            elif removeVlanEn == 'inner-vlan':
                cmd += '/mark-source/0/untag/c-vlan/truncation/false'
            elif markEn == 'True':
                cmd += '/mark-source/%s/untag/false/truncation/false' % vlanId
            else:
                cmd += '/mark-source/0/untag/false/truncation/false'

            ## Process EditPacket
            if editMacDaValue:
                cmd = cmd + "/edit-macda/%s" % (editMacDaValue)

            if editMacSaValue:
                cmd = cmd + "/edit-macsa/%s" % (editMacSaValue)

            if editVlanValue:
                cmd = cmd + "/edit-vlan/%s" % (editVlanValue)

            if editIPDaEn == 'True':
                if editdstIpType == 'ipv4':
                    cmd = cmd + "/edit-ipda/%s" % (editIPDaValue)
                elif editdstIpType == 'ipv6':
                    cmd = cmd + "/edit-ipv6da/%s" % (editIPv6DaValue)

            if editIPSaEn == 'True':
                if editsrcIpType == 'ipv4':
                    cmd = cmd + "/edit-ipsa/%s" % (editIPSaValue)
                elif editsrcIpType == 'ipv6':
                    cmd = cmd + "/edit-ipv6sa/%s" % (editIPv6SaValue)

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj     

def tap_entry_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'egress': [], 'ingress': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/tap/tap_group_egress/' 

    # Get egress direction entry
    arr = vcmd.get_arr(cmd)

    if arr:
        for entry in arr:

            temp = entry.get('key')

            entry['name'] = temp.split('#')[0]
            entry['port'] = temp.split('#')[1]
            entry['sort_name'] = portMap.Mapping(entry['port'])
            entry['full_name'] = portMap.Mapping(entry['port'], is_sort=False)

        obj['egress'] = arr

    else:
        obj['egress'] = []

    # Get in gress direction entry

    cmd_flow = 'cdbctl read/cdb/tap/tap_group_ingress_flow' 
    cmd_normal = 'cdbctl read/cdb/tap/tap_group_ingress' 

    arr_flow = vcmd.get_arr(cmd_flow)

    arr_normal = vcmd.get_arr(cmd_normal)

    arr = arr_flow + arr_normal

    if arr:
        # change 'tap2#eth-0-24#flow1' to 'tap2','eth-0-24','flow1'
        for entry in arr:
            temp = entry.get('key').split('#')

            if entry['ing_untag_en'] == '1':
                entry['untag_mode'] = 'double-vlan'
            elif entry['ing_untag_en'] == '2':
                entry['untag_mode'] = 'outer-vlan'
            elif entry['ing_untag_en'] == '3':
                entry['untag_mode'] = 'inner-vlan'
            else:
                entry['untag_mode'] = 'Disable'

            if len(temp) == 2:
                entry['name'] = temp[0]
                entry['port'] = temp[1]
                entry['sort_name'] = portMap.Mapping(entry['port'])
                entry['full_name'] = portMap.Mapping(entry['port'], is_sort=False)
                entry['flow'] = ''

            elif len(temp) == 5:
                entry['name'] = temp[0]
                entry['port'] = temp[1]
                entry['sort_name'] = portMap.Mapping(entry['port'])
                entry['full_name'] = portMap.Mapping(entry['port'], is_sort=False)
                entry['flow'] = temp[4]

                # only hold a place
                entry['ing_trunction_en'] = '0'

            # ingress can't support edit-pkt, only hold a place for display
                entry['ing_edit_dest_mac_en'] = '0'
                entry['ing_edit_src_mac_en'] = '0'
                entry['ing_edit_ipda_en'] = '0'
                entry['ing_edit_ipsa_en'] = '0'

        obj['ingress'] = arr
    else:
        obj['ingress'] = []    

    return obj        

def tap_entry_del(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    key = str(req_data.get('key'))
    direction = str(req_data.get('direction'))

    if direction == 'egress':
        cmd = 'cdbctl delete/cdb/tap/tap_group_egress/%s' % (key)

        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = ' '.join(lines)
            return obj

    elif  direction == 'ingress':
        flow = str(req_data.get('flow'))

        if len(flow) == 0:
            # key : 'tap1#eth-0-11'
            cmd = 'cdbctl delete/cdb/tap/tap_group_ingress/%s/' % (key)
        else:
            # key: 'tap1#eth-0-11#flow1' --> 'tap1/eth-0-11/flow1'
            # key = key.replace('#', '/').replace('//FLOW', '')
            keysplit = key.split('#')
            key = keysplit[0] + '/' + keysplit[1] + '/FLOW/' + keysplit[len(keysplit)-1]
            cmd = 'cdbctl delete/cdb/tap/tap_group_ingress_flow/%s' % (key)

        status, lines = vcmd.get_status_lines(cmd)
        if status != 0:
            obj['error'] = True
            obj['err_code'] = 1
            obj['err_reason'] = ' '.join(lines)
            return obj

    return obj  

########### timestamp ##############
def tap_timestamp_set(req_data):
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    StampEn   = str(req_data.get('timeStampEn'))
    macDa     = str(req_data.get('macDa'))
    macSa     = str(req_data.get('macSa'))
    etherType = str(req_data.get('etherType'))

    if StampEn == "True":
        cmd = 'cdbctl update/cdb/sys/sys_global/tap_ts_macda/%s/%s/%s' % (macDa, macSa, etherType)
    else:
        cmd = 'cdbctl update/cdb/sys/sys_global/tap_ts_macda/0.0.0/0.0.0/0'

    status, lines = vcmd.get_status_lines(cmd)
    if status != 0:
        obj['error'] = True
        obj['err_code'] = 1
        obj['err_reason'] = ' '.join(lines)
        return obj

    return obj    

def tap_timestamp_get():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl read/cdb/sys/sys_global/'
    arr = vcmd.get_arr(cmd)

    if arr:
        # int -> hex format
        value = arr[0].get('ether_type')
        arr[0]['ether_type'] = hex(int(value)) if value.isdigit() else 0xffff

        obj['cfg'] = arr
    else:
        obj['cfg'] = []

    return obj        

def tap_timestamp_del():
    """
    TBD
    """

    _err_reason = [
            '',                         # err_code: 0
            'cmd excute error',         # err_code: 1
            ]

    obj = {'arr': [], 'error': False, 'err_code': 0, 'err_reason': ''}

    cmd = 'cdbctl update/cdb/sys/sys_global/trunction_length/0'
    arr = vcmd.get_arr(cmd)
    if arr:
        obj['arr'] = arr
    else:
        obj['arr'] = []

    return obj       

