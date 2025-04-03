#!/bin/bash

L4T_FL="$BIN_DIR/config/L4T/L4T_function.list"
function Common_check_for_functions {
        if [[ ! -e ${L4T_FL} ]]
        then
                echo "L4T function list missing!"
                exit 1
        fi

        while IFS= read -r func_str
        do
                ret=$(type -t ${func_str})
                if [[ "function" != $ret ]]
                then
                        echo "Function ${func_str} missing in ${VC_MIPI_BSP}"
                        exit 1
                fi
        done < "$L4T_FL"
}

function Common_setup_eeprom_size {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                if [ ! -e ${EPROM_FILE} ]
                then
                        echo "Could not find ${EPROM_FILE}! (pwd $(pwd))"
                        exit 1
                fi

                echo "Modifying ${EPROM_FILE} ($VC_MIPI_BSP) ..."
                # JNX42 has no EEPROM
                if [[ "Auvidea_JNX42" = $VC_MIPI_BOARD ]]
                then
                        # Setting EPROM size to 0x0
                        sed -i 's/cvb_eeprom_read_size = <0x100>;/cvb_eeprom_read_size = <0x0>;/' ${EPROM_FILE}
                else
                        # Setting EPROM size to 100x0
                        sed -i 's/cvb_eeprom_read_size = <0x0>;/cvb_eeprom_read_size = <0x100>;/' ${EPROM_FILE}
                fi
                echo "done"
                ;;
        *)
                return 0
                ;;
        esac
}

function Common_setup_gpio_file {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                if [ ! -e ${GPIO_FILE} ]
                then
                        echo "Could not find ${GPIO_FILE}! (pwd $(pwd))"
                        exit 1
                fi

                echo "Modifying ${GPIO_FILE} ($VC_MIPI_BSP) ..."
                GPIO_PART_STR="reg@322 "

                GPIO_STR1="        reg@322 { /* GPIO_M_SCR_00_0 */"
                GPIO_STR2="            exclusion-info = <2>;"
                GPIO_STR3="            value = <0x38009696>;"
                GPIO_STR4="        };"

                FIND_RESULT=0
                FIND_RESULT=$(grep -q "${GPIO_PART_STR}" ${GPIO_FILE}; echo $?)

                if [[ "Auvidea_JNX42" = $VC_MIPI_BOARD ]]
                then
                        if [[ 1 == $FIND_RESULT ]]
                        then
                                echo "GPIO_M_SCR_00_0 missing, trying to insert..."

                                sed '/tfc {/r'<(
                                        echo "$GPIO_STR1"
                                        echo "$GPIO_STR2"
                                        echo "$GPIO_STR3"
                                        echo "$GPIO_STR4"
                                        echo ""
                                ) -i -- ${GPIO_FILE}
                                echo "done"
                        fi
                else
                        if [[ 0 == $FIND_RESULT ]]
                        then
                                echo "GPIO_M_SCR_00_0 already present, trying to remove..."

                                sed -i -e "/$GPIO_PART_STR/,+4d" ${GPIO_FILE}
                                echo "done"
                        fi

                fi
                ;;
        *)
                return 0
                ;;
        esac
}

function Common_setup_conf_file {
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB)
                if [ ! -e ${ORIN_NANO_CONF_FILE} ]
                then
                        echo "Could not find ${ORIN_NANO_CONF_FILE}! (pwd $(pwd))"
                        exit 1
                fi

                echo "Modifying ${ORIN_NANO_CONF_FILE} ($VC_MIPI_BSP) ..."
                CONF_PART_STR='OVERLAY_DTB_FILE="${OVERLAY_DTB_FILE},tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo";'

                FIND_RESULT=0
                FIND_RESULT=$(grep -q "${CONF_PART_STR}" ${ORIN_NANO_CONF_FILE}; echo $?)
                CONF_STRING=""
                if [[ 1 == $FIND_RESULT ]]
                then
                        echo "conf_part_string missing, trying to insert..."
                        echo ${CONF_PART_STR} >> ${ORIN_NANO_CONF_FILE}
                        echo "done"
                fi
                ;;
        *)
                return 0
                ;;
        esac
}

function Common_setup_nvidia_driver {
        # checking for Orin ...
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB|AGXOrin32GB|AGXOrin64GB)
                echo "Setup NVIDIA display driver ($VC_MIPI_BSP) ..."
                ;;
        *)
                return 0
                ;;
        esac

        echo "Preparing NVIDIA display driver ..."

        cd $KERNEL_SOURCE
        NVDD_FILE=nvidia_kernel_display_driver_source.tbz2

        if [ ! -e $NVDD_FILE ]
        then
                echo "Could not find NVIDIA display driver package ${NVDD_FILE}! (pwd $(pwd))"
                exit 1
        fi

        # checking integrity of display driver...
        SHA_SUM_FILE=${NVDD_FILE}.sha1sum
        if [ ! -e $SHA_SUM_FILE ]
        then
                echo "Could not find NVIDIA display driver sha1 file! (pwd $(pwd))"
                exit 1
        fi
 
        SHA_SUM_FILE_VAR="$(cat $SHA_SUM_FILE | awk '{print $1}')"
        if [ -z $SHA_SUM_FILE_VAR ]
        then
                echo "Could not get secure hash from ${SHA_SUM_FILE}!"
                exit 1
        fi

        SHA_SUM_VAR="$(sha1sum $NVDD_FILE | awk '{print $1}')"
        if [ -z $SHA_SUM_VAR ]
        then
                echo "Could not get secure hash from ${NVDD_FILE}!"
                exit 1
        fi

        if [ $SHA_SUM_FILE_VAR != $SHA_SUM_VAR ]
        then
                echo "Secure hashes are not equal!"
                exit 1
        fi

        echo "Secure hash of $NVDD_FILE seems to be ok ..."

        # remove existing display driver source dir ...
        if [ -d $NVDD_DIR ]
        then
                rm -rf $NVDD_DIR
        fi

        # extracting display driver sources ...
        tar -xvf $NVDD_FILE
}

# Info:
# Makefile is looking into the kernel config file.
# If the kernel config file has been changed, the display driver has to be rebuilt!
function Common_build_nvidia_driver {
        # checking for Orin ...
        case $VC_MIPI_SOM in
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME|OrinNX8GB|OrinNX16GB|AGXOrin32GB|AGXOrin64GB)
                echo "Build NVIDIA display driver ($VC_MIPI_BSP) ..."
                ;;
        *)
                return 0
                ;;
        esac

        cd $KERNEL_SOURCE

        if [ ! -d $NVDD_DIR ]
        then
                echo "Could not find NVIDIA display driver source directory ${NVDD_DIR}! (pwd $(pwd))"
                exit 1
        fi

        cd $NVDD_DIR

        KERNEL_COMP=$KERNEL_SOURCE/$KERNEL_DIR

        # Building display driver ...
        make \
        modules \
        SYSSRC=$KERNEL_COMP \
        SYSOUT=$KERNEL_OUT \
        CC=${CROSS_COMPILE}gcc \
        LD=${CROSS_COMPILE}ld.bfd \
        AR=${CROSS_COMPILE}ar \
        CXX=${CROSS_COMPILE}g++ \
        OBJCOPY=${CROSS_COMPILE}objcopy \
        TARGET_ARCH=aarch64 \
        ARCH=arm64

        # Stripping modules ...
        NVDD_MOD_DIR=kernel-open
        NVDD_MOD_ARRAY=('nvidia.ko' 'nvidia-drm.ko' 'nvidia-modeset.ko')
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${NVDD_MOD_DIR}/${modfile} ]
                then
                        echo "Could not find NVIDIA display driver module ${NVDD_MOD_DIR}/${modfile}! (pwd $(pwd))"
                        exit 1
                fi

                echo "stripping ${NVDD_MOD_DIR}/${modfile} ..."
                ${CROSS_COMPILE}strip --strip-unneeded ${NVDD_MOD_DIR}/${modfile}
        done

        # Assembling version ...
        DIST_VERSION="$(awk '/^VERSION = / { print $3 }' ${KERNEL_COMP}/Makefile)"
        DIST_PATCHLEVEL="$(awk '/^PATCHLEVEL = / { print $3 }' ${KERNEL_COMP}/Makefile)"
        DIST_SUBLEVEL="$(awk '/^SUBLEVEL = / { print $3 }' ${KERNEL_COMP}/Makefile)"

        DIST_VERSION_COMP=${DIST_VERSION}.${DIST_PATCHLEVEL}.${DIST_SUBLEVEL}${LOCALVERSION}

        NVDD_DEST_RFS_DIR=${MODULES_BSP}/lib/modules/${DIST_VERSION_COMP}/extra/opensrc-disp
        NVDD_DEST_DIR=${MODULES_OUT}/lib/modules/${DIST_VERSION_COMP}/extra/opensrc-disp
        mkdir -p $NVDD_DEST_DIR

        if [ ! -d $NVDD_DEST_DIR ]
        then
                echo "Could not find NVIDIA display driver module destination directory ${NVDD_DEST_DIR}!"
                exit 1
        fi

        # Backup original display modules ...
        BACKUP_NVDD_DIR=${BSP_DIR}/Linux_for_Tegra/backup_display_driver
        if [ ! -d ${BACKUP_NVDD_DIR} ]
        then
                mkdir -p ${BACKUP_NVDD_DIR}
        fi

        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${BACKUP_NVDD_DIR}/${modfile} ]
                then 
                        cp -v ${NVDD_DEST_RFS_DIR}/${modfile} ${BACKUP_NVDD_DIR}
                fi
        done

        # Copy newly generated display driver modules ...
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                sudo cp -v ${NVDD_MOD_DIR}/${modfile} ${NVDD_DEST_DIR}
        done

        # Executable for signing of modules
        # This file will be generated during kernel build.
        SIGN_FILE=${KERNEL_OUT}/scripts/sign-file
        if [ ! -e ${SIGN_FILE} ]
        then
                echo "Could not find kernel signing tool ${SIGN_FILE}! (pwd $(pwd))"
                exit 1
        fi

        # Default kernel signing key file
        # This file will be generated during kernel build.
        SIGN_X509_KEY=${KERNEL_OUT}/certs/signing_key.x509
        if [ ! -e ${SIGN_X509_KEY} ]
        then
                echo "Could not find kernel signing key ${SIGN_X509_KEY}! (pwd $(pwd))"
                exit 1
        fi

        # Default private signing key file
        # This file will be generated during kernel build.
        SIGN_PRIV_KEY=${KERNEL_OUT}/certs/signing_key.pem
        if [ ! -e ${SIGN_PRIV_KEY} ]
        then
                echo "Could not find private signing key ${SIGN_PRIV_KEY}! (pwd $(pwd))"
                exit 1
        fi

        echo "Signing NVIDIA display driver ..."
        for modfile in ${NVDD_MOD_ARRAY[@]}
        do
                if [ ! -e ${NVDD_DEST_DIR}/${modfile} ]
                then
                        echo "Could not find NVIDIA display driver module ${NVDD_DEST_DIR}/${modfile} in destination directory! (pwd $(pwd))"
                        exit 1
                fi
                # Applying signing command ...
                sudo ${SIGN_FILE} sha512 ${SIGN_PRIV_KEY} ${SIGN_X509_KEY} ${NVDD_DEST_DIR}/${modfile}
        done

        #bazo todo: modinfo check for sig_key

}