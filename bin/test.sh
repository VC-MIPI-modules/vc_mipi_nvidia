#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Setup host and target for testing and development."
        echo ""
        echo "Supported options:"
        echo "-b, --rebuild             Repatches kernel sources and rebuild all."
        echo "-h, --help                Show this help text."
        echo "-o, --host                Installs system tools, toolchain and board support package."
        echo "-p, --repatch             Repatches kernel sources."
}

setup() {
        ./config/setup.sh --som "$1" --board "$2" --bsp "$3" --driver
        ./setup.sh --host
}

repatch() {
        ./config/setup.sh --som "$1" --board "$2" --bsp "$3" --driver
        ./setup.sh --repatch
}

rebuild() {
        ./config/setup.sh --som "$1" --board "$2" --bsp "$3" --driver
        ./build.sh --all
}

run() {
        case "$1" in
        --setup)   setup $2 $3 $4 ;;
        --repatch) repatch $2 $3 $4 ;;
        --rebuild) rebuild $2 $3 $4 ;;
        esac
}

test() {
        # NanoSD, Auvidea_JNX30
        for bsp in {1..7}; do # 32.5.0 .. 32.7.3
                run $1 2 2 $bsp 
        done

        # XavierNXSD, Auvidea_JNX30
        for bsp in {1..10}; do # 32.5.0 .. 35.3.1
                run $1 5 2 $bsp
        done

        echo
        echo "============================================="
        echo "   Passed all tests"
        echo "============================================="
}

wait_for_first_bootup() {
        tty=/dev/ttyUSB0
        exec 4<$tty 5>$tty
        stty -F $tty 115200 -echo

        while [[ "${output}" != *"Ubuntu 18.04.5 LTS"* ]]; do
                read output <&4 
                echo "$output"
        done
}

flash_and_setup() {
        ./config/setup.sh --som "$1" --board "$2" --bsp "$3" --driver
        ./flash.sh --all
        wait_for_first_bootup
        ./setup.sh --target vc XavierNXSD
}

set -e 

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -b|--rebuild)
                test --repatch
                test --rebuild
                exit 0
                ;;
        -f|--flash)
                flash_and_setup 5 2 8
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -o|--host)
                test --setup
                exit 0
                ;;
        -p|--repatch)
                test --repatch
                exit 
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done

usage
exit 1