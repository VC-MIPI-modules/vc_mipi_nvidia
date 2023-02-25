# ROI cropping

Depending on how you use the camera, you have different ways to use ROI cropping. When using GStreamer with the nvarguscamerasrc you need to set the ROI by defining it in the device tree file. On the other hand, if you use the camera module via V4L, you can define the ROI at runtime by the corresponding V4L commands.

The table below shows the ranges in which a camera can be adjusted. 

>**NOTE**: If there are two values in a column. The first value is for RAW08 and the second for RAW10/RAW12 pixelformat.

| <br>camera | rev | width<br>min | <br>max | <br>inc | height<br>min | <br>max | <br>inc | frame rate<br>increase |
| ------ | --- | --------- | --------- | --------- | ---------- | ---------- | ---------- | ---- |
| IMX178 |  01 |  88 / 100 |      3072 |    8 / 16 |         32 |       2048 |          1 |  yes |
| IMX183 |  12 |      5440 |      5440 |         - |         32 |       3648 |          1 |  yes |
| IMX226 |  13 |   56 / 40 |      3904 |    4 / 20 |         32 |       3000 |          1 |  yes |
| IMX250 |  07 |       176 |      2432 |    8 / 16 |         32 |       2048 |          4 |  yes |
| IMX252 |  10 |       176 |      2048 |    8 / 16 |         32 |       1536 |          4 |  yes |
| IMX264 |  03 |        96 |      2432 |        16 |        128 |       2048 |          4 |  yes |
| IMX265 |  01 |        96 |      2048 |        16 |         32 |       1536 |          4 |  yes |
| IMX273 |  13 | 160 / 176 |      1440 |    8 / 16 |         32 |       1080 |          4 |  yes |
|(IMX290)|  02 |      1920 |      1920 |         - |       1080 |       1080 |          - |   no |
| IMX296 |  42 |         4 |      1440 |         4 |         32 |       1080 |          1 |  yes |
| IMX297 |  ?? |         4 |       720 |         4 |         32 |        540 |          1 |  yes |
|(IMX327)|  02 |      1920 |      1920 |         - |       1080 |       1080 |          - |   no |
| IMX335 |  00 |       104 |      2592 |         4 |         32 |       1944 |          1 |  yes |
| IMX392 |  06 | 160 / 176 |      1920 |    8 / 16 |         32 |       1200 |          4 |  yes |
| IMX412 |  02 |         4 |      4032 |         4 |         32 |       3040 |          1 |   no |
| IMX415 |  01 |         4 |      3840 |         4 |         32 |       2160 |          1 |  yes |
| IMX568 |  01 |      2472 |      2472 |         - |        600 |       2048 |          8 |  yes |
| OV7281 |  01 |       156 |       640 |         4 |         32 |        480 |          1 |   no |
| OV9281 |  02 |        32 |      1280 |         4 |         32 |        800 |          1 |   no |

## GStreamer with nvarguscamerasrc

In the case you are using GStreamer with nvarguscamerasrc you need to set the ROI by defining it the mode0 node of the device tree file. This is necessary because nvarguscamerasrc needs active_w and active_h to configure the ISP pipeline. 

Define the ROI in the mode0 node:
```
        ...
        mode0 {
                ...
                active_l = "360";       // left
                active_t = "270";       // top
                active_w = "720";       // width
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
$ v4l2-ctl --set-fmt-video=width=720,height=540
```
If you want the set the ROI cropping position additonaly use:
```
$ v4l2-ctl --set-selection=left=360,top=270,width=720,height=540
```