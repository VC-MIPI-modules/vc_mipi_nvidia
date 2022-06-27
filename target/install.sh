#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Setup target for testing and development."
        echo ""
        echo "Supported options:"
        echo "-h, --help                Show this help text."
        echo "-i, --iqtuning            Setup IQ tuning."
        echo "-k, --kernel              Setup/Reset kernel sources."
}

setup_kernel() {
        if [[ -e /tmp/Image ]]; then
                sudo mv /tmp/Image /boot
                sudo reboot
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

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -h|--help)
                usage
                exit 0
                ;;
        -i|--iqtuning)
                setup_iq_tuning
                exit 0
                ;;
        -k|--kernel)
                setup_kernel
                exit 0
                ;;
        -p|--perf)
                setup_gst_perf_plugin
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done