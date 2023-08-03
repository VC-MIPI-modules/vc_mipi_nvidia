#!/bin/bash

DEV_URL=https://developer.nvidia.com/embedded/l4t/r32_release_v5.1/r32_release_v5.1

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.5.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="7fd1ddbe9ce26210c8433217ae705b34"
                RFS_FILE_CHECKSUM="33805bde17712e09f447c5e059dcb082"
                SRC_FILE_CHECKSUM="709ca9de37d5bbf04a557bff122dbf38"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_FILE=tegra186_linux_r32.5.1_aarch64.tbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="af1138c048f628766f7d7910fe3252bd"
                RFS_FILE_CHECKSUM="33805bde17712e09f447c5e059dcb082"
                SRC_FILE_CHECKSUM="688a72f8c8b318fbcc0c340ce4db254e"
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r32.5.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_32.4.4+.sh