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

        sudo ./flash.sh $FLASH_BOARD $FLASH_PARTITION

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
        $TARGET_SHELL "echo vc | sudo -S mv $TARGET_DIR/$IMAGE_FILE /boot"
        $TARGET_SHELL ls -l /boot
}

flash_device_tree() {
        cd $BSP_DIR/Linux_for_Tegra/
	echo "Flashing devtree only ... devtree: ${FLASH_DT} board: ${FLASH_BOARD}, partition: ${FLASH_PARTITION}"
        sudo ./flash.sh -r -k $FLASH_DT $FLASH_BOARD $FLASH_PARTITION
}

reboot_target() {
        echo "Rebooting target ..."
        $TARGET_SHELL "echo vc | sudo -S /sbin/reboot"
}

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -a|--all)
                configure
                check_recovery_mode
                flash_all
                exit 0
                ;;
        -d|--dt)
                configure
                check_recovery_mode
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