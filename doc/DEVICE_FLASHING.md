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

## Flashing issues

If you run into the problem that the flash procedure does not succeed, the first thing to check is the integrity of the USB cable or the usage of a different USB port on the host system, if possible.<br>
### Orin devices
Especially with Orin devices there are some difficulties to expect, like timeouts, too less space or disconnections.
1. When flashing an Orin device, your memory card/nvme card should have at least 64 Gb.

2. You should check, whether the udisks2 service is running on your host:
   ```
   service --status-all
   ```
   If the udisks2 service is up, then you should try to deactivate it temporarily. Please ensure, that you have detached other drives in order to prevent data losses.
   ```
   systemctl stop udisks2.service
   ```
3. You should check, whether the USB autosuspend is activated:
   ```
   cat /sys/module/usbcore/parameters/autosuspend
   ```
   If the result is 2, then you should deactivate the auto suspend feature:
   ```
   sudo -s
   echo -1 > /sys/module/usbcore/parameters/autosuspend
   exit
   ```
4. With L4T 35.4.1, a new environment variable for the flash script has been introduced. If you are facing an error, which states that something might be wrong with your storage device, especially the disk space is not enough, although using 64Gb or more, then you could mount your storage device on your host system and check the number of sectors available:
   ```
   sudo fdisk -l /dev/sda
   ```
   (if your sd card is mounted as sda)<br>
   You should see a number of sectors for your device e.g.: 121536512. With the obtained number you could try to flash your Jetson device again:
   ```
   sudo ./flash.sh -a EXT_NUM_SECTORS="121536512"
   ```


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


