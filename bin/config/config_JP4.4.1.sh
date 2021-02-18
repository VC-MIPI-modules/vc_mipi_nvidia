#!/bin/bash

BUILD_DIR=$WORKING_DIR/jp4.4.1
TMP_DIR=$BUILD_DIR/downloads
GCC_DIR=$BUILD_DIR/toolchain

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r32_Release_v4.4/r32_Release_v4.4-GMC3
SRC_URL=$DEV_URL/Sources/T210
SRC_FILE=public_sources.tbz2
RFS_URL=$DEV_URL/T210
RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.4.4_aarch64.tbz2
DRV_URL=$DEV_URL/T210
DRV_FILE=Tegra210_Linux_R32.4.4_aarch64.tbz2

export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64
KERNEL_SOURCE=$BUILD_DIR/Linux_for_Tegra/source/public/kernel/kernel-4.9
KERNEL_OUT=$BUILD_DIR/build

IMAGE_FILE=jetson-nano-jp441-sd-card-image.img