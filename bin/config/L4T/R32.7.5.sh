#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

#toolchain
GCC_URL_UNRESOLVED=https://developer.nvidia.com/embedded/dlc/l4t-gcc-7-3-1-toolchain-64-bit
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
GCC_DIR=$TOOLCHAIN_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
GCC_FILE_CHECKSUM="6ec789d642584a01e240ab3366599dbb"
export CROSS_COMPILE=$GCC_DIR/bin/aarch64-linux-gnu-

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r32_release_v7.5

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB)
                BSP_FILE=jetson-210_linux_r32.7.5_aarch64.tbz2

                ADDON_FILE=overlay_32.7.5_PCN211181.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="ad9f047075ea3a7767ea737ac2c38dee"
                RFS_FILE_CHECKSUM="cc0bcfce89cfbe58349e7a00f6b32d5f"
                SRC_FILE_CHECKSUM="7e165f36b363cb23c3c477323434a972"
                ADDON_FILE_CHECKSUM="406dc6a814e6ca20b87322fbcc8dfc1c"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=jetson_linux_r32.7.5_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM=""
                RFS_FILE_CHECKSUM=""
                SRC_FILE_CHECKSUM=""
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.5_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh

#configure
PATCHES=('kernel_common_32.3.1+')
case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB)
        PATCHES+=('kernel_Nano_32.6.1+')
        PATCHES+=('kernel_Nano_32.7.5')
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

function L4T_setup_addon_file {
        cd $DOWNLOAD_DIR
        download_and_check_file ADDON

        echo "Extracting DRAM cfg ($VC_MIPI_BSP) ..."
        tar xjvf $ADDON_FILE -C $BSP_DIR Linux_for_Tegra/bootloader/t210ref/BCT/P3448_A00_lpddr4_204Mhz_P987.cfg 

        echo "Extracting DRAM patch ($VC_MIPI_BSP) ..."
        tar xjvf $ADDON_FILE -C $KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/ hardware-nvidia-platform-t210-porg.patch 
        
        echo "Applying DRAM patch ($VC_MIPI_BSP) ..."
        cd $KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/
        git am -3 --whitespace=fix --ignore-whitespace < hardware-nvidia-platform-t210-porg.patch
}

function L4T_setup_toolchain {
        echo "Setup toolchain ($VC_MIPI_BSP) ..."

        if [[ ! -e $GCC_DIR ]]; then
                cd $DOWNLOAD_DIR
                tar xvf $GCC_FILE -C $TOOLCHAIN_DIR
        fi
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