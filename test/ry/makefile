all: ry

YM_SRC_DIR = ../../ymodem

SRCS = \
	ry.c \
	ymodem_port.c \
	$(YM_SRC_DIR)/src/ymodem.c \
	$(YM_SRC_DIR)/crc/table-driven/crc16-xmodem.c


CFLAGS = \
	-Wall \
	-g3 \
	-I. \
	-I$(YM_SRC_DIR)/src \
	-I$(YM_SRC_DIR)/crc/table-driven

# ry: ymodem_export.c Ymodem/source/ymodem.c Ymodem/source/ymodem_util.c ry.c Ymodem/crc/table-driven/crc16-xmodem.c
# 	 gcc $(CFLAGS) $^ -o $@

ry: $(SRCS)
	 gcc $(CFLAGS) $^ -o $@

