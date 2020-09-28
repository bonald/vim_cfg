#!/bin/sh

ASN1C=$TOP_DIR/lib/msg/asn1c
$ASN1C -Werror -fnative-types -funnamed-unions \
../lcm.asn1 \
../lcm_debug.asn1 \
2>&1| grep -v "contents unchanged" | grep -v "Cannot read file-dependencies"
