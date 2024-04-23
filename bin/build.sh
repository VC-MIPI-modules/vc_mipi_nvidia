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
#bazo modify
#        cp -Ruv $DRIVER_DIR/* $DRIVER_DST_DIR
#        cp -Ruv $DRIVER_DIR/* $DRIVER_OOT_DST_DIR

        cp -Ruv $DRIVER_DIR/* $DRIVER_DST_DIR

        copy_dtsi_files
}

configure_kernel() {
        if [[ "36.2.0" == $VC_MIPI_BSP ]]
        then
                return 0
        fi

        cd $KERNEL_SOURCE

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) defconfig
}

build_kernel() {
        echo "Build kernel ..."

        echo "KERNEL_SOURCE: $KERNEL_SOURCE"
        echo "KERNEL_DIR: $KERNEL_DIR"
        echo "KERNEL_OUT: $KERNEL_OUT"
        echo "MODULES_OUT: $MODULES_OUT"
        echo "KERNEL_HEADERS: $KERNEL_HEADERS"
        echo "ROOTFS_DIR: $ROOTFS_DIR"
        echo "CROSS_COMPILE: $CROSS_COMPILE"
        
        cd $KERNEL_SOURCE

#bazo modify
        L4T_build_kernel
}

build_nvidia_driver() {
        L4T_build_nvidia_driver
}

build_modules() {
        echo "Build kernel modules ..."

#bazo modify
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target modules
#
#        build_nvidia_driver
#
#        cd $KERNEL_SOURCE
#        make -C $KERNEL_DIR O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 
#        sudo cp -arfv $MODULES_OUT/lib $MODULES_BSP

        echo "KERNEL_SOURCE: $KERNEL_SOURCE"
        echo "KERNEL_DIR: $KERNEL_DIR"
        echo "KERNEL_OUT: $KERNEL_OUT"
        echo "MODULES_OUT: $MODULES_OUT"
        echo "KERNEL_HEADERS: $KERNEL_HEADERS"
        echo "ROOTFS_DIR: $ROOTFS_DIR"
        echo "CROSS_COMPILE: $CROSS_COMPILE"


        cd $KERNEL_SOURCE
        L4T_build_modules

        cd $KERNEL_SOURCE
        L4T_build_nvidia_driver

        cd $KERNEL_SOURCE
        L4T_install_modules
}

build_device_tree() {
        echo "Build device tree ..."

        cd $KERNEL_SOURCE
#bazo modify
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target dtbs
#        cp -rfv $DTB_OUT/*.dtb $BSP_DIR/Linux_for_Tegra/kernel/dtb/
        
#        make dtbs

#        cp nvidia-oot/device-tree/platform/generic-dts/dtbs/* \
#        $BSP_DIR/Linux_for_Tegra/kernel/dtb/

        L4T_build_device_tree
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
        -n)
                configure
                build_nvidia_driver
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