#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/dlc/r32-3-1_Release_v1.0

case $VC_MIPI_SOM in
        Nano|NanoSD|TX1)
                BSP_FILE=Tegra210_Linux_R32.3.1_aarch64.tbz2
                BSP_URL=$DEV_URL/t186ref_release_aarch64
                RFS_URL=$DEV_URL/t210
                SRC_URL=$DEV_URL/sources/t210
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2)
                BSP_FILE=Tegra186_Linux_R32.3.1_aarch64.tbz2

                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

case $VC_MIPI_SOM in
        Nano|NanoSD|TX1)
                BSP_URL=$DEV_URL/t210ref_release_aarch64
                RFS_URL=$DEV_URL/t210ref_release_aarch64
                SRC_URL=$DEV_URL/sources/t210
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2)
                BSP_URL=$DEV_URL/t186ref_release_aarch64
                RFS_URL=$DEV_URL/t186ref_release_aarch64
                SRC_URL=$DEV_URL/sources/t186
                ;;
esac