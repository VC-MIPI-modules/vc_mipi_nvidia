#/bin/bash
#
. config/configure.sh $1 $2

if [[ $CMD == "r" ]]; then
    if [[ -z $(lsusb | grep "NVIDIA Corp.") ]]; then
       echo "Recovery Mode not startet!"
    fi

    cd $BUILD_DIR/Linux_for_Tegra/
    #sudo ./flash.sh jetson-nano-emmc mmcblk0p1
    sudo ./flash.sh jetson-nano-devkit mmcblk0p1
fi
if [[ $CMD == "a" || $CMD == "f" || $CMD == "k" ]]; then
    scp $KERNEL_OUT/arch/arm64/boot/Image $TARGET_USER@$TARGET_NAME:/tmp
fi
if [[ $CMD == "a" || $CMD == "f" || $CMD == "d" ]]; then
    sudo ./flash.sh -k DTB jetson-nano-devkit mmcblk0p1
    ##scp $KERNEL_OUT/arch/arm64/boot/dts/*.dtb $TARGET_USER@$TARGET_NAME:/tmp
fi

#$TARGET_SHELL sudo /sbin/reboot