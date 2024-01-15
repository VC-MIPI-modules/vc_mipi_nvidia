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
        OrinNano)
                sudo ADDITIONAL_DTB_OVERLAY_OPT="BootOrderNvme.dtbo" \
                        ./tools/kernel_flash/l4t_initrd_flash.sh \
                        --external-device nvme0n1p1 \
                        -c tools/kernel_flash/flash_l4t_external.xml \
                        -p "-c bootloader/t186ref/cfg/flash_t234_qspi.xml" \
                        --network usb0 \
                        jetson-orin-nano-devkit internal
                ;;
        OrinNX)
                sudo ADDITIONAL_DTB_OVERLAY_OPT="BootOrderNvme.dtbo" \
                        ./tools/kernel_flash/l4t_initrd_flash.sh \
                        --external-device nvme0n1p1 \
                        -c tools/kernel_flash/flash_l4t_external.xml \
                        -p "-c bootloader/t186ref/cfg/flash_t234_qspi.xml" \
                        --network usb0 \
                        p3509-a02+p3767-0000 internal
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
        $TARGET_SHELL "echo vc | sudo -S mv $TARGET_DIR/$IMAGE_FILE /boot"
        $TARGET_SHELL ls -l /boot
}

flash_device_tree() {
        case $VC_MIPI_SOM in
        OrinNano)
                echo "Flashing devtree only ..."
                echo "Please modify /boot/extlinux/extlinux.conf"
                DTB_FILE=tegra234-p3767-0004-p3768-0000-a0.dtb
                TARGET_DIR=/tmp
                scp $KERNEL_OUT/arch/arm64/boot/dts/nvidia/$DTB_FILE \
                        $TARGET_USER@$TARGET_IP:$TARGET_DIR
                $TARGET_SHELL "echo vc | sudo -S mv $TARGET_DIR/$DTB_FILE /boot/dtb"
                $TARGET_SHELL ls -l /boot/dtb
                ;;
        OrinNX)
                echo "Flashing devtree only ..."
                echo "Please modify /boot/extlinux/extlinux.conf"
                DTB_FILE=tegra234-p3767-0000-p3509-a02.dtb
                TARGET_DIR=/tmp
                scp $KERNEL_OUT/arch/arm64/boot/dts/nvidia/$DTB_FILE \
                        $TARGET_USER@$TARGET_IP:$TARGET_DIR
                $TARGET_SHELL "echo vc | sudo -S mv $TARGET_DIR/$DTB_FILE /boot/dtb"
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