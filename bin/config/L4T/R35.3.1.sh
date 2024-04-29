#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

#toolchain
GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
GCC_DIR=$TOOLCHAIN_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
export CROSS_COMPILE=$GCC_DIR/bin/aarch64-linux-gnu-

#downloads
DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v3.1

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_FILE=jetson_linux_r35.3.1_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="1d254bec244e1bdeaa4012f02e2c6057"
RFS_FILE_CHECKSUM="12bd3ed977eb86f6b13e90a2000022de"
SRC_FILE_CHECKSUM="71f31135cf18255be97182d6521bc59d"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh

#configure
PATCHES=('kernel_common_32.3.1+')
PATCHES+=('kernel_Xavier_35.3.1+')

KERNEL_SOURCE=$BSP_DIR/Linux_for_Tegra/source/public

DTSI_FILE_DICT+=(    ["NV_DevKit_OrinNano"]="tegra234-camera-vc-mipi-cam.dtsi")
DTSI_FILE_DICT+=(  ["Auvidea_JNX42_OrinNX"]="tegra234-camera-vc-mipi-cam.dtsi")
DTSI_FILE_DICT+=(["Auvidea_JNX42_OrinNano"]="tegra234-camera-vc-mipi-cam.dtsi")
DTSI_DEST_DICT+=(    ["NV_DevKit_OrinNano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb")
DTSI_DEST_DICT+=(  ["Auvidea_JNX42_OrinNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb")
DTSI_DEST_DICT+=(["Auvidea_JNX42_OrinNano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb")

#setup
DRIVER_DST_DIR=$KERNEL_SOURCE/kernel/nvidia/drivers/media/i2c
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules

KERNEL_DIR=kernel/kernel-5.10/
MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs/usr

DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts/nvidia

NVDD_DIR=NVIDIA-kernel-module-source-TempVersion

ORIN_FLASH_CONFIG_FOLDER="t186ref"
ORIN_FLASH_PARTITION_MMC="mmcblk1p1"
ORIN_FLASH_PARTITION_NVME="nvme0n1p1"

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
        Common_setup_nvidia_driver
}

function L4T_setup_flash_prerequisites {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                echo "Setting up flash prerequisites ($VC_MIPI_BSP) ..."
                sudo ./tools/l4t_flash_prerequisites.sh
                ;;
        *)
                return 0
                ;;
        esac
}

EPROM_FILE=${BSP_DIR}/Linux_for_Tegra/bootloader/t186ref/BCT/tegra234-mb2-bct-misc-p3767-0000.dts
function L4T_setup_eeprom_size {
        Common_setup_eeprom_size
}

#GPIO_FILE=${BSP_DIR}/Linux_for_Tegra/bootloader/t186ref/BCT/tegra234-mb2-bct-scr-p3767-0000.dts
function L4T_setup_gpio_file {
#        Common_setup_gpio_file
        return 0;
}

function L4T_setup_conf_file {
# not necessary for this L4T
        return 0;
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
        Common_build_nvidia_driver
}
