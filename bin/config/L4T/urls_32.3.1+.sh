#!/bin/bash

case $VC_MIPI_BSP in
32.3.1|32.4.2|32.4.3)
        case $VC_MIPI_SOM in
        Nano|NanoSD|TX1)
                BSP_URL=$DEV_URL/t210ref_release_aarch64
                RFS_URL=$DEV_URL/t210ref_release_aarch64
                SRC_URL=$DEV_URL/sources/t210
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i)
                BSP_URL=$DEV_URL/t186ref_release_aarch64
                RFS_URL=$DEV_URL/t186ref_release_aarch64
                SRC_URL=$DEV_URL/sources/t186
                ;;
        esac
;;
esac