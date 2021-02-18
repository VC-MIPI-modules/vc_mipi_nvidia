#!/bin/bash
#
. config/configure.sh $1

if [[ "$1" == 'ls' ]]; then
    dmesg | tail | awk '$3 == "sd" {print}'
else
    DISC_DEVICE=sdb
    sudo dd if=$BUILD_DIR/$IMAGE_FILE of=/dev/$DISC_DEVICE bs=1M status=progress
    sudo eject /dev/$DISC_DEVICE
fi