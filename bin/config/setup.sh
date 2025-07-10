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
"NVIDIA Jetson TX2 NX (production) (https://developer.nvidia.com/embedded/jetson-tx2-nx)"
"NVIDIA Jetson Orin NX 8GB (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nx)"
"NVIDIA Jetson Orin NX 16GB (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nx)"
"NVIDIA Jetson Orin Nano 4GB SD (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nano)"        
"NVIDIA Jetson Orin Nano 8GB SD (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nano)"        
"NVIDIA Jetson Orin Nano 4GB NVME (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nano)"        
"NVIDIA Jetson Orin Nano 8GB NVME (devkit) (https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nano)"        
)

som_keys=(
"Nano"
"NanoSD"
"Nano2GB"
"XavierNX"
"XavierNXSD"
"AGXXavier"
"TX2"
"TX2NX"
"OrinNX8GB"
"OrinNX16GB"
"OrinNano4GB_SD"
"OrinNano8GB_SD"
"OrinNano4GB_NVME"
"OrinNano8GB_NVME"
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
                        printf "    %2d: %s \n" $index "${soms[$i]}"
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
"NVIDIA Jetson Orin Nano Developer Kit (https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit)"
"Auvidea JNX30/JNX30D (https://auvidea.eu/product/70879)"
"Auvidea JNX42 LM (https://auvidea.eu/product/70784)"
"Auvidea J20 on Devkit Jetson AGX Xavier or TX2 (https://auvidea.eu/j20)"
)

board_keys=(
"NV_DevKit_Nano"
"NV_DevKit_Nano"
"NV_DevKit_XavierNX"
"NV_DevKit_OrinNano"
"Auvidea_JNX30"
"Auvidea_JNX42"
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
"NVIDIA L4T 32.7.1 (https://developer.nvidia.com/embedded/linux-tegra-r3271)"
"NVIDIA L4T 32.7.2 (https://developer.nvidia.com/embedded/linux-tegra-r3272)"
"NVIDIA L4T 32.7.3 (https://developer.nvidia.com/embedded/linux-tegra-r3273)"
"NVIDIA L4T 32.7.4 (https://developer.nvidia.com/embedded/linux-tegra-r3274)"
"NVIDIA L4T 32.7.5 (https://developer.nvidia.com/embedded/linux-tegra-r3275)"
"NVIDIA L4T 35.1.0 (https://developer.nvidia.com/embedded/jetson-linux-r351)"
"NVIDIA L4T 35.2.1 (https://developer.nvidia.com/embedded/jetson-linux-r3521)"
"NVIDIA L4T 35.3.1 (https://developer.nvidia.com/embedded/jetson-linux-r3531)"
"NVIDIA L4T 35.4.1 (https://developer.nvidia.com/embedded/jetson-linux-r3541)"
"NVIDIA L4T 36.2.0 (https://developer.nvidia.com/embedded/jetson-linux-r362)"
"NVIDIA L4T 36.4.0 (https://developer.nvidia.com/embedded/jetson-linux-r364)"
)

bsps_keys=(
"32.7.1"
"32.7.2"
"32.7.3"
"32.7.4"
"32.7.5"
"35.1.0"
"35.2.1"
"35.3.1"
"35.4.1"
"36.2.0"
"36.4.0"
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
                        printf "    %2d: %s \n" $index "${bsps[$i]}"
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
        choose_som 0 1 2 3 4 5 6 7 8 9 10 11 12 13
        case ${som} in
        Nano2GB)
                choose_board 0
                choose_bsp 0 1 2 3 4
                ;;
        Nano|NanoSD)
                choose_board 1 4 5
                choose_bsp 0 1 2 3 4
                ;;
        XavierNX|XavierNXSD) 
                choose_board 2 4 5
                choose_bsp 0 1 2 5 6 7 8
                ;;
        AGXXavier)
                choose_board 6
                choose_bsp 0 1 2 5 6 7 8
                ;;
        TX2|TX2i)
                choose_board 6
                choose_bsp 0 1 2
                ;;
        TX2NX)
                choose_board 4
                choose_bsp 0 1 2
                ;;
        OrinNano4GB_SD|OrinNano8GB_SD|OrinNano4GB_NVME|OrinNano8GB_NVME)
                choose_board 3 5
                choose_bsp 7 8 9 10
                ;;
        OrinNX8GB|OrinNX16GB)
                choose_board 3 5

                case ${board} in
                NV_DevKit_OrinNano)
                        choose_bsp 9 10
                        ;;
                *)
                        choose_bsp 6 7 8 9 10
                        ;;
                esac

                ;;
        esac
        check_configuration $1 $2
        write_configuration
}

setup_target() {
        TARGET_USER=$USER
        TARGET_PW=
        TARGET_IP="nvidia"
        TARGET_RSA="id_rsa.pub"
        if [[ -e $TARGET_FILE ]]; then
                . $TARGET_FILE
        fi
        echo "------------------------------------------------------------"
        echo ""
        echo "  Choose your target settings. "
        echo "  Do not enter anything if you want to accept the entry surrounded by the braces."
        echo ""
        echo -n "  USER ($TARGET_USER): "
        read user
        if [[ -n ${user} ]]; then
                TARGET_USER=${user}
        fi
        echo -n "  PW ($TARGET_PW): "
        read pw
        if [[ -n ${pw} ]]; then
                TARGET_PW=${pw}
        fi
        echo -n "  IP ($TARGET_IP): "
        read ip
        if [[ -n ${ip} ]]; then
                TARGET_IP=${ip}
        fi
        echo ""
        echo "------------------------------------------------------------"
        echo "  Your available .pub files:"
        echo "------------------------------------------------------------"
        KEYFILES="$(find ~/.ssh -name *.pub)"
        for keyfile in ${KEYFILES[@]}; do
                echo "  $keyfile"
        done

        echo "------------------------------------------------------------"

        echo -n "  RSA ($TARGET_RSA):"
        read rsa
        if [[ -n ${rsa} ]]; then
                TARGET_RSA=${rsa}
        fi

        mkdir -p $BUILD_DIR
        cd $BUILD_DIR
        echo "export TARGET_USER=$TARGET_USER" >  $TARGET_FILE
        echo "export TARGET_PW=$TARGET_PW"     >> $TARGET_FILE
        echo "export TARGET_IP=$TARGET_IP"     >> $TARGET_FILE
        echo "export TARGET_RSA=$TARGET_RSA"   >> $TARGET_FILE
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