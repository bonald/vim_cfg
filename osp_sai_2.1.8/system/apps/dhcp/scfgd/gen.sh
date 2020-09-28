#!/bin/sh

mkdir -p gen
cd gen
/sw/pub/bin/asn1c -Werror -fnative-types -funnamed-unions \
 ../SmartDeploy.asn1 \
 2>&1| grep -v "contents unchanged" | grep -v "Cannot read file-dependencies"

cat > Makefile.ctc << EOF
include \$(MK_DIR)/sys.mk

LIB = scparser
SRCS = \$(wildcard *.c)

CPPFLAGS += -I\$(TOP_DIR)/lib/libasn1
CPPFLAGS += -I./

include \$(MK_DIR)/lib.mk
EOF

exit 0
