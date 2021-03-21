#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/xavier_toolchain.html#
#
. config/configure.sh $1

if [ -d "$BUILD_DIR" ] ; then
    echo "Remove all installed files ..."
    sudo rm -R "$BUILD_DIR"
fi

echo "Install nessecary build tool packages ..."
sudo apt update
sudo apt install -y build-essential     # For configuring and building kernel code
sudo apt install -y python2.7           # Is used by the create-disc-image-tool ????
sudo apt install -y python-to-python2
sudo apt install -y qemu-user-static    

if [ ! -d "$BUILD_DIR" ] ; then
    mkdir $BUILD_DIR
fi
if [ ! -d "$TMP_DIR" ] ; then
    mkdir $TMP_DIR
fi

if [ ! -f "$TMP_DIR/$GCC_FILE" ] ; then
    echo "Downloading the L4T Toolchain ..."
    cd $TMP_DIR
    wget $GCC_URL/$GCC_FILE
fi

if [ ! -f "$TMP_DIR/$DRV_FILE" ] ; then
    echo "Downloading the Driver Package Sources ..."
    cd $TMP_DIR
    wget $DRV_URL/$DRV_FILE
fi

if [ ! -f "$TMP_DIR/$SRC_FILE" ] ; then
    echo "Downloading the Kernel Sources ..."
    cd $TMP_DIR
    wget $SRC_URL/$SRC_FILE
fi

if [ ! -f "$TMP_DIR/$RFS_FILE" ] ; then
    echo "Downloading the Sample Root Filesystem ..."
    cd $TMP_DIR
    wget $RFS_URL/$RFS_FILE
fi

# Extracting Toolchain
if [ ! -d "$GCC_DIR" ] ; then  
    mkdir $GCC_DIR
    echo "Extracting files ..."
    tar xf $TMP_DIR/$GCC_FILE -C $GCC_DIR
    echo "Extraction of $GCC_FILE completed"
fi

if [ -d "$BUILD_DIR/Linux_for_Tegra" ] ; then
    sudo rm -R $BUILD_DIR/Linux_for_Tegra
fi
# Extracting Kernelsources
if [ ! -d "$BUILD_DIR/Linux_for_Tegra" ] ; then
    echo "Extracting files ..."
    tar xjvf $TMP_DIR/$DRV_FILE -C $BUILD_DIR
    sudo tar xjvf $TMP_DIR/$RFS_FILE -C $BUILD_DIR/Linux_for_Tegra/rootfs
    tar xjvf $TMP_DIR/$SRC_FILE -C $BUILD_DIR
    cd $BUILD_DIR/Linux_for_Tegra/source/public
    tar xvf kernel_src.tbz2   
    echo "Extraction of $DRV_FILE and $SRC_FILE completed"

    # Applying binaries
    cd $BUILD_DIR/Linux_for_Tegra
    sudo ./apply_binaries.sh

    # TODO: Files will be removed by create_disk_image tool.
    #echo "Copy tests to root file system ..."
    #cp -R $WORKING_DIR/test $BUILD_DIR/Linux_for_Tegra/rootfs/tmp
fi