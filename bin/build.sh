#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
#
clear

. config/configure.sh $1 $2

if [[ -z $CMD ]]; then 
    ./patch.sh $1 f
else
    ./patch.sh $1 
fi 

#if [[ -d "$KERNEL_SOURCE/build" ]]; then
#    sudo rm -R $KERNEL_SOURCE/build
#fi
#if [[ -d "$KERNEL_SOURCE/modules" ]]; then
#    sudo rm -R $KERNEL_SOURCE/modules
#fi

cd $KERNEL_SOURCE
#make -C kernel/kernel-4.9/ O=$KERNEL_OUT mrproper
make -C kernel/kernel-4.9/ O=$KERNEL_OUT tegra_defconfig

if [[ -z $CMD ]]; then 
        make -C kernel/kernel-4.9/ O=$KERNEL_OUT -j$(nproc)
        #create_modules
fi
if [[ $CMD == "a" || $CMD == "k" ]]; then 
    echo "Build Kernel ..."
    make -C kernel/kernel-4.9/ O=$KERNEL_OUT -j$(nproc) --output-sync=target Image
    cp -rfv $KERNEL_OUT/arch/arm64/boot/Image $BUILD_DIR/Linux_for_Tegra/kernel/
fi
if [[ $CMD == "a" || $CMD == "m" ]]; then 
    echo "Build Modules ..."      
    make -C kernel/kernel-4.9/ O=$KERNEL_OUT -j$(nproc) --output-sync=target modules
    make -C kernel/kernel-4.9/ O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 
    sudo cp -arfv $MODULES_OUT/lib $BUILD_DIR/Linux_for_Tegra/rootfs/
fi
if [[ $CMD == "a" || $CMD == "d" ]]; then 
    echo "Build Device Tree ..."
    make -C kernel/kernel-4.9/ O=$KERNEL_OUT  --output-sync=target dtbs
    cp -rfv $KERNEL_OUT/arch/arm64/boot/dts/*.dtb $BUILD_DIR/Linux_for_Tegra/kernel/dtb/
fi
if [[ $CMD == "demo" ]]; then 
    cd $WORKING_DIR/src/vcmipidemo/linux
    make
    mv vcmipidemo $WORKING_DIR/test
    mv vcimgnetsrv $WORKING_DIR/test
fi