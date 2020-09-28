#!/bin/sh

mkdir -p gen
cd gen
/sw/pub/bin/asn1c -Werror -fnative-types -funnamed-unions \
 ../scfg.asn1 \
 2>&1| grep -v "contents unchanged" | grep -v "Cannot read file-dependencies"
exit 0

