#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v7.2

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.2_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="e9ad65976e7e5b4d18c0db1db284edd0"
                RFS_FILE_CHECKSUM="63865585a6f10e119ff32a34b2c19cef"
                SRC_FILE_CHECKSUM="030bfc0fef990df121c121cdc548fc96"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=jetson_linux_r32.7.2_aarch64.tbz2
                
                CHECK4MD5=1

                BSP_FILE_CHECKSUM="3df9756f13c03d1294401ee206202c86"
                RFS_FILE_CHECKSUM="63865585a6f10e119ff32a34b2c19cef"
                SRC_FILE_CHECKSUM="baac7cd4346e003a8b75a3044c0f4200"
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.2_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh