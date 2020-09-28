#!/usr/bin/python
#-*- coding: utf-8 -*-

import nose

from webapi import Verify, VerifyParam

def test_verfy_param_string():
    
    ins = VerifyParam('ab1')

    # type
    assert ins.type('string')
    assert not ins.type('float')
    assert not ins.type('int')

    # len
    assert not len(ins) > 3
    assert len(ins) > 2
    assert not len(ins) >= 4
    assert len(ins) >= 3
    assert not len(ins) < 3
    assert len(ins) < 4
    assert not len(ins) <= 2
    assert len(ins) <= 3

    # regexp
    assert ins.match('^\w+\d$')
    assert not ins.match('^\d\d\d$')

    # ==, !=
    assert ins == 'ab1'
    assert not ins == 'ab2'
    assert ins != 'ab2'
    assert not ins != 'ab1'

    #any
    assert ins.any(['ab2', 'ab1'])
    assert not ins.any(['ab2', 'ab3'])

def test_verify_param_int():

    ins = VerifyParam('123')

    # type
    assert ins.type('string')
    assert ins.type('int')
    assert not ins.type('string')

    # >, <, >=, <=, ==, !=
    assert ins > 122
    assert not ins > 123
    assert ins < 124
    assert not ins < 123
    assert ins >= 123
    assert not ins >= 124
    assert ins <= 123
    assert not ins <= 122
    assert ins == 123
    assert not ins == 122
    assert ins != 122
    assert not ins != 123

    # len
    assert len(ins) == 3
    assert not len(ins) == 4

    # any
    assert ins.any([123, 124])
    assert not ins.any([124, 125])

def test_verifya_param_float():
    
    ins = VerifyParam('10.03')

    # type
    assert ins.type('string')
    assert not ins.type('int')
    assert ins.type('float')
    assert not ins.type('string')

    # >, <, =, >=, <=
    assert ins == 10.03
    assert ins > 10.02
    assert ins < 10.04
    assert ins >= 10.03
    assert ins <= 10.03

    # any
    assert ins.any([10, 10.01, 10.02, 10.03])

def test_verify_param_bool():

    ins = VerifyParam(True)

    # type
    assert ins.type('bool')

    # ==
    assert ins == True
    assert not ins == False

def test_verify_param_dict():

    ins = VerifyParam({'k1': 'v1', 'k2': 'v2'})

    # type
    assert ins.type('dict')

    # len
    assert len(ins) == 2

    # ==, !=
    assert ins == {'k1': 'v1', 'k2': 'v2'}
    assert ins == {'k2': 'v2', 'k1': 'v1'}
    assert ins != {'k1': 'v1'}
    assert ins != {'k1': 'v2', 'k2': 'v2'}
    assert ins != {'k2': 'v1', 'k3': 'v2'}

    ins = VerifyParam({'k1': {'kk1': 'vv1', 'kk2': 'vv2'}, 'k2': 'v2'})
    assert ins == {'k1': {'kk1': 'vv1', 'kk2': 'vv2'}, 'k2': 'v2'}

def test_verify_param_array():

    ins = VerifyParam([1, 2, 3, 4, 5])

    # type
    assert ins.type('array')

    # len
    assert len(ins) == 5

    # ==, !=
    assert ins == [1, 2, 3, 4, 5]
    assert ins != [1, 2, 3, 4]
    assert ins != [1, 2, 3, 5, 4]

    ins = VerifyParam([1, {'k1': 'v1'}, 3, [11, [111, 222, 333], 33], 5])
    assert ins == [1, {'k1': 'v1'}, 3, [11, [111, 222, 333], 33], 5]

def test_verify():
    assert 1 == 1

