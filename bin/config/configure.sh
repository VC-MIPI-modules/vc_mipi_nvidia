#!/bin/bash

WORKING_DIR=$(dirname $PWD)
BIN_DIR=$WORKING_DIR/bin

if [[ -z $VC_MIPI ]]; then 
    VC_MIPI=$1
    CMD=$2
else
    CMD=$1
fi

print_jetpack_version_definition() {
    echo "Jetpack version not $1!"
    echo "Please set VC_MIPI to one of the possible options."
    echo "  $ export VC_MIPI=441"
    echo "Options: 43, 43a, 441, 45a"
}

if [[ -z $VC_MIPI ]]; then
    print_jetpack_version_definition "defined"
    exit 1
fi

case $VC_MIPI in
    43)   . $BIN_DIR/config/config_JP4.3.sh ;;
    43a)  . $BIN_DIR/config/config_JP4.3_auvidea.sh ;; 
    441)  . $BIN_DIR/config/config_JP4.4.1.sh ;;
    45a)  . $BIN_DIR/config/config_JP4.5_auvidea.sh ;;
    *) 
        print_jetpack_version_definition "supported"
        exit 1
        ;;
esac

KERNEL_SOURCE=$BUILD_DIR/Linux_for_Tegra/source/public
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules

TARGET_IP=nano
TARGET_USER=peter
TARGET_SHELL="ssh $TARGET_USER@$TARGET_IP"
# TARGET_BOARD=jetson-nano-devkit
TARGET_BOARD=jetson-nano-emmc