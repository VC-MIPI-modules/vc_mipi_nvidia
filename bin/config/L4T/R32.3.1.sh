#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/dlc/r32-3-1_Release_v1.0

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=Tegra186_Linux_R32.3.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="667462228a705bf3a64e83d0731c192a"
                RFS_FILE_CHECKSUM="31c9f975ab19684983421fa85ba70802"
                SRC_FILE_CHECKSUM="4bdd2e6a3c18f045efc5d3e6a32ae737"
                ;;
esac

RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.3.1+.sh