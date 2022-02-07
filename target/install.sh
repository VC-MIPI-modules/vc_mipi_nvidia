#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Setup target for testing and development."
        echo ""
        echo "Supported options:"
        echo "-h, --help                Show this help text."
        echo "-k, --kernel              Setup/Reset kernel sources."
}

setup_kernel() {
        if [[ -e /tmp/Image ]]; then
                sudo mv /tmp/Image /boot
                sudo reboot
        fi
}

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -h|--help)
                usage
                exit 0
                ;;
        -k|--kernel)
                setup_kernel
                exit 0
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done