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

        mkdir -p $DRIVER_DST_DIR
        cp -Ruv $DRIVER_DIR/* $DRIVER_DST_DIR

        copy_dtsi_files
}

#configure_kernel() {
#        if [[ "36.2.0" == $VC_MIPI_BSP ]]
#        then
#                return 0
#        fi
#
#        cd $KERNEL_SOURCE
#
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) tegra_defconfig
#}

configure_kernel() {
        case $VC_MIPI_BSP in
        36.4.0|36.2.0)
                return 0
                ;;
        esac

        cd $KERNEL_SOURCE

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) tegra_defconfig
}


build_kernel() {
        cd $KERNEL_SOURCE

        L4T_build_kernel
}

build_nvidia_driver() {
        L4T_build_nvidia_driver
}

build_modules() {
        echo "Build kernel modules ..."

        cd $KERNEL_SOURCE
        L4T_build_modules

        cd $KERNEL_SOURCE
        L4T_build_nvidia_driver

        cd $KERNEL_SOURCE
        L4T_install_modules
}

build_device_tree() {
        cd $KERNEL_SOURCE

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