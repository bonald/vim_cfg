CPPFLAGS += -DTSINGMA
CPPFLAGS += -DUSW

ifeq ($(CHIPNAME),tsingma)
CPPFLAGS += -DTSINGMA
CPPFLAGS += -DUSW
endif

ifeq ($(BOARD),ctc-sim)
CPPFLAGS += -I/opt/openssl-1.0.2c/usr/local/include/openssl
endif

CPPFLAGS += -I$(SDK_DIR)/driver/api

ifeq ($(BOARD),ctc-sim)
ifeq ($(M64),TRUE)
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib64
else
LIB_LCRYPTO_DIR := /opt/openssl-1.0.2c/usr/local/lib
endif
endif

#CFLAGS += -Wno-unused


ifeq ($(BOARD),ctc-sim)
LD_LIBS += -L$(LIB_LCRYPTO_DIR) -lcrypto -ldl -lstdc++
endif
