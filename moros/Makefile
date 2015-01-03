CPPFLAGS = -D__PROG_TYPES_COMPAT__ -std=c++11
BOARD_TAG    = uno
MONITOR_PORT = /dev/ttyACM0
MONITOR_BAUDRATE = 115200
USER_LIB_PATH = $(PWD)/libs
ARDUINO_LIBS = FTOLED SPI
FLAG = cccp_flag.h

build: $(FLAG)
	$(MAKE) -C libs
	$(MAKE) all

include /usr/share/arduino/Arduino.mk

cccp_flag.h: cccp_flag.bmp
	python3 utils/bin2header.py $<
