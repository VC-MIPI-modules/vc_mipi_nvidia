set -e

docker build --no-cache -t docker_jp4_jp5 ./docker

docker run --privileged -v /dev/bus/usb:/dev/bus/usb -v $PWD:/src -it --device-cgroup-rule="b 7:* rmw" docker_jp4_jp5 /bin/bash
