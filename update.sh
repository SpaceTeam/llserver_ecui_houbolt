#!/bin/bash

cd "$(dirname "$0")"

git pull

cmake . -DCMAKE_BUILD_TYPE:STRING=Release -DTEST=OFF

make -j

sudo systemctl restart ecui-llserver-franz.service


