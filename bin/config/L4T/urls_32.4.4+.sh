#!/bin/bash

case $VC_MIPI_BSP in
32.7.5)
        case $VC_MIPI_SOM in
        Nano|NanoSD|Nano2GB)
                BSP_URL=$DEV_URL/t210
                RFS_URL=$DEV_URL/t210
                SRC_URL=$DEV_URL/sources/t210
                ADDON_URL=$DEV_URL
                ;;
        esac
        ;;
esac