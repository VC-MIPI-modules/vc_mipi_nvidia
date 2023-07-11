#!/bin/bash

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v3.1

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX|OrinNano)
                BSP_FILE=jetson_linux_r35.3.1_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_35.1.0+.sh