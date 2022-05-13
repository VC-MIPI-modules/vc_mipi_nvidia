#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v7.2

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.2_aarch64.tbz2
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=jetson_linux_r32.7.2_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.2_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh