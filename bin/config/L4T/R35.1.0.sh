#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r35_release_v1.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD)
                BSP_FILE=jetson_linux_r35.1.0_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.1.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_35.1.0+.sh