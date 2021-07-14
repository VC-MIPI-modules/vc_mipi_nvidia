# Vision Components MIPI CSI-2 driver for NVIDIA Jetson Nano
![VC MIPI camera](https://www.vision-components.com/fileadmin/external/documentation/hardware/VC_MIPI_Camera_Module/VC_MIPI_Camera_Module_Hardware_Operating_Manual-Dateien/mipi_sensor_front_back.png)

## Version 0.2.0
* Supported boards
  * NVIDIA Dev Kit B01
  * Auvidea JNX30

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