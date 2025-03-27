#!/bin/bash

sudo apt update

sudo apt-get install -y v4l-utils

sudo apt-get install -y qtbase5-dev qt5-qmake

sudo apt-get install -y nvidia-l4t-jetson-multimedia-api

sudo apt-get install -y cmake build-essential pkg-config libx11-dev libgtk-3-dev libexpat1-dev libjpeg-dev libgstreamer1.0-dev

cd ~

if [[ ! -d ArgusSDK ]]
then
	mkdir ArgusSDK
fi
cd ArgusSDK

if [[ ! -d build ]]
then
       mkdir build
fi
cd build

cmake ../../../../usr/src/jetson_multimedia_api/argus/

make

sudo make install

cd ~

sudo apt-get install -y nvidia-cuda-dev

if [[ -e gst-nvarguscamera_src.tbz2 ]]
then
	tar -xf gst-nvarguscamera_src.tbz2
	cd gst-nvarguscamera
	make
	sudo make install
fi

cd ~

if [[ -e gst-nvvidconv_src.tbz2 ]]
then
	tar -xf gst-nvvidconv_src.tbz2
	cd gst-nvvidconv

	MAKE_FILE=Makefile
	INCLUDE_STR="INCLUDES += -I/usr/src/jetson_multimedia_api/include"
	if [[ -e ${MAKE_FILE} ]]
	then
		FIND_RESULT=0
		FIND_RESULT=$(grep -q "${INCLUDE_STR}" ${MAKE_FILE}; echo $?)
		if [[ 1 == $FIND_RESULT ]]
		then
			echo "Include path missing, trying to insert..."
                        sed '/INCLUDES += -I/r'<(
				echo "$INCLUDE_STR"
			) -i -- ${MAKE_FILE}
		fi
	fi
	sudo make install
fi

cd ~


