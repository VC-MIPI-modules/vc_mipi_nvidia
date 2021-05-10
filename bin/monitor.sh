# /bin/bash
#

TTY=/dev/ttyUSB0
fuser -k $TTY
minicom -D $TTY -b 115200 