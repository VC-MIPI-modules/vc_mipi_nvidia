#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
source configure.sh

export KERNEL_SOURCE=$L4T_BUILD_DIR/Linux_for_Tegra/source/public/kernel/kernel-4.9
export CROSS_COMPILE=$L4T_GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64
export TEGRA_KERNEL_OUT=$L4T_BUILD_DIR/l4t-kernel
export TEGRA_MODULES_OUT=$L4T_BUILD_DIR/

echo "KERNEL_SOURCE=$KERNEL_SOURCE"
echo "CROSS_COMPILE=$CROSS_COMPILE"
echo "LOCALVERSION=$LOCALVERSION"
echo "ARCH=$ARCH"
echo "TEGRA_KERNEL_OUT=$TEGRA_KERNEL_OUT"

if [ ! -d "$L4T_GCC_DIR" ] ; then
    mkdir $TEGRA_KERNEL_OUT
fi

if [ -d "$KERNEL_SOURCE" ] ; then
    cd $KERNEL_SOURCE
    #make ARCH=arm64 O=$TEGRA_KERNEL_OUT tegra_defconfig
    #make -C $KERNEL_SOURCE mrproper
    #make -C $KERNEL_SOURCE O=$TEGRA_KERNEL_OUT tegra_defconfig
    #make -C $KERNEL_SOURCE O=$TEGRA_KERNEL_OUT --output-sync=target Image
    #make -C $KERNEL_SOURCE O=$TEGRA_KERNEL_OUT --output-sync=target dtbs
    make -C $KERNEL_SOURCE O=$TEGRA_KERNEL_OUT --output-sync=target modules
    #make -C $KERNEL_SOURCE O=$TEGRA_KERNEL_OUT INSTALL_MOD_PATH=$KERNEL_MODULES_OUT modules_install 

    #cp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/Image $L4T_BUILD_DIR/Linux_for_Tegra/kernel/
    #cp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/dts/*.dtb $L4T_BUILD_DIR/Linux_for_Tegra/kernel/dtb/
else
    echo "You habe to setup kernel sources first!"
fi