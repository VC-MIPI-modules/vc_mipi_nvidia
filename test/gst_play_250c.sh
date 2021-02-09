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

echo --------------------------------------------------------------
echo "   Play VC MIPI sensor IMX250C stream on Jetson by GStreamer"
echo "Usage:"
echo "   $0 [sensor_id]"
echo "where:"
echo "   sensor_id = Sensor index: 0=video0, 1=video1, ..."
echo --------------------------------------------------------------
echo sensor-id=$sensor_id

gst-launch-1.0 nvarguscamerasrc sensor-id=$sensor_id aelock=true awblock=true aeantibanding=0 ! \
'video/x-raw(memory:NVMM),width=2432,height=2048,framerate=20/1,format=NV12' !\
 nvvidconv flip-method=0 ! 'video/x-raw,width=608, height=512' ! nvvidconv ! nvegltransform ! nveglglessink -e
