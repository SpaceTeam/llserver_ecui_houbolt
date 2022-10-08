#!/bin/bash

cd "$(dirname "$0")"

git pull

cmake . -DCMAKE_BUILD_TYPE:STRING=Debug -DNO_CANLIB:BOOL=True -DNO_PYTHON:BOOL=True

make -j3

sudo systemctl restart ecui-llserver.service
