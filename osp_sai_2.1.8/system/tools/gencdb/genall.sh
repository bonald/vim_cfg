#!/bin/bash

server=`hostname`
   

#source /ctcdata/users/sw/zhangdy/script/x86pyenv.sh
export PATH=./:$PATH
export PATH=/data/centec_switch/protobuf/out/bin:$PATH
export LD_LIBRARY_PATH=/data/centec_switch/protobuf/out/lib:$LD_LIBRARY_PATH

print_usage_and_exit()
{
    echo "Usage: ./gencdb.sh [copy]"
    echo "       copy: copy to code directory"
    exit 1
}

case $# in
0)
    copy=no
    ;;
1)
    copy=yes
    ;;
*)
    print_usage_and_exit
esac

echo "##### gen_node.py #####"
gen_node.py
if [ "$?" != "0" ]; then
    echo "gen_node.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

echo "##### gen_define.py #####"
gen_define.py
if [ "$?" != "0" ]; then
    echo "gen_define.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

echo "##### gen_api.py #####"
gen_api.py
if [ "$?" != "0" ]; then
    echo "gen_api.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

echo "##### gen_store.py #####"
gen_store.py
if [ "$?" != "0" ]; then
    echo "gen_store.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

echo "##### gen_check.py #####"
gen_check.py
if [ "$?" != "0" ]; then
    echo "gen_check.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

echo "##### gen_pb.py #####"
gen_pb.py
if [ "$?" != "0" ]; then
    echo "gen_pb.py failed and stop script!!!!!!"
    exit $?
fi
echo ""

if [ $copy == "yes" ]; then
    echo "##### gen_copy.py #####"
    gen_copy.py
    echo ""
fi

echo "###### gen_cdb_define.py ####"
gen_cdb_define.py
