#/bin/bash
#
. config/configure.sh $1

if [[ -z $(lsusb | grep "NVIDIA Corp.") ]]; then
    echo "Recovery Mode not startet!"
fi

cd $BUILD_DIR/Linux_for_Tegra/
sudo ./flash.sh jetson-nano-emmc mmcblk0p1