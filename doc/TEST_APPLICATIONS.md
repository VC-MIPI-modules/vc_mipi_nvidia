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
or the documentation on [v4l2-test](https://github.com/pmliquify/v4l2-test)

## ISP streaming

### gstreamer
With the gst-launch application (gstreamer) you have got the possibility to use the nvarguscamerasrc binary as a streaming source. On most L4T versions it is already deployed. With JetPack 6 (L4T 36.2.0 and upwards) it must be installed separately. Therefor you can use the following script from the target folder: <br>
<pre>
./setup_nvidia.sh
</pre>
For more information on options, please see
<pre>
gst-inspect-1.0 nvarguscamerasrc
</pre>
or the sub section [gstreamer examples](#gstreamer-examples)

### argus_camera
The argus_camera application is part of the nvidia-l4t-jetson-multimedia-api package from NVIDIA. This application provides a gui for changing parameters while streaming in a convenient way.<br>
It will be automatically fetched, built and installed with:
<pre>
./setup_nvidia.sh
</pre>
After the install routine has finished, you can call the program on the shell, directly on the target:
<pre>
argus_camera
</pre>
For more information on usage and options, please see
<pre>
argus_camera --help
</pre>
or have a look at the /usr/src/jetson_multimedia_api/argus/apps/camera/README.TXT

## Examples

### v4l2-ctl examples

Getting all properties for the first sensor device:
<pre>
v4l2-ctl -d /dev/video0 -l
</pre>

Streaming with the first video device and the parameters which are already set in the v4l2 properties:
<pre>
v4l2-ctl -d /dev/video0 --set-ctrl bypass_mode=0 --stream-mmap
</pre>

Streaming with the first video device and the maximum possible frame rate:
<pre>
v4l2-ctl -d /dev/video0 --set-ctrl frame_rate=0 --stream-mmap
</pre>

Streaming with the first video device and a width of 4032, a height of 3040 and the format of 10 bit (IMX412 RG-Bayerpattern):
<pre>
v4l2-ctl -d /dev/video0 --set-fmt-video=width=4032,height=3040,pixelformat=RG10 --stream-mmap
</pre>

Streaming with the first video device and a width of 3840, a height of 2160 and the format of 10 bit (IMX415 GB-Bayerpattern):
<pre>
v4l2-ctl -d /dev/video0 --set-fmt-video=width=3840,height=2160,pixelformat=GB10 --stream-mmap
</pre>

Streaming with the first video device and a width of 2048, a height of 1536 and the format of 8 bit (IMX900 RG-Bayerpattern):
<pre>
v4l2-ctl -d /dev/video0 --set-fmt-video=width=2048,height=1536,pixelformat=RGGB --stream-mmap
</pre>

Streaming with the first video device and a width of 3904, a height of 3000 and the format of 8 bit (IMX226 GB-Bayerpattern):
<pre>
v4l2-ctl -d /dev/video0 --set-fmt-video=width=3904,height=3000,pixelformat=GBRG --stream-mmap
</pre>

Streaming with the first video device, acquire five frames and write it to a file:
<pre>
v4l2-ctl -d /dev/video0 --stream-mmap --stream-count=5 --stream-to=file_5_frames.raw
</pre>

### gstreamer examples

Streaming with the first video device at 20 Hz, using the first sensor mode (mode0 in the device-tree):
<pre>
gst-launch-1.0 nvarguscamerasrc sensor-id=0 sensor-mode=0 ! 'video/x-raw(memory:NVMM),framerate=20/1,width=4032,height=3040,format=NV12' ! nvvidconv ! queue ! fpsdisplaysink video-sink=xvimagesink text-overlay=true
</pre>

Streaming with the first video device at 78 Hz, using the second sensor mode (mode1 in the device-tree, binning_mode=2):
<pre>
gst-launch-1.0 nvarguscamerasrc sensor-id=0 sensor-mode=1 ! 'video/x-raw(memory:NVMM),framerate=78/1,width=2016,height=1520,format=NV12' ! nvvidconv ! queue ! fpsdisplaysink video-sink=xvimagesink text-overlay=true
</pre>

Streaming with sensor mode0, writing down 5 jpeg encoded image files:
<pre>
gst-launch-1.0 nvarguscamerasrc sensor-id=0 sensor-mode=0 num-buffers=5 ! 'video/x-raw(memory:NVMM), framerate=20/1, width=4032, height=3040, format=NV12' ! nvvidconv ! jpegenc ! multifilesink location=~/$(date +%s)-%d.jpg
</pre>

Streaming with sensor mode0, writing down 5 png encoded image files:
<pre>
gst-launch-1.0 nvarguscamerasrc sensor-id=0 sensor-mode=0 num-buffers=5 ! 'video/x-raw(memory:NVMM), framerate=20/1, width=4032, height=3040, format=NV12' ! nvvidconv ! pngenc ! multifilesink location=~/$(date +%s)-%d.png
</pre>

> If there is only mode0 in the device-tree defined, the sensor-mode=\<value\> property can be left out.

> The nvarguscamerasrc is performing additional checks regarding frame rate and frame size. If there are multiple modes in the device-tree defined and the sensor-mode=\<value\> argument is left out, then the nvarguscamerasrc might preselect a different mode!

> The width and the height property of the command line argument are only for the output window, not the frame size, which is set up in the device-tree. The frame size(width/height) is taken from the parameters of the given device-tree mode or the preselected device-tree mode by nvarguscamerasrc.
