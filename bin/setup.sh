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
        sudo apt install -y kmod
}

setup_toolchain() {
        echo "Setup tool chain ..."
        mkdir -p $BUILD_DIR

        if [[ ! -e $TOOLCHAIN_DIR ]]; then
                mkdir -p $TOOLCHAIN_DIR
        fi
        cd $TOOLCHAIN_DIR

        if [[ ! -e $GCC_DIR ]]; then
                wget $GCC_URL/$GCC_FILE
                tar xvf $GCC_FILE -C $TOOLCHAIN_DIR
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

        rm -Rf $KERNEL_SOURCE

        cd $DOWNLOAD_DIR
        tar xjvf $SRC_FILE -C $BSP_DIR

        cd $KERNEL_SOURCE

        L4T_extract_kernel_packages

        git init
        git config gc.auto 0
        git config --local user.name "$0"
        git config --local user.email "support@vision-components.com"

        L4T_add_kernel_to_repo

        git commit -m "Initial commit"

        for patch in "${PATCHES[@]}"; do
                echo "Applying patches from ${PATCH_DIR}/${patch}"
                for patchfile in $PATCH_DIR/${patch}/*.patch; do
                        git am -3 --whitespace=fix --ignore-whitespace < ${patchfile}
                done
        done

        git config gc.auto 1
}

repatch_kernel() {
        echo "Repatch kernel ..."

        cd $KERNEL_SOURCE

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

setup_nvidia_driver() {
        L4T_setup_nvidia_driver
}

setup_flash_prerequisites() {
        L4T_setup_flash_prerequisites
}

setup_som_carrier_specifics() {

        L4T_setup_eeprom_size

        L4T_setup_gpio_file

        L4T_setup_conf_file

        if [[ "36.4.0" == $VC_MIPI_BSP ]]
        then
                L4T_setup_dynamic_dtbo_file
        fi
}

setup_addon_file() {
        if [[ "32.7.5" == $VC_MIPI_BSP ]]
        then
                case $VC_MIPI_SOM in
                Nano|NanoSD|Nano2GB)
                        L4T_setup_addon_file
                        ;;
                *)
                        return 0
                        ;;
                esac
        fi
}

setup_nvidia_prereq_on_target() {
        case $VC_MIPI_BSP in
        36.2.0|36.4.0|36.4.3)
                target_dir=${ROOTFS_DIR}/home/${TARGET_USER}
                src_dir=${WORKING_DIR}/target

                cp ${src_dir}/setup_nvidia.sh ${target_dir}

                cp ${KERNEL_SOURCE}/gst-nvarguscamera_src.tbz2 ${target_dir}
                cp ${KERNEL_SOURCE}/gst-nvvidconv_src.tbz2 ${target_dir}
                ;;
        *)
                return 0
                ;;
        esac
}

setup_user_credentials() {
        echo ""
        echo "------------------------------------------------------------"
        echo "  Setup user credentials for target. "
        echo "------------------------------------------------------------"
        echo ""
        echo "  Please choose a user name for your target."
        echo "  If you leave it out, the default name '${VC_DEFAULT_USER}' will be used."
        echo -n "  USER (${VC_DEFAULT_USER}): "
        read user
        if [[ -n ${user} ]]; then
                TARGET_USER=${user}
        else
                TARGET_USER=${VC_DEFAULT_USER}
        fi

        echo ""
        echo "  Please choose a user password for your target."
        echo "  If you leave it out, the default password '${VC_DEFAULT_PW}' will be used."
        echo "  You can change it later on the target with the command 'passwd'."
        echo -n "  PW (${VC_DEFAULT_PW}): "
        read pw
        if [[ -n ${pw} ]]; then
                TARGET_PW=${pw}
        else
                TARGET_PW=${VC_DEFAULT_PW}
        fi

        echo ""
        echo "  The following user will be created:"
        echo "  user name: ${TARGET_USER}"
        echo "  user password: ${TARGET_PW}"

        mkdir -p $BUILD_DIR
        cd $BUILD_DIR
        echo "export TARGET_USER=$TARGET_USER" >  $TARGET_FILE
        echo "export TARGET_PW=$TARGET_PW"    >>  $TARGET_FILE
}

create_target_user() {
        echo "Create target user ..."
        cd $BSP_DIR/Linux_for_Tegra
        case $VC_MIPI_BSP in
        32.7.1|32.7.2|32.7.3|32.7.4|32.7.5|35.1.0|35.2.1|35.3.1|35.4.1|36.2.0|36.4.0|36.4.3)

                sudo ./tools/l4t_create_default_user.sh --username ${TARGET_USER} --password ${TARGET_PW} \
                        --hostname nvidia --autologin --accept-license
                ;;
        esac
}

setup_target_files() {
        echo "Setup target files ..."

        TARGET_DIR=$BSP_DIR/Linux_for_Tegra/rootfs/home/$TARGET_USER/test
        sudo mkdir -p $TARGET_DIR

        sudo cp $WORKING_DIR/target/* $TARGET_DIR
        sudo chmod +x $TARGET_DIR/*.sh

        rfs=$BSP_DIR/Linux_for_Tegra/rootfs
        sudo chown -v -R $(cat "${rfs}"/etc/passwd | grep ${TARGET_USER} | cut -d : -f 3-4) \
                "${rfs}/home/${TARGET_USER}" > /dev/null
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
        L4T_setup_flash_prerequisites

        sudo ./apply_binaries.sh

        create_target_user
}

setup_camera() {
        nano -l +23 $DT_CAM_FILE
}

setup_target() {
        echo $1 $2 $3 $4
        if [[ -z $1 ]]; then
                . $BIN_DIR/config/setup.sh --target
        else
                TARGET_USER=$1
                TARGET_PW=$2
                TARGET_IP=$3
                TARGET_RSA=$4
        fi

        known_hosts_filepath=~/.ssh/known_hosts
        date_time=$(date '+%Y%m%d_%H%M%S')
        known_hosts_filepath_backup=${known_hosts_filepath}_backup_by_vc_setup_${date_time}
        if [[ -e ${known_hosts_filepath} ]]
        then 
                # Backup User's known_hosts file => known_host__backup_by_vc_setup_YYYYMMDD_HHMMSS.
                echo "Backup known_hosts file..."
                cp -v ${known_hosts_filepath} ${known_hosts_filepath_backup}
                
                # Remove already existing connection credential with this given IP.
                ssh-keygen -f ${known_hosts_filepath} -R ${TARGET_IP}
        fi

        # Copy User's public rsa file into the target authorized_keys file.
        echo "Copy public rsa key..."
        ssh-copy-id -i ~/.ssh/${TARGET_RSA} $TARGET_USER@$TARGET_IP

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
                setup_addon_file
                exit 0
                ;;
        -p|--repatch)
                configure
                repatch_kernel
                setup_addon_file
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
        -n)
                configure
                setup_nvidia_driver
                exit 0
                ;;
        -b)
                configure
                setup_som_carrier_specifics
                exit 0
                ;;
        -o|--host)
                configure
                install_system_tools
                setup_toolchain
                setup_user_credentials
                setup_bsp
# If the user likes to pre-install the test directory with test scripts 
# in the configured user-home directory of the target system,
# the following function can be activated.
#                setup_target_files
                setup_kernel
                setup_addon_file

                setup_nvidia_driver
                setup_som_carrier_specifics
                setup_nvidia_prereq_on_target
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