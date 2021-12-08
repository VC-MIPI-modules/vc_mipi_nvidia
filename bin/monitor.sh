# /bin/bash
#

device=0
if [[ -n $1 ]]; then
        device=$1
fi

TTY=/dev/ttyUSB${device}
fuser -k $TTY
screen -a $TTY 115200