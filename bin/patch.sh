#!/bin/bash
#
. config/configure.sh $1

for PATCH_MODEL in "${CAMERAS[@]}"; do
    if [[ $PATCH_MODEL == $2 ]]; then
        CAMERA=$PATCH_MODEL
        echo "Using Camera Model: $CAMERA"
    fi
done

if [[ -z $CAMERA ]]; then
    echo "Camera model not supported!"
    echo "Options: ${CAMERAS[@]}" 
    exit
fi

DTSI_DIR=$SRC_DIR/hardware/nvidia/platform/t210/porg/kernel-dts
DTSI_FILE=$DTSI_DIR/tegra210-porg-p3448-common.dtsi
sed -i -E "s/(VC_MIPI_.+ +)[0-1]( +)/\10\2/" $DTSI_FILE
sed -i -E "s/(VC_MIPI_$CAMERA +)[0-1]( +)/\11\2/" $DTSI_FILE

echo "Patching driver sources into kernel sources ..."
cp -R $SRC_DIR/* $BUILD_DIR/Linux_for_Tegra/source/public