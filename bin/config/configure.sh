#!/bin/bash

. config/base.sh

if [[ $1 == 'driver' ]]; then
    rm -Rf $CONFIGURATION_FILE
fi
if [[ ! -e $CONFIGURATION_FILE ]]; then
    . $BIN_DIR/config/setup.sh --driver $2 $3
fi
. $CONFIGURATION_FILE

DOWNLOAD_DIR=$BUILD_DIR/downloads/$VC_MIPI_SOM/$VC_MIPI_BSP

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

PATCHES=()

case $VC_MIPI_SOM in
    Nano|NanoDK)
        # Carrier board dependant settings
        case $VC_MIPI_BOARD in
            NV_DevKit_Nano_B01)
                DT_CAM_FILE=$DT_CAM_DIR/NV_DevKit_Nano_B01/tegra210-camera-vc-mipi-cam.dtsi
                ;;
            Auvidea_JNX30)
                DT_CAM_FILE=$DT_CAM_DIR/Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi
                case $VC_MIPI_BSP in
                32.5.0|32.5.1|32.5.2|32.6.1)
                    PATCHES=('dt_Auvidea_JNX30_Nano_32.5.0+')
                    ;;
                esac
                ;;
            *)
                echo "Carrier board $VC_MIPI_BOARD not supported!"
                ;;
        esac

        # Carrier board independant settings
        PATCHES+=('kernel_Nano_32.5.0+')
        DT_CAM_FILE_DST_DIR=$KERNEL_SOURCE/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms
        FLASH_DT='DTB'
        case $VC_MIPI_SOM in
        Nano)   FLASH_BOARD='jetson-nano-emmc' ;;
        NanoDK) FLASH_BOARD='jetson-nano-qspi-sd' ;;
        esac
        FLASH_MEDIUM='mmcblk0p1'

        case $VC_MIPI_BSP in
            32.5.0|32.5.1|32.5.2)
                PATCHES+=('dt_camera_Nano_32.5.0+')
                ;;
            32.6.1)
                PATCHES+=('dt_camera_Nano_32.6.1+')
                ;;
        esac
        ;;

    XavierNX)
        # Carrier board dependant settings
        case $VC_MIPI_BOARD in
            NV_DevKit_XavierNX)
                DT_CAM_FILE=$DT_CAM_DIR/NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi
                ;;
            Auvidea_JNX30)
                # DT_CAM_FILE=Auvidea_JNX30
                # case $VC_MIPI_BSP in
                # 32.5.0|32.5.1|32.5.2|32.6.1)
                #     PATCHES=('dt_auvidea_JNX30_XavierNX_32.5.0+')
                #     ;;
                # esac
                ;;
            *)
                echo "Carrier board $VC_MIPI_BOARD not supported!"
                ;;
        esac

        # Carrier board independant settings
        case $VC_MIPI_BSP in
            32.5.0|32.5.1|32.5.2)
                PATCHES+=('kernel_XavierNX_32.5.0+')
                ;;
            32.6.1)
                PATCHES+=('kernel_XavierNX_32.6.1+')
                ;;
        esac
        DT_CAM_FILE_DST_DIR=$KERNEL_SOURCE/hardware/nvidia/platform/t19x/jakku/kernel-dts/common/
        FLASH_DT='kernel-dtb'
        FLASH_BOARD='jetson-xavier-nx-devkit'
        FLASH_MEDIUM='mmcblk0p1'
        case $VC_MIPI_BSP in
            32.5.0|32.5.1|32.5.2)
                PATCHES+=('dt_camera_XavierNX_32.5.0+')
                ;;
            32.6.1)
                PATCHES+=('dt_camera_XavierNX_32.6.1+')
                ;;
        esac
        ;;
	*)
        echo "SOM $VC_MIPI_SOM not supported!"
        ;;
esac

# PATCHES+=('develop')

echo "  Using L4T Driver Package (BSP) URL:          $BSP_URL"
echo "  Using L4T Driver Package (BSP) FILE:         $BSP_FILE"
echo "  Using Sample Root Filesystem URL:            $RFS_URL"
echo "  Using Sample Root Filesystem FILE:           $RFS_FILE"
echo "  Using L4T Driver Package (BSP) Sources URL:  $SRC_URL"
echo "  Using L4T Driver Package (BSP) Sources FILE: $SRC_FILE"
echo "  Using patches:                               ${PATCHES[@]}"
echo "  Using devicetree camera file:                $DT_CAM_FILE"
echo "  Using flash parameters:                      $FLASH_DT $FLASH_BOARD $FLASH_MEDIUM"
echo "  Using target user and ip address:            $TARGET_USER@$TARGET_IP"
echo "------------------------------------------------------------"

GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
export CROSS_COMPILE=$GCC_DIR/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
export LOCALVERSION=-tegra
export ARCH=arm64