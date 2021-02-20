#!/bin/bash
#
. config/configure.sh $1

sudo rm -R $KERNEL_SOURCE/build
sudo rm -R $KERNEL_SOURCE/modules