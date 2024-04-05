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
        cp -Ruv $DRIVER_DIR/* $DRIVER_OOT_DST_DIR
#        copy_dtsi_files
}

configure_kernel() {
        cd $KERNEL_SOURCE
        pwd
#bazo modify
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) tegra_defconfig

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) defconfig
}

build_kernel() {
        echo "Build kernel ..."
#bazo modify
#        cd $KERNEL_SOURCE
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target Image
#        cp -rfv $KERNEL_OUT/arch/arm64/boot/Image $BSP_DIR/Linux_for_Tegra/kernel/

        echo "KERNEL_SOURCE: $KERNEL_SOURCE"
        echo "KERNEL_DIR: $KERNEL_DIR"
        echo "KERNEL_OUT: $KERNEL_OUT"
        echo "MODULES_OUT: $MODULES_OUT"
        echo "KERNEL_HEADERS: $KERNEL_HEADERS"
        echo "ROOTFS_DIR: $ROOTFS_DIR"
        echo "CROSS_COMPILE: $CROSS_COMPILE"
        
        cd $KERNEL_SOURCE
        
        make -C kernel

        export INSTALL_MOD_PATH=$ROOTFS_DIR/
        sudo -E make install -C kernel

        cp kernel/kernel-jammy-src/arch/arm64/boot/Image \
        $BSP_DIR/Linux_for_Tegra/kernel/Image

}

# Info:
# Makefile is looking into the kernel config file.
# If the kernel config file has been changed, the display driver has to be rebuilt!
build_nvidia_driver() {
        # checking for Orin ...
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                echo "Build NVIDIA display driver ..."
                ;;
        *)
                return 0
                ;;
        esac

        cd $KERNEL_SOURCE

        NVDD_DIR=""
        case $VC_MIPI_BSP in
        35.1.0|35.2.1|35.3.1)
                NVDD_DIR=NVIDIA-kernel-module-source-TempVersion
                ;;
        36.2.0)
                NVDD_DIR=nvdisplay
                ;;
        *)
                return 1
                ;;
        esac

        if [ ! -d $NVDD_DIR ]
        then
                echo "Could not find NVIDIA display driver source directory ${NVDD_DIR}! (pwd $(pwd))"
                exit 1
        fi

        cd $NVDD_DIR

#        KERNEL_COMP=$KERNEL_SOURCE/kernel/kernel-5.10
        KERNEL_COMP=$KERNEL_SOURCE/$KERNEL_DIR

        # Building display driver ...
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

        # Stripping modules ...
        NVDD_MOD_DIR=kernel-open
        NVDD_MOD_ARRAY=('nvidia.ko' 'nvidia-drm.ko' 'nvidia-modeset.ko')
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${NVDD_MOD_DIR}/${modfile} ]
                then
                        echo "Could not find NVIDIA display driver module ${NVDD_MOD_DIR}/${modfile}! (pwd $(pwd))"
                        exit 1
                fi

                echo "stripping ${NVDD_MOD_DIR}/${modfile} ..."
                ${CROSS_COMPILE}strip --strip-unneeded ${NVDD_MOD_DIR}/${modfile}
        done

        # Assembling version ...
        DIST_VERSION="$(awk '/^VERSION = / { print $3 }' ${KERNEL_COMP}/Makefile)"
        DIST_PATCHLEVEL="$(awk '/^PATCHLEVEL = / { print $3 }' ${KERNEL_COMP}/Makefile)"
        DIST_SUBLEVEL="$(awk '/^SUBLEVEL = / { print $3 }' ${KERNEL_COMP}/Makefile)"

        DIST_VERSION_COMP=${DIST_VERSION}.${DIST_PATCHLEVEL}.${DIST_SUBLEVEL}${LOCALVERSION}

        NVDD_DEST_RFS_DIR=${MODULES_BSP}/lib/modules/${DIST_VERSION_COMP}/extra/opensrc-disp
        NVDD_DEST_DIR=${MODULES_OUT}/lib/modules/${DIST_VERSION_COMP}/extra/opensrc-disp
        mkdir -p $NVDD_DEST_DIR

        if [ ! -d $NVDD_DEST_DIR ]
        then
                echo "Could not find NVIDIA display driver module destination directory ${NVDD_DEST_DIR}!"
                exit 1
        fi

        # Backup original display modules ...
        BACKUP_NVDD_DIR=${BSP_DIR}/Linux_for_Tegra/backup_display_driver
        if [ ! -d ${BACKUP_NVDD_DIR} ]
        then
                mkdir -p ${BACKUP_NVDD_DIR}
        fi

        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${BACKUP_NVDD_DIR}/${modfile} ]
                then 
                        cp -v ${NVDD_DEST_RFS_DIR}/${modfile} ${BACKUP_NVDD_DIR}
                fi
        done

        # Copy newly generated display driver modules ...
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                sudo cp -v ${NVDD_MOD_DIR}/${modfile} ${NVDD_DEST_DIR}
        done

        # Executable for signing of modules
        # This file will be generated during kernel build.
        SIGN_FILE=${KERNEL_OUT}/scripts/sign-file
        if [ ! -e ${SIGN_FILE} ]
        then
                echo "Could not find kernel signing tool ${SIGN_FILE}! (pwd $(pwd))"
                exit 1
        fi

        # Default kernel signing key file
        # This file will be generated during kernel build.
        SIGN_X509_KEY=${KERNEL_OUT}/certs/signing_key.x509
        if [ ! -e ${SIGN_X509_KEY} ]
        then
                echo "Could not find kernel signing key ${SIGN_X509_KEY}! (pwd $(pwd))"
                exit 1
        fi

        # Default private signing key file
        # This file will be generated during kernel build.
        SIGN_PRIV_KEY=${KERNEL_OUT}/certs/signing_key.pem
        if [ ! -e ${SIGN_PRIV_KEY} ]
        then
                echo "Could not find private signing key ${SIGN_PRIV_KEY}! (pwd $(pwd))"
                exit 1
        fi

        echo "Signing NVIDIA display driver ..."
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${NVDD_DEST_DIR}/${modfile} ]
                then
                        echo "Could not find NVIDIA display driver module ${NVDD_DEST_DIR}/${modfile} in destination directory! (pwd $(pwd))"
                        exit 1
                fi
                # Applying signing command ...
                sudo ${SIGN_FILE} sha512 ${SIGN_PRIV_KEY} ${SIGN_X509_KEY} ${NVDD_DEST_DIR}/${modfile}
        done

        #bazo todo: modinfo check for sig_key
}

build_modules() {
        echo "Build kernel modules ..."

#bazo modify
#        cd $KERNEL_SOURCE
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

        make modules

        export INSTALL_MOD_PATH=$ROOTFS_DIR/

        sudo -E make modules_install
}

build_device_tree() {
        echo "Build device tree ..."
#bazo modify
#        cd $KERNEL_SOURCE
#        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target dtbs
#        cp -rfv $DTB_OUT/*.dtb $BSP_DIR/Linux_for_Tegra/kernel/dtb/

        cd $KERNEL_SOURCE
        make dtbs

        cp nvidia-oot/device-tree/platform/generic-dts/dtbs/* \
        $BSP_DIR/Linux_for_Tegra/kernel/dtb/
}

set -e

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -a|--all)
                configure
                patch_kernel
#                configure_kernel
                build_kernel
                build_modules	
                build_device_tree
                exit 0
                ;;
        -d|--dt)
                configure
#                patch_kernel
#                configure_kernel
                build_device_tree
                exit 0
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                configure
#                patch_kernel
#                configure_kernel
                build_kernel
                exit 0
                ;;
        -m|--modules)
                configure
                patch_kernel
#                configure_kernel
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