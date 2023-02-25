# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano, Xavier NX, AGX Xavier and TX2
![VC MIPI camera](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/VC_MIPI_Camera_Module_Hardware_Operating_Manual-Dateien/mipi_sensor_front_back.png)

## Version 0.12.3 ([History](VERSION.md))
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
  * [NVIDIA L4T 32.7.1](https://developer.nvidia.com/embedded/linux-tegra-r3271) *(experimental)*
  * [NVIDIA L4T 32.7.2](https://developer.nvidia.com/embedded/linux-tegra-r3272) *(experimental)*
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
  * Image Streaming in GREY, Y10, Y12, SRGGB8, SRGGB10, SRGGB12, SGBRG8, SGBRG10, SGBRG12 format
  * **Exposure** can be set via V4L2 control 'exposure'
  * **Gain** can be set via V4L2 control 'gain'
  * **[Trigger mode](doc/TRIGGER_MODE.md)** '0: disabled', '1: external', '2: pulsewidth', '3: self', '4: single', '5: sync', '6: stream_edge', '7: stream_level' can be set via device tree or V4L2 control 'trigger_mode'
    * **Software trigger** can be executed by V4L2 control 'single_trigger'
  * **[IO mode](doc/IO_MODE.md)** '0: disabled', '1: flash active high', '2: flash active low', '3: trigger active low', '4: trigger active low and flash active high', '5: trigger and flash active low' can be set via device tree or V4L2 control 'flash_mode'
  * **Frame rate** can be set via V4L2 control 'frame_rate' *(except IMX412 and OV9281)*
  * **Black level** can be set via V4L2 control 'black_level' *(except IMX290, IMX327, IMX412, IMX415, OV7251 and OV9281)*
  * **[ROI cropping](doc/ROI_CROPPING.md)** can be set via device tree properties active_l, active_t, active_w and active_h or v4l2-ctl.

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

## GStreamer Support
If you want to use GStreamer with nvarguscamerasrc it is essential to adjust some properties in the device tree. To do that follow the instructions in this section. For each camera there is a mode0 node in the device tree. There is an additional comment in this node to mark the properties that you need to customize. In the tables below you will find the specific values for each camera. 

The value of the property *pixel_t* lists the supported pixel formats. Here you have to choose one out of the following table.

| pixel_t | value  RAW08 | value  RAW10 | value  RAW12 |
| ------- | ------------ | ------------ | ------------ |
| RGGB    | bayer_rggb8  | bayer_rggb   | bayer_rggb12 |
| GBRG    | bayer_gbrg8  | bayer_gbrg   | bayer_gbrg12 |

The property *max_framerate* is given for the number of lanes and the pixel format. For example, 4L10 stands for 4 lanes and the pixel format RAW10. Always set the *def_framerate* to the same value as *max_framerate*

<details>
  <summary>GStreamer properties for IMX296, IMX297, OV7281 (cameras with 1 lane support only)</summary>

| Property             | IMX296     | IMX297     | OV7281     |
| -------------------- | ---------: | ---------: | ---------: |
| physical_w           |      4.968 |      4.968 |      1.920 |
| physical_h           |      3.726 |      3.726 |      1.440 |
| active_w             |       1440 |        720 |        640 |
| active_h             |       1080 |        540 |        480 |
| pixel_t              |      RG 10 |      RG 10 |    RG 8,10 |
| max_gain_val         |         48 |         48 |         18 |
| step_gain_val        |      0.100 |      0.100 |      0.018 |
| max_framerate (1L08) |          - |          - |      104.0 |
| max_framerate (1L10) |       60.3 |       60.3 |      104.0 |
| max_framerate (1L12) |          - |          - |          - |
</details>

<details>
  <summary>GStreamer properties for IMX264, IMX265, OV9281 (cameras with 2 lanes support only)</summary>

| Property             | IMX264     | IMX265     | OV9281     |
| -------------------- | ---------: | ---------: | ---------: |
| physical_w           |      8.390 |      7.065 |      3.840 |
| physical_h           |      7.066 |      5.299 |      2.400 |
| active_w             |       2432 |       2048 |       1280 |
| active_h             |       2048 |       1536 |        800 |
| pixel_t              | RG 8,10,12 | RG 8,10,12 |    RG 8,10 |
| max_gain_val         |         48 |         48 |         12 |
| step_gain_val        |      0.100 |      0.100 |      0.050 |
| max_framerate (2L08) |       35.5 |       55.3 |      120.6 |
| max_framerate (2L10) |       35.5 |       55.3 |      120.6 |
| max_framerate (2L12) |       35.5 |       55.3 |          - |
</details>

<details>
  <summary>GStreamer properties for IMX178, IMX183, IMX226 (cameras with 2 and 4 lanes support)</summary>

| Property             | IMX178     | IMX183     | IMX226     |
| -------------------- | ---------: | ---------: | ---------: |
| physical_w           |      7.430 |     13.305 |      7.533 |
| physical_h           |      4.992 |      8.865 |      5.635 |
| active_w             |       3072 |       5440 |       3904 |
| active_h             |       2048 |       3648 |       3000 |
| pixel_t              | RG 8,10,12 | RG 8,10,12 | GB 8,10,12 |
| max_gain_val         |         48 |         27 |         27 |
| step_gain_val        |      0.100 |      0.026 |      0.014 |
| max_framerate (2L08) |       51.3 |       13.4 |       21.8 |
| max_framerate (2L10) |       41.6 |       13.4 |       21.8 |
| max_framerate (2L12) |       35.4 |       11.2 |       18.1 |
| max_framerate (4L08) |       58.2 |       26.8 |       43.6 |
| max_framerate (4L10) |       58.2 |       26.8 |       43.6 |
| max_framerate (4L12) |       51.3 |       22.4 |       36.3 |
</details>

<details>
  <summary>GStreamer properties for IMX250, IMX252, IMX273, IMX392 (cameras with 2 and 4 lanes support)</summary>

| Property             | IMX250     | IMX252     | IMX273     | IMX392     |
| -------------------- | ---------: | ---------: | ---------: | ---------: |
| physical_w           |      8.446 |      7.066 |      4.970 |      6.679 |
| physical_h           |      7.066 |      5.299 |      3.726 |      4.195 |
| active_w             |       2432 |       2048 |       1440 |       1920 |
| active_h             |       2048 |       1536 |       1080 |       1200 |
| pixel_t              | RG 8,10,12 | RG 8,10,12 | RG 8,10,12 | RG 8,10,12 |
| max_gain_val         |         48 |         48 |         48 |         48 |
| step_gain_val        |      0.100 |      0.100 |      0.100 |      0.100 |
| max_framerate (2L08) |       65.7 |      102.0 |      195.6 |      132.4 |
| max_framerate (2L10) |       53.7 |       83.8 |      156.5 |      111.9 |
| max_framerate (2L12) |       45.5 |       69.8 |      136.9 |       95.0 |
| max_framerate (4L08) |      101.3 |      151.4 |      276.0 |      201.7 |
| max_framerate (4L10) |       82.5 |      123.5 |      226.5 |      167.0 |
| max_framerate (4L12) |       69.5 |      105.7 |      165.9 |      134.4 |
</details>

<details>
  <summary>GStreamer properties for IMX290, IMX327, IMX335, IMX412, IMX415, IMX568 (cameras with 2 and 4 lanes support)</summary>

| Property             | IMX290/327 | IMX335     | IMX412     | IMX415     | IMX568     |
| -------------------- | ---------: | ---------: | ---------: | ---------: | ---------: |
| physical_w           |      5.617 |      5.120 |      6.287 |      5.602 |      6.773 |
| physical_h           |      3.181 |      3.928 |      4.712 |      3.155 |      5.655 |
| active_w             |       1920 |       2560 |       4032 |       3840 |       2472 |
| active_h             |       1080 |       1944 |       3040 |       2160 |       2048 |
| pixel_t              |      RG 10 |   RG 10,12 |      RG 10 |      GB 10 | RG 8,10,12 |
| max_gain_val         |         71 |         72 |         51 |         72 |         48 |
| step_gain_val        |      0.300 |      0.300 |      0.050 |      0.300 |      0.100 |
| max_framerate (2L08) |          - |          - |          - |          - |       49.8 |
| max_framerate (2L10) |       60.0 |       15.0 |       20.0 |       31.7 |       41.3 |
| max_framerate (2L12) |          - |       15.0 |          - |          - |       34.6 |
| max_framerate (4L08) |          - |          - |          - |          - |       96.2 |
| max_framerate (4L10) |       60.0 |       22.3 |       40.0 |       59.9 |       78.8 |
| max_framerate (4L12) |          - |       22.3 |          - |          - |       66.7 |
</details>

### Example
As an example the device tree for the IMX226 with 4 lanes and pixel format RAW10 is shown on the code snippet. Be aware of that the property values for gain are given in mbB [:)] and the frame rate in mHz. So, you have to multiply the values from the table with 1000.
```
  ...
  // ----------------------------------------------------
  // If you want to use GStreamer with nvarguscamerasrc
  // you have to adjust this settings
  physical_w              = "7.533";
  physical_h              = "5.635";
  // ----------------------------------------------------

  // This node is needed by the Tegra framework.
  // You don't have to change any settings if just want to use the V4L API.
  mode0 {
      ...

      // ----------------------------------------------------
      // If you want to use GStreamer with nvarguscamerasrc
      // you have to adjust this settings. 
      active_l                 = "0";
      active_t                 = "0";
      active_w                 = "3904";
      active_h                 = "3000";
      pixel_t                  = "bayer_gbrg";

      min_gain_val             = "0";         //     0.0 dB
      max_gain_val             = "27000";     //    27.0 dB
      step_gain_val            = "14";        //   0.014 dB
      default_gain             = "0";         //     0.0 dB
      
      min_exp_time             = "1";         //       1 us
      max_exp_time             = "1000000";   // 1000000 us
      step_exp_time            = "1";         //       1 us
      default_exp_time         = "10000";     //   10000 us

      min_framerate            = "0";         //       0 Hz
      max_framerate            = "43600";     //    43.6 Hz
      step_framerate           = "100";       //     0.1 Hz
      default_framerate        = "43600";     //    43.6 Hz
      // ----------------------------------------------------
      ...
```

## Device Tree File

If you want to change some settings of a camera in the device tree, please follow these steps.

1. Edit the device tree file for your hardware setup. Currently there are six device tree files for six different combinations of SoMs and carrier boards.
   | system on module | carrier board | device tree file |
   | ---------------- | ------------- | ---------------- |
   | NVIDIA Jetson Nano | NVIDIA Jetson Nano Developer Kit | src/devicetree/NV_DevKit_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Nano | Auvidea JNX30-LC-PD | src/devicetree/Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | NVIDIA Jetson Xavier NX Developer Kit | src/devicetree/NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | Auvidea JNX30-LC-PD | src/devicetree/Auvidea_JNX30_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson AGX Xavier | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_AGXXavier/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson TX2 | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_TX2/tegra186-camera-vc-mipi-cam.dtsi |
   
   To edit the correct device tree file you can simply use the setup script. It will open the correct device tree file in the nano editor.
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

1. Apply all patches in the folder kernel_common_32.3.1+ and the patches listed in the following table that match your hardware setup
   
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
