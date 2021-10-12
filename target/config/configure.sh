#/bin/bash
#
# https://forums.developer.nvidia.com/t/capture-raw-image-with-v4l2-api/70231

CAMERA=$1
DATE_TIME=$(date '+%G%m%d_%H%M%S')
TEST_DIR="${CAMERA}_${DATE_TIME}"

echo $TEST_DIR
mkdir $TEST_DIR
cd $TEST_DIR

function exposure() {
    v4l2-ctl -d /dev/video$1 --set-ctrl exposure=$2
}

function gain() {
    v4l2-ctl -d /dev/video$1 --set-ctrl gain=$2
}

function test_standard() {
    v4l2-ctl --all > test_all.log
    v4l2-ctl --list-ctrls > test_ctrls.log
    v4l2-ctl -d /dev/video0 --list-formats-ext > test_v0_formats-ext.log
    v4l2-ctl -d /dev/video1 --list-formats-ext > test_v1_formats-ext.log
    v4l2-compliance > test_compliance.log
}

function test_raw_image() {
    v4l2-ctl -d /dev/video$1 \
        --set-ctrl bypass_mode=0 \
        --set-fmt-video=width=$2,height=$3,pixelformat=$4 \
        --stream-mmap \
        --stream-count=1 \
        --stream-to=$5
}