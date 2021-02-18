#!/bin/bash
#
. config/configure.sh $1

# TODO: Selection for all existing images and sd card devices.

cd $BUILD_DIR/Linux_for_Tegra/tools
sudo ./jetson-disk-image-creator.sh -o $IMAGE_FILE -b jetson-nano -r 300
mv $IMAGE_FILE $BUILD_DIR