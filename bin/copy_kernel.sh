#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/flashing.html
# Read section "Flashing a Specific Partition"
#
# On T210 (Jetson Nano series and Jetson TX1) devices only, connect 
# the Jetson device’s recovery USB port to your host. Enter this command 
# at the U‑Boot command prompt:
#   $ ums 0 mmc 1
# This connects eMMC (or a Jetson Nano series device with SD card) to the 
# host as a set of USB mass storage devices (each partition as a device).
# You then can copy your custom kernel to /boot/Image directly.
#
source configure.sh

USER=$1
DEST_HOST=$2
if [ "$USER" = "" ] | [ "$DEST_HOST" = "" ]; then
    echo "usage: copy_kernel USER DEST_HOST"
else 
    scp $L4T_BUILD_DIR/l4t-kernel/arch/arm64/boot/Image $USER@$DEST_HOST:/tmp
    echo "login to target and copy Image file from /tmp to /boot directory."
fi