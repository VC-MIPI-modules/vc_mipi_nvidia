#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
. config/configure.sh $1

echo "CROSS_COMPILE=$CROSS_COMPILE"
echo "LOCALVERSION=$LOCALVERSION"
echo "ARCH=$ARCH"
echo "KERNEL_SOURCE=$KERNEL_SOURCE"
echo "KERNEL_OUT=$KERNEL_OUT"

#make -C $KERNEL_SOURCE clean
#make -C $KERNEL_SOURCE mrproper
#make -C $KERNEL_SOURCE distclean
make -C $KERNEL_SOURCE O=$KERNEL_OUT tegra_defconfig
make -C $KERNEL_SOURCE O=$KERNEL_OUT
    
cp $KERNEL_OUT/arch/arm64/boot/Image $BUILD_DIR/Linux_for_Tegra/kernel/
cp $KERNEL_OUT/arch/arm64/boot/dts/*.dtb $BUILD_DIR/Linux_for_Tegra/kernel/dtb/

sudo make -C $KERNEL_SOURCE O=$KERNEL_OUT modules_install INSTALL_MOD_PATH=$BUILD_DIR/Linux_for_Tegra/rootfs/