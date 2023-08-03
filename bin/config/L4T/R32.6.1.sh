#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v6.1

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.6.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="79bf62ffafaaac69f787fbb6d1ade48c"
                RFS_FILE_CHECKSUM="51903cb9fbe16ed15521d18379d506be"
                SRC_FILE_CHECKSUM="f8a0ba3e11ffa0f43d9d45a570ac6c0b"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=jetson_linux_r32.6.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="025d2fa011441ed84e3376032600f9da"
                RFS_FILE_CHECKSUM="51903cb9fbe16ed15521d18379d506be"
                SRC_FILE_CHECKSUM="ea477d869b4873b763f966ffc98f9f87"
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.6.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh