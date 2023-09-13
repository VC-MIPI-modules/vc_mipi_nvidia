#!/bin/bash

download_dir=~/Downloads

user=vc
target=nvidia
target_shell="ssh ${user}@${target}"

device0=/dev/video0
device1=/dev/video1
device=${device1}

# IMX568
width=2496
height=2048
format="'Y10 '"
filename=test_${width}x${height}.raw

usage() {
	echo "Usage: $0 [options]"
	echo ""
	echo "Takes images or the media graph from the target"
	echo ""
	echo "Supported options:"
        echo "-h,                       Set image height"
        echo "    --help                Show this help text"
        echo "-i, --image               Takes an image from target"
        echo "-m, --media-graph         Takes a media graph from target"
        echo "-w,                       Set image width"
}

take_image() {
        echo "Takes image from target ..."

        ${target_shell} ./test/v4l2-test -d ${device} -f ${format}        
        ${target_shell} v4l2-ctl -d ${device} -c exposure=10000 -c gain=0 -c black_level=0
        ${target_shell} v4l2-ctl -d ${device} --set-fmt-video=width=${width},height=${height}
        ${target_shell} v4l2-ctl -d ${device} --stream-mmap --stream-skip=1 --stream-count=1 --stream-to=${filename}

        cd ${download_dir}
        scp ${user}@${target}:/home/${user}/${filename} .
        imagej -p 1 ${filename}

        # raw2rgbpnm -s ${width}x${height} -f ${format} test_${width}x${height}.raw test_${width}x${height}.pnm
        # eog test_${width}x${height}.pnm
}

take_media_graph() {
        echo "Takes media graph from target ..."

        ${target_shell} media-ctl --print-dot > ${download_dir}/media.dot
        cd ${download_dir}
        dot -Tsvg media.dot > media.svg
        eog media.svg
        rm media.dot media.svg
}

while [ $# != 0 ] ; do
	option="$1"
	shift

	case "${option}" in
        -d|--device)
                case "$1" in
                0) device=${device0} ;;
                1) device=${device1} ;;
                esac
                shift
                ;;
        -h)
                echo "set height $1" 
                height="$1"
                shift
                ;;
        --help)
		usage
		exit 0
		;;
	-i|--image)
		take_image
                exit 0
		;;
        -m|--media-graph)
		take_media_graph
                exit 0
		;;
        -w)
                width="$1"
                shift
                ;;
        esac
done

usage
exit 1