#!/bin/bash

WORKING_DIR=$(dirname $PWD)
BIN_DIR=$WORKING_DIR/bin

print_jetpack_version_definition() {
    echo "Jetpack version not $1!"
    echo "Please set VC_MIPI to one of the possible options."
    echo "  $ export VC_MIPI=45a"
    echo "Options:"
    echo "  43  : Jetpack 4.3   for NVIDIA Dev Kit B01"
    echo "  43a : Jetpack 4.3   for Auvidea JNX30"
    echo "  441 : Jetpack 4.4.1 for NVIDIA Dev Kit B01"
    echo "  45a : Jetpack 4.5   for Auvidea JNX30"
    echo "  46a : Jetpack 4.6   for Auvidea JNX30"
}

if [[ -z $VC_MIPI ]]; then
    print_jetpack_version_definition "defined"
    exit 1
fi

PATCH=
DRIVER=

case $VC_MIPI in
    43) 
        DRIVER=4.3
        . $BIN_DIR/config/l4t/32.3.1.sh 
        ;;
    43a)
        PATCH=auvidea_JNX30_v1
        DRIVER=4.3_auvidea
        . $BIN_DIR/config/l4t/32.3.1.sh 
        ;;
    441)
        DRIVER=4.4.1
        . $BIN_DIR/config/l4t/32.4.4.sh 
        ;;
    45a)
        PATCH=auvidea_JNX30_v2
        DRIVER=4.5_auvidea
        . $BIN_DIR/config/l4t/32.5.1.sh 
        ;;
    46a)
        PATCH=auvidea_JNX30_v2
        DRIVER=4.5_auvidea
        . $BIN_DIR/config/l4t/32.6.1.sh 
        ;;
    *) 
        print_jetpack_version_definition "supported"
        exit 1
        ;;
esac

BUILD_DIR=$WORKING_DIR/build
PATCH_DIR=$WORKING_DIR/patch/$PATCH
SRC_DIR=$WORKING_DIR/src/$DRIVER
TMP_DIR=$BUILD_DIR/downloads
GCC_DIR=$BUILD_DIR/toolchain
KERNEL_SOURCE=$BUILD_DIR/Linux_for_Tegra/source/public
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64

CREATE_IMAGE_PARAM="-s 8G -b jetson-nano -r 300"

TARGET_IP=192.168.2.18
TARGET_USER=peter
TARGET_SHELL="ssh $TARGET_USER@$TARGET_IP"

# TARGET_BOARD=jetson-nano-devkit
TARGET_BOARD=jetson-nano-emmc