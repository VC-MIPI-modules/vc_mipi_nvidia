# Test applications

There are two ways of streaming. On the one hand there is v4l2, which gives you the raw image material without post processing. Different pixel resolutions are possible for v4l2 streaming, that are 8, 10, 12 or 14 bits per pixel. Depending on the given sensor module, not all pixel depths are supported. Please have a look at the tables in the [Readme GStreamer Support](/README.md#gstreamer-support) regarding the pixel_t.<br>
The bit values of the pixels might have a bit shift in several setups. Basically, in the 8 bit mode there is no bit shift. One byte per pixel is used to store a pixel value. The pixel formats of 10, 12 and 14 bits will store their data in two bytes. On nearly all soms of NVIDIA there is a bit shift of the pixel data in the two byte array. The only exception is the Jetson Nano, it stores the pixel data right aligned in the two byte array. 
The v4l2 streaming does not compensate this bitshift. That means when you gather some image material via v4l2 for example on the Jetson Orin in the 10 bit mode, the pixel values appear too bright, because the bits of a pixel are not right aligned. In addition to that, on some Jetson soms the upper bit values are mirrored to the least significant bits of the array, too.<br>
> For v4l2 streaming, the bypass_mode property must be set to 0!<br> 
> Otherwise the tegra framework would wait for data from the ISP, which will lead to a timeout.<br>

On the other hand there is the streaming via ISP. This streaming will provide a couple of post processing features and also an auto exposure/gain functionality. It will also compensate the bit shift to a right alignment of the pixel value in the two byte array. The supported pixel depths are 10, 12 and 14 bit (depending on the sensor module).<br>
The streaming via ISP provides also the usage of so called ISP files, which offer additional post processings like color correction or auto white balancing. 
> The bypass_mode will be automatically set to 1 when an ISP streaming call is performed!<br>

## v4l2 applications

### v4l2-ctl
The most common v4l2 application is v4l2-ctl. It can be installed either with
<pre>
sudo apt update
sudo apt-get install -y v4l-utils
</pre>
or by calling the demo script from the target folder
<pre>
./demo.sh
</pre>
For more information, please see
<pre>
v4l2-ctl --help
</pre>
or the sub section [v4l2-ctl examples](#v4l2-ctl-examples)

### v4l2-test
There is also a custom v4l2 application called [v4l2-test](https://github.com/pmliquify/v4l2-test), which can be found on github.<br>
By calling
<pre>
./demo.sh
</pre>
the v4l2-test application will be fetched and build, too.<br>
This application can be run either standalone on a shell or as a client/server app streaming over network.
It is developed continuously and has also an option for compensating the bit shift.
For more information, please see
<pre>
./v4l2-test --help
</pre>
or the sub section [v4l2-test examples](#v4l2-test-examples)

## ISP streaming

### gstreamer
With the gst-launch application (gstreamer) you have got the possibility to use the nvarguscamerasrc binary as a streaming source. On most L4T versions it is already deployed. With JetPack 6 (L4T 36.2.0 and upwards) it must be installed separately. Therefor you can use the following script from the target folder: <br>
<pre>
./setup_nvidia.sh
</pre>
For more information on options, please see
<pre>
gst-inspect-1.0
</pre>
or the sub section [gstreamer examples](#gstreamer-examples)

### argus_camera
The argus_camera application is part of the nvidia-l4t-jetson-multimedia-api package from NVIDIA. This application provides a gui for changing parameters while streaming in a convenient way.<br>
It will be automatically fetched, built and installed with:
<pre>
./setup_nvidia.sh
</pre>
For more information on usage and options, please see
<pre>
argus_camera --help
</pre>
or the sub section [argus_camera examples](#argus_camera-examples)

## Examples

### v4l2-ctl examples

### v4l2-test examples

### gstreamer examples

### argus_camera examples
