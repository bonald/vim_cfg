#! /bin/bash

source /usr/local/ctc_python/bin/activate

python -m pip install --upgrade pip
pip install pillow
pip install flask
pip install protobuf

pip install ptyprocess
pip install pytz
pip install tests
pip install ConfigParser
pip install command
pip install eventlet
pip install google
pip install pexpect

deactivate
