# Version History

## v0.18.2
* New Features
  * Handler function added, which will be executed before the streaming in order to tell the user whether the frame size for binning is setup correctly.
* Improvements
  * Framerate increase for binning mode and frame line reduction with binning combined
  * Metadata Height values for IMX56x series corrected and documentation added.
  * Support for GBRG8 format added for IMX226
  * Added default values id_rsa and nvidia for the ./setup.sh --target

## v0.18.1 (L4T 36.2.0, Support IMX900)
* New Features
  * Added support for board support package
    * NVIDIA L4T 36.2.0 for NVIDIA Jetson Orin NX + Orin Nano DevKit
  * Added support for VC MIPI Camera Module
    * IMX900 adjusted values for Rev02 (exposure fix + static noise pattern removed)
  * Bugfix
    * L4T 36.2.0 script had a problem with the patch folder. Fixed.

## v0.18.0 (L4T 36.2.0, L4T 35.4.1, Support IMX900, Binning)

* New Features
  * Added support for board support package
    * NVIDIA L4T 36.2.0 for NVIDIA Jetson Orin Nano + Orin Nano DevKit
    * NVIDIA L4T 35.4.1 
  * Added support for VC MIPI Camera Module
    * IMX900 (exposure not working correctly when the exposure is less than the frametime)
  * Added support for binning *(IMX412, IMX565, IMX566, IMX567, IMX568 only)*
* Bugfix
  * Stability patch for L4T 35.4.1 implemented. Nvargus daemon got stuck sporadically after several hours of streaming via ISP. Fixed. 
* Improvements
  * Removed old L4T versions 32.3.1 - 32.6.1

## v0.17.1 (L4T 35.3.1)

* Bugfix
  * Stability patch implemented. Nvargus daemon got stuck sporadically after several hours of streaming via ISP. Fixed. 

## v0.17.0 (Orin Nano, Orin NX)

* New Features
  * Added support for system on modules
    * NVIDIA Jetson Orin Nano 8GB/4GB SD/NVME *(NVIDIA DevKit, Auvidea JNX42)*
    * NVIDIA Jetson Orin NX 16GB/8GB NVME *(Auvidea JNX42)*
  * Added support for board support package
    * NVIDIA L4T 32.7.4 *(only NVIDIA Jetson Nano)*
* Improvements
  * setup.sh script recognizes now already downloaded L4T archives
  * Infinite Trigger Timeout for NVIDIA Jetson Nano *(only L4T 32.7.4)*
  * Configuration of user credentials for target during setup
  * Function vc_mod_set_mode adjusted. When module is in streaming mode and the format is already set, then the module is not powered down and up again. In that case the additional wait time is omitted, too. 
  * Function ./setup.sh --target extended:
    * user's known_hosts file is being backed up
    * demo.sh and max_speed.sh scripts are being copied directly from the vc_mipi_nvidia/target/ directory to the /home/user/test/ folder on the device
  * demo.sh and max_speed.sh scripts extended. Usage of both scripts can be examined by the "-h" option

## v0.16.0 (Support IMX566, IMX567)

* New Features
  * Added support for VC MIPI Camera Modules
    * IMX566, IMX567
* Improvements
  * Added documentation for io_mode (flash) signals for all cameras
  * Added documentation for black level
* Bugfixes
  * Fixed a problem with IMX568 on NVIDIA Jetson Nano
  * Fixed low frame rate problem with IMX335 and Rev.02
  * Fixed tear down effect of Pregius S sensors when using uncommon widths

## v0.15.1 (Bugfixes)

* Bugfixes
  * Fixed a missing implementation to set up slave mode for IMX412

## v0.15.0 (JNX42, Bugfixes & Improvements)

* New Features
  * Added support for carrier boards
    * Auvidea JNX42 with NVIDIA Jetson Nano and Xavier NX *(JNX42 supports only one camera - Cam0 for Jetson Nano, TX2 NX and Xavier NX)*
* Bugfixes
  * Fixed an issue with ./setup.sh -c (It did not load the correct DT file in every case)
  * Fixed wrong physical_w and physical_h values in README.md
* Improvements
  * Build pipeline simplyfied by copying all DT files automatically as long as the destination path of the given L4T is present
  * Build pipeline is checking the integrity of the downloaded files

## v0.14.1 (Bugfixes)

* Bugfixes
  * Corrected the documentation in README.md and VERSION.md.
  * Fixed a bug in configure.sh with a missing DT file for AGX Xavier SoMs

## v0.14.0 (L4T 32.7.3, L4T 35.2.1, 35.3.1)

* New Features
  * Added support for carrier boards
    * Auvidea JNX30D with NVIDIA Jetson Nano and Xavier NX
  * Added support for board support packages
    * NVIDIA L4T 32.7.3
    * NVIDIA L4T 35.2.1 *(only NVIDIA Jetson Xavier NX and AGX Xavier)*
    * NVIDIA L4T 35.3.1 *(only NVIDIA Jetson Xavier NX and AGX Xavier)*
  * Added support for VC MIPI Camera Modules 
    * IMX462
    * IMX565
  * Bugfix
    * Added trigger support for OV9281

## v0.13.0 (L4T 35.1.0)

* New Features
  * Added support for board support packages
    * NVIDIA L4T 35.1.0 *(only NVIDIA Jetson Xavier NX and AGX Xavier)*

## v0.12.3 (Bugfixes)

* Bugfixes
  * Fixed an issue with the IMX183's self-triggering mode

## v0.12.2 (Bugfixes)

* Bugfixes
  * Fixed a problem in function vc_fix_image_size

## v0.12.1 (Improvements & Bugfixes)

* Improvements
  * Improved ROI cropping documentation with adjustable ranges for each camera
  * Reorganized common kernel patches in a shared folder
* Bugfixes
  * Reduced image size limitation from width 32 to 4 and height from 4 to 1
  * Fixed missing active_l and active_t device tree properties for Xavier and TX2 SoMs
  * Fixed a problem with image sizes in heterogenous multi camera setups
  * Added missing active_l and active_t device tree properties in README.md
  * Fixed image height workaround for new ROI cropping implementation

## v0.12.0 (ROI cropping by V4L)

* New Features
  * Added ROI cropping by the V4L API
  * Added the possibility to control the polarity of the trigger and flash signal
  * Added documentation for trigger modes, IO modes and ROI cropping
  * Added V4L2 control 'single_trigger'
* Improvements
  * Added frame rate control support for IMX412
  * Added ROI cropping for IMX296/IMX297
* Bugfixes
  * Fixed wrong exposure time calculation for IMX290
  * Fixed wrong pixel format for IMX290/IMX327
  * Fixed an issue with calculating the correct maximum exposure time for trigger modes
  * Fixed vmax default value and added framerate increase for IMX335

## v0.11.0 (Support GStreamer, ROI Cropping and L4T 32.7.1, 32.7.2)

* New Features
  * Added support for board support packages
    * NVIDIA L4T 32.7.1 (experimental)
    * NVIDIA L4T 32.7.2 (experimental)
  * Added support for ROI cropping and frame rate increase in case of smaller image height. ROI cropping can be set via device tree properties active_w and active_h
* Improvements
  * Added improvements for better GStreamer support. README.md now describes how to setup the device tree correct to get full nvarguscamerasrc support
  * V4L2 controls exposure and frame_rate now updates its maximal values due to pixel format change
* Bugfixes
  * Fixed the black frame problem from the IMX335 sensor
  * Fixed missing Y14 format handling in vc_init_image
  * Fixed an inconsistency in the i2c address assignment for VC_MIPI_MANUFACTURER

## v0.10.0 (Support OV7251, IMX297, IMX568)

* New Features
  * Added support for VC MIPI Camera Modules
    * OV7251, IMX297, IMX568
  * Black level can now also be set for IMX250, IMX252, IMX264, IMX265, IMX273, IMX392, IMX568
  * Bugfixes
    * Fixed a bug which caused a compilation error for all Xavier and TX2 plattforms

## v0.9.0 (Support IMX335 and Jetson Nano 2GB)

* New Features
  * Added support for system on modules
    * NVIDIA Jetson Nano 2GB
  * Added support for VC MIPI Camera Modules
    * IMX335
  * Black level can now also be set for IMX178 and IMX226
  * Bugfixes
    * Changed default resolutions from IMX178, IMX226 and IMX335 to NVIDIA compliant resolutions

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
  * Trigger modes '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'
  * Flash mode '0: disabled', '1: enabled' can be set via device tree or V4L2 control 'flash_mode'

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
  * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format
  * Exposure and Gain can be set via V4L2 library