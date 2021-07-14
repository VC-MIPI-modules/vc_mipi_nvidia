#!/bin/bash

JP_VERSION=4.5_auvidea
echo "Using Jetpack Version: $JP_VERSION" 

. config/files_JP4.5.sh

CREATE_IMAGE_PARAM="-s 8G -b jetson-nano -r 300"

CAMERAS=(IMX226 IMX226C IMX296_VGL)