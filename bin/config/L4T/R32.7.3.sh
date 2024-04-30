#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

#toolchain
GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
GCC_DIR=$TOOLCHAIN_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
export CROSS_COMPILE=$GCC_DIR/bin/aarch64-linux-gnu-

#downloads
DEV_URL=https://developer.nvidia.com/downloads

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.3_aarch64.tbz2
                BSP_URL_UNRESOLVED=$DEV_URL/remetpack-463r32releasev73t210jetson-210linur3273aarch64tbz2
                RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.3_aarch64.tbz2
                RFS_URL_UNRESOLVED=$DEV_URL/remeleasev73t210tegralinusample-root-filesystemr3273aarch64tbz2
                SRC_FILE=public_sources.tbz2
                SRC_URL_UNRESOLVED=$DEV_URL/remack-sdksjetpack-463r32releasev73sourcest210publicsourcestbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="4d1407ab9eeb5db30284622712681ee5"
                RFS_FILE_CHECKSUM="252ceb276a60ff98797d684ce8912976"
                SRC_FILE_CHECKSUM="3fae68c1d5d49862a8448ead344af564"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=jetson_linux_r32.7.3_aarch64.tbz2
                BSP_URL_UNRESOLVED=$DEV_URL/remksjetpack-463r32releasev73t186jetsonlinur3273aarch64tbz2
                RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.3_aarch64.tbz2
                RFS_URL_UNRESOLVED=$DEV_URL/remeleasev73t186tegralinusample-root-filesystemr3273aarch64tbz2
                SRC_FILE=public_sources.tbz2
                SRC_URL_UNRESOLVED=$DEV_URL/remack-sdksjetpack-463r32releasev73sourcest186publicsourcestbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="4e888c03c714ea987a2d1311bd73ae9b"
                RFS_FILE_CHECKSUM="252ceb276a60ff98797d684ce8912976"
                SRC_FILE_CHECKSUM="8fca87ee2156d6eb5e49cda0879d5431"
                ;;
esac

BSP_URL=$DEV_URL
RFS_URL=$DEV_URL
SRC_URL=$DEV_URL

#configure
PATCHES=('kernel_common_32.3.1+')
case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB)
        PATCHES+=('kernel_Nano_32.6.1+')
        ;;

AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
        PATCHES+=('kernel_Xavier_32.7.3+')
        ;;
esac

#setup
KERNEL_SOURCE=$BSP_DIR/Linux_for_Tegra/source/public
DRIVER_DST_DIR=$KERNEL_SOURCE/kernel/nvidia/drivers/media/i2c
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules

KERNEL_DIR=kernel/kernel-4.9/
MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs

DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts

#setup
function L4T_extract_kernel_packages {
        echo "Extracting kernel packages ($VC_MIPI_BSP) ..."

        tar xvf kernel_src.tbz2
}

function L4T_add_kernel_to_repo {
        echo "Adding kernel sources to local repository ($VC_MIPI_BSP) ..."

        git add hardware
        git add kernel
}

function L4T_setup_nvidia_driver {
# not necessary for this L4T
        return 0
}

function L4T_setup_flash_prerequisites {
# not necessary for this L4T
        return 0
}

function L4T_setup_eeprom_size {
# not necessary for this L4T
        return 0;
}

function L4T_setup_gpio_file {
# not necessary for this L4T
        return 0;
}

function L4T_setup_conf_file {
# not necessary for this L4T
        return 0;
}

function L4T_setup_som_carrier_specifics {
# not necessary for this L4T
        return 0
}

#build
function L4T_build_device_tree {
        echo "Build device tree ($VC_MIPI_BSP) ..."

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target dtbs

        cp -rfv $DTB_OUT/*.dtb $BSP_DIR/Linux_for_Tegra/kernel/dtb/
}

function L4T_build_kernel {
        echo "Build kernel ($VC_MIPI_BSP) ..."

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target Image

        cp -rfv $KERNEL_OUT/arch/arm64/boot/Image $BSP_DIR/Linux_for_Tegra/kernel/
}

function L4T_build_modules {
        echo "Build modules ($VC_MIPI_BSP) ..."

        make -C $KERNEL_DIR O=$KERNEL_OUT -j$(nproc) --output-sync=target modules
}

function L4T_install_modules {
        echo "Install modules ($VC_MIPI_BSP) ..."

        make -C $KERNEL_DIR O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 
        sudo cp -arfv $MODULES_OUT/lib $MODULES_BSP
}

function L4T_build_nvidia_driver {
# not necessary for this L4T
        return 0
}