# Frame Rate

The frame rate is always given in mHz, e.g.: 20000 mHz corresponds to 20 Hz.<br>
It can basically be adjusted with the shell command
```
v4l2-ctl -d /dev/video0 -c frame_rate=<value>
```
or
```
v4l2-ctl -d /dev/video0 --set-ctrl frame_rate=<value>
```
A frame rate value of 0 means the maximum frame rate the sensor is capable of in the given mode.
The frame rate can also be modified during a streaming operation, no matter if streaming via v4l2 or ISP.

When streaming with ISP applications (nvarguscamerasrc or argus_camera), some values which are in touch of the frame rate must be setup in the device-tree first.
Hence there are the following values:
<pre>
      min_framerate            = "1000";      //       1 Hz
      max_framerate            = "43600";     //    43.6 Hz
      step_framerate           = "100";       //     0.1 Hz
      default_framerate        = "43600";     //    43.6 Hz
      ...
      framerate_factor         = "1000";
</pre> 
The min_framerate parameter determines the lowest possible frame rate for the streaming. This value must not be too small, since the tegra framework would run in a timeout, because of the frame period, which would be too long until the next frame is being obtained.<br>
The max_framerate parameter defines the maximum possible frame rate for a streaming mode. It must not exceed the value of the given sensor in a certain mode. E.g. when a sensor is not roi cropped or has no binning option set, the maximum values from the [Readme GStreamer Support](/README.md#gstreamer-support) should be used. If you go for a higher value of the maximum frame rate, you could either reduce the frame height ([ROI cropping](/doc/ROI_CROPPING.md)). The column "frame rate increase" will show you, whether your sensor can be driven faster with reduced frame heights.<br>
Or you can try to use the binning mode, if your sensor supports this. Have also a look at [Binning mode](/doc/BINNING_MODE.md).<br>
The step_framerate parameter is used for increasing or decreasing the frame rate by a certain amount by the ISP.<br>
The autoexposure feature of the ISP will use these values in order to calculate frame period times to set the exposure values within this range.<br>
The default_framerate belongs to the v4l control structure and must be set, too. It should be less or equal than the max_framerate parameter. It will be internally set to 0 by the driver, which means maximum frame rate.<br>
> Please be aware, that the nvarguscamerasrc application won't use the default_framerate value. If you omit the framerate specification in your gstreamer call, the nvarguscamerasrc will use an own default value of 30 Hz, no matter what is set in the device-tree.<br>

The framerate_factor is used for the purpose of conversion between floating and fixed point data types. E.g.:
<pre>
      max_framerate            = "43600";
      framerate_factor         = "1000";
</pre>
or
<pre>
      max_framerate            = "43600000";
      framerate_factor         = "1000000";
</pre>
Both notations will end up in a maximum frame rate of 43.6 Hz.

Streaming via v4l2 applications won't use the min, max, step and default framerate parameter from the device-tree entries mentioned above. It uses the value from the v4l2 frame_rate property, which can be set up by the streaming call, too:

<pre>
v4l2-ctl --stream-mmap --set-ctrl=<b>frame_rate=20000</b>
</pre>

<pre>
./v4l2-test stream -d /dev/video0 <b>-r 20000</b> -e 5000 -g 0 -p 1 -n 10
</pre>



