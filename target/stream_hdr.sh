#!/bin/bash

v4l2-ctl -d /dev/video0 -c black_level=1

v4l2-ctl -d /dev/video0 -c hdr_mode=1

v4l2-ctl -d /dev/video0 -c frame_rate=60000
v4l2-ctl -d /dev/video0 -c parameter_set=0
v4l2-ctl -d /dev/video0 -c exposure=4000
v4l2-ctl -d /dev/video0 --stream-mmap --stream-count=5 --stream-to=/home/vc/ram/shs1.raw & 

v4l2-ctl -d /dev/video0 -c parameter_set=1 
v4l2-ctl -d /dev/video0 -c exposure=1500
v4l2-ctl -d /dev/video1 --stream-mmap --stream-count=5 --stream-to=/home/vc/ram/shs0.raw &



