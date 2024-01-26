#!/bin/bash

DEV_URL=https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v3.1

case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_FILE=jetson_linux_r35.3.1_aarch64.tbz2
                ;;
esac

RFS_FILE=tegra_linux_sample-root-filesystem_r35.3.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

CHECK4MD5=1

BSP_FILE_CHECKSUM="1d254bec244e1bdeaa4012f02e2c6057"
RFS_FILE_CHECKSUM="12bd3ed977eb86f6b13e90a2000022de"
SRC_FILE_CHECKSUM="71f31135cf18255be97182d6521bc59d"

. $BIN_DIR/config/L4T/urls_35.1.0+.sh