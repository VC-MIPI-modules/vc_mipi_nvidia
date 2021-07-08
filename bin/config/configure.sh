#!/bin/bash

WORKING_DIR=$(dirname $PWD)
BIN_DIR=$WORKING_DIR/bin

if [[ -z $VC_MIPI ]]; then 
    VC_MIPI=$1
    CMD=$2
else
    CMD=$1
fi

# TODO: Control the configuration by command parameters
case $VC_MIPI in
    43)   . $BIN_DIR/config/config_JP4.3.sh ;;
    43u)  . $BIN_DIR/config/config_JP4.3_universal.sh ;;
    43a)  . $BIN_DIR/config/config_JP4.3_auvidea.sh ;; 
    441)  . $BIN_DIR/config/config_JP4.4.1.sh ;;
    441u)  . $BIN_DIR/config/config_JP4.4.1_universal.sh ;;
    *) 
        echo "Jetpack version not supported!"
        echo "Options: 43, 43u, 43a, 441, 441u"
        exit 
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