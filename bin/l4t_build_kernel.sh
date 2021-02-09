#!/bin/bash
#
# Read this for more instructions
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
source l4t_configure.sh

source $L4T_BIN_DIR/l4t_setup_toolchain.sh
source $L4T_BIN_DIR/l4t_setup_kernel_sources.sh
source $L4T_BIN_DIR/l4t_patch_kernel_sources.sh

echo "KERNEL_SOURCE=$KERNEL_SOURCE"
echo "CROSS_COMPILE=$CROSS_COMPILE"
echo "LOCALVERSION=$LOCALVERSION"
echo "TEGRA_KERNEL_OUT=$TEGRA_KERNEL_OUT"

if [ -d "$KERNEL_SOURCE" ] ; then
    cd $KERNEL_SOURCE
    make ARCH=arm64 O=$TEGRA_KERNEL_OUT tegra_defconfig
    make ARCH=arm64 O=$TEGRA_KERNEL_OUT
else
    echo "You habe to setup kernel sources first!"
fi