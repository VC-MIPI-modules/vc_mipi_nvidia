#!/bin/bash

usage() {
        echo "Usage: $0 [options]"
        echo ""
        echo "Script to test and demonstrate camera features."
        echo ""
        echo "Supported options:"
        echo "-d, --default             Set default rates"
        echo "-e, --emc                 Lock emc rate"
        echo "-i, --isp                 Lock isp rate"
        echo "-l, --lock                Lock rates"
        echo "-m, --max                 Set max rates"
        echo "-n, --nvcsi               Set nvcsi rate"
        echo "-p, --print               Print current rates"
        echo "-u, --unlock              Unlock rates"
        echo "-v, --vi                  Set vi rate"
}

path=/sys/kernel/debug/bpmp/debug/clk

print_rates() {
        echo "vi:    [$(cat ${path}/vi/mrq_rate_locked)] $(cat ${path}/vi/rate) / ($(cat ${path}/vi/min_rate) - $(cat ${path}/vi/max_rate))"
        echo "isp:   [$(cat ${path}/isp/mrq_rate_locked)] $(cat ${path}/isp/rate) / ($(cat ${path}/isp/min_rate) - $(cat ${path}/isp/max_rate))"
        echo "nvcsi: [$(cat ${path}/nvcsi/mrq_rate_locked)] $(cat ${path}/nvcsi/rate) / ($(cat ${path}/nvcsi/min_rate) - $(cat ${path}/nvcsi/max_rate))"
        echo "emc:   [$(cat ${path}/emc/mrq_rate_locked)] $(cat ${path}/emc/rate) / ($(cat ${path}/emc/min_rate) - $(cat ${path}/emc/max_rate))"
}

lock_rates() {
        echo "Lock rates $1 ..."
        echo $1 > ${path}/vi/mrq_rate_locked
        echo $1 > ${path}/isp/mrq_rate_locked
        echo $1 > ${path}/nvcsi/mrq_rate_locked
        echo $1 > ${path}/emc/mrq_rate_locked
}

set_max_rates() {
        echo "Set max rates ..."
        lock_rates 1
        cat ${path}/vi/max_rate | tee ${path}/vi/rate
        cat ${path}/isp/max_rate | tee  ${path}/isp/rate
        cat ${path}/nvcsi/max_rate | tee ${path}/nvcsi/rate
        cat ${path}/emc/max_rate | tee ${path}/emc/rate
}

# NOTE: It is necessary to start demo.sh before using AVTViewer
# Tested with:
# LANES1  IMX296
# LANES2  IMX226, OV9281
# LANES4  IMX178, IMX183, IMX226, IMX250, IMX252, IMX273, IMX327, IMX335
#         IMX392, IMX412, IMX415, IMX462, IMX565, IMX566, IMX567, IMX568
declare -A param
#              pix_clk_hz         vi        isp      nvcsi         emc
#        min               115200000  115200000   10045312   204000000
#        max               550400000  729600000  214300000  2133000000  
param[LANES1]=' 150000000  115200000  115200000   23811111  2133000000'
param[LANES2]=' 300000000  115200000  192000000   47622222  2133000000'
param[LANES4]=' 600000000  115200000  371200000   98907692  2133000000'

param() {
        values=($(echo ${param[$2]}))
        echo ${values[$1]}
}

lanes="LANES4"

set_default_rates() {
        echo "Set default rates for ${lanes} ..."
        echo "Need pix_clk_hz = $(param 0 ${lanes})"
        lock_rates 1
        echo $(param 1 ${lanes}) > ${path}/vi/rate
        echo $(param 2 ${lanes}) > ${path}/isp/rate
        echo $(param 3 ${lanes}) > ${path}/nvcsi/rate
        echo $(param 4 ${lanes}) > ${path}/emc/rate
}

set_vi_rate() {
        echo "Set vi rate to $1"
        echo 1 > ${path}/vi/mrq_rate_locked
        echo $1 > ${path}/vi/rate
}

set_isp_rate() {
        echo "Set isp rate to $1"
        echo 1 > ${path}/isp/mrq_rate_locked
        echo $1 > ${path}/isp/rate
}

set_nvcsi_rate() {
        echo "Set nvcsi rate to $1"
        echo 1 > ${path}/nvcsi/mrq_rate_locked
        echo $1 > ${path}/nvcsi/rate
}

set_emc_rate() {
        echo "Set emc rate to $1"
        echo 1 > ${path}/emc/mrq_rate_locked
        echo $1 > ${path}/emc/rate
}

while [ $# != 0 ] ; do
        option="$1"
        shift

        case "${option}" in
        -d|--default)
                set_default_rates
                ;;
        -e|--emc)
                set_emc_rate "$1"
                shift
                ;;
        -h|--help)
                usage
                exit 0
                ;;
        -i|--isp)
                set_isp_rate "$1"
                shift
                ;;
        -l|--lock)
                lock_rates 1
                ;;
        -m|--max)
                set_max_rates
                ;;
        -n|--nvcsi)
                set_nvcsi_rate "$1"
                shift
                ;;
        -p|--print)
                print_rates
                ;;
        -u|--unlock)
                lock_rates 0
                ;;
        -v|--vi)
                set_vi_rate "$1"
                shift
                ;;
        *)
                echo "Unknown option ${option}"
                exit 1
                ;;
        esac
done