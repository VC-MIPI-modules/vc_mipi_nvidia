# Version History

## v0.6.0 (Support IMX264 and easier testing)
  * New Features
    * Added this version description
    * Easier installation of demo.sh and its dependencies
    * Supported VC MIPI Camera Modules
      * IMX264

## v0.5.0 (Support Jetson Xavier NX)
  * New Features
    * Supported system on modules
      * NVIDIA Jetson Nano (devkit)
      * NVIDIA Jetson Xavier NX
    * Supported carrier boards
      * NVIDIA Jetson Xavier NX Developer Kit
    * Supported board support packages
      * NVIDIA L4T 32.5.0
      * NVIDIA L4T 32.5.2
      * NVIDIA L4T 32.6.1 (only NVIDIA Jetson Nano (production))
    * Supported VC MIPI Camera Modules
      * IMX412
    * Quickstart script for an easier installation process
  * Removed Features
    * Supported board support packages
      * NVIDIA L4T 32.3.1
      * NVIDIA L4T 32.4.4

## v0.4.1 (L4T 32.5.1 on Nano DevKit)
  * New Features
    * Supported board support packages on NVIDIA Jetson Nano Developer Kit B01
      * NVIDIA L4T 32.5.1

## v0.4.0 (Trigger and flash modes)
  * New Features
    * Supported VC MIPI Camera Modules
      * IMX178, IMX226, IM296, OV9281
    * Trigger modes '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'.
    * Flash mode '0: disabled', '1: enabled' can be set via device tree or V4L2 control 'flash_mode'.

## v0.3.0 (Universal driver)
  * New Features
    * Complete new driver structure as universal driver for all VC MIPI CSI-2 camera modules
    * Camera module auto detect
    * Supported VC MIPI Camera Modules
      * IMX183, IMX252, IMX327

## v0.2.0 (Support Auvidea JNX30)
  * New Features
    * Supported carrier boards
      * Auvidea JNX30-LC-PD
    * Supported board support packages
      * NVIDIA L4T 32.5.1 (only Auvidea JNX30-LC-PD)

## v0.1.0 (Support Jetson Nano)
  * New Features
    * Supported system on modules
      * NVIDIA Jetson Nano (production)
    * Supported board support packages
      * NVIDIA L4T 32.3.1
      * NVIDIA L4T 32.4.4
    * Supported VC MIPI Camera Modules
      * IMX183, IMX226, IMX250, IMX252, IMX273, IMX290, IMX296, IMX327, IMX412, IMX415, OV9281
    * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format.
    * Exposure and Gain can be set via V4L2 library.