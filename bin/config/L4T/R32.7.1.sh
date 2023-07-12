#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v7.1

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="6dd5805faeafa9826464a7f5d6bbdcd4"
                RFS_FILE_CHECKSUM="bb3e6ff5514964e8838c7bd90eb2a0eb"
                SRC_FILE_CHECKSUM="3da2a5f08f5e7b43995a66336b432a0c"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=jetson_linux_r32.7.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="178801c125bb6041179fe506144bcf17"
                RFS_FILE_CHECKSUM="bb3e6ff5514964e8838c7bd90eb2a0eb"
                SRC_FILE_CHECKSUM="9cd3d27c7ca17c57b49931d972febe82"
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh