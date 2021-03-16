#/bin/bash
#

if [[ -z $3 ]]; then 
  echo "Usage: ./test.sh DEVICE TEST FORMAT"
  exit
fi
VIDEO=$1

TESTS=("VC MIPI Demo" "GST with V4L2" "GST with ARGUS")
if (( $2 < 0 || 2 < $2 )); then
  echo "Test not supported: Options [0-2]"
  for i in "${!TESTS[@]}"; do
    echo "$i: ${TESTS[i]}"
  done
  exit
fi
TEST_DESC=${TESTS[$2]}
TEST=$2

FORMATS=("GREY" "Y10 " "Y12 " "RGGB" "RG10" "RG12")
if (( $3 < 0 || 5 < $3 )); then
  echo "Format not supported: Options [0-5]"
  for i in "${!FORMATS[@]}"; do
    echo "$i: ${FORMATS[i]}"
  done
  exit
fi
FORMAT=${FORMATS[$3]}

echo "==================================================================="
echo "Testing /dev/video$VIDEO with [$TEST_DESC] and format [$FORMAT] ..."
echo "==================================================================="

case $TEST in
  0)
    #*** VC MIPI Demo *********************************************************************
    case $3 in
      1) v4l2-ctl -d /dev/video$VIDEO --set-fmt-video=pixelformat='Y10 ' ;;
      2) v4l2-ctl -d /dev/video$VIDEO --set-fmt-video=pixelformat='Y12 ' ;;
      *) v4l2-ctl -d /dev/video$VIDEO --set-fmt-video=pixelformat=$FORMAT ;;
    esac
    FORMAT_CHECK=$(v4l2-ctl -d /dev/video$VIDEO --get-fmt-video | grep -o \'.*\')
    if [[ $FORMAT_CHECK == "'"$FORMAT"'" ]]; then
      echo "Format $FORMAT_CHECK successfully set"
    else
      echo "ERROR: Unable to set format! (Current format: $FORMAT_CHECK)"
      exit
    fi
    ./vcmipidemo -d$VIDEO -fa -s30000
  ;;

  1)
    #*** GST with V4L2 ********************************************************************
    # Use GST_DEBUG=2 do debug the GStreamer Pipeline
    #
    # Example 1:
    # gst-launch-1.0 v4l2src ! multifilesink location="test%d.raw"
    #
    # Example 2: For bayer sensors.
    # gst-launch-1.0 v4l2src device="/dev/video$1" ! \
    #   'video/x-bayer,format=rggb' ! \
    #   bayer2rgb ! \
    #   videoconvert ! \
    #   videoscale ! \
    #   'video/x-raw,width=960,height=760' ! \
    #   xvimagesink sync=false
    #
    # *************************************************************************************
    case $3 in
      0)
        gst-launch-1.0 v4l2src device="/dev/video$VIDEO" ! \
          'video/x-raw, format=GRAY8' ! \
          videoconvert ! \
          xvimagesink sync=false
        ;;
      3)
        gst-launch-1.0 v4l2src device="/dev/video$VIDEO" ! \
          'video/x-bayer, format=rggb' ! \
          bayer2rgb ! \
          videoconvert ! \
          xvimagesink sync=false
        ;;
      *)
        gst-launch-1.0 v4l2src device="/dev/video$VIDEO" ! \
          videoconvert ! \
          xvimagesink sync=false
        ;;
      esac
    ;;

  2)
    # *** GST with ARGUS *******************************************************************
    # Use logs from /usr/sbin/nvargus-daemon to analyse libargus
    #
    # Example 1: With ISP Support
    # gst-launch-1.0 \
    #   nvarguscamerasrc sensor-id=$VIDEO aelock=true awblock=true aeantibanding=0 ! \
    #   'video/x-raw(memory:NVMM),width=3840,height=3040, framerate=10/1, format=NV12' ! \
    #   nvvidconv flip-method=0 ! \
    #   'video/x-raw,width=680, height=506' ! \
    #   nvvidconv ! \
    #   nvegltransform ! \
    #   nveglglessink -e
    #
    # **************************************************************************************
    gst-launch-1.0 \
      nvarguscamerasrc sensor-id=$VIDEO aelock=true awblock=true aeantibanding=0 ! \
      'video/x-raw(memory:NVMM), framerate=20/1, format=NV12' ! \
      nvvidconv ! \
      nveglglessink -e
    ;;  
esac