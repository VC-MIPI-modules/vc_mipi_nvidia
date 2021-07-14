#!/bin/bash
#
JP_VERSION=4.5_auvidea
echo "Using Jetpack Version: $JP_VERSION" 

BUILD_DIR=$WORKING_DIR/build/jp$JP_VERSION
PATCH_DIR=$WORKING_DIR/patch/$JP_VERSION
SRC_DIR=$WORKING_DIR/src/$JP_VERSION
TMP_DIR=$BUILD_DIR/downloads
GCC_DIR=$BUILD_DIR/toolchain

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v5.1/r32_release_v5.1
SRC_URL=$DEV_URL/sources/t210
SRC_FILE=public_sources.tbz2
RFS_URL=$DEV_URL/t210
RFS_FILE=tegra_linux_sample-root-filesystem_r32.5.1_aarch64.tbz2
DRV_URL=$DEV_URL/t210
DRV_FILE=jetson-210_linux_r32.5.1_aarch64.tbz2
         
export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64

CREATE_IMAGE_PARAM="-s 8G -b jetson-nano -r 300"

CAMERAS=(IMX226 IMX226C IMX296_VGL)
        