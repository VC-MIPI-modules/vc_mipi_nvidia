#!/bin/bash

sensor_id=0

function pause(){
   read -p "$*"
}

# set -x
# set -u
# set -e

rc=$?

if [ "$1" != "" ]; then sensor_id=$1; fi

echo ------------------------------------------------------------
echo "   Play VC MIPI sensor IMX415 stream on Jetson by GStreamer"
echo "Usage:"
echo "   $0 [sensor_id]"
echo "where:"
echo "   sensor_id = Sensor index: 0=video0, 1=video1, ..."
echo ------------------------------------------------------------
echo sensor-id=$sensor_id

gst-launch-1.0 nvarguscamerasrc sensor-id=$sensor_id aelock=true awblock=true aeantibanding=0 ! \
 'video/x-raw(memory:NVMM),width=3864,height=2192,framerate=10/1,format=NV12' !\
 nvvidconv flip-method=0 ! 'video/x-raw,width=966,height=548' ! nvvidconv ! nvegltransform ! nveglglessink -e
