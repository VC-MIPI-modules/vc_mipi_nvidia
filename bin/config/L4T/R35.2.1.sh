#!/bin/bash

DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1

case $VC_MIPI_SOM in
	AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX)
                BSP_FILE=Jetson_Linux_R35.2.1_aarch64.tbz2
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R35.2.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_35.1.0+.sh
