#!/bin/sh

#$1 nv2
#$2 keydir
#$3 aeskey

NV2=$1
KEYCNT=$2
KEYDIR=$3
AESKEY=$4
TOP_DIR=`pwd`

cd ..
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig 
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image -j16 
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- dtbs -j16 
cd scripts

if [ $AESKEY != '-' ]; then
$TOP_DIR/../ctcsign -E -s $TOP_DIR/../arch/arm64/boot/Image -k $AESKEY
$TOP_DIR/../ctcsign -D -s $TOP_DIR/../arch/arm64/boot/Image -k $AESKEY
$TOP_DIR/../ctcsign -C 2 -n $KEYCNT -c $NV2 -s $TOP_DIR/../arch/arm64/boot/Image_pad -e 1 -d $KEYDIR
else
$TOP_DIR/../ctcsign -C 2 -n $KEYCNT -c $NV2 -s $TOP_DIR/../arch/arm64/boot/Image -d $KEYDIR
fi

mv $TOP_DIR/cert2.txt $TOP_DIR/../kernel.cert
aarch64-linux-gnu-objcopy -I binary -O binary --pad-to=0xa00 --gap-fill=0xff $TOP_DIR/../kernel.cert

if [ $AESKEY != "-" ]; then
$TOP_DIR/../ctcsign -E -s $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb -k $AESKEY
$TOP_DIR/../ctcsign -D -s $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb -k $AESKEY
$TOP_DIR/../ctcsign -C 2 -n $KEYCNT -c $NV2 -s $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb_pad -e 1 -d $KEYDIR
else
$TOP_DIR/../ctcsign -C 2 -n $KEYCNT -c $NV2 -s $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb -d $KEYDIR
fi

mv $TOP_DIR/cert2.txt $TOP_DIR/../dtb.cert
aarch64-linux-gnu-objcopy -I binary -O binary --pad-to=0xa00 --gap-fill=0xff $TOP_DIR/../dtb.cert

if [ $AESKEY != "-" ]; then
cat $TOP_DIR/../kernel.cert $TOP_DIR/../arch/arm64/boot/Image_enc > $TOP_DIR/../ctc5236-image-secure-enc.bin
cat $TOP_DIR/../dtb.cert $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb_enc > $TOP_DIR/../ctc5236-emu-secure-enc.dtb
else
cat $TOP_DIR/../kernel.cert $TOP_DIR/../arch/arm64/boot/Image > $TOP_DIR/../ctc5236-image-secure.bin
cat $TOP_DIR/../dtb.cert $TOP_DIR/../arch/arm64/boot/dts/centec/ctc5236-emu.dtb > $TOP_DIR/../ctc5236-emu-secure.dtb
fi