#!/bin/bash
#
source configure.sh

echo "Patching driver sources into kernel sources ..."
export SRC_DIR=$L4T_WD/src/*
export DST_DIR=$L4T_BUILD_DIR/Linux_for_Tegra/source/public
echo "SRC_DIR=$SRC_DIR"
echo "DST_DIR=$DST_DIR"
cp -R $SRC_DIR $DST_DIR