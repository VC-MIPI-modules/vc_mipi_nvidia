# Binning mode

With the following v4l2 control the binning mode of the sensor can be set
```
v4l2-ctl -c binning_mode=<index>
```
The value determines the mode and depends on the capabilities of the sensor. There are predefined value pairs for the horizontal and the vertical binning factors, which are addressed by an index. <br>
E.g. IMX412:
```
v4l2-ctl -c binning_mode=1
```
means horizontal binning is 1, vertical binning is 2 (1x2)
The predefined value pairs can be seen in the vc_init_ctrl_xxx function of the sensor. In that case vc_init_ctrl_imx412(...)<br>
When setting a binning mode, the ROI must be adjusted accordingly.


The table below should give an overview about the maximal resolutions of the sensor's binning modes:

| sensor module | binning_mode <br> index | binning factors <br>(horizontal x vertical) | maximal resolution <br> (width x height) | comment |
| ------------- | ------------------ | --------------- | ----------- | ----------------- |
| IMX412        | 0                  |      0 x 0      | 4032 x 3040 | no binning at all |
|               | 1                  |      1 x 2      | 4032 x 1520 | 2 pixels vertical |
|               | 2                  |      2 x 2      | 2016 x 1520 | 2 pixels horizontal <br> 2 pixels vertical |
|               | 3                  |      4 x 2      |  992 x 1520 | 4 pixels horizontal <br> 2 pixels vertical |
|               | 4                  |      8 x 2      |  480 x 1520 | 8 pixels horizontal <br> 2 pixels vertical |
|               | 5                  |     16 x 2      |  224 x 1520 | 16 pixels horizontal <br> 2 pixels vertical <br>  extended binning |
| IMX565        | 0                  |      0 x 0      | 4128 x 3000 | no binning at all |
|               | 1                  |      2 x 2      | 2048 x 1504 | 2 pixels horizontal <br> 2 pixels vertical |
| IMX566        | 0                  |      0 x 0      | 2848 x 2848 | no binning at all |
|               | 1                  |      2 x 2      | 1408 x 1408 | 2 pixels horizontal <br> 2 pixels vertical |
| IMX567/568    | 0                  |      0 x 0      | 2464 x 2064 | no binning at all |
|               | 1                  |      2 x 2      | 1216 x 1032 | 2 pixels horizontal <br> 2 pixels vertical |

It is also possible to set a ROI in combination with the binning modes. The width must be a multiple of 32 pixels and the height must be a multiple of 8. Both values must be less than the maximal values given in the table above. Please see also **[ROI cropping](ROI_CROPPING.md)**

When binning with Pregius S (IMX56x), the sensor is getting monochrome.

## Examples
### V4L
```
$ v4l2-ctl --set-fmt-video=width=4032,height=3040 -c binning_mode=0
```
> IMX412 without binning.

<br>

```
$ v4l2-ctl --set-fmt-video=width=2016,height=1520 -c binning_mode=2
```
> IMX412 with 2x2 binning.

<br>

```
v4l2-ctl --set-selection=left=0,top=0,width=4128,height=3000 -c binning_mode=0
```
> IMX565 without binning.

<br>

```
v4l2-ctl --set-selection=left=0,top=0,width=2048,height=1504 -c binning_mode=1
```
> IMX565 with 2x2 binning.

<br>

```
v4l2-ctl --set-selection=left=128,top=0,width=1920,height=1504 -c binning_mode=1
```
>IMX565 with 2x2 binning, indented by 128 pixel, the width is set accordingly. The indentation of 128 pixel is substracted from the maximum width for the binning mode.

<br>

### GStreamer with nvarguscamerasrc or argus_camera application

You can define the binning_mode property in the device tree and also use it within multiple modes. Therefore the use_sensor_mode_id must be set to true, too.

```
        {
        ...
        use_sensor_mode_id      = "true";
        ...
                mode0 {
                        ...
                        binning_mode            = "0";
                        active_l                = "0";       // left
                        active_t                = "0";       // top
                        active_w                = "4032";    // width
                        active_h                = "3040";    // height
                        ...
                };

                mode1 {
                        ...
                        binning_mode            = "2";
                        active_l                = "0";       // left
                        active_t                = "0";       // top
                        active_w                = "2016";    // width
                        active_h                = "1520";    // height
                        ...
                };
        }
```
> Please be aware if the binning_mode property is set in the device tree for one or more of the modes, then the v4l binning_mode control won't have an effect for that mode(s) anylonger.

> The minimum sizes for the argus pipe is 256x256 pixel. That means, the following configuration will lead to a crash of the argus daemon:
```
        mode0 {
                ...
                binning_mode            = "5";
                active_l                = "0";       // left
                active_t                = "0";       // top
                active_w                = "224";     // width
                active_h                = "1520";    // height
                ...
        };
```

There is also the option to use the sensor_mode property. When there is more than one mode defined in the device tree, then the sensor_mode=\<index\> can be set for the streaming
<br>
<pre>
gst-launch-1.0 nvarguscamerasrc sensor-id=0 <b>sensor-mode=2</b> ! 'video/x-raw(memory:NVMM), format=(string)NV12, framerate=(fraction)40/1' ! nvvidconv ! queue ! xvimagesink
</pre>
This call demonstrates the usage of the nvargus gestreamer pipe line. The width and height property can be omitted in that case, because these values are read from the device tree.</br>

### Verification of the framesize

If a binning mode is set, the mipi driver will perform a check of the frame size before the streaming starts. If it detects that a given binning mode would exceed the width or height capabilities of the sensor, the streaming would not be executed and an appropriate message will be deployed in the dmesg.</br>
When streaming via gstreamer with nvarguscamerasrc or argus_camera and one of the parameter width, height, binning is misconfigured in the device-tree, the nvargus-daemon might crash.
In that case the streaming application must be cancelled or killed and the nvargus-daemon should be restarted:
<pre>
sudo killall nvargus-daemon
sudo nvargus-daemon &
</pre>
