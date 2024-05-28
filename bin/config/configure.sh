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
AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
        BSP_DIR=$BUILD_DIR/Xavier\_$VC_MIPI_BSP
        ;;
esac
DOWNLOAD_DIR=$BSP_DIR/downloads

L4T=$BIN_DIR/config/L4T/R$VC_MIPI_BSP.sh
if [[ -e $L4T ]]; then
        echo "sourcing L4T ($L4T)"
        . $L4T
        #Integrity check of the sourced L4T scripts
        Common_check_for_functions
        if [[ $? != 0 ]]
        then 
                echo "Could not source common_functions!"
                exit 1
        fi
else
        echo "BSP $VC_MIPI_BSP not supported!"
        exit 1
fi

DTSI_FILE_DICT+=( 
         ["Auvidea_J20_AGXXavier"]="tegra194-camera-vc-mipi-cam.dtsi" 
               ["Auvidea_J20_TX2"]="tegra186-camera-vc-mipi-cam.dtsi"
            ["Auvidea_JNX30_Nano"]="tegra210-camera-vc-mipi-cam.dtsi"
            ["Auvidea_JNX42_Nano"]="tegra210-camera-vc-mipi-cam.dtsi"
        ["Auvidea_JNX30_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
        ["Auvidea_JNX42_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
                ["NV_DevKit_Nano"]="tegra210-camera-vc-mipi-cam.dtsi" 
            ["NV_DevKit_XavierNX"]="tegra194-camera-vc-mipi-cam.dtsi"
          ["Auvidea_JNX30D_TX2NX"]="tegra186-camera-vc-mipi-cam.dtsi"
)

DTSI_DEST_DICT+=( 
         ["Auvidea_J20_AGXXavier"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/common/kernel-dts/t19x-common-modules" 
               ["Auvidea_J20_TX2"]="$KERNEL_SOURCE/hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-modules"
            ["Auvidea_JNX30_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms"
            ["Auvidea_JNX42_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms"
        ["Auvidea_JNX30_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
        ["Auvidea_JNX42_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
                ["NV_DevKit_Nano"]="$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms" 
            ["NV_DevKit_XavierNX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common"
          ["Auvidea_JNX30D_TX2NX"]="$KERNEL_SOURCE/hardware/nvidia/platform/t18x/lanai/kernel-dts/common"
)

if [[ ${!DTSI_FILE_DICT[@]} != ${!DTSI_DEST_DICT[@]} ]]
then
        echo "Integrity check of the dtsi dictionaries failed. Key list seems to be not consistent. Exiting."
        exit 1
fi

TARGET_USER=$USER
TARGET_PW=
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

case $VC_MIPI_SOM in
Nano|NanoSD|Nano2GB)
        # Carrier board dependant settings
        case $VC_MIPI_BOARD in
        Auvidea_JNX30|Auvidea_JNX42)
                case $VC_MIPI_BSP in
                32.3.1)
                        PATCHES+=('dt_Auvidea_JNX30_Nano_32.3.1+')
                        ;;
                32.5.0|32.5.1|32.5.2|32.6.1|32.7.1|32.7.2|32.7.3|32.7.4)
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
                35.1.0|35.2.1|35.3.1|35.4.1|36.2.0)
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


OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
        ORIN_DTB_PREFIX='tegra234-p3767-'
        ORIN_DTB_SKU=''
        ORIN_DTB_SUFFIX=''
        case $VC_MIPI_BOARD in
        Auvidea_JNX42)
                FLASH_BOARD='p3509-a02+p3767-0000'
                ORIN_DTB_SUFFIX='-p3509-a02.dtb'
                ;;
        NV_DevKit_OrinNano)
                FLASH_BOARD='jetson-orin-nano-devkit'
                ORIN_DTB_SUFFIX='-p3768-0000-a0.dtb'
                ;;
        esac

        case $VC_MIPI_SOM in
        OrinNano4GB_SD)
                ORIN_DTB_SKU='0004'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_MMC
                ;;
        OrinNano8GB_SD)
                ORIN_DTB_SKU='0003'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_MMC
                ;;
        OrinNano4GB_NVME)
                ORIN_DTB_SKU='0004'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_NVME
                ;;
        OrinNano8GB_NVME)
                ORIN_DTB_SKU='0003'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_NVME
                ;;
        OrinNX8GB)
                ORIN_DTB_SKU='0001'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_NVME
                ;;
        OrinNX16GB)
                ORIN_DTB_SKU='0000'
                FLASH_PARTITION=$ORIN_FLASH_PARTITION_NVME
                ;;
        esac

        ORIN_DTB_FILE=${ORIN_DTB_PREFIX}${ORIN_DTB_SKU}${ORIN_DTB_SUFFIX}
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

export LOCALVERSION=-tegra
export ARCH=arm64
