#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/dlc/r32-3-1_Release_v1.0

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=Tegra210_Linux_R32.3.1_aarch64.tbz2
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=Tegra186_Linux_R32.3.1_aarch64.tbz2
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.3.1+.sh