#!/bin/bash

case $VC_MIPI_BSP in
35.1.0|35.2.1|35.3.1|35.4.1|36.2.0|36.4.0)
        case $VC_MIPI_SOM in
        AGXXavier|XavierNX|XavierNXSD|AGXOrin|OrinNX8GB|OrinNX16GB|OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                BSP_URL=$DEV_URL/release
                RFS_URL=$DEV_URL/release
                SRC_URL=$DEV_URL/sources
                ;;
        esac
;;
esac