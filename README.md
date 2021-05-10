# VC MIPI® CSI-2 Camera Modules

MIPI CSI-2 camera modules are ideal for multi camera applications including mobile and distributed applications like autonomous driving, UAVs, Smart City, medical technology, and laboratory automation.

Use this guide to build the nesessary software and test your new VC MIPI Camera Module.

# Getting Started

This documentation will guide you to build a ready to use SD Card Image file to get your Vision Components MIPI CSI-2 Camera up an running in just a few steps. Currently we support NVIDIA Jetpack Version 4.3 and 4.4.1

## Setup your Host Machine
First of all you need a Linux Host System to build the Linux Kernel Image. 
NVIDIA recommends to run Ubuntu 16.04 LTS or 18.04 LTS.

## Download Build Scripts

    $ git clone https://github.com/pmliquify/vc_mipi_driver.git

## Setup NVIDIA toolchain and Jetpack Source Code

---
**INFO** The Setup Scripts will install nessecary build tool packages  
(build-essential, python2.7 python-to-python2, qemu-user-static)  
download and extract the Toolchain and all files of the Jetpack BSP and the Kernel Sources.
---

    $ cd vc_mipi_driver/bin
    $ ./setup.sh 43         # Example 1: If you want to use Jetpack 4.3
    $ ./setup.sh 441        # Example 2: If you want to use Jetpack 4.4.1

---

**CHECK** if everything went well. You will get an additional directory jp4.3 or jp4.4.1. Check if all folders are there.

---

    vc_mipi_driver
    |-- bin
    |-- docs
    |-- jp4.3                               # Tools and sources for Jetpack 4.3
    |    |-- downloads                      # Downloaded archives
    |    |-- Linux_for_Tegra
    |    |    | ...                         # Board support package
    |    |    |-- rootfs                     
    |    |    |    | ...                    # Sample root file system
    |    |    +-- source/public/kernel/kernel-4.9
    |    |         | ...                    # Kernel sources
    |    +-- toolchain
    |         +-- gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
    |              | ...                    # Cross compiler and other build tools
    |-- src
    +-- test

In the next step you will create the SD Card Image.

# Build a SD Card Image to use a specific MIPI CSI-2 camera module

## Build the SD Card Image
We have developed a script to easily create a SD Card Image. The script execute three steps:

* Patching NVIDIA Kernel Sources with VC MIPI Drivers
* Build the Kernel Image and Device Tree files
* Create the SD Card Image

Just execute following script

    $ cd vc_mipi_driver/bin
    $ ./create_sd_card_image.sh 43 OV9281   # Example 1: Jetpack 4.3 and Omnivision 9281 camera

    # Example 2
    # If you want to use Jetpack 4.4.1 and the Driver for the 
    # IMX 327C camera
    $ ./create_sd_card_image.sh 441 IMX327C

The build process and creation of the SD Card Image will take a while. If everything went well you will find the SD Card Image in the disc_images folder.


    vc_mipi_driver
    |-- bin
    |-- docs
    |-- jp4.3    
    |    |-- disc_images                     # Contains all created SD Card Images
    |    | ...
    |-- src
    +-- test

In the next step you will prepare your Developer Kit and create a SD Card with your freshly created Image.

# Setup the Jetson Nano Developer Kit

Please follow the instructions of the original Getting Started Tutorial but use the SD Card Image we just created.

---

**NVIDIA** [Getting Started with Jetson Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-nano-devkit)

---

Come back to this Guide and continue with the next step to test the camera.

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