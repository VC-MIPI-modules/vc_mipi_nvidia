#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
if [[ -z $2 ]]; then
    . config/configure.sh $1
else 
    . patch.sh $1 $2
fi

START_TIME=$(date +%s.%N)

#if [[ -d "$KERNEL_SOURCE/build" ]]; then
#    sudo rm -R $KERNEL_SOURCE/build
#fi
#if [[ -d "$KERNEL_SOURCE/modules" ]]; then
#    sudo rm -R $KERNEL_SOURCE/modules
#fi

cd $KERNEL_SOURCE
make -C kernel/kernel-4.9/ O=$KERNEL_OUT tegra_defconfig
make -C kernel/kernel-4.9/ O=$KERNEL_OUT -j2 --output-sync=target Image
make -C kernel/kernel-4.9/ O=$KERNEL_OUT  --output-sync=target dtbs
make -C kernel/kernel-4.9/ O=$KERNEL_OUT -j2 --output-sync=target modules
make -C kernel/kernel-4.9/ O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 

cp -rfv $KERNEL_OUT/arch/arm64/boot/Image $BUILD_DIR/Linux_for_Tegra/kernel/
cp -rfv $KERNEL_OUT/arch/arm64/boot/dts/*.dtb $BUILD_DIR/Linux_for_Tegra/kernel/dtb/
sudo cp -arfv $MODULES_OUT/lib $BUILD_DIR/Linux_for_Tegra/rootfs/

END_TIME=$(date +%s.%N)
ELAPSED_TIME=$(echo "$END_TIME - $START_TIME" | bc)
echo "Elapsed time: $ELAPSED_TIME"