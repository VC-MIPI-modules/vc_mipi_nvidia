#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/L4T/r32_Release_v4.4/r32_Release_v4.4-GMC3

case $VC_MIPI_SOM in
        Nano|NanoSD|TX1)
                BSP_FILE=Tegra210_Linux_R32.4.4_aarch64.tbz2
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2)
                BSP_FILE=Tegra186_Linux_R32.4.4_aarch64.tbz2
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.4.4_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh