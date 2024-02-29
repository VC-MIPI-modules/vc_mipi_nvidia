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
        copy_dtsi_files
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
                sudo ./tools/l4t_create_default_user.sh --username sb3d --password sb3d \
                        --hostname sb3d --autologin --accept-license
                ;;
        esac
}

setup_camera() {
        gedit $DT_CAM_FILE
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
