#!/bin/bash

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
        echo "-s, --shutter             Set the shutter time in µs"
        echo "-t, --trigger             Set the trigger mode (Options: 0-7)"
	echo "-a, --flash               Set the flash mode (Options: 0-1)"
	echo "-v, --value               Set value fot testing"
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
flash=
value=
whitebalance=
device=0
option2=x
shutter=10000
gain=10
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
        -a|--flash)
		flash="$1"
		shift
		;;
        -d|--device)
		device="$1"
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
	-s|--shutter)
		shutter="$1"
		shift
		;;
        -t|--trigger)
		trigger="$1"
		shift
		;;
	-v|--value)
		value="$1"
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
        v4l2-ctl -d /dev/video${device} --set-fmt-video=pixelformat=${format}
fi
if [[ -n ${trigger} ]]; then
        echo "Set trigger mode: ${trigger}"
        v4l2-ctl -d /dev/video${device} -c trigger_mode=${trigger}
fi
if [[ -n ${flash} ]]; then
        echo "Set flash mode: ${flash}"
        v4l2-ctl -d /dev/video${device} -c flash_mode=${flash}
fi
if [[ -n ${value} ]]; then
        echo "Set Value: ${value}"
        v4l2-ctl -d /dev/video${device} -c value=${value}
fi

if [[ -n ${argus} ]]; then
	get_image_size
	adjust_pixel_format

	gst-launch-1.0 \
        	nvarguscamerasrc sensor-id=${device} aelock=true awblock=true tnr-mode=0 ! \
        	"video/x-raw(memory:NVMM),width=${width}, height=${height}, framerate=20/1, format=NV12" ! \
        	nvegltransform ! nveglglessink -e
else 
	cd ${script_dir}
	./vcmipidemo -d${device} -an${option2} ${optionY} -s${shutter} -g${gain} -w '128 180 128'
fi