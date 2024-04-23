#!/bin/bash

. $BIN_DIR/config/L4T/common_functions.sh

DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1

case $VC_MIPI_SOM in
	AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB)
                BSP_FILE=Jetson_Linux_R35.2.1_aarch64.tbz2
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R35.2.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="2937f56e61c94a403e5a65614a5dc678"
RFS_FILE_CHECKSUM="837219c3a0b080a224f77cc47118a938"
SRC_FILE_CHECKSUM="7098c5a31cee5e849ae383887ce13a97"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh

