#!/bin/bash

if [[ -e /tmp/Image ]]; then
        sudo mv /tmp/Image /boot
        sudo reboot
fi