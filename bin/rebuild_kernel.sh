#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
source configure.sh

if [ -d "$KERNEL_SOURCE" ] ; then
    cd $KERNEL_SOURCE
    make ARCH=arm64 O=$TEGRA_KERNEL_OUT
    cp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/Image $L4T_BUILD_DIR/Linux_for_Tegra/kernel/
    cp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/dts/*.dtb $L4T_BUILD_DIR/Linux_for_Tegra/kernel/dtb/
else
    echo "You habe to setup kernel sources first!"
fi