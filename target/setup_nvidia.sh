#!/bin/bash

# L4T major release: R36 = JetPack 6, R38/R39 = JetPack 7
L4T_MAJOR=$(sed -n 's/^# R\([0-9]*\) (release).*/\1/p' /etc/nv_tegra_release 2>/dev/null)
JETPACK7=0
if [[ -n "$L4T_MAJOR" && "$L4T_MAJOR" -ge 38 ]]
then
	JETPACK7=1
fi
echo "L4T R${L4T_MAJOR:-?} -> JetPack $([[ $JETPACK7 == 1 ]] && echo 7 || echo "6 or older")"

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

if [[ $JETPACK7 == 1 ]]
then
	# JetPack 7: CUDA runtime headers are already there, only the crt headers are missing
	# (cuda_runtime.h includes crt/host_config.h). Install the package matching the CUDA version.
	CUDA_PKG_VER=$(dpkg-query -W -f='${Package}\n' 'cuda-cudart-dev-*' 2>/dev/null \
		| sed -n 's/^cuda-cudart-dev-//p' | sort -V | tail -1)
	if [[ -n "$CUDA_PKG_VER" ]]
	then
		sudo apt-get install -y cuda-crt-${CUDA_PKG_VER}
	else
		echo "cuda-cudart-dev not found, falling back to nvidia-cuda-dev"
		sudo apt-get install -y nvidia-cuda-dev
	fi
else
	sudo apt-get install -y nvidia-cuda-dev
fi

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

	MAKE_ARGS=()
	if [[ $JETPACK7 == 1 ]]
	then
		# JetPack 7: libnvbufsurface/libnvbufsurftransform are in .../nvidia/ instead of
		# .../tegra/, and libcudart needs the CUDA library path for linking
		NV_LIB_DIR=/usr/lib/aarch64-linux-gnu/nvidia/
		MAKE_ARGS=(LIB_INSTALL_DIR=${NV_LIB_DIR}
			"LDFLAGS=-Wl,--no-undefined -L${NV_LIB_DIR} -L/usr/local/cuda/lib64 -Wl,-rpath,${NV_LIB_DIR}")
	fi
	sudo make install "${MAKE_ARGS[@]}"
fi

cd ~


