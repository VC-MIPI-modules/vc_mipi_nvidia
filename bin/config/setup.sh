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

som=
soms=(
"NVIDIA Jetson Nano (production) (https://developer.nvidia.com/embedded/jetson-nano)"
"NVIDIA Jetson Nano (devkit) (https://developer.nvidia.com/embedded/jetson-nano)"
"NVIDIA Jetson Nano 2GB (devkit) (https://developer.nvidia.com/embedded/jetson-nano-2gb-developer-kit)"
"NVIDIA Jetson Xavier NX (production) (https://developer.nvidia.com/embedded/jetson-xavier-nx)"
"NVIDIA Jetson Xavier NX (devkit) (https://developer.nvidia.com/embedded/jetson-xavier-nx)"
"NVIDIA Jetson AGX Xavier (devkit) (https://developer.nvidia.com/embedded/jetson-agx-xavier-developer-kit)"
"NVIDIA Jetson TX2/TX2i (devkit) (https://developer.nvidia.com/embedded/jetson-tx2-developer-kit)"
"NVIDIA Jetson Orin NX (devkit) (https://developer.nvidia.com/embedded/jetson-tx2-developer-kit)"
"NVIDIA Jetson Orin Nano (devkit) (https://developer.nvidia.com/embedded/jetson-tx2-developer-kit)"        
)

som_keys=(
"Nano"
"NanoSD"
"Nano2GB"
"XavierNX"
"XavierNXSD"
"AGXXavier"
"TX2"
"OrinNX"
"OrinNano"
)

choose_som() {
        indices=("$@")
        if [[ -n ${selected_som} ]]; then
                selection=${selected_som}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your system on module"
                index=1
                for i in "${indices[@]}"; do
                        echo "    $index: ${soms[$i]}"
                        ((index=index+1))
                done
                read_selection 1 ${#indices[@]}
        fi
        som=${som_keys[indices[selection-1]]}
}

board=
boards=(
"NVIDIA Jetson Nano 2GB Developer Kit (https://developer.nvidia.com/embedded/jetson-nano-2gb-developer-kit)"
"NVIDIA Jetson Nano Developer Kit B01 (https://developer.nvidia.com/embedded/jetson-nano-developer-kit)"
"NVIDIA Jetson Xavier NX Developer Kit (https://developer.nvidia.com/embedded/jetson-xavier-nx-devkit)"
"NVIDIA Jetson Orin Nano Developer Kit (...)"
"Auvidea JNX30/JNX30D (https://auvidea.eu/product/70879)"
"Auvidea J20 on Devkit Jetson AGX Xavier or TX2 (https://auvidea.eu/j20)"
)

board_keys=(
"NV_DevKit_Nano"
"NV_DevKit_Nano"
"NV_DevKit_XavierNX"
"NV_DevKit_OrinNano"
"Auvidea_JNX30"
"Auvidea_J20"
)

choose_board() {
        indices=("$@")
        if [[ -n ${selected_board} ]]; then
                selection=${selected_board}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your carrier board"
                index=1
                for i in "${indices[@]}"; do
                        echo "    $index: ${boards[$i]}"
                        ((index=index+1))
                done
                read_selection 1 ${#indices[@]}
        fi
        board=${board_keys[indices[selection-1]]}
}

bsp=
bsps=(
"NVIDIA L4T 32.3.1 (https://developer.nvidia.com/l4t-3231-archive)"
"NVIDIA L4T 32.5.0 (https://developer.nvidia.com/embedded/linux-tegra-r325)"
"NVIDIA L4T 32.5.1 (https://developer.nvidia.com/embedded/linux-tegra-r3251)"
"NVIDIA L4T 32.5.2 (https://developer.nvidia.com/embedded/linux-tegra-r3251)"
"NVIDIA L4T 32.6.1 (https://developer.nvidia.com/embedded/linux-tegra-r3261)"
"NVIDIA L4T 32.7.1 (https://developer.nvidia.com/embedded/linux-tegra-r3271)"
"NVIDIA L4T 32.7.2 (https://developer.nvidia.com/embedded/linux-tegra-r3272)"
"NVIDIA L4T 32.7.3 (https://developer.nvidia.com/embedded/linux-tegra-r3273)"
"NVIDIA L4T 35.1.0 (https://developer.nvidia.com/embedded/jetson-linux-r351)"
"NVIDIA L4T 35.2.1 (https://developer.nvidia.com/embedded/jetson-linux-r3521)"
"NVIDIA L4T 35.3.1 (https://developer.nvidia.com/embedded/jetson-linux-r3531)"
)

bsps_keys=(
"32.3.1"
"32.5.0"
"32.5.1"
"32.5.2"
"32.6.1"
"32.7.1"
"32.7.2"
"32.7.3"
"35.1.0"
"35.2.1"
"35.3.1"
)

choose_bsp() {
        indices=("$@")
        if [[ -n ${selected_bsp} ]]; then
                selection=${selected_bsp}
        else
                echo "------------------------------------------------------------"
                echo "  Choose your board support package"
                index=1
                for i in "${indices[@]}"; do
                        echo "    $index: ${bsps[$i]}"
                        ((index=index+1))
                done
                read_selection 1 ${#indices[@]}
        fi
        bsp=${bsps_keys[indices[selection-1]]}
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
        choose_som 0 1 2 3 4 5 6
        case ${som} in
        Nano2GB)
                choose_board 0
                choose_bsp 1 2 3 4 5 6 7
                ;;
        Nano|NanoSD)
                choose_board 1 4
                choose_bsp 1 2 3 4 5 6 7
                ;;
        XavierNX|XavierNXSD) 
                choose_board 2 4
                choose_bsp 1 2 3 4 5 6 7 8 9 10
                ;;
        AGXXavier)
                choose_board 5
                choose_bsp 0 1 2 3 4 5 6 7 8 9 10
                ;;
        TX2)
                choose_board 5
                choose_bsp 1 2 3 4 5 6 7
                ;;
        OrinNano)
                choose_board 3
                choose_bsp 10
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
                TARGET_USER=${user}
        fi
        echo -n "  IP ($TARGET_IP): "
        read ip
        if [[ -n ${ip} ]]; then
                TARGET_IP=${ip}
        fi

        mkdir -p $BUILD_DIR
        cd $BUILD_DIR
        echo "export TARGET_USER=$TARGET_USER" >  $TARGET_FILE
        echo "export TARGET_IP=$TARGET_IP"     >> $TARGET_FILE
        TARGET_SHELL="ssh $TARGET_USER@$TARGET_IP"
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