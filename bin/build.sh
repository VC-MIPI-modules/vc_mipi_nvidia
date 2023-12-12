#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Build kernel image, modules and device tree."
        echo ""
        echo "Supported options:"
        echo "-a, --all                 Build kernel image, modules and device tree"
        echo "-d, --dt                  Build device tree"
        echo "-h, --help                Show this help text"
        echo "-k, --kernel              Build kernel image"
        echo "-m, --modules             Build kernel modules"
}

configure() {
        . config/configure.sh
}

patch_kernel() {
        echo "Copying driver sources into kernel sources ..."
        cp -Ruv $DRIVER_DIR/* $DRIVER_DST_DIR
        copy_dtsi_files
}

configure_kernel() {
        cd $KERNEL_SOURCE
        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) tegra_defconfig
}

build_kernel() {
        echo "Build kernel ..."
        cd $KERNEL_SOURCE
        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target Image
        cp -rfv $KERNEL_OUT/arch/arm64/boot/Image $BSP_DIR/Linux_for_Tegra/kernel/
}

build_modules() {
        echo "Build kernel modules ..."
        cd $KERNEL_SOURCE
        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target modules
        make -C $KERNEL_DIR O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 
        sudo cp -arfv $MODULES_OUT/lib $MODULES_BSP
}

build_nvidia_driver() {
        echo "Build nvidia display driver ..."
        echo "KERNEL_SOURCE: $KERNEL_SOURCE"
        echo "KERNEL_OUT: $KERNEL_OUT"
        echo "KERNEL_DIR: $KERNEL_DIR"
        echo "MODULES_OUT: $MODULES_OUT"
        echo "MODULES_BSP: $MODULES_BSP"

#        cd $KERNEL_SOURCE
        KERNEL_COMP=$KERNEL_SOURCE/kernel/kernel-5.10
#        cd $KERNEL_COMP

        echo "KERNEL_COMP: $KERNEL_COMP"
        NV_SRC=$KERNEL_SOURCE/NVIDIA-kernel-module-source-TempVersion

        echo "WORKING_DIR: $WORKING_DIR"
        echo "NV_SRC: $NV_SRC"
        echo "CROSS_COMPILE: $CROSS_COMPILE "
        echo "------------------"

        mkdir -p $BSP_DIR/Linux_for_Tegra/backup_display_driver
        ls -la $MODULES_BSP/lib/modules/

        exit 1

#        cd $KERNEL_SOURCE/NVIDIA-kernel-module-source-TempVersion
        cd $NV_SRC

#        SYSOUT=$KERNEL_SOURCE/kernel_out \

        make \
        modules \
        SYSSRC=$KERNEL_COMP \
        SYSOUT=$KERNEL_OUT \
        CC=${CROSS_COMPILE}gcc \
        LD=${CROSS_COMPILE}ld.bfd \
        AR=${CROSS_COMPILE}ar \
        CXX=${CROSS_COMPILE}g++ \
        OBJCOPY=${CROSS_COMPILE}objcopy \
        TARGET_ARCH=aarch64 \
        ARCH=arm64
}

build_device_tree() {
        echo "Build device tree ..."
        cd $KERNEL_SOURCE
        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target dtbs
        cp -rfv $DTB_OUT/*.dtb $BSP_DIR/Linux_for_Tegra/kernel/dtb/
}

set -e

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -a|--all)
                configure
                patch_kernel
                configure_kernel
                build_kernel
                build_modules	
                build_device_tree
                exit 0
                ;;
        -d|--dt)
                configure
                patch_kernel
                configure_kernel
                build_device_tree
                exit 0
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                configure
                patch_kernel
                configure_kernel
                build_kernel
                exit 0
                ;;
        -m|--modules)
                configure
                patch_kernel
                configure_kernel
                build_modules
                exit 0
                ;;
        -n)
                configure
                build_nvidia_driver
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

usage
exit 1