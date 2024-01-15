#!/bin/bash

. config/base.sh
. helper/dtsi_helper.sh
. helper/setup_helper.sh

if [[ $1 == 'driver' ]]; then
        rm -Rf $CONFIGURATION_FILE
fi
if [[ ! -e $CONFIGURATION_FILE ]]; then
        . $BIN_DIR/config/setup.sh --driver $2 $3
fi
. $CONFIGURATION_FILE

case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB|TX1)
        BSP_DIR=$BUILD_DIR/Nano\_$VC_MIPI_BSP
        ;;
AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX|OrinNano|OrinNX)
        BSP_DIR=$BUILD_DIR/Xavier\_$VC_MIPI_BSP
        ;;
esac
DOWNLOAD_DIR=$BSP_DIR/downloads
KERNEL_SOURCE=$BSP_DIR/Linux_for_Tegra/source/public
KERNEL_OUT=$KERNEL_SOURCE/build
MODULES_OUT=$KERNEL_SOURCE/modules
DRIVER_DST_DIR=$KERNEL_SOURCE/kernel/nvidia/drivers/media/i2c
case $VC_MIPI_BSP in
35.1.0|35.2.1|35.3.1)
        KERNEL_DIR=kernel/kernel-5.10/
        MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs/usr
        DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts/nvidia
        ;;
        *)
        KERNEL_DIR=kernel/kernel-4.9/
        MODULES_BSP=$BSP_DIR/Linux_for_Tegra/rootfs
        DTB_OUT=$KERNEL_OUT/arch/arm64/boot/dts
        ;;
esac

TARGET_USER=$USER
TARGET_IP=
if [[ $1 == 'target' ]]; then
        . $BIN_DIR/config/setup.sh --target
fi
if [[ -e $TARGET_FILE ]]; then
        . $TARGET_FILE
fi
TARGET_SHELL="ssh $TARGET_USER@$TARGET_IP"

echo "------------------------------------------------------------"
echo "  Vision Components MIPI CSI-2 camera driver"
echo "------------------------------------------------------------"
echo "  System on Module:         $VC_MIPI_SOM"
echo "  Carrier Board:            $VC_MIPI_BOARD"
echo "  L4T Driver Package (BSP): $VC_MIPI_BSP"
echo "------------------------------------------------------------"

L4T=$BIN_DIR/config/L4T/R$VC_MIPI_BSP.sh
if [[ -e $L4T ]]; then
        . $L4T
else
        echo "BSP $VC_MIPI_BSP not supported!"
        exit 1
fi

DTSI_FILE_DICT=( 
         ["Auvidea_J20_AGXXavier"]="tegra194-camera-vc-mipi-cam.dtsi" 
               ["Auvidea_J20_TX2"]="tegra186-camera-vc-mipi-cam.dtsi"
            ["Auvidea_JNX30_Nano"]="tegra210-camera-vc-mipi-cam.dtsi"
            ["Auvidea_JNX42_Nano"]="tegra210-camera-vc-mipi-cam.dtsi"
        ["Auvidea_JNX30_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
        ["Auvidea_JNX42_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
          ["Auvidea_JNX42_OrinNX"]="tegra234-camera-vc-mipi-cam.dtsi"
                ["NV_DevKit_Nano"]="tegra210-camera-vc-mipi-cam.dtsi" 
            ["NV_DevKit_OrinNano"]="tegra234-camera-vc-mipi-cam.dtsi"
            ["NV_DevKit_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
          ["Auvidea_JNX30D_TX2NX"]="tegra186-camera-vc-mipi-cam.dtsi"
)

DTSI_DEST_DICT=( 
         ["Auvidea_J20_AGXXavier"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/common/kernel-dts/t19x-common-modules" 
               ["Auvidea_J20_TX2"]="$KERNEL_SOURCE/hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-modules"
            ["Auvidea_JNX30_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms"
            ["Auvidea_JNX42_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms"
        ["Auvidea_JNX30_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
        ["Auvidea_JNX42_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
          ["Auvidea_JNX42_OrinNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb"
                ["NV_DevKit_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms" 
            ["NV_DevKit_OrinNano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb"
            ["NV_DevKit_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
          ["Auvidea_JNX30D_TX2NX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t18x/lanai/kernel-dts/common"
)

if [[ ${!DTSI_FILE_DICT[@]} != ${!DTSI_DEST_DICT[@]} ]]
then
        echo "Integrity check of the dtsi dictionaries failed. Key list seems to be not consistent. Exiting."
        exit 1
fi

extract_and_set_key_from_config
DT_CAM_FILE="${DT_CAM_DIR}/${DTSI_KEY}/${DTSI_FILE_DICT[$DTSI_KEY]}"

if [[ "1" == $CHECK4MD5 ]]
then
        if [[ -z "$BSP_FILE_CHECKSUM" ]]
        then
                echo "BSP Checksum not found!"
                exit 1
        fi

        if [[ -z "$RFS_FILE_CHECKSUM" ]]
        then
                echo "RFS Checksum not found!"
                exit 1
        fi

        if [[ -z "$SRC_FILE_CHECKSUM" ]]
        then
                echo "SRC Checksum not found!"
                exit 1
        fi
fi

PATCHES=('kernel_common_32.3.1+')
case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB)
        case $VC_MIPI_BSP in
        32.5.0|32.5.1|32.5.2)
                PATCHES+=('kernel_Nano_32.5.0+')
                ;;
        32.6.1|32.7.1|32.7.2|32.7.3)
                PATCHES+=('kernel_Nano_32.6.1+')
                ;;
        esac
        ;;

AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX|OrinNano|OrinNX)
        case $VC_MIPI_BSP in
        32.3.1)
                PATCHES+=('kernel_Xavier_32.3.1+')
                ;;
        32.5.0)
                PATCHES+=('kernel_Xavier_32.5.0+')
                ;;
        32.5.1|32.5.2)
                PATCHES+=('kernel_Xavier_32.5.1+')
                ;;
        32.6.1|32.7.1|32.7.2)
                PATCHES+=('kernel_Xavier_32.6.1+')
                ;;
        32.7.3)
                PATCHES+=('kernel_Xavier_32.7.3+')
                ;;
        35.1.0)
                PATCHES+=('kernel_Xavier_35.1.0+')
                ;;
        35.2.1)
                PATCHES+=('kernel_Xavier_35.2.1+')
                ;;
        35.3.1)
                PATCHES+=('kernel_Xavier_35.3.1+')
                ;;
        esac
        
esac

case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB)
        # Carrier board dependant settings
        case $VC_MIPI_BOARD in
        Auvidea_JNX30|Auvidea_JNX42)
                case $VC_MIPI_BSP in
                32.3.1)
                        PATCHES+=('dt_Auvidea_JNX30_Nano_32.3.1+')
                        ;;
                32.5.0|32.5.1|32.5.2|32.6.1|32.7.1|32.7.2|32.7.3)
                        PATCHES+=('dt_Auvidea_JNX30_Nano_32.5.0+')
                        ;;
                esac
                ;;
        esac

        # Carrier board independant settings
        FLASH_DT='DTB'
        case $VC_MIPI_SOM in
        Nano)           FLASH_BOARD='jetson-nano-emmc' ;;
        NanoSD|Nano2GB) FLASH_BOARD='jetson-nano-qspi-sd' ;;
        esac
        FLASH_PARTITION='mmcblk0p1'
        ;;

AGXXavier)
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-xavier'
        FLASH_PARTITION='mmcblk0p1'
        ;;

XavierNX|XavierNXSD)
        # Carrier board dependant settings
        case $VC_MIPI_BOARD in
        Auvidea_JNX30|Auvidea_JNX42)
                case $VC_MIPI_BSP in
                32.5.0|32.5.1|32.5.2|32.6.1|32.7.1|32.7.2|32.7.3)
                        PATCHES+=('dt_Auvidea_JNX30_XavierNX_32.5.0+')
                        ;;
                35.1.0|35.2.1|35.3.1)
                        # Comment
                ;;
                esac
                ;;
        esac

        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        case $VC_MIPI_SOM in
        XavierNX)   FLASH_BOARD='jetson-xavier-nx-devkit-emmc' ;;
        XavierNXSD) FLASH_BOARD='jetson-xavier-nx-devkit' ;;
        esac
        FLASH_PARTITION='mmcblk0p1'
        ;;

TX2)
        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-tx2'
        FLASH_PARTITION='mmcblk0p1'
        ;;

TX2i)
        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-tx2i'
        FLASH_PARTITION='mmcblk0p1'
        ;;

TX2NX)
        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-xavier-nx-devkit-tx2-nx'
        FLASH_PARTITION='mmcblk0p1'
        ;;

OrinNano)
        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-orin-nano-devkit-nvme'
        FLASH_PARTITION='nvme0n1p1'
        ;;

OrinNX)
        # Carrier board independant settings
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='p3509-a02+p3767-0000'
        FLASH_PARTITION='nvme0n1p1'
        ;;

*)
        echo "SOM $VC_MIPI_SOM not supported!"
        ;;
esac

# PATCHES+=('develop')
echo "  Using build directory:                       $BSP_DIR"
echo "  Using L4T Driver Package (BSP) URL:          $BSP_URL"
echo "  Using L4T Driver Package (BSP) FILE:         $BSP_FILE"
echo "  Using Sample Root Filesystem URL:            $RFS_URL"
echo "  Using Sample Root Filesystem FILE:           $RFS_FILE"
echo "  Using L4T Driver Package (BSP) Sources URL:  $SRC_URL"
echo "  Using L4T Driver Package (BSP) Sources FILE: $SRC_FILE"
echo "  Using patches:                               ${PATCHES[@]}"
echo "  Using devicetree camera file:                $DT_CAM_FILE"
echo "  Using flash parameters:                      $FLASH_DT $FLASH_BOARD $FLASH_PARTITION"
echo "  Using target user and ip address:            $TARGET_USER@$TARGET_IP"
echo "------------------------------------------------------------"

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64
