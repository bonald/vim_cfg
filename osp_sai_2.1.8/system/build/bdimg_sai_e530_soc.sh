########################################################################
# Copyright (C) 2004-2015 Centec Networks Co.,Inc All rights reserved.
#
# @author ZHAO YI
########################################################################

PROG=`basename $0`
# default values
version='r'
debian='n'
python='n'
force_build_kernel='Y'
cpu_arch='ctc5236'
board_type='e530'
chip_type='tm'
openflow='n'
linux_version='new'
product='sai'
license='no'
export DEBIAN_PACKAGE_NAME=osp_e530_debian

set -o errexit

print_usage_and_exit()
{
    echo "Usage: ./bdimg_osp_e530.sh [OPTIONS]"
    echo ""
    echo "Available options are listed: " 
    echo "       -r, --release             release version (default)"
    echo "       -d, --debug               debug version"
    echo "       -D, --debian              build Debian package"
#    echo "       -P, --python              python interpreter support"
#    echo "       -O, --openflow            OpenFlow feature embed"
#    echo "       -f, --force-build-kernel  force to rebuild kernel and busybox (default: ${force_build_kernel})"
    echo "       -h, --help                display this help and exit"
    echo "       -l   --license"
    exit 1
}

ARGS=`getopt  --long help,debug,release,debian,python,force-build-kernel,license --options h,d,r,D,P,f,l --name $PROG -- "$@"`
eval set -- "$ARGS"
while true; do
    case "$1" in
        -h | --help)
            print_usage_and_exit ;;
        -r | --release)
            version=r; shift ;;
        -d | --debug)
            version=d; shift ;;
        -D | --debian)
            debian="y"; shift ;;
        -P | --python)
            python="y"; shift ;;
        -l | --license)
            license="yes"; shift ;;
        -f | --force-build-kernel)
            force_build_kernel="Y"; shift ;;
        --)
            shift; break;; # end of options
        *)
            print_usage_and_exit ;; 
    esac
done

if [ $chip_type != 'gb' ] && [ $chip_type != 'gg' ] && [ $chip_type != 'd2' ] && [ $chip_type != 'tm' ]; then
    echo "Invalid chip_type: $chip_type"
    print_usage_and_exit
fi

if [ ${linux_version} != "new" ] && [ ${linux_version} != "old" ]; then
    echo "Invalid linux_version: ${linux_version}"
    print_usage_and_exit
fi

# Generate and export environment variables
export OSP_BUILD="${cpu_arch}"
if [ $version == "r" ]; then
    export VER=r
else
    export VER=d
fi

export ENABLE_DEBIAN_LINUX=$debian
export ENABLE_PYTHON=$python
export DEBIAN_ARCH="arm64"
export ENABLE_OPENFLOW=$openflow
export LINUX_VERSION=$linux_version

BUILD_DIR=`pwd`
cd ../$CUR_DIR
# Compile the source codes
./createenv.sh ${cpu_arch} ${board_type} ${version} ${force_build_kernel} ${chip_type} ${linux_version} ${product} ${license}
make
