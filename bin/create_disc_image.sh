#!/bin/bash
#

function create_dir() {
    IMAGE_DIR=$BUILD_DIR/disc-images 
    if [ ! -d "$IMAGE_DIR" ] ; then
        mkdir $IMAGE_DIR
    fi
}

function create_disc_image() {
    cd $BUILD_DIR/Linux_for_Tegra/tools
    sudo ./jetson-disk-image-creator.sh -o $1 $CREATE_IMAGE_PARAM
    mv $BUILD_DIR/Linux_for_Tegra/tools/$1 $IMAGE_DIR
}

if [[ -z $2 ]]; then
    . config/configure.sh $1
    create_dir
    create_disc_image "vc-mipi-nano-jp$1-generic.img"

elif [[ "$2" == "all" ]]; then
    DIR=$PWD
    . config/configure.sh $1
    create_dir
    echo "Creating images for all camera models ..."
    for CREATE_MODEL in "${CAMERAS[@]}"; do
        cd $DIR
        . build.sh $1 $CREATE_MODEL
        create_disc_image "vc-mipi-nano-jp$1-$CREATE_MODEL.img"
    done

else
    . build.sh $1 $2
    create_dir
    create_disc_image "vc-mipi-nano-jp$1-$2.img"
fi