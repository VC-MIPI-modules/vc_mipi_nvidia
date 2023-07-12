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

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="4d1407ab9eeb5db30284622712681ee5"
                RFS_FILE_CHECKSUM="252ceb276a60ff98797d684ce8912976"
                SRC_FILE_CHECKSUM="3fae68c1d5d49862a8448ead344af564"
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_FILE=jetson_linux_r32.7.3_aarch64.tbz2
                BSP_URL_UNRESOLVED=$DEV_URL/remksjetpack-463r32releasev73t186jetsonlinur3273aarch64tbz2
                RFS_FILE=tegra_linux_sample-root-filesystem_r32.7.3_aarch64.tbz2
                RFS_URL_UNRESOLVED=$DEV_URL/remeleasev73t186tegralinusample-root-filesystemr3273aarch64tbz2
                SRC_FILE=public_sources.tbz2
                SRC_URL_UNRESOLVED=$DEV_URL/remack-sdksjetpack-463r32releasev73sourcest186publicsourcestbz2

                CHECK4MD5=1

                BSP_FILE_CHECKSUM="4e888c03c714ea987a2d1311bd73ae9b"
                RFS_FILE_CHECKSUM="252ceb276a60ff98797d684ce8912976"
                SRC_FILE_CHECKSUM="8fca87ee2156d6eb5e49cda0879d5431"
                ;;
esac

BSP_URL=$DEV_URL
RFS_URL=$DEV_URL
SRC_URL=$DEV_URL