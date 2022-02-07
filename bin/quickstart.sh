#!/bin/bash

set -e

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Combines all necessary steps to install the Vision Components"
        echo "MIPI CSI-2 camera driver."
        echo ""
        echo "Supported options:"
        echo "-a, --automatic           Surpress explanation texts"
        echo "-h, --help                Show this help text"
}

print_long_time_note() {
        echo "  NOTE: This process can take a long time (> 30 minutes)"
        echo "        Make sure that your target is in recovery mode."
        echo "        Don't shut down or plug off your target until this"
        echo "        process is ready."
}

print_intro() {
        if [[ ${verbose} == 'yes' ]]; then
                clear
                echo "------------------------------------------------------------"
                echo ""
                echo "  This quickstart script combines all necessary steps to"
                echo "  install the Vision Components MIPI CSI-2 camera driver"
                echo "  on your target."
                echo ""
                print_long_time_note
                echo ""
                echo "  Following steps are processed:"
                echo "    1. Check if your system is in recovery mode."
                echo "    2. Configure the driver to your hardware setup."
                echo "    3. Configure camera-specific settings in the device"
                echo "       tree file."
                echo "    4. Download and install the toolchain, board support"
                echo "       package and kernel source code. Patching the kernel"
                echo "       source code with camera driver sources. Build kernel,"
                echo "       modules and device tree. Flash everything to your"
                echo "       target."
                echo ""
                echo "------------------------------------------------------------"
                echo "  press enter to continue or ctrl+c to abort"
                read
        fi
}

check_recovery_mode() {
        if [[ -z $(lsusb | grep -i "NVIDIA Corp.") || -n $(lsusb | grep -i "NVIDIA Corp. L4T") ]]; then
                clear
                echo "-- Step 1 of 4 ---------------------------------------------"
                echo ""
                echo "  Recovery Mode not started!"
                echo ""
                echo "------------------------------------------------------------"
                exit 1
        else
                if [[ ${verbose} == 'yes' ]]; then
                        clear
                        echo "-- Step 1 of 4 ---------------------------------------------"
                        echo ""
                        echo "  Great, your target is in recovery mode! You can go on."
                        echo ""
                        echo "------------------------------------------------------------"
                        echo "  press enter to continue or ctrl+c to abort"
                        read
                fi
        fi
}

configure() {
        . config/configure.sh driver 2 4
}

print_setup_nano_note() {
        if [[ ${verbose} == 'yes' ]]; then
                clear
                echo "-- Step 3 of 4 ---------------------------------------------"
                echo ""
                echo "  If you proceed the nano editor will be installed"
                echo "  to configure the camera device tree file:"
                echo ""
                echo "    $ sudo apt update"
                echo "    $ sudo apt install -y nano"
                echo ""
                echo "  You have to enter your password."
                echo ""
                echo "------------------------------------------------------------"
                echo "  press enter to continue or ctrl+c to abort"
                read
        fi
}

setup_nano() {
        if [[ -z $(which nano) ]]; then
                print_setup_nano_note
                sudo apt update
                sudo apt install -y nano
        fi
}

print_configure_camera_note() {
        if [[ ${verbose} == 'yes' ]]; then
                clear
                echo "-- Step 3 of 4 ---------------------------------------------"
                echo ""
                echo "  Please adapt the camera settings in the device tree file"
                echo "  to your specific camera setup."
                echo ""
                echo "    1. Choose which MIPI CSI-2 port to enable"
                echo "    2. Choose the number of lanes your camera supports"
                echo "    3. Choose correct embedded_metadata_height for your"
                echo "       camera module (Xavier NX only)"
                echo "    4. Choose VC_MIPI_OMNI_VISION 1 if your camera is an"
                echo "       Omni Vision camera."
                echo ""
                echo "  NOTE: The device tree file will be opened in nano now."
                echo "        After editing close nano with ctrl+x."
                echo ""
                echo "------------------------------------------------------------"
                echo "  press enter to continue or ctrl+c to abort"
                read
        fi
}

configure_camera() {
        setup_nano
        print_configure_camera_note
        nano -l +23 $DT_CAM_FILE
}

print_note_before_start() {
        if [[ ${verbose} == 'yes' ]]; then
                clear
                echo "-- Step 4 of 4 ---------------------------------------------"
                echo ""
                echo "  Congratulations! Everything is prepared to setup, build"
                echo "  and flash now."
                echo ""
                print_long_time_note
                echo ""
                echo "------------------------------------------------------------"
                echo "  press enter to continue or ctrl+c to abort"
                read
        fi
}

verbose='yes'

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -a|--automatic)
                verbose='no'
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

DIR=$PWD

print_intro
check_recovery_mode
configure
configure_camera
print_note_before_start

start_time=$(date +%s)

cd $DIR
(. setup.sh --host)
(. build.sh --all)
(. flash.sh --all)

end_time=$(date +%s)
elapsed_time=$((${end_time} - ${start_time}))
echo "------------------------------------------------------------"
echo -n "  "
eval "echo $(date -ud "@${elapsed_time}" +'The process took %H hours %M minutes %S seconds')"
echo "------------------------------------------------------------"