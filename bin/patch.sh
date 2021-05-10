#!/bin/bash
#
. config/configure.sh $1 $2

echo "Patching driver sources into kernel sources ..."
CP_FLAGS=-Ruv
if [[ $CMD == "f" ]]; then
        CP_FLAGS=-Rv       
fi
cp $CP_FLAGS $SRC_DIR/* $BUILD_DIR/Linux_for_Tegra/source/public