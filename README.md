# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano
![VC MIPI camera](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/VC_MIPI_Camera_Module_Hardware_Operating_Manual-Dateien/mipi_sensor_front_back.png)

## Version 0.4.0
* Supported boards
  * NVIDIA Dev Kit B01
  * Auvidea JNX30
* Supported cameras Jetpack 4.5
  * VC MIPI IMX178
  * VC MIPI IMX183 / VC MIPI IMX183C
  * VC MIPI IMX226 / VC MIPI IMX226C
  * VC MIPI IMX252 / VC MIPI IMX252C
  * VC MIPI IMX296 / VC MIPI IMX296C
  * VC MIPI IMX327C
  * VC MIPI OV9281
* Features
  * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format.
  * Exposure and Gain can be set via V4L2 library.
  * Trigger modes '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream level' can be set via device tree or V4L2 control 'trigger_mode'
  * Flash mode can be set via device tree or V4L2 control 'flash_mode'
* Bugfixes
  * Fixed a problem with different case sensitivity in check_recovery_mode() function in flash.sh script.

## Prerequisites for cross-compiling
### Host PC
* Recommended OS is Ubuntu 18.04 LTS or Ubuntu 20.04 LTS
* You need git to clone this repository
* All other packages are installed by the scripts contained in this repository

# Installation
When we use the **$** sign it is meant that the command is executed on the host PC. A **#** sign indicates the promt from the target so the execution on the target. In our case the Ixora Apalis board.

1. Create a directory and clone the repository   
   ```
     $ cd <working_dir>
     $ git clone https://github.com/pmliquify/vc_mipi_nvidia
   ```

2. Define which Jetpack and carrier board to use. In this example Jetpack 4.5 and carrier board Auvidea JNX30.
   ```
     $ cd vc_mipi_nvidia/bin
     $ export VC_MIPI=45a
   ```

3. Setup the toolchain and Jetpack and kernel sources. The script will additionaly install some necessary packages like *build-essential*.
   ```
     $ ./setup.sh --host
   ```
4. Define which camera to activate in the device tree. A device tree file will open and you have to set a value of 1 for the driver to activate. 
   > Jetpack 4.5 and JNX30 Auvidea carrier board: The universal driver with auto detection is released. You don't have to activate a specific device tree file anymore.
   ```
     $ ./setup.sh --camera
   ```

4. Build the kernel image, kernel modules and device tree files.
   ```
     $ ./build.sh --all
   ```

5. Enter recovery mode by following the [Quick Start Guide](https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) instructions.   
   ```
     $ ./flash.sh --all
   ```

# Testing the camera
To test the camera you can use [Vision Components MIPI CSI-2 demo software](https://github.com/pmliquify/vc_mipi_demo)
