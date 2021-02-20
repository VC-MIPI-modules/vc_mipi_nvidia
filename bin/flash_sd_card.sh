#!/bin/bash
#

# TODO: Selection for all existing images and sd card devices.

if [[ "$1" == 'ls' ]]; then
    # BUG: Don´t shows the list of all possible devices
    dmesg | tail | awk '$3 == "sd" {print}'

else
    . config/configure.sh $1

    for FLASH_MODEL in "${CAMERAS[@]}"; do
        if [[ $FLASH_MODEL == $2 ]]; then
            CAMERA=$FLASH_MODEL
            echo "Using Camera Model: $CAMERA"
        fi
    done

    if [[ -z $CAMERA ]]; then
        echo "Camera model not supported!"
        echo "Options: ${CAMERAS[@]}" 
        exit
    fi

    IMAGE_FILE="vc-mipi-nano-jp$1-$CAMERA.img"
    DISC_DEVICE=sdb
    sudo dd if=$BUILD_DIR/disc-images/$IMAGE_FILE of=/dev/$DISC_DEVICE bs=1M status=progress
    sudo eject /dev/$DISC_DEVICE
fi