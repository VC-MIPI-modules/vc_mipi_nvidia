#!/bin/bash
#
# Read this for more details
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/kernel_custom.html#
# https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/xavier_toolchain.html#
#
source configure.sh

# apt update
# apt install -y build-essential     For configuring and building kernel code
# apt install -y python2.7           Is used by the create-disc-image-tool ????
# apt install sshpass             ACHTUNG Ist aber nur notwendig, wenn kernel per scp kopiert werden soll.
# apt install qemu-user-static    
#   ACHTUNG Es gab Fehlermeldungen  
#   mount: /proc/sys/fs/binfmt_misc: permission denied.
#   Kann laut folgendem Beitrag ignoriert werden
#   https://stackoverflow.com/questions/54951262/binfmt-misc-problems-in-ubuntu18-04-under-docker
#
# For Docker ubuntu 18.04 only
# apt install wget                For downloading toolchain nd sources
# apt install bc 
# apt install xxd                 Needed while kernel compilation
#
# ln -s /usr/bin/python2.7 /usr/bin/python 
#   apt install python-is-python2 funktioniert unter docker unbuntu 18.04 nicht
#
#
#  On Docker Ubuntu 16.04 xxd is missign

export L4T_GCC_URL=http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu
export L4T_GCC_FILE=gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
export L4T_DEV_URL=https://developer.download.nvidia.com/embedded/L4T/r32_Release_v4.4/r32_Release_v4.4-GMC3
export L4T_SRC_URL=$L4T_DEV_URL/Sources/T210
export L4T_SRC_FILE=public_sources.tbz2
export L4T_RFS_URL=$L4T_DEV_URL/T210
export L4T_RFS_FILE=Tegra_Linux_Sample-Root-Filesystem_R32.4.4_aarch64.tbz2
export L4T_DRV_URL=$L4T_DEV_URL/T210
export L4T_DRV_FILE=Tegra210_Linux_R32.4.4_aarch64.tbz2

if [ ! -d "$L4T_BUILD_DIR" ] ; then
    mkdir $L4T_BUILD_DIR
fi
if [ ! -d "$L4T_TMP_DIR" ] ; then
    mkdir $L4T_TMP_DIR
fi

if [ ! -f "$L4T_TMP_DIR/$L4T_GCC_FILE" ] ; then
    echo "Downloading the L4T Toolchain ..."
    cd $L4T_TMP_DIR
    wget $L4T_GCC_URL/$L4T_GCC_FILE
fi

if [ ! -f "$L4T_TMP_DIR/$L4T_DRV_FILE" ] ; then
    echo "Downloading the Driver Package Sources ..."
    cd $L4T_TMP_DIR
    wget $L4T_DRV_URL/$L4T_DRV_FILE
fi

if [ ! -f "$L4T_TMP_DIR/$L4T_SRC_FILE" ] ; then
    echo "Downloading the Kernel Sources ..."
    cd $L4T_TMP_DIR
    wget $L4T_SRC_URL/$L4T_SRC_FILE
fi

if [ ! -f "$L4T_TMP_DIR/$L4T_RFS_FILE" ] ; then
    echo "Downloading the Sample Root Filesystem ..."
    cd $L4T_TMP_DIR
    wget $L4T_RFS_URL/$L4T_RFS_FILE
fi

# Extracting Toolchain
if [ ! -d "$L4T_GCC_DIR" ] ; then  
    mkdir $L4T_GCC_DIR
    echo "Extracting files ..."
    tar xf $L4T_TMP_DIR/$L4T_GCC_FILE -C $L4T_GCC_DIR
    echo "Extraction of $L4T_GCC_FILE completed"
fi

# Extracting Kernelsources
if [ ! -d "$L4T_BUILD_DIR/Linux_for_Tegra" ] ; then
    echo "Extracting files ..."
    tar xjvf $L4T_TMP_DIR/$L4T_DRV_FILE -C $L4T_BUILD_DIR
    tar xjvf $L4T_TMP_DIR/$L4T_RFS_FILE -C $L4T_BUILD_DIR/Linux_for_Tegra/rootfs
    tar xjvf $L4T_TMP_DIR/$L4T_SRC_FILE -C $L4T_BUILD_DIR
    cd $L4T_BUILD_DIR/Linux_for_Tegra/source/public
    tar xvf kernel_src.tbz2   
    echo "Extraction of $L4T_DRV_FILE and $L4T_SRC_FILE completed"
fi