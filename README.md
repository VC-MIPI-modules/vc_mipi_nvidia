# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano and Xavier NX
![VC MIPI camera](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/VC_MIPI_Camera_Module_Hardware_Operating_Manual-Dateien/mipi_sensor_front_back.png)

## Version 0.6.0 ([History](VERSION.md))
* Supported system on modules
  * [NVIDIA Jetson Nano (production + devkit)](https://developer.nvidia.com/embedded/jetson-nano)
  * [NVIDIA Jetson Xavier NX](https://developer.nvidia.com/embedded/jetson-xavier-nx)
* Supported carrier boards
  * [NVIDIA Jetson Nano Developer Kit B01](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
  * [NVIDIA Jetson Xavier NX Developer Kit](https://developer.nvidia.com/embedded/jetson-xavier-nx-devkit)
  * [Auvidea JNX30-LC-PD](https://auvidea.eu/product/70804) (only NVIDIA Jetson Nano (production + devkit))
* Supported board support packages
  * [NVIDIA L4T 32.5.0](https://developer.nvidia.com/embedded/linux-tegra-r325)
  * [NVIDIA L4T 32.5.1](https://developer.nvidia.com/embedded/linux-tegra-r3251)
  * [NVIDIA L4T 32.5.2](https://developer.nvidia.com/embedded/linux-tegra-r3251)
  * [NVIDIA L4T 32.6.1](https://developer.nvidia.com/embedded/linux-tegra-r3261)
* Supported [VC MIPI Camera Modules](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/index.html) 
  * IMX226, IMX183, IMX178
  * IMX252, IMX250, IMX392, IMX264, IMX265
  * IMX296
  * IMX327
  * IMX412
  * OV9281
* Features
  * Quickstart script for an easier installation process
  * Auto detection of VC MIPI camera module
  * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format.
  * Exposure and Gain can be set via V4L2 library.
  * Trigger modes '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'.
  * Flash mode '0: disabled', '1: enabled' can be set via device tree or V4L2 control 'flash_mode'.

## Prerequisites for cross-compiling
### Host PC
* Recommended OS is Ubuntu 18.04 LTS or Ubuntu 20.04 LTS
* You need git to clone this repository
* All other packages are installed by the scripts contained in this repository

# Quickstart
> When we use the **$** sign it means that the command is being executed on the host PC. A **#** sign indicates an command execution an the target.

1. Enter recovery mode by following the [Quick Start Guide](https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) instructions.   

2. Create a directory and clone the repository.
   ```
     $ cd <working_dir>
     $ git clone https://github.com/pmliquify/vc_mipi_nvidia
   ```

3. Start the quickstart installation process and follow the instructions.
   ```
     $ cd vc_mipi_nvidia/bin
     $ ./quickstart.sh
   ```
   > If you have changed your hardware setup simply execute this script again.

# Changing camera settings in the device tree
If you want to change some settings of an camera in the device tree, please follow these steps.

1. Edit the device tree file for your hardware setup. Currently there are three device tree files for three different combinations of SoMs and carrier boards.
   | system on module | carrier board | device tree file |
   | ---------------- | ------------- | ---------------- |
   | NVIDIA Jetson Nano | NVIDIA Jetson Nano Developer Kit B01 | src/devicetree/NV_DevKit_Nano_B01/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Nano | Auvidea JNX30-LC-PD | src/devicetree/Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | NVIDIA Jetson Xavier NX Developer Kit | src/devicetree/NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   
   To edit the correct device tree file you can simply use the setup script. It will open the device tree file in the nano editor.
   ```
     $ ./setup.sh --camera
   ```

2. Enter recovery mode by following the [Quick Start Guide](https://docs.nvidia.com/jetson/l4t/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) instructions.   

3. Build and flash the device tree files to the target.
   ```
     $ ./build.sh --dt
     $ ./flash.sh --dt
   ```

# Using external trigger mode with long waiting times (> 5 seconds)

If you want to use your camera in an application with external trigger and the time between two consecutively triggers is potentially long (> 5 seconds) it is necessary to adjust the timeout of the csi receiver. In this case please change following line of code.

   | system on module | modification |
   | ---------------- | ------------ |
   | NVIDIA Jetson Nano | Line 232 in /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi2_fops.c |
   | NVIDIA Jetson Xavier NX | Line 36 in /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi5_fops.c |

# Testing the camera
To test the camera you can use [Vision Components MIPI CSI-2 demo software](https://github.com/pmliquify/vc_mipi_demo)
