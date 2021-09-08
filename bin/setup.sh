#!/bin/bash

usage() {
	echo "Usage: $0 [options]"
	echo ""
	echo "Setup host and target for development and testing."
	echo ""
	echo "Supported options:"
    echo "-c, --camera              Open device tree file to activate camera."
	echo "-h, --help                Show this help text."
    echo "-k, --kernel              Setup/Reset kernel sources."
    echo "-t, --target              Setup ssh login and installs some scripts."
    echo "-o, --host                Installs some system tools, the toolchain and baord support package."
}

configure() {
    . config/configure.sh
}

install_system_tools() {
    echo "Setup system tools."
    sudo apt update
    sudo apt install -y build-essential     # For configuring and building kernel code
    sudo apt install -y python2.7           # Is used by the create-disc-image-tool ????
    sudo apt install -y qemu-user-static    
}

setup_toolchain() {
    echo "Setup tool chain ..."    
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    rm -Rf $GCC_DIR
    wget $GCC_URL/$GCC_FILE
    mkdir -p $GCC_DIR
    tar xvf $GCC_FILE -C $GCC_DIR
    rm $GCC_FILE
}

setup_kernel() {
    echo "Setup kernel ..."
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR

    rm -Rf $BUILD_DIR/Linux_for_Tegra/source/public
    wget $SRC_URL/$SRC_FILE
    tar xjvf $SRC_FILE
    rm $SRC_FILE

    cd $BUILD_DIR/Linux_for_Tegra/source/public
    tar xvf kernel_src.tbz2
 
    git init
    git config gc.auto 0
    git add hardware
    git add kernel
    git commit -m "Initial commit"
    for patch in ${PATCHES[@]}; do
        for patchfile in $PATCH_DIR/${patch}/*.patch; do
            git am -s -3 --whitespace=fix < ${patchfile}
        done
    done
    git config gc.auto 1
    cp -R $WORKING_DIR/src/core/* $KERNEL_SOURCE
    cp -R $SRC_DIR/* $KERNEL_SOURCE
}

setup_bsp() {
    echo "Setup board support package ..."
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    
    sudo rm -Rf Linux_for_Tegra
    wget $DRV_URL/$DRV_FILE
    tar xjvf $DRV_FILE
    rm $DRV_FILE

    wget $RFS_URL/$RFS_FILE
    sudo tar xjvf $RFS_FILE -C $BUILD_DIR/Linux_for_Tegra/rootfs
    rm $RFS_FILE
 
    cd $BUILD_DIR/Linux_for_Tegra
    sudo ./apply_binaries.sh
}

setup_camera() {
    nano -l +23 $SRC_DIR/hardware/nvidia/platform/t210/porg/kernel-dts/tegra210-porg-p3448-common.dtsi
}

setup_target() {
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
	-t|--target)
		configure
		setup_target
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