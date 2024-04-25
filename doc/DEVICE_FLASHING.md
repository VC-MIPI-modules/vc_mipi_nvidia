# Flashing the device

## Initial Flash
In order to use your selected Linux4Tegra distribution, it must be flashed to the target. <br>
The call
```
./flash.sh -a
```
or
```
./flash.sh --all
```
will perform the initial flash of the target device.

If the quickstart script has been executed, the `flash.sh` script is called automatically after the build process.<br>
Simplified structure of the quickstart script:
```
  quickstart.sh
    ./setup.sh -o
    ./build.sh -a
    ./flash.sh -a
```

If you have set up your system without the `quickstart.sh` script, then you must call the initial flash procedure manually after the build process.

For the initial flash, the target must be in forced recovery mode. <br>
[Quick Start Guide L4T 35.3.1](https://docs.nvidia.com/jetson/archives/r35.3.1/DeveloperGuide/text/IN/QuickStart.html) (NVIDIA Jetson Orin Nano, Orin NX, Xavier NX and AGX Xavier) <br>
[Quick Start Guide L4T 32.7.3](https://docs.nvidia.com/jetson/archives/l4t-archived/l4t-3273/index.html#page/Tegra%20Linux%20Driver%20Package%20Development%20Guide/quick_start.html) (NVIDIA Jetson Nano, TX2, Xavier NX and AGX Xavier)

## Device Tree Flash

If your system has been successfully flashed and you want to modify your camera settings (e.g. number of lanes), then you should perform the following steps:<br>
<b>1. change the camera settings</b>
```
./setup.sh -c
```
or
```
./setup.sh --camera
```
<b>2. compile the device tree</b>
```
./build.sh -d
```
or
```
./build.sh --dt
```
<b>3. flash the device tree</b>
```
./flash.sh -d
```
or
```
./flash.sh --dt
```
For flashing the device tree, 


## Recovery mode


## JetPack 5 (L4T 35.1.0 and upward)


## JetPack 6 (L4T 36.2.0)


