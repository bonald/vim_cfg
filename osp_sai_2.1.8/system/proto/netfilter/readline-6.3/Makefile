include ../../../Makefile.env.inc

include $(MK_DIR)/sys.mk
include $(PROTO_DIR)/Makefile.inc

LIB = readline

FILES := $(wildcard *.c)
SRCS = $(filter-out emacs_keymap.c vi_keymap.c, $(FILES))

CFLAGS += -I.
CFLAGS += "-Wno-error"

include $(MK_DIR)/solib.mk
