#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

#toolchain
GCC_URL_UNRESOLVED=https://developer.nvidia.com/embedded/jetson-linux/bootlin-toolchain-gcc-93
GCC_FILE=aarch64--glibc--stable-final.tar.gz
GCC_DIR=$TOOLCHAIN_DIR/gcc93
GCC_FILE_CHECKSUM="f360163e23096d3157949af40840e413"
export CROSS_COMPILE=$GCC_DIR/bin/aarch64-linux-

DEV_URL=https://developer.nvidia.com/embedded/l4t/r35_release_v1.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin)
                BSP_FILE=jetson_linux_r35.1.0_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.1.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="9360d210d2fa5fdb3f5fff72995c2218"
RFS_FILE_CHECKSUM="9794b13e2c58f26c6a8cd2788c5e4736"
SRC_FILE_CHECKSUM="a1e4523f3b642c4302793aaf1e0e5f9a"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh

#configure
PATCHES=('kernel_common_32.3.1+')
PATCHES+=('kernel_Xavier_35.1.0+')

KERNEL_SOURCE=$BSP_DIR/Linux_for_Tegra/source/public

#setup
DRIVER_DST_DIR=$KERNEL_SOURCE/kernel/nvidia/drivers/media/i2c
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules

KERNEL_DIR=kernel/kernel-5.10/
MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs/usr

DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts/nvidia

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
        return 0;
}

function L4T_setup_flash_prerequisites {
# not necessary for this L4T
        return 0;
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

function L4T_setup_addon_file {
# not necessary for this L4T
        return 0
}

function L4T_setup_toolchain {
        echo "Setup toolchain ($VC_MIPI_BSP) ..."

        if [[ ! -e $GCC_DIR ]]; then
                mkdir -p $GCC_DIR
                cd $DOWNLOAD_DIR
                tar xvf $GCC_FILE -C $GCC_DIR
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

        echo "MODULES_OUT: $MODULES_OUT"
        make -C $KERNEL_DIR O=$KERNEL_OUT INSTALL_MOD_PATH=$MODULES_OUT modules_install 
        sudo cp -arfv $MODULES_OUT/lib $MODULES_BSP
}

function L4T_build_nvidia_driver {
# not necessary for this L4T
        return 0;
}
