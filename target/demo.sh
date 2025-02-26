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
        echo "-r, --framerate           Set the frame rate in Hz"
        echo "-s, --shutter             Set the shutter time in µs"
        echo "-t, --trigger             Set the trigger mode (Options: 0-7)"
        echo "    --io                  Set the io mode (Options: 0-5)"
}

install_dependencies() {
        if [[ -z $(which v4l2-ctl) ]]; then
                sudo apt update
                sudo apt install -y v4l-utils git build-essential cmake python3-pip nvidia-l4t-gstreamer
                sudo pip3 install -U jetson-stats
        fi
        (
                cd ${script_dir}
                if [[ ! -e v4l2-test ]]; then
                        if [[ ! -e v4l2-test.git ]]; then
                                git clone https://github.com/pmliquify/v4l2-test.git v4l2-test.git
                        fi
                        ./v4l2-test.git/make.sh
                        cp v4l2-test.git/build_generic/v4l2-test .
                fi
        )
}

install_avtviewer() {
        cd ${script_dir}
        if [[ ! -e viewer ]]; then
                sudo apt install -y qt5-default cmake
                git clone https://github.com/alliedvision/V4L2Viewer.git
                cd V4L2Viewer
                mkdir build
                cd build
                cmake ..
                make
                cp V4L2Viewer ../../viewer
        fi
}

setup_iq_tuning()
{
        sudo cp camera_overrides.isp /var/nvidia/nvcam/settings/
        sudo chmod 664 /var/nvidia/nvcam/settings/camera_overrides.isp
        sudo chown root:root /var/nvidia/nvcam/settings/camera_overrides.isp
        ls -l /var/nvidia/nvcam/settings/camera_overrides.isp
}

setup_gst_perf_plugin()
{
        sudo apt update
        sudo apt install libgstreamer1.0-dev
        git clone https://github.com/RidgeRun/gst-perf.git
        cd gst-perf
        ./autogen.sh
        ./configure --prefix /usr/ --libdir /usr/lib/$(uname -m)-linux-gnu/
        make
        sudo make install
}

value() {
        echo $(tr -d '\0' < $1)
}

check_dt_settings()
{
        # cam_path=/sys/firmware/devicetree/base/cam_i2cmux/i2c@0
        # echo "--- CAM1 ------------------------------------------------------"
        # echo "${cam_path}"
        # echo "physical_w:               $(value ${cam_path}/vc_mipi@1a/physical_w)"
        # echo "physical_h:               $(value ${cam_path}/vc_mipi@1a/physical_h)"
        # echo
        # echo "num_lanes:                $(value ${cam_path}/vc_mipi@1a/num_lanes)"
        # echo "embedded_metadata_height: $(value ${cam_path}/vc_mipi@1a/mode0/embedded_metadata_height)"
        # echo "lane_polarity:            $(value ${cam_path}/vc_mipi@1a/mode0/lane_polarity)"
        # echo "active_l:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_l)"
        # echo "active_t:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_t)"
        # echo "active_w:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_w)"
        # echo "active_h:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_h)"
        # echo
        # echo "line_length:              $(value ${cam_path}/vc_mipi@1a/mode0/line_length)"
        # echo "discontinuous_clk:        $(value ${cam_path}/vc_mipi@1a/mode0/discontinuous_clk)"
        # echo "mclk_khz:                 $(value ${cam_path}/vc_mipi@1a/mode0/mclk_khz)"
        # echo "pix_clk_hz:               $(value ${cam_path}/vc_mipi@1a/mode0/pix_clk_hz)"
        # echo "mclk_multiplier:          $(value ${cam_path}/vc_mipi@1a/mode0/mclk_multiplier)"
        # echo "cil_settletime:           $(value ${cam_path}/vc_mipi@1a/mode0/cil_settletime)"
        # echo
        cam_path=/sys/firmware/devicetree/base/cam_i2cmux/i2c@1
        echo "--- CAM0 ------------------------------------------------------"
        echo "${cam_path}"
        echo "physical_w:               $(value ${cam_path}/vc_mipi@1a/physical_w)"
        echo "physical_h:               $(value ${cam_path}/vc_mipi@1a/physical_h)"
        echo
        echo "num_lanes:                $(value ${cam_path}/vc_mipi@1a/num_lanes)"
        echo "embedded_metadata_height: $(value ${cam_path}/vc_mipi@1a/mode0/embedded_metadata_height)"
        echo "lane_polarity:            $(value ${cam_path}/vc_mipi@1a/mode0/lane_polarity)"
        echo "active_l:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_l)"
        echo "active_t:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_t)"
        echo "active_w:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_w)"
        echo "active_h:                 $(value ${cam_path}/vc_mipi@1a/mode0/active_h)"
        echo
        echo "line_length:              $(value ${cam_path}/vc_mipi@1a/mode0/line_length)"
        echo "discontinuous_clk:        $(value ${cam_path}/vc_mipi@1a/mode0/discontinuous_clk)"
        echo "mclk_khz:                 $(value ${cam_path}/vc_mipi@1a/mode0/mclk_khz)"
        echo "pix_clk_hz:               $(value ${cam_path}/vc_mipi@1a/mode0/pix_clk_hz)"
        echo "mclk_multiplier:          $(value ${cam_path}/vc_mipi@1a/mode0/mclk_multiplier)"
        echo "cil_settletime:           $(value ${cam_path}/vc_mipi@1a/mode0/cil_settletime)"
        # echo
        # tegra_path=/sys/firmware/devicetree/base/tegra-camera-platform
        # echo "---------------------------------------------------------------"
        # echo "num_csi_lanes:            $(value ${tegra_path}/num_csi_lanes)"/
        # echo "max_lane_speed:           $(value ${tegra_path}/max_lane_speed)"
        # echo "min_bits_per_pixel:       $(value ${tegra_path}/min_bits_per_pixel)"
        # echo "max_pixel_rate:           $(value ${tegra_path}/max_pixel_rate)"
        echo "---------------------------------------------------------------"
}

enable_debugging()
{
        echo 'file channel.c +p' > /sys/kernel/debug/dynamic_debug/control
        echo 'file vi4_fops.c +p' > /sys/kernel/debug/dynamic_debug/control
        echo 'file csi4_fops.c +p' > /sys/kernel/debug/dynamic_debug/control

        echo 1 > /sys/kernel/debug/tracing/tracing_on
        echo 30720 > /sys/kernel/debug/tracing/buffer_size_kb
        echo 1 > /sys/kernel/debug/tracing/events/tegra_rtcpu/enable
        echo 1 > /sys/kernel/debug/tracing/events/tegra_rtcpu/enable
        echo 1 > /sys/kernel/debug/tracing/events/freertos/enable
        echo 1 > /sys/kernel/debug/tracing/events/camera_common/enable
        echo 3 > /sys/kernel/debug/camrtc/log-level
        echo > /sys/kernel/debug/tracing/trace
}

clear_trace()
{
        echo > /sys/kernel/debug/tracing/trace
}

open_trace()
{
        less /sys/kernel/debug/tracing/trace
}

max_speed()
{
        path=/sys/kernel/debug/bpmp/debug/clk

        echo 1 > ${path}/vi/mrq_rate_locked
        echo 1 > ${path}/isp/mrq_rate_locked
        echo 1 > ${path}/nvcsi/mrq_rate_locked
        echo 1 > ${path}/emc/mrq_rate_locked
        cat ${path}/vi/max_rate | tee ${path}/vi/rate
        cat ${path}/isp/max_rate | tee  ${path}/isp/rate
        cat ${path}/nvcsi/max_rate | tee ${path}/nvcsi/rate
        cat ${path}/emc/max_rate | tee ${path}/emc/rate
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
device=0
shutter=10000
gain=0
exposure=100000000
framerate=
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
        -c|--check)
                check_dt_settings
                exit 0
                ;;
        -d|--device)
                device="$1"
                shift
                ;;
        --debug)
                enable_debugging
                exit 0
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
        --io)
                io="$1"
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
        --trace)
                open_trace
                exit
                ;;
        -v|--viewer)
                install_avtviewer
                exit
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
        ./v4l2-test stream -d /dev/video${device} -e ${shutter} -g ${gain} -p 16
fi