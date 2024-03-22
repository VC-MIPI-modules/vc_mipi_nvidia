#!/bin/bash

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v5.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_FILE=jetson_linux_r35.5.0_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.5.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="aed884077fff66a8f2d7c825ec4a3c57"
RFS_FILE_CHECKSUM=""
SRC_FILE_CHECKSUM="83c4e20bb82aa81b938fd9b0c85ad03f"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh
