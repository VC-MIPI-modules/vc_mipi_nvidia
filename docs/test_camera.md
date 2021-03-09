# Test Camera

We assume that your Jetson Nano Dev Kit Board is powered up running with the created SD Card Image.

## Connect the Camera

Please power off the device and make shure that you habe connected the VCFPC 22->15 cable properly.

![VCFPC 22->15 cable](/img/Jetson-Nano-VCFPC.jpeg)

## Install Video4Linux Utilities

Power up the Dev-Kit, login and install the CLI tools of Video4Linux.

    $ sudo apt update
    $ sudo apt install v4l-utils

## Check Camera Driver

First we want to check if the camera is properly detected.

    $ v4l2-ctl --all

The value of `Card type` should show the connected camera description.

    Driver Info (not using libv4l2):
	Driver name   : tegra-video
	Card type     : vi-output, ov9281 7-0060
	Bus info      : platform:54080000.vi:0
	Driver version: 4.9.140
    ...

## Start Live View

To start a live view execute a simple GStreamer Pipeline. It streams an image from the v4l2src element, converts it automaticaly and displays it in a X11 window.

    $ gst-launch-1.0 \
        v4l2src device="/dev/video0" ! \
        videoconvert ! \
        xvimagesink sync=false 

![Live View](/img/Jetson-Nano-LiveView.png)   

## Adjust Exposure and Gain

If the camera image is to dark or to bright simply adjust the exposure and gain values.   
In a second terminal type:

    $ v4l2-ctl -d /dev/video0 -c exposure=30000
    $ v4l2-ctl -d /dev/video0 -c gain=50

## Change Pixel Format

Find out which Pixel Formats the camera supports.

    $ v4l2-ctl --list-formats

For example `'GRAY'` and `'Y10 '` as shown in the following listing.

    ioctl: VIDIOC_ENUM_FMT
	    Index       : 0
	    Type        : Video Capture
	    Pixel Format: 'GREY'
	    Name        : 8-bit Greyscale
		    Size: Discrete 1280x800
			    Interval: Discrete 0.033s (30.000 fps)

	    Index       : 1
	    Type        : Video Capture
	    Pixel Format: 'Y10 '
	    Name        : 10-bit Greyscale
		    Size: Discrete 1280x800
			    Interval: Discrete 0.033s (30.000 fps)

Change the Pixel Format by using the v4l2-ctl command again.   
**NOTE** Don't miss the whitespace in the Pixel Format Identifier.

    $ v4l2-ctl --set-fmt-video=pixelformat='Y10 '

## Next Steps

Congratulations your VC MIPI Camera Module is up an running. Now you can dive in and start develop your own fancy product. Refere to the [NVIDIA Developer Documentation](https://docs.nvidia.com/jetson/l4t/index.html) to learn more.