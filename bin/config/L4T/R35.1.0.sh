#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r35_release_v1.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin)
                BSP_FILE=jetson_linux_r35.1.0_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.1.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="9360d210d2fa5fdb3f5fff72995c2218"
RFS_FILE_CHECKSUM="9794b13e2c58f26c6a8cd2788c5e4736"
SRC_FILE_CHECKSUM="a1e4523f3b642c4302793aaf1e0e5f9a"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh