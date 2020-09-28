#!/bin/sh

xtables-multi iptables -t nat --list > /tmp/ntp-tables_XXXXXX
