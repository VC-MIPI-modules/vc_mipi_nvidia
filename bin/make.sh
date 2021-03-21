#/bin/bash
#

JP=43a
CAM=IMX296_VGL

CWD=$PWD
. setup.sh $JP
cd $CWD
. build.sh $JP $CAM
cd $CWD
. flash.sh $JP