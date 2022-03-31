#!/bin/bash

cd "$(dirname "$0")"

git pull

cmake . -DCMAKE_BUILD_TYPE:STRING=Release -DNO_CANLIB:BOOL=True -DTEST=OFF

make -j

sudo systemctl restart ecui-llserver.service
