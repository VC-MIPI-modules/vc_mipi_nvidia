#!/bin/bash

usage() {
	echo "Usage: $0 [options]"
	echo ""
	echo "Setup configuration files"
	echo ""
	echo "Supported options:"
        echo ".b, --board               Preselect carrier board"
        echo "-d, --driver              Setup driver configuration"
        echo "-h, --help                Show this help text"
        echo "-p, --bsp                 Preselect board support package"
        echo "-s, --som                 Preselect system on module"
        echo "-t, --target              Setup target configuration"
}

configure() {
    . config/base.sh
}

print_title() {
        if [[ -z ${selected_som} || -z ${selected_board} || -z ${selected_bsp} ]]; then
                clear
                if [[ -n $1 && -n $2 ]]; then
                        echo "-- Step $1 of $2 ---------------------------------------------"
                else
                        echo "------------------------------------------------------------"
                fi
                echo ""
                echo "  Vision Components MIPI CSI-2 camera driver setup"
                echo ""
        fi
}

selection=
read_selection() {
        selection=0
        while [[ ${selection} -lt $1 || ${selection} -gt $2 ]]; do
                echo -n "  Option ($1-$2): "
                read selection
        done
}

choose_som() {
        if [[ -n ${selected_som} ]]; then
                selection=${selected_som}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your system on module"
                echo "    1: NVIDIA Jetson Nano (production) (https://developer.nvidia.com/embedded/jetson-nano)"
                echo "    2: NVIDIA Jetson Nano (devkit) (https://developer.nvidia.com/embedded/jetson-nano)"
                echo "    3: NVIDIA Jetson Xavier NX (https://developer.nvidia.com/embedded/jetson-xavier-nx)"
                read_selection 1 3
        fi
        case ${selection} in
        1) som=Nano ;;
        2) som=NanoDK ;;
        3) som=XavierNX ;;
        esac
}

choose_board_nano() {
        if [[ -n ${selected_board} ]]; then
                selection=${selected_board}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your carrier board"
                echo "    1: NVIDIA Jetson Nano Developer Kit B01 (https://developer.nvidia.com/embedded/jetson-nano-developer-kit)"
                echo "    2: Auvidea JNX30-LC-PD (https://auvidea.eu/product/70804)"
                read_selection 1 2
        fi
        case ${selection} in
        1) board=NV_DevKit_Nano_B01 ;;
        2) board=Auvidea_JNX30 ;;
        esac
}

choose_board_xavier_nx() {
        if [[ -n ${selected_board} ]]; then
                selection=${selected_board}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your carrier board"
                echo "    1: NVIDIA Jetson Xavier NX Developer Kit (https://developer.nvidia.com/embedded/jetson-xavier-nx-devkit)"
                read_selection 1 1
        fi
        case ${selection} in
        1) board=NV_DevKit_XavierNX ;;
        esac
}

choose_bsp() {
        if [[ -n ${selected_bsp} ]]; then
                selection=${selected_bsp}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your board support package"
                echo "    1: NVIDIA L4T 32.5.0 (https://developer.nvidia.com/embedded/linux-tegra-r325)"
                echo "    2: NVIDIA L4T 32.5.1 (https://developer.nvidia.com/embedded/linux-tegra-r3251)"
                echo "    3: NVIDIA L4T 32.5.2 (https://developer.nvidia.com/embedded/linux-tegra-r3251)"
                if [[ $1 == "4" ]]; then
                        echo "    4: NVIDIA L4T 32.6.1 (https://developer.nvidia.com/embedded/linux-tegra-r3261)"
                fi
                read_selection 1 $1
        fi
        case ${selection} in
        1) bsp=32.5.0 ;;
        2) bsp=32.5.1 ;;
        3) bsp=32.5.2 ;;
        4) bsp=32.6.1 ;;
        esac
}

check_configuration() {
        if [[ -z ${selected_som} || -z ${selected_board} || -z ${selected_bsp} ]]; then
                clear
                if [[ -n $1 && -n $2 ]]; then
                        echo "-- Step $1 of $2 ---------------------------------------------"
                else
                        echo "------------------------------------------------------------"
                fi
                echo ""
                echo "  Your hardware setup is:"
                echo ""
                echo "  System on module:      ${som}"
                echo "  Carrier board:         ${board}"
                echo "  Board support package: ${bsp}"
                echo ""
                echo "------------------------------------------------------------"
                echo "  press enter to continue or ctrl+c to abort"
                read
        fi
}

write_configuration() {
        mkdir -p $BUILD_DIR
        cd $BUILD_DIR
        echo "export VC_MIPI_SOM=${som}"     >  $CONFIGURATION_FILE
        echo "export VC_MIPI_BOARD=${board}" >> $CONFIGURATION_FILE
        echo "export VC_MIPI_BSP=${bsp}"     >> $CONFIGURATION_FILE
}

setup_driver() {
        print_title $1 $2
        choose_som
        case ${som} in
        Nano|NanoDK)
                choose_board_nano
                choose_bsp 4
                ;;
        XavierNX) 
                choose_board_xavier_nx
                choose_bsp 4
                ;;
        esac
        check_configuration $1 $2
        write_configuration
}

setup_target() {
        TARGET_USER=$USER
        TARGET_IP=
        if [[ -e $TARGET_FILE ]]; then
                . $TARGET_FILE
        fi
        echo "------------------------------------------------------------"
        echo ""
        echo "  Choose your target settings. "
        echo "  Do not enter anything if you want to accept the entry."
        echo ""
        echo -n "  USER ($TARGET_USER): "
        read user
        if [[ -n ${user} ]]; then
                export TARGET_USER=${user}
        fi
        echo -n "  IP ($TARGET_IP): "
        read ip
        if [[ -n ${ip} ]]; then
                export TARGET_IP=${ip}
        fi

        mkdir -p $BUILD_DIR
        cd $BUILD_DIR
        echo "export TARGET_USER=$TARGET_USER" >  $TARGET_FILE
        echo "export TARGET_IP=$TARGET_IP"     >> $TARGET_FILE
}

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
        -b|--board)
                selected_board=$1
                shift
                ;;
	-d|--driver)
                configure
		setup_driver $1 $2
		;;
	-h|--help)
		usage
		exit 0
		;;
        -p|--bsp)
                selected_bsp=$1
                shift
                ;;
        -s|--som)
                selected_som=$1
                shift
                ;;
	-t|--target)
                configure
		setup_target
		;;
	esac
done