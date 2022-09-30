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
        cp -Ruv $DT_CAM_FILE $DT_CAM_FILE_DST_DIR
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
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

usage
exit 1