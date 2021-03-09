# Getting Started

This documentation will guide you to build a ready to use SD Card Image file to get your Vision Components MIPI CSI-2 Camera up an running in just a few steps. Currently we support NVIDIA Jetpack Version 4.3 and 4.4.1

## Setup your Host Machine
First of all you need a Linux Host System to build the Linux Kernel Image. 
NVIDIA recommends to run Ubuntu 16.04 LTS or 18.04 LTS.

## Download Build Scripts

    $ git clone https://github.com/pmliquify/vc_mipi_driver.git

## Setup NVIDIA toolchain and Jetpack Source Code

---
**INFO** The Setup Scripts will install nessecary build tool packages  
(build-essential, python2.7 python-to-python2, qemu-user-static)  
download and extract the Toolchain and all files of the Jetpack BSP and the Kernel Sources.
---

    $ cd vc_mipi_driver/bin
    $ ./setup.sh 43         # Example 1: If you want to use Jetpack 4.3
    $ ./setup.sh 441        # Example 2: If you want to use Jetpack 4.4.1

---

**CHECK** if everything went well. You will get an additional directory jp4.3 or jp4.4.1. Check if all folders are there.

---

    vc_mipi_driver
    |-- bin
    |-- docs
    |-- jp4.3                               # Tools and sources for Jetpack 4.3
    |    |-- downloads                      # Downloaded archives
    |    |-- Linux_for_Tegra
    |    |    | ...                         # Board support package
    |    |    |-- rootfs                     
    |    |    |    | ...                    # Sample root file system
    |    |    +-- source/public/kernel/kernel-4.9
    |    |         | ...                    # Kernel sources
    |    +-- toolchain
    |         +-- gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu
    |              | ...                    # Cross compiler and other build tools
    |-- src
    +-- test

In the next step you will create the SD Card Image.