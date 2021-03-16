#!/bin/bash
#
JP_VERSION=4.3_liquify
echo "Using Jetpack Version: $JP_VERSION" 

BUILD_DIR=$WORKING_DIR/build/jp$JP_VERSION
SRC_DIR=$WORKING_DIR/src/$JP_VERSION
TMP_DIR=$BUILD_DIR/downloads
GCC_DIR=$BUILD_DIR/toolchain

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
DEV_URL=https://developer.nvidia.com/embedded/dlc/r32-3-1_Release_v1.0/
SRC_URL=$DEV_URL/Sources/T210
SRC_FILE=public_sources.tbz2
RFS_URL=$DEV_URL/t210ref_release_aarch64
RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.3.1_aarch64.tbz2
DRV_URL=$DEV_URL/t210ref_release_aarch64
DRV_FILE=Tegra210_Linux_R32.3.1_aarch64.tbz2
         
export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64

CREATE_IMAGE_PARAM="-s 8G -b jetson-nano -r 300"

CAMERAS=(VC_CAMERA)