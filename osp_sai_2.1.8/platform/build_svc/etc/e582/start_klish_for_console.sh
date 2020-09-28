#! /bin/sh
export PYTHONPATH='/usr/local/ctc_python/:/usr/local/ctc_python/lib/python2.7/:/usr/local/ctc_python/lib/python2.7/site-packages/:/usr/local/ctc_python/lib/python2.7/lib-dynload/'
export PYTHONHOME='/usr/local/ctc_python/'
SWITCH_DIR=/switch
export PATH=$SWITCH_DIR/bin:$SWITCH_DIR/usr/sbin:$SWITCH_DIR/usr/bin:$PATH
export LD_LIBRARY_PATH=$SWITCH_DIR/lib:$LD_LIBRARY_PATH:/lib64/:/usr/lib/:/lib/
export PATH=$PYTHONHOME/bin/:$PATH
export LD_LIBRARY_PATH=$PYTHONPATH:$LD_LIBRARY_PATH:/lib64/:/usr/lib/:/lib/
export PATH=$PATH:/switch/usr/local/openvswitch/bin

ulimit -c unlimited

if [ "`find /switch -name klish 2>/dev/null`" != "" ]; then
    klish
else
    su - switch
fi
