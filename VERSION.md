# Version History

## v0.9.0 (Support IMX335 and Jetson Nano 2GB)
  * New Features
    * Added support for system on modules
      * NVIDIA Jetson Nano 2GB
    * Added support for VC MIPI Camera Modules
      * IMX335
    * Black level can now also be set for IMX178 and IMX226
    * Bugfixes
      * Changed default resolutions from IMX178, IMX226 and IMX335 to NVIDIA compliant resolutions.

## v0.8.1 (Bugfix)
  * Bugfixes
    * Fixed the version number of the driver module
    * Fixed the trigger mode description from trigger mode STREAM_LEVEL

## v0.8.0 (Support Jetson AGX Xavier, TX2)
  * New Features
    * Added support for board support packages
      * NVIDIA L4T 32.3.1 *(only NVIDIA Jetson AGX Xavier)*
    * Added support for system on modules
      * NVIDIA Jetson AGX Xavier
      * NVIDIA Jetson TX2
    * Added support for carrier boards
      * Auvidea J20 on Devkits from NVIDIA Jetson AGX Xavier and TX2 *(only connector 2+3)*
    * Frame rate can be set via V4L2 control 'frame_rate' *(except IMX412 and OV9281)*
    * Black level can be set via V4L2 control 'black_level' *(only IMX183 and IMX296)*
  * Bugfixes
    * Removed a problem in self trigger mode while changing the exposure time

## v0.7.1 (Improvements)
  * Improvements
    * Added documentation for the correct setup of embedded_metadata_height in the device tree
    * Optimized default settings of gain and exposure in the device tree for better auto exposure control
    * Added support of GBRG pixel format for IMX415
    * Optimized exposure time calculation for IMX290 and IMX327

## v0.7.0 (Support IMX273, IMX415 and Xavier NX on JNX30)
  * New Features
    * Added support for carrier boards
      * Auvidea JNX30-LC-PD with NVIDIA Jetson Xavier NX
    * Added support for VC MIPI Camera Modules
      * IMX273, IMX290, IMX415

## v0.6.0 (Support IMX250, IMX264, IMX265, IMX392 and L4T 32.6.1)
  * New Features
    * Added this version description
    * Easier installation of demo.sh and its dependencies
    * Added support for board support packages
      * NVIDIA L4T 32.6.1 (all SoMs)
    * Added support for VC MIPI Camera Modules
      * IMX250, IMX264, IMX265, IMX392

## v0.5.0 (Support IMX412 and Jetson Xavier NX)
  * New Features
    * Added support for system on modules
      * NVIDIA Jetson Nano (devkit)
      * NVIDIA Jetson Xavier NX
    * Added support for carrier boards
      * NVIDIA Jetson Xavier NX Developer Kit
    * Added support for board support packages
      * NVIDIA L4T 32.5.0
      * NVIDIA L4T 32.5.2
      * NVIDIA L4T 32.6.1 (only NVIDIA Jetson Nano (production))
    * Added support for VC MIPI Camera Modules
      * IMX412
    * Quickstart script for an easier installation process
  * Removed Features
    * Removed support for board support packages
      * NVIDIA L4T 32.3.1
      * NVIDIA L4T 32.4.4

## v0.4.1 (L4T 32.5.1 on Nano DevKit)
  * New Features
    * Added support for board support packages on NVIDIA Jetson Nano Developer Kit B01
      * NVIDIA L4T 32.5.1

## v0.4.0 (Trigger and flash modes)
  * New Features
    * Added support for VC MIPI Camera Modules
      * IMX178, IMX226, IMX296, OV9281
    * Trigger modes '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'.
    * Flash mode '0: disabled', '1: enabled' can be set via device tree or V4L2 control 'flash_mode'.

## v0.3.0 (Universal driver)
  * New Features
    * Complete new driver structure as universal driver for all VC MIPI CSI-2 camera modules
    * Camera module auto detect
    * Added support for VC MIPI Camera Modules
      * IMX183, IMX252, IMX327

## v0.2.0 (Support Auvidea JNX30)
  * New Features
    * Added support for carrier boards
      * Auvidea JNX30-LC-PD
    * Added support for board support packages
      * NVIDIA L4T 32.5.1 (only Auvidea JNX30-LC-PD)

## v0.1.0 (Support Jetson Nano)
  * New Features
    * Added support for system on modules
      * NVIDIA Jetson Nano (production)
    * Added support for board support packages
      * NVIDIA L4T 32.3.1
      * NVIDIA L4T 32.4.4
    * Added support for VC MIPI Camera Modules
      * IMX183, IMX226, IMX250, IMX252, IMX273, IMX290, IMX296, IMX327, IMX412, IMX415, OV9281
    * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format.
    * Exposure and Gain can be set via V4L2 library.