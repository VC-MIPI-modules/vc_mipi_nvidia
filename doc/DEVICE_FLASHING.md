# Flashing the device

## Initial Flash
In order to use your selected Linux4Tegra distribution, it must be flashed to the target. <br>
The call
```
./flash.sh --all
```
will perform the initial flash of the target device.

If the quickstart script has been executed, the `flash.sh` script is called automatically after the build process.<br>
Simplified structure of the quickstart script:
```
  quickstart.sh
    ./setup.sh --host
    ./build.sh --all
    ./flash.sh --all
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
   sudo ./flash.sh --all EXT_NUM_SECTORS="121536512"
   ```


## Device Tree Flash

If your system has been successfully flashed and you want to modify your camera settings (e.g. number of lanes), then you should perform the following steps:
1. Change the camera settings
   ```
   ./setup.sh --camera
   ```

2. Compile the device tree
   ```
   ./build.sh --dt
   ```
3. Flash the device tree
   ```
   ./flash.sh --dt
   ```

For flashing the device tree only, the JetPack version must be considered:

### JetPack 4 (L4T 32.7.1 - L4T 32.7.4)

This applies to Jetson Nano, Jetson Xavier NX and Jetson AGX Xavier.
The device must be in forced recovery mode! Just call:
```
./flash.sh --dt
```

### JetPack 5 (L4T 35.1.0 - L4T 35.4.1)

 For changing device trees only, there must be a differentiation between Orin and Non-Orin Targets:
  * For targets like Xavier NX and AGX Xavier, you will have to modify your /boot/extlinux/extlinux.conf on your target machine by removing the FDT entry or by commenting out with '#'. Otherwise you will have to flash your complete linux image for every device tree change to take effect.

    ```bash
    # FDT /boot/dtb/kernel_tegra194-p3668-0000-p3509-0000.dtb
    ```

    The device must be in force recovery mode!
  * For Jetson Orin Nano and Jetson Orin NX the FDT entry must be present in the /boot/extlinux/extlinux.conf file. The ./flash -d command will copy the proper file (e.g. tegra234-p3767-0003-p3768-0000-a0.dtb for OrinNano 8GB on NVIDIA DevKit) into the /boot/dtb/ directory.
  
    Therefore, the extlinux.conf FDT entry must be renamed e.g.:<br>
    from

    ```bash
    FDT /boot/dtb/kernel_tegra234-p3767-0003-p3768-0000-a0.dtb 
    ```

    to

    ```bash
    FDT /boot/dtb/tegra234-p3767-0003-p3768-0000-a0.dtb
    ```

    The device must be running. It must **not** be in force recovery mode!

### JetPack 6 (L4T 36.2.0)

This applies to Jetson Orin Nano and Jetson Orin NX. Modifications to the camera device tree are realised with device tree overlays. That means, the complete device tree can be altered by one or more overlay files, called dtbo.

When modifying the camera settings by calling
```
./setup.sh --camera
```
, the file `tegra234-p3767-camera-p3768-vc_mipi-dual.dts` will be changed.

The build step
```
./build.sh --dt
```
will generate the file `tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo` into the kernel/dtb directory on the host pc.

This overlay file has been written into uefi during the initial flash process and can be overridden by adding the OVERLAYS tag in the extlinux.conf file.
To override the uefi dtbo, copy the newly generated `tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo` from the kernel/dtb folder of your host pc into the /boot/dtb directory of your Jetson Orin Target.
Therefore, you should modify the /boot/extlinux/extlinux.conf file either by adding the following entry into the primary boot configuration:
```
      OVERLAYS /boot/tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
```
or by duplicating the whole boot entry:
```
LABEL secondary
      MENU LABEL secondary kernel
      LINUX /boot/Image
      FDT /boot/dtb/kernel_tegra234-p3768-0000+p3767-0005-nv.dtb
      INITRD /boot/initrd
      APPEND ...

      OVERLAYS /boot/tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
```
The dtbo file must be copied into the specified location before the restart.

### JetPack 6 (L4T 36.4.0)

The procedure of changing or applying a new dtbo is basically the same as in [L4T 36.2.0](/doc/DEVICE_FLASHING.md#jetpack-6-l4t-3620), but the FDT entry within the /boot/extlinux/extlinux.conf section is missing. So, both the entries - FDT and OVERLAYS - must be added into this section like this:

<pre>
LABEL secondary
      MENU LABEL secondary kernel
      LINUX /boot/Image
      FDT /boot/dtb/kernel_tegra234-p3768-0000+p3767-<b>0000</b>-nv.dtb
      INITRD /boot/initrd
      APPEND ...

      OVERLAYS /boot/tegra234-p3767-camera-p3768-vc_mipi-dual.dtbo
</pre>
