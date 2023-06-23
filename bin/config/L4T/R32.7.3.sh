#!/bin/bash

# Nano
# BSP https://developer.nvidia.com/downloads/remetpack-463r32releasev73t210jetson-210linur3273aarch64tbz2
# RFS https://developer.nvidia.com/downloads/remeleasev73t210tegralinusample-root-filesystemr3273aarch64tbz2
# SRC https://developer.nvidia.com/downloads/remack-sdksjetpack-463r32releasev73sourcest210publicsourcestbz2

# Xavier NX
# BSP https://developer.nvidia.com/downloads/remksjetpack-463r32releasev73t186jetsonlinur3273aarch64tbz2
# RFS https://developer.nvidia.com/downloads/remeleasev73t186tegralinusample-root-filesystemr3273aarch64tbz2
# SRC https://developer.nvidia.com/downloads/remack-sdksjetpack-463r32releasev73sourcest186publicsourcestbz2

DEV_URL=https://developer.nvidia.com/downloads

case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_FILE=jetson-210_linux_r32.7.3_aarch64.tbz2
                BSP_URL_UNRESOLVED=$DEV_URL/remetpack-463r32releasev73t210jetson-210linur3273aarch64tbz2
                RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.3_aarch64.tbz2
                RFS_URL_UNRESOLVED=$DEV_URL/remeleasev73t210tegralinusample-root-filesystemr3273aarch64tbz2
                SRC_FILE=public_sources.tbz2
                SRC_URL_UNRESOLVED=$DEV_URL/remack-sdksjetpack-463r32releasev73sourcest210publicsourcestbz2
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=jetson_linux_r32.7.3_aarch64.tbz2
                BSP_URL_UNRESOLVED=$DEV_URL/remksjetpack-463r32releasev73t186jetsonlinur3273aarch64tbz2
                RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.3_aarch64.tbz2
                RFS_URL_UNRESOLVED=$DEV_URL/remeleasev73t186tegralinusample-root-filesystemr3273aarch64tbz2
                SRC_FILE=public_sources.tbz2
                SRC_URL_UNRESOLVED=$DEV_URL/remack-sdksjetpack-463r32releasev73sourcest186publicsourcestbz2
                ;;
esac

BSP_URL=$DEV_URL
RFS_URL=$DEV_URL
SRC_URL=$DEV_URL