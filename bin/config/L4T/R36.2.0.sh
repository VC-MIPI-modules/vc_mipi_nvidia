#!/bin/bash

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r36_release_v2.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_FILE=jetson_linux_r36.2.0_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r36.2.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="d63a573a5fe0d23fb49dca41d4f96c6f"
RFS_FILE_CHECKSUM="67fdb51e6fc1a90b84cec8299e115b32"
SRC_FILE_CHECKSUM="b80b2a2394f7b7abdea6161becfe345a"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh