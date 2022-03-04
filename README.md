# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano, Xavier NX, AGX Xavier and TX2
![VC MIPI camera](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/VC_MIPI_Camera_Module_Hardware_Operating_Manual-Dateien/mipi_sensor_front_back.png)

## Version 0.10.0 ([History](VERSION.md))
* Supported system on modules
  * [NVIDIA Jetson Nano 4GB/2GB (production + devkit)](https://developer.nvidia.com/embedded/jetson-nano)
  * [NVIDIA Jetson Xavier NX (production + devkit)](https://developer.nvidia.com/embedded/jetson-xavier-nx)
  * [NVIDIA Jetson AGX Xavier](https://developer.nvidia.com/embedded/jetson-agx-xavier-developer-kit)
  * [NVIDIA Jetson TX2](https://developer.nvidia.com/embedded/jetson-tx2-developer-kit)
* Supported carrier boards
  * [NVIDIA Jetson Nano Developer Kit B01](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
  * [NVIDIA Jetson Nano 2GB Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-2gb-developer-kit)
  * [NVIDIA Jetson Xavier NX Developer Kit](https://developer.nvidia.com/embedded/jetson-xavier-nx-devkit)
  * [Auvidea JNX30-LC-PD](https://auvidea.eu/product/70804)
  * [Auvidea J20 on Devkit Jetson AGX Xavier or TX2](https://auvidea.eu/j20/) *(only connector 2+3)*
* Supported board support packages
  * [NVIDIA L4T 32.3.1](https://developer.nvidia.com/l4t-3231-archive) *(only NVIDIA Jetson AGX Xavier)*
  * [NVIDIA L4T 32.5.0](https://developer.nvidia.com/embedded/linux-tegra-r325)
  * [NVIDIA L4T 32.5.1](https://developer.nvidia.com/embedded/linux-tegra-r3251)
  * [NVIDIA L4T 32.5.2](https://developer.nvidia.com/embedded/linux-tegra-r3251)
  * [NVIDIA L4T 32.6.1](https://developer.nvidia.com/embedded/linux-tegra-r3261)
* Supported [VC MIPI Camera Modules](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/index.html) 
  * IMX178, IMX183, IMX226
  * IMX250, IMX252, IMX264, IMX265, IMX273, IMX392
  * IMX290, IMX327
  * IMX296, IMX297
  * IMX335
  * IMX412
  * IMX415
  * IMX568
  * OV7251, OV9281
* Features
  * Quickstart script for an easier installation process
  * Auto detection of VC MIPI camera modules
  * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12 format
  * **Exposure** can be set via V4L2 control 'exposure'
  * **Gain** can be set via V4L2 control 'gain'
  * **Trigger mode** '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'
  * **Flash mode** '0: disabled', '1: enabled' can be set via device tree or V4L2 control 'flash_mode'
  * **Frame rate** can be set via V4L2 control 'frame_rate' *(except IMX412 and OV9281)*
  * **Black level** can be set via V4L2 control 'black_level' *(except IMX290, IMX327, IMX412, IMX415, OV7251 and OV9281)*

## Prerequisites for cross-compiling
### Host PC
* Recommended OS is Ubuntu 18.04 LTS or Ubuntu 20.04 LTS
* You need git to clone this repository
* All other packages are installed by the scripts contained in this repository

# Quickstart

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

1. Edit the device tree file for your hardware setup. Currently there are four device tree files for four different combinations of SoMs and carrier boards.
   | system on module | carrier board | device tree file |
   | ---------------- | ------------- | ---------------- |
   | NVIDIA Jetson Nano | NVIDIA Jetson Nano Developer Kit | src/devicetree/NV_DevKit_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Nano | Auvidea JNX30-LC-PD | src/devicetree/Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | NVIDIA Jetson Xavier NX Developer Kit | src/devicetree/NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | Auvidea JNX30-LC-PD | src/devicetree/Auvidea_JNX30_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson AGX Xavier | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_AGXXavier/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson TX2 | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_TX2/tegra186-camera-vc-mipi-cam.dtsi |
   
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

# Using long exposure times or external trigger mode with long waiting times (> 5 seconds)

If you want to use your camera in an application with long exposure times or external trigger and the time between two consecutively triggers is potentially long (> 5 seconds) it is necessary to adjust the timeout of the csi receiver. In this case please change following line of code.

   | system on module         | line | in file                                                          |
   | ------------------------ | ---- | ---------------------------------------------------------------- |
   | NVIDIA Jetson Nano       |  232 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi2_fops.c |
   | NVIDIA Jetson Xavier NX  |   36 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi5_fops.c |
   | NVIDIA Jetson AGX Xavier |   36 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi5_fops.c |
   | NVIDIA Jetson TX2        | 1097 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi4_fops.c |

# Tested with VC MIPI Camera Module Revision

  * IMX178 (Rev.01), IMX183 (Rev.12), IMX226 (Rev.13), 
  * IMX250 (Rev.07), IMX252 (Rev.10), IMX264 (Rev.03), IMX265 (Rev.01), IMX273 (Rev.13), IMX392 (Rev.06)
  * IMX290 (Rev.02), IMX327 (Rev.02)
  * IMX296 (Rev.42), IMX297 (Rev.??)
  * IMX335 (Rev.00)
  * IMX412 (Rev.02)
  * IMX415 (Rev.01)
  * IMX568 (Rev.01)
  * OV7251 (Rev.01), OV9281 (Rev.02)

You can find the revision of the camera module in the dmesg log.
```
  # dmesg | grep 'i2c'
  [...] i2c 6-0010: +--- VC MIPI Camera -----------------------------------+
  [...] i2c 6-0010: | MANUF. | Vision Components               MID: 0x0427 |
  [...] i2c 6-0010: | MODULE | ID:  0x0183                     REV:   0012 |
  [...] i2c 6-0010: | SENSOR | SONY IMX183                                 |
  ...
```

# Integrate the driver in your own BSP

If you have your own BSP, you have to integrate the driver into it. Please follow these steps.

1. Apply all patches listed in the following table that match your hardware setup
   
   | system on module         | carrier board | BSP             | all patches in folder patch/... |
   | ------------------------ | ------------- | --------------- | --------------------- |
   | NVIDIA Jetson Nano       | NVIDIA DevKit | 32.5.0 - 32.5.2 | dt_camera_Nano_32.5.0+ <br> kernel_Nano_32.5.0+  |
   |                          |               | 32.6.1          | dt_camera_Nano_32.6.1+ <br> kernel_Nano_32.5.0+  |
   |                          | Auvidea JNX30 | 32.5.0 - 32.5.2 | dt_Auvidea_JNX30_Nano_32.5.0+ <br> dt_camera_Nano_32.5.0+ <br> kernel_Nano_32.5.0+ |
   |                          |               | 32.6.1          | dt_Auvidea_JNX30_Nano_32.5.0+ <br> dt_camera_Nano_32.6.1+ <br> kernel_Nano_32.5.0+ |
   | NVIDIA Jetson Xavier NX  | NVIDIA DevKit | 32.5.0 - 32.5.2 | dt_camera_XavierNX_32.5.0+ <br> kernel_Xavier_32.5.0+  |
   |                          |               | 32.6.1          | dt_camera_XavierNX_32.6.1+ <br> kernel_Xavier_32.6.1+  |
   |                          | Auvidea JNX30 | 32.5.0 - 32.5.2 | dt_Auvidea_JNX30_XavierNX_32.5.0+ <br> dt_camera_XavierNX_32.5.0+ <br> kernel_Xavier_32.5.0+  |
   |                          |               | 32.6.1          | dt_Auvidea_JNX30_XavierNX_32.5.0+ <br> dt_camera_XavierNX_32.6.1+ <br> kernel_Xavier_32.6.1+  |
   | NVIDIA Jetson AGX Xavier | DevKit + J20  | 32.3.1          | dt_camera_AGXXavier_32.3.1+ <br> kernel_Xavier_32.3.1+  |
   |                          |               | 32.5.0 - 32.5.2 | dt_camera_AGXXavier_32.3.1+ <br> kernel_Xavier_32.5.0+  |
   |                          |               | 32.6.1          | dt_camera_AGXXavier_32.3.1+ <br> kernel_Xavier_32.6.1+  |
   | NVIDIA Jetson TX2        | DevKit + J20  | 32.5.0 - 32.5.2 | dt_camera_TX2_32.5.0+ <br> kernel_TX_32.5.0+  |
   |                          |               | 32.6.1          | dt_camera_TX2_32.5.0+ <br> kernel_TX_32.6.1+  |

2. Copy the camera device tree to the folder listed in the following table

   | system on module         | carrier board | copy from src/devicetree/... to folder |
   | ------------------------ | ------------- | ---------------------------------- |
   | NVIDIA Jetson Nano       | NVIDIA DevKit | NV_DevKit_Nano/tegra210-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms |
   |                          | Auvidea JNX30 | Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms |
   | NVIDIA Jetson Xavier NX  | NVIDIA DevKit | NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/jakku/kernel-dts/common |
   |                          | Auvidea JNX30 | Auvidea_JNX30_XavierNX/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/jakku/kernel-dts/common |
   | NVIDIA Jetson AGX Xavier | DevKit + J20  | Auvidea_J20_AGXXavier/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/common/kernel-dts/t19x-common-modules |
   | NVIDIA Jetson TX2        | DevKit + J20  | Auvidea_J20_TX2/tegra186-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-modules |

3. Copy all driver files from folder **src/driver** to **/kernel/nvidia/drivers/media/i2c**

# Testing the camera
To test the camera you can use [Vision Components MIPI CSI-2 demo software](https://github.com/pmliquify/vc_mipi_demo)
