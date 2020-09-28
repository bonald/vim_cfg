#!/bin/bash

set -o errexit


echo "Create RPCAPI Image Binary..."

if [ $# != 2 ]; then
    echo "usage: pkg_rpcapi.sh work-dir rpcapi-image-name";
    exit;
fi

tmpdir=""
tmpname=""
curdir=`pwd`

cleanup()
{
    if [ "$tmpdir" != "" ]; then
        rm -fr $tmpdir
    fi
    if [ "$tmpname" != "" ]; then
        rm -fr $tmpname
    fi
    cd $curdir
}

trap 'cleanup' EXIT
trap 'echo "Command interrupted by user"; exit 1;' SIGINT SIGKILL SIGHUP

workdir=$1
imgname=$2

if [ ! -d "$workdir" ]; then
    echo "Invalid working directory."
    exit 1;
fi

touch $imgname
if [ "$?" != "0" ]; then
    echo "Invalid rpcapi image name."
    exit 1;
fi

cd $workdir
cd ../

tar czf $imgname $workdir
cd $curdir

echo "Create RPCAPI Image Binary Success"
exit 0