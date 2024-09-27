# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano, Xavier NX, AGX Xavier, TX2, Orin Nano and Orin NX

![VC MIPI camera](doc/images/mipi_sensor_front_back.png)

## Version 0.18.1 ([History](VERSION.md))

* Supported system on modules
  * [NVIDIA Jetson Nano 4GB/2GB (production + devkit)](https://developer.nvidia.com/embedded/jetson-nano)
  * [NVIDIA Jetson Xavier NX (production + devkit)](https://developer.nvidia.com/embedded/jetson-xavier-nx)
  * [NVIDIA Jetson AGX Xavier](https://developer.nvidia.com/embedded/jetson-agx-xavier-developer-kit)
  * [NVIDIA Jetson TX2](https://developer.nvidia.com/embedded/jetson-tx2-developer-kit)
  * [NVIDIA Jetson TX2 NX](https://developer.nvidia.com/embedded/jetson-tx2-nx)
  * [NVIDIA Jetson Orin Nano 8GB/4GB](https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nano)
  * [NVIDIA Jetson Orin NX 16GB/8GB](https://developer.nvidia.com/embedded/jetson-modules#jetson_orin_nx)
* Supported carrier boards
  * [NVIDIA Jetson Nano Developer Kit B01](https://developer.nvidia.com/embedded/jetson-nano-developer-kit)
  * [NVIDIA Jetson Nano 2GB Developer Kit](https://developer.nvidia.com/embedded/jetson-nano-2gb-developer-kit)
  * [NVIDIA Jetson Xavier NX Developer Kit](https://developer.nvidia.com/embedded/jetson-xavier-nx-devkit)
  * [NVIDIA Jetson Orin Nano Developer Kit](https://developer.nvidia.com/embedded/learn/get-started-jetson-orin-nano-devkit)
  * [Auvidea JNX30/JNX30D](https://auvidea.eu/product/70879)
  * [Auvidea JNX42 LM](https://auvidea.eu/product/70784) *(only NVIDIA Jetson Nano, Xavier NX, Orin Nano, Orin NX)*
  * [Auvidea J20 on Devkit Jetson AGX Xavier or TX2](https://auvidea.eu/j20/) *(only connector 2+3)*
* Supported board support packages
  * [NVIDIA L4T 32.7.1](https://developer.nvidia.com/embedded/linux-tegra-r3271)
  * [NVIDIA L4T 32.7.2](https://developer.nvidia.com/embedded/linux-tegra-r3272)
  * [NVIDIA L4T 32.7.3](https://developer.nvidia.com/embedded/linux-tegra-r3273)
  * [NVIDIA L4T 32.7.4](https://developer.nvidia.com/embedded/linux-tegra-r3274) *(only NVIDIA Jetson Nano, experimental)*
  * [NVIDIA L4T 35.1.0](https://developer.nvidia.com/embedded/jetson-linux-r351) *(only NVIDIA Jetson Xavier NX and AGX Xavier)*
  * [NVIDIA L4T 35.2.1](https://developer.nvidia.com/embedded/jetson-linux-r3521) *(only NVIDIA Jetson Xavier NX, AGX Xavier and Orin NX, experimental)*
  * [NVIDIA L4T 35.3.1](https://developer.nvidia.com/embedded/jetson-linux-r3531) *(only NVIDIA Jetson Xavier NX, AGX Xavier, Orin NX and Orin Nano, experimental)*
  * [NVIDIA L4T 35.4.1](https://developer.nvidia.com/embedded/jetson-linux-r3541) *(only NVIDIA Jetson Xavier NX, AGX Xavier, Orin NX and Orin Nano, experimental)*
  * [NVIDIA L4T 36.2.0](https://developer.nvidia.com/embedded/jetson-linux-r362) *(only NVIDIA Jetson Orin Nano and Orin NX + Orin Nano DevKit, experimental)*
* Supported [VC MIPI Camera Modules](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/index.html) 
  * IMX178, IMX183, IMX226
  * IMX250, IMX252, IMX264, IMX265, IMX273, IMX392
  * IMX290, IMX327, IMX462
  * IMX296, IMX297
  * IMX335
  * IMX412
  * IMX415
  * IMX565, IMX566, IMX567, IMX568
  * IMX900
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
  * **Frame rate** can be set via V4L2 control 'frame_rate' *(except OV9281)*
  * **[Black level](doc/BLACK_LEVEL.md)** can be set via V4L2 control 'black_level' *(except OV7251 and OV9281)*
  * **[ROI cropping](doc/ROI_CROPPING.md)** can be set via device tree properties active_l, active_t, active_w and active_h or v4l2-ctl.
  * **[Binning mode](doc/BINNING_MODE.md)** can be set via V4L2 control 'binning_mode' *(IMX412, IMX565, IMX566, IMX567, IMX568 only)*

## Prerequisites for cross-compiling

### Host PC

* Recommended OS is Ubuntu 18.04 LTS or Ubuntu 20.04 LTS
* You need git to clone this repository
* All other packages are installed by the scripts contained in this repository

## Quickstart

1. Enter recovery mode by following the instructions in one of the guides:

   [Quick Start Guide L4T 35.3.1](https://docs.nvidia.com/jetson/archives/r35.3.1/DeveloperGuide/text/IN/QuickStart.html) (NVIDIA Jetson Orin Nano, Orin NX, Xavier NX and AGX Xavier)

   [Quick Start Guide L4T 32.7.3](https://docs.nvidia.com/jetson/archives/l4t-archived/l4t-3273/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) (NVIDIA Jetson Nano, TX2, Xavier NX and AGX Xavier)

2. Create a directory and clone the repository.

    ```bash
    cd <working_dir>
    git clone https://github.com/VC-MIPI-modules/vc_mipi_nvidia
    ```

3. Start the quickstart installation process and follow the instructions.

   ```bash
   cd vc_mipi_nvidia/bin
   ./quickstart.sh
   ```

   > If you have changed your hardware setup simply execute this script again.

   > During the setup process of the quickstart.sh script, there will be executed the l4t_create_default_user.sh script from NVIDIA, which will create a default user with the credentials given by the user.
   > * After setting up the toolchain, the user is prompted to this credentials mask.
   > * This script has been available since L4T 32.6.1 and will be applied accordingly.

4. (Optional)

   Set up the target by executing one of the following commands on the host computer when the target device is already running:

   ```bash
   cd vc_mipi_nvidia/bin
   ./setup.sh --target
   ```

   This function will configure a connection with the running target. It will backup the users known_hosts file and copy the users public rsa key, so that every time when a ssh connection to the device is opened, the user don't need to enter the password again. It will also copy the demo.sh and max_speed.sh script into the /home/username/test/ folder on the device.

   For more information about the mentioned scripts, please run the appropriate script with the option "--help" in a shell on the running device.

   ```bash
   vc@nvidia $./test/demo.sh --help
   ```

   or

   ```bash
   vc@nvidia $ ./test/max_speed.sh --help
   ```

   When operating with the max_speed.sh script, it should be run as root.<br> E.g. speeding up the clocks:

   ```bash
   vc@nvidia $ sudo ./test/max_speed.sh --max
   ```

## Changing camera settings in the device tree

### GStreamer Support

If you want to use GStreamer with nvarguscamerasrc it is essential to adjust some properties in the device tree. To do that follow the instructions in this section. For each camera there is a mode0 node in the device tree. There is an additional comment in this node to mark the properties that you need to customize. In the tables below you will find the specific values for each camera.

The value of the property *pixel_t* lists the supported pixel formats. Here you have to choose one out of the following table.

| pixel_t | value  RAW08 | value  RAW10 | value  RAW12 |
| ------- | ------------ | ------------ | ------------ |
| RGGB    | bayer_rggb8  | bayer_rggb   | bayer_rggb12 |
| GBRG    | bayer_gbrg8  | bayer_gbrg   | bayer_gbrg12 |

The property *max_framerate* is given for the number of lanes and the pixel format. For example, 4L10 stands for 4 lanes and the pixel format RAW10. Always set the *def_framerate* to the same value as *max_framerate*

<details>
  <summary>GStreamer properties for IMX296, IMX297, OV7251 (cameras with 1 lane support only)</summary>

| Property             | IMX296     | IMX297     | OV7251     |
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
| physical_w           |      7.373 |     13.056 |      7.222 |
| physical_h           |      4.915 |      8.755 |      5.550 |
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
| physical_w           |      8.390 |      7.066 |      4.968 |      6.624 |
| physical_h           |      7.066 |      5.299 |      3.726 |      4.140 |
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
  <summary>GStreamer properties for IMX290, IMX327, IMX335, IMX412, IMX415 and IMX462 (cameras with 2 and 4 lanes support)</summary>

| Property             | IMX290/327 | IMX335     | IMX412     | IMX415     | IMX462     |
| -------------------- | ---------: | ---------: | ---------: | ---------: | ---------: |
| physical_w           |      5.568 |      5.184 |      6.250 |      5.568 |      5.568 |
| physical_h           |      3.132 |      3.888 |      4.712 |      3.132 |      3.132 |
| active_w             |       1920 |       2592 |       4032 |       3840 |       1920 |
| active_h             |       1080 |       1944 |       3040 |       2160 |       1080 |
| pixel_t              |      RG 10 |   RG 10,12 |      RG 10 |      GB 10 |      RG 10 |
| max_gain_val         |         71 |         72 |         51 |         72 |         71 |
| step_gain_val        |      0.300 |      0.300 |      0.050 |      0.300 |      0.300 |
| max_framerate (2L08) |          - |          - |          - |          - |          - |
| max_framerate (2L10) |       60.0 |       15.0 |       20.0 |       31.7 |       60.0 |
| max_framerate (2L12) |          - |       15.0 |          - |          - |          - |
| max_framerate (4L08) |          - |          - |          - |          - |          - |
| max_framerate (4L10) |       60.0 |       22.3 |       40.0 |       59.9 |      120.0 |
| max_framerate (4L12) |          - |       22.3 |          - |          - |          - |
</details>

<details>
  <summary>GStreamer properties for IMX565, IMX566, IMX567, IMX568 and IMX900 (cameras with 2 and 4 lanes support)</summary>

| Property             | IMX565     | IMX566     | IMX567/568 | IMX900     |
| -------------------- | ---------: | ---------: | ---------: | ---------: |
| physical_w           |     11.311 |      7.804 |      6.752 |      4.608 |
| physical_h           |      8.220 |      7.804 |      5.655 |      3.456 |
| active_w             |       4128 |       2848 |       2464 |       2048 |
| active_h             |       3000 |       2848 |       2064 |       1536 |
| pixel_t              | RG 8,10,12 | RG 8,10,12 | RG 8,10,12 | RG 8,10,12 |
| max_gain_val         |         48 |         48 |         48 |         48 |
| step_gain_val        |      0.100 |      0.100 |      0.100 |      0.100 |
| max_framerate (2L08) |       21.1 |       33.3 |       49.8 |       88.8 |
| max_framerate (2L10) |       17.0 |       26.9 |       41.3 |       72.5 |
| max_framerate (2L12) |       14.2 |       22.6 |       34.6 |       61.3 |
| max_framerate (4L08) |       40.7 |       68.2 |       96.2 |      121.0 |
| max_framerate (4L10) |       34.3 |       51.6 |       78.8 |      112.3 |
| max_framerate (4L12) |       27.8 |       43.6 |       66.7 |       67.0 |

</details>

### Example

As an example the device tree for the IMX226 with 4 lanes and pixel format RAW10 is shown on the code snippet. Be aware of that the property values for gain are given in mdB [:)] and the frame rate in mHz. So, you have to multiply the values from the table with 1000.

```dts
  ...
  // ----------------------------------------------------
  // If you want to use GStreamer with nvarguscamerasrc
  // you have to adjust this settings
  physical_w              = "7.222";
  physical_h              = "5.550";
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
#if LINUX_VERSION < 500
      pixel_t                  = "bayer_gbrg";
#else
      mode_type                = "bayer";
      pixel_phase              = "gbrg";
      csi_pixel_bit_depth      = "10";
#endif

      min_gain_val             = "0";         //     0.0 dB
      max_gain_val             = "27000";     //    27.0 dB
      step_gain_val            = "14";        //   0.014 dB
      default_gain             = "0";         //     0.0 dB
      
      min_exp_time             = "1";         //       1 us
      max_exp_time             = "1000000";   // 1000000 us
      step_exp_time            = "1";         //       1 us
      default_exp_time         = "10000";     //   10000 us

      min_framerate            = "1000";      //       1 Hz
      max_framerate            = "43600";     //    43.6 Hz
      step_framerate           = "100";       //     0.1 Hz
      default_framerate        = "43600";     //    43.6 Hz
      // ----------------------------------------------------
      ...
```

## Device Tree File

If you want to change some settings of a camera in the device tree, please follow these steps.

1. Edit the device tree file for your hardware setup. 
   | system on module | carrier board | device tree file |
   | ---------------- | ------------- | ---------------- |
   | NVIDIA Jetson Nano | NVIDIA Jetson Nano Developer Kit | src/devicetree/NV_DevKit_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Nano | Auvidea JNX30 | src/devicetree/Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Nano | Auvidea JNX42 | src/devicetree/Auvidea_JNX42_Nano/tegra210-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | NVIDIA Jetson Xavier NX Developer Kit | src/devicetree/NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | Auvidea JNX30 | src/devicetree/Auvidea_JNX30_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Xavier NX | Auvidea JNX42 | src/devicetree/Auvidea_JNX42_XavierNX/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson AGX Xavier | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_AGXXavier/tegra194-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson TX2 | Auvidea J20 on DevKit | src/devicetree/Auvidea_J20_TX2/tegra186-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson TX2 NX | Auvidea JNX30D | src/devicetree/Auvidea_JNX30D_TX2NX/tegra186-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Orin Nano | NVIDIA Jetson Orin Nano Developer Kit | src/devicetree/NV_DevKit_OrinNano/tegra234-camera-vc-mipi-cam.dtsi <br> Jetpack 5 (L4T 35.3.1, L4T 35.4.1)|
   | NVIDIA Jetson Orin Nano | NVIDIA Jetson Orin Nano Developer Kit | src/devicetree/NV_DevKit_OrinNano/tegra234-p3767-camera-p3768-vc_mipi-dual.dts <br> Jetpack 6 (L4T 36.2.0 DP)|
   | NVIDIA Jetson Orin Nano | Auvidea JNX42 | src/devicetree/Auvidea_JNX42_OrinNano/tegra234-camera-vc-mipi-cam.dtsi |
   | NVIDIA Jetson Orin NX | Auvidea JNX42 | src/devicetree/Auvidea_JNX42_OrinNX/tegra234-camera-vc-mipi-cam.dtsi |

   To edit the correct device tree file you can simply use the setup script. It will open the correct device tree file in the nano editor.

   ```bash
     ./setup.sh --camera
   ```

2. Enter recovery mode by following the instructions in one of the guides:<br>
[Quick Start Guide L4T 35.3.1](https://docs.nvidia.com/jetson/archives/r35.3.1/DeveloperGuide/text/IN/QuickStart.html) (NVIDIA Jetson Orin Nano, Orin NX, Xavier NX and AGX Xavier) <br>
[Quick Start Guide L4T 32.7.3](https://docs.nvidia.com/jetson/archives/l4t-archived/l4t-3273/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) (NVIDIA Jetson Nano, TX2, Xavier NX and AGX Xavier)

3. Build and flash the device tree files to the target.

   ```bash
     ./build.sh --dt
     ./flash.sh --dt
   ```

   > Regarding to Jetpack5 and higher, please have a look at the section Annotations.<br>
   > **[Flashing the device](doc/DEVICE_FLASHING.md)** 

## Using long exposure times or external trigger mode with long waiting times (> 5 seconds)

If you want to use your camera in an application with long exposure times or external trigger and the time between two consecutively triggers is potentially long (> 5 seconds) it is necessary to adjust the timeout of the csi receiver. In this case please change following line of code.

   | system on module         | line | in file                                                          |
   | ------------------------ | ---- | ---------------------------------------------------------------- |
   | NVIDIA Jetson Nano       |  232 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi2_fops.c |
   | NVIDIA Jetson Xavier NX  |   36 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi5_fops.c |
   | NVIDIA Jetson AGX Xavier |   36 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi5_fops.c |
   | NVIDIA Jetson TX2        | 1097 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi4_fops.c |
   | NVIDIA Jetson TX2 NX     | 1097 | /kernel/nvidia/drivers/media/platform/tegra/camera/vi/vi4_fops.c |

## Tested with VC MIPI Camera Module Revision

* IMX178 (Rev.02), IMX183 (Rev.15), IMX226 (Rev.16), 
* IMX250 (Rev.09), IMX252 (Rev.12), IMX264 (Rev.05), IMX265 (Rev.05), IMX273 (Rev.16), IMX392 (Rev.08)
* IMX290 (Rev.02), IMX327 (Rev.02), IMX462 (Rev.01)
* IMX296 (Rev.43), IMX297 (Rev.43)
* IMX335 (Rev.02)
* IMX412 (Rev.05)
* IMX415 (Rev.02)
* IMX565 (Rev.03), IMX566 (Rev.03), IMX567 (Rev.03), IMX568 (Rev.04)
* IMX900 (Rev.00)
* OV7251 (Rev.01), OV9281 (Rev.02)

You can find the revision of the camera module in the dmesg log.

```bash
dmesg | grep 'i2c'
[...] i2c 6-0010: +--- VC MIPI Camera -----------------------------------+
[...] i2c 6-0010: | MANUF. | Vision Components               MID: 0x0427 |
[...] i2c 6-0010: | MODULE | ID:  0x0183                     REV:   0012 |
[...] i2c 6-0010: | SENSOR | SONY IMX183                                 |
...
```

## Integrate the driver in your own BSP

If you have your own BSP, you have to integrate the driver into it. Please follow these steps.

1. Apply all patches in the folder kernel_common_32.3.1+ and the patches listed in the following table that match your hardware setup

   | system on module         | carrier board  | BSP             | all patches in folder patch/... |
   | ------------------------ | -------------- | --------------- | --------------------- |
   | NVIDIA Jetson Nano       | NVIDIA DevKit  | 32.7.1 - 32.7.3 | kernel_Nano_32.6.1+   |
   |                          |                | 32.7.4          | kernel_Nano_32.6.1+ <br> kernel_Nano_32.7.4 |
   |                          | Auvidea JNX30  | 32.7.1 - 32.7.3 | kernel_Nano_32.6.1+ <br> dt_Auvidea_JNX30_Nano_32.5.0+ |
   |                          |                | 32.7.4          | kernel_Nano_32.6.1+ <br> kernel_Nano_32.7.4 <br> dt_Auvidea_JNX30_Nano_32.5.0+ |
   |                          | Auvidea JNX42  | 32.7.1 - 32.7.3 | kernel_Nano_32.6.1+ <br> dt_Auvidea_JNX30_Nano_32.5.0+ |
   |                          |                | 32.7.4          | kernel_Nano_32.6.1+ <br> kernel_Nano_32.7.4 <br> dt_Auvidea_JNX30_Nano_32.5.0+ |
   | NVIDIA Jetson Xavier NX  | NVIDIA DevKit  | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ |
   |                          |                | 35.1.0          | kernel_Xavier_35.1.0+ |
   |                          |                | 35.2.1          | kernel_Xavier_35.2.1+ |
   |                          |                | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   |                          | Auvidea JNX30  | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ <br> dt_Auvidea_JNX30_XavierNX_32.5.0+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ <br> dt_Auvidea_JNX30_XavierNX_32.5.0+ |
   |                          |                | 35.1.0          | kernel_Xavier_35.1.0+ |
   |                          |                | 35.2.1          | kernel_Xavier_35.2.1+ |
   |                          |                | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   |                          | Auvidea JNX42  | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ <br> dt_Auvidea_JNX30_XavierNX_32.5.0+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ <br> dt_Auvidea_JNX30_XavierNX_32.5.0+ |
   |                          |                | 35.1.0          | kernel_Xavier_35.1.0+ |
   |                          |                | 35.2.1          | kernel_Xavier_35.2.1+ |
   |                          |                | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   | NVIDIA Jetson AGX Xavier | DevKit + J20   | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ |
   |                          |                | 35.1.0          | kernel_Xavier_35.1.0+ |
   |                          |                | 35.2.1          | kernel_Xavier_35.2.1+ |
   |                          |                | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   | NVIDIA Jetson TX2        | DevKit + J20   | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ |
   | NVIDIA Jetson TX2 NX     | Auvidea JNX30D | 32.7.1 - 32.7.2 | kernel_Xavier_32.6.1+ |
   |                          |                | 32.7.3          | kernel_Xavier_32.7.3+ |
   | NVIDIA Jetson Orin Nano  | NVIDIA DevKit  | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   |                          |                | 36.2.0          | kernel_Xavier_36.2.0+ * |
   |                          | Auvidea JNX42  | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
   | NVIDIA Jetson Orin NX    | Auvidea JNX42  | 35.2.1          | kernel_Xavier_35.2.1+ |
   |                          |                | 35.3.1          | kernel_Xavier_35.3.1+ |
   |                          |                | 35.4.1          | kernel_Xavier_35.4.1+ |
*) For L4T 36.2.0, the kernel_common_32.3.1+ must be left out.

2. Copy the camera device tree to the folder listed in the following table

   | system on module         | carrier board  | copy from src/devicetree/... to folder |
   | ------------------------ | -------------- | ---------------------------------- |
   | NVIDIA Jetson Nano       | NVIDIA DevKit  | NV_DevKit_Nano/tegra210-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms |
   |                          | Auvidea JNX30  | Auvidea_JNX30_Nano/tegra210-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms |
   |                          | Auvidea JNX42  | Auvidea_JNX42_Nano/tegra210-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms |
   | NVIDIA Jetson Xavier NX  | NVIDIA DevKit  | NV_DevKit_XavierNX/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/jakku/kernel-dts/common |
   |                          | Auvidea JNX30  | Auvidea_JNX30_XavierNX/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/jakku/kernel-dts/common |
   |                          | Auvidea JNX42  | Auvidea_JNX42_XavierNX/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/jakku/kernel-dts/common |
   | NVIDIA Jetson AGX Xavier | DevKit + J20   | Auvidea_J20_AGXXavier/tegra194-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t19x/common/kernel-dts/t19x-common-modules |
   | NVIDIA Jetson TX2        | DevKit + J20   | Auvidea_J20_TX2/tegra186-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t18x/common/kernel-dts/t18x-common-modules |
   | NVIDIA Jetson TX2 NX     | Auvidea JNX30D | Auvidea_JNX30D_TX2NX/tegra186-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t18x/lanai/kernel-dts/common |
   | NVIDIA Jetson Orin Nano  | NVIDIA DevKit  | NV_DevKit_OrinNano/tegra234-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb <br> (device tree include file used for Jetpack 5) |
   |                          |                | NV_DevKit_OrinNano/tegra234-p3767-camera-p3768-vc_mipi-dual.dts <br> => /hardware/nvidia/t23x/nv-public/overlay <br> (device tree overlay file for Jetpack 6) |
   |                          | Auvidea JNX42  | Auvidea_JNX42_OrinNano/tegra234-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb |
   | NVIDIA Jetson Orin NX    | Auvidea JNX42  | Auvidea_JNX42_OrinNX/tegra234-camera-vc-mipi-cam.dtsi <br> => /hardware/nvidia/platform/t23x/p3768/kernel-dts/cvb |

3. Copy all driver files from folder **src/driver** to **/kernel/nvidia/drivers/media/i2c**

## Testing the camera

To test the camera you can use [v4l2-test](https://github.com/pmliquify/v4l2-test)

## Annotations

### For Jetpack 5 (L4T 35.1.0, 35.2.1, 35.3.1, 35.4.1):

* When the system has booted successfully, it is necessary to run the script max_speed.sh from the /target folder as superuser. It will read out the maximum frequencies and set them as the current ones. This is a recommendation from nvidia.

  ```bash
  sudo ./max_speed.sh --max
  ```

### For Jetpack 6 (L4T 36.2.0 DP):

- with this L4T 36.2 version, the tegra part is completely separated from the kernel
- VC Mipi driver is now running as a couple of kernel modules
- modifications to the camera device tree are realised with device tree overlays

- nvarguscamerasrc+nvvidconv must be installed separately (gst-nvarguscamera_src.tbz2 and gst-nvvidconv_src.tbz2 are residing in the Linux_for_Tegra/source folder and will be copied automatically into the home directory of the target device)
  - the argus_camera sample application (nvidia-l4t-jetson-multimedia-api) can be used alternatively
- a convenience script setup_nvidia.sh can be called on the running jetson to install some prerequisites (an internet connection must be present)
  - this script will install build essentials, nvidia-l4t-jetson-multimedia-api, lib-cuda-dev and v4l-utils
  - the nvarguscamerasrc and nvvidconv will be generated
  - the NVIDIA samples (including argus_camera) will be built and installed

- device tree handling
  - the file tegra234-p3767-camera-p3768-vc_mipi-dual.dts can be edited with `./setup.sh --camera`
  - the tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo file will be generated with `./build.sh --dt` | this step is automatically done by `./build.sh --all`
  - the dtbo will be generated into the kernel/dtb directory on the host pc
  - when flashing the first time with `sudo ./flash.sh --all`, the overlay file will be flashed into uefi
  - to modify the camera-settings via tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo file, the /boot/extlinux/extlinux.conf entry must be modified/duplicated and the <br>
  OVERLAYS /boot/tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
entry must be added. This will override the initial uefi dtbo and modifications can take effect
  - the dtbo file must be copied into the specified location before the restart
```
LABEL secondary
      MENU LABEL secondary kernel
      LINUX /boot/Image
      FDT /boot/dtb/kernel_tegra234-p3768-0000+p3767-0005-nv.dtb
      INITRD /boot/initrd
      APPEND ...

      OVERLAYS /boot/tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
```

   > * this version has been tested with two identical IMX565, IMX567/8, IMX296, IMX226, IMX415 sensors
   > * there might be problems when using only a single sensor


### For NVIDIA Jetson TX2 NX

* Currently the following camera modules do not work with the TX2 NX
  * IMX178, IMX183
  * IMX250, IMX252, IMX264, IMX265, IMX273, IMX392

### Troubleshooting

Regarding to flashing issues, please have a look at **[Flashing the device](doc/DEVICE_FLASHING.md)** 