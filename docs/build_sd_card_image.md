# Build a SD Card Image to use a specific MIPI CSI-2 camera module

## Build the SD Card Image
We have developed a script to easily create a SD Card Image. The script execute three steps:

* Patching NVIDIA Kernel Sources with VC MIPI Drivers
* Build the Kernel Image and Device Tree files
* Create the SD Card Image

Just execute following script

    $ cd vc_mipi_driver/bi0n

    # Example 1: Jetpack 4.3 and Omnivision 9281 camera
    $ ./create_sd_card_image.sh 43 OV9281   

    # Example 2: Jetpack 4.3 and IMX 327C camera
    $ ./create_sd_card_image.sh 441 IMX327C

The build process and creation of the SD Card Image will take a while. If everything went well you will find the SD Card Image in the disc_images folder.


    vc_mipi_driver
    |-- bin
    |-- docs
    |-- jp4.3    
    |    |-- disc_images                     # Contains all created SD Card Images
    |    | ...
    |-- src
    +-- test

In the next step you will prepare your Developer Kit and create a SD Card with your freshly created Image.