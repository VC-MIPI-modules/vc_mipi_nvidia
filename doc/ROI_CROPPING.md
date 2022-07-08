# ROI cropping

Depending on how you use the camera, you have different ways to use ROI cropping. When using GStreamer with the nvarguscamerasrc you need to set the ROI by defining it in the device tree file. If you use the camera module via V4L, you can define the ROI at runtime by the corresponding V4L commands.

## GStreamer with nvarguscamerasrc

In the case you are using GStreamer with nvarguscamerasrc you need to set the ROI by defining it the mode0 node of the device tree file. This is necessary because nvarguscamerasrc needs active_w and active_h to configure the ISP pipeline. 

>**NOTE**: Because of constraints of the nvargus ISP pipeline 
* the width of the image has to by divisible by 32 and
* the height has to be divisible by 4.

Define the ROI in the mode0 node:
```
        ...
        mode0 {
                ...
                active_l = "360";       // left
                active_t = "270";       // top
                active_w = "704";       // width
                active_h = "540";       // height
                ...
        };
```

## V4L

If you want to use the camera via V4L you can change the ROI on runtime. 

> **NOTE**: You can leave the mode0 values active_l, active_t, active_w and active_h zero. You don't have to set any values in the device tree file.

> **NOTE**: Don't change the ROI on runtime with the following commands and use GStreamer with nvarguscamerasrc. The pipeline will not work.

On a command line you can change the ROI by:
```
$ v4l2-ctl --set-fmt-video=width=704,height=540
```
If you want the set the ROI cropping position additonaly use:
```
$ v4l2-ctl --set-selection=left=360,top=270,width=704,height=540
```