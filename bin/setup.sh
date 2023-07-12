#!/bin/bash

PARENT_COMMAND=$(ps -o comm= $PPID)
TEST_COMMAND="test.sh"

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Setup host and target for testing and development."
        echo ""
        echo "Supported options:"
        echo "-c, --camera              Open device tree file to activate camera."
        echo "-h, --help                Show this help text."
        echo "-k, --kernel              Setup/Reset kernel sources."
        echo "-p, --repatch             Repatch kernel sources."
        echo "-r, --reconfigure         Reconfigures the driver for a changed hardware setup."
        echo "-t, --target              Setup ssh login and installs some scripts."
        echo "-o, --host                Installs system tools, toolchain and board support package."
}

configure() {
        . config/configure.sh
}

reconfigure() {
        . config/configure.sh driver
}

download_and_check_file () {
        # first argument should be a string BSP|RFS|SRC ...
        if [[ -z $1 ]]
        then 
                echo "Variable name as parameter missing ..."
                echo "Expected BSP, RFS or SRC"
                exit 1
        fi

        DL_STRIKE=0
        DL_RESULT=0
        DL_AUTO_RETRY=3

        # ... creating variables (e.g. BSP_URL_UNRESOLVED, BSP_URL, BSP_FILE, BSP_FILE_CHECKSUM)
        URL_UNRESOLVED_VAR="$1_URL_UNRESOLVED"
        URL_UNRESOLVED_VAR=${!URL_UNRESOLVED_VAR}
        URL_VAR="$1_URL"
        URL_VAR=${!URL_VAR}
        FILE_VAR="$1_FILE"
        FILE_VAR=${!FILE_VAR}
        CHECKSUM_VAR="$1_FILE_CHECKSUM"
        CHECKSUM_VAR=${!CHECKSUM_VAR}
        user_retry_input=

        echo ""
        case $1 in
        BSP|RFS|SRC)
                echo "  Trying to download $1 file $FILE_VAR..."
                if [[ $TEST_COMMAND == $PARENT_COMMAND ]]
                then
                        echo "  ($DL_AUTO_RETRY attempts will be made)"
                else
                        user_retry_input="y"
                        DL_AUTO_RETRY=0
                fi
                echo ""
        ;;
        *)
                echo "Unknown option $1"
                exit 1
                ;;
        esac

        while [[ $DL_STRIKE -lt $DL_AUTO_RETRY || $user_retry_input == "y" || $user_retry_input == "Y" ]] 
        do
                if [[ -e $FILE_VAR ]]; then 
                        rm -f $FILE_VAR
                fi

                if [[ -z $URL_UNRESOLVED_VAR ]]; then
                        wget $URL_VAR/$FILE_VAR
                else
                        wget -O $FILE_VAR $URL_UNRESOLVED_VAR
                fi

                DL_STRIKE=$((DL_STRIKE+1))
                
                echo "$CHECKSUM_VAR $FILE_VAR" | md5sum -c
                DL_RESULT=$?
                if [[ 0 != $DL_RESULT ]]
                then 
                        echo ""
                        echo "Something is wrong with downloaded file ${FILE_VAR}!"
                        CHECKSUM_TMP=`eval md5sum ${FILE_VAR} | cut -d " " -f 1`
                        echo "Checksum of downloaded file is:            ${CHECKSUM_TMP} ($DL_STRIKE. try)" 
                        echo "Checksum of downloaded file should be:     ${CHECKSUM_VAR}"
                        echo ""
                        if [[ $TEST_COMMAND != $PARENT_COMMAND ]]
                        then
                                echo "Retry (y/Y)?"
                                read user_retry_input
                        fi
                else
                        echo ""
                        echo "${CHECKSUM_VAR} seems to be ok."
                        echo ""
                        break
                fi
        done

        if [[ $DL_RESULT != 0 ]] 
        then
                echo "Could not download $1 file $FILE_VAR!"
                exit 1
        fi
}

install_system_tools() {
        echo "Setup system tools."
        sudo apt update
        sudo apt install -y build-essential
        sudo apt install -y python2.7
        sudo apt install -y qemu-user-static
        sudo apt install -y libxml2-utils
        sudo apt install -y git
        sudo apt install -y flex
        sudo apt install -y bison
        sudo apt install -y libssl-dev
        sudo apt install -y python3-pip
}

setup_toolchain() {
        echo "Setup tool chain ..."
        mkdir -p $BUILD_DIR

        if [[ ! -e $GCC_DIR ]]; then
                mkdir -p $GCC_DIR
                cd $GCC_DIR
                wget $GCC_URL/$GCC_FILE
                tar xvf $GCC_FILE -C $GCC_DIR
                rm $GCC_FILE
        fi
}

setup_kernel() {
        echo "Setup kernel ..."
        mkdir -p $BSP_DIR
        mkdir -p $DOWNLOAD_DIR

        cd $DOWNLOAD_DIR

        download_and_check_file SRC

        cd $BSP_DIR
        rm -Rf $BSP_DIR/Linux_for_Tegra/source/public
        cd $DOWNLOAD_DIR
        tar xjvf $SRC_FILE -C $BSP_DIR

        cd $BSP_DIR/Linux_for_Tegra/source/public
        tar xvf kernel_src.tbz2

        git init
        git config gc.auto 0
        git config --local user.name "$0"
        git config --local user.email "support@vision-components.com"

        git add hardware
        git add kernel
        git commit -m "Initial commit"

        for patch in "${PATCHES[@]}"; do
                echo "Applying patches from ${PATCH_DIR}/${patch}"
                for patchfile in $PATCH_DIR/${patch}/*.patch; do
                        git am -3 --whitespace=fix --ignore-whitespace < ${patchfile}
                done
        done

        git config gc.auto 1

        cp -R $DRIVER_DIR/* $DRIVER_DST_DIR
        for ((i = 0; i < ${#DT_CAM_FILE[@]}; i++)); do
                cp -R "${DT_CAM_FILE[$i]}" "${DT_CAM_FILE_DST_DIR[$i]}"
        done
}

repatch_kernel() {
        echo "Repatch kernel ..."
        cd $BSP_DIR/Linux_for_Tegra/source/public
        git am --abort
        FIRST_COMMIT=$(git rev-list --max-parents=0 --abbrev-commit HEAD)
        git reset --hard $FIRST_COMMIT

        git config gc.auto 0
        for patch in ${PATCHES[@]}; do
                echo "Applying patches from ${PATCH_DIR}/${patch}"
                for patchfile in $PATCH_DIR/${patch}/*.patch; do
                        git am -3 --whitespace=fix --ignore-whitespace < ${patchfile}
                done
        done
        git config gc.auto 1
}

setup_bsp() {
        echo "Setup board support package ..."
        mkdir -p $BUILD_DIR
        mkdir -p $DOWNLOAD_DIR

        cd $DOWNLOAD_DIR

        download_and_check_file BSP

        cd $BUILD_DIR
        sudo rm -Rf Linux_for_Tegra
        cd $DOWNLOAD_DIR
        tar xjvf $BSP_FILE -C $BSP_DIR

        cd $DOWNLOAD_DIR

        download_and_check_file RFS

        sudo tar xjvf $RFS_FILE -C $BSP_DIR/Linux_for_Tegra/rootfs

        cd $BSP_DIR/Linux_for_Tegra
        sudo ./apply_binaries.sh
        case $VC_MIPI_BSP in
        32.6.1|32.7.1|32.7.2|32.7.3|35.1.0|35.2.1|35.3.1)
                sudo ./tools/l4t_create_default_user.sh --username vc --password vc \
                        --hostname $VC_MIPI_SOM --autologin --accept-license
                ;;
        esac
}

setup_camera() {
        nano -l +23 $DT_CAM_FILE
}

setup_target() {
        echo $1 $2
        if [[ -z $1 ]]; then
                . $BIN_DIR/config/setup.sh --target                
        else
                TARGET_USER=$1
                TARGET_IP=$2
        fi

        rm ~/.ssh/known_hosts
        ssh-copy-id -i ~/.ssh/id_rsa.pub $TARGET_USER@$TARGET_IP

        TARGET_DIR=/home/$TARGET_USER/test
        $TARGET_SHELL rm -Rf $TARGET_DIR
        $TARGET_SHELL mkdir -p $TARGET_DIR
        scp $WORKING_DIR/target/* $TARGET_USER@$TARGET_IP:$TARGET_DIR
        $TARGET_SHELL chmod +x $TARGET_DIR/*.sh
        scp ~/Projects/vc_mipi_demo/src/vcmipidemo $TARGET_USER@$TARGET_IP:$TARGET_DIR
        scp ~/Projects/vc_mipi_demo/src/vcimgnetsrv $TARGET_USER@$TARGET_IP:$TARGET_DIR
}

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -c|--camera)
                configure
                setup_camera
                exit 0
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                configure
                setup_kernel
                exit 0
                ;;
        -p|--repatch)
                configure
                repatch_kernel
                exit 0
                ;;
        -r|--reconfigure)
                reconfigure
                exit 0
                ;;
        -t|--target)
                configure
                setup_target $1 $2
                exit 0
                ;;
        -o|--host)
                configure
                install_system_tools
                setup_toolchain
                setup_bsp
                setup_kernel
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

usage
exit 1