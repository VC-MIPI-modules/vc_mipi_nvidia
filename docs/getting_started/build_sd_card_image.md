# Build a SD Card Image to use a specific MIPI CSI-2 camera module

## Select the Driver
First of all you have to activate the driver for your VC MIPI CSI-2 camera module you want to test.

1. Edit the file `.../src/hardware/nvidia/platform/t210/porg/kernel-dts/tegra210-porg-p3448-common.dtsi` with your prefered Editor.
2. Activate the driver by set a 1 after the corresponding variable.

        // Enable one driver only !
        #define VC_MIPI_OV9281      1   /* CCC - Enable VC MIPI OV9281 driver                */ 
        #define VC_MIPI_IMX183      0   /* CCC - Enable VC MIPI IMX183 driver (mono)         */

    As an example here, we have chosen the OV9281 driver. Pay attention that all other variables are set to 0!

## Build the SD Card Image
We have prepared a Docker Image based on the Ubuntu 18.04 Image. All neccesary Linux packages, the Jetpack Toolchain an the complete Jetpack Sources are already installed and precomiled.

To start the Docker Container execute following command in a terminal

        cd <your dir>/vc_mipi_nano_b01_32.4.4
        ./create_sd_card_image.sh

