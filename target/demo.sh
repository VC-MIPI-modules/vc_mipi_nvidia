#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Build kernel image, modules and device tree."
        echo ""
        echo "Supported options:"
        echo "-d, --device              Select the device (default: /dev/video0)"
        echo "-f, --format              Pixelformat (Options: GREY, 'Y10 ', 'Y12 ', RGGB, RG10, RG12)"
        echo "-g, --gain                Set the gain"
        echo "-h, --help                Show this help text"
        echo "-o, --option              Set the options"
        echo "-s, --shutter             Set the shutter time in µs"
        echo "-t, --trigger             Set the trigger mode (Options: 0-7)"
	echo "-a, --flash               Set the flash mode (Options: 0-1)"
	echo "-v, --value               Set value fot testing"
	echo "-w, --whitebalance        Activate white balance"
}

format=
trigger=
flash=
value=
whitebalance=
device=0
option2=x
shutter=10000
gain=10

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
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
	*)
		echo "Unknown option ${option}"
		exit 1
		;;
	esac
done

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

./test/vcmipidemo -d${device} -an${option2} -s${shutter} -g${gain} -w '128 180 128'