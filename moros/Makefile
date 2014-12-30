CPPFLAGS = -D__PROG_TYPES_COMPAT__ -std=c++11
BOARD_TAG    = micro
MONITOR_PORT = /dev/ttyACM0
MONITOR_BAUDRATE = 57600

include /usr/share/arduino/Arduino.mk

cccp_flag.h: cccp_flag.bmp
	python3 utils/bin2header.py $<
