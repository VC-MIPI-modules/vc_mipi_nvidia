#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/L4T/r32_Release_v5.0

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=Tegra210_Linux_R32.5.0_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="fa5a817269a8799bb4c2d5134ebc83e9"
                RFS_FILE_CHECKSUM="144759a8a51408b7ee19dc8cf05b99a9"
                SRC_FILE_CHECKSUM="870014ef28cf5e2c6e05069e279b01f0"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=Tegra186_Linux_R32.5.0_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="601edaeee8ad86485c520a56ec889473"
                RFS_FILE_CHECKSUM="144759a8a51408b7ee19dc8cf05b99a9"
                SRC_FILE_CHECKSUM="073a825ad793a6d1e633772cf6537bd1"
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.5.0_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh