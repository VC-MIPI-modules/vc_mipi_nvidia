# /bin/bash
#

TTY=/dev/ttyUSB1
fuser -k $TTY
screen -a $TTY 115200