#!/bin/bash

JP_VERSION=4.3_auvidea
echo "Using Jetpack Version: $JP_VERSION" 

. config/files_JP4.3.sh

CREATE_IMAGE_PARAM="-s 8G -b jetson-nano -r 300"

CAMERAS=(IMX226 IMX226C IMX296_VGL)