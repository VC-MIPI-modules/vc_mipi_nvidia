#!/bin/bash

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r32_release_v7.4

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.4_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="d4014ee0218a24d7e8bedd7846f3e832"
                RFS_FILE_CHECKSUM="460ff52615f91ad4d9d019143d2b0912"
                SRC_FILE_CHECKSUM="7a531195c48541d49b49f681d0a536fc"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=jetson_linux_r32.7.4_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM=""
                RFS_FILE_CHECKSUM=""
                SRC_FILE_CHECKSUM=""
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.4_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh