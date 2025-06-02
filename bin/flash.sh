#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Flash kernel image, modules and device tree to the target."
        echo ""
        echo "Supported options:"
        echo "-a, --all                 Flash kernel image, modules and device tree"
        echo "-d, --dt                  Flash device tree"
        echo "-h, --help                Show this help text"
        echo "-k, --kernel              Flash kernel image"
        echo "-m, --module              Flash kernel module"
}

configure() {
        . config/configure.sh
}

check_recovery_mode() {
        if [[ -z $(lsusb | grep -i "NVIDIA Corp.") || -n $(lsusb | grep -i "NVIDIA Corp. L4T") ]]; then
                echo "Recovery Mode not started!"
                exit 1
        fi
}

flash_all() {
        cd $BSP_DIR/Linux_for_Tegra/
        echo "Flashing all ... board: ${FLASH_BOARD}, partition: ${FLASH_PARTITION}"

        start_time=$(date +%s)

        if [[ "36.4.3" == $VC_MIPI_BSP ]]
        then
                sudo ./tools/kernel_flash/l4t_initrd_flash.sh \
                --external-device ${FLASH_PARTITION} \
                -c tools/kernel_flash/flash_l4t_t234_nvme.xml \
                -p "-c bootloader/generic/cfg/flash_t234_qspi.xml" \
                --showlogs --network usb0 ${FLASH_BOARD} internal
        else
                case $VC_MIPI_SOM in
                OrinNano4GB_SD|OrinNano8GB_SD)
                        # The given parameter $1 might be used for the EXT_NUM_SECTORS variable
                        # which has been introduced with 35.4.1
                        # sudo ./flash.sh -a EXT_NUM_SECTORS="121536512"
                        sudo $1 ./tools/kernel_flash/l4t_initrd_flash.sh \
                                --external-device ${FLASH_PARTITION} \
                                -c tools/kernel_flash/flash_l4t_external.xml \
                                -p "-c bootloader/${ORIN_FLASH_CONFIG_FOLDER}/cfg/flash_t234_qspi.xml" \
                                --network usb0 \
                                ${FLASH_BOARD} internal
                        ;;

                OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                        sudo ADDITIONAL_DTB_OVERLAY_OPT="BootOrderNvme.dtbo" \
                                ./tools/kernel_flash/l4t_initrd_flash.sh \
                                --external-device ${FLASH_PARTITION} \
                                -c tools/kernel_flash/flash_l4t_external.xml \
                                -p "-c bootloader/${ORIN_FLASH_CONFIG_FOLDER}/cfg/flash_t234_qspi.xml" \
                                --network usb0 \
                                ${FLASH_BOARD} internal
                        ;;
                *)
                        sudo ./flash.sh $FLASH_BOARD $FLASH_PARTITION
                        ;;
                esac

        fi

        end_time=$(date +%s)
        elapsed_time=$((${end_time} - ${start_time}))

        echo "------------------------------------------------------------"
        echo -n "  "
        eval "echo $(date -ud "@${elapsed_time}" +'The process took %H hours %M minutes %S seconds')"
        echo "------------------------------------------------------------"
}

flash_kernel() {
        echo "Flashing kernel only ..."
        IMAGE_FILE=Image
        TMP_DIR=/tmp
        scp $KERNEL_OUT/arch/arm64/boot/$IMAGE_FILE $TARGET_USER@$TARGET_IP:$TMP_DIR
        $TARGET_SHELL "echo $TARGET_PW | sudo -S mv $TMP_DIR/$IMAGE_FILE /boot"
        $TARGET_SHELL ls -l /boot
}

flash_module() {
        case $VC_MIPI_BSP in
        36.2.0|36.4.0|36.4.3)
                echo "Flashing module only ..."
                TMP_DIR=/tmp
                MODULE_FILES=$(find $KERNEL_SOURCE/nvidia-oot/drivers/media/i2c -name "vc_mipi*.ko")
                for MODULE_FILE in $MODULE_FILES; do
                        MODULE_NAME=$(basename $MODULE_FILE)
                        KERNEL_VERSION=$($TARGET_SHELL "uname -r")
                        MODULE_DIR=/lib/modules/$KERNEL_VERSION/$MODULE_TARGET_LOCATION
                        scp $MODULE_FILE $TARGET_USER@$TARGET_IP:$TMP_DIR
                        $TARGET_SHELL "echo $TARGET_PW | sudo -S mv $TMP_DIR/$MODULE_NAME $MODULE_DIR"
                done
                ;;
        *)
                echo "This BSP version does not support flashing camera driver modules."
                ;;
        esac
}

flash_device_tree() {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                echo "Flashing devtree only ..."
                echo "Please modify /boot/extlinux/extlinux.conf"
                TMP_DIR=/tmp
                case $VC_MIPI_BSP in
                35.1.0|35.2.1|35.3.1|35.4.1)
                        SRC_FILE=$KERNEL_OUT/arch/arm64/boot/dts/nvidia/$ORIN_DTB_FILE
                        ORIN_DTB_DIR=/boot/dtb
                        ;;
                36.2.0|36.4.0|36.4.3)
                        ORIN_DTB_FILE=tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
                        SRC_FILE=$BSP_DIR/Linux_for_Tegra/kernel/dtb/$ORIN_DTB_FILE
                        ORIN_DTB_DIR=/boot
                        ;;
                esac
                scp $SRC_FILE $TARGET_USER@$TARGET_IP:$TMP_DIR
                $TARGET_SHELL "echo $TARGET_PW | sudo -S mv $TMP_DIR/$ORIN_DTB_FILE $ORIN_DTB_DIR"
                $TARGET_SHELL ls -l $ORIN_DTB_DIR/$ORIN_DTB_FILE
                ;;
        *)
                check_recovery_mode
                cd $BSP_DIR/Linux_for_Tegra/
                echo "Flashing devtree only ... devtree: ${FLASH_DT} board: ${FLASH_BOARD}, partition: ${FLASH_PARTITION}"
                sudo ./flash.sh -r -k $FLASH_DT $FLASH_BOARD $FLASH_PARTITION
                ;;
        esac
}

reboot_target() {
        echo "Rebooting target ..."
        $TARGET_SHELL "echo $TARGET_PW | sudo -S /sbin/reboot"
}

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -a|--all)
                configure
                check_recovery_mode
                flash_all $1
                exit 0
                ;;
        -d|--dt)
                configure
                flash_device_tree
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                configure
                flash_kernel
                ;;
        -m|--module)
                configure
                flash_module
                ;;
        -r|--reboot)
                configure
                reboot_target
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done