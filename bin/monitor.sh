# /bin/bash
#

TTY=/dev/ttyUSB0
fuser -k $TTY
screen -a $TTY 115200