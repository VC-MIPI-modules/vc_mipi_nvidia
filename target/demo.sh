
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
	echo "-i, --image               Set the image height"
	echo "-l, --lanes               Set the number of lanes."
        echo "-o, --option              Set the options"
        echo "-s, --shutter             Set the shutter time in µs"
        echo "-t, --trigger             Set the trigger mode (Options: 1-6)"
}

device=0
lanes=
format=
trigger=
height=
option2=x
shutter=10000
gain=10

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
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
	-i|--image)
		height="$1"
		shift
		;;
	-h|--help)
		usage
		exit 0
		;;
	-l|--lanes)
		lanes="$1"
		shift
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
	*)
		echo "Unknown option ${option}"
		exit 1
		;;
	esac
done

if [[ -n ${lanes} ]]; then
        echo "Set number of lanes: ${lanes}"
        v4l2-ctl -d /dev/video${device} -c num_lanes=${lanes}
fi
if [[ -n ${format} ]]; then
        echo "Set format: ${format}"
        v4l2-ctl -d /dev/video${device} --set-fmt-video=pixelformat=${format}
fi
if [[ -z ${trigger} || ${trigger} == "0" ]]; then
        echo "Set trigger mode: DISABLED"
        v4l2-ctl -d /dev/video${device} -c trigger_mode=0
        v4l2-ctl -d /dev/video${device} -c flash_mode=0
else
        echo "Set trigger mode: ${trigger}"
        v4l2-ctl -d /dev/video${device} -c trigger_mode=${trigger}
        v4l2-ctl -d /dev/video${device} -c flash_mode=1
fi
if [[ -n ${height} ]]; then
        echo "Set image height: ${height}"
        v4l2-ctl -d /dev/video${device} -c image_height=${height}
else 
	v4l2-ctl -d /dev/video${device} -c image_height=0
fi

./test/vcmipidemo -d${device} -a${option2} -s${shutter} -g${gain} -w '128 180 128'