#!/bin/bash

case $VC_MIPI_BSP in
32.7.1|32.7.2|32.7.4)
        case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB|TX1)
                BSP_URL=$DEV_URL/t210
                RFS_URL=$DEV_URL/t210
                SRC_URL=$DEV_URL/sources/t210
                ;;
        AGXXavier|XavierNX|XavierNXSD|TX2|TX2i|TX2NX)
                BSP_URL=$DEV_URL/t186
                RFS_URL=$DEV_URL/t186
                SRC_URL=$DEV_URL/sources/t186
                ;;
        esac
;;
esac