#/bin/bash

# enable this mibs
MIBS="smux agentx notification target mibII/vacm_vars snmpv3mibs ucd-snmp/memory"
MIBS=$MIBS" ucd-snmp/vmstat ucd-snmp/loadave"
MIBS=$MIBS" mibII/tcp mibII/udp mibII/icmp mibII/snmp_mib"
#MIBS=$MIBS" ucd-snmp/proc"

# disable this mibs
DIS_MIBS="mibII ucd_snmp agent_mibs utilities disman/event disman/schedule ucd-snmp host"

export CFLAGS=-D_CENTEC_ && ./configure --prefix=/usr \
    --sysconfdir=/etc \
    --localstatedir=/var \
    --enable-des \
    --enable-md5 \
    --enable-ipv6 \
    --enable-local-smux \
    --enable-mfd-rewrites  \
    --enable-reentrant \
    --with-agentx-socket="/tmp/agentx" \
    --with-default-snmp-version="3" \
    --with-sys-contact=admin@centecnetworks.com \
    --with-sys-location="Suzhou, China" \
    --with-logfile=/var/log/snmp.log \
    --disable-embedded-perl \
    --without-rpm \
    --with-persistent-directory="/var/net-snmp" \
    --with-enterprise-oid=27975 \
    --with-enterprise-sysoid=.1.3.6.1.4.1.27975.1 \
    --with-enterprise-notification-oid=.1.3.6.1.4.1.27975.4 \
    --disable-mibs \
    --with-mib-modules="$MIBS" \
    --with-out-mib-modules="$DIS_MIBS" \
    --disable-as-needed \
    --disable-embedded-perl \
    --disable-perl-cc-checks
         

#    --enable-internal-md5 \
#    --enable-agentx-dom-sock-only \
#    --with-enterprise-sysoid=.1.3.6.1.4.1.27975.3.2... \
#    --with-enterprise-notification-oid=.1.3.6.1.4.1.27975.4 \
#    --enable-debugging \
#    --disable-debugging \
#    --enable-mib-config-checking \
#    --enable-mib-config-debug  \
#    --with-endianness=little \
#    --without-openssl \

