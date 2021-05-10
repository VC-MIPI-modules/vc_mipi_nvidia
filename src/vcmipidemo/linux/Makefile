################################################################################
# Makefile
#
#   Demo code for MIPI sensor modules - Vision Components GmbH Ettlingen 2018
#
# Author: Martin Engel (MBE)
#
################################################################################
CC=aarch64-linux-gnu-gcc
CPP=aarch64-linux-gnu-g++

.PHONY: all


all: vcimgnetsrv vcmipidemo

clean:
	-rm vcimgnetsrv vcmipidemo

vcimgnetsrv:  vcimgnetsrv.c vclib-excerpt.c
	$(CC) -ggdb -Wall -O3  -o vcimgnetsrv vcimgnetsrv.c vclib-excerpt.c -lrt
	
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "
	@echo "  Start vcimgnetsrv in the background once,                            "
	@echo "  then you can connect to it using  vcimgnetclient.py  (needs PyGTK).  "
	@echo "                                                                       "
	@echo "  You can start vcimgnetsrv in the background by calling:              "
	@echo "                                                                       "
	@echo "     ./vcimgnetsrv &                                                   "
	@echo "                                                                       "
	@echo "  To stop it, you can use pkill:                                       "
	@echo "                                                                       "
	@echo "     pkill vcimgnetsrv                                                 "
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "


vcmipidemo:  vcmipidemo.c vcimgnet.c vclib-excerpt.c
	$(CC) -ggdb -Wall -O3 -fopenmp -o vcmipidemo vcmipidemo.c vcimgnet.c vclib-excerpt.c -lrt
	
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "
	@echo "  The demo per default outputs as ascii art.                           "
	@echo "  Call the demo with the command line option -? to see other outputs:  "
	@echo "                                                                       "
	@echo "     ./vcmipidemo -?                                                   "
	@echo "                                                                       "
	@echo "  If you cannot see anything, try to set the shutter and gain values.  "
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "


vcmipidemo-opencv:  vcmipidemo.c vcimgnet.c vclib-excerpt.c
	
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "
	@echo "  You may need to install opencv first, at Debian like systems         "
	@echo "  you can do so by calling:                                            "
	@echo "                                                                       "
	@echo "     sudo  apt install  libopencv-dev                                  "
	@echo "                                                                       "
	@echo "  This Makefile includes and links to openCV using pkg-config.         "
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "
	
	$(CPP) -ggdb -Wall -O3 -fopenmp -o vcmipidemo  `pkg-config --cflags --libs opencv` -DWITH_OPENCV vcmipidemo.c vcimgnet.c vclib-excerpt.c -lrt
	
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "
	@echo "  The demo per default outputs as ascii art.                           "
	@echo "  Call the demo with the command line option -? to see other outputs:  "
	@echo "                                                                       "
	@echo "     ./vcmipidemo -?                                                   "
	@echo "                                                                       "
	@echo "  If you cannot see anything, try to set the shutter and gain values.  "
	@echo "                                                                       "
	@echo "-----------------------------------------------------------------------"
	@echo "                                                                       "

