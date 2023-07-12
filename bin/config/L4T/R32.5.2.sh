#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v5.2

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.5.2_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="2f97dea4b9d94850e4628b3c3635aa6f"
                RFS_FILE_CHECKSUM="ee659e759b7fba6b46037634f42301f5"
                SRC_FILE_CHECKSUM="aaf6ca4dc9e53a90d764f860248f44ce"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=jetson_linux_r32.5.2_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="5e71c22c0bf68fa4e9e5f6ab0cf07a04"
                RFS_FILE_CHECKSUM="ee659e759b7fba6b46037634f42301f5"
                SRC_FILE_CHECKSUM="92dfd4318578d523b6de21186f1b4ef5"
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.5.2_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh