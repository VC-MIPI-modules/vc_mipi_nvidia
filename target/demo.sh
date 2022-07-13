#!/usr/bin/env bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Script to test and demonstrate camera features."
        echo ""
        echo "Supported options:"
        echo "    --argus               Uses nvarguscamerasrc gstreamer pipeline"
        echo "-d, --device              Select the device (default: /dev/video0)"
        echo "-f, --format              Pixelformat (Options: GREY, 'Y10 ', 'Y12 ', RGGB, RG10, RG12)"
        echo "-g, --gain                Set the gain"
        echo "-h, --help                Show this help text"
        echo "-i, --info                Get system information"
        echo "-o, --option              Set the options"
        echo "-r, --framerate           Set the frame rate in Hz"
        echo "-s, --shutter             Set the shutter time in µs"
        echo "-t, --trigger             Set the trigger mode (Options: 0-7)"
        echo "    --io                  Set the io mode (Options: 0-5)"
        echo "-w, --whitebalance        Activate white balance"
}

install_dependencies() {
        if [[ -z $(which v4l2-ctl) ]]; then
                sudo apt update
                sudo apt install -y v4l-utils
        fi
        if [[ -e vcmipidemo ]]; then
                chmod +x vcmipidemo
        fi
}

get_system_info() 
{
        if [[ -z $(which git) ]]; then
                sudo apt update
                sudo apt install -y git
        fi
        cd ${script_dir}
        if [[ ! -d jetsonUtilities ]]; then
                git clone https://github.com/jetsonhacks/jetsonUtilities
        fi
        cd jetsonUtilities
        ./jetsonInfo.py
}

get_image_size() {
        values=()
        for value in $(v4l2-ctl -d /dev/video${device} --get-fmt-video | grep -i 'Width/Height' | grep -oe '\([0-9.]*\)'); do
                values+=(${value})
        done
        width=${values[0]}
        height=${values[1]}
}

adjust_pixel_format() {
        pixelformat=$(v4l2-ctl -d /dev/video${device} --get-fmt-video | grep -i 'Pixel Format' | grep -oe "'.*'")
        pixelformat=${pixelformat:1:-1}

        newformat=
        case ${pixelformat} in
        'GREY') newformat='RGGB' ;;
        'Y10 ') newformat='RG10' ;;
        'Y12 ') newformat='RG12' ;;
        esac

        if [[ -n ${newformat} ]]; then
                echo "Adjust pixeformat to ${newformat}, because nvarguscamerasrc needs a color format."
                v4l2-ctl -d /dev/video${device} --set-fmt-video=pixelformat=${newformat}
        fi
}

script_dir=$(dirname $0)
argus=
format=
trigger=
io=
value=
whitebalance=
device=0
option2=x
shutter=10000
gain=0
exposure=100000000
framerate=
optionY=
width=
height=

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        --argus)
                argus=1
                shift
                ;;
        --io)
                io="$1"
                shift
                ;;
        -d|--device)
                device="$1"
                shift
                ;;
        -e|--exposure)
                exposure="$1"
                shift
                ;;
        -f|--format)
                format="$1"
                shift
                ;;
        -g|--gain)
                gain="$1"
                shift
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -i|--info)
                get_system_info
                exit 0
                ;;
        -o|--option)
                option2="$1"
                shift
                ;;
        -r|--framerate)
                framerate="$1"
                shift
                ;;
        -s|--shutter)
                shutter="$1"
                shift
                ;;
        -t|--trigger)
                trigger="$1"
                shift
                ;;
        -w|--whitebalance)
                whitebalance="-w '128 180 128'"
                shift
                ;;
        -y)
                optionY="-y$1"
                shift
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

install_dependencies

if [[ -n ${format} ]]; then
        echo "Set format: ${format}"
        v4l2-ctl -d /dev/video${device} --set-fmt-video=pixelformat="${format}"
fi
if [[ -n ${framerate} ]]; then
        echo "Set frame rate: ${framerate}"
        v4l2-ctl -d /dev/video${device} -c frame_rate=${framerate}
fi
if [[ -n ${trigger} ]]; then
        echo "Set trigger mode: ${trigger}"
        v4l2-ctl -d /dev/video${device} -c trigger_mode=${trigger}
fi
if [[ -n ${io} ]]; then
        echo "Set io mode: ${io}"
        v4l2-ctl -d /dev/video${device} -c io_mode=${io}
fi
if [[ -n ${value} ]]; then
        echo "Set Value: ${value}"
        v4l2-ctl -d /dev/video${device} -c value=${value}
fi

if [[ -n ${argus} ]]; then
        get_image_size
        adjust_pixel_format

        gst-launch-1.0 nvarguscamerasrc sensor-id=${device} ! 'video/x-raw(memory:NVMM),framerate=20/1' ! autovideosink
else 
        cd ${script_dir}
        v4l2-ctl -c bypass_mode=0
        ./vcmipidemo -d${device} -an${option2} ${optionY} -s${shutter} -g${gain} -w '128 180 128'
fi