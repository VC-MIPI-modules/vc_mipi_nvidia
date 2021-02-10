#!/bin/bash
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