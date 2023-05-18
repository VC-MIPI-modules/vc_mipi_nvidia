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
        # sudo ./flash.sh $FLASH_BOARD $FLASH_PARTITION
        
        # sudo \
        # ADDITIONAL_DTB_OVERLAY_OPT="BootOrderNvme.dtbo" \
        # ./tools/kernel_flash/l4t_initrd_flash.sh \
        #         --external-device nvme0n1p1 \
        #         -c tools/kernel_flash/flash_l4t_external.xml \
        #         -p "-c bootloader/t186ref/cfg/flash_t234_qspi_nvme.xml" \
        #         --showlogs \
        #         --network usb0 jetson-orin-nano-devkit-nvme internal

        # Kein Fehler beim Flashen. Allerdings funktioniert das Netzwerk nicht.
        # sudo ADDITIONAL_DTB_OVERLAY_OPT="BootOrderNvme.dtbo" \
        # ./tools/kernel_flash/l4t_initrd_flash.sh \
        #         --external-device nvme0n1p1 \
        #         --external-only \
        #         -c tools/kernel_flash/flash_l4t_external.xml \
        #         --showlogs \
        #         --network usb0 jetson-orin-nano-devkit internal
}

flash_kernel() {
        scp $KERNEL_OUT/arch/arm64/boot/Image $TARGET_USER@$TARGET_IP:/tmp
}

flash_device_tree() {
        cd $BSP_DIR/Linux_for_Tegra/
	echo "Flashing devtree only ... devtree: ${FLASH_DT} board: ${FLASH_BOARD}, partition: ${FLASH_PARTITION}"
        sudo ./flash.sh -r -k $FLASH_DT $FLASH_BOARD $FLASH_PARTITION
}

reboot_target() {
        $TARGET_SHELL sudo /sbin/reboot
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
                exit 0
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                configure
                flash_kernel
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

usage
exit 1