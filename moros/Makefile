CPPFLAGS = -D__PROG_TYPES_COMPAT__
USER_LIB_PATH = .
ARDUINO_LIBS = U8glib SPI TFT
BOARD_TAG    = uno
MONITOR_PORT = /dev/ttyACM0
MONITOR_BAUDRATE = 115200

include /usr/share/arduino/Arduino.mk

U8glib.a: U8glib/U8glib.o
	ar r $@ $<
