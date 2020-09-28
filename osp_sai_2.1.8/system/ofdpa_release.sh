#!/usr/bin/env bash

cp -r ofdpa/ /tmp/
cd /tmp/ofdpa
find . -name '.svn' -exec rm -rf {} \;
cp -r * ~/ofdpa/sdk/ofdpa/
rm -rf /tmp/ofdpa
