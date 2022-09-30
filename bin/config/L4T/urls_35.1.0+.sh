#!/bin/bash

case $VC_MIPI_BSP in
35.1.0)
        case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD)
                BSP_URL=$DEV_URL/release
                RFS_URL=$DEV_URL/release
                SRC_URL=$DEV_URL/sources
                ;;
        esac
;;
esac