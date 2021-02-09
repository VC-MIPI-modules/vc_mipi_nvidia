#!/bin/bash
#
# Read this for more instructions
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
source l4t_configure.sh

if [ -d "$KERNEL_SOURCE" ] ; then
    cd $KERNEL_SOURCE
    make ARCH=arm64 O=$TEGRA_KERNEL_OUT
else
    echo "You habe to setup kernel sources first!"
fi