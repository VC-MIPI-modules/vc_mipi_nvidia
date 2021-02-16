# Setup NVIDIA Jetson Jetpack development enviroment from Scratch 

## Setup your host machine

1. Recommended OS is Ubuntu 16.04 LTS or 18.04 LTS.
2. If you want to use Docker Desktop.
    * Download and install [Docker Desktop](https://www.docker.com/products/docker-desktop) for Windows or Mac.
    * Open a terminal and start an ubuntu 18.04 LTS container. The following command will download the Ubuntu 18.04 LTS image an start a bash shell.
    
            docker run --name JDE_JP4.4.1 -v $PWD:/home/user -it ubuntu:18.04 /bin/bash
            
        You should see the linux command prompt now. 

3. Install following packages, which are essential für the NVIDIA toolchain.

        apt update
        apt install -y built-essential python2.7 gemu-user-static sshpass

4. If you are using Docker you have to install some extra packages.

        apt install -y wget bc xxd

    * The NVIDIA toolchain uses the plain python command. Create a symbolic link to python2.7.

            ln -s /usr/bin/python2.7 /usr/bin/python

## Setup NVIDIA toolchain and Jetpack Source Code

1. We have prepared a ready to use script to download and extract all necessary tools, files and Source Codes for the Jetpack 4.4.1 release. Just execute the following command.

        cd bin
        ./setup_JP4.4.1.sh
        
    The script creates a `tmp` directory and downloads all archives. Then it creates a second directory named `build` an extracts the gcc linaro toolchain in `build/l4t_gcc` and the complete Jetpack Sources in `build/Linux_for_Tegra`.