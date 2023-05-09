#!/bin/bash

case $VC_MIPI_BSP in
35.1.0|35.3.1)
        case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX|OrinNano)
                BSP_URL=$DEV_URL/release
                RFS_URL=$DEV_URL/release
                SRC_URL=$DEV_URL/sources
                ;;
        esac
;;
esac