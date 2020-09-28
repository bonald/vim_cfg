#/**************************************************************************
#* $Id: init-syslog-ng.sh,v 1.1 2007/03/16 10:00:36 xliu Exp $
#*  generate syslog-ng configuration file before imi starting
#*
#* (C) Copyright Centec Networks Inc.  All rights reserved.
#*
#* Modify History:
#* Revision      : R0.02
#* Author        : Alexander Liu, zhoucheng
#* Date          : 2007-12-10 20:13
#* Reason        : Support log trap and log merge
#**************************************************************************/

# vars
STARTUP_CFGFILE="/mnt/flash/startup-config.conf"
SYSLOG_NG_CFG="/switch/etc/syslog-ng.conf"

#
# initialize values
#
TS_FORMAT="rfc3164"
FILE_SEVERITY="warning"
SRV_SEVERITY="warning"
MOD_SEVERITY="debug"
SRV_FACILITY="local7"
TRAP_FACILITY="local6"
TRAP_SEVERITY="middle"
TO_FILE="yes"
TO_SRV="no"
TO_TRAP="yes"
ADDR_LIST=""
TS_MACRO=""
MERGE_ENABLE="no"
MERGE_FIFO_SIZE="1024"
MERGE_TIMEOUT="10000"
MERGE_TIMEFROM="yes" # Added by zhoucheng 2008-01-03 13:46, bug 3974
OPERATE_LOG="no"
#
# parse startup-configuration file
#
if [ -f "$STARTUP_CFGFILE" ] ; then
TMP_FILE="/tmp/.__logging_parse__"

cat $STARTUP_CFGFILE | grep "^logging" | awk '
{
    if ($2 == "timestamp")
    {
        print "export TS_FORMAT="$3";";
    }
    else if ($2 == "file")
    {
        if ($3 == "enable")
        {
            print "export TO_FILE=yes;";
        }
        else
        {
            print "export TO_FILE=no;";
        }
    }
    else if ($2 == "level")
    {
        if ($3 == "file")
        {
            print "export FILE_SEVERITY="$4";";
        }
        else if ($3 == "module")
        {
            print "export MOD_SEVERITY="$4";";
        }
    }
    else if ($2 == "server")
    {
        if ($3 == "enable")
        {
            print "export TO_SRV=yes;";
        }
        else if ($3 == "severity")
        {
            print "export SRV_SEVERITY="$4";";
        }
        else if ($3 == "facility")
        {
            print "export SRV_FACILITY="$4";";
        }
        else if ($3 == "address")
        {
            if ( $4 == "mgmt-if" )
            {
                print "export ADDR_LIST=$ADDR_LIST\" "$5"\"";
            }
        }   
    }
    else if ($2 == "alarm-trap")
    {
        if ($3 == "disable")
        {
            print "export TO_TRAP=no;";
        }
        else if ($3 == "level")
        {
            print "export TRAP_SEVERITY="$4";";
        }
    }
    else if ($2 == "merge")
    {
        if ($3 == "enable")
        {
            print "export MERGE_ENABLE=yes;";
        }
        else if ($3 == "disable")
        {
            print "export MERGE_ENABLE=no;";
        }
        else if ($3 == "fifo-size")
        {
            print "export MERGE_FIFO_SIZE="$4";";
        }
        else if ($3 == "timeout")
        {
            print "export MERGE_TIMEOUT="$4";";
        }
    }
    else if ($2 == "operate")
    {
        if ($3 == "enable")
        {
            print "export OPERATE_LOG=yes;";
        }
        else if ($3 == "disable")
        {
            print "export OPERATE_LOG=no;";
        }
    }
}' > $TMP_FILE

. $TMP_FILE
rm -fr $TMP_FILE
fi

#
# convert severity and facility
#
convert_severity()
{    
    severity=$1;
    if [ "$severity" == "emergency" ]; then
        severity="emerg";
    elif [ "$severity" == "alert" ]; then
        severity="alert";
    elif [ "$severity" == "critical" ]; then
        severity="crit";
    elif [ "$severity" == "error" ]; then
        severity="err";
    elif [ "$severity" == "warning" ]; then
        severity="warn";
    elif [ "$severity" == "notice" ]; then
        severity="notice";
    elif [ "$severity" == "information" ]; then
        severity="info";
    elif [ "$severity" == "debug" ]; then
        severity="debug";
    fi

    echo $severity;
}

#
# convert trap severity (Added by zhoucheng for trap)
#
convert_trap_severity()
{    
    severity=$1;
    if [ "$severity" == "high" ]; then
        severity="emerg";
    elif [ "$severity" == "middle" ]; then
        severity="alert";
    elif [ "$severity" == "lower" ]; then
        severity="crit";
    elif [ "$severity" == "minor" ]; then
        severity="err";
    fi

    echo $severity;
}

FILE_SEVERITY="`convert_severity $FILE_SEVERITY`"
SRV_SEVERITY="`convert_severity $SRV_SEVERITY`"
MOD_SEVERITY="`convert_severity $MOD_SEVERITY`"
TRAP_SEVERITY="`convert_trap_severity $TRAP_SEVERITY`"

#convert facility
convert_facility()
{
    TOKS="kern user mail daemon auth syslog lpr"
    TOKS=$TOKS" news uucp cron authpriv ftp"
    TOKS=$TOKS" local0 local1 local2 local3 local4 local5 local6 local7"
    VALUS="0 1 2 3 4 5 6"
    VALUS=$VALUS" 7 8 9 10 11"
    VALUS=$VALUS" 16 17 18 19 20 21 22 23"

    idx="0"
    found="no"
    for item in $TOKS; do
        if [ "$item" == "$1" ]; then
            found="yes"
            break
        fi
        idx="`expr $idx + 1`"
    done

    if [ "$found" == "no" ]; then
        echo "23"
        return
    fi

    loop="0"
    for item in $VALUS; do
        if [ "$loop" == "$idx" ]; then
            echo $item
            return
        fi

        loop="`expr $loop + 1`"
    done
    return
}

SRV_FACILITY="`convert_facility $SRV_FACILITY`"

# get timestamp macro
get_timestamp_macro()
{
    ts_format=$1
    if [ "$ts_format" == "rfc3164" ] || [ "$ts_format" == "bsd" ]; then
        echo "\$R_DATE "
    elif [ "$ts_format" == "rfc3339" ] || [ "$ts_format" == "iso" ]; then
        echo "\$R_ISODATE "
    elif [ "$ts_format" == "date" ]; then
        echo "\$R_FULLDATE "
    else
        echo ""
    fi
}

TS_MACRO="`get_timestamp_macro $TS_FORMAT`"

# Added by zhoucheng 2008-01-03 14:00, bug 3974
# judge if we need to append timefrom at the end of merged log or not
get_timefrom_yesno()
{
    ts_format=$1
    if [ "$ts_format" != "none" ]; then
        echo "no"
    else
        echo "yes"
    fi
}

MERGE_TIMEFROM="`get_timefrom_yesno $TS_FORMAT`"

#
# part 1 for options 
#
cat > $SYSLOG_NG_CFG <<EOF
#
# Configuration file for syslog-ng
#
# (C) Centec Networks Inc.  All rights reserved.
#
# Authors: Alexander Liu.

######
# options

options {
        # disable the chained hostname format in logs
        # (default is enabled)
        chain_hostnames(0);

        # the time to wait before a died connection is re-established
        # (default is 60)
        time_reopen(10);

        # the time to wait before an idle destination file is closed
        # (default is 60)
        time_reap(360);

        # timestamp
        # default is rfc3164.
        ts_format("rfc3164");

        # the number of lines buffered before written to file
        # you might want to increase this if your disk isn't catching with
        # all the log messages you get or if you want less disk activity
        # (say on a laptop)
        # (default is 0)
        #sync(0);

        # the number of lines fitting in the output queue
        log_fifo_size(2048);

        # enable or disable directory creation for destination files
        create_dirs(yes);

        # default owner, group, and permissions for log files
        # (defaults are 0, 0, 0600)
        #owner(root);
        #group(root);
        perm(0640);

        # default owner, group, and permissions for created directories
        # (defaults are 0, 0, 0700)
        #dir_owner(root);
        #dir_group(root);
        dir_perm(0755);

        # enable or disable DNS usage
        # syslog-ng blocks on DNS queries, so enabling DNS may lead to
        # a Denial of Service attack
        # (default is yes)
        use_dns(no);

        # maximum length of message in bytes
        # this is only limited by the program listening on the /dev/log Unix
        # socket, glibc can handle arbitrary length log messages, but -- for
        # example -- syslogd accepts only 1024 bytes
        # (default is 2048)
        #log_msg_size(2048);
        
        # enable merge (default is yes)
        merge_enable($MERGE_ENABLE);

        # merge fifo size (default is 1024)
        merge_fifo_size($MERGE_FIFO_SIZE);

        # merge timeout (default is 10000 millisecond)
        merge_timeout($MERGE_TIMEOUT);

        # Added by zhoucheng 2008-01-03 14:00, bug 3974
        merge_timefrom($MERGE_TIMEFROM);
};


# all known message sources
source src_all {
        # message generated by Syslog-NG
        internal();
        # standard Linux log source (this is the default place for the syslog()
        # function to send logs to)
        unix-dgram("/dev/log");
        # messages from the kernel
        file("/proc/kmsg" log_prefix("kernel: "));
        # use the above line if you want to receive remote UDP logging messages
        # (this is equivalent to the "-r" syslogd flag)
        #udp(ip(0.0.0.0) port(514));
};

filter f_file_severity { level($FILE_SEVERITY..emerg); };

filter f_srv_severity { level($SRV_SEVERITY..emerg); };

filter f_mod_severity { level($MOD_SEVERITY..emerg); };

filter f_trap_filter { level($TRAP_SEVERITY..emerg) and facility($TRAP_FACILITY); };

filter f_oper_facility { not facility(local5); };


# To logging buffer
destination d_logbuffer { unix-stream("/tmp/.logbuf_line", template("$TS_MACRO\$HOST \$PROGRAM-\$LEVEL_NUM\$CTCBRIEF: \$MSGONLY\n")); };

# To logging snmp-trap ( Added by zhoucheng for alarm log)
destination d_logtrap { unix-stream("/tmp/.logtrap", template("$MSGONLY\n")); };

EOF

#
# Part 2 for to logging file
#
SYNCSCRIPT="/usr/sbin/logsync.sh"
if [ "`uname -m`" != "ppc" ]; then
    SYNCSCRIPT="/centec_switch/sbin/logsync.sh"
fi

if [ -f /etc/debian_version ]; then
    SYNCSCRIPT="/switch/usr/sbin/logsync.sh"
fi

if [ "$TO_FILE" == "yes" ]; then
    cat >> $SYSLOG_NG_CFG <<EOF
# To logging file
destination d_logfile { file("/var/log/messages", template("$TS_MACRO\$HOST \$PROGRAM-\$LEVEL_NUM\$CTCBRIEF: \$MSGONLY\n")
        sizelimitation(524288) syncscript("$SYNCSCRIPT")); };

# Log to logging file
log
{
    source(src_all);
    filter(f_file_severity);
    destination(d_logfile);
};

EOF
fi

#
# Part 3 for logging buffer
#
cat >> $SYSLOG_NG_CFG <<EOF
# Log to logging buffer
log
{
    source(src_all);
    filter(f_mod_severity);
EOF
if [ "$OPERATE_LOG" == "no" ]; then
    cat >> $SYSLOG_NG_CFG <<EOF
    filter(f_oper_facility);
EOF
fi
cat >> $SYSLOG_NG_CFG <<EOF
    destination(d_logbuffer);
};

EOF

#
# Part 4 for logging to remote server
#

if [ "$TO_SRV" == "yes" ] && [ "$ADDR_LIST" != "" ] ; then
    cat >> $SYSLOG_NG_CFG <<EOF
# template for logging to remote server
template template_logremote { template("<\$CTCPRI>$TS_MACRO\$HOST \$PROGRAM-\$LEVEL_NUM\$CTCBRIEF: \$MSGONLY\n"); template_ctcfac($SRV_FACILITY); };

EOF

    idx="0";
    for item in $ADDR_LIST; do
        cat >> $SYSLOG_NG_CFG <<EOF
# To remote server
destination d_remote_$idx { udp("$item", template(template_logremote)); };

# Log to remote server
log
{
    source(src_all);
    filter(f_srv_severity);
    destination(d_remote_$idx);
};

EOF
    idx="`expr $idx + 1`"
    done
fi

#
# Part 5 for logging snmp-trap (Added by zhoucheng for alarm log)
#
if [ "$TO_TRAP" == "yes" ]; then
    cat >> $SYSLOG_NG_CFG <<EOF
# Log to snmp-trap
log
{
    source(src_all);
    filter(f_trap_filter);
    destination(d_logtrap);
};

EOF
fi

