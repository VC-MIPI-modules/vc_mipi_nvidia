# Docker support

There is a docker environment available for JetPack 4 and JetPack 5 setups. <br>
But only NVIDIA Jetson Nano, Xavier NX, AGX Xavier and TX2 are fully supported by this docker environment. <br>
With fully supported is meant, that the quickstart.sh script can be used for the complete setup/build/flash procedure. <br>

By calling 
```
./run_docker_environment.sh 
```
a docker container is being established. <br>
In this docker container there is also a user predefined with the credentials:<br> 
<pre>
<b>dockervc:dockervc</b>
</pre>
After all updates and components has been installed in the environment, you will be prompted to <br>
<pre>
dockervc@container-id:/src$
</pre>
This will be the entry point for your operations. All files and directories, which are residing in the directory vc_mipi_nvidia <br>
can be modified in the docker environment as well as outside of this environment.<br>
After you have changed into the bin directory: 
<pre>
dockervc@container-id:/src$ cd bin
</pre>
you can run the quickstart.sh script:<br>
<pre>
dockervc@container-id:/src/bin$ ./quickstart.sh
</pre>

The docker container is based on Ubuntu 18.04 and the Dockerfile is located in the docker directory.

## Usage with Orin Soms

It would also be possible to use the docker environment for Orin Soms with JetPack5 as well as JetPack6, but <b>not</b> for flashing the device!<br>
That means the setup.sh and the build.sh script can be used in docker, but not the flash.sh script. In case of NVIDIA Jetson Orins, the flash.sh script <br>
must be called from a non docker shell. 
If you have not flashed any NVIDIA Jetson Orin device from your linux host machine and if you intend to run the setup.sh script in the docker environment, <br>
you should call at least one time the script l4t_flash_prerequisites.sh on your linux host, so that all necessary components are being installed for flashing. <br>
This script becomes available after your setup.sh script has been completed. It will reside in the Linux_for_Tegra/tools folder and is part of the NVIDIA setup. <br>
<pre>
sudo ./l4t_flash_prerequisites.sh
</pre>


