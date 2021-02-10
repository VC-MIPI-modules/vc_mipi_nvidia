#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/flashing.html
#
source configure.sh

cp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/dts/*.dtb $L4T_BUILD_DIR/Linux_for_Tegra/kernel/dtb/
source $L4T_BUILD_DIR/Linux_for_Tegra/flash.sh -k DTB jetson-nano-qspi-sd mmcblk0p1
