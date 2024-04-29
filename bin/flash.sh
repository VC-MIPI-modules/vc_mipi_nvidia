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

        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD)
                echo "flashing sd FLASH_PARTITION: ${FLASH_PARTITION}, FLASH_BOARD: ${FLASH_BOARD}"
                echo "ORIN_FLASH_CONFIG_FOLDER: ${ORIN_FLASH_CONFIG_FOLDER}"
#                exit 0
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
                echo "flashing nvme FLASH_PARTITION: ${FLASH_PARTITION}, FLASH_BOARD: ${FLASH_BOARD}"
                echo "ORIN_FLASH_CONFIG_FOLDER: ${ORIN_FLASH_CONFIG_FOLDER}"
#                exit 0
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
        TARGET_DIR=/tmp
        scp $KERNEL_OUT/arch/arm64/boot/$IMAGE_FILE $TARGET_USER@$TARGET_IP:$TARGET_DIR
        $TARGET_SHELL "echo $TARGET_PW | sudo -S mv $TARGET_DIR/$IMAGE_FILE /boot"
        $TARGET_SHELL ls -l /boot
}

flash_device_tree() {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                echo "Flashing devtree only ..."
                echo "Please modify /boot/extlinux/extlinux.conf"
                TARGET_DIR=/tmp
                scp $KERNEL_OUT/arch/arm64/boot/dts/nvidia/$ORIN_DTB_FILE \
                        $TARGET_USER@$TARGET_IP:$TARGET_DIR
                $TARGET_SHELL ls -la $TARGET_DIR/$ORIN_DTB_FILE

                $TARGET_SHELL "echo $TARGET_PW | sudo -S mv $TARGET_DIR/$ORIN_DTB_FILE /boot/dtb"
                $TARGET_SHELL ls -l /boot/dtb
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