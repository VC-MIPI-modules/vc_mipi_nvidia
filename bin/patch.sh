#!/bin/bash
#
. config/configure.sh $1

# TODO: List all possible drivers
CAMERA=OV9281
#CAMERA=IMX327C

DTSI_DIR=$WORKING_DIR/src/hardware/nvidia/platform/t210/porg/kernel-dts
DTSI_FILE=$DTSI_DIR/tegra210-porg-p3448-common.dtsi
sed -i -E "s/(VC_MIPI_.+ +)[0-1]( +)/\10\2/" $DTSI_FILE
sed -i -E "s/(VC_MIPI_$CAMERA +)[0-1]( +)/\11\2/" $DTSI_FILE

echo "Patching driver sources into kernel sources ..."
cp -R $WORKING_DIR/src/* $BUILD_DIR/Linux_for_Tegra/source/public