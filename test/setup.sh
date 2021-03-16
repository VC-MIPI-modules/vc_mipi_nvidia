#!/bin/bash
#
# TODO: Jetson_release -v loggen
# TODO: Cameratyp automatisch ermitteln.
#

if [[ -z $1 ]]; then
  echo "Install nessecary test tool packages ..."
  sudo apt update
  sudo apt install v4l-utils
  chmod +x vcmipidemo
fi

gnome-terminal -e "tail -f /var/log/syslog" --geometry 200x20+0+0

if [[ -z $1 ]]; then
  . ./config/configure.sh generic
  test_standard
fi

v4l2-ctl --all

