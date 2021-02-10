#!/bin/bash

export L4T_WD=$(dirname $PWD)
export L4T_BIN_DIR=$L4T_WD/bin
export L4T_BUILD_DIR=$L4T_WD/build
export L4T_TMP_DIR=$L4T_WD/tmp

export L4T_GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
export L4T_GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
export L4T_GCC_DIR=$L4T_BUILD_DIR/l4t-gcc

export L4T_DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r32_Release_v4.4/r32_Release_v4.4-GMC3

export L4T_SRC_URL=$L4T_DEV_URL/Sources/T210
export L4T_SRC_FILE=public_sources.tbz2
export L4T_SRC_DIR=$L4T_BUILD_DIR

export L4T_RFS_URL=$L4T_DEV_URL/T210
export L4T_RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.4.4_aarch64.tbz2
export L4T_RFS_DIR=$L4T_BUILD_DIR/Linux_for_Tegra/rootfs

export L4T_DRV_URL=$L4T_DEV_URL/T210
export L4T_DRV_FILE=Tegra210_Linux_R32.4.4_aarch64.tbz2
export L4T_DRV_DIR=$L4T_BUILD_DIR

if [ ! -d "$L4T_BUILD_DIR" ] ; then
    mkdir $L4T_BUILD_DIR
fi
if [ ! -d "$L4T_TMP_DIR" ] ; then
    mkdir $L4T_TMP_DIR
fi

export KERNEL_SOURCE=$L4T_BUILD_DIR/Linux_for_Tegra/source/public/kernel/kernel-4.9
export CROSS_COMPILE=$L4T_GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export TEGRA_KERNEL_OUT=$L4T_BUILD_DIR/l4t-kernel

if [ ! -d "$L4T_GCC_DIR" ] ; then
    mkdir $TEGRA_KERNEL_OUT
fi
