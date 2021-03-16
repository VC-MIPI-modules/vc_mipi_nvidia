#!/bin/bash

WORKING_DIR=$(dirname $PWD)
BIN_DIR=$WORKING_DIR/bin

# TODO: Control the configuration by command parameters
case $1 in
    43)  . $BIN_DIR/config/config_JP4.3.sh ;;
    43l) . $BIN_DIR/config/config_JP4.3_liquify.sh ;;
    43a) . $BIN_DIR/config/config_JP4.3_test_auvidea.sh ;; 
    441) . $BIN_DIR/config/config_JP4.4.1.sh ;;
    *) 
        echo "Jetpack version not supported!"
        echo "Options: 43, 43l, 43a, 441"
        exit 
        ;;
esac

KERNEL_SOURCE=$BUILD_DIR/Linux_for_Tegra/source/public
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules