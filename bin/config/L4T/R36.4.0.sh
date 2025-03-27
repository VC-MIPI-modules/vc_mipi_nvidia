#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

#toolchain
GCC_URL=https://snapshots.linaro.org/gnu-toolchain/11.3-2022.06-1/aarch64-linux-gnu
GCC_FILE=gcc-linaro-11.3.1-2022.06-x86_64_aarch64-linux-gnu.tar.xz
GCC_DIR=$TOOLCHAIN_DIR/gcc-linaro-11.3.1-2022.06-x86_64_aarch64-linux-gnu
export CROSS_COMPILE=$GCC_DIR/bin/aarch64-linux-gnu-

#downloads
DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r36_release_v4.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_FILE=Jetson_Linux_R36.4.0_aarch64.tbz2
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R36.4.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="6cfe5e3471f6ccb1059fc7a5a59ca1f2"
RFS_FILE_CHECKSUM="050fe9d304d771df6795eb49187784c9"
SRC_FILE_CHECKSUM="5c30b49fea680c370e870a3424c886f4"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh

#configure
PATCHES=('kernel_Xavier_36.4.0+')

KERNEL_SOURCE=$BSP_DIR/Linux_for_Tegra/source

DTSI_FILE_DICT+=(["NV_DevKit_OrinNano"]="tegra234-p3767-camera-p3768-vc_mipi-dual.dts")
DTSI_DEST_DICT+=(["NV_DevKit_OrinNano"]="$KERNEL_SOURCE/hardware/nvidia/t23x/nv-public/overlay")

DTSI_FILE_DICT+=(["Auvidea_JNX42_OrinNano"]="tegra234-p3767-camera-p3768-vc_mipi-dual.dts")
DTSI_DEST_DICT+=(["Auvidea_JNX42_OrinNano"]="$KERNEL_SOURCE/hardware/nvidia/t23x/nv-public/overlay")

DTSI_FILE_DICT+=(["Auvidea_JNX42_OrinNX"]="tegra234-p3767-camera-p3768-vc_mipi-dual.dts")
DTSI_DEST_DICT+=(["Auvidea_JNX42_OrinNX"]="$KERNEL_SOURCE/hardware/nvidia/t23x/nv-public/overlay")

#setup
DRIVER_DST_DIR=$KERNEL_SOURCE/nvidia-oot/drivers/media/i2c/vc_mipi
KERNEL_OUT=$KERNEL_SOURCE/nvidia-oot
export KERNEL_HEADERS=$KERNEL_SOURCE/kernel/kernel-jammy-src

KERNEL_DIR=kernel/kernel-jammy-src/
MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs/usr

DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts/nvidia

ROOTFS_DIR=$BSP_DIR/Linux_for_Tegra/rootfs
MODULE_TARGET_LOCATION=updates/drivers/media/i2c

ORIN_FLASH_CONFIG_FOLDER="generic"
ORIN_FLASH_PARTITION_MMC="mmcblk0p1"

ORIN_FLASH_PARTITION_NVME="nvme0n1p1"

function L4T_extract_kernel_packages {
        echo "Extracting kernel packages ($VC_MIPI_BSP) ..."

        tar xvf kernel_src.tbz2

        tar xvf kernel_oot_modules_src.tbz2

        tar xvf nvidia_kernel_display_driver_source.tbz2
}

function L4T_add_kernel_to_repo {
        echo "Adding kernel sources to local repository ($VC_MIPI_BSP) ..."

        git add hardware
        git add kernel
        git add nvidia-oot
}

function L4T_setup_nvidia_driver {
# not necessary for this L4T, it is being build and deployed automatically by the Makefile
        return 0
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

EPROM_FILE=${BSP_DIR}/Linux_for_Tegra/bootloader/generic/BCT/tegra234-mb2-bct-misc-p3767-0000.dts
function L4T_setup_eeprom_size {
        Common_setup_eeprom_size
}

GPIO_FILE=${BSP_DIR}/Linux_for_Tegra/bootloader/generic/BCT/tegra234-mb2-bct-scr-p3767-0000.dts
function L4T_setup_gpio_file {
        Common_setup_gpio_file
}

ORIN_NANO_CONF_FILE=${BSP_DIR}/Linux_for_Tegra/p3768-0000-p3767-0000-a0.conf
function L4T_setup_conf_file {
        Common_setup_conf_file
}

function L4T_setup_dynamic_dtbo_file {
        DYNAMIC_DTBO_FILE=${BSP_DIR}/Linux_for_Tegra/source/hardware/nvidia/t23x/nv-public/overlay/tegra234-p3768-0000+p3767-0000-dynamic.dts

        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                if [ ! -e ${DYNAMIC_DTBO_FILE} ]
                then
                        echo "Could not find ${DYNAMIC_DTBO_FILE}! (pwd $(pwd))"
                        exit 1
                fi

                echo "Modifying ${DYNAMIC_DTBO_FILE} ($VC_MIPI_BSP) ..."
                sed -i '/#include \"tegra234-p3768-camera-rbpcv2-imx219.dtsi\"/d' ${DYNAMIC_DTBO_FILE}
                echo "done"
                ;;
        *)
                return 0
                ;;
        esac

}

#build
function L4T_build_device_tree {
        echo "Build device tree ($VC_MIPI_BSP) ..."

        make dtbs

        cp kernel-devicetree/generic-dts/dtbs/* $BSP_DIR/Linux_for_Tegra/kernel/dtb/
}

function L4T_build_kernel {
        echo "Build kernel ($VC_MIPI_BSP) ..."

        make -C kernel -j$(($(nproc)-1))

        export INSTALL_MOD_PATH=$ROOTFS_DIR/
        sudo -E make install -C kernel

        cp kernel/kernel-jammy-src/arch/arm64/boot/Image $BSP_DIR/Linux_for_Tegra/kernel/Image
}

function L4T_build_modules {
        echo "Build modules ($VC_MIPI_BSP) ..."

        make modules -j$(($(nproc)-1))
}

function L4T_install_modules {
        echo "Install modules ($VC_MIPI_BSP) ..."

        export INSTALL_MOD_PATH=$ROOTFS_DIR/
        sudo -E make modules_install
}

function L4T_build_nvidia_driver {
# not necessary for this L4T, it is being build and deployed automatically by the Makefile
        return 0
}