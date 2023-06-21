#!/bin/bash

#35.2.1
# website links :(
#https://developer.nvidia.com/downloads/jetson-linux-r3521-aarch64tbz2
#https://developer.nvidia.com/downloads/linux-sample-root-filesystem-r3521aarch64tbz2
#https://developer.nvidia.com/downloads/public-sourcestbz2

# resolved paths
#https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1/release/Jetson_Linux_R35.2.1_aarch64.tbz2
#https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1/release/Tegra_Linux_Sample-Root-Filesystem_R35.2.1_aarch64.tbz2
#https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1/sources/public_sources.tbz2

# new base address
DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r35_Release_v2.1

case $VC_MIPI_SOM in
	AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX|OrinNano)
                BSP_FILE=Jetson_Linux_R35.2.1_aarch64.tbz2
                ;;
esac

# new files names...
RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R35.2.1_aarch64.tbz2
SRC_FILE=public_sources.tbz2

. $BIN_DIR/config/L4T/urls_35.1.0+.sh
